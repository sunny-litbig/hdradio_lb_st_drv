/*******************************************************************************

*   FileName : tchdr_fader.c

*   Copyright (c) Telechips Inc.

*   Description : Audio fader functions and definitions

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
*        Include 			   					   *
****************************************************/
#include "tchdr_common.h"
#include "tchdr_fader.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
static pfnGetUserMuteStatus_t pfnGetUserMuteStatus;
static pfnGetTuneMuteStatus_t pfnGetTuneMuteStatus;

/***************************************************
*        Imported variable declarations            *
****************************************************/

/***************************************************
*        Imported function declartions             *
****************************************************/

/***************************************************
*        Local preprocessor                        *
****************************************************/

/***************************************************
*        Local type definitions                    *
****************************************************/

/***************************************************
*        Local constant definitions                *
****************************************************/
static stFADER_CTRL_t stFaderCtrl;

/***************************************************
*        Local function prototypes                 *
****************************************************/

/***************************************************
*        function definition			           *
****************************************************/
void tchdrfader_configUserMuteStatus(S32(*getUserMuteStatus)(void))
{
	pfnGetUserMuteStatus = getUserMuteStatus;
}

void tchdrfader_configTuneMuteStatus(S32(*getTuneMuteStatus)(void))
{
	pfnGetTuneMuteStatus = getTuneMuteStatus;
}

static eFADER_MUTE_STS_t tchdrfader_getUserMuteStatus(void)
{
	eFADER_MUTE_STS_t ret = eFADER_MUTE_OFF;
	if(pfnGetUserMuteStatus != NULL) {
		if((*pfnGetUserMuteStatus)() > 0) {
			ret = eFADER_MUTE_ON;
		}
	}
	return ret;
}

static eFADER_MUTE_STS_t tchdrfader_getTuneMuteStatus(void)
{
	eFADER_MUTE_STS_t ret = eFADER_MUTE_OFF;
	if(pfnGetTuneMuteStatus != NULL) {
		if((*pfnGetTuneMuteStatus)() > 0) {
			ret = eFADER_MUTE_ON;
		}
	}
	return ret;
}

void tchdrfader_init(void)
{
	stFaderCtrl.curMuteStatus = eFADER_MUTE_ON;
	stFaderCtrl.prevMuteStatus = eFADER_MUTE_ON;
	stFaderCtrl.parameters.bFaderEn = 1;
	stFaderCtrl.parameters.samplingRate = 44100;
	stFaderCtrl.parameters.mute.fadeoutTime = 100;
	stFaderCtrl.parameters.mute.fadeinTime = 100;
	stFaderCtrl.faderate = 0.0f;
	stFaderCtrl.volumeratio = 0.0f;
	stFaderCtrl.muteFadeinSamples = stFaderCtrl.parameters.mute.fadeinTime * stFaderCtrl.parameters.samplingRate / 1000U;
	stFaderCtrl.muteFadeoutSamples = stFaderCtrl.parameters.mute.fadeoutTime * stFaderCtrl.parameters.samplingRate / 1000U;
}

void tchdrfader_getParams(stFADER_PARAMS_t *params)
{
	(void)(*stOsal.osmemcpy)((void*)params, (void*)(&stFaderCtrl.parameters), (U32)sizeof(stFADER_PARAMS_t));
}

HDRET tchdrfader_setParams(stFADER_PARAMS_t params)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(params.bFaderEn > 0U) {
		stFaderCtrl.parameters.bFaderEn = 1;
		if(params.samplingRate == 44100U) {
			stFaderCtrl.parameters.samplingRate = 44100;
			stFaderCtrl.parameters.mute.fadeoutTime = params.mute.fadeoutTime;
			stFaderCtrl.parameters.mute.fadeinTime = params.mute.fadeinTime;
			stFaderCtrl.muteFadeinSamples = (*stArith.u32mul)(stFaderCtrl.parameters.mute.fadeinTime, stFaderCtrl.parameters.samplingRate) / 1000U;
			stFaderCtrl.muteFadeoutSamples = (*stArith.u32mul)(stFaderCtrl.parameters.mute.fadeoutTime, stFaderCtrl.parameters.samplingRate) / 1000U;
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
	}
	else {
	// If the fader is disabled, the fade in/out samples will be zero and the fader will be zero when the mute turned on/off so that mute works without fader.
		stFaderCtrl.parameters.bFaderEn = 0;
		stFaderCtrl.parameters.mute.fadeoutTime = 0;
		stFaderCtrl.parameters.mute.fadeinTime = 0;
		stFaderCtrl.muteFadeinSamples = 0;
		stFaderCtrl.muteFadeoutSamples = 0;
	}

	return ret;
}

void tchdrfader_process(HDR_pcm_stereo_t *pcmBuf, S32 frames)
{
	S32 count = frames;
	HDR_pcm_stereo_t *pBuf = pcmBuf;

	if((pcmBuf != NULL) && (frames > 0)) {
		stFaderCtrl.curMuteStatus = tchdrfader_getUserMuteStatus();

		if(stFaderCtrl.prevMuteStatus != stFaderCtrl.curMuteStatus) {
			if(stFaderCtrl.curMuteStatus == eFADER_MUTE_OFF) {
				// fade-in
				U32 fadeinSamples = stFaderCtrl.muteFadeinSamples;
				if(fadeinSamples != 0U) {
					stFaderCtrl.faderate = 1.0f / (*stCast.u32tof32)(fadeinSamples);
				}
				else {
					stFaderCtrl.faderate = 0.0f;
					stFaderCtrl.volumeratio = 1.0f;
				}
			}
			else {
				// fade-out
				U32 fadeoutSamples = stFaderCtrl.muteFadeoutSamples;
				if(fadeoutSamples != 0U) {
					stFaderCtrl.faderate = -1.0f / (*stCast.u32tof32)(fadeoutSamples);
				}
				else {
					stFaderCtrl.faderate = 0.0f;
					stFaderCtrl.volumeratio = 0.0f;
				}
			}
			stFaderCtrl.prevMuteStatus = stFaderCtrl.curMuteStatus;
		}

		// process volume control
		while(count > 0) {
			count--;
			if(stFaderCtrl.faderate > 0.0f) {
				// check upper limit of ratio
				stFaderCtrl.volumeratio += stFaderCtrl.faderate;
				if(stFaderCtrl.volumeratio > 1.0f) {
					stFaderCtrl.volumeratio = 1.0f;
					stFaderCtrl.faderate = 0.0f;
				}
			}
			else if(stFaderCtrl.faderate < 0.0f) {
				// check lower limit of ratio
				stFaderCtrl.volumeratio += stFaderCtrl.faderate;
				if(stFaderCtrl.volumeratio < 0.0f) {
					stFaderCtrl.volumeratio = 0.0f;
					stFaderCtrl.faderate = 0.0f;
				}
			}
			else {
				stFaderCtrl.faderate = 0.0f;
			}

			// The float type should not be used the 'equal' and 'not equal' operators.
			if(stFaderCtrl.volumeratio > 0.0f) {
				if(stFaderCtrl.volumeratio < 1.0f) {
					// The current volume is calculated based on the ratio of volume that gradually increases(mute off) or decreases(mute on).
					F32 pcmLeft = (F32)(pBuf->left) * stFaderCtrl.volumeratio;
					F32 pcmRight = (F32)(pBuf->right) * stFaderCtrl.volumeratio;
					pBuf->left = (*stCast.u32tos16)((*stCast.f32tou32)(pcmLeft));
					pBuf->right = (*stCast.u32tos16)((*stCast.f32tou32)(pcmRight));
					pBuf++;
				}
				else {
					// When stFaderCtrl.volumeratio is 1.0f,
					// The while statement breaks because it has recovered to the original volume ratio(1.0).
					break;
				}
			}
			else {
				// When stFaderCtrl.volumeratio is 0.0f,
				// Since the volume ratio is 0.0, write zero in the buffer.
				pBuf->left = 0;
				pBuf->right = 0;
				pBuf++;
			}
		}
	}
}

