/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "./hrt/hrt.h"

#include <stdio.h>
#include <zephyr/kernel.h>
#include <hal/nrf_vpr_csr.h>
#include <hal/nrf_vpr_csr_vio.h>
#include <haly/nrfy_gpio.h>

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

void configure_clock(enum mspi_cpp_mode cpp_mode)
{
	nrf_vpr_csr_vio_config_t vio_config = {
		.input_sel = 0,
		.stop_cnt = 0,
	};

	nrf_vpr_csr_vio_dir_set(PIN_DIR_OUT_MASK(SCLK_PIN));

	switch (cpp_mode) {
	case MSPI_CPP_MODE_0: {
		vio_config.clk_polarity = 0;
		nrf_vpr_csr_vio_out_set(PIN_OUT_LOW_MASK(SCLK_PIN));
		break;
	}
	case MSPI_CPP_MODE_1: {
		vio_config.clk_polarity = 1;
		nrf_vpr_csr_vio_out_set(PIN_OUT_LOW_MASK(SCLK_PIN));
		break;
	}
	case MSPI_CPP_MODE_2: {
		vio_config.clk_polarity = 1;
		nrf_vpr_csr_vio_out_set(PIN_OUT_HIGH_MASK(SCLK_PIN));
		break;
	}
	case MSPI_CPP_MODE_3: {
		vio_config.clk_polarity = 0;
		nrf_vpr_csr_vio_out_set(PIN_OUT_HIGH_MASK(SCLK_PIN));
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

int main(void)
{
	IRQ_DIRECT_CONNECT(HRT_VEVIF_IDX_WRITE_SINGLE, HRT_IRQ_PRIORITY, hrt_handler_write_single,
			   0);
	nrf_vpr_clic_int_enable_set(NRF_VPRCLIC, VEVIF_IRQN(HRT_VEVIF_IDX_WRITE_SINGLE), true);

	IRQ_DIRECT_CONNECT(HRT_VEVIF_IDX_WRITE_QUAD, HRT_IRQ_PRIORITY, hrt_handler_write_quad, 0);
	nrf_vpr_clic_int_enable_set(NRF_VPRCLIC, VEVIF_IRQN(HRT_VEVIF_IDX_WRITE_QUAD), true);

	nrf_vpr_csr_rtperiph_enable_set(true);

	nrf_gpio_pin_dir_t dir = NRF_GPIO_PIN_DIR_OUTPUT;
	nrf_gpio_pin_input_t input = NRF_GPIO_PIN_INPUT_DISCONNECT;
	nrf_gpio_pin_pull_t pull = NRF_GPIO_PIN_NOPULL;
	nrf_gpio_pin_drive_t drive = NRF_GPIO_PIN_E0E1;

	nrfy_gpio_reconfigure(NRF_GPIO_PIN_MAP(2, SCLK_PIN), &dir, &input, &pull, &drive, NULL);
	nrfy_gpio_reconfigure(NRF_GPIO_PIN_MAP(2, D0_PIN), &dir, &input, &pull, &drive, NULL);
	nrfy_gpio_reconfigure(NRF_GPIO_PIN_MAP(2, D1_PIN), &dir, &input, &pull, &drive, NULL);
	nrfy_gpio_reconfigure(NRF_GPIO_PIN_MAP(2, D2_PIN), &dir, &input, &pull, &drive, NULL);
	nrfy_gpio_reconfigure(NRF_GPIO_PIN_MAP(2, D3_PIN), &dir, &input, &pull, &drive, NULL);
	nrfy_gpio_reconfigure(NRF_GPIO_PIN_MAP(2, CS_PIN), &dir, &input, &pull, &drive, NULL);

	nrfy_gpio_pin_control_select(NRF_GPIO_PIN_MAP(2, SCLK_PIN), NRF_GPIO_PIN_SEL_VPR);
	nrfy_gpio_pin_control_select(NRF_GPIO_PIN_MAP(2, D0_PIN), NRF_GPIO_PIN_SEL_VPR);
	nrfy_gpio_pin_control_select(NRF_GPIO_PIN_MAP(2, D1_PIN), NRF_GPIO_PIN_SEL_VPR);
	nrfy_gpio_pin_control_select(NRF_GPIO_PIN_MAP(2, D2_PIN), NRF_GPIO_PIN_SEL_VPR);
	nrfy_gpio_pin_control_select(NRF_GPIO_PIN_MAP(2, D3_PIN), NRF_GPIO_PIN_SEL_VPR);
	nrfy_gpio_pin_control_select(NRF_GPIO_PIN_MAP(2, CS_PIN), NRF_GPIO_PIN_SEL_VPR);

	mspi_dev_configs.ce_polarity = MSPI_CE_ACTIVE_LOW;
	mspi_dev_configs.io_mode = MSPI_IO_MODE_SINGLE;
	mspi_dev_configs.cmd_length = 32;
	mspi_dev_configs.addr_length = 32;

	uint8_t data[30] = {0xa3, 0x21, 0x54, 0x3a, 0x55, 0xa5, 0x45, 0x35, 0x34, 0x23,
			    0xa3, 0xad, 0x97, 0xb2, 0x56, 0x54, 0x38, 0x88, 0x0,  0x5,
			    0x33, 0x6,	0x34, 0x6,  0x57, 0x7,	0xbb, 0xba, 0xa3, 0xf6};
	prepare_and_send_data(data, 30);

	mspi_dev_configs.io_mode = MSPI_IO_MODE_QUAD;

	prepare_and_send_data(data, 30);

	while (true) {
		k_cpu_idle();
	}

	return 0;
}
