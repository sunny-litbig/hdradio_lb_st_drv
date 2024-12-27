/*******************************************************************************

*   FileName : tchdr_service.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio framework service functions and definitions

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
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#include "tchdr_common.h"

#include "hdrBbSrc.h"
#include "tchdr_log.h"
#include "hdrSis.h"
#include "hdrAudio.h"
#include "hdrBlend.h"
#include "hdrPsd.h"
#include "hdrPhy.h"
#include "hdrSig.h"
#include "hdrAas.h"
#include "hdrAlerts.h"
#include "hdrTest.h"

#include "tchdr_bytestream.h"
#include "tchdr_cmdtune.h"
#include "tchdr_cmdcallbacks.h"
#include "tchdr_callback_conf.h"
#include "tchdr_msg.h"
#include "tchdr_api.h"
#include "tchdr_psd.h"
#include "tchdr_sis.h"
#include "tchdr_sig.h"
#include "tchdr_aas.h"
#include "tchdr_alert.h"
#include "tchdr_service.h"
#include "tchdr_framework.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
stTCHDR_TUNE_INFO_t stTcHdrTuneInfo;

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/

/***************************************************
*                Local preprocessor                *
****************************************************/
#if 0
#define	PSD_DEBUG		/* for psd debugging */
#endif

#define	TCHDR_SERVICE_THREAD_TIME_INTERVAL		(10)		// 10ms

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*           Local constant definitions              *
****************************************************/
static stTCHDR_SVC_t stTcHdrService;

static stTC_HDR_PSD_t	stTcHdrMainPsd[eTC_HDR_PROGRAM_MAX];
static stTC_HDR_PSD_t	stTcHdrBsPsd[eTC_HDR_PROGRAM_MAX];
static stTC_HDR_SIS_t	stTcHdrMainSis;
static stTC_HDR_SIS_t	stTcHdrBsSis;
static stTC_HDR_LOT_t 	stTcHdrMainLot[HDR_MAX_NUM_SIG_SERVICES_PER_STATION];
static stTC_HDR_LOT_t	stTcHdrBsLot[HDR_MAX_NUM_SIG_SERVICES_PER_STATION];
static stTC_HDR_ALERT_MESSAGE_t stTcHdrMainAlert;
static stTC_HDR_ALERT_MESSAGE_t stTcHdrBsAlert;
static stTC_HDR_SIGNAL_STATUS_t stTcHdrPriSigStatus;
static stTC_HDR_SIGNAL_STATUS_t stTcHdrBsSigStatus;

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
static struct timespec service_ChkTimer, service_ChkTimeNow, service_ChkTimeDiff;
static U32 service_AccumMs=0, service_LoopMs=0, service_DumpCount=0;
static FILE *service_DumpFile;
#endif

/***************************************************
*          Local function prototypes               *
****************************************************/
static S32 tchdrsvc_isHDRadioData(U32 mainOrBs);	// main Hdr (0) or bs Hdr (1)
static U32 tchdrsvc_getCurrentProgram(U8 mask);
static U32 tchdrsvc_getAppMIMEType(HDR_instance_t *hdrInstance, U32 service_number, U16 port_number);
static void tchdrsvc_setEventMode(eTCHDR_SVC_CMD_t evtmode);
static HDRET tchdrsvc_flushLotObjects(U32 serviceNum);
static HDRET tchdrsvc_parsingXhdr(stTC_HDR_PSD_XHDR_FRAME_t* xhdr, const U8* data, U32 dataLen);
static HDRET tchdrsvc_enableGetAlert(eTC_HDR_ID_t id, U32 fEn);
static HDRET tchdrsvc_enableLotReassembly(HDR_instance_t *hdrInstance, U32 serviceNum, U32 en);
static HDRET tchdrsvc_checkLotComplete(stTCHDR_LOTPROC_PARAM_t* lpp);
static eTCHDR_SVC_CMD_t tchdrsvc_getEventMode(void);
static void tchdrsvc_doReassembly_Process(void);
static void tchdrsvc_doReassembly(eTC_HDR_ID_t id, U32 en);
static void tchdrsvc_appMessageParser(stTcHdrMsgBuf_t *pstMsg);
static void tchdrsvc_eventHandler(stTcHdrMsgBuf_t stRcvMsgQ);
static void tchdrsvc_notifyHandler(void);
static void tchdrsvc_gatherHdrDataHandler(void);
static void tchdrsvc_checkStatusPeriodicallyHandler(void);
static void tchdrsvc_callbackAppFunction(stTcHdrMsgBuf_t *pstMsg);
static eTCHDR_EVT_STS_t tchdrsvc_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_reset_bbp(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_set_mainAudioMode(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_set_tune(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_set_program(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_get_status(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_enable_get_psd(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_enable_get_sis(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_enable_get_sig(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_enable_get_aas(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_enable_get_lot(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_enable_get_alert(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_test(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_set_mute(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrsvc_event_set_audio_ctrl(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);

/***************************************************
*			function definition				*
****************************************************/
static void tchdrsvc_eventMessageProcess(stTcHdrMsgBuf_t *pstMsg)
{
	(void)tchdrsvc_getMessage(pstMsg);
	if(pstMsg->fNewMsg == (U32)eTCHDR_NEW_MSG_EXIST) {
		tchdrsvc_appMessageParser(pstMsg);
	}
	tchdrsvc_eventHandler(*pstMsg);
}

void *tchdrsvc_mainThread(void* arg)
{
	struct timespec timer;
	struct timespec timeNow;
	stTcHdrMsgBuf_t stRecivedMessage;

	(*stOsal.setPostThreadAttr)(stTcHdrManagerThdAttr, eTAG_SYS);	// Set thread nice & name
	(void)(*stOsal.osmemset)((void*)&stRecivedMessage, (S8)0, (U32)sizeof(stTcHdrMsgBuf_t));

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	service_DumpFile = fopen(DUMP_PATH"manager_thread_status.csv", "w");
	clock_gettime(LINUX_CLOCK_TIMER_TYPE, &service_ChkTimer);
#endif

	(void)clock_gettime(LINUX_CLOCK_TIMER_TYPE,&timer);	// Get current time.

	stTcHdrManagerThdAttr.thread_running = 1;
	while(stTcHdrManagerThdAttr.thread_running > 0) {
		tchdrsvc_eventMessageProcess(&stRecivedMessage);
#ifndef DISABLE_TCHDR_DATA_API
        tchdrsvc_checkStatusPeriodicallyHandler();
        tchdrsvc_gatherHdrDataHandler();
#endif
		tchdrsvc_notifyHandler();

		// Wait until next shot.
		(void)clock_nanosleep(LINUX_CLOCK_TIMER_TYPE, TIMER_ABSTIME, &timer, NULL);

		// Calculate next shot.
		//timer.tv_nsec += (TCHDR_SERVICE_THREAD_TIME_INTERVAL * 1000000) + 500000;		// 10ms = 10,500,000ns
		timer.tv_nsec = (*stArith.slongadd)(timer.tv_nsec, (SLONG)(*stArith.s32mul)(TCHDR_SERVICE_THREAD_TIME_INTERVAL, 1000000));	// 10ms = 10,500,000ns
		timer.tv_nsec = (*stArith.slongadd)(timer.tv_nsec, 500000L);

		while (timer.tv_nsec >= NSEC_PER_SEC) {
			timer.tv_nsec -= NSEC_PER_SEC;
			timer.tv_sec++;
		}

		// If we fall too far behind, reset the timer
		(void)clock_gettime(LINUX_CLOCK_TIMER_TYPE, &timeNow);
		if(timeNow.tv_sec > timer.tv_sec) {
			timer = timeNow;
		}
#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &service_ChkTimeNow);
		service_ChkTimeDiff.tv_sec = service_ChkTimeNow.tv_sec - service_ChkTimer.tv_sec;
		service_ChkTimeDiff.tv_nsec = service_ChkTimeNow.tv_nsec - service_ChkTimer.tv_nsec;
		if(service_ChkTimeDiff.tv_nsec < 0) {
			service_ChkTimeDiff.tv_sec--;
			service_ChkTimeDiff.tv_nsec += 1e9;
		}
		service_LoopMs = (service_ChkTimeDiff.tv_sec*1000) + (service_ChkTimeDiff.tv_nsec/1000000);
		if(service_DumpFile != NULL) {
			fprintf(service_DumpFile, "%d,%d,%d\n", service_DumpCount++, service_AccumMs+=service_LoopMs, service_LoopMs);
			fflush(service_DumpFile);
		}
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &service_ChkTimer);
#endif
	}

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	if(service_DumpFile != NULL) {
		fclose(service_DumpFile);
	}
#endif
	stTcHdrManagerThdAttr.thread_running = -1;
	(void)tchdrsvc_deinit();
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrManager Thread Sequence 11...\n");
	return pNULL;
}

HDRET tchdr_getHdrIdFromInstanceNumber(eTC_HDR_ID_t *hdrID, U32 instanceNum)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(hdrID != NULL) {
		U32 currentHdrType = tchdrfwk_getHdrType();
		if(currentHdrType == HDR_1p5_DUAL_MRC_CONFIG) {
			if(instanceNum == 0U) {
				*hdrID = eTC_HDR_ID_MAIN;
			}
			else if(instanceNum == 1U) {
				*hdrID = eTC_HDR_ID_MRC;
			}
			else if(instanceNum == 2U) {
				*hdrID = eTC_HDR_ID_BS;
			}
			else if(instanceNum == 3U) {
				*hdrID = eTC_HDR_ID_BS_MRC;
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to get HDR ID because of invalid instance number.\n");
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else if(currentHdrType == HDR_1p5_MRC_CONFIG) {
			if(instanceNum == 0U) {
				*hdrID = eTC_HDR_ID_MAIN;
			}
			else if(instanceNum == 1U) {
				*hdrID = eTC_HDR_ID_MRC;
			}
			else if(instanceNum == 2U) {
				*hdrID = eTC_HDR_ID_BS;
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to get HDR ID because of invalid instance number.\n");
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else if(currentHdrType == HDR_1p5_CONFIG) {
			if(instanceNum == 0U) {
				*hdrID = eTC_HDR_ID_MAIN;
			}
			else if(instanceNum == 1U) {
				*hdrID = eTC_HDR_ID_BS;
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to get HDR ID because of invalid instance number.\n");
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else if(currentHdrType == HDR_1p0_MRC_CONFIG) {
			if(instanceNum == 0U) {
				*hdrID = eTC_HDR_ID_MAIN;
			}
			else if(instanceNum == 1U) {
				*hdrID = eTC_HDR_ID_MRC;
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to get HDR ID because of invalid instance number.\n");
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else {
			if(instanceNum == 0U) {
				*hdrID = eTC_HDR_ID_MAIN;
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to get HDR ID because of invalid instance number.\n");
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
	}
	else {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to get HDR ID because of null pointer parameter(*hdrID).\n");
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
	}

	return ret;
}

HDRET tchdr_getHdrTuneInfoWithInstanceNumber(stTCHDR_TUNE_t *tuneTo, U32 instanceNum)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(tuneTo != NULL) {
		U32 currentHdrType = tchdrfwk_getHdrType();
		if(currentHdrType == HDR_1p5_DUAL_MRC_CONFIG) {
			if(instanceNum == 0U) {
				*tuneTo = stTcHdrTuneInfo.mainInstance;
			}
			else if(instanceNum == 1U) {
				*tuneTo = stTcHdrTuneInfo.mrcInstance;
			}
			else if(instanceNum == 2U) {
				*tuneTo = stTcHdrTuneInfo.bsInstance;
			}
			else if(instanceNum == 3U) {
				*tuneTo = stTcHdrTuneInfo.bsmrcInstance;
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else if(currentHdrType == HDR_1p5_MRC_CONFIG) {
			if(instanceNum == 0U) {
				*tuneTo = stTcHdrTuneInfo.mainInstance;
			}
			else if(instanceNum == 1U) {
				*tuneTo = stTcHdrTuneInfo.mrcInstance;
			}
			else if(instanceNum == 2U) {
				*tuneTo = stTcHdrTuneInfo.bsInstance;
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else if(currentHdrType == HDR_1p5_CONFIG) {
			if(instanceNum == 0U) {
				*tuneTo = stTcHdrTuneInfo.mainInstance;
			}
			else if(instanceNum == 1U) {
				*tuneTo = stTcHdrTuneInfo.bsInstance;
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else if(currentHdrType == HDR_1p0_MRC_CONFIG) {
			if(instanceNum == 0U) {
				*tuneTo = stTcHdrTuneInfo.mainInstance;
			}
			else if(instanceNum == 1U) {
				*tuneTo = stTcHdrTuneInfo.mrcInstance;
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else {
			if(instanceNum == 0U) {
				*tuneTo = stTcHdrTuneInfo.mainInstance;
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}

		if(ret < 0) {
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to get tune information because of invalid instance number.\n");
		}
	}
	else {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to get tune information because of null pointer parameter(*tuneTo).\n");
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
	}

	return ret;
}

void tchdrsvc_setOpenStatus(U32 sts)
{
	if(sts > 0U) {
		stTcHdrService.fOpen = 1;
	}
	else {
		stTcHdrService.fOpen = 0;
	}
}

U32 tchdrsvc_getOpenStatus(void)
{
	return stTcHdrService.fOpen;
}

static void tchdrsvc_setEventMode(eTCHDR_SVC_CMD_t evtmode)
{
	stTcHdrService.eEventMode = evtmode;
}

static eTCHDR_SVC_CMD_t tchdrsvc_getEventMode(void)
{
	return stTcHdrService.eEventMode;
}

static void tchdrsvc_sendNotifyMessage(U32 eRcvID, U32 mode, const U32 *data, const void *pdata, S32 err)
{
	U32 txmode = (*stArith.u32add)(mode, (U32)TCHDR_SVC_NOTIFY_WT);

	switch(eRcvID) {
		case (U32)eTCHDR_SENDER_ID_APP:
			(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_SERVICE:
			(void)tchdrsvc_sendMessage(eTCHDR_SENDER_ID_SERVICE, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_AUDIO:
			(void)tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_SERVICE, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_BBINPUT:
			(void)tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_SERVICE, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MAIN:
			(void)tchdrmain_sendMessage(eTCHDR_SENDER_ID_SERVICE, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MRC:
			(void)tchdrmrc_sendMessage(eTCHDR_SENDER_ID_SERVICE, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BS:
			(void)tchdrbs_sendMessage(eTCHDR_SENDER_ID_SERVICE, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BLENDING:
			(void)tchdrblending_sendMessage(eTCHDR_SENDER_ID_SERVICE, txmode, data, pdata, err);
			break;
		default:
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, " %s : Invalid received ID[%d].\n", __func__, eRcvID);
			break;
	}
}

static void tchdrsvc_appMessageParser(stTcHdrMsgBuf_t *pstMsg)
{
	switch(pstMsg->uiMode) {
		case (U32)eTCHDR_SVC_CMD_OPEN:
		case (U32)eTCHDR_SVC_CMD_CLOSE:
		case (U32)eTCHDR_SVC_CMD_RESET_BBP:
		case (U32)eTCHDR_SVC_CMD_SET_MAIN_AUDIO_MODE:
		case (U32)eTCHDR_SVC_CMD_SET_TUNE:
		case (U32)eTCHDR_SVC_CMD_SET_PROGRAM:
		case (U32)eTCHDR_SVC_CMD_SET_MUTE:
		case (U32)eTCHDR_SVC_CMD_SET_AUDIO_CTRL:
		case (U32)eTCHDR_SVC_CMD_GET_STATUS:
		case (U32)eTCHDR_SVC_CMD_ENABLE_GET_PSD:
		case (U32)eTCHDR_SVC_CMD_ENABLE_GET_SIS:
		case (U32)eTCHDR_SVC_CMD_ENABLE_GET_SIG:
		case (U32)eTCHDR_SVC_CMD_ENABLE_GET_AAS:
		case (U32)eTCHDR_SVC_CMD_ENABLE_GET_LOT:
		case (U32)eTCHDR_SVC_CMD_ENABLE_GET_ALERT:
		case (U32)eTCHDR_SVC_CMD_TEST:
			tchdrsvc_setEventMode((eTCHDR_SVC_CMD_t)pstMsg->uiMode);
			break;

		default:
			// Notification of the other thread: Do it here.
			switch(pstMsg->uiMode) {
				// BBInput Notification
				case (U32)eTCHDR_BBINPUT_NOTIFY_OPEN:
					tchdrsvc_setOpenStatus(1);
					stTcHdrService.mainHdr.pty.fNotify = 1;
					(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_OPEN, pNULL, pNULL, pstMsg->iError);
					break;

				case (U32)eTCHDR_BBINPUT_NOTIFY_CLOSE:
					tchdrsvc_setOpenStatus(0);
					(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_CLOSE, pNULL, pNULL, pstMsg->iError);
					break;

				case (U32)eTCHDR_BBINPUT_NOTIFY_RESET_MAIN:
					break;

				case (U32)eTCHDR_BBINPUT_NOTIFY_RESET_BS:
					break;

				case (U32)eTCHDR_BBINPUT_NOTIFY_TUNE:
					(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_TUNE, pstMsg->uiData, pNULL, pstMsg->iError);
                    // tchdrsvc_event_set_tune 에서 여기로 변경 (이전 서비스의 PTY 가 전달되는 이슈 수정)
                    stTcHdrService.mainHdr.pty.fNotify = 1U;
                    stTcHdrService.mainHdr.psd.checkInterval = 10U;
                    stTcHdrService.mainHdr.sis.checkInterval = 100U;
                    //stTcHdrService.mainHdr.status.fNotify = 1U;
                    //stTcHdrService.mainSts.acqStatus.all = 0U;
                    stTcHdrService.mainSts.cdno = 0U;
                    stTcHdrService.mainSts.hybrid = false;
                    // ~tchdrsvc_event_set_tune 에서 여기로 변경 (이전 서비스의 PTY 가 전달되는 이슈 수정)
				#if 0
					if(pstMsg->iError == 0) {
						S32 ret = 0;
						eTC_HDR_ID_t id = (eTC_HDR_ID_t)pstMsg->uiData[0];
						stHDR_FRAMEWORK_DATA_t* frameworkData = tchdrfwk_getDataStructPtr();
						if(id == eTC_HDR_ID_MAIN) {
							U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
							if((pstMsg->uiData[1] > (U32)0) || (pstMsg->uiData[2] > (U32)0)) {		// When the band or frequency is changed
								(void)HDR_get_playing_program(&frameworkData->hdrInstance[0], &stTcHdrService.mainHdrProgNum);
								uiSendMsg[0] = (U32)stTcHdrService.mainHdrProgNum;
								(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_PROGRAM, uiSendMsg, pNULL, ret);
							}
						}
					}
				#endif
					break;

				// HDR Audio Notification
				case (U32)eTCHDR_AUDIO_NOTIFY_START:
				case (U32)eTCHDR_AUDIO_NOTIFY_STOP:
					(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_AUDIO_CTRL, pstMsg->uiData, pNULL, pstMsg->iError);
					break;

				case (U32)eTCHDR_AUDIO_NOTIFY_MUTE:
					(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_MUTE, pstMsg->uiData, pNULL, pstMsg->iError);
					break;

				// HDR Blending Notification
				case (U32)eTCHDR_BLENDING_NOTIFY_AUDIO_MODE:
					switch(pstMsg->uiData[0]) {
						case (U32)eHDR_AUDIO_BLEND:
							pstMsg->uiData[0] = (U32)eTC_HDR_AUDIO_BLEND;
							break;
						case (U32)eHDR_AUDIO_ANALOG_ONLY:
							pstMsg->uiData[0] = (U32)eTC_HDR_AUDIO_ANALOG;
							break;
						case (U32)eHDR_AUDIO_DIGITAL_ONLY:
							pstMsg->uiData[0] = (U32)eTC_HDR_AUDIO_DIGITAL;
							break;
						case (U32)eHDR_AUDIO_ANALOG_SPLIT:
							pstMsg->uiData[0] = (U32)eTC_HDR_AUDIO_SPLIT;
							break;
						default:
							pstMsg->iError = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
							break;
					}
					(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_AUDIO_MODE, pstMsg->uiData, pNULL, pstMsg->iError);
					break;

				default:
					/* Nothing To Do */
					break;
			}
			break;
	}
}

static void tchdrsvc_eventHandler(stTcHdrMsgBuf_t stRcvMsgQ)
{
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	eTCHDR_SVC_CMD_t eNowEvtMode;
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};

	eNowEvtMode = tchdrsvc_getEventMode();

	switch(eNowEvtMode) {
		case eTCHDR_SVC_CMD_OPEN:
			eEvtSt = tchdrsvc_event_open(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_CLOSE:
			eEvtSt = tchdrsvc_event_close(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_RESET_BBP:
			eEvtSt = tchdrsvc_event_reset_bbp(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_SET_MAIN_AUDIO_MODE:
			eEvtSt = tchdrsvc_event_set_mainAudioMode(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_SET_TUNE:
			eEvtSt = tchdrsvc_event_set_tune(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_SET_PROGRAM:
			eEvtSt = tchdrsvc_event_set_program(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_SET_MUTE:
			eEvtSt = tchdrsvc_event_set_mute(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_SET_AUDIO_CTRL:
			eEvtSt = tchdrsvc_event_set_audio_ctrl(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_GET_STATUS:
			eEvtSt = tchdrsvc_event_get_status(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_ENABLE_GET_PSD:
			eEvtSt = tchdrsvc_event_enable_get_psd(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_ENABLE_GET_SIS:
			eEvtSt = tchdrsvc_event_enable_get_sis(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_ENABLE_GET_SIG:
			eEvtSt = tchdrsvc_event_enable_get_sig(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_ENABLE_GET_AAS:
			eEvtSt = tchdrsvc_event_enable_get_aas(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_ENABLE_GET_LOT:
			eEvtSt = tchdrsvc_event_enable_get_lot(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_ENABLE_GET_ALERT:
			eEvtSt = tchdrsvc_event_enable_get_alert(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_SVC_CMD_TEST:
			eEvtSt = tchdrsvc_event_test(stRcvMsgQ, uiSendMsg, &ret);
			break;

		default:
			/* Nothing To Do */
			break;
	}

	tchdrsvc_setEventMode(eTCHDR_SVC_CMD_NULL);

	switch(eEvtSt) {
		/* Job Good Complete -> No Notify */
		case eTCHDR_EVT_STS_DONE:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eTCHDR_EVT_STS_DONE_NOTIFY:
			tchdrsvc_sendNotifyMessage(stRcvMsgQ.uiSender, (U32)eNowEvtMode, uiSendMsg, pNULL, ret);
			break;

		/* Return Error */
		default:
			/* Nothing To Do */
			break;
	}
}

static void tchdrsvc_getPsd(HDR_instance_t* hdrInstance, U8 userFieldBitmask, U8 programBitmask, stTC_HDR_PSD_t *dataOut)
{
	U8 program;
	HDR_psd_fields_t enabledFields;
	HDR_psd_fields_t fieldBitmask;
	HDR_psd_data_t psdData;
//	U32 commentSubfields = 0x0FF;
//	U32 commercialSubfields = 0x0FF;

	enabledFields.all = HDR_psd_get_enabled_fields(hdrInstance).all;
	fieldBitmask.all = userFieldBitmask;
#ifdef PSD_DEBUG
	(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "\n");
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "enable fields bitmask [0x%02x], user fields bitmask [0x%02x], program bitmask[0x%02x]\n", enabledFields.all, fieldBitmask.all, programBitmask);
#endif

	if((hdrInstance != NULL) && (dataOut != NULL)) {
		for(program = 0U; program < HDR_MAX_NUM_PROGRAMS; ++program) {
			if((programBitmask & ((U32)1 << program)) == 0U) {
				continue;
			}

			// Title
			if((fieldBitmask.title == 1U) && (enabledFields.title == 1U)) {
				psdData.length = 0; // erase the old value
				(void)(*stOsal.osmemset)((void*)&psdData, (S8)0, (U32)sizeof(HDR_psd_data_t));
				(void)HDR_psd_get_title(hdrInstance, (HDR_program_t)program, &psdData);

				dataOut[program].title.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
				dataOut[program].title.len = psdData.length;
				(void)(*stOsal.osmemcpy)((void*)dataOut[program].title.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
			#ifdef PSD_DEBUG
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Title: PN[%02d]\n", program);
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "CharType[0x%02x] DataLength[%d]\n", dataOut[program].title.charType, dataOut[program].title.len);
				if(psdData.length > 0) {
					U32 i;
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "");
					for(i=0; i<psdData.length; i++) {
						(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "%c", dataOut[program].title.data[i]);
					}
					(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "\n");
				}
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "-----------------------------------------------\n");
			#endif
			}

			// Artist
			if((fieldBitmask.artist == 1U) && (enabledFields.artist == 1U)) {
				psdData.length = 0; // erase the old value
				(void)(*stOsal.osmemset)((void*)&psdData, (S8)0, (U32)sizeof(HDR_psd_data_t));
				(void)HDR_psd_get_artist(hdrInstance, (HDR_program_t)program, &psdData);

				dataOut[program].artist.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
				dataOut[program].artist.len = psdData.length;
				(void)(*stOsal.osmemcpy)((void*)dataOut[program].artist.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
			#ifdef PSD_DEBUG
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Artist: PN[%02d]\n", program);
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "CharType[0x%02x] DataLength[%d]\n", dataOut[program].artist.charType, dataOut[program].artist.len);
				if(psdData.length > 0) {
					U32 i;
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "");
					for(i=0; i<psdData.length; i++) {
						(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "%c", dataOut[program].artist.data[i]);
					}
					(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "\n");
				}
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "-----------------------------------------------\n");
			#endif
			}

			// Album
			if((fieldBitmask.album == 1U) && (enabledFields.album == 1U)) {
				psdData.length = 0; // erase the old value
				(void)(*stOsal.osmemset)((void*)&psdData, (S8)0, (U32)sizeof(HDR_psd_data_t));
				(void)HDR_psd_get_album(hdrInstance, (HDR_program_t)program, &psdData);

				dataOut[program].album.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
				dataOut[program].album.len = psdData.length;
				(void)(*stOsal.osmemcpy)((void*)dataOut[program].album.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
			#ifdef PSD_DEBUG
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Album: PN[%02d]\n", program);
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "CharType[0x%02x] DataLength[%d]\n", dataOut[program].album.charType, dataOut[program].album.len);
				if(psdData.length > 0) {
					U32 i;
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "");
					for(i=0; i<psdData.length; i++) {
						(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "%c", dataOut[program].album.data[i]);
					}
					(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "\n");
				}
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "-----------------------------------------------\n");
			#endif
			}

			// Genre
			if((fieldBitmask.genre == 1U) && (enabledFields.genre == 1U)) {
				psdData.length = 0; // erase the old value
				(void)(*stOsal.osmemset)((void*)&psdData, (S8)0, (U32)sizeof(HDR_psd_data_t));
				(void)HDR_psd_get_genre(hdrInstance, (HDR_program_t)program, &psdData);

				dataOut[program].genre.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
				dataOut[program].genre.len = psdData.length;
				(void)(*stOsal.osmemcpy)((void*)dataOut[program].genre.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
			#ifdef PSD_DEBUG
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Genre: PN[%02d]\n", program);
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "CharType[0x%02x] DataLength[%d]\n", dataOut[program].genre.charType, dataOut[program].genre.len);
				if(psdData.length > 0) {
					U32 i;
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "");
					for(i=0; i<psdData.length; i++) {
						(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "%c", dataOut[program].genre.data[i]);
					}
					(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "\n");
				}
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "-----------------------------------------------\n");
			#endif
			}

			// Comment
			if((fieldBitmask.comment == 1U) && (enabledFields.comment == 1U)) {
				U32 subfield=0;
				for(subfield = 0; subfield < (U32)HDR_PSD_NUM_COMMENT_SUBFIELDS; subfield++)
				{
				#if 0
					// Dead Code: Execution cannot reach this statement: continue
					if((commentSubfields & ((U32)1 << subfield)) == 0U) {
						// subfield wasn't requested
						continue;
					}
				#endif

					psdData.length = 0; // erase the old value
					(void)(*stOsal.osmemset)((void*)&psdData, (S8)0, (U32)sizeof(HDR_psd_data_t));
					(void)HDR_psd_get_comment(hdrInstance, (HDR_program_t)program, (HDR_psd_comm_subfield_t)subfield, &psdData);

					if(subfield == (U32)HDR_PSD_COMMENT_LANGUAGE) {
						dataOut[program].comment.language.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
						dataOut[program].comment.language.len = psdData.length;
						(void)(*stOsal.osmemcpy)((void*)dataOut[program].comment.language.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
					#ifdef PSD_DEBUG
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Comment Language: PN[%02d]\n", program);
					#endif
					}
					else if(subfield == (U32)HDR_PSD_COMMENT_SHORT_CONTENT) {
						dataOut[program].comment.shortContent.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
						dataOut[program].comment.shortContent.len = psdData.length;
						(void)(*stOsal.osmemcpy)((void*)dataOut[program].comment.shortContent.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
					#ifdef PSD_DEBUG
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Comment Short Content: PN[%02d]\n", program);
					#endif
					}
					else if(subfield == (U32)HDR_PSD_COMMENT_ACTUAL_TEXT) {
						dataOut[program].comment.actualText.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
						dataOut[program].comment.actualText.len = psdData.length;
						(void)(*stOsal.osmemcpy)((void*)dataOut[program].comment.actualText.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
					#ifdef PSD_DEBUG
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Comment Actual Text: PN[%02d]\n", program);
					#endif
					}
					else {
					#ifdef PSD_DEBUG
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Comment Unknown[%d]: PN[%02d]\n", subfield, program);
					#endif
					}

				#ifdef PSD_DEBUG
					if(subfield < (U32)HDR_PSD_NUM_COMMENT_SUBFIELDS) {
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "CharType[0x%02x] DataLength[%d]\n", psdData.data_type, psdData.length);
						if(psdData.length > 0) {
							U32 i;
							(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "");
							for(i=0; i<psdData.length; i++) {
								(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "%c", psdData.data[i]);
							}
							(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "\n");
						}
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "-----------------------------------------------\n");
					}
				#endif
				}
			}

#if 0
			// UFID
			if(fieldBitmask.UFID == 1 && enabledFields.UFID == 1) {
				U32 n, subfield;
				for(n = 0; n < HDR_MAX_NUM_UFIDS; n++) {
					for(subfield = 0; subfield < HDR_PSD_NUM_UFID_SUBFIELDS; subfield++) {
						if((ufidSubfields & (1 << subfield)) == 0) {
							// subfield wasn't requested
							continue;
						}

						psdData.length = 0; // erase the old value
						(void)(*stOsal.osmemset)((void*)&psdData, (S8)0, (U32)sizeof(HDR_psd_data_t));
						HDR_psd_get_ufid(hdrInstance, program, n, subfield, &psdData);

						dataOut[offset] = (U8)program;
						offset++;
						dataOut[offset] = 0x20;
						offset++;
						dataOut[offset] = 0; // Field takes up 2 bytes
						offset++;
						dataOut[offset] = (1 << subfield); // Subfield
						offset++;
						dataOut[offset] = (U8)psdData.data_type;
						offset++;
						dataOut[offset] = (U8)psdData.length;
						offset++;
						(void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
						offset += psdData.length;
					#ifdef PSD_DEBUG
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Program Number [%02d]\n", program);
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "UFID[0x%02x] SubField[0x%02x]\n", dataOut[index+1], dataOut[index+3]);
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "DataType[0x%02x] DataLength[%d]\n", dataOut[index+4], dataOut[index+5]);
						for( ; index<offset; ) {
							(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "");
							for(i=0; i<16; ) {
								(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "%02x ", dataOut[index + i]);
								i++;
								if(index + i >= offset) {
									break;
								}
							}
							index += i;
							(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "\n");
						}
						if(psdData.length > 0) {
							(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "");
							(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "UFID: ");
							for(i=0; i<psdData.length; i++) {
								(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "%c", psdData.data[i]);
							}
							(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "\n");
						}
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "-----------------------------------------------\n");
					#endif
					}
				}
			}
#endif
			// Commercial
			if((fieldBitmask.commercial == 1U) && (enabledFields.commercial == 1U)) {
				U32 subfield=0;
				for(subfield = 0; subfield < (U32)HDR_PSD_NUM_COMR_SUBFIELDS; subfield++)
				{
				#if 0
					// Dead Code: Execution cannot reach this statement: continue
					if((commercialSubfields & ((U32)1 << subfield)) == 0U) {
						// subfield wasn't requested
						continue;
					}
				#endif

					psdData.length = 0; // erase the old value
					(void)(*stOsal.osmemset)((void*)&psdData, (S8)0, (U32)sizeof(HDR_psd_data_t));
					(void)HDR_psd_get_commercial(hdrInstance, (HDR_program_t)program, (HDR_psd_comr_subfield_t)subfield, &psdData);

					if(subfield == (U32)HDR_PSD_COMR_PRICE_STRING) {
						dataOut[program].commercial.priceString.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
						dataOut[program].commercial.priceString.len = psdData.length;
						(void)(*stOsal.osmemcpy)((void*)dataOut[program].commercial.priceString.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
					#ifdef PSD_DEBUG
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Commercial Price String: PN[%02d]\n", program);
					#endif
					}
					else if(subfield == (U32)HDR_PSD_COMR_VALID_UNTIL) {
						dataOut[program].commercial.validUntil.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
						dataOut[program].commercial.validUntil.len = psdData.length;
						(void)(*stOsal.osmemcpy)((void*)dataOut[program].commercial.validUntil.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
					#ifdef PSD_DEBUG
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Commercial Valid Until: PN[%02d]\n", program);
					#endif
					}
					else if(subfield == (U32)HDR_PSD_COMR_CONTACT_URL) {
						dataOut[program].commercial.contactURL.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
						dataOut[program].commercial.contactURL.len = psdData.length;
						(void)(*stOsal.osmemcpy)((void*)dataOut[program].commercial.contactURL.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
					#ifndef PSD_DEBUG
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Commercial Received Contact URL: PN[%02d]\n", program);
					#endif
					}
					else if(subfield == (U32)HDR_PSD_COMR_RECEIVED_AS) {
						dataOut[program].commercial.receivedAs.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
						dataOut[program].commercial.receivedAs.len = psdData.length;
						(void)(*stOsal.osmemcpy)((void*)dataOut[program].commercial.receivedAs.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
					#ifdef PSD_DEBUG
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Commercial Received As: PN[%02d]\n", program);
					#endif
					}
					else if(subfield == (U32)HDR_PSD_COMR_SELLER_NAME) {
						dataOut[program].commercial.sellerName.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
						dataOut[program].commercial.sellerName.len = psdData.length;
						(void)(*stOsal.osmemcpy)((void*)dataOut[program].commercial.sellerName.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
					#ifdef PSD_DEBUG
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Commercial Seller Name: PN[%02d]\n", program);
					#endif
					}
					else if(subfield == (U32)HDR_PSD_COMR_DESCRIPTION) {
						dataOut[program].commercial.description.charType = (eTC_HDR_PSD_CHAR_TYPE_t)psdData.data_type;
						dataOut[program].commercial.description.len = psdData.length;
						(void)(*stOsal.osmemcpy)((void*)dataOut[program].commercial.description.data, (void*)psdData.data, TC_HDR_PSD_MAX_LEN);
					#ifdef PSD_DEBUG
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Commercial Description: PN[%02d]\n", program);
					#endif
					}
					else {
					#ifdef PSD_DEBUG
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Commercial Unknown[%d]: PN[%02d]\n", subfield, program);
					#endif
					}

				#ifdef PSD_DEBUG
					if(subfield < (U32)HDR_PSD_NUM_COMR_SUBFIELDS) {
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "CharType[0x%02x] DataLength[%d]\n", psdData.data_type, psdData.length);
						if(psdData.length > 0) {
							U32 i;
							(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "");
							for(i=0; i<psdData.length; i++) {
								(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "%c", psdData.data[i]);
							}
							(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "\n");
						}
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "-----------------------------------------------\n");
					}
				#endif
				}
			}


			// XHDR
			if((fieldBitmask.XHDR == 1U) && (enabledFields.XHDR == 1U)) {
				U32 n = 0;
				S32 ret = -1;

				(void)(*stOsal.osmemset)((void*)&(dataOut[program].xhdr), (S8)0, (U32)sizeof(stTC_HDR_PSD_XHDR_FRAME_t));

				for(n = 0; n < HDR_MAX_NUM_XHDRS; n++) {
					psdData.length = 0; // erase the old value
					(void)(*stOsal.osmemset)((void*)&psdData, (S8)0, (U32)sizeof(HDR_psd_data_t));

					ret = HDR_psd_get_xhdr(hdrInstance, (HDR_program_t)program, n, &psdData);
					if(ret == 0) {
						dataOut[program].xhdr.program = program;
						(void)tchdrsvc_parsingXhdr(&(dataOut[program].xhdr), (U8*)(psdData.data), psdData.length);

						#ifdef PSD_DEBUG
						if(psdData.length > 0) {
							(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) Num[%d] numParams[0x%02d]", n, dataOut[program].xhdr.numParams);
							(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) PN[%02d] len[%d]", program, psdData.length);
							(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) Data ==> ");
							for(S32 index = 0; index < psdData.length; index++) {
								(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) 0x%02X ", psdData.data[index]);
							}
						}
						#endif // PSD_DEBUG
					}
				}

				#ifdef PSD_DEBUG
				for(S32 index = 0; index < dataOut[program].xhdr.numParams; index++) {
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, " ");
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) body Index (%d)", index);
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) MIME hash : 0x%X", dataOut[program].xhdr.mime_hash);
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) parameter ID : 0x%X", dataOut[program].xhdr.params[index].param_id);
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) length	: %d", dataOut[program].xhdr.params[index].length);
					switch(dataOut[program].xhdr.params[index].param_id) {
					case 0:
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) ParameterID (00) ===> lot ID	: %d", dataOut[program].xhdr.params[index].lot_id);
						break;
					case 1:
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) ParameterID (01) ===> Blank display");
						break;
					case 2:
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) ParameterID (02) ===> Flush Memory");
						break;
					default:
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) ParameterID (Unkown) ===> Value	: 0x");
						for(S32 i=0; i<dataOut[program].xhdr.params[index].length;i++) {
							(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "(XHDR) %X", dataOut[program].xhdr.params[index].value[i]);
						}
						(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, " ");
						break;
					}
				}
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "-----------------------------------------------\n");
				#endif // PSD_DEBUG
			}
	        /* Change : Pass the program number that has been changed.*/
	        (void)HDR_psd_clear_changed_program(hdrInstance,(HDR_program_t)program);
		}
	}
#ifdef PSD_DEBUG
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Finished to get PSD.\n");
#endif
}

static U32 tchdrsvc_getAppMIMEType(HDR_instance_t *hdrInstance, U32 service_number, U16 port_number)
{
	HDRET ret;
	U32 MIME_type = 0;
	U32 numComponent;
	HDR_sig_service_info_t serviceInfo;
	HDR_sig_service_component_t component;

	(void)(*stOsal.osmemset)((void*)&serviceInfo, (S8)0, (U32)sizeof(HDR_sig_service_info_t));
	ret = HDR_sig_get_service_info(hdrInstance, service_number, &serviceInfo);
	if(ret == 0) {
		for(numComponent = 0; numComponent < serviceInfo.num_components; numComponent++){
			(void)(*stOsal.osmemset)((void*)&component, (S8)0, (U32)sizeof(HDR_sig_service_component_t));
			ret = HDR_sig_get_service_component(hdrInstance, service_number, numComponent, &component);
			if((ret == 0) && (component.channel == port_number)){
				MIME_type = component.mime_hash_value;
				break;
			}
		}
	}

	return MIME_type;
}

static U32 tchdrsvc_getCurrentProgram(U8 mask)
{
	U32 i;
	U32 ret = 0U;

	for(i=0; i<HDR_MAX_NUM_PROGRAMS; i++) {
		if((mask & ((U8)1 << i)) == 0U) {
			continue;
		}
		else {
			ret = i+1U;
		}
		break;
	}
	return ret;
}

static void tchdrsvc_checkStatusPeriodicallyHandler(void)
{
	if(stTcHdrService.statusChkInterval > 0U) {
		stTcHdrService.statusChkInterval--;
	}
	else {
		// HD Radio Acquisition
		U32 curProgNum = (U32)stTcHdrService.mainHdrProgNum;
		HDR_program_types_t mainptys;
		HDR_program_types_t bsptys;

		(void)tchdrsvc_getHdrSignalStatus(eTC_HDR_ID_MAIN, &stTcHdrPriSigStatus);

		if((stTcHdrService.mainSts.acqStatus.all != stTcHdrPriSigStatus.acqStatus) ||
			((stTcHdrService.mainHdr.status.fNotify == 1U) && (stTcHdrPriSigStatus.acqStatus == 0U)))
		{
			void *pData[TCHDR_MSGQ_PDATA_LENGTH];
			stTcHdrService.mainSts.acqStatus.all = (U8)stTcHdrPriSigStatus.acqStatus;
			(void)(*stOsal.osmemset)((void*)pData, (S8)0, (U32)TCHDR_MSGQ_PDATA_LENGTH*(U32)sizeof(void*));
			pData[0] = (void*)&stTcHdrPriSigStatus;
			(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_SIGNAL, pNULL, (void*)pData, 0);
			stTcHdrService.mainHdr.status.fNotify = 0U;
		}

		if((stTcHdrService.mainSts.acqStatus.status.hdsig > 0U) && (stTcHdrService.mainSts.acqStatus.status.hdaud > 0U)) {
			HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(eTC_HDR_ID_MAIN);
			(void)HDR_get_program_types(hdrInstance, &mainptys);
			if((stTcHdrService.mainHdr.pty.value[curProgNum] != mainptys.value[curProgNum]) || (stTcHdrService.mainHdr.pty.fNotify > (U8)0)) {
				U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
				stTcHdrService.mainHdr.pty.fNotify = 0;
				stTcHdrService.mainHdr.pty.value[curProgNum] = mainptys.value[curProgNum];
				uiSendMsg[0] = (U32)eTC_HDR_ID_MAIN;
				uiSendMsg[1] = curProgNum;
				uiSendMsg[2] = stTcHdrService.mainHdr.pty.value[curProgNum];
				(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_PTY, uiSendMsg, pNULL, 0);
			}
		}

		if((tchdrfwk_getHdrType() == HDR_1p5_CONFIG) || (tchdrfwk_getHdrType() == HDR_1p5_MRC_CONFIG) || (tchdrfwk_getHdrType() == HDR_1p5_DUAL_MRC_CONFIG)) {
			(void)tchdrsvc_getHdrSignalStatus(eTC_HDR_ID_BS, &stTcHdrBsSigStatus);

			if((stTcHdrService.bsSts.acqStatus.all != stTcHdrBsSigStatus.acqStatus) ||
				((stTcHdrService.bsHdr.status.fNotify == 1U) && (stTcHdrPriSigStatus.acqStatus == 0U)))
			{
				void *pData[TCHDR_MSGQ_PDATA_LENGTH];
				stTcHdrService.bsSts.acqStatus.all = (U8)stTcHdrBsSigStatus.acqStatus;
				(void)(*stOsal.osmemset)((void*)pData, (S8)0, (U32)TCHDR_MSGQ_PDATA_LENGTH*(U32)sizeof(void*));
				pData[0] = (void*)&stTcHdrBsSigStatus;
				(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_SIGNAL, pNULL, (void*)pData, 0);
				stTcHdrService.bsHdr.status.fNotify = 0U;
			}

			if((stTcHdrService.bsSts.acqStatus.status.hdsig > 0U) && (stTcHdrService.bsSts.acqStatus.status.sisok > 0U)) {
				HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(eTC_HDR_ID_BS);
				if(hdrInstance != NULL) {
					(void)HDR_get_program_types(hdrInstance, &bsptys);
					if((stTcHdrService.bsHdr.pty.value[curProgNum] != bsptys.value[curProgNum]) || (stTcHdrService.bsHdr.pty.fNotify > (U8)0)) {
						U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
						stTcHdrService.bsHdr.pty.fNotify = 0;
						stTcHdrService.bsHdr.pty.value[curProgNum] = bsptys.value[curProgNum];
						uiSendMsg[0] = (U32)eTC_HDR_ID_BS;
						uiSendMsg[1] = curProgNum;
						uiSendMsg[2] = stTcHdrService.bsHdr.pty.value[curProgNum];
						(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_PTY, uiSendMsg, pNULL, 0);
					}
				}
			}
		}

		//LOT enable Reassembly
		tchdrsvc_doReassembly_Process();

		stTcHdrService.statusChkInterval = 25;		// thread interval time 20ms x 25 = 500ms
	}
}

static void tchdrsvc_doReassembly_Process(void)
{
	if(stTcHdrService.mainHdr.lot.fNotify == 1U) {
		(void)tchdrsvc_doReassembly(eTC_HDR_ID_MAIN, (U32)ON);
	}

	if(stTcHdrService.bsHdr.lot.fNotify == 1U) {
		(void)tchdrsvc_doReassembly(eTC_HDR_ID_BS, (U32)ON);
	}
}

static void tchdrsvc_doReassembly(eTC_HDR_ID_t id, U32 en)
{
	HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);

	HDR_sig_service_list_t serviceList;
	HDR_sig_service_info_t serviceInfo;

	if(hdrInstance != NULL) {
		for(U8 serviceType = 0U; serviceType < (U8)HDR_SIG_NUM_SERVICE_TYPES; ++serviceType) {
			if(HDR_sig_get_service_list(hdrInstance, (HDR_sig_service_type_t)serviceType, &serviceList) < 0) {
				break;
			}
			else {
				/* Nothing To Do */
			}

			for(U32 s = 0; s < serviceList.num_services; ++s) {
				if(HDR_sig_get_service_info(hdrInstance, serviceList.item[s].service_number, &serviceInfo) < 0) {
					break;
				}
				else {
					(void)tchdrsvc_enableLotReassembly(hdrInstance, serviceList.item[s].service_number, en);
				}
			}
		}
	}
}

static S32 tchdrsvc_isHDRadioData(U32 mainOrBs)	// main Hdr (0) or bs Hdr (1)
{
	S32 ret = 0;
	U8 acqStatus;

	if(mainOrBs == 0U) {
		acqStatus = stTcHdrService.mainSts.acqStatus.all;
        // fix 801.auto, 80.2.auto
        // HD signal & SIS enabled is the condition to display SIS & PSD
        // eBITMASK_SIGNAL_STATUS_HD_SIGNAL	= 0x01U,
        // eBITMASK_SIGNAL_STATUS_SIS			= 0x02U,
        // eBITMASK_SIGNAL_STATUS_SIS_OK		= 0x04U,
        // eBITMASK_SIGNAL_STATUS_HD_AUDIO		= 0x08U
		// if((acqStatus & (U8)0x0d) == 0x0dU) {
		if((acqStatus & (U8)0x07) == 0x07U) {
			ret = 1;
		}
	}
	else if(mainOrBs == 1U) {
		acqStatus = stTcHdrService.bsSts.acqStatus.all;
		if((acqStatus & (U8)0x07) == 0x07U) {
			ret = 1;
		}
	}
	else {
		ret = -1;
	}

	return ret;
}

static void tchdrsvc_tchdrsvc_buildLotFree(const stTCHDR_LOTPROC_PARAM_t* lpp)
{
	const stTCHDR_LOTPROC_PARAM_t* llpp = lpp;

	if(llpp->ServIdx < HDR_MAX_NUM_SIG_SERVICES_PER_STATION) {
		if(llpp->id == eTC_HDR_ID_BS) {
			stTcHdrBsLot[llpp->ServIdx].service_number = 0;
			stTcHdrBsLot[llpp->ServIdx].app_mime_hash = 0;
			stTcHdrBsLot[llpp->ServIdx].body_bytes_written = 0;
			(void)(*stOsal.osmemset)((void*)&(stTcHdrBsLot[llpp->ServIdx].header), (S8)0, (U32)sizeof(stTC_HDR_AAS_LOT_OBJECT_HEADER_t));
			if(stTcHdrBsLot[llpp->ServIdx].body != NULL) {
				(*stOsal.osfree)(stTcHdrBsLot[llpp->ServIdx].body);
				stTcHdrBsLot[llpp->ServIdx].body = NULL;
			}
		}
		else {
			stTcHdrMainLot[llpp->ServIdx].service_number = 0;
			stTcHdrMainLot[llpp->ServIdx].app_mime_hash = 0;
			stTcHdrMainLot[llpp->ServIdx].body_bytes_written = 0;
			(void)(*stOsal.osmemset)((void*)&(stTcHdrMainLot[llpp->ServIdx].header), (S8)0, (U32)sizeof(stTC_HDR_AAS_LOT_OBJECT_HEADER_t));
			if(stTcHdrMainLot[llpp->ServIdx].body != NULL) {
				(*stOsal.osfree)(stTcHdrMainLot[llpp->ServIdx].body);
				stTcHdrMainLot[llpp->ServIdx].body = NULL;
			}
		}
	}
}

static void tchdrsvc_tchdrsvc_buildLotAlloc(const stTCHDR_LOTPROC_PARAM_t* lpp)
{
	const stTCHDR_LOTPROC_PARAM_t* llpp = lpp;

	if(llpp->ServIdx < HDR_MAX_NUM_SIG_SERVICES_PER_STATION) {
		if(llpp->id == eTC_HDR_ID_BS) {
			stTcHdrBsLot[llpp->ServIdx].service_number = llpp->ServNum;
			stTcHdrBsLot[llpp->ServIdx].app_mime_hash = tchdrsvc_getAppMIMEType(llpp->hdrInstance, stTcHdrBsLot[llpp->ServIdx].service_number, llpp->port_number);
			stTcHdrBsLot[llpp->ServIdx].body = (U8*)(*stOsal.oscalloc)((size_t)stTcHdrBsLot[llpp->ServIdx].header.file_size, sizeof(S8));
			if(stTcHdrBsLot[llpp->ServIdx].body != NULL) {
				(void)(*stOsal.osmemset)((void*)&stTcHdrBsLot[llpp->ServIdx].body[0], (S8)0, stTcHdrBsLot[llpp->ServIdx].header.file_size);
			}
		}
		else {
			stTcHdrMainLot[llpp->ServIdx].service_number = llpp->ServNum;
			stTcHdrMainLot[llpp->ServIdx].app_mime_hash = tchdrsvc_getAppMIMEType(llpp->hdrInstance, stTcHdrMainLot[llpp->ServIdx].service_number, llpp->port_number);
			stTcHdrMainLot[llpp->ServIdx].body = (U8*)(*stOsal.oscalloc)((size_t)stTcHdrMainLot[llpp->ServIdx].header.file_size, sizeof(S8));
			if(stTcHdrMainLot[llpp->ServIdx].body != NULL) {
				(void)(*stOsal.osmemset)((void*)&stTcHdrMainLot[llpp->ServIdx].body[0], (S8)0, stTcHdrMainLot[llpp->ServIdx].header.file_size);
			}
		}
	}
}

static HDRET tchdrsvc_buildLotBody(const stTCHDR_LOTPROC_PARAM_t* lpp)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	const stTCHDR_LOTPROC_PARAM_t* llpp = lpp;
	U32 s = llpp->ServIdx;
	U32 End = 0U;
	U32 bytes_written = 0U;
	U32 Added_written_size = 0U;
	stTC_HDR_LOT_t* stTcHdrXXXLot;

	if(llpp->id == eTC_HDR_ID_BS) {
		stTcHdrXXXLot = &stTcHdrBsLot[0];
	}
	else {
		stTcHdrXXXLot = &stTcHdrMainLot[0];
	}

	ret = HDR_aas_get_lot_object_body(llpp->hdrInstance, llpp->port_number, llpp->lotId, stTcHdrXXXLot[s].body, stTcHdrXXXLot[s].header.file_size, &bytes_written);
	if(ret < 0) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Get Object Body failed .. (port number: 0x%X, LOT ID: %d, ret %d), PN(%d)\n",
			llpp->port_number, llpp->lotId, ret, llpp->Program_number);
	}
	else if(ret == 0) { /*0: Not end-of-file */
		Added_written_size = bytes_written;

		while(End == 0U) {
			// (*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "LOT Object Body .. (port number: 0x%X, LOT ID: %d) s(%d), PN(%d)\n",llpp->port_number, llpp->lotId, llpp->ServNum, llpp->Program_number);

			ret = HDR_aas_get_lot_object_body(llpp->hdrInstance, llpp->port_number, llpp->lotId, stTcHdrXXXLot[s].body, stTcHdrXXXLot[s].header.file_size, &bytes_written);

			// (*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "LOT Object Body .. FSize(%d), Written(%d)\n",stTcHdrXXXLot[s].header.file_size, bytes_written);

			/*1: End-of-file*/
			if(ret == 1) {
				U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
				void *pData[TCHDR_MSGQ_PDATA_LENGTH] = {0,};

				stTcHdrXXXLot[s].body_bytes_written = Added_written_size;

				uiSendMsg[0] = (U32)llpp->id;
				uiSendMsg[1] = llpp->port_number;
				uiSendMsg[2] = llpp->lotId;
				uiSendMsg[3] = Added_written_size;
				uiSendMsg[4] = stTcHdrXXXLot[s].service_number;
				uiSendMsg[5] = 300; /* LOT Timeout is fixed at 300 seconds. (Refer: hdrAas.h) */
				uiSendMsg[6] = llpp->Program_number;
				pData[0] = &stTcHdrXXXLot[s];

                // (*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "LOT Object Send Event .. (port number: 0x%X, LOT ID: %d) s(%d), PN(%d), BodySize(%d)\n",
				//     llpp->port_number, llpp->lotId, llpp->ServNum, llpp->Program_number,Added_written_size);

				(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_LOT, uiSendMsg, pData, 0);
				End = 1;
			}
			else if(ret < 0) {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "LOT Object Body .. Return is Wrong.. ret(%d)\n",ret);
				break;
			}
			else if(ret == 0/*0: Not end-of-file */) {
				Added_written_size = Added_written_size + bytes_written;
			}
			else {
				/* Nothing To Do */
			}

			if(Added_written_size > stTcHdrXXXLot[s].header.file_size) {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "LOT Object Body .. Added written size exceeds Header size.(Hader: %d / Add: %d)\n",
					stTcHdrXXXLot[s].header.file_size,Added_written_size);
				End = 1;
			}
		}
	}
	else if(ret == 1) {
		// (*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "LOT Object Body .. FSize(%d), Written(%d), ret(%d) - Call and Finsh\n", stTcHdrXXXLot[s].header.file_size, bytes_written,ret);
	}
	else {
		/* Nothing To Do */
	}

	return ret;
}

static HDRET tchdrsvc_buildLotHeader(const stTCHDR_LOTPROC_PARAM_t* lpp)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	const stTCHDR_LOTPROC_PARAM_t* llpp = lpp;

	tchdrsvc_tchdrsvc_buildLotFree(llpp);

	// (*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "LOT Object Complete .. (port number: 0x%X, LOT ID: %d) s(%d), PN(%d)\n",
	//     llpp->port_number, llpp->lotId, llpp->ServNum, llpp->Program_number);

	HDR_aas_lot_object_header_t Lot_H;
	(void)(*stOsal.osmemset)((void*)&Lot_H, (S8)0, (U32)sizeof(HDR_aas_lot_object_header_t));

	ret = HDR_aas_get_lot_object_header(llpp->hdrInstance, llpp->port_number, llpp->lotId, &Lot_H);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		// (*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "LOT Object Head Complete .. (port number: 0x%X, LOT ID: %d) s(%d), PN(%d)\n",
		//     llpp->port_number, llpp->lotId, llpp->ServNum, llpp->Program_number);

		if(llpp->id == eTC_HDR_ID_BS) {
			(void)(*stOsal.osmemcpy)((void*)&stTcHdrBsLot[llpp->ServIdx].header, (void*)&Lot_H, (U32)sizeof(stTC_HDR_AAS_LOT_OBJECT_HEADER_t));
		}
		else {
			(void)(*stOsal.osmemcpy)((void*)&stTcHdrMainLot[llpp->ServIdx].header, (void*)&Lot_H, (U32)sizeof(stTC_HDR_AAS_LOT_OBJECT_HEADER_t));
		}

		tchdrsvc_tchdrsvc_buildLotAlloc(llpp);

		ret = tchdrsvc_buildLotBody(llpp);
	}

	return ret;
}

static HDRET tchdrsvc_buildLotMain(stTCHDR_LOTPROC_PARAM_t* lpp)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	stTCHDR_LOTPROC_PARAM_t* llpp = lpp;

	ret = tchdrsvc_checkLotComplete(llpp);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdrsvc_buildLotHeader(llpp);
	}

	return ret;
}

static U32 tchdrsvc_findLotServiceComponent(const stTCHDR_LOTPROC_PARAM_t* lpp, const HDR_sig_service_info_t* Srvinfo)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	U32 Program_number=0;
	const stTCHDR_LOTPROC_PARAM_t* llpp = lpp;
	const HDR_sig_service_info_t *serviceInfo = Srvinfo;
	HDR_sig_service_component_t serviceComponent;

	for(U32 ComIdx = 0; ComIdx < serviceInfo->num_components; ComIdx++) {
		(void)(*stOsal.osmemset)((void*)&serviceComponent, (S8)0, (U32)sizeof(HDR_sig_service_component_t));

		ret = HDR_sig_get_service_component(llpp->hdrInstance, llpp->ServNum, ComIdx, &serviceComponent);
		if(ret < 0) {
			continue;
		}

		/* Get Program Number */
		if(serviceComponent.component_type == HDR_SIG_AUDIO_COMPONENT) {
			Program_number = serviceComponent.channel;
		}
	}

	return Program_number;
}

static void tchdrsvc_findLotServiceinfo(stTCHDR_LOTPROC_PARAM_t* lpp, const HDR_sig_service_list_t* Srvlist)
{
	const HDR_sig_service_list_t *serviceList = Srvlist;
	HDR_sig_service_info_t serviceInfo;
	stTCHDR_LOTPROC_PARAM_t* llpp = lpp;

	for(U32 ServIdx = 0U;ServIdx < serviceList->num_services; ++ServIdx) {

		llpp->ServNum = serviceList->item[ServIdx].service_number;
		llpp->ServIdx = ServIdx;

		if(HDR_sig_get_service_info(llpp->hdrInstance, llpp->ServNum, &serviceInfo) < 0) {
			break;
		}
		else {
			if(serviceInfo.num_components > 0U) {
				llpp->Program_number = tchdrsvc_findLotServiceComponent(llpp,&serviceInfo);
			}

			if(llpp->Program_number < HDR_MAX_NUM_PROGRAMS) {
				if (llpp->id == eTC_HDR_ID_BS) {
					if((stTcHdrService.bsHdr.lot.progBitmask & ((U8)0x01 << llpp->Program_number)) == 0U) {
						continue;
					}
				}
				else {
					if((stTcHdrService.mainHdr.lot.progBitmask & ((U8)0x01 << llpp->Program_number)) == 0U) {
						continue;
					}
				}
			}

			(void)tchdrsvc_buildLotMain(llpp);
		}
	}
}

static void tchdrsvc_init_LotProParamStr(stTCHDR_LOTPROC_PARAM_t* lpp)
{
	lpp->hdrInstance = pNULL;
	lpp->id=eTC_HDR_ID_MAIN;
	lpp->ServIdx=0U;
	lpp->ServNum=0U;
	lpp->Program_number=0U;
	lpp->lotId=0U;
	lpp->port_number=0U;
}

static void tchdrsvc_doLotProcess(eTC_HDR_ID_t id)
{
	HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
	HDR_sig_service_list_t serviceList;

	if(hdrInstance != pNULL) {
		stTCHDR_LOTPROC_PARAM_t lpp;

		tchdrsvc_init_LotProParamStr(&lpp);
		lpp.hdrInstance = hdrInstance;
		lpp.id = id;

		for(U8 serviceType = 0U; serviceType < (U8)HDR_SIG_NUM_SERVICE_TYPES; ++serviceType) {
			if(HDR_sig_get_service_list(lpp.hdrInstance, (HDR_sig_service_type_t)serviceType, &serviceList) < 0) {
				break;
			}
			else {
				/* Nothing To Do */
			}

			tchdrsvc_findLotServiceinfo(&lpp, &serviceList);
		}
	}
}

static void tchdrsvc_doEAProcess(eTC_HDR_ID_t id)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
	HDR_alerts_msg_status_t status;

	if(hdrInstance != pNULL) {
		(void)(*stOsal.osmemset)((void*)&status, (S8)0, (U32)sizeof(HDR_alerts_msg_status_t));
		ret = HDR_alert_get_message_status(hdrInstance, &status);
		if((status.frame_received == true) && (status.full_message == true)) {
			HDR_alert_message_t stEaMsg = {0,};
			ret = HDR_alert_get_message(hdrInstance, &stEaMsg);
			if(ret == (HDRET)eTC_HDR_RET_OK) {
				U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
				void *pData[TCHDR_MSGQ_PDATA_LENGTH] = {0,};
				U32 msgOffset;

				if(id == eTC_HDR_ID_BS) {
					(void)(*stOsal.osmemcpy)((void*)&stTcHdrBsAlert, (void*)&stEaMsg, (U32)sizeof(stTC_HDR_ALERT_MESSAGE_t));
					if(stEaMsg.text_message != NULL) {
						msgOffset = (*stArith.u32add)(stTcHdrBsAlert.cnt_length, 1U);
						stTcHdrBsAlert.text_message = &stTcHdrBsAlert.payload[msgOffset];
					}
					else {
						stTcHdrBsAlert.text_message = NULL;
					}
					uiSendMsg[0] = (U32)eTC_HDR_ID_BS;
					pData[0] = (void*)(&stTcHdrBsAlert);
				}
				else {
					(void)(*stOsal.osmemcpy)((void*)&stTcHdrMainAlert, (void*)&stEaMsg, (U32)sizeof(stTC_HDR_ALERT_MESSAGE_t));
					if(stEaMsg.text_message != NULL) {
						msgOffset = (*stArith.u32add)(stTcHdrMainAlert.cnt_length, 1U);
						stTcHdrMainAlert.text_message = &stTcHdrMainAlert.payload[msgOffset];
					}
					else {
						stTcHdrMainAlert.text_message = NULL;
					}
					uiSendMsg[0] = (U32)eTC_HDR_ID_MAIN;
					pData[0] = (void*)(&stTcHdrMainAlert);

					/* Alert Tone */
					stHDR_FRAMEWORK_DATA_t* frameworkData;
					frameworkData = tchdrfwk_getDataStructPtr();
					if(frameworkData != pNULL) {
						frameworkData->playAlertTone = true;
					}
				}
				(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_ALERT, uiSendMsg, pData, 0);
			}
		}
	}
}

static void tchdrsvc_gatherHdrDataHandler(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDR_instance_t *hdrInstance;
	HDR_program_bitmap_t changedProgramBitMask;
	U8 userFieldBitmask;
	HDBOOL bSisAcquired;
	HDBOOL bSisCrcOk;
	U32 i;

	// PSD
	if((stTcHdrService.mainHdr.psd.progBitmask > 0U) || (stTcHdrService.bsHdr.psd.progBitmask > 0U)) {
        // (*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "tchdrsvc_gatherHdrDataHandler tchdrsvc_isHDRadioData(0) : %d ", tchdrsvc_isHDRadioData(0));
		if((tchdrsvc_isHDRadioData(0) == 1) && (stTcHdrService.mainHdr.psd.progBitmask > 0U)) {
			if(stTcHdrService.mainHdr.psd.checkInterval == 0U) {
				hdrInstance = tchdrfwk_getHdrInstancePtr(eTC_HDR_ID_MAIN);
				if(stTcHdrService.mainHdr.psd.fNotify > 0U) {
					HDR_psd_fields_t tmpFieldBitmask;
					tmpFieldBitmask.all = stTcHdrService.mainHdr.psd.fieldBitmask;
					ret = HDR_psd_enable_fields(hdrInstance, tmpFieldBitmask);
					if(ret == (HDRET)eTC_HDR_RET_OK) {
						changedProgramBitMask = HDR_psd_get_changed_programs(hdrInstance);
						changedProgramBitMask.all = stTcHdrService.mainHdr.psd.progBitmask;
						stTcHdrService.mainHdr.psd.fNotify = 0;
					}
					else {
						changedProgramBitMask.all = 0;
					}
				}
				else {
					changedProgramBitMask = HDR_psd_get_changed_programs(hdrInstance);
					changedProgramBitMask.all &= stTcHdrService.mainHdr.psd.progBitmask;
				}
				userFieldBitmask = stTcHdrService.mainHdr.psd.fieldBitmask;
				if(changedProgramBitMask.all > 0U) {
					U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
					void *pData[TCHDR_MSGQ_PDATA_LENGTH];
					(void)(*stOsal.osmemset)((void*)pData, (S8)0, (U32)TCHDR_MSGQ_PDATA_LENGTH*(U32)sizeof(void*));
					tchdrsvc_getPsd(hdrInstance, userFieldBitmask, changedProgramBitMask.all, &stTcHdrMainPsd[eTC_HDR_PROGRAM_HD1]);
					for(i=0; i<HDR_MAX_NUM_PROGRAMS; i++) {
						if((changedProgramBitMask.all & ((U8)1 << i)) == 0U) {
							continue;
						}
						uiSendMsg[0] = (U32)eTC_HDR_ID_MAIN;
						uiSendMsg[1] = i;
						pData[0] = (void*)(&stTcHdrMainPsd[i]);
						(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_PSD, uiSendMsg, (void*)pData, 0);
						stTcHdrService.mainHdr.psd.checkInterval = 10;		// 10 x TCHDR_SERVICE_THREAD_TIME_INTERVAL(10ms) = 200ms
					}
				}
			}
		}

		if((tchdrsvc_isHDRadioData(1) == 1) && (stTcHdrService.bsHdr.psd.progBitmask > 0U)) {
			if(stTcHdrService.bsHdr.psd.checkInterval == 0U) {
				hdrInstance = tchdrfwk_getHdrInstancePtr(eTC_HDR_ID_BS);
				if(hdrInstance != NULL) {
					if(stTcHdrService.bsHdr.psd.fNotify > 0U) {
						HDR_psd_fields_t tmpFieldBitmask;
						tmpFieldBitmask.all = stTcHdrService.bsHdr.psd.fieldBitmask;
						ret = HDR_psd_enable_fields(hdrInstance, tmpFieldBitmask);
						if(ret == (HDRET)eTC_HDR_RET_OK) {
							changedProgramBitMask = HDR_psd_get_changed_programs(hdrInstance);
							changedProgramBitMask.all = stTcHdrService.bsHdr.psd.progBitmask;
							stTcHdrService.bsHdr.psd.fNotify = 0;
						}
						else {
							changedProgramBitMask.all = 0;
						}
					}
					else {
						changedProgramBitMask = HDR_psd_get_changed_programs(hdrInstance);
						changedProgramBitMask.all &= stTcHdrService.bsHdr.psd.progBitmask;
					}
					userFieldBitmask = stTcHdrService.bsHdr.psd.fieldBitmask;
					if(changedProgramBitMask.all > 0U) {
						U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
						void *pData[TCHDR_MSGQ_PDATA_LENGTH];
						(void)(*stOsal.osmemset)((void*)pData, (S8)0, (U32)TCHDR_MSGQ_PDATA_LENGTH*(U32)sizeof(void*));
						tchdrsvc_getPsd(hdrInstance, userFieldBitmask, changedProgramBitMask.all, &stTcHdrBsPsd[eTC_HDR_PROGRAM_HD1]);
						for(i=0; i<HDR_MAX_NUM_PROGRAMS; i++) {
							if((changedProgramBitMask.all & ((U8)1 << i)) == 0U) {
								continue;
							}
							uiSendMsg[0] = (U32)eTC_HDR_ID_BS;
							uiSendMsg[1] = i;
							pData[0] = (void*)(&stTcHdrBsPsd[i]);
							(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_PSD, uiSendMsg, (void*)pData, 0);
							stTcHdrService.bsHdr.psd.checkInterval = 10;		// 10 x TCHDR_SERVICE_THREAD_TIME_INTERVAL(10ms) = 200ms
						}
					}
				}
			}
		}
		if(stTcHdrService.mainHdr.psd.checkInterval > 0U) {
			stTcHdrService.mainHdr.psd.checkInterval--;
		}
		if(stTcHdrService.bsHdr.psd.checkInterval > 0U) {
			stTcHdrService.bsHdr.psd.checkInterval--;
		}
	}

	// SIS
	if((stTcHdrService.mainHdr.sis.fieldBitmask > 0U) || (stTcHdrService.bsHdr.sis.fieldBitmask > 0U)) {
		S32 fHdrData = tchdrsvc_isHDRadioData(0U);
		if((stTcHdrService.mainHdr.sis.fieldBitmask > 0U) && (fHdrData == 1)) {
			if(stTcHdrService.mainHdr.sis.checkInterval == 0U) {
				hdrInstance = tchdrfwk_getHdrInstancePtr(eTC_HDR_ID_MAIN);
				if(stTcHdrService.mainHdr.sis.fNotify > 0U) {
					HDR_sis_enabled_basic_types_t enabledTypes;
					enabledTypes.all = stTcHdrService.mainHdr.sis.enDefaultType;
					ret = HDR_sis_enable_basic_types(hdrInstance, enabledTypes);
					if(ret == (HDRET)eTC_HDR_RET_OK) {
						stTcHdrService.mainHdr.sis.fNotify = 0;
					}
				}

				bSisAcquired = HDR_sis_acquired(hdrInstance);
				bSisCrcOk = HDR_sis_crc_ok(hdrInstance);
				if((stTcHdrService.mainHdr.sis.fNotify == 0U) && (bSisAcquired == true) && (bSisCrcOk == true)) {
					U32 status=0;
					(void)(*stOsal.osmemset)((void*)&stTcHdrMainSis, (S8)0, (U32)sizeof(stTC_HDR_SIS_t));
					ret = tchdrsvc_getSisBasicData(eTC_HDR_ID_MAIN, &stTcHdrMainSis, &status);
					if((ret >=(HDRET)eTC_HDR_RET_OK) && (status > 0U)) {
						U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
						void *pData[TCHDR_MSGQ_PDATA_LENGTH];
						(void)(*stOsal.osmemset)((void*)pData, (S8)0, (U32)TCHDR_MSGQ_PDATA_LENGTH*(U32)sizeof(void*));
						uiSendMsg[0] = (U32)eTC_HDR_ID_MAIN;
						pData[0] = (void*)(&stTcHdrMainSis);
						(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_SIS, uiSendMsg, (void*)pData, 0);
						stTcHdrService.mainHdr.sis.checkInterval = 100;		// 1sec
					}
				}
			}
		}
		fHdrData = tchdrsvc_isHDRadioData(1U);
		if((stTcHdrService.bsHdr.sis.fieldBitmask > 0U) && (fHdrData == 1)) {
			if(stTcHdrService.bsHdr.sis.checkInterval == 0U) {
				hdrInstance = tchdrfwk_getHdrInstancePtr(eTC_HDR_ID_MAIN);
				if(stTcHdrService.bsHdr.sis.fNotify > 0U) {
					HDR_sis_enabled_basic_types_t enabledTypes;
					enabledTypes.all = stTcHdrService.bsHdr.sis.enDefaultType;
					ret = HDR_sis_enable_basic_types(hdrInstance, enabledTypes);
					if(ret == (HDRET)eTC_HDR_RET_OK) {
						stTcHdrService.bsHdr.sis.fNotify = 0;
					}
				}

				bSisAcquired = HDR_sis_acquired(hdrInstance);
				bSisCrcOk = HDR_sis_crc_ok(hdrInstance);
				if((stTcHdrService.bsHdr.sis.fNotify == 0U) && (bSisAcquired == true) && (bSisCrcOk == true)) {
					U32 status=0;
					(void)(*stOsal.osmemset)((void*)&stTcHdrBsSis, (S8)0, (U32)sizeof(stTC_HDR_SIS_t));
					ret = tchdrsvc_getSisBasicData(eTC_HDR_ID_BS, &stTcHdrBsSis, &status);
					if((ret >=(HDRET)eTC_HDR_RET_OK) && (status > 0U)) {
						U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
						void *pData[TCHDR_MSGQ_PDATA_LENGTH];
						(void)(*stOsal.osmemset)((void*)pData, (S8)0, (U32)TCHDR_MSGQ_PDATA_LENGTH*(U32)sizeof(void*));
						uiSendMsg[0] = (U32)eTC_HDR_ID_BS;
						pData[0] = (void*)(&stTcHdrBsSis);
						(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_SVC_NOTIFY_SIS, uiSendMsg, (void*)pData, 0);
						stTcHdrService.bsHdr.sis.checkInterval = 100;		// 1sec
					}
				}
			}
		}
		if(stTcHdrService.mainHdr.sis.checkInterval > 0U) {
			stTcHdrService.mainHdr.sis.checkInterval--;
		}
		if(stTcHdrService.bsHdr.sis.checkInterval > 0U) {
			stTcHdrService.bsHdr.sis.checkInterval--;
		}
	}


	/////////////////////////////////////////////////////////////////////////////////
	// LOT (Large Object Transfer)
	/////////////////////////////////////////////////////////////////////////////////
	if(stTcHdrService.mainHdr.lot.checkInterval == 0U) {
		if(tchdrsvc_isHDRadioData(0) == 1) {
			if(stTcHdrService.mainHdr.lot.fNotify == (U8)1) {
				tchdrsvc_doLotProcess(eTC_HDR_ID_MAIN);
				stTcHdrService.mainHdr.lot.checkInterval = 50;
			}
		}
	}

	if(stTcHdrService.bsHdr.lot.checkInterval == 0U) {
		if(tchdrsvc_isHDRadioData(1) == 1) {
			if(stTcHdrService.bsHdr.lot.fNotify == (U8)1) {
				tchdrsvc_doLotProcess(eTC_HDR_ID_BS);
				stTcHdrService.bsHdr.lot.checkInterval = 50;
			}
		}
	}


	if(stTcHdrService.mainHdr.lot.checkInterval > 0U) {
		stTcHdrService.mainHdr.lot.checkInterval--;
	}

	if(stTcHdrService.bsHdr.lot.checkInterval > 0U) {
		stTcHdrService.bsHdr.lot.checkInterval--;
	}

	/////////////////////////////////////////////////////////////////////////////////
	// EA (Emergency Alert)
	/////////////////////////////////////////////////////////////////////////////////
	if(stTcHdrService.mainHdr.alert.checkInterval == 0U) {
		if(tchdrsvc_isHDRadioData(0) == 1) {
			if(stTcHdrService.mainHdr.alert.fNotify == 1U) {
				tchdrsvc_doEAProcess(eTC_HDR_ID_MAIN);
			}
			stTcHdrService.mainHdr.alert.checkInterval = 50;
		}
	}

	if(stTcHdrService.bsHdr.alert.checkInterval == 0U) {
		if(tchdrsvc_isHDRadioData(1) == 1) {
			if(stTcHdrService.bsHdr.alert.fNotify == 1U) {
				tchdrsvc_doEAProcess(eTC_HDR_ID_BS);
			}
			stTcHdrService.bsHdr.alert.checkInterval = 50;
		}
	}

	if(stTcHdrService.mainHdr.alert.checkInterval > 0U) {
		stTcHdrService.mainHdr.alert.checkInterval--;
	}

	if(stTcHdrService.bsHdr.alert.checkInterval > 0U) {
		stTcHdrService.bsHdr.alert.checkInterval--;
	}
	/////////////////////////////////////////////////////////////////////////////////
}

static void tchdrsvc_notifyHandler(void)
{
	U32 i;
	stTcHdrMsgBuf_t stRecivedMessage;

	(void)(*stOsal.osmemset)((void*)&stRecivedMessage, (S8)0, (U32)sizeof(stTcHdrMsgBuf_t));

	for(i=0; i<HDR_MAX_NUM_PROGRAMS; i++) {	// Notify only the number of programs
		(void)tchdrapp_getMessage(&stRecivedMessage);
		if(stRecivedMessage.fNewMsg == (U32)eTCHDR_NEW_MSG_EXIST) {
			if(stRecivedMessage.uiSender == (U32)eTCHDR_SENDER_ID_SERVICE) {
				tchdrsvc_callbackAppFunction(&stRecivedMessage);
			}
		}
		else {
			break;
		}
	}
}

static void tchdrsvc_callbackAppFunction(stTcHdrMsgBuf_t *pstMsg)
{
	if(pstMsg->uiMode != 0U) {
		if(pfnTcHdrNotificationCallBack != NULL) {
			(*pfnTcHdrNotificationCallBack)(pstMsg->uiMode, pstMsg->uiData, pstMsg->pData, pstMsg->iError);
		}
		else {
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Callback error : Not registered the notification callback function of the TC HD Radio!!!\n");
		}
	}
	else {
		(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Callback error : A message mode is null!!!\n");
	}
}

HDRET tchdrsvc_setProgramNumber(const HDR_instance_t *hdrInstance, HDR_program_t program)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(hdrInstance != NULL) {
		if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
			stTcHdrService.mainHdrProgNum = program;
		}
		else if(hdrInstance->instance_type == HDR_DATA_ONLY_INSTANCE) {
			stTcHdrService.bsHdrProgNum = program;
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_INSTANCE;
	}

	return ret;
}

HDRET tchdrsvc_getProgramNumber(const HDR_instance_t *hdrInstance, HDR_program_t *program)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(hdrInstance != NULL) {
		if(program != NULL) {
			if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
				*program = stTcHdrService.mainHdrProgNum;
			}
			else if(hdrInstance->instance_type == HDR_DATA_ONLY_INSTANCE) {
				*program = stTcHdrService.bsHdrProgNum;
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_INSTANCE;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/*								Telechips HD Radio Service API Functions							*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
HDRET tchdrsvc_init(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDRET rc;

	rc = tchdrsvc_messageInit();
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_INIT_MESSAGE_QUEUE;
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to initialize HDR service message.\n");
	}

	rc = tchdrapp_messageInit();
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_INIT_MESSAGE_QUEUE;
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to initialize HDR app message.\n");
	}

	(void)(*stOsal.osmemset)((void*)&stTcHdrService, (S8)0, (U32)sizeof(stTCHDR_SVC_t));

	stTcHdrService.statusChkInterval = 25;				// thread interval time 10ms x 25 = 250ms
	stTcHdrService.mainHdr.psd.checkInterval = 10;		// thread interval time 10ms x 10 = 100ms
	stTcHdrService.bsHdr.psd.checkInterval = 10;		// thread interval time 10ms x 10 = 100ms
	stTcHdrService.mainHdr.sis.checkInterval = 100;		// thread interval time 10ms x 100 = 1000ms
	stTcHdrService.bsHdr.sis.checkInterval = 100;		// thread interval time 10ms x 100 = 1000ms

	return ret;
}

HDRET tchdrsvc_deinit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDRET rc;

	rc = tchdrsvc_messageDeinit();
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to deinit manager message.\n");
	}
	rc = tchdrapp_messageDeinit();
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to deinit app message.\n");
	}

	return ret;
}

HDRET tchdrsvc_open(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	if(stTcHdrService.fOpen == 0U) {
		tchdrsvc_setOpenStatus(1);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_ALREADY_OPEN;
	}
	return ret;
}

HDRET tchdrsvc_close(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	if(stTcHdrService.fOpen > 0U) {
		tchdrsvc_setOpenStatus(0);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}
	return ret;
}

HDRET tchdrsvc_getHdrSignalStatus(eTC_HDR_ID_t id, stTC_HDR_SIGNAL_STATUS_t *dataOut)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);

	if(dataOut != NULL) {
		if(hdrInstance != pNULL) {
			U32 acqStatus=0;
			HDBOOL hybridProg = false;
			HDBOOL digAcq = false;
			HDR_program_t program = HDR_PROGRAM_HD1;
			HDR_program_bitmap_t availablePrograms;

			(void)(*stOsal.osmemset)((void*)dataOut, (S8)0, (U32)sizeof(stTC_HDR_SIGNAL_STATUS_t));
			availablePrograms.all = 0;
			(void)HDR_get_available_programs(hdrInstance, &availablePrograms);

			if(id == eTC_HDR_ID_MAIN) {
			    (void)HDR_get_playing_program(hdrInstance, &program);
			#if 1
				digAcq = tchdrfwk_getDigitalAudioAcquired(hdrInstance);
			#else
				digAcq = HDR_digital_audio_acquired(hdrInstance);
			#endif
				(void)HDR_hybrid_program(hdrInstance, &hybridProg);		// Hybrid Program
				dataOut->hybridProgram = (*stCast.booltou32)(hybridProg);
			}

			acqStatus = (*stCast.booltou32)(HDR_hd_signal_acquired(hdrInstance));
			acqStatus |= ((*stCast.booltou32)(HDR_sis_acquired(hdrInstance))) << 1U;
			acqStatus |= ((*stCast.booltou32)(HDR_sis_crc_ok(hdrInstance))) << 2U;
			acqStatus |= ((*stCast.booltou32)(digAcq)) << 3U;

			dataOut->hdrID = id;										// Selected HDR ID
			dataOut->curPN = (U32)program;								// Current program number
			dataOut->pmap = (U32)availablePrograms.all;					// Available program bitmap
			dataOut->acqStatus = acqStatus;								// Acquired Status, [3:0] digital audio:sis crc ok:sis:hd signal
			(void)HDR_get_cdno(hdrInstance, &dataOut->cnr);				// CDNO
            dataOut->ballgameMode = tchdrfwk_getBallGameMode(hdrInstance);

		//	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "00) HDR ID: %d\n", dataOut->hdrId);
		//	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "01) Playing Program Number: %d\n", dataOut->curPN);
		//	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "02) Available Program Bitmap: %d\n", dataOut->pmap);
		//	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "03) Acquired Status: 0x%02x, digital_audio_acquired[%d]:sis_crc_ok[%d]:sis_acquired[%d]:hd_signal_acquired[%d]\n",
		//			acqStatus, acqStatus&0x08 ? 1:0, acqStatus&0x04 ? 1:0, acqStatus&0x02 ? 1:0, acqStatus&0x01 ? 1:0);
		//	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "04) CNR: %d\n", dataOut->cnr);
		//	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "05) Hybrid Program Status: %d\n", dataOut->hybridProgram);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	return ret;
}

HDRET tchdrsvc_getHdrStatus(eTC_HDR_ID_t id, stTC_HDR_STATUS_t *dataOut)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);

	if(dataOut != NULL) {
		if(hdrInstance != NULL) {
			U32 acqStatus=0;
			HDBOOL hybridProg = false;
			HDBOOL digAcq = false;
			HDR_audio_quality_report_t audioQualityReport;
			HDR_program_bitmap_t availablePrograms;
			HDR_program_t program = HDR_PROGRAM_HD1;
			HDR_audio_codec_mode_t codecMode = HDR_AUDIO_CODEC_MODE0;
			HDR_program_types_t programTypes;

			(void)(*stOsal.osmemset)((void*)dataOut, (S8)0, (U32)sizeof(stTC_HDR_STATUS_t));
			(void)(*stOsal.osmemset)((void*)&audioQualityReport, (S8)0, (U32)sizeof(HDR_audio_quality_report_t));
			availablePrograms.all = 0;
			(void)HDR_get_available_programs(hdrInstance, &availablePrograms);

			if(id == eTC_HDR_ID_MAIN) {
				(void)HDR_get_playing_program(hdrInstance, &program);
			#if 1
				digAcq = tchdrfwk_getDigitalAudioAcquired(hdrInstance);
			#else
				digAcq = HDR_digital_audio_acquired(hdrInstance);
			#endif

				(void)HDR_get_audio_quality_report(hdrInstance, &audioQualityReport);
				dataOut->audioQualityIndicator = audioQualityReport.quality_indicator;			// Digital audio quality indicator
                dataOut->frameCount = audioQualityReport.frame_count;
                dataOut->coreErrors = audioQualityReport.core_errors;            
                dataOut->enhErrors = audioQualityReport.enh_errors;

				(void)HDR_get_tx_dig_audio_gain(hdrInstance, &dataOut->digitalAudioGain);		// TX Digital audio gain
				(void)HDR_get_playing_program_type(hdrInstance, &dataOut->curPty);				// Current Playing Program Type
				(void)HDR_test_get_raw_tx_blend_control(hdrInstance, &dataOut->blendControl);	// TX Blend control
				(void)HDR_hybrid_program(hdrInstance, &hybridProg);								// Hybrid Program
				dataOut->hybridProgram = (*stCast.booltou32)(hybridProg);
			}

			(void)HDR_get_codec_mode(hdrInstance, &codecMode);
			acqStatus = (*stCast.booltou32)(HDR_hd_signal_acquired(hdrInstance));
			acqStatus |= ((*stCast.booltou32)(HDR_sis_acquired(hdrInstance))) << 1U;
			acqStatus |= ((*stCast.booltou32)(HDR_sis_crc_ok(hdrInstance))) << 2U;
			acqStatus |= ((*stCast.booltou32)(digAcq)) << 3U;

			dataOut->hdrID = id;													// Selected HDR Instance Number
			dataOut->curPN = (U32)program;											// Current Playing Program Number
			dataOut->acqStatus = acqStatus;											// Acquired Status, [3:0] digital audio:sis crc ok:sis:hd signal

			(void)HDR_get_cdno(hdrInstance, &dataOut->cnr);							// CD/NO
            dataOut->ballgameMode = tchdrfwk_getBallGameMode(hdrInstance);

		    (void)(*stOsal.osmemset)((void*)&programTypes, (S8)0, (U32)sizeof(HDR_program_types_t));
			(void)HDR_get_program_types(hdrInstance, &programTypes);
		    (void)(*stOsal.osmemcpy)((void*)dataOut->pty, (void*)programTypes.value, (U32)sizeof(programTypes.value));

			dataOut->pmap = availablePrograms.all;									// Available Program bitmap
			dataOut->chgPmap = (U32)HDR_psd_get_changed_programs(hdrInstance).all;	// Changed PSD Program bitmap
			dataOut->psm = (U32)HDR_get_primary_service_mode(hdrInstance);			// Primary Service Mode(PSM)
			dataOut->codecMode = (U32)codecMode;									// Codec Mode
			(void)HDR_get_filt_dsqm(hdrInstance, &dataOut->dsqm);					// Filtered Digital Signal Quality Measurement(DSQM) value
			(void)HDR_get_raw_snr(hdrInstance, &dataOut->rawSnr);					// Raw-SNR

		#if 0
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "00) HDR Instance Number: %d\n", dataOut->hdrID);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "01) Playing Program Number: %d\n", dataOut->curPN);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "02) Acquired Status: 0x%02x, digital_audio_acquired[%d]:sis_crc_ok[%d]:sis_acquired[%d]:hd_signal_acquired[%d]\n",
					dataOut->acqStatus, dataOut->acqStatus&0x08 ? 1:0, dataOut->acqStatus&0x04 ? 1:0, dataOut->acqStatus&0x02 ? 1:0, dataOut->acqStatus&0x01 ? 1:0);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "03) Digital Audio Quality Indicator: %d\n", dataOut->audioQualityIndicator);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "04) CNR: %d\n", dataOut->cnr);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "05) TX Digital Audio Gain: %d\n", dataOut->digitalAudioGain);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "06) TX Blend Control: %d\n", dataOut->blendControl);
			for(i=0; i<HDR_MAX_NUM_PROGRAMS; i++) {
				if(i==0) (*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "07) Program Types: ");
				(*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "[PTY%d: %d] ", i, dataOut->pty[i]);
				if(i==7) (*pfnHdrLog)(eTAG_NOTAG, eLOG_DBG, "\n");
			}
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "08) Playing Program Type: %d\n", dataOut->curPty);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "09) Available Program Bitmap: 0x%02x\n", dataOut->pmap);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "10) Changed PSD Program Bitmap: 0x%02x\n", dataOut->chgPmap);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "11) Primary Service Mode(PSM): %d\n", dataOut->psm);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "12) Codec Mode: %d\n", dataOut->codecMode);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "13) Filtered Digital Siganal Quality Measurement(DSQM): %d\n", dataOut->dsqm);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "14) Hybrid Program Status: %d\n", dataOut->hybridProgram);
			(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "15) Raw SNR: %d\n", dataOut->rawSnr);
		#endif
		#if 1 // for test : refer to procGetStatus() function.
			HDR_test_ber_t ber;
		    (void)(*stOsal.osmemset)((void*)&ber, (S8)0, (U32)sizeof(HDR_test_ber_t));

			if(HDR_test_ber_mode_enabled(hdrInstance) == true) {
				HDR_test_get_ber(hdrInstance, &ber);
                dataOut->pidsBlockErrors = ber.pids_block_errors;  /**< Total number of PIDS blocks(80 bits) that had at least one error */
                dataOut->pidsBlocksTested = ber.pids_blocks_tested; /**< Total number of PIDS blocks(80 bits) tested */
                dataOut->pidsBitErrors = ber.pids_bit_errors;    /**< Total number of PIDS bit errors */
                dataOut->pidsBitsTested = ber.pids_bits_tested;   /**< Total number of PIDS bits tested */
                dataOut->p1BitErrors = ber.p1_bit_errors;      /**< Total number of P1 bit errors */
                dataOut->p1BitsTested = ber.p1_bits_tested;     /**< Total number of P1 bit bits tested */
                dataOut->p2BitErrors = ber.p2_bit_errors;      /**< Total number of P2 bit errors */
                dataOut->p2BitsTested = ber.p2_bits_tested;     /**< Total number of P2 bit bits tested */
                dataOut->p3BitErrors = ber.p3_bit_errors;      /**< Total number of P3 bit errors */
                dataOut->p3BitsTested = ber.p3_bits_tested;     /**< Total number of P3 bit bits tested */
                dataOut->p4BitErrors = ber.p4_bit_errors;      /**< Total number of P4 bit errors */
                dataOut->p4BitsTested = ber.p4_bits_tested;     /**< Total number of P4 bit bits tested */
			}
		#endif
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	return ret;
}

#if 0 // Incomplete function.
static void tchdrsvc_getMrcStatus(HDR_instance_t* hdrInstance, U32 *uiData)
{
}
#endif

HDRET tchdrsvc_enableGetPsd(eTC_HDR_ID_t id, U8 progBitmask, U8 psdBitmask, U32 fEn)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDR_psd_fields_t enabled_fields;

	if((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) {
		enabled_fields.all = psdBitmask;
		if(id == eTC_HDR_ID_MAIN) {
			if(fEn > 0U) {
				stTcHdrService.mainHdr.psd.progBitmask = progBitmask;
				stTcHdrService.mainHdr.psd.fieldBitmask = psdBitmask;
				stTcHdrService.mainHdr.psd.fNotify = 1;
				stTcHdrService.mainHdr.psd.checkInterval = 10;
			}
			else {
				stTcHdrService.mainHdr.psd.progBitmask = 0;
				stTcHdrService.mainHdr.psd.fieldBitmask = 0;
				stTcHdrService.mainHdr.psd.fNotify = 0;
			}
		}
		else {	// eTC_HDR_ID_BS
			if(fEn > 0U) {
				stTcHdrService.bsHdr.psd.progBitmask = progBitmask;
				stTcHdrService.bsHdr.psd.fieldBitmask = psdBitmask;
				stTcHdrService.bsHdr.psd.fNotify = 1;
				stTcHdrService.bsHdr.psd.checkInterval = 10;
			}
			else {
				stTcHdrService.bsHdr.psd.progBitmask = 0;
				stTcHdrService.bsHdr.psd.fieldBitmask = 0;
				stTcHdrService.bsHdr.psd.fNotify = 0;
			}
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
	}

	return ret;
}


HDRET tchdrsvc_enableGetSis(eTC_HDR_ID_t id, U32 sisBitmask, U32 fEn)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDR_sis_enabled_basic_types_t enabledTypes;

	enabledTypes.all = 0;
	if((sisBitmask & (U32)eBITMASK_SIS_STATION_ID) > 0U) {
		enabledTypes.stationId = 1;
	}
	if((sisBitmask & (U32)eBITMASK_SIS_SHORT_NAME) > 0U) {
		enabledTypes.shortName = 1;
	}

	if((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) {
		if(id == eTC_HDR_ID_MAIN) {
			if(fEn > 0U) {
				stTcHdrService.mainHdr.sis.enDefaultType = enabledTypes.all;
				stTcHdrService.mainHdr.sis.fieldBitmask = sisBitmask;
				stTcHdrService.mainHdr.sis.checkInterval = 0;
				stTcHdrService.mainHdr.sis.fNotify = 1;
			}
			else {
				stTcHdrService.mainHdr.sis.enDefaultType = 0;
				stTcHdrService.mainHdr.sis.fieldBitmask = 0;
				stTcHdrService.mainHdr.sis.fNotify = 0;
			}
		}
		else {	// eTC_HDR_ID_BS
			if(fEn > 0U) {
				stTcHdrService.bsHdr.sis.enDefaultType = enabledTypes.all;
				stTcHdrService.bsHdr.sis.fieldBitmask = sisBitmask;
				stTcHdrService.bsHdr.sis.checkInterval = 0;
				stTcHdrService.bsHdr.sis.fNotify = 1;
			}
			else {
				stTcHdrService.bsHdr.sis.enDefaultType = 0;
				stTcHdrService.bsHdr.sis.fieldBitmask = 0;
				stTcHdrService.bsHdr.sis.fNotify = 0;
			}
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
	}

	return ret;
}


HDRET tchdrsvc_enableGetLot(eTC_HDR_ID_t id, U8 progBitmask, U32 fEn)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) {
		if(id == eTC_HDR_ID_MAIN) {
			if(fEn > (U32)OFF) {
				stTcHdrService.mainHdr.lot.progBitmask = progBitmask;
				stTcHdrService.mainHdr.lot.fNotify = 1;
				(void)tchdrsvc_doReassembly(id, (U32)ON);
			}
			else {
				stTcHdrService.mainHdr.lot.progBitmask = progBitmask;
				stTcHdrService.mainHdr.lot.fNotify = 0;
				(void)tchdrsvc_doReassembly(id, (U32)OFF);
			}
		}
		else { // eTC_HDR_ID_BS
			if(fEn > (U32)OFF) {
				stTcHdrService.bsHdr.lot.progBitmask = progBitmask;
				stTcHdrService.bsHdr.lot.fNotify = 1;
				(void)tchdrsvc_doReassembly(id, (U32)ON);
			}
			else {
				stTcHdrService.bsHdr.lot.progBitmask = progBitmask;
				stTcHdrService.bsHdr.lot.fNotify = 0;
				(void)tchdrsvc_doReassembly(id, (U32)OFF);
			}
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
	}

	return ret;
}

static HDRET tchdrsvc_enableGetAlert(eTC_HDR_ID_t id, U32 fEn)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) {
		if(id == eTC_HDR_ID_MAIN) {
			if(fEn > (U32)OFF) {
				stTcHdrService.mainHdr.alert.fNotify = 1;
			}
			else {
				stTcHdrService.mainHdr.alert.fNotify = 0;
			}
		}
		else {	// eTC_HDR_ID_BS
			if(fEn > (U32)OFF) {
				stTcHdrService.bsHdr.alert.fNotify = 1;
			}
			else {
				stTcHdrService.bsHdr.alert.fNotify = 0;
			}
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
	}

	return ret;
}

static HDRET tchdrsvc_parsingXhdr(stTC_HDR_PSD_XHDR_FRAME_t* xhdr, const U8* data, U32 dataLen)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U8 param_id = 0;
	U32 mime_hash = 0;
	U32 idx = 0;

	/////////////////////////////////////////////////////////////////////
	//    datalen = MIME(4) + Body(ParamID(1)+Len(1)+Value(Variable))
	//
	//    <Minimum>
	//        In XHDR, "datalen" cannot be less than 5.
	//        MIME(4) + Body(ParamID(1)+Len(1) = Minimum --> 6byte
	//    <Maximum>
	//        In XHDR, "datalen" cannot be over than 127.
	/////////////////////////////////////////////////////////////////////

	if(dataLen < 6U) {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_SUPPORT;
	}
	else if(dataLen > 127U) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[XHDR] The dataLen size exceeds the maximum size. (%d)\n",dataLen);
		ret = (HDRET)eTC_HDR_RET_NG_READ_SIZE;
	}
	else {
		(void)(*stOsal.osmemcpy)((void*)&mime_hash, &data[idx], 4U);
		xhdr->mime_hash = mime_hash;
		idx += 4U;	/*MIME hash size(4)*/

		while(idx < dataLen) {
			param_id = data[idx];
			xhdr->params[xhdr->numParams].param_id = param_id;
			idx += 1U;
			xhdr->params[xhdr->numParams].length = data[idx];
			idx += 1U;

			switch(param_id) {
				case 0: //Display Trigger for Image <LOTID>
					if(xhdr->params[xhdr->numParams].length == 0x02U) {
						(void)(*stOsal.osmemcpy)((void*)&(xhdr->params[xhdr->numParams].lot_id), &data[idx], 2U);
						(void)(*stOsal.osmemcpy)((void*)(xhdr->params[xhdr->numParams].value), &data[idx], 2U);
						idx += 2U;
					}
					else {
						(*pfnHdrLog)(eTAG_SYS, eLOG_WRN, "\n[XHDR] Broken parameter[%d] data. paramID[%d], length[%d]\n", xhdr->numParams, param_id, xhdr->params[xhdr->numParams].length);
						if(xhdr->params[xhdr->numParams].length <= (TC_HDR_MAX_LEN_XHDR_PARAM_VALUES-idx+6U)) {
							(void)(*stOsal.osmemcpy)((void*)(xhdr->params[xhdr->numParams].value), &data[idx], (U32)xhdr->params[xhdr->numParams].length);
							idx += (U32)xhdr->params[xhdr->numParams].length;
						}
						else {
							idx = TC_HDR_MAX_LEN_XHDR_PARAM_VALUES+6U;
						}
					}
					break;
				case 1: //blank display (primary image only: Discard the image and displays "station logo" if present)
					if(xhdr->params[xhdr->numParams].length != 0x00U) {
						(*pfnHdrLog)(eTAG_SYS, eLOG_WRN, "\n[XHDR] Broken parameter[%d] data. paramID[%d]\n", xhdr->numParams, param_id);
						if(xhdr->params[xhdr->numParams].length <= (TC_HDR_MAX_LEN_XHDR_PARAM_VALUES-idx+6U)) {
							idx += (U32)xhdr->params[xhdr->numParams].length;
						}
						else {
							idx = TC_HDR_MAX_LEN_XHDR_PARAM_VALUES+6U;
						}
					}
					break;
				case 2: //flush LOT memory
					if(xhdr->params[xhdr->numParams].length != 0x00U) {
						(*pfnHdrLog)(eTAG_SYS, eLOG_WRN, "\n[XHDR] Broken parameter[%d] data. paramID[%d]\n", xhdr->numParams, param_id);
						if(xhdr->params[xhdr->numParams].length <= (TC_HDR_MAX_LEN_XHDR_PARAM_VALUES-idx+6U)) {
							idx += (U32)xhdr->params[xhdr->numParams].length;
						}
						else {
							idx = TC_HDR_MAX_LEN_XHDR_PARAM_VALUES+6U;
						}
					}
					break;
				default: //The rest of parameter ID would not be contained in XHDR structure. Skip anyway.
					if(xhdr->params[xhdr->numParams].length <= (TC_HDR_MAX_LEN_XHDR_PARAM_VALUES-idx+6U)) {
						(void)(*stOsal.osmemcpy)((void*)(xhdr->params[xhdr->numParams].value), &data[idx], (U32)xhdr->params[xhdr->numParams].length);
						idx += (U32)xhdr->params[xhdr->numParams].length;
					}
					else {
						(*pfnHdrLog)(eTAG_SYS, eLOG_WRN, "\n[XHDR] Broken parameter[%d] data. paramID[%d]\n", xhdr->numParams, param_id);
						idx = TC_HDR_MAX_LEN_XHDR_PARAM_VALUES+6U;
					}
					break;
			}

			xhdr->numParams += 1U;
		}
	}
	return ret;
}

static HDRET tchdrsrv_setLotEnable(HDR_instance_t *hdrInstance, HDR_sig_service_component_t serviceComponent, U32 serviceNum, U32 en)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(serviceComponent.processing == 3U) {
		if(en == (U32)ON) {
			/*
			 * [ processing ]
			 *
			 *	  0U - RLS Byte Streaming
			 *	  1U - RLS Packet
			 *	  2U - Reserved - Not Used
			 *	  3U - LOT - Packet
			 */
			ret = HDR_aas_enable_lot_reassembly(hdrInstance, serviceNum, serviceComponent.channel);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Failed to enalbe LOT. InstanceNum[%d], ServiceNumber[%d] PortNumber[0x%X]\n",
					hdrInstance->instance_number, serviceNum, serviceComponent.channel);
				}
				else if(ret == -2) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_RESERVED_PORT_REQ;
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Failed to enalbe LOT. Reserved port number requested. InstanceNum[%d], ServiceNumber[%d] PortNumber[0x%X]\n",
					hdrInstance->instance_number, serviceNum, serviceComponent.channel);
				}
				else if(ret == -3) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_MAX_AAS_PORTS_ALREADY_ENABLED;
					// Continue to run and comment out
					//(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Maximum number of AAS ports already enabled. InstanceNum[%d], ServiceNumber[%d] PortNumber[0x%X]\n",
					//hdrInstance->instance_number, serviceNum, serviceComponent.channel);
				}
				else if(ret == -4) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_MAX_LOT_PORTS_ALREADY_ENABLED;
					// Continue to run and comment out
					//(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Maximum number of LOT ports already enabled. InstanceNum[%d], ServiceNumber[%d] PortNumber[0x%X]\n",
					//hdrInstance->instance_number, serviceNum, serviceComponent.channel);
				}
				else if(ret == -5) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_ALEADY_OPEN_PORT;
					// Continue to run and comment out
					//(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Specified Port Number is already enabled. InstanceNum[%d], ServiceNumber[%d] PortNumber[0x%X]\n",
					//hdrInstance->instance_number, serviceNum, serviceComponent.channel);
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "LOT is enabled. InstanceNum[%d], ServiceNumber[%d] PortNumber[0x%X]\n",
				hdrInstance->instance_number, serviceNum, serviceComponent.channel);
			}
		}
		else {
			ret = HDR_aas_disable_lot_reassembly(hdrInstance, serviceNum, serviceComponent.channel);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Failed to disable LOT. InstanceNum[%d], ServiceNumber[%d] PortNumber[0x%X]\n",
					hdrInstance->instance_number, serviceNum, serviceComponent.channel);
				}
				else if(ret == -2) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_NOT_FOUND_PORT_OR_SERVICE;
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Failed to disable LOT. Specified port number or service number not found. InstanceNum[%d], ServiceNumber[%d] PortNumber[0x%X]\n",
					hdrInstance->instance_number, serviceNum, serviceComponent.channel);
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "LOT is disabled. InstanceNum[%d], ServiceNumber[%d] PortNumber[0x%X]\n",
				hdrInstance->instance_number, serviceNum, serviceComponent.channel);
			}
		}
	}

	return ret;
}

static HDRET tchdrsvc_enableLotReassembly(HDR_instance_t *hdrInstance, U32 serviceNum, U32 en)
{
	U32 compoIndex;
	U32 Program_number=0;
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDR_sig_service_info_t serviceInfo;
	HDR_sig_service_component_t serviceComponent;
	eTC_HDR_ID_t hdrId;
	U8 programBitmask;

	ret = tchdr_getHdrIdFromInstanceNumber(&hdrId, hdrInstance->instance_number);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = HDR_sig_get_service_info(hdrInstance, serviceNum, &serviceInfo);
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			U32 stop = 0U;
			if(hdrId == eTC_HDR_ID_BS) {
				programBitmask = stTcHdrService.bsHdr.lot.progBitmask;
			}
			else {
				programBitmask = stTcHdrService.mainHdr.lot.progBitmask;
			}
			for(compoIndex = 0; compoIndex < serviceInfo.num_components; compoIndex++) {
				(void)(*stOsal.osmemset)((void*)&serviceComponent, (S8)0, (U32)sizeof(HDR_sig_service_component_t));
				ret = HDR_sig_get_service_component(hdrInstance, serviceNum, compoIndex, &serviceComponent);
				if(ret == (HDRET)eTC_HDR_RET_OK) {
					/* Get Program Number */
					if(serviceComponent.component_type == HDR_SIG_AUDIO_COMPONENT) {
						/*
						 *  In the case of a receivable program, one audio component is mandatory.
						 *  In case of (component_type == HDR_SIG_AUDIO_COMPONENT), serviceComponent.channel means (program number).
						 *  In other cases, (component_type == HDR_SIG_DATA_COMPONENT), the purpose is used as the (port number).
						 */
						Program_number = serviceComponent.channel;

						if(Program_number < HDR_MAX_NUM_PROGRAMS) {
							if((programBitmask & ((U8)1 << Program_number)) == 0U) {
								stop = 1U;
							}
						}
						else {
							stop = 1U;
						}
					}
					else {
						ret = tchdrsrv_setLotEnable(hdrInstance, serviceComponent, serviceNum, en);
					}

					/*
					(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "[COMPO-]NumCom(%d), type(%d), process(%d) (service number : %d, channel: 0x%X) ComType(%d)\n",
					serviceInfo.num_components,
					serviceInfo.type,
					serviceComponent.processing,
					serviceNum,
					serviceComponent.channel,
					serviceComponent.component_type);
					*/
				}
				else {
					if(ret == -1) {
						ret = (HDRET)eTC_HDR_RET_NG_SIG_NO_SERVICE;
					}
					else if(ret == -2) {
						ret = (HDRET)eTC_HDR_RET_NG_SIG_NO_COMPONENT;
					}
					else {
						ret = tchdr_convertHdrError((HDR_error_code_t)ret);
					}
					stop = 1U;
				}

				if(stop > 0U) {
					break;
				}
			}
		}
		else if(ret == -1) {
			ret = (HDRET)eTC_HDR_RET_NG_SIG_NO_SERVICE;
		}
		else {
			ret = tchdr_convertHdrError((HDR_error_code_t)ret);
		}
	}

	return ret;
}


static HDRET tchdrsvc_flushLotObjects(U32 serviceNum)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDR_aas_lot_object_list_t lot_list;
	HDR_instance_t *hdrInstance;
	U32 numObject;

	hdrInstance = tchdrfwk_getHdrInstancePtr(eTC_HDR_ID_MAIN);
	(void)(*stOsal.osmemset)((void*)&lot_list, (S8)0, (U32)sizeof(HDR_aas_lot_object_list_t));
	ret = HDR_aas_get_lot_object_list(hdrInstance, serviceNum, &lot_list);
	if(ret == 0) {
		for(numObject = 0; numObject < lot_list.num_objects; numObject++) {
			ret = HDR_aas_flush_lot_object(hdrInstance, lot_list.item[numObject].port_number, lot_list.item[numObject].lot_id);
			if(ret < -2) {
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "[ERR] flush lot object (Port Number: 0x%X, LOT ID: %d) Failed.\n",
					lot_list.item[numObject].port_number, lot_list.item[numObject].lot_id);
				UNUSED(0);
			}
		}

		/*hdrInstance = tchdrfwk_getHdrInstancePtr(eTC_HDR_ID_BS);
		(void)(*stOsal.osmemset)((void*)&lot_list, (S8)0, (U32)sizeof(HDR_aas_lot_object_list_t));
		ret = HDR_aas_get_lot_object_list(hdrInstance, stTcHdrMainLot.service_number, &lot_list);
		if(ret < 0) {
			goto exit;
		}

		for(U32 numObject = 0; numObject < lot_list.num_objects; numObject++) {
			ret = HDR_aas_flush_lot_object(hdrInstance, lot_list.item[numObject].port_number, lot_list.item[numObject].lot_id);
			if(ret < -2){
				(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "[ERR] flush lot object (Port Number: 0x%X, LOT ID: %d) Failed.\n", lot_list.item[numObject].port_number, lot_list.item[numObject].lot_id);
			}
		}*/

		(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "[INFO] All LOT flushed.. \n");
	}
	else {
		if(ret == -1) {
			ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
		}
		else if(ret == -2) {
			ret = (HDRET)eTC_HDR_RET_NG_AAS_NOT_FOUND_OBJECT;
		}
		else {
			ret = tchdr_convertHdrError((HDR_error_code_t)ret);
		}
	}

	return ret;
}


static HDRET tchdrsvc_checkLotComplete(stTCHDR_LOTPROC_PARAM_t* lpp)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDR_aas_lot_object_list_t objectList;
	stTCHDR_LOTPROC_PARAM_t* llpp = lpp;
	U32 objectIdx = 0U;

	ret = HDR_aas_get_lot_object_list(llpp->hdrInstance, llpp->ServNum, &objectList);
	if(ret == 0) {
		ret = (HDRET)eTC_HDR_RET_NG_AAS_NO_COMPLETE_OBJECT;
		for(objectIdx = 0; objectIdx < objectList.num_objects; objectIdx++) {
			if(objectList.item[objectIdx].complete == 1U) {
				llpp->port_number = objectList.item[objectIdx].port_number;
				llpp->lotId = objectList.item[objectIdx].lot_id;
				ret = (HDRET)eTC_HDR_RET_OK;
				break;
			}
		}
	}
	else {
		(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "HDR_aas_get_lot_object_list return %d (serviceNum: %d)\n", ret, llpp->ServNum);
		if(ret == -1) {
			ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
		}
		else {
			ret = tchdr_convertHdrError((HDR_error_code_t)ret);
		}
	}

	return ret;
}

#if 1 // Under fix Coverity warning.
static void tchdrsvc_getStationIdWithConditions(HDR_instance_t *hdrInstance, U8 enabled, HDR_sis_station_id_t *stationId)
{
	if((enabled == 1U) && ((stTcHdrService.mainHdr.sis.fieldBitmask & (U32)eBITMASK_SIS_STATION_ID) > 0U)){
		(void)HDR_sis_get_station_id(hdrInstance, stationId);
    }
}

static void tchdrsvc_getShortNameWithConditions(HDR_instance_t *hdrInstance, U8 enabled, HDR_sis_short_name_t *shortName)
{
	HDR_sis_univ_name_t univName;

	if((enabled == 1U) && ((stTcHdrService.mainHdr.sis.fieldBitmask & (U32)eBITMASK_SIS_SHORT_NAME) > 0U)) {
		S32 rc;
        rc = HDR_sis_get_station_short_name(hdrInstance, shortName);

#if 0
        if((shortName->status == HDR_SIS_NO_DATA) || (shortName->status == HDR_SIS_ERROR)) {
            // No short name; try universal name
            (void)(*stOsal.osmemset)((void*)(&univName), (S8)0, (U32)sizeof(HDR_sis_univ_name_t));
            (void)HDR_sis_get_universal_name(hdrInstance, &univName);

            if(univName.length <= TC_HDR_SIS_SHORT_NAME_MAX_LEN){
				if(univName.status == HDR_SIS_NEW_DATA) {
					shortName->status = HDR_SIS_NEW_DATA;
					shortName->length = univName.length;
					(void)(*stOsal.osmemcpy)((void*)shortName->text, (void*)univName.text, univName.length);
				}
            }
        }
#else   // Modify for 6o-3.auto
		if(rc != 0) {
			shortName->status = HDR_SIS_ERROR;
		}
#endif
    }
}

static void tchdrsvc_getUniversalNameWithConditions(HDR_instance_t *hdrInstance, HDR_sis_univ_name_t *univName)
{
	if((stTcHdrService.mainHdr.sis.fieldBitmask & (U32)eBITMASK_SIS_UNIVERSAL_SHORT_NAME) > 0U) {
		S32 rc;
		rc = HDR_sis_get_universal_name(hdrInstance, univName);
		if(rc != 0) {
			univName->status = HDR_SIS_ERROR;
		}
	}
}

static void tchdrsvc_getSloganWithConditions(HDR_instance_t *hdrInstance, HDR_sis_station_slogan_t *slogan)
{
	if((stTcHdrService.mainHdr.sis.fieldBitmask & (U32)eBITMASK_SIS_SLOGAN) > 0U) {
		S32 rc;
		rc = HDR_sis_get_station_slogan(hdrInstance, slogan);
		if(rc != 0) {
			slogan->status = HDR_SIS_ERROR;
		}
	}
}

HDRET tchdrsvc_getSisBasicData(eTC_HDR_ID_t id, stTC_HDR_SIS_t *dataOut, U32 *status)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDR_sis_enabled_basic_types_t enabledTypes;
	HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);

	if((dataOut != NULL) && (status != NULL)) {
		*status = 0;
		if((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) {
			HDR_sis_station_id_t stationId;
			HDR_sis_short_name_t shortName;
			HDR_sis_univ_name_t univName;
			HDR_sis_station_slogan_t slogan;

			enabledTypes.all = 0;
	        (void)HDR_sis_get_enabled_basic_types(hdrInstance, &enabledTypes);

			// Station ID
			stationId.all = 0U;
			stationId.status = HDR_SIS_NO_DATA;
			tchdrsvc_getStationIdWithConditions(hdrInstance, enabledTypes.stationId, &stationId);
			if(stationId.status == HDR_SIS_NEW_DATA) {
				*status |= (U32)eBITMASK_SIS_STATION_ID;
				dataOut->stationID.all = stationId.all;
			}

			// Short Name
			(void)(*stOsal.osmemset)((void*)&shortName, (S8)0, (U32)sizeof(HDR_sis_short_name_t));
			tchdrsvc_getShortNameWithConditions(hdrInstance, enabledTypes.shortName, &shortName);
			if(shortName.status == HDR_SIS_NEW_DATA) {
				*status |= (U32)eBITMASK_SIS_SHORT_NAME;
				dataOut->shortName.len = shortName.length;
	            (void)(*stOsal.osmemcpy)((void*)(dataOut->shortName.text), (void*)(shortName.text), shortName.length);
			}

			// Universal Name
			(void)(*stOsal.osmemset)((void*)&univName, (S8)0, (U32)sizeof(HDR_sis_univ_name_t));
			tchdrsvc_getUniversalNameWithConditions(hdrInstance, &univName);
			if(univName.status == HDR_SIS_NEW_DATA) {
				*status |= (U32)eBITMASK_SIS_UNIVERSAL_SHORT_NAME;
				dataOut->universalName.len = univName.length;
				dataOut->universalName.charType = (eTC_HDR_SIS_TEXT_ENCODING_t)univName.text_encoding;
				dataOut->universalName.appendFm = (*stCast.booltou32)(univName.append_fm);
				(void)(*stOsal.osmemcpy)((void*)dataOut->universalName.text, (void*)univName.text, univName.length);
			}

			// Slogan
			(void)(*stOsal.osmemset)((void*)&slogan, (S8)0, (U32)sizeof(HDR_sis_station_slogan_t));
			tchdrsvc_getSloganWithConditions(hdrInstance, &slogan);
			if(slogan.status == HDR_SIS_NEW_DATA) {
				*status |= (U32)eBITMASK_SIS_SLOGAN;
				dataOut->slogan.len = slogan.length;
				dataOut->slogan.charType = (eTC_HDR_SIS_TEXT_ENCODING_t)slogan.text_encoding;
				(void)(*stOsal.osmemcpy)((void*)dataOut->slogan.text, (void*)slogan.text, slogan.length);
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	return ret;
}

#else

HDRET tchdrsvc_getSisBasicData(eTC_HDR_ID_t id, stTC_HDR_SIS_t *dataOut, U32 *status)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDRET rc;
	HDR_sis_enabled_basic_types_t enabledTypes;
	HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
	HDR_sis_univ_name_t univName;
	HDR_sis_station_slogan_t slogan;

	if((dataOut != NULL) && (status != NULL)) {
		*status = 0;
		if((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) {
			enabledTypes.all = 0;
			(void)HDR_sis_get_enabled_basic_types(hdrInstance, &enabledTypes);

			if((enabledTypes.stationId == (U8)1) && ((stTcHdrService.mainHdr.sis.fieldBitmask & (U32)eBITMASK_SIS_STATION_ID) > 0U)) {
				HDR_sis_station_id_t stationId;
				(void)(*stOsal.osmemset)((void*)&stationId, (S8)0, (U32)sizeof(HDR_sis_station_id_t));
				(void)HDR_sis_get_station_id(hdrInstance, &stationId);

				dataOut->stationID.all = stationId.all;
				if(stationId.status == HDR_SIS_NEW_DATA) {
					*status |= (U32)eBITMASK_SIS_STATION_ID;
				}
				//dataOut->stationID.status = stationId.status;
			}
			else {
				dataOut->stationID.all = 0;
			}

			if((enabledTypes.shortName == (U8)1) && ((stTcHdrService.mainHdr.sis.fieldBitmask & (U32)eBITMASK_SIS_SHORT_NAME) > 0U)) {
				HDR_sis_short_name_t shortName;
	            (void)(*stOsal.osmemset)((void*)&shortName, (S8)0, (U32)sizeof(HDR_sis_short_name_t));

				(void)HDR_sis_get_station_short_name(hdrInstance, &shortName);

				if((shortName.status != HDR_SIS_NO_DATA) && (shortName.status != HDR_SIS_ERROR)) {
					if(shortName.status == HDR_SIS_NEW_DATA) {
						*status |= (U32)eBITMASK_SIS_SHORT_NAME;
					}
					//dataOut->shortName.status = (eTC_HDR_DATA_STATUS_t)shortName.status;
					dataOut->shortName.len = shortName.length;
	                (void)(*stOsal.osmemcpy)((void*)(dataOut->shortName.text), (void*)(shortName.text), shortName.length);
				}
				else {
					// No short name; try universal name
	                (void)(*stOsal.osmemset)((void*)(&univName), (S8)0, (U32)sizeof(HDR_sis_univ_name_t));
					(void)HDR_sis_get_universal_name(hdrInstance, &univName);

					if(univName.length <= TC_HDR_SIS_SHORT_NAME_MAX_LEN) {
						if(univName.status == HDR_SIS_NEW_DATA) {
							*status |= (U32)eBITMASK_SIS_SHORT_NAME;
						}
						//dataOut->shortName.status = univName.status;
						dataOut->shortName.len = univName.length;
	 					(void)(*stOsal.osmemcpy)((void*)dataOut->shortName.text, (void*)univName.text, univName.length);
					}
					else {
						//dataOut->shortName.status = HDR_SIS_NO_DATA;
						dataOut->shortName.len = 0;
						(void)(*stOsal.osmemset)((void*)dataOut->shortName.text, (S8)0, (U32)sizeof(dataOut->shortName.text));
					}
				}
			}
			else {
				dataOut->shortName.len = 0;
			}

			(void)(*stOsal.osmemset)((void*)&univName, (S8)0, (U32)sizeof(HDR_sis_univ_name_t));
			rc = HDR_sis_get_universal_name(hdrInstance, &univName);
			if(((stTcHdrService.mainHdr.sis.fieldBitmask & (U32)eBITMASK_SIS_UNIVERSAL_SHORT_NAME) > 0U)  && (rc == 0)) {
				if(univName.status == HDR_SIS_NEW_DATA) {
					*status |= (U32)eBITMASK_SIS_UNIVERSAL_SHORT_NAME;
				}
				//taOut->universalName.status = (eTC_HDR_DATA_STATUS_t)univName.status;
				dataOut->universalName.charType = (eTC_HDR_SIS_TEXT_ENCODING_t)univName.text_encoding;
				if(univName.append_fm == true) {
					dataOut->universalName.appendFm = 1;
				}
				else {
					dataOut->universalName.appendFm = 0;
				}
				dataOut->universalName.len = univName.length;
				(void)(*stOsal.osmemset)((void*)(dataOut->universalName.text), (S8)0, (U32)sizeof(dataOut->universalName.text));
				if(univName.length > 0U) {
					(void)(*stOsal.osmemcpy)((void*)dataOut->universalName.text, (void*)univName.text, univName.length);
				}
			}
			else {
				dataOut->universalName.len = 0;
				dataOut->universalName.charType = eTC_HDR_SIS_ISO_IEC_8859_1_1998;
				dataOut->universalName.appendFm = 0;
			}

			(void)(*stOsal.osmemset)((void*)&slogan, (S8)0, (U32)sizeof(HDR_sis_station_slogan_t));
			rc = HDR_sis_get_station_slogan(hdrInstance, &slogan);
			if(((stTcHdrService.mainHdr.sis.fieldBitmask & (U32)eBITMASK_SIS_SLOGAN) > 0U) && (rc == 0)) {
				if(slogan.status == HDR_SIS_NEW_DATA) {
					*status |= (U32)eBITMASK_SIS_SLOGAN;
				}
				dataOut->slogan.charType = (eTC_HDR_SIS_TEXT_ENCODING_t)slogan.text_encoding;
				dataOut->slogan.len = slogan.length;
				(void)(*stOsal.osmemset)((void*)dataOut->slogan.text, (S8)0, (U32)sizeof(dataOut->slogan.text));
				if(slogan.length > 0U) {
					(void)(*stOsal.osmemcpy)((void*)dataOut->slogan.text, (void*)slogan.text, slogan.length);
				}
			}
			else {
				dataOut->slogan.len = 0;
				dataOut->slogan.charType = eTC_HDR_SIS_ISO_IEC_8859_1_1998;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	return ret;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////
/*								Telechips HD Radio Service Event Functions							*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
static eTCHDR_EVT_STS_t tchdrsvc_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen == 0U) {
		ret = tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_BBINPUT_CMD_OPEN, stRcvMsgQ.uiData, pNULL, 0);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_ALREADY_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		ret = tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_BBINPUT_CMD_CLOSE, pNULL, pNULL, 0);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	UNUSED(stRcvMsgQ);
	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_reset_bbp(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		if(stRcvMsgQ.uiData[0] > 0U) {
			ret = tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_BBINPUT_CMD_RESET_BS, stRcvMsgQ.uiData, pNULL, 0);
		}
		else {
			ret = tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_BBINPUT_CMD_RESET_MAIN, stRcvMsgQ.uiData, pNULL, 0);
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_set_mainAudioMode(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		ret = tchdrblending_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_BLENDING_CMD_AUDIO_MODE, stRcvMsgQ.uiData, pNULL, 0);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_set_tune(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	U32 id = stRcvMsgQ.uiData[0];

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		stTcHdrService.statusChkInterval = 0U;
		if(id == (U32)eTC_HDR_ID_MAIN) {
            // eTCHDR_BBINPUT_NOTIFY_TUNE 이벤트 발생 시점으로 변경 (이전 서비스의 PTY 가 전달되는 이슈 수정)
			// stTcHdrService.mainHdr.pty.fNotify = 1U;
			// stTcHdrService.mainHdr.psd.checkInterval = 10U;
			// stTcHdrService.mainHdr.sis.checkInterval = 100U;
			// //stTcHdrService.mainHdr.status.fNotify = 1U;
			// //stTcHdrService.mainSts.acqStatus.all = 0U;
			// stTcHdrService.mainSts.cdno = 0U;
			// stTcHdrService.mainSts.hybrid = false;
            // ~eTCHDR_BBINPUT_NOTIFY_TUNE 이벤트 발생 시점으로 변경 (이전 서비스의 PTY 가 전달되는 이슈 수정)
			ret = tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_BBINPUT_CMD_SET_TUNE, stRcvMsgQ.uiData, pNULL, 0);
		}
		else if(id == (U32)eTC_HDR_ID_BS) {
			stTcHdrService.bsHdr.pty.fNotify = 1U;
			stTcHdrService.bsHdr.psd.checkInterval = 10U;
			stTcHdrService.bsHdr.sis.checkInterval = 100U;
			//stTcHdrService.bsHdr.status.fNotify = 1U;
			//stTcHdrService.bsSts.acqStatus.all = 0U;
			stTcHdrService.bsSts.cdno = 0U;
			stTcHdrService.bsSts.hybrid = false;
			ret = tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_BBINPUT_CMD_SET_TUNE, stRcvMsgQ.uiData, pNULL, 0);
		}
		else if(id == (U32)eTC_HDR_ID_MRC) {
			ret = tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_BBINPUT_CMD_SET_TUNE, stRcvMsgQ.uiData, pNULL, 0);
		}
		else if(id == (U32)eTC_HDR_ID_BS_MRC) {
			ret = tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_BBINPUT_CMD_SET_TUNE, stRcvMsgQ.uiData, pNULL, 0);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_set_program(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
	U32 uiID = stRcvMsgQ.uiData[0];
	U32 numOfProgram = stRcvMsgQ.uiData[1];
	stHDR_FRAMEWORK_DATA_t* frameworkData = tchdrfwk_getDataStructPtr();

	if(stTcHdrService.fOpen > 0U) {
		if((frameworkData != NULL) && (numOfProgram <= (U32)HDR_PROGRAM_HD8)) {
			if(uiID == (U32)eTC_HDR_ID_MAIN) {
				ret = HDR_set_playing_program(&frameworkData->hdrInstance[0] , (HDR_program_t)numOfProgram);
				if(ret < 0) {
					ret = (HDRET)eTC_HDR_RET_NG_SET_PROGRAM;
				}
				else {
					stTcHdrService.mainHdrProgNum = (HDR_program_t)numOfProgram;
				}
			}
			else if(uiID == (U32)eTC_HDR_ID_BS) {
				if(tchdrfwk_getHdrType() == HDR_1p5_CONFIG) {
					 ret = HDR_set_playing_program(&frameworkData->hdrInstance[1] , (HDR_program_t)numOfProgram);
				}
				else if((tchdrfwk_getHdrType() == HDR_1p5_MRC_CONFIG) || (tchdrfwk_getHdrType() == HDR_1p5_DUAL_MRC_CONFIG)) {
					 ret = HDR_set_playing_program(&frameworkData->hdrInstance[2] , (HDR_program_t)numOfProgram);
				}
				else {
					ret = -1;
				}
				if(ret < 0) {
					ret = (HDRET)eTC_HDR_RET_NG_SET_PROGRAM;
				}
				else {
					stTcHdrService.bsHdrProgNum = (HDR_program_t)numOfProgram;
				}
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_INSTANCE;
		}

		if(ret == (HDRET)eTC_HDR_RET_OK) {
			if(uiSendMsg != NULL) {
				uiSendMsg[0] = stRcvMsgQ.uiData[0];		// id
				uiSendMsg[1] = stRcvMsgQ.uiData[1];		// program number
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_EVENT_ERROR;
			}
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
	}
	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_get_status(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(stTcHdrService.fOpen > 0U) {
		if(uiSendMsg != NULL) {
			stTC_HDR_STATUS_t rcvStatus;
			(void)(*stOsal.osmemset)((void*)&rcvStatus, (S8)0, (U32)sizeof(stTC_HDR_STATUS_t));
			if(stRcvMsgQ.uiData[0] == (U32)eTC_HDR_ID_MAIN) {
				ret = tchdrsvc_getHdrStatus(eTC_HDR_ID_MAIN, &rcvStatus);
			}
			else if(stRcvMsgQ.uiData[0] == (U32)eTC_HDR_ID_BS) {
				ret = tchdrsvc_getHdrStatus(eTC_HDR_ID_BS, &rcvStatus);
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
			}
			(void)(*stOsal.osmemcpy)((void*)uiSendMsg, (void*)&rcvStatus, (U32)sizeof(stTC_HDR_STATUS_t));
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_EVENT_ERROR;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_enable_get_psd(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	const U32 *msg = stRcvMsgQ.uiData;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		if(msg[0] == (U32)eTC_HDR_ID_MAIN) {
			ret = tchdrsvc_enableGetPsd(eTC_HDR_ID_MAIN, (*stCast.u32tou8)(msg[1]), (*stCast.u32tou8)(msg[2]), msg[3]);
		}
		else if(msg[0] == (U32)eTC_HDR_ID_BS) {
			ret = tchdrsvc_enableGetPsd(eTC_HDR_ID_BS, (*stCast.u32tou8)(msg[1]), (*stCast.u32tou8)(msg[2]), msg[3]);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_enable_get_sis(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	const U32 *msg = stRcvMsgQ.uiData;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		if(stRcvMsgQ.uiData[0] == (U32)eTC_HDR_ID_MAIN) {
			ret = tchdrsvc_enableGetSis(eTC_HDR_ID_MAIN, msg[1], msg[2]);
		}
		else if(stRcvMsgQ.uiData[1] == (U32)eTC_HDR_ID_BS) {
			ret = tchdrsvc_enableGetSis(eTC_HDR_ID_BS, msg[1], msg[2]);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_enable_get_sig(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		// To Do Something
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	UNUSED(stRcvMsgQ);
	return eEvtSt;
}


static eTCHDR_EVT_STS_t tchdrsvc_event_enable_get_lot(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	const U32 *msg = stRcvMsgQ.uiData;

	if(uiSendMsg != NULL) {
		uiSendMsg[0] = stRcvMsgQ.uiData[0];
		uiSendMsg[1] = stRcvMsgQ.uiData[1];
		uiSendMsg[2] = stRcvMsgQ.uiData[2];
	}

	if(stTcHdrService.fOpen > 0U) {
		if(msg[0] == (U32)eTC_HDR_ID_MAIN) {
			ret = tchdrsvc_enableGetLot(eTC_HDR_ID_MAIN, (*stCast.u32tou8)(msg[1]), msg[2]);
		}
		else if(msg[0] == (U32)eTC_HDR_ID_BS) {
			ret = tchdrsvc_enableGetLot(eTC_HDR_ID_BS,  (*stCast.u32tou8)(msg[1]), msg[2]);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	return eEvtSt;
}


static eTCHDR_EVT_STS_t tchdrsvc_event_enable_get_aas(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		// To Do Something
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	UNUSED(stRcvMsgQ);
	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_enable_get_alert(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	const U32 *msg = stRcvMsgQ.uiData;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		if(msg[0] == (U32)eTC_HDR_ID_MAIN) {
			ret = tchdrsvc_enableGetAlert(eTC_HDR_ID_MAIN, msg[1]);
		}
		else if(msg[0] == (U32)eTC_HDR_ID_BS) {
			ret = tchdrsvc_enableGetAlert(eTC_HDR_ID_BS, msg[1]);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	UNUSED(stRcvMsgQ);
	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_set_mute(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		ret = tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_AUDIO_CMD_MUTE, stRcvMsgQ.uiData, pNULL, 0);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_set_audio_ctrl(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	U32 audio_ctrl = stRcvMsgQ.uiData[0];

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		if(audio_ctrl > 0U) {
			ret = tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_AUDIO_CMD_START, stRcvMsgQ.uiData, pNULL, 0);
		}
		else {
			ret = tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_AUDIO_CMD_STOP, stRcvMsgQ.uiData, pNULL, 0);
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_test(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		U32 num_api = stRcvMsgQ.uiData[0];
		switch (num_api) {
			case (U32)eTCHDR_DEBUG_BBINPUT_BUFFER_RESET:
			case (U32)eTCHDR_DEBUG_BBINPUT_DRIVER_RESET:
			case (U32)eTCHDR_DEBUG_BBINPUT_DRV_BUF_RESET:
			case (U32)eTCHDR_DEBUG_BBINPUT_IQ_DUMP:
				ret = tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_BBINPUT_CMD_TEST, stRcvMsgQ.uiData, pNULL, 0);
				break;
			case (U32)eTCHDR_DEBUG_AUDIO_OUTPUT_DUMP:
				ret = tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_SERVICE, (U32)eTCHDR_AUDIO_CMD_TEST, stRcvMsgQ.uiData, pNULL, 0);
				break;
			default:
				/* Nothing To Do */
				break;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrsvc_event_set_temp(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(stTcHdrService.fOpen > 0U) {
		// To Do Something
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	return eEvtSt;
}

