/*******************************************************************************
*
* (C) copyright 2014 - 2017, iBiquity Digital Corporation, U.S.A.
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
 * @file hdrBbSrc.h
 * @brief Baseband Resampler API
 * @defgroup hdrBbSrc Baseband Resampler API
 * @brief Baseband Resampler API
 * @ingroup HdrApi
 * @{
 *
 * This module contains functions and definitions for the baseband resampler.
 * This API module is an extension to HD Radio Library and is therefore optional
 * to use.
 */
#ifndef HDR_BB_SRC_H
#define HDR_BB_SRC_H

#include "hdrCore.h"

/**
 * @brief Required memory for baseband resampler
 *
 * This amount(in bytes) must be allocated and provided to the resampler at initialization.
 */
#define HDR_BB_SRC_MEM_SIZE        (6000)

/**
 * @brief Defines resampler handle
 */
typedef struct HDR_bb_src_t HDR_bb_src_t;

/**
 * @brief Initializes baseband resampler instance
 *
 * @param[in] bb_src_memory: Pointer to memory to be used by the baseband resampler instance.
 *                    The required size of memory is #HDR_BB_SRC_MEM_SIZE bytes.
 *
 * @returns Handle to the baseband resampler instance. NULL indicates a failure.
 */
HDR_bb_src_t* HDR_bb_src_init(void* bb_src_memory);

/**
 * @brief Supported input sample rates to the baseband resampler
 */
typedef enum {
    HDR_BB_SRC_650_KHZ,
    HDR_BB_SRC_675_KHZ,
    HDR_BB_SRC_744_KHZ,
    HDR_BB_SRC_768_KHZ,
    HDR_BB_SRC_1024_KHZ
}HDR_bb_src_input_rate_t;

/**
 * @brief Changes operation parameters
 *
 * @param[in] bb_src: Handle to the baseband resampler instance
 * @param[in] input_sample_rate: bb src input sample rate
 * @param[in] band: Specifies AM or FM operation.
 */
void HDR_bb_src_reset(HDR_bb_src_t* bb_src, HDR_bb_src_input_rate_t input_sample_rate, HDR_tune_band_t band);

/**
 * @brief Changes the sample rate of the input samples to 744.1875 kHz for FM or 46.51171875 kHz for AM
 *
 * @param[in] bb_src: Baseband resampler handle
 * @param[in] bb_input_samples: Pointer to baseband input samples buffer.
 * @param[in] num_input_samples: Number of input IQ samples
 * @param[out] bb_output_samples: Pointer to the ouput samples buffer. Must be allocated by the caller.
 * @param[out] num_output_samples: Number of output samples produced by the resampler. May not equal to
 *                                the number of input samples.
 */
void HDR_bb_src_exec(HDR_bb_src_t* bb_src, int16c_t* bb_input_samples, uint_t num_input_samples,
                     int16c_t* bb_output_samples, uint32_t* num_output_samples);

#ifdef USE_HDRLIB_2ND_CHG_VER
/**
 * @ brief Retieve the bb src offset
 *
 * @ param[in]  bb_src: Baseband resampler handle
 */
int32_t HDR_bb_src_get_offset(HDR_bb_src_t* bb_src);
#endif

#endif // HDR_BB_SRC_H

/** @} */
