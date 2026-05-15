#define DT_DRV_COMPAT nordic_nrf_saadct

#include <stdlib.h>
#include <soc.h>
#include <zephyr/kernel.h>
#include <helpers/nrfx_gppi.h>
#include <nrfx_timer.h>
#include <assert.h>
#include <zephyr/irq.h>
#include <saadct.h>
#include <zephyr/dt-bindings/gpio/nordic-nrf-gpio.h>
#include <stdlib.h>
#include <soc.h>
#include <zephyr/kernel.h>
#include <helpers/nrfx_gppi.h>
#include <nrfx_timer.h>
#include <assert.h>
#include <zephyr/irq.h>
#include <zephyr/dt-bindings/gpio/nordic-nrf-gpio.h>
#include <zephyr/drivers/gpio.h>
#include <nrfx_saadc.h>
#include <hal/nrf_gpio.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/clock_control/nrf_clock_control.h>

#define INTERNAL_TIMER_FREQ_MHZ 16
K_FIFO_DEFINE(saadct_fifo);

#define TIME_TO_WAIT_US(timer_freq, sample_rate) (uint32_t)(timer_freq / sample_rate)

static const struct device *saadc_device;

struct saadct_meas_block {
	void *fifo_reserved;
	uint16_t data[];
};

typedef enum {
	idle,
	running,
	stopping,
	error,
} state_t;

struct saadct_drv_data {
	nrfx_timer_t timer;
	nrfx_gppi_handle_t ppi_sample;
	uint8_t num_of_channels;
	nrf_saadc_resolution_t resolution;
	uint32_t sample_rate;
	struct saadct_meas_block *curr_block;
	struct saadct_meas_block *next_block;
	atomic_t pending_series;
	void (*user_handler)(void * context);
	void *user_context;
	struct k_mem_slab *mem_slab;
	volatile state_t state;
	saadct_mode_t mode;
	uint32_t num_of_meas;
	nrfx_saadc_channel_t const * channels_config;
};

struct saadct_drv_cfg {

};

static struct onoff_manager *clk_mgr;
static struct onoff_client clk_cli;
static volatile bool clock_requested;

static void saadct_handler(nrfx_saadc_evt_t const *p_event);

/* Callback for clock request. */
static void clock_started_callback(struct onoff_manager *mgr,
				   struct onoff_client *cli,
				   uint32_t state,
				   int res)
{
	(void)mgr;
	(void)cli;
	(void)state;
	(void)res;
	int ret;
	struct saadct_drv_data *p_data = saadc_device->data;

	ret = nrfx_saadc_offset_calibrate(saadct_handler);
	if (ret < 0) {
		printf("Failed to calibrate SAADC");
		p_data->state = error;
		return;
	}
	clock_requested = true;
}

static int hf_clock_request(void)
{
	sys_notify_init_callback(&clk_cli.notify, clock_started_callback);
	clock_control_subsys_t subsys = CLOCK_CONTROL_NRF_SUBSYS_HF;

	clk_mgr = z_nrf_clock_control_get_onoff(subsys);
	return onoff_request(clk_mgr, &clk_cli);
}

static int hf_clock_release(void)
{
	return onoff_release(clk_mgr);
}

static void gppi_enable(const struct device *dev)
{
	struct saadct_drv_data *p_data = dev->data;

	nrfx_gppi_conn_enable(p_data->ppi_sample);
}

static void gppi_disable(const struct device *dev)
{
	struct saadct_drv_data *p_data = dev->data;

	nrfx_gppi_conn_disable(p_data->ppi_sample);
}

static void meas_stop(const struct device *dev)
{
	struct saadct_drv_data *p_data = dev->data;

	nrfx_timer_disable(&p_data->timer);
	gppi_disable(dev);
	if (clock_requested) {
		(void)hf_clock_release();
	}
}

static struct saadct_meas_block * buffer_prepare(const struct device *dev)
{
	int ret;
	struct saadct_drv_data *p_data = dev->data;
	struct saadct_meas_block *block;

	ret = k_mem_slab_alloc(p_data->mem_slab, (void **)&block, K_NO_WAIT);
	if (ret < 0) {
		printf("Failed to allocate TX block: %d\n", ret);
			return NULL;
	}

	ret = nrfx_saadc_buffer_set(block->data, p_data->num_of_meas * p_data->num_of_channels);
	if (ret < 0) {
		p_data->state = error;
		return NULL;
	}
	return block;
}

static void saadct_handler(nrfx_saadc_evt_t const *p_event)
{
	int status;
	struct saadct_drv_data *p_data = saadc_device->data;

	switch (p_event->type) {
	case NRFX_SAADC_EVT_CALIBRATEDONE:
		status = nrfx_saadc_mode_trigger();
		if (status < 0) {
			p_data->state = error;
			meas_stop(saadc_device);
		}
		break;

	case NRFX_SAADC_EVT_READY:
		nrfx_timer_clear(&p_data->timer);
		nrfx_timer_enable(&p_data->timer);
		gppi_enable(saadc_device);
		break;

	case NRFX_SAADC_EVT_BUF_REQ:
		if (p_data->mode == saadct_mode_continuous && p_data->state == running) {
			p_data->next_block = buffer_prepare(saadc_device);
			if (p_data->next_block == NULL) {
				meas_stop(saadc_device);
				p_data->state = error;
				break;
			}
		}
		break;

	case NRFX_SAADC_EVT_DONE:
		k_fifo_put(&saadct_fifo, p_data->curr_block);
		atomic_inc(&p_data->pending_series);
		p_data->curr_block = p_data->next_block;
		break;

	case NRFX_SAADC_EVT_FINISHED:
		meas_stop(saadc_device);
		break;

	default:
		break;
	}
}

static int timer_init(const struct device *dev)
{
	int rc;
	struct saadct_drv_data *p_data = dev->data;
	uint32_t sample_rate = p_data->sample_rate;
	nrfx_timer_t * timer = &p_data->timer;
	nrfx_timer_config_t timer_cfg = {
		.frequency = NRFX_MHZ_TO_HZ(INTERNAL_TIMER_FREQ_MHZ),
		.mode = NRF_TIMER_MODE_TIMER,
		.bit_width = NRF_TIMER_BIT_WIDTH_32,
		.interrupt_priority = NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,
		.p_context = NULL
	};
	uint32_t desired_ticks = nrfx_timer_us_to_ticks(timer, TIME_TO_WAIT_US(timer_cfg.frequency, sample_rate));

    nrfx_timer_extended_compare(timer,
                                NRF_TIMER_CC_CHANNEL0,
                                desired_ticks,
                                NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                false);

    rc = nrfx_timer_init(timer, &timer_cfg, NULL);
	if (rc < 0) {
		return rc;
	}
	nrfx_timer_clear(timer);
	return rc;
}

static int saadc_init(const struct device *dev)
{
	int rc;
	struct saadct_drv_data *p_data = dev->data;

	IRQ_CONNECT(DT_INST_IRQN(0), DT_INST_IRQ(0, priority), nrfx_isr, nrfx_saadc_irq_handler, 0); 
	rc = nrfx_saadc_init(NRFX_SAADC_DEFAULT_CONFIG_IRQ_PRIORITY);
	if (rc < 0) {
		return rc;
	}

	rc = nrfx_saadc_channels_config(p_data->channels_config, p_data->num_of_channels);
	if (rc < 0) {
		return rc;
	}

	/*
	 * Setting the advanced configuration with triggering sampling by the internal timer
	 * disabled (internal_timer_cc = 0) and without software start task on end event
	 * (start_on_end = false).
	 */
	nrfx_saadc_adv_config_t adv_config = NRFX_SAADC_DEFAULT_ADV_CONFIG;
	adv_config.internal_timer_cc = 0;
	adv_config.start_on_end = false;

	uint32_t channel_mask = nrfx_saadc_channels_configured_get();
	rc = nrfx_saadc_advanced_mode_set(channel_mask, p_data->resolution, &adv_config, saadct_handler);
	if (rc < 0) {
		return rc;
	}

	return rc;
}

static int gppi_init(const struct device *dev)
{
	struct saadct_drv_data *p_data = dev->data;
	uint32_t evt = nrfx_timer_event_address_get(&p_data->timer, NRF_TIMER_EVENT_COMPARE0);;
	uint32_t tsk = nrfy_saadc_task_address_get(NRF_SAADC, NRF_SAADC_TASK_SAMPLE);
	int ret;

	ret = nrfx_gppi_conn_alloc(evt, tsk, &p_data->ppi_sample);

	return ret;
}

static int saadct_nrfx_configure(const struct device *dev, struct api_saadct_config * cfg)
{
	struct saadct_drv_data *p_data = dev->data;
	int ret;	

	p_data->num_of_channels = cfg->num_of_channels;
	p_data->resolution = cfg->resolution;
	p_data->sample_rate = cfg->sample_rate;
	p_data->user_handler = cfg->user_handler;
	p_data->user_context = cfg->user_context;
	p_data->num_of_meas = cfg->num_of_meas;
	p_data->mode = cfg->mode;
	saadc_device = dev;
	p_data->channels_config = cfg->channels_config;

	ret = timer_init(dev);
	if (ret < 0) {
		return ret;
	}

	ret = gppi_init(dev);
	if (ret < 0) {
		return ret;
	}

	ret = saadc_init(dev);

	return ret;
}

static int saadct_nrfx_start(const struct device *dev, struct k_mem_slab * slab)
{
	struct saadct_drv_data *p_data = dev->data;
	int ret;

	ret = hf_clock_request();
	if (ret < 0) {
		return ret;
	}

	p_data->mem_slab = slab;
	p_data->curr_block = buffer_prepare(dev);
	if (p_data->curr_block == NULL) {
		return -ENOMEM;
	}

	if (p_data->mode == saadct_mode_one_shot) {
		p_data->state = stopping;
	} else {
		p_data->state = running;
	}

	NRF_SAADC->SHORTS = (SAADC_SHORTS_END_START_Enabled << SAADC_SHORTS_END_START_Pos);

	return 0;
}

static int saadct_nrfx_stop(const struct device *dev, bool wait_until_series_finished)
{
	struct saadct_drv_data *p_data = dev->data;
	struct saadct_meas_block *block = p_data->curr_block;

	if (!wait_until_series_finished) {
		meas_stop(dev);
		k_mem_slab_free(p_data->mem_slab, block);
		return 0;
	}

	p_data->state = stopping;
	return 0;
}

static int saadct_nrfx_meas_get(const struct device *dev, uint16_t ** data)
{
	struct saadct_meas_block *block;
	struct saadct_drv_data *p_data = dev->data;

	block = k_fifo_get(&saadct_fifo, K_NO_WAIT);
	if (!block) {
		if (k_mem_slab_num_used_get(p_data->mem_slab) > 0) {
			return -EAGAIN;
		}
		return -EIO;
	}
	atomic_dec(&p_data->pending_series);
	*data = block->data;

	return 0;
}

static void saadct_nrfx_meas_free(const struct device *dev, uint16_t *data)
{
	struct saadct_drv_data *p_data = dev->data;

	struct saadct_meas_block *block =
		CONTAINER_OF((void *)data, struct saadct_meas_block, data);

	k_mem_slab_free(p_data->mem_slab, block);
}

static uint32_t saadct_nrfx_meas_pending(const struct device *dev)
{
    struct saadct_drv_data *p_data = dev->data;
    return atomic_get(&p_data->pending_series);
}

static const struct saadct_driver_api saadct_nrf_drv_api = {
	.configure = saadct_nrfx_configure,
	.start = saadct_nrfx_start,
	.stop = saadct_nrfx_stop,
	.meas_get = saadct_nrfx_meas_get,
	.meas_free = saadct_nrfx_meas_free,
	.meas_pending = saadct_nrfx_meas_pending,
};

#define TIMER_NODE(instance) DT_INST_PHANDLE(instance, timer_instance)

#define NRFX_SAADCT_DEVICE(inst)									\
static struct saadct_drv_data drv_data_##inst = {					\
	.timer = NRFX_TIMER_INSTANCE(DT_REG_ADDR(TIMER_NODE(inst))),	\
};																	\
static const struct saadct_drv_cfg drv_cfg_##inst = {				\
};																	\
static int saadct_##inst##_init(const struct device *dev)			\
{																	\
	IRQ_CONNECT(DT_IRQN(TIMER_NODE(inst)), IRQ_PRIO_LOWEST,			\
                nrfx_timer_irq_handler, &drv_data_##inst.timer, 0); \
    return 0;														\
}																	\
DEVICE_DT_INST_DEFINE(inst, saadct_##inst##_init,					\
					  NULL,											\
					  &drv_data_##inst,								\
					  &drv_cfg_##inst,								\
					  POST_KERNEL,									\
					  50,											\
					  &saadct_nrf_drv_api);

NRFX_SAADCT_DEVICE(0)
