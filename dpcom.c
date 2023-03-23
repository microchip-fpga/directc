// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpcom.c                                                 */
/*                                                                          */
/*  Description:    Contains functions for data table initialization &      */
/*  pointers passing of the various data blocks needed.                     */
/*                                                                          */
/* ************************************************************************ */
#include "dpcom.h"
#include "dpuser.h"

/*
 * Paging System Specific Implementation.  User attention required:
 */
unsigned char *image_buffer; /* Pointer to the image in the system memory */
unsigned char *page_buffer_ptr;
unsigned char page_global_buffer[PAGE_BUFFER_SIZE]; /* Page_global_buffer simulating the global_buf1fer
						 that is accessible by DirectC code*/
#ifdef USE_PAGING
unsigned long page_address_offset;
unsigned long start_page_address = 0u;
unsigned long end_page_address = 0u;
#endif

unsigned long return_bytes;
unsigned long requested_bytes;
/*
 * current_block_address holds the data block starting address in the image file that the page
 * global_buf1fer data is currently holding
 */
unsigned long current_block_address = 0U;
unsigned char current_var_ID = Header_ID;
unsigned long image_size = MIN_IMAGE_SIZE;

/*
 * Module: dp_init_com_vars(void)
 * 		purpose: This function is called at the begining to initialize starting and ending
 *page addresses. Arguments: None. Return value: None.
 *
 */
void dp_init_com_vars(void)
{
#ifdef USE_PAGING
	start_page_address = 0u;
	end_page_address = 0u;
#endif
	return;
}

/*
 * Module: dp_get_data
 * 		purpose: This function is called only when access to the any data block within the
 *image is needed. It is designed to return the pointer address of the first byte containing
 *bit_index in the data block requested by the var_identifier (var_ID). Arguments: unsigned char * var_ID,
 *an unsigned integer variable contains an identifier specifying which block address to return.
 *		unsigned long bit_index: The bit location of the first bit to be processed (clocked in)
 *within the data block specified by Var_ID. Return value: Address point to the first element in the
 *block.
 *
 */
unsigned char *dp_get_data(unsigned char var_ID, unsigned long bit_index)
{
	unsigned char *data_address = (unsigned char *)DPNULL;
	dp_get_data_block_address(var_ID);
	if ((current_block_address == 0U) && (var_ID != Header_ID)) {
		return_bytes = 0U;
	} else {
		data_address = dp_get_data_block_element_address(bit_index);
	}
	return data_address;
}

/*
 * Module: dp_get_header_data
 * 		purpose: This function is called only when access to the header data block within
 *the image is needed. It is designed to return the pointer address of the first byte containing
 *bit_index in the header block. Arguments: unsigned long bit_index: The bit location of the first bit to
 *be processed (clocked in) within the data block specified by Var_ID. Return value: Address point
 *to the first element in the block.
 *
 */
unsigned char *dp_get_header_data(unsigned long bit_index)
{
	/* The first block in the image is the header.  There is no need to get the address of its
	 * block.  It is zero. */
	current_block_address = 0U;

	/* Calculating the relative address of the data block needed within the image */
	return dp_get_data_block_element_address(bit_index);
}

/*
 * User attention:
 * Module: dp_get_page_data
 * 		purpose: This function is called by dp_get_data function.  This is done every time a
 *new data block is needed or when requested data is not in the current page.  This function expects
 *the location of the first byte to return and will fill the entire page with valid data. Arguments:
 *		unsigned long image_requested_address, a ulong variable containing the relative address
 *location of the first needed byte Return value: None.
 *
 */
#ifdef USE_PAGING
void dp_get_page_data(unsigned long image_requested_address)
{
#ifdef ENABLE_EMBEDDED_SUPPORT
	return_bytes = PAGE_BUFFER_SIZE;
	/* Image size will initially be the header size which is part of the image file.
	 * This must be done to avoid accessing data that is outside the image boundaries in case
	 * the page_global_buffer size is greater than the image itself image_size variable will be
	 * read from the image file at a later time.
	 */

	/* This is needed to avoid out of bound memory access*/
	if (image_requested_address + return_bytes > image_size) {
		return_bytes = image_size - image_requested_address;
	}

	/*
	 *  Add a funciton call here to get the page data from the external storage device.
	 */

#endif
	return;
}
#endif

/*
 * Module: dp_get_data_block_address
 * 		purpose: This function sets current_block_address to the first relative address of
 * the requested data block within the data file. Return value: None.
 */
void dp_get_data_block_address(unsigned char requested_var_ID)
{
	unsigned int var_idx;
	unsigned long image_index;
	unsigned int num_vars;
	unsigned char variable_ID;

	/* If the current data block ID is the same as the requested one, there is no need to
	 * caluclate its starting address. it is already calculated.
	 */
	if (current_var_ID != requested_var_ID) {
		current_block_address = 0U;
		current_var_ID = Header_ID;
		if (requested_var_ID != Header_ID) {
			/*The lookup table is at the end of the header*/
			image_index = dp_get_header_bytes((HEADER_SIZE_OFFSET), 1U);
			image_size = dp_get_header_bytes(IMAGE_SIZE_OFFSET, 4U);

			/* The last byte in the header is the number of data blocks in the dat file
			 */
			num_vars = (unsigned int)dp_get_header_bytes(image_index - 1U, 1U);

			for (var_idx = 0U; var_idx < num_vars; var_idx++) {
				variable_ID = (unsigned char)dp_get_header_bytes(
				    image_index + BTYES_PER_TABLE_RECORD * var_idx, 1U);
				if (variable_ID == requested_var_ID) {
					current_block_address = dp_get_header_bytes(
					    image_index + BTYES_PER_TABLE_RECORD * var_idx + 1U,
					    4U);
					current_var_ID = variable_ID;
					break;
				}
			}
		}
	}
	return;
}

/*
 * Module: dp_get_data_block_element_address
 * 		purpose: This function return unsigned char pointer of the byte containing bit_index
 *       within the data file.
 * Return value:
 * 		unsigned char *: unsigned character pointer to the byte containing bit_index.
 */
unsigned char *dp_get_data_block_element_address(unsigned long bit_index)
{
	unsigned long image_requested_address;
	/* Calculating the relative address of the data block needed within the image */
	image_requested_address = current_block_address + bit_index / 8U;

#ifdef USE_PAGING
	/* If the data is within the page, adjust the pointer to point to the particular element
	 * requested */
	/* For first load, start_page_address = end_page_address = 0u */
	if ((start_page_address != end_page_address) &&
	    (image_requested_address >= start_page_address) &&
	    (image_requested_address <= end_page_address) &&
	    ((image_requested_address + MIN_VALID_BYTES_IN_PAGE) <= end_page_address)) {
		page_address_offset = image_requested_address - start_page_address;
		return_bytes = end_page_address - image_requested_address + 1u;
	}
	/* Otherwise, call dp_get_page_data which would fill the page with a new data block */
	else {
		dp_get_page_data(image_requested_address);
		page_address_offset = 0u;
	}
	return &page_global_buffer[page_address_offset];
#else
	return_bytes = image_size - image_requested_address;
	/*
	Misra - C 2004 deviation:
	image_buffer is a pointer that is being treated as an array.
	Refer to DirectC user guide for more information.
	*/
	return &image_buffer[image_requested_address];
#endif
}

/*
 * Module: dp_get_bytes
 * 		purpose: This function is designed to return all the requested bytes specified.
 *Maximum is 4. signed int * var_ID, an integer variable contains an identifier specifying which block
 *address to return. unsigned long byte_index: The relative address location of the first byte in the
 *specified data block. unsigned long requested_bytes: The number of requested bytes. Return value:
 * 		unsigned long:  The combined value of all the requested bytes.
 * Restrictions:
 *		requested_bytes cannot exceed 4 bytes since unsigned long can only hold 4 bytes.
 *
 */
unsigned long dp_get_bytes(unsigned char var_ID, unsigned long byte_index, unsigned char bytes_requested)
{
	unsigned long ret = 0U;
	unsigned char i;
	unsigned char j;
	j = 0U;

	while (bytes_requested) {
		page_buffer_ptr = dp_get_data(var_ID, byte_index * 8U);
		/* If Data block does not exist, need to exit */
		if (return_bytes == 0u) {
			break;
		}
		if (return_bytes > (unsigned long)bytes_requested) {
			return_bytes = (unsigned long)bytes_requested;
		}
		for (i = 0u; i < (unsigned char)return_bytes; i++) {
			/*
			Misra - C 2004 deviation:
			page_buffer_ptr is a pointer that is being treated as an array.
			Refer to DirectC user guide for more information.
			*/
			ret |= ((unsigned long)(page_buffer_ptr[i])) << (j++ * 8U);
		}
		byte_index += return_bytes;
		bytes_requested = bytes_requested - (unsigned char)return_bytes;
	}
	return ret;
}

/*
 * Module: dp_get_header_bytes
 * 		purpose: This function is designed to return all the requested bytes specified from
 *the header section. Maximum is 4. signed int * var_ID, an integer variable contains an identifier
 *specifying which block address to return. unsigned long byte_index: The relative address location of the
 *first byte in the specified data block. unsigned long requested_bytes: The number of requested bytes.
 * Return value:
 * 		unsigned long:  The combined value of all the requested bytes.
 * Restrictions:
 *		requested_bytes cannot exceed 4 bytes since unsigned long can only hold 4 bytes.
 *
 */
unsigned long dp_get_header_bytes(unsigned long byte_index, unsigned char bytes_requested)
{
	unsigned long ret = 0U;
	unsigned char i;
	unsigned char j = 0u;

	while (bytes_requested) {
		page_buffer_ptr = dp_get_header_data(byte_index * 8U);
		/* If Data block does not exist, need to exit */
		if (return_bytes == 0U) {
			break;
		}
		if (return_bytes > (unsigned long)bytes_requested) {
			return_bytes = (unsigned long)bytes_requested;
		}
		for (i = 0u; i < (unsigned char)return_bytes; i++) {
			/*
			Misra - C 2004 deviation:
			page_buffer_ptr is a pointer that is being treated as an array.
			Refer to DirectC user guide for more information.
			*/
			ret |= (((unsigned long)page_buffer_ptr[i])) << (j++ * 8U);
		}
		byte_index += return_bytes;
		bytes_requested = bytes_requested - (unsigned char)return_bytes;
	}
	return ret;
}

/*   *************** End of File *************** */
