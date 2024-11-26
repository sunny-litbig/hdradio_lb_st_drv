/*******************************************************************************

*   FileName : tchdr_hdlibcb.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio core library callback functions and definitions
                  Callback functions used by the HD Radio Library
                  Application framework must provide

********************************************************************************
*
*   TCC Version 1.0

This source code contains confidential information of Telechips.

Any unauthorized use without a written permission of Telechips including not
limited to re-distribution in source or binary form is strictly prohibited.

This source code is provided "AS IS" and nothing contained in this source code
shall constitute any express or implied warranty of any kind, including without
limitation, any warranty of merchantability, fitness for a particular purpose
or non-infringement of any patent, copyright or other third party intellectual
property right.
No warranty is made, express or implied, regarding the information's accuracy,
completeness, or performance.

In no event shall Telechips be liable for any claim, damages or other
liability arising from, out of or in connection with this source code or
the use in the source code.

This source code is provided subject to the terms of a Mutual Non-Disclosure
Agreement between Telechips and Company.
*
*******************************************************************************/
#ifndef TCHDR_HDLIBCB_H__
#define TCHDR_HDLIBCB_H__

/***************************************************
*               Include                            *
****************************************************/
#include "hdrConfig.h"

/***************************************************
*               Defines                            *
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
*               Enumeration                        *
****************************************************/

/***************************************************
*               Typedefs                           *
****************************************************/

/***************************************************
*               Constant definitions               *
****************************************************/

/***************************************************
*               Variable definitions               *
****************************************************/

/***************************************************
*               Function declaration               *
****************************************************/
#ifdef USE_CHANGED_HDRLIB_CB
/**
 * brief Callback function template to post an HDRadio Taask
 * param[in] task_id: Task ID to be posted to run.
 */
void HDLIB_cb_post_task(HDR_instance_t* hdr_instance, HDR_task_id_t task_id);

/**
 * brief Callback function template to define start of a critical function so that access to shared resources are blocked.
 * param[in] hdr_instance: Pointer to the HDRadio Instance.
 */
int_t HDLIB_cb_enter_critical_section(const HDR_instance_t* hdr_instance);

/**
 * brief Callback function template to define end of a critical function so that access to shared resources are now freed.
 * param[in] hdr_instance: Pointer to the HDRadio Instance.
 */
int_t HDLIB_cb_exit_critical_section(const HDR_instance_t* hdr_instance);

/**
 * brief Callback function template to define implementation for a mutex lock.
 * param[in] mutex: Pointer to the mutex.
 * param[in] timeout: Timeout value for the mutex.
 */
int_t HDLIB_cb_lock_mutex(void* mutex, int_t timeout);

/**
 * brief Callback function template to define implementation for a mutex free.
 * param[in] mutex: Pointer to the mutex.
 */
#ifdef USE_HDRLIB_2ND_CHG_VER
int_t HDLIB_cb_unlock_mutex(void* mutex);
#else
void HDLIB_cb_unlock_mutex(void* mutex);
#endif

/**
 * brief Callback function template to define a micro second sleep.
 * param[in] usec: No. of microseconds to be in sleep mode.
 */
void HDLIB_cb_usleep(U32 usec);


int_t HDLIB_cb_get_sys_time(ULONG* time_msec);



/**
 * brief Definition for queueing a Viterbi job
 *
 * This function is called by the HDR library instance when there is data ready for
 * viterbi decoding. Viterbi job includes de-puncturing, viterbi decoding
 * and descrambling. The function must not block waiting for a job to finish.
 *
 * If hardware Viterbi is not used, this function still needs to be implemented for
 * the application to compile. It can be an empty function, which the library will never call.
 *
 * param [in] hdr_instance: Pointer to the HDR Library instance handle
 * param [in] sd_in: Pointer to the buffer containing input soft decisions. (8 bits per soft decision)
 * param [in] sd_count: Number of input soft decisions.
 * param [in] block_number: Block number of input data.
 * param [in] frame_number: Frame number of input data.
 * param [in] timestamp: Timestamp of the input data.
 * param [in] code_rate: Defines the code rate and generator polynomial set used in processing data.
 *                        For FM Digital Diversity, combining of soft decisions will be done in software, so
 *                        #HDR_FM_RATE_1_OVER_3 will be used to indicate that no
 *                        de-puncturing is required.
 * param [out] bits_out: Pointer to decoded output buffer.
 * param [out] vit_job:  Pointer to viterbi job handle. The outside framework should assign the handle an internal
 *                        structure that defines a Viterbi job handle. HDR library will use that handle to query the
 *                        result of that Viterbi job. (see #HDR_query_vit_job())
 * returns
 *     0 - Success <br>
 *    <1 - Failure
 */
int_t HDLIB_cb_queue_vit_job(const HDR_instance_t* hdr_instance, S8C* sd_in, U32 sd_count,
                             U32 block_number, U32 frame_number, U32 timestamp,
                             HDR_vit_code_rate_t code_rate, U8* bits_out, HDR_vit_job_handle_t* vit_job);


/**
 * brief Queries the status of a Viterbi job
 *
 * If hardware Viterbi is not used, this function still needs to be implemented for
 * the application to compile. It can be an empty function, which the library will never call.
 *
 * param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * param [in] vit_job: Pointer to viterbi job handle.
 * param [out] error_count: Number of errors corrected by Viterbi when job is completed. Optional for debugging.
 * returns
 *     0 - Complete <br>
 *     1 - Pending <br>
 *    <0 - Error
 */
int_t HDLIB_cb_query_vit_job(const HDR_instance_t* hdr_instance, HDR_vit_job_handle_t vit_job, U32* error_count);

/**
 * brief Removes specified Viterbi job from the processing queue
 *
 * If hardware Viterbi is not used, this function still needs to be implemented for
 * the application to compile. It can be an empty function, which the library will never call.
 *
 * This function is called by the HDR library when the results of a previously
 * queued viterbi job are no longer needed, such as during a re-acquisition. In this case,
 * the job handle must be reset to the unused state(NULL).
 *
 * param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * param [in] vit_job: Viterbi job handle.
 * returns
 *     0 - Success <br>
 *    <0 - Failure
 */
int_t HDLIB_cb_flush_vit_job(const HDR_instance_t* hdr_instance, HDR_vit_job_handle_t vit_job);

/**
 * brief Framework should provide implementation to cache for the specified memory region
 *
 * The implementation of this function can be empty if cache configurable memory is not used.
 *
 * param [in] hdr_instance: Pointer to the HDR Library instance
 * param [in] block_num: Index of cache configurable memory block for which cache will be enabled
 * see HDR_mem_set_uncached().
 * returns
 *     0 - Success <br>
 *    <0 - Failure
 */
int_t HDLIB_cb_mem_set_cached(const HDR_instance_t* hdr_instance, U32 block_num);


/**
 * brief Framework should provide implementation to uncache for the specified memory region HDR_instance_t
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
 * The implementation of this function can be empty if cache configurable memory is not used.
 *
 * param [in] hdr_instance: Pointer to the HDR Library instance
 * param [in] block_num: Index of cache configurable memory block for which cache will be disabled
 * returns
 *     0 - Success <br>
 *    <0 - Failure
 */
int_t HDLIB_cb_mem_set_uncached(const HDR_instance_t* hdr_instance, U32 block_num);

/**
 * brief Framework should provide a way to deal with error conditions returned by the library.
 *
 * param[in] hdr_instance: Pointer to the HDR Library instance
 * param[in] error: Error type
 * param[in] caller_func_addr: Address of the calling HDR function
 * param[in] func_name: Character string of the calling HDR function
 */
void HDLIB_cb_error_handler(const HDR_instance_t* hdr_instance, int_t error, intptr_t caller_func_addr, const S8* func_name);

/**
 * brief Framework should provide cleanup for different blocks once the HDRadio library completes a reacquisition.
 *
 * param[in] hdr_instance: Pointer to the HDR Library instance
 */
#ifdef USE_HDRLIB_2ND_CHG_VER
void HDLIB_cb_reacq_complete(const HDR_instance_t* hdr_instance, HDR_tune_band_t band);
#else
void HDLIB_cb_reacq_complete(const HDR_instance_t* hdr_instance);
#endif


/**
 * brief Requests the status of audio alignment
 *
 * HD Radio library will output mute audio for MPS until audio is time and level aligned
 * with analog.
 *
 * param[in] hdr_instance: Pointer to the HDR Library instance
 * returns
 *     true  - MPS time/level alignmenet finished <br>
 *     false - Waiting for successful MPS time/level alignment
 */
HDBOOL HDLIB_cb_is_mps_alignment_finished(const HDR_instance_t* hdr_instance);


/**
 * brief Implementation Template for sending Logger data.
 */
int_t HDLIB_cb_logger_send(U8* data, U32 length);

#ifdef USE_HDRLIB_2ND_CHG_VER
/**
 * @brief Framework should provide cleanup for different blocks when the HDRadio
 * library needs to perform audio alignment.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 */
void HDLIB_cb_audio_align(const HDR_instance_t *hdr_instance);
#endif

#else // USE_CHANGED_HDRLIB_CB

/**
 * brief Requests to run an HDR library task function
 *
 * HD Radio processing is split into multiple execution functions(tasks), where each execution function
 * can be called in the context of an OS task. When an HDR instance wants to run a specific task, it will
 * call this function to request that the outside framework execute specified task function on its behalf.
 *
 * see hdrCore.h
 *
 * param[in] hdr_instance: Pointer to the HDR Library instance
 * param[in] task_id: Numeric identifier of the HDR task
 */
void HDR_post_task(HDR_instance_t* hdr_instance, HDR_task_id_t task_id);

/**
 * brief Locks a mutex, blocks if necessary
 *
 * Protects a resource or group of resources from concurrent access.
 *
 * param[in] mutex: Pointer to mutex object
 * param[in] timeout: Timeout period measured in milliseconds. Negative value means no timeout.
 * returns
 *		0	 Success <br>
 *	   <0	 Failure
 */
int_t HDR_lock_mutex(void* mutex, int_t timeout);

/**
 * brief Unlocks a mutex
 *
 * param[in] mutex: Pointer to mutex object.
 */
void HDR_unlock_mutex(void* mutex);

/**
 * brief Blocks all other critical sections
 *
 * HD Radio library instance will call this function to gain exclusive access to a shared
 * resource and prevent access by another critical section function of this instance.
 * Critical sections are used by the library for time critical mutual exclusion with minimum
 * overhead, where a mutex locking may be too slow. The HDR library critical sections are
 * guranteed to be very short.
 *
 * param[in] hdr_instance: Pointer to the HDR Library instance
 * returns
 *		0	 Success <br>
 *	   <0	 Failure
 */
int_t HDR_enter_critical_section(const HDR_instance_t* hdr_instance);

/**
 * brief Unblocks all other critical sections
 *
 * HD Radio instance will call this function to release access to a shared resource from a prior
 * call to HDR_enter_critical_section().
 *
 * param [in] hdr_instance Pointer to the current HDR instance.
 */
void HDR_exit_critical_section(const HDR_instance_t* hdr_instance);

/**
 * brief Signals completion of HDR re-acquisition
 *
 * HDR library will call this function when an HDR instance completes re-acquisition.
 * Re-acquisition is triggered either when station is changed on the same band
 * or when self-triggered re-acquisition occurs due to adverse channel conditions.
 *
 * param[in] hdr_instance: Pointer to the HDR Library instance
 */
void HDR_reacq_complete(HDR_instance_t* hdr_instance);

/**
 * brief Requests the status of audio alignment
 *
 * HD Radio library will output mute audio for MPS until audio is time and level aligned
 * with analog.
 *
 * param[in] hdr_instance: Pointer to the HDR Library instance
 * returns
 *	   true  - MPS time/level alignmenet finished <br>
 *	   false - Waiting for successful MPS time/level alignment
 */
HDBOOL HDR_is_mps_alignment_finished(const HDR_instance_t* hdr_instance);

/**
 * brief Request the current OS thread to suspend
 *
 * param[in] usec: Number of microseconds to sleep
 */
void HDR_usleep(U32 usec);

/**
 * brief Retrieves system time measured in milliseconds
 * param[out] time_msec: Time in milliseconds
 * returns
 *	   0 - Success <br>
 *	  <0 - Failure
 */
int_t HDR_get_sys_time(ULONG* time_msec);

/**
 * brief Notifies outside framework that HDR instance encountered an error
 *
 * Gives the outside framework the flexibility of dealing with errors in its own way in one central location.
 * Furthermore, it gives the developer the ability to break the execution precisely at the time
 * of the error encountered while registers and stack information are still fresh.
 *
 * param[in] hdr_instance: Pointer to the HDR Library instance
 * param[in] error: Error type
 * param[in] caller_func_addr: Address of the calling HDR function
 * param[in] func_name: Character string of the calling HDR function
 */
void HDR_error_handler(const HDR_instance_t* hdr_instance, int_t error, intptr_t caller_func_addr, const S8* func_name);

/**
 * brief Disables cache for the specified memory region HDR_instance_t
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
 * The implementation of this function can be empty if cache configurable memory is not used.
 *
 * param [in] hdr_instance: Pointer to the HDR Library instance
 * param [in] block_num: Index of cache configurable memory block for which cache will be disabled
 * returns
 *	   0 - Success <br>
 *	  <0 - Failure
 */
int_t HDR_mem_set_uncached(HDR_instance_t* hdr_instance, U32 block_num);

/**
 * brief Enables cache for the specified memory region
 *
 * The implementation of this function can be empty if cache configurable memory is not used.
 *
 * param [in] hdr_instance: Pointer to the HDR Library instance
 * param [in] block_num: Index of cache configurable memory block for which cache will be enabled
 * see HDR_mem_set_uncached().
 * returns
 *	   0 - Success <br>
 *	  <0 - Failure
 */
int_t HDR_mem_set_cached(HDR_instance_t* hdr_instance, U32 block_num);

/**
 * brief Viterbi code rate configuration
 *
 * | enum				 | band | gen poly			| K | code rate | puncture matrix					  |
 * |---------------------|------|-------------------|---|-----------|-------------------------------------|
 * |HDR_FM_RATE_1_OVER_3 | FM	| 133 \n 171 \n 165 | 7 | 1/3		| 1 1		\n 1 1		 \n 1 1 	  |
 * |HDR_FM_RATE_2_OVER_5 | FM	| 133 \n 171 \n 165 | 7 | 2/5		| 1 1		\n 1 1		 \n 1 0 	  |
 * |HDR_FM_RATE_1_OVER_2 | FM	| 133 \n 171 \n 165 | 7 | 1/2		| 1 1		\n 0 0		 \n 1 1 	  |
 * |HDR_AM_RATE_1_OVER_3 | AM	| 561 \n 753 \n 711 | 9 | 1/3		| 1 		\n 1		 \n 1		  |
 * |HDR_AM_RATE_5_OVER_12| AM	| 561 \n 657 \n 711 | 9 | 5/12		| 1 1 1 1 1 \n 0 0 0 1 1 \n 1 1 1 1 1 |
 * |HDR_AM_RATE_2_OVER_3 | AM	| 561 \n 753 \n 711 | 9 | 2/3		| 1 1 1 1	\n 0 0 0 0	 \n 1 0 1 0   |
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
 * brief Defines a Viterbi job handle
 */
typedef void* HDR_vit_job_handle_t;

/**
 * brief Queues a Viterbi job
 *
 * This function is called by the HDR library instance when there is data ready for
 * viterbi decoding. Viterbi job includes de-puncturing, viterbi decoding
 * and descrambling. The function must not block waiting for a job to finish.
 *
 * If hardware Viterbi is not used, this function still needs to be implemented for
 * the application to compile. It can be an empty function, which the library will never call.
 *
 * param [in] hdr_instance: Pointer to the HDR Library instance handle
 * param [in] sd_in: Pointer to the buffer containing input soft decisions. (8 bits per soft decision)
 * param [in] sd_count: Number of input soft decisions.
 * param [in] block_number: Block number of input data.
 * param [in] frame_number: Frame number of input data.
 * param [in] timestamp: Timestamp of the input data.
 * param [in] code_rate: Defines the code rate and generator polynomial set used in processing data.
 *						  For FM Digital Diversity, combining of soft decisions will be done in software, so
 *						  #HDR_FM_RATE_1_OVER_3 will be used to indicate that no
 *						  de-puncturing is required.
 * param [out] bits_out: Pointer to decoded output buffer.
 * param [out] vit_job:  Pointer to viterbi job handle. The outside framework should assign the handle an internal
 *						  structure that defines a Viterbi job handle. HDR library will use that handle to query the
 *						  result of that Viterbi job. (see #HDR_query_vit_job())
 * returns
 *	   0 - Success <br>
 *	  <1 - Failure
 */
int_t HDR_queue_vit_job(HDR_instance_t* hdr_instance, S8C* sd_in, U32 sd_count,
					  U32 block_number, U32 frame_number, U32 timestamp,
					  HDR_vit_code_rate_t code_rate, U8* bits_out, HDR_vit_job_handle_t* vit_job);

/**
 * brief Queries the status of a Viterbi job
 *
 * If hardware Viterbi is not used, this function still needs to be implemented for
 * the application to compile. It can be an empty function, which the library will never call.
 *
 * param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * param [in] vit_job: Pointer to viterbi job handle.
 * param [out] error_count: Number of errors corrected by Viterbi when job is completed. Optional for debugging.
 * returns
 *	   0 - Complete <br>
 *	   1 - Pending <br>
 *	  <0 - Error
 */
int_t HDR_query_vit_job(HDR_instance_t* hdr_instance, HDR_vit_job_handle_t vit_job, U32 * error_count);

/**
 * brief Removes specified Viterbi job from the processing queue
 *
 * If hardware Viterbi is not used, this function still needs to be implemented for
 * the application to compile. It can be an empty function, which the library will never call.
 *
 * This function is called by the HDR library when the results of a previously
 * queued viterbi job are no longer needed, such as during a re-acquisition. In this case,
 * the job handle must be reset to the unused state(NULL).
 *
 * param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * param [in] vit_job: Viterbi job handle.
 * returns
 *	   0 - Success <br>
 *	  <0 - Failure
 */
int_t HDR_flush_vit_job(HDR_instance_t* hdr_instance, HDR_vit_job_handle_t vit_job);

/**
 * brief Requests to send logging data to host
 *
 * param [in] data: Pointer to data
 * param [in] length: Length of data in bytes
 * returns
 *	   0 - Success <br>
 *	  <0 - Failure
 */
int_t HDR_logger_send(U8* data, U32 length);
#endif // USE_CHANGED_HDRLIB_CB

#ifdef __cplusplus
}
#endif

#endif
