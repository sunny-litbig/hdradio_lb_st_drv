/*******************************************************************************

*   FileName : x0tuner_core.c

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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tcradio_types.h"
#include "tcradio_peri_config.h"
#include "tcradio_drv.h"
#include "x0tuner_core.h"
#include "x0tuner_hal.h"

#define	__X0TUNER_DRIVER_VER_PREFIX__			'X'
#define	__X0TUNER_DRIVER_VER_RELEASE_NUMBER__	0x02U
#define	__X0TUNER_DRIVER_VER_MAJOR_NUMBER__		0x00U
#define	__X0TUNER_DRIVER_VER_MINOR_NUMBER__		0x00U

stTUNER_DRV_CONFIG_t gX0Conf;

uint32 x0tuner_drv_initialized = 0;
uint32 x0tuner_drv_current_band[X0_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};
uint32 x0tuner_drv_fm_frequency[X0_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};
uint32 x0tuner_drv_am_frequency[X0_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};

void x0tuner_reset(void);
RET x0tuner_setTune(uint32 mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner);

/*========================================================
Internal Functions For Application Programming Interface
==========================================================*/
RET x0tuner_open(stTUNER_DRV_CONFIG_t type)
{
	RET ret = eRET_OK;
	uint32 trycnt=0;

	X0_DBG("[%s] \n", __func__);

	if(x0tuner_drv_initialized != 0) {
		ret = eRET_NG_ALREADY_OPEN;
		return ret;
	}

	gX0Conf = type;

	if(pfnI2cOpen != NULL && pfnI2cClose != NULL && pfnI2cTx != NULL && pfnI2cRx != NULL) {
		(*pfnI2cOpen)();
		x0tuner_reset();
		ret = x0tunerhal_bootup(0);
		if(ret == eRET_OK) {
			switch(type.numTuners) {
				case eTUNER_DRV_CONF_TYPE_QUAD:		x0tuner_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_QUATERNARY);
				case eTUNER_DRV_CONF_TYPE_TRIPLE:	x0tuner_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_TERTIARY);
				case eTUNER_DRV_CONF_TYPE_DUAL:		x0tuner_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_SECONDARY);
				default:							x0tuner_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_PRIMARY);	break;
			}
		}
	}
	else {
		ret = eRET_NG_NO_RSC;
	}

	if (ret == eRET_OK) {
		x0tuner_drv_initialized = 1;
		x0tuner_setMute(0, 0);
	}
	else  {
		x0tuner_drv_initialized = 0;
	}

	return ret;
}

RET x0tuner_close(void)
{
	RET ret = eRET_OK;

	X0_DBG("[%s] \n", __func__);

	if(pfnI2cClose != NULL)			(*pfnI2cClose)();
	if(pfnTunerPower != NULL)		(*pfnTunerPower)(OFF);
	if(pfnAntPower != NULL)			(*pfnAntPower)(OFF);
	if(pfnTunerReset != NULL)		(*pfnTunerReset)(OFF);
	if(pfnRadioGpioClose != NULL)	(*pfnRadioGpioClose)();

	x0tuner_drv_initialized = 0;
	x0tunerhal_memset(x0tuner_drv_current_band, 0xff, sizeof(x0tuner_drv_current_band));
	x0tunerhal_memset(x0tuner_drv_fm_frequency, 0xff, sizeof(x0tuner_drv_fm_frequency));
	x0tunerhal_memset(x0tuner_drv_am_frequency, 0xff, sizeof(x0tuner_drv_am_frequency));

	return ret;
}

RET x0tuner_setTune(uint32 mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner)
{
	RET ret = eRET_OK;

	X0_DBG("[%s] \n", __func__);

	if(mod_mode == eTUNER_DRV_FM_MODE) {
		ret = x0tunerhal_tuneTo(ntuner, mod_mode, freq, tune_mode);
		if(ret == eRET_OK) {
			x0tuner_drv_current_band[ntuner] = eTUNER_DRV_FM_MODE;
			x0tuner_drv_fm_frequency[ntuner] = freq;
		}
	}
	else if(mod_mode == eTUNER_DRV_AM_MODE) {
		ret = x0tunerhal_tuneTo(ntuner, mod_mode, freq, tune_mode);
		if(ret == eRET_OK) {
			x0tuner_drv_current_band[ntuner] = eTUNER_DRV_AM_MODE;
			x0tuner_drv_am_frequency[ntuner] = freq;
		}
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET x0tuner_getTune(uint32 *mod_mode, uint32 *curfreq, uint32 ntuner)
{
	RET ret = eRET_OK;

	X0_DBG("[%s] \n", __func__);

	*mod_mode = x0tuner_drv_current_band[ntuner];
	if(*mod_mode == eTUNER_DRV_FM_MODE) {
		*curfreq = x0tuner_drv_fm_frequency[ntuner];
	}
	else if(*mod_mode == eTUNER_DRV_AM_MODE) {
		*curfreq = x0tuner_drv_am_frequency[ntuner];
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET x0tuner_getQuality(uint32 mod_mode, stX0_DRV_QUALITY_t *qdata, uint32 ntuner)

{
	RET ret = eRET_OK;

	X0_DBG("[%s] \n", __func__);

	x0tunerhal_getQualityData(ntuner, mod_mode, qdata);

	return ret;
}

float32 x0tuner_getPreciseIqSampleRate(uint32 ntuner)
{
	float32 samplerate;

	X0_DBG("[%s] \n", __func__);

	samplerate = (float32)x0tunerhal_getInterfaceStatus(ntuner);

	return (float32)samplerate*1000.0;
}

int32 x0tuner_getIqSampleRate(uint32 ntuner)
{
	uint16 samplerate;

	X0_DBG("[%s] \n", __func__);

	samplerate = x0tunerhal_getInterfaceStatus(ntuner);

	return (int32)samplerate*1000;
}

int32 x0tuner_getIqSamplingBit(uint32 ntuner)
{
	return (int32)16;
}

RET x0tuner_setMute(uint32 fOnOff, uint32 ntuner)
{
	RET ret = eRET_OK;

	X0_DBG("[%s] : fOnOff[%d] ", __func__, fOnOff);
	if(fOnOff == 0) {
		ret = x0tunerhal_setMute(ntuner, 0);
	}
	else {
		ret = x0tunerhal_setMute(ntuner, 1);
	}
	return ret;
}

RET x0tuner_i2cTx(uint8 addr, uint8 *tx, uint32 len)
{
	RET ret = eRET_OK;

	X0_DBG("[%s] : x0tuner_i2cTx \n", __func__);
	ret = (*pfnI2cTx)(addr, tx, len);

	return ret;
}

RET x0tuner_i2cRx(uint8 addr, uint8 *reg, uint32 reglen, uint8* rx, uint32 rxlen)
{
	RET ret = eRET_OK;

	X0_DBG("[%s] : x0tunerhal_i2cRx[%d]  \n", __func__);
	ret = (*pfnI2cRx)(addr, reg, reglen, rx, rxlen);

	return ret;
}

uint32 x0tuner_getVersion(void)
{
	return ((__X0TUNER_DRIVER_VER_PREFIX__<<24) | (__X0TUNER_DRIVER_VER_RELEASE_NUMBER__<<16) |
			(__X0TUNER_DRIVER_VER_MAJOR_NUMBER__<<8) | (__X0TUNER_DRIVER_VER_MINOR_NUMBER__<<0));
}

void x0tuner_reset(void)
{
	if(pfnRadioGpioOpen != NULL)	(*pfnRadioGpioOpen)();
	if(pfnTunerPower != NULL)		(*pfnTunerPower)(ON);
	if(pfnAntPower != NULL)			(*pfnAntPower)(OFF);
	x0tuner_mwait(50);
	if(pfnAntPower != NULL)			(*pfnAntPower)(ON);
	x0tuner_mwait(20);
	if(pfnTunerReset != NULL)		(*pfnTunerReset)(OFF);
	x0tuner_mwait(50);
	if(pfnTunerReset != NULL)		(*pfnTunerReset)(ON);
}

#ifdef INCLUDE_ONE_TUNER_DRIVER
RET tunerdrv_open(stTUNER_DRV_CONFIG_t type)
{
	return x0tuner_open(type);
}

RET tunerdrv_close(void)
{
	return x0tuner_close();
}

RET tunerdrv_setTune(uint32 mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner)
{
	return x0tuner_setTune(mod_mode, freq, tune_mode, ntuner);
}

RET tunerdrv_getFreq(uint32 *mod_mode, uint32 *curfreq, uint32 ntuner)
{
	return x0tuner_getTune(mod_mode, curfreq, ntuner);
}

RET tunerdrv_getQuality(uint32 mod_mode, void *qdata, uint32 ntuner)
{
	stX0_DRV_QUALITY_t *tempQ = (stX0_DRV_QUALITY_t *)qdata;

	return x0tuner_getQuality(mod_mode, tempQ, ntuner);
}

RET tunerdrv_setMute(uint32 fOnOff, uint32 ntuner)
{
	return x0tuner_setMute(fOnOff, ntuner);
}

RET tunerdrv_i2cTx(uint8 addr, uint8 *tx, uint32 len)
{
	return x0tuner_i2cTx(addr, tx, len);
}

RET tunerdrv_i2cRx(uint8 addr, uint8 *reg, uint32 reglen, uint8* rx, uint32 rxlen)
{
	return x0tuner_i2cRx(addr, reg, reglen, rx, rxlen);
}

RET tunerdrv_cspiTxRxData(uint8 *tx, uint8 *rx, uint32 len, uint32 cs)
{
	return eRET_NG_NOT_SUPPORT;
}

uint32 tunerdrv_getVersion(void)
{
	return x0tuner_getVersion();
}
#endif

