/*******************************************************************************

*   FileName : si479xx_hal.h

*   Copyright (c) Telechips Inc.

*   Description :

********************************************************************************
*
*   TCC Version 1.0

This source code contains confidential information of Telechips.

Any unauthorized use without a written permission of Telechips including not
limited to re-distribution in source or binary form is strictly prohibited.

This source code is provided "AS IS" and nothing contained in this source code
shall constitute any express or implied warranty of any kind, including without
limitation, any warranty of merchantability, fitness for a particular purpose
or non-infringement of any patent, copyright or other third party intellectual
property right.
No warranty is made, express or implied, regarding the information's accuracy,
completeness, or performance.

In no event shall Telechips be liable for any claim, damages or other
liability arising from, out of or in connection with this source code or
the use in the source code.

This source code is provided subject to the terms of a Mutual Non-Disclosure
Agreement between Telechips and Company.
*
*******************************************************************************/
#ifndef _SI479XX_HAL_H_
#define _SI479XX_HAL_H_

/***************************************************
*				Include					*
****************************************************/
#ifdef __ANDROID__
#include <android/log.h>
#endif

/***************************************************
*				Defines					*
****************************************************/
/* Silab Debug message control */
//#define SILAB_DEBUG

#ifdef __ANDROID__

#define SILAB_TAG			("[RADIO][SILAB]")
#define SILAB_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,SILAB_TAG, __VA_ARGS__))
#define SILAB_INF(...)		(__android_log_print(ANDROID_LOG_INFO,SILAB_TAG, __VA_ARGS__))
#ifdef SILAB_DEBUG
#define SILAB_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,SILAB_TAG, __VA_ARGS__))
#else
#define	SILAB_DBG(...)
#endif
#define SILAB_PRINTF(...)

#else // #ifdef __ANDROID__

#define SILAB_ERR(...)		((void)printf("[ERROR][RADIO][SILAB]: " __VA_ARGS__))
#define SILAB_INF(...)		((void)printf("[INFO][RADIO][SILAB]: " __VA_ARGS__))
#ifdef SILAB_DEBUG
#define SILAB_DBG(...)		((void)printf("[DEBUG][RADIO][SILAB]: " __VA_ARGS__))
#else
#define	SILAB_DBG(...)
#endif
#define SILAB_PRINTF(...)	((void)printf(__VA_ARGS__))

#endif // #ifdef __ANDROID__

//#define USE_COMMON_IQ_CLOCK
//#define USE_DAB_IQ_CLKINV                     // Invert the bit clock input of the I/Q I2S block.
//#define USE_THIRD_TUNER_AS_SINGLE_TUNER       // Use the third tuner as the first tuner.
#define USE_DAB_DIGITAL_AGC                     // Only available definition in FW v4.x.x.x.x
#define SI47952_DAB_ISSUE

//#define SILAB_FW_VER			(30)            // 30 or 40 (Firmare Release Version: v2.5 or v3.0 or v4.0), v2.5 is not more supported.
//#define SILAB_FW_VER			(40)            // pre-release version
#define SILAB_FW_VER			(50)            // Released in 2023-06-16
#if SILAB_FW_VER == (50)
#define SILAB_FW_VER_STRING     "v5.2.0.0.5"
#elif SILAB_FW_VER == (40)
#define SILAB_FW_VER_STRING     "v4.1.0.0.4"
#else
#define SILAB_FW_VER_STRING     "v3.0.0.0.2"
#endif

#define si479xx_mwait(X)			usleep((X)*(1000))

#define SILAB_MAX_TUNER_IC_NUM		4

#define CMD_BUFF_SIZE				256
#define RSQ_BUFF_SIZE				516		// consider the size of the f/w tx 1 unit!!!

#define SI479XX_FW_IMAGE_BUFF_SIZE  4100	// 4096 + 4
#define MAX_IMAGE_COUNT				0x0f
#define IMAGE_INFO_SIZE				0x10
#define IMAGE_CNT_OFFSET			0x0100
#define GUID_IN_TABLE_OFFSET		0x0
#define IC_BITFIELD_IN_TABLE_OFFSET	0x8

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum
{
    eCLK_OFF,
    eCLK_XTAL,
    eCLK_SINGLE,
    eCLK_SINGLE_AC,
    eCLK_SINGLE_DC,
    eCLK_DIFFERENTIAL
}eCLK_MODE_t;

typedef enum
{
    TUNER_FW,
    FLASH_UTIL
}eFW_IMG_t;

typedef enum
{
	POWERUP_RESET = 0x01,
	POWERUP_BOOTLOADER = 0x02,
	POWERUP_BOOTREADY = 0x04,
	POWERUP_APPLICATION = 0x08,
}ePOWERUP_STATE_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct
{
	uint8	CTSIEN;
	uint8	CLKO_CURRENT;
	uint8	VIO;
	uint8	CLKOUT;
	eCLK_MODE_t 	CLK_MODE;
	uint8	TR_SIZE;
	uint8	CTUN;
	uint32	XTAL_FREQ;	//crystal clock
	uint8	IBIAS;		//only for crytal oscillator
	uint8	AFS; 		// 0: 48kS/s 1:44.1kS/s
	uint8	DIGDAT;		// 0: HD
	uint8	CHIPID;
	uint8	ISFSMODE;
	uint8	ICLINK;

	uint8	IQCHANNELS;
	uint8	IQFMT;
	uint8	IQMUX;
	uint8	IQRATE;

	uint8	IQSWAP1;
	uint8	IQSWAP0;
	uint8	IQSLOT;
	uint8	IQEARLY;
	uint8	IQOUT;

	uint8	EZIQ_MASTER;	// available since v4p0
	uint8	EZIQ_ENABLE;	// available since v4p0
}stPOWER_UP_ARGS_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern uint8 cmd_buff[CMD_BUFF_SIZE];
extern uint8 rsp_buff[RSQ_BUFF_SIZE];

/***************************************************
*			Function declaration				*
****************************************************/
extern RET si479xx_command(uint32 cmd_size, uint8 *cmd, uint32 reply_size, uint8 *reply, uint32 ntuner);
extern RET si479xx_waitForSTC(uint32 ntuner);
extern RET si479xx_bootup(uint32 tunertype);
extern void si479xx_16bto8b(uint8 *buf, uint16 data);
extern uint32 si479xx_u8btou32b(uint8 src);
extern void *si479xx_memset(void *pvdst, uint8 ubch, uint32 uilen);
extern void *si479xx_memcpy(void *pvDst, void *pvSrc, uint32 uilen);
extern uint8 si479xx_getError(uint32 ntuner);
extern RET si479xx_setDabFreqListToReg(uint32 *dab_freq_table, uint8 num_freq, uint32 ntuner);
extern uint8 si479xx_getMaxDabFreqCount(void);
extern uint8 si479xx_getCurrentDabFreqCount(uint32 ntuner);
extern RET si479xx_getDabFreqListFromReg(uint32 *dab_freq_table, uint8 *num_freq, uint32 ntuner);
extern uint32 si479xx_getDabFreqFromList(uint16 index, uint32 ntuner);
extern RET si479xx_setIQTestPatternControl(uint32 fOnOff, uint32 sel);
extern RET si479xx_setTunerProperty(uint8 group, uint8 index, uint8 hdata, uint8 ldata, uint32 ntuner);
extern RET si479xx_setProperty(uint8 group, uint8 number, uint8 hdata, uint8 ldata, uint32 ntuner);
extern void si479xx_setIqProperty(uint32 tunertype);

#endif
