/*******************************************************************************

*   FileName : tcradio_service.c

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
/***************************************************
*		Include 			   					*
****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "tcradio_types.h"
#include "tcradio_api.h"
#include "tcradio_utils.h"
#include "tcradio_thread.h"
#include "tcradio_peri_config.h"
#include "tcradio_hal.h"
#include "tcradio_service.h"
#include "tcradio_sound.h"
#include "tcradio_rds_api.h"
#include "tcradio_rds_def.h"
#include "tcradio_msgq.h"
#include "tcradio_bg.h"
#ifdef USE_HDRADIO
#include "tcradio_hdr_if.h"
#endif

/***************************************************
*        Global variable definitions               *
****************************************************/
uint32 radioArea[][5] = {
	{ 87500,	108000,	100	},		// FM 87.5 ~ 108.0 MHz 	(100k step)
	{ 531,		1602,	9	},		// MW 531 ~ 1629 kHz	(9k step)
	{ 144,		290,	1	},		// LW 144 ~ 290 kHz		(1k step)
	{ 2950,		7000,  	5 	},		// SW1 2950 ~ 7000KHz	(5k step)
//	{ 9500,		18135,	5	},		// SW2 9500 ~ 18135KHz	(5k step)
	{ 6500,		7400,	30	}		// OIRT 65.0 ~ 74.0 MHz	(30k step)
};

static uint32 *dabFreqList;

stRADIO_SERVICE_t stRadioService;
stRADIO_CONFIG_t stTunerConfig;

static pthread_t serviceMainThreadID = (pthread_t)NULL;

uint32 uiSeek5msCnt = 0;
uint32 uiCheckPiCnt = 0;
stRADIO_LIST_t stRadioList[_MaxPresetNum];

uint32 fStationListTx = 0;
uint32 uiStationListCounter = 0;
stRADIO_LIST_t _stStationList[_AmFmDBSize];

stMsgBuf_t stRadioRcvMsgQ;

uint32 scanPiList[128];
stRDS_SCAN_t stScanPiFreq;

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/

/***************************************************
*             Local preprocessor                   *
****************************************************/

/***************************************************
*           Local constant definitions              *
****************************************************/

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/
void tcradioservice_initVariable(void);
RET tcradioservice_getFreqCounter(eRADIO_MOD_MODE_t mod_mode, uint32 *freq, uint8 dir);
void tcradioservice_stopSeek(uint32 retune);
int32 tcradioservice_checkSignalQualityStatus(stRADIO_QUALITY_t qdata);
int32 tcradioservice_precheckSignalQuality(stRADIO_QUALITY_t qdata);
int32 tcradioservice_checkSignalQuality(stRADIO_QUALITY_t qdata);
int32 tcradioservice_isFmBand(void);
int32 tcradioservice_isAmBand(void);
int32 tcradioservice_isDabBand(void);
int32 tcradioservice_isSeeking(void);
void tcradioservice_saveAllPresetMemory(void);
#if 0
void tcradioservice_sortPresetMemory(void);
void tcradioservice_loadPresetMemory(uint8 pnum);
#endif
static void tcradioservice_eventHandler(void);
void tcradioservice_initSeek(eRADIO_SEEK_MODE_t nextSeekMode);
void tcradioservice_makeStationList(void);
void *tcradioservice_mainThread(void *arg);
eRADIO_STS_t tcradioservice_seekProcess(void);
static void tcradioservice_seekHandler(void);
int32 tcradioservice_checkValidFreqAccordingToConfig(eRADIO_MOD_MODE_t mod_mode, uint32 freq);
RET tcradioservice_checkValidBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 start_freq, uint32  end_freq, uint32 step);
void tcradioservice_appNotifyHandler(void);
void tcradioservice_soundMessageParser(stMsgBuf_t *pstMsg);
void tcradioservice_appMessageParser(stMsgBuf_t *pstMsg);
void tcradioservice_callbackAppFunction(stMsgBuf_t *pstMsg);
int32 tcradioservice_findBestStation(void);

/***************************************************
*			function definition				*
****************************************************/
int32 tcradioservice_isHdRadio(void)
{
	int32 ret = NO;
#ifdef USE_HDRADIO
	if(stTunerConfig.sdr == eRADIO_SDR_HD) {
		ret = YES;
	}
#endif
	return ret;
}

int32 tcradioservice_is2ndTunerMRC(void)
{
	int32 ret = NO;
#ifdef USE_HDRADIO
	if(tcradioservice_isHdRadio() == YES) {
		if((stTunerConfig.hdType == eRADIO_HD_TYPE_HD1p0_MRC) || (stTunerConfig.hdType == eRADIO_HD_TYPE_HD1p5_MRC) ||
			(stTunerConfig.hdType == eRADIO_HD_TYPE_HD1p5_DUAL_MRC))
		{
			ret = YES;
		}
	}
#endif
	return ret;
}

int32 tcradioservice_getBsTunerNumber(void)
{
	int32 ret = -1;
#ifdef USE_HDRADIO
	if(tcradioservice_isHdRadio() == YES) {
		if(stTunerConfig.hdType == eRADIO_HD_TYPE_HD1p5) {
			ret = 2;
		}
		else if((stTunerConfig.hdType == eRADIO_HD_TYPE_HD1p5_MRC) || (stTunerConfig.hdType == eRADIO_HD_TYPE_HD1p5_DUAL_MRC)) {
			ret = 3;
		}
		else {
			ret = 0;
		}
	}
#endif
	return ret;
}

int32 tcradioservice_is2ndTunerPD(void)
{
	int32 ret = NO;

	if(stTunerConfig.fPhaseDiversity) {
		ret = YES;
	}

	return ret;
}

RET tcradioservice_init(void)
{
	RET ret = eRET_OK;

	tcradioservice_initVariable();
	if(serviceMainThreadID == (pthread_t)NULL) {
		ret = tcradio_createThread(&serviceMainThreadID, &tcradioservice_mainThread, "TCRADIO_SERVICE", eRADIO_SCHED_OTHER, 00, pNULL);
		if(ret != eRET_OK) {
			RSRV_ERR("[%s:%d] Can not make Radio Main Thread!!!\n", __func__, __LINE__);
			ret = eRET_NG_CREATE_THREAD;
			goto error_init;
		}
	}
	else {
		RSRV_WRN("[%s:%d] Already radio service thread was created!!!\n", __func__, __LINE__);
	}

	ret= tcradioservice_messageInit();

error_init:

	return ret;
}

RET tcradioservice_deinit(void)
{
	RET ret;
	stRadioService.fThreadRunning = 0;
	serviceMainThreadID = (pthread_t)NULL;
#ifdef USE_HDRADIO
	if(tcradioservice_isHdRadio() == YES) {
		tcradioservice_deinitHdr();
	}
#endif
	ret = tcradio_joinThread(serviceMainThreadID, (void**)NULL);
	return ret;
}

RET tcradioservice_close(void)
{
	RET ret;
	if(stRadioService.fRadioOpened) {
		tcradiosound_close();
		if(tcrds_getEnable() == YES) {
			tcrds_close(eRADIO_ID_PRIMARY);
		}
	#ifdef USE_HDRADIO
		if(tcradioservice_isHdRadio() == YES) {
			tcradioservice_closeHdr();
			tcradioservice_relHdrIQ01Drv();
			tcradioservice_relHdrIQ23Drv();
			tcradioservice_relHdrBlendAudioDrv();
		}
	#endif
		ret = tcradiohal_close();
		tcradioservice_mutexLock();
		stRadioService.fRadioOpened = 0;
		tcradioservice_mutexUnlock();
	}
	else {
		ret = eRET_NG_NOT_OPEN;
	}
	return ret;
}

void tcradioservice_initVariable(void)
{
	uint32 i;

	for(i=0; i < _MaxPresetNum; i++) {
		tcradio_memset(&stRadioList[i], 0x00, sizeof(stRADIO_LIST_t));
	}

	for(i=0; i<_AmFmDBSize; i++) {
		tcradio_memset(&_stStationList[i], 0x00, sizeof(stRADIO_LIST_t));
	}

	stRadioService.curBand = (uint32)(-1);
	stRadioService.curFreq = (uint32)(-1);
#if 1
	stRadioService.fm.startFreq = 87500;
	stRadioService.fm.endFreq = 108000;
	stRadioService.fm.step = 100;

	stRadioService.am.startFreq = 531;
	stRadioService.am.endFreq = 1620;
	stRadioService.am.step = 9;
#else
	stRadioService.fm.startFreq = 87500;
	stRadioService.fm.endFreq = 107900;
	stRadioService.fm.step = 200;

	stRadioService.am.startFreq = 530;
	stRadioService.am.endFreq = 1710;
	stRadioService.am.step = 10;
#endif

	for(i=0; i < _MaxPresetNum; i++) {
		stRadioService.currentBandPresetFreq[eRADIO_FM_MODE][i] = stRadioService.fm.startFreq;
	}
	for(i=0; i < _MaxPresetNum; i++) {
		stRadioService.currentBandPresetFreq[eRADIO_AM_MODE][i] = stRadioService.am.startFreq;
	}
	for(i=0; i < _MaxPresetNum; i++) {
		stRadioService.currentBandPresetFreq[eRADIO_DAB_MODE][i] = stRadioService.dab.startIndex;
	}

#if 1	// Asia 100Khz Step
	stRadioService.currentBandPresetFreq[eRADIO_FM_MODE][1] = 90000;
	stRadioService.currentBandPresetFreq[eRADIO_FM_MODE][2] = 98000;
	stRadioService.currentBandPresetFreq[eRADIO_FM_MODE][3] = 106000;
#else	// USA 200KHz Step
	stRadioService.currentBandPresetFreq[eRADIO_FM_MODE][1] = 90100;
	stRadioService.currentBandPresetFreq[eRADIO_FM_MODE][2] = 98100;
	stRadioService.currentBandPresetFreq[eRADIO_FM_MODE][3] = 106100;
#endif
	stRadioService.currentBandPresetFreq[eRADIO_FM_MODE][4] = stRadioService.fm.endFreq;

#if 1	// Asia 9KHz Step
	stRadioService.currentBandPresetFreq[eRADIO_AM_MODE][1] = 603;
	stRadioService.currentBandPresetFreq[eRADIO_AM_MODE][2] = 999;
	stRadioService.currentBandPresetFreq[eRADIO_AM_MODE][3] = 1404;
#else	// USA 10KHz Step
	stRadioService.currentBandPresetFreq[eRADIO_AM_MODE][1] = 600;
	stRadioService.currentBandPresetFreq[eRADIO_AM_MODE][2] = 1000;
	stRadioService.currentBandPresetFreq[eRADIO_AM_MODE][3] = 1400;
#endif
	stRadioService.currentBandPresetFreq[eRADIO_AM_MODE][4] = stRadioService.am.endFreq;
}

RET tcradioservice_checkConfig(stRADIO_CONFIG_t *config)
{
	RET ret = eRET_OK;
	if ( (config->numTuners != 1 && config->numTuners != 2 && config->numTuners != 3 && config->numTuners != 4) ||
	   (config->fPhaseDiversity != 0 && config->fPhaseDiversity != 1) ||
	   (config->numTuners == 1 && config->fPhaseDiversity == 1) ||
	   (config->audioSamplerate != 44100 && config->audioSamplerate != 48000))
	{
		ret = eRET_NG_INVALID_PARAM;
	}
	return ret;
}

void tcradioservice_setMainMode(eRADIO_EVENT_t mode)
{
	stRadioService.eMainMode = mode;
}

eRADIO_EVENT_t tcradioservice_getMainMode(void)
{
	return stRadioService.eMainMode;
}

void tcradioservice_setSeekStep(eSEEK_STEP_t step)
{
	stRadioService.eSeekStep = step;
}

eSEEK_STEP_t tcradioservice_getSeekStep(void)
{
	return stRadioService.eSeekStep;
}

void tcradioservice_setSeekMode(eRADIO_SEEK_MODE_t mode)
{
	stRadioService.eSeekMode = mode;
}

eRADIO_SEEK_MODE_t tcradioservice_getSeekMode(void)
{
	return stRadioService.eSeekMode;
}

void *tcradioservice_mainThread(void *arg)
{
	RET ret = eRET_OK;
	stMsgBuf_t stRecivedMessage = {0,};

	prctl(PR_SET_NAME, "TCRADIO_SERVICE",0,0,0);

	stRadioService.fThreadRunning = 1;
	while(stRadioService.fThreadRunning > 0) {
		tcradioservice_getMessage(&stRecivedMessage);
		if(stRecivedMessage.fNewMsg == eNEW_MSG_EXIST) {
			switch(stRecivedMessage.uiSender) {
				case eSENDER_ID_APP:
					tcradioservice_appMessageParser(&stRecivedMessage);
					break;

				case eSENDER_ID_SOUND:
					tcradioservice_soundMessageParser(&stRecivedMessage);
					break;

				default:
					break;
			}
		}
		tcradioservice_eventHandler();		// Service Command Message Event Handler
		tcradioservice_seekHandler();
	//	tcradioservice_hdrDataManager();
		tcradioservice_appNotifyHandler();	// Application Notify Message Handler ( Notify Message -> Callback Function )
		tcradio_mssleep(SERVICE_THREAD_TIME_INTERVAL);
	}

	stRadioService.fThreadRunning = -1;
	tcradioservice_messageDeinit();
	tcradiohal_deinit();
	tcradio_exitThread((void*)0);

	return ((void*)0);
}

void tcradioservice_appNotifyHandler(void)
{
	int i;
	stMsgBuf_t stRecivedMessage = {0,};

	for(i=0; i<3; i++) {
		tcradioapp_getMessage(&stRecivedMessage);
		if(stRecivedMessage.fNewMsg == eNEW_MSG_EXIST) {
			if(stRecivedMessage.uiSender == eSENDER_ID_SERVICE) {
				tcradioservice_callbackAppFunction(&stRecivedMessage);
			}
		}
		else {
			break;
		}
	}
}

void tcradioservice_soundMessageParser(stMsgBuf_t *pstMsg)
{
	uint32 uiSendMsg[MSGQ_DATA_LENGTH] = {0,};

	switch((eSOUND_Notify_t)pstMsg->uiMode) {
	/* Sound */
		case eSOUND_NOTIFY_OPEN:
		//	RSRV_DBG("[%s:%d] eSOUND_NOTIFY_OPEN source[%d], ret[%d]\n", __func__, __LINE__, pstMsg->uiData[0], pstMsg->iError);
		//	if(pstMsg->iError == eRET_OK) {
		//		tcradiosound_sendMessage(eSENDER_ID_SERVICE, eSOUND_CMD_PLAYBACK, uiSendMsg, pNULL, 0);
		//	}
			break;

		case eSOUND_NOTIFY_CLOSE:
		//	RSRV_DBG("[%s:%d] eSOUND_NOTIFY_CLOSE ret[%d]\n", __func__, __LINE__, pstMsg->iError);
			break;

		case eSOUND_NOTIFY_PLAYBACK:
		//	RSRV_DBG("[%s:%d] eSOUND_NOTIFY_PLAYBACK ret[%d]\n", __func__, __LINE__, pstMsg->iError);
			break;

		case eSOUND_NOTIFY_PAUSE:
		//	RSRV_DBG("[%s:%d] eSOUND_NOTIFY_PAUSE ret[%d]\n", __func__, __LINE__, pstMsg->iError);
			break;

		case eSOUND_NOTIFY_RESET:
		//	RSRV_DBG("[%s:%d] eSOUND_NOTIFY_RESET ret[%d]\n", __func__, __LINE__, pstMsg->iError);
		//	if(pstMsg->iError == eRET_OK) {
		//		tcradiosound_sendMessage(eSENDER_ID_SERVICE, eSOUND_CMD_PLAYBACK, uiSendMsg, pNULL, 0);
		//	}
			break;

		default:
			break;
	}
}

void tcradioservice_appMessageParser(stMsgBuf_t *pstMsg)
{
	uint32 i;
	stRADIO_CONFIG_t *radioconf;

	stRadioRcvMsgQ.uiMode = pstMsg->uiMode;
	tcradio_memcpy(stRadioRcvMsgQ.uiData, &pstMsg->uiData, MSGQ_DATA_LENGTH*sizeof(uint32));

	switch((eRADIO_CMD_t)pstMsg->uiMode) {
	/* Radio */
		case eRADIO_CMD_OPEN:
			stTunerConfig.area = (eRADIO_CONF_AREA_t)stRadioRcvMsgQ.uiData[0];
			stTunerConfig.initMode = (eRADIO_MOD_MODE_t)stRadioRcvMsgQ.uiData[1];
			stTunerConfig.initFreq = stRadioRcvMsgQ.uiData[2];
			stTunerConfig.numTuners = stRadioRcvMsgQ.uiData[3];
			stTunerConfig.fPhaseDiversity = stRadioRcvMsgQ.uiData[4];
			stTunerConfig.fIqOut = stRadioRcvMsgQ.uiData[5];
			stTunerConfig.audioSamplerate = stRadioRcvMsgQ.uiData[6];
			stTunerConfig.fExtAppCtrl = stRadioRcvMsgQ.uiData[7];
			stTunerConfig.sdr = stRadioRcvMsgQ.uiData[8];
			stTunerConfig.hdType = stRadioRcvMsgQ.uiData[9];
			tcradioservice_setMainMode(eRADIO_EVT_OPEN);
			break;

		case eRADIO_CMD_DEINIT:
			tcradioservice_setMainMode(eRADIO_EVT_DEINIT);
			break;

		case eRADIO_CMD_SET_DAB_FREQ_LIST:
			dabFreqList = (uint32*)pstMsg->pData[0];
			tcradioservice_setMainMode(eRADIO_EVT_SET_DAB_FREQ_LIST);
			break;

		case eRADIO_CMD_SET_TUNE:
			tcradioservice_setMainMode(eRADIO_EVT_SET_TUNE);
			break;

		case eRADIO_CMD_SET_SEEK:
			tcradioservice_initSeek((eRADIO_SEEK_MODE_t)pstMsg->uiData[0]);
			tcradioservice_setMainMode(eRADIO_EVT_SET_SEEK);
			break;

		case eRADIO_CMD_SET_COEF:
			tcradioservice_setMainMode(eRADIO_EVT_SET_COEF);
			break;

		case eRADIO_CMD_GET_COEF:
			tcradioservice_setMainMode(eRADIO_EVT_GET_COEF);
			break;

		default:
			break;
	}
}

static void tcradioservice_eventHandler(void)
{
	eRADIO_STS_t eRadioSt = eRADIO_STS_OK;
	eRADIO_EVENT_t eRadioNowExeMode;
	RET ret = eRET_OK;
	uint32 uiSendMsg[MSGQ_DATA_LENGTH] = {0,}, i, j;

	eRadioNowExeMode = tcradioservice_getMainMode();

	switch(eRadioNowExeMode)
	{
		case eRADIO_EVT_OPEN:
			ret = tcradioservice_checkConfig(&stTunerConfig);
			if(ret == eRET_OK) {
				stTUNER_DRV_CONFIG_t stConfigSet;
				stConfigSet.area = (eTUNER_DRV_CONF_AREA_t)stTunerConfig.area;
				stConfigSet.initMode = (eTUNER_DRV_MOD_MODE_t)stTunerConfig.initMode;
				stConfigSet.initFreq = stTunerConfig.initFreq;
				stConfigSet.numTuners = stTunerConfig.numTuners;
				stConfigSet.fPhaseDiversity = stTunerConfig.fPhaseDiversity;
				stConfigSet.fIqOut = stTunerConfig.fIqOut;
				stConfigSet.audioSamplerate = stTunerConfig.audioSamplerate;
				stConfigSet.fExtAppCtrl = stTunerConfig.fExtAppCtrl;
				stConfigSet.sdr = (eTUNER_DRV_SDR_t)stTunerConfig.sdr;
				stConfigSet.reserved[0] = (uint32)stTunerConfig.hdType;
				ret = tcradiohal_open(stConfigSet);
			#if 0	// Since change to control audio devices independently, this codes were commented out.
				if(ret == eRET_OK) {
					if(stTunerConfig.fExtAppCtrl == 0) {
					#ifdef USE_HDRADIO
						if(stTunerConfig.sdr != eRADIO_SDR_NONE) {
							ret = tcradiosound_open(stTunerConfig.audioSamplerate, eSOUND_SOURCE_SDR);
						}
						else {
							ret = tcradiosound_open(stTunerConfig.audioSamplerate, eSOUND_SOURCE_TC_I2S);
						}
					#else
						ret = tcradiosound_open(stTunerConfig.audioSamplerate, eSOUND_SOURCE_TC_I2S);
					#endif
					}
				}
			#endif
			}

			if(ret == eRET_OK) {
				uiSendMsg[0] = stRadioService.curBand = stTunerConfig.initMode;
				uiSendMsg[1] = stRadioService.curFreq = stTunerConfig.initFreq;
				stRadioService.fRadioOpened = 1;
				eRadioSt = eRADIO_STS_OK_NOTIFY;
			#ifdef USE_HDRADIO
				if(tcradioservice_isHdRadio() == YES) {
					eTC_HDR_BBSRC_RATE_t iqSR = tcradioservice_getIqSampleRate(eRADIO_ID_PRIMARY);
					uint32 iqSbit = (uint32)tcradiohal_getIqSamplingBit(eRADIO_ID_PRIMARY);
					if(iqSR < eTC_HDR_BBSRC_UNKNOWN) {
						if(stTunerConfig.hdType == eRADIO_HD_TYPE_HD1p0) {
							if(stTunerConfig.numTuners == eRADIO_CONF_TYPE_SINGLE) {
							#if 1
								// If USE_THIRD_TUNER_AS_SINGLE_TUNER is not defined in si479xx_hal.h, set as follows.
								tcradioservice_confHdrIQ01Drv(pfnIQ01I2sOpen, pfnIQ01I2sClose, pfnIQ01I2sSetParams, pfnIQ01I2sStart, pfnIQ01I2sStop, pfnIQ01I2sRead);
							#else
								// If USE_THIRD_TUNER_AS_SINGLE_TUNER is defined in si479xx_hal.h, set as follows.
								tcradioservice_confHdrIQ01Drv(pfnIQ23I2sOpen, pfnIQ23I2sClose, pfnIQ23I2sSetParams, pfnIQ23I2sStart, pfnIQ23I2sStop, pfnIQ23I2sRead);
							#endif
							}
							else {
								tcradioservice_confHdrIQ01Drv(pfnIQ01I2sOpen, pfnIQ01I2sClose, pfnIQ01I2sSetParams, pfnIQ01I2sStart, pfnIQ01I2sStop, pfnIQ01I2sRead);
							}
						}
						else if(stTunerConfig.hdType == eRADIO_HD_TYPE_HD1p5 || stTunerConfig.hdType == eRADIO_HD_TYPE_HD1p0_MRC) {
							tcradioservice_confHdrIQ01Drv(pfnIQ01I2sOpen, pfnIQ01I2sClose, pfnIQ01I2sSetParams, pfnIQ01I2sStart, pfnIQ01I2sStop, pfnIQ01I2sRead);
						}
						else {
							tcradioservice_confHdrIQ01Drv(pfnIQ01I2sOpen, pfnIQ01I2sClose, pfnIQ01I2sSetParams, pfnIQ01I2sStart, pfnIQ01I2sStop, pfnIQ01I2sRead);
							tcradioservice_confHdrIQ23Drv(pfnIQ23I2sOpen, pfnIQ23I2sClose, pfnIQ23I2sSetParams, pfnIQ23I2sStart, pfnIQ23I2sStop, pfnIQ23I2sRead);
						}
						tcradioservice_confHdrBlendAudioDrv(pfnBlendAudioI2sOpen, pfnBlendAudioI2sClose, pfnBlendAudioI2sSetParams, pfnBlendAudioI2sStart, pfnBlendAudioI2sStop, pfnBlendAudioI2sRead);
						ret = tcradioservice_initHdr(stTunerConfig.hdType, iqSR, iqSbit);
					}
					else {
						ret = eRET_NG_INVALID_PARAM;
					}
					if(ret == eRET_OK || ret == eTC_HDR_RET_NG_ALREADY_INIT) {
						ret = tcradioservice_openHdr(stRadioService.curBand, stRadioService.curFreq, iqSR);
					}
					if(ret != eRET_OK) {
						eRadioSt = eRADIO_STS_ERROR;
					}
				}
			#endif
			}
			else {
				eRadioSt = eRADIO_STS_ERROR;
			}
			break;

		case eRADIO_EVT_DEINIT:
			tcradioservice_close();
			tcradioservice_deinit();
			tcradiosound_deinit();
			tcrds_deinit();
			break;

		case eRADIO_EVT_SET_DAB_FREQ_LIST:
			if(stTunerConfig.sdr == eRADIO_SDR_DAB) {
				ret = tcradiohal_setDabFreqList(dabFreqList, stRadioRcvMsgQ.uiData[0], stRadioRcvMsgQ.uiData[1]);
				if(ret == eRET_OK) {
					eRadioSt = eRADIO_STS_OK_NOTIFY;
				}
				else {
					eRadioSt = eRADIO_STS_ERROR;
				}
			}
			else {
				ret = eRET_NG_NO_RSC;
				eRadioSt = eRADIO_STS_ERROR;
			}
			break;

		case eRADIO_EVT_SET_TUNE:
			switch(tcradioservice_getNumberOfTuners()) {
				case eRADIO_CONF_TYPE_SINGLE:	if(stRadioRcvMsgQ.uiData[3] != eRADIO_ID_PRIMARY)	ret = eRET_NG_NOT_SUPPORT;	break;
				case eRADIO_CONF_TYPE_DUAL:
					if(stRadioRcvMsgQ.uiData[3] <= eRADIO_ID_SECONDARY) {
						if(stRadioRcvMsgQ.uiData[3] == eRADIO_ID_SECONDARY && (tcradioservice_is2ndTunerMRC() == YES || tcradioservice_is2ndTunerPD() == YES))
							ret = eRET_NG_NOT_SUPPORT;
					}
					else {
						ret = eRET_NG_NOT_SUPPORT;
					}
					break;
				case eRADIO_CONF_TYPE_TRIPLE:	if(stRadioRcvMsgQ.uiData[3] > eRADIO_ID_TERTIARY)	ret = eRET_NG_NOT_SUPPORT;	break;
				case eRADIO_CONF_TYPE_QUAD:		if(stRadioRcvMsgQ.uiData[3] > eRADIO_ID_QUATERNARY) ret = eRET_NG_NOT_SUPPORT;	break;
				default:						ret = eRET_NG_NOT_SUPPORT;														break;
			}

			if(ret == eRET_OK) {
				if(stRadioRcvMsgQ.uiData[0] == eRADIO_FM_MODE || stRadioRcvMsgQ.uiData[0] == eRADIO_AM_MODE ||
					(stRadioRcvMsgQ.uiData[0] == eRADIO_DAB_MODE && stTunerConfig.sdr == eRADIO_SDR_DAB))
				{
					if(tcradioservice_isSeeking() == 0 && stRadioRcvMsgQ.uiData[3] == eRADIO_ID_PRIMARY) {
						tcradioservice_setSeekStep(eSEEK_STEP_ON_AIR);
						tcradioservice_stopSeek(0);
						uiSendMsg[0] = stRadioService.curBand;
						uiSendMsg[1] = stRadioService.curFreq;
						uiSendMsg[2] = tcradioservice_getSeekMode();
						uiSendMsg[3] = stRadioService.curSeekResult;
						tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_NOTIFY_SEEK_MODE, uiSendMsg, pNULL, ret);
						tcradio_memset(uiSendMsg, 0, sizeof(uiSendMsg));
					}
					ret = tcradioservice_checkValidFreq(stRadioRcvMsgQ.uiData[0], stRadioRcvMsgQ.uiData[1]);
					if(ret == eRET_OK) {
						if(tcradioservice_isHdRadio() == YES && stTunerConfig.sdr == eRADIO_SDR_HD) {
						#ifdef USE_HDRADIO
							// HD Radio
							if(tcradioservice_is2ndTunerMRC()) {
								// With MRC
								if(stRadioRcvMsgQ.uiData[3] == eRADIO_ID_PRIMARY) {
								//	tcradioservice_setHdrMute(ON);
									ret = tcradiohal_setTune(stRadioRcvMsgQ.uiData[0], stRadioRcvMsgQ.uiData[1], stRadioRcvMsgQ.uiData[2], stRadioRcvMsgQ.uiData[3]);
									if(ret == eRET_OK) {
										uint32 fChgBand=(stRadioService.curBand != stRadioRcvMsgQ.uiData[0] ? 1 : 0);
										stTC_HDR_TUNE_TO_t tuneTo1, tuneTo2;
										stRadioService.curBand = stRadioRcvMsgQ.uiData[0];
										stRadioService.curFreq = stRadioRcvMsgQ.uiData[1];
										if(stRadioRcvMsgQ.uiData[0] == eRADIO_FM_MODE) {
											tuneTo1.band = tuneTo2.band = eTC_HDR_FM_BAND;
										}
										else {
											tuneTo1.band = tuneTo2.band = eTC_HDR_AM_BAND;
										}
										tuneTo1.freq = tuneTo2.freq = stRadioRcvMsgQ.uiData[1];
										tuneTo1.iqsamplerate = tcradioservice_getIqSampleRate(stRadioRcvMsgQ.uiData[3]);
										tcradioservice_setHdrTune(eTC_HDR_ID_MAIN, tuneTo1);
										tcradioservice_setHdrProgramNumber(0);

										ret = tcradiohal_setTune(stRadioRcvMsgQ.uiData[0], stRadioRcvMsgQ.uiData[1], stRadioRcvMsgQ.uiData[2], eRADIO_ID_SECONDARY);
										if(ret == eRET_OK) {
											tuneTo2.iqsamplerate = tcradioservice_getIqSampleRate(eRADIO_ID_SECONDARY);
											tcradioservice_setHdrTune(eTC_HDR_ID_MRC, tuneTo2);
										}
										tcradioservice_enableHdrSis(eTC_HDR_ID_MAIN, eBITMASK_SIS_SHORT_NAME, 1);
									#if 0    // BS tuner can tune separately and disable this part
										if(fChgBand && tcradioservice_getBsTunerNumber() == 3) {
											ret = tcradiohal_setTune(stRadioRcvMsgQ.uiData[0], stRadioRcvMsgQ.uiData[1], stRadioRcvMsgQ.uiData[2], eRADIO_ID_TERTIARY);
											if(ret == eRET_OK) {
												stTC_HDR_TUNE_TO_t tuneTo3;
												tuneTo3.band = tuneTo1.band;
												tuneTo3.freq = tuneTo1.freq;
												tuneTo3.iqsamplerate = tcradioservice_getIqSampleRate(eRADIO_ID_TERTIARY);
												tcradioservice_setHdrTune(eTC_HDR_ID_BS, tuneTo3);
											}
										}
									#endif
									}
								//	tcradioservice_setHdrMute(OFF);
								}
								else {
									ret = tcradiohal_setTune(stRadioRcvMsgQ.uiData[0], stRadioRcvMsgQ.uiData[1], stRadioRcvMsgQ.uiData[2], stRadioRcvMsgQ.uiData[3]);
									if(ret == eRET_OK) {
										stTC_HDR_TUNE_TO_t tuneTo1;
										if(stRadioRcvMsgQ.uiData[0] == eRADIO_FM_MODE) {
											tuneTo1.band = eTC_HDR_FM_BAND;
										}
										else {
											tuneTo1.band = eTC_HDR_AM_BAND;
										}
										tuneTo1.freq = stRadioRcvMsgQ.uiData[1];
										tuneTo1.iqsamplerate = tcradioservice_getIqSampleRate(stRadioRcvMsgQ.uiData[3]);
										if(stRadioRcvMsgQ.uiData[3] == eRADIO_ID_TERTIARY) {
											tcradioservice_setHdrTune(eTC_HDR_ID_BS, tuneTo1);
										}
										else {
											;
										}
									}
								}
							}
							else {
								// Without MRC (eTC_HDR_TYPE_HD_1p0 or eTC_HDR_TYPE_HD_1p5)
								uint32 fChgBand=(stRadioService.curBand != stRadioRcvMsgQ.uiData[0] ? 1 : 0);
								if(stRadioRcvMsgQ.uiData[3] == eRADIO_ID_PRIMARY) {
								//	tcradioservice_setHdrMute(ON);
								}
								ret = tcradiohal_setTune(stRadioRcvMsgQ.uiData[0], stRadioRcvMsgQ.uiData[1], stRadioRcvMsgQ.uiData[2], stRadioRcvMsgQ.uiData[3]);
								if(ret == eRET_OK) {
									stTC_HDR_TUNE_TO_t tuneTo1;
									if(stRadioRcvMsgQ.uiData[3] == eRADIO_ID_PRIMARY) {
										stRadioService.curBand = stRadioRcvMsgQ.uiData[0];
										stRadioService.curFreq = stRadioRcvMsgQ.uiData[1];
									}

									if(stRadioRcvMsgQ.uiData[0] == eRADIO_FM_MODE) {
										tuneTo1.band = eTC_HDR_FM_BAND;
									}
									else {
										tuneTo1.band = eTC_HDR_AM_BAND;
									}
									tuneTo1.freq = stRadioRcvMsgQ.uiData[1];
									tuneTo1.iqsamplerate = tcradioservice_getIqSampleRate(stRadioRcvMsgQ.uiData[3]);
									if(stRadioRcvMsgQ.uiData[3] == eRADIO_ID_PRIMARY) {
										tcradioservice_setHdrTune(eTC_HDR_ID_MAIN, tuneTo1);
										tcradioservice_setHdrProgramNumber(0);
										tcradioservice_enableHdrSis(eTC_HDR_ID_MAIN, eBITMASK_SIS_SHORT_NAME, 1);
									#if 0    // BS tuner can tune separately and disable this part
										if(fChgBand && tcradioservice_getBsTunerNumber() == 2) {
											ret = tcradiohal_setTune(stRadioRcvMsgQ.uiData[0], stRadioRcvMsgQ.uiData[1], stRadioRcvMsgQ.uiData[2], eRADIO_ID_SECONDARY);
											if(ret == eRET_OK) {
												stTC_HDR_TUNE_TO_t tuneTo2;
												tuneTo2.band = tuneTo1.band;
												tuneTo2.freq = tuneTo1.freq;
												tuneTo2.iqsamplerate = tcradioservice_getIqSampleRate(eRADIO_ID_SECONDARY);
												tcradioservice_setHdrTune(eTC_HDR_ID_BS, tuneTo2);
											}
										}
									#endif
									}
									else if(stRadioRcvMsgQ.uiData[3] == eRADIO_ID_SECONDARY) {
										tcradioservice_setHdrTune(eTC_HDR_ID_BS, tuneTo1);
									}
									else if(stRadioRcvMsgQ.uiData[3] == eRADIO_ID_TERTIARY) {
										tcradioservice_setHdrTune(eTC_HDR_ID_BS, tuneTo1);
									}
									else {
										;
									}
								}
								if(stRadioRcvMsgQ.uiData[3] == eRADIO_ID_PRIMARY) {
								//	tcradioservice_setHdrMute(OFF);
								}
							}
						#endif	// #ifdef USE_HDRADIO
						}
						else {
							// Only Analog Radio
							ret = tcradiohal_setTune(stRadioRcvMsgQ.uiData[0], stRadioRcvMsgQ.uiData[1], stRadioRcvMsgQ.uiData[2], stRadioRcvMsgQ.uiData[3]);
							if(ret == eRET_OK && stRadioRcvMsgQ.uiData[3] == eRADIO_ID_PRIMARY) {
								stRadioService.curBand = stRadioRcvMsgQ.uiData[0];
								stRadioService.curFreq = stRadioRcvMsgQ.uiData[1];
							}
						}
					}
				}
				else {
					ret = eRET_NG_INVALID_PARAM;
				}
			}
			eRadioSt = eRADIO_STS_OK_NOTIFY;
			break;

		case eRADIO_EVT_SET_SEEK:
			if(stRadioService.curBand != eRADIO_FM_MODE && stRadioService.curBand != eRADIO_AM_MODE && stRadioService.curBand != eRADIO_DAB_MODE) {
				tcradioservice_setSeekStep(eSEEK_STEP_ON_AIR);
				tcradioservice_stopSeek(0);
				ret = eRET_NG_NOT_SUPPORT;
				eRadioSt = eRADIO_STS_ERROR;
			}
			break;

		case eRADIO_EVT_SET_COEF:
			break;

		case eRADIO_EVT_GET_COEF:
			break;

		default:
			break;
	}

	if(tcradioservice_getMainMode() == eRadioNowExeMode) {
		tcradioservice_setMainMode(eRADIO_EVT_NULL);
	}

	switch(eRadioSt) {
		/* Job End -> No Notify */
		case eRADIO_STS_WAIT:
			break;

		/* Job Good Complete */
		case eRADIO_STS_OK:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eRADIO_STS_OK_NOTIFY:
			switch(eRadioNowExeMode) {
				case eRADIO_EVT_OPEN:
					tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_NOTIFY_OPEN, uiSendMsg, pNULL, ret);
					break;

				case eRADIO_EVT_SET_TUNE:
					uiSendMsg[0] = stRadioRcvMsgQ.uiData[0];
					uiSendMsg[1] = stRadioRcvMsgQ.uiData[1];
					uiSendMsg[2] = tcradioservice_getSeekMode();
					uiSendMsg[3] = stRadioRcvMsgQ.uiData[3];
					tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_NOTIFY_TUNE, uiSendMsg, pNULL, ret);
					break;

				case eRADIO_EVT_SET_SEEK:
					uiSendMsg[0] = stRadioService.curBand;
					uiSendMsg[1] = stRadioService.curFreq;
					uiSendMsg[2] = tcradioservice_getSeekMode();
					uiSendMsg[3] = stRadioService.curSeekResult;
					tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_NOTIFY_SEEK_MODE, uiSendMsg, pNULL, ret);
					break;

				case eRADIO_EVT_SET_DAB_FREQ_LIST:
					uiSendMsg[0] = stRadioRcvMsgQ.uiData[0];
					uiSendMsg[1] = stRadioRcvMsgQ.uiData[1];
					tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_NOTIFY_DAB_FREQ_LIST, uiSendMsg, dabFreqList, ret);
					break;

				default:
					break;
			}
			break;

		/* Job Continue */
		case eRADIO_STS_DOING:
			tcradioservice_setMainMode(eRadioNowExeMode);
			break;

		/* Job Continue -> Information Dynamic Notify */
		case eRADIO_STS_DOING_NOTIFY :
			tcradioservice_setMainMode(eRadioNowExeMode);
			switch(eRadioNowExeMode) {
				case eRADIO_EVT_SET_SEEK:
					uiSendMsg[0] = stRadioService.curBand;
					uiSendMsg[1] = stRadioService.curFreq;
					uiSendMsg[2] = tcradioservice_getSeekMode();
					tcradio_memcpy(uiSendMsg+3, &stRadioService.stSchQdata, sizeof(stRadioService.stSchQdata));
					break;

				default:
					break;
			}
			tcradioapp_sendMessage(eSENDER_ID_SERVICE, (uint32)eRadioNowExeMode, uiSendMsg, pNULL, (RET)NULL);
			break;

		/* Job Continue -> Error Notify */
		case eRADIO_STS_DOING_ERROR_NOTIFY :
			tcradioservice_setMainMode(eRadioNowExeMode);	/* No break because of doing below function */

		/* Job Error -> Return Error */
		case eRADIO_STS_ERROR:
			switch(eRadioNowExeMode) {
				case eRADIO_EVT_SET_TUNE:
					tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_NOTIFY_TUNE, uiSendMsg, pNULL, ret);
					break;

				default:
					tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRadioNowExeMode, NULL, pNULL, ret);
					break;
			}
			break;

		/* Return Error */
		default:
			break;
	}


}

#ifdef USE_HDRADIO
RET tcradioservice_setHdrTuneTo(eRADIO_MOD_MODE_t mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner)
{
	RET ret = eRET_OK;
	uint32 uiSendMsg[MSGQ_DATA_LENGTH] = {0,};

	switch(tcradioservice_getNumberOfTuners()) {
		case eRADIO_CONF_TYPE_SINGLE:
			if(ntuner > eRADIO_ID_PRIMARY) {
				ret = eRET_NG_NOT_SUPPORT;
			}
			break;

		case eRADIO_CONF_TYPE_DUAL:
			if(ntuner <= eRADIO_ID_SECONDARY) {
				if(ntuner == eRADIO_ID_SECONDARY && stTunerConfig.fPhaseDiversity == 1)
					ret = eRET_NG_NOT_SUPPORT;
			}
			else {
				ret = eRET_NG_NOT_SUPPORT;
			}
			break;

		case eRADIO_CONF_TYPE_TRIPLE:
			if(ntuner > eRADIO_ID_TERTIARY) {
				ret = eRET_NG_NOT_SUPPORT;
			}
			break;

		case eRADIO_CONF_TYPE_QUAD:
			if(ntuner > eRADIO_ID_QUATERNARY) {
				ret = eRET_NG_NOT_SUPPORT;
			}
			break;

		default:
			ret = eRET_NG_NOT_SUPPORT;
			break;
	}
	if(ret == eRET_OK) {
		if(mod_mode == eRADIO_FM_MODE || mod_mode == eRADIO_AM_MODE) {
			if(tcradioservice_isSeeking() == 0 && ntuner == eRADIO_ID_PRIMARY) {
				tcradioservice_setSeekStep(eSEEK_STEP_ON_AIR);
				tcradioservice_stopSeek(0);
				uiSendMsg[0] = stRadioService.curBand;
				uiSendMsg[1] = stRadioService.curFreq;
				uiSendMsg[2] = tcradioservice_getSeekMode();
				uiSendMsg[3] = stRadioService.curSeekResult;
				tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_NOTIFY_SEEK_MODE, uiSendMsg, pNULL, ret);
			}
			ret = tcradioservice_checkValidFreq(mod_mode, freq);
			if(ret == eRET_OK) {
				ret = tcradiohal_setTune(mod_mode, freq, tune_mode, ntuner);
				if(ret == eRET_OK && ntuner == eRADIO_ID_PRIMARY) {
					stRadioService.curBand = mod_mode;
					stRadioService.curFreq = freq;
				//	tcradiosound_sendMessage(eSENDER_ID_SERVICE, eSOUND_CMD_RESET, pNULL, pNULL, 0);
				}
				tcradioservice_setHdrProgramNumber(0);
			}
		}
		else {
			ret = eRET_NG_INVALID_PARAM;
		}
	}

	return ret;
}
#endif	// #ifdef USE_HDRADIO

void tcradioservice_callbackAppFunction(stMsgBuf_t *pstMsg)
{
	switch((eRADIO_NOTIFY_t)pstMsg->uiMode) {
		case eRADIO_NOTIFY_OPEN:
	    case eRADIO_NOTIFY_SEEK_MODE:
		case eRADIO_NOTIFY_TUNE:
		case eRADIO_NOTIFY_DAB_FREQ_LIST:
		case eRADIO_HD_NOTIFY_OPEN:
		case eRADIO_HD_NOTIFY_AUDIO_MODE:
		case eRADIO_HD_NOTIFY_PROGRAM:
		case eRADIO_HD_NOTIFY_STATUS:
		case eRADIO_HD_NOTIFY_PSD:
		case eRADIO_HD_NOTIFY_SIS:
		case eRADIO_HD_NOTIFY_SIG:
		case eRADIO_HD_NOTIFY_AAS:
		case eRADIO_HD_NOTIFY_ALERT:
		case eRADIO_HD_NOTIFY_SIGNAL_STATUS:
		case eRADIO_HD_NOTIFY_PTY:
		case eRADIO_HD_NOTIFY_LOT:
#if 0
			if(pfnOnGetNotificationCallBack) {
				if(fStationListTx && pstMsg->uiMode == eRADIO_NOTIFY_SEEK_MODE) {
					fStationListTx = 0;
					(*pfnOnGetNotificationCallBack)(pstMsg->uiMode, pstMsg->uiData, pstMsg->pData, pstMsg->iError);
					if(pfnOnGetStationListCallBack) {
						(*pfnOnGetStationListCallBack)(uiStationListCounter, (void *)_stStationList,pstMsg->iError);
					}
				}
				else {
					(*pfnOnGetNotificationCallBack)(pstMsg->uiMode, pstMsg->uiData, pstMsg->pData, pstMsg->iError);
				}
			}
#else
			if(pfnOnGetNotificationCallBack) 
            {
				(*pfnOnGetNotificationCallBack)(pstMsg->uiMode, pstMsg->uiData, pstMsg->pData, pstMsg->iError);

                if((pstMsg->uiData[4] == 1) && pstMsg->uiMode == eRADIO_NOTIFY_SEEK_MODE)
                {
                    if(pfnOnGetStationListCallBack)
                    {
                        if((pstMsg->uiData[5] == 0))
                        {
                            RBG_DBG("[%s] StationListCallBack : tcradio_service \n", __func__);
						    (*pfnOnGetStationListCallBack)(uiStationListCounter, (void *)_stStationList,pstMsg->iError);
                        }
                        else
                        {
                            RBG_DBG("[%s] StationListCallBack : tcradio_bg stRadioBG.curBGResult %d \n", __func__, stRadioBG.curBGResult);
						    (*pfnOnGetStationListCallBack)(stRadioBG.curBGResult, (void *)_stBGStationList,pstMsg->iError);
                        }
                    }
                }
            }
#endif
			else {
				RSRV_DBG("Error : Not registered the notification call-back function !!!\n");
			}
			break;

		default:
			break;
	}
}

RET tcradioservice_setBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 start_freq, uint32  end_freq, uint32 step)
{
	RET ret = eRET_OK;

	if(tcradioservice_getSeekMode() > eRADIO_SEEK_STOP && tcradioservice_getSeekMode() < eRADIO_SEEK_END) {
		ret = eRET_NG_BUSY;		// Seek Mode
	}
	else {
		ret = tcradioservice_checkValidBandFreqConfig(mod_mode, start_freq, end_freq, step);
		if(ret == eRET_OK) {
			if(mod_mode == eRADIO_FM_MODE) {
				stRadioService.fm.startFreq = start_freq;
				stRadioService.fm.endFreq = end_freq;
				stRadioService.fm.step = step;
			}
			else if(mod_mode == eRADIO_AM_MODE) {
				stRadioService.am.startFreq = start_freq;
				stRadioService.am.endFreq = end_freq;
				stRadioService.am.step = step;
			}
			else if(mod_mode == eRADIO_DAB_MODE) {
				stRadioService.dab.startIndex = start_freq;
				stRadioService.dab.endIndex = end_freq;
				stRadioService.dab.step = step;
			}
			else {
				ret = eRET_NG_INVALID_RESP;
			}
		}
	}
	return ret;
}

RET tcradioservice_getBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 *start_freq, uint32 *end_freq, uint32 *step)
{
	RET ret = eRET_OK;

	if(mod_mode == eRADIO_FM_MODE) {
		*start_freq = stRadioService.fm.startFreq;
		*end_freq = stRadioService.fm.endFreq;
		*step = stRadioService.fm.step;
	}
	else if(mod_mode == eRADIO_AM_MODE) {
		*start_freq = stRadioService.am.startFreq;
		*end_freq = stRadioService.am.endFreq;
		*step = stRadioService.am.step;
	}
	else if(mod_mode == eRADIO_DAB_MODE) {
		*start_freq = stRadioService.dab.startIndex;
		*end_freq = stRadioService.dab.endIndex;
		*step = stRadioService.dab.step;
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

int32 tcradioservice_checkValidFreqAccordingToConfig(eRADIO_MOD_MODE_t mod_mode, uint32 freq)
{
	RET ret = 0;

	if(mod_mode == eRADIO_FM_MODE) {
		if(freq < stRadioService.fm.startFreq || freq > stRadioService.fm.endFreq) {
			ret = -1;
		}
		else {
			if(((freq - stRadioService.fm.startFreq) % stRadioService.fm.step) != 0) {
				ret = -2;
			}
		}
	}
	else if(mod_mode == eRADIO_AM_MODE) {
		if(freq < stRadioService.am.startFreq || freq > stRadioService.am.endFreq) {
			ret = -1;
		}
		else {
			if(((freq - stRadioService.am.startFreq) % stRadioService.am.step) != 0) {
				ret = -2;
			}
		}
	}
	else if(mod_mode == eRADIO_DAB_MODE) {
		if(freq < stRadioService.dab.startIndex || freq > stRadioService.dab.endIndex) {
			ret = -1;
		}
		else {
			if(((freq - stRadioService.dab.startIndex) % stRadioService.dab.step) != 0) {
				ret = -2;
			}
		}
	}
	else {
		ret = -3;
	}

	return ret;
}

RET tcradioservice_checkValidFreq(eRADIO_MOD_MODE_t mod_mode, uint32 freq)
{
	RET ret = eRET_OK;

	if(mod_mode == eRADIO_FM_MODE) {
		if(freq < 65000 || freq > 108000) {
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else if(mod_mode == eRADIO_AM_MODE) {
		if(freq < 144 || freq > 27000) {
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else if(mod_mode == eRADIO_DAB_MODE) {
		// DAB freq is index. not Freq.
		if(freq >= 64) {	// Max. 64
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET tcradioservice_checkValidBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 start_freq, uint32  end_freq, uint32 step)
{
	RET ret = eRET_OK;

	if(mod_mode == eRADIO_FM_MODE) {
		if( (tcradioservice_checkValidFreq(mod_mode, start_freq) != eRET_OK) ||
			(tcradioservice_checkValidFreq(mod_mode, end_freq) != eRET_OK) ||
			(start_freq >= end_freq) ||
			(step != 200 && step != 100 && step != 50 && step != 10) ||
			((end_freq-start_freq)%step != 0) )
		{
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else if(mod_mode == eRADIO_AM_MODE) {
		if( (tcradioservice_checkValidFreq(mod_mode, start_freq) != eRET_OK) ||
			(tcradioservice_checkValidFreq(mod_mode, end_freq) != eRET_OK) ||
			(start_freq >= end_freq) ||
			(step != 10 && step != 9 && step != 1) ||
			((end_freq-start_freq)%step != 0) )
		{
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else if(mod_mode == eRADIO_DAB_MODE) {
		if( (tcradioservice_checkValidFreq(mod_mode, start_freq) != eRET_OK) ||
			(tcradioservice_checkValidFreq(mod_mode, end_freq) != eRET_OK) ||
			(start_freq >= end_freq) )
		{
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET tcradioservice_getFreqCounter(eRADIO_MOD_MODE_t mod_mode, uint32 *freq, uint8 dir)
{
	RET ret = eRET_OK;
	uint32 tfreq = *freq, sf = 0, ef = 0, step = 0;

	if(mod_mode == eRADIO_FM_MODE) {
		ef = stRadioService.fm.endFreq;
		sf = stRadioService.fm.startFreq;
		step = 	stRadioService.fm.step;
	}
	else if(mod_mode == eRADIO_AM_MODE) {
		ef = stRadioService.am.endFreq;
		sf = stRadioService.am.startFreq;
		step = 	stRadioService.am.step;
	}
	else if(mod_mode == eRADIO_DAB_MODE) {
		ef = stRadioService.dab.endIndex;
		sf = stRadioService.dab.startIndex;
		step = 	stRadioService.dab.step;
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
		return ret;
	}

	if((sf == 0 && mod_mode != eRADIO_DAB_MODE) || ef == 0 || step == 0) {
		ret = eRET_NG_INVALID_PARAM;
		return ret;
	}

	if(dir == UP || dir == DN) {
		if(dir == UP) {
			if(tfreq >= ef) {
				tfreq = sf;
			}
			else {
				tfreq += step;
			}
		}
		else {
			if(tfreq <= sf) {
				tfreq = ef;
			}
			else {
				tfreq -= step;
			}
		}

		*freq = tfreq;
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

static void tcradioservice_seekHandler(void)
{
	eRADIO_STS_t eRadioSt = eRADIO_STS_DOING;
	eSEEK_STEP_t eRadioSeekStep = tcradioservice_getSeekStep();
	RET ret = eRET_OK;
	uint32 uiSendMsg[MSGQ_DATA_LENGTH] = {0,};
	int32 retValid;

	if(uiSeek5msCnt)	uiSeek5msCnt--;

	switch(eRadioSeekStep)
	{
		case eSEEK_STEP_START:
			tcradiohal_setMute(1, 0);		//Mute On
#ifdef USE_HDRADIO
			if(ret == eRET_OK && tcradioservice_isHdRadio() == YES) {
				// HD Radio
				stTC_HDR_TUNE_TO_t tuneTo1;
				if(stRadioService.curBand == eRADIO_FM_MODE) {
					tuneTo1.band = eTC_HDR_FM_BAND;
				}
				else {
					tuneTo1.band = eTC_HDR_AM_BAND;
				}
				tuneTo1.freq = stRadioService.curFreq;
				tuneTo1.iqsamplerate = tcradioservice_getIqSampleRate(eRADIO_ID_PRIMARY);
				tcradioservice_setHdrTune(eTC_HDR_ID_MAIN, tuneTo1);
			}
#endif
			eRadioSeekStep = eSEEK_STEP_SET_FREQ;

		case eSEEK_STEP_SET_FREQ :
			if(tcradioservice_getSeekMode() == eRADIO_SEEK_MAN_DOWN || tcradioservice_getSeekMode() == eRADIO_SEEK_AUTO_DOWN) {
				tcradioservice_getFreqCounter((eRADIO_MOD_MODE_t)stRadioService.curBand, &stRadioService.curFreq, DN);
			}
			else {
				tcradioservice_getFreqCounter((eRADIO_MOD_MODE_t)stRadioService.curBand, &stRadioService.curFreq, UP);
			}

			tcradiohal_setTune(stRadioService.curBand, stRadioService.curFreq, (uint32)eTUNER_DRV_TUNE_NORMAL, (uint32)eRADIO_ID_PRIMARY);

			eRadioSeekStep = eSEEK_STEP_PRECHK_QDATA;

			if(tcradioservice_getSeekMode() == eRADIO_SEEK_MAN_UP || tcradioservice_getSeekMode() == eRADIO_SEEK_MAN_DOWN) {
				eRadioSeekStep = eSEEK_STEP_STOP;
			}
			else if(tcradioservice_getSeekMode() == eRADIO_SEEK_SCAN_STATION || tcradioservice_getSeekMode() == eRADIO_SEEK_SCAN_PI) {
				eRadioSt = eRADIO_STS_DOING_NOTIFY;
			}
			else {
				;
			}
			break;

		case eSEEK_STEP_PRECHK_QDATA:
			tcradiohal_getQuality(stRadioService.curBand, (stTUNER_QUALITY_t *)&stRadioService.stSchQdata, eRADIO_ID_PRIMARY);

			if(pfnOnPrecheckSeekQual != NULL) {
				retValid = (*pfnOnPrecheckSeekQual)(stRadioService.curBand, stRadioService.stSchQdata);

			}
			else {
				retValid = tcradioservice_precheckSignalQuality(stRadioService.stSchQdata);
			}

			if(retValid == 0) {	// ok
				if(tcradioservice_isFmBand() != 0 || tcradioservice_isDabBand() != 0) {
					uiSeek5msCnt = 90 / SERVICE_THREAD_TIME_INTERVAL;
				}
				else {
					uiSeek5msCnt = 60 / SERVICE_THREAD_TIME_INTERVAL;
				}

				eRadioSeekStep = eSEEK_STEP_CHK_QDATA_STATUS;
			}
			else {
				if(stRadioService.curFreq == stRadioService.curStartFreq) {
					eRadioSeekStep = eSEEK_STEP_STOP;
					if(tcradioservice_getSeekMode() == eRADIO_SEEK_SCAN_PI) {
						if(stScanPiFreq.uiFreq != -1) {
							stRadioService.curFreq = stScanPiFreq.uiFreq;
							RSRV_DBG("<<<<<<<< Radio RDS PI Found!! PI[%02xh], FREQ[%d], Quality[%d] >>>>>>>>>>\n", stScanPiFreq.uiPi, stScanPiFreq.uiFreq, stScanPiFreq.iQuality);
						}
						else {
							RSRV_DBG("<<<<<<<< Radio RDS PI didn't find !!! >>>>>>>>>>\n");
						}
					}
				}
				else {
					eRadioSeekStep = eSEEK_STEP_SET_FREQ;
				}

				if(tcradioservice_getSeekMode() != eRADIO_SEEK_SCAN_STATION && tcradioservice_getSeekMode() != eRADIO_SEEK_SCAN_PI) {
					eRadioSt = eRADIO_STS_DOING_NOTIFY;			// for returning current frequency quality values
				}
			}

			break;

		case eSEEK_STEP_CHK_QDATA_STATUS:
			if (uiSeek5msCnt <= 0) {
				eRadioSeekStep = eSEEK_STEP_CHK_QDATA;
			}
			else {
				tcradiohal_getQuality(stRadioService.curBand, (stTUNER_QUALITY_t *)&stRadioService.stSchQdata, eRADIO_ID_PRIMARY);
				if(tcradioservice_checkSignalQualityStatus(stRadioService.stSchQdata) == 0) {
					eRadioSeekStep = eSEEK_STEP_CHK_QDATA;
				}
			}
			break;

		case eSEEK_STEP_CHK_QDATA :
			tcradiohal_getQuality(stRadioService.curBand, (stTUNER_QUALITY_t *)&stRadioService.stSchQdata, eRADIO_ID_PRIMARY);

			if(pfnOnCheckSeekQual != NULL) {
				retValid = (*pfnOnCheckSeekQual)(stRadioService.curBand, stRadioService.stSchQdata);
			}
			else {
				retValid = tcradioservice_checkSignalQuality(stRadioService.stSchQdata);
			}

			if(retValid == 0) {	// ok
				if(tcradioservice_getSeekMode() == eRADIO_SEEK_SCAN_STATION) {
					tcradioservice_makeStationList();
					if(stRadioService.curFreq == stRadioService.curStartFreq) {
						eRadioSeekStep = eSEEK_STEP_STOP;
					}
					else {
						stRadioService.curSeekResult++;
						eRadioSeekStep = eSEEK_STEP_SET_FREQ;
					}
				}
				else if(tcradioservice_getSeekMode() == eRADIO_SEEK_SCAN_PI) {
					if(stRadioService.curFreq == stRadioService.curStartFreq) {
						eRadioSeekStep = eSEEK_STEP_STOP;
						if(stScanPiFreq.uiFreq != (uint32)-1) {
							stRadioService.curFreq = stScanPiFreq.uiFreq;
							RSRV_DBG("<<<<<<<< Radio RDS PI Found!! PI[%02xh], FREQ[%d], Quality[%d] >>>>>>>>>>\n", stScanPiFreq.uiPi, stScanPiFreq.uiFreq, stScanPiFreq.iQuality);
						}
						else {
							RSRV_DBG("<<<<<<<< Radio RDS PI didn't find !!! >>>>>>>>>>\n");
						}
					}
					else {
						eRadioSeekStep = eSEEK_STEP_CHK_PI;
						uiSeek5msCnt = 100 / SERVICE_THREAD_TIME_INTERVAL;
						uiCheckPiCnt = 3;
					}
				}
				else {
					stRadioService.curSeekResult = 1;
					eRadioSeekStep = eSEEK_STEP_STOP;
				}
			}
			else {	// nok
				if(stRadioService.curFreq == stRadioService.curStartFreq) {
					eRadioSeekStep = eSEEK_STEP_STOP;
					if(tcradioservice_getSeekMode() == eRADIO_SEEK_SCAN_PI) {
						if(stScanPiFreq.uiFreq != -1) {
							stRadioService.curFreq = stScanPiFreq.uiFreq;
							RSRV_DBG("<<<<<<<< Radio RDS PI Found!! PI[%02xh], FREQ[%d], Quality[%d] >>>>>>>>>>\n", stScanPiFreq.uiPi, stScanPiFreq.uiFreq, stScanPiFreq.iQuality);
						}
						else {
							RSRV_DBG("<<<<<<<< Radio RDS PI didn't find !!! >>>>>>>>>>\n");
						}
					}
				}
				else {
					eRadioSeekStep = eSEEK_STEP_SET_FREQ;
				}
			}

			if(tcradioservice_getSeekMode() != eRADIO_SEEK_SCAN_STATION && tcradioservice_getSeekMode() != eRADIO_SEEK_SCAN_PI)
				eRadioSt = eRADIO_STS_DOING_NOTIFY;			// for returning current frequency quality values

			break;

		case eSEEK_STEP_CHK_PI:
			if (uiSeek5msCnt <= 0) {
				if(tcradioservice_findBestStation() == 0) {
					stRadioService.curSeekResult = 1;
					eRadioSeekStep = eSEEK_STEP_SET_FREQ;
				//	RSRV_DBG("<<<<<<<<<<<<<<<<< Found RDS Frequency >>>>>>>>>>>>>>>>> : uiSeek5msCnt[%d], uiCheckPiCnt[%d]\n", uiSeek5msCnt, uiCheckPiCnt);
				}
				else {
					if(uiCheckPiCnt) {
						uiSeek5msCnt = 100 / SERVICE_THREAD_TIME_INTERVAL;
						uiCheckPiCnt--;
					}
					else {
						eRadioSeekStep = eSEEK_STEP_SET_FREQ;
					}
				}
			}
			break;

		case eSEEK_STEP_STOP:
			if(tcradioservice_getSeekMode() == eRADIO_SEEK_SCAN_STATION) {
				fStationListTx = 1;
			}
			tcradioservice_stopSeek(1);
			eRadioSt = eRADIO_STS_OK_NOTIFY;
			// Mute Off
			break;

		default:	// On Air
			if(tcradioservice_getSeekMode() != eRADIO_SEEK_STOP) {
				tcradioservice_setSeekMode(eRADIO_SEEK_STOP);
				eRadioSt = eRADIO_STS_OK_NOTIFY;
			}
			else {
				eRadioSt = eRADIO_STS_OK;
			}
			break;
	}

	if(tcradioservice_getSeekStep() == eRadioSeekStep) {
		tcradioservice_setSeekStep(eSEEK_STEP_ON_AIR);
	}

	switch(eRadioSt) {
		/* Job End -> No Notify */
		case eRADIO_STS_WAIT:
			break;

		/* Job Good Complete */
		case eRADIO_STS_OK:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eRADIO_STS_OK_NOTIFY:
			uiSendMsg[0] = stRadioService.curBand;
			uiSendMsg[1] = stRadioService.curFreq;
			uiSendMsg[2] = tcradioservice_getSeekMode();
			uiSendMsg[3] = stRadioService.curSeekResult;
			uiSendMsg[4] = fStationListTx;
			uiSendMsg[5] = 0;   // Service Scan Result --> 0
			tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_NOTIFY_SEEK_MODE, uiSendMsg, pNULL, ret);
			break;

		/* Job Continue */
		case eRADIO_STS_DOING:
			tcradioservice_setSeekStep(eRadioSeekStep);
			break;

		/* Job Continue -> Information Dynamic Notify */
		case eRADIO_STS_DOING_NOTIFY :
			tcradioservice_setSeekStep(eRadioSeekStep);
			uiSendMsg[0] = stRadioService.curBand;
			uiSendMsg[1] = stRadioService.curFreq;
			uiSendMsg[2] = tcradioservice_getSeekMode();
			tcradio_memcpy(uiSendMsg+3, &stRadioService.stSchQdata, sizeof(stRadioService.stSchQdata));
			tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_NOTIFY_SEEK_MODE, uiSendMsg, pNULL, ret);
			break;

		/* Job Continue -> Error Notify */
		case eRADIO_STS_DOING_ERROR_NOTIFY :
			tcradioservice_setSeekStep(eRadioSeekStep);	/* No break because of doing below function */

		/* Job Error -> Return Error */
		case eRADIO_STS_ERROR:
			break;

		/* Return Error */
		default:
			break;
	}
}

void tcradioservice_stopSeek(uint32 retune)
{
	if(retune != 0) {
		RET ret;
		if(tcradioservice_getSeekMode() == eRADIO_SEEK_SCAN_STATION) {
			stRadioService.curFreq = stRadioService.curStartFreq;
		}
		ret = tcradiohal_setTune(stRadioService.curBand, stRadioService.curFreq, eTUNER_DRV_TUNE_NORMAL, eRADIO_ID_PRIMARY);
#ifdef USE_HDRADIO
		if(ret == eRET_OK && tcradioservice_isHdRadio() == YES) {
			// HD Radio
			if(tcradioservice_is2ndTunerMRC()) {
				// With MRC
				stTC_HDR_TUNE_TO_t tuneTo1, tuneTo2;
				if(stRadioService.curBand == eRADIO_FM_MODE) {
					tuneTo1.band = tuneTo2.band = eTC_HDR_FM_BAND;
				}
				else {
					tuneTo1.band = tuneTo2.band = eTC_HDR_AM_BAND;
				}
				tuneTo1.freq = tuneTo2.freq = stRadioService.curFreq;
				tuneTo1.iqsamplerate = tcradioservice_getIqSampleRate(eRADIO_ID_PRIMARY);
				tcradioservice_setHdrTune(eTC_HDR_ID_MAIN, tuneTo1);
				tcradioservice_setHdrProgramNumber(0);

				ret = tcradiohal_setTune(stRadioService.curBand, stRadioService.curFreq, eTUNER_DRV_TUNE_NORMAL, eRADIO_ID_SECONDARY);
				if(ret == eRET_OK) {
					tuneTo2.iqsamplerate = tcradioservice_getIqSampleRate(eRADIO_ID_SECONDARY);
					tcradioservice_setHdrTune(eTC_HDR_ID_MRC, tuneTo2);
				}
			}
			else {
				// Without MRC (eTC_HDR_TYPE_HD_1p0 or eTC_HDR_TYPE_HD_1p5)
				stTC_HDR_TUNE_TO_t tuneTo1;
				if(stRadioService.curBand == eRADIO_FM_MODE) {
					tuneTo1.band = eTC_HDR_FM_BAND;
				}
				else {
					tuneTo1.band = eTC_HDR_AM_BAND;
				}
				tuneTo1.freq = stRadioService.curFreq;
				tuneTo1.iqsamplerate = tcradioservice_getIqSampleRate(eRADIO_ID_PRIMARY);
				tcradioservice_setHdrTune(eTC_HDR_ID_MAIN, tuneTo1);
				tcradioservice_setHdrProgramNumber(0);
			}
		}
#endif
	}

	tcradioservice_setSeekMode(eRADIO_SEEK_STOP);
	tcradiohal_setMute(0, 0);
#ifdef USE_HDRADIO
//	if(tcradioservice_isHdRadio() == YES) {
//		tcradioservice_setHdrMute(OFF);
//	}
#endif
}

int32 tcradioservice_checkSignalQualityStatus(stRADIO_QUALITY_t qdata)
{
	int32 ret = -1;

	if(qdata.type== eRADIO_TYPE1)
	{
		stRADIO_TYPE1_QUALITY_t temp_qdata;
		tcradio_memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));
		if(tcradioservice_isFmBand() == 0) {
			if(qdata.qual.fm.Qvalue[8] & 0x00000030) {
				ret = 0;
			}
		}
		else if(tcradioservice_isAmBand() == 0) {
			if(qdata.qual.am.Qvalue[5] & 0x00000030) {
				ret = 0;
			}
		}
		else if(tcradioservice_isDabBand() == 0) {
			if(qdata.qual.am.Qvalue[5] & 0x00000010) {
				ret = 0;
			}
		}
		else {
			;
		}
	}
	else if(qdata.type == eRADIO_TYPE2)
	{
		stRADIO_TYPE2_QUALITY_t temp_qdata;
		tcradio_memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));
		if(tcradioservice_isFmBand() == 0) {
			if((temp_qdata.fm.Status & 0x000003ff) >= 320) {
				ret = 0;
			}
		}
		else {
			if((temp_qdata.am.Status & 0x000003ff) >= 320) {
				ret = 0;
			}
		}
	}
	else
	{
		ret = 0;
	}
	return ret;
}

int32 tcradioservice_precheckSignalQuality(stRADIO_QUALITY_t qdata)
{
	int32 ret=-1, rssi=0;

	if(qdata.type == eRADIO_TYPE1)
	{
		stRADIO_TYPE1_QUALITY_t temp_qdata;
		tcradio_memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(tcradioservice_isFmBand() == 0) {
			rssi = (int32)temp_qdata.fm.Rssi;
			if(rssi >= 20)
			{
				ret = 0;
			}
		}
		else if(tcradioservice_isAmBand() == 0) {
			rssi = (int32)temp_qdata.am.Rssi;
			if(rssi >= 38)
			{
				ret = 0;
			}
		}
		else if(tcradioservice_isDabBand() == 0) {
			rssi = (int32)temp_qdata.am.Rssi;
			if(rssi >= -107)
			{
				ret = 0;
			}
		}
		else {
			;
		}
	}
	else if(qdata.type == eRADIO_TYPE2)
	{
		stRADIO_TYPE2_QUALITY_t temp_qdata;
		tcradio_memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(tcradioservice_isFmBand() == 0) {
			rssi = (int32)temp_qdata.fm.Rssi;
			if(rssi >= 280)
			{
				ret = 0;
			}
		}
		else if(tcradioservice_isAmBand() == 0) {
			rssi = (int32)temp_qdata.am.Rssi;
			if(rssi >= 630)
			{
				ret = 0;
			}

		}
		else {
			;
		}
	}
	else
	{
		stRADIO_TYPE0_QUALITY_t temp_qdata;
		tcradio_memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));
		;
	}
	return ret;
}

int32 tcradioservice_checkSignalQuality(stRADIO_QUALITY_t qdata)
{
	int32 ret=-1;
	int32 rssi=0, snr=0, offs=0, sqi=0;
	uint32 usn=0, mpth=0, noise=0, detect=0;

	if(qdata.type == eRADIO_TYPE1)
	{
		stRADIO_TYPE1_QUALITY_t temp_qdata;
		tcradio_memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(tcradioservice_isFmBand() == 0) {
			rssi = (int32)temp_qdata.fm.Rssi;
			snr = (int32)temp_qdata.fm.Snr;
			offs = (int32)temp_qdata.fm.Offs;
			if(rssi >= 20 && snr > 4 && offs > -6 && offs < 6)
			{
				ret = 0;
			}
		}
		else if(tcradioservice_isAmBand() == 0) {
			rssi = (int32)temp_qdata.am.Rssi;
			snr = (int32)temp_qdata.am.Snr;
			offs = (int32)temp_qdata.am.Offs;
			if(rssi >= 38 && snr > 6 && offs > -6 && offs < 6)
			{
				ret = 0;
			}
		}
		else if(tcradioservice_isDabBand() == 0) {
			rssi = (int32)temp_qdata.dab.Rssi;
			sqi = (int32)temp_qdata.dab.Sqi;
			detect = (int32)temp_qdata.dab.Detect;
			if(rssi >= -107 && sqi > 8 && detect > 0)
			{
				ret = 0;
			}
		}
		else {
			;
		}
	}
	else if(qdata.type == eRADIO_TYPE2)
	{
		stRADIO_TYPE2_QUALITY_t temp_qdata;
		tcradio_memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(tcradioservice_isFmBand() == 0) {
			rssi = (int32)temp_qdata.fm.Rssi;
			offs = (int32)temp_qdata.fm.Offs;
			usn = temp_qdata.fm.Usn;
			mpth = temp_qdata.fm.Mpth;
			if(rssi >= 280 && offs > -100 && offs < 100 && usn < 120 && mpth < 200)
			{
				ret = 0;
			}
		}
		else if(tcradioservice_isAmBand() == 0) {
			rssi = (int32)temp_qdata.am.Rssi;
			offs = (int32)temp_qdata.am.Offs;
			noise = temp_qdata.am.Hfn;
			if(rssi >= 630 && offs > -50 && offs < 50 && noise < 100)
			{
				ret = 0;
			}
		}
		else {
			;
		}
	}
	else
	{
		stRADIO_TYPE0_QUALITY_t temp_qdata;
		tcradio_memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));
	}
	return ret;
}

void tcradioservice_saveAllPresetMemory(void)
{
	int32 i;

	for(i=0; i < _MaxPresetNum; i++) {
		if(stRadioList[i].uiFreq != 0) {
			stRadioService.currentBandPresetFreq[stRadioService.curBand][i] = stRadioList[i].uiFreq;
		}
	}
	stRadioService.curFreq = stRadioService.currentBandPresetFreq[stRadioService.curBand][1];
	// Add preset list update function on here

#if 1		// for degugging
	for (i=0; i < _MaxPresetNum; i++) {
		RSRV_DBG(" Current Band Preset %d = %d Mhz\n",  i+1, stRadioService.currentBandPresetFreq[stRadioService.curBand][i]/100);
	}
#endif
}

void tcradioservice_savePresetMemory(uint8 pnum)
{
	if (pnum >= _MaxPresetNum || stRadioService.currentBandPresetFreq[stRadioService.curBand][pnum] == stRadioService.curFreq)
		return;

	if (tcradioservice_getMainMode() == eRADIO_EVT_SET_SEEK) {
		tcradioservice_stopSeek(1);
	}

	stRadioService.currentBandPresetFreq[stRadioService.curBand][pnum] = stRadioService.curFreq;
	// Add preset list update function on here
}

#if 0
void tcradioservice_sortPresetMemory(void)
{
	int32 i, j;

	if (tcradioservice_isFmBand() == 0) {
		for(i = 0; i < _MaxPresetNum; i++) {
			if (stRadioService.stSchQdata.fm.Rssi > stRadioList[i].stQdata.fm.Rssi) {
				for(j=_MaxPresetNum-1; j > i; j--) {
					stRadioList[j].uiFreq = stRadioList[j-1].uiFreq;
					tcradio_memcpy(&stRadioList[j].stQdata, &stRadioList[j-1].stQdata, sizeof(stRADIO_QUALITY_t));
				}
				stRadioList[i].uiFreq = stRadioService.curFreq;
				tcradio_memcpy(&stRadioList[i].stQdata, &stRadioService.stSchQdata, sizeof(stRADIO_QUALITY_t));
				break;
			}
		}
	}
	else if (tcradioservice_isAmBand() == 0) {
		for(i = 0; i < _MaxPresetNum; i++) {
			if (stRadioService.stSchQdata.am.Rssi > stRadioList[i].stQdata.am.Rssi) {
				for(j=_MaxPresetNum-1; j > i; j--) {
					stRadioList[j].uiFreq = stRadioList[j-1].uiFreq;
					tcradio_memcpy(&stRadioList[j].stQdata, &stRadioList[j-1].stQdata, sizeof(stRADIO_QUALITY_t));
				}
				stRadioList[i].uiFreq = stRadioService.curFreq;
				tcradio_memcpy(&stRadioList[i].stQdata, &stRadioService.stSchQdata, sizeof(stRADIO_QUALITY_t));
				break;
			}
		}
	}
	else {
		;
	}
}

void tcradioservice_loadPresetMemory(uint8 pnum)
{
	uint8 ip;

	if (pnum >= _MaxPresetNum || stRadioService.currentBandPresetFreq[stRadioService.curBand][pnum] == stRadioService.curFreq)
		return;

	if (stRadioService.eMainMode == eRADIO_EVT_SET_SEEK) {
		stRadioService.eMainMode = eRADIO_EVT_NULL;
		stRadioService.eSeekMode = eRADIO_SEEK_STOP;
	}

	stRadioService.curFreq = stRadioService.currentBandPresetFreq[stRadioService.curBand][pnum];
	tcradiohal_setTune(stRadioService.curBand, stRadioService.curFreq, NULL, NULL);
	// Add current frequency update notification on here
}
#endif

void tcradioservice_makeStationList(void)
{
	if(uiStationListCounter < _AmFmDBSize) {
		_stStationList[uiStationListCounter].uiFreq = stRadioService.curFreq;
		tcradio_memcpy(&_stStationList[uiStationListCounter].stQdata, &stRadioService.stSchQdata, sizeof(stRADIO_QUALITY_t));
		uiStationListCounter++;
	}
	else {
		RSRV_DBG("%s : The station list is already full!! \n", __func__);
	}
}

int32 tcradioservice_findBestStation(void)
{
	uint16 pi = tcrds_getPi();
	uint32 i, uiFreq;
	int32 ret = -1, iQuality = 0;

	if(pi != NO_PI) {
		for(i=0 ; i<128; i++) {
			if(scanPiList[i] != (uint32)NULL && scanPiList[i] == (uint32)pi) {
				if(stRadioService.stSchQdata.type == eTUNER_IC_S0) {
					stRADIO_TYPE1_QUALITY_t *temp_qdata = (stRADIO_TYPE1_QUALITY_t *)&stRadioService.stSchQdata.qual;
					iQuality = (int32)temp_qdata->fm.Rssi;
				}
				else if(stRadioService.stSchQdata.type == eTUNER_IC_X0) {
					stRADIO_TYPE2_QUALITY_t *temp_qdata = (stRADIO_TYPE2_QUALITY_t *)&stRadioService.stSchQdata.qual;
					iQuality = (int32)temp_qdata->fm.Rssi;
				}
				else {
					stRADIO_TYPE0_QUALITY_t *temp_qdata = (stRADIO_TYPE0_QUALITY_t *)&stRadioService.stSchQdata.qual;
					iQuality = (int32)temp_qdata->fm.Rssi;
				}

				if(stScanPiFreq.iQuality < iQuality) {
					stScanPiFreq.uiPi = scanPiList[i];
					stScanPiFreq.uiFreq = stRadioService.curFreq;
					stScanPiFreq.iQuality = iQuality;
					ret = 0;
				}
			}
		}
	}

	return ret;
}

void tcradioservice_initSeek(eRADIO_SEEK_MODE_t nextSeekMode)
{
	uint32 cf = stRadioService.curFreq;
	uint32 sf= 0, ef = 0, step = 0;
	int32 chkret;

	tcradioservice_setSeekStep(eSEEK_STEP_START);
	stRadioService.curSeekResult = 0;

	chkret = tcradioservice_checkValidFreqAccordingToConfig(stRadioService.curBand, cf);
	if(chkret != 0) {
		if(stRadioService.curBand == eRADIO_AM_MODE) {
			sf = stRadioService.am.startFreq;
			ef = stRadioService.am.endFreq;
			step = stRadioService.am.step;
		}
		else if(stRadioService.curBand == eRADIO_FM_MODE) {
			sf = stRadioService.fm.startFreq;
			ef = stRadioService.fm.endFreq;
			step = stRadioService.fm.step;
		}
		else if(stRadioService.curBand == eRADIO_DAB_MODE) {
			sf = stRadioService.dab.startIndex;
			ef = stRadioService.dab.endIndex;
			step = stRadioService.dab.step;
		}
		else {

		}

		if(chkret == -2) {
			if(nextSeekMode == eRADIO_SEEK_MAN_DOWN || nextSeekMode == eRADIO_SEEK_AUTO_DOWN) {
				cf += step - ((cf-sf)%step);
			}
			else {
				cf -= ((cf-sf)%step);
			}
		}
		else {
			if(cf < sf) {
				cf = sf;
			}
			else {
				cf = ef;
			}
		}

		stRadioService.curFreq = cf;
	}

	stRadioService.curStartFreq = stRadioService.curFreq;

	if(nextSeekMode == eRADIO_SEEK_STOP) {
		tcradioservice_setSeekStep(eSEEK_STEP_STOP);
	}
	else if(nextSeekMode < eRADIO_SEEK_END) {
	#ifdef USE_HDRADIO
	//	if(tcradioservice_isHdRadio() == YES) {
	//		tcradioservice_setHdrMute(ON);
	//	}
	#endif
		if(nextSeekMode == eRADIO_SEEK_SCAN_STATION) {
			fStationListTx = 0;
			uiStationListCounter = 0;
		}
		if(nextSeekMode == eRADIO_SEEK_SCAN_PI) {
			stScanPiFreq.uiFreq = (uint32)-1;
			stScanPiFreq.iQuality = 0;
		}
		tcradioservice_setSeekMode(nextSeekMode);
	}
	else {
		tcradioservice_setSeekMode(eRADIO_SEEK_END);
	}
}

int32 tcradioservice_isFmBand(void)
{
	int32 ret = -1;
	if(stRadioService.curBand == eRADIO_FM_MODE)
		ret = 0;
	return ret;
}

int32 tcradioservice_isAmBand(void)
{
	int32 ret = -1;
	if(stRadioService.curBand == eRADIO_AM_MODE)
		ret = 0;
	return ret;
}

int32 tcradioservice_isDabBand(void)
{
	int32 ret = -1;
	if(stRadioService.curBand == eRADIO_DAB_MODE)
		ret = 0;
	return ret;
}

int32 tcradioservice_isSeeking(void)
{
	int32 ret = -1;
	if(tcradioservice_getSeekMode() > eRADIO_SEEK_STOP && tcradioservice_getSeekMode() < eRADIO_SEEK_END)
		ret = 0;
	return ret;
}

uint32 tcradioservice_getCurrentFrequency(void)
{
	return stRadioService.curFreq;
}

uint32 tcradioservice_getNumberOfTuners(void)
{
	return stTunerConfig.numTuners;
}

