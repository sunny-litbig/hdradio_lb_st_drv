/*******************************************************************************
*
* (C) copyright 2003 - 2016, iBiquity Digital Corporation, U.S.A.
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
 * @file hdrConfig.h
 * @brief HDR Configuration structure
 * @defgroup hdrConfig Configuration
 * @brief HDR Configuration API
 * @ingroup HdrApi
 * @{
 *
 * HDR Config API contains configuration for initializing HD Radio Library instance.
 */
#ifndef HDR_CONFIG_H_
#define HDR_CONFIG_H_

#include "hdrBlend.h"

#ifdef USE_CHANGED_HDRLIB_CB
/**
 * @brief Viterbi code rate configuration
 *
 * | enum                | band | gen poly          | K | code rate | puncture matrix                     |
 * |---------------------|------|-------------------|---|-----------|-------------------------------------|
 * |HDR_FM_RATE_1_OVER_3 | FM   | 133 \n 171 \n 165 | 7 | 1/3       | 1 1       \n 1 1       \n 1 1       |
 * |HDR_FM_RATE_2_OVER_5 | FM   | 133 \n 171 \n 165 | 7 | 2/5       | 1 1       \n 1 1       \n 1 0       |
 * |HDR_FM_RATE_1_OVER_2 | FM   | 133 \n 171 \n 165 | 7 | 1/2       | 1 1       \n 0 0       \n 1 1       |
 * |HDR_AM_RATE_1_OVER_3 | AM   | 561 \n 753 \n 711 | 9 | 1/3       | 1         \n 1         \n 1         |
 * |HDR_AM_RATE_5_OVER_12| AM   | 561 \n 657 \n 711 | 9 | 5/12      | 1 1 1 1 1 \n 0 0 0 1 1 \n 1 1 1 1 1 |
 * |HDR_AM_RATE_2_OVER_3 | AM   | 561 \n 753 \n 711 | 9 | 2/3       | 1 1 1 1   \n 0 0 0 0   \n 1 0 1 0   |
 */
typedef enum HDR_vit_code_rate_t{
    HDR_FM_RATE_1_OVER_3,  /**< HDR_FM_RATE_1_OVER_3 */
    HDR_FM_RATE_2_OVER_5,  /**< HDR_FM_RATE_2_OVER_5 */
    HDR_FM_RATE_1_OVER_2,  /**< HDR_FM_RATE_1_OVER_2 */
    HDR_AM_RATE_1_OVER_3,  /**< HDR_AM_RATE_1_OVER_3 */
    HDR_AM_RATE_5_OVER_12, /**< HDR_AM_RATE_5_OVER_12 */
    HDR_AM_RATE_2_OVER_3,  /**< HDR_AM_RATE_2_OVER_3 */
    HDR_INVALID_CFG = 99
}HDR_vit_code_rate_t;

/**
 * @brief Defines a Viterbi job handle
 */
typedef void* HDR_vit_job_handle_t;

/**
 * @brief Callback functions required by HD Radio Library
 */
typedef struct {
    /**
     * @brief Requests to run an HDR library task function
     *
     * HD Radio processing is split into multiple execution functions(tasks), where each execution function
     * can be called in the context of an OS task. When an HDR instance wants to run a specific task, it will
     * call this function to request that the outside framework execute specified task function on its behalf.
     *
     * @see hdrCore.h
     *
     * @param[in] hdr_instance: Pointer to the HDR Library instance
     * @param[in] task_id: Numeric identifier of the HDR task
     */
    void (*post_task)(HDR_instance_t* hdr_instance, HDR_task_id_t task_id);

    /**
     * @brief Locks a mutex, blocks if necessary
     *
     * Protects a resource or group of resources from concurrent access.
     *
     * @param[in] mutex: Pointer to mutex object
     * @param[in] timeout: Timeout period measured in milliseconds. Negative value means no timeout.
     * @returns
     *      0    Success <br>
     *     <0    Failure
     */
    int_t (*lock_mutex)(void* mutex, int_t timeout);

    /**
     * @brief Unlocks a mutex
     *
     * @param[in] mutex: Pointer to mutex object.
     * @returns
     *      0    Success <br>
     *     <0    Failure
     */
#ifdef USE_HDRLIB_2ND_CHG_VER
    int_t (*unlock_mutex)(void* mutex);
#else
	void (*unlock_mutex)(void* mutex);
#endif

    /**
     * @brief Blocks all other critical sections
     *
     * HD Radio library instance will call this function to gain exclusive access to a shared
     * resource and prevent access by another critical section function of this instance.
     * Critical sections are used by the library for time critical mutual exclusion with minimum
     * overhead, where a mutex locking may be too slow. The HDR library critical sections are
     * guranteed to be very short.
     *
     * @param[in] hdr_instance: Pointer to the HDR Library instance
     * @returns
     *      0    Success <br>
     *     <0    Failure
     */
    int_t (*enter_critical_section)(const HDR_instance_t* hdr_instance);

    /**
     * @brief Unblocks all other critical sections
     *
     * HD Radio instance will call this function to release access to a shared resource from a prior
     * call to HDRLIB_cb_enter_critical_section().
     *
     * @param [in] hdr_instance Pointer to the current HDR instance.
     */
    void (*exit_critical_section)(const HDR_instance_t* hdr_instance);

    /**
     * @brief Signals completion of HDR re-acquisition
     *
     * HDR library will call this function when an HDR instance completes re-acquisition.
     * Re-acquisition is triggered either when station is changed on the same band
     * or when self-triggered re-acquisition occurs due to adverse channel conditions.
     *
     * @param[in] hdr_instance: Pointer to the HDR Library instance
     * @param[in] band:         Band that will be used on the next acquisition
     */
#ifdef USE_HDRLIB_2ND_CHG_VER
	void (*reacq_complete)(const HDR_instance_t* hdr_instance, HDR_tune_band_t band);
#else
    void (*reacq_complete)(const HDR_instance_t* hdr_instance);
#endif
    /**
     * @brief Retrieves system time measured in milliseconds
     * @param[out] time_msec: Time in milliseconds
     * @returns
     *     0 - Success <br>
     *    <0 - Failure
     */
    int_t (*get_sys_time)(uint64_t* time_msec);

    /**
     * @brief Request the current OS thread to suspend
     *
     * @param[in] usec: Number of microseconds to sleep
     */
    void (*usleep)(uint_t usec);

    /**
     * @brief Queues a Viterbi job
     *
     * This function is called by the HDR library instance when there is data ready for
     * viterbi decoding. Viterbi job includes de-puncturing, viterbi decoding
     * and descrambling. The function must not block waiting for a job to finish.

     * If hardware Viterbi is not used, this function pointer should be set to NULL.
     *
     * @param [in] hdr_instance: Pointer to the HDR Library instance handle
     * @param [in] sd_in: Pointer to the buffer containing input soft decisions. (8 bits per soft decision)
     * @param [in] sd_count: Number of input soft decisions.
     * @param [in] block_number: Block number of input data.
     * @param [in] frame_number: Frame number of input data.
     * @param [in] timestamp: Timestamp of the input data.
     * @param [in] code_rate: Defines the code rate and generator polynomial set used in processing data.
     *                        For FM Digital Diversity, combining of soft decisions will be done in software, so
     *                        #HDR_FM_RATE_1_OVER_3 will be used to indicate that no
     *                        de-puncturing is required.
     * @param [out] bits_out: Pointer to decoded output buffer.
     * @param [out] vit_job:  Pointer to viterbi job handle. The outside framework should assign the handle an internal
     *                        structure that defines a Viterbi job handle. HDR library will use that handle to query the
     *                        result of that Viterbi job. (see #HDR_query_vit_job())
     * @returns
     *     0 - Success <br>
     *    <1 - Failure
     */
    int_t (*queue_vit_job)(const HDR_instance_t* hdr_instance, int8_t* sd_in, uint32_t sd_count,
                           uint32_t block_number, uint32_t frame_number, uint32_t timestamp,
                           HDR_vit_code_rate_t code_rate, uint8_t* bits_out, HDR_vit_job_handle_t* vit_job);
    /**
     * @brief Queries the status of a Viterbi job
     *
     * If hardware Viterbi is not used, this function pointer should be set to NULL.
     *
     * @param [in] hdr_instance: Pointer to the HDR Library instance handle.
     * @param [in] vit_job: Pointer to viterbi job handle.
     * @param [out] error_count: Number of errors corrected by Viterbi when job is completed. Optional for debugging.
     * @returns
     *     0 - Complete <br>
     *     1 - Pending <br>
     *    <0 - Error
     */
    int_t (*query_vit_job)(const HDR_instance_t* hdr_instance, HDR_vit_job_handle_t vit_job, uint_t* error_count);

    /**
     * @brief Removes specified Viterbi job from the processing queue
     *
     * If hardware Viterbi is not used, this function pointer should be set to NULL.
     *
     * This function is called by the HDR library when the results of a previously
     * queued viterbi job are no longer needed, such as during a re-acquisition. In this case,
     * the job handle must be reset to the unused state(NULL).
     *
     * @param [in] hdr_instance: Pointer to the HDR Library instance handle.
     * @param [in] vit_job: Viterbi job handle.
     * @returns
     *     0 - Success <br>
     *    <0 - Failure
     */
    int_t (*flush_vit_job)(const HDR_instance_t* hdr_instance, HDR_vit_job_handle_t vit_job);

    /**
     * @brief Enables cache for the specified memory region
     *
     * This function pointer can be NULL if cache configurable memory is not used.
     *
     * @param [in] hdr_instance: Pointer to the HDR Library instance
     * @param [in] block_num: Index of cache configurable memory block for which cache will be enabled
     * @see HDR_mem_set_uncached().
     * @returns
     *     0 - Success <br>
     *    <0 - Failure
     */
    int_t (*mem_set_uncached)(const HDR_instance_t* hdr_instance, uint_t block_num);

    /**
     * @brief Disables cache for the specified memory region HDR_instance_t
     *
     * This function will change memory regions #HDR_instance_t::cache_cfg_memory_0 or
     * #HDR_instance_t::cache_cfg_memory_1 be cachable or uncachable.
     *
     * Changing memory to cached/uncached allows HDR library to improve the performance of the
     * deinterleavers. The sparse, pseudo random nature of the deinterleaver operation does not benefit
     * from memory cache, and can actually evict data from cache that could be of benefit to other functions.
     * For some deinterleaver modes using uncached memory was found to lower the MIPS consumption, while for other
     * modes, cached memory was better.
     *
     * This function pointer can be NULL if cache configurable memory is not used.
     *
     * @param [in] hdr_instance: Pointer to the HDR Library instance
     * @param [in] block_num: Index of cache configurable memory block for which cache will be disabled
     * @returns
     *     0 - Success <br>
     *    <0 - Failure
     */
    int_t (*mem_set_cached)(const HDR_instance_t* hdr_instance, uint_t block_num);

    /**
     * @brief Notifies outside framework that HDR instance encountered an error
     *
     * Gives the outside framework the flexibility of dealing with errors in its own way in one central location.
     * Furthermore, it gives the developer the ability to break the execution precisely at the time
     * of the error encountered while registers and stack information are still fresh.
     *
     * @param[in] hdr_instance: Pointer to the HDR Library instance
     * @param[in] error: Error type
     * @param[in] caller_func_addr: Address of the calling HDR function
     * @param[in] func_name: Character string of the calling HDR function
     */
    void  (*error_handler)(const HDR_instance_t* hdr_instance, int_t error, intptr_t caller_func_addr, const char* func_name);

    /**
     * @brief Requests the status of audio alignment
     *
     * HD Radio library will output mute audio for MPS until audio is time and level aligned
     * with analog.
     *
     * @param[in] hdr_instance: Pointer to the HDR Library instance
     * @returns
     *     true  - MPS time/level alignmenet finished <br>
     *     false - Waiting for successful MPS time/level alignment
     */
    bool  (*is_mps_alignment_finished)(const HDR_instance_t* hdr_instance);

    /**
     * @brief Requests to send logging data to host
     *
     * @param [in] data: Pointer to data
     * @param [in] length: Length of data in bytes
     * @returns
     *     0 - Success <br>
     *    <0 - Failure
     */
    int_t (*logger_send)(uint8_t* data, uint_t length);

#ifdef USE_HDRLIB_2ND_CHG_VER
    /**
     * @brief Notifies framework to prepare for audio alignment
     *
     * HDR library will call this function when an HDR instance determines that
     * it is necessary to align the digital and and analog audio.
     *
     * @param[in] hdr_instance: Pointer to the HDR Library instance
     */
    void (*audio_align)(const HDR_instance_t* hdr_instance);

    /**
     * @brief Notifies framework when audio alignment has been completed
     */
    void (*alignment_complete)(void);

    /**
     * @brief Requests the analog timestamp corresponding to the next analog audio output frame to be played.
     *
     * @returns analog timestamp
     */
    uint32_t (*get_analog_input_timestamp)(void);
#endif

}HDR_callbacks_t;
#endif // USE_CHANGED_HDRLIB_CB

/**
 *@brief Specifies which HD logical channels will be decoded in hardware Viterbi
 */
typedef union {
    struct {
        uint32_t fm_pids:1;
        uint32_t fm_p1:1;
        uint32_t fm_p2:1;
        uint32_t fm_p3:1;
        uint32_t fm_p4:1;
        uint32_t am_pids:1;
        uint32_t am_p1:1;
        uint32_t am_p3:1;
    };
    uint32_t all;
}HDR_hw_vit_enabled_t;

/**
 * @brief HD Radio Library configuration structure
 *
 * Contains all configuration parameters for initializing an HDR instance. System integrator should allocate
 * an instance of this structure, assign valid values and pass the pointer to HDR_init().
 *
 * @see #HDR_init()
 */
struct HDR_config_t{
#ifdef USE_CHANGED_HDRLIB_CB
    /**
     * @brief HD Radio callback functions
     */
    HDR_callbacks_t callbacks;
#endif
    /**
     * @brief Initial blend parameters
     *
     * Can be later modified at runtime.
     */
    HDR_blend_params_t blend_params;
    /**
     * @brief Initial advanced blend parameters
     *
     * Can be later modified at runtime.
     */
    HDR_blend_adv_params_t adv_blend_params;
    /**
     * @brief Enables/Disables emergency alerts
     *
     * @see hdrAlerts.h
     */
    bool emergency_alerts_enabled;
    /**
     * @brief Enables/Disables MP11 primary service mode
     *
     * If disabled, when MP11 mode is received the radio will fall back to MP3.
     */
    bool mp11_enabled;
    /**
     * @brief Initializes DSQM filter time constant
     *
     * Can be modified later at runtime. Recommended value = 32 (11.9 seconds).
     *
     * @see HDR_set_dsqm_filt_time_const()
     */
    uint_t dsqm_filt_time_const;
    /**
     * @brief AAS pakcet time-to-live(in milliseconds)
     *
     * The amount of time that HD Radio Library guarantees to hold an AAS packet in its memory.
     * If an AAS packet is not retrieved before this time, it will automatically get flushed.
     * The larger the value the more memory will be required by the HDR instance.
     *
     * Minimum value: 1000(1s).
     */
    uint_t aas_packet_ttl;
    /**
     * @brief Enables/Disables Large-Object-Transfer(LOT)
     *
     * If LOT is disabled, no memory related to LOT processing will be allocated.
     *
     * @see hdrAas.h
     */
    bool lot_enabled;
    /**
     * @brief Amount of memory(in bytes) available for LOT objects
     *
     * Minimum recommended value is 512KB but more can be provided if desired.
     * If #lot_enabled is set to false, this parameter is ignored.
     *
     * <b>Note: Pool size includes overhead for LOT processing; therefore, actual space
     *          available for LOT objects is less than this number.</b>
     */
    uint_t lot_pool_size;
    /**
     * @brief Tells the library if hardware Isolation filter is enabled
     *
     * If Isolation filter is executed in hardware, the HD Radio library will bypass
     * its internal software ISO filter. If hardware ISO is enabled,
     * use HDR_sideband_inputs() function for FM to pass IS samples into the library.
     * Otherwise, use the standard HDR_baseband_input() function.
     */
    bool hw_iso_enabled;
    /**
     *@brief Tells the library if hardware Viterbi is enabled
     *
     * Hardware Viterbi enabled/disabled switch can be controlled
     * per HD logical channel. For those logical channels that the
     * hardware Viterbi is enabled, the HDR library will call HDR_queue_vit_job()
     * to run the Viterbi job.
     */
    HDR_hw_vit_enabled_t hw_vit_enabled;
    /**
     * @brief Enables/Disables tx digital audio gain for MPS program
     *
     * It is recommended that MPS tx audio gain is disabled when automatic level alignment is used.
     * If this flag is set to true, #HDR_get_tx_dig_audio_gain() will always return 0 when MPS(Program 1)
     * program is selected.
     */
    bool mps_tx_audio_gain_enabled;
    /**
     * @brief Disables internal time alignment
     *
     * Unless disabled, the HD Radio library will automatically correct for the jitter in the system using timestamps
     * to make the propagation delay through the system constant to time-align digital audio with analog audio.
     */
    bool disable_time_alignment;

#ifdef USE_HDRLIB_2ND_CHG_VER
    /**
     * @brief Notiifes library about synchronization between analog and digital audio
     *
     * When the analog and digital audio are derived from the same clock source, this parameter may be set \c true and
     * audio alignment will be performed using bb input sample counts.  When the analog and digital audio are not
     * synchronous, set the parameter to \c false and audio alignment will be performed using an external clock (such
     * as the CPU cycle count register or a clock provided by linux syscall).  In this case the framework must read
     * the external clock.
     */
    bool audio_and_digital_clocks_synced;
#endif

    /**
     * @brief Error message for HDR configuration
     *
     * This pointer will be assigned to a buffer containing an error message if HDR Library
     * fails to initialize due to invalid HDR configuration parameter value.
     */
    char* error_message;
};

/**
 * @brief Retrieves current HDR configuration
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param [out] hdr_config: Pointer to the HDR configuration parameters. Must be allocated by the caller.
 * @returns
 *      0 - Success <br>
 *     -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_config(const HDR_instance_t* hdr_instance, HDR_config_t* hdr_config);

/**
 * @brief Retrieves default instance configuration
 *
 * Simplifies initialization when most parameters are left with default values
 *
 * @param [out] hdr_config: Pointer to the HDR configuration parameters. Must be allocated by the caller.
 * @returns
 *      0 - Success <br>
 *     -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_default_config(HDR_config_t* hdr_config);

/**
 * @brief Tells the caller if hardware ISO enabled or disabled
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param hw_iso_enabled: true - ISO is executed externally in hardware
 *                        false - ISO is executed internally in software
 * @returns
 *      0 - Success <br>
 *     -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_get_hw_iso_enabled(const HDR_instance_t* hdr_instance, bool* hw_iso_enabled);

#endif //HDR_CONFIG_H_

/** @} */
