/*******************************************************************************
*
* (C) copyright 2003 - 2016, iBiquity Digital Corporation, U.S.A.
*
********************************************************************************

    This confidential and proprietary software may be used only as
    authorized by a licensing agreement from iBiquity Digital Corporation.
    In the event of publication, the following notice is applicable:

    The availability of this material does not provide any license
    by implication, or otherwise under any patent rights of iBiquity
    Digital Corporation or others covering any use of the
    contents herein.

    Any copies or derivative works must include this and all other
    proprietary notices.

        iBiquity Digital Corporation
        6711 Columbia Gateway Drive, Suite 500
        Columbia, MD USA 21046
*******************************************************************************/
#ifndef TCHDR_LOG_H__
#define TCHDR_LOG_H__

/***************************************************
*				Include					*
****************************************************/
#include "hdrBasicTypes.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif
#include <stdarg.h>

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define NUM_MODULES          (36)
/*#define HDR_CDM_LOG*/
#ifdef	HDR_CDM_LOG
#ifdef __ANDROID__
#define LOG(module, fmt, ...) (__android_log_print(ANDROID_LOG_ERROR,module,__VA_ARGS__))
#else	/* #ifdef __ANDROID__ */
#define LOG(module, fmt, ...) ((void)printf("\n\r", fmt, ## __VA_ARGS__))
#endif	/* #ifdef __ANDROID__ */
#else
#define LOG(module, fmt, ...)
#endif

/***************************************************
*				Enumeration 			*
****************************************************/

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
void HDR_Logger_init(void *mutex);

#ifdef __cplusplus
}
#endif

#endif //TCHDR_LOG_H__
