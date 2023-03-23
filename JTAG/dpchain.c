// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpchain.c                                               */
/*                                                                          */
/*  Description:    Contains chain functions                                */
/*                                                                          */
/* ************************************************************************ */
#include "dpchain.h"
#include "dpcom.h"
#include "dpuser.h"
#include "dpjtag.h"

#ifdef CHAIN_SUPPORT
/* *****************************************************************************
 * Variable that must be intialized with appropriate data depending on the chain
 * configuration.  See user guide for more information.
 *******************************************************************************/
unsigned char dp_preir_data[PREIR_DATA_SIZE] = {0xff};
unsigned char dp_predr_data[PREDR_DATA_SIZE] = {0x0};
unsigned char dp_postir_data[POSTIR_DATA_SIZE] = {0xff};
unsigned char dp_postdr_data[POSTDR_DATA_SIZE] = {0x0};

unsigned int dp_preir_length = PREIR_LENGTH_VALUE;
unsigned int dp_predr_length = PREDR_LENGTH_VALUE;
unsigned int dp_postir_length = POSTIR_LENGTH_VALUE;
unsigned int dp_postdr_length = POSTDR_LENGTH_VALUE;

/****************************************************************************
 * Purpose: clock data stored in tdi_data into the device.
 * terminate is a flag needed to determine if shifting to pause state should
 * be done with the last bit shift.
 ****************************************************************************/
void dp_shift_in(struct gpio_handle *jtag_gpio, unsigned long start_bit, unsigned int num_bits,
		 unsigned char tdi_data[], unsigned char terminate)
{
	if (current_jtag_state == JTAG_SHIFT_IR) {
		if (dp_preir_length > 0U) {
			dp_do_shift_in(jtag_gpio, 0U, dp_preir_length, dp_preir_data, 0U);
		}
		if (dp_postir_length > 0U) {
			dp_do_shift_in(jtag_gpio, start_bit, num_bits, tdi_data, 0U);
			dp_do_shift_in(jtag_gpio, 0U, dp_postir_length, dp_postir_data, terminate);
		} else {
			dp_do_shift_in(jtag_gpio, start_bit, num_bits, tdi_data, terminate);
		}
	} else if (current_jtag_state == JTAG_SHIFT_DR) {
		if (dp_predr_length > 0U) {
			dp_do_shift_in(jtag_gpio, 0U, dp_predr_length, dp_predr_data, 0U);
		}
		if (dp_postdr_length > 0U) {
			dp_do_shift_in(jtag_gpio, start_bit, num_bits, tdi_data, 0U);
			dp_do_shift_in(jtag_gpio, 0U, dp_postdr_length, dp_postdr_data, terminate);
		} else {
			dp_do_shift_in(jtag_gpio, start_bit, num_bits, tdi_data, terminate);
		}
	} else {
	}
	return;
}
/****************************************************************************
 * Purpose:  clock data stored in tdi_data into the device.
 *           capture data coming out of tdo into tdo_data.
 * This function will always clock data starting bit postion 0.
 * Jtag state machine will always set the pauseDR or pauseIR state at the
 * end of the shift.
 ****************************************************************************/
void dp_shift_in_out(struct gpio_handle *jtag_gpio, unsigned int num_bits, unsigned char tdi_data[],
		     unsigned char tdo_data[])
{
	if (current_jtag_state == JTAG_SHIFT_IR) {
		if (dp_preir_length > 0U) {
			dp_do_shift_in(jtag_gpio, 0U, dp_preir_length, dp_preir_data, 0U);
		}
		if (dp_postir_length > 0U) {
			dp_do_shift_in_out(jtag_gpio, num_bits, tdi_data, tdo_data, 0U);
			dp_do_shift_in(jtag_gpio, 0U, dp_postir_length, dp_postir_data, 1U);
		} else {
			dp_do_shift_in_out(jtag_gpio, num_bits, tdi_data, tdo_data, 1U);
		}
	} else if (current_jtag_state == JTAG_SHIFT_DR) {
		if (dp_predr_length > 0U) {
			dp_do_shift_in(jtag_gpio, 0U, dp_predr_length, dp_predr_data, 0U);
		}
		if (dp_postdr_length > 0U) {
			dp_do_shift_in_out(jtag_gpio, num_bits, tdi_data, tdo_data, 0U);
			dp_do_shift_in(jtag_gpio, 0U, dp_postdr_length, dp_postdr_data, 1U);
		} else {
			dp_do_shift_in_out(jtag_gpio, num_bits, tdi_data, tdo_data, 1U);
		}
	} else {
	}
	return;
}

void dp_do_shift_in(struct gpio_handle *jtag_gpio, unsigned long start_bit, unsigned int num_bits,
		    unsigned char tdi_data[], unsigned char terminate)
{
	idx = (unsigned char)start_bit >> 3;
	bit_buf = 1U << (unsigned char)(start_bit & 0x7U);
	if (tdi_data == (unsigned char *)DPNULL) {
		data_buf = 0U;
	} else {
		data_buf = tdi_data[idx] >> ((unsigned char)(start_bit & 0x7U));
	}
	if (terminate == 0U) {
		num_bits++;
	}
	while (--num_bits) {
		dp_jtag_tms_tdi(jtag_gpio, 0U, data_buf & 0x1U);
		data_buf >>= 1;
		bit_buf <<= 1;
		if ((bit_buf & 0xffU) == 0U) {
			bit_buf = 1U;
			idx++;
			if (tdi_data == (unsigned char *)DPNULL) {
				data_buf = 0U;
			} else {
				data_buf = tdi_data[idx];
			}
		}
	}
	if (terminate) {
		dp_jtag_tms_tdi(jtag_gpio, 1U, data_buf & 0x1U);
		if (current_jtag_state == JTAG_SHIFT_IR) {
			current_jtag_state = JTAG_EXIT1_IR;
		} else if (current_jtag_state == JTAG_SHIFT_DR) {
			current_jtag_state = JTAG_EXIT1_DR;
		} else {
		}
	}
	return;
}

void dp_do_shift_in_out(struct gpio_handle *jtag_gpio, unsigned int num_bits, unsigned char tdi_data[],
			unsigned char tdo_data[], unsigned char terminate)
{
	bit_buf = 1U;
	idx = 0U;
	tdo_data[idx] = 0U;

	if (tdi_data == (unsigned char *)DPNULL) {
		data_buf = 0U;
	} else {
		data_buf = tdi_data[idx];
	}

	while (--num_bits) {
		if ((bit_buf & 0xffU) == 0U) {
			bit_buf = 1U;
			idx++;
			tdo_data[idx] = 0U;
			if (tdi_data == (unsigned char *)DPNULL) {
				data_buf = 0U;
			} else {
				data_buf = tdi_data[idx];
			}
		}
		if (dp_jtag_tms_tdi_tdo(jtag_gpio, 0U, data_buf & 0x1U)) {
			tdo_data[idx] |= bit_buf;
		}
		bit_buf <<= 1;
		data_buf >>= 1;
	}
	if ((bit_buf & 0xffU) == 0U) {
		bit_buf = 1U;
		idx++;
		tdo_data[idx] = 0U;
		if (tdi_data == (unsigned char *)DPNULL) {
			data_buf = 0U;
		} else {
			data_buf = tdi_data[idx];
		}
	}
	if (terminate) {
		if (dp_jtag_tms_tdi_tdo(jtag_gpio, 1U, data_buf & 0x1U)) {
			tdo_data[idx] |= bit_buf;
		}
		if (current_jtag_state == JTAG_SHIFT_IR) {
			current_jtag_state = JTAG_EXIT1_IR;
		} else if (current_jtag_state == JTAG_SHIFT_DR) {
			current_jtag_state = JTAG_EXIT1_DR;
		} else {
		}
	} else {
		if (dp_jtag_tms_tdi_tdo(jtag_gpio, 0U, data_buf & 0x1U)) {
			tdo_data[idx] |= bit_buf;
		}
	}
	return;
}
/****************************************************************************
 * Purpose:  Gets the data block specified by Variable_ID from the image dat
 * file and clocks it into the device.
 ****************************************************************************/
void dp_get_and_shift_in(struct gpio_handle *jtag_gpio, unsigned char Variable_ID,
			 unsigned int total_bits_to_shift, unsigned long start_bit_index)
{
	unsigned long page_start_bit_index;
	unsigned int bits_to_shift;
	unsigned char terminate;
	page_start_bit_index = start_bit_index & 0x7U;
	requested_bytes = (unsigned long)(page_start_bit_index + total_bits_to_shift + 7U) >> 3U;

	if (current_jtag_state == JTAG_SHIFT_IR) {
		if (dp_preir_length > 0U) {
			dp_do_shift_in(jtag_gpio, 0U, dp_preir_length, dp_preir_data, 0U);
		}
	} else if (current_jtag_state == JTAG_SHIFT_DR) {
		if (dp_predr_length > 0U) {
			dp_do_shift_in(jtag_gpio, 0U, dp_predr_length, dp_predr_data, 0U);
		}
	} else {
	}

	terminate = 0U;
	while (requested_bytes) {
		page_buffer_ptr = dp_get_data(Variable_ID, start_bit_index);

		if (return_bytes >= requested_bytes) {
			return_bytes = requested_bytes;
			bits_to_shift = total_bits_to_shift;
			terminate = 1U;
			if (((current_jtag_state == JTAG_SHIFT_IR) && dp_postir_length) ||
			    ((current_jtag_state == JTAG_SHIFT_DR) && dp_postdr_length)) {
				terminate = 0U;
			}
		} else {
			bits_to_shift = (unsigned char)(return_bytes * 8U - page_start_bit_index);
		}
		dp_do_shift_in(jtag_gpio, page_start_bit_index, bits_to_shift, page_buffer_ptr,
			       terminate);

		requested_bytes = requested_bytes - return_bytes;
		total_bits_to_shift = total_bits_to_shift - bits_to_shift;
		start_bit_index += bits_to_shift;
		page_start_bit_index = start_bit_index & 0x7U;
	}

	if (current_jtag_state == JTAG_SHIFT_IR) {
		if (dp_postir_length > 0U) {
			dp_do_shift_in(jtag_gpio, 0U, dp_postir_length, dp_postir_data, 1U);
		}
	} else if (current_jtag_state == JTAG_SHIFT_DR) {
		if (dp_postdr_length > 0U) {
			dp_do_shift_in(jtag_gpio, 0U, dp_postdr_length, dp_postdr_data, 1U);
		}
	} else {
	}
	return;
}

/****************************************************************************
 * Purpose:  Get the data block specified by Variable_ID from the image dat
 * file and clocks it into the device.  Capture the data coming out of tdo
 * into tdo_data
 ****************************************************************************/
void dp_get_and_shift_in_out(struct gpio_handle *jtag_gpio, unsigned char Variable_ID,
			     unsigned char total_bits_to_shift, unsigned long start_bit_index,
			     unsigned char *tdo_data)
{
	requested_bytes = ((unsigned long)total_bits_to_shift + 7u) >> 3u;
	page_buffer_ptr = dp_get_data(Variable_ID, start_bit_index);

	if (return_bytes >= requested_bytes) {
		return_bytes = requested_bytes;
		dp_shift_in_out(jtag_gpio, (unsigned int)total_bits_to_shift, page_buffer_ptr, tdo_data);
	} else {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nError: Page buffer size is not big enough...");
#endif
	}

	return;
}
#endif

/* *************** End of File *************** */
