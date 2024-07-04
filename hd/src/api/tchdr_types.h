/*******************************************************************************

*   FileName : tchdr_types.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio framework variables type definitions

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
#ifndef TCHDR_TYPES_H__
#define TCHDR_TYPES_H__

/***************************************************
*				Include					*
****************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef U8
typedef unsigned char		U8;
#endif

#ifndef S8
typedef char				S8;
#endif

#ifndef S8
typedef signed char			S8C;
#endif

#ifndef U16
typedef unsigned short		U16;
#endif

#ifndef S16
typedef short				S16;
#endif

#ifndef U32
typedef unsigned int		U32;
#endif

#ifndef S32
typedef int					S32;
#endif

#ifndef U64
typedef unsigned long long	U64;
#endif

#ifndef S64
typedef long long			S64;
#endif

#ifndef ULONG
typedef unsigned long		ULONG;
#endif

#ifndef SLONG
typedef long				SLONG;
#endif

#ifndef F32
typedef float				F32;
#endif

#ifndef F64
typedef double				F64;
#endif

#ifndef HDRET
typedef int					HDRET;
#endif

#ifndef HDBOOL
typedef bool				HDBOOL;
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

#ifndef pNULL
#define	pNULL				(void*)(0)
#endif

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eTC_HDR_RET_OK	= 0,

	// Common Error
	eTC_HDR_RET_NG_NOT_ENABLED = -10,
	eTC_HDR_RET_NG_LOCK_MEMORY = -20,
	eTC_HDR_RET_NG_INIT = -21,
	eTC_HDR_RET_NG_BYTESTREAM_OPEN = -22,
	eTC_HDR_RET_NG_BB_SRC_INIT = -23,
	eTC_HDR_RET_NG_INSTANCE_INIT = -24,
	eTC_HDR_RET_NG_CORE_INIT = -25,
	eTC_HDR_RET_NG_AUD_RESAMPLER_INIT = -26,
	eTC_HDR_RET_NG_AUD_RESAMPLER_OUTPUT = -27,
	eTC_HDR_RET_NG_AUD_RESAMPLER_HANDLER = -28,
	eTC_HDR_RET_NG_SET_BAND = -29,
	eTC_HDR_RET_NG_INVALID_PARAMETERS = -30,
	eTC_HDR_RET_NG_BLEND_CROSSFADE_RESET = -31,
	eTC_HDR_RET_NG_NOT_YET_INIT = -32,
	eTC_HDR_RET_NG_NOT_YET_OPEN = -33,
	eTC_HDR_RET_NG_ALREADY_INIT = -34,
	eTC_HDR_RET_NG_ALREADY_OPEN = -35,
	eTC_HDR_RET_NG_INVALID_HDR_TYPE = -36,
	eTC_HDR_RET_NG_INVALID_IQ_BUFFER = -37,
	eTC_HDR_RET_NG_EXT_IQ_DRV_OPEN = -38,
	eTC_HDR_RET_NG_MALLOC = -39,
	eTC_HDR_RET_NG_INVALID_BUFFER_POINTER = -40,
	eTC_HDR_RET_NG_READ_SIZE = -41,
	eTC_HDR_RET_NG_RSC = -42,
	eTC_HDR_RET_NG_MUTEX_INIT = -43,
	eTC_HDR_RET_NG_MUTEX_DEINIT = -44,
	eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS = -45,
	eTC_HDR_RET_NG_NULL_POINTER_MESSAGE = -46,
	eTC_HDR_RET_NG_INIT_MESSAGE_QUEUE = -47,
	eTC_HDR_RET_NG_SET_PROGRAM = -48,
	eTC_HDR_RET_NG_ENABLE_PSD = -49,
	eTC_HDR_RET_NG_NOT_SUPPORT = -50,
	eTC_HDR_RET_NG_INVALID_HDR_ID = -51,
	eTC_HDR_RET_NG_NULL_POINTER_MUTEX = -53,
	eTC_HDR_RET_NG_GET_PROGRAM = -54,
	eTC_HDR_RET_NG_GET_DATA = -55,
	eTC_HDR_RET_NG_LIB_ERROR = -56,
	eTC_HDR_RET_NG_NULL_INSTANCE = -57,
	eTC_HDR_RET_NG_SET_VALUE = -58,
	eTC_HDR_RET_NG_NOT_YET_CLOSE = -59,
	eTC_HDR_RET_NG_IQ01IN_XRUN = -60,
	eTC_HDR_RET_NG_IQ23IN_XRUN = -61,
	eTC_HDR_RET_NG_IQ_INPUT_DRIVER = -62,
	eTC_HDR_RET_NG_INVALID_BAND = -63,
	eTC_HDR_RET_NG_MUTEX_LOCK = -64,
	eTC_HDR_RET_NG_MUTEX_UNLOCK = -65,
	eTC_HDR_RET_NG_EVENT_ERROR = -66,
	eTC_HDR_RET_NG_DEINIT = -67,
	eTC_HDR_RET_NG_ALREADY_CLOSE = -68,

	// DEMOD Error
	eTC_HDR_RET_NG_DEMOD_INVALID_PARAMETERS = -200,
    eTC_HDR_RET_NG_DEMOD_BUSY = -201,
    eTC_HDR_RET_NG_DEMOD_IDLE = -202,
	eTC_HDR_RET_NG_DEMOD_NOT_INIT = -203,
    eTC_HDR_RET_NG_DEMOD_INSTANCE_TYPE = -204,

	// AAS Error
	eTC_HDR_RET_NG_AAS_NOT_FOUND_PORT_OR_SERVICE = -210,
	eTC_HDR_RET_NG_AAS_RESERVED_PORT_REQ = -211,
	eTC_HDR_RET_NG_AAS_MAX_AAS_PORTS_ALREADY_ENABLED = -212,
	eTC_HDR_RET_NG_AAS_MAX_LOT_PORTS_ALREADY_ENABLED = -213,
	eTC_HDR_RET_NG_AAS_ALEADY_OPEN_PORT = -214,
	eTC_HDR_RET_NG_AAS_NOT_FOUND_OBJECT = -215,
	eTC_HDR_RET_NG_AAS_NO_DATA_AVAILABLE = -216,
	eTC_HDR_RET_NG_AAS_CORRUPTED_PACKET = -217,
	eTC_HDR_RET_NG_AAS_NO_COMPLETE_OBJECT = -218,

	// PSD Error
	eTC_HDR_RET_NG_PSD_INVALID_LENGTH = -220,

	// SIG Error
	eTC_HDR_RET_NG_SIG_NO_SERVICE = -230,
	eTC_HDR_RET_NG_SIG_NO_COMPONENT = -231,

	// ALERT Error
	eTC_HDR_RET_NG_ALERT_NO_NEW_MESSAGE = -240,

	// Unkown Error
	eTC_HDR_RET_NG_UNKNOWN = -1000
}eTC_HDR_RET_t;

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

#ifdef __cplusplus
}
#endif

#endif
