/*******************************************************************************
*
* (C) copyright 2003-2014, iBiquity Digital Corporation, U.S.A.
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
#include "hdrPsd.h"
#include "hdrAudioResampler.h"
#include "hdrAudio.h"
#include "hdrTest.h"

#include "tchdr_cmdcallbacks.h"
#include "tchdr_bytestream.h"
#include "tchdr_api.h"
#include "tchdr_sis.h"
#include "tchdr_service.h"
#include "tchdr_framework.h"
#include "tchdr_callback.h"

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
static BBP_sys_config_t bbpSysConfig;
static BBP_sys_config_info_t bbpSysConfigInfo;

/* Change: Number of members for BBP_tune_select*/
BBP_tune_select_t stBbpTuneSelect[MAX_NUM_OF_INSTANCES] = {
	{
	    HDR_BAND_FM,
	    0x222E,
	},
	{
	    HDR_BAND_FM,
	    0x222E,
	},
	{
	    HDR_BAND_FM,
	    0x222E,
	},
	{
	    HDR_BAND_FM,
	    0x222E,
	}
};

static BBP_tune_params_t bbpTuneParams = {
    0x222E,		// 8750
    0x2A26,		// 10790
    0x14,		// 20
    0x0212,		// 530
    0x06AE,		// 1710
    0x0A		// 10
};

static BBP_iboc_config_t bbpIbocConfig = {
    0x8000 // dsqm_seek_threshold;
};

/***************************************************
*          Local function prototypes               *
****************************************************/
eTC_HDR_BBSRC_RATE_t CMD_getIqSampleRate(U32 ntuner);
HDR_tune_band_t CMD_cb_get_band_select(HDR_instance_t* hdr_instance);

/***************************************************
*			function definition				*
****************************************************/
/*==================================================
		Callback Function Configuration
====================================================*/
static S32 BBP_checkValidTune(const BBP_tune_select_t* tune_select)
{
	S32 ret;

	if(tune_select->band == HDR_BAND_FM) {
		if((tune_select->rfFreq < 8750U) || (tune_select->rfFreq > 10800U)) {
			ret = -1;
		}
		else {
			ret = 0;
		}
	}
	else if(tune_select->band == HDR_BAND_AM) {
		if((tune_select->rfFreq < 530U) || (tune_select->rfFreq > 1710U)) {
			ret = -1;
		}
		else {
			ret = 0;
		}
	}
	else if(tune_select->band == HDR_BAND_IDLE) {
		ret = 0;
	}
	else {
		ret = -1;
	}

	return ret;
}

void BBP_sys_init_default_config(void)
{
	const U32 numOfHdrInstances = tchdrfwk_getNumOfHdrInstance();

    bbpSysConfig.supported_services.byte0.bit.digital_fm_1 = 1;
    bbpSysConfig.supported_services.byte0.bit.digital_am_1 = 1;
    bbpSysConfig.supported_services.byte3.bit.psd_decode = 1;
    bbpSysConfig.supported_services.byte3.bit.tagging = 0;
    bbpSysConfig.supported_services.byte3.bit.active_radio = 1;
    bbpSysConfig.supported_services.byte3.bit.lot_offchip = 0;
	bbpSysConfig.activated_services.byte0.bit.analog_fm =1;
    bbpSysConfig.activated_services.byte0.bit.digital_am_1=1;
    bbpSysConfig.activated_services.byte0.bit.digital_fm_1=1;
    bbpSysConfig.activated_services.byte2.bit.inst1_cap_0 = 0;
    bbpSysConfig.activated_services.byte2.bit.inst1_cap_1 = 0;
    bbpSysConfig.activated_services.byte3.bit.auto_alignment = 1;
    bbpSysConfig.activated_services.byte3.bit.lot_offchip = 0;
    bbpSysConfig.activated_services.byte3.bit.psd_decode=1;
    bbpSysConfig.activated_services.byte3.bit.tagging=0;
#if NUM_HDR_INSTANCES > 1
	if(numOfHdrInstances > 1U) {
	    // enable 2nd Instance
	    bbpSysConfig.supported_services.byte0.bit.digital_fm_2 = 1;
	    bbpSysConfig.supported_services.byte0.bit.digital_am_2 = 1;
	    bbpSysConfig.activated_services.byte0.bit.digital_fm_2 = 1;
	    bbpSysConfig.activated_services.byte0.bit.digital_am_2 = 1;
		bbpSysConfig.activated_services.byte2.bit.inst1_cap_0 = 0;
    	bbpSysConfig.activated_services.byte2.bit.inst1_cap_1 = 1;
    	bbpSysConfig.activated_services.byte2.bit.inst2_cap_0 = 0;
    	bbpSysConfig.activated_services.byte2.bit.inst2_cap_1 = 0;
	}
#endif
#if NUM_HDR_INSTANCES > 2
	if(numOfHdrInstances > 2U) {
	    // enable 3rd Instance
	    bbpSysConfig.supported_services.byte0.bit.digital_fm_3 = 1;
	    bbpSysConfig.supported_services.byte0.bit.digital_am_3 = 1;
	    bbpSysConfig.activated_services.byte0.bit.digital_fm_3 = 1;
	    bbpSysConfig.activated_services.byte0.bit.digital_am_3 = 1;
	}
#endif
    bbpSysConfig.storage_device = 0x10;         //Flash
    bbpSysConfig.flash_size = 0x01;             //1Mbyte
    bbpSysConfig.bb_src_mode = 0x52;            //AM BB Sample rate=46.5kHz, FM BB Sample Rate=744kHz
    bbpSysConfig.bb_src_mode_alternate = 0x42;  //AM BB Sample rate=55.1kHz, FM BB Sample Rate=744kHz
    bbpSysConfig.bb_src_flags = 0x22;           //AM and FM Software Switching Enabled
    bbpSysConfig.tuner_hw = 0x0303;             //

    //(void)(*stOsal.osmemcpy)((void*)&bbpSysConfig[ACTIVE], (void*)&bbpSysConfig[DEFAULT], (U32)sizeof(BBP_sys_config_t));
    UNUSED(numOfHdrInstances);
}

HDBOOL CMD_cb_bbp_busy(HDR_instance_t* hdr_instance)
{
	HDBOOL ret;
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
	if(hdr_instance->instance_number >= NUM_HDR_INSTANCES) {
        (*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "Invalid instance number for Get Tune Select \n");
        ret = false;
    }
	else {
    	const stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();
		ret = frameworkData->busyFlag[hdr_instance->instance_number];
	}
    return ret;

}

S32 CMD_cb_bbp_get_tune_select(HDR_instance_t* hdr_instance, BBP_tune_select_t* tune_select)
{
	// API to Get Tuner frequency
	S32 ret = 0;
#ifdef DEBUG_ENABLE_FREQUENT_CB_LOG
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
#endif
    if(hdr_instance->instance_number >= NUM_HDR_INSTANCES) {
        (*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "Invalid instance number for Get Tune Select \n");
        ret = -1;
    }
#if 1
	if(ret == 0) {
		(void)(*stOsal.osmemcpy)((void*)tune_select, (void*)&stBbpTuneSelect[hdr_instance->instance_number], (U32)sizeof(BBP_tune_select_t));
	}
#else
	if(hdr_instance->instance_number == 0U) {
		tune_select->band = stTcHdrTuneInfo.mainInstance.band;
		tune_select->rfFreq = stTcHdrTuneInfo.mainInstance.freq;
	}
	else if(hdr_instance->instance_number == 1U) {
		if(tchdrfwk_getHdrType() == HDR_1p5_CONFIG) {
			tune_select->band = stTcHdrTuneInfo.bsInstance.band;
			tune_select->rfFreq = stTcHdrTuneInfo.bsInstance.freq;
		}
		else {
			tune_select->band = stTcHdrTuneInfo.mrcInstance.band;
			tune_select->rfFreq = stTcHdrTuneInfo.mrcInstance.freq;
		}
	}
	else if(hdr_instance->instance_number == 2U) {
		tune_select->band = stTcHdrTuneInfo.bsInstance.band;
		tune_select->rfFreq = stTcHdrTuneInfo.bsInstance.freq;
    }
#ifndef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
	else if(hdr_instance->instance_number == 3U) {	// When you use 4 tuners, uncomment and use this if statement.
		tune_select->band = HDR_BAND_IDLE;
		tune_select->rfFreq = 0;
	}
#endif
	else {
		tune_select->band = HDR_BAND_IDLE;
		tune_select->rfFreq = 0;
		ret = -1;
	}

	if(ret == 0) {
		(void)(*stOsal.osmemcpy)((void*)&stBbpTuneSelect[hdr_instance->instance_number], (void*)tune_select, (U32)sizeof(BBP_tune_select_t));
	}
#endif

    return ret;
}

eTC_HDR_BBSRC_RATE_t CMD_getIqSampleRate(U32 ntuner)
{
	S32 srRet = tchdr_cb_getIqSampleRate(ntuner);

    return (eTC_HDR_BBSRC_RATE_t)srRet;
}

S32 CMD_cb_bbp_set_tune_select(HDR_instance_t* hdr_instance, const BBP_tune_select_t* tune_select)
{
	S32 ret;
    stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();

	//frameworkData->busyFlag[hdr_instance->instance_number] = true;
    (*pfnHdrLog)(eTAG_CDM, eLOG_INF, "Instance number[%d]: Set to Band[%d] and Freq[%d].\n",hdr_instance->instance_number, (U32)tune_select->band, tune_select->rfFreq);

	// To prevent errors when the band of Tune Status and band of AM/FM button are different on the CDM4 application.
	ret = BBP_checkValidTune(tune_select);
	if(ret < 0) {
		(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "Failed to tune with the wrong parameter of CDM.\n");
		ret = -1;
	}
	else {
    // Here one should call the tuner control functions which set the tuner frequency and band
#if 0 // XPERI sample code
	 #if HDR_CONFIG == HDR_MRC_CONFIG || HDR_CONFIG == HDR_1_5_PLUS_MRC_CONFIG
		HDR_instance_t* mrcSlaveInstance;
		U32 slaveInstanceNumber;
		 for (slaveInstanceNumber=1;slaveInstanceNumber<4;slaveInstanceNumber++) {
			mrcSlaveInstance = CMD_cb_bbp_get_hdr_instance(slaveInstanceNumber);
			if (mrcSlaveInstance==NULL) continue;
			if (mrcSlaveInstance->instance_type == HDR_DEMOD_ONLY_INSTANCE)  break;
		}
		memcpy(&bbpTuneSelect[slaveInstanceNumber-1], tune_select, (U32)sizeof(BBP_tune_select_t));

	 #endif

		if(tune_select->rfFreq != bbpTuneSelect[hdr_instance->instance_number].rfFreq){
			if(bbpTuneSelect[hdr_instance->instance_number].band != tune_select->band){
				HDR_set_band(hdr_instance, tune_select->band);
			}else{
				HDR_reacquire(hdr_instance);
			}
		}
		memcpy(&bbpTuneSelect[hdr_instance->instance_number], tune_select, (U32)sizeof(BBP_tune_select_t));
		ret = 0;
#else
		eTC_HDR_ID_t apiHdrID = eTC_HDR_ID_MAIN;
		stTC_HDR_TUNE_TO_t apiTuneTo;
		if(tune_select->band == HDR_BAND_FM) {
			apiTuneTo.band = eTC_HDR_FM_BAND;
			apiTuneTo.freq = (U32)tune_select->rfFreq*(U32)10;
		}
		else if(tune_select->band == HDR_BAND_AM) {
			apiTuneTo.band = eTC_HDR_AM_BAND;
			apiTuneTo.freq = (U32)tune_select->rfFreq;
		}
		else {
			apiTuneTo.band = eTC_HDR_IDLE_BAND;
			apiTuneTo.freq = (U32)0;
		}
		apiTuneTo.iqsamplerate = eTC_HDR_BBSRC_744_KHZ;	// set default value

		ret = tchdr_getHdrIdFromInstanceNumber(&apiHdrID, hdr_instance->instance_number);
		if(ret == 0) {
			if(apiTuneTo.band != eTC_HDR_IDLE_BAND) {
				ret = tchdr_cb_setTune(apiTuneTo.band, apiTuneTo.freq, hdr_instance->instance_number);
			}
			// The below conditions were separated for when the IDLE band is set.
			if(ret == 0) {
				U32 hdrType = tchdrfwk_getHdrType();
				eTC_HDR_BBSRC_RATE_t retRate = CMD_getIqSampleRate(hdr_instance->instance_number);
				if(retRate < eTC_HDR_BBSRC_UNKNOWN) {
					apiTuneTo.iqsamplerate = retRate;
					ret = tchdr_setTune(apiHdrID, apiTuneTo);
				}
				// for MRC
				if((hdrType == HDR_1p0_MRC_CONFIG) || (hdrType == HDR_1p5_MRC_CONFIG) || (hdrType == HDR_1p5_DUAL_MRC_CONFIG)) {
					// When MRC is enabled, the 2nd tuner also had to tune to the same frequency as the main tuner.
					if(ret == 0) {
						if(apiHdrID == eTC_HDR_ID_MAIN) {
							HDBOOL bMrcSts = tchdrfwk_getMrcStatus();
							if(bMrcSts == true) {
								if(apiTuneTo.band != eTC_HDR_IDLE_BAND) {
									ret = tchdr_cb_setTune(apiTuneTo.band, apiTuneTo.freq, 1U);
								}
								// The below conditions were separated for when the IDLE band is set.
								if(ret == 0) {
									retRate = CMD_getIqSampleRate(1U);
									if(retRate < eTC_HDR_BBSRC_UNKNOWN) {
										apiTuneTo.iqsamplerate = retRate;
										ret = tchdr_setTune(eTC_HDR_ID_MRC, apiTuneTo);
									}
								}
							}
						}
						if((hdrType == HDR_1p5_DUAL_MRC_CONFIG) && (apiHdrID == eTC_HDR_ID_BS)) {
							HDBOOL bBsMrcSts = tchdrfwk_getBsMrcStatus();
							if(bBsMrcSts == true) {
								if(apiTuneTo.band != eTC_HDR_IDLE_BAND) {
									ret = tchdr_cb_setTune(apiTuneTo.band, apiTuneTo.freq, 3U);
								}
								// The below conditions were separated for when the IDLE band is set.
								if(ret == 0) {
									retRate = CMD_getIqSampleRate(3U);
									if(retRate < eTC_HDR_BBSRC_UNKNOWN) {
										apiTuneTo.iqsamplerate = retRate;
										ret = tchdr_setTune(eTC_HDR_ID_BS_MRC, apiTuneTo);
									}
								}
							}
						}
					}
				}
			}
		}

		if(ret == 0) {
#ifdef USE_HDRLIB_2ND_CHG_VER
		    if(hdr_instance->instance_type==HDR_FULL_INSTANCE){
		        S32 blend_transition_frames = get_d2a_blend_holdoff(hdr_instance);
		        HDR_set_blend_transition_time(frameworkData->blendCrossfade, blend_transition_frames);
		        HDR_blend_crossfade_reset (frameworkData->blendCrossfade);
		        HDR_audio_resampler_reset (frameworkData->hdaoutResampler);
		        frameworkData->digitalAudioStarted = false;
		    }
#else
			frameworkData->digitalAudioStarted = false;
#endif
		}
#endif
	}

    return ret;
}

S32 CMD_cb_bbp_play_alert_tone(HDR_instance_t* hdr_instance)
{
    stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();

	if(hdr_instance != NULL) {
		(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
		frameworkData->playAlertTone = true;
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] HDR Instance argument is null pointer.\n", __func__);
	}

    return 0;
}

S32 CMD_cb_bbp_set_playing_program(HDR_instance_t* hdr_instance, HDR_program_t program)
{
	S32 rc = 0;

	if(hdr_instance != NULL) {
		if(hdr_instance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
			(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d], Program[%d]\n", __func__, __LINE__, hdr_instance->instance_number, program);
		    rc = HDR_set_playing_program(hdr_instance, program);
			(void)tchdrsvc_setProgramNumber(hdr_instance, program);
			(*pfnHdrLog)(eTAG_CDM, eLOG_INF, "Set to the program[%d] of the instance[%d].\n", (U32)program, hdr_instance->instance_number);
		}
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] HDR Instance argument is null pointer.\n", __func__);
		rc = -1;
	}

    return rc;
}

S32 CMD_cb_bbp_get_tune_params(BBP_tune_params_t* tune_params)
{
	S32 ret = 0;

	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
	if ((bbpTuneParams.fmFreqSpacing != 20U) || (bbpTuneParams.fmMinTuningFreq < 8700U) || (bbpTuneParams.fmMaxTuningFreq > 10800U) ||
		(bbpTuneParams.amFreqSpacing != 10U) || (bbpTuneParams.amMinTuningFreq < 530U) || (bbpTuneParams.amMaxTuningFreq > 1710U)) {
		ret = -1;
	}
	else {
		if(tune_params != NULL) {
			(void)(*stOsal.osmemcpy)((void*)tune_params, (void*)&bbpTuneParams, (U32)sizeof(BBP_tune_params_t));
		} else {
			(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] Tune parameter is null pointer.\n", __func__);
		}
	}

    return ret;
}

S32 CMD_cb_bbp_set_tune_params(const BBP_tune_params_t* tune_params)
{
	S32 ret = 0;

	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
	if(tune_params != NULL) {
		if ((tune_params->fmFreqSpacing != 20U) || (tune_params->fmMinTuningFreq < 8700U) || (tune_params->fmMaxTuningFreq > 10800U) ||
			(tune_params->amFreqSpacing != 10U) || (tune_params->amMinTuningFreq < 530U) || (tune_params->amMaxTuningFreq > 1710U)) {
			ret = -1;
		}
		else {
			(void)(*stOsal.osmemcpy)(&bbpTuneParams, tune_params, (U32)sizeof(BBP_tune_params_t));
		}
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] Tune parameter is null pointer.\n", __func__);
	}

    return ret;
}

S32 CMD_cb_bbp_get_iboc_config(BBP_iboc_config_t* iboc_config, BBP_config_select_t config_select)
{
	UNUSED(config_select);
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
	if(iboc_config != NULL) {
    	(void)(*stOsal.osmemcpy)((void*)iboc_config, (void*)&bbpIbocConfig, (U32)sizeof(BBP_iboc_config_t));
	}

    return 0;
}

S32 CMD_cb_bbp_set_iboc_config(const BBP_iboc_config_t* iboc_config)
{
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
    (void)(*stOsal.osmemcpy)(&bbpIbocConfig, iboc_config, (U32)sizeof(BBP_iboc_config_t));

    return 0;
}

S32 CMD_cb_bbp_reset_iboc_config(void)
{
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
    return 0;
}

S32 CMD_cb_bbp_get_activated_services(HDR_instance_t* hdr_instance, BBP_services_t* bbp_services)
{
	if((hdr_instance != NULL) && (bbp_services != NULL)) {
		(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
    	(void)(*stOsal.osmemcpy)((void*)bbp_services, (void*)&bbpSysConfig.activated_services, (U32)sizeof(BBP_services_t));
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] Arguments are null\n", __func__);
	}

    return 0;
}

S32 CMD_cb_bbp_set_activated_services(HDR_instance_t* hdr_instance, const BBP_services_t* bbp_services)
{
	if((hdr_instance != NULL) && (bbp_services != NULL)) {
		(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
    	(void)(*stOsal.osmemcpy)(&bbpSysConfig.activated_services, bbp_services, (U32)sizeof(BBP_services_t));
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] Arguments are null\n", __func__);
	}

    return 0;
}

S32 CMD_cb_bbp_get_supported_services(BBP_services_t* bbp_services)
{
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
    (void)(*stOsal.osmemcpy)((void*)bbp_services, (void*)&bbpSysConfig.supported_services, (U32)sizeof(BBP_services_t));

    return 0;
}

S32 CMD_cb_bbp_get_sys_config_info(BBP_sys_config_info_t* sys_config_info, BBP_config_select_t config_select)
{
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
    UNUSED(config_select);
	if(sys_config_info != NULL) {
    	(void)(*stOsal.osmemcpy)((void*)sys_config_info, (void*)&bbpSysConfigInfo, (U32)sizeof(BBP_sys_config_info_t));
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] Argument is null\n", __func__);
	}

    return 0;
}

S32 CMD_cb_bbp_get_sys_config(BBP_sys_config_t* sys_config, BBP_config_select_t config_select)
{
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
    UNUSED(config_select);
	if(sys_config != NULL) {
    	(void)(*stOsal.osmemcpy)((void*)sys_config, (void*)&bbpSysConfig, (U32)sizeof(BBP_sys_config_t));
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] Argument is null\n", __func__);
	}

    return 0;
}

S32 CMD_cb_bbp_set_sys_config(const BBP_sys_config_t* sys_config)
{
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
    (void)(*stOsal.osmemcpy)(&bbpSysConfig, sys_config, (U32)sizeof(BBP_sys_config_t));

    return 0;
}

S32 CMD_cb_bbp_reset_sys_config(void)
{
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
    return 0;
}

S32 CMD_cb_bbp_get_sys_config_param(BBP_sys_config_param_t param, U32* value)
{
	S32 rc = 0;
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
    switch(param){
        case SPDIF_ENABLE:
            *value = bbpSysConfig.i2s_aes_spdif_on;
            break;
        case AWSx256_ENABLE:
            *value = bbpSysConfig.i2s_awsx256_on;
            break;
        case I2S_MSTRSLV_CNTRL:
        {
            *value = (U32)bbpSysConfig.i2s_input_mode << 24;
            *value |= (U32)bbpSysConfig.i2s_output_mode << 16;
            *value |= (U32)bbpSysConfig.i2s_input_dst << 8;
            *value |= (U32)bbpSysConfig.i2s_output_src;
            break;
        }
        case UART1_CLK_RATE:
            *value = (U32)bbpSysConfig.uart_1_rate;
            break;
        case UART2_CLK_RATE:
            *value = (U32)bbpSysConfig.uart_2_rate;
            break;
        default:
            rc = -1;
            break;
    }

    return rc;
}

S32 CMD_cb_bbp_set_sys_config_param(BBP_sys_config_param_t param, U32 value)
{
	S32 rc = 0;
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
    switch(param){
        case SPDIF_ENABLE:
            bbpSysConfig.i2s_aes_spdif_on = (U8)(value & 0xffU);
            break;
        case AWSx256_ENABLE:
            bbpSysConfig.i2s_awsx256_on = (U8)(value & 0xffU);
            break;
        case I2S_MSTRSLV_CNTRL:
            bbpSysConfig.i2s_input_mode = (U8)((value >> 24U) & 0xffU);
            bbpSysConfig.i2s_output_mode = (U8)((value >> 16U) & 0xffU);
            bbpSysConfig.i2s_input_dst = (U8)((value >> 8U) & 0xffU);
            bbpSysConfig.i2s_output_src = (U8)(value & 0xffU);
            break;
        case UART1_CLK_RATE:
            bbpSysConfig.uart_1_rate = (U8)(value & 0xffU);
            break;
        case UART2_CLK_RATE:
            bbpSysConfig.uart_2_rate = (U8)(value & 0xffU);
            break;
        default:
            rc = -1;
            break;
    }

    return rc;
}

S32 CMD_cb_bbp_reset_dsqm_time_const(void)
{
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
    return 0;
}

S32 CMD_cb_bbp_set_dsqm_seek_thresh(U32 thresh)
{
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
    bbpIbocConfig.dsqm_seek_threshold = thresh;

    return 0;
}

S32 CMD_cb_bbp_get_dsqm_seek_thresh(U32* thresh)
{
#ifdef DEBUG_ENABLE_FREQUENT_CB_LOG
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
#endif
    *thresh = bbpIbocConfig.dsqm_seek_threshold;

    return 0;
}

S32 CMD_cb_bbp_reset_dsqm_seek_thresh(void)
{
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]\n", __func__, __LINE__);
    return 0;
}

HDR_tune_band_t CMD_cb_get_band_select(HDR_instance_t* hdr_instance)
{
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
    return HDR_get_band_select(hdr_instance);
}

S32 CMD_cb_bbp_enable_audio_split_mode(HDR_instance_t *hdr_instance)
{
    stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();

	if(hdr_instance != NULL) {
		(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
		frameworkData->audioMode = eHDR_AUDIO_ANALOG_SPLIT;
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] HDR Instance argument is null pointer.\n", __func__);
	}

    return 0;
}

S32 CMD_cb_bbp_disable_audio_split_mode(HDR_instance_t *hdr_instance)
{
    stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();

	if(hdr_instance != NULL) {
		(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
		frameworkData->audioMode = eHDR_AUDIO_BLEND;
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] HDR Instance argument is null pointer.\n", __func__);
	}

    return 0;
}

S32 CMD_cb_digital_audio_acquired(HDR_instance_t* hdr_instance, HDBOOL* audio_acquired)
{
    const stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();

	if(hdr_instance->instance_type == HDR_FULL_INSTANCE) {
#ifdef DEBUG_ENABLE_FREQUENT_CB_LOG
		(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d], Digital Audio Acquired[%d]\n", __func__, __LINE__, hdr_instance->instance_number, *audio_acquired);
#endif
		*audio_acquired = frameworkData->digitalAudioAcquired;
	}

    return 0;
}

S32 CMD_cb_set_auto_alignment_config(HDR_instance_t* hdr_instance, const CMD_auto_alignment_config_t* config)
{
	S32 rc;
    const stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();
    HDR_auto_align_config_t configParams;

	if(hdr_instance != NULL) {
		(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
    	configParams.am_auto_time_align_enabled = config->am_auto_time_align_enabled;
    	configParams.fm_auto_time_align_enabled = config->fm_auto_time_align_enabled;
    	configParams.am_auto_level_align_enabled = config->am_auto_level_align_enabled;
    	configParams.fm_auto_level_align_enabled = config->fm_auto_level_align_enabled;
		rc = HDR_auto_align_set_config(frameworkData->autoAlign, &configParams);
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] HDR Instance argument is null pointer.\n", __func__);
		rc = -1;
	}

    return rc;
}

S32 CMD_cb_get_auto_alignment_config(HDR_instance_t* hdr_instance, CMD_auto_alignment_config_t* config)
{
	S32 rc;
    const stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();
    HDR_auto_align_config_t config_params;

	if(hdr_instance != NULL) {
		rc = HDR_auto_align_get_config(frameworkData->autoAlign, &config_params);
		if(rc < 0){
	        rc = -1;
	    } else {
		    config->am_auto_time_align_enabled = config_params.am_auto_time_align_enabled;
		    config->fm_auto_time_align_enabled = config_params.fm_auto_time_align_enabled;
		    config->am_auto_level_align_enabled = config_params.am_auto_level_align_enabled;
		    config->fm_auto_level_align_enabled = config_params.fm_auto_level_align_enabled;
	    }
#ifdef DEBUG_ENABLE_FREQUENT_CB_LOG
		(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]return[%d], Instance Number[%d]\n", __func__, __LINE__, rc, hdr_instance->instance_number);
#endif
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] HDR Instance argument is null pointer.\n", __func__);
		rc = -1;
	}

    return rc;
}

S32 CMD_cb_get_auto_alignment_spec(HDR_instance_t* hdr_instance, CB_auto_alignment_spec_t* spec)
{
	S32 rc;
    U32 value = 0;

	if(hdr_instance != NULL) {
		(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
	    (void)HDR_blend_get_param(hdr_instance, am_mps_dig_audio_delay, &value);
	    spec->amMaxPosRange = HDR_AUTO_ALIGN_MAX_RANGE;
	    spec->amMaxNegRange = (HDR_AUTO_ALIGN_MAX_RANGE < value) ? HDR_AUTO_ALIGN_MAX_RANGE : value;
	    spec->fmMaxPosRange = HDR_AUTO_ALIGN_MAX_RANGE;
	    (void)HDR_blend_get_param(hdr_instance, fm_mps_dig_audio_delay, &value);
	    spec->fmMaxNegRange = (HDR_AUTO_ALIGN_MAX_RANGE < value) ? HDR_AUTO_ALIGN_MAX_RANGE : value;
		rc = 0;
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] HDR Instance argument is null pointer.\n", __func__);
		rc = -1;
	}

    return rc;
}

S32 CMD_cb_get_auto_alignment_status(HDR_instance_t* hdr_instance, CMD_auto_alignment_status_t* status)
{
	S32 rc;
    const stHDR_FRAMEWORK_DATA_t* frameworkData  = tchdrfwk_getDataStructPtr();
    HDR_auto_align_status_t autoAlignStatus;

	if(hdr_instance != NULL) {
		U32 tmpOffset;
		(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
		if(HDR_auto_align_get_status(frameworkData->autoAlign, &autoAlignStatus) < 0){
	        rc = -1;
	    }
		else {
		    status->alignmentFound = autoAlignStatus.alignment_found;
		    status->phaseInverted = autoAlignStatus.phase_inverted;
		    status->alignmentOffset = autoAlignStatus.time_offset;
		    tmpOffset = (U32)autoAlignStatus.level_offset>>8;
		    status->levelOffset = (S32)(tmpOffset);
		    status->confidenceLevel = autoAlignStatus.confidence_level;
		    status->digAudioAvailable = false;

		    HDR_program_t program = HDR_PROGRAM_HD1;
		    (void)HDR_get_playing_program(hdr_instance, &program);

		    if(program == HDR_PROGRAM_HD1){
		        HDR_audio_quality_report_t qualityReport;
		        (void)(*stOsal.osmemset)((void*)&qualityReport, (S8)0, (U32)sizeof(HDR_audio_quality_report_t));

		        (void)HDR_get_audio_quality_report(hdr_instance, &qualityReport);

		        if(qualityReport.quality_indicator > 9U){
		            status->digAudioAvailable = true;
		        }
		    }
			rc = 0;
		}
	}
	else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] HDR Instance argument is null pointer.\n", __func__);
		rc = -1;
	}

    return rc;
}

S32 CMD_cb_get_auto_alignment_confidence_level(HDR_instance_t* hdr_instance, HDR_tune_band_t band, U32* level)
{
	if((hdr_instance != NULL) && (level != NULL)) {
		(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] The argument is null pointer.\n", __func__);
	}
	UNUSED(band);
	UNUSED(level);
    return 0;
}

S32 CMD_cb_set_auto_alignment_confidence_level(HDR_instance_t* hdr_instance, HDR_tune_band_t band, U32 level)
{
	if(hdr_instance != NULL) {
		(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d] Instance Number[%d]\n", __func__, __LINE__, hdr_instance->instance_number);
	} else {
		(*pfnHdrLog)(eTAG_CB, eLOG_ERR, "[%s] HDR Instance argument is null pointer.\n", __func__);
	}
	UNUSED(band);
	UNUSED(level);
    return 0;
}

S32 CMD_cb_write_protect_error(const S8* parameter_name)
{
	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]: Paramter can be set only in idle mode\n",  __func__, __LINE__);
	if(parameter_name != NULL) {
    	(*pfnHdrLog)(eTAG_CB, eLOG_DBG, "[%s:%d]: Paramter string is [%s]\n",  __func__, __LINE__, parameter_name);
	}
	return 0;
}

