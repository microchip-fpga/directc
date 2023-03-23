// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */

/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpalg.c                                                 */
/*                                                                          */
/*  Description:    Contains initialization and data checking functions.    */
/*                                                                          */
/* ************************************************************************ */
#include "dpalg.h"
#include "dpG5alg.h"
#include "dpjtag.h"
#include "dpSPIalg.h"
#include "dpSPIprog.h"
#include "dpcom.h"
#include "dputil.h"

unsigned char Action_code; /* used to hold the action codes as defined in dpalg.h */
unsigned char Action_done; /* used to hold the action codes as defined in dpalg.h */
unsigned char opcode;	     /* Holds the opcode value of the IR register prior to loading */

unsigned long device_ID;	    /* Holds the device ID */
unsigned char device_rev;	    /* Holds the device revision */
unsigned char device_family = 0U; /* Read from the data file AFS */
unsigned char device_exception = 0u;

#ifdef ENABLE_DISPLAY
unsigned long old_progress = 0;
unsigned long new_progress;
#endif

unsigned char bsr_buffer[MAX_BSR_BYTE_SIZE];
unsigned char bsr_sample_buffer[MAX_BSR_BYTE_SIZE];

/* DataIndex variable is used to keep track of the position of the data
 * loaded in the file
 */
unsigned long DataIndex;

/* error_code holds the error code that could be set in the programming
 * functions
 */
unsigned char error_code;
unsigned int unique_exit_code;
unsigned char core_is_enabled = 0xffu;

unsigned char dp_top(struct gpio_handle *jtag_gpio)
{
	error_code = DPE_SUCCESS;
	dp_init_com_vars();
	Action_done = FALSE;
#ifdef ENABLE_SPI_FLASH_SUPPORT
	if ((Action_code == DP_SPI_FLASH_READ_ID_ACTION_CODE) ||
	    (Action_code == DP_SPI_FLASH_READ_ACTION_CODE) ||
	    (Action_code == DP_SPI_FLASH_BLANK_CHECK_ACTION_CODE) ||
	    (Action_code == DP_SPI_FLASH_ERASE_ACTION_CODE) ||
	    (Action_code == DP_SPI_FLASH_PROGRAM_ACTION_CODE) ||
	    (Action_code == DP_SPI_FLASH_VERIFY_ACTION_CODE)) {
		goto_jtag_state(jtag_gpio, JTAG_TEST_LOGIC_RESET, 0u);
		dp_read_idcode(jtag_gpio);
		if ((device_ID & G5M_FAMILY_MASK) == (G5M_FAMILY)) {
			dp_top_spi_flash(jtag_gpio);
			Action_done = TRUE;
		}
	}
#endif

	if (Action_done == FALSE) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nIdentifying device...");
#endif

		dp_check_and_get_image_size();

		if (error_code == DPE_SUCCESS) {
			goto_jtag_state(jtag_gpio, JTAG_TEST_LOGIC_RESET, 0u);
			error_code = DPE_CODE_NOT_ENABLED;
			Action_done = FALSE;

#ifdef ENABLE_G5_SUPPORT
			if (Action_done == FALSE) {
#ifdef ENABLE_DISPLAY
				dp_display_text("\r\nLooking for MPF device...");
#endif
				error_code = DPE_SUCCESS;
				dp_read_idcode(jtag_gpio);
				dp_check_G5_device_ID();
				if ((error_code == DPE_SUCCESS) &&
				    ((device_family == G5_FAMILY) ||
				     (device_family == G5SOC_FAMILY))) {
					dp_top_g5(jtag_gpio);
					Action_done = TRUE;
				}
			}
#endif
		}
	}
	return error_code;
}

void dp_read_idcode(struct gpio_handle *jtag_gpio)
{
	opcode = IDCODE;
	IRSCAN_in(jtag_gpio);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, 0u);
	DRSCAN_out(jtag_gpio, IDCODE_LENGTH, (unsigned char *)DPNULL, global_buf1);
	device_ID = (unsigned long)global_buf1[0] | (unsigned long)global_buf1[1] << 8u |
		    (unsigned long)global_buf1[2] << 16u | (unsigned long)global_buf1[3] << 24u;
	device_rev = (unsigned char)(device_ID >> 28);

#ifdef ENABLE_DISPLAY
	dp_display_text("\r\nActID = ");
	dp_display_value(device_ID, HEX);
#endif

	return;
}

#ifdef ENABLE_DISPLAY
void dp_read_idcode_action(void) { return; }
#endif

/* *************** End of File *************** */
