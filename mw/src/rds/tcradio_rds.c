/*******************************************************************************

*   FileName : tcradio_rds.c

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
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "tcradio_types.h"
#include "tcradio_api.h"
#include "tcradio_utils.h"
#include "tcradio_thread.h"
#include "tcradio_msgq.h"
#include "tcradio_hal.h"
#include "tcradio_service.h"
#include "tcradio_rds_api.h"
#include "tcradio_rds_def.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
stRDS_t stRds;

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/
extern void tcrds_clearData(void);

/***************************************************
*             Local preprocessor                   *
****************************************************/

/***************************************************
*           Local constant definitions              *
****************************************************/
static pthread_t rdsMainThreadID = (pthread_t)NULL;

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/
static void *tcrds_mainThread(void *arg);
static void tcrds_notifyHandler(void);
static void tcrds_setMainMode(eRDS_EVENT_t mode);
static eRDS_EVENT_t tcrds_getMainMode(void);
static void tcrds_serviceMessageParser(stMsgBuf_t *pstMsg);
static void tcrds_mainHandler(void);
static eRDS_STS_t tcrds_event_open(uint32 *uiSendMsg, int32 *iError);
static eRDS_STS_t tcrds_event_close(uint32 *uiSendMsg, int32 *iError);
static eRDS_STS_t tcrds_event_deinit(uint32 *uiSendMsg, int32 *iError);
static eRDS_STS_t tcrds_event_reset(uint32 *uiSendMsg, int32 *iError);

/***************************************************
*			function definition				*
****************************************************/
RET tcrds_init(void)
{
	RET ret = eRET_OK;

	if(rdsMainThreadID == (pthread_t)NULL) {
		ret = tcradio_createThread(&rdsMainThreadID, &tcrds_mainThread, "TCRADIO_RDS", eRADIO_SCHED_RR, 5, pNULL);
		if(ret != 0) {
			RDS_ERR("[%s:%d] Can not make RDS thread!!!\n", __func__, __LINE__);
			ret = eRET_NG_CREATE_THREAD;
			goto error_init;
		}
	}
	else {
		RDS_DBG("[%s:%d] Already RDS thread was created!!!\n", __func__, __LINE__);
	}

	ret = tcradiodatasystem_messageInit();

error_init:

	return ret;
}

RET tcrds_deinit(void)
{
	RET ret;
	stRds.fThreadRunning = 0;
	rdsMainThreadID = (pthread_t)NULL;
	ret = tcradio_joinThread(rdsMainThreadID, (void**)NULL);
	return ret;
}

RET tcrds_open(uint32 ntuner)
{
	RET ret = eRET_OK;

	tcradiodatasystem_mutexLock();
	ret = tcradiohal_setRdsConfig(ON, (uint32)0, ntuner);
	if(ret == eRET_OK) {
		tcrds_setEnable(ON);
	}
	tcradiodatasystem_mutexUnlock();

	return ret;
}

RET tcrds_close(uint32 ntuner)
{
	RET ret = eRET_OK;

	tcradiodatasystem_mutexLock();
	tcradiohal_setRdsConfig(OFF, (uint32)0, ntuner);
	tcrds_clearData();
	tcrds_setEnable(OFF);
	tcradiodatasystem_mutexUnlock();

	return ret;
}

void tcrds_reset(uint32 ntuner)
{
	tcradiodatasystem_mutexLock();
	tcrds_clearData();
	tcradiodatasystem_mutexUnlock();
}

static void *tcrds_mainThread(void *arg)
{
	RET ret = eRET_OK;
	stMsgBuf_t stRecivedMessage = {0,};

	prctl(PR_SET_NAME, "TCRADIO_RDS",0,0,0);

    uint32 rds_dsp_cnt = 0;

	stRds.fThreadRunning = 1;
	while(stRds.fThreadRunning > 0) {
        if (rds_dsp_cnt == 50)
        {
            RDS_ERR("Received PI Code = 0x%04x, PTY Code = %d, Ps Name = %s\n",
                ((uint16)stRds.pih << 8 | (uint16)stRds.pil), (stRds.ptyStatus & 0x1F), stRds.psname);
            rds_dsp_cnt = 0;
        }

		tcradiodatasystem_getMessage(&stRecivedMessage);
		if(stRecivedMessage.fNewMsg == eNEW_MSG_EXIST) {
			switch(stRecivedMessage.uiSender) {
				case eSENDER_ID_SERVICE:
					tcrds_serviceMessageParser(&stRecivedMessage);
					break;

				default:
					break;
			}
		}
		tcrds_mainHandler();
		tcrds_notifyHandler();
		tcrds_fetchRdsDataHandler();

        rds_dsp_cnt ++;
		tcradio_mssleep(RDS_THREAD_TIME_INTERVAL);
	}

	stRds.fThreadRunning = -1;
	tcradiodatasystem_messageDeinit();
	tcradio_exitThread((void *)0);

	return ((void*)0);
}

static void tcrds_notifyHandler(void)
{
	// Handle notify from other thread.
}

static void tcrds_setMainMode(eRDS_EVENT_t mode)
{
	stRds.eMainMode = mode;
}

static eRDS_EVENT_t tcrds_getMainMode(void)
{
	return stRds.eMainMode;
}

static void tcrds_serviceMessageParser(stMsgBuf_t *pstMsg)
{
	uint32 i;

	switch((eRDS_CMD_t)pstMsg->uiMode) {
		case eRDS_CMD_DEINIT:
			tcrds_setMainMode(eRDS_EVT_DEINIT);
			break;

		case eRDS_CMD_OPEN:
			tcrds_setMainMode(eRDS_EVT_OPEN);
			break;

		case eRDS_CMD_CLOSE:
			tcrds_setMainMode(eRDS_EVT_CLOSE);
			break;

		case eRDS_CMD_RESET:
			tcrds_setMainMode(eRDS_EVT_RESET);
			break;

		default:
			break;
	}
}

static void tcrds_mainHandler(void)
{
	eRDS_STS_t eRdsSt = eRDS_STS_OK;
	eRDS_EVENT_t eRdsNowExeMode;
	RET ret = eRET_OK;
	uint32 uiSendMsg[MSGQ_DATA_LENGTH] = {0,}, i, j;

	eRdsNowExeMode = tcrds_getMainMode();

	switch(eRdsNowExeMode) {
		case eRDS_EVT_DEINIT:
			eRdsSt = tcrds_event_deinit(uiSendMsg, &ret);
			break;

		case eRDS_EVT_OPEN:
			eRdsSt = tcrds_event_open(uiSendMsg, &ret);
			break;

		case eRDS_EVT_CLOSE:
			eRdsSt = tcrds_event_close(uiSendMsg, &ret);
			break;

		case eRDS_EVT_RESET:
			eRdsSt = tcrds_event_reset(uiSendMsg, &ret);
			break;

		default:
			break;
	}

	if(tcrds_getMainMode() == eRdsNowExeMode) {
		tcrds_setMainMode(eRDS_EVT_NULL);
	}

	switch(eRdsSt) {
		/* Job End -> No Notify */
		case eRDS_STS_WAIT:
			break;

		/* Job Good Complete */
		case eRDS_STS_OK:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eRDS_STS_OK_NOTIFY:
			switch(eRdsNowExeMode) {
				case eRDS_EVT_OPEN:
					tcradioservice_sendMessage(eSENDER_ID_RDS, eRDS_NOTIFY_OPEN, uiSendMsg, pNULL, ret);
					break;

				case eRDS_EVT_CLOSE:
					tcradioservice_sendMessage(eSENDER_ID_RDS, eRDS_NOTIFY_CLOSE, uiSendMsg, pNULL, ret);
					break;

				case eRDS_EVT_DEINIT:
					tcradioservice_sendMessage(eSENDER_ID_RDS, eRDS_NOTIFY_DEINIT, uiSendMsg, pNULL, ret);
					break;

				case eRDS_EVT_RESET:
					tcradioservice_sendMessage(eSENDER_ID_RDS, eRDS_NOTIFY_RESET, uiSendMsg, pNULL, ret);
					break;

				default:
					break;
			}
			break;

		/* Job Continue */
		case eRDS_STS_DOING:
			tcrds_setMainMode(eRdsNowExeMode);
			break;

		/* Job Continue -> Information Dynamic Notify */
		case eRDS_STS_DOING_NOTIFY:
			tcrds_setMainMode(eRdsNowExeMode);
			tcradioservice_sendMessage(eSENDER_ID_RDS, eRdsNowExeMode, uiSendMsg, pNULL, ret);
			break;

		/* Job Continue -> Error Notify */
		case eRDS_STS_DOING_ERROR_NOTIFY:
			tcrds_setMainMode(eRdsNowExeMode);	/* No break because of doing below function */

		/* Job Error -> Return Error */
		case eRDS_STS_ERROR:
			switch(eRdsNowExeMode) {
				case eRDS_EVT_OPEN:
					tcradioservice_sendMessage(eSENDER_ID_RDS, eRDS_NOTIFY_OPEN, uiSendMsg, pNULL, ret);
					break;
				default:
					break;
			}
			break;

		/* Return Error */
		default:
			break;
	}
}

static eRDS_STS_t tcrds_event_open(uint32 *uiSendMsg, int32 *iError)
{
	RET ret = eRET_OK;
	eRDS_STS_t eRdsSt = eRDS_STS_OK_NOTIFY;
	RDS_DBG("[%s:%d] \n", __func__, __LINE__);

	ret = tcradiohal_setRdsConfig(ON, (uint32)0, eRADIO_ID_PRIMARY);
	if(ret == eRET_OK) {
		stRds.fEnable = 1;
	}

	*iError = ret;
	return eRdsSt;
}

static eRDS_STS_t tcrds_event_close(uint32 *uiSendMsg, int32 *iError)
{
	RET ret = eRET_OK;
	eRDS_STS_t eRdsSt = eRDS_STS_OK_NOTIFY;
	RDS_DBG("[%s:%d] \n", __func__, __LINE__);

	tcradiohal_setRdsConfig(OFF, (uint32)0, eRADIO_ID_PRIMARY);
	tcrds_clearData();
	stRds.fEnable = 0;

	*iError = ret;
	return eRdsSt;
}

static eRDS_STS_t tcrds_event_deinit(uint32 *uiSendMsg, int32 *iError)
{

	RET ret;
	eRDS_STS_t eRdsSt = eRDS_STS_OK_NOTIFY;
	RDS_DBG("[%s:%d] \n", __func__, __LINE__);

	ret = tcrds_deinit();
	if(ret != eRET_OK) {
		eRdsSt= eRDS_STS_ERROR;
	}

	*iError = ret;
	return eRdsSt;
}

static eRDS_STS_t tcrds_event_reset(uint32 *uiSendMsg, int32 *iError)
{
	RET ret = eRET_OK;
	eRDS_STS_t eRdsSt = eRDS_STS_OK_NOTIFY;
	RDS_DBG("[%s:%d] \n", __func__, __LINE__);

	tcrds_clearData();

	*iError = ret;
	return eRdsSt;
}

void tcrds_setEnable(int32 fonoff)
{
	if(fonoff)
		stRds.fEnable = 1;
	else
		stRds.fEnable = 0;
}