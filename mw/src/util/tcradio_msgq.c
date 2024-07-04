/*******************************************************************************

*   FileName : tcradio_msgq.c

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
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "tcradio_api.h"
#include "tcradio_utils.h"
#include "tcradio_memory.h"
#include "tcradio_msgq.h"
#include "tcradio_service.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
pthread_mutex_t gpRadioAppMsgQSema = PTHREAD_MUTEX_INITIALIZER;
stMsgQueue_t *pstRadioAppMsgQ;

pthread_mutex_t gpRadioServiceMsgQSema = PTHREAD_MUTEX_INITIALIZER;
stMsgQueue_t *pstRadioServiceMsgQ;

pthread_mutex_t gpRadioDataSystemMsgQSema = PTHREAD_MUTEX_INITIALIZER;
stMsgQueue_t *pstRadioDataSystemMsgQ;

pthread_mutex_t gpRadioSoundMsgQSema = PTHREAD_MUTEX_INITIALIZER;
stMsgQueue_t *pstRadioSoundMsgQ;

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
static RET tcradio_mutexInit(pthread_mutex_t *mutex)
{
	RET ret = eRET_OK;
	int32 lret;
	lret = pthread_mutex_init(mutex, NULL);
	if(lret) {
		RSRV_ERR("[%s:%d] %s\n", __func__, __LINE__, strerror(lret));
		ret = eRET_NG_MUTEX_INIT;
	}
	return ret;
}

static RET tcradio_mutexDeinit (pthread_mutex_t *mutex)
{
	RET ret = eRET_OK;
	int32 lret;
	pthread_mutex_lock(mutex);
	pthread_mutex_unlock(mutex);
	lret = pthread_mutex_destroy (mutex);
	if(lret) {
		RSRV_ERR("[%s:%d] %s\n", __func__, __LINE__, strerror(lret));
		ret = eRET_NG_MUTEX_DEINIT;
	}
	return ret;
}

static void tcradio_mutexLock(pthread_mutex_t *mutex)
{
	pthread_mutex_lock(mutex);
}

static void tcradio_mutexUnlock(pthread_mutex_t *mutex)
{
	pthread_mutex_unlock(mutex);
}

/////////////////////////////// Radio Service ///////////////////////////////////////
static RET tcradioservice_messageQueueInit(void)
{
	RET ret = eRET_OK;

	pstRadioServiceMsgQ = (stMsgQueue_t *)tcradio_malloc(sizeof(stMsgQueue_t));
	if(pstRadioServiceMsgQ != NULL) {
		tcradio_memset(pstRadioServiceMsgQ, 0x00, sizeof(stMsgQueue_t));
	}
	else {
		RSRV_ERR("[%s:%d] Service message queue malloc fail!\n", __func__, __LINE__);
		ret = eRET_NG_MALLOC;
	}
	return ret;
}

static void tcradioservice_messageQueueDeinit(void)
{
	if(pstRadioServiceMsgQ != NULL) {
		tcradio_free(pstRadioServiceMsgQ);
	}
	pstRadioServiceMsgQ = NULL;
}

/////////////////////////////// Radio Application ///////////////////////////////////////
static RET tcradioapp_messageQueueInit(void)
{
	RET ret = eRET_OK;
	pstRadioAppMsgQ = (stMsgQueue_t *)tcradio_malloc(sizeof(stMsgQueue_t));
	if(pstRadioAppMsgQ != NULL) {
		tcradio_memset(pstRadioAppMsgQ, 0x00, sizeof(stMsgQueue_t));
	}
	else {
		RSRV_ERR("[%s:%d] App message queue malloc fail!\n", __func__, __LINE__);
		ret = eRET_NG_MALLOC;
	}
	return ret;
}

static void tcradioapp_messageQueueDeinit(void)
{
	if(pstRadioAppMsgQ != NULL) {
		tcradio_free(pstRadioAppMsgQ);
	}
	pstRadioAppMsgQ = NULL;
}

/////////////////////////////// Radio Sound ///////////////////////////////////////
static RET tcradiodatasystem_messageQueueInit(void)
{
	RET ret = eRET_OK;

	pstRadioDataSystemMsgQ = (stMsgQueue_t *)tcradio_malloc(sizeof(stMsgQueue_t));
	if(pstRadioDataSystemMsgQ != NULL) {
		tcradio_memset(pstRadioDataSystemMsgQ, 0x00, sizeof(stMsgQueue_t));
	}
	else {
		RSRV_ERR("[%s:%d] RDS message queue malloc fail!\n", __func__, __LINE__);
		ret = eRET_NG_MALLOC;
	}
	return ret;
}

static void tcradiodatasystem_messageQueueDeinit(void)
{
	if(pstRadioDataSystemMsgQ != NULL) {
		tcradio_free(pstRadioDataSystemMsgQ);
	}
	pstRadioDataSystemMsgQ = NULL;
}

/////////////////////////////// Radio Sound ///////////////////////////////////////
static RET tcradiosound_messageQueueInit(void)
{
	RET ret = eRET_OK;

	pstRadioSoundMsgQ = (stMsgQueue_t *)tcradio_malloc(sizeof(stMsgQueue_t));
	if(pstRadioSoundMsgQ != NULL) {
		tcradio_memset(pstRadioSoundMsgQ, 0x00, sizeof(stMsgQueue_t));
	}
	else {
		RSRV_ERR("[%s:%d] Service message queue malloc fail!\n", __func__, __LINE__);
		ret = eRET_NG_MALLOC;
	}
	return ret;
}

static void tcradiosound_messageQueueDeinit(void)
{
	if(pstRadioSoundMsgQ != NULL) {
		tcradio_free(pstRadioSoundMsgQ);
	}
	pstRadioSoundMsgQ = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*	Radio Message Queue API																					*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
RET tcradioservice_messageInit(void)
{
	RET ret = eRET_OK;

	ret = tcradio_mutexInit(&gpRadioAppMsgQSema);
	if(ret != eRET_OK) {
		RSRV_ERR("[%s:%d] Can't create radio app mutex member!!! mutex fail!\n", __func__, __LINE__);
		return ret;
	}

	ret = tcradio_mutexInit(&gpRadioServiceMsgQSema);
	if(ret != eRET_OK) {
		RSRV_ERR("[%s:%d] Can't create radio service mutex member!!! mutex fail!\n", __func__, __LINE__);
		return ret;
	}

	ret = tcradioservice_messageQueueInit();
	if(ret != eRET_OK) {
		RSRV_ERR("[%s:%d] tcradioservice_messageQueueInit() fail!\n", __func__, __LINE__);
		return ret;
	}

	ret = tcradioapp_messageQueueInit();
	if(ret != eRET_OK) {
		RSRV_ERR("[%s:%d] tcradioapp_messageQueueInit() fail!\n", __func__, __LINE__);
		return ret;
	}

	return ret;
}

RET tcradioservice_messageDeinit(void)
{
	RET ret = eRET_OK;
	tcradio_mutexDeinit(&gpRadioAppMsgQSema);
	tcradio_mutexDeinit(&gpRadioServiceMsgQSema);
	tcradioservice_messageQueueDeinit();
	tcradioapp_messageQueueDeinit();
	return ret;
}

RET tcradiodatasystem_messageInit(void)
{
	RET ret = eRET_OK;

	ret = tcradio_mutexInit(&gpRadioDataSystemMsgQSema);
	if(ret != eRET_OK) {
		RSRV_ERR("[%s:%d] Can't create RDS mutex member!!! mutex fail!\n", __func__, __LINE__);
		return ret;
	}

	ret = tcradiodatasystem_messageQueueInit();
	if(ret != eRET_OK) {
		RSRV_ERR("[%s:%d] tcradiodatasystem_messageQueueInit() fail!\n", __func__, __LINE__);
		return ret;
	}

	return ret;
}

RET tcradiodatasystem_messageDeinit(void)
{
	RET ret = eRET_OK;
	tcradio_mutexDeinit(&gpRadioDataSystemMsgQSema);
	tcradiodatasystem_messageQueueDeinit();
	return ret;
}

RET tcradiosound_messageInit(void)
{
	RET ret = eRET_OK;

	ret = tcradio_mutexInit(&gpRadioSoundMsgQSema);
	if(ret != eRET_OK) {
		RSRV_ERR("[%s:%d] Can't create radio sound mutex member!!! mutex fail!\n", __func__, __LINE__);
		return ret;
	}

	ret = tcradiosound_messageQueueInit();
	if(ret != eRET_OK) {
		RSRV_ERR("[%s:%d] tcradiosound_messageQueueInit() fail!\n", __func__, __LINE__);
		return ret;
	}

	return ret;
}

RET tcradiosound_messageDeinit(void)
{
	RET ret = eRET_OK;
	tcradio_mutexDeinit(&gpRadioSoundMsgQSema);
	tcradiosound_messageQueueDeinit();
	return ret;
}

/////////////////////////////// Radio Service ///////////////////////////////////////
void tcradioservice_mutexLock(void)
{
	pthread_mutex_lock(&gpRadioServiceMsgQSema);
}

void tcradioservice_mutexUnlock(void)
{
	pthread_mutex_unlock(&gpRadioServiceMsgQSema);
}

RET tcradioservice_sendMessage(eSENDER_ID_t eSenderID, uint32 uiMode, uint32 *uiData, void *pData, int32 iError)
{
	RET ret = eRET_OK;
	stMsgQueue_t *pstSendMsg;

	if(pstRadioServiceMsgQ == NULL) {
		return eRET_NG_NOT_SUPPORT;
	}

	tcradioservice_mutexLock();
	pstSendMsg = pstRadioServiceMsgQ;
	pstSendMsg->msg[pstSendMsg->wp].uiSender = (uint32)eSenderID;
	pstSendMsg->msg[pstSendMsg->wp].fNewMsg = (uint32)eNEW_MSG_EXIST;
	pstSendMsg->msg[pstSendMsg->wp].uiMode = uiMode;
	pstSendMsg->msg[pstSendMsg->wp].iError = iError;
	if(uiData != NULL) {
		tcradio_memcpy(pstSendMsg->msg[pstSendMsg->wp].uiData, uiData, MSGQ_DATA_LENGTH*sizeof(uint32));
	}
	else {
		tcradio_memset(pstSendMsg->msg[pstSendMsg->wp].uiData, 0, MSGQ_DATA_LENGTH*sizeof(uint32));
	}

	if(pData != NULL) {
		tcradio_memcpy(pstSendMsg->msg[pstSendMsg->wp].pData, pData, MSGQ_PDATA_LENGTH*sizeof(void*));
	}
	else {
		tcradio_memset(pstSendMsg->msg[pstSendMsg->wp].pData, 0, MSGQ_PDATA_LENGTH*sizeof(void*));
	}

	if(++pstSendMsg->wp >= MSGQ_SIZE) {
		pstSendMsg->wp = 0;
	}
	tcradioservice_mutexUnlock();

	return ret;
}

RET tcradioservice_getMessage(stMsgBuf_t *pstRsvMsgBuf)
{
	RET ret = eRET_OK;
	stMsgQueue_t *pstSendMsg;

	if(pstRsvMsgBuf == NULL) {
		ret = eRET_NG_INVALID_PARAM;
	}

	if(pstRadioServiceMsgQ == NULL) {
		ret = eRET_NG_NOT_SUPPORT;
	}

	if(ret == eRET_OK) {
		tcradioservice_mutexLock();
		pstRsvMsgBuf->fNewMsg = eNEW_MSG_NULL;
		pstSendMsg = pstRadioServiceMsgQ;
		if(pstSendMsg->wp != pstSendMsg->rp) {
			tcradio_memcpy(pstRsvMsgBuf, &pstSendMsg->msg[pstSendMsg->rp], sizeof(stMsgBuf_t));
			pstSendMsg->msg[pstSendMsg->rp].fNewMsg = eNEW_MSG_NULL;
			if(++pstSendMsg->rp >= MSGQ_SIZE) {
				pstSendMsg->rp = 0;
			}
		}
		tcradioservice_mutexUnlock();
	}

	return ret;
}

RET tcradioservice_sendMessageWithoutMutex(eSENDER_ID_t eSenderID, uint32 uiMode, uint32 *uiData, void *pData, int32 iError)
{
	RET ret = eRET_OK;
	stMsgQueue_t *pstSendMsg;

	if(pstRadioServiceMsgQ == NULL) {
		return eRET_NG_NOT_SUPPORT;
	}

	pstSendMsg = pstRadioServiceMsgQ;
	pstSendMsg->msg[pstSendMsg->wp].uiSender = (uint32)eSenderID;
	pstSendMsg->msg[pstSendMsg->wp].fNewMsg = (uint32)eNEW_MSG_EXIST;
	pstSendMsg->msg[pstSendMsg->wp].uiMode = uiMode;
	pstSendMsg->msg[pstSendMsg->wp].iError = iError;
	if(uiData != NULL) {
		tcradio_memcpy(pstSendMsg->msg[pstSendMsg->wp].uiData, uiData, MSGQ_DATA_LENGTH*sizeof(uint32));
	}
	else {
		tcradio_memset(pstSendMsg->msg[pstSendMsg->wp].uiData, 0, MSGQ_DATA_LENGTH*sizeof(uint32));
	}

	if(pData != NULL) {
		tcradio_memcpy(pstSendMsg->msg[pstSendMsg->wp].pData, pData, MSGQ_PDATA_LENGTH*sizeof(void*));
	}
	else {
		tcradio_memset(pstSendMsg->msg[pstSendMsg->wp].pData, 0, MSGQ_PDATA_LENGTH*sizeof(void*));
	}

	if(++pstSendMsg->wp >= MSGQ_SIZE) {
		pstSendMsg->wp = 0;
	}

	return ret;
}
/////////////////////////////// Radio Application ///////////////////////////////////////
void tcradioapp_mutexLock(void)
{
	pthread_mutex_lock(&gpRadioAppMsgQSema);
}

void tcradioapp_mutexUnlock(void)
{
	pthread_mutex_unlock(&gpRadioAppMsgQSema);
}

RET tcradioapp_sendMessage(eSENDER_ID_t eSenderID, uint32 uiMode, uint32 *uiData, void *pData, int32 iError)
{
	RET ret = eRET_OK;
	stMsgQueue_t *pstSendMsg;

	if(pstRadioAppMsgQ == NULL) {
		return eRET_NG_NOT_SUPPORT;
	}

	tcradioapp_mutexLock();
	pstSendMsg = pstRadioAppMsgQ;
	pstSendMsg->msg[pstSendMsg->wp].uiSender = (uint32)eSenderID;
	pstSendMsg->msg[pstSendMsg->wp].fNewMsg = (uint32)eNEW_MSG_EXIST;
	pstSendMsg->msg[pstSendMsg->wp].uiMode = uiMode;
	pstSendMsg->msg[pstSendMsg->wp].iError = iError;
	if(uiData != NULL) {
		tcradio_memcpy(pstSendMsg->msg[pstSendMsg->wp].uiData, uiData, MSGQ_DATA_LENGTH*sizeof(uint32));
	}
	else {
		tcradio_memset(pstSendMsg->msg[pstSendMsg->wp].uiData, 0, MSGQ_DATA_LENGTH*sizeof(uint32));
	}

	if(pData != NULL) {
		tcradio_memcpy(pstSendMsg->msg[pstSendMsg->wp].pData, pData, MSGQ_PDATA_LENGTH*sizeof(void*));
	}
	else {
		tcradio_memset(pstSendMsg->msg[pstSendMsg->wp].pData, 0, MSGQ_PDATA_LENGTH*sizeof(void*));
	}

	if(++pstSendMsg->wp >= MSGQ_SIZE) {
		pstSendMsg->wp = 0;
	}
	tcradioapp_mutexUnlock();

	return ret;
}

RET tcradioapp_getMessage(stMsgBuf_t *pstRsvMsgBuf)
{
	RET ret = eRET_OK;
	stMsgQueue_t *pstSendMsg;

	if(pstRsvMsgBuf == NULL) {
		ret = eRET_NG_INVALID_PARAM;
	}

	if(pstRadioAppMsgQ == NULL) {
		ret = eRET_NG_NOT_SUPPORT;
	}

	if(ret == eRET_OK) {
		tcradioapp_mutexLock();
		pstRsvMsgBuf->fNewMsg = eNEW_MSG_NULL;
		pstSendMsg = pstRadioAppMsgQ;
		if(pstSendMsg->wp != pstSendMsg->rp) {
			tcradio_memcpy(pstRsvMsgBuf, &pstSendMsg->msg[pstSendMsg->rp], sizeof(stMsgBuf_t));
			pstSendMsg->msg[pstSendMsg->rp].fNewMsg = eNEW_MSG_NULL;
			if(++pstSendMsg->rp >= MSGQ_SIZE) {
				pstSendMsg->rp = 0;
			}
		}
		tcradioapp_mutexUnlock();
	}

	return ret;
}

/////////////////////////////// Radio Data System ///////////////////////////////////////
void tcradiodatasystem_mutexLock(void)
{
	pthread_mutex_lock(&gpRadioDataSystemMsgQSema);
}

void tcradiodatasystem_mutexUnlock(void)
{
	pthread_mutex_unlock(&gpRadioDataSystemMsgQSema);
}

RET tcradiodatasystem_sendMessage(eSENDER_ID_t eSenderID, uint32 uiMode, uint32 *uiData, void *pData, int32 iError)
{
	RET ret = eRET_OK;
	stMsgQueue_t *pstSendMsg;

	if(pstRadioDataSystemMsgQ == NULL) {
		return eRET_NG_NOT_SUPPORT;
	}

	tcradiodatasystem_mutexLock();
	pstSendMsg = pstRadioDataSystemMsgQ;
	pstSendMsg->msg[pstSendMsg->wp].uiSender = (uint32)eSenderID;
	pstSendMsg->msg[pstSendMsg->wp].fNewMsg = (uint32)eNEW_MSG_EXIST;
	pstSendMsg->msg[pstSendMsg->wp].uiMode = uiMode;
	pstSendMsg->msg[pstSendMsg->wp].iError = iError;
	if(uiData != NULL) {
		tcradio_memcpy(pstSendMsg->msg[pstSendMsg->wp].uiData, uiData, MSGQ_DATA_LENGTH*sizeof(uint32));
	}
	else {
		tcradio_memset(pstSendMsg->msg[pstSendMsg->wp].uiData, 0, MSGQ_DATA_LENGTH*sizeof(uint32));
	}

	if(pData != NULL) {
		tcradio_memcpy(pstSendMsg->msg[pstSendMsg->wp].pData, pData, MSGQ_PDATA_LENGTH*sizeof(void*));
	}
	else {
		tcradio_memset(pstSendMsg->msg[pstSendMsg->wp].pData, 0, MSGQ_PDATA_LENGTH*sizeof(void*));
	}

	if(++pstSendMsg->wp >= MSGQ_SIZE) {
		pstSendMsg->wp = 0;
	}
	tcradiodatasystem_mutexUnlock();

	return ret;
}

RET tcradiodatasystem_getMessage(stMsgBuf_t *pstRsvMsgBuf)
{
	RET ret = eRET_OK;
	stMsgQueue_t *pstSendMsg;

	if(pstRsvMsgBuf == NULL) {
		ret = eRET_NG_INVALID_PARAM;
	}

	if(pstRadioDataSystemMsgQ == NULL) {
		ret = eRET_NG_NO_MEM;
	}

	if(ret == eRET_OK) {
		tcradiodatasystem_mutexLock();
		pstRsvMsgBuf->fNewMsg = eNEW_MSG_NULL;
		pstSendMsg = pstRadioDataSystemMsgQ;
		if(pstSendMsg->wp != pstSendMsg->rp) {
			tcradio_memcpy(pstRsvMsgBuf, &pstSendMsg->msg[pstSendMsg->rp], sizeof(stMsgBuf_t));
			pstSendMsg->msg[pstSendMsg->rp].fNewMsg = eNEW_MSG_NULL;
			if(++pstSendMsg->rp >= MSGQ_SIZE) {
				pstSendMsg->rp = 0;
			}
		}
		tcradiodatasystem_mutexUnlock();
	}

	return ret;
}

/////////////////////////////// Radio Sound ///////////////////////////////////////
void tcradiosound_mutexLock(void)
{
	pthread_mutex_lock(&gpRadioSoundMsgQSema);
}

void tcradiosound_mutexUnlock(void)
{
	pthread_mutex_unlock(&gpRadioSoundMsgQSema);
}

RET tcradiosound_sendMessage(eSENDER_ID_t eSenderID, uint32 uiMode, uint32 *uiData, void *pData, int32 iError)
{
	RET ret = eRET_OK;
	stMsgQueue_t *pstSendMsg;

	if(pstRadioSoundMsgQ == NULL) {
		return eRET_NG_NOT_SUPPORT;
	}

	tcradiosound_mutexLock();
	pstSendMsg = pstRadioSoundMsgQ;
	pstSendMsg->msg[pstSendMsg->wp].uiSender = (uint32)eSenderID;
	pstSendMsg->msg[pstSendMsg->wp].fNewMsg = (uint32)eNEW_MSG_EXIST;
	pstSendMsg->msg[pstSendMsg->wp].uiMode = uiMode;
	pstSendMsg->msg[pstSendMsg->wp].iError = iError;
	if(uiData != NULL) {
		tcradio_memcpy(pstSendMsg->msg[pstSendMsg->wp].uiData, uiData, MSGQ_DATA_LENGTH*sizeof(uint32));
	}
	else {
		tcradio_memset(pstSendMsg->msg[pstSendMsg->wp].uiData, 0, MSGQ_DATA_LENGTH*sizeof(uint32));
	}

	if(pData != NULL) {
		tcradio_memcpy(pstSendMsg->msg[pstSendMsg->wp].pData, pData, MSGQ_PDATA_LENGTH*sizeof(void*));
	}
	else {
		tcradio_memset(pstSendMsg->msg[pstSendMsg->wp].pData, 0, MSGQ_PDATA_LENGTH*sizeof(void*));
	}

	if(++pstSendMsg->wp >= MSGQ_SIZE) {
		pstSendMsg->wp = 0;
	}
	tcradiosound_mutexUnlock();

	return ret;
}

RET tcradiosound_getMessage(stMsgBuf_t *pstRsvMsgBuf)
{
	RET ret = eRET_OK;
	stMsgQueue_t *pstSendMsg;

	if(pstRsvMsgBuf == NULL) {
		ret = eRET_NG_INVALID_PARAM;
	}

	if(pstRadioSoundMsgQ == NULL) {
		ret = eRET_NG_NOT_SUPPORT;
	}

	if(ret == eRET_OK) {
		tcradiosound_mutexLock();
		pstRsvMsgBuf->fNewMsg = eNEW_MSG_NULL;
		pstSendMsg = pstRadioSoundMsgQ;
		if(pstSendMsg->wp != pstSendMsg->rp) {
			tcradio_memcpy(pstRsvMsgBuf, &pstSendMsg->msg[pstSendMsg->rp], sizeof(stMsgBuf_t));
			pstSendMsg->msg[pstSendMsg->rp].fNewMsg = eNEW_MSG_NULL;
			if(++pstSendMsg->rp >= MSGQ_SIZE) {
				pstSendMsg->rp = 0;
			}
		}
		tcradiosound_mutexUnlock();
	}

	return ret;
}

