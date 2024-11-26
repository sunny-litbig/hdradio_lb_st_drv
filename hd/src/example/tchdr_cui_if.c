/*******************************************************************************

*   FileName : tchdr_cui_if.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio CUI Interface functions and definitions

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
/***************************************************
*        Include                                   *
****************************************************/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#if defined(USE_TELECHIPS_EVB) && defined(BUILD_HDR_EXAMPLE_CUI)
#include "tchdr_api.h"
#include "tchdr_callback.h"
#include "tchdr_psd.h"
#include "tchdr_sis.h"
#include "tchdr_sig.h"
#include "tchdr_aas.h"
#include "tchdr_alert.h"
#include "tchdr_cui_if.h"
#include "tchdr_cui_audio.h"

#include "tcradio_types.h"
#include "dev_iq_i2s.h"
#include "dev_blend_audio_i2s.h"
#include "tcradio_hal.h"

/***************************************************
*        Global variable definitions               *
****************************************************/

/***************************************************
*        Imported variable declarations            *
****************************************************/

/***************************************************
*        Imported function declartions             *
****************************************************/

/***************************************************
*       Local preprocessor                         *
****************************************************/
#define	HDR_AUDIO_SAMPLERATE	(44100U)	// hz

#define	HDR_FM_INIT_FREQ		(87900U)	// khz
#define	HDR_FM_START_FREQ		(87900U)	// khz
#define	HDR_FM_END_FREQ			(107900U)	// khz
#define HDR_FM_STEP_FREQ		(200U)		// khz
#define	HDR_MW_INIT_FREQ		(530U)		// khz
#define	HDR_MW_START_FREQ		(530U)		// khz
#define	HDR_MW_END_FREQ			(1710U)		// khz
#define HDR_MW_STEP_FREQ		(10U)		// khz

/***************************************************
*        Local type definitions                    *
****************************************************/

/***************************************************
*        Local constant definitions                *
****************************************************/
static U32 fHdrCuiInit;
static U32 fHdrCuiOpen;
static stTC_HDR_CONF_t stHdrCuiConf;
static stTUNER_DRV_CONFIG_t stTunerConf;

/***************************************************
*        Local function prototypes                 *
****************************************************/
void tchdradiocui_getTcHdrNotificationCallBack(U32 notifyID, const U32 *arg, void* const *pData, S32 errorCode);

/***************************************************
*        function definition                       *
****************************************************/
void HDRCUI_MAIN_LOG(U8 log_type, U8 en_tag, const S8 *format, ...)
{
#ifndef HDRCUI_AVOID_MC2012_RULE_21P6_USING_PRINTF
	char strLog[1024] = {0,};
	va_list arg;

	va_start(arg, format);
	(void)vsprintf(strLog, format, arg);
	va_end(arg);

	if(en_tag != 0U) {
		switch(log_type) {
			case CUI_ERR_LOG:
				CUI_MAIN_LOGE("%s", strLog);
				break;

			case CUI_WRN_LOG:
				CUI_MAIN_LOGW("%s", strLog);
				break;

			case CUI_INF_LOG:
				CUI_MAIN_LOGI("%s", strLog);
				break;

			case CUI_DBG_LOG:
				CUI_MAIN_LOGD("%s", strLog);
				break;

			default:
				CUI_MAIN_LOGMSG("%s", strLog);
				break;
		}
	}
	else {
		(void)printf("%s", strLog);
	}
#endif
}

HDRET tchdradiocui_getConfFromArg(S32 cnt, S8* const sz[])
{
	HDRET hdret = (HDRET)eRET_OK;

	stTUNER_DRV_CONFIG_t *tuner_conf = &stTunerConf;
	stTC_HDR_CONF_t *hdr_conf = &stHdrCuiConf;

	// Set tuner configurations
	tuner_conf->area = eTUNER_DRV_CONF_AREA_NA;
	tuner_conf->initMode = eTUNER_DRV_FM_MODE;
	tuner_conf->initFreq = HDR_FM_INIT_FREQ;
	tuner_conf->numTuners = (U32)eTUNER_DRV_CONF_TYPE_DUAL;		// default: dual tuner
	tuner_conf->fPhaseDiversity = 0;						// disable
	tuner_conf->fIqOut = 1;									// enable
	tuner_conf->audioSamplerate = HDR_AUDIO_SAMPLERATE;		// 44.1Khz
	tuner_conf->fExtAppCtrl = 0;							// disable (not used)
	tuner_conf->sdr = eTUNER_SDR_HD;

	// Set HD Radio configurations
	hdr_conf->iq.samplingBit = 16;							// All Tuner's I/Q sampling bit for HD Radio should be 16bit.
	hdr_conf->iq.maxSampleRate = eTC_HDR_BBSRC_744_KHZ;

	if(cnt == 2) {
	    if ((sz != NULL) && (sz[1] != NULL)) {
			if(SCMP(sz[1], "hd10", 5)==0) {
			//	Logically, single is correct, but there is a dual tuner chip on TC-EVB H/W, so it is commented out.
			//	tuner_conf->numTuners = eTUNER_DRV_CONF_TYPE_SINGLE;
				hdr_conf->hdrType = eTC_HDR_TYPE_HD_1p0;
				HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Selected HD1.0 Radio.\n");
	 		}
			else if(SCMP(sz[1], "hd10mrc", 8)==0) {
				tuner_conf->numTuners = (U32)eTUNER_DRV_CONF_TYPE_DUAL;
				hdr_conf->hdrType = eTC_HDR_TYPE_HD_1p0_MRC;
				HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Selected HD1.0+MRC Radio.\n");
	 		}
			else if(SCMP(sz[1], "hd15", 5)==0) {
				tuner_conf->numTuners = (U32)eTUNER_DRV_CONF_TYPE_DUAL;
				hdr_conf->hdrType = eTC_HDR_TYPE_HD_1p5;
				HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Selected HD1.5 Radio.\n");
	 		}
			else if(SCMP(sz[1], "hd15mrc", 8)==0) {
				tuner_conf->numTuners = (U32)eTUNER_DRV_CONF_TYPE_TRIPLE;
				hdr_conf->hdrType = eTC_HDR_TYPE_HD_1p5_MRC;
				HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Selected HD1.5+MRC Radio.\n");
	 		}
			else if(SCMP(sz[1], "hd15dualmrc", 12)==0) {
				tuner_conf->numTuners = (U32)eTUNER_DRV_CONF_TYPE_QUAD;
				hdr_conf->hdrType = eTC_HDR_TYPE_HD_1p5_DUAL_MRC;
				HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Selected HD1.5+DUALMRC Radio.\n");
	 		}
			else {
				hdret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "Invalid Argument: optarg[%s]\n", sz[1]);
	 		}
	    }
	}
	else {
		hdret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "Invalid Argument.\n");
	}
    return hdret;
}

static void tchdradiocui_setDefaultThreadNicePriority(void)
{
	// Using Nice priority has not been validated yet. Use the default Real-Time priority.
	stTC_HDR_THREAD_PR_t userPriority;
	(void)tchdr_getDefaultThreadNicePriority(eTHREAD_MANAGER, &userPriority);
	(void)tchdr_setThreadPriority(eTHREAD_MANAGER, userPriority);
	(void)tchdr_getDefaultThreadNicePriority(eTHREAD_IQINPUT, &userPriority);
	(void)tchdr_setThreadPriority(eTHREAD_IQINPUT, userPriority);
	(void)tchdr_getDefaultThreadNicePriority(eTHREAD_AUDINPUT, &userPriority);
	(void)tchdr_setThreadPriority(eTHREAD_AUDINPUT, userPriority);
	(void)tchdr_getDefaultThreadNicePriority(eTHREAD_BBINPUT, &userPriority);
	(void)tchdr_setThreadPriority(eTHREAD_BBINPUT, userPriority);
	(void)tchdr_getDefaultThreadNicePriority(eTHREAD_DEMOD, &userPriority);
	(void)tchdr_setThreadPriority(eTHREAD_DEMOD, userPriority);
	(void)tchdr_getDefaultThreadNicePriority(eTHREAD_BLENDING, &userPriority);
	(void)tchdr_setThreadPriority(eTHREAD_BLENDING, userPriority);
	(void)tchdr_getDefaultThreadNicePriority(eTHREAD_AUDOUTPUT, &userPriority);
	(void)tchdr_setThreadPriority(eTHREAD_AUDOUTPUT, userPriority);
	(void)tchdr_getDefaultThreadNicePriority(eTHREAD_CMDPROC, &userPriority);
	(void)tchdr_setThreadPriority(eTHREAD_CMDPROC, userPriority);
	(void)tchdr_getDefaultThreadNicePriority(eTHREAD_LOGGER, &userPriority);
	(void)tchdr_setThreadPriority(eTHREAD_LOGGER, userPriority);
}

static void tchdradiocui_setConfiguration(eTC_HDR_TYPE_t	hdrType)
{
	// I/Q I2S Input Driver
	if((hdrType == eTC_HDR_TYPE_HD_1p0) || (hdrType == eTC_HDR_TYPE_HD_1p0_MRC) || (hdrType == eTC_HDR_TYPE_HD_1p5)) {
		tchdr_configTunerIQ01Driver(&dev_iq01_i2s_open, &dev_iq01_i2s_close, &dev_iq01_i2s_setParameters, &dev_iq01_i2s_start, &dev_iq01_i2s_stop, &dev_iq01_i2s_read);
	}
	else { // for eTC_HDR_TYPE_HD_1p5_MRC
		tchdr_configTunerIQ01Driver(&dev_iq01_i2s_open, &dev_iq01_i2s_close, &dev_iq01_i2s_setParameters, &dev_iq01_i2s_start, &dev_iq01_i2s_stop, &dev_iq01_i2s_read);
		tchdr_configTunerIQ23Driver(&dev_iq23_i2s_open, &dev_iq23_i2s_close, &dev_iq23_i2s_setParameters, &dev_iq23_i2s_start, &dev_iq23_i2s_stop, &dev_iq23_i2s_read);
	}

	// Audio I2S Input Driver
	tchdr_configTunerBlendAudioDriver(&dev_blend_audio_i2s_open, &dev_blend_audio_i2s_close, &dev_blend_audio_i2s_setParameters, &dev_blend_audio_i2s_start, &dev_blend_audio_i2s_stop, &dev_blend_audio_i2s_read);

	// Callback Functions
	tchdr_configTcHdrNotificationCallBack(&tchdradiocui_getTcHdrNotificationCallBack);
	tchdr_configTcHdrAudioQueueCallBack(&tchdradiocui_audioQueueCallBack);
}

HDRET tchdradiocui_init(U32 nice)
{
	RET ret;
	HDRET hdret;
	const stTC_HDR_CONF_t *hdr_conf = &stHdrCuiConf;

	ret = tcradiohal_init();    // Init tuner driver HAL
	if(ret == (RET)eRET_OK) {
		if(nice != 0U) {        // 0: Real-Time, 1: Nice
			tchdradiocui_setDefaultThreadNicePriority();
		}

		tchdradiocui_setConfiguration(hdr_conf->hdrType);

		hdret = tchdr_init(*hdr_conf);
		if(hdret == (HDRET)eTC_HDR_RET_OK) {
			hdret = tchdradiocui_audio_init();
		}
	}
	else {
		hdret = (HDRET)eTC_HDR_RET_NG_INIT;
	}

	if(hdret == (HDRET)eTC_HDR_RET_OK) {
		fHdrCuiInit = 1U;
	} else {
		fHdrCuiInit = 0U;
		HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d] Failed to open HD Radio\n", __func__, __LINE__);
	}

	return hdret;
}

static void tchdradiocui_releaseConfiguredFunctions(void)
{
	tchdr_configTunerIQ01Driver(NULL, NULL, NULL, NULL, NULL, NULL);
	tchdr_configTunerIQ23Driver(NULL, NULL, NULL, NULL, NULL, NULL);
	tchdr_configTunerBlendAudioDriver(NULL, NULL, NULL, NULL, NULL, NULL);
	tchdr_configTcHdrNotificationCallBack(NULL);
	tchdr_configTcHdrAudioQueueCallBack(NULL);
}

HDRET tchdradiocui_deinit(void)
{
	HDRET hdret;

	if(fHdrCuiInit == 1U) {
		(void)tchdradiocui_audio_deinit();
		(void)tcradiohal_deinit();            // Deinit tuner driver HAL. Replace with your tuner API.

		tchdradiocui_releaseConfiguredFunctions();
		hdret = tchdr_deinit();

		if(hdret == (HDRET)eTC_HDR_RET_OK) {
			fHdrCuiInit = 0U;
		} else {
			HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d] Failed to deinitialize HD Radio\n", __func__, __LINE__);
		}
	}
	else {
		hdret = (HDRET)eTC_HDR_RET_NG_NOT_YET_INIT;
		HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d] Aready HD Radio was deinitialized\n", __func__, __LINE__);
	}

	return hdret;
}

HDRET tchdradiocui_open(void)
{
	HDRET hdret;
	RET ret;
	float64 out_hz = (float64)0.0;
	const stTUNER_DRV_CONFIG_t *tuner_conf = &stTunerConf;
	const stTC_HDR_CONF_t *hdr_conf = &stHdrCuiConf;
	stTC_HDR_TUNE_INFO_t tuneInfo;

	if(fHdrCuiOpen == (U32)0) {
		ret = tcradiohal_open(*tuner_conf);    // Open tuner driver HAL. Replace with your tuner API.
		if((ret == (RET)eRET_OK) || (ret == (RET)eRET_NG_ALREADY_OPEN)) {
			(void)memset((void*)&tuneInfo, (S32)0x00, sizeof(stTC_HDR_TUNE_INFO_t));

			if(tcradiohal_getTunerChip() == eTUNER_IC_S0) {
				tchdr_setAudioResamplerSlips(0U, -0.0522655, &out_hz);
			}

			if(tuner_conf->initMode == eTUNER_DRV_FM_MODE) {
				tuneInfo.mainTuner.band = eTC_HDR_FM_BAND;
				tuneInfo.mainTuner.freq = tuner_conf->initFreq;
				tuneInfo.mainTuner.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)eTUNER_DRV_ID_PRIMARY);
				if(hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p5_DUAL_MRC) {
					tuneInfo.bsmrcTuner.band = eTC_HDR_FM_BAND;
					tuneInfo.bsmrcTuner.freq = tuner_conf->initFreq;
					tuneInfo.bsmrcTuner.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)eTUNER_DRV_ID_QUATERNARY);
				}
				if((hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p0_MRC) || (hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p5_MRC) || (hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p5_DUAL_MRC)) {
					tuneInfo.mrcTuner.band = eTC_HDR_FM_BAND;
					tuneInfo.mrcTuner.freq = tuner_conf->initFreq;
					tuneInfo.mrcTuner.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)eTUNER_DRV_ID_SECONDARY);
				}
				if(hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p5) {
					tuneInfo.bsTuner.band = eTC_HDR_FM_BAND;
					tuneInfo.bsTuner.freq = tuner_conf->initFreq;
					tuneInfo.bsTuner.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)eTUNER_DRV_ID_SECONDARY);
				}
				if((hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p5_MRC) || (hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p5_DUAL_MRC)) {
					tuneInfo.bsTuner.band = eTC_HDR_FM_BAND;
					tuneInfo.bsTuner.freq = tuner_conf->initFreq;
					tuneInfo.bsTuner.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)eTUNER_DRV_ID_TERTIARY);
				}
				hdret = tchdr_open(tuneInfo);
			}
			else if(tuner_conf->initMode == eTUNER_DRV_AM_MODE) {
				tuneInfo.mainTuner.band = eTC_HDR_AM_BAND;
				tuneInfo.mainTuner.freq = tuner_conf->initFreq;
				tuneInfo.mainTuner.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)eTUNER_DRV_ID_PRIMARY);
				if(hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p5_DUAL_MRC) {
					tuneInfo.bsmrcTuner.band = eTC_HDR_AM_BAND;
					tuneInfo.bsmrcTuner.freq = tuner_conf->initFreq;
					tuneInfo.bsmrcTuner.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)eTUNER_DRV_ID_QUATERNARY);
				}
				if((hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p0_MRC) || (hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p5_MRC) || (hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p5_DUAL_MRC)) {
					// In AM, MRC does not operate, but tune information should be input.
					tuneInfo.mrcTuner.band = eTC_HDR_AM_BAND;
					tuneInfo.mrcTuner.freq = tuner_conf->initFreq;
					tuneInfo.mrcTuner.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)eTUNER_DRV_ID_SECONDARY);
				}
				if(hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p5) {
					tuneInfo.bsTuner.band = eTC_HDR_AM_BAND;
					tuneInfo.bsTuner.freq = tuner_conf->initFreq;
					tuneInfo.bsTuner.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)eTUNER_DRV_ID_SECONDARY);
				}
				if((hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p5_MRC) || (hdr_conf->hdrType == eTC_HDR_TYPE_HD_1p5_DUAL_MRC)) {
					tuneInfo.bsTuner.band = eTC_HDR_AM_BAND;
					tuneInfo.bsTuner.freq = tuner_conf->initFreq;
					tuneInfo.bsTuner.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)eTUNER_DRV_ID_TERTIARY);
				}
				hdret = tchdr_open(tuneInfo);
			}
			else {
				hdret = (HDRET)eTC_HDR_RET_NG_INVALID_BAND;
			}
		}
		else {
			hdret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
	}
	else {
		hdret = (HDRET)eTC_HDR_RET_NG_ALREADY_OPEN;
	}

	if(hdret != (HDRET)eTC_HDR_RET_OK) {
		if(hdret == (HDRET)eTC_HDR_RET_NG_ALREADY_OPEN) {
			HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d] Already, HD Radio opened.\n", __func__, __LINE__);
			hdret = (HDRET)eTC_HDR_RET_OK;
		}
		else {
			HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d] Failed to open HD Radio\n", __func__, __LINE__);
		}
	}
	else {
		fHdrCuiOpen = 1;
	}

	return hdret;
}

HDRET tchdradiocui_close(void)
{
	HDRET hdret;

	if(fHdrCuiOpen == (U32)1) {
		(void)tchdradiocui_audio_close();
		hdret = tchdr_close();
		(void)tcradiohal_close();            // Close tuner driver HAL. Replace with your tuner API.
		if(hdret == (HDRET)eTC_HDR_RET_OK) {
			fHdrCuiOpen = 0;
		}
		else {
			HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d] Failed to close HD Radio\n", __func__, __LINE__);
		}
	}
	else {
		hdret = (HDRET)eTC_HDR_RET_NG_ALREADY_CLOSE;
		HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d] Aready HD Radio closed\n", __func__, __LINE__);
	}

	return hdret;
}

static HDRET tchdradiocui_checkTuneFreq(eTC_HDR_BAND_t hdr_band, U32 freq)
{
	HDRET hdret = (HDRET)eTC_HDR_RET_OK;

	// check hdr_band and frequency
	if(hdr_band == eTC_HDR_FM_BAND) {
		if((freq < (U32)HDR_FM_START_FREQ) || (freq > (U32)HDR_FM_END_FREQ) || (((freq - (U32)HDR_FM_START_FREQ) % (U32)HDR_FM_STEP_FREQ) != (U32)0)) {
			hdret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
	}
	else if(hdr_band == eTC_HDR_AM_BAND) {
		if((freq < (U32)HDR_MW_START_FREQ) || (freq > (U32)HDR_MW_END_FREQ) || (((freq - (U32)HDR_MW_START_FREQ) % (U32)HDR_MW_STEP_FREQ) != (U32)0)) {
			hdret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
	}
	else {
		hdret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
	}

	return hdret;
}

HDRET tchdradiocui_setReacquire(eTC_HDR_ID_t hdr_id)
{
	HDRET hdret = (HDRET)eTC_HDR_RET_OK;

	hdret = tchdr_setReacquire(hdr_id);

	return hdret;
}

HDRET tchdradiocui_setTune(eTC_HDR_BAND_t hdr_band, U32 freq, eTC_HDR_ID_t hdr_id)
{
	HDRET hdret = (HDRET)eTC_HDR_RET_OK;
	U32 ntuner;

	hdret = tchdradiocui_checkTuneFreq(hdr_band, freq);

	// check hdr_id
	if(hdret == (HDRET)eTC_HDR_RET_OK) {
		if(hdr_id == eTC_HDR_ID_MAIN) {
			ntuner = (U32)eTUNER_DRV_ID_PRIMARY;
		}
		else if(hdr_id == eTC_HDR_ID_MRC) {
			if((stHdrCuiConf.hdrType == eTC_HDR_TYPE_HD_1p0_MRC) || (stHdrCuiConf.hdrType == eTC_HDR_TYPE_HD_1p5_MRC) || (stHdrCuiConf.hdrType == eTC_HDR_TYPE_HD_1p5_DUAL_MRC)) {
				ntuner = (U32)eTUNER_DRV_ID_SECONDARY;
			}
			else {
				hdret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else if(hdr_id == eTC_HDR_ID_BS) {
			if(stHdrCuiConf.hdrType == eTC_HDR_TYPE_HD_1p5) {
				ntuner = (U32)eTUNER_DRV_ID_SECONDARY;
			}
			else if(stHdrCuiConf.hdrType == eTC_HDR_TYPE_HD_1p5_MRC) {
				ntuner = (U32)eTUNER_DRV_ID_TERTIARY;
			}
			else {
				hdret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else if(hdr_id == eTC_HDR_ID_BS_MRC) {
			if(stHdrCuiConf.hdrType == eTC_HDR_TYPE_HD_1p5_DUAL_MRC) {
				ntuner = (U32)eTUNER_DRV_ID_QUATERNARY;
			}
			else {
				hdret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else {
			hdret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
	}

	if(hdret == (HDRET)eTC_HDR_RET_OK) {
		hdret = tchdr_cb_setTune(hdr_band, freq, ntuner);    // Set primary tuner band and frequency
		if(hdret == (HDRET)eTC_HDR_RET_OK) {
			stTC_HDR_TUNE_TO_t tuneTo;
			tuneTo.band = hdr_band;
			tuneTo.freq = freq;
			tuneTo.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)ntuner);    // Get I/Q sample rate from primary tuner
			hdret = tchdr_setTune(hdr_id, tuneTo);
		}
	}

	// For MRC
	if(hdret == (HDRET)eTC_HDR_RET_OK) {
		if(hdr_id == eTC_HDR_ID_MAIN) {
			if((stHdrCuiConf.hdrType == eTC_HDR_TYPE_HD_1p0_MRC) || (stHdrCuiConf.hdrType == eTC_HDR_TYPE_HD_1p5_MRC)) {
				hdret = tchdr_cb_setTune(hdr_band, freq, (U32)eTUNER_DRV_ID_SECONDARY);    // Set MRC tuner band and frequency
				if(hdret == (HDRET)eTC_HDR_RET_OK) {
					stTC_HDR_TUNE_TO_t tuneTo;
					tuneTo.band = hdr_band;
					tuneTo.freq = freq;
					tuneTo.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)eTUNER_DRV_ID_SECONDARY);    // Get I/Q sample rate from MRC tuner
					hdret = tchdr_setTune(eTC_HDR_ID_MRC, tuneTo);
				}
			}
		}
		if(hdr_id == eTC_HDR_ID_BS) {
			if(stHdrCuiConf.hdrType == eTC_HDR_TYPE_HD_1p5_DUAL_MRC) {
				hdret = tchdr_cb_setTune(hdr_band, freq, (U32)eTUNER_DRV_ID_QUATERNARY);    // Set BS MRC tuner band and frequency
				if(hdret == (HDRET)eTC_HDR_RET_OK) {
					stTC_HDR_TUNE_TO_t tuneTo;
					tuneTo.band = hdr_band;
					tuneTo.freq = freq;
					tuneTo.iqsamplerate = (eTC_HDR_BBSRC_RATE_t)tchdr_cb_getIqSampleRate((U32)eTUNER_DRV_ID_QUATERNARY);    // Get I/Q sample rate from MRC tuner
					hdret = tchdr_setTune(eTC_HDR_ID_BS_MRC, tuneTo);
				}
			}
		}
	}

	return hdret;
}

HDRET tchdradiocui_setAudioDevice(U32 audio_samplerate, U32 OnOff)
{
	HDRET hdret;

	if(OnOff == (U32)0) {
		hdret = tchdradiocui_audio_close();
	}
	else {
		if(fHdrCuiOpen == (U32)1) {
			hdret = tchdradiocui_audio_open(audio_samplerate);
		}
		else {
			hdret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
		}
	}

	return hdret;
}

HDRET tchdradiocui_createThread(pthread_t *pHandle, void*(*pfThread)(void *arg))
{
	S32 ret;
	HDRET hdret = (HDRET)eTC_HDR_RET_OK;
    pthread_attr_t thread_attr;
	S8 errBuf[128]={0,};

    if((pHandle == NULL) || (pfThread == NULL)) {
		HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d]: An error occurred: The arguments is null.\n", __func__, __LINE__);
		hdret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
    }

	if(hdret == (HDRET)eTC_HDR_RET_OK){
		ret = pthread_attr_init(&thread_attr);
		if(ret != 0){
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			hdret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
    }

	if(hdret == (HDRET)eTC_HDR_RET_OK){
		ret = pthread_attr_setdetachstate(&thread_attr, (S32)PTHREAD_CREATE_DETACHED);
		if(ret != 0){
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			hdret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
    }

    if(hdret == (HDRET)eTC_HDR_RET_OK){
		ret = pthread_create(pHandle, &thread_attr, pfThread, NULL);
		if(ret != 0){
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			hdret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
    }

	if(hdret == (HDRET)eTC_HDR_RET_OK) {
		ret = pthread_attr_destroy(&thread_attr);
		if(ret != 0){
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			hdret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
	}

    return hdret;
}

void tchdradiocui_getTcHdrNotificationCallBack(U32 notifyID, const U32 *arg, void* const *pData, S32 errorCode)
{
	U32 i;
	U32 notiID = notifyID;

	if(arg == NULL) {
		HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d]: arg is NULL! notifyID[%u]\n", __func__, __LINE__, notifyID);
		notiID = (U32)eTC_HDR_NOTIFY_NULL;
	}

	switch(notiID) {
		case (U32)eTC_HDR_NOTIFY_OPEN:
			if(errorCode != (HDRET)eTC_HDR_RET_OK) {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "eTC_HDR_NOTIFY_OPEN : Error[%d]!\n", errorCode);
			}
			else {
				HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "eTC_HDR_NOTIFY_OPEN : Done!\n");
				(void)tchdr_setProgram(eTC_HDR_ID_MAIN, eTC_HDR_PROGRAM_HD1);
				(void)tchdr_enablePsdNotification(eTC_HDR_ID_MAIN, 0x01U, PSD_BITMASK, 1U);
				(void)tchdr_enableSisNotification(eTC_HDR_ID_MAIN, (U32)eBITMASK_SIS_SHORT_NAME, 1U);
				(void)tchdradiocui_setAudioDevice(44100U, 1U);
			}
			break;

		case (U32)eTC_HDR_NOTIFY_AUDIO_MODE:
			if(errorCode != (HDRET)eTC_HDR_RET_OK) {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "eTC_HDR_NOTIFY_AUDIO_MODE : Error[%d]!\n", errorCode);
			}
			else {
				HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "eTC_HDR_NOTIFY_AUDIO_MODE : [%u] Done!\n", arg[0]);
			}
			break;

		case (U32)eTC_HDR_NOTIFY_TUNE:
			if(errorCode != (HDRET)eTC_HDR_RET_OK) {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "eTC_HDR_NOTIFY_TUNE : Error[%d]!\n", errorCode);
			}
			else {
				HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "eTC_HDR_NOTIFY_TUNE : Done! id[%u], fChgBand[%u], fChgFreq[%u], fChgSR[%u]\n", arg[0], arg[1], arg[2], arg[3]);
				if(arg[0] == (U32)eTC_HDR_ID_MAIN) {
					(void)tchdr_enablePsdNotification(eTC_HDR_ID_MAIN, 0x01U, PSD_BITMASK, 1U);
					(void)tchdr_enableSisNotification(eTC_HDR_ID_MAIN, (U32)eBITMASK_SIS_SHORT_NAME, 1U);
				}
				else {
					if(arg[0] == (U32)eTC_HDR_ID_BS) {
						(void)tchdr_enablePsdNotification(eTC_HDR_ID_BS, 0x01U, PSD_BITMASK, 1U);
						(void)tchdr_enableSisNotification(eTC_HDR_ID_BS, (U32)eBITMASK_SIS_SHORT_NAME, 1U);
					}
				}
			}
			break;

		case (U32)eTC_HDR_NOTIFY_MUTE:
			if(errorCode != (HDRET)eTC_HDR_RET_OK) {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "eTC_HDR_NOTIFY_MUTE : Error[%d]!\n", errorCode);
			}
			else {
				HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "eTC_HDR_NOTIFY_MUTE : %s!\n", (arg[0]==0U) ? "Mute off" : "Mute on");
			}
			break;

		case (U32)eTC_HDR_NOTIFY_AUDIO_CTRL:
			if(errorCode != (HDRET)eTC_HDR_RET_OK) {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "eTC_HDR_NOTIFY_AUDIO_CTRL : Error[%d]!\n", errorCode);
			}
			else {
				HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "eTC_HDR_NOTIFY_AUDIO_CTRL : %s!\n", (arg[0]==0U) ? "Stop" : "Start");
			}
			break;

		case (U32)eTC_HDR_NOTIFY_PSD:
			if(errorCode != (HDRET)eTC_HDR_RET_OK) {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "eTC_HDR_NOTIFY_PSD : Error[%d]!\n", errorCode);
			}
			else {
				if((pData != NULL) && (*pData != NULL)) {
					U32 type = 0;
					stTC_HDR_PSD_t hdrPsd;
					(void)memcpy((void*)&hdrPsd, *pData, sizeof(stTC_HDR_PSD_t));
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "----------------------------------------------- PSD (META)\n");
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "eTC_HDR_NOTIFY_PSD : Done! HDR ID[%u], PN[%02u] \n", arg[0], arg[1]);

					const stTC_HDR_PSD_XHDR_FRAME_t *tXhdrData=NULL;
					const stTC_HDR_PSD_FORM_t *tPsdData=NULL;

					for(type=(U32)eTC_HDR_PSD_TITLE; type <= (U32)eTC_HDR_PSD_COMMENT_ACTUAL_TEXT; type++) {
						switch(type) {
							case (U32)eTC_HDR_PSD_TITLE:
								tPsdData = &hdrPsd.title;
								break;
							case (U32)eTC_HDR_PSD_ARTIST:
								tPsdData = &hdrPsd.artist;
								break;
							case (U32)eTC_HDR_PSD_ALBUM:
								tPsdData = &hdrPsd.album;
								break;
							case (U32)eTC_HDR_PSD_GENRE:
								tPsdData = &hdrPsd.genre;
								break;
							case (U32)eTC_HDR_PSD_COMMENT_LANGUAGE:
								tPsdData = &hdrPsd.comment.language;
								break;
							case (U32)eTC_HDR_PSD_COMMENT_SHORT_CONTENT:
								tPsdData = &hdrPsd.comment.shortContent;
								break;
							default:
								// eTC_HDR_PSD_COMMENT_ACTUAL_TEXT
								tPsdData = &hdrPsd.comment.actualText;
								break;
						}

						if(tPsdData->len > (U32)0) {
							HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "PsdType[%u], CharType[0x%02x], Length[%u]\n", type, (U8)tPsdData->charType, tPsdData->len);
							HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "");

							for(i=0; i<tPsdData->len; i++) {
								HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "%c", tPsdData->data[i]);
							}
							HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "\n");
						}
					}

					tPsdData = &hdrPsd.commercial.receivedAs;
					if(tPsdData->len > (U32)0) {
						HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "--------------------------------------------------------- PSD (COMMERCIAL)\n");
						HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "PsdType[Received AS], CharType[0x%02x], Length[%u], AS[0x%02x]\n",
						(U8)tPsdData->charType, tPsdData->len, tPsdData->data[0]);

						S8 As = tPsdData->data[0];

						if(As == (S8)0x00) {
							for(type=(U32)eTC_HDR_PSD_COMMERCIAL_PRICE_STRING; type <= (U32)eTC_HDR_PSD_COMMERCIAL_DESCRIPTION; type++) {
								switch(type) {
									case (U32)eTC_HDR_PSD_COMMERCIAL_PRICE_STRING:
										tPsdData = &hdrPsd.commercial.priceString;
										break;
									case (U32)eTC_HDR_PSD_COMMERCIAL_VALID_UNTIL:
										tPsdData = &hdrPsd.commercial.validUntil;
										break;
									case (U32)eTC_HDR_PSD_COMMERCIAL_CONTACT_URL:
										tPsdData = &hdrPsd.commercial.contactURL;
										break;
									case (U32)eTC_HDR_PSD_COMMERCIAL_SELLER_NAME:
										tPsdData = &hdrPsd.commercial.sellerName;
										break;
									case (U32)eTC_HDR_PSD_COMMERCIAL_DESCRIPTION:
										tPsdData = &hdrPsd.commercial.description;
										break;
									default:
										(void)(type);
										break;
								}

								if((tPsdData->len > (U32)0) && (type != (U32)eTC_HDR_PSD_COMMERCIAL_RECEIVED_AS)) {
									HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "PsdType[%u], CharType[0x%02x], Length[%u]\n", type, (U8)tPsdData->charType, tPsdData->len);
									HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "");

									for(i=0; i<tPsdData->len; i++) {
										HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "%c", tPsdData->data[i]);
									}
									HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "\n");
								}
							}
						}else{
							// PSD Spec: "0x01 ~ 0xFF" Receivers shall not display the Commercial frame.
						}
					}

					tXhdrData = &hdrPsd.xhdr;
					if(tXhdrData->numParams > (U32)0) {
						U32 num;
						U32 idx;
						HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "--------------------------------------------------------- PSD (XHDR)\n");
						HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "XHDR PsdType[%u], numParams[%u], Program[%u]\n",
							eTC_HDR_PSD_XHDR, tXhdrData->numParams,tXhdrData->program);

						for(num = 0; num < tXhdrData->numParams; num++) {
							if(tXhdrData->params[num].param_id == 0U) {
								HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "XHDR ParamID[0x%02X], MIME[0x%X], LotID[%u]\n", tXhdrData->params[num].param_id, tXhdrData->mime_hash, tXhdrData->params[num].lot_id);
							} else if(tXhdrData->params[num].param_id == 1U) {
								HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "XHDR ParamID[0x%02X], MIME[0x%X]\n", tXhdrData->params[num].param_id, tXhdrData->mime_hash);
							} else if(tXhdrData->params[num].param_id == 2U) {
								HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "XHDR ParamID[0x%02X], MIME[0x%X]\n", tXhdrData->params[num].param_id, tXhdrData->mime_hash);
							} else {
								HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "XHDR ParamID[0x%02X], MIME[0x%X], Value[0x", tXhdrData->params[num].param_id, tXhdrData->mime_hash);
								for(idx = 0; idx < tXhdrData->params[num].length; idx++) {
									HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "%X", tXhdrData->params[num].value[idx]);
								}
								HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "]\n");
							}
						}
					}
				}
				else {
					(void)(notifyID);
				}
			}
			break;

		case (U32)eTC_HDR_NOTIFY_SIS:
			if(errorCode != (HDRET)eTC_HDR_RET_OK) {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "eTC_HDR_NOTIFY_SIS : Error[%d]!\n", errorCode);
			}
			else {
				if((pData != NULL) && (*pData != NULL)) {
					stTC_HDR_SIS_t sisData;
					(void)memcpy((void*)&sisData, *pData, sizeof(stTC_HDR_SIS_t));

					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "--------------------------------------------------------- INFO (STATION)\n");
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Station Information Service Data: HDR ID[%u]\n", arg[0]);

					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Station ID[%04xh]\n", sisData.stationID.all);
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Short Name Len[%u]\n", sisData.shortName.len);
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Short Name : ");
					if(sisData.shortName.len > (U32)0) {
						for(i=0; i<sisData.shortName.len; i++) {
							HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "%c", sisData.shortName.text[i]);
						}
					}
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "\n");

					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Universal Name Len[%u], charType[%u], appendFm[%u]\n", sisData.universalName.len, (U8)sisData.universalName.charType, sisData.universalName.appendFm);
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Universal Name : ");
					if(sisData.universalName.len > (U32)0) {
						for(i=0; i<sisData.universalName.len; i++) {
							HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "%c", sisData.universalName.text[i]);
						}
					}
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "\n");

					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Slogan Len[%u], charType[%u]\n", sisData.slogan.len, (U8)sisData.slogan.charType);
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Slogan : ");
					if(sisData.slogan.len > (U32)0) {
						for(i=0; i<sisData.slogan.len; i++) {
							HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "%c", sisData.slogan.text[i]);
						}
					}
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "\n");
				}
				else {
					(void)(notifyID);
				}
			}
			break;

		case (U32)eTC_HDR_NOTIFY_SIGNAL_STATUS:
			if(errorCode != (HDRET)eTC_HDR_RET_OK) {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "eTC_HDR_NOTIFY_SIGNAL_STATUS : Error[%d]!\n", errorCode);
			}
			else {
				stTC_HDR_SIGNAL_STATUS_t hdrSigStatus;
				if((pData != NULL) && (*pData != NULL)) {
					U32 hd_signal=0;
					U32 hd_audio=0;
					U32 hd_sis=0;
					U32 hd_sisok=0;

					(void)memcpy((void*)&hdrSigStatus, *pData, sizeof(stTC_HDR_SIGNAL_STATUS_t));
					if((hdrSigStatus.acqStatus & (U32)eBITMASK_SIGNAL_STATUS_HD_SIGNAL) > (U32)0) {
						hd_signal = 1;
					}
					if((hdrSigStatus.acqStatus & (U32)eBITMASK_SIGNAL_STATUS_HD_AUDIO) > (U32)0) {
						hd_audio = 1;
					}
					if((hdrSigStatus.acqStatus & (U32)eBITMASK_SIGNAL_STATUS_SIS) > (U32)0) {
						hd_sis = 1;
					}
					if((hdrSigStatus.acqStatus & (U32)eBITMASK_SIGNAL_STATUS_SIS_OK) > (U32)0) {
						hd_sisok = 1;
					}
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "eTC_HDR_NOTIFY_SIGNAL_STATUS :  ID[%u], ALL[%02xh], HDSIG[%u], HDAUD[%u], SIS[%u], SISOK[%u] Done!\n",
						hdrSigStatus.hdrID, hdrSigStatus.acqStatus, hd_signal, hd_audio, hd_sis, hd_sisok);
				}
				else {
					(void)(notifyID);
				}
			}
			break;

		case (U32)eTC_HDR_NOTIFY_PTY:
			if(errorCode != (HDRET)eTC_HDR_RET_OK) {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "eTC_HDR_NOTIFY_PTY : Error[%d]!\n", errorCode);
			}
			else {
				HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "eTC_HDR_NOTIFY_PTY : ID[%u], Current Program[%u], PTY[%u]\n", arg[0], arg[1], arg[2]);
			}
			break;

		case (U32)eTC_HDR_NOTIFY_LOT:
			if(errorCode != (HDRET)eTC_HDR_RET_OK) {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "eTC_HDR_NOTIFY_LOT : not found [%d]!\n", errorCode);
			}
			else {
				if((pData != NULL) && (*pData != NULL)) {
					U32 loop = 0;
					stTC_HDR_LOT_t lotData;

					(void)memcpy((void*)&lotData, *pData, sizeof(stTC_HDR_LOT_t));
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "--------------------------------------------------------- LOT\n");
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "eTC_HDR_NOTIFY_LOT : Done! Service Number[%u], app_mime_hash[0x%X], mimhash[0x%X]\n",
						lotData.service_number, lotData.app_mime_hash, lotData.header.mime_hash);

					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "HdrID [%u], PortNum[%u], LotID[%u], ByteWritten[%u], Service Number[%u], Dtime[%u], ProNum[%u]\n",
						arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6]);

					if(lotData.header.filename_length > (U8)0) {
						HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "* File Name: ");
						for(loop=0; loop<lotData.header.filename_length; loop++) {
							HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "%c", lotData.header.filename[loop]);
						}
						HDRCUI_MAIN_LOG(CUI_DBG_LOG, 0U, "\n");
					}else{
						HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "* Empty\n");
					}

					if(lotData.app_mime_hash == TC_HDR_SIG_APP_MIME_HASH_ALBUM_ART){
						HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "* APP MIME : Primary Image\n");
					}else{
						HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "* APP MIME : Station Logo\n");
					}

					if(lotData.header.mime_hash == TC_HDR_AAS_LOT_MIME_HASH_IMAGE_PNG){
						HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "* FILE MIME : Ext ( PNG )\n");
					}else if(lotData.header.mime_hash == TC_HDR_AAS_LOT_MIME_HASH_IMAGE_JPEG){
						HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "* FILE MIME : Ext ( JPG )\n");
					}else {
						HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "* FILE MIME : Ext ( Other )\n");
					}

					/* The image data is in "lotData.body" and the size is "lotData.body_bytes_written". */
				}
				else {
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "eTC_HDR_NOTIFY_LOT : Done! HID[%u], Program Bitmask[%u], On/Off[%u]!\n", arg[0],arg[1],arg[2]);
				}
			}
			break;

		case (U32)eTC_HDR_NOTIFY_ALERT:
			if(errorCode != (HDRET)eTC_HDR_RET_OK) {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "eTC_HDR_NOTIFY_ALERT : Error[%d]!\n", errorCode);
			}
			else {
				if((pData != NULL) && (*pData != NULL)) {
					U8 alert_message[TC_HDR_MAX_ALERT_PAYLOAD_LENGTH] = {0,};
					stTC_HDR_ALERT_MESSAGE_t msg;

					(void)memcpy((void*)&msg, *pData, sizeof(stTC_HDR_ALERT_MESSAGE_t));
					if(msg.text_message != NULL) {
						(void)memcpy((void*)alert_message, msg.text_message, msg.text_length);
					}

					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "--------------------------------------------------------- EA\n");
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "eTC_HDR_NOTIFY_ALERT : Done! HID[%u], text len[%u], payload len[%u]\n",
						arg[0], msg.text_length, msg.payload_length);
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Alert Message	: %s\n", alert_message);
				}
				else {
					(void)(notifyID);
				}
			}
			break;

		default:
			(void)(notifyID);
			break;
	}
}

#endif
