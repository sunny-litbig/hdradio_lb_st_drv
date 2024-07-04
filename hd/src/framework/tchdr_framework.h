/*******************************************************************************

*   FileName : tchdr_framework.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio framework threads header

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
#ifndef TCHDR_FRAMEWORK_H__
#define TCHDR_FRAMEWORK_H__

/***************************************************
*				Include					*
****************************************************/
#include "hdrAudio.h"
#include "hdrAudioResampler.h"
#include "hdrBlendCrossfade.h"
#include "hdrAutoAlign.h"

#include "tchdr_bytestream.h"

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
*				Enumeration				*
****************************************************/
/** Different audio output modes */
typedef enum {
    eHDR_AUDIO_BLEND,         /**< normal operation, follows blend flag from HDR library */
    eHDR_AUDIO_ANALOG_SPLIT,  /**< test mode for tesing alignment between HD and analog */
    eHDR_AUDIO_ANALOG_ONLY,   /**< test mode; force to analog */
    eHDR_AUDIO_DIGITAL_ONLY  /**< test mode; force to digital */
}eHDR_AUDIO_MODE_t;

/***************************************************
*				Typedefs					*
****************************************************/
/** Example framework data structure that contains all the elements for comlete HD Radio operation */
typedef struct {
    HDR_instance_t hdrInstance[NUM_HDR_INSTANCES];
	HDBOOL busyFlag[NUM_HDR_INSTANCES];
    HDBOOL playAlertTone;
    HDR_blend_crossfade_t* blendCrossfade;
    HDR_audio_resampler_t* hdaoutResampler;
    HDR_auto_align_t* autoAlign;
    HDBOOL digitalAudioAcquired;
    HDBOOL alignmentSuccess;
#ifdef USE_HDRLIB_2ND_CHG_VER
	HDR_auto_align_rc_t aaaState;
#endif
	HDBOOL digitalAudioStarted; // Flag to indicate start of digital audio playback. Used to correct for sample slips.
    HDR_pcm_stereo_t digitalAudio[HDR_AUDIO_FRAME_SIZE];
    HDR_pcm_stereo_t analogAudio[HDR_AUDIO_FRAME_SIZE];
    eHDR_AUDIO_MODE_t audioMode;
}stHDR_FRAMEWORK_DATA_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern stHDR_BYTE_STREAM_t loggerByteStream;

/***************************************************
*			Function declaration				*
****************************************************/
stHDR_FRAMEWORK_DATA_t *tchdrfwk_getDataStructPtr(void);
HDR_instance_t *tchdrfwk_getHdrInstancePtr(eTC_HDR_ID_t id);
void tchdrfwk_setHdrType(U32 type);
U32 tchdrfwk_getHdrType(void);
void tchdrfwk_setNumOfHdrInstance(U32 num);
U32 tchdrfwk_getNumOfHdrInstance(void);
HDRET tchdrfwk_close(void);
HDRET tchdrfwk_open(void);
HDBOOL tchdrfwk_getMrcStatus(void);
HDBOOL tchdrfwk_getBsMrcStatus(void);
void tchdrfwk_setAnalogAudioMute(U32 fOnOff);
HDBOOL tchdrfwk_getDigitalAudioAcquired(const HDR_instance_t* hdr_instance);
void tchdraudinput_ready(void);
S32 tchdraudinput_getReadySemaValue(void);
S32 tchdrbbinput_getReadySemaValue(U32 instanceNum);
void tchdrblending_setAAMute(HDBOOL fOnOff);
HDBOOL tchdrblending_getAAMute(void);


/**
 * brief Initializes HD Radio framework
 * param[in] options: run options extracted from command line arguments
 * returns
 *     0 - Success<br>
 *    -1 - Failure
 */
HDRET tchdrfwk_init(void);

/**
 * brief Signals HD Radio threads to run HDR_exec()
 * param[in] instanceNum: HDR instance number
 */
void tchdrbbinput_ready(U32 instanceNum);

/**
 * brief HDR execution thread for instance 1
 *
 * If multi-instance non-threaded configuration is enabled, it executes all isntances sequentially
 *
 * param arg: not used
 */
void *tchdr_mainThread(void* arg);

#if (HDR_CONFIG == HDR_1p0_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG)
/**
 * brief HDR execution thread for instance 2
 *
 * Only used for multi-instance configuration with threaded execution enabled
 *
 * param arg: not used
 */
void *tchdr_mrcThread(void* arg);
#endif

#if (HDR_CONFIG == HDR_1p5_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG)
/**
 * brief HDR execution thread for instance 3
 *
 * Only used for multi-instance configuration with threaded execution enabled
 *
 * param arg: not used
 */
void *tchdr_bsThread(void* arg);
#endif

/**
 * brief HDR audio blending thread function
 * param arg: not used
 */
void *tchdr_audioBlendingThread(void* arg);

/**
 * brief Command processor thread function
 * param arg: not used
 */
void *tchdr_cmdProcThread(void* arg);

/**
 * brief HDR library logger thread function
 * param arg: not used
 */
void *tchdr_loggerReaderThread(void *arg);

#ifdef DEBUG_ENABLE_TRACE_THREAD
void *tchdr_traceReaderThread(void *arg);
#endif

#ifdef __cplusplus
}
#endif

#endif /* TCHDR_FRAMEWORK_H__ */
