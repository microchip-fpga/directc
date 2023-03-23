// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */
 
/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpuser.c                                                */
/*                                                                          */
/*  Description:    user specific file containing JTAG interface functions  */
/*                  and delay function                                      */
/*                                                                          */
/****************************************************************************/
#include "dpuser.h"
#include "dpSPIalg.h"
#include "dpalg.h"
#include "dpcom.h"

#include <ctype.h>
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* This variable is used to select external programming */
unsigned char hardware_interface = GPIO_SEL;
unsigned char enable_mss_support = FALSE;

#ifdef ENABLE_EMBEDDED_SUPPORT
/*
 * Module: dp_jtag_init
 * 		purpose: Set tck and trstb pins to logic level one
 * Arguments:
 * 		None
 * Return value: None
 *
 */
void dp_jtag_init(struct gpio_handle *jtag_gpio)
{
	gpiod_line_set_value(jtag_gpio->tck, 1);
	gpiod_line_set_value(jtag_gpio->trst, 1);
	return;
}

/*
 * Module: dp_jtag_tms
 * 		purpose: Set tms pin to a logic level one or zero and pulse tck.
 * Arguments:
 * 		tms: 8 bit value containing the new state of tms
 * Return value: None
 * Constraints: Since jtag_outp function sets all the jtag pins, jtag_port_reg is used
 * 				to modify the required jtag pins and preseve the reset.
 *
 */
void dp_jtag_tms(struct gpio_handle *jtag_gpio, unsigned char tms)
{
	gpiod_line_set_value(jtag_gpio->tms, tms);
	gpiod_line_set_value(jtag_gpio->tck, 0);
	gpiod_line_set_value(jtag_gpio->tck, 1);
	return;
}

/*
 * Module: dp_jtag_tms_tdi
 * 		purpose: Set tms amd tdi pins to a logic level one or zero and pulse tck.
 * Arguments:
 * 		tms: 8 bit value containing the new state of tms
 * 		tdi: 8 bit value containing the new state of tdi
 * Return value: None
 * Constraints: Since jtag_outp function sets all the jtag pins, jtag_port_reg is used
 * 				to modify the required jtag pins and preseve the reset.
 *
 */
void dp_jtag_tms_tdi(struct gpio_handle *jtag_gpio, unsigned char tms, unsigned char tdi)
{
	gpiod_line_set_value(jtag_gpio->tdi, tdi);
	gpiod_line_set_value(jtag_gpio->tms, tms);
	gpiod_line_set_value(jtag_gpio->tck, 0);

	gpiod_line_set_value(jtag_gpio->tck, 1);

	return;
}

/*
 * Module: dp_jtag_tms_tdi_tdo
 * 		purpose: Set tms amd tdi pins to a logic level one or zero,
 * 				 pulse tck and return tdi level
 * Arguments:
 * 		tms: 8 bit value containing the new state of tms
 * 		tdi: 8 bit value containing the new state of tdi
 * Return value:
 * 		ret: 8 bit variable ontaining the state of tdo.
 * Valid return values:
 * 		0x80: indicating a logic level high on tdo
 * 		0: indicating a logic level zero on tdo
 * Constraints: Since jtag_outp function sets all the jtag pins, jtag_port_reg is used
 * 				to modify the required jtag pins and preseve the reset.
 *
 */
unsigned char dp_jtag_tms_tdi_tdo(struct gpio_handle *jtag_gpio, unsigned char tms, unsigned char tdi)
{
	unsigned char ret = 0u;
	gpiod_line_set_value(jtag_gpio->tdi, tdi);
	gpiod_line_set_value(jtag_gpio->tms, tms);
	gpiod_line_set_value(jtag_gpio->tck, 0);
	if (gpiod_line_get_value(jtag_gpio->tdo))
		ret = 0x80;
	gpiod_line_set_value(jtag_gpio->tck, 1);

	return ret;
}

/*
 * User attention:
 * Module: dp_delay
 * 		purpose: Execute a time delay for a specified amount of time.
 * Arguments:
 * 		microseconeds: 32 bit value containing the amount of wait time in microseconds.
 * Return value: None
 *
 */
void dp_delay(unsigned long microseconds)
{
	usleep(microseconds);
	return;
}

#ifdef ENABLE_DISPLAY
void dp_report_progress(unsigned char value)
{
	if (old_progress == 0)
		dp_display_text("\n");
	dp_display_text("\rProgress: ");
	dp_display_value(value, DEC);
	dp_display_text(" %");

	return;
}

void dp_display_text(signed char *text)
{
	printf("%s", text);
	fflush(stdout);
	return;
}

void dp_display_value(unsigned long value, unsigned int descriptive)
{
	if (descriptive == HEX) {
		printf("%lX", value);
	} else if (descriptive == DEC) {
		printf("%2ld", value);
	} else if (descriptive == CHR) {
		printf("%c", (unsigned char)value);
	} else {
	}
	fflush(stdout);

	return;
}

void dp_display_array(unsigned char *outbuf, unsigned int bytes, unsigned int descriptive)
{
	unsigned int i;
	for (i = 0u; i < bytes; i++) {
		if ((i != 0) && (i % 16) == 0) {
			printf("\r\n");
		}
		if (descriptive == HEX) {
			printf("%2lX ", outbuf[bytes - i - 1]);
		} else if (descriptive == DEC) {
			printf("%ld ", outbuf[bytes - i - 1]);
		} else if (descriptive == CHR) {
			printf("%c ", (unsigned char)outbuf[bytes - i - 1]);
		} else {
		}
	}
	fflush(stdout);
	return;
}
void dp_display_array_reverse(unsigned char *outbuf, unsigned int bytes, unsigned int descriptive)
{
	unsigned int i;
	for (i = 0u; i < bytes; i++) {
		if ((i != 0) && (i % 16) == 0) {
			printf("\r\n");
		}
		if (descriptive == HEX) {
			printf("%2lX ", outbuf[i]);
		} else if (descriptive == DEC) {
			printf("%ld ", outbuf[i]);
		} else if (descriptive == CHR) {
			printf("%c ", (unsigned char)outbuf[i]);
		} else {
		}
	}
	fflush(stdout);
	return;
}

#endif
#endif

void *dp_malloc(unsigned long size) { return malloc(size); }

void dp_free(void *ptr) { free(ptr); }

unsigned char dp_get_Action_code(signed char *pAction)
{
	unsigned char Action_code_value = 0;
	if (strcasecmp(pAction, DP_DEVICE_INFO_ACTION) == 0) {
		Action_code_value = DP_DEVICE_INFO_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_READ_IDCODE_ACTION) == 0) {
		Action_code_value = DP_READ_IDCODE_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_ERASE_ACTION) == 0) {
		Action_code_value = DP_ERASE_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_PROGRAM_ACTION) == 0) {
		Action_code_value = DP_PROGRAM_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_VERIFY_ACTION) == 0) {
		Action_code_value = DP_VERIFY_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_ENC_DATA_AUTHENTICATION_ACTION) == 0) {
		Action_code_value = DP_ENC_DATA_AUTHENTICATION_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_VERIFY_DIGEST) == 0) {
		Action_code_value = DP_VERIFY_DIGEST_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_VALIDATE_USER_ENC_KEYS) == 0) {
		Action_code_value = DP_VALIDATE_USER_ENC_KEYS_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_READ_DEVICE_CERTIFICATE) == 0) {
		Action_code_value = DP_READ_DEVICE_CERTIFICATE_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_ZEROIZE_LIKE_NEW) == 0) {
		Action_code_value = DP_ZEROIZE_LIKE_NEW_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_ZEROIZE_UNRECOVERABLE) == 0) {
		Action_code_value = DP_ZEROIZE_UNRECOVERABLE_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_SPI_FLASH_READ_ID) == 0) {
		Action_code_value = DP_SPI_FLASH_READ_ID_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_SPI_FLASH_READ) == 0) {
		Action_code_value = DP_SPI_FLASH_READ_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_SPI_FLASH_ERASE) == 0) {
		Action_code_value = DP_SPI_FLASH_ERASE_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_SPI_FLASH_PROGRAM) == 0) {
		Action_code_value = DP_SPI_FLASH_PROGRAM_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_SPI_FLASH_VERIFY) == 0) {
		Action_code_value = DP_SPI_FLASH_VERIFY_ACTION_CODE;
	} else if (strcasecmp(pAction, DP_SPI_FLASH_BLANK_CHECK) == 0) {
		Action_code_value = DP_SPI_FLASH_BLANK_CHECK_ACTION_CODE;
	} else {
		Action_code_value = DP_NO_ACTION_FOUND;
	}
	return Action_code_value;
}

int gpio_config(struct gpio_handle *jtag_gpio)
{
	char compatible[100];
	char *gpiochip;
	unsigned int TCK_PIN, TDI_PIN, TMS_PIN, TRST_PIN, TDO_PIN;

	FILE *file = fopen("/proc/device-tree/compatible", "r");
	if(file != NULL && fscanf(file, "%s", compatible)) {

		if(strncmp(compatible, "ti,am335x-bone", 14) == 0) {
			gpiochip = "/dev/gpiochip1";
			TCK_PIN = 28;
			TDI_PIN = 16;
			TMS_PIN = 15;
			TRST_PIN = 14;
			TDO_PIN = 29; 
		}

		else if(strncmp(compatible, "raspberry", 9) == 0) {
			gpiochip = "/dev/gpiochip0";
			TCK_PIN = 4;
			TDI_PIN = 2;
			TMS_PIN = 3;
			TRST_PIN = 14;
			TDO_PIN = 15; 
		}
	}
	if (file != NULL) {
		fclose(file);
	}

	char *chipname = gpiochip;
	struct gpiod_chip *chip;
	chip = gpiod_chip_open(chipname);
	/* open the GPIO line */
	jtag_gpio->tck = gpiod_chip_get_line(chip, TCK_PIN);
	jtag_gpio->tdi = gpiod_chip_get_line(chip, TDI_PIN);
	jtag_gpio->tms = gpiod_chip_get_line(chip, TMS_PIN);
	jtag_gpio->trst = gpiod_chip_get_line(chip, TRST_PIN);
	jtag_gpio->tdo = gpiod_chip_get_line(chip, TDO_PIN);

	/* set the direction of GPIO */
	gpiod_line_request_output(jtag_gpio->tck, "gpio-tck", GPIOD_LINE_ACTIVE_STATE_HIGH);
	gpiod_line_request_output(jtag_gpio->tdi, "gpio-tdi", GPIOD_LINE_ACTIVE_STATE_HIGH);
	gpiod_line_request_output(jtag_gpio->tms, "gpio-tms", GPIOD_LINE_ACTIVE_STATE_HIGH);
	gpiod_line_request_output(jtag_gpio->trst, "gpio-trst", GPIOD_LINE_ACTIVE_STATE_HIGH);
	gpiod_line_request_input(jtag_gpio->tdo, "gpio-tdo");

	if (!chip) {
		printf("Error: Failed to initialize GPIO module.\n");
		return -1;
	}
}

void displayActions()
{
	printf("Usage: directc_programmer [-h] [-a<action>] [filename]\n");
	printf("-a<action>, Performs required action\n");
	printf("Available actions:\n");
	printf("\tprogram                 - Performs erase, program, and verify operations for supported blocks in data file\n");
	printf("\terase                   - Erases supported blocks in data file\n");
	printf("\tread_idcode             - Reads and displays the content of the IDCODE register\n");
	printf("\tverify                  - Performs verify operation for supported blocks in data file\n");
	printf("\tdevice_info             - Displays device design information and status, including security settings\n");
	printf("\tenc_data_authentication - Performs data authentication for array to make sure data was encrypted with same encryption key as the device\n");
	printf("\tverify_digest           - PolarFire specific action\n");
	printf("\tvalidate_user_enc_keys  - Validates user encryption keys\n");
	printf("\tread_device_certificate - Reads and displays device certificate\n");
	printf("\tzeroize_like_new        - Performs zeroization. Device is recoverable\n");
	printf("\tzeroize_unrecoverable   - Performs zeroization. Device is not recoverable\n");
	printf("\tspi_flash_read_idcode   - Returns 3 bytes ID data as a response to RDID (9Fh) instruction\n");
	printf("\tspi_flash_read          - Reads entire content of the SPI-Flash memory device. This operation could be extremely slow depending on how the read data is transmitted to host\n");
	printf("\tspi_flash_erase         - Erases entire content of the SPI-Flash memory device\n");
	printf("\tspi_flash_program       - Determines sectors needed to store the loaded image and then performs erasing of sectors followed by programming the image\n");
	printf("\tspi_flash_verify        - Verifies device content against loaded image. Only memory region occupied by loaded image is verified\n");
	printf("\tspi_flash_blank_check   - Verifies entire memory space of device is 0xFFh. This action can be very slow but is useful for debugging purposes\n\n");
	printf("-h, Print this message\n\n");

	printf("This program is built for arm-linux-gnueabihf-gcc \n");
}

int main(int argc, char **argv)
{
	signed int iExitStatus = 0;
	signed int iArg;
	signed char *pAction = (signed char *)DPNULL;
	signed char *pFileName = (signed char *)DPNULL;
	unsigned char bDATFileExists = FALSE;
	struct stat sglobal_buf1;
	unsigned long ulFileLength = 0L;
	unsigned char *pFile_buffer = (unsigned char *)DPNULL;
	FILE *fp = (FILE *)DPNULL;
	signed int iExecResult = DPE_SUCCESS;
	time_t start_time;
	time_t end_time;
	signed int iTimeDelta;
	struct gpio_handle *jtag_gpio = malloc(sizeof(struct gpio_handle));
	
	for (iArg = 1; iArg < argc; iArg++) {
		if ((argv[iArg][0] == '-')) {
			switch (toupper(argv[iArg][1])) {
				case 'A': /* set action name */
					pAction = &argv[iArg][2];
					if (argc < 3) {
#ifdef ENABLE_DISPLAY
						dp_display_text("-a<action> : specify action name\r\n\n");
						displayActions();
#endif
					return -1;
					}
					break;
				case 'H':
					displayActions();
					return 0;
					break;
				default:
					printf("Invalid option\n");
			}
		} else {
			/* it's a filename */
			pFileName = argv[iArg];
		}
	} 

	if ((pFileName != (signed char *)DPNULL)) {
		bDATFileExists = TRUE;
		/* get length of file */
		if (stat(pFileName, &sglobal_buf1) == 0)
			ulFileLength = sglobal_buf1.st_size;

		if ((fp = fopen(pFileName, "rb")) == (FILE *)DPNULL) {
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nError: can't open file \n");
			dp_display_text(pFileName);
#endif
			iExitStatus = 103;
			} else {
			/*
			 *    Read entire file into a global_buf1fer
			 */
			pFile_buffer = (unsigned char *)dp_malloc(ulFileLength);
			if (pFile_buffer == (unsigned char *)DPNULL) {
#ifdef ENABLE_DISPLAY
				dp_display_text("\r\nError: can't allocate memory (");
				dp_display_value(ulFileLength / 1024, DEC);
				dp_display_text(" Kbytes)\n");
#endif
				iExitStatus = 104;
			} else {
				if (fread(pFile_buffer, 1, (size_t)ulFileLength, fp) !=
				    (size_t)ulFileLength) {
#ifdef ENABLE_DISPLAY
					dp_display_text("\r\nError reading file \n");
					dp_display_text(pFileName);
#endif
					iExitStatus = 105;
				} else
					image_size = ulFileLength;
			}

			fclose(fp);
		}
	}
	if (iExitStatus == 0) {

		/*
		 *    Execute the directc program
		 */
		image_buffer = pFile_buffer;
		Action_code = dp_get_Action_code(pAction);
		if (bDATFileExists == FALSE) {
			time(&start_time);
			dp_display_text("\r\nError: Dat file is required...\n");
			iExecResult = 106;
			time(&end_time);
		} else {
			gpio_config(jtag_gpio);
			time(&start_time);
			iExecResult = dp_top(jtag_gpio);
			time(&end_time);
		}

		if (iExecResult != DPE_SUCCESS) {
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nError return code ");
			dp_display_value(iExecResult, DEC);
#endif
		} else {
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nExit code = 0... Success\n");
#endif
		}

		/*
		 *    Print out elapsed time
		 */
		iTimeDelta = (int)(end_time - start_time);
#ifdef ENABLE_DISPLAY
		printf("\r\nElapsed time = %02u:%02u:%02u", iTimeDelta / 3600, /* hours */
			 (iTimeDelta % 3600) / 60,				 /* minutes */
			 iTimeDelta % 60);					 /* seconds */
#endif
		/*
		 *    Print out elapsed time
		 */

		printf(" Done.\n");
}

	if (pFile_buffer != (unsigned char *)DPNULL)
		dp_free(pFile_buffer);
	return (iExitStatus);
}

/* *************** End of File *************** */
