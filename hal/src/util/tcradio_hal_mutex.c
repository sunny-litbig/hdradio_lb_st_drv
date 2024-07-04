/*******************************************************************************

*   FileName : tcradio_hal_mutex.c

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
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "tcradio_types.h"
#include "tcradio_peri_config.h"
#include "tcradio_hal.h"
#include "tcradio_hal_mutex.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
static pthread_mutex_t gpRadioDriverMsgQSema = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t gpRadioAudioFifoSema = PTHREAD_MUTEX_INITIALIZER;

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
static RET rhal_mutexInit(pthread_mutex_t *mutex)
{
	RET ret = eRET_OK;
	int32 lret;
	lret = pthread_mutex_init(mutex, NULL);
	if(lret) {
		RHAL_ERR("[%s:%d] %s\n", __func__, __LINE__, strerror(lret));
		ret = eRET_NG_MUTEX_INIT;
	}
	return ret;
}

static RET rhal_mutexDeinit(pthread_mutex_t *mutex)
{
	RET ret = eRET_OK;
	int32 lret;
	pthread_mutex_lock(mutex);
	pthread_mutex_unlock(mutex);
	lret = pthread_mutex_destroy (mutex);
	if(lret) {
		RHAL_ERR("[%s:%d] %s\n", __func__, __LINE__, strerror(lret));
		ret = eRET_NG_MUTEX_DEINIT;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*	Radio Message Queue API																					*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
RET tcradiohal_mutexInit(void)
{
	RET ret = eRET_OK;

	ret = rhal_mutexInit(&gpRadioDriverMsgQSema);
	if(ret != eRET_OK) {
		RHAL_ERR("[%s:%d] Can't create radio hal mutex member!!! mutex fail!\n", __func__, __LINE__);
		return ret;
	}

	return ret;
}

RET tcradiohal_mutexDeinit(void)
{
	RET ret = eRET_OK;
	ret = rhal_mutexDeinit(&gpRadioDriverMsgQSema);
	if(ret != eRET_OK) {
		RHAL_ERR("[%s:%d] Can't deinit radio hal mutex member!!! mutex fail!\n", __func__, __LINE__);
		return ret;
	}
	return ret;
}

RET tcradiohal_audiofifo_mutexInit(void)
{
	RET ret = eRET_OK;

	ret = rhal_mutexInit(&gpRadioAudioFifoSema);
	if(ret != eRET_OK) {
		RHAL_ERR("[%s:%d] Can't create radio sound fifo hal mutex member!!! mutex fail!\n", __func__, __LINE__);
		return ret;
	}

	return ret;
}

RET tcradiohal_audiofifo_mutexDeinit(void)
{
	RET ret = eRET_OK;
	ret = rhal_mutexDeinit(&gpRadioAudioFifoSema);
	if(ret != eRET_OK) {
		RHAL_ERR("[%s:%d] Can't deinit radio sound fifo hal mutex member!!! mutex fail!\n", __func__, __LINE__);
		return ret;
	}
	return ret;
}

/////////////////////////////// Radio HAL ///////////////////////////////////////
void tcradiohal_mutexLock(void)
{
	pthread_mutex_lock(&gpRadioDriverMsgQSema);
}

void tcradiohal_mutexUnlock(void)
{
	pthread_mutex_unlock(&gpRadioDriverMsgQSema);
}

void tcradiohal_audiofifo_mutexLock(void)
{
	pthread_mutex_lock(&gpRadioAudioFifoSema);
}

void tcradiohal_audiofifo_mutexUnlock(void)
{
	pthread_mutex_unlock(&gpRadioAudioFifoSema);
}
