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
/**
 * file: cmdTune.c
 * brief:
 */
#include <string.h>
#include "tchdr_common.h"			// To avoid redundant condition warnings of codesonar
#include "tchdr_log.h"
#include "hdrSis.h"
#include "hdrAudio.h"
#include "hdrBlend.h"
#include "hdrConfig.h"
#include "hdrPsd.h"
#include "hdrPhy.h"
#include "hdrTest.h"
#include "tchdr_cmdtune.h"
#include "tchdr_cmdcallbacks.h"
#include "tchdr_cmdopcode.h"

enum{
    CMD_TUNE_INVALID = 0,
    CMD_TUNE_VALID = 1
};

enum SYS_TUNE_FUNC_CODES
{
    //RESERVED  0x0
    TUNE_SELECT            = 0x1,
    TUNE_STEP              = 0x2,
    //RESERVED  0x3-0x5
    TUNE_GET_STATUS        = 0x6,
    //RESERVED  0x7-0x9
    TUNE_SET_PARAMS        = 0xA,
    TUNE_GET_PARAMS        = 0xB,
    GET_DSQM_STATUS        = 0xC,
    GET_ALIGN_STATUS       = 0xD
};

/**
 * To stop a current seek in progress, the HC can issue ANY Sys_Tune command.
 */
S32 SYS_procTune(HDR_instance_t* hdrInstance, const U8* dataIn, U32 inLength, U8* dataOut, U32* outLength)
{
	S32 rc1;
	S32 rc2;

	if(dataIn == NULL) {
		(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "SYS_procTune input data is null.\n");
		return -1;
    }
    U32 funcCode = (U32)dataIn[0]; //BYTE 0 specifies the function code

    if(inLength <= (U32)0){
        *outLength = 0;
        return -1;
    }

    switch(funcCode){
        //TODO: Cancel any Seek or Scan in Progress - see SYS_TuneSeek()
        case (U32)TUNE_SELECT:
        {
            LOG(CMD,1U, "Received SysTune->TUNE_SELECT");
            HDR_tune_band_t band = (HDR_tune_band_t)dataIn[1];
            U32 frequency = dataIn[2] + ((U32)dataIn[3] << 8);
            HDR_program_t program = (HDR_program_t)dataIn[4];

			BBP_tune_select_t tuneSelect;
            BBP_tune_params_t tuneParams;

            dataOut[0] = (U8)TUNE_SELECT;
            dataOut[1] = CMD_TUNE_VALID;
            *outLength = 2;

			rc1 = CMD_cb_bbp_get_tune_select(hdrInstance, &tuneSelect);
			rc2 = CMD_cb_bbp_get_tune_params(&tuneParams);
            if((rc1 != 0) || (rc2 != 0)){
                dataOut[1] = CMD_TUNE_INVALID;
                break;
            }

            // Verify program # in range
            if ((U8)program >= HDR_MAX_NUM_PROGRAMS){
                dataOut[1] = CMD_TUNE_INVALID;
                break;
            }

            // No frequency change; set the playing program only
            if((band == tuneSelect.band) && (frequency == tuneSelect.rfFreq)){
                (void)CMD_cb_bbp_set_playing_program(hdrInstance, program);
                break;
            }

            tuneSelect.band = band;

            // If frequency is 0, we're reacquiring; so use current frequency and band
            if(frequency != (U32)0){
                // Verify frequency in range
                switch (band){
                    case HDR_BAND_FM:
                        if ((frequency > tuneParams.fmMaxTuningFreq) || (frequency < tuneParams.fmMinTuningFreq)){
                            LOG(CMD,1U, "Frequency %d out of FM Band range ",frequency);
                            dataOut[1] = CMD_TUNE_INVALID;
                        }
                        break;
                    case HDR_BAND_AM:
                        if ((frequency > tuneParams.amMaxTuningFreq) || (frequency < tuneParams.amMinTuningFreq)){
                            LOG(CMD,1U, "Frequency %d out of AM Band range ",frequency);
                            dataOut[1] = CMD_TUNE_INVALID;
                        }
                        break;
                    default:
                        LOG(CMD,1U, "Invalid RF band encountered.");
                        dataOut[1] = CMD_TUNE_INVALID;
						break;
                }

                if(dataOut[1] == (U8)CMD_TUNE_INVALID){
                    break;
                }

                tuneSelect.rfFreq = (U16)frequency;
            }

			rc1 = CMD_cb_bbp_set_tune_select(hdrInstance, &tuneSelect);
			rc2 = CMD_cb_bbp_set_playing_program(hdrInstance, program);
            if((rc1 < 0) || (rc2 < 0)){
                LOG(CMD,1U, "Failed to tune.");
                dataOut[1] = CMD_TUNE_INVALID;
                break;
            }
            break;
        }
        case (U32)TUNE_STEP:
        {
            LOG(CMD,1U, "Received SysTune->TUNE_STEP");

            BBP_tune_select_t tuneSelect;
            BBP_tune_params_t tuneParams;

			rc1 = CMD_cb_bbp_get_tune_select(hdrInstance, &tuneSelect);
			rc2 = CMD_cb_bbp_get_tune_params(&tuneParams);
            if((rc1 != 0) || (rc2 != 0)){
                dataOut[1] = CMD_TUNE_INVALID;
                break;
            }

            HDR_tune_direction_t direction = (HDR_tune_direction_t)dataIn[2];
            HDR_tune_multicast_t multicast = (HDR_tune_multicast_t)dataIn[3];

            dataOut[0] = (U8)TUNE_STEP;
            *outLength = 1;

            if((multicast == TUNE_MULTICAST_USE_SPS) && (hdrInstance->instance_type == HDR_FULL_INSTANCE)){
                HDR_program_bitmap_t availablePrograms;
                availablePrograms.all = 0;

                (void)HDR_get_available_programs(hdrInstance, &availablePrograms);
                HDR_program_t playingProgram = HDR_PROGRAM_HD1;
                (void)HDR_get_playing_program(hdrInstance, &playingProgram);

                HDBOOL multicastStep = false;
                U8 i;

                if(direction == TUNE_UP){
                    for(i = (U8)playingProgram + 1U; i < (U8)HDR_MAX_NUM_PROGRAMS; ++i){
                        if((availablePrograms.all & ((U8)1 << i)) > 0U){
                            (void)CMD_cb_bbp_set_playing_program(hdrInstance, (HDR_program_t)i);
                            multicastStep = true;
                            break;
                        }
                    }
                } else { // TUNE_DOWN
                    for(i = (U8)playingProgram - 1U; i < (U8)HDR_MAX_NUM_PROGRAMS; --i){
                       if((availablePrograms.all & ((U8)1 << i)) > 0U){
                           (void)CMD_cb_bbp_set_playing_program(hdrInstance, (HDR_program_t)i);
                           multicastStep = true;
                            break;
                        }
                    }
                }

                if(multicastStep == true){
                    // Multicast step; no frequency change necessasry
                    break;
                }
            }

            if(direction == TUNE_UP){
                // Freqeuncy step; must retune.
                if (tuneSelect.band == HDR_BAND_FM){
                    if(tuneSelect.rfFreq == tuneParams.fmMaxTuningFreq){
                        tuneSelect.rfFreq = (U16)(tuneParams.fmMaxTuningFreq & 0xffffU);
                    } else {
                        tuneSelect.rfFreq += (U16)(tuneParams.fmFreqSpacing & 0xffffU);
                    }
                } else {
                	if (tuneSelect.band == HDR_BAND_AM) {
	                    if(tuneSelect.rfFreq == tuneParams.amMaxTuningFreq){
	                        tuneSelect.rfFreq = (U16)(tuneParams.amMaxTuningFreq & 0xffffU);
	                    } else {
	                        tuneSelect.rfFreq += (U16)(tuneParams.amFreqSpacing & 0xffffU);
	                    }
                	}
                }
            } else { // TUNE_DOWN
                if (tuneSelect.band == HDR_BAND_FM){
                    if (tuneSelect.rfFreq == tuneParams.fmMinTuningFreq){
                        tuneSelect.rfFreq = (U16)(tuneParams.fmMinTuningFreq & 0xffffU);
                    } else {
                        tuneSelect.rfFreq -= (U16)(tuneParams.fmFreqSpacing & 0xffffU);
                    }
                } else {
                	if (tuneSelect.band == HDR_BAND_AM) {
	                    if (tuneSelect.rfFreq == tuneParams.amMinTuningFreq){
	                        tuneSelect.rfFreq = (U16)(tuneParams.amMinTuningFreq & 0xffffU);
	                    } else {
	                        tuneSelect.rfFreq -= (U16)(tuneParams.amFreqSpacing & 0xffffU);
	                    }
                	}
                }
            }

			rc1 = CMD_cb_bbp_set_tune_select(hdrInstance, &tuneSelect);
			rc2 = CMD_cb_bbp_set_playing_program(hdrInstance, HDR_PROGRAM_HD1);
            if((rc1 < 0) || (rc2 < 0)){
                LOG(CMD,1U, "Failed to tune.");
                dataOut[1] = CMD_TUNE_INVALID;
                break;
            }

            break;
        }
        case (U32)TUNE_GET_STATUS:
        {
            HDR_tune_method_t  tuneMethod = (HDR_tune_method_t)dataIn[1];
            U32 offset = 0;
            dataOut[offset] = (U8)TUNE_GET_STATUS;
			offset++;
            dataOut[offset] = (U8)tuneMethod;
			offset++;

            BBP_tune_select_t tuneSelect;

            CMD_auto_alignment_config_t auto_alignment_config;
			auto_alignment_config.am_auto_time_align_enabled = false;
			auto_alignment_config.fm_auto_time_align_enabled = false;
			auto_alignment_config.am_auto_level_align_enabled = false;
			auto_alignment_config.fm_auto_level_align_enabled = false;
#ifdef USE_HDRLIB_3RD_CHG_VER
			auto_alignment_config.am_auto_level_correction_enabled = false;
			auto_alignment_config.fm_auto_level_correction_enabled = false;
#else
			auto_alignment_config.apply_level_adjustment = false;
#endif
            (void)CMD_cb_get_auto_alignment_config(hdrInstance,&auto_alignment_config);


            if(CMD_cb_bbp_get_tune_select(hdrInstance, &tuneSelect) != 0){
                break;
            }

            HDR_program_bitmap_t availablePrograms;
            availablePrograms.all = 0;
            if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                (void)HDR_get_available_programs(hdrInstance, &availablePrograms);
            }

            HDR_program_t program = HDR_PROGRAM_HD1;
            if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
                (void)HDR_get_playing_program(hdrInstance, &program);
            }

            HDR_audio_codec_mode_t codecMode = HDR_AUDIO_CODEC_MODE0;
            if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                (void)HDR_get_codec_mode(hdrInstance, &codecMode);
            }

            U8 acqStatus = (*stCast.booltou8)(HDR_hd_signal_acquired(hdrInstance));
            if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                acqStatus |= (*stCast.booltou8)(HDR_sis_acquired(hdrInstance)) << 1;
            }

            HDBOOL digAcq = false;
            (void)CMD_cb_digital_audio_acquired(hdrInstance, &digAcq);

            acqStatus |= (*stCast.booltou8)(digAcq) << 2U;

            dataOut[offset] = (U8)tuneSelect.band;
            offset++;
            if(tuneSelect.band == HDR_BAND_IDLE){
                dataOut[offset] = 0U;
                offset++;
                dataOut[offset] = 0U;
                offset++;
                *outLength = offset;
                break;
            }

            dataOut[offset] = (U8)(tuneSelect.rfFreq & 0xffU);
			offset++;
            dataOut[offset] = (U8)((tuneSelect.rfFreq >> 8U) & 0xffU);
			offset++;

            switch (tuneMethod){
                case TUNE_ALL:
                {
                    LOG(CMD,2U, "Received SysTune->TUNE_GET_STATUS->TUNE_ALL");
                    U32 value = 0;

                    offset += (U32)4; // Analog seek strength
                    dataOut[offset] = 0; // Mono/Stereo
					offset++;
                    dataOut[offset] = 0; // Seek and Scan
					offset++;
                    dataOut[offset] = 0; // RBDS Change
					offset++;
                    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)6); // Bytes 12 - 17: Reserved
                    offset += (U32)6;
                    dataOut[offset] = (U8)program;
					offset++;
                    dataOut[offset] = acqStatus;
					offset++;

                    HDR_audio_quality_report_t audioQualityReport;
                    (void)(*stOsal.osmemset)((void*)&audioQualityReport, (S8)0, (U32)sizeof(HDR_audio_quality_report_t));

                    if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
                        (void)HDR_get_audio_quality_report(hdrInstance, &audioQualityReport);
                    }

                    dataOut[offset] = (U8)(audioQualityReport.quality_indicator & 0xffU);
					offset++;
                    if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                        (void)HDR_get_cdno(hdrInstance, &value);
                    }else{
                        value=(U32)0;
                    }
                    dataOut[offset] = (U8)(value & 0xffU);
					offset++;
                    if (((tuneSelect.band == HDR_BAND_FM) && (auto_alignment_config.fm_auto_level_align_enabled==true))||
                        ((tuneSelect.band == HDR_BAND_AM) && (auto_alignment_config.am_auto_level_align_enabled==true))){
                        // If auto level alignment algorithm is enabled, update TX_GAIN value with the result, unless
                        // on-chip correction is enabled, in which case return 0.
                    #ifdef USE_HDRLIB_3RD_CHG_VER
                        if (((tuneSelect.band == HDR_BAND_FM) && (auto_alignment_config.fm_auto_level_correction_enabled == true)) ||
                            ((tuneSelect.band == HDR_BAND_AM) && (auto_alignment_config.am_auto_level_correction_enabled == true))) {
                            value=0;
                        }
                        else {
                            CMD_auto_alignment_status_t status;
                            if(CMD_cb_get_auto_alignment_status(hdrInstance, &status) < 0){
                                value = 0;
                            }
                            else {
                                value = status.levelOffset & 0x1f;
                            }
                        }
                    #else
                        value=0;
                    #endif
                    }else{
                        if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                            (void)HDR_get_tx_dig_audio_gain(hdrInstance, &value);
                        }else{
                            value=(U32)0;
                        }
                    }
                    dataOut[offset] = (U8)(value & 0xffU);
					offset++;
                    if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                        (void)HDR_test_get_raw_tx_blend_control(hdrInstance, &value);
                    }else{
                        value=(U32)0;
                    }
                    dataOut[offset] = (U8)(value & 0xffU);
					offset++;
                    if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
                        (void)HDR_get_playing_program_type(hdrInstance, &value);
                    }else{
                        value=0;
                    }
                    dataOut[offset] = (U8)(value & 0xffU);
					offset++;
                    dataOut[offset] = availablePrograms.all;
					offset++;
                    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)3); // Bytes 26 - 28: Reserved
                    offset += (U32)3;
                    dataOut[offset]  = 0; // Audio Program Conditional Access
                    offset++;
#ifdef USE_HDRLIB_3RD_CHG_VER
                    HDR_program_bitmap_t audiblePrograms;
                    audiblePrograms.all = 0;
                    (void)HDR_get_audible_programs(hdrInstance, &audiblePrograms);
                    dataOut[offset] = audiblePrograms.all;
                    offset++;
                    dataOut[offset] = (U8)HDR_get_acquisition_status(hdrInstance);
                    offset++;
                    dataOut[offset] = (U8)audioQualityReport.filt_quality_indicator;
                    offset++;
#else
                    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)3); // Bytes 30 - 32: Reserved
                    offset += (U32)3;
#endif
                    if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                        dataOut[offset] = HDR_psd_get_changed_programs(hdrInstance).all;
                    }else{
                        dataOut[offset] = 0;
                    }
					offset++;
                    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)3); // Bytes 34 - 36: Reserved
                    offset += (U32)3;

                    dataOut[offset] = 0; // Tagging is no longer supported
					offset++;
                    dataOut[offset] = 0; // Current Sample Rate Selection
					offset++;
                    dataOut[offset] = 0; // Currently Selected Sample Rate Value
					offset++;

                    dataOut[offset] = (U8)HDR_get_primary_service_mode(hdrInstance);
					offset++;
                    dataOut[offset] = (U8)codecMode;
					offset++;

                    U32 filtDsqm = 0;
                    if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                        (void)HDR_get_filt_dsqm(hdrInstance, &filtDsqm);
                    }
                    U32 thresh = 0;
                    (void)CMD_cb_bbp_get_dsqm_seek_thresh(&thresh);
                    if(filtDsqm >= thresh){
                        dataOut[offset] = 0x80;
						offset++;
                    } else {
                        dataOut[offset] = 0;
						offset++;
                    }
                    dataOut[offset] = (U8)(filtDsqm & 0xffU);
					offset++;
                    dataOut[offset] = (U8)((filtDsqm >> 8U) & 0xffU);
					offset++;

#ifdef USE_HDRLIB_2ND_CHG_VER
                    HDR_program_types_t program_types;
                    value = HDR_get_program_types(hdrInstance, &program_types);
                    for (U32 i=0; i<HDR_MAX_NUM_PROGRAMS; i++) {
                        dataOut[offset] = (U8)program_types.value[i];
                        offset++;
                    }
#else
                    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)8); // Bytes 45 - 52: Reserved
                    offset += (U32)8;
#endif
                    *outLength = offset;
                    break;
                }
                case TUNE_ANALOG:
                    LOG(CMD,2U, "Received SysTune->TUNE_GET_STATUS->TUNE_ANALOG");
                    offset += (U32)4; // Analog seek strength
                    dataOut[offset] = 0; // Mono/Stereo
					offset++;
                    dataOut[offset] = 0; // Seek and Scan
					offset++;
                    dataOut[offset] = 0; // RBDS Change
					offset++;
                    offset += (U32)6;           // Bytes 12 - 17: Reserved
                    *outLength = offset;
                    break;
                case TUNE_DIGITAL:
                {
                    LOG(CMD,2U, "Received SysTune->TUNE_GET_STATUS->TUNE_DIGITAL");
                    U32 value = 0;
                    dataOut[offset] = (U8)program; // byte 5
					offset++;
                    dataOut[offset] = acqStatus;
					offset++;

                    HDR_audio_quality_report_t audioQualityReport;
                    (void)(*stOsal.osmemset)((void*)&audioQualityReport, (S8)0, (U32)sizeof(HDR_audio_quality_report_t));

                    if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
                        (void)HDR_get_audio_quality_report(hdrInstance, &audioQualityReport);
                    }

                    dataOut[offset] = (U8)(audioQualityReport.quality_indicator & 0xffU);
					offset++;
                    if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                        (void)HDR_get_cdno(hdrInstance, &value);
                    }else{
                        value=(U32)0;
                    }
                    dataOut[offset] = (U8)(value & 0xffU);
					offset++;
                    // IF AAA is used , we should ignore this value and send 0 to the host.

                    if (((tuneSelect.band == HDR_BAND_FM) && (auto_alignment_config.fm_auto_level_align_enabled==true))||
                        ((tuneSelect.band == HDR_BAND_AM) && (auto_alignment_config.am_auto_level_align_enabled==true))){
                        // If auto level alignment algorithm is enabled, update TX_GAIN value with the result, unless
                        // on-chip correction is enabled, in which case return 0.
#ifdef USE_HDRLIB_3RD_CHG_VER
                        if (((tuneSelect.band == HDR_BAND_FM) && (auto_alignment_config.fm_auto_level_correction_enabled == true)) ||
                            ((tuneSelect.band == HDR_BAND_AM) && (auto_alignment_config.am_auto_level_correction_enabled == true))) {
                            value=0;
                        }
                        else {
                            CMD_auto_alignment_status_t status;
                            if(CMD_cb_get_auto_alignment_status(hdrInstance, &status) < 0){
                                value = 0;
                            }
                            else {
                                value = status.levelOffset & 0x1f;
                            }
                        }
#else
                        value=(U32)0;
#endif
                    }else{
                        if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                            (void)HDR_get_tx_dig_audio_gain(hdrInstance, &value);
                        }else{
                            value=(U32)0;
                        }
                    }
                    dataOut[offset] = (U8)(value & 0xffU);
                    offset++;
                    if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                        (void)HDR_test_get_raw_tx_blend_control(hdrInstance, &value);
                    }else{
                        value=(U32)0;
                    }
                    dataOut[offset] = (U8)(value & 0xffU);
					offset++;
                    if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
                        (void)HDR_get_playing_program_type(hdrInstance, &value);
                    }
                    dataOut[offset] = (U8)(value & 0xffU);
                    offset++;
                    dataOut[offset] = availablePrograms.all;
                    offset++;
					(void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)3);
                    offset += (U32)3; // Bytes 13 - 15: Reserved
                    dataOut[offset]  = 0; // Audio Program Conditional Access
                    offset++;
#ifdef USE_HDRLIB_3RD_CHG_VER
                    HDR_program_bitmap_t audiblePrograms;
                    audiblePrograms.all = 0;
                    if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
                        (void)HDR_get_audible_programs(hdrInstance, &audiblePrograms);
                    }
                    dataOut[offset] = audiblePrograms.all;
                    offset++;
                    dataOut[offset]  = (U8)HDR_get_acquisition_status(hdrInstance);
                    offset++;
                    dataOut[offset] = (U8)audioQualityReport.filt_quality_indicator;
                    offset++;
#else
                    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)3);
                    offset += (U32)3; // Bytes 17 - 19: Reserved
#endif
                    if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                        dataOut[offset] = HDR_psd_get_changed_programs(hdrInstance).all;
                    }else{
                        dataOut[offset] = 0;
                    }
                    offset++;
                    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)3);
                    offset += (U32)3; // Bytes 21 - 23: Reserved

                    dataOut[offset] = 0; // Tagging is no longer supported
					offset++;
                    dataOut[offset] = 0; // Current Sample Rate Selection
					offset++;
                    dataOut[offset] = 0; // Currently Selected Sample Rate Value
					offset++;
                    dataOut[offset] = (U8)HDR_get_primary_service_mode(hdrInstance);
					offset++;
                    dataOut[offset] = (U8)codecMode;
					offset++;

                    U32 filtDsqm = 0;
                    if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                        (void)HDR_get_filt_dsqm(hdrInstance, &filtDsqm);
                    }
                    U32 thresh = 0;
                    (void)CMD_cb_bbp_get_dsqm_seek_thresh(&thresh);
                    if(filtDsqm >= thresh){
                        dataOut[offset] = 0x80;
						offset++;
                    }else {
                        dataOut[offset] = 0;
						offset++;
                    }
                    dataOut[offset] = (U8)filtDsqm;
                    offset++;
                    dataOut[offset] = (U8)((filtDsqm >> 8U) & 0xffU);
                    offset++;
#ifdef USE_HDRLIB_2ND_CHG_VER
                    HDR_program_types_t program_types;
					value = HDR_get_program_types(hdrInstance, &program_types);
					for(U32 i=0; i<HDR_MAX_NUM_PROGRAMS; i++) {
						dataOut[offset] = (U8)program_types.value[i];
						offset++;
					}
#else
                    offset += (U32)8; // Bytes 32 - 39: Reserved
#endif
                    *outLength = offset;
                    break;
                }
                default:
					LOG(CMD,2U, "Not Recognized SysTune Method %d",tuneMethod);
					break;
            }

            break;
        }
        case (U32)TUNE_SET_PARAMS: // TODO need to implement valid input checks
		{
            LOG(CMD,4U, "Received SysTune->SET_TUNE_PARAMS");
            dataOut[0] = (U8)TUNE_SET_PARAMS;
            *outLength = 1;

            BBP_tune_params_t tuneParams;

            // FM
            tuneParams.fmMinTuningFreq = dataIn[1];
            tuneParams.fmMinTuningFreq |= (U32)dataIn[2] << 8;
            tuneParams.fmMaxTuningFreq = dataIn[3];
            tuneParams.fmMaxTuningFreq |=(U32) dataIn[4] << 8;
            tuneParams.fmFreqSpacing = dataIn[5];
            // AM
            tuneParams.amMinTuningFreq = dataIn[6];
            tuneParams.amMinTuningFreq |= (U32)dataIn[7] << 8;
            tuneParams.amMaxTuningFreq = dataIn[8];
            tuneParams.amMaxTuningFreq |= (U32)dataIn[9] << 8;
            tuneParams.amFreqSpacing = dataIn[10];

            if(CMD_cb_bbp_set_tune_params(&tuneParams) == 0){
                dataOut[1] = CMD_TUNE_VALID; // FM Valid
                dataOut[2] = CMD_TUNE_VALID; // AM Valid
            } else {
                // TODO figure out the checks
                dataOut[1] = CMD_TUNE_INVALID; // FM Invalid
                dataOut[2] = CMD_TUNE_INVALID; // AM Invalid
            }

            *outLength += (U32)2;
		}
        break;
        case (U32)TUNE_GET_PARAMS:
        {
            LOG(CMD,4U, "Received SysTune->>GET_TUNE_PARAMS");
            U32 offset = 0;
            dataOut[offset] = (U8)TUNE_GET_PARAMS;
			offset++;

            BBP_tune_params_t tuneParams;

            if(CMD_cb_bbp_get_tune_params(&tuneParams) != 0){
                *outLength = 1;
                break;
            }

            // FM
            dataOut[offset] = (U8)(tuneParams.fmMinTuningFreq & 0xffU);
			offset++;
            dataOut[offset] = (U8)((tuneParams.fmMinTuningFreq >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)(tuneParams.fmMaxTuningFreq & 0xffU);
			offset++;
            dataOut[offset] = (U8)((tuneParams.fmMaxTuningFreq >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)(tuneParams.fmFreqSpacing & 0xffU);
			offset++;
            // AM
            dataOut[offset] = (U8)(tuneParams.amMinTuningFreq & 0xffU);
			offset++;
            dataOut[offset] = (U8)((tuneParams.amMinTuningFreq >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)(tuneParams.amMaxTuningFreq & 0xffU);
			offset++;
            dataOut[offset] = (U8)((tuneParams.amMaxTuningFreq >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)(tuneParams.amFreqSpacing & 0xffU);
			offset++;

            *outLength = offset;

            break;
        }
        case (U32)GET_DSQM_STATUS:
        {
            LOG(CMD,8U, "Received SysTune->GET_DSQM_STATUS");
            dataOut[0] = (U8)GET_DSQM_STATUS;

            HDR_dsqm_t dsqm;
            U32 seekThresh;
            int_t rc = (int_t)-1;
            if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                rc = HDR_get_dsqm(hdrInstance, &dsqm);
            }
#ifdef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
            (void)CMD_cb_bbp_get_dsqm_seek_thresh(&seekThresh);
            if(rc < 0)
#else
            if(rc < 0 || CMD_cb_bbp_get_dsqm_seek_thresh(&seekThresh) < 0)
#endif
			{
               *outLength = 1;
                break;
            }

            dataOut[1] = 0;

            if(dsqm.dsqm_value >= seekThresh){
                dataOut[1] = ((U8)1 << 7);
            }
            dataOut[1] |= (U8)(dsqm.sequence_number & (U32)0x7F);
            dataOut[2] = (U8)(dsqm.dsqm_value & 0xffU);
            dataOut[3] = (U8)((dsqm.dsqm_value >> 8U) & 0xffU);

            *outLength = 4;
            break;
        }
        case (U32)GET_ALIGN_STATUS:
        {
            LOG(CMD,16U, "Received SysTune->GET_DSQM_STATUS");
            dataOut[0] = (U8)GET_ALIGN_STATUS;

            CMD_auto_alignment_config_t auto_alignment_config;
            auto_alignment_config.am_auto_time_align_enabled = false;
            auto_alignment_config.fm_auto_time_align_enabled = false;
            auto_alignment_config.am_auto_level_align_enabled = false;
            auto_alignment_config.fm_auto_level_align_enabled = false;
#ifdef USE_HDRLIB_3RD_CHG_VER
            auto_alignment_config.am_auto_level_correction_enabled = false;
            auto_alignment_config.fm_auto_level_correction_enabled = false;
#else
            auto_alignment_config.apply_level_adjustment = false;
#endif
            (void)CMD_cb_get_auto_alignment_config(hdrInstance,&auto_alignment_config);
            HDR_tune_band_t band;
            HDR_config_t config;

            //S32 ota_tx_gain=0;
            S32 timeOffset;
			F32 tmpConfidenceLevel;
            band = HDR_get_band_select(hdrInstance);


            CMD_auto_alignment_status_t status;
            if(CMD_cb_get_auto_alignment_status(hdrInstance, &status) < 0){
                *outLength = 1;
                return (S32)CMD_UNSPECIFIED_ERROR;
            }

            dataOut[1] = (*stCast.booltou8)(status.alignmentFound);
            dataOut[1] |= (*stCast.booltou8)(status.digAudioAvailable) << 1U;

            if(status.alignmentFound == true){
                dataOut[1] |= (*stCast.booltou8)(status.phaseInverted) << 2U;
            }

            (void)HDR_get_config(hdrInstance,&config);
			if (((band == HDR_BAND_FM) && (auto_alignment_config.fm_auto_time_align_enabled==false)) ||
                ((band == HDR_BAND_AM) && (auto_alignment_config.am_auto_time_align_enabled==false))){
                timeOffset=0;
            }else {
                timeOffset =status.alignmentOffset;
            }
            dataOut[2] = (U8)((U32)timeOffset);
            dataOut[3] = (U8)((U32)timeOffset >> 8U);
            dataOut[4] = (U8)((U32)timeOffset >> 16U);
            dataOut[5] = (U8)((U32)timeOffset >> 24U);

            if (((band == HDR_BAND_FM) && (auto_alignment_config.fm_auto_level_align_enabled==false))||((band == HDR_BAND_AM) && (auto_alignment_config.am_auto_level_align_enabled==false))){
                    dataOut[6]=0;
            }else{
                dataOut[6] = (U8)(status.levelOffset);
            }
			tmpConfidenceLevel = status.confidenceLevel*128.0f;
            dataOut[7] = (U8)tmpConfidenceLevel;

            *outLength = 8;
            break;
        }
        default:
            LOG(CMD,1U, "Received UNKNOWN function (0x%X) for Sys_Tune command.", funcCode);
            return (S32)CMD_UNSUPPORTED_OPCODE;
			break;
    }

    return 0;
}

