/*******************************************************************************

*   FileName : tcaud_resampler.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio Framework Audio Output functions and definitions

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
#ifndef TCAUD_RESAMPLER__H__
#define TCAUD_RESAMPLER__H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define TC_AUDIO_RESAMPLER_MEM_SIZE        (16U * sizeof(S32))

/***************************************************
*				Enumeration				*
****************************************************/

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct stTC_AUDIO_RESAMPLER_t stTC_AUDIO_RESAMPLER_t;				// audio resampler handler definition

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern stTC_AUDIO_RESAMPLER_t* tcaudio_resampler_init(void *audio_src_memory);
extern S32 tcaudio_resampler_exec(stTC_AUDIO_RESAMPLER_t *handler, const S16 *pIn, S16 *pOut, S32 samples, F64 in_hz, F64 out_hz);

#ifdef __cplusplus
}
#endif

#endif	// #define TCAUD_RESAMPLER__H__
