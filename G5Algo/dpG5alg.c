// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */

/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpG5alg.c                                               */
/*                                                                          */
/*  Description:    Contains initialization and data checking functions.    */
/*                                                                          */
/* ************************************************************************ */

#include "dpuser.h"
#ifdef ENABLE_G5_SUPPORT
#include "dpjtag.h"
#include "dpalg.h"
#include "dpcom.h"
#include "dputil.h"
#include "dpG5alg.h"

unsigned char g5_pgmmode;
unsigned char g5_pgmmode_flag;
unsigned char g5_shared_buf[1024];
unsigned char g5_poll_buf[17];
unsigned long g5_poll_index;
unsigned char g5_component_digest[32];
unsigned char g5_component_type;
unsigned char g5_componenet_Supports_Cert;

unsigned int g5_prev_failed_component = 0;
unsigned long g5_prev_failed_block = 0;
unsigned int g5_prev_unique_error_code = 0;

unsigned int g5_current_failed_component = 0;
unsigned long g5_current_failed_block = 0;
unsigned int g5_current_unique_error_code = 0;

/****************************************************************************
 * Purpose: main entry function
 *  This function needs to be called from the main application function with
 *  the approppriate action code set to intiate the desired action.
 ****************************************************************************/
unsigned char dp_top_g5(struct gpio_handle *jtag_gpio)
{
	dp_init_vars();
	dp_init_G5_vars();
	goto_jtag_state(jtag_gpio, JTAG_TEST_LOGIC_RESET, 0u);
	dp_check_G5_action();
	if (error_code == DPE_SUCCESS) {
		dp_perform_G5_action(jtag_gpio);
	}

	return error_code;
}

void dp_init_G5_vars(void)
{
	g5_pgmmode = 0u;
	g5_pgmmode_flag = FALSE;

	return;
}

void dp_check_G5_action(void)
{
	if ((Action_code == DP_READ_IDCODE_ACTION_CODE) ||
	    (Action_code == DP_DEVICE_INFO_ACTION_CODE)) {
#ifndef ENABLE_DISPLAY
		error_code = DPE_CODE_NOT_ENABLED;
#endif
	} else if (!((Action_code == DP_ERASE_ACTION_CODE) ||
		     (Action_code == DP_PROGRAM_ACTION_CODE) ||
		     (Action_code == DP_VERIFY_ACTION_CODE) ||
		     (Action_code == DP_ENC_DATA_AUTHENTICATION_ACTION_CODE) ||
		     (Action_code == DP_VERIFY_DIGEST_ACTION_CODE) ||
		     (Action_code == DP_READ_DEVICE_CERTIFICATE_ACTION_CODE) ||
		     (Action_code == DP_ZEROIZE_LIKE_NEW_ACTION_CODE) ||
		     (Action_code == DP_ZEROIZE_UNRECOVERABLE_ACTION_CODE))) {
		error_code = DPE_ACTION_NOT_SUPPORTED;
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nInvalid action.");
#endif
	}
	return;
}

void dp_perform_G5_action(struct gpio_handle *jtag_gpio)
{
	Action_done = FALSE;
	dp_G5M_poll_device_ready(jtag_gpio);
	if (error_code == DPE_SUCCESS) {
		switch (Action_code) {
		case DP_ZEROIZE_LIKE_NEW_ACTION_CODE:
			Action_done = TRUE;
			dp_G5M_zeroize_like_new_action(jtag_gpio);
			break;
		case DP_ZEROIZE_UNRECOVERABLE_ACTION_CODE:
			Action_done = TRUE;
			dp_G5M_zeroize_unrecoverable_action(jtag_gpio);
			break;
#ifdef ENABLE_DISPLAY
		case DP_READ_IDCODE_ACTION_CODE:
			Action_done = TRUE;
			dp_read_idcode_action();
			break;
		case DP_DEVICE_INFO_ACTION_CODE:
			Action_done = TRUE;
			dp_G5M_device_info_action(jtag_gpio);
			break;
		case DP_READ_DEVICE_CERTIFICATE_ACTION_CODE:
			Action_done = TRUE;
			dp_G5M_read_device_certificate_action(jtag_gpio);
			break;
#endif
		}
		if (Action_done == FALSE) {
			dp_G5M_display_bitstream_digest();
			dp_check_image_crc();
			if (error_code == DPE_SUCCESS) {
				dp_check_G5_device_ID();
				if (error_code == DPE_SUCCESS) {
					switch (Action_code) {
					case DP_ERASE_ACTION_CODE:
						Action_done = TRUE;
						dp_G5M_erase_action(jtag_gpio);
						break;
					case DP_PROGRAM_ACTION_CODE:
						Action_done = TRUE;
						dp_G5M_program_action(jtag_gpio);
						break;
					case DP_VERIFY_ACTION_CODE:
						Action_done = TRUE;
						dp_G5M_verify_action(jtag_gpio);
						break;
					case DP_ENC_DATA_AUTHENTICATION_ACTION_CODE:
						Action_done = TRUE;
						dp_G5M_enc_data_authentication_action(jtag_gpio);
						break;
					case DP_VERIFY_DIGEST_ACTION_CODE:
						Action_done = TRUE;
						dp_G5M_verify_digest_action(jtag_gpio);
						break;
					}
				}
			}
		}
		dp_G5M_exit(jtag_gpio);
	}
	return;
}

void dp_G5M_erase_action(struct gpio_handle *jtag_gpio)
{
#ifdef ENABLE_DISPLAY
	dp_display_text("\r\nPerforming ERASE action: ");
#endif
	dp_G5M_initialize(jtag_gpio);
	if (error_code == DPE_SUCCESS) {
		g5_pgmmode = 0x1u;
		dp_G5M_set_mode(jtag_gpio);

		if (error_code == DPE_SUCCESS) {
			/* Global unit1 is used to hold the number of components */
			global_uint1 = (unsigned int)dp_get_bytes(Header_ID, G5M_NUMOFCOMPONENT_OFFSET,
							    G5M_NUMOFCOMPONENT_BYTE_LENGTH);
			global_uint2 = global_uint1 -
				       ((unsigned int)dp_get_bytes(Header_ID, G5M_ERASEDATASIZE_OFFSET,
							     G5M_DATASIZE_BYTE_LENGTH) -
					1u);

			dp_G5M_process_data(jtag_gpio, G5M_erasedatastream_ID);

			if (error_code != DPE_SUCCESS) {
				error_code = DPE_ERASE_ERROR;
			}
		}
	}
	return;
}

void dp_G5M_clear_errors(struct gpio_handle *jtag_gpio)
{
	g5_prev_failed_component = 0;
	g5_current_failed_component = 0;
	g5_prev_failed_block = 0;
	g5_current_failed_block = 0;

	return;
}

void dp_G5M_program_action(struct gpio_handle *jtag_gpio)
{
#ifdef ENABLE_DISPLAY
	dp_display_text("\r\nPerforming PROGORAM action: ");
#endif
	dp_G5M_initialize(jtag_gpio);
	if (error_code == DPE_SUCCESS) {
		dp_G5M_do_program(jtag_gpio);
	}

	return;
}

void dp_G5M_do_program(struct gpio_handle *jtag_gpio)
{
#ifdef ENABLE_DISPLAY
	dp_display_text("\r\nPerforming stand alone program...");
#endif
	dp_G5M_check_cycle_count(jtag_gpio);
	if (error_code == DPE_SUCCESS) {
		g5_pgmmode = 0x1u;
		dp_G5M_set_mode(jtag_gpio);

		if (error_code == DPE_SUCCESS) {
			global_uint1 = (unsigned int)dp_get_bytes(Header_ID, G5M_DATASIZE_OFFSET,
							    G5M_DATASIZE_BYTE_LENGTH);
			global_uint2 = 1u;
			dp_G5M_process_data(jtag_gpio, G5M_datastream_ID);

			if (error_code != DPE_SUCCESS) {
				error_code = DPE_CORE_PROGRAM_ERROR;
			}
		}
	}
	return;
}

void dp_G5M_verify_action(struct gpio_handle *jtag_gpio)
{
#ifdef ENABLE_DISPLAY
	dp_display_text("\r\nPerforming VERIFY action: ");
#endif

	dp_G5M_initialize(jtag_gpio);
	if (error_code == DPE_SUCCESS) {
		dp_G5M_do_verify(jtag_gpio);
	}

	return;
}

void dp_G5M_do_verify(struct gpio_handle *jtag_gpio)
{
#ifdef ENABLE_DISPLAY
	dp_display_text("\r\nPerforming stand alone verify...");
#endif

	g5_pgmmode = 0x2u;
	dp_G5M_set_mode(jtag_gpio);

	if (error_code == DPE_SUCCESS) {
		/* Global unit1 is used to hold the number of components */
		global_uint2 = 1u;
		global_uint1 =
		    (unsigned int)dp_get_bytes(Header_ID, G5M_DATASIZE_OFFSET, G5M_DATASIZE_BYTE_LENGTH);
		dp_G5M_process_data(jtag_gpio, G5M_datastream_ID);
		if (error_code != DPE_SUCCESS) {
			error_code = DPE_VERIFY_ERROR;
		}
	}
	return;
}

void dp_G5M_enc_data_authentication_action(struct gpio_handle *jtag_gpio)
{
#ifdef ENABLE_DISPLAY
	dp_display_text("\r\nPerforming AUTHENTICATION action: ");
#endif
	dp_G5M_initialize(jtag_gpio);
	if (error_code == DPE_SUCCESS) {
		g5_pgmmode = 0x0u;
		dp_G5M_set_mode(jtag_gpio);

		if (error_code == DPE_SUCCESS) {
			/* Global unit1 is used to hold the number of components */
			global_uint1 = (unsigned int)dp_get_bytes(Header_ID, G5M_DATASIZE_OFFSET,
							    G5M_DATASIZE_BYTE_LENGTH);
			global_uint2 = 1u;

			dp_G5M_process_data(jtag_gpio, G5M_datastream_ID);

			if (error_code != DPE_SUCCESS) {
				error_code = DPE_AUTHENTICATION_FAILURE;
			}
		}
	}
	return;
}

void dp_G5M_verify_digest_action(struct gpio_handle *jtag_gpio)
{
#ifdef ENABLE_DISPLAY
	dp_display_text("\r\nPerforming VERIFY_DIGEST action: ");
#endif
	dp_G5M_initialize(jtag_gpio);
	if (error_code == DPE_SUCCESS) {
		dp_G5M_query_security(jtag_gpio);
	}
	if ((error_code == DPE_SUCCESS) &&
	    ((g5_shared_buf[1] & G5M_UL_EXTERNAL_DIGEST_CHECK) == G5M_UL_EXTERNAL_DIGEST_CHECK)) {
#ifdef ENABLE_DISPLAY
		dp_display_text("r\nExternal digest check via JTAG/SPI Slave is disabled.");
#endif
		error_code = DPE_VERIFY_DIGEST_ERROR;
	}

	if (error_code == DPE_SUCCESS) {
		global_buf1[0] = 0x1u;

		opcode = G5M_CHECK_DIGESTS;
		IRSCAN_in(jtag_gpio);
		DRSCAN_in(jtag_gpio, 0u, G5M_SECURITY_STATUS_REGISTER_BIT_LENGTH, global_buf1);
		goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
		dp_delay(G5M_STANDARD_DELAY);
		dp_G5M_device_poll(jtag_gpio, 16u, 15u);

		if (error_code != DPE_SUCCESS) {
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nFailed to verify digest.");
#endif
			error_code = DPE_VERIFY_DIGEST_ERROR;
		} else {
			if (g5_poll_buf[1] == 0x40u) {
#ifdef ENABLE_DISPLAY
				dp_display_text("\r\nFailed to verify digest.");
#endif
				error_code = DPE_VERIFY_DIGEST_ERROR;
			} else {
#ifdef ENABLE_DISPLAY
				if ((g5_poll_buf[0] & 0x1u) == 0x1u) {
					dp_display_text(
					    "\r\n --- FPGA Fabric digest verification: PASS");
				} else {
					dp_display_text("\r\nWarning: --- FPGA Fabric digest "
							"verification: FAIL");
				}
				if ((g5_poll_buf[0] & 0x2u) == 0x2u) {
					dp_display_text("\r\n --- Fabric Configuration digest "
							"verification: PASS");
				} else {
					dp_display_text("\r\nWarning: --- Fabric Configuration "
							"digest verification: FAIL");
				}
				if ((g5_poll_buf[0] & 0x4u) == 0x4u) {
					dp_display_text("\r\n --- sNVM digest verification: PASS");
				} else {
					dp_display_text(
					    "\r\nWarning: --- sNVM digest verification: FAIL");
				}
				if ((g5_poll_buf[0] & 0x8u) == 0x8u) {
					dp_display_text("\r\n --- User security policies segment "
							"digest verification: PASS");
				} else {
					dp_display_text("\r\nWarning: --- User security policies "
							"segment digest verification: FAIL");
				}
				if ((g5_poll_buf[0] & 0x10u) == 0x10u) {
					dp_display_text(
					    "\r\n --- SMK segment digest verification: PASS");
				} else {
					dp_display_text("\r\nWarning: --- SMK segment digest "
							"verification: FAIL");
				}
				if ((g5_poll_buf[0] & 0x20u) == 0x20u) {
					dp_display_text("\r\n --- User Public Key segment digest "
							"verification: PASS");
				} else {
					dp_display_text("\r\nWarning: --- User Public Key segment "
							"digest verification: FAIL");
				}
				if ((g5_poll_buf[0] & 0x40u) == 0x40u) {
					dp_display_text(
					    "\r\n --- UPK1 segment digest verification: PASS");
				} else {
					dp_display_text("\r\nWarning: --- UPK1 segment digest "
							"verification: FAIL");
				}
				if ((g5_poll_buf[0] & 0x80u) == 0x80u) {
					dp_display_text(
					    "\r\n --- UEK1 segment digest verification: PASS");
				} else {
					dp_display_text("\r\nWarning: --- UEK1 segment digest "
							"verification: FAIL");
				}
				if ((g5_poll_buf[1] & 0x1u) == 0x1u) {
					dp_display_text(
					    "\r\n --- DPK segment digest verification: PASS");
				} else {
					dp_display_text("\r\nWarning: --- DPK segment digest "
							"verification: FAIL");
				}
				if ((g5_poll_buf[1] & 0x2u) == 0x2u) {
					dp_display_text(
					    "\r\n --- UPK2 segment digest verification: PASS");
				} else {
					dp_display_text("\r\nWarning: --- UPK2 segment digest "
							"verification: FAIL");
				}
				if ((g5_poll_buf[1] & 0x4u) == 0x4u) {
					dp_display_text(
					    "\r\n --- UEK2 segment digest verification: PASS");
				} else {
					dp_display_text("\r\nWarning: --- UEK2 segment digest "
							"verification: FAIL");
				}
				if ((g5_poll_buf[1] & 0x10u) == 0x10u) {
					dp_display_text("\r\n --- Factory row and factory key "
							"segment digest verification: PASS");
				} else {
					dp_display_text("\r\nWarning: --- Factory row and factory "
							"key segment digest verification: FAIL");
				}
#endif
			}
		}
	}
	return;
}

void dp_G5M_zeroize_like_new_action(struct gpio_handle *jtag_gpio)
{
#ifdef ENABLE_DISPLAY
	dp_display_text("\r\nPerforming ZEROIZE_LIKE_NEW action: ");
#endif
	dp_G5M_query_security(jtag_gpio);
	if ((error_code == DPE_SUCCESS) &&
	    ((g5_shared_buf[7] & G5M_UL_USER_KEY1) == G5M_UL_USER_KEY1)) {
		dp_G5M_unlock_upk1(jtag_gpio);
	}
	if ((error_code == DPE_SUCCESS) &&
	    ((g5_shared_buf[7] & G5M_UL_USER_KEY2) == G5M_UL_USER_KEY2)) {
		dp_G5M_unlock_upk2(jtag_gpio);
	}
	if (error_code == DPE_SUCCESS) {
		dp_G5M_do_zeroize(jtag_gpio, 1);
	}
	return;
}

void dp_G5M_zeroize_unrecoverable_action(struct gpio_handle *jtag_gpio)
{
#ifdef ENABLE_DISPLAY
	dp_display_text("\r\nPerforming ZEROIZE_UNRECOVERABLE action: ");
#endif
	dp_G5M_query_security(jtag_gpio);
	if ((error_code == DPE_SUCCESS) &&
	    ((g5_shared_buf[7] & G5M_UL_USER_KEY1) == G5M_UL_USER_KEY1)) {
		dp_G5M_unlock_upk1(jtag_gpio);
	}
	if ((error_code == DPE_SUCCESS) &&
	    ((g5_shared_buf[7] & G5M_UL_USER_KEY2) == G5M_UL_USER_KEY2)) {
		dp_G5M_unlock_upk2(jtag_gpio);
	}
	if (error_code == DPE_SUCCESS) {
		dp_G5M_do_zeroize(jtag_gpio, 3);
	}

	return;
}

void dp_G5M_check_core_status(struct gpio_handle *jtag_gpio)
{

	opcode = G5M_ISC_NOOP;
	IRSCAN_out(jtag_gpio, &global_uchar1);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, 1u);

	if ((global_uchar1 & 0x80u) == 0x80u) {
		core_is_enabled = 1;
	} else {
		core_is_enabled = 0;
	}

	return;
}

#ifdef ENABLE_DISPLAY
void dp_G5M_display_core_status(void)
{
	if (core_is_enabled == 1) {
		dp_display_text("\r\nFPGA Array is programmed and enabled.");
	} else if (core_is_enabled == 0) {
		dp_display_text("\r\nFPGA Array is not enabled.");
	} else {
		dp_display_text("\r\nWarning: CoreEnable bit is not inspected.");
	}
	return;
}

void dp_G5M_read_device_certificate_action(struct gpio_handle *jtag_gpio)
{
	dp_G5M_read_certificate(jtag_gpio);
	return;
}

void dp_G5M_device_info_action(struct gpio_handle *jtag_gpio)
{
	dp_display_text("\r\n\r\nDevice info:");
	dp_G5M_read_udv(jtag_gpio);
	dp_G5M_check_core_status(jtag_gpio);
	dp_G5M_display_core_status();
	dp_G5M_read_design_info(jtag_gpio);
	dp_G5M_read_digests(jtag_gpio);
	dp_G5M_read_debug_info(jtag_gpio);
	dp_G5M_dump_debug_info();
	dp_G5M_read_fsn(jtag_gpio);
	dp_G5M_read_tvs_monitor(jtag_gpio);
	dp_G5M_query_security(jtag_gpio);
	dp_G5M_dump_security();
	if (device_family == G5M_FAMILY_ID_IN_DAT)
		dp_G5M_read_dibs(jtag_gpio);

	return;
}

void dp_G5M_read_udv(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_UDV;
	IRSCAN_in(jtag_gpio);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);
	dp_G5M_device_poll(jtag_gpio, 32u, 31u);

	dp_display_text("\r\nUDV: ");
	dp_display_array(g5_poll_buf, 4u, HEX);

	return;
}

void dp_G5M_dump_security(void)
{
	if ((g5_poll_buf[0] & 0x1u) == 0x1u) {
		dp_display_text("\r\nSmartDebug user debug access and active probes are disabled.");
	}
	if ((g5_poll_buf[0] & 0x2u) == 0x2u) {
		dp_display_text("\r\nSmartDebug sNVM debug is disabled.");
	}
	if ((g5_poll_buf[0] & 0x4u) == 0x4u) {
		dp_display_text("\r\nSmartDebug Live probes are disabled.");
	}
	if ((g5_poll_buf[0] & 0x8u) == 0x8u) {
		dp_display_text("\r\nUser JTAG interface is disabled");
	}
	if ((g5_poll_buf[0] & 0x10u) == 0x10u) {
		dp_display_text("\r\nJTAG boundary scan is disabled.");
	}
	if ((g5_poll_buf[0] & 0x20u) == 0x20u) {
		dp_display_text(
		    "\r\nReading of temperature and voltage via JTAG/SPI Slave is disabled.");
	}
	if ((g5_poll_buf[1] & 0x1u) == 0x1u) {
		dp_display_text("\r\nPlaintext passcode unlock is disabled.");
	}
	if ((g5_poll_buf[1] & 0x2u) == 0x2u) {
		dp_display_text("\r\nFabric erase/write is disabled.");
	}
	if ((g5_poll_buf[1] & 0x4u) == 0x4u) {
		dp_display_text("\r\nExternal digest check via JTAG/SPI Slave is disabled.");
	}
	if ((g5_poll_buf[1] & 0x8u) == 0x8u) {
		dp_display_text("\r\nBack level protection is enabled.");
	}
	if ((g5_poll_buf[1] & 0x10u) == 0x10u) {
		dp_display_text("\r\nMicrosemi factory test mode is disabled.");
	} else {
		dp_display_text("\r\nMicrosemi factory test mode access is allowed.");
	}
	if ((g5_poll_buf[1] & 0x40u) == 0x40u) {
		dp_display_text("\r\nExternal zeroizations via JTAG/SPI Slave is disabled.");
	}
	if ((g5_poll_buf[1] & 0x80u) == 0x80u) {
		dp_display_text("\r\nSPI Slave port is disabled.");
	}
	if ((g5_poll_buf[2] & 0x1u) == 0x1u) {
		dp_display_text("\r\nUser lock segment is locked. FlashLock/UPK1 is required to "
				"make changes to security.");
	}
	if ((g5_poll_buf[2] & 0x2u) == 0x2u) {
		dp_display_text(
		    "\r\nAuthenticate programming action for JTAG/SPI Slave is disabled.");
	}
	if ((g5_poll_buf[2] & 0x4u) == 0x4u) {
		dp_display_text("\r\nProgram action for JTAG/SPI Slave is disabled.");
	}
	if ((g5_poll_buf[2] & 0x8u) == 0x8u) {
		dp_display_text("\r\nVerify action for JTAG/SPI Slave is disabled.");
	}

	if ((g5_poll_buf[2] & 0x40u) == 0x40u) {
		dp_display_text("\r\nBitstream Default encryption key (KLK) is disabled.");
	}
	if ((g5_poll_buf[2] & 0x80u) == 0x80u) {
		dp_display_text("\r\nBitstream User Encryption Key 1 is disabled.");
	}
	if ((g5_poll_buf[3] & 0x1u) == 0x1u) {
		dp_display_text("\r\nBitstream User Encryption Key 2 is disabled.");
	}
	if ((g5_poll_buf[4] & 0x40u) == 0x40u) {
		dp_display_text("\r\nDefault encryption key (KLK) is disabled.");
	}
	if ((g5_poll_buf[4] & 0x80u) == 0x80u) {
		dp_display_text("\r\nUser Encryption Key 1 is disabled.");
	}
	if ((g5_poll_buf[5] & 0x1u) == 0x1u) {
		dp_display_text("\r\nUser Encryption Key 2 is disabled.");
	}
	if ((g5_poll_buf[6] & 0x10u) == 0x10u) {
		dp_display_text("\r\nsNVM write is disabled.");
	}
	if ((g5_poll_buf[6] & 0x20u) == 0x20u) {
		dp_display_text("\r\nPUF emulation via JTAG/SPI Slave is disabled.");
	}
	if ((g5_poll_buf[7] & 0x2u) == 0x2u) {
		dp_display_text(
		    "\r\nUser Key Set 1 is locked. FlashLock/UPK1 is required to make changes.");
	}
	if ((g5_poll_buf[7] & 0x4u) == 0x4u) {
		dp_display_text(
		    "\r\nUser Key Set 2 is locked. FlashLock/UPK2 is required to make changes.");
	}
	if ((g5_poll_buf[7] & 0x8u) == 0x8u) {
		dp_display_text("\r\nMicrosemi factory test access is permanently disabled.");
	}
	if ((g5_poll_buf[7] & 0x10u) == 0x10u) {
		dp_display_text("\r\nSmartDebug debugging is permanently disabled.");
	}
	if ((g5_poll_buf[7] & 0x20u) == 0x20u) {
		dp_display_text("\r\nFabric erase/write is permanently disabled.");
	}
	if ((g5_poll_buf[7] & 0x40u) == 0x40u) {
		dp_display_text("\r\nFlashLock/UPK1 unlocking is permanently disabled");
	}
	if ((g5_poll_buf[7] & 0x80u) == 0x80u) {
		dp_display_text("\r\nFlashLock/UPK2 unlocking is permanently disabled.");
	}
	if ((g5_poll_buf[8] & 0x1u) == 0x1u) {
		dp_display_text("\r\nFlashLock/DPK unlocking is permanently disabled.");
	}
	if ((g5_poll_buf[8] & 0x2u) == 0x2u) {
		dp_display_text("\r\nUPERM segment is permanently locked.");
	}
	return;
}

void dp_G5M_read_dibs(struct gpio_handle *jtag_gpio)
{
	unsigned char dibs_in[16] = {0xB4, 0x70, 0xD8, 0x05, 0x01, 0x4F, 0x1C, 0x77,
			       0xDE, 0x47, 0x9E, 0xCE, 0x6A, 0x31, 0x72, 0x5C};

	opcode = G5M_READ_DEVICE_INTEGRITY;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_FRAME_BIT_LENGTH, dibs_in);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);
	dp_G5M_device_poll(jtag_gpio, 128u, 127u);
	if ((error_code != DPE_SUCCESS) || ((g5_poll_buf[0] & 0x1u) == 0x1u)) {
		error_code = DPE_POLL_ERROR;
		unique_exit_code = 33003;
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFailed to read device integrity bits.");
		dp_display_text("\r\nERROR_CODE: ");
		dp_display_value(unique_exit_code, HEX);
#endif
	} else {
		dp_G5M_read_shared_buffer(jtag_gpio, 11);
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nDevice Integrity Bits: ");
		dp_display_array(g5_shared_buf, 32, HEX);
#endif
	}

	return;
}

void dp_G5M_read_design_info(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_READ_DESIGN_INFO;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_STATUS_REGISTER_BIT_LENGTH, (unsigned char *)(unsigned char *)DPNULL);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);
	opcode = G5M_READ_DESIGN_INFO;
	dp_G5M_device_poll(jtag_gpio, 8u, 7u);
	if (error_code == DPE_SUCCESS) {
		dp_G5M_read_shared_buffer(jtag_gpio, 3u);
		if (error_code == DPE_SUCCESS) {
			dp_display_text("\r\nDesign Name: ");

			for (global_uchar1 = 2u; global_uchar1 < 32u; global_uchar1++) {
				dp_display_value(g5_shared_buf[global_uchar1], CHR);
			}
			dp_display_text("\r\nChecksum: ");
			dp_display_array(g5_shared_buf, 2u, HEX);
			dp_display_text("\r\nDesign Info: \r\n");
			dp_display_array(g5_shared_buf, 34u, HEX);
			dp_display_text("\r\nDESIGNVER: ");
			dp_display_array(&g5_shared_buf[32], 2u, HEX);
			dp_display_text("\r\nBACKLEVEL: ");
			dp_display_array(&g5_shared_buf[34], 2u, HEX);
			dp_display_text(
			    "\r\n-----------------------------------------------------");
		}
	}

	return;
}

void dp_G5M_read_digests(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_READ_DIGEST;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_STATUS_REGISTER_BIT_LENGTH, (unsigned char *)(unsigned char *)DPNULL);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);
	opcode = G5M_READ_DIGEST;
	dp_G5M_device_poll(jtag_gpio, 8u, 7u);
	if (error_code == DPE_SUCCESS) {
		dp_G5M_read_shared_buffer(jtag_gpio, 26u);
		if (error_code == DPE_SUCCESS) {
			dp_display_text("\r\nFabric digest: ");
			dp_display_array(&g5_shared_buf[0], 32u, HEX);

			dp_display_text("\r\nUFS CC segment digest: ");
			dp_display_array(&g5_shared_buf[32], 32u, HEX);

			dp_display_text("\r\nSNVM digest: ");
			dp_display_array(&g5_shared_buf[64], 32u, HEX);

			dp_display_text("\r\nUFS UL digest: ");
			dp_display_array(&g5_shared_buf[96], 32u, HEX);

			dp_display_text("\r\nUser Key digest 0: ");
			dp_display_array(&g5_shared_buf[128], 32u, HEX);

			dp_display_text("\r\nUser Key digest 1: ");
			dp_display_array(&g5_shared_buf[160], 32u, HEX);

			dp_display_text("\r\nUser Key digest 2: ");
			dp_display_array(&g5_shared_buf[192], 32u, HEX);

			dp_display_text("\r\nUser Key digest 3: ");
			dp_display_array(&g5_shared_buf[224], 32u, HEX);

			dp_display_text("\r\nUser Key digest 4: ");
			dp_display_array(&g5_shared_buf[256], 32u, HEX);

			dp_display_text("\r\nUser Key digest 5: ");
			dp_display_array(&g5_shared_buf[288], 32u, HEX);

			dp_display_text("\r\nUser Key digest 6: ");
			dp_display_array(&g5_shared_buf[320], 32u, HEX);

			dp_display_text("\r\nUFS UPERM segment digest: ");
			dp_display_array(&g5_shared_buf[352], 32u, HEX);

			dp_display_text("\r\nFactory digest: ");
			dp_display_array(&g5_shared_buf[384], 32u, HEX);
		}
	}

	return;
}

void dp_G5M_check_cycle_count(struct gpio_handle *jtag_gpio)
{
	unsigned int cycle_count = 0;
	opcode = G5M_READ_DEBUG_INFO;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_STATUS_REGISTER_BIT_LENGTH, (unsigned char *)(unsigned char *)DPNULL);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);

	opcode = G5M_READ_DEBUG_INFO;
	dp_G5M_device_poll(jtag_gpio, 128u, 127u);
	if (error_code == DPE_SUCCESS) {
		dp_G5M_read_shared_buffer(jtag_gpio, 6u);

		cycle_count = ((g5_shared_buf[61] << 8u) | (g5_shared_buf[60]));
#ifdef ENABLE_DISPLAY

		dp_display_text("\r\nCYCLE COUNT: ");
		if (cycle_count != 0xffffu) {
			dp_display_value(cycle_count, DEC);
			if (cycle_count > G5M_MAX_ALLOWED_PROGRAMMING_CYCLES)
				dp_display_text("\r\n***** WARNING: MAXIMUM ALLOWED PROGRAMMING "
						"CYCLE COUNT IS REACHED *****");
		} else {
			dp_display_text(" Not available. ");
		}
#endif
	}
	return;
}

//#pragma optimize=none
void dp_G5M_read_debug_info(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_READ_DEBUG_INFO;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_STATUS_REGISTER_BIT_LENGTH, (unsigned char *)(unsigned char *)DPNULL);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);

	opcode = G5M_READ_DEBUG_INFO;
	dp_G5M_device_poll(jtag_gpio, 128u, 127u);
	if (error_code == DPE_SUCCESS) {
		dp_G5M_read_shared_buffer(jtag_gpio, 6u);
		dp_display_text("\r\nDEBUG_INFO:\r\n");
		if (device_family == G5SOC_FAMILY) {
			dp_display_array(g5_shared_buf, 94u, HEX);
		} else {
			dp_display_array(g5_shared_buf, 84u, HEX);
		}
	}

	return;
}

void dp_G5M_dump_debug_info(void)
{

	global_uint1 = ((g5_shared_buf[61] << 8u) | (g5_shared_buf[60]));
	dp_display_text("\r\nCycle Count: ");
	dp_display_value(global_uint1, DEC);

	if (g5_shared_buf[36] == 1u) {
		dp_display_text("\r\nProgramming mode: JTAG");
	} else if (g5_shared_buf[36] == 3u) {
		dp_display_text("\r\nProgramming mode: SPI-Slave");
	}

	if (((g5_shared_buf[32] & 0x3fu) != 0) && ((g5_shared_buf[32] & 0x3fu) != 0x3fu)) {
		dp_display_text("\r\nAlgorithm version: ");
		dp_display_value(g5_shared_buf[32] & 0x3fu, DEC);
	}

	return;
}

void dp_G5M_read_tvs_monitor(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_TVS_MONITOR;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_FRAME_BIT_LENGTH, (unsigned char *)(unsigned char *)DPNULL);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);
	opcode = G5M_TVS_MONITOR;
	dp_G5M_device_poll(jtag_gpio, 128u, 127u);
	if (error_code != DPE_SUCCESS) {
		error_code = DPE_MATCH_ERROR;
		unique_exit_code = 32846;
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFailed to read tvs monitor.");
		dp_display_text("\r\nERROR_CODE: ");
		dp_display_value(unique_exit_code, HEX);
#endif
	} else {
		dp_display_text("\r\nTVS_MONITOR: ");
		dp_display_array(g5_poll_buf, G5M_FRAME_BYTE_LENGTH, HEX);
	}

	return;
}

void dp_G5M_read_fsn(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_READ_FSN;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_STATUS_REGISTER_BIT_LENGTH, (unsigned char *)(unsigned char *)DPNULL);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	opcode = G5M_READ_FSN;
	dp_G5M_device_poll(jtag_gpio, 129u, 128u);
	if ((error_code != DPE_SUCCESS) && (unique_exit_code == DPE_SUCCESS)) {
		unique_exit_code = 32769;
		dp_display_text("\r\nFailed to read DSN.\r\nERROR_CODE: ");
		dp_display_value(unique_exit_code, HEX);
	} else {
#ifdef ENABLE_DISPLAY
		dp_display_text(
		    "\r\n=====================================================================");
		dp_display_text("\r\nDSN: ");
		dp_display_array(g5_poll_buf, 16u, HEX);
		dp_display_text(
		    "\r\n=====================================================================");
#endif
	}
	return;
}
#endif

/* Checking device ID function.  ID is already read in dpalg.c */
void dp_check_G5_device_ID(void)
{
	/* DataIndex is a variable used for loading the array data but not used now.
	 * Therefore, it can be used to store the Data file ID for */
	DataIndex = dp_get_bytes(Header_ID, G5M_ID_OFFSET, G5M_ID_BYTE_LENGTH);

	global_ulong1 = dp_get_bytes(Header_ID, G5M_ID_MASK_OFFSET, 4U);
	device_exception = (unsigned char)dp_get_bytes(Header_ID, G5M_DEVICE_EXCEPTION_OFFSET,
						 G5M_DEVICE_EXCEPTION_BYTE_LENGTH);

	device_ID &= global_ulong1;
	DataIndex &= global_ulong1;

	/* Identifying target device and setting its parms */

	if ((DataIndex & 0xfff) == MICROSEMI_ID) {
		if (device_ID == DataIndex) {
			if (((device_exception == MPF300T_ES_DEVICE_CODE) ||
			     (device_exception == MPF300TS_ES_DEVICE_CODE) ||
			     (device_exception == MPF300XT_DEVICE_CODE)) &&
			    (device_rev > 4u)) {
				unique_exit_code = 32857;
				error_code = DPE_IDCODE_ERROR;
#ifdef ENABLE_DISPLAY
				dp_display_text("\r\nFailed to verify IDCODE");
				dp_display_text("\r\nMPF300(XT|T_ES|TS_ES) programming file is not "
						"compatible with MPF300 production devices.");
				dp_display_text("\r\nYou must use a programming file for "
						"MPF300(T|TS|TL|TLS) device.");
				dp_display_text("\r\nERROR_CODE: ");
				dp_display_value(unique_exit_code, HEX);
#endif
			} else if (((device_exception == MPF300T_DEVICE_CODE) ||
				    (device_exception == MPF300TS_DEVICE_CODE) ||
				    (device_exception == MPF300TL_DEVICE_CODE) ||
				    (device_exception == MPF300TLS_DEVICE_CODE)) &&
				   (device_rev < 5u)) {
				unique_exit_code = 32858;
				error_code = DPE_IDCODE_ERROR;
#ifdef ENABLE_DISPLAY
				dp_display_text("\r\nFailed to verify IDCODE");
				dp_display_text("\r\nMPF300(T|TS|TL|TLS) programming file is not "
						"compatible with MPF300(XT|T_ES|TS_ES) devices.");
				dp_display_text("\r\nYou must use a programming file for "
						"MPF300(XT|T_ES|TS_ES) device.");
				dp_display_text("\r\nERROR_CODE: ");
				dp_display_value(unique_exit_code, HEX);
#endif
			} else {
#ifdef ENABLE_DISPLAY
				dp_display_text("\r\nActID = ");
				dp_display_value(device_ID, HEX);
				dp_display_text(" ExpID = ");
				dp_display_value(DataIndex, HEX);
				dp_display_text("\r\nDevice Rev = ");
				dp_display_value(device_rev, HEX);
#endif
				device_family =
				    (unsigned char)dp_get_bytes(Header_ID, G5M_DEVICE_FAMILY_OFFSET,
							  G5M_DEVICE_FAMILY_BYTE_LENGTH);
			}
		} else {
			error_code = DPE_IDCODE_ERROR;
			unique_exit_code = 32772;
#ifdef ENABLE_DISPLAY
			dp_display_text(" ExpID = ");
			dp_display_value(DataIndex, HEX);
			dp_display_text("\r\nERROR_CODE: ");
			dp_display_value(unique_exit_code, HEX);
#endif
		}
	} else {
		error_code = DPE_IDCODE_ERROR;
	}

	return;
}

/* Check if system controller is ready to enter programming mode */
void dp_G5M_device_poll(struct gpio_handle *jtag_gpio, unsigned char bits_to_shift, unsigned char Busy_bit)
{
	for (g5_poll_index = 0U; g5_poll_index <= G5M_MAX_CONTROLLER_POLL; g5_poll_index++) {
		IRSCAN_in(jtag_gpio);
		DRSCAN_out(jtag_gpio, bits_to_shift, (unsigned char *)DPNULL, g5_poll_buf);
		dp_delay(G5M_STANDARD_DELAY);
		if (((g5_poll_buf[Busy_bit / 8] & (1 << (Busy_bit % 8))) == 0x0u)) {
			break;
		}
	}
	if (g5_poll_index > G5M_MAX_CONTROLLER_POLL) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nDevice polling failed: ");
		dp_display_array(g5_poll_buf, 16, HEX);
#endif
		error_code = DPE_POLL_ERROR;
	}

	return;
}

void dp_G5M_device_shift_and_poll(struct gpio_handle *jtag_gpio, unsigned char bits_to_shift,
				  unsigned char Busy_bit, unsigned char Variable_ID, unsigned long start_bit_index)
{
	for (g5_poll_index = 0U; g5_poll_index <= G5M_MAX_CONTROLLER_POLL; g5_poll_index++) {
		IRSCAN_in(jtag_gpio);
		dp_get_and_DRSCAN_in_out(jtag_gpio, Variable_ID, bits_to_shift, start_bit_index,
					 g5_poll_buf);
		// DRSCAN_in(jtag_gpio, jtag_gpio, bits_to_shift, (unsigned char*)DPNULL, g5_poll_buf);
		dp_delay(G5M_STANDARD_DELAY);
		if (((g5_poll_buf[Busy_bit / 8] & (1 << (Busy_bit % 8))) == 0x0u)) {
			break;
		}
	}
	if (g5_poll_index > G5M_MAX_CONTROLLER_POLL) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nDevice polling failed.");
#endif
		error_code = DPE_POLL_ERROR;
	}

	return;
}

void dp_G5M_read_shared_buffer(struct gpio_handle *jtag_gpio, unsigned char ucNumOfBlocks)
{

	dp_flush_global_buf1();
	for (global_uchar1 = 0u; global_uchar1 < ucNumOfBlocks; global_uchar1++) {
		global_buf1[0] = (global_uchar1 << 1u);
		opcode = G5M_READ_BUFFER;
		IRSCAN_in(jtag_gpio);
		DRSCAN_in(jtag_gpio, 0u, G5M_FRAME_STATUS_BIT_LENGTH, global_buf1);
		goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
		dp_delay(G5M_STANDARD_DELAY);
		opcode = G5M_READ_BUFFER;
		dp_G5M_device_poll(jtag_gpio, 129u, 128u);
		for (global_uchar2 = 0; global_uchar2 < 16u; global_uchar2++) {
			g5_shared_buf[global_uchar1 * 16u + global_uchar2] =
			    g5_poll_buf[global_uchar2];
		}
	}

	return;
}

void dp_G5M_poll_device_ready(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_ISC_NOOP;
	for (g5_poll_index = 0U; g5_poll_index <= G5M_MAX_CONTROLLER_POLL; g5_poll_index++) {
		IRSCAN_in(jtag_gpio);
		goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
		dp_delay(G5M_STANDARD_DELAY);
		DRSCAN_out(jtag_gpio, 8u, (unsigned char *)DPNULL, g5_poll_buf);

		if ((g5_poll_buf[0] & 0x80u) == 0x0u) {
			break;
		}
	}
	if (g5_poll_index > G5M_MAX_CONTROLLER_POLL) {
		error_code = DPE_POLL_ERROR;
		unique_exit_code = 32818;
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nDevice is busy.");
		dp_display_text("\r\nERROR_CODE: ");
		dp_display_value(unique_exit_code, HEX);
#endif
	}

	return;
}

void dp_G5M_set_pgm_mode(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_MODE;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_STATUS_REGISTER_BIT_LENGTH, (unsigned char *)(unsigned char *)DPNULL);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);
	opcode = G5M_MODE;
	dp_G5M_device_poll(jtag_gpio, 8u, 7u);

	return;
}

/****************************************************************************
 * Purpose: Loads the BSR regsiter with data specified in the data file.
 *   If BSR_SAMPLE is enabled, the data is not loaded.  Instead, the last known
 *   State of the IOs is maintained by stepping through DRCapture JTAG state.
 ****************************************************************************/

void dp_G5M_load_bsr(struct gpio_handle *jtag_gpio)
{
	unsigned char capture_last_known_io_state = 0;
	unsigned int index;
	unsigned char mask;
	unsigned char c_mask;
	unsigned int bsr_bits;

	dp_G5M_check_core_status(jtag_gpio);

	bsr_bits = (unsigned int)dp_get_bytes(G5M_Header_ID, G5M_NUMOFBSRBITS_OFFSET,
					G5M_NUMOFBSRBITS_BYTE_LENGTH);
	opcode = ISC_SAMPLE;
	IRSCAN_in(jtag_gpio);

	dp_get_bytes(G5M_BsrPattern_ID, 0u, 1u);
	if (return_bytes) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nLoading BSR...");
#endif
		dp_get_and_DRSCAN_in(jtag_gpio, G5M_BsrPattern_ID, bsr_bits, 0u);
		goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, 0u);
	}

	/* Capturing the last known state of the IOs is only valid if the core
	was programmed.  Otherwise, load the BSR with what is in the data file. */
	if (core_is_enabled == 1) {
		for (index = 0; index < (unsigned int)(bsr_bits + 7u) / 8u; index++) {
			if (dp_get_bytes(G5M_BsrPatternMask_ID, index, 1u) != 0) {
				capture_last_known_io_state = 1;
				break;
			}
		}
		if (capture_last_known_io_state) {
			if (bsr_bits > MAX_BSR_BIT_SIZE) {
#ifdef ENABLE_DISPLAY
				dp_display_text(
				    "\r\nError: number of bsr bits > max buffer size.\r\nSkipping "
				    "maintain last known state of the IOs...");
#endif
			} else {
				DRSCAN_out(jtag_gpio, bsr_bits, (unsigned char *)DPNULL,
					   bsr_sample_buffer);

				for (index = 0; index < (unsigned int)(bsr_bits + 7u) / 8u; index++) {
					bsr_buffer[index] =
					    dp_get_bytes(G5M_BsrPattern_ID, index, 1u);
					mask = dp_get_bytes(G5M_BsrPatternMask_ID, index, 1u);

					if (mask != 0u) {
						c_mask = ~mask;
						bsr_buffer[index] =
						    (bsr_buffer[index] & c_mask) |
						    (bsr_sample_buffer[index] & mask);
					}
				}

				opcode = ISC_SAMPLE;
				IRSCAN_in(jtag_gpio);
				DRSCAN_in(jtag_gpio, 0, bsr_bits, bsr_buffer);
				goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, 0u);
			}
		}
	}

	return;
}

void dp_G5M_perform_isc_enable(struct gpio_handle *jtag_gpio)
{

	g5_pgmmode_flag = TRUE;
	dp_flush_global_buf1();
	global_buf1[0] |= (G5M_ALGO_VERSION & 0x3fu);
	global_buf1[2] |= (G5M_DIRECTC_VERSION & 0x3fu) << 1u;
	global_buf1[2] |= (DIRECTC_PROGRAMMING & 0x7u) << 7u;
	global_buf1[3] |= (DIRECTC_PROGRAMMING & 0x7u) >> 1u;
	global_buf1[3] |= (JTAG_PROGRAMMING_PROTOCOL & 0x7u) << 2u;

	opcode = G5M_ISC_ENABLE;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, ISC_STATUS_REGISTER_BIT_LENGTH, global_buf1);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);

	opcode = G5M_ISC_ENABLE;
	dp_G5M_device_poll(jtag_gpio, 32u, 31u);

	if ((error_code != DPE_SUCCESS) || ((g5_poll_buf[0] & 0x1u) == 1u)) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFailed to enter programming mode.");
#endif
		error_code = DPE_INIT_FAILURE;
	}

#ifdef ENABLE_DISPLAY
	dp_display_text("\r\nISC_ENABLE_RESULT: ");
	dp_display_array(g5_poll_buf, 4u, HEX);

	/* Display CRCERR */
	global_uchar1 = g5_poll_buf[0] & 0x1u;
	dp_display_text("\r\nCRCERR: ");
	dp_display_value(global_uchar1, HEX);
#endif

	return;
}
/* Enter programming mode */
void dp_G5M_initialize(struct gpio_handle *jtag_gpio)
{
	if (error_code == DPE_SUCCESS) {
		dp_G5M_query_security(jtag_gpio);
		if ((error_code == DPE_SUCCESS) &&
		    ((g5_shared_buf[7] & G5M_UL_USER_KEY1) == G5M_UL_USER_KEY1)) {
			dp_G5M_unlock_upk1(jtag_gpio);
		}
		if ((error_code == DPE_SUCCESS) &&
		    ((g5_shared_buf[7] & G5M_UL_USER_KEY2) == G5M_UL_USER_KEY2)) {
			dp_G5M_unlock_upk2(jtag_gpio);
		}
		if (error_code == DPE_SUCCESS) {
			dp_G5M_load_bsr(jtag_gpio);
			if (error_code == DPE_SUCCESS) {
				dp_G5M_perform_isc_enable(jtag_gpio);
			}
		}
	}

	return;
}

void dp_G5M_poll_device_ready_during_exit(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_ISC_NOOP;
	for (g5_poll_index = 0U; g5_poll_index <= G5M_MAX_EXIT_POLL; g5_poll_index++) {
		IRSCAN_in(jtag_gpio);
		goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
		dp_delay(G5M_EXIT_POLL_DELAY);
		DRSCAN_out(jtag_gpio, 8u, (unsigned char *)DPNULL, g5_poll_buf);

		if ((g5_poll_buf[0] & 0x80u) == 0x0u) {
			break;
		}
	}
	if (g5_poll_index > G5M_MAX_CONTROLLER_POLL) {
		error_code = DPE_POLL_ERROR;
		unique_exit_code = 32818;
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nDevice is busy.");
		dp_display_text("\r\nERROR_CODE: ");
		dp_display_value(unique_exit_code, HEX);
#endif
	} else {
		// SAR 110023 wait for worst case IO calibration time.
		goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
		dp_delay(G5M_IO_CALIBRATION_DELAY);
	}

	return;
}

/* Function is used to exit programming mode */
void dp_G5M_exit(struct gpio_handle *jtag_gpio)
{
	if (g5_pgmmode_flag == TRUE) {
		opcode = G5M_ISC_DISABLE;
		IRSCAN_in(jtag_gpio);
		goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
		dp_delay(G5M_STANDARD_DELAY);

		opcode = G5M_ISC_DISABLE;
		dp_G5M_device_poll(jtag_gpio, 32u, 31u);
#ifdef ENABLE_DISPLAY
		if ((error_code != DPE_SUCCESS) && (unique_exit_code == DPE_SUCCESS)) {
			dp_display_text("\r\nFailed to disable programming mode.");
		}
#endif
	}
	opcode = G5M_EXTEST2;
	IRSCAN_in(jtag_gpio);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_EXTEST2_DELAY);

	dp_G5M_poll_device_ready_during_exit(jtag_gpio);

	goto_jtag_state(jtag_gpio, JTAG_TEST_LOGIC_RESET, 5u);
	return;
}

void dp_G5M_set_mode(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_FRAME_INIT;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_STATUS_REGISTER_BIT_LENGTH, &g5_pgmmode);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);
	dp_G5M_device_poll(jtag_gpio, 8u, 7u);
#ifdef ENABLE_DISPLAY
	if (error_code != DPE_SUCCESS) {
		unique_exit_code = 32770;
		dp_display_text("r\nFailed to set programming mode.");
		dp_display_text("\r\nERROR_CODE: ");
		dp_display_value(unique_exit_code, HEX);
	}
#endif

	return;
}
#include <stdio.h>

void dp_G5M_process_data(struct gpio_handle *jtag_gpio, unsigned char BlockID)
{
	unsigned char tmp_buf;
#ifdef ENABLE_DISPLAY
	unsigned char owpKeymode;
#endif
	DataIndex = 0u;
/* Global unit1 is used to hold the number of components */
/* Loop through the number of components */
#ifdef ENABLE_DISPLAY
	dp_display_text("\r\n");
#endif

	for (; global_uint2 <= global_uint1; global_uint2++) {
		/* get the number of blocks */
		/* Global ulong1 is used to hold the number of blocks within the components */
		global_ulong1 = dp_get_bytes(G5M_NUMBER_OF_BLOCKS_ID,
					     (unsigned long)(((global_uint2 - 1u) * 22u) / 8u), 4u);
		global_ulong1 >>= ((global_uint2 - 1U) * 22u) % 8u;
		global_ulong1 &= 0x3FFFFFu;

		g5_component_type = (unsigned char)dp_get_bytes(
		    G5M_datastream_ID, G5M_COMPONENT_TYPE_IN_HEADER_BYTE + DataIndex / 8, 1);
		g5_componenet_Supports_Cert =
		    (unsigned char)dp_get_bytes(G5M_datastream_ID, G5M_GEN_CERT_BYTE + DataIndex / 8, 1) &
		    0x2u;

#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nProcessing component ");
		dp_display_value(global_uint2, DEC);
		dp_display_text(". Please wait...\n");
#endif

		opcode = G5M_FRAME_DATA;
		IRSCAN_in(jtag_gpio);
		dp_get_and_DRSCAN_in(jtag_gpio, BlockID, G5M_FRAME_BIT_LENGTH, DataIndex);

#ifdef ENABLE_DISPLAY
		old_progress = 0;
		new_progress = 0;
#endif
		for (global_ulong2 = 1u; global_ulong2 <= global_ulong1; global_ulong2++) {
#ifdef ENABLE_DISPLAY
			new_progress = (unsigned long)(global_ulong2 * 100 / global_ulong1);
			if (new_progress != old_progress) {
				dp_report_progress(new_progress);
				old_progress = new_progress;
			}
			if ((global_ulong2 == 1) && (g5_component_type == G5M_COMP_OWP)) {
				owpKeymode = (unsigned char)dp_get_bytes(
				    G5M_datastream_ID, G5M_OWP_KEY_MODE + DataIndex / 8, 1);
				dp_display_text("\r\nOWP is being used and its keymode is ");
				dp_display_value(owpKeymode, DEC);
			}

#endif

			goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
			dp_delay(G5M_STANDARD_DELAY);

			opcode = G5M_FRAME_DATA;
			if (global_ulong2 == global_ulong1) {
				dp_G5M_device_poll(jtag_gpio, 128u, 127u);
			} else {
				dp_G5M_device_shift_and_poll(jtag_gpio, 128u, 127u, BlockID,
							     DataIndex + G5M_FRAME_BIT_LENGTH);
			}

			if (error_code != DPE_SUCCESS) {
				error_code = DPE_PROCESS_DATA_ERROR;
				if (Action_code == DP_PROGRAM_ACTION_CODE)
					unique_exit_code = 32824;
				else if (Action_code == DP_VERIFY_ACTION_CODE)
					unique_exit_code = 32822;
				else if (Action_code == DP_ERASE_ACTION_CODE)
					unique_exit_code = 32820;
				else if (Action_code == DP_ENC_DATA_AUTHENTICATION_ACTION_CODE)
					unique_exit_code = 32818;

#ifdef ENABLE_DISPLAY
				dp_display_text("\r\nInstruction timed out.");
				dp_display_text("\r\ncomponentNo: ");
				dp_display_value(global_uint2, DEC);
				dp_display_text("\r\nblockNo: ");
				dp_display_value(global_ulong2, DEC);
				dp_display_text("\r\nERROR_CODE: ");
				dp_display_value(unique_exit_code, HEX);
#endif
				g5_current_failed_component = global_uint2;
				g5_current_failed_block = global_ulong2;
				g5_current_unique_error_code = unique_exit_code;

				global_uint2 = global_uint1;
				break;
			} else if ((g5_poll_buf[0] & 0x8u) != 0u) {
#ifdef ENABLE_DISPLAY
				dp_display_text("\r\nComponentNo: ");
				dp_display_value(global_uint2, DEC);
				dp_display_text("\r\nblockNo: ");
				dp_display_value(global_ulong2, DEC);
				dp_display_text("\r\nFRAME_DATA_RESULT: ");
				dp_display_array(g5_poll_buf, 2u, HEX);
#endif

				dp_G5M_get_data_status(jtag_gpio);
				if (error_code != DPE_SUCCESS) {
#ifdef ENABLE_DISPLAY
					dp_display_text("\r\nInstruction timed out.");
#endif
					g5_current_failed_component = global_uint2;
					g5_current_failed_block = global_ulong2;
					g5_current_unique_error_code = unique_exit_code;

				} else if ((g5_poll_buf[0] & 0x4u) != 0u) {
#ifdef ENABLE_DISPLAY
					dp_display_text("\r\nDATA_STATUS_RESULT: ");
					dp_display_array(g5_poll_buf, 8u, HEX);
#endif

					if ((g5_poll_buf[1] == 1u) || (g5_poll_buf[1] == 2u) ||
					    (g5_poll_buf[1] == 4u) || (g5_poll_buf[1] == 8u) ||
					    (g5_poll_buf[1] == 127u) || (g5_poll_buf[1] == 132u) ||
					    (g5_poll_buf[1] == 133u) || (g5_poll_buf[1] == 134u) ||
					    (g5_poll_buf[1] == 135u)) {
						unique_exit_code = 32799;
#ifdef ENABLE_DISPLAY
						dp_display_text(
						    "\r\nBitstream or data is corrupted or noisy.");
#endif
					} else if (g5_poll_buf[1] == 3u) {
						unique_exit_code = 32801;
#ifdef ENABLE_DISPLAY
						dp_display_text(
						    "\r\nInvalid/Corrupted encryption key.");
#endif
					} else if (g5_poll_buf[1] == 5u) {
						unique_exit_code = 32803;
#ifdef ENABLE_DISPLAY
						dp_display_text("\r\nBack level not satisfied.");
#endif
					} else if (g5_poll_buf[1] == 6u) {
						unique_exit_code = 32847;
#ifdef ENABLE_DISPLAY
						dp_display_text("\r\nBitstream programming action "
								"is disabled.");
#endif
					} else if (g5_poll_buf[1] == 7u) {
						unique_exit_code = 32805;
#ifdef ENABLE_DISPLAY
						dp_display_text("\r\nDSN binding mismatch.");
#endif
					} else if (g5_poll_buf[1] == 9u) {
						unique_exit_code = 32807;
#ifdef ENABLE_DISPLAY
						dp_display_text(
						    "\r\nInsufficient device capabilities.");
#endif
					} else if (g5_poll_buf[1] == 10u) {
						unique_exit_code = 32809;
#ifdef ENABLE_DISPLAY
						dp_display_text("\r\nIncorrect DEVICEID.");
#endif
					} else if (g5_poll_buf[1] == 11u) {
						unique_exit_code = 32811;
#ifdef ENABLE_DISPLAY
						dp_display_text("\r\nProgramming file is out of "
								"date, please regenerate.");
#endif
					} else if (g5_poll_buf[1] == 12u) {
						unique_exit_code = 32813;
#ifdef ENABLE_DISPLAY
						dp_display_text("\r\nProgramming file does not "
								"support verification.");
#endif
					} else if (g5_poll_buf[1] == 13u) {
						unique_exit_code = 32816;
#ifdef ENABLE_DISPLAY
						dp_display_text("\r\nInvalid or inaccessible "
								"Device Certificate.");
#endif
					} else if (g5_poll_buf[1] == 129u) {
						unique_exit_code = 32797;
#ifdef ENABLE_DISPLAY
						dp_display_text(
						    "\r\nDevice security prevented operation.");
#endif
					} else if (g5_poll_buf[1] == 128u) {
						if (((g5_poll_buf[4] >> 2u) & 0x1fu) < 16u) {
							unique_exit_code = 32773;
#ifdef ENABLE_DISPLAY
							dp_display_text(
							    "\r\nFailed to verify FPGA Array.");
#endif
						} else {
							unique_exit_code = 32774;
#ifdef ENABLE_DISPLAY
							dp_display_text("\r\nFailed to verify "
									"Fabric Configuration.");
#endif
						}
					} else if (g5_poll_buf[1] == 131u) {
						tmp_buf =
						    (g5_poll_buf[4] >> 2u) | (g5_poll_buf[5] << 6u);
						if (((g5_poll_buf[4] & 0x3u) == 1u) &&
						    (tmp_buf >= 2u) && (tmp_buf <= 222u) &&
						    (((g5_poll_buf[6] >> 1u) & 0x3u) == 1u)) {
							unique_exit_code = 32776;
#ifdef ENABLE_DISPLAY
							dp_display_text(
							    "\r\nFailed to verify sNVM.");
#endif
						} else if (((g5_poll_buf[4] & 0x3u) == 1u) &&
							   (tmp_buf >= 2u) && (tmp_buf <= 222u) &&
							   (((g5_poll_buf[6] >> 1u) & 0x3u) ==
							    2u)) {
							unique_exit_code = 32857;
#ifdef ENABLE_DISPLAY
							dp_display_text(
							    "\r\nFailed to verify pNVM.");
#endif
						} else if ((g5_poll_buf[4] & 0x3u) == 3u) {
							unique_exit_code = 32775;
#ifdef ENABLE_DISPLAY
							dp_display_text(
							    "\r\nFailed to verify Security.");
#endif
						}
					}
#ifdef ENABLE_DISPLAY
					dp_display_text("\r\nERROR_CODE: ");
					dp_display_value(unique_exit_code, HEX);
#endif
					g5_current_failed_component = global_uint2;
					g5_current_failed_block = global_ulong2;
					g5_current_unique_error_code = unique_exit_code;
				}
#ifdef ENABLE_DISPLAY
				dp_G5M_read_debug_info(jtag_gpio);
#endif
				error_code = DPE_PROCESS_DATA_ERROR;
				global_uint2 = global_uint1;
				break;
			}
			DataIndex += G5M_FRAME_BIT_LENGTH;
		}
#ifdef ENABLE_DISPLAY
		if ((Action_code == DP_PROGRAM_ACTION_CODE) && g5_componenet_Supports_Cert &&
		    (error_code == DPE_SUCCESS)) {
			dp_G5M_report_certificate(jtag_gpio);
			if (g5_component_type == G5M_COMP_BITS)
				dp_display_text("\r\nBITS component bitstream digest: ");
			else if (g5_component_type == G5M_COMP_FPGA)
				dp_display_text("\r\nFabric component bitstream digest: ");
			else if (g5_component_type == G5M_COMP_KEYS)
				dp_display_text("\r\nSecurity component bitstream digest: ");
			else if (g5_component_type == G5M_COMP_SNVM)
				dp_display_text("\r\nsNVM component bitstream digest: ");
			else if (g5_component_type == G5M_COMP_ENVM)
				dp_display_text("\r\neNVM component bitstream digest: ");
			else if (g5_component_type == G5M_COMP_OWP)
				dp_display_text("\r\nOWP component bitstream digest: ");
			else if (g5_component_type == G5M_COMP_EOB)
				dp_display_text("\r\nEOB component bitstream digest: ");
			dp_display_array(g5_component_digest, G5M_COMPONENT_DIGEST_BYTE_SIZE, HEX);
		}
#endif
	}

	return;
}

void dp_G5M_get_data_status(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_FRAME_STATUS;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_DATA_STATUS_REGISTER_BIT_LENGTH, (unsigned char *)(unsigned char *)DPNULL);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);

	opcode = G5M_FRAME_STATUS;
	dp_G5M_device_poll(jtag_gpio, G5M_DATA_STATUS_REGISTER_BIT_LENGTH,
			   G5M_DATA_STATUS_REGISTER_BIT_LENGTH - 1);

	return;
}

void dp_G5M_report_certificate(struct gpio_handle *jtag_gpio)
{
	unsigned int index;
	dp_G5M_read_shared_buffer(jtag_gpio,
				  G5M_NUMBER_OF_COFC_BLOCKS); // CofC is 928 bits which is 116 bytes
							      // which is 7.25 blocks of data

	for (index = 0; index < G5M_COMPONENT_DIGEST_BYTE_SIZE; index++) {
		// 20 is the byte location of the digest
		g5_component_digest[index] = g5_shared_buf[20 + index];
	}
	return;
}

void dp_G5M_read_certificate(struct gpio_handle *jtag_gpio)
{
	unsigned char device_certificate_validated = 0u;
	opcode = G5M_READ_DEVICE_CERT;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_STATUS_REGISTER_BIT_LENGTH, (unsigned char *)(unsigned char *)DPNULL);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);

	opcode = G5M_READ_DEVICE_CERT;
	dp_G5M_device_poll(jtag_gpio, 8u, 7u);
	if (error_code != DPE_SUCCESS) {
		unique_exit_code = 33000;
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFailed to read device certificate, device is busy");
		dp_display_text("\r\nERROR_CODE: ");
		dp_display_value(unique_exit_code, HEX);
#endif
	} else if ((g5_poll_buf[0] & 0x2u) == 0x0u) {
		device_certificate_validated = g5_poll_buf[0] & 0x1u;
		dp_G5M_read_shared_buffer(jtag_gpio, 64u);
		if (device_certificate_validated) {
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nDevice certificate signature has been "
					"verified.\r\nDEVICE_CERTIFICATE(LSB->MSB): ");
			dp_display_array_reverse(g5_shared_buf, 1024, HEX);
#endif
		}
	}
	return;
}

void dp_G5M_read_security(struct gpio_handle *jtag_gpio)
{
	dp_G5M_query_security(jtag_gpio);
	if (error_code == DPE_SUCCESS) {
		dp_G5M_unlock_dpk(jtag_gpio);
		if (error_code == DPE_SUCCESS) {
			dp_G5M_query_security(jtag_gpio);
			if (error_code == DPE_SUCCESS) {
#ifdef ENABLE_DISPLAY
				dp_display_text("\r\nWarning: Security cannot be read even after "
						"unlocking debug pass key.");
#endif
			}
		}
	}

	return;
}
void dp_G5M_query_security(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_QUERY_SECURITY;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_SECURITY_STATUS_REGISTER_BIT_LENGTH,
		  (unsigned char *)(unsigned char *)DPNULL);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	opcode = G5M_QUERY_SECURITY;
	dp_G5M_device_poll(jtag_gpio, 16u, 15u);
	if (error_code != DPE_SUCCESS) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFailed to query security information.");
#endif
	} else {
		if (device_family == G5SOC_FAMILY) {
			dp_G5M_read_shared_buffer(jtag_gpio, 3u);
#ifdef ENABLE_DISPLAY
			dp_display_text(
			    "\r\n--- Security locks and configuration settings ---\r\n");
			dp_display_array(g5_shared_buf, 33u, HEX);
#endif
		} else {
			dp_G5M_read_shared_buffer(jtag_gpio, 1u);
#ifdef ENABLE_DISPLAY
			dp_display_text(
			    "\r\n--- Security locks and configuration settings ---\r\n");
			dp_display_array(g5_shared_buf, 9u, HEX);
#endif
		}
	}
	return;
}

void dp_G5M_unlock_dpk(struct gpio_handle *jtag_gpio)
{
	dp_get_data(G5M_DPK_ID, 0u);
	if (return_bytes == 0u) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nWarning: DPK data is missing.");
#endif
	} else {
		dp_G5M_load_dpk(jtag_gpio);
		if (error_code == DPE_SUCCESS) {
			opcode = G5M_UNLOCK_DEBUG_PASSCODE;
			IRSCAN_in(jtag_gpio);
			DRSCAN_in(jtag_gpio, 0u, G5M_STATUS_REGISTER_BIT_LENGTH,
				  (unsigned char *)(unsigned char *)DPNULL);
			goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
			dp_delay(G5M_STANDARD_DELAY);
		}
		dp_G5M_device_poll(jtag_gpio, 8u, 7u);
		if ((error_code != DPE_SUCCESS) || ((g5_poll_buf[0] & 0x3u) != 0x1u)) {
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nFailed to unlock debug pass key.");
#endif
			error_code = DPE_MATCH_ERROR;
		} else {
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nDebug security (DPK) is unlocked.");
#endif
		}
	}
	return;
}

void dp_G5M_unlock_upk1(struct gpio_handle *jtag_gpio)
{
	dp_get_data(G5M_UPK1_ID, 0u);
	if (return_bytes == 0u) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nWarning: UPK1 data is missing.");
#endif
	} else {
		dp_G5M_load_upk1(jtag_gpio);
		if (error_code == DPE_SUCCESS) {
			opcode = G5M_UNLOCK_USER_PASSCODE;
			IRSCAN_in(jtag_gpio);
			DRSCAN_in(jtag_gpio, 0u, G5M_STATUS_REGISTER_BIT_LENGTH,
				  (unsigned char *)(unsigned char *)DPNULL);
			goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
			dp_delay(G5M_STANDARD_DELAY);
		}
		dp_G5M_device_poll(jtag_gpio, 8u, 7u);
		if ((error_code != DPE_SUCCESS) || ((g5_poll_buf[0] & 0x3u) != 0x1u)) {
			error_code = DPE_MATCH_ERROR;
			unique_exit_code = 32784;
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nFailed to unlock user pass key 1.");
			dp_display_text("\r\nERROR_CODE: ");
			dp_display_value(unique_exit_code, HEX);
#endif
		} else {
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nUser security (DPK1) is unlocked.");
#endif
		}
	}
	return;
}

void dp_G5M_unlock_upk2(struct gpio_handle *jtag_gpio)
{
	dp_get_data(G5M_UPK2_ID, 0u);
	if (return_bytes == 0u) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nWarning: UPK2 data is missing.");
#endif
	} else {
		dp_G5M_load_upk2(jtag_gpio);
		if (error_code == DPE_SUCCESS) {
			opcode = G5M_UNLOCK_VENDOR_PASSCODE;
			IRSCAN_in(jtag_gpio);
			DRSCAN_in(jtag_gpio, 0u, G5M_STATUS_REGISTER_BIT_LENGTH,
				  (unsigned char *)(unsigned char *)DPNULL);
			goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
			dp_delay(G5M_STANDARD_DELAY);
		}
		dp_G5M_device_poll(jtag_gpio, 8u, 7u);
		if ((error_code != DPE_SUCCESS) || ((g5_poll_buf[0] & 0x3u) != 0x1u)) {
			error_code = DPE_MATCH_ERROR;
			unique_exit_code = 32785;
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nFailed to unlock user pass key 2.");
			dp_display_text("\r\nERROR_CODE: ");
			dp_display_value(unique_exit_code, HEX);
#endif
		} else {
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nUser security (DPK2) is unlocked.");
#endif
		}
	}
	return;
}

void dp_G5M_load_dpk(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_KEYLO;
	IRSCAN_in(jtag_gpio);
	dp_get_and_DRSCAN_in(jtag_gpio, G5M_DPK_ID, G5M_FRAME_BIT_LENGTH, 0u);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);
	dp_G5M_device_poll(jtag_gpio, 128u, 127u);
	if (error_code != DPE_SUCCESS) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFailed to load keylo. \r\nkeylo_result: ");
		dp_display_array(g5_poll_buf, G5M_FRAME_BYTE_LENGTH, HEX);
#endif
		error_code = DPE_MATCH_ERROR;
	} else {
		opcode = G5M_KEYHI;
		IRSCAN_in(jtag_gpio);
		dp_get_and_DRSCAN_in(jtag_gpio, G5M_DPK_ID, G5M_FRAME_BIT_LENGTH,
				     G5M_FRAME_BIT_LENGTH);
		goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
		dp_delay(G5M_STANDARD_DELAY);
		dp_G5M_device_poll(jtag_gpio, 128u, 127u);
		if (error_code != DPE_SUCCESS) {
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nFailed to load keyhi. \r\nkeyhi_result: ");
			dp_display_array(g5_poll_buf, G5M_FRAME_BYTE_LENGTH, HEX);
#endif
			error_code = DPE_MATCH_ERROR;
		}
	}

	return;
}

void dp_G5M_load_upk1(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_KEYLO;
	IRSCAN_in(jtag_gpio);
	dp_get_and_DRSCAN_in(jtag_gpio, G5M_UPK1_ID, G5M_FRAME_BIT_LENGTH, 0u);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);
	dp_G5M_device_poll(jtag_gpio, 128u, 127u);
	if (error_code != DPE_SUCCESS) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFailed to load keylo. \r\nkeylo_result: ");
		dp_display_array(g5_poll_buf, G5M_FRAME_BYTE_LENGTH, HEX);
#endif
		error_code = DPE_MATCH_ERROR;
	} else {
		opcode = G5M_KEYHI;
		IRSCAN_in(jtag_gpio);
		dp_get_and_DRSCAN_in(jtag_gpio, G5M_UPK1_ID, G5M_FRAME_BIT_LENGTH,
				     G5M_FRAME_BIT_LENGTH);
		goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
		dp_delay(G5M_STANDARD_DELAY);
		dp_G5M_device_poll(jtag_gpio, 128u, 127u);
		if (error_code != DPE_SUCCESS) {
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nFailed to load keyhi. \r\nkeyhi_result: ");
			dp_display_array(g5_poll_buf, G5M_FRAME_BYTE_LENGTH, HEX);
#endif
			error_code = DPE_MATCH_ERROR;
		}
	}

	return;
}

void dp_G5M_load_upk2(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_KEYLO;
	IRSCAN_in(jtag_gpio);
	dp_get_and_DRSCAN_in(jtag_gpio, G5M_UPK2_ID, G5M_FRAME_BIT_LENGTH, 0u);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	dp_delay(G5M_STANDARD_DELAY);
	dp_G5M_device_poll(jtag_gpio, 128u, 127u);
	if (error_code != DPE_SUCCESS) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFailed to load keylo. \r\nkeylo_result: ");
		dp_display_array(g5_poll_buf, G5M_FRAME_BYTE_LENGTH, HEX);
#endif
		error_code = DPE_MATCH_ERROR;
	} else {
		opcode = G5M_KEYHI;
		IRSCAN_in(jtag_gpio);
		dp_get_and_DRSCAN_in(jtag_gpio, G5M_UPK2_ID, G5M_FRAME_BIT_LENGTH,
				     G5M_FRAME_BIT_LENGTH);
		goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
		dp_delay(G5M_STANDARD_DELAY);
		dp_G5M_device_poll(jtag_gpio, 128u, 127u);
		if (error_code != DPE_SUCCESS) {
#ifdef ENABLE_DISPLAY
			dp_display_text("\r\nFailed to load keyhi. \r\nkeyhi_result: ");
			dp_display_array(g5_poll_buf, G5M_FRAME_BYTE_LENGTH, HEX);
#endif
			error_code = DPE_MATCH_ERROR;
		}
	}

	return;
}
void dp_G5M_display_bitstream_digest(void)
{

	DataIndex = 0u;
	global_uint1 =
	    (unsigned int)dp_get_bytes(Header_ID, G5M_DATASIZE_OFFSET, G5M_DATASIZE_BYTE_LENGTH);
	for (global_uint2 = 1u; global_uint2 <= global_uint1; global_uint2++) {
		/* get the number of blocks */
		/* Global ulong1 is used to hold the number of blocks within the components */
		global_ulong1 = dp_get_bytes(G5M_NUMBER_OF_BLOCKS_ID,
					     (unsigned long)(((global_uint2 - 1u) * 22u) / 8u), 4u);
		global_ulong1 >>= ((global_uint2 - 1U) * 22u) % 8u;
		global_ulong1 &= 0x3FFFFFu;

#ifdef ENABLE_DISPLAY
		g5_component_type = (unsigned char)dp_get_bytes(
		    G5M_datastream_ID, G5M_COMPONENT_TYPE_IN_HEADER_BYTE + DataIndex / 8, 1);
		if (g5_component_type == G5M_COMP_BITS) {
			unsigned char *data_address = (unsigned char *)DPNULL;
			data_address = dp_get_data(G5M_datastream_ID,
						   G5M_BSDIGEST_BYTE_OFFSET * 8 + DataIndex);
			dp_display_text("\r\nBITSTREAM_DIGEST = ");
			dp_display_array(data_address, G5M_BSDIGEST_BYTE_SIZE, HEX);
		}
#endif
		DataIndex += G5M_FRAME_BIT_LENGTH * global_ulong1;
	}

	return;
}

void dp_G5M_do_zeroize(struct gpio_handle *jtag_gpio, unsigned char zmode)
{
	unsigned char zeroize_result[16] = {0x00, 0xB6, 0x16, 0x3B, 0x25, 0xC3, 0x0A, 0xE5,
				      0x7B, 0x5D, 0x19, 0x00, 0x45, 0x06, 0x31, 0xA8};
	zeroize_result[0] = zmode;

	opcode = G5M_ZEROIZE;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_FRAME_BIT_LENGTH, zeroize_result);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	opcode = G5M_ZEROIZE;
	dp_G5M_device_poll(jtag_gpio, 128u, 127u);
	if ((error_code != DPE_SUCCESS) && (unique_exit_code == DPE_SUCCESS)) {
		unique_exit_code = 32848;
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFailed to load zeroize instruction.\r\nERROR_CODE: ");
		dp_display_value(unique_exit_code, HEX);
#endif
	} else if (g5_poll_buf[0] & 0x1u == 1u) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nZEROIZE_RESULT: ");
		dp_display_array(g5_poll_buf, 16, HEX);
#endif
		error_code = DPE_POLL_ERROR;
		unique_exit_code = 32849;
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFailed to zeroize the device.\r\nERROR_CODE: ");
		dp_display_value(unique_exit_code, HEX);
#endif
	} else {
		dp_G5M_do_read_zeroization_result(jtag_gpio);
	}

	return;
}

void dp_G5M_do_read_zeroization_result(struct gpio_handle *jtag_gpio)
{
	opcode = G5M_READ_ZEROIZATION_RESULT;
	IRSCAN_in(jtag_gpio);
	DRSCAN_in(jtag_gpio, 0u, G5M_FRAME_BIT_LENGTH, (unsigned char *)(unsigned char *)DPNULL);
	goto_jtag_state(jtag_gpio, JTAG_RUN_TEST_IDLE, G5M_STANDARD_CYCLES);
	opcode = G5M_READ_ZEROIZATION_RESULT;
	dp_G5M_device_poll(jtag_gpio, 128u, 127u);
	if ((error_code != DPE_SUCCESS) && (unique_exit_code == DPE_SUCCESS)) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nread_zeroize_result: ");
		dp_display_array(g5_poll_buf, 16, HEX);
#endif
		unique_exit_code = 32853;
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFailed to load read zeroization instruction.\r\nERROR_CODE: ");
		dp_display_value(unique_exit_code, HEX);
#endif
	} else if (g5_poll_buf[0] & 0x3u > 0u) {
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nread_zeroize_result: ");
		dp_display_array(g5_poll_buf, 16, HEX);
#endif
		error_code = DPE_POLL_ERROR;
		unique_exit_code = 32854;
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFailed to read zeroization certificate.\r\nERROR_CODE: ");
		dp_display_value(unique_exit_code, HEX);
#endif
	} else {
		dp_G5M_read_shared_buffer(jtag_gpio, 9);
#ifdef ENABLE_DISPLAY
		dp_display_text("\r\nFETCH_ZEROIZATION_RESULT: ");
		dp_display_array(g5_shared_buf, 131, HEX);
#endif
	}

	return;
}

#endif /* ENABLE_G5_SUPPORT */

/* *************** End of File *************** */
