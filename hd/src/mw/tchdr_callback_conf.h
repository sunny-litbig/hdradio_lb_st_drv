/*******************************************************************************

*   FileName : tchdr_callback_conf.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio Framework callback configuration functions header

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
#ifndef TCHDR_CALLBACK_CONF_H__
#define TCHDR_CALLBACK_CONF_H__

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
typedef S32 (*pfnTcHdrIQ01Open_t)(void);
typedef S32 (*pfnTcHdrIQ01Close_t)(void);
typedef S32 (*pfnTcHdrIQ01SetParams_t)(S32 nchannels, S32 nbit, S32 samplerate, S32 buffersize, S32 periodsize);
typedef S32 (*pfnTcHdrIQ01Start_t)(void);
typedef S32 (*pfnTcHdrIQ01Stop_t)(void);
typedef S32 (*pfnTcHdrIQ01Read_t)(S8 *data, S32 readsize);

typedef S32 (*pfnTcHdrIQ23Open_t)(void);
typedef S32 (*pfnTcHdrIQ23Close_t)(void);
typedef S32 (*pfnTcHdrIQ23SetParams_t)(S32 nchannels, S32 nbit, S32 samplerate, S32 buffersize, S32 periodsize);
typedef S32 (*pfnTcHdrIQ23Start_t)(void);
typedef S32 (*pfnTcHdrIQ23Stop_t)(void);
typedef S32 (*pfnTcHdrIQ23Read_t)(S8 *data, S32 readsize);

typedef S32 (*pfnTcHdrBlendAudioOpen_t)(void);
typedef S32 (*pfnTcHdrBlendAudioClose_t)(void);
typedef S32 (*pfnTcHdrBlendAudioSetParams_t)(S32 nchannels, S32 nbit, S32 samplerate, S32 buffersize, S32 periodsize);
typedef S32 (*pfnTcHdrBlendAudioStart_t)(void);
typedef S32 (*pfnTcHdrBlendAudioStop_t)(void);
typedef S32 (*pfnTcHdrBlendAudioRead_t)(S8 *data, S32 readsize);

typedef void(*pfnTcHdrNotificationCallBack_t)(U32 notifyID, const U32 *pArg, void* const *pData, S32 errorCode);
typedef void(*pfnTcHdrAudioQueueCallBack_t)(void *pOutBuf, S32 frames, U32 samplerate);

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern pfnTcHdrIQ01Open_t      pfnTcHdrIQ01Open;
extern pfnTcHdrIQ01Close_t     pfnTcHdrIQ01Close;
extern pfnTcHdrIQ01SetParams_t pfnTcHdrIQ01SetParams;
extern pfnTcHdrIQ01Start_t     pfnTcHdrIQ01Start;
extern pfnTcHdrIQ01Stop_t      pfnTcHdrIQ01Stop;
extern pfnTcHdrIQ01Read_t      pfnTcHdrIQ01Read;

extern pfnTcHdrIQ23Open_t      pfnTcHdrIQ23Open;
extern pfnTcHdrIQ23Close_t     pfnTcHdrIQ23Close;
extern pfnTcHdrIQ23SetParams_t pfnTcHdrIQ23SetParams;
extern pfnTcHdrIQ23Start_t     pfnTcHdrIQ23Start;
extern pfnTcHdrIQ23Stop_t      pfnTcHdrIQ23Stop;
extern pfnTcHdrIQ23Read_t      pfnTcHdrIQ23Read;

extern pfnTcHdrBlendAudioOpen_t       pfnTcHdrBlendAudioOpen;
extern pfnTcHdrBlendAudioClose_t      pfnTcHdrBlendAudioClose;
extern pfnTcHdrBlendAudioSetParams_t  pfnTcHdrBlendAudioSetParams;
extern pfnTcHdrBlendAudioStart_t      pfnTcHdrBlendAudioStart;
extern pfnTcHdrBlendAudioStop_t       pfnTcHdrBlendAudioStop;
extern pfnTcHdrBlendAudioRead_t       pfnTcHdrBlendAudioRead;

extern pfnTcHdrNotificationCallBack_t pfnTcHdrNotificationCallBack;
extern pfnTcHdrAudioQueueCallBack_t   pfnTcHdrAudioQueueCallBack;

/***************************************************
*			Function declaration				*
****************************************************/

#ifdef __cplusplus
}
#endif

#endif

