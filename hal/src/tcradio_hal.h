/*******************************************************************************

*   FileName : tcradio_hal.h

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
#ifndef __TCRADIO_HAL_H__
#define __TCRADIO_HAL_H__

/***************************************************
*				Include					*
****************************************************/
#include "tcradio_drv.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define RHAL_DEBUG

#ifdef __ANDROID__

#define RHAL_TAG			("[RADIO][HAL]")
#define RHAL_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,RHAL_TAG, __VA_ARGS__))
#ifdef RHAL_DEBUG
#define RHAL_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,RHAL_TAG, __VA_ARGS__))
#else
#define	RHAL_DBG(...)
#endif

#else // #ifdef __ANDROID__

#define RHAL_ERR(...)		((void)printf("[ERROR][RADIO][HAL]: " __VA_ARGS__))
#ifdef RHAL_DEBUG
#define RHAL_DBG(...)		((void)printf("[DEBUG][RADIO][HAL]: " __VA_ARGS__))
#else
#define	RHAL_DBG(...)
#endif

#endif // #ifdef __ANDROID__

#define	INCLUDE_ALL_TUNER_DRIVER
#define	TUNER_QVALUE_COUNT	20

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eTUNER_IC_M0	= 0,
	eTUNER_IC_S0	= 1,
	eTUNER_IC_X0	= 2,
	eTUNER_IC_T0	= 3
}eTUNER_HAL_IC_t;

/***************************************************
*				Typedefs					*
****************************************************/
// Align each band with 32bit x 20 values
typedef union {
	struct {
		uint32 Qvalue[TUNER_QVALUE_COUNT];
	}fm;

	struct {
		uint32 Qvalue[TUNER_QVALUE_COUNT];
	}am;

	struct {
		uint32 Qvalue[TUNER_QVALUE_COUNT];
	}dab;
}stTUNER_QVALUE_t;

typedef struct {
	uint32 type;
	stTUNER_QVALUE_t qual;
}stTUNER_QUALITY_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern RET tcradiohal_init(void);
extern RET tcradiohal_deinit(void);
extern RET tcradiohal_open(stTUNER_DRV_CONFIG_t conf);
extern RET tcradiohal_close(void);
extern RET tcradiohal_getMaxDabFreqList(uint8 *maxNums);
extern RET tcradiohal_getCurrentDabFreqList(uint8 *curNums, uint32 ntuner);
extern RET tcradiohal_setDabFreqList(uint32 *dab_freq_hz_table, uint8 num_freq, uint32 ntuner);	// num_freq = max.64
extern RET tcradiohal_getDabFreqList(uint32 *dab_freq_hz_table, uint8 *num_freq, uint32 ntuner);
extern RET tcradiohal_setTune(uint32 mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner);
extern RET tcradiohal_getTune(uint32 *mod_mode, uint32 *freq, uint32 ntuner);
extern RET tcradiohal_getQuality(uint32 mod_mode, stTUNER_QUALITY_t *qdata, uint32 ntuner);
extern RET tcradiohal_setRdsConfig(uint32 fOnOff, uint32 reserved, uint32 ntuner);
extern RET tcradiohal_getRdsData(uint8 *rdsbuf, uint32 ntuner);
extern RET tcradiohal_setMute(uint32 fOnOff, uint32 ntuner);
extern RET tcradiohal_spi_writeRead(uint8 *tx, uint8 *rx, uint32 len, uint32 cs);
extern RET tcradiohal_i2c_write(uint8 addr, uint8 *tx, uint32 len);
extern RET tcradiohal_i2c_read(uint8 addr, uint8 *reg, uint32 reglen, uint8* rx, uint32 rxlen);
extern uint32 tcradiohal_getHalVersion(void);
extern uint32 tcradiohal_getTunerDrvVersion(void);
extern int32 tcradiohal_getTunerChip(void);

/******************************************
* I/Q data driver	I/F for digital radio *
*******************************************/
extern float32 tcradiohal_getPreciseIqSampleRate(uint32 ntuner);
extern int32 tcradiohal_getIqSampleRate(uint32 ntuner);
extern int32 tcradiohal_getIqSamplingBit(uint32 ntuner);
extern RET tcradiohal_setDigitalAGC(uint8 fOnOff, int8 target_power, uint8 inc_ms, uint8 dec_ms, uint32 ntuner);

// for test
extern RET tcradiohal_setTunerCommand(uint8 *txdata, uint32 txlen, uint8 *rxdata, uint32 rxlen, uint32 ntuner);
extern RET tcradiohal_getTunerStatus(uint32 cmd, uint8 *buf, uint32 len, uint32 ntuner);
extern RET tcradiohal_setIQTestPattern(uint32 fOnOff, uint32 sel);
extern RET tcradiohal_setPropertyOfPrimaryTuner(uint8 group, uint8 number, uint8 hdata, uint8 ldata);
extern void tcradiohal_testReliability(void);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __TCRADIO_HAL_H__
