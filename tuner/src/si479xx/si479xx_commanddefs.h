/***************************************************************************************
                  Silicon Laboratories Broadcast Si475x/6x/9x module code

   EVALUATION AND USE OF THIS SOFTWARE IS SUBJECT TO THE TERMS AND CONDITIONS OF
     THE SOFTWARE LICENSE AGREEMENT IN THE DOCUMENTATION FILE CORRESPONDING
     TO THIS SOURCE FILE.
   IF YOU DO NOT AGREE TO THE LIMITED LICENSE AND CONDITIONS OF SUCH AGREEMENT,
     PLEASE RETURN ALL SOURCE FILES TO SILICON LABORATORIES.

   Date: June 06 2014
  (C) Copyright 2014, Silicon Laboratories, Inc. All rights reserved.
****************************************************************************************/


/**
 *@file this header file define the common command for si479x headunit.
 *
 * @author Michael.Yi@silabs.com
*/
#ifndef _SI479X_COMMAND_DEFS_H_
#define _SI479X_COMMAND_DEFS_H_

//==================================================================
// General Commands
//==================================================================
#define READ_REPLY          0x00
#define POWER_UP            0X01
#define READ_MORE           0x02

#define HOST_LOAD           0X04
#define FLASH_LOAD          0X05
#define LOAD_INIT           0X06
#define BOOT                0x07
#define GET_PART_INFO       0X08
#define GET_SYS_STATE       0X09
#define GET_POWER_UP_ARGS   0X0a
#define LOAD_CONFIG         0X0c
#define GET_CHIP_INFO       0x0d

#define GET_FUNC_INFO       0x12
#define SET_PROPERTY        0x13
#define GET_PROPERTY        0x14
#define SET_MODE            0x15
#define GET_MODE            0x16
#define AGC_STATUS          0x17
#define PIN_STATUS          0x19
#define PIN_CONFIG_HDBLEND 	0x1a

#define I2STX0_CONFIG		0x20
#define I2STX1_CONFIG		0x21

#define I2SRX0_CONFIG		0x28
#define I2SRX1_CONFIG		0x29
#define I2SRX2_CONFIG		0x2a

#define I2S_CONTROL  		0x2f

#define FM_TUNE_FREQ        0x30
#define FM_RSQ_STATUS       0x32
#define FM_RDS_STATUS       0x33
#define FM_RDS_BLOCKCOUNT   0x34

#define FM_SET_SERVO        0x35
#define FM_GET_SERVO        0x36
#define FM_SERVO_STATUS     0X37

#define FM_SET_SERVO_TRANSFORM  0x38
#define FM_GET_SERVO_TRANSFORM  0x39

#define FM_SET_FAST_SERVO_TRANSFORM  0x68
#define FM_GET_FAST_SERVO_TRANSFORM  0x69

#define AM_SET_SERVO        0x45
#define AM_GET_SERVO        0x46
#define AM_SET_SERVO_TRANSFORM 0x48
#define AM_GET_SERVO_TRANSFORM 0x49

#define AM_TUNE_FREQ        0x40
#define AM_RSQ_STATUS       0X42
#define AM_SERVO_STATUS     0x47

#define FM_PHASE_DIVERSITY  0x50
#define FM_PHASE_DIV_STATUS 0x51

#define FM_SIGNAL_EVENT_COUNT 0x60

#define AM_SIGNAL_EVENT_COUNT   0x70

#define DAB_TUNER_FREQ		0xb0
#define DAB_RSQ_STATUS		0xb2

#define HIFI_CMD0           0x80
#define HIFI_CMD1           0x81
#define HIFI_CMD2           0x82
#define HIFI_CMD3           0x83

#define HIFI_RESP0          0x88
#define HIFI_RESP1          0x89
#define HIFI_RESP2          0x8a
#define HIFI_RESP3          0x8b

#define DAC_CONFIG          0x90
#define DAC_STATUS          0x91
#define ADC_CONNECT         0x92
#define ADC_DISCONNECT      0x93
#define ADC_STATUS          0x94

#define DAB_SET_FREQ_LIST	0xb3
#define DAB_GET_FREQ_LIST	0xb4

#define TUNER_CMD_GATEWAY	0xc0

#define TEST_SEND_TEST_PATTERN	0xe0

#define PART_OVERRIDE       0xcf

//==================================================================
// Status bit
//==================================================================
#define STCINT  0x01    ///< STC bit postision in status
#define TRIGINT 0x02    ///< TRIGINT bit postision in status
#define RDSINT  0x04    ///< RDSINT bit postision in status
#define SWERR   0x20    ///< SWERR bit postision in status
#define APIERR  0x40    ///< APIERR bit postision in status
#define CTS     0x80    ///< CTS bit postision in status
#define ACTS0   0x01    ///< ACTS0 bit postision in status
#define ACTS1   0x02    ///< ACTS1 bit postision in status
#define ACTS2   0x04    ///< ACTS2 bit postision in status
#define ACTS3   0x08    ///< ACTS3 bit postision in status

#define HOTWRN  0x20    ///< HOTWRN bit
#define TMPERR  0x10    ///< TMPERR bit
#define CMDERR  0x08    ///< CMDERR bit
#define REPERR  0x04    ///< REPERR bit
#define ARBERR  0x02    ///< ARBERR bit
#define NRERR   0x01    ///< NRERR bit

#endif
