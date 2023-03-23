// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpS25F.h                                                */
/*                                                                          */
/*  Description:    Contains S25F specific SPI-Flash function prototypes.   */
/*                                                                          */
/* Device Manufacturer: Cypress / Spansion                                  */
/* ************************************************************************ */

#ifndef INC_DPS25L_H
#define INC_DPS25L_H

#define S25F_RESET_ENABLE 0xF0u

#define S25F_WRITE_ENABLE_CMD  0x06u
#define S25F_WRITE_DISABLE_CMD 0x04u

#define S25F_READ_STATUS_REGISTER_CMD  0x05u
#define S25F_WRITE_STATUS_REGISTER_CMD 0x01u
#define S25F_CLEAR_STATUS_REGISTER_CMD 0x30u

#define S25F_READ_STATUS_REGISTER2_CMD	     0x07u
#define S25F_READ_CONFIGURATION_REGISTER_CMD 0x35u

#define S25F_READ_BANK_ADDRESS_REGISTER_CMD  0x16u
#define S25F_WRITE_BANK_ADDREES_REGISTER_CMD 0x17u

#define S25F_PAGE_PROGRAM_CMD	      0x02u
#define S25F_SECTOR_ERASE_CMD	      0xD8u
#define S25F_DIE_ERASE_CMD	      0xC7u
#define S25F_ENABLE_4BYTE_ADDRESS_CMD 0xB7u

#define CYPRESS_VENDOR_ID	0x01u
#define CYPRESS_MEMORY_TYPE1_ID 0x02u // 512 Mb Sample device
#define CYPRESS_MEMORY_TYPE2_ID 0x20u // 128 Mb Sample device
#define CYPRESS_MEMORY_TYPE3_ID 0x60u // 256 Mb Sample device

#define S25F_128MB_BYTE_SIZE_ID 0x18u
#define S25F_256MB_BYTE_SIZE_ID 0x19u
#define S25F_512MB_BYTE_SIZE_ID 0x20u

#define S25F_PAGE_256_BYTE_SIZE 256u
#define S25F_PAGE_512_BYTE_SIZE 512u

#define S25F_SECTOR_64K_BYTE_SIZE  65536u
#define S25F_SECTOR_256K_BYTE_SIZE 262144u

#define SR1_ERASE_ERROR_BIT   0x20u
#define SR1_PROGRAM_ERROR_BIT 0x40u

#define SR1_ERASE_ERROR_BIT   0x20u
#define SR1_PROGRAM_ERROR_BIT 0x40u

#define SR2_ERASE_ERROR_BIT   0x40u
#define SR2_PROGRAM_ERROR_BIT 0x20u

unsigned char dp_top_S25F(struct gpio_handle *jtag_gpio);
void dp_perform_S25F_action(struct gpio_handle *jtag_gpio);
void dp_S25F_erase_action(struct gpio_handle *jtag_gpio);
void dp_S25F_image_erase_action(struct gpio_handle *jtag_gpio);
void dp_S25F_program_action(struct gpio_handle *jtag_gpio);
void dp_S25F_erase(struct gpio_handle *jtag_gpio);

void S25F_parse_idcode(void);
void S25F_reset(struct gpio_handle *jtag_gpio);
unsigned char S25F_busy_wait(struct gpio_handle *jtag_gpio);
void S25F_clear_status_register(struct gpio_handle *jtag_gpio);
unsigned char S25F_read_status_register(struct gpio_handle *jtag_gpio);
unsigned char S25F_read_status_register2(struct gpio_handle *jtag_gpio);
unsigned char S25F_read_configuration_register(struct gpio_handle *jtag_gpio);
void S25F_write_status_register(struct gpio_handle *jtag_gpio, unsigned char data_in);
unsigned char S25F_read_bank_address_register(struct gpio_handle *jtag_gpio);
void S25F_write_bank_address_register(struct gpio_handle *jtag_gpio, unsigned char data_in);
void S25F_write_enable(struct gpio_handle *jtag_gpio);
void S25F_write_disable(struct gpio_handle *jtag_gpio);
void S25F_enable_4byte_address_mode(struct gpio_handle *jtag_gpio);

void S25F_die_erase(struct gpio_handle *jtag_gpio);
void S25F_sector_erase(struct gpio_handle *jtag_gpio, unsigned long start_address);
void S25F_program_memory(struct gpio_handle *jtag_gpio, unsigned long start_address,
			 unsigned long number_of_bytes, unsigned char *data);
#endif
