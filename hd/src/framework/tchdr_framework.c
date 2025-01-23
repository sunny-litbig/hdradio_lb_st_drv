/*******************************************************************************

*   FileName : tchdr_framework.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio framework threads for demodulation and audio decoder

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
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <unistd.h>
#include <semaphore.h>

#include "tchdr_common.h"

#include "hdrTest.h"
#include "hdrBbSrc.h"
#include "hdrAlerts.h"
#include "hdrConfig.h"
#include "hdrAudio.h"
#include "hdrBlend.h"
#include "hdrPhy.h"

#include "tchdr_std.h"
#include "tchdr_log.h"
#include "tchdr_cmdproc.h"
#include "tchdr_cbuffer.h"
#include "tchdr_msg.h"
#include "tchdr_hdlibcb.h"
#include "tchdr_cmdcallbacks.h"
#include "tchdr_api.h"
#include "tchdr_psd.h"
#include "tchdr_sis.h"
#include "tchdr_service.h"
#include "tchdr_framework.h"
#include "tchdr_bbinput.h"
#include "tchdr_audio.h"

#ifdef DEBUG_TCDAT_TEST_DUMP
#include "tchdr_dat.h"
#endif

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
// Memory amount required by full HD Radio instance
// For HDR_DATA_ONLY_INSTANCE and especially for HDR_DEMOD_ONLY_INSTANCE,
// required memory is much smaller
#define HDR_CACHED_MEM_SIZE         (4 * 1024 * 1024)

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*           Local constant definitions              *
****************************************************/
static U8 cached_memory_1[HDR_CACHED_MEM_SIZE];
#if NUM_HDR_INSTANCES > 1
static U8 cached_memory_2[HDR_CACHED_MEM_SIZE];
#endif
#if NUM_HDR_INSTANCES > 2
static U8 cached_memory_3[HDR_CACHED_MEM_SIZE];
#endif
#if NUM_HDR_INSTANCES > 3
static U8 cached_memory_4[HDR_CACHED_MEM_SIZE];
#endif
static stHDR_FRAMEWORK_DATA_t stHdrFrameworkData;
static U8 blendCrossfadeMem[HDR_BLEND_CROSSFADE_MEM_SIZE];
static U8 hdaoutResamplerMem[HDR_AUDIO_RESAMPLER_MEM_SIZE];
static U8 autoAlignMem[HDR_AUTO_ALIGN_MEM_SIZE];

static pthread_mutex_t hdrMutex[NUM_HDR_INSTANCES];
static pthread_mutex_t mrcMutex[2];
#ifdef USE_HDRLIB_2ND_CHG_VER
static pthread_mutex_t reacqCtlMutex[2];
static pthread_mutexattr_t mutexAttr;
#endif

static int16c_t hdrBbInputBuf[NUM_HDR_INSTANCES][HDR_FM_BB_INPUT_BUFFER_SIZE];

// Semaphore for scheduling HDR_exec()
static sem_t bbSamplesReadySem[NUM_HDR_INSTANCES];
static sem_t audioBlendingThreadSem;

static U32 theNumOfHdrInstances;
static U32 typeOfHdr;

static eTCHDR_MAIN_CMD_t eTcHdrMainEventMode;
#if (HDR_CONFIG == HDR_1p0_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG)
static eTCHDR_MRC_CMD_t eTcHdrMrcEventMode;
#endif
#if (HDR_CONFIG == HDR_1p5_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG)
static eTCHDR_BS_CMD_t eTcHdrBsEventMode;
#endif
#if (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
static eTCHDR_BSMRC_CMD_t eTcHdrBsMrcEventMode;
#endif
static eTCHDR_BLENDING_CMD_t eTcHdrBlendingEventMode;

static HDBOOL bAnalogAudioMute = false;
static HDBOOL bMrcConnection = false;
static HDBOOL bBsMrcConnection = false;

static HDBOOL bAAMuteForTune = false;

static S32 extClockOffset = 0U;		// Clock difference between the external I/Q I2S input and the analog audio I2S input.

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
static struct timespec primary_ChkTimer, primary_ChkTimeNow, primary_ChkTimeDiff;
static U32 primary_AccumMs=0, primary_LoopMs=0, primary_DumpCount=0;
static FILE *primary_DumpFile;

static struct timespec mrc_ChkTimer, mrc_ChkTimeNow, mrc_ChkTimeDiff;
static U32 mrc_AccumMs=0, mrc_LoopMs=0, mrc_DumpCount=0;
static FILE *mrc_DumpFile;

static struct timespec bs_ChkTimer, bs_ChkTimeNow, bs_ChkTimeDiff;
static U32 bs_AccumMs=0, bs_LoopMs=0, bs_DumpCount=0;
static FILE *bs_DumpFile;

static struct timespec bsmrc_ChkTimer, bsmrc_ChkTimeNow, bsmrc_ChkTimeDiff;
static U32 bsmrc_AccumMs=0, bsmrc_LoopMs=0, bsmrc_DumpCount=0;
static FILE *bsmrc_DumpFile;

static struct timespec audblend_ChkTimer, audblend_ChkTimeNow, audblend_ChkTimeDiff;
static U32 audblend_AccumMs=0, audblend_LoopMs=0, audblend_DumpCount=0;
static FILE *audblend_DumpFile;
#endif

/***************************************************
*          Local function prototypes               *
****************************************************/
static void tchdrblending_audioOutputHandler(HDBOOL blendFlag, HDBOOL hybridProgram);
static eTCHDR_EVT_STS_t tchdrmain_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrmain_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrmain_event_reset(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrmain_event_dummy(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrmrc_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrmrc_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrmrc_event_reset(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrmrc_event_dummy(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrbs_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrbs_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrbs_event_reset(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrbs_event_dummy(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrblending_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrblending_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrblending_event_reset(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrblending_event_audioMode(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrblending_event_dummy(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);

/***************************************************
*			function definition				*
****************************************************/
void tchdrblending_setAAMute(HDBOOL fOnOff)
{
	if(fOnOff) {
		bAAMuteForTune = true;
	}
	else {
		bAAMuteForTune = false;
	}
}

HDBOOL tchdrblending_getAAMute(void)
{
	return bAAMuteForTune;
}

stHDR_FRAMEWORK_DATA_t *tchdrfwk_getDataStructPtr(void)
{
	return &stHdrFrameworkData;
}

void tchdrfwk_setHdrType(U32 type)
{
	typeOfHdr = type;
}

U32 tchdrfwk_getHdrType(void)
{
	return typeOfHdr;
}

void tchdrfwk_setNumOfHdrInstance(U32 num)
{
	if(num <= NUM_HDR_INSTANCES) {
		theNumOfHdrInstances = num;
	}
	else {
		theNumOfHdrInstances = NUM_HDR_INSTANCES;
	}
}

U32 tchdrfwk_getNumOfHdrInstance(void)
{
 	return theNumOfHdrInstances;
}

void tchdrfwk_setAnalogAudioMute(U32 fOnOff)
{
	if(fOnOff > 0U) {
		bAnalogAudioMute = true;
	}
	else {
		bAnalogAudioMute = false;
	}
}

HDRET tchdrfwk_setExtnalClockOffset(S32 clkOffset)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	// clkOffset range: -/+150.0ppm
	if((clkOffset <= 0x960000) && (clkOffset >= 0xFF6A0000)) {
		extClockOffset = clkOffset;
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
	}

	return ret;
}

S32 tchdrfwk_getExtnalClockOffset(void)
{
	return extClockOffset;
}

static HDBOOL tchdrframework_getAnalogAudioMute(void)
{
	return bAnalogAudioMute;
}

// This function is same as "HDR_instance_t *tchdr_getHdrInstanceAddr(eTC_HDR_ID_t id)" in tchdr_api.c.
HDR_instance_t *tchdrfwk_getHdrInstancePtr(eTC_HDR_ID_t id)
{
	stHDR_FRAMEWORK_DATA_t* frameworkData = tchdrfwk_getDataStructPtr();
	U32 currentHdrType = tchdrfwk_getHdrType();
	HDR_instance_t *hdrInstance = pNULL;

	if(id == eTC_HDR_ID_MAIN) {
		hdrInstance = &frameworkData->hdrInstance[0];
	}
	else if(id == eTC_HDR_ID_BS) {
		if(currentHdrType == (U32)HDR_1p5_CONFIG) {
			 hdrInstance = &frameworkData->hdrInstance[1];
		}
		else if((currentHdrType == (U32)HDR_1p5_MRC_CONFIG) || (currentHdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
			 hdrInstance = &frameworkData->hdrInstance[2];
		}
		else {
			hdrInstance = pNULL;
		}
	}
	else if(id == eTC_HDR_ID_MRC) {
		if((currentHdrType == (U32)HDR_1p0_MRC_CONFIG) || (currentHdrType == (U32)HDR_1p5_MRC_CONFIG) ||
			(currentHdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG))
		{
			 hdrInstance = &frameworkData->hdrInstance[1];
		}
		else {
			hdrInstance = pNULL;
		}
	}
	else if(id == eTC_HDR_ID_BS_MRC) {
		if(currentHdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG) {
			 hdrInstance = &frameworkData->hdrInstance[3];
		}
		else {
			hdrInstance = pNULL;
		}
	}
	else {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to get instance because of invalid ID.\n");
		hdrInstance = pNULL;
	}

	return hdrInstance;
}

static HDRET tchdrfwk_initMessage(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_NG_INIT_MESSAGE_QUEUE;

	// Init Main Message Queue
	ret = tchdrmain_messageInit();

	// Init MRC Message Queue
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if((typeOfHdr == HDR_1p0_MRC_CONFIG) || (typeOfHdr == HDR_1p5_MRC_CONFIG)) {
			ret = tchdrmrc_messageInit();
		}
	}

	// Init BS Message Queue
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if((typeOfHdr == HDR_1p5_CONFIG) || (typeOfHdr == HDR_1p5_MRC_CONFIG)) {
			ret = tchdrbs_messageInit();
		}
	}

	// Init BS MRC Message Queue
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if(typeOfHdr == HDR_1p5_DUAL_MRC_CONFIG) {
			ret = tchdrbsmrc_messageInit();
		}
	}

	return ret;
}

static HDRET tchdrfwk_setConfiguration(U32 numOfHdrInstances)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 i;
	HDR_auto_align_config_t auto_align_config;
	HDR_config_t hdrConfig;
	HDR_mem_usage_t mem_usage;

	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "[FWK] Number of Instances = %d\n",numOfHdrInstances);

	stHdrFrameworkData.audioMode = eHDR_AUDIO_BLEND;
    stHdrFrameworkData.blendCrossfade = HDR_blend_crossfade_init((void*)blendCrossfadeMem);
    stHdrFrameworkData.hdaoutResampler = HDR_audio_resampler_init((void*)hdaoutResamplerMem, HDR_BB_SAMPLE_SLIPS_CORRECTION, NULL);
	stHdrFrameworkData.digitalAudioStarted=false;

    auto_align_config.am_auto_time_align_enabled = false;
    auto_align_config.fm_auto_time_align_enabled = false;
    auto_align_config.am_auto_level_align_enabled = false;
    auto_align_config.fm_auto_level_align_enabled = false;
#ifdef USE_HDRLIB_3RD_CHG_VER
    auto_align_config.am_auto_level_correction_enabled = false;
    auto_align_config.fm_auto_level_correction_enabled = false;
#else
    auto_align_config.apply_level_adjustment = false;
#endif

	stHdrFrameworkData.autoAlign = HDR_auto_align_init((void*)autoAlignMem, HDR_AUTO_ALIGN_MEM_SIZE, &auto_align_config , HDR_AUDIO_SAMPLING_RATE_44KHz);
	if(stHdrFrameworkData.autoAlign == NULL) {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Failed to initialize the AAA instance.\n");
	}

    // Initialize and Configure the HD Library - HDR_init() is the first API function to call
    stHdrFrameworkData.hdrInstance[0].cached_memory = cached_memory_1;
    stHdrFrameworkData.hdrInstance[0].cached_mem_size = HDR_CACHED_MEM_SIZE;
#if NUM_HDR_INSTANCES > 1
	if(numOfHdrInstances > 1U) {
		stHdrFrameworkData.hdrInstance[1].cached_memory = cached_memory_2;
    	stHdrFrameworkData.hdrInstance[1].cached_mem_size = HDR_CACHED_MEM_SIZE;
	}
#endif
#if NUM_HDR_INSTANCES > 2
	if(numOfHdrInstances > 2U) {
		stHdrFrameworkData.hdrInstance[2].cached_memory = cached_memory_3;
		stHdrFrameworkData.hdrInstance[2].cached_mem_size = HDR_CACHED_MEM_SIZE;
	}
#endif
#if NUM_HDR_INSTANCES > 3
		if(numOfHdrInstances > 3U) {
			stHdrFrameworkData.hdrInstance[3].cached_memory = cached_memory_4;
			stHdrFrameworkData.hdrInstance[3].cached_mem_size = HDR_CACHED_MEM_SIZE;
		}
#endif

    (void)HDR_get_default_config(&hdrConfig);
#ifdef USE_CHANGED_HDRLIB_CB
	hdrConfig.callbacks.post_task = &HDLIB_cb_post_task;
    hdrConfig.callbacks.lock_mutex = &HDLIB_cb_lock_mutex;
    hdrConfig.callbacks.unlock_mutex = &HDLIB_cb_unlock_mutex;
    hdrConfig.callbacks.enter_critical_section = &HDLIB_cb_enter_critical_section;
    hdrConfig.callbacks.exit_critical_section = &HDLIB_cb_exit_critical_section;
    hdrConfig.callbacks.reacq_complete = &HDLIB_cb_reacq_complete;
    hdrConfig.callbacks.get_sys_time = &HDLIB_cb_get_sys_time;
    hdrConfig.callbacks.usleep = &HDLIB_cb_usleep;
    hdrConfig.callbacks.queue_vit_job = &HDLIB_cb_queue_vit_job;
    hdrConfig.callbacks.query_vit_job = &HDLIB_cb_query_vit_job;
    hdrConfig.callbacks.flush_vit_job = &HDLIB_cb_flush_vit_job;
    hdrConfig.callbacks.mem_set_uncached = &HDLIB_cb_mem_set_uncached;
    hdrConfig.callbacks.mem_set_cached = &HDLIB_cb_mem_set_cached;
    hdrConfig.callbacks.error_handler = &HDLIB_cb_error_handler;
    hdrConfig.callbacks.is_mps_alignment_finished = &HDLIB_cb_is_mps_alignment_finished;
    hdrConfig.callbacks.logger_send = &HDLIB_cb_logger_send;
#ifdef USE_HDRLIB_2ND_CHG_VER
	hdrConfig.callbacks.audio_align = &HDLIB_cb_audio_align;
#endif
#endif

// For Silab Tuner Si47962
#ifdef USE_HDRLIB_2ND_CHG_VER	// 2.10.3
	// As you decrease the audio delay value, d-audio becomes slower than a-audio.
	// - = <- left move, + = -> right move
	hdrConfig.blend_params.fm_mps_dig_audio_delay = 62333;		// silab: 62333, st: 62375
	hdrConfig.blend_params.am_mps_dig_audio_delay = 13345;		// silab: 13345, st: 13415
	hdrConfig.blend_params.fm_mps_audio_scaling = 16400;		// silab: 16400, st:
	hdrConfig.blend_params.fm_all_dig_audio_scaling = 16400;	// silab: 16400, st:
	hdrConfig.blend_params.am_mps_audio_scaling = 17200;		// silab: 17200, st:
	hdrConfig.blend_params.am_all_dig_audio_scaling = 17200;	// silab: 17200, st:
#else	// 2.6.3
	hdrConfig.blend_params.fm_mps_dig_audio_delay = 64383;		// 64384 <- 64362
	hdrConfig.blend_params.am_mps_dig_audio_delay = 15398;		// 15400
	hdrConfig.blend_params.fm_mps_audio_scaling = 16400;
	hdrConfig.blend_params.fm_all_dig_audio_scaling = 16400;
	hdrConfig.blend_params.am_mps_audio_scaling = 17200;		// 15107
	hdrConfig.blend_params.am_all_dig_audio_scaling = 17200;
#endif

#ifdef USE_HDRLIB_3RD_CHG_VER
	if((auto_align_config.am_auto_level_correction_enabled == true) || (auto_align_config.fm_auto_level_correction_enabled == true)){
        hdrConfig.tx_audio_gain_enabled =false;
    }else{
        hdrConfig.tx_audio_gain_enabled = true;
    }
#else
	if(auto_align_config.apply_level_adjustment == true){
        hdrConfig.mps_tx_audio_gain_enabled =false;
    }else{
        hdrConfig.mps_tx_audio_gain_enabled = true;
    }
#endif
    hdrConfig.adv_blend_params.ramp_up_enabled = true;
    hdrConfig.adv_blend_params.ramp_down_enabled=true;
#ifdef USE_MP11_SERVICE_MODE
	hdrConfig.mp11_enabled = true;
#else
	hdrConfig.mp11_enabled = false;
	(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "MP11 DISABLED\n");
#endif

    (void)(*stOsal.osmemset)((void*)&mem_usage, (S8)0, (U32)sizeof(HDR_mem_usage_t));

	(void)(*stOsal.mutexinit)(&mrcMutex[0], NULL);
	(void)(*stOsal.mutexinit)(&mrcMutex[1], NULL);
#ifdef USE_HDRLIB_2ND_CHG_VER
	pthread_mutexattr_init(&mutexAttr);
	pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_ERRORCHECK);
	(void)(*stOsal.mutexinit)(&reacqCtlMutex[0], &mutexAttr);
	(void)(*stOsal.mutexinit)(&reacqCtlMutex[1], &mutexAttr);
#endif

    for(i = 0; i < numOfHdrInstances; i++) {
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			break;
		}

        switch(i) {
            case 1:		// Secondary Instance
				if(typeOfHdr == HDR_1p5_CONFIG) {
                	stHdrFrameworkData.hdrInstance[i].instance_type = HDR_DATA_ONLY_INSTANCE;
				#ifdef USE_HDRLIB_2ND_CHG_VER
					stHdrFrameworkData.hdrInstance[i].reacq_ctl_mutex = &reacqCtlMutex[1];
				#endif
				}
            	else {
                	stHdrFrameworkData.hdrInstance[i].instance_type = HDR_DEMOD_ONLY_INSTANCE;
				#ifdef USE_HDRLIB_2ND_CHG_VER
					stHdrFrameworkData.hdrInstance[i].reacq_ctl_mutex = &reacqCtlMutex[0];
				#endif
            	}
				stHdrFrameworkData.hdrInstance[i].mrc_mutex = (void*)(NULL);
                break;
            case 2:		// Tertiary Instance
                stHdrFrameworkData.hdrInstance[i].instance_type = HDR_DATA_ONLY_INSTANCE;
			#ifdef USE_HDRLIB_2ND_CHG_VER
				stHdrFrameworkData.hdrInstance[i].reacq_ctl_mutex = &reacqCtlMutex[1];
			#endif
				if(typeOfHdr == HDR_1p5_DUAL_MRC_CONFIG) {
					stHdrFrameworkData.hdrInstance[i].mrc_mutex = (void*)(&mrcMutex[1]);
				}
				else {
					stHdrFrameworkData.hdrInstance[i].mrc_mutex = (void*)(NULL);
				}
                break;
			case 3:		// Quaternary Instance
				stHdrFrameworkData.hdrInstance[i].instance_type = HDR_DEMOD_ONLY_INSTANCE;
			#ifdef USE_HDRLIB_2ND_CHG_VER
				stHdrFrameworkData.hdrInstance[i].reacq_ctl_mutex = &reacqCtlMutex[1];
			#endif
				stHdrFrameworkData.hdrInstance[i].mrc_mutex = (void*)(NULL);
				break;
            default:	// Primary Instance
                stHdrFrameworkData.hdrInstance[i].instance_type = HDR_FULL_INSTANCE;
			#ifdef USE_HDRLIB_2ND_CHG_VER
				stHdrFrameworkData.hdrInstance[i].reacq_ctl_mutex = &reacqCtlMutex[0];
			#endif
				if(typeOfHdr == HDR_1p0_MRC_CONFIG || typeOfHdr == HDR_1p5_DUAL_MRC_CONFIG) {
					stHdrFrameworkData.hdrInstance[i].mrc_mutex = (void*)(&mrcMutex[0]);
				}
				else {
					stHdrFrameworkData.hdrInstance[i].mrc_mutex = (void*)(NULL);
				}
				break;
        }

        stHdrFrameworkData.hdrInstance[i].instance_number = i;

        (void)sem_init(&bbSamplesReadySem[i], 0, 0);
        (void)(*stOsal.mutexinit)(&hdrMutex[i], NULL);
        stHdrFrameworkData.hdrInstance[i].mutex = (void*)(&hdrMutex[i]);
       	stHdrFrameworkData.busyFlag[i] = true;

        ret = HDR_init(&stHdrFrameworkData.hdrInstance[i], &hdrConfig, &mem_usage);

        if(ret < 0){
            (*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Failed to initialize HDR instance[%d] at framework init. Error: %d\n", i, ret);
            ret = (HDRET)eTC_HDR_RET_NG_CORE_INIT;
        }
		else {
			ret = (HDRET)eTC_HDR_RET_OK;
			if(stHdrFrameworkData.hdrInstance[i].instance_type != HDR_DEMOD_ONLY_INSTANCE) {
				ret = HDR_set_band(&stHdrFrameworkData.hdrInstance[i], HDR_BAND_IDLE);
				if(ret < 0){
					(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Failed to set HDR band at framework init. HDR Instance[%d], Error: %d\n", i, ret);
					if(i == 1U) {
						ret = (HDRET)eTC_HDR_RET_OK;	// If main instance is OK, proceed.
					}
					else {
						ret = (HDRET)eTC_HDR_RET_NG_SET_BAND;
					}
				}
				else {
					ret = (HDRET)eTC_HDR_RET_OK;
				}
			}
		}
		stHdrFrameworkData.busyFlag[i] = false;
#ifdef USE_HDRLIB_2ND_CHG_VER
		if (stHdrFrameworkData.hdrInstance[i].instance_type == HDR_FULL_INSTANCE) {
            S32 blend_transition_frames = get_d2a_blend_holdoff(&stHdrFrameworkData.hdrInstance[i]);
            HDR_set_blend_transition_time(stHdrFrameworkData.blendCrossfade, blend_transition_frames);
        }
#endif
    }

	return ret;
}

HDRET tchdrfwk_init(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	const U32 numOfHdrInstances = tchdrfwk_getNumOfHdrInstance();

	ret = tchdrfwk_initMessage();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		tchdrfwk_setAnalogAudioMute(0);
		ret = tchdrfwk_setConfiguration(numOfHdrInstances);

		if(ret == (HDRET)eTC_HDR_RET_OK) {
			(void)tchdrblending_messageInit();
			(void)sem_init(&audioBlendingThreadSem, 0, 0);

		#ifdef DEBUG_TCDAT_TEST_DUMP
			tchdr_datInit(0);
		#endif
		}
	}
	else {
		(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Failed to initialize HDR framework message.\n");
		ret = (HDRET)eTC_HDR_RET_NG_INIT_MESSAGE_QUEUE;
	}

    return ret;
}

static HDRET tchdr_deinitPrimary(U32 instance_number)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDRET rc;

	rc = tchdrmain_messageDeinit();
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_PRI, eLOG_ERR, "Failed to deinit message.\n");
	}
	rc = sem_destroy(&bbSamplesReadySem[instance_number]);
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_PRI, eLOG_ERR, "Failed to deinit semapore.\n");
	}
	rc = (*stOsal.mutexdeinit)(&hdrMutex[instance_number]);
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_PRI, eLOG_ERR, "Failed to deinit mutex.\n");
	}
#ifdef DEBUG_TCDAT_TEST_DUMP
	tchdr_datDeinit(0);
#endif

	return ret;
}

static HDRET tchdr_deinitMRC(U32 instance_number)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDRET rc;

	rc = tchdrmrc_messageDeinit();
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_MRC, eLOG_ERR, "Failed to deinit message.\n");
	}
	rc = sem_destroy(&bbSamplesReadySem[instance_number]);
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_MRC, eLOG_ERR, "Failed to deinit semapore.\n");
	}
	rc = (*stOsal.mutexdeinit)(&hdrMutex[instance_number]);
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_MRC, eLOG_ERR, "Failed to deinit mutex.\n");
	}
	rc = (*stOsal.mutexdeinit)(&mrcMutex[0]);
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_MRC, eLOG_ERR, "Failed to deinit mrc mutex for main.\n");
	}
	rc = (*stOsal.mutexdeinit)(&mrcMutex[1]);
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_MRC, eLOG_ERR, "Failed to deinit mrc mutex for bs.\n");
	}
	bMrcConnection = false;
	bBsMrcConnection = false;

	return ret;
}

static HDRET tchdr_deinitBS(U32 instance_number)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDRET rc;

	rc = tchdrbs_messageDeinit();
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_BS, eLOG_ERR, "Failed to deinit message.\n");
	}
	rc = sem_destroy(&bbSamplesReadySem[instance_number]);
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_BS, eLOG_ERR, "Failed to deinit semapore.\n");
	}
	rc = (*stOsal.mutexdeinit)(&hdrMutex[instance_number]);
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_BS, eLOG_ERR, "Failed to deinit mutex.\n");
	}

	return ret;
}

static HDRET tchdr_deinitAudioBlending(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	HDRET rc;

	rc = tchdrblending_messageDeinit();
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_BLD, eLOG_ERR, "Failed to deinit message.\n");
	}
	rc = sem_destroy(&audioBlendingThreadSem);
	if(rc != (HDRET)eTC_HDR_RET_OK) {
		ret = (HDRET)eTC_HDR_RET_NG_DEINIT;
		(*pfnHdrLog)(eTAG_BLD, eLOG_ERR, "Failed to deinit semapore.\n");
	}

	return ret;
}

HDRET tchdrfwk_open(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 i;
	const U32 numOfHdrInstances = tchdrfwk_getNumOfHdrInstance();

	if(numOfHdrInstances <= NUM_HDR_INSTANCES) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "[%s:%d]:Start to open framework.\n", __func__, __LINE__);

		tchdrfwk_setAnalogAudioMute(0);
		stHdrFrameworkData.digitalAudioStarted=false;

		for(i = 0; i < numOfHdrInstances; i++){
			if(stHdrFrameworkData.hdrInstance[i].instance_type != HDR_DEMOD_ONLY_INSTANCE) {
				stTCHDR_TUNE_t tuneTo = {HDR_BAND_IDLE, 8750, HDR_BB_SRC_744_KHZ};
				(void)tchdr_getHdrTuneInfoWithInstanceNumber(&tuneTo, i);
				ret = HDR_set_band(&stHdrFrameworkData.hdrInstance[i], tuneTo.band);
				if(ret != (HDRET)eTC_HDR_RET_OK){
					(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "[%s:%d]Failed to HDR_set_band function. HDR Instance[%d], Error: %d\n", __func__, __LINE__, i, ret);
				#if (HDR_CONFIG == HDR_1p0_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
					if((i == 1U) || (i == 3U)) {
						ret = (HDRET)eTC_HDR_RET_OK;
					}
					else {
						ret = (HDRET)eTC_HDR_RET_NG_SET_BAND;
						break;
					}
				#else
					ret = (HDRET)eTC_HDR_RET_NG_SET_BAND;
					break;
				#endif
				}
			}
		}

		if(ret == (HDRET)eTC_HDR_RET_OK) {
			if((typeOfHdr == HDR_1p0_MRC_CONFIG) || (typeOfHdr == HDR_1p5_MRC_CONFIG) ||(typeOfHdr == HDR_1p5_DUAL_MRC_CONFIG)) {
				if(bMrcConnection == false) {
					ret = HDR_mrc_connect(&stHdrFrameworkData.hdrInstance[0], &stHdrFrameworkData.hdrInstance[1]);
					if(ret == (S32)0) {
						bMrcConnection = true;
					}
					else {
						bMrcConnection = false;
					}
				}

				// MRC can only be enabled after connection is established
				if(stTcHdrTuneInfo.mainInstance.band == HDR_BAND_FM) {
					ret = HDR_mrc_enable(&stHdrFrameworkData.hdrInstance[0]);
					if(ret != (HDRET)eTC_HDR_RET_OK) {
						(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Failed to enalbe primary MRC when framework is opennig. ret[%d]\n", ret);
					}
					else {
						(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "Enalbed primary MRC when framework is opennig.\n");
					}
				}

				if((typeOfHdr == HDR_1p5_DUAL_MRC_CONFIG) && (bBsMrcConnection == false)) {
					ret = HDR_mrc_connect(&stHdrFrameworkData.hdrInstance[2], &stHdrFrameworkData.hdrInstance[3]);
					if(ret == (S32)0) {
						bBsMrcConnection = true;
					}
					else {
						bBsMrcConnection = false;
					}

					// MRC can only be enabled after connection is established
					if(stTcHdrTuneInfo.bsInstance.band == HDR_BAND_FM) {
						ret = HDR_mrc_enable(&stHdrFrameworkData.hdrInstance[2]);
						if(ret != (HDRET)eTC_HDR_RET_OK) {
							(*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Failed to enalbe backscan MRC when framework is opennig. ret[%d]\n", ret);
						}
						else {
							(*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "Enalbed backscan MRC when framework is opennig.\n");
						}
					}
				}
			}
		}
	}
	else {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]:Invalid number[%d] of instances.\n", __func__, __LINE__, numOfHdrInstances);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_TYPE;
	}

	return ret;
}

HDRET tchdrfwk_close(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 i;
	const U32 numOfHdrInstances = tchdrfwk_getNumOfHdrInstance();

#if (HDR_CONFIG == HDR_1p0_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
	if((typeOfHdr == HDR_1p0_MRC_CONFIG) || (typeOfHdr == HDR_1p5_MRC_CONFIG) ||(typeOfHdr == HDR_1p5_DUAL_MRC_CONFIG)) {
		HDBOOL mrc_enabled = false;
		(void)HDR_mrc_enabled(&stHdrFrameworkData.hdrInstance[0], &mrc_enabled);
		if(mrc_enabled == true) {
			(void)HDR_mrc_disable(&stHdrFrameworkData.hdrInstance[0]);
		}

		if(typeOfHdr == HDR_1p5_DUAL_MRC_CONFIG) {
			(void)HDR_mrc_enabled(&stHdrFrameworkData.hdrInstance[2], &mrc_enabled);
			if(mrc_enabled == true) {
				(void)HDR_mrc_disable(&stHdrFrameworkData.hdrInstance[2]);
			}
		}
	}
#endif

	for(i = 0; i < numOfHdrInstances; i++) {
		if(stHdrFrameworkData.hdrInstance[i].instance_type != HDR_DEMOD_ONLY_INSTANCE) {
			(void)HDR_set_band(&stHdrFrameworkData.hdrInstance[i], HDR_BAND_IDLE);
		}
	}

	return ret;
}

static void tchdrmain_setEventMode(eTCHDR_MAIN_CMD_t evtmode)
{
	eTcHdrMainEventMode = evtmode;
}

static eTCHDR_MAIN_CMD_t tchdrmain_getEventMode(void)
{
	return eTcHdrMainEventMode;
}

HDBOOL tchdrfwk_getMrcStatus(void)
{
	return bMrcConnection;
}

HDBOOL tchdrfwk_getBsMrcStatus(void)
{
	return bBsMrcConnection;
}

static void tchdrmain_sendNotifyMessage(U32 eRcvID, U32 mode, const U32 *data, const void *pdata, S32 err)
{
	U32 txmode = (*stArith.u32add)(mode, (U32)TCHDR_MAIN_NOTIFY_WT);

	switch(eRcvID) {
		case (U32)eTCHDR_SENDER_ID_APP:
			(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_HDR_MAIN, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_SERVICE:
			(void)tchdrsvc_sendMessage(eTCHDR_SENDER_ID_HDR_MAIN, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_AUDIO:
			(void)tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_HDR_MAIN, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_BBINPUT:
			(void)tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_HDR_MAIN, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MAIN:
			(void)tchdrmain_sendMessage(eTCHDR_SENDER_ID_HDR_MAIN, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MRC:
			(void)tchdrmrc_sendMessage(eTCHDR_SENDER_ID_HDR_MAIN, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BS:
			(void)tchdrbs_sendMessage(eTCHDR_SENDER_ID_HDR_MAIN, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BLENDING:
			(void)tchdrblending_sendMessage(eTCHDR_SENDER_ID_HDR_MAIN, txmode, data, pdata, err);
			break;
		default:
			UNUSED(0);
			(*pfnHdrLog)(eTAG_PRI, eLOG_DBG, " %s : Invalid received ID[%d].\n", __func__, eRcvID);
			break;
	}
}

static void tchdrmain_messageParser(const stTcHdrMsgBuf_t *pstMsg)
{
	switch(pstMsg->uiMode) {
		// Event
		case (U32)eTCHDR_MAIN_CMD_OPEN:
			tchdrmain_setEventMode(eTCHDR_MAIN_CMD_OPEN);
			break;

		case (U32)eTCHDR_MAIN_CMD_CLOSE:
			tchdrmain_setEventMode(eTCHDR_MAIN_CMD_CLOSE);
			break;

		case (U32)eTCHDR_MAIN_CMD_RESET:
			tchdrmain_setEventMode(eTCHDR_MAIN_CMD_RESET);
			break;

		// Notification of the other thread: Do it here.

		default:
			UNUSED(0);
			break;
	}
}

static void tchdrmain_eventHandler(stTcHdrMsgBuf_t stRcvMsgQ)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_MAIN_CMD_t eNowEvtMode = tchdrmain_getEventMode();
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};

	switch(eNowEvtMode) {
		// Event
		case eTCHDR_MAIN_CMD_OPEN:
			eEvtSt = tchdrmain_event_open(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_MAIN_CMD_CLOSE:
			eEvtSt = tchdrmain_event_close(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_MAIN_CMD_RESET:
			eEvtSt = tchdrmain_event_reset(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_MAIN_CMD_DUMMY:
			eEvtSt = tchdrmain_event_dummy(stRcvMsgQ, uiSendMsg, &ret);
			break;

		default:
			UNUSED(0);
			break;
	}

	tchdrmain_setEventMode(eTCHDR_MAIN_CMD_NULL);

	switch(eEvtSt) {
		/* Job Good Complete -> No Notify */
		case eTCHDR_EVT_STS_DONE:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eTCHDR_EVT_STS_DONE_NOTIFY:
			tchdrmain_sendNotifyMessage(stRcvMsgQ.uiSender, (U32)eNowEvtMode, uiSendMsg, pNULL, ret);
			break;

		/* Return Error */
		default:
			UNUSED(0);
			break;
	}
}

#ifdef USE_CHANGED_HDRLIB_CB
void HDLIB_cb_post_task(HDR_instance_t* hdr_instance, HDR_task_id_t task_id)
{
    switch(task_id){
        case HDR_FRONT_END_TASK:
            (void)HDR_front_end_task(hdr_instance);
            break;
        case HDR_DEMOD_TASK:
            (void)HDR_demod_task(hdr_instance);
            break;
        case HDR_BIT_PROC_TASK:
            (void)HDR_bit_proc_task(hdr_instance);
            break;
        case HDR_TRANSPORT_TASK:
            (void)HDR_transport_task(hdr_instance);
            break;
        case HDR_AUDIO_DECODER_TASK:
            (void)HDR_audio_decoder_task(hdr_instance);
            break;
        default:
			UNUSED(0);
            break;
    }
}
#else
// tell cpd to start ignoring code - CPD-OFF
void HDR_post_task(HDR_instance_t* hdr_instance, HDR_task_id_t task_id)
{
    switch(task_id){
        case HDR_FRONT_END_TASK:
            (void)HDR_front_end_task(hdr_instance);
            break;
        case HDR_DEMOD_TASK:
            (void)HDR_demod_task(hdr_instance);
            break;
        case HDR_BIT_PROC_TASK:
            (void)HDR_bit_proc_task(hdr_instance);
            break;
        case HDR_TRANSPORT_TASK:
            (void)HDR_transport_task(hdr_instance);
            break;
        case HDR_AUDIO_DECODER_TASK:
            (void)HDR_audio_decoder_task(hdr_instance);
            break;
        default:
			UNUSED(0);
            break;
    }
}
// resume CPD analysis - CPD-ON
#endif

// hdrExecThread takes the samples from the IQ Buffer and Calls the Baseband Input API,
// then executes the HDR library by calling HDR_exec().
// When HDR_exec() returns, the Audio Output API is called and the
// PCM Audio Samples are written into the PCM samples Buffer for the ALSA thread.

#ifdef LONG_TERM_PLAYBACK_DEBUG_PRINT
static S32 count=0;
static S32 seconds=0;
static struct timeval t1;
#endif

static void tchdr_demodProcess(eTC_HDR_ID_t hdrID, U32 instanceNum)
{
	HDR_tune_band_t band;
	S32 numValidSamples = 0;
	U32 numInputSamples = 0;

	band = HDR_get_band_select(&stHdrFrameworkData.hdrInstance[instanceNum]);

    if(band == HDR_BAND_AM){
       numInputSamples = HDR_AM_BB_INPUT_BUFFER_SIZE;
    }
	else {
		// When band is HDR_BAND_IDLE, use the FM sample size as is.
		numInputSamples = HDR_FM_BB_INPUT_BUFFER_SIZE;
	}

	numValidSamples = tchdrbbinput_getSamplesValid(instanceNum);
	if(numValidSamples > 0) {
		U32 numReadSamples = numInputSamples;
		if(numInputSamples == HDR_AM_BB_INPUT_BUFFER_SIZE) {
			// FM(or IDLE) -> AM
			if((U32)numValidSamples >= HDR_FM_BB_INPUT_BUFFER_SIZE) {
				//AM, but when input as FM sample size.
				numReadSamples = HDR_FM_BB_INPUT_BUFFER_SIZE;
			}
		}
		else {
			// AM -> FM(or IDLE)
			if((U32)numValidSamples < HDR_FM_BB_INPUT_BUFFER_SIZE) {
				//FM, but when input as AM sample size.
				numReadSamples = HDR_AM_BB_INPUT_BUFFER_SIZE;
			}
			else {
				UNUSED(numReadSamples);		// for codesonar
			}
		}
		if(numInputSamples > (U32)numValidSamples) {
			eLOG_TAG_t tag;
			if(hdrID == eTC_HDR_ID_MAIN) {
				tag = eTAG_PRI;
			}
			else if(hdrID == eTC_HDR_ID_MRC) {
				tag = eTAG_MRC;
			}
			else if(hdrID == eTC_HDR_ID_BS) {
				tag = eTAG_BS;
			}
			else {
				tag = eTAG_BSMRC;
			}
			(*pfnHdrLog)(tag, eLOG_DBG, "instanceNum[%d]: numInputSamples[%d], numValidSamples[%d], numReadSAmples[%d]\n", instanceNum, numInputSamples, numValidSamples, numReadSamples);
		}
        (void)tchdrbbinput_getSamples(hdrBbInputBuf[instanceNum], numReadSamples, instanceNum);
        // Call the Input API with the IQ samples
#ifdef USE_HDRLIB_2ND_CHG_VER
		(void)HDR_baseband_input(&stHdrFrameworkData.hdrInstance[instanceNum], &hdrBbInputBuf[instanceNum][0], numInputSamples, 0);
#else
        (void)HDR_baseband_input(&stHdrFrameworkData.hdrInstance[instanceNum], &hdrBbInputBuf[instanceNum][0], numInputSamples);
#endif
        /* Change : Call front end task after getting the input.*/
        (void)HDR_front_end_task(&stHdrFrameworkData.hdrInstance[instanceNum]);
	}
}

static void tchdrmain_eventMessageProcess(stTcHdrMsgBuf_t *pstMsg)
{
	(void)tchdrmain_getMessage(pstMsg);
	if(pstMsg->fNewMsg == (U32)eTCHDR_NEW_MSG_EXIST) {
		tchdrmain_messageParser(pstMsg);
	}
	tchdrmain_eventHandler(*pstMsg);
}

void *tchdr_mainThread(void* arg)
{
	stTcHdrMsgBuf_t stRecivedMessage;
	U32 priInstanceNum = 0;

	(*stOsal.setPostThreadAttr)(stPrimaryHdThdAttr, eTAG_PRI);	// Set thread nice & name
	(void)(*stOsal.osmemset)((void*)&stRecivedMessage, (S8)0, (U32)sizeof(stTcHdrMsgBuf_t));

	if(arg != NULL) {
		priInstanceNum = *(U32*)arg;
	}

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	primary_DumpFile = fopen(DUMP_PATH"primary_thread_status.csv", "w");
	clock_gettime(LINUX_CLOCK_TIMER_TYPE, &primary_ChkTimer);
#endif

	stPrimaryHdThdAttr.thread_running = 1;
    while (stPrimaryHdThdAttr.thread_running > 0) {
		tchdrmain_eventMessageProcess(&stRecivedMessage);

        // Send the samples into the HDR Input API at the rate controlled by this semaphore
        (void)sem_wait(&bbSamplesReadySem[priInstanceNum]);
		if(stPrimaryHdThdAttr.thread_running != 1) {
			break;
		}

		tchdr_demodProcess(eTC_HDR_ID_MAIN, priInstanceNum);

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &primary_ChkTimeNow);
		primary_ChkTimeDiff.tv_sec = primary_ChkTimeNow.tv_sec - primary_ChkTimer.tv_sec;
		primary_ChkTimeDiff.tv_nsec = primary_ChkTimeNow.tv_nsec - primary_ChkTimer.tv_nsec;
		if(primary_ChkTimeDiff.tv_nsec < 0) {
			primary_ChkTimeDiff.tv_sec--;
			primary_ChkTimeDiff.tv_nsec += 1e9;
		}
		primary_LoopMs = (primary_ChkTimeDiff.tv_sec*1000) + (primary_ChkTimeDiff.tv_nsec/1000000);
		if(primary_DumpFile != NULL) {
			fprintf(primary_DumpFile, "%d,%d,%d\n", primary_DumpCount++, primary_AccumMs+=primary_LoopMs, primary_LoopMs);
			fflush(primary_DumpFile);
		}
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &primary_ChkTimer);
#endif
    } // End of the thread while loop

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	if(primary_DumpFile != NULL) {
		fclose(primary_DumpFile);
	}
#endif
	stPrimaryHdThdAttr.thread_running = -1;
	(void)tchdr_deinitPrimary(priInstanceNum);
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrPrimary Thread Sequence 06...\n");
	return pNULL;
}

#if (HDR_CONFIG == HDR_1p0_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
static void tchdrmrc_setEventMode(eTCHDR_MRC_CMD_t evtmode)
{
	eTcHdrMrcEventMode = evtmode;
}

static eTCHDR_MRC_CMD_t tchdrmrc_getEventMode(void)
{
	return eTcHdrMrcEventMode;
}

static void tchdrmrc_sendNotifyMessage(U32 eRcvID, U32 mode, const U32 *data, const void *pdata, S32 err)
{
	U32 txmode = (*stArith.u32add)(mode, (U32)TCHDR_MRC_NOTIFY_WT);

	switch(eRcvID) {
		case (U32)eTCHDR_SENDER_ID_APP:
			(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_HDR_MRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_SERVICE:
			(void)tchdrsvc_sendMessage(eTCHDR_SENDER_ID_HDR_MRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_AUDIO:
			(void)tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_HDR_MRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_BBINPUT:
			(void)tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_HDR_MRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MAIN:
			(void)tchdrmain_sendMessage(eTCHDR_SENDER_ID_HDR_MRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MRC:
			(void)tchdrmrc_sendMessage(eTCHDR_SENDER_ID_HDR_MRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BS:
			(void)tchdrbs_sendMessage(eTCHDR_SENDER_ID_HDR_MRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BLENDING:
			(void)tchdrblending_sendMessage(eTCHDR_SENDER_ID_HDR_MRC, txmode, data, pdata, err);
			break;
		default:
			UNUSED(0);
			(*pfnHdrLog)(eTAG_MRC, eLOG_DBG, " %s : Invalid received ID[%d].\n", __func__, eRcvID);
			break;
	}
}

static void tchdrmrc_messageParser(const stTcHdrMsgBuf_t *pstMsg)
{
	switch(pstMsg->uiMode) {
		// Event
		case (U32)eTCHDR_MRC_CMD_OPEN:
			tchdrmrc_setEventMode(eTCHDR_MRC_CMD_OPEN);
			break;

		case (U32)eTCHDR_MRC_CMD_CLOSE:
			tchdrmrc_setEventMode(eTCHDR_MRC_CMD_CLOSE);
			break;

		case (U32)eTCHDR_MRC_CMD_RESET:
			tchdrmrc_setEventMode(eTCHDR_MRC_CMD_RESET);
			break;

		// Notification of the other thread: Do it here.

		default:
			UNUSED(0);
			break;
	}
}

static void tchdrmrc_eventHandler(stTcHdrMsgBuf_t stRcvMsgQ)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_MRC_CMD_t eNowEvtMode = tchdrmrc_getEventMode();
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};

	switch(eNowEvtMode) {
		// Event
		case eTCHDR_MRC_CMD_OPEN:
			eEvtSt = tchdrmrc_event_open(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_MRC_CMD_CLOSE:
			eEvtSt = tchdrmrc_event_close(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_MRC_CMD_RESET:
			eEvtSt = tchdrmrc_event_reset(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_MRC_CMD_DUMMY:
			eEvtSt = tchdrmrc_event_dummy(stRcvMsgQ, uiSendMsg, &ret);
			break;

		default:
			UNUSED(0);
			break;
	}

	tchdrmrc_setEventMode(eTCHDR_MRC_CMD_NULL);

	switch(eEvtSt) {
		/* Job Good Complete -> No Notify */
		case eTCHDR_EVT_STS_DONE:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eTCHDR_EVT_STS_DONE_NOTIFY:
			tchdrmrc_sendNotifyMessage(stRcvMsgQ.uiSender, (U32)eNowEvtMode, uiSendMsg, pNULL, ret);
			break;

		/* Return Error */
		default:
			UNUSED(0);
			break;
	}
}

static void tchdrmrc_eventMessageProcess(stTcHdrMsgBuf_t *pstMsg)
{
	(void)tchdrmrc_getMessage(pstMsg);
	if(pstMsg->fNewMsg == (U32)eTCHDR_NEW_MSG_EXIST) {
		tchdrmrc_messageParser(pstMsg);
	}
	tchdrmrc_eventHandler(*pstMsg);
}

void *tchdr_mrcThread(void* arg)
{
	stTcHdrMsgBuf_t stRecivedMessage;
	U32 mrcInstanceNum = 1;

	(*stOsal.setPostThreadAttr)(stMrcHdThdAttr, eTAG_MRC);	// Set thread nice & name
	(void)(*stOsal.osmemset)((void*)&stRecivedMessage, (S8)0, (U32)sizeof(stTcHdrMsgBuf_t));

	if(arg != NULL) {
		mrcInstanceNum = *(U32*)arg;
	}

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	mrc_DumpFile = fopen(DUMP_PATH"mrc_thread_status.csv", "w");
	clock_gettime(LINUX_CLOCK_TIMER_TYPE, &mrc_ChkTimer);
#endif

	stMrcHdThdAttr.thread_running = 1;
    while (stMrcHdThdAttr.thread_running > 0) {
		tchdrmrc_eventMessageProcess(&stRecivedMessage);

        // Send the Samples into the HDR Input API at the rate controlled by this semaphore
        (void)sem_wait(&bbSamplesReadySem[mrcInstanceNum]);
		if(stMrcHdThdAttr.thread_running != 1) {
			break;
		}

        tchdr_demodProcess(eTC_HDR_ID_MRC, mrcInstanceNum);

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &mrc_ChkTimeNow);
		mrc_ChkTimeDiff.tv_sec = mrc_ChkTimeNow.tv_sec - mrc_ChkTimer.tv_sec;
		mrc_ChkTimeDiff.tv_nsec = mrc_ChkTimeNow.tv_nsec - mrc_ChkTimer.tv_nsec;
		if(mrc_ChkTimeDiff.tv_nsec < 0) {
			mrc_ChkTimeDiff.tv_sec--;
			mrc_ChkTimeDiff.tv_nsec += 1e9;
		}
		mrc_LoopMs = (mrc_ChkTimeDiff.tv_sec*1000) + (mrc_ChkTimeDiff.tv_nsec/1000000);
		if(mrc_DumpFile != NULL) {
			fprintf(mrc_DumpFile, "%d,%d,%d\n", mrc_DumpCount++, mrc_AccumMs+=mrc_LoopMs, mrc_LoopMs);
			fflush(mrc_DumpFile);
		}
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &mrc_ChkTimer);
#endif
    }

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	if(mrc_DumpFile != NULL) {
		fclose(mrc_DumpFile);
	}
#endif
	stMrcHdThdAttr.thread_running = -1;
	(void)tchdr_deinitMRC(mrcInstanceNum);
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrMrc Thread Sequence 05...\n");
	return pNULL;
}
#endif

#if (HDR_CONFIG == HDR_1p5_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG)
static void tchdrbs_setEventMode(eTCHDR_BS_CMD_t evtmode)
{
	eTcHdrBsEventMode = evtmode;
}

static eTCHDR_BS_CMD_t tchdrbs_getEventMode(void)
{
	return eTcHdrBsEventMode;
}

static void tchdrbs_sendNotifyMessage(U32 eRcvID, U32 mode, const U32 *data, const void *pdata, S32 err)
{
	U32 txmode = (*stArith.u32add)(mode, (U32)TCHDR_BS_NOTIFY_WT);

	switch(eRcvID) {
		case (U32)eTCHDR_SENDER_ID_APP:
			(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_HDR_BS, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_SERVICE:
			(void)tchdrsvc_sendMessage(eTCHDR_SENDER_ID_HDR_BS, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_AUDIO:
			(void)tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_HDR_BS, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_BBINPUT:
			(void)tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_HDR_BS, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MAIN:
			(void)tchdrmain_sendMessage(eTCHDR_SENDER_ID_HDR_BS, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MRC:
			(void)tchdrmrc_sendMessage(eTCHDR_SENDER_ID_HDR_BS, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BS:
			(void)tchdrbs_sendMessage(eTCHDR_SENDER_ID_HDR_BS, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BLENDING:
			(void)tchdrblending_sendMessage(eTCHDR_SENDER_ID_HDR_BS, txmode, data, pdata, err);
			break;
		default:
			UNUSED(0);
			(*pfnHdrLog)(eTAG_BS, eLOG_DBG, " %s : Invalid received ID[%d].\n", __func__, eRcvID);
			break;
	}
}

static void tchdrbs_messageParser(const stTcHdrMsgBuf_t *pstMsg)
{
	switch(pstMsg->uiMode) {
		// Event
		case (U32)eTCHDR_BS_CMD_OPEN:
			tchdrbs_setEventMode(eTCHDR_BS_CMD_OPEN);
			break;

		case (U32)eTCHDR_BS_CMD_CLOSE:
			tchdrbs_setEventMode(eTCHDR_BS_CMD_CLOSE);
			break;

		case (U32)eTCHDR_BS_CMD_RESET:
			tchdrbs_setEventMode(eTCHDR_BS_CMD_RESET);
			break;

		// Notification of the other thread: Do it here.

		default:
			UNUSED(0);
			break;
	}
}

static void tchdrbs_eventHandler(stTcHdrMsgBuf_t stRcvMsgQ)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_BS_CMD_t eNowEvtMode = tchdrbs_getEventMode();
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};

	switch(eNowEvtMode) {
		// Event
		case eTCHDR_BS_CMD_OPEN:
			eEvtSt = tchdrbs_event_open(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BS_CMD_CLOSE:
			eEvtSt = tchdrbs_event_close(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BS_CMD_RESET:
			eEvtSt = tchdrbs_event_reset(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BS_CMD_DUMMY:
			eEvtSt = tchdrbs_event_dummy(stRcvMsgQ, uiSendMsg, &ret);
			break;

		default:
			UNUSED(0);
			break;
	}

	tchdrbs_setEventMode(eTCHDR_BS_CMD_NULL);

	switch(eEvtSt) {
		/* Job Good Complete -> No Notify */
		case eTCHDR_EVT_STS_DONE:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eTCHDR_EVT_STS_DONE_NOTIFY:
			tchdrbs_sendNotifyMessage(stRcvMsgQ.uiSender, (U32)eNowEvtMode, uiSendMsg, pNULL, ret);
			break;

		/* Return Error */
		default:
			UNUSED(0);
			break;
	}
}

static void tchdrbs_eventMessageProcess(stTcHdrMsgBuf_t *pstMsg)
{
	(void)tchdrbs_getMessage(pstMsg);
    if(pstMsg->fNewMsg == (U32)eTCHDR_NEW_MSG_EXIST) {
       tchdrbs_messageParser(pstMsg);
    }
    tchdrbs_eventHandler(*pstMsg);
}

void *tchdr_bsThread(void* arg)
{
	stTcHdrMsgBuf_t stRecivedMessage;
	U32 bsInstanceNum = 1;

	(*stOsal.setPostThreadAttr)(stBackscanHdThdAttr, eTAG_BS);	// Set thread nice & name
	(void)(*stOsal.osmemset)((void*)&stRecivedMessage, (S8)0, (U32)sizeof(stTcHdrMsgBuf_t));

	if(arg != NULL) {
		bsInstanceNum = *(U32*)arg;
	}

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	bs_DumpFile = fopen(DUMP_PATH"bs_thread_status.csv", "w");
	clock_gettime(LINUX_CLOCK_TIMER_TYPE, &bs_ChkTimer);
#endif

	stBackscanHdThdAttr.thread_running = 1;
    while (stBackscanHdThdAttr.thread_running > 0) {
		tchdrbs_eventMessageProcess(&stRecivedMessage);

        // Send the Samples into the HDR Input API at the rate controlled by this semaphore
        (void)sem_wait(&bbSamplesReadySem[bsInstanceNum]);
        if(stBackscanHdThdAttr.thread_running != 1) {
            break;
        }

		tchdr_demodProcess(eTC_HDR_ID_BS, bsInstanceNum);

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &bs_ChkTimeNow);
		bs_ChkTimeDiff.tv_sec = bs_ChkTimeNow.tv_sec - bs_ChkTimer.tv_sec;
		bs_ChkTimeDiff.tv_nsec = bs_ChkTimeNow.tv_nsec - bs_ChkTimer.tv_nsec;
		if(bs_ChkTimeDiff.tv_nsec < 0) {
			bs_ChkTimeDiff.tv_sec--;
			bs_ChkTimeDiff.tv_nsec += 1e9;
		}
		bs_LoopMs = (bs_ChkTimeDiff.tv_sec*1000) + (bs_ChkTimeDiff.tv_nsec/1000000);
		if(bs_DumpFile != NULL) {
			fprintf(bs_DumpFile, "%d,%d,%d\n", bs_DumpCount++, bs_AccumMs+=bs_LoopMs, bs_LoopMs);
			fflush(bs_DumpFile);
		}
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &bs_ChkTimer);
#endif
    }

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	if(bs_DumpFile != NULL) {
		fclose(bs_DumpFile);
	}
#endif
	stBackscanHdThdAttr.thread_running = -1;
	(void)tchdr_deinitBS(bsInstanceNum);
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrBackscan Thread Sequence 04...\n");
	return pNULL;
}
#endif

#if (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
static void tchdrbsmrc_setEventMode(eTCHDR_BSMRC_CMD_t evtmode)
{
	eTcHdrBsMrcEventMode = evtmode;
}

static eTCHDR_BSMRC_CMD_t tchdrbsmrc_getEventMode(void)
{
	return eTcHdrBsMrcEventMode;
}

static void tchdrbsmrc_sendNotifyMessage(U32 eRcvID, U32 mode, const U32 *data, const void *pdata, S32 err)
{
	U32 txmode = (*stArith.u32add)(mode, (U32)TCHDR_BSMRC_NOTIFY_WT);

	switch(eRcvID) {
		case (U32)eTCHDR_SENDER_ID_APP:
			(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_HDR_BSMRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_SERVICE:
			(void)tchdrsvc_sendMessage(eTCHDR_SENDER_ID_HDR_BSMRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_AUDIO:
			(void)tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_HDR_BSMRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_BBINPUT:
			(void)tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_HDR_BSMRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MAIN:
			(void)tchdrmain_sendMessage(eTCHDR_SENDER_ID_HDR_BSMRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MRC:
			(void)tchdrmrc_sendMessage(eTCHDR_SENDER_ID_HDR_BSMRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BS:
			(void)tchdrbs_sendMessage(eTCHDR_SENDER_ID_HDR_BSMRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BSMRC:
			(void)tchdrbsmrc_sendMessage(eTCHDR_SENDER_ID_HDR_BSMRC, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BLENDING:
			(void)tchdrblending_sendMessage(eTCHDR_SENDER_ID_HDR_BSMRC, txmode, data, pdata, err);
			break;
		default:
			UNUSED(0);
			(*pfnHdrLog)(eTAG_MRC, eLOG_DBG, " %s : Invalid received ID[%d].\n", __func__, eRcvID);
			break;
	}
}

static void tchdrbsmrc_messageParser(const stTcHdrMsgBuf_t *pstMsg)
{
	switch(pstMsg->uiMode) {
		// Event
		case (U32)eTCHDR_BSMRC_CMD_OPEN:
			tchdrbsmrc_setEventMode(eTCHDR_BSMRC_CMD_OPEN);
			break;

		case (U32)eTCHDR_BSMRC_CMD_CLOSE:
			tchdrbsmrc_setEventMode(eTCHDR_BSMRC_CMD_CLOSE);
			break;

		case (U32)eTCHDR_BSMRC_CMD_RESET:
			tchdrbsmrc_setEventMode(eTCHDR_BSMRC_CMD_RESET);
			break;

		// Notification of the other thread: Do it here.

		default:
			UNUSED(0);
			break;
	}
}

static void tchdrbsmrc_eventHandler(stTcHdrMsgBuf_t stRcvMsgQ)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_BSMRC_CMD_t eNowEvtMode = tchdrbsmrc_getEventMode();
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};

	switch(eNowEvtMode) {
		// Event
		case eTCHDR_BSMRC_CMD_OPEN:
			eEvtSt = tchdrbsmrc_event_open(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BSMRC_CMD_CLOSE:
			eEvtSt = tchdrbsmrc_event_close(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BSMRC_CMD_RESET:
			eEvtSt = tchdrbsmrc_event_reset(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BSMRC_CMD_DUMMY:
			eEvtSt = tchdrbsmrc_event_dummy(stRcvMsgQ, uiSendMsg, &ret);
			break;

		default:
			UNUSED(0);
			break;
	}

	tchdrbsmrc_setEventMode(eTCHDR_BSMRC_CMD_NULL);

	switch(eEvtSt) {
		/* Job Good Complete -> No Notify */
		case eTCHDR_EVT_STS_DONE:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eTCHDR_EVT_STS_DONE_NOTIFY:
			tchdrbsmrc_sendNotifyMessage(stRcvMsgQ.uiSender, (U32)eNowEvtMode, uiSendMsg, pNULL, ret);
			break;

		/* Return Error */
		default:
			UNUSED(0);
			break;
	}
}

static void tchdrbsmrc_eventMessageProcess(stTcHdrMsgBuf_t *pstMsg)
{
	(void)tchdrbsmrc_getMessage(pstMsg);
	if(pstMsg->fNewMsg == (U32)eTCHDR_NEW_MSG_EXIST) {
		tchdrbsmrc_messageParser(pstMsg);
	}
	tchdrbsmrc_eventHandler(*pstMsg);
}

void *tchdr_bsMrcThread(void* arg)
{
	stTcHdrMsgBuf_t stRecivedMessage;
	U32 bsMrcInstanceNum = 1;

	(*stOsal.setPostThreadAttr)(stBsMrcHdThdAttr, eTAG_BSMRC);	// Set thread nice & name
	(void)(*stOsal.osmemset)((void*)&stRecivedMessage, (S8)0, (U32)sizeof(stTcHdrMsgBuf_t));

	if(arg != NULL) {
		bsMrcInstanceNum = *(U32*)arg;
	}

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	bsmrc_DumpFile = fopen(DUMP_PATH"bsmrc_thread_status.csv", "w");
	clock_gettime(LINUX_CLOCK_TIMER_TYPE, &bsmrc_ChkTimer);
#endif

	stBsMrcHdThdAttr.thread_running = 1;
    while (stBsMrcHdThdAttr.thread_running > 0) {
		tchdrbsmrc_eventMessageProcess(&stRecivedMessage);

        // Send the Samples into the HDR Input API at the rate controlled by this semaphore
        (void)sem_wait(&bbSamplesReadySem[bsMrcInstanceNum]);
		if(stBsMrcHdThdAttr.thread_running != 1) {
			break;
		}

        tchdr_demodProcess(eTC_HDR_ID_BS_MRC, bsMrcInstanceNum);

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &bsmrc_ChkTimeNow);
		bsmrc_ChkTimeDiff.tv_sec = bsmrc_ChkTimeNow.tv_sec - bsmrc_ChkTimer.tv_sec;
		bsmrc_ChkTimeDiff.tv_nsec = bsmrc_ChkTimeNow.tv_nsec - bsmrc_ChkTimer.tv_nsec;
		if(bsmrc_ChkTimeDiff.tv_nsec < 0) {
			bsmrc_ChkTimeDiff.tv_sec--;
			bsmrc_ChkTimeDiff.tv_nsec += 1e9;
		}
		bsmrc_LoopMs = (bsmrc_ChkTimeDiff.tv_sec*1000) + (bsmrc_ChkTimeDiff.tv_nsec/1000000);
		if(bsmrc_DumpFile != NULL) {
			fprintf(bsmrc_DumpFile, "%d,%d,%d\n", bsmrc_DumpCount++, bsmrc_AccumMs+=bsmrc_LoopMs, bsmrc_LoopMs);
			fflush(bsmrc_DumpFile);
		}
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &bsmrc_ChkTimer);
#endif
    }

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	if(bsmrc_DumpFile != NULL) {
		fclose(bsmrc_DumpFile);
	}
#endif
	stBsMrcHdThdAttr.thread_running = -1;
	(void)tchdr_deinitMRC(bsMrcInstanceNum);
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrBsMrc Thread Sequence 05...\n");
	return pNULL;
}
#endif

static void tchdrblending_setEventMode(eTCHDR_BLENDING_CMD_t evtmode)
{
	eTcHdrBlendingEventMode = evtmode;
}

static eTCHDR_BLENDING_CMD_t tchdrblending_getEventMode(void)
{
	return eTcHdrBlendingEventMode;
}

static void tchdrblending_sendNotifyMessage(U32 eRcvID, U32 mode, const U32 *data, const void *pdata, S32 err)
{
	U32 txmode = (*stArith.u32add)(mode, (U32)TCHDR_BLENDING_NOTIFY_WT);

	switch(eRcvID) {
		case (U32)eTCHDR_SENDER_ID_APP:
			(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_HDR_BLENDING, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_SERVICE:
			(void)tchdrsvc_sendMessage(eTCHDR_SENDER_ID_HDR_BLENDING, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_AUDIO:
			(void)tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_HDR_BLENDING, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_BBINPUT:
			(void)tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_HDR_BLENDING, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MAIN:
			(void)tchdrmain_sendMessage(eTCHDR_SENDER_ID_HDR_BLENDING, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MRC:
			(void)tchdrmrc_sendMessage(eTCHDR_SENDER_ID_HDR_BLENDING, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BS:
			(void)tchdrbs_sendMessage(eTCHDR_SENDER_ID_HDR_BLENDING, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BLENDING:
			(void)tchdrblending_sendMessage(eTCHDR_SENDER_ID_HDR_BLENDING, txmode, data, pdata, err);
			break;
		default:
			UNUSED(0);
			(*pfnHdrLog)(eTAG_BLD, eLOG_DBG, " %s : Invalid received ID[%d].\n", __func__, eRcvID);
			break;
	}
}

static void tchdrblending_messageParser(const stTcHdrMsgBuf_t *pstMsg)
{
	switch(pstMsg->uiMode) {
		// Event
		case (U32)eTCHDR_BLENDING_CMD_OPEN:
			tchdrblending_setEventMode(eTCHDR_BLENDING_CMD_OPEN);
			break;

		case (U32)eTCHDR_BLENDING_CMD_CLOSE:
			tchdrblending_setEventMode(eTCHDR_BLENDING_CMD_CLOSE);
			break;

		case (U32)eTCHDR_BLENDING_CMD_RESET:
			tchdrblending_setEventMode(eTCHDR_BLENDING_CMD_RESET);
			break;

		case (U32)eTCHDR_BLENDING_CMD_AUDIO_MODE:
			tchdrblending_setEventMode(eTCHDR_BLENDING_CMD_AUDIO_MODE);
			break;

		// Notification of the other thread: Do it here.

		default:
			UNUSED(0);
			break;
	}
}

static void tchdrblending_eventHandler(stTcHdrMsgBuf_t stRcvMsgQ)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_BLENDING_CMD_t eNowEvtMode = tchdrblending_getEventMode();
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};

	switch(eNowEvtMode) {
		// Event
		case eTCHDR_BLENDING_CMD_OPEN:
			eEvtSt = tchdrblending_event_open(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BLENDING_CMD_CLOSE:
			eEvtSt = tchdrblending_event_close(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BLENDING_CMD_RESET:
			eEvtSt = tchdrblending_event_reset(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BLENDING_CMD_AUDIO_MODE:
			eEvtSt = tchdrblending_event_audioMode(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BLENDING_CMD_DUMMY:
			eEvtSt = tchdrblending_event_dummy(stRcvMsgQ, uiSendMsg, &ret);
			break;

		default:
			UNUSED(0);
			break;
	}

	tchdrblending_setEventMode(eTCHDR_BLENDING_CMD_NULL);

	switch(eEvtSt) {
		/* Job Good Complete -> No Notify */
		case eTCHDR_EVT_STS_DONE:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eTCHDR_EVT_STS_DONE_NOTIFY:
			tchdrblending_sendNotifyMessage(stRcvMsgQ.uiSender, (U32)eNowEvtMode, uiSendMsg, pNULL, ret);
			break;

		/* Return Error */
		default:
			UNUSED(0);
			break;
	}
}

static void tchdrblending_eventMessageProcess(stTcHdrMsgBuf_t *pstMsg)
{
	(void)tchdrblending_getMessage(pstMsg);
	if(pstMsg->fNewMsg == (U32)eTCHDR_NEW_MSG_EXIST) {
		tchdrblending_messageParser(pstMsg);
	}
	tchdrblending_eventHandler(*pstMsg);
}

static void tchdrblending_getSlipInfo(HDR_instance_t *hdrInstance, HDBOOL *hybridProgram, HDR_bb_sample_slips_t *bbSampleSlips, S32 *ppmEst)
{
	(void)HDR_hybrid_program(hdrInstance, hybridProgram);
	(void)HDR_get_bb_sample_slips(hdrInstance, bbSampleSlips);
	(void)HDR_get_clock_offset(hdrInstance, ppmEst);
}

static void tchdrblending_audioResamplerProcess(HDR_tune_band_t band, U32 *audioQuality, HDBOOL *blendFlag, HDBOOL *hybridProgram)
{
	HDR_bb_sample_slips_t bbSampleSlips;
	S32 ppmEst = 0;
    S32 rc;

#if 0 // Analog output of the silab tuner is not phase invered. Only the half phase has disappeared.
	// Analog is phase inverted. Telechips need to fix this in the tuner.The analog is scaled down by 3.4 dB to accomodate
	// for Headroom to support TX Gain parameter. Please scale the analog from the tuner only for FM. This is needed to pass
	// certification.
	S32 i;
	for(i = 0; i < HDR_AUDIO_FRAME_SIZE; i++){
		stHdrFrameworkData.analogAudio[i].left = (S16)(((S32)stHdrFrameworkData.analogAudio[i].left * -22145)/32768);
		stHdrFrameworkData.analogAudio[i].right = (S16)(((S32)stHdrFrameworkData.analogAudio[i].right * -22145)/32768);
	}
#endif

	(void)(*stOsal.osmemset)((void*)&bbSampleSlips, (S8)0, (U32)sizeof(HDR_bb_sample_slips_t));
	if(tchdrframework_getAnalogAudioMute() == true) {
		(void)(*stOsal.osmemset)((void*)stHdrFrameworkData.analogAudio, (S8)0, HDR_AUDIO_FRAME_SIZE * (U32)sizeof(HDR_pcm_stereo_t));
	}

	tchdrblending_getSlipInfo(&stHdrFrameworkData.hdrInstance[0], hybridProgram, &bbSampleSlips, &ppmEst);

	//HDR_get_freq_offset(&stHdrFrameworkData.hdrInstance[0],&freqEst);
	//(*pfnHdrLog)(eTAG_BLD, eLOG_DBG, "ppmEst %d %d %d\n",ppmEst,bbSampleSlips.num_samples, bbSampleSlips.symbol_count);

	// Read Audio PCM samples from HD Radio Output API
	rc = HDR_get_audio_output(&stHdrFrameworkData.hdrInstance[0], stHdrFrameworkData.digitalAudio, HDR_AUDIO_FRAME_SIZE,
			  audioQuality, blendFlag);

	if((rc > 0) && !stHdrFrameworkData.digitalAudioStarted) {
		stHdrFrameworkData.digitalAudioStarted = true;
	}

	// keep updating the sample slips so that we dont drift if RC becomes 0 once in a while when HD Audio is playing.
	if(stHdrFrameworkData.digitalAudioStarted) {
#ifdef USE_HDRLIB_2ND_CHG_VER
		// ex) clk_offset = -28.26ppm x (2^16) = -28.26 x 65536 = -1,852,047.36 ~= -1852047 = 0xFFE3BD71
		(void)HDR_audio_resample_update_slips(stHdrFrameworkData.hdaoutResampler, bbSampleSlips.num_samples, band, extClockOffset, ppmEst);
#else
		(void)HDR_audio_resample_update_slips(stHdrFrameworkData.hdaoutResampler, bbSampleSlips.num_samples, band, ppmEst);
#endif
	}

	if(rc > 0) {
		(void)HDR_audio_resampler_input(stHdrFrameworkData.hdaoutResampler, stHdrFrameworkData.digitalAudio);
	}

	// If analog audio output rate is slower than HD rate(tuner clock is slower than transmitter clock),
	// eventually HD audio will accumulate an additional frame - a second call is needed to account for
	// that. The audio resampler will slowly remove the extra samples to match the analog rate.
	rc = HDR_get_audio_output(&stHdrFrameworkData.hdrInstance[0], stHdrFrameworkData.digitalAudio, HDR_AUDIO_FRAME_SIZE,
		  audioQuality, blendFlag);

	if(rc > 0) {
		(void)HDR_audio_resampler_input(stHdrFrameworkData.hdaoutResampler, stHdrFrameworkData.digitalAudio);
	}

	(void)HDR_audio_resampler_output(stHdrFrameworkData.hdaoutResampler, stHdrFrameworkData.digitalAudio);

#ifdef DEBUG_TCDAT_TEST_DUMP
	tchdr_datProc(0, stHdrFrameworkData.digitalAudio, stHdrFrameworkData.analogAudio, HDR_AUDIO_FRAME_SIZE);
#endif
}

static void tchdrblending_automaticAudioAlignmentProcess(U32 audioQuality, HDBOOL *blendFlag)
{
	S32 timeOffset = 0;
	S32 levelOffset = 0;
	HDR_auto_align_rc_t alignRc;
	HDBOOL blendAlignInProgress = true;
	static HDBOOL bAAAEnabled = false;	// variable for AAA status log

    (void)HDR_get_blend_align_progress_status(&stHdrFrameworkData.hdrInstance[0], &blendAlignInProgress);

	alignRc = HDR_auto_align_exec(stHdrFrameworkData.autoAlign, stHdrFrameworkData.analogAudio, stHdrFrameworkData.digitalAudio,
	              audioQuality, *blendFlag, blendAlignInProgress, &timeOffset, &levelOffset);

	if(alignRc != HDR_AUTO_ALIGN_DISABLED) {
		if(bAAAEnabled != true) {
			bAAAEnabled = true;
			if(alignRc != HDR_AUTO_ALIGN_ERROR) {
				(*pfnHdrLog)(eTAG_BLD, eLOG_INF, "Auto-alignment enabled.\n");
			}
			else {
				(*pfnHdrLog)(eTAG_BLD, eLOG_ERR, "Auto-alignment error occurred.\n");
			}
		}
		if(alignRc == HDR_AUTO_ALIGN_APPLY_OFFSET) {
#ifdef USE_HDRLIB_2ND_CHG_VER
			U32 decodedAudioDelay = 0;

			// Remember that we were in APPLY_OFFSET state
			stHdrFrameworkData.aaaState = alignRc;
			if(HDR_blend_adjust_audio_delay(&stHdrFrameworkData.hdrInstance[0], timeOffset, &decodedAudioDelay) == 0) {
				(*pfnHdrLog)(eTAG_BLD, eLOG_DBG, "Auto-alignment sample adjustment: %d.\n", (S32)timeOffset);
				decodedAudioDelay += HDR_audio_resampler_avail_data(stHdrFrameworkData.hdaoutResampler);
				HDR_auto_align_set_holdoff(stHdrFrameworkData.autoAlign, decodedAudioDelay);
				stHdrFrameworkData.alignmentSuccess = true;
			}
			else {
				(*pfnHdrLog)(eTAG_BLD, eLOG_DBG, "Auto-alignment failed; sample adjustment is too large: %d\n", (S32)timeOffset);
				stHdrFrameworkData.alignmentSuccess = false;
			}
#else
			if(HDR_blend_adjust_audio_delay(&stHdrFrameworkData.hdrInstance[0], timeOffset) == 0) {
				(*pfnHdrLog)(eTAG_BLD, eLOG_DBG, "Auto-alignment sample adjustment: %d.\n", (S32)timeOffset);
				stHdrFrameworkData.alignmentSuccess = true;
			} else {
				(*pfnHdrLog)(eTAG_BLD, eLOG_DBG, "Auto-alignment failed; sample adjustment is too large: %d\n", (S32)timeOffset);
				stHdrFrameworkData.alignmentSuccess = false;
			}
#endif
		}

#ifdef USE_HDRLIB_2ND_CHG_VER
		if(stHdrFrameworkData.aaaState == HDR_AUTO_ALIGN_PLAY_DIGITAL) {
			// AAA was not running, not aligning, or already playing Digital
			// - pass blendFlag through
			if (*blendFlag == false) {
				// Not playing Digital - reset AAA state
				stHdrFrameworkData.aaaState = 0;
			}
		}
		else {
			if((alignRc == HDR_AUTO_ALIGN_PLAY_DIGITAL)  && (stHdrFrameworkData.alignmentSuccess == true)) {
				*blendFlag = true;
				// Playing Digital - reset saved state
				stHdrFrameworkData.aaaState = alignRc;
			} else {
				*blendFlag = false;
			}
		}
#else
		if((alignRc == HDR_AUTO_ALIGN_PLAY_DIGITAL)  && (stHdrFrameworkData.alignmentSuccess == true)) {
			*blendFlag = true;
		} else {
			*blendFlag = false;
		}
#endif
	}
	else {
		if(bAAAEnabled != false) {
			bAAAEnabled = false;
			(*pfnHdrLog)(eTAG_BLD, eLOG_INF, "Auto-alignment disabled.\n");
		}
		else {
			UNUSED(bAAAEnabled);
		}
	}
	stHdrFrameworkData.digitalAudioAcquired = *blendFlag;
}

void *tchdr_audioBlendingThread(void* arg)
{
	stTcHdrMsgBuf_t stRecivedMessage;

	(*stOsal.setPostThreadAttr)(stAudioBlendingThdAttr, eTAG_BLD);	// Set thread nice & name
	(void)(*stOsal.osmemset)((void*)&stRecivedMessage, (S8)0, (U32)sizeof(stTcHdrMsgBuf_t));

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	audblend_DumpFile = fopen(DUMP_PATH"audblend_thread_status.csv", "w");
	clock_gettime(LINUX_CLOCK_TIMER_TYPE, &audblend_ChkTimer);
#endif

	stAudioBlendingThdAttr.thread_running = 1;
	while(stAudioBlendingThdAttr.thread_running > 0) {
		tchdrblending_eventMessageProcess(&stRecivedMessage);

		(void)sem_wait(&audioBlendingThreadSem);
		if(stAudioBlendingThdAttr.thread_running != 1) {
			break;
		}

		if((*stCircFunc.cb_read)(&analogAudioBuffer, (void*)(stHdrFrameworkData.analogAudio), HDR_AUDIO_FRAME_SIZE) > 0)
		{
			U32 audioQuality = 0;
			HDBOOL blendFlag = false;
			HDBOOL hybridProgram = true;
			HDR_tune_band_t band;

			band = HDR_get_band_select(&stHdrFrameworkData.hdrInstance[0]);
			tchdrblending_audioResamplerProcess(band, &audioQuality, &blendFlag, &hybridProgram);

#ifdef USE_AUTO_AUDIO_ALIGN
			if(hybridProgram == true) {
				tchdrblending_automaticAudioAlignmentProcess(audioQuality, &blendFlag);
			}
			else {
				stHdrFrameworkData.digitalAudioAcquired = HDR_digital_audio_acquired(&stHdrFrameworkData.hdrInstance[0]);
			#ifdef USE_HDRLIB_2ND_CHG_VER
				// SPS Digital Audio - reset AAA
				HDR_auto_align_reset(stHdrFrameworkData.autoAlign, band);
				// and remember this satate
				stHdrFrameworkData.aaaState = HDR_AUTO_ALIGN_PLAY_DIGITAL;
			#endif
			}
#else
			if(hybridProgram == true) {
				stHdrFrameworkData.digitalAudioAcquired = blendFlag;
			}
			else {
				stHdrFrameworkData.digitalAudioAcquired = HDR_digital_audio_acquired(&stHdrFrameworkData.hdrInstance[0]);
			}
#endif
#ifdef USE_HDRLIB_2ND_CHG_VER
			// Pass updated blendFlag to library
			HDR_blend_set_flag(&stHdrFrameworkData.hdrInstance[0], blendFlag);
#endif
			tchdrblending_audioOutputHandler(blendFlag, hybridProgram);
		}
		else {
			(*pfnHdrLog)(eTAG_BLD, eLOG_DBG, ">>>>>>>>>>>>>>>>>>>>>  Audio Input Buffer Empty!!!!! >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		}
#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &audblend_ChkTimeNow);
		audblend_ChkTimeDiff.tv_sec = audblend_ChkTimeNow.tv_sec - audblend_ChkTimer.tv_sec;
		audblend_ChkTimeDiff.tv_nsec = audblend_ChkTimeNow.tv_nsec - audblend_ChkTimer.tv_nsec;
		if(audblend_ChkTimeDiff.tv_nsec < 0) {
			audblend_ChkTimeDiff.tv_sec--;
			audblend_ChkTimeDiff.tv_nsec += 1e9;
		}
		audblend_LoopMs = (audblend_ChkTimeDiff.tv_sec*1000) + (audblend_ChkTimeDiff.tv_nsec/1000000);
		if(audblend_DumpFile != NULL) {
			fprintf(audblend_DumpFile, "%d,%d,%d\n", audblend_DumpCount++, audblend_AccumMs+=audblend_LoopMs, audblend_LoopMs);
			fflush(audblend_DumpFile);
		}
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &audblend_ChkTimer);
#endif
	}

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	if(audblend_DumpFile != NULL) {
		fclose(audblend_DumpFile);
	}
#endif
	stAudioBlendingThdAttr.thread_running = -1;
	(void)tchdr_deinitAudioBlending();
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrBlending Thread Sequence 07...\n");
	return pNULL;
}

static void tchdrblending_audioOutputHandler(HDBOOL blendFlag, HDBOOL hybridProgram)
{
    HDR_pcm_stereo_t* audioOutput = NULL;
	HDR_blend_control_t blendControl = HDR_BLEND_CROSSFADE;
    static HDR_blend_control_t blendControlOld;
	HDBOOL blendStatus = blendFlag;
	U32 curBlendCtrlMode = 2;
	static U32 prevBlendCtrlMode;
	S32 rc = 0;

#ifdef USE_ANALOG_AUDIO_MUTE_FOR_TUNE
	if(tchdrblending_getAAMute()) {
		(void)(*stOsal.osmemset)((void*)stHdrFrameworkData.analogAudio, (S8)0, HDR_AUDIO_FRAME_SIZE * (U32)sizeof(HDR_pcm_stereo_t));
		tchdrblending_setAAMute(false);
	}
#endif

    switch(stHdrFrameworkData.audioMode) {
	    case eHDR_AUDIO_BLEND:
	    case eHDR_AUDIO_ANALOG_SPLIT:
	        if(stHdrFrameworkData.audioMode == eHDR_AUDIO_BLEND) {
				//During Re-Acquisition this function will return an error condition and blendControl should hold on to its last value.
				rc = HDR_get_blend_control(&stHdrFrameworkData.hdrInstance[0], &blendControl);

	            if(stHdrFrameworkData.playAlertTone == true) {
	                HDBOOL alertFinished = true;
	                (void)HDR_alert_get_tone_audio(&stHdrFrameworkData.hdrInstance[0], stHdrFrameworkData.digitalAudio, HDR_AUDIO_FRAME_SIZE, &alertFinished);
	                stHdrFrameworkData.playAlertTone = !alertFinished;
	                blendControl = HDR_PLAY_DIGITAL;
	            }
                //Changed the blend Control and crossfade sequence to fix a bug. Please retain this change.
                if(blendControl == HDR_PLAY_DIGITAL){
                    blendStatus = true;
                    (void)(*stOsal.osmemset)((void*)stHdrFrameworkData.analogAudio, (S8)0, HDR_AUDIO_FRAME_SIZE * (U32)sizeof(HDR_pcm_stereo_t));
                }
                if(blendControl == HDR_PLAY_ANALOG){
                    blendStatus = false;
                    if(hybridProgram == false){
                    // If this is an all-digital program and force-analog is requested, play mute
                        (void)(*stOsal.osmemset)((void*)stHdrFrameworkData.analogAudio, (S8)0, HDR_AUDIO_FRAME_SIZE * (U32)sizeof(HDR_pcm_stereo_t));
                    }
                    (void)(*stOsal.osmemset)((void*)stHdrFrameworkData.digitalAudio, (S8)0, HDR_AUDIO_FRAME_SIZE * (U32)sizeof(HDR_pcm_stereo_t));
                    audioOutput = stHdrFrameworkData.analogAudio;
                }
                (void)HDR_blend_crossfade(stHdrFrameworkData.blendCrossfade, stHdrFrameworkData.digitalAudio, stHdrFrameworkData.analogAudio, blendStatus, &audioOutput);

#if 1	// for test
				(void)HDR_test_get_raw_tx_blend_control(&stHdrFrameworkData.hdrInstance[0], &curBlendCtrlMode);
				if(blendControl != blendControlOld) {
					//(*pfnHdrLog)(eTAG_BLD, eLOG_INF, "blendControl[%d], blendControlOld[%d]\n", blendControl, blendControlOld);
					if(blendControl == HDR_BLEND_CROSSFADE || blendControl == HDR_PLAY_DIGITAL) {
						if(rc != 0 && blendControl == HDR_BLEND_CROSSFADE) {
							(*pfnHdrLog)(eTAG_BLD, eLOG_INF, "Switching to analog audio[%d]. rc[%d]\n", (U32)blendControl, rc);
						}
						else {
							(*pfnHdrLog)(eTAG_BLD, eLOG_INF, "Switching to digital audio[%d].\n", (U32)blendControl);
						}
					}
					else {
						(*pfnHdrLog)(eTAG_BLD, eLOG_INF, "Switching to analog audio[%d].\n", (U32)blendControl);
					}
				}
				if(prevBlendCtrlMode != curBlendCtrlMode) {
					if(curBlendCtrlMode == 0) {
						(*pfnHdrLog)(eTAG_BLD, eLOG_INF, "The blend control is not required.\n");
                        stHdrFrameworkData.ballgameMode = false;
					}
					else {
						(*pfnHdrLog)(eTAG_BLD, eLOG_INF, "Ball game mode is %s.\n", (curBlendCtrlMode == 1) ? "on" : "off");
                        if (curBlendCtrlMode == 1) {
                            stHdrFrameworkData.ballgameMode = true;
                        } else {
                            stHdrFrameworkData.ballgameMode = false;
                        }
					}
				}
#endif
				UNUSED(curBlendCtrlMode);	// for codesonar
				UNUSED(rc);					// for codesonar
	        }
			else {
	            // analog-split
	            U32 n;
	            for (n = 0; n < HDR_AUDIO_FRAME_SIZE; n++) {
	                // Left = HD audio, L; Right = analog audio
	                stHdrFrameworkData.analogAudio[n].left = stHdrFrameworkData.digitalAudio[n].right;
	            }
	            audioOutput = stHdrFrameworkData.analogAudio;
	        }
	        break;

	    case eHDR_AUDIO_ANALOG_ONLY:
		#ifdef USE_AUTO_MUTE_WHEN_ONLY_ANALOG_AUDIO_MODE
			blendStatus = false;
			if(hybridProgram == false) {
             // If this is an all-digital program and force-analog is requested, play mute
				(void)(*stOsal.osmemset)((void*)stHdrFrameworkData.analogAudio, 0, HDR_AUDIO_FRAME_SIZE * sizeof(HDR_pcm_stereo_t));
			}
		#endif
	        (void)HDR_blend_crossfade(stHdrFrameworkData.blendCrossfade, stHdrFrameworkData.digitalAudio, stHdrFrameworkData.analogAudio, false, &audioOutput);
	        //audioOutput = stHdrFrameworkData.analogAudio;
	        break;

	    case eHDR_AUDIO_DIGITAL_ONLY:
	        (void)HDR_blend_crossfade(stHdrFrameworkData.blendCrossfade, stHdrFrameworkData.digitalAudio, stHdrFrameworkData.analogAudio, true, &audioOutput);
	        //audioOutput = stHdrFrameworkData.digitalAudio;
	        break;

	    default:
			UNUSED(0);
			break;
    }

	blendControlOld = blendControl;
	prevBlendCtrlMode = curBlendCtrlMode;

    if(audioOutput != NULL) {
        tchdraudoutput_rxHandler(audioOutput);
    }
}

void tchdraudinput_ready(void)
{
    (void)sem_post(&audioBlendingThreadSem);
}

void tchdrbbinput_ready(U32 instanceNum)
{
    (void)sem_post(&bbSamplesReadySem[instanceNum]);
}

S32 tchdrbbinput_getReadySemaValue(U32 instanceNum)
{
	S32 val;
	S32 rc;
	rc = sem_getvalue(&bbSamplesReadySem[instanceNum], &val);
	if(rc == 0) {
		rc = val;
	}

	return rc;
}

S32 tchdraudinput_getReadySemaValue(void)
{
	S32 val;
	S32 rc;
	rc = sem_getvalue(&audioBlendingThreadSem, &val);
	if(rc == 0) {
		rc = val;
	}

	return rc;
}

HDBOOL tchdrfwk_getBallGameMode(const HDR_instance_t* hdr_instance)
{
	HDBOOL audio_acquired = false;
	if(hdr_instance->instance_type == HDR_FULL_INSTANCE) {
		audio_acquired = stHdrFrameworkData.ballgameMode;
	}
	return audio_acquired;
}

HDBOOL tchdrfwk_getDigitalAudioAcquired(const HDR_instance_t* hdr_instance)
{
	HDBOOL audio_acquired = false;
	if(hdr_instance->instance_type == HDR_FULL_INSTANCE) {
		audio_acquired = stHdrFrameworkData.digitalAudioAcquired;
	}
	return audio_acquired;
}

HDR_instance_t* CMD_cb_bbp_get_hdr_instance(U32 instanceNum)
{
	HDR_instance_t* ret = NULL;
    // Removed the Assert to fix a bug. This will return NULL to the 2206 command and the command processor will deal with it.
   if((instanceNum > 0U) && (instanceNum <= NUM_HDR_INSTANCES)) {
       ret = &stHdrFrameworkData.hdrInstance[instanceNum - 1U];
   }

    return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/*								Telechips HD Radio Service Event Functions							*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
static eTCHDR_EVT_STS_t tchdrmain_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrmain_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}
static eTCHDR_EVT_STS_t tchdrmain_event_reset(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrmain_event_dummy(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrmrc_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrmrc_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}
static eTCHDR_EVT_STS_t tchdrmrc_event_reset(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrmrc_event_dummy(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrbs_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrbs_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}
static eTCHDR_EVT_STS_t tchdrbs_event_reset(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrbs_event_dummy(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrbsmrc_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrbsmrc_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}
static eTCHDR_EVT_STS_t tchdrbsmrc_event_reset(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrbsmrc_event_dummy(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrblending_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrblending_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}
static eTCHDR_EVT_STS_t tchdrblending_event_reset(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrblending_event_audioMode(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE_NOTIFY;


	switch(stRcvMsgQ.uiData[0]) {
		case (U32)eHDR_AUDIO_BLEND:			stHdrFrameworkData.audioMode = eHDR_AUDIO_BLEND;		break;
		case (U32)eHDR_AUDIO_ANALOG_ONLY:	stHdrFrameworkData.audioMode = eHDR_AUDIO_ANALOG_ONLY;	break;
		case (U32)eHDR_AUDIO_DIGITAL_ONLY:	stHdrFrameworkData.audioMode = eHDR_AUDIO_DIGITAL_ONLY; break;
		case (U32)eHDR_AUDIO_ANALOG_SPLIT:	stHdrFrameworkData.audioMode = eHDR_AUDIO_ANALOG_SPLIT; break;
		default:
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			break;
	}

	if(uiSendMsg != NULL) {
		uiSendMsg[0] = (U32)stHdrFrameworkData.audioMode;
	}

	if(iError != NULL) {
		*iError = ret;
	}
	return eEvtSts;
}

static eTCHDR_EVT_STS_t tchdrblending_event_dummy(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSts = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	UNUSED(uiSendMsg);
	return eEvtSts;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/*					      Telechips HD Radio Command Processor Thread							    */
//////////////////////////////////////////////////////////////////////////////////////////////////////

#define CMD_ETHERNET_PORT       (8086)

static U8 cmdMsgBuffer[COMMAND_MESSAGE_BUFFER_SIZE];

// HDR command processor thread function
// byteStream.c must be compiled with defined HDR_ETH_ENABLED macro for
// command processor to work over ethernet
void *tchdr_cmdProcThread(void* arg)
{
    COMMAND_PROC_CONFIG_T cmdConfig;

	(*stOsal.setPostThreadAttr)(stCmdProcThdAttr, eTAG_CDM);	// Set thread nice & name

    cmdConfig.byteStream.deviceType = eHDR_DEV_ETHER;
    cmdConfig.byteStream.dev.ether.port = CMD_ETHERNET_PORT;

    if(tchdr_bytestream_open(&cmdConfig.byteStream) < 0){
        (*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "Failed to initialize byte stream for CDM I/F.\n");
    }

    cmdConfig.msgBuffer = cmdMsgBuffer;

    if (commandProcessorInit(&cmdConfig) < 0){
		stCmdProcThdAttr.thread_running = 0;
        (*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "Failed to initialize Command Processor, terminating command processor thread!\n");
    }
	else {
	    BBP_sys_init_default_config();
		stCmdProcThdAttr.thread_running = 1;
	}

    while (stCmdProcThdAttr.thread_running > 0){
         commandProcessorExec();
    }

	stCmdProcThdAttr.thread_running = -1;
	(void)tchdr_bytestream_close(&cmdConfig.byteStream);
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrCmdProc Thread Sequence 09...\n");
	return pNULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/*					         Telechips HD Radio Logger Reader Thread							    */
//////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOGGER_PORT         (8888)
#define LOG_BUFFER_SIZE     (2048U)
#define HEADER_PATTERN      (0xA5A5A5A5U)

stHDR_BYTE_STREAM_t loggerByteStream;
static U8 loggerRxBuffer[LOG_BUFFER_SIZE];

void *tchdr_loggerReaderThread(void *arg)
{
	S32 retry = 0;
	S32 ret = 0;
	U32 messageLength = 0;
	pthread_mutex_t loggerMutex;

	(*stOsal.setPostThreadAttr)(stLoggerThdAttr, eTAG_XLOG);	// Set thread nice & name

    (*pfnHdrLog)(eTAG_XLOG, eLOG_DBG, "Log Reader Thread %u Started\n", (pid_t)syscall( __NR_gettid));

    loggerByteStream.deviceType = eHDR_DEV_ETHER;
    loggerByteStream.dev.ether.port = LOGGER_PORT;

    if(tchdr_bytestream_open(&loggerByteStream) < 0){
        stLoggerThdAttr.thread_running = 0;
		(*pfnHdrLog)(eTAG_XLOG, eLOG_ERR, "Failed to initialize logger byte stream.\n");
    }
	else {
		(void)(*stOsal.mutexinit)(&loggerMutex, NULL);
		HDR_Logger_init((void*)&loggerMutex);
		stLoggerThdAttr.thread_running = 1;
	}

    while(stLoggerThdAttr.thread_running > 0){
		U32 header;
		U32 numModules;
        U32 module;
        U32 bitmask;
        U32 i;

		ret = tchdr_bytestream_read(&loggerByteStream, loggerRxBuffer, 8);
		if(ret < 0) {
			if(ret == -2) {	// failed to get TCP/IP socket
				if(retry > 10) {
					(*pfnHdrLog)(eTAG_XLOG, eLOG_ERR, "Failed to run logger thread!!!\n");
					stLoggerThdAttr.thread_running = 0;
					break;
				}
				else {
					(void)sleep(1);
				}
				retry++;
			}
			else {
				retry = 0;
			}
			continue;
		}

        (void)(*stOsal.osmemcpy)((void*)&header, (void*)loggerRxBuffer, 4);

        if(header != HEADER_PATTERN){
            continue;
        }

        (void)(*stOsal.osmemcpy)((void*)&messageLength, (void*)&loggerRxBuffer[4], 4);

        if(messageLength > LOG_BUFFER_SIZE) {
            (*pfnHdrLog)(eTAG_XLOG, eLOG_ERR, "Invalid log message length!!!\n");
            continue;
        }

        if(tchdr_bytestream_read(&loggerByteStream, loggerRxBuffer, messageLength) < 0){
            continue;
        }

        (void)HDR_logger_disable_all();

        numModules = (messageLength) / 8U;

        for(i = 0; i < numModules; ++i){
            (void)(*stOsal.osmemcpy)((void*)&module, (void*)&loggerRxBuffer[8U * i], 4);
            (void)(*stOsal.osmemcpy)((void*)&bitmask, (void*)&loggerRxBuffer[(8U * i) + 4U], 4);

            (void)HDR_logger_enable(module, bitmask);
        }
    }
	stLoggerThdAttr.thread_running = -1;
	(void)(*stOsal.mutexdeinit)(&loggerMutex);
	(void)tchdr_bytestream_close(&loggerByteStream);
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrLogReader Thread Sequence 10...\n");
	return pNULL;
}

U32 get_d2a_blend_holdoff(HDR_instance_t *hdr_instance)
{
    U32 blend_transition_frames = 0;

    HDR_blend_get_param(hdr_instance, d2a_blend_holdoff, &blend_transition_frames);
    if ((HDR_get_band_select(hdr_instance) == HDR_BAND_FM) && (blend_transition_frames > MAX_FM_BLEND_TRANSITION_FRAMES)) {
        blend_transition_frames = MAX_FM_BLEND_TRANSITION_FRAMES;
    }
    return blend_transition_frames;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
/*					         Telechips HD Radio Logger Reader Thread							    */
//////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG_ENABLE_TRACE_THREAD
#define LOGGER_PORT         (8889)
#define TRACE_BUFFER_SIZE     (20)
#define HEADER_PATTERN      (0xA6A6A6A6U)

stHDR_BYTE_STREAM_t traceByteStream;
U8 traceRxBuffer[TRACE_BUFFER_SIZE];

void *tchdr_traceReaderThread(void *arg)
{
	U32 messageLength = 0;

	(*stOsal.setPostThreadAttr)(stTraceThdAttr, eTAG_CORE);	// Set thread nice & name

    (*pfnHdrLog)(eTAG_CORE, eLOG_DBG, "Trace Thread %u Started\n",(pid_t)syscall( __NR_gettid));

    traceByteStream.deviceType = eHDR_DEV_ETHER;
    traceByteStream.dev.ether.port = LOGGER_PORT;

    if(tchdr_bytestream_open(&traceByteStream) < 0){
        (*pfnHdrLog)(eTAG_CORE, eLOG_ERR, "Failed to initialize logger byte stream.\n");
        return pNULL;
    }

    stTraceThdAttr.thread_running = 1;
    while(stTraceThdAttr.thread_running){
		U32 header;
		U32 numModules;
        U32 i;

        if(tchdr_bytestream_read(&traceByteStream, traceRxBuffer, 8) < 0){
            continue;
        }

        (void)(*stOsal.osmemcpy)((void*)&header, (void*)traceRxBuffer, 4);

        if(header != HEADER_PATTERN){
            continue;
        }

		numModules = 1;

        for(i = 0; i < numModules; ++i){
			(void)HDR_trace_enable(1);
        }
    }
    stTraceThdAttr.thread_running = -1;
    (void)tchdr_bytestream_close(&traceByteStream);
	return pNULL;
}
#endif

