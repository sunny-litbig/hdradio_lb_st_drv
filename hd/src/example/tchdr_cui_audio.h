/*******************************************************************************

*   FileName : tchdr_cui_audio.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio CUI Audio header

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
#ifndef TCHDR_CUI_AUDIO_H__
#define TCHDR_CUI_AUDIO_H__

/***************************************************
*				Include					*
****************************************************/
#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef __cplusplus
extern    "C"
{
#endif

/***************************************************
*				Defines					*
****************************************************/
#ifdef __ANDROID__
#define CUI_AUD_LOGMSG(...)			(__android_log_print(ANDROID_LOG_DEBUG,"[HDRCUI]", __VA_ARGS__))
#define CUI_AUD_LOGE(...)			(__android_log_print(ANDROID_LOG_ERROR,"[HDRCUI][AUDIO]", __VA_ARGS__))
#define CUI_AUD_LOGW(...)			(__android_log_print(ANDROID_LOG_WARN,"[HDRCUI][AUDIO]", __VA_ARGS__))
#define CUI_AUD_LOGI(...)			(__android_log_print(ANDROID_LOG_INFO,"[HDRCUI][AUDIO]", __VA_ARGS__))
#define CUI_AUD_LOGD(...)			(__android_log_print(ANDROID_LOG_DEBUG,"[HDRCUI][AUDIO]", __VA_ARGS__))
#else // #ifdef __ANDROID__
#define CUI_AUD_LOGMSG(...)			((void)printf(__VA_ARGS__))
#define CUI_AUD_LOGE(...)			((void)printf("[ERR][HDRCUI][AUDIO]: " __VA_ARGS__))
#define CUI_AUD_LOGW(...)			((void)printf("[WRN][HDRCUI][AUDIO]: " __VA_ARGS__))
#define CUI_AUD_LOGI(...)			((void)printf("[INF][HDRCUI][AUDIO]: " __VA_ARGS__))
#define CUI_AUD_LOGD(...)			((void)printf("[DBG][HDRCUI][AUDIO]: " __VA_ARGS__))
#endif // #ifdef __ANDROID__

#define	SOUND_THREAD_TIME_INTERVAL	(10)

#define	SOUND_NUM_OF_CHANNEL		(2)
#define	SOUND_BUFFER_SIZE			(8192)
#define	SOUND_PERIOD_SIZE			(2048)

/* The Max. number of the audio fifo can be modified in 'tcradio_hal_fifo.h'. */
#define	HDR_AUD_FIFO_INDEX			(0)

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
extern HDRET tchdradiocui_audio_init(void);
extern HDRET tchdradiocui_audio_deinit(void);
extern HDRET tchdradiocui_audio_open(U32 samplerate);
extern HDRET tchdradiocui_audio_close(void);
extern void tchdradiocui_audioQueueCallBack(void *pOutBuf, S32 frames, U32 samplerate);

#ifdef __cplusplus
}
#endif

#endif

