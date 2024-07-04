/*******************************************************************************

*   FileName : tcradio_hal.c

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
#include "tcradio_types.h"
#include "tcradio_peri_config.h"
#include "tcradio_hal.h"
#include "tcradio_hal_config.h"
#include "tcradio_hal_mutex.h"
#include "dev_audio.h"
#ifdef USE_S0_TUNER
#include "si479xx_core.h"
#endif
#ifdef USE_X0_TUNER
#include "x0tuner_core.h"
#endif
#ifdef USE_T0_TUNER
#include "star_driver.h"
#endif

#define	__RADIO_HAL_VER_PREFIX__			'H'
#define	__RADIO_HAL_VER_RELEASE_NUMBER__	0x02
#define	__RADIO_HAL_VER_MAJOR_NUMBER__		0x00
#define	__RADIO_HAL_VER_MINOR_NUMBER__		0x00

static uint8 fTunerInit = 0;
static int32 curTunerIC = eRET_NG_NOT_SUPPORT;

static void *tcradiohal_memset(void *pdst, uint8 ch, uint32 len)
{
	uint8 *pW;
	uint32 i;

	pW = (uint8 *)pdst;

	if(len == 0)
		return (void *)pW;

	for(i=0; i<len; i++)
	{
		*pW++ = ch;
	}
	return (void *)pW;
}

static void tcradiohal_deinitTunerPeriDrivers(void)
{
	tcradio_releaseTunerPeriDrivers();
}

RET tcradiohal_init(void)
{
	RET ret = eRET_OK;

	ret= tcradiohal_mutexInit();
	if(ret != eRET_OK) {
		RHAL_ERR("[%s:%d] Failed to init radio hal mutex!!!\n", __func__, __LINE__);
	}

	tcradio_configTunerPeriDrivers();
	tcradio_configAoutDriver(dev_aout_open, dev_aout_close, dev_aout_write, dev_aout_command);
	tcradio_configAinDriver(dev_ain_open, dev_ain_close, dev_ain_read, dev_ain_command);

	tcradiohal_extraInit();

	if(tcradio_getM0TunerConfig() != 0) {
		curTunerIC = eTUNER_IC_M0;
	}
	else if(tcradio_getX0TunerConfig() != 0) {
		curTunerIC = eTUNER_IC_X0;
	}
	else if(tcradio_getS0TunerConfig() != 0) {
		curTunerIC = eTUNER_IC_S0;
	}
	else if(tcradio_getT0TunerConfig() != 0) {
		curTunerIC = eTUNER_IC_T0;
	}
	else {
		curTunerIC = -1;
	}

	RHAL_DBG("Current tuner type num is [%d]\n", curTunerIC);

	return ret;
}

RET tcradiohal_deinit(void)
{
	RET ret = eRET_OK;

	tcradiohal_deinitTunerPeriDrivers();
	tcradio_releaseAoutDriver();
	tcradio_releaseAinDriver();

	tcradiohal_extraDeinit();

	ret= tcradiohal_mutexDeinit();

	return ret;
}

RET tcradiohal_open(stTUNER_DRV_CONFIG_t conf)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:	ret = max2175_open(conf);	break;
	#endif
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:	ret = si479xx_open(conf);	break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:	ret = x0tuner_open(conf);	break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:	ret = star_open(conf);		break;
	#endif
		default:
			ret = eRET_NG_INVALID_PARAM;
			break;
	}
#else
	ret = tunerdrv_open(conf);
#endif
	if (ret == eRET_OK) {
		fTunerInit = 1;
	}
	else  {
		fTunerInit = 0;
		RHAL_DBG("[%s] : Failed to open radio!!! ret[%d] \n", __func__, ret);
	}
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_close(void)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:	ret = max2175_close();			break;
	#endif
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:	ret = si479xx_close();			break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:	ret = x0tuner_close();			break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:	ret = star_close();				break;
	#endif
		default:			ret = eRET_NG_INVALID_PARAM;	break;
	}
#else
	ret = tunerdrv_close();
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_getMaxDabFreqList(uint8 *maxNums)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
	*maxNums = 0;
#ifdef INCLUDE_ALL_TUNER_DRIVER
		switch(curTunerIC) {
	#ifdef USE_M0_TUNER
			case eTUNER_IC_M0:	ret = eRET_NG_NO_RSC;			break;
	#endif
	#ifdef USE_S0_TUNER
			case eTUNER_IC_S0:	*maxNums = si479xx_getMaxDabFreqList();	break;
	#endif
	#ifdef USE_X0_TUNER
			case eTUNER_IC_X0:	ret = eRET_NG_NO_RSC;			break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:		ret = eRET_NG_NO_RSC;			break;
	#endif
			default:			ret = eRET_NG_INVALID_PARAM;	break;
		}
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_getCurrentDabFreqList(uint8 *curNums, uint32 ntuner)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
	*curNums = 0;
#ifdef INCLUDE_ALL_TUNER_DRIVER
		switch(curTunerIC) {
	#ifdef USE_M0_TUNER
			case eTUNER_IC_M0:	ret = eRET_NG_NO_RSC;			break;
	#endif
	#ifdef USE_S0_TUNER
			case eTUNER_IC_S0:	*curNums = si479xx_getCurrentDabFreqList(ntuner);	break;
	#endif
	#ifdef USE_X0_TUNER
			case eTUNER_IC_X0:	ret = eRET_NG_NO_RSC;			break;
	#endif
	#ifdef USE_T0_TUNER
			case eTUNER_IC_T0:	ret = eRET_NG_NO_RSC;			break;
	#endif
			default:			ret = eRET_NG_INVALID_PARAM;	break;
		}
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_setDabFreqList(uint32 *dab_freq_hz_table, uint8 num_freq, uint32 ntuner)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
		switch(curTunerIC) {
	#ifdef USE_M0_TUNER
			case eTUNER_IC_M0:	ret = eRET_NG_NO_RSC;			break;
	#endif
	#ifdef USE_S0_TUNER
			case eTUNER_IC_S0:	ret = si479xx_setDabFreqList(dab_freq_hz_table, num_freq, ntuner);	break;
	#endif
	#ifdef USE_X0_TUNER
			case eTUNER_IC_X0:	ret = eRET_NG_NO_RSC;			break;
	#endif
	#ifdef USE_T0_TUNER
			case eTUNER_IC_T0:	ret = eRET_NG_NO_RSC;			break;
	#endif
			default:			ret = eRET_NG_INVALID_PARAM;	break;
		}
#else
		ret = tunerdrv_setDabFreqList(dab_freq_hz_table, num_freq, ntuner);
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_getDabFreqList(uint32 *dab_freq_hz_table, uint8 *num_freq, uint32 ntuner)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
		switch(curTunerIC) {
	#ifdef USE_M0_TUNER
			case eTUNER_IC_M0:	ret = eRET_NG_NO_RSC;			break;
	#endif
	#ifdef USE_S0_TUNER
			case eTUNER_IC_S0:	ret = si479xx_getDabFreqList(dab_freq_hz_table, num_freq, ntuner);	break;
	#endif
	#ifdef USE_X0_TUNER
			case eTUNER_IC_X0:	ret = eRET_NG_NO_RSC;			break;
	#endif
	#ifdef USE_T0_TUNER
			case eTUNER_IC_T0:	ret = eRET_NG_NO_RSC;			break;
	#endif
			default:			ret = eRET_NG_INVALID_PARAM;	break;
		}
#else
		ret = tunerdrv_getDabFreqList(dab_freq_hz_table, num_freq, ntuner);
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_setTune(uint32 mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:	ret = max2175_setTune(mod_mode, freq, tune_mode, ntuner);	break;
	#endif
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:	ret = si479xx_setTune(mod_mode, freq, tune_mode, ntuner);	break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:	ret = x0tuner_setTune(mod_mode, freq, tune_mode, ntuner);	break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:	ret = star_setTune(mod_mode, freq, tune_mode, ntuner);		break;
	#endif
		default:			ret = eRET_NG_INVALID_PARAM;								break;
	}
#else
	ret = tunerdrv_setTune(mod_mode, freq, tune_mode, ntuner);
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_getTune(uint32 *mod_mode, uint32 *freq, uint32 ntuner)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:	ret = max2175_getTune(mod_mode, freq, ntuner);		break;
	#endif
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:	ret = si479xx_getTune(mod_mode, freq, ntuner);		break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:	ret = x0tuner_getTune(mod_mode, freq, ntuner);		break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:	ret = star_getTune(mod_mode, freq, ntuner);			break;
	#endif
		default:			ret = eRET_NG_INVALID_PARAM;						break;
	}
#else
	ret = tunerdrv_getFreq(mod_mode, freq, ntuner);
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_getQuality(uint32 mod_mode, stTUNER_QUALITY_t *qdata, uint32 ntuner)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
	if(mod_mode == eTUNER_DRV_FM_MODE || mod_mode == eTUNER_DRV_AM_MODE || mod_mode == eTUNER_DRV_DAB_MODE) {
		tcradiohal_memset(qdata, 0, sizeof(stTUNER_QUALITY_t));
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:
			qdata->type	= eTUNER_IC_M0;
			if(mod_mode == eTUNER_DRV_FM_MODE) {
				max2175_getRFgain(&qdata->qual.fm.Qvalue[0], ntuner);
			}
			else {
				max2175_getRFgain(&qdata->qual.am.Qvalue[0], ntuner);
			}
			break;
	#endif
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:
			{
				stSILAB_DRV_QUALITY_t tempQ;
				ret = si479xx_getQuality(mod_mode, &tempQ, ntuner);
				if(ret == eRET_OK) {
					qdata->type	= eTUNER_IC_S0;
					if(mod_mode == eTUNER_DRV_FM_MODE) {
						qdata->qual.fm.Qvalue[0] = tempQ.fm.Rssi;
						qdata->qual.fm.Qvalue[1] = tempQ.fm.Snr;
						qdata->qual.fm.Qvalue[2] = tempQ.fm.Dev;
						qdata->qual.fm.Qvalue[3] = tempQ.fm.Offs;
						qdata->qual.fm.Qvalue[4] = tempQ.fm.Pilot;
						qdata->qual.fm.Qvalue[5] = tempQ.fm.Mpth;
						qdata->qual.fm.Qvalue[6] = tempQ.fm.Usn;
						qdata->qual.fm.Qvalue[7] = tempQ.fm.Flag;
						qdata->qual.fm.Qvalue[8] = tempQ.fm.Valid;
						qdata->qual.fm.Qvalue[9] = tempQ.fm.Freq;
						qdata->qual.fm.Qvalue[10] = tempQ.fm.Hdlevel;
					}
					else if(mod_mode == eTUNER_DRV_AM_MODE) {
						qdata->qual.am.Qvalue[0] = tempQ.am.Rssi;
						qdata->qual.am.Qvalue[1] = tempQ.am.Snr;
						qdata->qual.am.Qvalue[2] = tempQ.am.Mod;
						qdata->qual.am.Qvalue[3] = tempQ.am.Offs;
						qdata->qual.am.Qvalue[4] = tempQ.am.Flag;
						qdata->qual.am.Qvalue[5] = tempQ.am.Valid;
						qdata->qual.am.Qvalue[6] = tempQ.am.Freq;
						qdata->qual.am.Qvalue[7] = tempQ.am.Hdlevel;
					}
					else {
						// eTUNER_DRV_DAB_MODE
						qdata->qual.dab.Qvalue[0] = tempQ.dab.Rssi;
						qdata->qual.dab.Qvalue[1] = tempQ.dab.Sqi;
						qdata->qual.dab.Qvalue[2] = tempQ.dab.Detect;
						qdata->qual.dab.Qvalue[3] = tempQ.dab.RssiAdj;
						qdata->qual.dab.Qvalue[4] = tempQ.dab.Dagc;
						qdata->qual.dab.Qvalue[5] = tempQ.dab.Valid;
						qdata->qual.dab.Qvalue[6] = tempQ.dab.Index;
						qdata->qual.dab.Qvalue[7] = tempQ.dab.Freq;
					}
				}
			}
			break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:
			{
				stX0_DRV_QUALITY_t tempQ;
				ret = x0tuner_getQuality(mod_mode, &tempQ, ntuner);
				if(ret == eRET_OK) {
					qdata->type	= eTUNER_IC_X0;
					if(mod_mode == eTUNER_DRV_FM_MODE) {
						qdata->qual.fm.Qvalue[0] = tempQ.fm.Status;
						qdata->qual.fm.Qvalue[1] = tempQ.fm.Rssi;
						qdata->qual.fm.Qvalue[2] = tempQ.fm.Usn;
						qdata->qual.fm.Qvalue[3] = tempQ.fm.Mpth;
						qdata->qual.fm.Qvalue[4] = tempQ.fm.Offs;
						qdata->qual.fm.Qvalue[5] = tempQ.fm.Bwth;
						qdata->qual.fm.Qvalue[6] = tempQ.fm.Mod;
					}
					else {
						qdata->qual.am.Qvalue[0] = tempQ.am.Status;
						qdata->qual.am.Qvalue[1] = tempQ.am.Rssi;
						qdata->qual.am.Qvalue[2] = tempQ.am.Hfn;
						qdata->qual.am.Qvalue[3] = tempQ.am.Coch;
						qdata->qual.am.Qvalue[4] = tempQ.am.Offs;
						qdata->qual.am.Qvalue[5] = tempQ.am.Bwth;
						qdata->qual.am.Qvalue[6] = tempQ.am.Mod;
					}
				}
			}
			break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:
		{
				stSTAR_DRV_QUALITY_t tempQ;
				ret = star_getQuality(mod_mode, &tempQ, ntuner);
				if(ret == eRET_OK) {
					qdata->type	= eTUNER_IC_T0;
					if(mod_mode == eTUNER_DRV_FM_MODE) {
						qdata->qual.fm.Qvalue[0] = tempQ.fm.FstRF;
						qdata->qual.fm.Qvalue[1] = tempQ.fm.FstBB;
						qdata->qual.fm.Qvalue[2] = tempQ.fm.Det;
						qdata->qual.fm.Qvalue[3] = tempQ.fm.Snr;
						qdata->qual.fm.Qvalue[4] = tempQ.fm.Adj;
						qdata->qual.fm.Qvalue[5] = tempQ.fm.Dev;
						qdata->qual.fm.Qvalue[6] = tempQ.fm.Mpth;
						qdata->qual.fm.Qvalue[7] = tempQ.fm.MpxNoise;
						qdata->qual.fm.Qvalue[8] = tempQ.fm.Stereo;
						qdata->qual.fm.Qvalue[9] = tempQ.fm.CoCh;
					}
					else if(mod_mode == eTUNER_DRV_AM_MODE) {
						qdata->qual.fm.Qvalue[0] = tempQ.am.FstRF;
						qdata->qual.fm.Qvalue[1] = tempQ.am.FstBB;
						qdata->qual.fm.Qvalue[2] = tempQ.am.Det;
						qdata->qual.fm.Qvalue[3] = tempQ.am.Snr;
						qdata->qual.fm.Qvalue[4] = tempQ.am.Adj;
						qdata->qual.fm.Qvalue[5] = tempQ.am.Dev;
					}
					else {
						// eTUNER_DRV_DAB_MODE
						qdata->qual.dab.Qvalue[0] = tempQ.dab.FstRF;
						qdata->qual.dab.Qvalue[1] = tempQ.dab.FstBB;
					}
				}
			}
		break;
	#endif
		default:
			ret = eRET_NG_INVALID_PARAM;
			break;
	}
#else
	ret = tunerdrv_getQuality(mod_mode, qdata, ntuner);
#endif
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_setRdsConfig(uint32 fOnOff, uint32 reserved, uint32 ntuner)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:	ret = eRET_NG_NO_RSC;			break;
	#endif
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:	ret = si479xx_setRdsConfig(fOnOff, 0x02, 0x02, ntuner);		break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:	ret = eRET_NG_NO_RSC;			break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:	ret = eRET_NG_NO_RSC;			break;
	#endif
		default:			ret = eRET_NG_INVALID_PARAM;	break;
	}
#else
	ret = tunerdrv_setMute(fOnOff, ntuner);
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_getRdsData(uint8 *rdsbuf, uint32 ntuner)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:	ret = eRET_NG_INVALID_PARAM;				break;
	#endif
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:	ret = si479xx_getRdsData(rdsbuf, ntuner);	break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:	ret = eRET_NG_INVALID_PARAM;				break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:	ret = eRET_NG_NO_RSC;						break;
	#endif
		default:			ret = eRET_NG_INVALID_PARAM;				break;
	}
#else
	ret = tunerdrv_getRdsData(ntuner);
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_setMute(uint32 fOnOff, uint32 ntuner)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:	ret = eRET_NG_NO_RSC;						break;
	#endif
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:	ret = si479xx_setMute(fOnOff, ntuner);		break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:	ret = x0tuner_setMute(fOnOff, ntuner);		break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:	ret = star_setMute(fOnOff, ntuner);			break;
	#endif
		default:			ret = eRET_NG_INVALID_PARAM;				break;
	}
#else
	ret = tunerdrv_setMute(fOnOff, ntuner);
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_spi_writeRead(uint8 *tx, uint8 *rx, uint32 len, uint32 cs)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:	ret = si479xx_cspiTxRxData(tx, rx, len, cs);	break;
	#endif
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:	ret = eRET_NG_NOT_SUPPORT;						break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:	ret = eRET_NG_NOT_SUPPORT;						break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:	ret = eRET_NG_NOT_SUPPORT;						break;
	#endif
		default:			ret = eRET_NG_INVALID_PARAM;					break;
	}
#else
	ret = tunerdrv_cspiTxRxData(tx, rx, len, cs);
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_i2c_write(uint8 addr, uint8 *tx, uint32 len)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:	ret = eRET_NG_NOT_SUPPORT;				break;
	#endif
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:	ret = max2175_i2cTx(addr, tx, len);		break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:	ret = x0tuner_i2cTx(addr, tx, len);		break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:	ret = eRET_NG_NOT_SUPPORT;				break;
	#endif
		default:			ret = eRET_NG_INVALID_PARAM;			break;
	}
#else
	ret = tunerdrv_i2cTx(addr, tx, len);
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_i2c_read(uint8 addr, uint8 *reg, uint32 reglen, uint8* rx, uint32 rxlen)
{
	RET ret = eRET_OK;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:	ret = eRET_NG_NOT_SUPPORT;								break;
	#endif
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:	ret = max2175_i2cRx(addr, reg, reglen, rx, rxlen);		break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:	ret = x0tuner_i2cRx(addr, reg, reglen, rx, rxlen);		break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:	ret = eRET_NG_NOT_SUPPORT;								break;
	#endif
		default:			ret = eRET_NG_INVALID_PARAM;							break;
	}
#else
	ret = tunerdrv_i2cRx(addr, reg, reglen, rx, rxlen);
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

uint32 tcradiohal_getHalVersion(void)
{
	uint32 ret = 0;

	tcradiohal_mutexLock();
	ret = (((uint32)__RADIO_HAL_VER_PREFIX__<<24) | ((uint32)__RADIO_HAL_VER_RELEASE_NUMBER__<<16) |
			((uint32)__RADIO_HAL_VER_MAJOR_NUMBER__<<8) | ((uint32)__RADIO_HAL_VER_MINOR_NUMBER__<<0));

	tcradiohal_mutexUnlock();

	return ret;
}

uint32 tcradiohal_getTunerDrvVersion(void)
{
	uint32 ret = 0;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:	ret = max2175_getVersion();	break;
	#endif
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:	ret = si479xx_getVersion();	break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:	ret = x0tuner_getVersion();	break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:	ret = star_getVersion();	break;
	#endif
		default:										break;
	}
#else
	ret = tunerdrv_getVersion();
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

int32 tcradiohal_getTunerChip(void)
{
	int32 ret;

	tcradiohal_mutexLock();
	ret = curTunerIC;
	tcradiohal_mutexUnlock();

	return ret;
}

float32 tcradiohal_getPreciseIqSampleRate(uint32 ntuner)
{
	float32 ret = 0.0;
	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:
			ret = max2175_getPreciseIqSampleRate(ntuner);
			break;
	#endif
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:
			ret = si479xx_getPreciseIqSampleRate(ntuner);
			break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:
			ret = x0tuner_getPreciseIqSampleRate(ntuner);
			break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:
			ret = star_getPreciseIqSampleRate(ntuner);
			break;
	#endif
		default:
			break;
	}
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

int32 tcradiohal_getIqSampleRate(uint32 ntuner)
{
	int32 ret = eRET_NG_NOT_SUPPORT;
	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:
			ret = max2175_getIqSampleRate(ntuner);
			break;
	#endif
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:
			ret = si479xx_getIqSampleRate(ntuner);
			break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:
			ret = x0tuner_getIqSampleRate(ntuner);
			break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:
			ret = star_getIqSampleRate(ntuner);
			break;
	#endif
		default:
			break;
	}
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

int32 tcradiohal_getIqSamplingBit(uint32 ntuner)
{
	int32 ret = eRET_NG_NOT_SUPPORT;
	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
	switch(curTunerIC) {
	#ifdef USE_M0_TUNER
		case eTUNER_IC_M0:
			ret = max2175_getIqSamplingBit(ntuner);
			break;
	#endif
	#ifdef USE_S0_TUNER
		case eTUNER_IC_S0:
			ret = si479xx_getIqSamplingBit(ntuner);
			break;
	#endif
	#ifdef USE_X0_TUNER
		case eTUNER_IC_X0:
			ret = x0tuner_getIqSamplingBit(ntuner);
			break;
	#endif
	#ifdef USE_T0_TUNER
		case eTUNER_IC_T0:
			ret = star_getIqSamplingBit(ntuner);
			break;
	#endif
		default:
			break;
	}
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_setTunerCommand(uint8 *txdata, uint32 txlen, uint8 *rxdata, uint32 rxlen, uint32 ntuner)
{
	RET ret = eRET_NG_NOT_SUPPORT;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
		switch(curTunerIC) {
	#ifdef USE_M0_TUNER
			case eTUNER_IC_M0:
				break;
	#endif
	#ifdef USE_S0_TUNER
			case eTUNER_IC_S0:
				ret = si479xx_setCommand(txlen, txdata, rxlen, rxdata, ntuner);
				break;
	#endif
	#ifdef USE_X0_TUNER
			case eTUNER_IC_X0:
				break;
	#endif
	#ifdef USE_T0_TUNER
			case eTUNER_IC_T0:
				break;
	#endif
			default:
				break;
		}
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_setDigitalAGC(uint8 fOnOff, int8 target_power, uint8 inc_ms, uint8 dec_ms, uint32 ntuner)
{
	RET ret = eRET_NG_NOT_SUPPORT;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
		switch(curTunerIC) {
	#ifdef USE_M0_TUNER
			case eTUNER_IC_M0:
				break;
	#endif
	#ifdef USE_S0_TUNER
			case eTUNER_IC_S0:
				ret = si479xx_setDigitalAGC(fOnOff, target_power, inc_ms, dec_ms, ntuner);
				break;
	#endif
	#ifdef USE_X0_TUNER
			case eTUNER_IC_X0:
				break;
	#endif
	#ifdef USE_T0_TUNER
			case eTUNER_IC_T0:
				break;
	#endif
			default:
				break;
		}
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_getTunerStatus(uint32 cmd, uint8 *buf, uint32 len, uint32 ntuner)
{
	RET ret = eRET_NG_NOT_SUPPORT;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
			switch(curTunerIC) {
	#ifdef USE_M0_TUNER
				case eTUNER_IC_M0:
					break;
	#endif
	#ifdef USE_S0_TUNER
				case eTUNER_IC_S0:
					ret = si479xx_getStatus(ntuner, cmd, buf, (uint8)len);
					break;
	#endif
	#ifdef USE_X0_TUNER
				case eTUNER_IC_X0:
					break;
	#endif
	#ifdef USE_T0_TUNER
				case eTUNER_IC_T0:
					break;
	#endif
				default:
					break;
			}
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_setIQTestPattern(uint32 fOnOff, uint32 sel)
{
	RET ret = eRET_NG_NOT_SUPPORT;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
			switch(curTunerIC) {
	#ifdef USE_M0_TUNER
				case eTUNER_IC_M0:
					break;
	#endif
	#ifdef USE_S0_TUNER
				case eTUNER_IC_S0:
					ret = si479xx_setIQTestPattern(fOnOff, sel);
					break;
	#endif
	#ifdef USE_X0_TUNER
				case eTUNER_IC_X0:
					break;
	#endif
	#ifdef USE_T0_TUNER
				case eTUNER_IC_T0:
					ret = star_setIQTestPattern(fOnOff, sel);
					break;
	#endif
				default:
					break;
			}
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

RET tcradiohal_setPropertyOfPrimaryTuner(uint8 group, uint8 number, uint8 hdata, uint8 ldata)
{
	RET ret = eRET_NG_NOT_SUPPORT;

	tcradiohal_mutexLock();
#ifdef INCLUDE_ALL_TUNER_DRIVER
		switch(curTunerIC) {
	#ifdef USE_M0_TUNER
			case eTUNER_IC_M0:
				break;
	#endif
	#ifdef USE_S0_TUNER
			case eTUNER_IC_S0:
				ret = si479xx_setPropertyOfPrimaryTuner(group, number, hdata, ldata);
				break;
	#endif
	#ifdef USE_X0_TUNER
			case eTUNER_IC_X0:
				break;
	#endif
	#ifdef USE_T0_TUNER
			case eTUNER_IC_T0:
				break;
	#endif
			default:
				break;
		}
#endif
	tcradiohal_mutexUnlock();

	return ret;
}

void tcradiohal_testReliability(void)
{
	unsigned int test_count = 1;
	while(test_count)
	{
		unsigned int i=0;
		unsigned int channel = 4;
		unsigned int iqbuffer_size = 512;
		unsigned int audbuffer_size = 32;
		unsigned int iqread_size = 4320*2;
		unsigned int audread_size = 2048;
		char * iqsingle_buf = NULL;
		char * audsingle_buf = NULL;

		printf("COUNT: %d\n",test_count);
		test_count++;

		iqsingle_buf = (char *)malloc((sizeof(char))*(channel)*(iqbuffer_size*1024));
		audsingle_buf = (char *)malloc((sizeof(char))*(channel)*(audbuffer_size*1024));

		(*pfnIQ01I2sOpen)();
		(*pfnIQ01I2sSetParams)(channel, 16, tcradiohal_getIqSampleRate(eTUNER_DRV_ID_PRIMARY)/2, iqbuffer_size, (iqbuffer_size*1024)/16);

		(*pfnBlendAudioI2sOpen)();
		(*pfnBlendAudioI2sSetParams)(2, 16, 44100, audbuffer_size, (audbuffer_size*1024)/64);

		(*pfnIQ01I2sStart)();
		(*pfnBlendAudioI2sStart)();

		(*pfnIQ01I2sRead)(iqsingle_buf, iqread_size);
		(pfnBlendAudioI2sRead)(audsingle_buf, audread_size);

		(*pfnIQ01I2sRead)(iqsingle_buf, iqread_size);
		(pfnBlendAudioI2sRead)(audsingle_buf, audread_size);

		(*pfnIQ01I2sRead)(iqsingle_buf, iqread_size);
		(pfnBlendAudioI2sRead)(audsingle_buf, audread_size);

		(*pfnIQ01I2sRead)(iqsingle_buf, iqread_size);
		(pfnBlendAudioI2sRead)(audsingle_buf, audread_size);

		(*pfnIQ01I2sRead)(iqsingle_buf, iqread_size);
		(pfnBlendAudioI2sRead)(audsingle_buf, audread_size);

		(*pfnIQ01I2sRead)(iqsingle_buf, iqread_size);
		(pfnBlendAudioI2sRead)(audsingle_buf, audread_size);

		(*pfnIQ01I2sRead)(iqsingle_buf, iqread_size);
		(pfnBlendAudioI2sRead)(audsingle_buf, audread_size);

		(*pfnIQ01I2sStop)();
		(*pfnBlendAudioI2sStop)();
		(*pfnIQ01I2sClose)();
		(*pfnBlendAudioI2sClose)();

		if(iqsingle_buf != NULL) {
			free(iqsingle_buf);
			iqsingle_buf = NULL;
		}
		if(audsingle_buf != NULL) {
			free(audsingle_buf);
			audsingle_buf = NULL;
		}
		usleep(5);
	}
}

