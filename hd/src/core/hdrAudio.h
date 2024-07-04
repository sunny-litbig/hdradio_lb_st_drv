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
*******************************************************************************/
/**
 * @file hdrAudio.h
 * @brief Audio API Definitions and Functions
 * @defgroup HdrAudio Audio
 * @brief Audio API
 * @ingroup HdrApi
 * @{
 *
 * This module provides ability to control and manage audio functionality of the HD Radio.
 * The API provides interface for obtaining information
 * about available station audio programs, switching between audio programs,
 * and monitoring audio statuses, like availability and quality.
 *
 * <b>For additional information see:</b><br>
 *    RX_IDD_2206 - Baseband Processor Command and Data Interface Definition - Revision<X>.pdf
 */

#ifndef HDR_AUDIO_H_
#define HDR_AUDIO_H_

#include "hdrCore.h"

/**
 * @brief HDR audio frame size
 *
 * Number of samples(left/right pairs) produced by the HD Radio library every 46.6 milliseconds
 */
#define HDR_AUDIO_FRAME_SIZE        (2048U)

/**
 * @brief Returns the status of whether audio was acquired for the currently playing program
 *
 * Digital audio is considered to be acquired when audio of good quality is decoded for some period of time.
 * How good and how long depends on the blend threshold option (see #HDR_blend_thresh_sel_t).
 * All data related to HD Radio audio is invalid unless digital audio acquired status is true.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 *
 * @returns
 *     true  - audio acquired; audio samples are available <br>
 *     false - no audio samples available
 */
bool HDR_digital_audio_acquired(HDR_instance_t* hdr_instance);

/**
 * @brief List of possible audio programs for any given station
 *
 * HD Radio enables stations to "multicast" additional channels on a single frequency.
 * A station can have up to 8 independent audio programs defined by this enum.
 */
typedef enum HDR_program_t{
    HDR_PROGRAM_HD1 = 0,    /**< Program-1/MPS-Audio/HD-1  */
    HDR_PROGRAM_HD2,        /**< Program-2/SPS1-Audio/HD-2 */
    HDR_PROGRAM_HD3,        /**< Program-3/SPS2-Audio/HD-3 */
    HDR_PROGRAM_HD4,        /**< Program-4/SPS3-Audio/HD-4 */
    HDR_PROGRAM_HD5,        /**< Program-5/SPS4-Audio/HD-5 */
    HDR_PROGRAM_HD6,        /**< Program-6/SPS5-Audio/HD-6 */
    HDR_PROGRAM_HD7,        /**< Program-7/SPS6-Audio/HD-7 */
    HDR_PROGRAM_HD8        /**< Program-8/SPS7-Audio/HD-8 */
}HDR_program_t;

#define HDR_MAX_NUM_PROGRAMS (8U)

/**
 * @brief Sets playing program number
 *
 * If the selected audio program is not available on this station, the HD Radio Library will output
 * mute audio.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] program: Audio program to play
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_set_playing_program(HDR_instance_t* hdr_instance, HDR_program_t program);

/**
 * @brief Retrieves currently playing program number
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] program: Pointer to playing program. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_playing_program(HDR_instance_t* hdr_instance, HDR_program_t* program);

/**
 * @brief Retrieves the HD audio PCM samples
 *
 * The samples are interleaved: Left sample, right sample, left, right, etc., to produce stereo
 * audio. Each channel sample is 16 bits, sampled at 44.1 KHz giving a 32-bit PCM sample pair that's
 * written to user allocated buffer.
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle
 * @param [out] pcm_output: Buffer storing output audio samples(32-bit). Must be allocated by the caller.
 * @param [in]  num_samples: Specifies number of samples(32-bit) to copy to the output buffer
 * @param [out] audio_quality: Audio quality of the current output frame. Ranges 0 - 15, where 15 is perfect and 0 is worst.
 * @param [out] blend_flag: Notifies the system when it's ok to transition to digital audio
 *                          true - transition to digital, false - transition to analog
 *
 * @returns
 *    >= 0  Number of samples written <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_audio_output(HDR_instance_t* hdr_instance, HDR_pcm_stereo_t* pcm_output, uint_t num_samples, uint_t* audio_quality, bool* blend_flag);

/**
 * @brief Indicates the audio codec configuration for an audio program
 *
 * Audio codec mode determines the number of streams used and the maximum bit rate.
 * For dual-stream modes, bit rate is given by two numbers separated by slash
 * character, for core/enhanced.
 */
typedef enum {
    HDR_AUDIO_CODEC_MODE0,  /**< FM hybrid single-stream on P1 channel (up to 96 kbits/s)           */
    HDR_AUDIO_CODEC_MODE1,  /**< FM all digital dual-stream with mono core (up to 48/48 kbits/s)    */
    HDR_AUDIO_CODEC_MODE2,  /**< AM hybrid/all digital dual-stream (up to 20/20 kbits/s) */
    HDR_AUDIO_CODEC_MODE3,  /**< FM all digital dual-stream with stereo core (up to 24/72 kbits/s)  */
    HDR_AUDIO_CODEC_MODE4,  /**< Reserved */
    HDR_AUDIO_CODEC_MODE5,  /**< Reserved */
    HDR_AUDIO_CODEC_MODE6,  /**< Reserved */
    HDR_AUDIO_CODEC_MODE7,  /**< Reserved */
    HDR_AUDIO_CODEC_MODE8,  /**< Reserved */
    HDR_AUDIO_CODEC_MODE9,  /**< Reserved */
    HDR_AUDIO_CODEC_MODE10, /**< FM dual-stream on SPS (up to 22/24 kbits/s) */
    HDR_AUDIO_CODEC_MODE11, /**< Reserved */
    HDR_AUDIO_CODEC_MODE12, /**< Reserved */
    HDR_AUDIO_CODEC_MODE13, /**< FM hybrid/all digital single-stream (up to 24 kbits/s) */
    HDR_MAX_NUM_CODEC_MODES
}HDR_audio_codec_mode_t;

/**
 * @brief Retrieves the audio codec mode of the currently playing program
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle.
 * @param[out] codec_mode: Pointer to audio codec mode of the currently playing program. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_codec_mode(HDR_instance_t* hdr_instance, HDR_audio_codec_mode_t* codec_mode);

/**
 * @brief Bitmap of audio programs
 */
typedef union HDR_program_bitmap_t{
    struct {
        uint8_t program1:1;
        uint8_t program2:1;
        uint8_t program3:1;
        uint8_t program4:1;
        uint8_t program5:1;
        uint8_t program6:1;
        uint8_t program7:1;
        uint8_t program8:1;
    };
    uint8_t all;
}HDR_program_bitmap_t;

/**
 * @brief Retrieves a bitmap of available audio programs.
 *
 * The MPS availability bit is similar to the acquisition status (i.e., for FM it remains a 1 for
 * one minute after a loss of signal). The SPS bits remain 1 for (256 audio frames or 11.888 seconds)
 * after signal loss occurs.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] avail_programs: Pointer to the bitmap of available programs. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_available_programs(HDR_instance_t* hdr_instance, HDR_program_bitmap_t* avail_programs);

/**
 * @brief Retrieves the currently playing program type
 *
 * Audio Service Program Types are specified to indicate categories that are available
 * to classify, define, or label the audio services. Program types zero to 31 mirror the
 * existing RBDS definitions. Program types 32 to 255 are HD Radio extensions that define
 * additional music and program formats available to broadcasters.
 *
 * <b>More detail available here:</b><br>
 * http://www.hdradio.com/broadcasters/us_regulatory/nrsc_supplemental_information
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] program_type: Pointer to program type of the currently playing program. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_playing_program_type(HDR_instance_t* hdr_instance, uint_t * program_type);

/**
 * @brief Lists program types on this station
 *
 * <b>More details available here:</b><br>
 * http://www.hdradio.com/broadcasters/us-regulatory/nrsc-supplemental-information
 */
typedef struct{
    uint_t value[HDR_MAX_NUM_PROGRAMS]; /**< Stores program types; one for each program number */
}HDR_program_types_t;

/**
 * @brief Retrieves the program types for all available audio programs
 *
 * Audio Service Program Types are specified to indicate categories that are available
 * to classify, define, or label the audio services.  Program types zero to 31 mirror the
 * existing RBDS definitions. Program types 32 to 255 are HD Radio extensions that define
 * additional music and program formats available to broadcasters.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] program_types: Pointer to program types of all audio programs on current station.
 *                            Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_program_types(HDR_instance_t* hdr_instance, HDR_program_types_t* program_types);

/**
 * @brief Audio quality report
 *
 * Diagnostics information about audio quality reported by the HD Radio Library.
 */
typedef struct {
    uint_t frame_count;            /**< Total audio frames received. Reset after re-acquisition or program switch. */
    uint_t core_errors;            /**< Core errors detected */
    uint_t enh_errors;             /**< Enhanced audio errors detected */
    uint_t quality_indicator;      /**< Instantanious quality indicator */
    uint_t filt_quality_indicator; /**< Quality indicator filtered over time. */
}HDR_audio_quality_report_t;

/**
 * @brief Retrieves audio quality report for the currently playing program
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] report: Pointer to quality report data. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_audio_quality_report(HDR_instance_t* hdr_instance, HDR_audio_quality_report_t* report);

/**
 * @brief Retrieves a value of the transmitter audio gain.
 *
 * This value indicates the amount of audio gain for the receiver to apply to the digital audio
 * of the currently selected program, relative to the analog audio for MPS programs or relative
 * to other digital audio programs for SPS programs, to achieve proper blend level alignment.
 *
 * If MPS tx digital audio gain is enabled , the gain is applied inside the library and the host
 * doesnt have to apply then gain, hence it will be 0.
 * If the host/framework uses auto level alignment , then the value will only be reported through this
 * function but not applied. We recommend that the host ignore this value in cases where an automatic level
 * alignment algorithm is used.
 *
 * In 5-bit two’s complement format:
 *
 * 0x00: 0 dB   0x18: -8 dB    <br>
 * 0x01: 1 dB   0x19: -7 dB    <br>
 * 0x02: 2 dB   0x1A: -6 dB    <br>
 * 0x03: 3 dB   0x1B: -5 dB    <br>
 * 0x04: 4 dB   0x1C: -4 dB    <br>
 * 0x05: 5 dB   0x1D: -3 dB    <br>
 * 0x06: 6 dB   0x1E: -2 dB    <br>
 * 0x07: 7 dB   0x1F: -1 dB    <br>
 * 0x08 to 0x17: Reserved      <br>
 * 0x20 to 0x7F: Reserved
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] tx_gain: Pointer to tx gain of the currently playing program. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_tx_dig_audio_gain(HDR_instance_t* hdr_instance, uint_t * tx_gain);

/**
 * @brief Checks if playing program is hybrid or all-digital
 *
 * Hybrid programs have both analog and digital audio available that have to be aligned in time
 * for smooth blending. All-digital programs have only digital audio component and therefore
 * do not need to be aligned to anything.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] hybrid: true - hybrid program
 *                    false - all-digital program
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_hybrid_program(HDR_instance_t* hdr_instance, bool* hybrid);

#endif /* HDR_AUDIO_H_ */

/** @} */
