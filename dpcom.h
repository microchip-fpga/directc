// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpcom.h                                                 */
/*                                                                          */
/*  Description:    Contains functions prototypes needed to access the data */
/*  from external flash or other means of communication                     */
/*                                                                          */
/* ************************************************************************ */

#ifndef INC_DPCOM_H
#define INC_DPCOM_H

extern unsigned long return_bytes;
extern unsigned long image_size;
extern unsigned long requested_bytes;
extern unsigned char *page_buffer_ptr;

/* user attention is required.  PAGE_BUFFER_SIZE needs to be specified in bytes */
#define PAGE_BUFFER_SIZE	1024u
#define MIN_VALID_BYTES_IN_PAGE 16u

extern unsigned char
    page_global_buffer[PAGE_BUFFER_SIZE]; /* Page_global_buffer simulating the global_buffer that is
					     accessible by DirectC code*/

/*
 * Location of special variables needed in the header section of the image file
 */
#define Header_ID	       0u
#define BTYES_PER_TABLE_RECORD 9u
#define ACTEL_HEADER_SIZE      24u
#define HEADER_SIZE_OFFSET     24u
#define IMAGE_SIZE_OFFSET      25u
#define MIN_IMAGE_SIZE	       56u

void dp_init_com_vars(void);
unsigned char *dp_get_data(unsigned char var_ID, unsigned long bit_index);
unsigned char *dp_get_header_data(unsigned long bit_index);
void dp_get_page_data(unsigned long image_requested_address);
void dp_get_data_block_address(unsigned char requested_var_ID);
unsigned char *dp_get_data_block_element_address(unsigned long bit_index);
unsigned long dp_get_bytes(unsigned char var_ID, unsigned long byte_index, unsigned char bytes_requested);
unsigned long dp_get_header_bytes(unsigned long byte_index, unsigned char bytes_requested);
#endif /* INC_DPCOM_H */

/*   *************** End of File *************** */
