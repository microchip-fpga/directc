// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpjtag.h                                                */
/*                                                                          */
/*  Description:    Definitions of JTAG constants, types, and functions     */
/*                                                                          */
/* ************************************************************************ */
#ifndef INC_DPJTAG_H
#define INC_DPJTAG_H
#include "dpuser.h"
/****************************************************************************/
/* JTAG states codes used to identify current and target JTAG states        */
/****************************************************************************/
#define JTAG_TEST_LOGIC_RESET 1u
#define JTAG_RUN_TEST_IDLE    2u
#define JTAG_SHIFT_DR	      3u
#define JTAG_SHIFT_IR	      4u
#define JTAG_EXIT1_DR	      5u
#define JTAG_EXIT1_IR	      6u
#define JTAG_PAUSE_DR	      7u
#define JTAG_PAUSE_IR	      8u
#define JTAG_UPDATE_DR	      9u
#define JTAG_UPDATE_IR	      10u
#define JTAG_CAPTURE_DR	      11u

/****************************************************************************/
/* Function prototypes                                                      */
/****************************************************************************/
#ifdef ENABLE_EMBEDDED_SUPPORT
void goto_jtag_state(struct gpio_handle *jtag_gpio, unsigned char target_state, unsigned char cycles);
void dp_shift_in(struct gpio_handle *jtag_gpio, unsigned long start_bit, unsigned int num_bits,
		 unsigned char tdi_data[], unsigned char terminate);
void dp_shift_in_out(struct gpio_handle *jtag_gpio, unsigned int num_bits, unsigned char tdi_data[],
		     unsigned char tdo_data[]);
void dp_get_and_shift_in(struct gpio_handle *jtag_gpio, unsigned char Variable_ID,
			 unsigned int total_bits_to_shift, unsigned long start_bit_index);
void dp_get_and_shift_in_out(struct gpio_handle *jtag_gpio, unsigned char Variable_ID,
			     unsigned char total_bits_to_shift, unsigned long start_bit_index,
			     unsigned char *tdo_data);
#endif

void dp_wait_cycles(struct gpio_handle *jtag_gpio, unsigned char cycles);
void IRSCAN_in(struct gpio_handle *jtag_gpio);
void IRSCAN_out(struct gpio_handle *jtag_gpio, unsigned char *outbuf);
void DRSCAN_in(struct gpio_handle *jtag_gpio, unsigned long start_bit_index,
	       unsigned int bits_to_shift, unsigned char *inbuf);
void DRSCAN_out(struct gpio_handle *jtag_gpio, unsigned int bits_to_shift, unsigned char *inbuf,
		unsigned char *outbuf);
void dp_get_and_DRSCAN_in(struct gpio_handle *jtag_gpio, unsigned char Variable_ID,
			  unsigned int total_bits_to_shift, unsigned long start_bit_index);
void dp_get_and_DRSCAN_in_out(struct gpio_handle *jtag_gpio, unsigned char Variable_ID,
			      unsigned char total_bits_to_shift, unsigned long start_bit_index,
			      unsigned char *tdo_data);

#ifdef ENABLE_EMBEDDED_SUPPORT
extern unsigned char global_jtag_i;
extern unsigned char current_jtag_state;
extern unsigned int idx;
extern unsigned char data_buf;
extern unsigned char bit_buf;
#endif
extern unsigned char error_code;

#endif /* INC_DPJTAG_H */

/* *************** End of File *************** */
