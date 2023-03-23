// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2023 Microchip Technology Inc. All rights reserved.
 */

/* ************************************************************************ */
/*                                                                          */
/*  Module:         dpG5alg.h                                               */
/*                                                                          */
/*  Description:    Contains function prototypes.                           */
/*                                                                          */
/* ************************************************************************ */

#ifndef INC_DPG5ALG_H
#define INC_DPG5ALG_H

/*
 * Data block ID definitions
 */
#define G5M_ALGO_VERSION	  1u
#define DIRECTC_PROGRAMMING	  2u
#define G5M_DIRECTC_VERSION	  0u
#define SPI_PROGRAMMING_PROTOCOL  2u
#define JTAG_PROGRAMMING_PROTOCOL 3u

#define G5M_Header_ID		    0u
#define G5M_USER_INFO_ID	    1u
#define G5M_ACT_UROW_DESIGN_NAME_ID 2u
#define G5M_BsrPattern_ID	    3u
#define G5M_BsrPatternMask_ID	    4u
#define G5M_NUMBER_OF_BLOCKS_ID	    5u
#define G5M_UPK1_ID		    6u
#define G5M_UPK2_ID		    7u
#define G5M_datastream_ID	    8u
#define G5M_erasedatastream_ID	    9u
#define G5M_DPK_ID		    13u

#define G5M_DEVICE_FAMILY_OFFSET	   36u
#define G5M_DEVICE_FAMILY_BYTE_LENGTH	   1u
#define G5M_ID_OFFSET			   37u
#define G5M_ID_BYTE_LENGTH		   4u
#define G5M_ID_MASK_OFFSET		   41u
#define G5M_ID_MASK_BYTE_LENGTH		   4u
#define G5M_SILSIG_OFFSET		   45u
#define G5M_SILSIG_BYTE_LENGTH		   4u
#define G5M_CHECKSUM_OFFSET		   49u
#define G5M_CHECKSUM_BYTE_LENGTH	   2u
#define G5M_NUMOFBSRBITS_OFFSET		   51u
#define G5M_NUMOFBSRBITS_BYTE_LENGTH	   2u
#define G5M_NUMOFCOMPONENT_OFFSET	   53u
#define G5M_NUMOFCOMPONENT_BYTE_LENGTH	   2u
#define G5M_DATASIZE_OFFSET		   55u
#define G5M_DATASIZE_BYTE_LENGTH	   2u
#define G5M_ERASEDATASIZE_OFFSET	   57u
#define G5M_ERASEDATASIZE_BYTE_LENGTH	   2u
#define G5M_VERIFYDATASIZE_OFFSET	   59u
#define G5M_VERIFYDATASIZE_BYTE_LENGTH	   2u
#define G5M_ENVMDATASIZE_OFFSET		   61u
#define G5M_ENVMDATASIZE_BYTE_LENGTH	   2u
#define G5M_ENVMVERIFYDATASIZE_OFFSET	   63u
#define G5M_ENVMVERIFYDATASIZE_BYTE_LENGTH 2u
#define G5M_UEK1_EXISTS_OFFSET		   65u
#define G5M_UEK1_EXISTS_BYTE_LENGTH	   1u
#define G5M_UEK2_EXISTS_OFFSET		   66u
#define G5M_UEK2_EXISTS_BYTE_LENGTH	   1u
#define G5M_DPK_EXISTS_OFFSET		   67u
#define G5M_DPK_EXISTS_BYTE_LENGTH	   1u
#define G5M_SEC_ERASE_EXISTS_OFFSET	   67u
#define G5M_SEC_ERASE_EXISTS_BYTE_LENGTH   1u
#define G5M_UEK3_EXISTS_OFFSET		   68u
#define G5M_UEK3_EXISTS_BYTE_LENGTH	   1u
#define G5M_DEVICE_EXCEPTION_OFFSET	   69u
#define G5M_DEVICE_EXCEPTION_BYTE_LENGTH   1u

#define G5M_FAMILY_ID_IN_DAT 0x7u
#define G5M_FAMILY_MASK	     0x0FFF0000u
#define G5M_FAMILY	     0x0F810000u

#define MPF300T_ES_DEVICE_CODE	10
#define MPF300TS_ES_DEVICE_CODE 11
#define MPF300XT_DEVICE_CODE	12
#define MPF300T_DEVICE_CODE	13
#define MPF300TS_DEVICE_CODE	14
#define MPF300TL_DEVICE_CODE	15
#define MPF300TLS_DEVICE_CODE	16

#define G5M_MAX_PROGRAM_ATTEMPTS 3
#define G5M_MAX_VERIFY_ATTEMPTS	 3

#define G5M_MAX_ALLOWED_PROGRAMMING_CYCLES 500

/*
 * G5Main JTAG instructions
 */
#define G5M_ISC_ENABLE		    0x0bu
#define G5M_ISC_DISABLE		    0x0cu
#define G5M_ISC_NOOP		    0x0du
#define G5M_FRAME_INIT		    0xaeu
#define G5M_READ_DESIGN_INFO	    0xa6u
#define G5M_READ_DIGEST		    0xa3u
#define G5M_READ_DEBUG_INFO	    0xe7u
#define G5M_TVS_MONITOR		    0xe3u
#define G5M_READ_BUFFER		    0xf2u
#define G5M_READ_FSN		    0xf0u
#define G5M_QUERY_SECURITY	    0xb8u
#define G5M_MODE		    0xafu
#define G5M_FRAME_DATA		    0xeeu
#define G5M_FRAME_STATUS	    0xd8u
#define G5M_KEYLO		    0xebu
#define G5M_KEYHI		    0xecu
#define G5M_AUXLO		    0xe9u
#define G5M_AUXHI		    0xeau
#define G5M_UNLOCK_DEBUG_PASSCODE   0xa9u
#define G5M_UNLOCK_USER_PASSCODE    0xa8u
#define G5M_UNLOCK_VENDOR_PASSCODE  0xaau
#define G5M_MSSADDR		    0xdbu
#define G5M_MSSRD		    0xbdu
#define G5M_MSSWR		    0xceu
#define G5M_READ_DEVICE_CERT	    0xa2u
#define G5M_GENERATE_OTP	    0xf3u
#define G5M_MATCH_OTP		    0xedu
#define G5M_VALIDATE_KEYS	    0xacu
#define G5M_CHECK_DIGESTS	    0xbcu
#define G5M_EXTEST2		    0x09u
#define G5M_UDV			    0xcdu
#define G5M_READ_DEVICE_INTEGRITY   0xe1u
#define G5M_ZEROIZE		    0xe6u
#define G5M_READ_ZEROIZATION_RESULT 0xe2u

#define G5M_MAX_CONTROLLER_POLL 1000000u
#define G5M_MAX_EXIT_POLL	10000u

#define ISC_STATUS_REGISTER_BIT_LENGTH	    32u
#define G5M_DATA_STATUS_REGISTER_BIT_LENGTH 64u
#define G5M_STANDARD_CYCLES		    3u
#define G5M_STANDARD_DELAY		    10u
#define G5M_EXTEST2_DELAY		    1000u
#define G5M_EXIT_POLL_DELAY		    1000u
#define G5M_MSSADDR_BIT_LENGTH		    64u
#define G5M_MSSRD_BIT_LENGTH		    16u
#define G5M_MSSWR_BIT_LENGTH		    32u

#define G5M_IO_CALIBRATION_DELAY 1000000u

#define G5M_STATUS_REGISTER_BIT_LENGTH		8u
#define G5M_SECURITY_STATUS_REGISTER_BIT_LENGTH 16u
#define G5M_FRAME_BIT_LENGTH			128u
#define G5M_FRAME_STATUS_BIT_LENGTH		129u
#define G5M_FRAME_BYTE_LENGTH			16u
#define G5M_COMPONENT_DIGEST_BYTE_SIZE		32u
#define G5M_NUMBER_OF_COFC_BLOCKS		8u   // which is 928 / 128 rounded up
#define G5M_GEN_CERT_BYTE			340u // Bit 1 (0x2u)
#define G5M_COMPONENT_TYPE_IN_HEADER_BYTE	50u
#define G5M_OWP_KEY_MODE			51u

#define G5M_COMP_BITS 0u
#define G5M_COMP_FPGA 1u
#define G5M_COMP_KEYS 2u
#define G5M_COMP_SNVM 3u
#define G5M_COMP_ENVM 6u // G5SOC
#define G5M_COMP_OWP  7u // G5SOC
#define G5M_COMP_EOB  127u

#define G5M_UL_USER_KEY1	     0x2u
#define G5M_UL_USER_KEY2	     0x4u
#define G5M_UL_EXTERNAL_DIGEST_CHECK 0x4u

#define G5M_BSDIGEST_BYTE_OFFSET 308u
#define G5M_BSDIGEST_BYTE_SIZE	 32u

unsigned char dp_top_g5(struct gpio_handle *jtag_gpio);
void dp_init_G5_vars(void);
void dp_check_G5_action(void);
void dp_perform_G5_action(struct gpio_handle *jtag_gpio);

/* Supported Actions */
void dp_G5M_device_info_action(struct gpio_handle *jtag_gpio);
void dp_G5M_erase_action(struct gpio_handle *jtag_gpio);
void dp_G5M_program_action(struct gpio_handle *jtag_gpio);
void dp_G5M_verify_action(struct gpio_handle *jtag_gpio);
void dp_G5M_enc_data_authentication_action(struct gpio_handle *jtag_gpio);
void dp_G5M_verify_digest_action(struct gpio_handle *jtag_gpio);
void dp_G5M_read_device_certificate_action(struct gpio_handle *jtag_gpio);
void dp_G5M_zeroize_like_new_action(struct gpio_handle *jtag_gpio);
void dp_G5M_zeroize_unrecoverable_action(struct gpio_handle *jtag_gpio);

void dp_check_G5_device_ID(void);
void dp_G5M_do_program(struct gpio_handle *jtag_gpio);
void dp_G5M_do_verify(struct gpio_handle *jtag_gpio);
void dp_G5M_read_udv(struct gpio_handle *jtag_gpio);
void dp_G5M_read_design_info(struct gpio_handle *jtag_gpio);
void dp_G5M_read_digests(struct gpio_handle *jtag_gpio);

void dp_G5M_poll_device_ready(struct gpio_handle *jtag_gpio);
void dp_G5M_check_core_status(struct gpio_handle *jtag_gpio);
void dp_G5M_display_core_status(void);
void dp_G5M_read_debug_info(struct gpio_handle *jtag_gpio);
void dp_G5M_dump_debug_info(void);
void dp_G5M_read_tvs_monitor(struct gpio_handle *jtag_gpio);
void dp_G5M_read_fsn(struct gpio_handle *jtag_gpio);
void dp_G5M_read_security(struct gpio_handle *jtag_gpio);
void dp_G5M_query_security(struct gpio_handle *jtag_gpio);
void dp_G5M_dump_security(void);
void dp_G5M_read_dibs(struct gpio_handle *jtag_gpio);
void dp_G5M_unlock_dpk(struct gpio_handle *jtag_gpio);
void dp_G5M_unlock_upk1(struct gpio_handle *jtag_gpio);
void dp_G5M_unlock_upk2(struct gpio_handle *jtag_gpio);
void dp_G5M_load_dpk(struct gpio_handle *jtag_gpio);
void dp_G5M_load_upk1(struct gpio_handle *jtag_gpio);
void dp_G5M_load_upk2(struct gpio_handle *jtag_gpio);
void dp_G5M_read_shared_buffer(struct gpio_handle *jtag_gpio, unsigned char ucNumOfBlocks);
void dp_G5M_set_pgm_mode(struct gpio_handle *jtag_gpio);
void dp_G5M_load_bsr(struct gpio_handle *jtag_gpio);
void dp_G5M_perform_isc_enable(struct gpio_handle *jtag_gpio);
void dp_G5M_process_data(struct gpio_handle *jtag_gpio, unsigned char BlockID);
void dp_G5M_get_data_status(struct gpio_handle *jtag_gpio);
void dp_G5M_report_certificate(struct gpio_handle *jtag_gpio);
void dp_G5M_read_certificate(struct gpio_handle *jtag_gpio);
void dp_G5M_display_bitstream_digest(void);
void dp_G5M_do_zeroize(struct gpio_handle *jtag_gpio, unsigned char zmode);
void dp_G5M_do_read_zeroization_result(struct gpio_handle *jtag_gpioid);
void dp_G5M_check_cycle_count(struct gpio_handle *jtag_gpio);

/* Initialization functions */
void dp_G5M_device_poll(struct gpio_handle *jtag_gpio, unsigned char bits_to_shift, unsigned char Busy_bit);
void dp_G5M_device_shift_and_poll(struct gpio_handle *jtag_gpio, unsigned char bits_to_shift,
				  unsigned char Busy_bit, unsigned char Variable_ID, unsigned long start_bit_index);
void dp_G5M_initialize(struct gpio_handle *jtag_gpio);
void dp_G5M_exit(struct gpio_handle *jtag_gpio);
void dp_G5M_poll_device_ready_during_exit(struct gpio_handle *jtag_gpio);
void dp_G5M_set_mode(struct gpio_handle *jtag_gpio);
void dp_G5M_clear_errors(struct gpio_handle *jtag_gpio);

/* Erase function */
void dp_G5M_erase(void);
#endif /* INC_DPG5ALG_H */

/* *************** End of File *************** */
