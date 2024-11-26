/*******************************************************************************

*   FileName : tchdr_hdlibcb.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio core library callback functions and definitions

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
/***************************************************
*		Include 			   					*
****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>

#include "tchdr_common.h"

#include "hdrBbSrc.h"
#include "hdrAudioResampler.h"
#include "hdrAudio.h"

#include "tchdr_bytestream.h"
#include "tchdr_hdlibcb.h"
#include "tchdr_api.h"
#include "tchdr_sis.h"
#include "tchdr_service.h"
#include "tchdr_framework.h"

/***************************************************
*        Global variable definitions               *
****************************************************/

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/

/***************************************************
*             Local preprocessor                   *
****************************************************/

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*           Local constant definitions              *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition				*
****************************************************/
#ifdef USE_CHANGED_HDRLIB_CB
/* Callback to Enter a Protected section of code
 * */
int_t HDLIB_cb_enter_critical_section(const HDR_instance_t* hdr_instance)
{
	int_t rc = -1;
	if(hdr_instance != NULL) {
		if(hdr_instance->mutex != NULL) {
	    	rc = pthread_mutex_lock((pthread_mutex_t *)hdr_instance->mutex);
			if(rc != 0) {
				S8 errBuf[HDR_ERR_BUF_SIZE]={0,};
				(void)strerror_r(rc, errBuf, sizeof(errBuf));
				(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "[HDR Instance:%d] Failed to lock mutex. %s[%d].\n", hdr_instance->instance_number, errBuf, rc);
			}
		}
		else {
			(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "[HDR Instance:%d] Failed to lock mutex. Mutex is null.\n", hdr_instance->instance_number);
		}
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "[tchdr_hdlibcb.c:%d] null pointer parameter.\n", __LINE__);
	}
	return rc;
}

/* Callback to Exit a Protected section of code
 * */
int_t HDLIB_cb_exit_critical_section(const HDR_instance_t* hdr_instance)
{
	int_t rc= -1;
	if(hdr_instance != NULL) {
		if(hdr_instance->mutex != NULL) {
	    	rc = pthread_mutex_unlock((pthread_mutex_t *)hdr_instance->mutex);
			if(rc != 0) {
				S8 errBuf[HDR_ERR_BUF_SIZE]={0,};
				(void)strerror_r(rc, errBuf, sizeof(errBuf));
				(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "[HDR Instance:%d] Failed to unlock mutex. %s[%d].\n", hdr_instance->instance_number, errBuf, rc);
			}
		}
		else {
			(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "[HDR Instance:%d] Failed to unlock mutex. Mutex is null.\n", hdr_instance->instance_number);
		}
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "[tchdr_hdlibcb.c:%d] null pointer parameter.\n", __LINE__);
	}
	return rc;
}

/* Callback implementation to lock a mutex
 * */
int_t HDLIB_cb_lock_mutex(void* mutex, int_t timeout)
{
	int_t retVal;

	if(mutex != NULL) {
	    if(timeout < 0){
			if(pthread_mutex_lock((pthread_mutex_t *)mutex) < 0) {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to lock mutex.\n");
			}
	    }
		else {
		    struct timespec ts;

		    ts.tv_sec = 0;
		    ts.tv_nsec = (SLONG)timeout  * 1000000;

		    if(pthread_mutex_timedlock((pthread_mutex_t *)mutex, &ts) < 0){
		        (*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Failed to lock mutex with timeout.\n");
		    }
		}
		retVal = 0;
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Null HD library lock mutex.\n");
		retVal = -1;
	}

    return retVal;
}

#ifdef USE_HDRLIB_2ND_CHG_VER
int_t HDLIB_cb_unlock_mutex(void* mutex)
{
	int_t rc = -1;
	if(mutex != NULL) {
		rc = pthread_mutex_unlock((pthread_mutex_t *)mutex);
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Null HD library unlock mutex.\n");
	}
	return rc;
}
#else
void HDLIB_cb_unlock_mutex(void* mutex)
{
	if(mutex != NULL) {
		(void)pthread_mutex_unlock((pthread_mutex_t *)mutex);
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Null HD library unlock mutex.\n");
	}
}
#endif

void HDLIB_cb_usleep(U32 usec)
{
    (void)usleep(usec);
}

int_t HDLIB_cb_get_sys_time(ULONG* time_msec)
{
	int_t rc;

	if(time_msec != NULL) {
        struct timespec ts;

        rc = clock_gettime(LINUX_CLOCK_TIMER_TYPE, &ts);

        *time_msec = (*stArith.ulongmul)((*stCast.slongtoulong)(ts.tv_sec), 1000U);
        *time_msec = (*stArith.ulongadd)(*time_msec, ((*stCast.slongtoulong)(ts.tv_nsec) / 1000000UL)); // Convert nanoseconds to milliseconds
    }
    else {
        rc = -1;
    }

    return rc;
}

/*
 * Callback implementation to schedule a viterbi job if done on a seperate optimized hardware.
 * */
int_t HDLIB_cb_queue_vit_job(const HDR_instance_t* hdr_instance, S8C* sd_in, U32 sd_count,
                             U32 block_number, U32 frame_number, U32 timestamp,
                             HDR_vit_code_rate_t code_rate, U8* bits_out, HDR_vit_job_handle_t* vit_job)
{
	int_t rc;
    if((hdr_instance == NULL) || (sd_in == NULL) || (bits_out == NULL) || (vit_job == NULL)) {
        (*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HDLIB_cb_queue_vit_job called with a null pointer\n");
        rc = -1;
    } else {
        rc = 0;
    }
	UNUSED(sd_count);
	UNUSED(block_number);
	UNUSED(frame_number);
	UNUSED(timestamp);
	UNUSED(code_rate);
	return rc;
}

int_t HDLIB_cb_query_vit_job(const HDR_instance_t* hdr_instance, HDR_vit_job_handle_t vit_job, U32* error_count)
{
	int_t rc;
    if((hdr_instance == NULL) || (error_count == NULL) || (vit_job == NULL)) {
        (*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HDLIB_cb_query_vit_job called with a null pointer\n");
        rc = -1;
    } else {
        rc = 0;
    }
	return rc;
}

/*
 * Callback implementation to clear a viterbi job if done on a seperate optimized hardware.
 * */
int_t HDLIB_cb_flush_vit_job(const HDR_instance_t* hdr_instance, HDR_vit_job_handle_t vit_job)
{
	int_t rc;
    if((hdr_instance == NULL) || (vit_job == NULL)) {
        (*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HDLIB_cb_flush_vit_job called with a null pointer\n");
        rc = -1;
    } else {
        rc = 0;
    }
	return rc;
}

/*
 * Callback implementation to set the memory assigned to HDRadio as uncached.
 * */
int_t HDLIB_cb_mem_set_uncached(const HDR_instance_t* hdr_instance, U32 block_num)
{
    int_t rc;
    if(hdr_instance != NULL) {
        // DO Stuff Here
        rc = 0;
    }else{
        (*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HDLIB_cb_mem_set_uncached called with a null pointer\n");
		rc = -1;
    }
	UNUSED(block_num);
    return rc;
}

/*
 * Callback implementation to set the memory assigned to HDRadio as cached.
 * */
int_t HDLIB_cb_mem_set_cached(const HDR_instance_t* hdr_instance, U32 block_num)
{
    int_t rc;
    if(hdr_instance != NULL){
        // DO Stuff Here
        rc = 0;
    }else{
	    (*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HDLIB_cb_mem_set_cached called with a null pointer\n");
		rc = -1;
    }
	UNUSED(block_num);
    return rc;
}

/*
 * Callback implementation to deal with error conditions returned by the library.
 * */
void HDLIB_cb_error_handler(const HDR_instance_t* hdr_instance, int_t error, intptr_t caller_func_addr, const S8* func_name)
{
	if(hdr_instance != NULL) {
		if(func_name != NULL){
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Error function name is %s. Instance[%d]\n", func_name, hdr_instance->instance_number);
	    }
		else {
	        (*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Error function name is null. Instance[%d]\n", hdr_instance->instance_number);
		}
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "[tchdr_hdlibcb.c:%d] null pointer parameter.\n", __LINE__);
	}

	switch(error){
        case (int_t)HDR_ERROR_INVAL_ARG:
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HD Radio Error Handler: HDR_ERROR_INVAL_ARG!!!\n");
            break;
        case (int_t)HDR_ERROR_REACQ:
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HD Radio Error Handler: HDR_ERROR_REACQ!!!\n");
            break;
        case (int_t)HDR_ERROR_IDLE:
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HD Radio Error Handler: HDR_ERROR_IDLE!!!\n");
            break;
        case (int_t)HDR_NOT_INITIALIZED:
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HD Radio Error Handler: HDR_NOT_INITIALIZED!!!\n");
            break;
        case (int_t)HDR_ERROR_INSTANCE_TYPE:
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HD Radio Error Handler: HDR_ERROR_INSTANCE_TYPE!!!\n");
            break;
        default:
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HD Radio Error Handler: Unknown!!!\n");
            break;
    }
	UNUSED(caller_func_addr);
}

/*
 * Callback implementation to clean up in the framework after the HDRadio library re-acquisition is complete.
 * */
#ifdef USE_HDRLIB_2ND_CHG_VER
void HDLIB_cb_reacq_complete(const HDR_instance_t* hdr_instance, HDR_tune_band_t band)
{
	if(hdr_instance != NULL) {
	 	stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();

	    if(hdr_instance->instance_type == HDR_FULL_INSTANCE){
	        (void)HDR_audio_resampler_reset(frameworkData->hdaoutResampler);
	        (void)HDR_blend_crossfade_reset(frameworkData->blendCrossfade);
		#ifdef USE_AUTO_AUDIO_ALIGN
		  	(void)HDR_auto_align_reset(frameworkData->autoAlign, band);
		#endif
	        frameworkData->alignmentSuccess = false;
	        frameworkData->digitalAudioStarted=false;
			(void)(*stOsal.osmemset)((void*)(frameworkData->digitalAudio), (S8)0, HDR_AUDIO_FRAME_SIZE * (U32)sizeof(HDR_pcm_stereo_t));
	    }
		if(frameworkData->busyFlag[hdr_instance->instance_number] == true) {
			(*pfnHdrLog)(eTAG_CORE, eLOG_INF, "HDR reacquisition completed [%d].\n", (S32)hdr_instance->instance_number);
		}
	#ifdef DEBUG_ENABLE_REACQ_LOG
		else {
			(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "HDR reacquisition completed [%d].\n", (S32)hdr_instance->instance_number);
		}
	#endif
		frameworkData->busyFlag[hdr_instance->instance_number] = false;
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "[tchdr_hdlibcb.c:%d] null pointer parameter.\n", __LINE__);
	}
}
#else
void HDLIB_cb_reacq_complete(const HDR_instance_t* hdr_instance)
{
	if(hdr_instance != NULL) {
	 	stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();

	    if(hdr_instance->instance_type == HDR_FULL_INSTANCE){
	        (void)HDR_audio_resampler_reset(frameworkData->hdaoutResampler);
	        (void)HDR_blend_crossfade_reset(frameworkData->blendCrossfade);
		#ifdef USE_AUTO_AUDIO_ALIGN
        	(void)HDR_auto_align_reset(frameworkData->autoAlign, HDR_get_band_select(hdr_instance));
		#endif
	        frameworkData->alignmentSuccess = false;
	        frameworkData->digitalAudioStarted=false;
			(void)(*stOsal.osmemset)((void*)(frameworkData->digitalAudio), (S8)0, HDR_AUDIO_FRAME_SIZE * (U32)sizeof(HDR_pcm_stereo_t));
	    }
		if(frameworkData->busyFlag[hdr_instance->instance_number] == true) {
			(*pfnHdrLog)(eTAG_CORE, eLOG_INF, "HDR reacquisition completed [%d].\n", (S32)hdr_instance->instance_number);
		}
		else {
			(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "HDR reacquisition completed [%d].\n", (S32)hdr_instance->instance_number);
		}
		frameworkData->busyFlag[hdr_instance->instance_number] = false;
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "[tchdr_hdlibcb.c:%d] null pointer parameter.\n", __LINE__);
	}
}
#endif

/*
 * Callback implementation to check if the alignment has been completed in case of using AAA
 * */
HDBOOL HDLIB_cb_is_mps_alignment_finished(const HDR_instance_t* hdr_instance)
{
	HDBOOL ret;
	if(hdr_instance != NULL) {
	    const stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();
	    ret = frameworkData->alignmentSuccess;
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "[tchdr_hdlibcb.c:%d] null pointer parameter.\n", __LINE__);
		ret = false;
	}
	return ret;
}

/*
 * Callback implementation to send logger data using a stream based protocol.
 * */
int_t HDLIB_cb_logger_send(U8* data, U32 length)
{
	int_t rc = 0;
    if(data != NULL){
		rc = tchdr_bytestream_write(&loggerByteStream, data, length, -1);
    }
    return rc;
}

#ifdef USE_HDRLIB_2ND_CHG_VER
/*
 * Callback implementation to clean up in the framework when the HDRadio library needs to perform audio alignment.
 * */
void HDLIB_cb_audio_align (const HDR_instance_t *hdr_instance) {
	stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();

    if (hdr_instance->instance_type == HDR_FULL_INSTANCE) {
        HDR_audio_resampler_reset (frameworkData->hdaoutResampler);
        frameworkData->alignmentSuccess = false;
        frameworkData->digitalAudioStarted = false;
        memset (frameworkData->digitalAudio, 0, HDR_AUDIO_FRAME_SIZE * sizeof (HDR_pcm_stereo_t));
    }

    frameworkData->busyFlag[hdr_instance->instance_number] = false;
}
#endif

#else // USE_CHANGED_HDRLIB_CB
// tell cpd to start ignoring code - CPD-OFF
/* Callback to Enter a Protected section of code
 * */
int_t HDR_enter_critical_section(const HDR_instance_t* hdr_instance)
{
	int_t rc = 0;
	if(hdr_instance != NULL) {
		if(hdr_instance->mutex != NULL) {
	    	rc = pthread_mutex_lock((pthread_mutex_t *)hdr_instance->mutex);
			if(rc != 0) {
				S8 errBuf[HDR_ERR_BUF_SIZE]={0,};
				(void)strerror_r(rc, errBuf, sizeof(errBuf));
				(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "[HDR Instance:%d] Failed to lock mutex. %s[%d].\n", hdr_instance->instance_number, errBuf, rc);
			}
		}
		else {
			(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "[HDR Instance:%d] Failed to lock mutex. Mutex is null.\n", hdr_instance->instance_number);
		}
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "[tchdr_hdlibcb.c:%d] null pointer parameter.\n", __LINE__);
	}
	return 0;
}

void HDR_exit_critical_section(const HDR_instance_t* hdr_instance)
{
	int_t rc=0;
	if(hdr_instance != NULL) {
		if(hdr_instance->mutex != NULL) {
	    	rc = pthread_mutex_unlock((pthread_mutex_t *)hdr_instance->mutex);
			if(rc != 0) {
				S8 errBuf[HDR_ERR_BUF_SIZE]={0,};
				(void)strerror_r(rc, errBuf, sizeof(errBuf));
				(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "[HDR Instance:%d] Failed to unlock mutex. %s[%d].\n", hdr_instance->instance_number, errBuf, rc);
			}
		}
		else {
			(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "[HDR Instance:%d] Failed to unlock mutex. Mutex is null.\n", hdr_instance->instance_number);
		}
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "[tchdr_hdlibcb.c:%d] null pointer parameter.\n", __LINE__);
	}
}

int_t HDR_lock_mutex(void* mutex, int_t timeout)
{
	int_t retVal;

	if(mutex != NULL) {
	    if(timeout < 0){
			if(pthread_mutex_lock((pthread_mutex_t *)mutex) < 0) {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to lock mutex.\n");
			}
	    }
		else {
		    struct timespec ts;

		    ts.tv_sec = 0;
		    ts.tv_nsec = timeout  * 1000000;

		    if(pthread_mutex_timedlock(mutex, &ts) < 0){
		        (*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Failed to lock mutex with timeout.\n");
		    }
		}
		retVal = 0;
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Null HD library lock mutex.\n");
		retVal = -1;
	}

    return retVal;
}

void HDR_unlock_mutex(void* mutex)
{
	if(mutex != NULL) {
    	pthread_mutex_unlock((pthread_mutex_t *)mutex);
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Null HD library unlock mutex.\n");
	}
}

void HDR_usleep(U32 usec)
{
    usleep(usec);
}

int_t HDR_get_sys_time(ULONG* time_msec)
{
	int_t rc;

	if(time_msec != NULL) {
        struct timespec ts;

        rc = clock_gettime(LINUX_CLOCK_TIMER_TYPE, &ts);

        *time_msec = (ts.tv_sec * 1000) + (ts.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    }
    else {
        rc = -1;
    }

    return rc;
}

/*
 * Callback implementation to schedule a viterbi job if done on a seperate optimized hardware.
 * */
int_t HDR_queue_vit_job(HDR_instance_t* hdr_instance, S8C* sd_in, U32 sd_count,
                           U32 block_number, U32 frame_number,U32 timestamp,
                           HDR_vit_code_rate_t code_rate, U8* bits_out, HDR_vit_job_handle_t* vit_job)
{
	return 0;
}

int_t HDR_query_vit_job(HDR_instance_t* hdr_instance, HDR_vit_job_handle_t vit_job, U32* error_count)
{
    return 0;
}

int_t HDR_flush_vit_job(HDR_instance_t* hdr_instance, HDR_vit_job_handle_t vit_job)
{
    return 0;
}

int_t HDR_mem_set_uncached(HDR_instance_t* hdr_instance, U32 block_num)
{
    return 0;
}

int_t HDR_mem_set_cached(HDR_instance_t* hdr_instance, U32 block_num)
{
    return 0;
}

void HDR_error_handler(const HDR_instance_t* hdr_instance, int_t error, intptr_t caller_func_addr, const S8* func_name)
{
    if(func_name != NULL){
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Error function name is %s. Instance[%d]\n", func_name, hdr_instance->instance_number);
    }
	else {
        (*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Error function name is null. Instance[%d]\n", hdr_instance->instance_number);
	}

	switch(error){
        case HDR_ERROR_INVAL_ARG:
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HD Radio Error Handler: HDR_ERROR_INVAL_ARG!!!\n");
            break;
        case HDR_ERROR_REACQ:
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HD Radio Error Handler: HDR_ERROR_REACQ!!!\n");
            break;
        case HDR_ERROR_IDLE:
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HD Radio Error Handler: HDR_ERROR_IDLE!!!\n");
            break;
        case HDR_NOT_INITIALIZED:
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HD Radio Error Handler: HDR_NOT_INITIALIZED!!!\n");
            break;
        case HDR_ERROR_INSTANCE_TYPE:
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HD Radio Error Handler: HDR_ERROR_INSTANCE_TYPE!!!\n");
            break;
        default:
			(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "HD Radio Error Handler: Unknown!!!\n");
            break;
    }
}

void HDR_reacq_complete(HDR_instance_t* hdr_instance)
{
    stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();

	(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "HDR reacquisition completed [%d].\n", hdr_instance->instance_number);

    if(hdr_instance->instance_type == HDR_FULL_INSTANCE){
        HDR_audio_resampler_reset(frameworkData->hdaoutResampler);
        HDR_blend_crossfade_reset(frameworkData->blendCrossfade);
	#ifdef USE_AUTO_AUDIO_ALIGN
        HDR_auto_align_reset(frameworkData->autoAlign, HDR_get_band_select(hdr_instance));
	#endif
        frameworkData->alignmentSuccess = false;
        frameworkData->digitalAudioStarted=false;
    }
}

int_t HDR_logger_send(U8* data, U32 length)
{
    int_t rc = 0;
    if(data != NULL){
		rc = tchdr_bytestream_write(&loggerByteStream, data, length, -1);
    }
    return rc;
}
// resume CPD analysis - CPD-ON
#endif /* USE_CHANGED_HDRLIB_CB */

