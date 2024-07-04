/*******************************************************************************

*   FileName : tchdr_fader.h

*   Copyright (c) Telechips Inc.

*   Description : Audio fader header

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
#ifndef TCHDR_FADER_H__
#define TCHDR_FADER_H__

/***************************************************
*               Include                            *
****************************************************/

/***************************************************
*               Defines                            *
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
*               Enumeration                        *
****************************************************/
typedef enum {
	eFADER_OP_MODE_NORMAL,	// Normal Mute
	eFADER_OP_MODE_TUNE		// Tune Mute
}eFADER_OP_MODE_t;

typedef enum {
	eFADER_MUTE_OFF,
	eFADER_MUTE_ON
}eFADER_MUTE_STS_t;

/***************************************************
*               Typedefs                           *
****************************************************/
typedef struct {
	U32 fadeoutTime;
	U32 fadeinTime;
}stFADER_TIME_t;

typedef struct {
	U32 bFaderEn;
	U32 samplingRate;
	stFADER_TIME_t mute;
}stFADER_PARAMS_t;

typedef struct {
	eFADER_OP_MODE_t mode;
	eFADER_MUTE_STS_t curMuteStatus;
	eFADER_MUTE_STS_t prevMuteStatus;
	stFADER_PARAMS_t parameters;

	F32 faderate;
	F32 volumeratio;

	U32 muteFadeinSamples;
	U32 muteFadeoutSamples;
	U32 tuneFadeinSamples;
	U32 tuneFadeoutSamples;
}stFADER_CTRL_t;

typedef S32(*pfnGetUserMuteStatus_t)(void);
typedef S32(*pfnGetTuneMuteStatus_t)(void);

/***************************************************
*               Constant definitions               *
****************************************************/

/***************************************************
*               Variable definitions               *
****************************************************/

/***************************************************
*               Function declaration               *
****************************************************/
extern void tchdrfader_configUserMuteStatus(S32(*getUserMuteStatus)(void));
extern void tchdrfader_configTuneMuteStatus(S32(*getTuneMuteStatus)(void));
extern void tchdrfader_init(void);
extern HDRET tchdrfader_setParams(stFADER_PARAMS_t params);
extern void tchdrfader_getParams(stFADER_PARAMS_t *params);
extern void tchdrfader_process(HDR_pcm_stereo_t *pcmBuf, S32 frames);

#ifdef __cplusplus
}
#endif

#endif
