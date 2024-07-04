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
 * @file hdrAudioResampler.h
 * @brief Audio Resampler API
 * @defgroup hdrAudioResampler Audio Resampler API
 * @brief Audio Resampler API
 * @ingroup HdrApi
 * @{
 *
 * This asynchronous audio sample rate converter is designed to match analog audio that's based on
 * the local clock and digital audio that is locked to transmitters clock.
 * Current design can correct up to +-150 PPM drift.
 *
 * This API module is an extension to HD Radio Library and is therefore optional
 * to use.
 */
#ifndef HDR_AUDIO_RESAMPLER_H_
#define HDR_AUDIO_RESAMPLER_H_

#include "hdrCore.h"

/**
 * @brief Required memory for audio resampler
 *
 * This amount(in bytes) must be allocated and provided to the resampler at initialization.
 */
#define HDR_AUDIO_RESAMPLER_MEM_SIZE    (84000)

/**
 * @brief Defines resampler handle
 */
typedef struct HDR_audio_resampler_t HDR_audio_resampler_t;

/**
 * @brief Audio resampler modes of operation
 */
typedef enum {
   HDR_BB_SAMPLE_SLIPS_CORRECTION,  /**< Resampling is driven by baseband sample slips provided to the resampler */
   HDR_BUFFER_LEVEL_MONITORING      /**< Automatically resamples the audio stream to match output rate by maintaining buffer level */
}HDR_audio_resampler_mode_t;

/**
 * @brief Initialize audio resampler instance
 *
 * @param[in] audio_resampler_memory: Pointer to memory to be used by the audio resampler instance.
 *                    The required size of memory is #HDR_AUDIO_RESAMPLER_MEM_SIZE bytes.
 * @param[in] mode: Audio resampler modes of operation
 * @param[in] mutex: Allows thread-safe operation. Can be set to NULL if thread safety is not required.
 * @returns Handle to the audio resampler instance. NULL indicates a failure.
 */
HDR_audio_resampler_t* HDR_audio_resampler_init(void* audio_resampler_memory, HDR_audio_resampler_mode_t mode, void* mutex);

/**
 * @brief Reset audio resampler to initial state
 *
 * This function will schedule a reset for the next call to #HDR_audio_resampler_input().
 * Reset is synchronized with the input function to make reset thread-safe without using
 * an OS mutex.
 *
 * All buffers will be flushed and the memory will be returned to initial state.
 *
 * @param[in] audio_resampler: Audio resampler handle
 * @returns
 *        0 - Success <br>
 *      < 0 - Failure
 */
int_t HDR_audio_resampler_reset(HDR_audio_resampler_t* audio_resampler);

/**
 * @brief Provides recent baseband sample slip information to the audio resampler
 *
 * @param[in] audio_resampler:  Audio resampler handle
 * @param[in] bb_sample_slips:  Number of baseband sample (186.046875 kHz FM / 46.51171875 kHz AM) slips since the last call to this function
 * @param[in] band:             Current band of the HDR instance, determines the conversion ratio of baseband samples to audio samples.
 * @param[in] clk_offset:       PPM offset between digital rx and analog rx clocks, Q16.16
 *                              Examples:
 *                                  0x0001_0000 = digital RX clock one ppm faster than analog RX clock
 *                                  0x0000_4000 = digital RX clock 1/4 ppm faster
 *                                  0xffff_0000 = digital RX clock one ppm slower
 *                                  0xffff_C000 = digital RX clock 1/4 ppm slower
 *
 * @param[in] ppm_est:          Parts per million(PPM) difference between transmitter clock and local clock.
 *                              #HDR_get_clock_offset() function in hdrPhy.h, retrieves the ppm estimate.
 * @returns
 *        0 - Success <br>
 *      < 0 - Failure
 */
#ifdef USE_HDRLIB_2ND_CHG_VER
int32_t HDR_audio_resample_update_slips(HDR_audio_resampler_t* audio_resampler, int32_t bb_sample_slips, HDR_tune_band_t band, int32_t clk_offset, int32_t ppm_est);
#else
int_t HDR_audio_resample_update_slips(HDR_audio_resampler_t* audio_resampler, int_t bb_sample_slips, HDR_tune_band_t band, int_t ppm_est);
#endif
/**
 * @brief Places input samples in the internal buffer
 *
 * @param[in] audio_resampler: Audio resampler handle
 * @param[in] input: Audio samples buffer. One audio frame of samples(2048) is expected.
 * @returns
 *       0 - Success <br>
 *     < 0 - Failure
 */
int_t HDR_audio_resampler_input(HDR_audio_resampler_t* audio_resampler, HDR_pcm_stereo_t* input);

/**
 * @brief Provides resampled audio output
 *
 * @param[in]  audio_resampler: Audio resampler handle
 * @param[out] output: Output buffer(2048 pcm samples) for the resampled audio. Must be allocated by the caller.
 * @returns
 *       0 - Success <br>
 *      -1 - Failure - resampler is about to reset; there is no output available
 *           a reset is triggered by a call to HDR_audio_resampler_reset buffer,
 *           or a resampler buffer overflow or underflow condition
 *      -2 - (ONLY IF MODE=HDR_BUFFER_LEVEL_MONITORING)
 *           Indicates that the resampler has reset and the audio frame is not valid. This condition should not happen in normal circumstances.
 *           If this condition is encountered, that means the resampler is being called at a wrong wrate.
 */
int_t HDR_audio_resampler_output(HDR_audio_resampler_t* audio_resampler, HDR_pcm_stereo_t* output);

#ifdef USE_HDRLIB_2ND_CHG_VER
/**
 * @brief Set the src offset
 *
 * @param[in]  audio_resampler: Audio resampler handle
 * @param[in]  srcOffset: clock offset due to fixed point operation of bb src, Q16.16
 */
void HDR_audio_resampler_set_src_offset(HDR_audio_resampler_t* audio_resampler, int32_t srcOffset);

/**
 * @brief Get number of audio samples held in the audio resampler buffer
 *
 * @param[in]  audio_resampler: Audio resampler handle
 * @returns    number of audio samples held in the audio resampler buffer
 */
uint32_t HDR_audio_resampler_avail_data(HDR_audio_resampler_t* audio_resampler);
#endif

#endif /* _HDR_AUDIO_RESAMPLER_H_ */
/** @} */
