// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpSPIalg.h                                              */
/*                                                                          */
/*  Description:    Contains common SPI-Flash function prototypes.          */
/*                                                                          */
/* ************************************************************************ */
#ifndef INC_DPSPIALG_H
#define INC_DPSPIALG_H

/* Programming method.  Common to all families */
extern unsigned long spi_target_address;
extern unsigned long long spi_flash_memory_byte_size;
extern unsigned long long bytes_processed;
extern unsigned char id_buf[3];
extern unsigned char spi_flash_vendor_id;
extern unsigned char spi_flash_memory_type_id;
extern unsigned char spi_flash_memory_size_id;
extern unsigned char address_mode;
extern unsigned int page_byte_size;
extern unsigned long sector_byte_size;

// SPI Flash memory specific actions/
#define DP_SPI_FLASH_READ_ID_ACTION_CODE       40u
#define DP_SPI_FLASH_READ_ACTION_CODE	       41u
#define DP_SPI_FLASH_PARTIAL_READ_ACTION_CODE  42u
#define DP_SPI_FLASH_ERASE_ACTION_CODE	       43u
#define DP_SPI_FLASH_PARTIAL_ERASE_ACTION_CODE 44u
#define DP_SPI_FLASH_PROGRAM_ACTION_CODE       45u
#define DP_SPI_FLASH_VERIFY_ACTION_CODE	       46u
#define DP_SPI_FLASH_BLANK_CHECK_ACTION_CODE   47u
#define DP_SPI_FLASH_ERASE_IMAGE_ACTION_CODE   48u

#define N4MBIT_BYTE_SIZE   0x80000u
#define N8MBIT_BYTE_SIZE   0x100000u
#define N16MBIT_BYTE_SIZE  0x200000u
#define N32MBIT_BYTE_SIZE  0x400000u
#define N64MBIT_BYTE_SIZE  0x800000u
#define N128MBIT_BYTE_SIZE 0x1000000u
#define N256MBIT_BYTE_SIZE 0x2000000u
#define N512MBIT_BYTE_SIZE 0x4000000u
#define N1GBIT_BYTE_SIZE   0x8000000u
#define N2GBIT_BYTE_SIZE   0x10000000u

#define ADDRESS_3BYTE_MODE 3u
#define ADDRESS_4BYTE_MODE 4u
#define TOP_SECTOR	   0u
#define BOTTOM_SECTOR	   1u

#define DPE_SPI_FLASH_DISABLE_SECURITY_ERROR 200u
#define DPE_SPI_FLASH_NVCR_ERROR	     201u
#define DPE_SPI_FLASH_ERASE_ERROR	     202u
#define DPE_SPI_FLASH_PROGRAM_ERROR	     203u
#define DPE_SPI_FLASH_VERIFY_ERROR	     204u
#define DPE_SPI_FLASH_BUFFER_SIZE_ERROR	     205u
#define DPE_SPI_FLASH_TIMEOUT_ERROR	     206u
#define DPE_SPI_FLASH_INVALID_IMAGE_ERROR    207u
#define DPE_ADDRESS_BOUNDARY_ERROR	     208u
#define DPE_IMAGE_SIZE_ERROR		     209u
#define DPE_SPI_FLASH_BLANK_CHECK_ERROR	     210u

#define TIMEOUT_MAX_VALUE 0x1000000u

// Common SPI-Flash opcodes
#define SPI_READ	0x03u
#define SPI_READ_ID_CMD 0x9fu

void dp_check_image_address_and_size(void);

unsigned char dp_top_spi_flash(struct gpio_handle *jtag_gpio);
void dp_SPI_read_idcode(struct gpio_handle *jtag_gpio);
void dp_SPI_blank_check_action(struct gpio_handle *jtag_gpio);
void dp_SPI_read_action(struct gpio_handle *jtag_gpio);
void dp_SPI_verify_action(struct gpio_handle *jtag_gpio);
void SPI_blank_check_memory(struct gpio_handle *jtag_gpio, unsigned long start_address,
			    unsigned long number_of_bytes);
void SPI_read_memory(struct gpio_handle *jtag_gpio, unsigned long start_address, unsigned long number_of_bytes);
void SPI_verify_memory(struct gpio_handle *jtag_gpio, unsigned long start_address,
		       unsigned long number_of_bytes, unsigned char *data);
#endif /* INC_DPSPIALG_H */

/* *************** End of File *************** */
