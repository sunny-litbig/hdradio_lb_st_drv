/*******************************************************************************

*   FileName : tchdr_std.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio standard support header

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
#ifndef TCHDR_STD_H__
#define TCHDR_STD_H__

/***************************************************
*				Include					*
****************************************************/
#include <pthread.h>

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
	extern "C" {
#endif

#define	tchdr_ussleep(X)		(usleep(X))
#define	tchdr_mssleep(X)		(usleep((X)*(1000)))

/**
 * @brief Reverses the bytes of 32-bit integer
 *
 * Converts integer from one endiness to the other
 * @param [in] x: integer to byte-reverse
 */
#define REVERSE_BYTES_32(x)  (((x) >> 24)&0x000000FFU) | (((x) >> 8)&0x0000FF00U) | (((x) << 8)&0x00FF0000U) | (((x) << 24)&0xFF000000U);

/**
 * @brief Reverses the bytes of 16-bit integer
 * @param [in] x: integer to byte-reverse
 */
#define REVERSE_BYTES_16(x)  (((x) >> 8) & 0xff) | (((x) & 0xff) << 8)


/***************************************************
*				Enumeration				*
****************************************************/

/***************************************************
*				Typedefs					*
****************************************************/
typedef void *(*FUNC_PTR_T)(void *arg);

typedef struct{
	S32			policy;
	S32			nice;
    S32        	priority;
    S32         affinity;
    U32         stack_size;
	S32			thread_running;
    pthread_t   thread_id;
	const S8*	thread_name;
}stThread_attr_t;

typedef struct {
	void *(*osmemset)(void *pdst, S8 ch, U32 len);
	void *(*osmemcpy)(void *pdst, const void *psrc, U32 len);
	S8 *(*osmalloc)(size_t iSize);
	S8 *(*oscalloc)(size_t iSize, size_t nmemb);
	S8 *(*osstrcpy)(S8 *pdst, const S8 *psrc);
	S8 *(*osstrncpy)(S8 *pdst, const S8 *psrc, U32 len);
	U32 (*osstrlen)(const S8 *psbStr);
	S32 (*osstrcmp)(const S8 *psbStr1, const S8 *psbStr2);
	S32 (*osstrncmp)(const S8 *psbStr1, const S8 *psbStr2, U32 len);
	S8 *(*osstrcat)(S8 *psbStr1, const S8 *psbStr2);
	HDRET (*mutexinit)(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
	HDRET (*mutexdeinit)(pthread_mutex_t *mutex);
	HDRET (*mutexlock)(pthread_mutex_t *mutex);
	void (*mutexunlock)(pthread_mutex_t *mutex);
	void (*osfree)(void *ptr);
	S32 (*oscreateThread)(FUNC_PTR_T funcPtr, stThread_attr_t *attr, void *parameter);
	void (*setPostThreadAttr)(stThread_attr_t threadAttr, eLOG_TAG_t tag);
}stTCHDR_OSAPI_t;

typedef struct {
	U8 (*booltou8)(HDBOOL in);
	U32 (*booltou32)(HDBOOL in);
	F32 (*s16tof32)(S16 in);
	S32 (*u32tos32)(U32 in);
	U32 (*s32tou32)(S32 in);
	U16 (*u32tou16)(U32 in);
	U8 (*u32tou8)(U32 in);
	S16 (*u32tos16)(U32 in);
	S32 (*f32tos32)(F32 in);
	U32 (*f32tou32)(F32 in);
	S32 (*f64tos32)(F64 in);
	F32 (*f64tof32)(F64 in);
	F32 (*u32tof32)(U32 in);
	F32 (*s32tof32)(S32 in);
	ULONG (*slongtoulong)(SLONG in);
	U32 (*slongtou32)(SLONG in);
	SLONG (*u32toslong)(U32 in);
}stTCHDR_CAST_t;

typedef struct {
	S16 (*s16mul)(S16 a, S16 b);
	SLONG (*slongadd)(SLONG a, SLONG b);
	S32 (*s32add)(S32 a, S32 b);
	S32 (*s32sub)(S32 a, S32 b);
	S32 (*s32mul)(S32 a, S32 b);
	S64 (*s64sub)(S64 a, S64 b);
	ULONG (*ulongadd)(ULONG a, ULONG b);
	U8 (*u8add)(U8 a, U8 b);
	U16 (*u16add)(U16 a, U16 b);
	U32 (*u32add)(U32 a, U32 b);
	U32 (*u32sub)(U32 a, U32 b);
	U32 (*u32mul)(U32 a, U32 b);
	ULONG (*ulongmul)(ULONG a, ULONG b);
}stTCHDR_ARITHMETIC_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern stTCHDR_OSAPI_t stOsal;
extern stTCHDR_CAST_t stCast;
extern stTCHDR_ARITHMETIC_t stArith;

/***************************************************
*			Function declaration				*
****************************************************/
extern void tchdr_16bto8b(U8 *buf, U16 data);
extern void tchdr_32bto8b(U8 *buf, U32 data);
extern void *tchdr_memset(void *pdst, S8 ch, U32 len);
extern void *tchdr_memcpy(void *pdst, const void *psrc, U32 len);
extern S8 *tchdr_malloc(size_t iSize);
extern S8 *tchdr_calloc(size_t iSize, size_t nmemb);
extern S8 *tchdr_strcpy(S8 *pdst, const S8 *psrc);
extern S8 *tchdr_strncpy(S8 *pdst, const S8 *psrc, U32 len);
extern U32 tchdr_strlen(const S8 *psbStr);
extern S32 tchdr_strcmp(const S8 *psbStr1, const S8 *psbStr2);
extern S32 tchdr_strncmp(const S8 *psbStr1, const S8 *psbStr2, U32 len);
extern S8 *tchdr_strcat(S8 *psbStr1, const S8 *psbStr2);

extern HDRET tchdr_mutexInit(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
extern HDRET tchdr_mutexDeinit (pthread_mutex_t *mutex);
extern HDRET tchdr_mutexLock(pthread_mutex_t *mutex);
extern void tchdr_mutexUnlock(pthread_mutex_t *mutex);

extern void tchdr_free(void *ptr);

extern S32 tchdr_createThread(FUNC_PTR_T funcPtr, stThread_attr_t *attr, void *parameter);
extern void tchdr_setPostThreadAttr(stThread_attr_t threadAttr, eLOG_TAG_t tag);

extern U8 tchdr_booltou8(HDBOOL in);
extern U32 tchdr_booltou32(HDBOOL in);
extern F32 tchdr_s16tof32(S16 in);
extern S32 tchdr_u32tos32(U32 in);
extern U32 tchdr_s32tou32(S32 in);
extern U16 tchdr_u32tou16(U32 in);
extern U8 tchdr_u32tou8(U32 in);
extern S16 tchdr_u32tos16(U32 in);
extern S32 tchdr_f32tos32(F32 in);
extern U32 tchdr_f32tou32(F32 in);
extern S32 tchdr_f64tos32(F64 in);
extern F32 tchdr_f64tof32(F64 in);
extern F32 tchdr_u32tof32(U32 in);
extern F32 tchdr_s32tof32(S32 in);
extern ULONG tchdr_slongtoulong(SLONG in);
extern U32 tchdr_slongtou32(SLONG in);
extern SLONG tchdr_u32toslong(U32 in);

extern S16 tchdr_s16mul(S16 a, S16 b);
extern SLONG tchdr_slongadd(SLONG a, SLONG b);
extern S32 tchdr_s32add(S32 a, S32 b);
extern S32 tchdr_s32sub(S32 a, S32 b);
extern S32 tchdr_s32mul(S32 a, S32 b);
extern S64 tchdr_s64sub(S64 a, S64 b);

extern ULONG tchdr_ulongadd(ULONG a, ULONG b);
extern U8 tchdr_u08add(U8 a, U8 b);
extern U16 tchdr_u16add(U16 a, U16 b);
extern U32 tchdr_u32add(U32 a, U32 b);
extern U32 tchdr_u32sub(U32 a, U32 b);
extern U32 tchdr_u32mul(U32 a, U32 b);
extern ULONG tchdr_ulongmul(ULONG a, ULONG b);

#ifdef __cplusplus
}
#endif

#endif

