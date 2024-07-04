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
 * @file hdrPhy.h
 * @brief Physical Layer API Definitions and Functions
 * @defgroup hdrPhy	Phy
 * @brief Physical Layer API
 * @ingroup HdrApi
 * @{
 *
 * This module provides physical layer monitoring and control that's not defined by
 * core API in hdrCore.h
 *
 * <b>For additional information see:</b><br>
 *    RX_IDD_2206 - Baseband Processor Command and Data Interface Definition - Revision<X>.pdf
 */
#ifndef HDR_PHY_H_
#define HDR_PHY_H_

#include "hdrCore.h"

/**
 * @brief Returns indication of HD signal acquisition
 *
 * When in MRC mode, this function will return combined status, meaning
 * it will be true if either HDR instance is acquired.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @returns
 *     true - HD signal acquired <br>
 *     false - HD signal not acquired
 */
bool HDR_hd_signal_acquired(const HDR_instance_t* hdr_instance);

/**
 * @brief Returns indication of HD signal acquisition
 *
 * When in MRC mode, this function will return acquisition status specific to HDR instance.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @returns
 *     true - HD signal acquired <br>
 *     false - HD signal not acquired
 */
bool HDR_demod_hd_signal_acquired(const HDR_instance_t* hdr_instance);

/**
 * @brief List of HD AM/FM Primary Service Modes
 */
typedef enum {
    HDR_FM_MP0 = 0,   /**< FM Primary Service Mode 0 - Initial/Invalid */
    HDR_AM_MA0 = 0,   /**< AM Primary Service Mode 0 - Initial/Invalid */
    HDR_FM_MP1 = 1,   /**< FM Primary Service Mode 1 */
    HDR_AM_MA1 = 1,   /**< AM Primary Service Mode 1 */
    HDR_FM_MP2 = 2,   /**< FM Primary Service Mode 2 */
    HDR_FM_MP3 = 3,   /**< FM Primary Service Mode 3 */
    HDR_AM_MA3 = 3,   /**< AM Primary Service Mode 3 */
    NUM_AM_PSM = 4,   /**< Number of AM Primary Service Modes */
    HDR_FM_MP4 = 4,   /**< FM Primary Service Mode 4  - Not Supported */
    HDR_FM_MP5 = 5,   /**< FM Primary Service Mode 5  */
    HDR_FM_MP6 = 6,   /**< FM Primary Service Mode 6 */
    HDR_FM_MP7 = 7,   /**< FM Primary Service Mode 7 - Not Supported */
    HDR_FM_MP8 = 8,   /**< FM Primary Service Mode 8 - Not Supported */
    HDR_FM_MP9 = 9,   /**< FM Primary Service Mode 9 - Not Supported */
    HDR_FM_MP10 = 10, /**< FM Primary Service Mode 10 - Not Supported */
    HDR_FM_MP11 = 11, /**< FM Primary Service Mode 11*/
    NUM_FM_PSM	 	  /**< Number of FM Primary Service Modes */
}HDR_psm_t;

/**
 * @brief Returns current Primary Service Mode(PSM)
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @returns
 *     HDR_psm_t (see #HDR_psm_t)
 */
HDR_psm_t HDR_get_primary_service_mode(const HDR_instance_t* hdr_instance);

/**
 * @brief Returns the raw Signal-to-Noise Ratio(SNR) value
 *
 * The SNR value (a value proportional to the carrier-to-noise ratio)
 * from which Cd/No or C/No is calculated. The SNR value is provided in Q16.16 format.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] raw_snr: Pointer to the raw SNR value. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_raw_snr(const HDR_instance_t* hdr_instance, uint_t * raw_snr);

/**
 * @brief Returns the raw Signal-to-Noise Ratio(SNR) value
 *
 * The SNR value (a value proportional to the carrier-to-noise ratio)
 * from which Cd/No or C/No is calculated. The SNR value is provided in Q16.16 format.
 * It returns this value from demod channel specific to "hdr_instance" instance.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] raw_snr: Pointer to the raw SNR value. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 *    raw_snr is 0 when return value is not 0.
 */
int_t HDR_get_demod_raw_snr(const HDR_instance_t* hdr_instance, uint_t* raw_snr);

/**
 * @brief Returns the Cd/No value for FM and C/No for AM
 *
 * <pre>
 * The formulas are: For FM:
 * Cd/No = [10 * log10(SNR)] + K
 *
 * Where K is 10log10 of the total digital bandwidth.
 *
 *       K = 51.4 for MP1
 *           51.8 for MP2
 *           52.2 for MP3
 *           52.9 for MP5 / MP6 / MP11
 * For AM:
 * C/No = [10*log10(SNR/Ts)] + K
 *
 * Where Ts=0.005805s is the OFDM symbol period and K is the difference between the main carrier level and
 * the digital sideband that is actually measured.
 *
 *       K = 30 for MA1
 *           15 for MA3
 *
 * Final result is rounded to the nearest dB.
 * </pre>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] cdno: Pointer to the Cd/No value. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_cdno(const HDR_instance_t* hdr_instance, uint_t * cdno);

/**
 * @brief Returns the Cd/No value for FM and C/No for AM
 *
 * <pre> Formula: same as HDR_get_cdno()
 * It returns this value from demod channel specific to "hdr_instance" instance.
 * </pre>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] cdno: Pointer to the Cd/No value. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 *    cdno is 0 when return value is not 0.
 */
int_t HDR_get_demod_cdno(const HDR_instance_t* hdr_instance, uint_t* cdno);

/**
 * @brief Digital Signal Quality Measurement(DSQM)
 *
 * Used for HD signal detection during station seek and scan.
 * Typically, a value of 32768 or greater implies HD signal is present
 *
 * <b> Note: DSQM is only applicable to FM </b>
 */
typedef struct {
    uint_t sequence_number;   /**< Sequence number that increments by one modulo 128 each time
                                         that a new DSQM value has been computed */
    uint32_t dsqm_value;	    /**< 16-bit DSQM value */
}HDR_dsqm_t;

/**
 * @brief Returns current DSQM information. 
 *
 * Digital Signal Quality Metric(DSQM) is computed every 16 Layer 1 symbols or (~46.6ms)
 * In MRC mode, it returns combined (Master & Slave) DSQM value when master instance is used. 
 * If slave instance is used, then it returns 0 with error code HDR_ERROR_INSTANCE_TYPE.
 * In non-MRC mode, it returns value from demod channel.
 * 
 * <b> Note: DSQM is only applicable to FM </b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] dsqm: Pointer to DSQM output data. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -1 - Not FM band <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 *    dsqm is 0 when return value is not 0.
 */
 int_t HDR_get_dsqm(const HDR_instance_t* hdr_instance, HDR_dsqm_t* dsqm);

/**
 * @brief Returns current DSQM information.
 *
 * Digital Signal Quality Metric(DSQM) is computed every 16 Layer 1 symbols or (~46.6ms)
 * It returns this value from demod channel specific to "hdr_instance" instance.
 * 
 * <b> Note: DSQM is only applicable to FM </b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] dsqm: Pointer to DSQM output data. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -1 - Not FM band <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 *    dsqm is 0 when return value is not 0.
 */
 int_t HDR_get_demod_dsqm(const HDR_instance_t* hdr_instance, HDR_dsqm_t* dsqm);

 /**
 * @brief Returns filtered DSQM value
 *
 * A filtered (see #HDR_set_dsqm_filt_time_const()) DSQM computed by the HD Radio Library
 * In MRC mode, it returns combined (Master & Slave) filtered DSQM value when master instance is used. 
 * If slave instance is used, then it returns 0 with error code HDR_ERROR_INSTANCE_TYPE.
 * In non-MRC mode, it returns value from demod channel.

 * <b> Note: DSQM is only applicable to FM </b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] filt_dsqm: Pointer to filtered DSQM value. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -1 - Not FM band <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 *    filt_dsqm is 0 when return value is not 0. 
 */
int_t HDR_get_filt_dsqm(const HDR_instance_t* hdr_instance, uint_t * filt_dsqm);

/**
 * @brief Returns filtered DSQM value
 *
 * A filtered (see #HDR_set_dsqm_filt_time_const()) DSQM computed by the HD Radio Library
 * It returns this value from demod channel specific to "hdr_instance" instance.
 * 
 * <b> Note: DSQM is only applicable to FM </b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] filt_dsqm: Pointer to filtered DSQM value. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -1 - Not FM band <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 *    filt_dsqm is 0 when return value is not 0.
 */
int_t HDR_get_demod_filt_dsqm(const HDR_instance_t* hdr_instance, uint_t * filt_dsqm);

/**
 * @brief Overwrites Filtered DSQM value calculated by the library
 *
 * This function may be used if it is desired to over-write the internal calculation and
 * force Filtered DSQM to a certain value. One typical use is to force Filtered DSQM to 0xFFFF
 * once a desired HD channel is acquired. This provides hysteresis so that Filtered DSQM cannot
 * then immediately drop below the desired detection threshold due to channel noise.
 *
 * <b>Note: The value will be used for the next calculation of the filtered DSQM and will be
 * overwritten when the new value is calculated. </b>
 *
 * @param hdr_instance: Pointer to the HDR Library instance handle
 * @param dsqm_value: Filtered DSQM value
 * @returns
 *     0 - Success <br>
 *    -1 - Not FM band <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_set_filt_dsqm(const HDR_instance_t* hdr_instance, uint_t dsqm_value);

/**
 * @brief Retrieves DSQM filter time constant configuration value
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] time_constant: Pointer to time constant parameter. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 * @see #HDR_set_dsqm_filt_time_const()
 */
int_t HDR_get_dsqm_filt_time_const(const HDR_instance_t* hdr_instance, uint_t * time_constant);

/**
 * @brief Modifies DSQM filter time constant
 *
 * <pre>
 *
 * Modifies alpha where:
 *
 * alpha = 1 / (8 * [value of this config parameter])
 *
 * Filtered_DSQM(n) = alpha * DSQM(n) + (1 - alpha) * Filtered_DSQM(n - 1)
 *
 * DSQM is computed every ~46.4 ms, therefore:
 *
 * Time Constant(in seconds) = 0.0464s * 8 * [value of this config parameter]
 *
 * Value ranges from 1 to 255. Recommended value = 32 (11.9 seconds)
 *
 * </pre>
 *
 * <b> Note: DSQM is only applicable to FM </b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] time_constant: New time constant configuration parameter
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_set_dsqm_filt_time_const(const HDR_instance_t* hdr_instance, uint_t time_constant);

/**
 * @brief HDR Library sample slip information
 */
typedef struct{
    /** Accumulated number and direction of sample slips over a period of time */
    int_t num_samples;
    /**
     * Period of time over which the number of sample slips was calculated. Measured in
     * symbols(~2.9ms/symbol - FM, ~5.8ms/symbol - AM).
     */
    uint_t symbol_count;
}HDR_bb_sample_slips_t;

/**
 * @brief Returns baseband sample slip information calculated by the HDR library
 *
 * The sample slip information is reset to zero after a successful call to this function.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] sample_slips: Pointer to sample slip information. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_bb_sample_slips(const HDR_instance_t* hdr_instance, HDR_bb_sample_slips_t* sample_slips);

/**
 * @brief Returns baseband sample slip information from the demodulator specific to "hdr_instance".
 * The sample slip information is reset to zero after a successful call to this function.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] sample_slips: Pointer to sample slip information. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 *    sample_slips structure value is 0 when return value is not 0
 */
int_t HDR_get_demod_bb_sample_slips(const HDR_instance_t* hdr_instance, HDR_bb_sample_slips_t* sample_slips);

/**
 * @brief Retrieves an estimate of the Rx clock offset. 
 * In MRC case, it gets this information from slave instance if master instance is not acquired.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] ppm: Pointer to clock offset estimate in ppm. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -1 - PPM estimate is not ready <br> 
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 *    ppm is 0 when return value is not 0.
 */
int_t HDR_get_clock_offset(const HDR_instance_t* hdr_instance, int_t* ppm);

/**
 * @brief Retrieves an estimate of the Rx clock offset for demod channel specific to "hdr_instance".
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] ppm: Pointer to clock offset estimate in ppm. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -1 - PPM estimate is not ready <br> 
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 *    ppm is 0 when return value is not 0.
 */
int_t HDR_get_demod_clock_offset(const HDR_instance_t* hdr_instance, int_t* ppm);

/**
 * @brief Retrieves carrier frequency offset in Hz. 
 * In MRC case, it gets this information from slave instance if master instance is not acquired.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] freq_offset: Pointer to carrier frequency offset value in Hz. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 *    freq_offset is 0 when return value is not 0.
 */
int_t HDR_get_freq_offset(const HDR_instance_t* hdr_instance, int32_t* freq_offset);

/**
 * @brief Retrieves carrier frequency offset in Hz for the demod channel specific to "hdr_instance".
 * 
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] freq_offset: Pointer to carrier frequency offset value in Hz. Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 *    freq_offset is 0 when return value is not 0.
 */
int_t HDR_get_demod_freq_offset(const HDR_instance_t* hdr_instance, int32_t* freq_offset);

/**
 * @brief Returns indication of whether the Master & slave instance demodulation info is active or not.
 *
 * Demodulation is considered active if it's currently contributing to the channel
 * decoding process. Bit0 for Master instance and bit1 for slave instance.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @returns 
 *     2bit value. Bit0 for Master instance and bit1 for slave instance.
 *     1 - Active <br>
 *     0 - Not active.
 */
uint8_t HDR_get_mrc_demod_active_state(HDR_instance_t* hdr_instance);

#endif //HDR_PHY_H_
/** @} */
