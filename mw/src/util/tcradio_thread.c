/*******************************************************************************

*   FileName : tcradio_thread.c

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
#include <pthread.h>
#include <sched.h>
#include "tcradio_api.h"
#include "tcradio_service.h"
#include "tcradio_thread.h"

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

/***************************************************
*           Local constant definitions             *
****************************************************/

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition			           *
****************************************************/
RET tcradio_createThread(void *pHandle, void*(*pFunc)(void *arg), const char *pName, eRADIO_THREAD_SCHPOLICY_t schedtype, int32 priority, void *pArg)
{
	RET ret;
	int iSchedType, iMinPriority, iMaxPriority, i;
	struct sched_param param;
	pthread_attr_t attr;

	ret = pthread_attr_init(&attr);
	if(ret != 0) {
		RSRV_ERR("[%s:%d] pthread_attr_init failed!!! error : %s\n", __func__, __LINE__, strerror(ret));
		goto error_thread;
	}

#if 0
	ret = pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);
	if(ret != 0) {
		RSRV_ERR("[%s:%d] pthread_attr_setinheritsched failed!!! error : %s\n", __func__, __LINE__, strerror(ret));
		goto error_thread;
	}

	ret = pthread_attr_setscope(&attr,PTHREAD_SCOPE_SYSTEM);
	if(ret != 0) {
		RSRV_ERR("[%s:%d] pthread_attr_setscope failed!!! error : %s\n", __func__, __LINE__, strerror(ret));
		goto error_thread;
	}
#endif

	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(ret != 0) {
		RSRV_ERR("[%s:%d] pthread_attr_setdetachstate failed!!! error : %s\n", __func__, __LINE__, strerror(ret));
		goto error_thread;
	}

#if 1
	if(schedtype >= eRADIO_SCHED_OTHER && schedtype <= eRADIO_SCHED_FIFO) {
		if(schedtype == eRADIO_SCHED_RR) {
			iSchedType = SCHED_RR;
		}
		else if(schedtype == eRADIO_SCHED_FIFO) {
			iSchedType = SCHED_FIFO;
		}
		else {
			iSchedType = SCHED_OTHER;
		}

		ret = pthread_attr_setschedpolicy(&attr, iSchedType);
		if(ret != 0) {
			RSRV_ERR("[%s:%d] pthread_attr_setschedpolicy failed!!! error : %s\n", __func__, __LINE__, strerror(ret));
			goto error_thread;
		}

		ret = pthread_attr_getschedparam (&attr, &param);
		if(ret != 0) {
			RSRV_ERR("[%s:%d] pthread_attr_getschedparam failed!!! error : %s\n", __func__, __LINE__, strerror(ret));
			goto error_thread;
		}

		iMinPriority = sched_get_priority_min(iSchedType);
		iMaxPriority = sched_get_priority_max(iSchedType);
//		RSRV_DBG("[%s:%d] get pthread schedule parameter priority Min.[%d] Max.[%d]\n", __func__, __LINE__, iMinPriority, iMaxPriority);
		if(iMinPriority > priority || iMaxPriority < priority) {
			param.sched_priority = iMaxPriority;
		}
		else {
			param.sched_priority = priority;
		}

		ret = pthread_attr_setschedparam (&attr, &param);
		if(ret != 0) {
			RSRV_ERR("[%s:%d] pthread pthread schedule and priority setting failed!!! error : %s\n", __func__, __LINE__, strerror(ret));
			goto error_thread;
		}
	}
#endif

	ret = pthread_create(pHandle, &attr, pFunc,  pArg);
	if(ret != 0) {
		RSRV_ERR("[%s:%d] pthread_create failed!!! error : %s\n", __func__, __LINE__, strerror(ret));
		goto error_thread;
	}

	ret = pthread_attr_destroy(&attr);
	if(ret != 0) {
		RSRV_ERR("[%s:%d] pthread_attr_destroy failed!!! error : %s\n", __func__, __LINE__, strerror(ret));
		goto error_thread;
	}

/*
	i = strlen(pName);
	if (i > 0) {
		char *pcName = pName;
		if (i > 15) {
			*(pcName+15)= '\0';
		}
		ret = pthread_setname_np (*(pthread_t *)pHandle, pcName);
	}
	if(ret != 0) {
		RSRV_ERR("[%s:%d] pthread_setname_np failed!!! error : %s\n", __func__, __LINE__, strerror(ret));
	}
*/
#if 0 //defined(_POSIX_PRIORITY_SCHEDULING)
	if(schedtype > eRADIO_SCHED_NULL && schedtype < eRADIO_SCHED_MAX) {
		ret = pthread_getschedparam (pthread_self(), &iSchedType, &param);
		if(ret != 0) {
			RSRV_ERR("[%s:%d] pthread_getschedparam failed!!! error : %s\n", __func__, __LINE__, strerror(ret));
			goto error_thread;
		}
		else {
			RSRV_DBG("[%s:%d] pthread_getschedparam - sched_type[%d]\n", __func__, __LINE__, iSchedType);
		}

		if(schedtype == eRADIO_SCHED_RR) {
			iSchedType = SCHED_RR;
			RSRV_ERR("[%s:%d] set iSchedType=SCHED_RR[%d] error : %s\n", __func__, __LINE__, iSchedType, strerror(ret));
		}
		else if(schedtype == eRADIO_SCHED_FIFO) {
			iSchedType = SCHED_FIFO;
			RSRV_ERR("[%s:%d] set iSchedType=SCHED_FIFO[%d] error : %s\n", __func__, __LINE__, iSchedType, strerror(ret));
		}
		else {
			iSchedType = SCHED_OTHER;
			RSRV_ERR("[%s:%d] set iSchedType=SCHED_OTHER[%d] error : %s\n", __func__, __LINE__, iSchedType, strerror(ret));
		}

		iMinPriority = sched_get_priority_min(iSchedType);
		iMaxPriority = sched_get_priority_max(iSchedType);
		RSRV_DBG("[%s:%d] get pthread schedule parameter priority Min.[%d] Max.[%d]\n", __func__, __LINE__, iMinPriority, iMaxPriority);
		if(iMinPriority > priority || iMaxPriority < priority) {
			param.sched_priority = iMaxPriority;
		}
		else {
			param.sched_priority = priority;
		}

		ret = pthread_setschedparam (pthread_self(), iSchedType, &param);
		if(ret != 0) {
			RSRV_ERR("[%s:%d] pthread pthread schedule and priority setting failed!!! error : %s\n", __func__, __LINE__, strerror(ret));
			goto error_thread;
		}
	}
#endif

error_thread:

	return ret;
}

RET tcradio_joinThread(void *pHandle, void **pThreadRet)
{
	RET ret = 0;
#if 0	// Because all thread functions are detached type, pthread_join is not used.
	ret = pthread_join(pHandle, pThreadRet);
#endif
	return ret;
}

void tcradio_exitThread(void *pHandle)
{
#if 0	// Because all thread functions are detached type, pthread_exit is not used.
	pthread_exit(pHandle);
#endif
}
