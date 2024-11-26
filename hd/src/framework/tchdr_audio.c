/*******************************************************************************

*   FileName : tchdr_audio.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio Framework Audio Output functions and definitions

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
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>

#include "tchdr_common.h"

#include "hdrAudio.h"
#include "hdrAudioResampler.h"

#include "tchdr_std.h"
#include "tchdr_callback_conf.h"
#include "tchdr_cbuffer.h"
#include "tchdr_msg.h"
#include "tchdr_audio.h"
#include "tchdr_fader.h"
#include "tcaud_resampler.h"
#include "tchdr_bbinput.h"

/***************************************************
*        Global variable definitions               *
****************************************************/

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/

/***************************************************
*             Local preprocessor                   *
****************************************************/
#define	MAX_AUDIO_BUFFER_SIZE	(HDR_AUDIO_FRAME_SIZE * 4U)	// 8192
#define	FILL_ZERO_BUFFER_SIZE	(HDR_AUDIO_FRAME_SIZE * 2U)	// 4096	// This value should be set in multiples of 2048 frames and less than MAX_AUDIO_BUFFER_SIZE.

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*           Local constant definitions              *
****************************************************/
static eTCHDR_AUDIO_CMD_t eTcHdrAudioEventMode;

static pthread_mutex_t gTcHdrDAudBufMutex = PTHREAD_MUTEX_INITIALIZER;

static HDBOOL bPlaybackReady = false;
static HDBOOL bPlaybackStart = false;
static HDBOOL bPlaybackUserMute = false;
static HDBOOL bPlaybackTuneMute = false;

stCIRC_BUFF_t digitalAudioBuffer;
static HDR_pcm_stereo_t  digitalAudBufData[MAX_AUDIO_BUFFER_SIZE];

#ifdef USE_AUDIO_OUTPUT_RESAMPLER
static stTC_AUDIO_RESAMPLER_t* tcAudioOutputResampler;
static S8 tcAudioOutputResamplerMem[TC_AUDIO_RESAMPLER_MEM_SIZE];
static F64 digout_hz = (F64)TCHDR_SAMPLERATE;
static F64 digin_hz = (F64)TCHDR_SAMPLERATE;
static F64 digout_ppm = 0.0;
static S16 tcResamplerOutBuf[HDR_AUDIO_FRAME_SIZE*4U];
#endif

static sem_t audioOutputThreadSem;

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
static struct timespec audoutput_ChkTimer, audoutput_ChkTimeNow, audoutput_ChkTimeDiff;
static U32 audoutput_AccumMs=0, audoutput_LoopMs=0, audoutput_DumpCount=0;
static FILE *audoutput_DumpFile;
#endif

/***************************************************
*          Local function prototypes               *
****************************************************/
S32 tchdraudoutput_getUserMute(void);
S32 tchdraudoutput_getTuneMute(void);

static void tchdraudoutput_start(void);
static void tchdraudoutput_stop(void);
static HDRET tchdraudoutput_reset(void);
static void tchdraudoutput_setUserMute(U32 fOnOff);
static S32 tchdraudoutput_getReady(void);
static S32 tchdraudoutput_getStart(void);

static eTCHDR_EVT_STS_t tchdraudoutput_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdraudoutput_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdraudoutput_event_start(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdraudoutput_event_stop(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdraudoutput_event_reset(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdraudoutput_event_mute(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdraudoutput_event_test(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);

/***************************************************
*			function definition				*
****************************************************/
static void tchdraudoutput_initFader(void)
{
	stFADER_PARAMS_t stFaderParams;
	tchdrfader_configUserMuteStatus(&tchdraudoutput_getUserMute);
	tchdrfader_configTuneMuteStatus(&tchdraudoutput_getTuneMute);
	tchdrfader_init();

	stFaderParams.bFaderEn = 1;
	stFaderParams.samplingRate = TCHDR_SAMPLERATE;
	stFaderParams.mute.fadeinTime = 100;
	stFaderParams.mute.fadeoutTime = 100;
	(void)tchdrfader_setParams(stFaderParams);
}

HDRET tchdraudoutput_init(void)
{
	HDRET ret;

	ret = tchdraudoutput_messageInit();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
	#ifdef USE_AUDIO_OUTPUT_RESAMPLER
		tcAudioOutputResampler = tcaudio_resampler_init((void*)tcAudioOutputResamplerMem);
		if(tcAudioOutputResampler != NULL) {
			(*stCircFunc.cb_init)(&digitalAudioBuffer, (void*)digitalAudBufData, MAX_AUDIO_BUFFER_SIZE, (U32)sizeof(HDR_pcm_stereo_t));
			(void)(*stOsal.mutexinit)(&gTcHdrDAudBufMutex, NULL);
			digitalAudioBuffer.mutex = &gTcHdrDAudBufMutex;
			digout_hz = digin_hz * (1. + (digout_ppm / 1e6));
			(*pfnHdrLog)(eTAG_AOUT, eLOG_DBG, "HDR audio output resampler slips: %fppm, in_hz: %fHz, out_hz: %fHz\n", digout_ppm, digin_hz, digout_hz);
		}
		else {
			(*pfnHdrLog)(eTAG_AOUT, eLOG_ERR, "Failed to initialize audio output resampler.\n");
			ret = (HDRET)eTC_HDR_RET_NG_AUD_RESAMPLER_INIT;
		}
	#else
		(*stCircFunc.cb_init)(&digitalAudioBuffer, (void*)digitalAudBufData, MAX_AUDIO_BUFFER_SIZE, (U32)sizeof(HDR_pcm_stereo_t));
		(void)(*stOsal.mutexinit)(&gTcHdrDAudBufMutex, NULL);
		digitalAudioBuffer.mutex = &gTcHdrDAudBufMutex;
	#endif
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			tchdraudoutput_initFader();
			(void)sem_init(&audioOutputThreadSem, 0, 0);
		}
	}
	else {
		(*pfnHdrLog)(eTAG_AOUT, eLOG_ERR, "Failed to initialize audio output message.\n");
		ret = (HDRET)eTC_HDR_RET_NG_INIT_MESSAGE_QUEUE;
	}

    return ret;
}

HDRET tchdraudoutput_deinit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	(void)tchdraudoutput_messageDeinit();
	(void)(*stCircFunc.cb_reset)(&digitalAudioBuffer);
	digitalAudioBuffer.mutex = pNULL;
	(void)(*stOsal.mutexdeinit)(&gTcHdrDAudBufMutex);
	(void)sem_destroy(&audioOutputThreadSem);
	return ret;
}

static void tchdraudoutput_start(void)
{
	bPlaybackStart = true;
	(*pfnHdrLog)(eTAG_AOUT, eLOG_DBG, "Start HDR Audio Playback\n");
}

static void tchdraudoutput_stop(void)
{
	bPlaybackStart = false;
	(*pfnHdrLog)(eTAG_AOUT, eLOG_DBG, "Stop HDR Audio Playback\n");
}

static HDRET tchdraudoutput_reset(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	bPlaybackReady = false;
#ifdef USE_AUDIO_BUFFER_FILL_ZERO_BEFORE_START
	(void)(*stCircFunc.cb_reset_and_fill)(&digitalAudioBuffer, (S8)0x00, FILL_ZERO_BUFFER_SIZE);
#else
	(void)(*stCircFunc.cb_reset)(&digitalAudioBuffer);
#endif
	return ret;
}

static void tchdraudoutput_setUserMute(U32 fOnOff)
{
	if(fOnOff > (U32)0) {
		bPlaybackUserMute = true;
		(*pfnHdrLog)(eTAG_AOUT, eLOG_DBG, "HDR audio mute on\n");
	}
	else {
		bPlaybackUserMute = false;
		(*pfnHdrLog)(eTAG_AOUT, eLOG_DBG, "HDR audio mute off\n");
	}
}

void tchdraudoutput_setTuneMute(U32 fOnOff)
{
	if(fOnOff > (U32)0) {
		bPlaybackTuneMute = true;
		(*pfnHdrLog)(eTAG_AOUT, eLOG_DBG, "HDR audio tune mute on\n");
	}
	else {
		bPlaybackTuneMute = false;
		(*pfnHdrLog)(eTAG_AOUT, eLOG_DBG, "HDR audio tune mute off\n");
	}
}

HDRET tchdraudoutput_open(void)
{
	HDRET ret;
	ret = tchdraudoutput_reset();
	tchdraudoutput_start();
	tchdraudoutput_setUserMute(0);	// mute off
	return ret;
}

HDRET tchdraudoutput_close(void)
{
	HDRET ret;
	ret = tchdraudoutput_reset();
	tchdraudoutput_stop();
	tchdraudoutput_setUserMute(1);	// mute on
	return ret;
}

S32 tchdraudoutput_getReadySemaValue(void)
{
	S32 val;
	S32 rc;
	rc = sem_getvalue(&audioOutputThreadSem, &val);
	if(rc == 0) {
		rc = val;
	}
	return rc;
}

void tchdraudoutput_ready(void)
{
	(void)sem_post(&audioOutputThreadSem);
}

void tchdraudoutput_rxHandler(HDR_pcm_stereo_t* audioFrame)
{
	S32 frames;

	if((tchdr_getHDRadioOpenStatus() == (HDRET)eTC_HDR_RET_OK)) {
		tchdrfader_process(audioFrame, (S32)HDR_AUDIO_FRAME_SIZE);
	#ifdef USE_AUDIO_BUFFER_FILL_ZERO_BEFORE_START
		if(bPlaybackReady == false) {
			(void)(*stCircFunc.cb_reset_and_fill)(&digitalAudioBuffer, (S8)0x00, FILL_ZERO_BUFFER_SIZE);
		}
	#endif
        frames = (*stCircFunc.cb_write)(&digitalAudioBuffer, (void*)audioFrame, HDR_AUDIO_FRAME_SIZE);
	    if(frames > 0) {
			tchdraudoutput_ready();
	    }
		else {
			(*pfnHdrLog)(eTAG_BLD, eLOG_ERR, "HD Radio blend PCM output buffer overflow.\n");
	    }
	    bPlaybackReady = true;
	}
}

// Telechips HD Radio Audio API
F64 tchdraudoutput_setResamplerSlips(F64 ppm)
{
#ifdef USE_AUDIO_OUTPUT_RESAMPLER
	digout_ppm = ppm;
	digout_hz = digin_hz * (1. + (digout_ppm / 1e6));
	(*pfnHdrLog)(eTAG_AOUT, eLOG_DBG, "HDR audio output resampler slips: %fppm, in_hz: %fHz, out_hz: %fHz\n", digout_ppm, digin_hz, digout_hz);
	return digout_hz;
#else
	return 0.0;
#endif
}

void tchdraudoutput_getResamplerSlips(F64 *ppm, F64 *out_hz)
{
	if((ppm != NULL) && (out_hz != NULL)) {
#ifdef USE_AUDIO_OUTPUT_RESAMPLER
		*ppm = digout_ppm;
		*out_hz = digout_hz;
#else
		*ppm = 0.0;
		*out_hz = 0.0;
#endif
	}
}

static S32 tchdraudoutput_getReady(void)
{
	return ((bPlaybackReady==true) ? 0x01 : 0x00);
}

static S32 tchdraudoutput_getStart(void)
{
	return ((bPlaybackStart==true) ? 0x01 : 0x00);
}

S32 tchdraudoutput_getUserMute(void)
{
	return ((bPlaybackUserMute==true) ? 0x01 : 0x00);
}

S32 tchdraudoutput_getTuneMute(void)
{
	return ((bPlaybackTuneMute==true) ? 0x01 : 0x00);
}

static void tchdraudoutput_setEventMode(eTCHDR_AUDIO_CMD_t evtmode)
{
	eTcHdrAudioEventMode = evtmode;
}

static eTCHDR_AUDIO_CMD_t tchdraudoutput_getEventMode(void)
{
	return eTcHdrAudioEventMode;
}

static void tchdraudoutput_sendNotifyMessage(U32 eRcvID, U32 mode, const U32 *data, const void *pdata, S32 err)
{
	U32 txmode = (*stArith.u32add)(mode, (U32)TCHDR_AUD_NOTIFY_WT);

	switch(eRcvID) {
		case (U32)eTCHDR_SENDER_ID_APP:
			(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_AUDIO, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_SERVICE:
			(void)tchdrsvc_sendMessage(eTCHDR_SENDER_ID_AUDIO, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_AUDIO:
			(void)tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_AUDIO, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_BBINPUT:
			(void)tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_AUDIO, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MAIN:
			(void)tchdrmain_sendMessage(eTCHDR_SENDER_ID_AUDIO, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MRC:
			(void)tchdrmrc_sendMessage(eTCHDR_SENDER_ID_AUDIO, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BS:
			(void)tchdrbs_sendMessage(eTCHDR_SENDER_ID_AUDIO, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BLENDING:
			(void)tchdrblending_sendMessage(eTCHDR_SENDER_ID_AUDIO, txmode, data, pdata, err);
			break;
		default:
			UNUSED(0);
			(*pfnHdrLog)(eTAG_AOUT, eLOG_DBG, " %s : Invalid received ID[%d].\n", __func__, eRcvID);
			break;
	}
}

static void tchdraudoutput_messageParser(const stTcHdrMsgBuf_t *pstMsg)
{
	switch(pstMsg->uiMode) {
		// Event
		case (U32)eTCHDR_AUDIO_CMD_OPEN:
		case (U32)eTCHDR_AUDIO_CMD_CLOSE:
		case (U32)eTCHDR_AUDIO_CMD_START:
		case (U32)eTCHDR_AUDIO_CMD_STOP:
		case (U32)eTCHDR_AUDIO_CMD_RESET:
		case (U32)eTCHDR_AUDIO_CMD_MUTE:
		case (U32)eTCHDR_AUDIO_CMD_TEST:
			tchdraudoutput_setEventMode((eTCHDR_AUDIO_CMD_t)pstMsg->uiMode);
			break;

		// Notification of the other thread: Do it here.

		default:
			UNUSED(0);
			break;
	}
}

static void tchdraudoutput_eventHandler(stTcHdrMsgBuf_t stRcvMsgQ)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_AUDIO_CMD_t eNowEvtMode = tchdraudoutput_getEventMode();
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};

	switch(eNowEvtMode) {
		// Event
		case eTCHDR_AUDIO_CMD_OPEN:
			eEvtSt = tchdraudoutput_event_open(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_AUDIO_CMD_CLOSE:
			eEvtSt = tchdraudoutput_event_close(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_AUDIO_CMD_START:
			eEvtSt = tchdraudoutput_event_start(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_AUDIO_CMD_STOP:
			eEvtSt = tchdraudoutput_event_stop(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_AUDIO_CMD_RESET:
			eEvtSt = tchdraudoutput_event_reset(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_AUDIO_CMD_MUTE:
			eEvtSt = tchdraudoutput_event_mute(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_AUDIO_CMD_TEST:
			eEvtSt = tchdraudoutput_event_test(stRcvMsgQ, uiSendMsg, &ret);
			break;

		default:
			UNUSED(0);
			break;
	}

	tchdraudoutput_setEventMode(eTCHDR_AUDIO_CMD_NULL);

	switch(eEvtSt) {
		/* Job Good Complete -> No Notify */
		case eTCHDR_EVT_STS_DONE:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eTCHDR_EVT_STS_DONE_NOTIFY:
			tchdraudoutput_sendNotifyMessage(stRcvMsgQ.uiSender, (U32)eNowEvtMode, uiSendMsg, pNULL, ret);
			break;

		/* Return Error */
		default:
			UNUSED(0);
			break;
	}
}

static void tchdraudoutput_eventMessageProcess(stTcHdrMsgBuf_t *pstMsg)
{
	(void)tchdraudoutput_getMessage(pstMsg);
	if(pstMsg->fNewMsg == (U32)eTCHDR_NEW_MSG_EXIST) {
		tchdraudoutput_messageParser(pstMsg);
	}
	tchdraudoutput_eventHandler(*pstMsg);
}

void *tchdr_audioPlaybackThread(void* arg)
{
	S32 frames;
	stTcHdrMsgBuf_t stRecivedMessage;
    S16 rbuffer[HDR_AUDIO_FRAME_SIZE*2U];

	(*stOsal.setPostThreadAttr)(stAudioPlaybackAttr, eTAG_AOUT);	// Set thread nice & name
	(void)(*stOsal.osmemset)((void*)&stRecivedMessage, (S8)0, (U32)sizeof(stTcHdrMsgBuf_t));
    (void)(*stOsal.osmemset)((void*)rbuffer, (S8)0, (U32)sizeof(rbuffer));

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	audoutput_DumpFile = fopen(DUMP_PATH"audoutput_thread_status.csv", "w");
	clock_gettime(LINUX_CLOCK_TIMER_TYPE, &audoutput_ChkTimer);
#endif

	stAudioPlaybackAttr.thread_running = 1;
    while (stAudioPlaybackAttr.thread_running > 0) {
		tchdraudoutput_eventMessageProcess(&stRecivedMessage);

		(void)sem_wait(&audioOutputThreadSem);
		if(stAudioPlaybackAttr.thread_running != 1) {
			break;
		}

		frames = (*stCircFunc.cb_read)(&digitalAudioBuffer, (void*)rbuffer, HDR_AUDIO_FRAME_SIZE);
		if(frames > 0) {
		#ifdef USE_AUDIO_OUTPUT_RESAMPLER
			frames = tcaudio_resampler_exec(tcAudioOutputResampler, rbuffer, tcResamplerOutBuf, (S32)HDR_AUDIO_FRAME_SIZE, digin_hz, digout_hz);
			if(frames > 0) {
				if(tchdraudoutput_getStart() > 0) {
					(*pfnTcHdrAudioQueueCallBack)((void*)tcResamplerOutBuf, frames, TCHDR_SAMPLERATE);
				}
			}
			else {
				(*pfnHdrLog)(eTAG_AOUT, eLOG_ERR, "Audio output resampler parameter error! [%d]\n", frames);
			}
		#else
			if(tchdraudoutput_getStart() > 0) {
				(*pfnTcHdrAudioQueueCallBack)((void*)rbuffer, frames, TCHDR_SAMPLERATE);
			}
		#endif
		}
		else {
			(*pfnHdrLog)(eTAG_AOUT, eLOG_ERR, "Audio output buffer underrun! [%d]\n", frames);
		}
#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &audoutput_ChkTimeNow);
		audoutput_ChkTimeDiff.tv_sec = audoutput_ChkTimeNow.tv_sec - audoutput_ChkTimer.tv_sec;
		audoutput_ChkTimeDiff.tv_nsec = audoutput_ChkTimeNow.tv_nsec - audoutput_ChkTimer.tv_nsec;
		if(audoutput_ChkTimeDiff.tv_nsec < 0) {
			audoutput_ChkTimeDiff.tv_sec--;
			audoutput_ChkTimeDiff.tv_nsec += 1e9;
		}
		audoutput_LoopMs = (audoutput_ChkTimeDiff.tv_sec*1000) + (audoutput_ChkTimeDiff.tv_nsec/1000000);
		if(audoutput_DumpFile != NULL) {
			S32 availspace = (*stCircFunc.cb_availData)(&digitalAudioBuffer);
			S32 inputavailspace = (*stCircFunc.cb_availData)(&analogAudioBuffer);
			fprintf(audoutput_DumpFile, "%d,%d,%d,%d,%d,%f,%d\n", audoutput_DumpCount++, audoutput_AccumMs+=audoutput_LoopMs, audoutput_LoopMs, availspace, frames, digout_hz, inputavailspace);
			fflush(audoutput_DumpFile);
		}
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &audoutput_ChkTimer);
#endif
    }

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	if(audoutput_DumpFile != NULL) {
		fclose(audoutput_DumpFile);
	}
#endif
	stAudioPlaybackAttr.thread_running = -1;
	(void)tchdraudoutput_deinit();
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrAudOutput Thread Sequence 08...\n");
	return pNULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/*								Telechips HD Radio Audio Event Functions							*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
static eTCHDR_EVT_STS_t tchdraudoutput_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	ret = tchdraudoutput_open();

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdraudoutput_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	ret = tchdraudoutput_close();

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdraudoutput_event_start(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) {
		uiSendMsg[0] = 1;
	}
	tchdraudoutput_start();

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdraudoutput_event_stop(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	tchdraudoutput_stop();

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdraudoutput_event_reset(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	ret = tchdraudoutput_reset();

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdraudoutput_event_mute(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) {
		uiSendMsg[0] = stRcvMsgQ.uiData[0];
	}

	if(stRcvMsgQ.uiData[0] > (U32)0) {
		tchdraudoutput_setUserMute(1);
	}
	else {
		tchdraudoutput_setUserMute(0);
	}

	if(iError != NULL) {
		*iError = ret;
	}
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdraudoutput_event_test(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	return eEvtSts;
}
