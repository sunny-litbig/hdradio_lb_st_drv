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
 * @file hdrTest.h
 * @brief Diagnostics/Test API
 * @defgroup hdrTest Test
 * @brief Diagnostics/Test API
 * @ingroup HdrApi
 * @{
 *
 * Functions to support diagnostics and testing of HD Radio Library.
 *
 * <b>For additional information see:</b><br>
 *    RX_IDD_2206_appendixD - Diagnostics - Revision<X>.pdf

 */
#ifndef HDR_TEST_H_
#define HDR_TEST_H_

#include "hdrCore.h"
#include "hdrPhy.h"

/**
 * @brief Places HDR instance into Bit Error Rate(BER) mode
 *
 * When this mode is enabled, no digital audio or data is available from HD Radio.
 * Each logical channel(e.g., P1, PIDS, etc.) is compared with a known data pattern and
 * checked for errors.
 *
 * <b> Note: The exciter needs to be configured to play special BER test vectors to get valid BER results </b>
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_test_enable_ber_mode(HDR_instance_t* hdr_instance);

/**
 * @brief Places HDR instance back to normal(audio) mode
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_test_disable_ber_mode(HDR_instance_t* hdr_instance);

/**
 * @brief Returns indication of whether BER is enabled
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance
 * @returns
 *     true - BER is enabled <br>
 *     false - BER is disabled
 */
bool HDR_test_ber_mode_enabled(HDR_instance_t* hdr_instance);

/**
 * @brief Bit Error Rate(BER) result output data structure
 */
typedef struct {
    uint_t pids_block_errors;  /**< Total number of PIDS blocks(80 bits) that had at least one error */
    uint_t pids_blocks_tested; /**< Total number of PIDS blocks(80 bits) tested */
    uint_t pids_bit_errors;    /**< Total number of PIDS bit errors */
    uint_t pids_bits_tested;   /**< Total number of PIDS bits tested */
    uint_t p1_bit_errors;      /**< Total number of P1 bit errors */
    uint_t p1_bits_tested;     /**< Total number of P1 bit bits tested */
    uint_t p2_bit_errors;      /**< Total number of P2 bit errors */
    uint_t p2_bits_tested;     /**< Total number of P2 bit bits tested */
    uint_t p3_bit_errors;      /**< Total number of P3 bit errors */
    uint_t p3_bits_tested;     /**< Total number of P3 bit bits tested */
    uint_t p4_bit_errors;      /**< Total number of P4 bit errors */
    uint_t p4_bits_tested;     /**< Total number of P4 bit bits tested */
}HDR_test_ber_t;

/**
 * @brief Get BER results
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance
 * @param [out] ber: Pointer to the output data. Must be allocated by the caller.
 * @see HDR_test_enable_ber
 * @returns
 *     0 - Success <br>
 *    -1 - BER is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_test_get_ber(HDR_instance_t* hdr_instance, HDR_test_ber_t* ber);

/**
 * @brief Resets all BER results to zero
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_test_reset_ber(HDR_instance_t* hdr_instance);

/**
 * @brief Resets all audio error counters to zero
 *
 * <b>Note: Calls to this function are invalid for data-only instance.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_test_reset_audio_errors(HDR_instance_t* hdr_instance);

/**
 * @brief AM audio bandwidth status output data structure
 */
typedef struct{
    /**
     * @brief Current Digital Audio bandwidth
     *
     * <pre>
     *  Range is 0 to 56, where
     *  0: 1.0  kHz
     *  1: 1.25 kHz
     *  2: 1.5  kHz
     *  ...
     *  56: 15.0 kHz
     * </pre>
     */
    uint_t currentBw;
    /**
     * @brief Current Digital Audio Separation
     *
     * Range is 0 to 16, where 0 = Mono, 16 = Full stereo.
     */
    uint_t currentSep;
}HDR_test_audio_bw_status_t;

/**
 * @brief Get AM audio bandwidth status
 *
 * <b>Note: Calls to this function are invalid for data-only instance.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[out] audio_bw_status: Pointer to the output data. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -1 - Not FM band <br>
 *    -2 - AM audio bandwidth management is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 *
 * @see #HDR_blend_adv_params_t::am_dig_audio_bw_mgmt_enabled in hdrBlend.h
 */
int_t HDR_test_get_audio_bw_status(HDR_instance_t* hdr_instance, HDR_test_audio_bw_status_t* audio_bw_status);

/**
 * @brief Retrieves raw tx blend control bits transmitted by the radio station
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[out] blend_control: Pointer to output raw blend control bits. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_test_get_raw_tx_blend_control(HDR_instance_t* hdr_instance, uint_t * blend_control);

/**
 * @brief Add instance demodulation to MRC
 *
 * Enables demodulator data from the specified instance to contribute
 * the channel decoding process.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @returns
 *     0 - Success <br>
 *    -1 - Not FM band <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_test_mrc_demod_enable(HDR_instance_t* hdr_instance);

/**
 * @brief Remove instance demodulation from MRC
 *
 * Disables demodulator data from the specified instance from contributing
 * to the channel decoding process.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @returns
 *     0 - Success <br>
 *    -1 - Not FM band <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_test_mrc_demod_disable(HDR_instance_t* hdr_instance);

/**
 * @brief Returns indication of whether the specified instance demodulation is enabled
 *
 * When instance demodulation is enabled, the demodulation output data from this instance
 * contributes to MRC.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @returns
 *     true - Enabled <br>
 *     false - Disabled
 */
bool HDR_test_mrc_demod_enabled(HDR_instance_t* hdr_instance);

/**
 * @brief Returns the alignment offset between two MRC instances
 *
 * The offset is measured in 744kHz samples
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance
 * @param [out] offset: alignment offset
 * @returns
 *     0 - Success <br>
 *    -1 - Not FM band <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_test_mrc_alignment_offset(HDR_instance_t* hdr_instance, int_t* offset);

/**
 * @brief Enables logging for specific module
 * @param [in] module: Module ID
 * @param [in] group_mask: Specifies the group within the module
 * @returns
 *     0 - Success <br>
 *    <0 - Failure
 */
int_t HDR_logger_enable(uint_t module, uint32_t group_mask);

/**
 * @brief Disables all logger modules
 * @returns
 *     0 - Success <br>
 *    <0 - Error
 */
int_t HDR_logger_disable_all(void);

#endif //HDR_TEST_H_

/** @} */ // doxygen end-bracket
