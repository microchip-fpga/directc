// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpG5alg.h                                               */
/*                                                                          */
/*  Description:    Contains SPIPROG related function prototypes.           */
/*                                                                          */
/* ************************************************************************ */

#ifndef INC_DPG5SPI_H
#define INC_DPG5SPI_H

/*
 * Data block ID definitions
 */
/* Register discription:
Bit 0: Enable interface.  Has to be 1
Bit 1: data out
Bit 2: Slave select CS
Bit 4:3: Clock:
00 - Low
01 - High
10 - Pulse clock after update
11 - SPI Clock value held
Bit 5: 0: SPI data sampled at postive edge
1: SPI data sampled at negative edge
*/

#define G5M_SPIPROG_REGISTER_BIT_LENGTH 6u
#define G5M_SPIPROG			0xb0u

#define DISABLE_SPIPROG_INSTRUCTION   0x0u
#define ENABLE_SPIPROG_INSTRUCTION    0x1u
#define SLVSEL_LOW		      0x0u
#define SLVSEL_HIGH		      0x4u
#define SPI_CLOCK_INACTIVE_STATE_LOW  0x0u
#define SPI_CLOCK_INACTIVE_STATE_HIGH 0x8u
#define SPI_CLOCK_TOGGLE	      0x10u
#define SPI_SAMPLE_ON_POSTIVE_EDGE    0x0u
#define SPI_SAMPLE_ON_NEGATIVE_EDGE   0x20u

void init_spiprog_port(struct gpio_handle *jtag_gpio);
void disable_spiprog_port(struct gpio_handle *jtag_gpio);
void enable_cs(struct gpio_handle *jtag_gpio);
void disable_cs(struct gpio_handle *jtag_gpio);

void spi_shift_dummy_bit(struct gpio_handle *jtag_gpio);
void spi_shift_byte_in(struct gpio_handle *jtag_gpio, unsigned char byte_in);
void spi_shift_byte_out(struct gpio_handle *jtag_gpio, unsigned char *byte_out);
void spi_scan(struct gpio_handle *jtag_gpio, unsigned char command, unsigned long total_bytes, unsigned char *data_in,
	      unsigned char *data_out);
#endif /* INC_DPG5ALG_H */
