/*******************************************************************************

*   FileName : tchdr_cui_if.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio CUI Interface Header

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
#ifndef TCHDR_CUI_IF_H__
#define TCHDR_CUI_IF_H__

/***************************************************
*               Include                            *
****************************************************/
#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef __cplusplus
extern    "C"
{
#endif

/***************************************************
*               Defines                            *
****************************************************/
//#define HDRCUI_AVOID_MC2012_RULE_21P6_USING_PRINTF

#define	CUI_MSG_LOG		(0U)
#define CUI_ERR_LOG		(1U)
#define CUI_WRN_LOG		(2U)
#define	CUI_INF_LOG		(3U)
#define CUI_DBG_LOG		(4U)

#ifdef __ANDROID__
#define CUI_MAIN_LOGMSG(...)		(__android_log_print(ANDROID_LOG_DEBUG,"[HDRCUI]", __VA_ARGS__))
#define CUI_MAIN_LOGE(...)			(__android_log_print(ANDROID_LOG_ERROR,"[HDRCUI][MAIN]", __VA_ARGS__))
#define CUI_MAIN_LOGW(...)			(__android_log_print(ANDROID_LOG_WARN,"[HDRCUI][MAIN]", __VA_ARGS__))
#define CUI_MAIN_LOGI(...)			(__android_log_print(ANDROID_LOG_INFO,"[HDRCUI][MAIN]", __VA_ARGS__))
#define CUI_MAIN_LOGD(...)			(__android_log_print(ANDROID_LOG_DEBUG,"[HDRCUI][MAIN]", __VA_ARGS__))
#else // #ifdef __ANDROID__
#define CUI_MAIN_LOGMSG(...)		((void)printf(__VA_ARGS__))
#define CUI_MAIN_LOGE(...)			((void)printf("[ERR][HDRCUI][MAIN] " __VA_ARGS__))
#define CUI_MAIN_LOGW(...)			((void)printf("[WRN][HDRCUI][MAIN] " __VA_ARGS__))
#define CUI_MAIN_LOGI(...)			((void)printf("[INF][HDRCUI][MAIN] " __VA_ARGS__))
#define CUI_MAIN_LOGD(...)			((void)printf("[DBG][HDRCUI][MAIN] " __VA_ARGS__))
#endif // #ifdef __ANDROID__

#define MAX_SCMP_LEN			(10)
#define PSD_BITMASK				(0xDFU)
#define SCMP(x, y, len)			(strncmp((const char*)(x), (const char*)(y), (size_t)len))

/***************************************************
*               Enumeration                        *
****************************************************/

/***************************************************
*               Typedefs                           *
****************************************************/

/***************************************************
*               Constant definitions               *
****************************************************/

/***************************************************
*               Variable definitions               *
****************************************************/

/***************************************************
*               Function declaration               *
****************************************************/
extern HDRET tchdradiocui_getConfFromArg(S32 cnt, S8* const sz[]);
extern HDRET tchdradiocui_init(U32 nice);
extern HDRET tchdradiocui_deinit(void);
extern HDRET tchdradiocui_open(void);
extern HDRET tchdradiocui_close(void);
extern HDRET tchdradiocui_setReacquire(eTC_HDR_ID_t hdr_id);
extern HDRET tchdradiocui_setTune(eTC_HDR_BAND_t hdr_band, U32 freq, eTC_HDR_ID_t hdr_id);
extern HDRET tchdradiocui_setAudioDevice(U32 audio_samplerate, U32 OnOff);

extern HDRET tchdradiocui_createThread(pthread_t *pHandle, void*(*pfThread)(void *arg));

extern void HDRCUI_MAIN_LOG(U8 log_type, U8 en_tag, const S8 *format, ...);
#ifdef __cplusplus
}
#endif

#endif
