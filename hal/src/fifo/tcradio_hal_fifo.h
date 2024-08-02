/*******************************************************************************

*   FileName : tcradio_hal_fifo.h

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
#ifndef __TCRADIO_HAL_FIFO_H__
#define __TCRADIO_HAL_FIFO_H__

/***************************************************
*				Include					*
****************************************************/
#ifdef __ANDROID__
#include <android/log.h>
#endif

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "DMBLog.h"

#define TCRADIO_AUDIO_HAL_DEBUG

#ifdef __ANDROID__

#define RAHAL_TAG			("[RADIO][AUD-FIFO]")
#define RAHAL_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,RAHAL_TAG, __VA_ARGS__))
#ifdef TCRADIO_AUDIO_HAL_DEBUG
#define RAHAL_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,RAHAL_TAG, __VA_ARGS__))
#else
#define	RAHAL_DBG(...)
#endif

#else // #ifdef __ANDROID__

// #define RAHAL_ERR(...)		((void)printf("[ERROR][RADIO][AUD-FIFO]: " __VA_ARGS__))
#define RAHAL_ERR(...)		((void)LB_PRINTF("[ERROR][RADIO][AUD-FIFO]: " __VA_ARGS__))
#ifdef TCRADIO_AUDIO_HAL_DEBUG
#define RAHAL_DBG(...)		((void)LB_PRINTF("[DEBUG][RADIO][AUD-FIFO]: " __VA_ARGS__))
#else
#define	RAHAL_DBG(...)
#endif

#endif // #ifdef __ANDROID__

#define	AUDIO_FIFO_NUM	2
#define	AUDIO_FIFO_BUFFER_SIZE	16*1024

/***************************************************
*				Enumeration				*
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
extern RET tcradiohal_audiofifo_init(int32 numOfAudfifo);
extern RET tcradiohal_audiofifo_deinit(int32 numOfAudfifo);
extern RET tcradiohal_audiofifo_reset(int32 audfifo_index);
extern int tcradiohal_audiofifo_pushData(int32 audfifo_index, uint8 *pucData, uint32 uiSize);
extern int tcradiohal_audiofifo_popData(int32 audfifo_index, uint8 *pucData, uint32 uiSize);
extern int tcradiohal_audiofifo_getBlankSize(int32 audfifo_index);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef __TCRADIO_HAL_FIFO_H__
