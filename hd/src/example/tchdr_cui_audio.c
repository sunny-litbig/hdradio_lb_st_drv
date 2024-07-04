/*******************************************************************************

*   FileName : tchdr_cui_audio.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio CUI audio functions and definitions

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
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/prctl.h>

#if defined(USE_TELECHIPS_EVB) && defined(BUILD_HDR_EXAMPLE_CUI)
#include "tchdr_api.h"
#include "tchdr_cui_if.h"
#include "tchdr_cui_audio.h"

#include "tcradio_types.h"
#include "tcradio_hal_config.h"
#include "tcradio_hal_fifo.h"

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
#define	HDR_AUDIO_BUFFER_SIZE    ((U32)TC_HDR_AUDIO_FRAME_SIZE*4U)

/***************************************************
*           Local constant definitions              *
****************************************************/
static pthread_t hdrAudioThreadId = (pthread_t)0;
static pthread_mutex_t hdrAudioMutex;

static S32 fThreadRunning=0;
static U32 fHdrAudioInit=0;
static U32 fHdrAudioOpen=0;

static stAUDIO_CONFIG_t stHdrAudioConfig;
static U8 hdrAudioBuffer[HDR_AUDIO_BUFFER_SIZE];

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/
static void HDRCUI_AUD_LOG(U8 log_type, U8 en_tag, const S8 *format, ...);
static void tchdradiocui_audio_release(void);
void *tchdradiocui_audioThread(void *arg);

/****************************************************
*			function definition						*
*****************************************************/
static void HDRCUI_AUD_LOG(U8 log_type, U8 en_tag, const S8 *format, ...)
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
				CUI_AUD_LOGE("%s", strLog);
				break;

			case CUI_WRN_LOG:
				CUI_AUD_LOGW("%s", strLog);
				break;

			case CUI_INF_LOG:
				CUI_AUD_LOGI("%s", strLog);
				break;

			case CUI_DBG_LOG:
				CUI_AUD_LOGD("%s", strLog);
				break;

			default:
				CUI_AUD_LOGMSG("%s", strLog);
				break;
		}
	}
	else {
		(void)printf("%s", strLog);
	}
#endif
}

static void tchdradiocui_audio_mutexLock(void)
{
	(void)pthread_mutex_lock(&hdrAudioMutex);
}

static void tchdradiocui_audio_mutexUnlock(void)
{
	(void)pthread_mutex_unlock(&hdrAudioMutex);
}

HDRET tchdradiocui_audio_init(void)
{
	HDRET hdret = (HDRET)eTC_HDR_RET_OK;

	if(hdrAudioThreadId == (pthread_t)0) {
		hdret = tchdradiocui_createThread(&hdrAudioThreadId, &tchdradiocui_audioThread);
		if(hdret != (HDRET)eTC_HDR_RET_OK) {
			HDRCUI_AUD_LOG(CUI_ERR_LOG, 1U, "[%s:%d] Failed to create audio thread!!!\n", __func__, __LINE__);
			hdret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
	}
	else {
		HDRCUI_AUD_LOG(CUI_DBG_LOG, 1U, "[%s:%d] Already audio thread was created!!!\n", __func__, __LINE__);
	}

	if(hdret == (HDRET)eTC_HDR_RET_OK) {
		if(fHdrAudioInit == (U32)0) {
			hdret = pthread_mutex_init(&hdrAudioMutex, NULL);
			if(hdret == 0) {
				hdret = tcradiohal_audiofifo_init(AUDIO_FIFO_NUM);    // Init audio fifo buffer. Replace with your audio buffer.
				if(hdret == 0) {
					fHdrAudioInit = 1;
				}
				else {
					hdret = (HDRET)eTC_HDR_RET_NG_RSC;
				}
			}
			else {
				hdret = (HDRET)eTC_HDR_RET_NG_MUTEX_INIT;
			}
		}
	}

	return hdret;
}

HDRET tchdradiocui_audio_deinit(void)
{
	HDRET hdret = (HDRET)eTC_HDR_RET_OK;
	if(fHdrAudioInit == (U32)1) {
		if(fHdrAudioOpen == (U32)0) {
			fThreadRunning = 0;
		}
		else {
			hdret = (HDRET)eTC_HDR_RET_NG_NOT_YET_CLOSE;
		}
	}
	else {
		hdret = (HDRET)eTC_HDR_RET_NG_NOT_YET_INIT;
	}

	return hdret;
}

static void tchdradiocui_audio_release(void)
{
	tchdradiocui_audio_mutexLock();
	(void)tcradiohal_audiofifo_deinit(AUDIO_FIFO_NUM);    // Deinit audio fifo buffer. Replace with your audio buffer.
	hdrAudioThreadId = (pthread_t)0;
	fHdrAudioInit = 0;
	tchdradiocui_audio_mutexUnlock();
	(void)pthread_mutex_destroy (&hdrAudioMutex);
}

HDRET tchdradiocui_audio_open(U32 samplerate)
{
	HDRET hdret = (HDRET)eTC_HDR_RET_OK;
	S32 i;

	tchdradiocui_audio_mutexLock();
	if(fHdrAudioInit == (U32)1) {
		if(fHdrAudioOpen  == (U32)0) {
			stHdrAudioConfig.samplerate = samplerate;
			stHdrAudioConfig.channels = SOUND_NUM_OF_CHANNEL;
			stHdrAudioConfig.buffersize = SOUND_BUFFER_SIZE;
			stHdrAudioConfig.periodsize = SOUND_PERIOD_SIZE;
			stHdrAudioConfig.aout_startThd = SOUND_BUFFER_SIZE;
			stHdrAudioConfig.ain_startThd = 0;
			for(i=HDR_AUD_FIFO_INDEX; i<AUDIO_FIFO_NUM; i++) {
				(void)tcradiohal_audiofifo_reset(i);
			}
			(void)memset(hdrAudioBuffer, 0, sizeof(hdrAudioBuffer));
			(void)tcradiohal_audiofifo_pushData(HDR_AUD_FIFO_INDEX, (U8*)hdrAudioBuffer, HDR_AUDIO_BUFFER_SIZE);
			if(pfnAoutOpen != NULL) {
				hdret = (*pfnAoutOpen)(stHdrAudioConfig);
			}
			else {
				hdret = (HDRET)eTC_HDR_RET_NG_RSC;
			}

			if(hdret == (HDRET)eTC_HDR_RET_OK) {
				fHdrAudioOpen = 1;
			}
			else {
				HDRCUI_AUD_LOG(CUI_ERR_LOG, 1U, "[%s:%d] Failed to open HD Radio audio.\n", __func__, __LINE__);
			}

		}
		else {
			HDRCUI_AUD_LOG(CUI_ERR_LOG, 1U, "[%s:%d] Already, HD Radio audio opened.\n", __func__, __LINE__);
			hdret = (HDRET)eTC_HDR_RET_NG_ALREADY_OPEN;
		}
	}
	else {
		HDRCUI_AUD_LOG(CUI_ERR_LOG, 1U, "[%s:%d] Failed to open HD Radio audio.\n", __func__, __LINE__);
		hdret = (HDRET)eTC_HDR_RET_NG_NOT_YET_INIT;
	}
	tchdradiocui_audio_mutexUnlock();

	return hdret;
}

HDRET tchdradiocui_audio_close(void)
{
	HDRET hdret = (HDRET)eTC_HDR_RET_OK;

	if(fHdrAudioOpen != (U32)0) {
		tchdradiocui_audio_mutexLock();
		fHdrAudioOpen = 0;
		(void)tcradiohal_audiofifo_reset(HDR_AUD_FIFO_INDEX);    // Reset audio fifo buffer. Replace with your audio buffer.
		if(pfnAoutClose != NULL) {
			(void)(*pfnAoutClose)();
		}
		tchdradiocui_audio_mutexUnlock();
	}
	else {
		HDRCUI_AUD_LOG(CUI_ERR_LOG, 1U, "[%s:%d] Failed to close HD Radio\n", __func__, __LINE__);
		hdret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	return hdret;
}

void tchdradiocui_audioQueueCallBack(void *pOutBuf, S32 frames, U32 samplerate)
{
	if(frames > 0) {
		U64 uiFrames = (U64)frames*4U;
		if(uiFrames > 0x0FFFFFFFFU) {
			uiFrames = 0x0FFFFFFFFU;
		}
		(void)tcradiohal_audiofifo_pushData((S32)HDR_AUD_FIFO_INDEX, (U8*)pOutBuf, (U32)uiFrames);    // Push audio data into the fifo buffer. Replace with your audio buffer.
	}
	(void)(samplerate);
}

static HDRET tchdradiocui_audio_playbackHandler(void)
{
	HDRET hdret = (HDRET)eTC_HDR_RET_NG_RSC;
	S32 frames = 0;

	if(fHdrAudioOpen > (U32)0) {
		tchdradiocui_audio_mutexLock();
		frames = tcradiohal_audiofifo_popData(HDR_AUD_FIFO_INDEX, hdrAudioBuffer, HDR_AUDIO_BUFFER_SIZE);    // Pop audio data from the fifo buffer. Replace with your audio buffer.
#if defined(__linux__) && !defined(__ANDROID__)
		// PCM data unit is bytes in android. but it is frames in linux.
		frames = frames/4;
#endif

		if(frames > 0) {
			if(pfnAoutWrite != NULL) {
				S32 ret = (*pfnAoutWrite)(hdrAudioBuffer, frames);
				if(ret == 0) {
					hdret = (HDRET)eTC_HDR_RET_OK;
				}
			}
		}
		tchdradiocui_audio_mutexUnlock();
	}

	return hdret;
}

void *tchdradiocui_audioThread(void *arg)
{
	HDRET hdret = (HDRET)eTC_HDR_RET_OK;
	SLONG sret = syscall(SYS_gettid);
#ifdef __aarch64__
	if((sret >= 0) && (sret <= 0x0FFFFFFFFL))
#else	// 32bit
	if(sret >= 0)
#endif
	{
		(void)setpriority((S32)PRIO_PROCESS, (U32)sret, -8);
	}
	(void)prctl(PR_SET_NAME, "TCHDRCUI_AUDIO",0,0,0);

	fThreadRunning = 1;
	while(fThreadRunning > 0) {
		hdret = tchdradiocui_audio_playbackHandler();
		if(hdret != (HDRET)eTC_HDR_RET_OK) {
			(void)usleep(SOUND_THREAD_TIME_INTERVAL*1000);
		}
		else {
			(void)usleep(1000);
		}
	}

	tchdradiocui_audio_release();
	fThreadRunning = -1;
	if(arg != NULL) {
		*(U8*)arg = 0;
	}
	return (NULL);
}

#endif
