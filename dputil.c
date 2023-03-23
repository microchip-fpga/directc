// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dputil.c                                                */
/*                                                                          */
/*  Description:    Contains initialization and data checking functions.    */
/*                                                                          */
/* ************************************************************************ */

#include "dputil.h"
#include "dpalg.h"
#include "dpcom.h"

/*
 * General purpose Global variables needed in the program
 */
unsigned char global_uchar1; /* Global tmp should be used once and released	*/
unsigned char global_uchar2; /* Global tmp should be used once and released	*/
unsigned long global_ulong1;
unsigned long global_ulong2;
unsigned int global_uint1;
unsigned int global_uint2;
/* global_buf_SIZE increasing the value does not hurt but not
needed as long as it does not exceed 255 */
unsigned char global_buf1[global_buf_SIZE]; /* General purpose global_buf1fer */
unsigned char global_buf2[global_buf_SIZE]; /* global_buffer to hold UROW data */

void dp_flush_global_buf1(void)
{
	unsigned char index;
	for (index = 0u; index < global_buf_SIZE; index++) {
		global_buf1[index] = 0u;
	}
	return;
}

void dp_flush_global_buf2(void)
{
	unsigned char index;
	for (index = 0u; index < global_buf_SIZE; index++) {
		global_buf2[index] = 0u;
	}
	return;
}

void dp_init_vars(void)
{
	error_code = DPE_SUCCESS;
	return;
}

/*
 * Module: dp_check_image_crc
 * 		purpose: Performs crc on the entire image.
 * Return value:
 * 		User defined integer value which reports DPE_SUCCESS if there is a match or
 * DPE_CRC_MISMATCH if failed.
 *
 */
void dp_check_image_crc(void)
{
#ifdef PERFORM_CRC_CHECK
	unsigned int expected_crc;
#endif
#ifdef PERFORM_CRC_CHECK
#ifdef ENABLE_DISPLAY
	dp_display_text("\r\nChecking data CRC...");
#endif
#endif

	global_ulong1 = dp_get_bytes(Header_ID, 0u, 4u);
	if ((global_ulong1 == 0x69736544u) || (global_ulong1 == 0x65746341u) ||
	    (global_ulong1 == 0x2D4D3447u) || (global_ulong1 == 0x34475452u) ||
	    (global_ulong1 == 0x2D4D3547u)) {
		requested_bytes = 0u;
		image_size = dp_get_bytes(Header_ID, IMAGE_SIZE_OFFSET, 4u);
#ifdef PERFORM_CRC_CHECK
		expected_crc = (unsigned int)dp_get_bytes(Header_ID, image_size - 2u, 2u);
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nExpected CRC=");
		dp_display_value(expected_crc, HEX);
#endif
#endif
		if (image_size == 0u) {
			error_code = DPE_CRC_MISMATCH;
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nData file is not loaded... \r\n");
#endif
		} else {
#ifdef PERFORM_CRC_CHECK
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nCalculating actual CRC.  Please wait...\r\n");
#endif
			/* Global_uint is used to hold the value of the calculated CRC */
			global_uint1 = 0u;
/* DataIndex is used to keep track the byte position in the image that is needed per
 * get_data_request */
#ifdef ENABLE_DISPLAY
			old_progress = 0;
#endif
			DataIndex = 0u;
			requested_bytes = image_size - 2u;
			while (requested_bytes) {
				page_buffer_ptr = dp_get_data(Header_ID, DataIndex * 8u);
				if (return_bytes > requested_bytes)
					return_bytes = requested_bytes;
				for (global_ulong1 = 0u; global_ulong1 < return_bytes;
				     global_ulong1++) {
					global_uchar1 = page_buffer_ptr[global_ulong1];
					dp_compute_crc();
				}
				DataIndex += return_bytes;
				requested_bytes -= return_bytes;

#ifdef ENABLE_DISPLAY
				new_progress = (DataIndex * 100 / (image_size - 2u));
				if (new_progress != old_progress) {
					dp_report_progress(new_progress);
					old_progress = new_progress;
				}
#endif
			}

			if (global_uint1 != expected_crc) {
#ifdef ENABLE_DISPLAY
				dp_display_text("\r\nCRC verification failed.  Expected CRC = ");
				dp_display_value(global_uint1, HEX);
				dp_display_text(" Actual CRC = ");
				dp_display_value((unsigned int)dp_get_bytes(Header_ID, image_size - 2, 2),
						 HEX);
				dp_display_text("\r\n");
#endif
				error_code = DPE_CRC_MISMATCH;
			}
#else
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\n\r\nWARNING: Skipping CRC verification...\r\n");
#endif
#endif
		}
	} else {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nData file is not valid. ");
#endif
		error_code = DPE_CRC_MISMATCH;
	}
	return;
}

void dp_compute_crc(void)
{
	for (global_uchar2 = 0u; global_uchar2 < 8u; global_uchar2++) {
		global_uint2 = (global_uchar1 ^ global_uint1) & 0x01u;
		global_uint1 >>= 1u;
		if (global_uint2) {
			global_uint1 ^= 0x8408u;
		}
		global_uchar1 >>= 1u;
	}

	return;
}

void dp_check_and_get_image_size(void)
{

	global_ulong1 = dp_get_bytes(Header_ID, 0u, 4u);
	if ((global_ulong1 == 0x69736544u) || (global_ulong1 == 0x65746341u) ||
	    (global_ulong1 == 0x2D4D3447u) || (global_ulong1 == 0x34475452u) ||
	    (global_ulong1 == 0x2D4D3547u)) {
		requested_bytes = 0u;
		image_size = dp_get_bytes(Header_ID, IMAGE_SIZE_OFFSET, 4u);
	} else {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nData file is not valid. ");
#endif
		error_code = DPE_DAT_ACCESS_FAILURE;
	}
	return;
}

#ifdef ENABLE_DISPLAY
#define NB_NIBBLES_IN_INT 8
int int_to_hex_int(int value, unsigned char *p_result, int result_size)
{
	int nibble_idx, nb_nibbles;
	unsigned char conv_array[NB_NIBBLES_IN_INT];
	unsigned int uvalue;
	nibble_idx = 0;
	uvalue = (unsigned int)value;

	do {
		int nibble = uvalue & 0x0F;

		if (nibble < 10)
			conv_array[nibble_idx] = nibble + '0';
		else
			conv_array[nibble_idx] = nibble - 10 + 'A';
		uvalue = (uvalue >> 4);
		nibble_idx++;
	} while ((nibble_idx < NB_NIBBLES_IN_INT) && (uvalue > 0));

	nb_nibbles = nibble_idx;
	for (nibble_idx = 0; (nibble_idx < nb_nibbles) && (nibble_idx < result_size);
	     nibble_idx++) {
		p_result[nibble_idx] = conv_array[nb_nibbles - nibble_idx - 1];
	}
	return nibble_idx;
}

int int_to_dec_int(int value, unsigned char *p_result)
{
	unsigned char conv_array[NB_NIBBLES_IN_INT];
	unsigned int uvalue;
	unsigned int remainder;
	unsigned int digit_idx, nb_digits;

	uvalue = (unsigned int)value;
	digit_idx = 0;
	if (uvalue) {
		while (uvalue) {
			remainder = uvalue % 10;
			conv_array[digit_idx] = remainder + '0';
			uvalue /= 10;
			digit_idx++;
		}
	} else {
		conv_array[digit_idx] = '0';
		digit_idx++;
	}

	nb_digits = digit_idx;
	for (digit_idx = 0; (digit_idx < nb_digits); digit_idx++) {
		p_result[digit_idx] = conv_array[nb_digits - digit_idx - 1];
	}
	return digit_idx;
}

int int_to_chr_int(int value, unsigned char *p_result)
{
	p_result[0] = value;
	return 1;
}
#endif

/* *************** End of File *************** */
