/*******************************************************************************

*   FileName : tchdr_cbuffer.h

*   Copyright (c) Telechips Inc.

*   Description : Generic circular buffer header

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
#ifndef TCHDR_CBUFFER_H__
#define TCHDR_CBUFFER_H__

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

/***************************************************
*				Typedefs					*
****************************************************/
/**
 * brief: Circular Buffer object structure.
 */
typedef struct {
    pthread_mutex_t* mutex;		/**< Assign a mutex for thread safe operations */
    U8* data;			/**< Private don't use directly */
    U32 size;			/**< Private don't use directly */
    U32 elemSize;		/**< Private don't use directly */
    U32 wp;				/**< Private don't use directly */
    U32 rp;				/**< Private don't use directly */
    U32 elemCount;		/**< Private don't use directly */
}stCIRC_BUFF_t;

typedef struct {
	void (*cb_init)(stCIRC_BUFF_t* cb, void* buffer, U32 size, U32 elemSize);
	S32 (*cb_reset)(stCIRC_BUFF_t* cb);
	S32 (*cb_write)(stCIRC_BUFF_t* cb, const void* inData, U32 numElems);
	S32 (*cb_reset_and_fill)(stCIRC_BUFF_t* cb, S8 value, U32 numElems);
	S32 (*cb_fill)(stCIRC_BUFF_t* cb, S8 value, U32 numElems);
	S32 (*cb_read)(stCIRC_BUFF_t* cb, void* outData, U32 numElems);
	S32 (*cb_availData)(const stCIRC_BUFF_t* cb);
	S32 (*cb_availSpace)(const stCIRC_BUFF_t* cb);
	S32 (*cb_isAllDataZero)(const stCIRC_BUFF_t* cb);
	S32 (*cb_moveReadIndex)(stCIRC_BUFF_t* cb, S32 n);
	void (*cb_flush)(const stCIRC_BUFF_t* cb);
}stCIRC_FUNC_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern stCIRC_FUNC_t stCircFunc;

/***************************************************
*			Function declaration				*
****************************************************/
/**
 * brief: Initialize the a circular buffer object
 * param cb: Pointer to the circular buffer object
 * param buffer: Pointer to the circular buffer memory location
 * param size: Specifies the total number of elements allocated to the buffer
 * param elemSize: Size(in bytes) of each element stored in the buffer
 */
void CB_init_(stCIRC_BUFF_t* cb, void* buffer, U32 size, U32 elemSize);

S32 CB_reset_(stCIRC_BUFF_t* cb);

/**
 * brief: Writes a block of data to circular buffer
 * param cb: Pointer to circualr buffer control object
 * param inData: Pointer to input data
 * param numElems: Specifies the number of elements to be written
 * return
 *     0 - success <br>
 *    -1 - failure
 */
S32 CB_write_(stCIRC_BUFF_t* cb, const void* inData, U32 numElems);

S32 CB_fill_(stCIRC_BUFF_t* cb, S8 value, U32 numElems);

/**
 * brief Reads a block of data
 * param cb: Pointer to circualr buffer control object
 * param outData: Pointer to the output data buffer
 * param numElems: Number of elements to read
 * param mutex: Whether an internal mutex is used
 * returns number of elements read
 */
S32 CB_read_(stCIRC_BUFF_t* cb, void* outData, U32 numElems);

/**
 * brief Returns number of available data elements
 * param cb: Pointer to circualr buffer control object
 * return Number of available elements
 */
S32 CB_availData_(const stCIRC_BUFF_t* cb);

/**
 * brief Returns available space measured in elements(may not be bytes)
 * param cb: Pointer to circualr buffer control object
 * return Available space
 */
S32 CB_availSpace_(const stCIRC_BUFF_t* cb);

/**
 * brief Moves read index without adding or removing data
 * param cb: Pointer to circualr buffer control object
 * param n: direction and magnitude of the move
 */
S32 CB_moveReadIndex_(stCIRC_BUFF_t* cb, S32 n);

/**
 * brief initializes buffer data to zero
 * param cb: Pointer to circualr buffer control object
 */
void CB_flush_(const stCIRC_BUFF_t* cb);

S32 CB_reset_and_fill_(stCIRC_BUFF_t* cb, S8 value, U32 numElems);
S32 CB_isAllDataZero_(const stCIRC_BUFF_t* cb);

#ifdef __cplusplus
}
#endif

#endif /* TCHDR_CBUFFER_H__ */
