// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpuser.h                                                */
/*                                                                          */
/*  Description:    DP user specific file                                   */
/*                  users should define their own functions                 */
/*                                                                          */
/* ************************************************************************ */
#ifndef INC_DPUSER_H
#define INC_DPUSER_H

/* Compiler switches */
#define ENABLE_EMBEDDED_SUPPORT

#define ENABLE_DISPLAY
#define ENABLE_GPIO_SUPPORT
#define PERFORM_CRC_CHECK
#define ENABLE_SPI_FLASH_SUPPORT
#define ENABLE_G5_SUPPORT

//#define USE_PAGING
/* #define CHAIN_SUPPORT */
/* Enable BSR_SAMPLE switch maintains the last known state of the IOs regardless
 *  of the data file setting. */
/* #define BSR_SAMPLE */

/*************** End of compiler switches ***********************************/
struct gpio_handle {
	/*Hardware related constants*/
	struct gpiod_line *tck;
	struct gpiod_line *tdi;
	struct gpiod_line *tms;
	struct gpiod_line *trst;
	struct gpiod_line *tdo;
};

#define DPNULL ((void *)0)
#define TRUE   1U
#define FALSE  0U

#define GPIO_SEL 1u

extern unsigned char *image_buffer;
extern unsigned char hardware_interface;
extern unsigned char enable_mss_support;

void dp_exit_avionics_mode(void);
void dp_delay(unsigned long microseconds);

#ifdef ENABLE_EMBEDDED_SUPPORT
unsigned char jtag_inp(void);
void jtag_outp(unsigned char outdata);
void dp_jtag_init(struct gpio_handle *jtag_gpio);
void dp_jtag_tms(struct gpio_handle *jtag_gpio, unsigned char tms);
void dp_jtag_tms_tdi(struct gpio_handle *jtag_gpio, unsigned char tms, unsigned char tdi);
unsigned char dp_jtag_tms_tdi_tdo(struct gpio_handle *jtag_gpio, unsigned char tms, unsigned char tdi);
#endif

#ifdef ENABLE_DISPLAY
#define HEX 0u
#define DEC 1u
#define CHR 2u

/******************************************************************************/
/* users should define their own functions to replace the following functions */
/******************************************************************************/
void dp_display_text(signed char *text);
void dp_display_value(unsigned long value, unsigned int descriptive);
void dp_display_array(unsigned char *value, unsigned int bytes, unsigned int descriptive);
void dp_display_array_reverse(unsigned char *outbuf, unsigned int bytes, unsigned int descriptive);
void dp_report_progress(unsigned char value);
#define PRINT_DELAY 250

#endif

void *dp_malloc(unsigned long size);
void dp_free(void *ptr);
unsigned char dp_get_Action_code(signed char *pAction);
/***********************************************
**	The following string action definitions could be ignored
**	The user can call dp_top with the corresponding action code defined in dpalg.h
***********************************************/
#define DP_PROGRAM_ACTION		  "program"
#define DP_ERASE_ACTION			  "erase"
#define DP_READ_IDCODE_ACTION		  "read_idcode"
#define DP_VERIFY_ACTION		  "verify"
#define DP_DEVICE_INFO_ACTION		  "device_info"
#define DP_ENC_DATA_AUTHENTICATION_ACTION "enc_data_authentication"
#define DP_VERIFY_DIGEST		  "verify_digest"
#define DP_VALIDATE_USER_ENC_KEYS	  "validate_user_enc_keys"
#define DP_READ_DEVICE_CERTIFICATE	  "read_device_certificate"
#define DP_ZEROIZE_LIKE_NEW		  "zeroize_like_new"
#define DP_ZEROIZE_UNRECOVERABLE	  "zeroize_unrecoverable"
#define DP_SPI_FLASH_READ_ID		  "spi_flash_read_idcode"
#define DP_SPI_FLASH_READ		  "spi_flash_read"
#define DP_SPI_FLASH_ERASE		  "spi_flash_erase"
#define DP_SPI_FLASH_PROGRAM		  "spi_flash_program"
#define DP_SPI_FLASH_VERIFY		  "spi_flash_verify"
#define DP_SPI_FLASH_BLANK_CHECK	  "spi_flash_blank_check"

#endif /* INC_DPUSER_H */

/* *************** End of File *************** */
