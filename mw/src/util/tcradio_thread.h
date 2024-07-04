/*******************************************************************************

*   FileName : tcradio_thread.h

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
#ifndef __TCRADIO_THREAD_H__
#define __TCRADIO_THREAD_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
*				Enumeration				*
****************************************************/
/** Enumerated Thread Scheduling Policy */
typedef enum {
	eRADIO_SCHED_OTHER	= 1,	/**Regular, non-real-time scheduling*/
	eRADIO_SCHED_RR		= 2,	/**Real-time, round-robin*/
	eRADIO_SCHED_FIFO	= 3		/**Real-time, first-in first-out*/
} eRADIO_THREAD_SCHPOLICY_t;

/***************************************************
*				Typedefs					*
****************************************************/

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
RET tcradio_createThread(void *pHandle, void*(*pFunc)(void *arg), const char *pName, eRADIO_THREAD_SCHPOLICY_t schedtype, int32 priority, void *pArg);
RET tcradio_joinThread(void *pHandle, void **pThreadRet);
void tcradio_exitThread(void *pHandle);

#ifdef __cplusplus
}
#endif

#endif
