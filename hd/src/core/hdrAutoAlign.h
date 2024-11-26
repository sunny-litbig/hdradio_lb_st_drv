/*******************************************************************************
*
* (C) copyright 2014 - 2018, iBiquity Digital Corporation, U.S.A.
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
*******************************************************************************/
/**
 * @file hdrAutoAlign.h
 * @brief Automatic Audio Alignment API
 * @defgroup hdrAutoAlign Automatic Audio Alignment API
 * @brief Automatic Audio Alignment(AAA) API
 * @ingroup HdrApi
 * @{
 *
 * Automatic audio alignment allows the HD Radio receiver to compensate for individual broadcast stations that
 * improperly configured the delay and relative level between digital and analog audio.
 *
 * This API module is an extension to HD Radio Library and is therefore optional
 * to use.
 */
#ifndef HDR_AUTO_ALIGN_H
#define HDR_AUTO_ALIGN_H

#include "hdrCore.h"

/**
 * @brief Required memory for the AAA
 *
 * This amount(in bytes) must be allocated and provided at initialization.
 */
#define HDR_AUTO_ALIGN_MEM_SIZE        (265000)

/**
 * @brief Maximum audio sample offset that can be detected by the AAA
 */
#define HDR_AUTO_ALIGN_MAX_RANGE        (45568U)

/**
 * @brief Defines AAA handle type
 */
typedef struct HDR_auto_align_t HDR_auto_align_t;

/**
 * @brief AAA configuration parameters
 *
 * AAA settings have no effect on the size requirements.
 */
typedef struct {
    bool am_auto_time_align_enabled;  /**< Enable/Disable AM time auto-alignment calculation */
    bool fm_auto_time_align_enabled;  /**< Enable/Disable FM time auto-alignment calculation */
    bool am_auto_level_align_enabled; /**< Enable/Disable AM level auto-alignment calculation */
    bool fm_auto_level_align_enabled; /**< Enable/Disable FM level auto-alignment calculation */

    /**
     * Enable/Disable adjustment of level alignment inside the routine.
     * If enabled, AAA will scale(in-place) digital audio frame to match analog when
     * misalignment is successfully determined.
     */
#ifdef USE_HDRLIB_3RD_CHG_VER
	bool am_auto_level_correction_enabled;
    bool fm_auto_level_correction_enabled;
#else
    bool apply_level_adjustment;
#endif
}HDR_auto_align_config_t;

/**
 * @brief Audio Sampling Rate
 * Enum type to determine the rate of analog audio being input to the AAA algorithm
 * We support 2 Sampling Rates : 44.1 Khz and 48Khz . We dont support 48 Khz , Meant for future use.
 */
typedef enum {
    HDR_AUDIO_SAMPLING_RATE_44KHz,
    HDR_AUDIO_SAMPLING_RATE_48KHz
}HDR_audio_rate_t;

/**
 * @brief Initializes AAA instance
 *
 * @param[in] auto_align_memory: Pointer to memory to be used by the AAA instance.
 * @param[in] size: Size(in bytes) of the memory buffer. Should be at least #HDR_AUTO_ALIGN_MEM_SIZE.
 * @param[in] config: Pointer to configuration data structure.
 * @param[in] audio_rate: Sample Rate for the audio sources to be aligned.
 *
 * @returns Handle to the AAA instance. NULL indicates a failure.
 */
HDR_auto_align_t* HDR_auto_align_init(void* auto_align_memory, uint_t size, const HDR_auto_align_config_t* config, HDR_audio_rate_t audio_rate);

/**
 * @brief Resets and changes band configuration of the AAA
 *
 * @param[in] auto_align: Handle to the AAA instance
 * @param[in] band: Specifies AM or FM operation.
 *
 * @returns
 *        0 - Success <br>
 *      < NULL - Failure
 */
int_t HDR_auto_align_reset(HDR_auto_align_t* auto_align, HDR_tune_band_t band);

/**
 * @brief AAA status data structure
 */
typedef struct {
    /**
     * Specifies whether AAA successfully found both time and level alignment difference between digital and analog audio streams
     * The rest of parameters are invalid until this flag is set to true.
     */
    bool alignment_found;
    /** Specifies whether digital and analog audio are phase-inverted with respect to each other */
    bool phase_inverted;
    /** First succesful time offset(in audio samples) calculated by the AAA before any adjustments were made */
    int_t time_offset;
    /** First succesful level offset(dB Q8.8) calculated by the AAA before any adjustments were made */
    int_t level_offset;
    /** Confidence level of the AAA time alignment. Value ranges from 0 to 1.0 */
    hdr_float32_t confidence_level;
}HDR_auto_align_status_t;

/**
 * @brief Retrieves AAA status information
 *
 * @param[in] auto_align: Handle to the AAA instance
 * @param[out] status: Pointer to the output status data structure. Must be allocated by the caller.
 * @returns
 *        0 - Success <br>
 *      < 0 - Failure
 */
int_t HDR_auto_align_get_status(const HDR_auto_align_t* auto_align, HDR_auto_align_status_t* status);

/**
 * @brief Sets the AAA configuration parameters
 *
 * @param[in] auto_align: Handle to the auto-alignment instance
 * @param[in] config: Pointer to the new configuration data structure
 * @returns
 *        0 - Success <br>
 *      < 0 - Failure
 */
int_t HDR_auto_align_set_config(HDR_auto_align_t* auto_align, const HDR_auto_align_config_t* config);

/**
 * @brief Retrieves AAA configuration parameters
 *
 * @param[in] auto_align: Handle to the AAA instance
 * @param[out] config: Pointer to the output configuration parameters. Must be allocated by the caller.
 * @returns
 *        0 - Success <br>
 *      < 0 - Failure
 */
int_t HDR_auto_align_get_config(const HDR_auto_align_t* auto_align, HDR_auto_align_config_t* config);

/**
 * @brief AAA return conditions
 */
typedef enum {
    HDR_AUTO_ALIGN_ERROR = -1,            /**< Error occurred  */
    HDR_AUTO_ALIGN_DISABLED,              /**< Time and level auto alignment is disabled */
    HDR_AUTO_ALIGN_OFFSET_SEARCH,         /**< Still searching */
    HDR_AUTO_ALIGN_APPLY_OFFSET,          /**< Alignment found; caller should apply it on this frame */
    HDR_AUTO_ALIGN_WAIT_FOR_BLEND_FLAG,   /**< Audio streams should be aligned; waiting for blend flag */
    HDR_AUTO_ALIGN_PLAY_DIGITAL           /**< Steady state; continue calculating offsets */
}HDR_auto_align_rc_t;

/**
 * @brief Runs AAA on the current digital and analog audio frames
 *
 * If #HDR_auto_align_config_t::apply_level_adjustment was set to true at initialization, AAA will scale digital audio to match analog automatically
 * once the misalignment has been calculated.
 *
 * @param[in] auto_align: Handle to the AAA instance
 * @param[in] dig_audio: Pointer to digital audio input samples buffer. Must be 2048 samples long.
 * @param[in] ana_audio: Pointer to analog audio samples buffer. Must be 2048 samples long.
 * @param[in] audio_quality: Audio quality of the current digital audio frame.
 * @param[in] blend_flag: Blend flag status associated with this audio frame.
 * @param[in] blend_alignInProg: Blend alignment progress status. Used to correctly apply the auto alignment offset.
 * @param[out] time_offset: Time offset(in audio samples) calculated by the AAA. Must be allocated by the caller.
 * @param[out] level_offset: Level offset(dB Q8.8) calculated by the AAA. Must be allocated by the caller.
 * @returns
 *    #HDR_AUTO_ALIGN_ERROR               - error occurred; AAA is not searching <br>
 *    #HDR_AUTO_ALIGN_OFFSET_SEARCH       - AAA is searching for time and level offset <br>
 *    #HDR_AUTO_ALIGN_APPLY_OFFSET        - time and audio offsets were found; framework should apply
 *                                          the time offset using #HDR_blend_adjust_audio_delay(). <br>
 *    #HDR_AUTO_ALIGN_WAIT_FOR_BLEND_FLAG - audio streams are aligned; wait for blend flag before playing digital. <br>
 *    #HDR_AUTO_ALIGN_PLAY_DIGITAL        - start/continue playing digital
 */
HDR_auto_align_rc_t HDR_auto_align_exec(HDR_auto_align_t* auto_align, HDR_pcm_stereo_t* ana_audio, HDR_pcm_stereo_t* dig_audio,
                                        uint_t audio_quality, bool blend_flag, bool blend_alignInProg, int_t* time_offset, int_t* level_offset);

#ifdef USE_HDRLIB_2ND_CHG_VER
/**
 * @brief Sets the AAA holdoff duration
 *
 * Once AAA finds the time alignment offset, it is necessary to allow any decoded
 * digital audio that is buffered to play out to avoid any artifacts during the
 * blending process.
 *
 * @param[in] auto_align: Handle to the AAA instance
 * @param[in] holdoff: Number of audio samples to holdoff
 */
void HDR_auto_align_set_holdoff(HDR_auto_align_t* auto_align, uint32_t holdoff);
#endif

#endif //HDR_AUTO_ALIGN_H

/** @} */
