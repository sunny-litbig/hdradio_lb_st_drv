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
#include "tcradio_bg.h"
#include "tcradio_msgq.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
stRADIO_BG_t stRadioBG;
stRADIO_CONFIG_t stBGTunerConfig;

static pthread_t bgMainThreadID = (pthread_t)NULL;

uint32 uiBGStateCnt = 0;
uint32 uiBGRestartCnt = 0;

uint32 fBGStationListTx = 0;
uint32 uiBGStationListCounter = 0;
stRADIO_LIST_t _stBGStationList[_AmFmDBSize];

stMsgBuf_t stBGRcvMsgQ;

uint32 scanPiList[128];

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
void tcradiobg_initVariable(void);
RET tcradiobg_getFreqCounter(eRADIO_MOD_MODE_t mod_mode, uint32 *freq, uint8 dir);
void tcradiobg_stopSeek(uint32 retune);
int32 tcradiobg_checkSignalQualityStatus(stRADIO_QUALITY_t qdata);
int32 tcradiobg_precheckSignalQuality(stRADIO_QUALITY_t qdata);
int32 tcradiobg_checkSignalQuality(stRADIO_QUALITY_t qdata);
int32 tcradiobg_isFmBand(void);
int32 tcradiobg_isAmBand(void);
static void tcradiobg_eventHandler(void);
void tcradiobg_initSeek(eRADIO_SEEK_MODE_t nextSeekMode);
void tcradiobg_setValueBGRestart(void);
void tcradiobg_makeStationList(void);
void *tcradiobg_mainThread(void *arg);
static void tcradiobg_stateHandler(void);
int32 tcradiobg_checkValidFreqAccordingToConfig(eRADIO_MOD_MODE_t mod_mode, uint32 freq);
RET tcradiobg_checkValidBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 start_freq, uint32  end_freq, uint32 step);
void tcradiobg_appMessageParser(stMsgBuf_t *pstMsg);

/***************************************************
*			function definition				*
****************************************************/
RET tcradiobg_init(void)
{
	RET ret = eRET_OK;
    
    RBG_DBG("[%s] \n", __func__);

	tcradiobg_initVariable();
	if(bgMainThreadID == (pthread_t)NULL) {
		ret = tcradio_createThread(&bgMainThreadID, &tcradiobg_mainThread, "TCRADIO_BG", eRADIO_SCHED_OTHER, 00, pNULL);
		if(ret != eRET_OK) {
			RBG_ERR("[%s:%d] Can not make Radio Main Thread!!!\n", __func__, __LINE__);
			ret = eRET_NG_CREATE_THREAD;
			goto error_init;
		}
	}
	else {
		RBG_WRN("[%s:%d] Already radio service thread was created!!!\n", __func__, __LINE__);
	}

	ret= tcradiobg_messageInit();

error_init:

	return ret;
}

RET tcradiobg_deinit(void)
{
	RET ret;
    
    RBG_DBG("[%s] \n", __func__);

	stRadioBG.fBGThreadRunning = 0;
	bgMainThreadID = (pthread_t)NULL;
	ret = tcradio_joinThread(bgMainThreadID, (void**)NULL);
	return ret;
}

RET tcradiobg_close(void)
{
	RET ret;
    
    RBG_DBG("[%s] \n", __func__);

	if(stRadioBG.fBGRadioOpened) {
		tcradiobg_mutexLock();
		stRadioBG.fBGRadioOpened = 0;
		tcradiobg_mutexUnlock();
	}
	else {
		ret = eRET_NG_NOT_OPEN;
	}
	return ret;
}

void tcradiobg_initVariable(void)
{
	uint32 i;
    
    RBG_DBG("[%s] \n", __func__);


	for(i=0; i<_AmFmDBSize; i++) {
		tcradio_memset(&_stBGStationList[i], 0x00, sizeof(stRADIO_LIST_t));
	}

	stRadioBG.curBand = (uint32)(-1);
	stRadioBG.curFreq = (uint32)(-1);
#if 0
	stRadioBG.fm.startFreq = 87500;
	stRadioBG.fm.endFreq = 108000;
	stRadioBG.fm.step = 100;

	stRadioBG.am.startFreq = 531;
	stRadioBG.am.endFreq = 1620;
	stRadioBG.am.step = 9;
#else
	stRadioBG.fm.startFreq = 87500;
	stRadioBG.fm.endFreq = 107900;
	stRadioBG.fm.step = 200;

	stRadioBG.am.startFreq = 530;
	stRadioBG.am.endFreq = 1710;
	stRadioBG.am.step = 10;
#endif
}

RET tcradiobg_checkConfig(stRADIO_CONFIG_t *config)
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

void tcradiobg_setMainMode(eRADIO_BG_EVENT_t mode)
{
	stRadioBG.eMainMode = mode;
}

eRADIO_BG_EVENT_t tcradiobg_getMainMode(void)
{
	return stRadioBG.eMainMode;
}

void tcradiobg_setState(eRADIO_BG_STATE_t bgstate)
{
	stRadioBG.eBGState = bgstate;
}

eRADIO_BG_STATE_t tcradiobg_getState(void)
{
	return stRadioBG.eBGState;
}

void tcradiobg_setSeekMode(eRADIO_SEEK_MODE_t mode)
{
	stRadioBG.eSeekMode = mode;
}

eRADIO_SEEK_MODE_t tcradiobg_getSeekMode(void)
{
	return stRadioBG.eSeekMode;
}

void *tcradiobg_mainThread(void *arg)
{
	RET ret = eRET_OK;
	stMsgBuf_t stRecivedMessage = {0,};

    RBG_DBG("%s\n", __func__);

	prctl(PR_SET_NAME, "TCRADIO_BG",0,0,0);

	stRadioBG.fBGThreadRunning = 1;
	while(stRadioBG.fBGThreadRunning > 0) {
		tcradiobg_getMessage(&stRecivedMessage);
		if(stRecivedMessage.fNewMsg == eNEW_MSG_EXIST) {
			switch(stRecivedMessage.uiSender) {
				case eSENDER_ID_APP:
					tcradiobg_appMessageParser(&stRecivedMessage);
					break;

				default:
					break;
			}
		}
        tcradiobg_eventHandler();		// Command Message Event Handler
		tcradiobg_stateHandler();
		tcradio_mssleep(BG_THREAD_TIME_INTERVAL);
	}

	stRadioBG.fBGThreadRunning = -1;
	tcradiobg_messageDeinit();

	return ((void*)0);
}

void tcradiobg_appMessageParser(stMsgBuf_t *pstMsg)
{
	uint32 i;
	stRADIO_CONFIG_t *radioconf;
    

	stBGRcvMsgQ.uiMode = pstMsg->uiMode;
	tcradio_memcpy(stBGRcvMsgQ.uiData, &pstMsg->uiData, MSGQ_DATA_LENGTH*sizeof(uint32));

	switch((eRADIO_BG_CMD_t)pstMsg->uiMode) {
		case eRADIO_BG_CMD_OPEN:
            RBG_DBG("[%s] eRADIO_BG_CMD_OPEN \n", __func__);

			stBGTunerConfig.area = (eRADIO_CONF_AREA_t)stBGRcvMsgQ.uiData[0];
			stBGTunerConfig.initMode = (eRADIO_MOD_MODE_t)stBGRcvMsgQ.uiData[1];
			stBGTunerConfig.initFreq = stBGRcvMsgQ.uiData[2];
			stBGTunerConfig.numTuners = stBGRcvMsgQ.uiData[3];
			stBGTunerConfig.fPhaseDiversity = stBGRcvMsgQ.uiData[4];
			stBGTunerConfig.fIqOut = stBGRcvMsgQ.uiData[5];
			stBGTunerConfig.audioSamplerate = stBGRcvMsgQ.uiData[6];
			stBGTunerConfig.fExtAppCtrl = stBGRcvMsgQ.uiData[7];
			stBGTunerConfig.sdr = stBGRcvMsgQ.uiData[8];
			stBGTunerConfig.hdType = stBGRcvMsgQ.uiData[9];

			tcradiobg_setMainMode(eRADIO_BG_EVT_OPEN);
			break;

		case eRADIO_BG_CMD_DEINIT:
            RBG_DBG("[%s] eRADIO_BG_CMD_DEINIT \n", __func__);

			tcradiobg_setMainMode(eRADIO_BG_EVT_DEINIT);
			break;

		case eRADIO_BG_CMD_START:
            RBG_DBG("[%s] eRADIO_BG_CMD_START \n", __func__);

            stRadioBG.curBand = pstMsg->uiData[1];
			tcradiobg_initSeek((eRADIO_SEEK_MODE_t)pstMsg->uiData[0]);
			tcradiobg_setMainMode(eRADIO_BG_EVT_SET_SEEK);
			break;

		case eRADIO_BG_CMD_STOP:
            RBG_DBG("[%s] eRADIO_BG_CMD_STOP \n", __func__);
            tcradiobg_stopBG();
			tcradiobg_setMainMode(eRADIO_BG_EVT_STOP);
			break;

		default:
			break;
	}
}

static void tcradiobg_eventHandler(void)
{
	eRADIO_BG_STS_t eRadioSt = eRADIO_BG_STS_OK;
	eRADIO_BG_EVENT_t eRadioNowExeMode;
	RET ret = eRET_OK;
	uint32 uiSendMsg[MSGQ_DATA_LENGTH] = {0,}, i, j;

	eRadioNowExeMode = tcradiobg_getMainMode();

	switch(eRadioNowExeMode)
	{
		case eRADIO_BG_EVT_OPEN:
            RBG_DBG("[%s] eRADIO_BG_EVT_OPEN \n", __func__);

			ret = tcradiobg_checkConfig(&stBGTunerConfig);
			if(ret == eRET_OK) {
				stTUNER_DRV_CONFIG_t stConfigSet;
				stConfigSet.area = (eTUNER_DRV_CONF_AREA_t)stBGTunerConfig.area;
				stConfigSet.initMode = (eTUNER_DRV_MOD_MODE_t)stBGTunerConfig.initMode;
				stConfigSet.initFreq = stBGTunerConfig.initFreq;
				stConfigSet.numTuners = stBGTunerConfig.numTuners;
				stConfigSet.fPhaseDiversity = stBGTunerConfig.fPhaseDiversity;
				stConfigSet.fIqOut = stBGTunerConfig.fIqOut;
				stConfigSet.audioSamplerate = stBGTunerConfig.audioSamplerate;
				stConfigSet.fExtAppCtrl = stBGTunerConfig.fExtAppCtrl;
				stConfigSet.sdr = (eTUNER_DRV_SDR_t)stBGTunerConfig.sdr;
				stConfigSet.reserved[0] = (uint32)stBGTunerConfig.hdType;
			}

			if(ret == eRET_OK) {
				uiSendMsg[0] = stRadioBG.curBand = stBGTunerConfig.initMode;
				uiSendMsg[1] = stRadioBG.curFreq = stBGTunerConfig.initFreq;
				stRadioBG.fBGRadioOpened = 1;
				eRadioSt = eRADIO_BG_STS_OK_NOTIFY;
			}
			else {
				eRadioSt = eRADIO_BG_STS_ERROR;
			}
			break;

		case eRADIO_BG_EVT_DEINIT:
			tcradiobg_close();
			tcradiobg_deinit();
			break;

		case eRADIO_BG_EVT_SET_SEEK:
            RBG_DBG("[%s] eRADIO_BG_EVT_SET_SEEK : curBand = %d, \n", __func__, stRadioBG.curBand);

			if(stRadioBG.curBand != eRADIO_FM_MODE && stRadioBG.curBand != eRADIO_AM_MODE && stRadioBG.curBand != eRADIO_DAB_MODE) {
				tcradiobg_setState(eRADIO_BG_STATE_ON_AIR);
				tcradiobg_stopSeek(0);
				ret = eRET_NG_NOT_SUPPORT;
				eRadioSt = eRADIO_BG_STS_ERROR;
			}
			break;

		case eRADIO_BG_EVT_STOP:
            RBG_DBG("[%s] eRADIO_BG_EVT_STOP : = seekMode = %d, BGState = %d\n", __func__,
                tcradiobg_getSeekMode(), tcradiobg_getState());
			break;

		default:
			break;
	}

	if(tcradiobg_getMainMode() == eRadioNowExeMode) {
		tcradiobg_setMainMode(eRADIO_BG_EVT_NULL);
	}

#if 0
	switch(eRadioSt) {
		/* Job End -> No Notify */
		case eRADIO_BG_STS_WAIT:
			break;

		/* Job Good Complete */
		case eRADIO_BG_STS_OK:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eRADIO_BG_STS_OK_NOTIFY:
			switch(eRadioNowExeMode) {
				case eRADIO_BG_EVT_OPEN:
                    RBG_DBG("[%s] eRADIO_BG_STS_OK_NOTIFY : eRADIO_BG_EVT_OPEN \n", __func__);

					tcradioapp_sendMessage(eSENDER_ID_BG, eRADIO_NOTIFY_OPEN, uiSendMsg, pNULL, ret);
					break;

				case eRADIO_BG_EVT_SET_SEEK:
					uiSendMsg[0] = stRadioBG.curBand;
					uiSendMsg[1] = stRadioBG.curFreq;
					uiSendMsg[2] = tcradiobg_getSeekMode();
					uiSendMsg[3] = stRadioBG.curBGResult;
                    RBG_DBG("[%s] eRADIO_BG_STS_OK_NOTIFY : eRADIO_BG_EVT_SET_SEEK  (%d) \n", __func__, uiSendMsg[2]);

					tcradioapp_sendMessage(eSENDER_ID_BG, eRADIO_NOTIFY_SEEK_MODE, uiSendMsg, pNULL, ret);
					break;

				default:
					break;
			}
			break;

		/* Job Continue */
		case eRADIO_BG_STS_DOING:
            RBG_DBG("[%s] eRADIO_BG_STS_DOING \n", __func__);

			tcradiobg_setMainMode(eRadioNowExeMode);
			break;

		/* Job Continue -> Information Dynamic Notify */
		case eRADIO_BG_STS_DOING_NOTIFY :
			tcradiobg_setMainMode(eRadioNowExeMode);
			switch(eRadioNowExeMode) {
				case eRADIO_BG_EVT_SET_SEEK:
					uiSendMsg[0] = stRadioBG.curBand;
					uiSendMsg[1] = stRadioBG.curFreq;
					uiSendMsg[2] = tcradiobg_getSeekMode();
                    RBG_DBG("[%s] eRADIO_BG_STS_DOING_NOTIFY : eRADIO_BG_EVT_SET_SEEK  (%d) \n", __func__, uiSendMsg[2]);

					tcradio_memcpy(uiSendMsg+3, &stRadioBG.stSchQdata, sizeof(stRadioBG.stSchQdata));
					break;

				default:
					break;
			}
			tcradioapp_sendMessage(eSENDER_ID_BG, (uint32)eRadioNowExeMode, uiSendMsg, pNULL, (RET)NULL);
			break;

		/* Job Continue -> Error Notify */
		case eRADIO_BG_STS_DOING_ERROR_NOTIFY :
            RBG_DBG("[%s] eRADIO_BG_STS_DOING_ERROR_NOTIFY [%d]\n", __func__, eRadioNowExeMode);

			tcradiobg_setMainMode(eRadioNowExeMode);	/* No break because of doing below function */

		/* Job Error -> Return Error */
		case eRADIO_BG_STS_ERROR:
            RBG_DBG("[%s] eRADIO_BG_STS_ERROR [%d]\n", __func__, eRadioNowExeMode);

			switch(eRadioNowExeMode) {
				case eRADIO_BG_EVT_SET_TUNE:
					tcradioapp_sendMessage(eSENDER_ID_BG, eRADIO_NOTIFY_TUNE, uiSendMsg, pNULL, ret);
					break;

				default:
					tcradioapp_sendMessage(eSENDER_ID_BG, eRadioNowExeMode, NULL, pNULL, ret);
					break;
			}
			break;

		/* Return Error */
		default:
			break;
	}
#endif
}

int32 tcradiobg_checkValidFreqAccordingToConfig(eRADIO_MOD_MODE_t mod_mode, uint32 freq)
{
	RET ret = 0;

	if(mod_mode == eRADIO_FM_MODE) {
		if(freq < stRadioBG.fm.startFreq || freq > stRadioBG.fm.endFreq) {
			ret = -1;
		}
		else {
			if(((freq - stRadioBG.fm.startFreq) % stRadioBG.fm.step) != 0) {
				ret = -2;
			}
		}
	}
	else if(mod_mode == eRADIO_AM_MODE) {
		if(freq < stRadioBG.am.startFreq || freq > stRadioBG.am.endFreq) {
			ret = -1;
		}
		else {
			if(((freq - stRadioBG.am.startFreq) % stRadioBG.am.step) != 0) {
				ret = -2;
			}
		}
	}
	else {
		ret = -3;
	}

	return ret;
}

RET tcradiobg_checkValidFreq(eRADIO_MOD_MODE_t mod_mode, uint32 freq)
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
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET tcradiobg_checkValidBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 start_freq, uint32  end_freq, uint32 step)
{
	RET ret = eRET_OK;

	if(mod_mode == eRADIO_FM_MODE) {
		if( (tcradiobg_checkValidFreq(mod_mode, start_freq) != eRET_OK) ||
			(tcradiobg_checkValidFreq(mod_mode, end_freq) != eRET_OK) ||
			(start_freq >= end_freq) ||
			(step != 200 && step != 100 && step != 50 && step != 10) ||
			((end_freq-start_freq)%step != 0) )
		{
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else if(mod_mode == eRADIO_AM_MODE) {
		if( (tcradiobg_checkValidFreq(mod_mode, start_freq) != eRET_OK) ||
			(tcradiobg_checkValidFreq(mod_mode, end_freq) != eRET_OK) ||
			(start_freq >= end_freq) ||
			(step != 10 && step != 9 && step != 1) ||
			((end_freq-start_freq)%step != 0) )
		{
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET tcradiobg_getFreqCounter(eRADIO_MOD_MODE_t mod_mode, uint32 *freq, uint8 dir)
{
	RET ret = eRET_OK;
	uint32 tfreq = *freq, sf = 0, ef = 0, step = 0;

	if(mod_mode == eRADIO_FM_MODE) {
		ef = stRadioBG.fm.endFreq;
		sf = stRadioBG.fm.startFreq;
		step = 	stRadioBG.fm.step;
	}
	else if(mod_mode == eRADIO_AM_MODE) {
		ef = stRadioBG.am.endFreq;
		sf = stRadioBG.am.startFreq;
		step = 	stRadioBG.am.step;
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
		return ret;
	}

	if(sf == 0  || ef == 0 || step == 0) {
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

static void tcradiobg_stateHandler(void)
{
	eRADIO_BG_STS_t eRadioSt = eRADIO_BG_STS_DOING;
	eRADIO_BG_STATE_t eRadioBGState = tcradiobg_getState();
	RET ret = eRET_OK;
	uint32 uiSendMsg[MSGQ_DATA_LENGTH] = {0,};
	int32 retValid;

	if(uiBGStateCnt) {
		uiBGStateCnt--;
	}	

	if(uiBGRestartCnt) {
		uiBGRestartCnt--;
	}

	switch(eRadioBGState)
	{
		case eRADIO_BG_STATE_START:
            RBG_DBG("[%s] eRADIO_BG_STATE_START : SeekMode = %d curFreq = %d, curStartFreq = %d \n", __func__,
                tcradiobg_getSeekMode(), stRadioBG.curFreq, stRadioBG.curStartFreq);

			eRadioBGState = eRADIO_BG_STATE_SET_FREQ;

		case eRADIO_BG_STATE_SET_FREQ :
			if(tcradiobg_getSeekMode() == eRADIO_SEEK_MAN_DOWN || tcradiobg_getSeekMode() == eRADIO_SEEK_AUTO_DOWN) {
				tcradiobg_getFreqCounter((eRADIO_MOD_MODE_t)stRadioBG.curBand, &stRadioBG.curFreq, DN);
			}
			else {
				tcradiobg_getFreqCounter((eRADIO_MOD_MODE_t)stRadioBG.curBand, &stRadioBG.curFreq, UP);
			}

			tcradiohal_setTune(stRadioBG.curBand, stRadioBG.curFreq, (uint32)eTUNER_DRV_TUNE_NORMAL, (uint32)eRADIO_ID_SECONDARY);

			eRadioBGState = eRADIO_BG_STATE_PRECHK_QDATA;

			if(tcradiobg_getSeekMode() == eRADIO_SEEK_MAN_UP || tcradiobg_getSeekMode() == eRADIO_SEEK_MAN_DOWN) {
				eRadioBGState = eRADIO_BG_STATE_STOP;
			}
			else if(tcradiobg_getSeekMode() == eRADIO_SEEK_SCAN_STATION || tcradiobg_getSeekMode() == eRADIO_SEEK_SCAN_PI) {
				eRadioSt = eRADIO_BG_STS_DOING_NOTIFY;
			}
			else {
				;
			}
			break;

		case eRADIO_BG_STATE_PRECHK_QDATA:
			tcradiohal_getQuality(stRadioBG.curBand, (stTUNER_QUALITY_t *)&stRadioBG.stSchQdata, eRADIO_ID_SECONDARY);

			if(pfnOnPrecheckSeekQual != NULL) {
				retValid = (*pfnOnPrecheckSeekQual)(stRadioBG.curBand, stRadioBG.stSchQdata, stRadioBG.curFreq, eRADIO_ID_SECONDARY);

			}
			else {
				retValid = tcradiobg_precheckSignalQuality(stRadioBG.stSchQdata);
			}

			if(retValid == 0) {	// ok
				if(tcradiobg_isFmBand() != 0) {
					uiBGStateCnt = 90 / BG_THREAD_TIME_INTERVAL;
				}
				else {
					uiBGStateCnt = 60 / BG_THREAD_TIME_INTERVAL;
				}

				eRadioBGState = eRADIO_BG_STATE_CHK_QDATA_STATUS;
			}
			else {
				if(stRadioBG.curFreq == stRadioBG.curStartFreq) {
					eRadioBGState = eRADIO_BG_STATE_STOP;
				}
				else {
					eRadioBGState = eRADIO_BG_STATE_SET_FREQ;
				}

				if(tcradiobg_getSeekMode() != eRADIO_SEEK_SCAN_STATION && tcradiobg_getSeekMode() != eRADIO_SEEK_SCAN_PI) {
					eRadioSt = eRADIO_BG_STS_DOING_NOTIFY;			// for returning current frequency quality values
				}
			}

			break;

		case eRADIO_BG_STATE_CHK_QDATA_STATUS:
			if (uiBGStateCnt <= 0) {
				eRadioBGState = eRADIO_BG_STATE_CHK_QDATA;
			}
			else {
				tcradiohal_getQuality(stRadioBG.curBand, (stTUNER_QUALITY_t *)&stRadioBG.stSchQdata, eRADIO_ID_SECONDARY);
				if(tcradiobg_checkSignalQualityStatus(stRadioBG.stSchQdata) == 0) {

					eRadioBGState = eRADIO_BG_STATE_CHK_QDATA;
				}
			}
			break;

		case eRADIO_BG_STATE_CHK_QDATA :
			tcradiohal_getQuality(stRadioBG.curBand, (stTUNER_QUALITY_t *)&stRadioBG.stSchQdata, eRADIO_ID_SECONDARY);

			if(pfnOnCheckSeekQual != NULL) {
				retValid = (*pfnOnCheckSeekQual)(stRadioBG.curBand, stRadioBG.stSchQdata, eRADIO_ID_SECONDARY);
			}
			else {
				retValid = tcradiobg_checkSignalQuality(stRadioBG.stSchQdata);
			}

			if(retValid == 0) {	// ok
				if(tcradiobg_getSeekMode() == eRADIO_SEEK_SCAN_STATION) {
					tcradiobg_makeStationList();
					if(stRadioBG.curFreq == stRadioBG.curStartFreq) {
						eRadioBGState = eRADIO_BG_STATE_STOP;
					}
					else {
						stRadioBG.curBGResult++;
						eRadioBGState = eRADIO_BG_STATE_SET_FREQ;
					}
				}
				else {
					stRadioBG.curBGResult = 1;
					eRadioBGState = eRADIO_BG_STATE_STOP;
				}

                RBG_DBG("[%s] eRADIO_BG_STATE_CHK_QDATA : SeekMode = %d BGState = %d, curFreq = %d \n", __func__,
                    tcradiobg_getSeekMode(), tcradiobg_getState(), stRadioBG.curFreq);
			}
			else {	// nok
				if(stRadioBG.curFreq == stRadioBG.curStartFreq) {
					eRadioBGState = eRADIO_BG_STATE_STOP;
				}
				else {
					eRadioBGState = eRADIO_BG_STATE_SET_FREQ;
				}
			}

			if((tcradiobg_getSeekMode() != eRADIO_SEEK_SCAN_STATION) && (tcradiobg_getSeekMode() != eRADIO_SEEK_SCAN_PI)) {
				eRadioSt = eRADIO_BG_STS_DOING_NOTIFY;			// for returning current frequency quality values
			}

			break;

		case eRADIO_BG_STATE_STOP:
			if(tcradiobg_getSeekMode() == eRADIO_SEEK_SCAN_STATION) {
				fBGStationListTx = 1;
			}
			tcradiobg_stopSeek(1);
			eRadioSt = eRADIO_BG_STS_OK_NOTIFY;
            eRadioBGState = eRADIO_BG_STATE_WAIT;
            uiBGRestartCnt = BG_WAIT_TIME_BGRESTART;
			break;

		case eRADIO_BG_STATE_WAIT:
            if (uiBGRestartCnt <= 0)
            {
                eRadioBGState = eRADIO_BG_STATE_START;
                tcradiobg_setValueBGRestart();
                tcradiobg_setSeekMode(eRADIO_SEEK_SCAN_STATION);
            }
            else
            {
                eRadioSt = eRADIO_BG_STS_OK;
            }
			break;

		default:	// On Air
			if(tcradiobg_getSeekMode() != eRADIO_SEEK_STOP) {
				tcradiobg_setSeekMode(eRADIO_SEEK_STOP);
				eRadioSt = eRADIO_BG_STS_OK_NOTIFY;
			}
			else {
				eRadioSt = eRADIO_BG_STS_OK;
			}
			break;
	}

	switch(eRadioSt) {
		/* Job End -> No Notify */
		case eRADIO_BG_STS_WAIT:
			break;

		/* Job Good Complete */
		case eRADIO_BG_STS_OK:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eRADIO_BG_STS_OK_NOTIFY:
#if 1
			uiSendMsg[0] = stRadioBG.curBand;
			uiSendMsg[1] = stRadioBG.curFreq;
			uiSendMsg[2] = tcradiobg_getSeekMode();
			uiSendMsg[3] = stRadioBG.curBGResult;
			uiSendMsg[4] = fBGStationListTx;
			uiSendMsg[5] = 1;   // BS Scan Result --> 1
			tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_NOTIFY_SEEK_MODE, uiSendMsg, pNULL, ret);
#endif
			tcradiobg_setState(eRadioBGState);

            RBG_DBG("[%s] eRADIO_BG_STS_OK_NOTIFY : SeekMode = %d, BGState = %d curBGResult = %d, uiBGStationListCounter = %d \n", __func__,
            tcradiobg_getSeekMode(), tcradiobg_getState(), stRadioBG.curBGResult, uiBGStationListCounter);
			break;

		/* Job Continue */
		case eRADIO_BG_STS_DOING:
			tcradiobg_setState(eRadioBGState);
			break;

		/* Job Continue -> Information Dynamic Notify */
		case eRADIO_BG_STS_DOING_NOTIFY :
			tcradiobg_setState(eRadioBGState);
			break;

		/* Job Continue -> Error Notify */
		case eRADIO_BG_STS_DOING_ERROR_NOTIFY :
			tcradiobg_setState(eRadioBGState);	/* No break because of doing below function */

		/* Job Error -> Return Error */
		case eRADIO_BG_STS_ERROR:
			break;

		/* Return Error */
		default:
			break;
	}
}

void tcradiobg_stopSeek(uint32 retune)
{
	if(retune != 0) {
		RET ret;
		if(tcradiobg_getSeekMode() == eRADIO_SEEK_SCAN_STATION) {
			stRadioBG.curFreq = stRadioBG.curStartFreq;
		}
		ret = tcradiohal_setTune(stRadioBG.curBand, stRadioBG.curFreq, eTUNER_DRV_TUNE_NORMAL, eRADIO_ID_SECONDARY);
	}

	tcradiobg_setSeekMode(eRADIO_SEEK_STOP);
}

void tcradiobg_stopBG(void)
{
	RET ret;
	stRadioBG.curFreq = stRadioBG.curStartFreq;
	ret = tcradiohal_setTune(stRadioBG.curBand, stRadioBG.curFreq, eTUNER_DRV_TUNE_NORMAL, eRADIO_ID_SECONDARY);

	tcradiobg_setState(eRADIO_BG_STATE_ON_AIR);
	tcradiobg_setSeekMode(eRADIO_SEEK_STOP);
}

int32 tcradiobg_checkSignalQualityStatus(stRADIO_QUALITY_t qdata)
{
	int32 ret = -1;

	if(qdata.type== eRADIO_TYPE1)
	{
		stRADIO_TYPE1_QUALITY_t temp_qdata;
		tcradio_memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));
		if(tcradiobg_isFmBand() == 0) {
			if(qdata.qual.fm.Qvalue[8] & 0x00000030) {
				ret = 0;
			}
		}
		else if(tcradiobg_isAmBand() == 0) {
			if(qdata.qual.am.Qvalue[5] & 0x00000030) {
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
		if(tcradiobg_isFmBand() == 0) {
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

int32 tcradiobg_precheckSignalQuality(stRADIO_QUALITY_t qdata)
{
	int32 ret=-1, rssi=0;

	if(qdata.type == eRADIO_TYPE1)
	{
		stRADIO_TYPE1_QUALITY_t temp_qdata;
		tcradio_memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(tcradiobg_isFmBand() == 0) {
			rssi = (int32)temp_qdata.fm.Rssi;
			if(rssi >= 20)
			{
				ret = 0;
			}
		}
		else if(tcradiobg_isAmBand() == 0) {
			rssi = (int32)temp_qdata.am.Rssi;
			if(rssi >= 38)
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

		if(tcradiobg_isFmBand() == 0) {
			rssi = (int32)temp_qdata.fm.Rssi;
			if(rssi >= 280)
			{
				ret = 0;
			}
		}
		else if(tcradiobg_isAmBand() == 0) {
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
	}
	return ret;
}

int32 tcradiobg_checkSignalQuality(stRADIO_QUALITY_t qdata)
{
	int32 ret=-1;
	int32 rssi=0, snr=0, offs=0, sqi=0;
	uint32 usn=0, mpth=0, noise=0, detect=0;

	if(qdata.type == eRADIO_TYPE1)
	{
		stRADIO_TYPE1_QUALITY_t temp_qdata;
		tcradio_memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(tcradiobg_isFmBand() == 0) {
			rssi = (int32)temp_qdata.fm.Rssi;
			snr = (int32)temp_qdata.fm.Snr;
			offs = (int32)temp_qdata.fm.Offs;
			if((rssi >= 20) && (snr > 4) && (offs > -6) && (offs < 6))
			{
				ret = 0;
			}
		}
		else if(tcradiobg_isAmBand() == 0) {
			rssi = (int32)temp_qdata.am.Rssi;
			snr = (int32)temp_qdata.am.Snr;
			offs = (int32)temp_qdata.am.Offs;
			if((rssi >= 38) && (snr > 6) && (offs > -6) && (offs < 6))
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

		if(tcradiobg_isFmBand() == 0) {
			rssi = (int32)temp_qdata.fm.Rssi;
			offs = (int32)temp_qdata.fm.Offs;
			usn = temp_qdata.fm.Usn;
			mpth = temp_qdata.fm.Mpth;
			if((rssi >= 280) && (offs > -100) && (offs < 100) && (usn < 120) && (mpth < 200))
			{
				ret = 0;
			}
		}
		else if(tcradiobg_isAmBand() == 0) {
			rssi = (int32)temp_qdata.am.Rssi;
			offs = (int32)temp_qdata.am.Offs;
			noise = temp_qdata.am.Hfn;
			if((rssi >= 630) && (offs > -50) && (offs < 50) && (noise < 100))
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

void tcradiobg_makeStationList(void)
{
	if(uiBGStationListCounter < _AmFmDBSize) {
		_stBGStationList[uiBGStationListCounter].uiFreq = stRadioBG.curFreq;
		tcradio_memcpy(&_stBGStationList[uiBGStationListCounter].stQdata, &stRadioBG.stSchQdata, sizeof(stRADIO_QUALITY_t));
		uiBGStationListCounter++;
	}
	else {
		RBG_DBG("%s : The station list is already full!! \n", __func__);
	}
}

void tcradiobg_initSeek(eRADIO_SEEK_MODE_t nextSeekMode)
{
	uint32 cf = stRadioBG.curFreq;
	uint32 sf= 0, ef = 0, step = 0;
	int32 chkret;

	tcradiobg_setState(eRADIO_BG_STATE_START);
	stRadioBG.curBGResult = 0;

	chkret = tcradiobg_checkValidFreqAccordingToConfig(stRadioBG.curBand, cf);
	if(chkret != 0) {
		if(stRadioBG.curBand == eRADIO_AM_MODE) {
			sf = stRadioBG.am.startFreq;
			ef = stRadioBG.am.endFreq;
			step = stRadioBG.am.step;
		}
		else if(stRadioBG.curBand == eRADIO_FM_MODE) {
			sf = stRadioBG.fm.startFreq;
			ef = stRadioBG.fm.endFreq;
			step = stRadioBG.fm.step;
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

		stRadioBG.curFreq = cf;
	}

	stRadioBG.curStartFreq = stRadioBG.curFreq;

	if(nextSeekMode == eRADIO_SEEK_STOP) {
		tcradiobg_setState(eRADIO_BG_STATE_STOP);
	}
	else if(nextSeekMode < eRADIO_SEEK_END) {
		if(nextSeekMode == eRADIO_SEEK_SCAN_STATION) {
			fBGStationListTx = 0;
			uiBGStationListCounter = 0;
		}
		tcradiobg_setSeekMode(nextSeekMode);
	}
	else {
		tcradiobg_setSeekMode(eRADIO_SEEK_END);
	}

    RBG_DBG("[%s] curBand = %d, curFreq = %d, curStartFreq = %d \n", __func__,
        stRadioBG.curBand, stRadioBG.curFreq, stRadioBG.curStartFreq);
}

void tcradiobg_setValueBGRestart(void)
{
    stRadioBG.curBGResult = 0;
    fBGStationListTx = 0;
    uiBGStationListCounter = 0;

    if (stRadioBG.curBand == eRADIO_FM_MODE)
    {
        stRadioBG.curFreq = stRadioBG.fm.startFreq;
        stRadioBG.curStartFreq = stRadioBG.fm.startFreq;
    }
    else
    {
        stRadioBG.curFreq = stRadioBG.am.startFreq;
        stRadioBG.curStartFreq = stRadioBG.am.startFreq;
    }
}

int32 tcradiobg_isFmBand(void)
{
	int32 ret = -1;
	if(stRadioBG.curBand == eRADIO_FM_MODE) {
		ret = 0;
	}
	return ret;
}

int32 tcradiobg_isAmBand(void)
{
	int32 ret = -1;
	if(stRadioBG.curBand == eRADIO_AM_MODE) {
		ret = 0;
	}
	return ret;
}
