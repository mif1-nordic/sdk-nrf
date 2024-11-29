/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "hrt/hrt.h"

#include <zephyr/kernel.h>
#include <zephyr/ipc/ipc_service.h>
#include <zephyr/drivers/mspi.h>

#include <hal/nrf_vpr_csr.h>
#include <hal/nrf_vpr_csr_vio.h>
#include <haly/nrfy_gpio.h>

#include <drivers/mspi/nrfe_mspi.h>
#include <zephyr/drivers/mspi.h>

#define MAX_DATA_LEN 256

#define HRT_IRQ_PRIORITY	   2
#define HRT_VEVIF_IDX_WRITE_SINGLE 17
#define HRT_VEVIF_IDX_WRITE_QUAD   18

#define VEVIF_IRQN(vevif)   VEVIF_IRQN_1(vevif)
#define VEVIF_IRQN_1(vevif) VPRCLIC_##vevif##_IRQn

struct mspi_config {
	uint8_t *data;
	uint8_t data_len;
	uint8_t word_size;
};

struct mspi_dev_config {
	enum mspi_io_mode io_mode;
	enum mspi_ce_polarity ce_polarity;
	uint32_t read_cmd;
	uint32_t write_cmd;
	uint8_t cmd_length;  /* Command length in bits. */
	uint8_t addr_length; /* Address length in bits. */
};

static struct mspi_dev_config mspi_dev_configs;

uint32_t data_buffer[MAX_DATA_LEN + 2];

volatile uint8_t counter_top = 4;
volatile uint8_t word_size;
volatile uint32_t *data_to_send;
volatile uint8_t data_len;
volatile uint8_t ce_hold;

static struct ipc_ept ep;
static atomic_t ipc_atomic_sem = ATOMIC_INIT(0);

void process_packet(const void *data, size_t len);

static void ep_bound(void *priv)
{
	atomic_set_bit(&ipc_atomic_sem, NRFE_MSPI_EP_BOUNDED);
}

static void ep_recv(const void *data, size_t len, void *priv)
{
	(void)priv;

	process_packet(data, len);
}

static struct ipc_ept_cfg ep_cfg = {
	.cb =
		{
			.bound = ep_bound,
			.received = ep_recv,
		},
};
static uint8_t custom_data[30] = {0xa3, 0x21, 0x54, 0x3a, 0x55, 0xa5, 0x45, 0x35, 0x34, 0x23,
			    0xa3, 0xad, 0x97, 0xb2, 0x56, 0x54, 0x38, 0x88, 0x0,  0x5,
			    0x33, 0x6,	0x34, 0x6,  0x57, 0x7,	0xbb, 0xba, 0xa3, 0xf6};

void process_packet(const void *data, size_t len)
{
	(void)len;
	nrfe_mspi_flpr_response_t response;
	uint8_t *buffer = (uint8_t *)data;
	uint8_t opcode = *buffer++;

	response.opcode = opcode;

	switch (opcode) {
	case NRFE_MSPI_CONFIG_PINS: {
		nrfe_mspi_pinctrl_soc_pin_t *pins_cfg = (nrfe_mspi_pinctrl_soc_pin_t *)buffer;

		for (uint8_t i = 0; i < NRFE_MSPI_PINS_MAX; i++) {
			uint32_t psel = NRF_GET_PIN(pins_cfg->pin[i]);
			uint32_t fun = NRF_GET_FUN(pins_cfg->pin[i]);
			NRF_GPIO_Type *reg = nrf_gpio_pin_port_decode(&psel);
			/* TODO: Process pinctrl config data */

			
		}
		custom_data[0] = NRFE_MSPI_CONFIG_PINS;
		prepare_and_send_data(custom_data, 1);
		break;
	}
	case NRFE_MSPI_CONFIG_CTRL: {
		struct mspi_cfg *cfg = (struct mspi_cfg *)buffer;
		/* TODO: Process controller config data */
		custom_data[0] = NRFE_MSPI_CONFIG_CTRL;
		prepare_and_send_data(custom_data, 1);
		break;
	}
	case NRFE_MSPI_CONFIG_DEV: {
		struct mspi_dev_cfg *cfg = (struct mspi_dev_cfg *)buffer;
		/* TODO: Process device config data */
		custom_data[0] = NRFE_MSPI_CONFIG_DEV;
		prepare_and_send_data(custom_data, 1);
		break;
	}
	case NRFE_MSPI_CONFIG_XFER: {
		struct mspi_xfer *cfg = (struct mspi_xfer *)buffer;
		/* TODO: Process xfer config data */
		custom_data[0] = NRFE_MSPI_CONFIG_XFER;
		prepare_and_send_data(custom_data, 1);
		break;
	}
	case NRFE_MSPI_TX: {
		struct mspi_xfer_packet *packet = (struct mspi_xfer_packet *)buffer;

		custom_data[0] = NRFE_MSPI_TX;
		prepare_and_send_data(custom_data, 4);
		
		for(int i=0; i<29 && i<len; i++)
		{
			custom_data[i+1] = buffer[i];
		}

		prepare_and_send_data(custom_data, len>30 ? 30 : len);

		if(packet->dir == MSPI_TX) {
			custom_data[1] = 0;
		} else {
			custom_data[1] = 1;
		}
		//custom_data[2] = packet->data_buf;
		//custom_data[3] = packet->num_bytes;
		prepare_and_send_data(custom_data, 4);
		break;
	}
	case NRFE_MSPI_TXRX: {
		struct mspi_xfer_packet *packet = (struct mspi_xfer_packet *)buffer;

		if (packet->dir == MSPI_RX) {
			/* TODO: Process received data */
		} else if (packet->dir == MSPI_TX) {
			/* TODO: Send data */
			prepare_and_send_data(packet->data_buf, packet->num_bytes);
		}
		custom_data[0] = NRFE_MSPI_TXRX;
		custom_data[1] = packet->dir;
		custom_data[2] = packet->data_buf;
		custom_data[3] = packet->num_bytes;
		prepare_and_send_data(custom_data, 4);
		break;
	}
	default:
		response.opcode = NRFE_MSPI_WRONG_OPCODE;
		break;
	}

	ipc_service_send(&ep, (const void *)&response.opcode, sizeof(response));
}

void configure_clock(enum mspi_cpp_mode cpp_mode)
{
	nrf_vpr_csr_vio_config_t vio_config = {
		.input_sel = 0,
		.stop_cnt = 0,
	};

	nrf_vpr_csr_vio_dir_set(PIN_DIR_OUT_MASK(VIO(NRFE_MSPI_SCK_PIN_NUMBER)));

	switch (cpp_mode) {
	case MSPI_CPP_MODE_0: {
		vio_config.clk_polarity = 0;
		nrf_vpr_csr_vio_out_set(PIN_OUT_LOW_MASK(VIO(NRFE_MSPI_SCK_PIN_NUMBER)));
		break;
	}
	case MSPI_CPP_MODE_1: {
		vio_config.clk_polarity = 1;
		nrf_vpr_csr_vio_out_set(PIN_OUT_LOW_MASK(VIO(NRFE_MSPI_SCK_PIN_NUMBER)));
		break;
	}
	case MSPI_CPP_MODE_2: {
		vio_config.clk_polarity = 1;
		nrf_vpr_csr_vio_out_set(PIN_OUT_HIGH_MASK(VIO(NRFE_MSPI_SCK_PIN_NUMBER)));
		break;
	}
	case MSPI_CPP_MODE_3: {
		vio_config.clk_polarity = 0;
		nrf_vpr_csr_vio_out_set(PIN_OUT_HIGH_MASK(VIO(NRFE_MSPI_SCK_PIN_NUMBER)));
		break;
	}
	}

	nrf_vpr_csr_vio_config_set(&vio_config);
}

void prepare_and_send_data(uint8_t *data, uint8_t data_length)
{
	/* this is here only temporarly to set command and address parameters */
	data_buffer[0] = 0xe5b326c1;
	data_buffer[1] = 0xaabbccdd;

	memcpy(&(data_buffer[2]), data, data_length);

	/* Send command */
	ce_hold = true;
	word_size = mspi_dev_configs.cmd_length;
	data_len = 1;
	data_to_send = data_buffer;

	if (mspi_dev_configs.io_mode == MSPI_IO_MODE_QUAD) {
		nrf_vpr_clic_int_pending_set(NRF_VPRCLIC, VEVIF_IRQN(HRT_VEVIF_IDX_WRITE_QUAD));
	} else {
		nrf_vpr_clic_int_pending_set(NRF_VPRCLIC, VEVIF_IRQN(HRT_VEVIF_IDX_WRITE_SINGLE));
	}

	/* Send address */
	word_size = mspi_dev_configs.addr_length;
	data_to_send = data_buffer + 1;

	if (mspi_dev_configs.io_mode == MSPI_IO_MODE_SINGLE ||
	    mspi_dev_configs.io_mode == MSPI_IO_MODE_QUAD_1_1_4) {
		nrf_vpr_clic_int_pending_set(NRF_VPRCLIC, VEVIF_IRQN(HRT_VEVIF_IDX_WRITE_SINGLE));
	} else {
		nrf_vpr_clic_int_pending_set(NRF_VPRCLIC, VEVIF_IRQN(HRT_VEVIF_IDX_WRITE_QUAD));
	}

	/* Send data */
	ce_hold = false;
	word_size = 32;
	data_len = (data_length - 1) / 4 + 1;
	data_to_send = data_buffer + 2;

	if (mspi_dev_configs.io_mode == MSPI_IO_MODE_SINGLE) {
		nrf_vpr_clic_int_pending_set(NRF_VPRCLIC, VEVIF_IRQN(HRT_VEVIF_IDX_WRITE_SINGLE));
	} else {
		nrf_vpr_clic_int_pending_set(NRF_VPRCLIC, VEVIF_IRQN(HRT_VEVIF_IDX_WRITE_QUAD));
	}
}

__attribute__((interrupt)) void hrt_handler_write_single(void)
{
	uint8_t ce_enable_state = (mspi_dev_configs.ce_polarity == MSPI_CE_ACTIVE_LOW) ? 0 : 1;

	write_single_by_word(data_to_send, data_len, counter_top, word_size, ce_enable_state,
			     ce_hold);
}

__attribute__((interrupt)) void hrt_handler_write_quad(void)
{
	uint8_t ce_enable_state = (mspi_dev_configs.ce_polarity == MSPI_CE_ACTIVE_LOW) ? 0 : 1;

	write_quad_by_word(data_to_send, data_len, counter_top, word_size, ce_enable_state,
			   ce_hold);
}

static int backend_init(void)
{
	int ret = 0;
	const struct device *ipc0_instance;
	volatile uint32_t delay = 0;

#if !defined(CONFIG_SYS_CLOCK_EXISTS)
	/* Wait a little bit for IPC service to be ready on APP side */
	while (delay < 1000) {
		delay++;
	}
#endif

	ipc0_instance = DEVICE_DT_GET(DT_NODELABEL(ipc0));

	ret = ipc_service_open_instance(ipc0_instance);
	if ((ret < 0) && (ret != -EALREADY)) {
		return ret;
	}

	ret = ipc_service_register_endpoint(ipc0_instance, &ep, &ep_cfg);
	if (ret < 0) {
		return ret;
	}

	/* Wait for endpoint to be bound */
	while (!atomic_test_and_clear_bit(&ipc_atomic_sem, NRFE_MSPI_EP_BOUNDED)) {
	}

	return 0;
}

int main(void)
{
	int ret = 0;

	ret = backend_init();
	if (ret < 0) {
		return 0;
	}

	IRQ_DIRECT_CONNECT(HRT_VEVIF_IDX_WRITE_SINGLE, HRT_IRQ_PRIORITY, hrt_handler_write_single,
			   0);
	nrf_vpr_clic_int_enable_set(NRF_VPRCLIC, VEVIF_IRQN(HRT_VEVIF_IDX_WRITE_SINGLE), true);

	IRQ_DIRECT_CONNECT(HRT_VEVIF_IDX_WRITE_QUAD, HRT_IRQ_PRIORITY, hrt_handler_write_quad, 0);
	nrf_vpr_clic_int_enable_set(NRF_VPRCLIC, VEVIF_IRQN(HRT_VEVIF_IDX_WRITE_QUAD), true);

	nrf_vpr_csr_rtperiph_enable_set(true);

	mspi_dev_configs.ce_polarity = MSPI_CE_ACTIVE_LOW;
	mspi_dev_configs.io_mode = MSPI_IO_MODE_SINGLE;
	mspi_dev_configs.cmd_length = 32;
	mspi_dev_configs.addr_length = 32;

	/*uint8_t data[30] = {0xa3, 0x21, 0x54, 0x3a, 0x55, 0xa5, 0x45, 0x35, 0x34, 0x23,
			    0xa3, 0xad, 0x97, 0xb2, 0x56, 0x54, 0x38, 0x88, 0x0,  0x5,
			    0x33, 0x6,	0x34, 0x6,  0x57, 0x7,	0xbb, 0xba, 0xa3, 0xf6};
	prepare_and_send_data(data, 30);

	mspi_dev_configs.io_mode = MSPI_IO_MODE_QUAD;

	prepare_and_send_data(data, 30);
*/
	while (true) {
		k_cpu_idle();
	}

	return 0;
}
