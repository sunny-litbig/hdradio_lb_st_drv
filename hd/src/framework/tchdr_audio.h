/*******************************************************************************

*   FileName : tchdr_audio.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio Framework Audio Output header

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
#ifndef TCHDR_AUDIO_H__
#define TCHDR_AUDIO_H__

/***************************************************
*				Include					*
****************************************************/
#include "hdrBasicTypes.h"

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

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern stCIRC_BUFF_t digitalAudioBuffer;

/***************************************************
*			Function declaration				*
****************************************************/
/**
 * brief Initializes audio playback module
 * returns
 *     0 - Success<br>
 *    -1 - Failure
 */
extern HDRET tchdraudoutput_init(void);
extern HDRET tchdraudoutput_deinit(void);
extern HDRET tchdraudoutput_open(void);
extern HDRET tchdraudoutput_close(void);
extern S32 tchdraudoutput_getReadySemaValue(void);
extern void tchdraudoutput_ready(void);
extern void tchdraudoutput_setTuneMute(U32 fOnOff);
extern F64 tchdraudoutput_setResamplerSlips(F64 ppm);
extern void tchdraudoutput_getResamplerSlips(F64 *ppm, F64 *out_hz);

/**
 * brief Writes 2048 audio samples to audio playback
 * param[in] audioFrame: Pointer to the audio samples buffer
 */
extern void tchdraudoutput_rxHandler(HDR_pcm_stereo_t* audioFrame);

/**
 * brief Audio playback execution thread
 * param arg: not used
 */
extern void *tchdr_audioPlaybackThread(void* arg);

#ifdef __cplusplus
}
#endif

#endif /* TCHDR_AUDIO_H__ */
