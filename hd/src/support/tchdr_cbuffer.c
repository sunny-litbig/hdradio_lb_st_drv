/*******************************************************************************

*   FileName : tchdr_cbuffer.c

*   Copyright (c) Telechips Inc.

*   Description : Generic circular buffer APIs and definitions

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
#include <string.h>

#include "hdrBasicTypes.h"

#include "tchdr_common.h"
#include "tchdr_hdlibcb.h"
#include "tchdr_cbuffer.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
stCIRC_FUNC_t stCircFunc = {
	&CB_init_,
	&CB_reset_,
	&CB_write_,
	&CB_reset_and_fill_,
	&CB_fill_,
	&CB_read_,
	&CB_availData_,
	&CB_availSpace_,
	&CB_isAllDataZero_,
	&CB_moveReadIndex_,
	&CB_flush_
};

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
*           Local type definitions                 *
****************************************************/

/***************************************************
*           Local constant definitions              *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/
static S32 cb_lock_(pthread_mutex_t *mutex);
static S32 cb_unlock_(pthread_mutex_t *mutex);

// For HIS metric violation (HIS_CALLING)
static S32 (*pfnCB_lock)(pthread_mutex_t *mutex) = &cb_lock_;
static S32 (*pfnCB_unlock)(pthread_mutex_t *mutex) = &cb_unlock_;

/***************************************************
*			function definition				*
****************************************************/
void CB_init_(stCIRC_BUFF_t* cb, void* buffer, U32 size, U32 elemSize)
{
	if((cb == NULL) || (buffer == NULL) || (size == 0U) || (elemSize == 0U)) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] One or more of the arguments is null.\n", __func__, __LINE__);
	}
	else {
		cb->size = size;
		cb->elemSize = elemSize;
		cb->data = buffer;
		cb->wp = 0;
		cb->rp = 0;
		cb->elemCount = 0;
	}
}

S32 CB_reset_(stCIRC_BUFF_t* cb)
{
	S32 ret = -1;
	if(cb == NULL) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] The argument is null.\n", __func__, __LINE__);
	}
	else {
		ret = (*pfnCB_lock)(cb->mutex);
		if(ret == 0) {
			cb->wp = 0;
			cb->rp = 0;
			cb->elemCount = 0;
			(void)(*pfnCB_unlock)(cb->mutex);
		}
	}
	return ret;
}

S32 CB_write_(stCIRC_BUFF_t* cb, const void* inData, U32 numElems)
{
	S32 ret = 0;
	if((cb == NULL) || (inData == NULL)) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] One or more of the arguments is null.\n", __func__, __LINE__);
		ret =  -2;
	}
	else if((*stCircFunc.cb_availSpace)(cb) < (*stCast.u32tos32)(numElems)) {
		ret = -1;
	}
	else {
		ret = (*pfnCB_lock)(cb->mutex);
		if(ret == 0) {
			U32 pos = cb->size - cb->wp;
			U32 offset;
			U32 size;
			if(numElems > pos) {
				// Won't fit at the end; copy in two chunks with wrap
				offset = (*stArith.u32mul)(cb->wp, cb->elemSize);
				size = (*stArith.u32mul)(pos, cb->elemSize);
				(void)(*stOsal.osmemcpy)(&cb->data[offset], inData, size);

				offset = (*stArith.u32mul)(pos, cb->elemSize);
				size = (*stArith.u32mul)(numElems - pos, cb->elemSize);
				(void)(*stOsal.osmemcpy)(cb->data, &((const U8*)inData)[offset], size);
				cb->wp = numElems - pos;
			} else {
				offset = (*stArith.u32mul)(cb->wp, cb->elemSize);
				size = (*stArith.u32mul)(numElems, cb->elemSize);
				(void)(*stOsal.osmemcpy)((void*)&cb->data[offset], inData, size);
				cb->wp += numElems;
			}
			cb->elemCount += numElems;
			ret = (*stCast.u32tos32)(numElems);
			(void)(*pfnCB_unlock)(cb->mutex);
		}
	}
	return ret;
}

S32 CB_reset_and_fill_(stCIRC_BUFF_t* cb, S8 value, U32 numElems)
{
	S32 ret;
	if(cb == NULL) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] The argument is null.\n", __func__, __LINE__);
		ret = -2;
	}
	else if(cb->size < numElems){
		ret = -1;
	}
	else {
		ret = (*pfnCB_lock)(cb->mutex);
		if(ret == 0) {
			U32 offset;
			U32 size;
			cb->wp = 0;
			cb->rp = 0;
			cb->elemCount = 0;
			offset = (*stArith.u32mul)(cb->wp, cb->elemSize);
			size  = (*stArith.u32mul)(numElems, cb->elemSize);
			(void)(*stOsal.osmemset)((void*)&cb->data[offset], value, size);
			cb->wp += numElems;
			cb->elemCount += numElems;
			ret = (*stCast.u32tos32)(numElems);
			(void)(*pfnCB_unlock)(cb->mutex);
		}
	}
	return ret;
}

S32 CB_fill_(stCIRC_BUFF_t* cb, S8 value, U32 numElems)
{
	S32 ret;
	if(cb == NULL) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] The pointer argument is null.\n", __func__, __LINE__);
		ret = -2;
	}
	else if((*stCircFunc.cb_availSpace)(cb) < (*stCast.u32tos32)(numElems)) {
		ret = -1;
	}
	else {
		ret = (*pfnCB_lock)(cb->mutex);
		if(ret == 0) {
			U32 pos = cb->size - cb->wp;
			U32 offset;
			U32 size;
			if(numElems > pos){
				// Won't fit at the end; copy in two chunks with wrap
				offset = (*stArith.u32mul)(cb->wp, cb->elemSize);
				size = (*stArith.u32mul)(pos, cb->elemSize);
				(void)(*stOsal.osmemset)((void*)&cb->data[offset], value, size);

				size = (*stArith.u32mul)(numElems - pos, cb->elemSize);
				(void)(*stOsal.osmemset)(cb->data, value, size);
				cb->wp = numElems - pos;
			} else {
				offset = (*stArith.u32mul)(cb->wp, cb->elemSize);
				size = (*stArith.u32mul)(numElems, cb->elemSize);
				(void)(*stOsal.osmemset)((void*)&cb->data[offset], value, size);
				cb->wp += numElems;
			}
			cb->elemCount += numElems;
			ret = (*stCast.u32tos32)(numElems);
			(void)(*pfnCB_unlock)(cb->mutex);
		}
	}
	return ret;
}

S32 CB_read_(stCIRC_BUFF_t* cb, void* outData, U32 numElems)
{
	S32 ret = 0;

	if((cb == NULL) || (outData == NULL)) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] One or more of the arguments is null.\n", __func__, __LINE__);
		ret = -2;
	}
	else if(cb->elemCount < numElems) {
		ret = -1;
	}
	else {
		ret = (*pfnCB_lock)(cb->mutex);
		if(ret == 0) {
			U32 pos = cb->size - cb->rp;
			U32 offset;
			U32 size;
			if(numElems > pos) {
				// Wrap needed; copy in two chunks
				offset = (*stArith.u32mul)(cb->rp, cb->elemSize);
				size = (*stArith.u32mul)(pos, cb->elemSize);
				(void)(*stOsal.osmemcpy)(outData, &cb->data[offset], size);

				offset = (*stArith.u32mul)(pos, cb->elemSize);
				size = (*stArith.u32mul)(numElems - pos, cb->elemSize);
				(void)(*stOsal.osmemcpy)(&((U8*)outData)[offset], cb->data, size);
				cb->rp = numElems - pos;
			} else {
				offset = (*stArith.u32mul)(cb->rp, cb->elemSize);
				size = (*stArith.u32mul)(numElems, cb->elemSize);
				(void)(*stOsal.osmemcpy)(outData, &cb->data[offset], size);
				cb->rp += numElems;
			}
			cb->elemCount -= numElems;
			ret = (*stCast.u32tos32)(numElems);
			(void)(*pfnCB_unlock)(cb->mutex);
		}
	}
	return ret;
}

S32 CB_availData_(const stCIRC_BUFF_t* cb)
{
	S32 ret;
	if(cb == NULL) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] The argument is null.\n", __func__, __LINE__);
		ret = -2;
	}
	else {
		ret = (*stCast.u32tos32)(cb->elemCount);
	}
	return ret;
}

S32 CB_availSpace_(const stCIRC_BUFF_t* cb)
{
	S32 ret;
	if(cb == NULL) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] The argument is null.\n", __func__, __LINE__);
		ret = -2;
	}
	else {
		ret = (*stCast.u32tos32)(cb->size - cb->elemCount);
	}
	return ret;
}

S32 CB_isAllDataZero_(const stCIRC_BUFF_t* cb)	// for test
{
	S32 ret = -1;
	if(cb == NULL) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] The argument is null.\n", __func__, __LINE__);
		ret = -2;
	}
	else {
		U32 i;
		for(i=0; i < cb->size; i++) {
			if(cb->data[i] != 0U) {
				break;
			}
		}
		if(i == cb->size) {
			ret = 0;
		}
	}
	return ret;
}

S32 CB_moveReadIndex_(stCIRC_BUFF_t* cb, S32 n)
{
	S32 ret;
	if(cb == NULL) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] The argument is null.\n", __func__, __LINE__);
		ret = -2;
	}
	else if((n > (*stCast.u32tos32)(cb->elemCount)) || ((*stArith.s32sub)((*stCast.u32tos32)(cb->elemCount), n) > (*stCast.u32tos32)(cb->size))) {
		ret = -1;
	}
	else {
		ret = (*pfnCB_lock)(cb->mutex);
		if(ret == 0) {
			S32 rp = (*stCast.u32tos32)(cb->rp);
			S32 size = (*stCast.u32tos32)(cb->size);
			S32 elemCount = (*stCast.u32tos32)(cb->elemCount);
			S32 pos = (*stArith.s32add)(rp, n);
			if(pos > size){
				// wrap to the beginning
				rp = (*stArith.s32sub)(n, (*stArith.s32sub)(size, rp));
			} else if(pos < 0){
				// wrap to the end
				rp = size + pos;
			} else {
				rp = pos;
			}
			cb->rp = (*stCast.s32tou32)(rp);
			cb->elemCount = (*stCast.s32tou32)((*stArith.s32sub)(elemCount, n));
			(void)(*pfnCB_unlock)(cb->mutex);
		}
	}
	return ret;
}

// Locks fifo
// It is intended to be shared between OS tasks(threads)
static S32 cb_lock_(pthread_mutex_t *mutex)
{
	S32 ret = -1;
    if(mutex != NULL){
		if(pthread_mutex_lock(mutex) != 0) {
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to cbuffer lock mutex.\n");
		}
		else {
			ret = 0;
		}
	}
	return ret;
}

// Unlocks fifo
// It is intended to be shared between OS tasks(threads)
static S32 cb_unlock_(pthread_mutex_t *mutex)
{
	S32 ret = -1;
    if(mutex != NULL){
		(void)pthread_mutex_unlock(mutex);
		ret = 0;
    }
	return ret;
}

void CB_flush_(const stCIRC_BUFF_t* cb)
{
    S32 ret;
    if(cb != NULL) {
	    ret = (*pfnCB_lock)(cb->mutex);
		if(ret == 0) {
		    (void)(*stOsal.osmemset)(cb->data, (S8)0, (U32)cb->size * (U32)cb->elemSize);
		    (void)(*pfnCB_unlock)(cb->mutex);
		}
    }
}
