/*******************************************************************************
*
* (C) copyright 2003-2018, iBiquity Digital Corporation, U.S.A.
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
 * @file hdrBlend.h
 * @brief Blend API Definitions and Functions
 * @defgroup hdrBlend Blend
 * @brief Blend API
 * @ingroup HdrApi
 * @{
 *
 * Blend is a transition from a digital to analog audio or vice versa, where the level of one signal is gradually
 * increased while the level of the other signal is simultaneously decreased. HD Radio receiver blends when digital audio
 * signal becomes available or ceases to be available. At the transmitter, the analog audio is delayed by a fixed time
 * relative to the digital audio to allow the receiver time to process the digital signal. Seamless blending is accomplished
 * by synchronizing the digital to the delayed analog audio to within 3 audio samples(68uS).
 *
 * All digital broadcast systems have an edge of coverage and therefore blends cannot be eliminated. However, frequent
 * blends can lead to poor user experience so it is important to minimize the number of blends when the signal conditions
 * are poor.
 *
 * This module provides an API for configuring and controlling blend functionality of the HD Radio.
 *
 * <b>For additional information see:</b><br>
 *    RX_IDD_2206 - Baseband Processor Command and Data Interface Definition - Revision<X>.pdf
 */
#ifndef HDR_BLEND_H_
#define HDR_BLEND_H_

#include "hdrAudio.h"

/**
 * @brief Blend threshold configuration
 *
 * The blend threshold is a means of controlling the audio quality level at which the transition
 * should occur between digital and analog audio signal.
 *
 * With blend threshold Q1 selected, the receiver will conservatively blend to analog
 * at the first indication of digital audio imperfection, limiting the digital coverage
 * area but ensuring a listening experience that is virtually free of channel-induced
 * digital audio artifacts. Conversely, blend threshold Q4 will result in a more aggressive
 * blend strategy in which the digital coverage area is maximized, but the likelihood of
 * audible artifacts is increased as well.
 */
typedef enum{
    HDR_BLEND_FORCE_ANALOG,  /**< No blending - always output analog audio */
    HDR_BLEND_Q1_THRESH,     /**< Blend with virtually no audible artifacts (most conservative blending) */
    HDR_BLEND_Q2_THRESH,     /**< Blend with rare audible artifacts */
    HDR_BLEND_Q3_THRESH,     /**< Blend with infrequent audible artifacts */
    HDR_BLEND_Q4_THRESH,     /**< Blend with noticeable audible artifacts (most aggressive blending) */
    HDR_BLEND_RESERVED1,     /**< N/A */
    HDR_BLEND_RESERVED2,     /**< N/A */
    HDR_BLEND_FORCE_DIGITAL, /**< No blending - always output digital audio */
    HDR_NUM_BLEND_THRESH
}HDR_blend_thresh_sel_t;

/**
 * @brief Blend configuration parameters
 */
typedef struct HDR_blend_params_t {
    /**
     * @brief Blend threshold for FM MPS program
     *
     * Sets the threshold for determining when to blend between digital audio and analog audio
     * for FM Hybrid MPS.
     */
    HDR_blend_thresh_sel_t fm_mps_blend_thresh;

    /**
     * @brief Blend threshold for FM all digital programs
     *
     * Sets the threshold for determining when to blend between digital audio and mute for FM SPS
     * programs as well as FM All Digital MPS programs.
     */
    HDR_blend_thresh_sel_t fm_all_dig_blend_thresh;
    /**
     * @brief FM Hybrid MPS fine audio level alignment
     *
     * Used to align the digital and analog audio levels for FM Hybrid MPS. Digital audio will be scaled down
     * to the level specified by this parameter, where 65335 is a factor of 1. Value ranges from 0 to 65535.
     */
    uint_t fm_mps_audio_scaling;
    /**
     * @brief All digital programs audio scaling
     *
     * Used to scale down the digital audio signal for FM SPS programs as well as FM All
     * Digital MPS programs. Digital audio will be scaled down to the level specified by this parameter,
     * where 65335 is a factor of 1. Value ranges from 0 to 65535.
     */
    uint_t fm_all_dig_audio_scaling;
    /**
     * @brief Controls the step size of the FM analog hold duration
     *
     * This parameter configures the hysteresis in the blending process. It controls the step size of
     * the analog hold duration. If the state of the blend line is analog, the blend line cannot
     * transition to digital again until the digital audio quality remains good for the full period of the
     * analog hold duration.
     *
     * The analog hold duration (in seconds) increases by the value of the Blend Rate parameter (X) each time
     * a blend from digital to analog occurs within the current analog hold duration. The analog hold duration
     * is reset to (1.1*X) seconds after the audio output remains digital for longer than 10*X seconds.
     * The maximum analog hold duration (in seconds) is (1.1*X) + 5*X. That is, the hold time is incremented
     * a maximum of 5 times.
     *
     * <pre>
     * For example a blend rate of 0x05:
     *  Step 1 = 5.5 seconds
     *  Step 2 = 10.5 seconds
     *  Step 3 = 15.5 seconds
     *  Step 4 = 20.5 seconds
     *  Step 5 = 25.5 seconds
     *  Step 6 = 30.5 seconds
     * </pre>
     *
     * Value ranges from 3 to 6. Recommended value is 3.
     */
    uint_t fm_mps_blend_rate;
    /**
     * @brief Controls the step size of the FM mute hold duration
     *
     * Controls the minimum amount of time that the audio output remains muted after loss of digital audio signal.
     * This applies to FM SPS programs as well as FM All Digital MPS programs.
     *
     * Allowed values are 1 and 3 to 6. Recommended value is 1.
     *
     * @see #fm_mps_blend_rate
     */
    uint_t fm_all_dig_blend_rate;
    /**
     * @brief FM Hybrid MPS digital audio delay
     *
     * Used to perform fine time alignment between digital audio and analog audio, to ensure
     * smooth blending. This parameter is utilized for blending whenever FM Hybrid MPS and the primary
     * sample rate are selected.
     */
    uint_t fm_mps_dig_audio_delay;
    /**
     * @brief Blend threshold for AM MPS program
     *
     * Sets the threshold for determining when to blend between digital audio and analog audio
     * for AM Hybrid MPS.
     */
    HDR_blend_thresh_sel_t am_mps_blend_thresh;
    /**
     * @brief Blend threshold for AM all digital programs
     *
     * Sets the threshold for determining when to blend between digital audio and mute for AM SPS
     * programs as well as AM All Digital MPS programs.
     */
    HDR_blend_thresh_sel_t am_all_dig_blend_thresh;
    /**
     * @brief AM Hybrid MPS fine audio level alignment
     *
     * Used to align the digital and analog audio levels for AM Hybrid MPS. Digital audio will be scaled down
     * to the level specified by this parameter, where 65335 is a factor of 1. Value ranges from 0 to 65535.
     */
    uint_t am_mps_audio_scaling;
    /**
     * @brief AM all digital programs audio scaling
     *
     * Used to scale down the digital audio signal for AM SPS programs as well as AM All
     * Digital MPS programs. Digital audio will be scaled down to the level specified by this parameter,
     * where 65335 is a factor of 1. Value ranges from 0 to 65535.
     */
    uint_t am_all_dig_audio_scaling;
    /**
     * @brief AM Hybrid MPS digital audio delay
     *
     * Used to perform fine time alignment between digital audio and analog audio, to ensure
     * smooth blending. This parameter is utilized for blending whenever AM Hybrid MPS and the primary
     * sample rate are selected. Value ranges from 0 to 16383 audio samples.
     */
    uint_t am_mps_dig_audio_delay;
    /**
     * @brief Controls the step size of the AM analog hold duration
     *
     * Value ranges from 3 to 6. Recommended value is 3.
     *
     * @see #fm_mps_blend_rate
     */
    uint_t am_mps_blend_rate;
    /**
     * @brief Controls the step size of the AM mute hold duration
     *
     * Allowed values are 1 and 3 to 6. Recommended value is 1.
     *
     * @see #fm_mps_blend_rate
     */
    uint_t am_all_dig_blend_rate;
    /**
     * @brief Digital-to-Analog Blend Hold-off
     *
     * Controls the delay(ranges 4 to 21 audio frames) of the digital audio samples
     * by adjusting the size of the blend delay buffer. Blend delay buffer is used to ensure
     * enough good audio frames are available for "smooth" blending if digital audio is lost.
     *
     * The maximum hold-off for FM is 18 audio frames; any value exceeding this maximum is limited to 18.
     *
     * <b>This parameter is write-protected and can be set during idle mode only.</b>
     */
    uint_t d2a_blend_holdoff;
    /**
     * @brief Analog to digital blend look ahead
     *
     * Blend Decision is the ability of the system to look ahead into the future of the incoming signal and make some
     * decisions about the feasibility of having a good signal for audio decoding. This hysteresis prevents the
     * rapid/frequent blending by requiring the system to have a more stable signal condition.
     *
     * In addition to the audio quality, blend threshold, and blend rate, blend decision takes into account the Cd/No
     * values on consecutive audio frames. Normal blend from analog to digital may be delayed based on the consecutive
     * audio frame Cd/No values. When enabled, once blend line changes state (to play analog) it will not change again
     * until Cd/No increases above the blend decision threshold setting.
     * <b> Applies only to MPS hybrid modes </b>
     */
    bool blend_decision;
    /**
     * @brief The minimum required Cd/No, in FM, on consecutive frames to allow normal blending.
     *
     * Value ranges from 52 to 60dB-Hz. Recommended value is 58dB-Hz.
     *
     * <b> Applies only to MPS hybrid modes when blend decision(blend look ahead) is enabled</b>
     */
    uint_t fm_cdno_blend_decision;
    /**
     * @brief The minimum required C/No, in AM, on consecutive frames to allow normal blending.
     *
     * Value ranges from 50 to 70dB-Hz. Recommended value is 67dB-Hz.
     *
     * <b> Applies only to MPS hybrid modes when blend decision(blend look ahead) is enabled</b>
     */
    uint_t am_cdno_blend_decision;
    /**
     * @brief Invert FM digital audio phase
     *
     * Setting the flag will invert the FM digital audio phase. Sometimes needed to phase-align analog
     * and digital audio during blending.
     */
    bool fm_audio_invert_phase;
    /**
     * @brief Invert AM digital audio phase
     *
     * Setting the flag will invert the AM digital audio phase. Sometimes needed to phase-align analog
     * and digital audio during blending.
     */
    bool am_audio_invert_phase;
    /**
     * @brief Disable audio scaling inside the HDR Library
     *
     * This will overwrite all other audio level modifiers to force digital audio output to always be
     * at full scale. This may be useful for cases when blend level alignment is done outside of the
     * HDR Library.
     */
    bool disable_audio_scaling;
}HDR_blend_params_t;

/**
 * @brief Advanced blend configuration parameters
 */
typedef struct HDR_blend_adv_params_t {
    /**
     * @brief Enables/Disables digital audio ramp up
     *
     * When this feature is enabled, a linear ramp will be applied to the digital audio level whenever
     * audio reception resumes from an outage condition. This is similar to blend crossfade used for mixing
     * digital and analog audio for hybrid modes except mixing is done with mute(zero) samples.
     *
     * <b> Only applies to a Hybrid SPS or any All Digital programs. </b>
     */
    bool ramp_up_enabled;
    /**
     * @brief Time duration to ramp up
     *
     * This parameter controls the time duration, measured in audio frames(46.4ms), to ramp up the audio
     * from mute after recovery from an outage. Value ranges from 1 to 16. Recommended value is 16(743ms).
     *
     * <b> Only applies to a Hybrid SPS or any All Digital program.
     *     Parameter has no effect if #ramp_up_enabled is set to false. </b>
     */
    uint_t ramp_up_time;
    /**
     * @brief Enables/Disables digital audio ramp down
     *
     * When this feature is enabled, a linear ramp will be applied to the digital audio level whenever
     * audio reception is lost. This is similar to blend crossfade used for mixing
     * digital and analog audio for hybrid modes except mixing is done with mute(zero) samples.
     *
     * <b> Only applies to a Hybrid SPS or any All Digital programs. </b>
     */
    bool ramp_down_enabled;
    /**
     * @brief Time duration to ramp down
     *
     * This parameter controls the time duration, measured in audio frames(46.4ms), to ramp down the audio
     * to mute after reception is lost. Value ranges from 1 to 16. Recommended value is 16(743ms).
     *
     * <b> Only applies to a Hybrid SPS or any All Digital program.
     *     Parameter has no effect if #ramp_down_enabled is set to false. </b>
     */
    uint_t ramp_down_time;
    /**
     * @brief Enables/Disables comfort noise
     *
     * When enabled, instead of just playing mute after ramp-down is complete, the radio will play comfort
     * noise(which will ramp up to the desired value #comfort_noise_level). The ramp up and down duration is set
     * to 4 audio frames(186ms).
     *
     * <b> Only applies to a Hybrid SPS or any All Digital program. Does not apply if the blend threshold for all
     * digital programs is set to Q7.</b>
     */
    bool comfort_noise_enabled;
    /**
     * @brief Comfort noise level(dBFS)
     *
     * Range: -100 to 0 dBFS. Recommended Value is -48dBFS.
     *
     * <b> Only applies to a Hybrid SPS or any All Digital program. Does not apply if the blend threshold for all
     * digital programs is set to Q7. Parameter has no effect if #comfort_noise_enabled is set to false. </b>
     */
    int_t comfort_noise_level;
    /**
     * @brief Enables/Disables enhanced audio stream hold-off
     *
     * Enhanced stream is combined with Core Stream to provide stereo with up to 15kHz of audio bandwidth. If the receiver is
     * near the edge of enhanced coverage so that the enhanced audio cuts in and out, a situation will arise similar
     * to analog/digital blending where audio will fluctuate between mono and stereo creating a bad user experience.
     *
     * If enabled, then under weak signal conditions, a hold-off is applied to enhanced audio until the signal quality
     * exceeds a certain C/No threshold for a predefined period of time (initially set to 5 seconds, but can max out
     * to 25 seconds).
     *
     * <b> AM Hybrid and AM All Digital Modes only </b>
     */
    bool am_enh_stream_holdoff_enabled;
    /**
     * @brief C/No threshold for hybrid programs enhanced stream hold-off
     *
     * Under weak signal conditions, a hold-off is applied to enhanced audio until the signal quality exceeds a
     * threshold specified by this parameter. Value ranges from 47 to 80 dB-Hz. Recommended value is 72.
     *
     * <b> AM Hybrid and AM All Digital Modes only. Parameter has no effect if #am_enh_stream_holdoff_enabled
     * is set to false. </b>
     */
    uint_t am_mps_enh_stream_holdoff_thresh;
    /**
     * @brief C/No threshold for all digital programs enhanced stream hold-off
     *
     * Under weak signal conditions, a hold-off is applied to enhanced audio until the signal quality exceeds a
     * threshold specified by this parameter. Value ranges from 47 to 80 dB-Hz. Recommended value is 72.
     *
     * <b> AM Hybrid and AM All Digital Modes only. Parameter has no effect if #am_enh_stream_holdoff_enabled
     * is set to false. </b>
     */
    uint_t am_all_dig_enh_stream_holdoff_thresh;
    /**
     * @brief Enable/Disable AM Audio Bandwidth Management
     *
     * When the system transitions from Analog to Digital, the audio BW changes from 3.5-5kHz to
     * 15kHz. This is a big jump in bandwidth and the wider bandwidth is perceived by the user as
     * sounding louder. Frequent blending will cause a poor user experience. Audio BW matches the digital audio
     * bandwidth to analog and then gradually increases it to maximum level eliminating abrupt change.
     *
     * <b> AM Hybrid modes only </b>
     */
    bool am_dig_audio_bw_mgmt_enabled;
    /**
     * @brief Sets AM digital audio bandwidth at blend point
     *
     * At the blend point (Transition from Analog to Digital, or vice versa), this will be the bandwidth
     * of the digital audio signal.
     *
     * <pre>
     *  Range is 0 to 56, where
     *  0: 1.0  kHz
     *  1: 1.25 kHz
     *  2: 1.5  kHz
     *  ...
     *  56: 15.0 kHz
     * </pre>
     *
     * <b> Applies only if #am_dig_audio_bw_mgmt_enabled is true. </b>
     */
    uint_t am_dig_audio_blend_start_bw;
    /**
     * @brief Maximum bandwidth of the digital audio signal.
     *
     * Range is 0 to 56, see #am_dig_audio_blend_start_bw. Recommended value is 56(15kHz).
     *
     * <b> Applies only if #am_dig_audio_bw_mgmt_enabled is true. </b>
     */
    uint_t am_dig_audio_max_bw;
    /**
     * @brief Step time duration
     *
     * If the signal strength value has exceeded the threshold(#am_dig_audio_bw_step_threshold)
     * since the timer was started, then the system will increase the digital audio bandwidth
     * by one step as defined by #am_dig_audio_bw_step_up_size, otherwise, the system will
     * decrease the digital audio bandwidth by one step as defined by #am_dig_audio_bw_step_down_size.
     *
     * <pre>
     * Range is 0 to 65535, where
     *  0: 0 ms
     *  1: 46.4 ms
     *  2: 92.9 ms
     *  ...
     *  65535: 3043.4 seconds
     * </pre>
     *
     * Recommended value is 87(4040.28ms).
     *
     * <b> Applies only if #am_dig_audio_bw_mgmt_enabled is true. </b>
     */
    uint_t am_dig_audio_bw_step_time;
    /**
     * @brief Bandwidth step up size
     *
     * Size by which the digital audio bandwidth will decrease for every step(#am_dig_audio_bw_step_time)
     * during blend transition.
     *
     * <pre>
     *  Range is 1 to 56, where
     *  1: 0.25 kHz
     *  2: 0.5  kHz
     *  3: 1.0  kHz
     *  ...
     *  56: 14.0 kHz
     * </pre>
     *
     * Recommended value is 2(0.5kHz).
     *
     * <b> Applies only if #am_dig_audio_bw_mgmt_enabled is true. </b>
     */
    uint_t am_dig_audio_bw_step_up_size;
    /**
     * @brief Bandwidth step down size
     *
     * Size by which the digital audio bandwidth will increase for every step(#am_dig_audio_bw_step_time)
     * during blend transition.
     *
     * Range is 1 to 56, see #am_dig_audio_bw_step_up_size.
     *
     * Recommended value is 3(1.0kHz).
     *
     * <b> Applies only if #am_dig_audio_bw_mgmt_enabled is true. </b>
     */
    uint_t am_dig_audio_bw_step_down_size;
    /**
     * @brief C/No bandwidth step threshold
     *
     * Determines the carrier-to-noise threshold used in determining whether to
     * step up/down in digital audio bandwidth.
     *
     * Range is 47 to 80 dB-Hz. Recommended value is 67db-Hz.
     *
     * <b> Applies only if #am_dig_audio_bw_mgmt_enabled is true. </b>
     */
    uint_t am_dig_audio_bw_step_threshold;
    /**
     * @brief Enable/Disable mono-to-stereo transition
     *
     * On top of gradual audio bandwidth increase, audio can gradually transition from mono to stereo
     * to further improve blend experience.
     *
     * <b> Applies only if #am_dig_audio_bw_mgmt_enabled is set to true.</b>
     */
    bool am_mono2stereo_enabled;
    /**
     * @brief AM mono-to-stereo starting bandwidth
     *
     * Sets the starting audio bandwidth at which digital audio mono to stereo blending
     * transitions occur. If the actual BW falls below the BW set by this parameter, then the digital
     * audio will switch from Stereo to Mono.
     *
     *  Range is 0 to 56, see #am_dig_audio_blend_start_bw. Recommended value is 16(5kHz).
     *
     * <b> Applies only if #am_dig_audio_bw_mgmt_enabled and #am_mono2stereo_enabled is set to true.</b>
     */
    uint_t am_mono2stereo_start_bw;
    /**
     * @brief AM mono-to-stereo step duration
     *
     * Number of audio frames to wait before making the next digital mono to stereo blend
     * adjustment. An adjustment will only occur if the current digital audio BW is greater than the value
     * specified by #am_mono2stereo_start_bw. Value ranges from 0 to 16. Recommended value is 8(372ms).
     */
    uint_t am_mono2stereo_step_time;
    /**
     * @brief Maximum stereo separation value
     *
     * Range is 0 to 16, where 0 = Mono, 16 = Full stereo. Recommended value is 16.
     *
     * <b> Applies only if #am_dig_audio_bw_mgmt_enabled and #am_mono2stereo_enabled is set to true.</b>
     */
    uint_t am_mono2stereo_max_sep;
}HDR_blend_adv_params_t;

/**
 * @brief Sets all blend parameters at once
 *
 * <b>Note:Some parameters are write-protected and can only be modified during idle mode.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[in] params: Pointer to the new blend parameters
 * @returns
 *     0 - Success <br>
 *    -1 - At least one parameter value is invalid <br>
 *    -2 - Modifying write-protected parameter(s) while not in IDLE mode <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_blend_set_all_params(HDR_instance_t* hdr_instance, const HDR_blend_params_t* params);

/**
 * @brief Retrieves current blend parameters
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[out] blend_params: Pointer to blend parameters. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_blend_get_all_params(HDR_instance_t* hdr_instance, HDR_blend_params_t* blend_params);

/**
 * @brief Retrieves an individual blend parameter
 *
 * Pass the name of the parameter from #HDR_blend_params_t data structure and cast.
 * The default output is uint_t so allocate minimum of 4 bytes for the result.
 *<pre>
 * For example:
 *     rc = HDR_blend_get_param(hdrInstance, fm_mps_blend_thresh, &fmMpsBlendThresh)
 * </pre>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[in] param: Name of the parameter from #HDR_blend_params_t.
 * @param[out] output: Pointer to the output value(must be at least 4 bytes long). Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
#define HDR_blend_get_param(hdr_instance, param, output) \
    HDR_blend_get_param_actual(hdr_instance, offsetof(HDR_blend_params_t, param), \
                               sizeof(((HDR_blend_params_t *) 0)->param), output)
/**
 * @brief DO NOT USE DIRECTLY. USE MACRO #HDR_blend_get_param() instead.
 * @param[in] hdr_instance
 * @param[in] offset
 * @param[in] size
 * @param[out] output
 * @returns @see HDR_blend_get_param
 */
int_t HDR_blend_get_param_actual(HDR_instance_t* hdr_instance, uint_t offset, uint_t size, uint_t* output);

/**
 * @brief Sets an individual blend parameter
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[in] param: Name of the parameter from #HDR_blend_params_t
 * @param[in] value: Value to the set the parameter to
 * @returns
 *     0 - Success <br>
 *    -1 - Invalid value for this parameter <br>
 *    -2 - Modifying write-protected parameter while not in IDLE mode <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
#define HDR_blend_set_param(hdr_instance, param, value) \
    HDR_blend_set_param_actual(hdr_instance, offsetof(HDR_blend_params_t, param), \
                               sizeof(((HDR_blend_params_t *) 0)->param), value)
/**
 * @brief DO NOT USE DIRECTLY. USE MACRO #HDR_blend_set_param() instead.
 * @param[in] hdr_instance
 * @param[in] offset
 * @param[in] size
 * @param[in] value
 * @returns @see HDR_blend_set_param_actual
 */
int_t HDR_blend_set_param_actual(HDR_instance_t* hdr_instance, uint_t offset, uint_t size, uint_t value);

/**
 * @brief Retrieves current advanced blend parameters
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[out] adv_blend_params: Pointer to advanced blend parameters. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_blend_get_all_adv_params(HDR_instance_t* hdr_instance, HDR_blend_adv_params_t* adv_blend_params);

/**
 * @brief Sets all advanced blend parameters at once
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[in] params: Pointer to the new advanced blend parameters
 * @returns
 *     0 - Success <br>
 *    -1 - At least one parameter value is invalid <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_blend_set_all_adv_params(HDR_instance_t* hdr_instance, const HDR_blend_adv_params_t* params);

/**
 * @brief Retrieves an individual advanced blend parameter
 *
 * Pass the name of the parameter from #HDR_blend_adv_params_t data structure.
 * The default output is uint_t so allocate minimum of 4 bytes for the result.
 *
 * <pre>
 *   For example:
 *      rc = HDR_blend_get_adv_param(hdrInstance, ramp_up_enabled, &rampUpEnabledVal)
 * </pre>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[in] param: Name of the parameter from #HDR_blend_adv_params_t
 * @param[out] output: Pointer to the output value. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
#define HDR_blend_get_adv_param(hdr_instance, param, output) \
                HDR_blend_get_adv_param_actual(hdr_instance, offsetof(HDR_blend_adv_params_t, param), \
                                               sizeof(((HDR_blend_adv_params_t *) 0)->param), output)
/**
 * @brief DO NOT USE DIRECTLY. USE MACRO #HDR_blend_get_adv_param instead.
 * @param[in] hdr_instance
 * @param[in] offset
 * @param[in] size
 * @param[out] output
 * @returns @see HDR_blend_get_adv_param
 */
int_t HDR_blend_get_adv_param_actual(HDR_instance_t* hdr_instance, uint_t offset, uint_t size, uint_t* output);

/**
 * @brief Sets an individual advanced blend parameter
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[in] param: Name of the parameter from #HDR_blend_adv_params_t
 * @param[in] value: Value to the set the parameter to
 * @returns
 *     0 - Success <br>
 *    -1 - Invalid value for this parameter <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
#define HDR_blend_set_adv_param(hdr_instance, param, value) \
    HDR_blend_set_adv_param_actual(hdr_instance, offsetof(HDR_blend_adv_params_t, param), \
                                   sizeof(((HDR_blend_adv_params_t *) 0)->param), value)
/**
 * @brief DO NOT USE DIRECTLY. USE MACRO #HDR_blend_set_adv_param instead.
 * @param[in] hdr_instance
 * @param[in] offset
 * @param[in] size
 * @param[in] value
 * @returns @see HDR_blend_set_adv_param
 */
#ifdef USE_HDRLIB_2ND_CHG_VER
int_t HDR_blend_set_adv_param_actual(HDR_instance_t* hdr_instance, uint_t offset, uint_t size, int_t value);
#else
int_t HDR_blend_set_adv_param_actual(HDR_instance_t* hdr_instance, uint_t offset, uint_t size, uint_t value);
#endif
/**
 * @brief Different blend operations
 */
typedef enum {
    HDR_BLEND_CROSSFADE, /**< Normal blending with crossfade */
    HDR_PLAY_DIGITAL,    /**< Force digital audio */
    HDR_PLAY_ANALOG      /**< Force analog audio */
}HDR_blend_control_t;

/**
 * @brief Specifies how blend crossfade should operate
 *
 * This function takes into account ballgame mode and forced digital/analog configurations
 *
 * <b>Note: Calls to this function are invalid for data-only instance.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[out] blend_control: Controls the blend operation. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_blend_control(HDR_instance_t* hdr_instance, HDR_blend_control_t* blend_control);

/**
 * @brief Applies specified blend delay to the mps audio output
 *
 * Usually used with automatic audio alignment(AAA)
 *
 * <b>Note: Calls to this function are invalid for data-only instance.</b>
 *
 * @param[in]  hdr_instance: Pointer to the HDR Library instance
 * @param[in]  sample_adjust: Direction and number of samples to adjust
 * @param[out] decodedAudioSamples: amount of decoded audio buffered within the
 * HDR Library.  This may be used in conjunction with AAA to determine when the
 * blend may begin.
 * @returns
 *     0 - success <br>
 *    -1 - Currently playing all digital program <br>
 *    -2 - The adjustment is too large. The number is platform specific. <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
#ifdef USE_HDRLIB_2ND_CHG_VER
int_t HDR_blend_adjust_audio_delay(HDR_instance_t* hdr_instance, int_t sample_adjust, uint32_t* decodedAudioSamples);
#else
int_t HDR_blend_adjust_audio_delay(HDR_instance_t* hdr_instance, int_t sample_adjust);
#endif

/**
 * @brief status of blend alignment in progress
 *
 * Usually used with automatic audio alignment(AAA)
 *
 * <b>Note: Calls to this function are invalid for data-only instance.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[in] align_status: Alignment progress status True = Alignment in Progress
 * @returns
 *     0 - success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_blend_align_progress_status(HDR_instance_t* hdr_instance, bool* align_status);

#ifdef USE_HDRLIB_2ND_CHG_VER
/**
 * @brief set the internal Blend Flag from the external one
 *
 * Usually used with automatic audio alignment(AAA)
 *
 * <b>Note: Calls to this function are invalid for data-only instance.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[in] blendFlag: External BVlend FDlag, as modified by AAA
 * @returns
 *     0 - success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_blend_set_flag(HDR_instance_t* hdr_instance, bool blendFlag);
#endif

#endif //HDR_BLEND_H_

/** @} */
