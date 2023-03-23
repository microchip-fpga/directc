// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpchain.h                                               */
/*                                                                          */
/*  Description:    Definitions of chain constants, and functions           */
/*                                                                          */
/* ************************************************************************ */
#include "dpuser.h"

#ifndef INC_DPCHAIN_H
#define INC_DPCHAIN_H

#ifdef CHAIN_SUPPORT

#define PREIR_DATA_SIZE	    1u
#define PREDR_DATA_SIZE	    1u
#define POSTIR_DATA_SIZE    1u
#define POSTDR_DATA_SIZE    1u
#define PREIR_LENGTH_VALUE  0u
#define PREDR_LENGTH_VALUE  0u
#define POSTIR_LENGTH_VALUE 0u
#define POSTDR_LENGTH_VALUE 0u

extern unsigned char dp_preir_data[PREIR_DATA_SIZE];
extern unsigned char dp_predr_data[PREDR_DATA_SIZE];
extern unsigned char dp_postir_data[POSTIR_DATA_SIZE];
extern unsigned char dp_postdr_data[POSTDR_DATA_SIZE];

extern unsigned int dp_preir_length;
extern unsigned int dp_predr_length;
extern unsigned int dp_postir_length;
extern unsigned int dp_postdr_length;

void dp_do_shift_in(struct gpio_handle *jtag_gpio, unsigned long start_bit, unsigned int num_bits,
		    unsigned char tdi_data[], unsigned char terminate);
void dp_do_shift_in_out(struct gpio_handle *jtag_gpio, unsigned int num_bits, unsigned char tdi_data[],
			unsigned char tdo_data[], unsigned char terminate);
#endif /* CHAIN_SUPPORT */
#endif /* INC_DPCHAIN_H */

/* *************** End of File *************** */
