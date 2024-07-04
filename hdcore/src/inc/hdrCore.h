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
 * @defgroup HdrApi API
 *
 * @file hdrCore.h
 * @brief DTS HD Radio Library Core API
 * @mainpage DTS HD Radio Library API Definitions
 * @copyright 2017, DTS Inc., U.S.A.
 * @defgroup hdrCore Core
 * @brief HD Radio Core API functions and definitions
 * @ingroup HdrApi
 * @{
 *
 * The DTS  HD Radio system is designed to permit a smooth evolution from current analog amplitude
 * modulation (AM) and frequency modulation (FM) to a fully digital in-band on-channel (IBOC) system.
 * This system delivers digital audio and data services to mobile, portable, and fixed receivers from
 * terrestrial transmitters in the existing medium frequency (MF) and very high frequency (VHF) radio
 * bands. Broadcasters may continue to transmit analog AM and FM simultaneously with the new,
 * higher-quality and more robust digital signals, allowing themselves and their listeners to convert
 * from analog to digital radio while maintaining their current frequency allocations.
 *
 * HDR Core API provides the most essential API definitions for initializing and running
 * the HD Radio Library. All other API modules include this file.
 */
#ifndef HDR_CORE_H_
#define HDR_CORE_H_

#include "hdrBasicTypes.h"

/**
 * @brief "Generic return error types that pertain to most API functions
 */
typedef enum {
    /** (-100) Invalid input parameter passed into API function */
    HDR_ERROR_INVAL_ARG = -100,
    /** (-101) API request failed because HD Radio was in the middle of re-acquistion */
    HDR_ERROR_REACQ     = -101,
    /** (-102) API request failed because HD Radio is configured to HDR_BAND_IDLE */
    HDR_ERROR_IDLE      = -102,
    /** (-103) API request failed because HD Radio library was not initialized */
    HDR_NOT_INITIALIZED = -103,
    /** (-104) API request failed because the instance type(#HDR_instance_type_t) doesn't support it. */
    HDR_ERROR_INSTANCE_TYPE = -104,
}HDR_error_code_t;

/**
 * @brief Enumeration for AM/FM/IDLE band selection
 */
typedef enum HDR_tune_band_t{
    HDR_BAND_AM                 = 0x00, /**< AM operation */
    HDR_BAND_FM                 = 0x01, /**< FM operation */
    /**
     * Some HD Radio Library configuration parameters can only be changed during IDLE mode.
     * This is done to ensure a better user experience.
     */
    HDR_BAND_IDLE               = 0x63
}HDR_tune_band_t;

/**
 * @brief HD Radio instance configuration type
 */
typedef enum{
    HDR_FULL_INSTANCE,         /**< Full HDR instance(audio + data) */
    HDR_DATA_ONLY_INSTANCE,    /**< Partial HDR instance that produces no audio but has data capabilities */
    HDR_DEMOD_ONLY_INSTANCE,   /**< Partial, demod only HDR instance used as a slave for maximum-ratio combining(MRC). This type of instance can
    be used with a FULL instance as well as a DATA ONLY instance */
    HDR_NUM_INSTANCE_TYPES
}HDR_instance_type_t;

/**
 * @brief Opaque structure type for private HDR Library usage
 */
typedef struct HDR_priv_t HDR_priv_t;

/**
 * @brief Defines the HDR instance
 *
 * Cached, cache configurable, and dma memories must be allocated by the HDR Library User
 * using the sizes defined in this header file, and the memory pointers must be initialized
 * to point to these respective memory regions. The HDR Library will optimally allocate its
 * internal data structures across these regions.
 *
 * Multiple HD Radio Instances can be created. A copy of this data structure is required for
 * each instance and is used by the library to identify the instance.
 */
typedef struct {
    uint8_t* cached_memory;		/**< Pointer to regular cached memory */

    /**
     * Cache configurable memory is optional but can be used to improve MIPS performance.
     *
     * Changing memory to cached/uncached allows HDR library to improve the performance of the
     * deinterleavers. The pseudo random nature of the deinterleaver operation does not
     * benefit from memory cache, and can actually evict data from cache that could be of benefit
     * to other functions. For some deinterleaver modes, using uncached memory was found to lower
     * the MIPS consumption, while for other modes, cached memory is better.
     *
     * If used, HDR library will call functions HDR_mem_set_cached() and HDR_mem_set_uncached()
     * to control the memory regions. Both block pointers can be set to NULL if the feature is not
     * wanted.
     */
    uint8_t* cache_cfg_memory_0;  /**< Pointer to cache configurable memory block 0 */
    uint8_t* cache_cfg_memory_1;  /**< Pointer to cache configurable memory block 1*/

    /** Pointer to dma memory. Only needed if hardware Viterbi is used; can be set to NULL otherwise */
    uint8_t* dma_memory;
    uint_t cached_mem_size;       /**< Size of cached memory in bytes */
    uint_t cached_cfg0_size;      /**< Size of cache configurable memory block 0 in bytes */
    uint_t cached_cfg1_size;      /**< Size of cache configurable memory block 1 in bytes */
    uint_t dma_size;              /**< Size of dma memory in bytes */
    void* handle_cache_cfg;             /**< Handle to cache configuration user-defined structure */
    void* mutex;                        /**< Handle to mutex object used by the HDR instance */
    void* mrc_mutex;
#ifdef USE_HDRLIB_2ND_CHG_VER
    void* reacq_ctl_mutex;              /**< Mutex to control when reacq may be performed */
#endif
    HDR_instance_type_t instance_type;  /**< Configures library to the specified instance type */
    uint_t instance_number;             /**< Instance identifier number */
    HDR_priv_t* priv;          			/**< Used internally in the HDR library. DO NOT MODIFY. */
} HDR_instance_t;

/**
 * @brief #HDR_init return error codes
 */
typedef enum {
    HDR_INVAL_ARG            = -3, /**< Invalid argument value passed into #HDR_init() */
    HDR_INVALID_CONFIG_PARAM = -2, /**< At least one invalid configuration parameter detected */
    HDR_INSUFFICIENT_MEMORY  = -1, /**< Insufficient total memory provided to initalize HDR Library for specified configuration. */
    HDR_NO_ERROR             =  0, /**< Initialization was successful */
}HDR_init_error_t;

/**
 * @brief Contains information about memory usage of HD Radio Library
 *
 * Memory usage will vary depending on the instance type and features requested.
 * Memory usage is specified in bytes.
 */
typedef struct {
    uint_t cached;       /**< Cached memory */
    uint_t cached_cfg0;  /**< Cache configurable block 0 memory */
    uint_t cached_cfg1;  /**< Cache configurable block 1 memory */
    uint_t dma;          /**< DMA memory used */
    uint_t total_memory; /**< Total memory used */
}HDR_mem_usage_t;

/**
 * @brief Forward declaration of struct HDR_config_t.
 *
 * Declaration is located in hdrConfig.h.
 */
typedef struct HDR_config_t HDR_config_t;

/**
 * @brief Initializes HD Radio Library instance
 *
 * Call HDR_init() once for every #HDR_instance_t that is allocated by the system.
 *
 * For every HDR instance, this function must be called only once and it must be the first
 * HDR Library function to be called or unexpected behavior may occur.
 * After successful execution, HDR_init() will initialize the HDR instance to HDR_BAND_IDLE.
 * Use HDR_set_band() to switch to the desired band configuration.
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance
 * @param [in] hdr_config: Pointer to initial library configuration parameters
 *                         HDR Library will copy the whole structure to its internal memory, so
 *                         after HDR_init() returns, the memory can be recycled if necessary.
 * @param[out] mem_usage: Pointer to memory usage output data structure. Must be allocated by the caller.
 *                        Set to NULL if not needed.
 * @returns #HDR_init_error_t
 */
int_t HDR_init(HDR_instance_t* hdr_instance, HDR_config_t* hdr_config, HDR_mem_usage_t* mem_usage);

/**
 * @brief Schedules a band switch for the specified HDR instance
 *
 * The actual band switch will occur during the next HDR_exec() call for that instance.
 *
 * HDR_init() starts the HDR instance in IDLE mode; therefore, this function
 * will need to be called to start the HDR processing.
 *
 * @param [in] hdr_instance Pointer to the current HDR instance.
 * @param [in] band Frequency Band, (AM, FM or IDLE).
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_set_band(HDR_instance_t* hdr_instance, HDR_tune_band_t band);

/**
 * @brief Returns selected band - AM, FM, or Idle
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance
 * @returns enum values HDR_BAND_AM, HDR_BAND_FM or HDR_BAND_IDLE.
 */
HDR_tune_band_t HDR_get_band_select(const HDR_instance_t* hdr_instance);

/**
 * @brief Force HDR instance to re-acquire(reinitialize)
 *
 * The instance will remain configured to the same band.
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_reacquire(HDR_instance_t* hdr_instance);

/**
 * @brief Passes full rate input baseband samples to the HDR Library instance.
 *
 * 744.1875 kHz sample rate for FM, 46.51171875 kHz for AM.
 *
 * This function should be called immediately before HDR_exec(). The data isn't copied internally so it's important
 * not to overwrite the buffer before the next HDR_exec() is finished.
 *
 * For AM, always use this function but for FM, use this function when the isolation filter is implemented
 * within the HD Radio Library; otherwise, use HDR_sideband_inputs().
 *
 * The input number of samples must be a multiple of HD symbol size - 2160 samples for FM
 * and 270 for AM. The minimum number of samples is one HD symbol.
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance.
 * @param [in] bb_input_samples: Pointer to buffer containing baseband IQ samples.
 * @param [in] num_samples: Number of input IQ samples in the buffer.
 * @param [in] timestamp: When <tt> hdr_instance->priv->hdrConfig->audio_and_digital_clocks_synced == false </tt>,
 *                             framework clock corresponding to when the last sample of \c bb_input_samples was received.
 *                        When <tt> hdr_instance->priv->hdrConfig->audio_and_digital_clocks_synced == true </tt>,
 *                             this parameter may be ignored (bb sample count base timestamp will be generated internally).
 *
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
#ifdef USE_HDRLIB_2ND_CHG_VER
int_t HDR_baseband_input(const HDR_instance_t* hdr_instance, int16c_t* bb_input_samples, uint_t num_samples, uint32_t timestamp);
#else
int_t HDR_baseband_input(const HDR_instance_t* hdr_instance, int16c_t* bb_input_samples, uint_t num_samples);
#endif

/**
 * @brief Passes lower and upper sideband input samples to the HDR Library instance
 *
 * 186.0469 kHz input sample rate.
 *
 * This function is only used for FM and when isolation filter is implemented outside the
 * HDR library; otherwise, use function HDR_baseband_input().
 *
 * This function should be called before HDR_exec().
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance.
 * @param [in] lsb_in: Pointer to lower sideband complex IQ input samples.
 * @param [in] usb_in: Pointer to upper sideband complex IQ input samples.
 * @param [in] num_samples: Number of IQ input samples.

 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sideband_inputs(const HDR_instance_t* hdr_instance, int16c_t* usb_in, int16c_t* lsb_in,
                          uint_t num_samples);


/**
 * @brief Connects two instances in Maximal Ratio Combining(MRC) mode
 *
 * MRC connect can be called anytime after both instances are successfully initialized.
 * The MRC connection is permanent and therefore it only needs to be done once. After connection
 * is established, MRC is enabled by default but can be disabled anytime via function,
 * #HDR_mrc_disable() without breaking the connection.
 *
 * Although two "full" HDR instances can be placed in MRC mode, it is recommended to configure one instance
 * to be #HDR_DEMOD_ONLY_INSTANCE as this would save a significant amount of memory. If one instance
 * is #HDR_DEMOD_ONLY_INSTANCE, it will automatically become "slave" no matter which of the two function
 * parameters it is assigned to. But, if neither instance type is #HDR_DEMOD_ONLY_INSTANCE, instance1
 * will automatically become master and instance2 will become slave.
 *
 * @param[in] instance1: Pointer to one of the MRC instances - could be either MASTER or SLAVE.
 * @param[in] instance2: Pointer to the other MRC instance - could be either SLAVE or MASTER.
 * @returns
 *     0 - Success <br>
 *    -1 - Both instances can't be MRC slave <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_mrc_connect(HDR_instance_t* instance1, HDR_instance_t* instance2);

/**
 * @brief Enable Maximal Ratio Combining(MRC) mode
 *
 * MRC can only be enabled if two instances are connected in MRC mode. See #HDR_mrc_connect().
 *
 * @param[in] hdr_instance: Pointer to one of the MRC instances(doesn't matter which)
 * @returns
 *     0 - Success <br>
 *    -1 - Instances are not connected
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_mrc_enable(const HDR_instance_t* hdr_instance);

/**
 * @brief Disable Maximal Ratio Combining(MRC) mode
 *
 * MRC can only be disabled if two instances are connected in MRC mode. See #HDR_mrc_connect().
 *
 * @param[in] hdr_instance: Pointer to one of the MRC instances(doesn't matter which)
 * @returns
 *     0 - Success <br>
 *    -1 - Instances are not connected
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_mrc_disable(const HDR_instance_t* hdr_instance);

/**
 * @brief Returns Maximal Ratio Combining(MRC) mode enabled status
 *
 * @param[in] hdr_instance: Pointer to one of the MRC instances(doesn't matter which)
 * @param[out] mrc_enabled: Pointer to the MRC enabled value. Must be allocated by the caller.

 * @returns
 *     0 - Success <br>
 *    -1 - Instances are not connected
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_mrc_enabled(const HDR_instance_t* hdr_instance, bool* mrc_enabled);

#ifdef USE_HDRLIB_2ND_CHG_VER
/**
 * @brief Returns Maximal Ratio Combining(MRC) instance for the input instance
 * Usage:
 * HDR_instance_t* mrc_instance;
 * HDR_get_mrc_instance(hdr_instance,&mrc_instance)
 *
 * @param[in] hdr_instance: Pointer to one of the MRC instances(doesn't matter which)
 * @param[in[ mrc_instance: Pointer type to HDR_instance_t*
 * @returns
 *    0  - Success.mrc_instance will point to the MRC connected Instance.<br>
 *         If an instance of type DEMOD_ONLY_INSTANCE is passed as an argument using Parameter
 *         then the mrc_instance will point to the HDR_FULL_INSTANCE/HDR_DATA_ONLY_INSTANCE connected to the DEMOD_ONLY_INSTANCE.
 *         If an instance of type HDR_FULL_INSTANCE/HDR_DATA_ONLY_INSTANCE is passed as an argument using the Parameter 1, then
 *         the mrc_instance pointer will point to the HDR_DEMOD_INSTANCE connected to the HDR_FULL_INSTANCE/HDR_DATA_ONLY_INSTANCE
 *    -1 - hdr_instance (Parameter 1) does not have an MRC instance connected to it.<br>
 *    -2 - hdr_instance (Parameter 1) has an instance connected to it but MRC combining is disabled.  *
 * ...-100 or less - Generic error.(see # HDR_error_code_t)
 */
int HDR_get_mrc_instance(const HDR_instance_t* hdr_instance, HDR_instance_t** mrc_instance);
#endif

/**
 * @brief List of HDR "task" functions
 *
 * HDR Library is split into 8 processes that can be executed in context of 8 or less OS tasks.
 */
typedef enum {
    HDR_FRONT_END_TASK = 0,
    HDR_DEMOD_TASK,
    HDR_BIT_PROC_TASK,
    HDR_TRANSPORT_TASK,
    HDR_AUDIO_DECODER_TASK,
    HDR_TOTAL_TASKS
}HDR_task_id_t;

/**
 * @brief HDR front end task function
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 *
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_front_end_task(HDR_instance_t* hdr_instance);

/**
 * @brief HDR demodulation task function
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 *
  * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_demod_task(HDR_instance_t* hdr_instance);

/**
 * @brief HDR bit-processing task function
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 *
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_bit_proc_task(HDR_instance_t* hdr_instance);

/**
 * @brief HDR audio and data transport task function
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 *
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_transport_task(HDR_instance_t* hdr_instance);

/**
 * @brief HDR audio decoder task function
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 *
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_audio_decoder_task(HDR_instance_t* hdr_instance);

/**
 * @brief Returns a pointer to a character string containing the HD library version
 *
 * <pre>
 * Example HDR-E-1.2.3.4
 *
 * HDR - HD Radio Library
 *   E - Engineering Release
 *   1 - Major. Major features and/or API changes since last version.
 *   2 - Minor. Some additional features and/or API changes since last version.
 *   3 - Patch. Only bug fixes and no additional features or API changes.
 *   4 - For internal use.
 * </pre>
 *
 * @return Character string identifying HDR library version
 */
const char* HDR_get_version_string(void);

#endif //HDR_CORE_H_

/**
 * @}  doxygen end hdrCore
 */
