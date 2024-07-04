/*******************************************************************************

*   FileName : tchdr_callback_conf.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio Framework callback configuration functions

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
*		Include 			   *
****************************************************/
#include "tchdr_common.h"
#include "tchdr_callback_conf.h"
#include "tchdr_api.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
pfnTcHdrIQ01Open_t	pfnTcHdrIQ01Open;
pfnTcHdrIQ01Close_t	pfnTcHdrIQ01Close;
pfnTcHdrIQ01SetParams_t pfnTcHdrIQ01SetParams;
pfnTcHdrIQ01Start_t	pfnTcHdrIQ01Start;
pfnTcHdrIQ01Stop_t	pfnTcHdrIQ01Stop;
pfnTcHdrIQ01Read_t	pfnTcHdrIQ01Read;

pfnTcHdrIQ23Open_t	pfnTcHdrIQ23Open;
pfnTcHdrIQ23Close_t	pfnTcHdrIQ23Close;
pfnTcHdrIQ23SetParams_t pfnTcHdrIQ23SetParams;
pfnTcHdrIQ23Start_t	pfnTcHdrIQ23Start;
pfnTcHdrIQ23Stop_t	pfnTcHdrIQ23Stop;
pfnTcHdrIQ23Read_t	pfnTcHdrIQ23Read;

pfnTcHdrBlendAudioOpen_t	pfnTcHdrBlendAudioOpen;
pfnTcHdrBlendAudioClose_t	pfnTcHdrBlendAudioClose;
pfnTcHdrBlendAudioSetParams_t	pfnTcHdrBlendAudioSetParams;
pfnTcHdrBlendAudioStart_t	pfnTcHdrBlendAudioStart;
pfnTcHdrBlendAudioStop_t	pfnTcHdrBlendAudioStop;
pfnTcHdrBlendAudioRead_t	pfnTcHdrBlendAudioRead;

pfnTcHdrNotificationCallBack_t pfnTcHdrNotificationCallBack;
pfnTcHdrAudioQueueCallBack_t pfnTcHdrAudioQueueCallBack;

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
*           Local constant definitions              *
****************************************************/

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition				*
****************************************************/
void tchdr_configTunerIQ01Driver(S32(*pfnIQ01DrvOpen)(void),
										  S32(*pfnIQ01DrvClose)(void),
										  S32(*pfnIQ01DrvSetParams)(S32 nchannels, S32 nbit, S32 samplerate, S32 buffersize, S32 periodsize),
										  S32(*pfnIQ01DrvStart)(void),
										  S32(*pfnIQ01DrvStop)(void),
										  S32(*pfnIQ01DrvRead)(S8 *data, S32 readsize))
{
	pfnTcHdrIQ01Open =pfnIQ01DrvOpen;
	pfnTcHdrIQ01Close =pfnIQ01DrvClose;
	pfnTcHdrIQ01SetParams =pfnIQ01DrvSetParams;
	pfnTcHdrIQ01Start =pfnIQ01DrvStart;
	pfnTcHdrIQ01Stop =pfnIQ01DrvStop;
	pfnTcHdrIQ01Read =pfnIQ01DrvRead;
}

void tchdr_configTunerIQ23Driver(S32(*pfnIQ23DrvOpen)(void),
										  S32(*pfnIQ23DrvClose)(void),
										  S32(*pfnIQ23DrvSetParams)(S32 nchannels, S32 nbit, S32 samplerate, S32 buffersize, S32 periodsize),
										  S32(*pfnIQ23DrvStart)(void),
										  S32(*pfnIQ23DrvStop)(void),
										  S32(*pfnIQ23DrvRead)(S8 *data, S32 readsize))
{
	pfnTcHdrIQ23Open =pfnIQ23DrvOpen;
	pfnTcHdrIQ23Close =pfnIQ23DrvClose;
	pfnTcHdrIQ23SetParams =pfnIQ23DrvSetParams;
	pfnTcHdrIQ23Start =pfnIQ23DrvStart;
	pfnTcHdrIQ23Stop =pfnIQ23DrvStop;
	pfnTcHdrIQ23Read =pfnIQ23DrvRead;
}

void tchdr_configTunerBlendAudioDriver(S32(*pfnBlendAudioDrvOpen)(void),
										  		S32(*pfnBlendAudioDrvClose)(void),
										  		S32(*pfnBlendAudioDrvSetParams)(S32 nchannels, S32 nbit, S32 samplerate, S32 buffersize, S32 periodsize),
										  		S32(*pfnBlendAudioDrvStart)(void),
										  		S32(*pfnBlendAudioDrvStop)(void),
										  		S32(*pfnBlendAudioDrvRead)(S8 *data, S32 readsize))
{
	pfnTcHdrBlendAudioOpen =pfnBlendAudioDrvOpen;
	pfnTcHdrBlendAudioClose =pfnBlendAudioDrvClose;
	pfnTcHdrBlendAudioSetParams =pfnBlendAudioDrvSetParams;
	pfnTcHdrBlendAudioStart =pfnBlendAudioDrvStart;
	pfnTcHdrBlendAudioStop =pfnBlendAudioDrvStop;
	pfnTcHdrBlendAudioRead =pfnBlendAudioDrvRead;
}

void tchdr_configTcHdrNotificationCallBack(void(*pfnNotificationCallBack)(U32 notifyID, const U32 *pArg, void* const *pData, S32 errorCode))
{
	pfnTcHdrNotificationCallBack = pfnNotificationCallBack;
}

void tchdr_configTcHdrAudioQueueCallBack(void(*pfnAudioQueueCallBack)(void *pOutBuf, S32 frames, U32 samplerate))
{
	pfnTcHdrAudioQueueCallBack = pfnAudioQueueCallBack;
}

