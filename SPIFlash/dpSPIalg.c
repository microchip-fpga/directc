// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpSPIalg.c                                              */
/*                                                                          */
/*  Description:    Contains common SPI-Flash functions.                    */
/*                                                                          */
/* ************************************************************************ */
#include "dpuser.h"
#ifdef ENABLE_SPI_FLASH_SUPPORT
#include "dpjtag.h"
#include "dpalg.h"
#include "dpcom.h"
#include "dpS25F.h"
#include "dpSPIalg.h"
#include "dpSPIprog.h"

unsigned long spi_target_address = 0;
unsigned long long spi_flash_memory_byte_size;
unsigned long long bytes_processed;
unsigned char id_buf[3] = {0, 0, 0};
unsigned char spi_flash_vendor_id = 0;
unsigned char spi_flash_memory_type_id = 0;
unsigned char spi_flash_memory_size_id = 0;
unsigned char address_mode = ADDRESS_3BYTE_MODE;
unsigned int page_byte_size = 0;
unsigned long sector_byte_size = 0;

unsigned char dp_top_spi_flash(struct gpio_handle *jtag_gpio)
{
	error_code = DPE_SUCCESS;
	dp_init_com_vars();
	Action_done = FALSE;

	init_spiprog_port(jtag_gpio);
	dp_SPI_read_idcode(jtag_gpio);

	if ((spi_flash_vendor_id == CYPRESS_VENDOR_ID) &&
	    ((spi_flash_memory_type_id == CYPRESS_MEMORY_TYPE1_ID) ||
	     (spi_flash_memory_type_id == CYPRESS_MEMORY_TYPE2_ID) ||
	     (spi_flash_memory_type_id == CYPRESS_MEMORY_TYPE3_ID))) {
		dp_display_text("\r\nCypress S25F device is found.");
		dp_display_text("\r\nSPI-Flash IDCode (HEX) = ");
		dp_display_array(id_buf, 3, HEX);
		S25F_parse_idcode();
		if (spi_flash_memory_byte_size > 1048576) {
			dp_display_text("\r\nDevice size (MBytes) = ");
			dp_display_value(spi_flash_memory_byte_size / 1048576, DEC);
		} else {
			dp_display_text("\r\nDevice size (KBytes) = ");
			dp_display_value(spi_flash_memory_byte_size / 1024, DEC);
		}
		if (spi_flash_memory_byte_size == 0) {
			dp_display_text("\r\nError: Failed to recognize device density.");
			error_code = DPE_IDCODE_ERROR;
		} else {
			dp_top_S25F(jtag_gpio);
			Action_done = TRUE;
		}
	}
	if (Action_done == FALSE) {
		dp_display_text("\r\nError: SPI-Flash is not connected or not supported.");
		error_code = DPE_IDCODE_ERROR;
	}

	return error_code;
}

void dp_check_image_address_and_size(void)
{
	if ((spi_target_address + image_size) > spi_flash_memory_byte_size) {
		dp_display_text(
		    "\r\nError: Image byte size is greater than available memory space.");
		dp_display_text("\r\nSPI Target Address + Image size: ");
		dp_display_value((spi_target_address + image_size), DEC);
		dp_display_text("\r\nAvailable memory space: ");
		dp_display_value(spi_flash_memory_byte_size, DEC);
		error_code = DPE_IMAGE_SIZE_ERROR;
	}
}

void dp_SPI_read_idcode(struct gpio_handle *jtag_gpio)
{
	spi_scan(jtag_gpio, SPI_READ_ID_CMD, 3u, DPNULL, id_buf);

	spi_flash_vendor_id = id_buf[0];
	spi_flash_memory_type_id = id_buf[1];
	spi_flash_memory_size_id = id_buf[2];
	return;
}

void dp_SPI_blank_check_action(struct gpio_handle *jtag_gpio)
{
	dp_display_text("\r\nPerforming SPI Flash Blank Check Action:\r\n");
	bytes_processed = 0u;
	SPI_blank_check_memory(jtag_gpio, 0u, spi_flash_memory_byte_size);

	return;
}

void dp_SPI_read_action(struct gpio_handle *jtag_gpio)
{
	unsigned long bytes_read = 0u; // This is used to keep track of how many bytes read and also used
				 // as the starting address to read
	unsigned long bytes_to_read = 0u;

	dp_display_text("\r\nPerforming SPI Flash Read Action:\r\n");

	// Image_size contains the data in ddr including header.
	while (bytes_read < (image_size)) {
		bytes_to_read = image_size - bytes_read;
		if (bytes_to_read > PAGE_BUFFER_SIZE)
			bytes_to_read = PAGE_BUFFER_SIZE;

		SPI_read_memory(jtag_gpio, bytes_read, bytes_to_read);
		bytes_read += bytes_to_read;
		dp_display_array_reverse(page_global_buffer, bytes_to_read, HEX);
	}

	return;
}

void dp_SPI_verify_action(struct gpio_handle *jtag_gpio)
{
	dp_display_text("\r\nPerforming SPI Flash Verify Action: ");

	bytes_processed = 0u;

	if (error_code == DPE_SUCCESS) {
		DataIndex = 0;
		dp_display_text("\r\nVerifying image from address = 0x");
		dp_display_value(spi_target_address + DataIndex, HEX);
		dp_display_text(" - 0x");
		dp_display_value(spi_target_address + DataIndex + image_size - 1u, HEX);

		do {
			page_buffer_ptr = dp_get_data(Header_ID, DataIndex * 8u);
			if (return_bytes > image_size)
				return_bytes = image_size;

			SPI_verify_memory(jtag_gpio, spi_target_address + DataIndex, return_bytes,
					  page_buffer_ptr);
			if (error_code != DPE_SUCCESS)
				break;
			DataIndex += return_bytes;

		} while (DataIndex < image_size);
	}
	return;
}

void SPI_blank_check_memory(struct gpio_handle *jtag_gpio, unsigned long start_address,
			    unsigned long number_of_bytes)
{
	unsigned long index;
	unsigned char data;
	unsigned char address[4];

	enable_cs(jtag_gpio);
	spi_shift_byte_in(jtag_gpio, SPI_READ);

	for (index = 0; index < address_mode; index++) {
		address[index] = (start_address >> ((address_mode - 1 - index) * 8)) & 0xff;
		spi_shift_byte_in(jtag_gpio, address[index]);
	}
	spi_shift_dummy_bit(jtag_gpio);

	for (index = 0; index < number_of_bytes; index++) {
		spi_shift_byte_out(jtag_gpio, &data);
		if (data != 0xffu) {
			error_code = DPE_SPI_FLASH_BLANK_CHECK_ERROR;
			dp_display_text("\r\nError: SPI-Flash Is not blank: Address = 0x");
			dp_display_value((start_address + index), HEX);
			dp_display_text(" - Actual value = 0x");
			dp_display_value(data, HEX);
			break;
		}
		bytes_processed++;
#ifdef ENABLE_DISPLAY
		new_progress = (unsigned char)(bytes_processed * 100 / spi_flash_memory_byte_size);
		if (new_progress != old_progress) {
			dp_report_progress(new_progress);
			old_progress = new_progress;
		}
#endif
	}
	disable_cs(jtag_gpio);
}

void SPI_read_memory(struct gpio_handle *jtag_gpio, unsigned long start_address, unsigned long number_of_bytes)
{
	unsigned long index;
	unsigned char data;
	unsigned char address[4];

	dp_display_text("\r\nReading data at address: 0x");
	dp_display_value(start_address, HEX);
	dp_display_text(" - 0x");
	dp_display_value(start_address + number_of_bytes - 1u, HEX);
	dp_display_text("\r\n");

	enable_cs(jtag_gpio);
	spi_shift_byte_in(jtag_gpio, SPI_READ);

	for (index = 0; index < address_mode; index++) {
		address[index] = (start_address >> ((address_mode - 1 - index) * 8)) & 0xff;
		spi_shift_byte_in(jtag_gpio, address[index]);
	}
	spi_shift_dummy_bit(jtag_gpio);

	for (index = 0; index < number_of_bytes; index++) {
		spi_shift_byte_out(jtag_gpio, &data);
		page_global_buffer[index] = data;
	}
	disable_cs(jtag_gpio);
}

void SPI_verify_memory(struct gpio_handle *jtag_gpio, unsigned long start_address,
		       unsigned long number_of_bytes, unsigned char *data)
{
	unsigned long index;
	unsigned char actual_data;
	unsigned char address[4];

	enable_cs(jtag_gpio);
	spi_shift_byte_in(jtag_gpio, SPI_READ);

	for (index = 0; index < address_mode; index++) {
		address[index] = (start_address >> ((address_mode - 1 - index) * 8)) & 0xff;
		spi_shift_byte_in(jtag_gpio, address[index]);
	}
	spi_shift_dummy_bit(jtag_gpio);
	for (index = 0; index < number_of_bytes; index++) {
		spi_shift_byte_out(jtag_gpio, &actual_data);
		if (data[index] != actual_data) {
			error_code = DPE_SPI_FLASH_VERIFY_ERROR;
			dp_display_text(
			    "\r\nError: SPI-Flash verify operation failed at address: 0x");
			dp_display_value((start_address + index), HEX);
			dp_display_text(" - Expected value = 0x");
			dp_display_value(data[index], HEX);
			dp_display_text(" - Actual value = 0x");
			dp_display_value(actual_data, HEX);
			break;
		}

		bytes_processed++;
#ifdef ENABLE_DISPLAY
		new_progress = (bytes_processed * 100u / image_size);
		if (new_progress != old_progress) {
			dp_report_progress(new_progress);
			old_progress = new_progress;
		}
#endif
	}
	disable_cs(jtag_gpio);
}
#endif
/* *************** End of File *************** */
