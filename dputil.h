// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dputil.h                                                */
/*                                                                          */
/*  Description:    Contains functions prototypes needed to access the data */
/*  from external flash or other means of communication                     */
/*                                                                          */
/* ************************************************************************ */

#ifndef INC_DPUTIL_H
#define INC_DPUTIL_H

/****************************************************************************/
/* External common global variables                                         */
/****************************************************************************/
/* global_buf_SIZE should not exceed 255 bytes.*/
#define global_buf_SIZE 17u

extern unsigned char global_uchar1; /* Global tmp should be used once and released */
extern unsigned char global_uchar2;
extern unsigned int global_uint1;
extern unsigned int global_uint2;
extern unsigned long global_ulong1;
extern unsigned long global_ulong2;
extern unsigned char global_buf1[global_buf_SIZE]; /* General purpose global_buffer */
extern unsigned char global_buf2[global_buf_SIZE];

void dp_flush_global_buf1(void);
void dp_flush_global_buf2(void);
void dp_init_vars(void);
/* Function used to identify which block is supported in the dat file and their encryption status.
 */

void dp_compute_crc(void);
void dp_check_image_crc(void);
void dp_check_and_get_image_size(void);

#ifdef ENABLE_DISPLAY
int int_to_hex_int(int value, unsigned char *p_result, int result_size);
int int_to_dec_int(int value, unsigned char *p_result);
int int_to_chr_int(int value, unsigned char *p_result);
#endif
#endif /* INC_DPUTIL_H */

/* *************** End of File *************** */
