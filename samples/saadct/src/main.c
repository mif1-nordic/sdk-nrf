#include <zephyr/kernel.h>
#include <assert.h>
#include <saadct.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/dt-bindings/gpio/nordic-nrf-gpio.h>

#define NUMBER_OF_MEASUREMENTS 100
#define NUMBER_OF_SERIES 10

#define SAADCT_CHAN_DEFAULT_CONFIG_SE(_pin, _idx, _tacq, _gain, _ref)  \
{                                                                      \
    .channel_config =                                                  \
    {                                                                  \
		.gain = _gain,												   \
        NRFX_COND_CODE_1(NRF_SAADC_HAS_CH_CONFIG_RES,                  \
                         (.resistor_p = NRF_SAADC_RESISTOR_DISABLED,   \
                          .resistor_n = NRF_SAADC_RESISTOR_DISABLED,), \
                         ())                                           \
        .reference  = _ref,                    						   \
        .conv_time = NRFX_SAADC_DEFAULT_CONV_TIME,                     \
        .acq_time   = _tacq,                                           \
        .mode       = NRF_SAADC_MODE_SINGLE_ENDED,                     \
    },                                                                 \
    .pin_p         = (nrfx_analog_input_t)_pin,                        \
    .pin_n         = NRFX_ANALOG_INPUT_DISABLED,                       \
    .channel_index = _idx,                                             \
}

static const nrfx_saadc_channel_t saadct_chan_cfg[] =
{
    SAADCT_CHAN_DEFAULT_CONFIG_SE(NRFX_ANALOG_EXTERNAL_AIN6, 0, 23, NRF_SAADC_GAIN1, NRF_SAADC_REFERENCE_INTERNAL),
    SAADCT_CHAN_DEFAULT_CONFIG_SE(NRFX_ANALOG_EXTERNAL_AIN5, 1, 23, NRF_SAADC_GAIN1_4, NRF_SAADC_REFERENCE_INTERNAL),
};

K_MEM_SLAB_DEFINE(my_meas_slab,
				  8 + NUMBER_OF_MEASUREMENTS * sizeof(uint16_t) * ARRAY_SIZE(saadct_chan_cfg),
				  128,
				  4);

uint16_t * my_data;

#define ADC_DEVICE DT_NODELABEL(adc)
const struct device *saadct = DEVICE_DT_GET(ADC_DEVICE);

void saadct_handler(void * context)
{
	(void)context;
	static uint32_t handler_cnt = 0;
	printf("Handler %u\n", handler_cnt);
	handler_cnt ++;
}

struct api_saadct_config saadct_cfg = {
	.num_of_channels = ARRAY_SIZE(saadct_chan_cfg),
	.num_of_meas = NUMBER_OF_MEASUREMENTS,
	.resolution = NRF_SAADC_RESOLUTION_10BIT,
	.mode = saadct_mode_continuous,
	.sample_rate = 10000,
	.user_handler = saadct_handler,
	.user_context = NULL,
	.channels_config = saadct_chan_cfg,
};

int main(void)
{
	int ret;
	uint32_t series_cnt = 0;

	printf("SAADC sample start.\n");
	if (!device_is_ready(saadct)) {
    	printk("SAADCT not ready\n");
	}

	ret = saadct_configure(saadct, &saadct_cfg);
	if (ret < 0) {
		printf("Error initializing SAADCT. \n");
	}

	ret = saadct_start(saadct, &my_meas_slab);
	if (ret < 0) {
		printf("Error starting SAADCT. \n");
	}
	k_msleep(100);
	saadct_stop(saadct, true);
	printk(" - %u measurements left\n", saadct_meas_pending(saadct));
	do {
		ret = saadct_meas_get(saadct, &my_data);
		if (ret >= 0) {
			for (size_t i = 0; i < NUMBER_OF_MEASUREMENTS * ARRAY_SIZE(saadct_chan_cfg); i++) {
				printf("d[%u][%u] = %04X\n", i, series_cnt, my_data[i]);
			}
			saadct_meas_free(saadct, my_data);
			series_cnt ++;
		}
		else if (ret != -EAGAIN) {
			printk("saadct_meas_get returned %d", ret);
			// Wait for the last series to be finished
			break;
		}
		
	} while (series_cnt < NUMBER_OF_SERIES);
	
	do {
		do {
			ret = saadct_meas_get(saadct, &my_data);
		} while(ret == -EAGAIN);
		if (ret >= 0) {
			for (size_t i = 0; i < NUMBER_OF_MEASUREMENTS * ARRAY_SIZE(saadct_chan_cfg); i++)
			{
				printf("d[%u][%u] = %04X\n", i, series_cnt, my_data[i]);
				k_msleep(10);
			}
			
			saadct_meas_free(saadct, my_data);
			printk(" - %u measurements left\n", saadct_meas_pending(saadct));
			series_cnt ++;
		} else if (ret == -EAGAIN) {
			printk("saadct_meas_get returned %d", ret);
			// Wait for the last series to be finished
			ret = 0;
		}
	} while(saadct_meas_pending(saadct) > 0);

	printk("Meas finished.\n");
	while (1) {
		k_msleep(100);
	}
}
