// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpSpiProg.c                                             */
/*                                                                          */
/*  Description:    Contains SPIPROG related functions                      */
/*                                                                          */
/* ************************************************************************ */

#include "dpuser.h"
#ifdef ENABLE_SPI_FLASH_SUPPORT
#include "dpjtag.h"
#include "dpalg.h"
#include "dpSPIprog.h"

/* See bit definition in dpSPI.h.
The hardware can automatically generate the SPI clock to reduce the vector cound by two.
This takes place when JTAG state machine goes through the UPDATE state.
Therefore, goto_jtag(IDLE_STATE) is required. */

unsigned char spiprog_reg = 0x0u;
unsigned char spiprog_reg_out = 0x0u;

void init_spiprog_port(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_SPIPROG;
	// Set the initial values prior to enabling the instruction
	spiprog_reg = DISABLE_SPIPROG_INSTRUCTION | SLVSEL_HIGH | SPI_CLOCK_INACTIVE_STATE_LOW |
		      SPI_SAMPLE_ON_POSTIVE_EDGE;
	IRSCAN_in(jtag_gpio); // Should only be required once.
	DRSCAN_in(jtag_gpio, 0u, G5M_SPIPROG_REGISTER_BIT_LENGTH, &spiprog_reg);

	// Enable the instruction
	spiprog_reg = ENABLE_SPIPROG_INSTRUCTION | SLVSEL_HIGH | SPI_CLOCK_INACTIVE_STATE_LOW |
		      SPI_SAMPLE_ON_POSTIVE_EDGE;
	DRSCAN_in(jtag_gpio, 0u, G5M_SPIPROG_REGISTER_BIT_LENGTH, &spiprog_reg);

	return;
}

void disable_spiprog_port(struct gpio_handle *jtag_gpio)
{
	// Set the initial values prior to enabling the instruction
	spiprog_reg = DISABLE_SPIPROG_INSTRUCTION | SLVSEL_HIGH | SPI_CLOCK_INACTIVE_STATE_LOW |
		      SPI_SAMPLE_ON_POSTIVE_EDGE;
	DRSCAN_in(jtag_gpio, 0u, G5M_SPIPROG_REGISTER_BIT_LENGTH, &spiprog_reg);

	return;
}

void enable_cs(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_SPIPROG;
	// Set the initial values prior to enabling the instruction
	spiprog_reg = ENABLE_SPIPROG_INSTRUCTION | SLVSEL_LOW | SPI_CLOCK_INACTIVE_STATE_LOW |
		      SPI_SAMPLE_ON_POSTIVE_EDGE;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_SPIPROG_REGISTER_BIT_LENGTH, &spiprog_reg);
}

void disable_cs(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_SPIPROG;
	// Set the initial values prior to enabling the instruction
	spiprog_reg = ENABLE_SPIPROG_INSTRUCTION | SLVSEL_HIGH | SPI_CLOCK_INACTIVE_STATE_LOW |
		      SPI_SAMPLE_ON_POSTIVE_EDGE;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_SPIPROG_REGISTER_BIT_LENGTH, &spiprog_reg);
}

void spi_shift_dummy_bit(struct gpio_handle *jtag_gpio)
{
	spiprog_reg =
	    ENABLE_SPIPROG_INSTRUCTION | SLVSEL_LOW | SPI_CLOCK_TOGGLE | SPI_SAMPLE_ON_POSTIVE_EDGE;
	DRSCAN_in(jtag_gpio, 0u, G5M_SPIPROG_REGISTER_BIT_LENGTH, &spiprog_reg);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, 0);
	return;
}

void spi_scan(struct gpio_handle *jtag_gpio, unsigned char command, unsigned long total_bytes, unsigned char *data_in,
	      unsigned char *data_out)
{
	unsigned long index;
	enable_cs(jtag_gpio);
	spi_shift_byte_in(jtag_gpio, command);
	if (data_in != DPNULL) {

		for (index = 0; index < total_bytes; index++) {
			spi_shift_byte_in(jtag_gpio, data_in[index]);
		}
	}
	if (data_out != DPNULL) {
		spi_shift_dummy_bit(jtag_gpio);
		for (index = 0; index < total_bytes; index++) {
			spi_shift_byte_out(jtag_gpio, &data_out[index]);
		}
	}
	disable_cs(jtag_gpio);
}

void spi_shift_byte_in(struct gpio_handle *jtag_gpio, unsigned char byte_in)
{
	unsigned char index;
	spiprog_reg =
	    ENABLE_SPIPROG_INSTRUCTION | SLVSEL_LOW | SPI_CLOCK_TOGGLE | SPI_SAMPLE_ON_POSTIVE_EDGE;

	for (index = 0x80; index > 0; index >>= 1) {
		spiprog_reg &= 0xfd;
		if (index & byte_in) {
			spiprog_reg |= 0x2;
		}
		DRSCAN_in(jtag_gpio, 0u, G5M_SPIPROG_REGISTER_BIT_LENGTH, &spiprog_reg);
		goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, 0);
	}
}

void spi_shift_byte_out(struct gpio_handle *jtag_gpio, unsigned char *byte_out)
{
	unsigned char index;

	spiprog_reg =
	    ENABLE_SPIPROG_INSTRUCTION | SLVSEL_LOW | SPI_CLOCK_TOGGLE | SPI_SAMPLE_ON_POSTIVE_EDGE;

	*byte_out = 0;
	for (index = 0x80; index > 0; index >>= 1) {
		DRSCAN_out(jtag_gpio, G5M_SPIPROG_REGISTER_BIT_LENGTH, &spiprog_reg,
			   &spiprog_reg_out);
		goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, 0);

		if (spiprog_reg_out & 0x1u) {
			*byte_out |= index;
		}
	}
}
#endif /* ENABLE_SPI_FLASH_SUPPORT */
