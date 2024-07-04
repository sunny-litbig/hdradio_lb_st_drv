/*******************************************************************************

*   FileName : tchdr_std.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio standard support functions

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
*		Include 			   *
****************************************************/
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#include "tchdr_common.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
stTCHDR_OSAPI_t stOsal = {
	&tchdr_memset,
	&tchdr_memcpy,
	&tchdr_malloc,
	&tchdr_calloc,
	&tchdr_strcpy,
	&tchdr_strncpy,
	&tchdr_strlen,
	&tchdr_strcmp,
	&tchdr_strncmp,
	&tchdr_strcat,
	&tchdr_mutexInit,
	&tchdr_mutexDeinit,
	&tchdr_mutexLock,
	&tchdr_mutexUnlock,
	&tchdr_free,
	&tchdr_createThread,
	&tchdr_setPostThreadAttr
};

stTCHDR_CAST_t stCast = {
	&tchdr_booltou8,
	&tchdr_booltou32,
	&tchdr_s16tof32,
	&tchdr_u32tos32,
	&tchdr_s32tou32,
	&tchdr_u32tou16,
	&tchdr_u32tou8,
	&tchdr_u32tos16,
	&tchdr_f32tos32,
	&tchdr_f32tou32,
	&tchdr_f64tos32,
	&tchdr_f64tof32,
	&tchdr_u32tof32,
	&tchdr_s32tof32,
	&tchdr_slongtoulong,
	&tchdr_slongtou32,
	&tchdr_u32toslong
};

stTCHDR_ARITHMETIC_t stArith = {
	&tchdr_s16mul,
	&tchdr_slongadd,
	&tchdr_s32add,
	&tchdr_s32sub,
	&tchdr_s32mul,
	&tchdr_s64sub,
	&tchdr_ulongadd,
	&tchdr_u08add,
	&tchdr_u16add,
	&tchdr_u32add,
	&tchdr_u32sub,
	&tchdr_u32mul,
	&tchdr_ulongmul
};

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/

/***************************************************
*           Local preprocessor                     *
****************************************************/
#define S64_MAX	(9223372036854775807LL)
#define S64_MIN	(-S64_MAX-1LL)
#define S32_MAX	(2147483647)
#define S32_MIN	(-2147483648)
#define F32_MAX	(2147483647.0f)
#define F32_MIN	(-2147483648.0f)
#define S16_MAX	(32767)
#define S16_MIN	(-32768)
#define S08_MAX (127)
#define S08_MIN (-128)

#define U64_MAX (18446744073709551615ULL)
#define U32_MAX	(4294967295U)
#define U16_MAX	(65535U)
#define U08_MAX	(255U)

#ifdef HDR_64BIT_SYSTEM
#define L_MAX	(9223372036854775807L)
#define L_MIN	(-L_MAX-1L)
#define UL_MAX	(0xFFFFFFFFFFFFFFFFUL)
#else
#define L_MAX	(0x7FFFFFFFL)
#define L_MIN	(-L_MAX-1L)
#define UL_MAX	(0xFFFFFFFFUL)
#endif

/***************************************************
*          Local type definitions                  *
****************************************************/

/***************************************************
*           Local constant definitions              *
****************************************************/
#ifdef	TCC_MEMORY_DEBUG
static S32 gMemallocCnt = 0;
static S32 gMemRemain = 0;
#endif

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition				*
****************************************************/
//////////////////////////////
/* 			MEMORY			*/
//////////////////////////////
S8 *tchdr_malloc(size_t iSize)
{
	S8 *ptr;
	ptr = (S8*)malloc(iSize);

	if(ptr == NULL) {
		ptr = pNULL;
	}

	return ptr;
}

S8 *tchdr_calloc(size_t iSize, size_t nmemb)
{
	S8 *ptr;
	ptr = (S8*)calloc(iSize, nmemb);

	if(ptr == NULL) {
		ptr = pNULL;
	}

	return ptr;
}

void tchdr_free(void *ptr)
{
	void *psrc = ptr;

	if(psrc != NULL) {
		free(psrc);
		psrc = NULL;
	}
}

//////////////////////////////////////
/* 	     Standard Libraries         */
//////////////////////////////////////
void tchdr_16bto8b(U8 *buf, U16 data)
{
	U16 indata = data;

	if(buf != NULL) {
		buf[0] = (U8)(indata & 0x0ffU);
		buf[1] = (U8)((indata >> 8U) & 0x0ffU);
	}
}

void tchdr_32bto8b(U8 *buf, U32 data)
{
	U32 indata = data;

	if(buf != NULL) {
		buf[0] = (U8)(indata & 0x0ffU);
		buf[1] = (U8)((indata >> 8U) & 0x0ffU);
		buf[2] = (U8)((indata >> 16U) & 0x0ffU);
		buf[3] = (U8)((indata >> 24U) & 0x0ffU);
	}
}

void *tchdr_memset(void *pdst, S8 ch, U32 len)
{
	S8 *pW;
	U32 i;

	pW = (S8 *)pdst;

	if((len != 0U) && (pW != NULL)) {
		for(i = 0U; i<len; i++)
		{
			*pW = ch;
			pW++;
		}
	}
	return (void *)pW;
}

void *tchdr_memcpy(void *pdst, const void *psrc, U32 len)
{
	U32 i;
	S8 *pW = (S8 *)pdst;
	const S8 *pR = psrc;

	if((len != 0U) && (pW != NULL) && (pR != NULL)) {
		for(i = 0U; i < len; i++)
		{
			*pW = *pR;
			pW++;
			pR++;
		}
	}
	return (void *)pW;
}

S8 *tchdr_strcpy(S8 *pdst, const S8 *psrc)
{
	S8 *pD;
	const S8 *pS = psrc;

	pD = pdst;

	if((pD != NULL) && (pS != NULL)) {
		while(*pS != (S8)0)
		{
			*pD = *pS;
			pD++;
			pS++;
		}
		*pD='\0';
	}

	return(pdst);
}

S8 *tchdr_strncpy(S8 *pdst, const S8 *psrc, U32 len)
{
	S8 *pD;
	const S8 *pS = psrc;
	U32 length = len;

	pD = pdst;

	if((pD != NULL) && (pS != NULL)) {
		while(length > 0U)
		{
			*pD = *pS;
			pD++;
			pS++;
			length--;
		}
		*pD='\0';
	}

	return(pdst);
}

U32 tchdr_strlen(const S8 *psbStr)
{
	const S8 *pStr = psbStr;
	U32 len;

	len = 0U;
	if(pStr != NULL) {
		while(*pStr != '\0')
		{
			pStr++;
			len = (*stArith.u32add)(len, 1U);
		}
	}
	return len;
}

S32 tchdr_strcmp(const S8 *psbStr1, const S8 *psbStr2)
{
	S32 result = 0;

	if((psbStr1 != NULL) && (psbStr2 != NULL)) {
		while(*psbStr1 != '\0')
		{
			if(*psbStr1 != *psbStr2){
				result = (*psbStr1) - (*psbStr2);
				break;
			}
			psbStr1++;
			psbStr2++;
		}
	}

	return result;
}

S32 tchdr_strncmp(const S8 *psbStr1, const S8 *psbStr2, U32 len)
{
	U32 length = len;
	S32 result = 0;

	if((psbStr1 != NULL) && (psbStr2 != NULL)) {
		while(length > 0U)
		{
			if(*psbStr1 != *psbStr2){
				result = (*psbStr1) - (*psbStr2);
				break;
			}
			psbStr1++;
			psbStr2++;
	        length--;
		}
	}

	return result;
}


S8 *tchdr_strcat(S8 *psbStr1, const S8 *psbStr2)
{
	if((psbStr1 != NULL) && (psbStr2 != NULL)) {
		while(*psbStr1 != (S8)0)
		{
			psbStr1++;
		}
		while(*psbStr2 != '\0')
		{
			*psbStr1 = *psbStr2;
			psbStr1++;
			psbStr2++;
		}
	}

	return psbStr1;
}

//////////////////////////////
/* 			MUTEX			*/
//////////////////////////////
HDRET tchdr_mutexInit(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	HDRET ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_MUTEX;
	HDRET lret;

	if(mutex != NULL) {
		lret = pthread_mutex_init(mutex, attr);
		if(lret != 0) {
			S8 errBuf[HDR_ERR_BUF_SIZE]={0,};
			(void)strerror_r(lret, errBuf, sizeof(errBuf));
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] %s\n", __func__, __LINE__, errBuf);
			ret = (HDRET)eTC_HDR_RET_NG_MUTEX_INIT;
		}
		else {
			ret = (HDRET)eTC_HDR_RET_OK;
		}
	}
	return ret;
}

HDRET tchdr_mutexDeinit (pthread_mutex_t *mutex)
{
	HDRET ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_MUTEX;
	HDRET lret;

	if(mutex != NULL) {
		(void)pthread_mutex_lock(mutex);
		(void)pthread_mutex_unlock(mutex);
	    lret = pthread_mutex_destroy(mutex);
        if(lret != 0) {
			S8 errBuf[HDR_ERR_BUF_SIZE]={0,};
			(void)strerror_r(lret, errBuf, sizeof(errBuf));
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] %s\n", __func__, __LINE__, errBuf);
			ret = (HDRET)eTC_HDR_RET_NG_MUTEX_DEINIT;
		}
		else {
			ret = (HDRET)eTC_HDR_RET_OK;
		}
	}
	return ret;
}

HDRET tchdr_mutexLock(pthread_mutex_t *mutex)
{
	S32 ret;

	if(mutex != NULL) {
		ret = pthread_mutex_lock(mutex);
		if(ret < 0) {
			S8 errBuf[HDR_ERR_BUF_SIZE]={0,};
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]: %s\n", __func__, __LINE__, errBuf);
			ret = (S32)eTC_HDR_RET_NG_MUTEX_LOCK;
		}
		else {
			ret = (S32)eTC_HDR_RET_OK;
		}
	}
	else {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Framework message mutex(lock) is NULL.");
		ret = (S32)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	return ret;
}

void tchdr_mutexUnlock(pthread_mutex_t *mutex)
{
	if(mutex != NULL) {
		(void)pthread_mutex_unlock(mutex);
	}
	else {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Framework message mutex(unlock) is NULL.");
	}
}

//////////////////////////////
/* 			THREAD			*/
//////////////////////////////
S32 tchdr_createThread(FUNC_PTR_T funcPtr, stThread_attr_t *attr, void *parameter)
{
	S32 ret = 0;
    pthread_attr_t thread_attr;
    struct sched_param thread_param;
	S8 errBuf[HDR_ERR_BUF_SIZE]={0,};

    if((funcPtr == NULL) || (attr == NULL)) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]: An error occurred: The arguments is null.\n", __func__, __LINE__);
		ret = -1;
    }

	if(ret == 0){
		ret = pthread_attr_init(&thread_attr);
		if(ret != 0){
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			ret = -1;
		}
    }

	if(ret == 0){
		ret = pthread_attr_setdetachstate(&thread_attr, (S32)PTHREAD_CREATE_DETACHED);
		if(ret != 0){
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			ret = -1;
		}
    }

	if(ret == 0){
		ret = pthread_attr_setschedpolicy(&thread_attr, attr->policy);
		if(ret != 0){
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			ret = -1;
		}
    }

	if((ret == 0) && (attr->policy != SCHED_OTHER)){
		thread_param.sched_priority = attr->priority;
	    ret = pthread_attr_setschedparam(&thread_attr, &thread_param);
		if(ret != 0){
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			ret = -1;
		}
    }

	if(ret == 0){
		ret = pthread_attr_setinheritsched (&thread_attr, (S32)PTHREAD_EXPLICIT_SCHED);
		if(ret != 0){
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			ret = -1;
		}
	}

    if(ret == 0){
		ret = pthread_attr_setstacksize(&thread_attr, attr->stack_size);
		if(ret != 0){
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			ret = -1;
		}
		(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "%s thread stack size = %d\n", attr->thread_name, attr->stack_size);
    }

    if(ret == 0){
		ret = pthread_create(&attr->thread_id, &thread_attr, funcPtr, parameter);
		if(ret != 0){
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			ret = -1;
		}
    }

	if(ret == 0) {
		ret = pthread_attr_destroy(&thread_attr);
		if(ret != 0){
			(void)strerror_r(ret, errBuf, sizeof(errBuf));
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			ret = -1;
		}
	}

#if 0
	if(ret == 0) {
		// The thread name is a meaningful C language string, whose length is restricted to 16 characters, including the terminating null byte ('\0').
		if(attr->thread_name != NULL) {
			i = tchdr_strlen(attr->thread_name);
		}
		if (i > 0) {
			S8 pcName[16] = {0,};
			if (i > 15) {
				tchdr_strncpy(pcName, attr->thread_name, 15);
			}
			else {
				tchdr_strncpy(pcName, attr->thread_name, (U32)i);
			}
			ret = pthread_setname_np(attr->thread_id, pcName);
		}
		if(ret != 0){
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]: An error occurred: %s\n", __func__, __LINE__, errBuf);
			ret = -1;
		}
	}
#endif

    return ret;
}

void tchdr_setPostThreadAttr(stThread_attr_t threadAttr, eLOG_TAG_t tag)
{
	if(threadAttr.policy == SCHED_OTHER) {
		SLONG sret = syscall(SYS_gettid);
#ifdef HDR_32BIT_SYSTEM
		if(sret >= 0)
#else
		if((sret >= 0) && (sret <= (SLONG)U32_MAX))
#endif
		{
			S32 pret = setpriority((S32)PRIO_PROCESS, (U32)(sret), threadAttr.nice);
			if(pret != 0) {
				(*pfnHdrLog)(tag, eLOG_ERR, "Failed to set the nice value[%d] of %s thread.\n", threadAttr.nice, threadAttr.thread_name);
			}
			else {
				(*pfnHdrLog)(tag, eLOG_DBG, "The nice value[%d] of %s thread[%ld] is successfully set.\n", threadAttr.nice, threadAttr.thread_name, sret);
			}
		}
		else {
			(*pfnHdrLog)(tag, eLOG_ERR, "Failed to get the thread ID of %s thread. sret=%ld\n", threadAttr.thread_name, sret);
		}
	}
	(void)prctl(PR_SET_NAME, threadAttr.thread_name, 0, 0, 0);
}

//////////////////////////////////////
/* 	      Convert Functions         */
/* 	       (for Coverity)           */
//////////////////////////////////////
U8 tchdr_booltou8(HDBOOL in)
{
	U8 out = 0U;
	if(in) {
		out = 1U;
	}
	return out;
}

U32 tchdr_booltou32(HDBOOL in)
{
	U32 out = 0U;
	if(in) {
		out = 1U;
	}
	return out;
}

S32 tchdr_u32tos32(U32 in)
{
	S32 out;
	if (in < (U32)S32_MAX) {
		out = (S32)in;
	}
	else {
		out = S32_MAX;
	}
	return out;
}

U32 tchdr_s32tou32(S32 in)
{
	U32 out = 0;
	if (in > 0) {
		out = (U32)in;
	}
	return out;
}

U16 tchdr_u32tou16(U32 in)
{
	U16 out;
	if(in < (U32)U16_MAX) {
		out = (U16)in;
	}
	else {
		out = U16_MAX;
	}
	return out;
}

U8 tchdr_u32tou8(U32 in)
{
	U8 out;
	if(in < (U32)U08_MAX) {
		out = (U8)in;
	}
	else {
		out = U08_MAX;
	}
	return out;
}

S16 tchdr_u32tos16(U32 in)
{
	S16 out;
	if(in < (U32)S16_MAX) {
		out = (S16)in;
	}
	else {
		out = S16_MAX;
	}

	return out;
}

U32 tchdr_f32tou32(F32 in)
{
	U32 out = 0U;
	if(in > 0.0f) {
		out = (U32)in;
	}
	return out;
}

S32 tchdr_f64tos32(F64 in)
{
	S32 out;
	if(in >= 0.0) {
		if(in < (F64)S32_MAX) {
			out = (S32)in;
		}
		else {
			out = S32_MAX;
		}
	}
	else {
		if(in > (F64)S32_MIN) {
			out = (S32)in;
		}
		else {
			out = (S32)S32_MIN;
		}
	}
	return out;
}

S32 tchdr_f32tos32(F32 in)
{
	S32 out;
	F64 temp = (F64)in;
	out = tchdr_f64tos32(temp);
	return out;
}

F32 tchdr_f64tof32(F64 in)
{
	F32 out;
	if(in >= 0.0) {
		if(in < (F64)F32_MAX) {
			out = (F32)in;
		}
		else {
			out = F32_MAX;
		}
	}
	else {
		if(in > (F64)F32_MIN) {
			out = (F32)in;
		}
		else {
			out = F32_MIN;
		}
	}
	return out;
}

F32 tchdr_s16tof32(S16 in)
{
	F32 out;
	F64 temp = (F64)in;

	// The unsigned int type and the int type can be cast directly to double type.
	out = tchdr_f64tof32(temp);

	return out;
}

F32 tchdr_u32tof32(U32 in)
{
	F32 out;
	F64 temp = (F64)in;

	// The unsigned int type and the int type can be cast directly to double type.
	out = tchdr_f64tof32(temp);

	return out;
}

F32 tchdr_s32tof32(S32 in)
{
	F32 out;
	F64 temp = (F64)in;

	// The unsigned int type and the int type can be cast directly to double type.
	out = tchdr_f64tof32(temp);

	return out;
}

ULONG tchdr_slongtoulong(SLONG in)
{
	ULONG out = 0;
	if(in > 0) {
		out = (ULONG)in;
	}
	return out;
}

U32 tchdr_slongtou32(SLONG in)
{
	U32 out = 0;
#ifdef HDR_32BIT_SYSTEM
	if(in > 0) {
		out = (U32)in;
	}
#else
	if(in > 0) {
		if(in < (SLONG)U32_MAX) {
			out = (U32)in;
		}
		else {
			out = (U32)U32_MAX;
		}
	}
#endif
	return out;
}

SLONG tchdr_u32toslong(U32 in)
{
	SLONG out=0;
#ifdef HDR_32BIT_SYSTEM
	if(in < (U32)L_MAX) {
		out = (SLONG)in;
	}
	else {
		out = L_MAX;
	}
#else
	out = (SLONG)in;
#endif
	return out;
}

///////////////////////////////////////
/* 	     Calculation Functions       */
///////////////////////////////////////
/* int(16bit) A * B */
S16 tchdr_s16mul(S16 a, S16 b)
{
	S32 tmp;
	S16 out;
	tmp = (S32)a * (S32)b;
	if(tmp >= 0) {
		if(tmp < S16_MAX) {
			out = (S16)tmp;
		}
		else {
			out = S16_MAX;
		}
	}
	else {
		if(tmp > S16_MIN) {
			out = (S16)tmp;
		}
		else {
			out = (S16)S16_MIN;
		}
	}
	return out;
}

SLONG tchdr_slongadd(SLONG a, SLONG b)
{
	SLONG out;
	if(a < 0) {
		if(b < 0) {
			// (a < 0) and (b < 0)
			if (a > (L_MIN - b)) {
				out = a + b;
			}
			else {
				out = L_MIN;
			}
		}
		else {
			// (a < 0) and (b >= 0)
			out = a + b;
		}
	}
	else{
		if(b < 0) {
			// (a >= 0) and (b < 0)
			out = a + b;
		}
		else {
			// (a >= 0) and (b >= 0)
			if (a < (L_MAX - b)) {
				out = a + b;
			}
			else {
				out = L_MAX;
			}
		}
	}
	return out;
}

/* int(32bit) A + B */
S32 tchdr_s32add(S32 a, S32 b)
{
	S32 out;
	if(a < 0) {
		if(b < 0) {
			// (a < 0) and (b < 0)
			if (a > (S32_MIN - b)) {
				out = a + b;
			}
			else {
				out = (S32)S32_MIN;
			}
		}
		else {
			// (a < 0) and (b >= 0)
			out = a + b;
		}
	}
	else{
		if(b < 0) {
			// (a >= 0) and (b < 0)
			out = a + b;
		}
		else {
			// (a >= 0) and (b >= 0)
			if (a < (S32_MAX - b)) {
				out = a + b;
			}
			else {
				out = S32_MAX;
			}
		}
	}
	return out;
}

/* int(32bit) A - B */
S32 tchdr_s32sub(S32 a, S32 b)
{
	S32 out;
	if(a < 0) {
		if(b > 0) {
			// (a < 0) and (b > 0)
			if (a > (S32_MIN + b)) {
				out = a - b;
			}
			else {
				out = (S32)S32_MIN;
			}
		}
		else {
			// (a < 0) and (b <= 0)
			out = a - b;
		}
	}
	else{
		if(b > 0) {
			// (a >= 0) and (b > 0)
			out = a - b;
		}
		else {
			// (a >= 0) and (b <= 0)
			if (a < (S32_MAX + b)) {
				out = a - b;
			}
			else {
				out = S32_MAX;
			}
		}
	}
	return out;
}

/* int(32bit) A * B */
S32 tchdr_s32mul(S32 a, S32 b)
{
	S64 tmp;
	S32 out;
	tmp = (S64)a * (S64)b;
	if(tmp >= 0) {
		if(tmp < S32_MAX) {
			out = (S32)tmp;
		}
		else {
			out = S32_MAX;
		}
	}
	else {
		if(tmp > S32_MIN) {
			out = (S32)tmp;
		}
		else {
			out = (S32)S32_MIN;
		}
	}
	return out;
}

/* long long(64bit) A - B */
S64 tchdr_s64sub(S64 a, S64 b)
{
	S64 out;
	if(a < 0) {
		if(b > 0) {
			// (a < 0) and (b > 0)
			if (a > (S64_MIN + b)) {
				out = a - b;
			}
			else {
				out = S64_MIN;
			}
		}
		else {
			// (a < 0) and (b <= 0)
			out = a - b;
		}
	}
	else{
		if(b > 0) {
			// (a >= 0) and (b > 0)
			out = a - b;
		}
		else {
			// (a >= 0) and (b <= 0)
			if (a < (S64_MAX + b)) {
				out = a - b;
			}
			else {
				out = S64_MAX;
			}
		}
	}
	return out;
}

ULONG tchdr_ulongadd(ULONG a, ULONG b)
{
	ULONG out = 0;
	if (a < (UL_MAX - b)) {
		out = a + b;
	}
	else {
		out = UL_MAX;
	}
	return out;
}

/* unsigned char(8bit) A + B */
U8 tchdr_u08add(U8 a, U8 b)
{
	U8 out;
	if (b < ((U8)U08_MAX - a)) {
		out = a + b;
	}
	else {
		out = (U8)U08_MAX;
	}
	return out;
}

/* unsigned short(16bit) A + B */
U16 tchdr_u16add(U16 a, U16 b)
{
	U16 out;
	if (b < ((U16)U16_MAX - a)) {
		out = a + b;
	}
	else {
		out = (U16)U16_MAX;
	}
	return out;
}

/* unsigned int(32bit) A + B */
U32 tchdr_u32add(U32 a, U32 b)
{
	U32 out;
	if (b < ((U32)U32_MAX - a)) {
		out = a + b;
	}
	else {
		out = (U32)U32_MAX;
	}
	return out;
}

/* unsigned int(32bit) A - B */
U32 tchdr_u32sub(U32 a, U32 b)
{
	U32 out = 0;
	if (a > b) {
		out = a - b;
	}
	return out;
}

/* unsigned int(32bit) A * B */
U32 tchdr_u32mul(U32 a, U32 b)
{
	U64 tmp;
	U32 out = 0;

	tmp = (U64)a * (U64)b;
	if (tmp <= U32_MAX) {
		out = (U32)tmp;
	}
	else {
		out = U32_MAX;
	}
	return out;
}

ULONG tchdr_ulongmul(ULONG a, ULONG b)
{
	ULONG out = 0U;
	if((a != 0U) && (b != 0U)) {
		out = UL_MAX/b;
		if(out >= a) {
			out = a * b;
		}
		else {
			out = UL_MAX;
		}
	}
	return out;
}

