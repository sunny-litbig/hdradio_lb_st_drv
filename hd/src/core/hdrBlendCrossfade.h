/*******************************************************************************
*
* (C) copyright 2003-2016, iBiquity Digital Corporation, U.S.A.
*
********************************************************************************

    This confidential and proprietary software may be used only as
    authorized by a licensing agreement from iBiquity Digital Corporation.
    In the event of publication, the following notice is applicable:

    The availability of this material does not provide any license
    by implication, or otherwise under any patent rights of iBiquity
    Digital Corporation or others covering any use of the
    contents herein.

    Any copies or derivative works must include this and all other
    proprietary notices.

        iBiquity Digital Corporation
        6711 Columbia Gateway Drive, Suite 500
        Columbia, MD USA 21046
*********************************************************************************/
/**
 * @file hdrBlendCrossfade.h
 * @brief Blend Crossfade API
 * @defgroup hdrBlendCrossfade Blend Crossfade API
 * @brief Blend Crossfade API
 * @ingroup HdrApi
 * @{
 *
 * This module contains functions and definitions for the blend crossfade function.
 * This API module is an extension to HD Radio Library and is therefore optional
 * to use.
 */
#ifndef HDR_BLEND_CROSSFADE_H
#define HDR_BLEND_CROSSFADE_H

#include "hdrAudio.h"

/**
 * @brief Required memory for blend crossfade
 *
 * This amount(in bytes) must be allocated and provided to the blend crossfade at initialization.
 */
#define HDR_BLEND_CROSSFADE_MEM_SIZE        (5U * sizeof(uint_t))

/**
 * @brief Defines blend crossfade handle
 */
typedef struct HDR_blend_crossfade_t HDR_blend_crossfade_t;

/**
 * @brief Initializes memory for blend crossfade
 *
 * @param[in] blend_mem: Pointer to memory to be used by the blend crossfade instance.
 *                       The required size of memory is #HDR_BLEND_CROSSFADE_MEM_SIZE bytes.
 * @returns Handle to the blend crossfade instance. NULL indicates a failure.
 */
HDR_blend_crossfade_t* HDR_blend_crossfade_init(void* blend_mem);

/**
 * @brief Configures blend crossfade transition time
 *
 * The new value will take effect on the next HDR_blend_crossfade() execution, in which case
 * the next crossfade will have the new transition time.
 *
 * <b>Note: If the transition time is changed during a crossfade, abnormal behavior may be observed.</b>
 *
 * @param [in] blend_handle: Blend crossfade handle
 * @param [in] transition_time: Duration of blend measured in audio frames(2048 pcm samples)
 * @returns
 *      0 - Success <br>
 *    < 0 - Failure
 */
int_t HDR_set_blend_transition_time(HDR_blend_crossfade_t* blend_handle, uint_t transition_time);

/**
 * @brief Reset blend crossfade to initial state
 *
 * This function will schedule a reset for the next call to #HDR_blend_crossfade().
 * Reset is synchronized with the input function to make reset thread-safe without using
 * an OS mutex.
 *
 * @param[in] blend_handle: Blend crossfade handle
 * @returns
 *        0 - Success <br>
 *      < 0 - Failure
 */
int_t HDR_blend_crossfade_reset(HDR_blend_crossfade_t* blend_handle);

/**
 * @brief Performs audio crossfade to produce blended audio output
 *
 * @param[in] blend_handle: Blend crossfade handle
 * @param[in] dig: Pointer to one audio frame(2048 pcm samples) of digital audio
 * @param[in] ana: Pointer to one audio frame(2048 pcm samples) of analog audio
 * @param[in] blend_flag: false - transition to analog audio;
 *                        true - transition to digital audio
 * @param[out] blended_audio: Address of a pointer provided by the caller. The function will set the pointer
 *                            to the buffer containing blended audio. The blended audio will be stored in
 *                            either analog or digital buffers provided depending on the state of the blend
 *                            crossfade.
 * @returns
 *        0 - Success <br>
 *      < 0 - Failure
 */
int_t HDR_blend_crossfade(HDR_blend_crossfade_t* blend_handle, HDR_pcm_stereo_t* dig, HDR_pcm_stereo_t* ana,
                        bool blend_flag, HDR_pcm_stereo_t** blended_audio);

#endif //BLEND_CROSSFADE_H

/**
 * @} // doxygen end hdrLog
 */
