/*******************************************************************************

*   FileName : tcradio_hal_rbuf.h

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
#ifndef _TCRADIO_HAL_RBUF_H_
#define _TCRADIO_HAL_RBUF_H_

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

//#define 		RBUF_SIZE		1024*1024
typedef enum {
	eBUFFER_EMPTY		=0,
	eBUFFER_FULL		=1,
	eBUFFER_NORMAL		=2
}eBUFFER_STATE_t;

/***************************************************
*				Enumeration				*
****************************************************/

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct {
	unsigned int	state;
	unsigned int	maxSize;
	unsigned int 	wp;
	unsigned int	rp;
	char *pBuf;
} stBUFFER_RING_t;
/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern void ringbuffer_init(stBUFFER_RING_t* stDst, char* pSrc, unsigned int size);
extern int ringbuffer_getBlankSize(stBUFFER_RING_t* stDst);
extern int ringbuffer_getData(stBUFFER_RING_t* stDst, char * pSrc, unsigned int RequestSize);
extern int ringbuffer_setData(stBUFFER_RING_t* stDst, char * pSrc, unsigned int RequestSize);
extern int ringbuffer_getDataEx(stBUFFER_RING_t* stDst, char * pSrc, unsigned int RequestSize) ;

#ifdef __cplusplus
}
#endif

#endif	// _TCRADIO_HAL_RBUF_H_
