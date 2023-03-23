// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpS25F.c                                                */
/*                                                                          */
/*  Description:    Contains S25F specific SPI-Flash functions.             */
/*                                                                          */
/* Device Manufacturer: Cypress / Spansion                                  */
/* Specs: https://www.cypress.com/file/177971/download                      */
/* Devices tested: S25FL512S                                                */
/* ************************************************************************ */

#include "dpuser.h"
#ifdef ENABLE_SPI_FLASH_SUPPORT
#include "dpjtag.h"
#include "dpalg.h"
#include "dpcom.h"
#include "dpS25F.h"
#include "dpSPIalg.h"
#include "dpSPIprog.h"

unsigned char dp_top_S25F(struct gpio_handle *jtag_gpio)
{
	if (error_code == DPE_SUCCESS) {
		dp_perform_S25F_action(jtag_gpio);
	}
	return error_code;
}

void dp_perform_S25F_action(struct gpio_handle *jtag_gpio)
{
	if (Action_code != DP_SPI_FLASH_READ_ID_ACTION_CODE) {
		S25F_reset(jtag_gpio);
		S25F_clear_status_register(jtag_gpio);
		S25F_write_status_register(jtag_gpio, 0x0);
		if ((address_mode == ADDRESS_4BYTE_MODE)) {
			dp_display_text("\r\nSetting address mode to 4 bytes in register");
			if (spi_flash_memory_type_id == CYPRESS_MEMORY_TYPE1_ID)
				S25F_write_bank_address_register(jtag_gpio, 0x80);
			else
				S25F_enable_4byte_address_mode(jtag_gpio);
		}
		if (error_code == DPE_SUCCESS) {
			switch (Action_code) {
			case DP_SPI_FLASH_READ_ACTION_CODE:
				dp_SPI_read_action(jtag_gpio);
				break;
			case DP_SPI_FLASH_ERASE_ACTION_CODE:
				dp_S25F_erase_action(jtag_gpio);
				break;
			case DP_SPI_FLASH_PROGRAM_ACTION_CODE:
				dp_S25F_program_action(jtag_gpio);
				break;
			case DP_SPI_FLASH_VERIFY_ACTION_CODE:
				dp_SPI_verify_action(jtag_gpio);
				break;
			case DP_SPI_FLASH_BLANK_CHECK_ACTION_CODE:
				dp_SPI_blank_check_action(jtag_gpio);
				break;
			case DP_SPI_FLASH_ERASE_IMAGE_ACTION_CODE:
				dp_S25F_image_erase_action(jtag_gpio);
				break;
			}
		}
		if (error_code == DPE_SUCCESS) {
			dp_display_text("\r\nOperation Status: Passed");
		} else {
			dp_display_text("\r\nError: Operation Status: Failed");
		}
	}
	return;
}

void dp_S25F_erase_action(struct gpio_handle *jtag_gpio)
{
	dp_display_text("\r\nPerforming SPI Flash Die Erase Action:\r\n");
	if (error_code == DPE_SUCCESS) {
		S25F_die_erase(jtag_gpio);
	}
}

void dp_S25F_image_erase_action(struct gpio_handle *jtag_gpio)
{
	dp_display_text("\r\nPerforming SPI Flash Image Erase Action: ");
	bytes_processed = 0u;

	if (error_code == DPE_SUCCESS) {
		dp_check_image_address_and_size();

		if (error_code == DPE_SUCCESS) {
			dp_S25F_erase(jtag_gpio);
		}
	}
	return;
}

void dp_S25F_program_action(struct gpio_handle *jtag_gpio)
{
	dp_display_text("\r\nPerforming SPI Flash Program Action: ");

	if (error_code == DPE_SUCCESS) {
		dp_check_image_address_and_size();

		if (error_code == DPE_SUCCESS) {
			dp_S25F_erase(jtag_gpio);
			bytes_processed = 0u;

			if (error_code == DPE_SUCCESS) {
#ifdef ENABLE_DISPLAY
				old_progress = 0;
				dp_display_text("\r\nProgramming... ");
#endif
				DataIndex = 0;
				do {
					page_buffer_ptr = dp_get_data(Header_ID, DataIndex * 8u);
					if (return_bytes > image_size)
						return_bytes = image_size;

					// Max buffer size should be multiple of 512 bytes which is
					// the minimum page size.
					S25F_program_memory(jtag_gpio,
							    spi_target_address + DataIndex,
							    return_bytes, page_buffer_ptr);
					if (error_code != DPE_SUCCESS)
						break;
					DataIndex += return_bytes;

				} while (DataIndex < image_size);
			}
		}
	}
	return;
}

void dp_S25F_erase(struct gpio_handle *jtag_gpio)
{
	unsigned long number_of_sectors_to_erase;
	unsigned long address_to_process;

	dp_display_text("\r\nSPI Flash memory region to erase: 0x");
	dp_display_value(spi_target_address, HEX);
	dp_display_text(" - 0x");
	dp_display_value(spi_target_address + image_size - 1u, HEX);
	dp_display_text(". Please wait...\r\n");

	// Only do this if the starting address is not sector aligned
	if ((spi_target_address % sector_byte_size) > 0u) {
		dp_display_text("\r\nWarning: SPI target address is not sector aligned.  Data in "
				"the entire sector will be erased. ");
	}

	address_to_process = spi_target_address;
	number_of_sectors_to_erase =
	    (unsigned long)((spi_target_address % sector_byte_size + image_size + sector_byte_size - 1) /
		      sector_byte_size);
	if (error_code == DPE_SUCCESS) {
		while (number_of_sectors_to_erase) {
			S25F_sector_erase(jtag_gpio, address_to_process);
			address_to_process += sector_byte_size;
			number_of_sectors_to_erase--;
			if (error_code != DPE_SUCCESS)
				break;
		}
	}

	return;
}

// SPI Flash memory specific functions.
void S25F_parse_idcode(void)
{
	spi_flash_memory_byte_size = 0;
	switch (spi_flash_memory_size_id) {
	case S25F_128MB_BYTE_SIZE_ID:
		dp_display_text("\r\n3 byte address mode is selected");
		address_mode = ADDRESS_3BYTE_MODE;
		page_byte_size = S25F_PAGE_256_BYTE_SIZE;
		sector_byte_size = S25F_SECTOR_64K_BYTE_SIZE;
		spi_flash_memory_byte_size = N128MBIT_BYTE_SIZE;
		break;
	case S25F_256MB_BYTE_SIZE_ID:
		address_mode = ADDRESS_4BYTE_MODE;
		page_byte_size = S25F_PAGE_256_BYTE_SIZE;
		sector_byte_size = S25F_SECTOR_64K_BYTE_SIZE;
		spi_flash_memory_byte_size = N256MBIT_BYTE_SIZE;
		break;
	case S25F_512MB_BYTE_SIZE_ID:
		address_mode = ADDRESS_4BYTE_MODE;
		page_byte_size = S25F_PAGE_512_BYTE_SIZE;
		sector_byte_size = S25F_SECTOR_256K_BYTE_SIZE;
		spi_flash_memory_byte_size = N512MBIT_BYTE_SIZE;
		break;
	}
}

void S25F_reset(struct gpio_handle *jtag_gpio)
{
	spi_scan(jtag_gpio, S25F_RESET_ENABLE, 0, DPNULL, DPNULL);
}

unsigned char S25F_busy_wait(struct gpio_handle *jtag_gpio)
{
	unsigned char status_register;
	unsigned long timeout = 0;

	do {
		status_register = S25F_read_status_register(jtag_gpio);
		if (timeout++ > TIMEOUT_MAX_VALUE) {
			dp_display_text("\r\nError: Time out polling error detected.");
			error_code = DPE_SPI_FLASH_TIMEOUT_ERROR;
			break;
		}
	} while ((status_register & 0x1) == 0x1);

	return status_register;
}

void S25F_clear_status_register(struct gpio_handle *jtag_gpio)
{
	spi_scan(jtag_gpio, S25F_CLEAR_STATUS_REGISTER_CMD, 0, DPNULL, DPNULL);
}

unsigned char S25F_read_status_register(struct gpio_handle *jtag_gpio)
{
	unsigned char data_out = 0;
	spi_scan(jtag_gpio, S25F_READ_STATUS_REGISTER_CMD, 1, DPNULL, &data_out);

	return data_out;
}

unsigned char S25F_read_status_register2(struct gpio_handle *jtag_gpio)
{
	unsigned char data_out = 0;
	spi_scan(jtag_gpio, S25F_READ_STATUS_REGISTER2_CMD, 1, DPNULL, &data_out);

	return data_out;
}

unsigned char S25F_read_configuration_register(struct gpio_handle *jtag_gpio)
{
	unsigned char data_out = 0;
	spi_scan(jtag_gpio, S25F_READ_CONFIGURATION_REGISTER_CMD, 1, DPNULL, &data_out);

	return data_out;
}

void S25F_write_status_register(struct gpio_handle *jtag_gpio, unsigned char data_in)
{
	S25F_write_enable(jtag_gpio);
	spi_scan(jtag_gpio, S25F_WRITE_STATUS_REGISTER_CMD, 1, &data_in, DPNULL);
	S25F_busy_wait(jtag_gpio);
}

unsigned char S25F_read_bank_address_register(struct gpio_handle *jtag_gpio)
{
	unsigned char data_out = 0;
	spi_scan(jtag_gpio, S25F_READ_BANK_ADDRESS_REGISTER_CMD, 1, DPNULL, &data_out);

	return data_out;
}

void S25F_write_bank_address_register(struct gpio_handle *jtag_gpio, unsigned char data_in)
{
	S25F_write_enable(jtag_gpio);
	spi_scan(jtag_gpio, S25F_WRITE_BANK_ADDREES_REGISTER_CMD, 1, &data_in, DPNULL);
	S25F_busy_wait(jtag_gpio);
}

void S25F_write_enable(struct gpio_handle *jtag_gpio)
{
	spi_scan(jtag_gpio, S25F_WRITE_ENABLE_CMD, 0, DPNULL, DPNULL);
}

void S25F_write_disable(struct gpio_handle *jtag_gpio)
{
	spi_scan(jtag_gpio, S25F_WRITE_DISABLE_CMD, 0, DPNULL, DPNULL);
}

void S25F_die_erase(struct gpio_handle *jtag_gpio)
{
	unsigned char status_register;
	dp_display_text("\r\nPerforming die erase. Please wait...");
	S25F_write_enable(jtag_gpio);
	spi_scan(jtag_gpio, S25F_DIE_ERASE_CMD, 0, DPNULL, DPNULL);
	status_register = S25F_busy_wait(jtag_gpio);
	if (error_code == DPE_SUCCESS) {
		if (spi_flash_memory_type_id == CYPRESS_MEMORY_TYPE3_ID) {
			status_register = S25F_read_status_register2(jtag_gpio);
			if ((status_register & SR2_ERASE_ERROR_BIT) == SR2_ERASE_ERROR_BIT) {
				dp_display_text("\nError: Failed to erase SPI Flash.");
				error_code = DPE_SPI_FLASH_ERASE_ERROR;
			}
		} else {
			if ((status_register & SR1_ERASE_ERROR_BIT) == SR1_ERASE_ERROR_BIT) {
				dp_display_text("\nError: Failed to erase SPI Flash.");
				error_code = DPE_SPI_FLASH_ERASE_ERROR;
			}
		}
	}
}

void S25F_sector_erase(struct gpio_handle *jtag_gpio, unsigned long start_address)
{
	unsigned char index;
	unsigned char address[4] = {0x0, 0x0, 0x0, 0x0};
	unsigned char status_register;
	for (index = 0; index < address_mode; index++) {
		address[index] = (start_address >> ((address_mode - 1 - index) * 8)) & 0xff;
	}
	enable_cs(jtag_gpio);
	spi_shift_byte_in(jtag_gpio, S25F_WRITE_ENABLE_CMD);
	disable_cs(jtag_gpio);
	spi_scan(jtag_gpio, S25F_SECTOR_ERASE_CMD, address_mode, address, DPNULL);
	status_register = S25F_busy_wait(jtag_gpio);
	if (error_code == DPE_SUCCESS) {
		if (spi_flash_memory_type_id == CYPRESS_MEMORY_TYPE3_ID) {
			status_register = S25F_read_status_register2(jtag_gpio);
			if ((status_register & SR2_ERASE_ERROR_BIT) == SR2_ERASE_ERROR_BIT) {
				dp_display_text("\nError: Failed to erase SPI Flash.");
				error_code = DPE_SPI_FLASH_ERASE_ERROR;
			}
		} else {
			if ((status_register & SR1_ERASE_ERROR_BIT) == SR1_ERASE_ERROR_BIT) {
				dp_display_text("\nError: Failed to erase SPI Flash.");
				error_code = DPE_SPI_FLASH_ERASE_ERROR;
			}
		}
	}
}

// Start programming address must be subsector aligned which is page aligned.
void S25F_program_memory(struct gpio_handle *jtag_gpio, unsigned long start_address,
			 unsigned long number_of_bytes, unsigned char *data)
{
	unsigned long index;
	unsigned char address[4] = {0x0, 0x0, 0x0, 0x0};
	unsigned long page_bytes;
	unsigned long processed_page_bytes = 0;
	unsigned char status_register;

	if (error_code == DPE_SUCCESS) {
		// Align the data with the first page by only programming valid data in page
		page_bytes = page_byte_size - start_address % page_byte_size;
		if (page_bytes > number_of_bytes)
			page_bytes = number_of_bytes;

		do {
			S25F_write_enable(jtag_gpio);
			enable_cs(jtag_gpio);
			spi_shift_byte_in(jtag_gpio, S25F_PAGE_PROGRAM_CMD);

			for (index = 0; index < address_mode; index++) {
				address[index] =
				    (start_address >> ((address_mode - 1 - index) * 8)) & 0xff;
				spi_shift_byte_in(jtag_gpio, address[index]);
			}

			for (index = processed_page_bytes;
			     index < (processed_page_bytes + page_bytes); index++) {
				spi_shift_byte_in(jtag_gpio, data[index]);
			}
			disable_cs(jtag_gpio);
			status_register = S25F_busy_wait(jtag_gpio);
			if (error_code == DPE_SUCCESS) {
				if (spi_flash_memory_type_id == CYPRESS_MEMORY_TYPE3_ID) {
					status_register = S25F_read_status_register2(jtag_gpio);
					if ((status_register & SR2_PROGRAM_ERROR_BIT) ==
					    SR2_PROGRAM_ERROR_BIT) {
						dp_display_text(
						    "\nError: Failed to program SPI Flash.");
						error_code = DPE_SPI_FLASH_PROGRAM_ERROR;
						break;
					}
				} else {
					if ((status_register & SR1_PROGRAM_ERROR_BIT) ==
					    SR1_PROGRAM_ERROR_BIT) {
						dp_display_text(
						    "\nError: Failed to program SPI Flash.");
						error_code = DPE_SPI_FLASH_PROGRAM_ERROR;
						break;
					}
				}
			} else {
				break;
			}
			number_of_bytes -= page_bytes;
			start_address += page_bytes;
			bytes_processed += page_bytes;
			processed_page_bytes += page_bytes;

#ifdef ENABLE_DISPLAY
			new_progress = (bytes_processed * 100u / image_size);
			if (new_progress != old_progress) {
				dp_report_progress(new_progress);
				old_progress = new_progress;
			}
#endif
			page_bytes = number_of_bytes;
			if (page_bytes > page_byte_size)
				page_bytes = page_byte_size;

		} while (number_of_bytes > 0);
	}
}

void S25F_enable_4byte_address_mode(struct gpio_handle *jtag_gpio)
{
	spi_scan(jtag_gpio, S25F_ENABLE_4BYTE_ADDRESS_CMD, 0, DPNULL, DPNULL);
}

#endif /* ENABLE_SPI_FLASH_SUPPORT */
