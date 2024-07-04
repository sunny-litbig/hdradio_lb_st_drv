/*******************************************************************************

*   FileName : tcradio_types.h

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device types header

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
#ifndef TCRADIO_TYPES_H_
#define TCRADIO_TYPES_H_

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef uint8
typedef unsigned char		uint8;
#endif

#ifndef int8
typedef char				int8;
#endif

#ifndef uint16
typedef unsigned short		uint16;
#endif

#ifndef int16
typedef short				int16;
#endif

#ifndef uint32
typedef unsigned int		uint32;
#endif

#ifndef int32
typedef int					int32;
#endif

#ifndef uint64
typedef unsigned long long	uint64;
#endif

#ifndef int64
typedef long long			int64;
#endif

#ifndef float32
typedef float				float32;
#endif

#ifndef float64
typedef double				float64;
#endif

#ifndef RET
typedef int					RET;
#endif

#ifndef BOOL
typedef int					BOOL;
#endif

#ifndef NULL
#define NULL 				(0)
#endif

#ifndef pNULL
#define	pNULL				((void*)0)
#endif

#ifndef	UP
#define	UP					(1)
#endif

#ifndef	DN
#define	DN					(0)
#endif

#ifndef	YES
#define	YES					(1)
#endif

#ifndef	NO
#define	NO					(0)
#endif

#ifndef	ON
#define	ON					(1)
#endif

#ifndef	OFF
#define	OFF					(0)
#endif

#ifndef TRUE
#define TRUE				(1)
#endif

#ifndef FALSE
#define FALSE				(0)
#endif

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eRET_OK	= 0,

	eRET_NG_CREATE_THREAD = -1,
	eRET_NG_MUTEX_INIT = -2,
	eRET_NG_MUTEX_DEINIT = -3,
	eRET_NG_MALLOC = -4,
	eRET_NG_SYSCALL = -5,
	eRET_NG_INVALID_ID	= -6,
	eRET_NG_INVALID_LENGTH = -7,
	eRET_NG_INVALID_PARAM	= -8,
	eRET_NG_INVALID_STATUS = -9,
	eRET_NG_INVALID_DATA = -10,
	eRET_NG_INVALID_TYPE = -11,
	eRET_NG_INVALID_RESP = -12,
	eRET_NG_NO_MEM = -13,
	eRET_NG_NO_RSC = -14,
	eRET_NG_IO = -15,
	eRET_NG_TIMEOUT = -16,
	eRET_NG_INCONSISTENCY = -17,
	eRET_NG_ALSA_UNDRRUN = -18,
	eRET_NG_ALSA_RESET = -19,
	eRET_NG_NO_FILE = -20,
	eRET_NG_NOT_SUPPORT = -21,
	eRET_NG_BUFFER_FULL = -22,
	eRET_NG_CHECKSUM = -23,
	eRET_NG_DECODE = -24,
	eRET_NG_BUSY = -25,
	eRET_NG_ALREADY_OPEN = -26,
	eRET_NG_NOT_OPEN = -27,
	eRET_NG_ALREADY_INIT = -28,
	eRET_NG_NOT_INIT = -29,
	eRET_NG_ALREADY_ENABLE = -30,
	eRET_NG_NOT_ENABLE = -31,
	eRET_NG_FIFO_INIT = -32,
	eRET_NG_EOF = -33,
	eRET_NG_IPC_DRV = -34,
	eRET_NG_COLLISION = -35,

	eRET_NG_UNKNOWN = -900
}eRET_t;

/***************************************************
*				Typedefs					*
****************************************************/

/***************************************************
*			Constant definitions				*
****************************************************/

#ifdef __cplusplus
}
#endif

#endif
