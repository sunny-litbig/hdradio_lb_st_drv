/*******************************************************************************

*   FileName : tchdr_bbinput.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio Framework Baseband Input header

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
#ifndef TCHDR_BBINPUT_H__
#define TCHDR_BBINPUT_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/** brief: HD Radio symbol size for FM */
#define SYMBOL_SIZE_FM                  ((U32)2160)   // 1 symbol = 2160 samples

/** brief: HD Radio symbol size for FM */
#define SYMBOL_SIZE_AM                  ((U32)270)    // 1 symbol = 270 samples

/** brief: Number of symbols processed by HDR library at one time(FM) */
#define FM_BB_NUM_INPUT_SYMBOLS         ((U32)16)

/** brief: Number of symbols processed by HDR library at one time(AM) */
#define AM_BB_NUM_INPUT_SYMBOLS         ((U32)8)

#define BB_INPUT_SYMBOL_SIZE            (SYMBOL_SIZE_FM * 2U)    // 4320 samples

/** brief: Input baseband sample buffer size in IQ samples(FM) */
#define HDR_FM_BB_INPUT_BUFFER_SIZE     (FM_BB_NUM_INPUT_SYMBOLS * SYMBOL_SIZE_FM)	// (16 x 2160) = 34560 samples

/** brief: Input baseband sample buffer size in IQ samples(AM) */
#define HDR_AM_BB_INPUT_BUFFER_SIZE		(AM_BB_NUM_INPUT_SYMBOLS * SYMBOL_SIZE_AM)	// (8 x 270) = 2160 samples

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
extern stCIRC_BUFF_t analogAudioBuffer;

/***************************************************
*			Function declaration				*
****************************************************/
/**
 * brief: Initializes baseband input module
 * param[in] options: run options extracted from command line arguments
 * returns
 *     0 - Success<br>
 *    -1 - Failure
 */
extern HDRET tchdrbbinput_init(void);
extern HDRET tchdrbbinput_deinit(void);
extern HDRET tchdrbbinput_open(void);
extern HDRET tchdrbbinput_close(void);
extern S32 tchdriqinput_getReadySemaValue(void);
extern void tchdriqinput_ready(void);

/**
 * brief: Baseband input thread function
 * param arg: not used
 */
extern void *tchdr_bbInputThread(void* arg);

/**
 * brief: Retrieves specified number of samples from baseband output buffer
 * param[out] bbSamples: Ouput buffer for baseband sampels
 * param[in] numSamples: Number of samples to read
 * param[in] instanceNum: Instance number of the baseband samples
 * returns Number of samples read.
 */
extern S32 tchdrbbinput_getSamples(int16c_t* bbSamples, U32 numSamples, U32 instanceNum);
extern S32 tchdrbbinput_getSamplesValid(U32 instanceNum);
extern void tchdrbbinput_resetSamples(U32 instanceNum);
extern S32 tchdrbbinput_fillSamples(S8 value, U32 numElems, U32 instanceNum);


extern HDRET tchdraudinput_init(void);
extern HDRET tchdraudinput_close(void);

extern HDRET tchdriqinput_init(void);
extern HDRET tchdriqinput_close(void);

extern void *tchdr_audioInputThread(void* arg);
extern void *tchdr_rfIqInputThread(void * arg);

extern F64 tchdraudinput_setResamplerSlips(F64 ppm);
extern void tchdraudinput_getResamplerSlips(F64 *ppm, F64 *out_hz);

#ifdef __cplusplus
}
#endif

#endif //TCHDR_BBINPUT_H__
