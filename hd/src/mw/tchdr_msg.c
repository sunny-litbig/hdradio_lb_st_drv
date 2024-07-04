/*******************************************************************************

*   FileName : tchdr_msg.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio framework message queue and mutex functions

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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "tchdr_common.h"
#include "tchdr_msg.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
static pthread_mutex_t gpTcHdrAppMsgMutex = PTHREAD_MUTEX_INITIALIZER;
static stTcHdrMsgQueue_t pstTcHdrAppMsg;

static pthread_mutex_t gpTcHdrServiceMsgMutex = PTHREAD_MUTEX_INITIALIZER;
static stTcHdrMsgQueue_t pstTcHdrServiceMsg;

static pthread_mutex_t gpTcHdrAudInputMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gpTcHdrIqInputMutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t gpTcHdrBbInputMsgMutex = PTHREAD_MUTEX_INITIALIZER;
static stTcHdrMsgQueue_t pstTcHdrBbInputMsg;

static pthread_mutex_t gpTcHdrMainMsgMutex = PTHREAD_MUTEX_INITIALIZER;
static stTcHdrMsgQueue_t pstTcHdrMainMsg;

static pthread_mutex_t gpTcHdrMrcMsgMutex = PTHREAD_MUTEX_INITIALIZER;
static stTcHdrMsgQueue_t pstTcHdrMrcMsg;

static pthread_mutex_t gpTcHdrBsMsgMutex = PTHREAD_MUTEX_INITIALIZER;
static stTcHdrMsgQueue_t pstTcHdrBsMsg;

static pthread_mutex_t gpTcHdrBsMrcMsgMutex = PTHREAD_MUTEX_INITIALIZER;
static stTcHdrMsgQueue_t pstTcHdrBsMrcMsg;

static pthread_mutex_t gpTcHdrBlendingMsgMutex = PTHREAD_MUTEX_INITIALIZER;
static stTcHdrMsgQueue_t pstTcHdrBlendingMsg;

static pthread_mutex_t gpTcHdrAudioMsgMutex = PTHREAD_MUTEX_INITIALIZER;
static stTcHdrMsgQueue_t pstTcHdrAudioMsg;

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

/***************************************************
*			function definition				*
****************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*	Telechips HD Radio Message Queue API																	*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
HDRET tchdrapp_messageInit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	(void)(*stOsal.osmemset)((void*)&pstTcHdrAppMsg, (S8)0x00, (U32)sizeof(stTcHdrMsgQueue_t));
	ret = (*stOsal.mutexinit)(&gpTcHdrAppMsgMutex, NULL);
	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] Failed to create mutex for application!\n", __func__, __LINE__);
	}

	return ret;
}

HDRET tchdrapp_messageDeinit(void)
{
	HDRET ret;
	ret = (*stOsal.mutexdeinit)(&gpTcHdrAppMsgMutex);
	return ret;
}

HDRET tchdrsvc_messageInit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	(void)(*stOsal.osmemset)((void*)&pstTcHdrServiceMsg, (S8)0x00, (U32)sizeof(stTcHdrMsgQueue_t));
	ret = (*stOsal.mutexinit)(&gpTcHdrServiceMsgMutex, NULL);
	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] Failed to create mutex for manager!\n", __func__, __LINE__);
	}

	return ret;
}

HDRET tchdrsvc_messageDeinit(void)
{
	HDRET ret;
	ret = (*stOsal.mutexdeinit)(&gpTcHdrServiceMsgMutex);
	return ret;
}

HDRET tchdrbbinput_messageInit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	(void)(*stOsal.osmemset)((void*)&pstTcHdrBbInputMsg, (S8)0x00, (U32)sizeof(stTcHdrMsgQueue_t));
	ret = (*stOsal.mutexinit)(&gpTcHdrBbInputMsgMutex, NULL);
	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "[%s:%d] Failed to create mutex for bbinput!\n", __func__, __LINE__);
	}

	return ret;
}

HDRET tchdrbbinput_messageDeinit(void)
{
	HDRET ret;
	ret = (*stOsal.mutexdeinit)(&gpTcHdrBbInputMsgMutex);
	return ret;
}

HDRET tchdriqinput_messageInit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	ret = (*stOsal.mutexinit)(&gpTcHdrIqInputMutex, NULL);
	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "[%s:%d] Failed to create mutex for I/Q input!\n", __func__, __LINE__);
	}

	return ret;
}

HDRET tchdriqinput_messageDeinit(void)
{
	HDRET ret;
	ret = (*stOsal.mutexdeinit)(&gpTcHdrIqInputMutex);
	return ret;
}

HDRET tchdraudinput_messageInit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	ret = (*stOsal.mutexinit)(&gpTcHdrAudInputMutex, NULL);
	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "[%s:%d] Failed to create mutex for audio input!\n", __func__, __LINE__);
	}

	return ret;
}

HDRET tchdraudinput_messageDeinit(void)
{
	HDRET ret;
	ret = (*stOsal.mutexdeinit)(&gpTcHdrAudInputMutex);
	return ret;
}

HDRET tchdrmain_messageInit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	(void)(*stOsal.osmemset)((void*)&pstTcHdrMainMsg, (S8)0x00, (U32)sizeof(stTcHdrMsgQueue_t));
	ret = (*stOsal.mutexinit)(&gpTcHdrMainMsgMutex, NULL);
	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*pfnHdrLog)(eTAG_PRI, eLOG_ERR, "[%s:%d] Failed to create mutex for primary demod!\n", __func__, __LINE__);
	}

	return ret;
}

HDRET tchdrmain_messageDeinit(void)
{
	HDRET ret;
	ret = (*stOsal.mutexdeinit)(&gpTcHdrMainMsgMutex);
	return ret;
}

HDRET tchdrmrc_messageInit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	(void)(*stOsal.osmemset)((void*)&pstTcHdrMrcMsg, (S8)0x00, (U32)sizeof(stTcHdrMsgQueue_t));
	ret = (*stOsal.mutexinit)(&gpTcHdrMrcMsgMutex, NULL);
	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*pfnHdrLog)(eTAG_MRC, eLOG_ERR, "[%s:%d] Failed to create mutex for MRC demod!\n", __func__, __LINE__);
	}

	return ret;
}

HDRET tchdrmrc_messageDeinit(void)
{
	HDRET ret;
	ret = (*stOsal.mutexdeinit)(&gpTcHdrMrcMsgMutex);
	return ret;
}

HDRET tchdrbs_messageInit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	(void)(*stOsal.osmemset)((void*)&pstTcHdrBsMsg, (S8)0x00, (U32)sizeof(stTcHdrMsgQueue_t));
	ret = (*stOsal.mutexinit)(&gpTcHdrBsMsgMutex, NULL);
	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*pfnHdrLog)(eTAG_BS, eLOG_ERR, "[%s:%d] Failed to create mutex for background-scan demod!\n", __func__, __LINE__);
	}

	return ret;
}

HDRET tchdrbs_messageDeinit(void)
{
	HDRET ret;
	ret = (*stOsal.mutexdeinit)(&gpTcHdrBsMsgMutex);
	return ret;
}

HDRET tchdrbsmrc_messageInit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	(void)(*stOsal.osmemset)((void*)&pstTcHdrBsMrcMsg, (S8)0x00, (U32)sizeof(stTcHdrMsgQueue_t));
	ret = (*stOsal.mutexinit)(&gpTcHdrBsMrcMsgMutex, NULL);
	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*pfnHdrLog)(eTAG_BSMRC, eLOG_ERR, "[%s:%d] Failed to create mutex for BS MRC demod!\n", __func__, __LINE__);
	}

	return ret;
}

HDRET tchdrbsmrc_messageDeinit(void)
{
	HDRET ret;
	ret = (*stOsal.mutexdeinit)(&gpTcHdrBsMrcMsgMutex);
	return ret;
}

HDRET tchdraudoutput_messageInit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	(void)(*stOsal.osmemset)((void*)&pstTcHdrAudioMsg, (S8)0x00, (U32)sizeof(stTcHdrMsgQueue_t));
	ret = (*stOsal.mutexinit)(&gpTcHdrAudioMsgMutex, NULL);
	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*pfnHdrLog)(eTAG_AOUT, eLOG_ERR, "[%s:%d] Failed to create mutex for audio output!\n", __func__, __LINE__);
	}

	return ret;
}

HDRET tchdraudoutput_messageDeinit(void)
{
	HDRET ret;
	ret = (*stOsal.mutexdeinit)(&gpTcHdrAudioMsgMutex);
	return ret;
}

HDRET tchdrblending_messageInit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	(void)(*stOsal.osmemset)((void*)&pstTcHdrBlendingMsg, (S8)0x00, (U32)sizeof(stTcHdrMsgQueue_t));
	ret = (*stOsal.mutexinit)(&gpTcHdrBlendingMsgMutex, NULL);
	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*pfnHdrLog)(eTAG_BLD, eLOG_ERR, "[%s:%d] Failed to create mutex for audio blending!\n", __func__, __LINE__);
	}

	return ret;
}

HDRET tchdrblending_messageDeinit(void)
{
	HDRET ret;
	ret = (*stOsal.mutexdeinit)(&gpTcHdrBlendingMsgMutex);
	return ret;
}

static void tchdrmsg_sendWithoutMutex(stTcHdrMsgQueue_t *pstSendMsg, eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError)
{
	U32 dataLen = (U32)TCHDR_MSGQ_DATA_LENGTH * (U32)sizeof(U32);
	U32 pDataLen = (U32)TCHDR_MSGQ_PDATA_LENGTH * (U32)sizeof(void*);

	pstSendMsg->msg[pstSendMsg->wp].uiSender = (U32)eSenderID;
	pstSendMsg->msg[pstSendMsg->wp].fNewMsg = (U32)eTCHDR_NEW_MSG_EXIST;
	pstSendMsg->msg[pstSendMsg->wp].uiMode = uiMode;
	pstSendMsg->msg[pstSendMsg->wp].iError = iError;
	if(uiData != NULL) {
		(void)(*stOsal.osmemcpy)((void*)pstSendMsg->msg[pstSendMsg->wp].uiData, uiData, dataLen);
	}
	else {
		(void)(*stOsal.osmemset)((void*)pstSendMsg->msg[pstSendMsg->wp].uiData, (S8)0, dataLen);
	}

	if(pData != NULL) {
		(void)(*stOsal.osmemcpy)((void*)pstSendMsg->msg[pstSendMsg->wp].pData, pData, pDataLen);
	}
	else {
		(void)(*stOsal.osmemset)((void*)pstSendMsg->msg[pstSendMsg->wp].pData, (S8)0, pDataLen);
	}

	if(pstSendMsg->wp < (TCHDR_MSGQ_SIZE-1U)) {
		pstSendMsg->wp++;
	}
	else {
		pstSendMsg->wp = 0;
	}
}

static void tchdrmsg_getWithoutMutex(stTcHdrMsgQueue_t *pstSendMsg, stTcHdrMsgBuf_t *pstRsvMsgBuf)
{
	pstRsvMsgBuf->fNewMsg = (U32)eTCHDR_NEW_MSG_NULL;
	if(pstSendMsg->wp != pstSendMsg->rp) {
		(void)(*stOsal.osmemcpy)((void*)pstRsvMsgBuf, (void*)(&pstSendMsg->msg[pstSendMsg->rp]), (U32)sizeof(stTcHdrMsgBuf_t));
		pstSendMsg->msg[pstSendMsg->rp].fNewMsg = (U32)eTCHDR_NEW_MSG_NULL;
		pstSendMsg->rp++;
		if(pstSendMsg->rp >= TCHDR_MSGQ_SIZE) {
			pstSendMsg->rp = 0;
		}
		if(pstRsvMsgBuf->uiSender >= (U32)eTCHDR_SENDER_ID_MAX) {
			pstRsvMsgBuf->fNewMsg = (U32)eTCHDR_NEW_MSG_NULL;
			(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "The sender[%d] of the received message is out of range.\n", pstRsvMsgBuf->uiSender);
		}
	}
}

/////////////////////////////// Telechips HD Radio Service ///////////////////////////////////////
HDRET tchdrsvc_mutexLock(void)
{
	return (*stOsal.mutexlock)(&gpTcHdrServiceMsgMutex);
}

void tchdrsvc_mutexUnlock(void)
{
	(*stOsal.mutexunlock)(&gpTcHdrServiceMsgMutex);
}

HDRET tchdrsvc_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError)
{
	HDRET ret;

	ret = (*stOsal.mutexlock)(&gpTcHdrServiceMsgMutex);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		tchdrmsg_sendWithoutMutex(&pstTcHdrServiceMsg, eSenderID, uiMode, uiData, pData, iError);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
	}
	(*stOsal.mutexunlock)(&gpTcHdrServiceMsgMutex);

	return ret;
}

HDRET tchdrsvc_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(pstRsvMsgBuf == NULL) {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = (*stOsal.mutexlock)(&gpTcHdrServiceMsgMutex);
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			tchdrmsg_getWithoutMutex(&pstTcHdrServiceMsg, pstRsvMsgBuf);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
		}
		(*stOsal.mutexunlock)(&gpTcHdrServiceMsgMutex);
	}

	return ret;
}

/////////////////////////////// Telechips HD Radio Baseband Input ///////////////////////////////////////
HDRET tchdrbbinput_mutexLock(void)
{
	return (*stOsal.mutexlock)(&gpTcHdrBbInputMsgMutex);
}

void tchdrbbinput_mutexUnlock(void)
{
	(*stOsal.mutexunlock)(&gpTcHdrBbInputMsgMutex);
}

HDRET tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError)
{
	HDRET ret;

	ret = (*stOsal.mutexlock)(&gpTcHdrBbInputMsgMutex);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		tchdrmsg_sendWithoutMutex(&pstTcHdrBbInputMsg, eSenderID, uiMode, uiData, pData, iError);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
	}
	(*stOsal.mutexunlock)(&gpTcHdrBbInputMsgMutex);

	return ret;
}

HDRET tchdrbbinput_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(pstRsvMsgBuf == NULL) {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = (*stOsal.mutexlock)(&gpTcHdrBbInputMsgMutex);
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			tchdrmsg_getWithoutMutex(&pstTcHdrBbInputMsg, pstRsvMsgBuf);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
		}
		(*stOsal.mutexunlock)(&gpTcHdrBbInputMsgMutex);
	}

	return ret;
}

/////////////////////////////// Telechips HD Radio IQ Input  ///////////////////////////////////////
HDRET tchdriqinput_mutexLock(void)
{
	return (*stOsal.mutexlock)(&gpTcHdrIqInputMutex);
}

void tchdriqinput_mutexUnlock(void)
{
	(*stOsal.mutexunlock)(&gpTcHdrIqInputMutex);
}

/////////////////////////////// Telechips HD Radio Audio Input  ///////////////////////////////////////
HDRET tchdraudinput_mutexLock(void)
{
	return (*stOsal.mutexlock)(&gpTcHdrAudInputMutex);
}

void tchdraudinput_mutexUnlock(void)
{
	(*stOsal.mutexunlock)(&gpTcHdrAudInputMutex);
}

/////////////////////////////// Telechips HD Radio Application ///////////////////////////////////////
HDRET tchdrapp_mutexLock(void)
{
	return (*stOsal.mutexlock)(&gpTcHdrAppMsgMutex);
}

void tchdrapp_mutexUnlock(void)
{
	(*stOsal.mutexunlock)(&gpTcHdrAppMsgMutex);
}

HDRET tchdrapp_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError)
{
	HDRET ret;

	ret = (*stOsal.mutexlock)(&gpTcHdrAppMsgMutex);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		tchdrmsg_sendWithoutMutex(&pstTcHdrAppMsg, eSenderID, uiMode, uiData, pData, iError);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
	}
	(*stOsal.mutexunlock)(&gpTcHdrAppMsgMutex);

	return ret;
}

HDRET tchdrapp_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(pstRsvMsgBuf == NULL) {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = (*stOsal.mutexlock)(&gpTcHdrAppMsgMutex);
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			tchdrmsg_getWithoutMutex(&pstTcHdrAppMsg, pstRsvMsgBuf);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
		}
		(*stOsal.mutexunlock)(&gpTcHdrAppMsgMutex);
	}

	return ret;
}

/////////////////////////////// Telechips HD Radio Primary Instance ///////////////////////////////////////
HDRET tchdrmain_mutexLock(void)
{
	return (*stOsal.mutexlock)(&gpTcHdrMainMsgMutex);
}

void tchdrmain_mutexUnlock(void)
{
	(*stOsal.mutexunlock)(&gpTcHdrMainMsgMutex);
}

HDRET tchdrmain_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError)
{
	HDRET ret;

	ret = (*stOsal.mutexlock)(&gpTcHdrMainMsgMutex);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		tchdrmsg_sendWithoutMutex(&pstTcHdrMainMsg, eSenderID, uiMode, uiData, pData, iError);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
	}
	(*stOsal.mutexunlock)(&gpTcHdrMainMsgMutex);

	return ret;
}

HDRET tchdrmain_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(pstRsvMsgBuf == NULL) {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = (*stOsal.mutexlock)(&gpTcHdrMainMsgMutex);
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			tchdrmsg_getWithoutMutex(&pstTcHdrMainMsg, pstRsvMsgBuf);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
		}
		(*stOsal.mutexunlock)(&gpTcHdrMainMsgMutex);
	}

	return ret;
}

/////////////////////////////// Telechips HD Radio Secondary Instance ///////////////////////////////////////
HDRET tchdrmrc_mutexLock(void)
{
	return (*stOsal.mutexlock)(&gpTcHdrMrcMsgMutex);
}

void tchdrmrc_mutexUnlock(void)
{
	(*stOsal.mutexunlock)(&gpTcHdrMrcMsgMutex);
}

HDRET tchdrmrc_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError)
{
	HDRET ret;

	ret = (*stOsal.mutexlock)(&gpTcHdrMrcMsgMutex);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		tchdrmsg_sendWithoutMutex(&pstTcHdrMrcMsg, eSenderID, uiMode, uiData, pData, iError);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
	}
	(*stOsal.mutexunlock)(&gpTcHdrMrcMsgMutex);

	return ret;
}

HDRET tchdrmrc_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(pstRsvMsgBuf == NULL) {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = (*stOsal.mutexlock)(&gpTcHdrMrcMsgMutex);
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			tchdrmsg_getWithoutMutex(&pstTcHdrMrcMsg, pstRsvMsgBuf);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
		}
		(*stOsal.mutexunlock)(&gpTcHdrMrcMsgMutex);
	}

	return ret;
}

/////////////////////////////// Telechips HD Radio Tertiary Instance ///////////////////////////////////////
HDRET tchdrbs_mutexLock(void)
{
	return (*stOsal.mutexlock)(&gpTcHdrBsMsgMutex);
}

void tchdrbs_mutexUnlock(void)
{
	(*stOsal.mutexunlock)(&gpTcHdrBsMsgMutex);
}

HDRET tchdrbs_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError)
{
	HDRET ret;

	ret = (*stOsal.mutexlock)(&gpTcHdrBsMsgMutex);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		tchdrmsg_sendWithoutMutex(&pstTcHdrBsMsg, eSenderID, uiMode, uiData, pData, iError);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
	}
	(*stOsal.mutexunlock)(&gpTcHdrBsMsgMutex);

	return ret;
}

HDRET tchdrbs_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(pstRsvMsgBuf == NULL) {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = (*stOsal.mutexlock)(&gpTcHdrBsMsgMutex);
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			tchdrmsg_getWithoutMutex(&pstTcHdrBsMsg, pstRsvMsgBuf);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
		}
		(*stOsal.mutexunlock)(&gpTcHdrBsMsgMutex);
	}

	return ret;
}

/////////////////////////////// Telechips HD Radio Quaternary Instance ///////////////////////////////////////
HDRET tchdrbsmrc_mutexLock(void)
{
	return (*stOsal.mutexlock)(&gpTcHdrBsMrcMsgMutex);
}

void tchdrbsmrc_mutexUnlock(void)
{
	(*stOsal.mutexunlock)(&gpTcHdrBsMrcMsgMutex);
}

HDRET tchdrbsmrc_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError)
{
	HDRET ret;

	ret = (*stOsal.mutexlock)(&gpTcHdrBsMrcMsgMutex);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		tchdrmsg_sendWithoutMutex(&pstTcHdrBsMrcMsg, eSenderID, uiMode, uiData, pData, iError);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
	}
	(*stOsal.mutexunlock)(&gpTcHdrBsMrcMsgMutex);

	return ret;
}

HDRET tchdrbsmrc_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(pstRsvMsgBuf == NULL) {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = (*stOsal.mutexlock)(&gpTcHdrBsMrcMsgMutex);
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			tchdrmsg_getWithoutMutex(&pstTcHdrBsMrcMsg, pstRsvMsgBuf);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
		}
		(*stOsal.mutexunlock)(&gpTcHdrBsMrcMsgMutex);
	}

	return ret;
}

/////////////////////////////// Telechips HD Radio Audio ///////////////////////////////////////
HDRET tchdraudoutput_mutexLock(void)
{
	return (*stOsal.mutexlock)(&gpTcHdrAudioMsgMutex);
}

void tchdraudoutput_mutexUnlock(void)
{
	(*stOsal.mutexunlock)(&gpTcHdrAudioMsgMutex);
}

HDRET tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError)
{
	HDRET ret;

	ret = (*stOsal.mutexlock)(&gpTcHdrAudioMsgMutex);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		tchdrmsg_sendWithoutMutex(&pstTcHdrAudioMsg, eSenderID, uiMode, uiData, pData, iError);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
	}
	(*stOsal.mutexunlock)(&gpTcHdrAudioMsgMutex);

	return ret;
}

HDRET tchdraudoutput_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(pstRsvMsgBuf == NULL) {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = (*stOsal.mutexlock)(&gpTcHdrAudioMsgMutex);
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			tchdrmsg_getWithoutMutex(&pstTcHdrAudioMsg, pstRsvMsgBuf);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
		}
		(*stOsal.mutexunlock)(&gpTcHdrAudioMsgMutex);
	}

	return ret;
}

/////////////////////////////// Telechips HD Radio Output ///////////////////////////////////////
HDRET tchdrblending_mutexLock(void)
{
	return (*stOsal.mutexlock)(&gpTcHdrBlendingMsgMutex);
}

void tchdrblending_mutexUnlock(void)
{
	(*stOsal.mutexunlock)(&gpTcHdrBlendingMsgMutex);
}

HDRET tchdrblending_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError)
{
	HDRET ret;

	ret = (*stOsal.mutexlock)(&gpTcHdrBlendingMsgMutex);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		tchdrmsg_sendWithoutMutex(&pstTcHdrBlendingMsg, eSenderID, uiMode, uiData, pData, iError);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
	}
	(*stOsal.mutexunlock)(&gpTcHdrBlendingMsgMutex);

	return ret;
}

HDRET tchdrblending_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(pstRsvMsgBuf == NULL) {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = (*stOsal.mutexlock)(&gpTcHdrBlendingMsgMutex);
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			tchdrmsg_getWithoutMutex(&pstTcHdrBlendingMsg, pstRsvMsgBuf);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_MUTEX_LOCK;
		}
		(*stOsal.mutexunlock)(&gpTcHdrBlendingMsgMutex);
	}

	return ret;
}

