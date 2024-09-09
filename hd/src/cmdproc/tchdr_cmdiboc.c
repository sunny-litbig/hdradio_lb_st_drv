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
#include <string.h>
#include "tchdr_common.h"			// To avoid redundant condition warnings of codesonar
#include "tchdr_log.h"
#include "tchdr_cmdopcode.h"
#include "hdrConfig.h"
#include "hdrTest.h"
#include "hdrAudio.h"
#include "hdrSis.h"
#include "hdrBlend.h"
#include "hdrAlerts.h"
#include "hdrPhy.h"
#include "tchdr_cmdiboc.h"
#include "tchdr_cmdcallbacks.h"

#define RESTORE_DEFAULTS    (0x00)
#define SET_CONFIG_PARAM    (0x01)

/**
 * Table 5-17: IBOC_Cntrl_Cnfg Command (opcode 0x91) Function Definitions
 */
enum IBOC_CNTRL_CONFIG_FUNC_CODES{
    GET_IBOC_CNFG                  = 0x09,
    SET_IBOC_CNFG                  = 0x0A,
    SET_ALL_MPS_SPS_BLEND_PARAMS   = 0x0B,
    GET_ALL_MPS_SPS_BLEND_PARAMS   = 0x0C,
    SET_MPS_SPS_BLEND_PARAM        = 0x0D,
    GET_MPS_SPS_BLEND_PARAM        = 0x0E,
    SET_ALL_ADV_AUDIO_BLEND_PARAMS = 0x0F,
    GET_ALL_ADV_AUDIO_BLEND_PARAMS = 0x10,
    SET_ADV_AUDIO_BLEND_PARAM      = 0x11,
    GET_ADV_AUDIO_BLEND_PARAM      = 0x12,
    SET_MRC_CNFG                   = 0x13,
    GET_MRC_CNFG                   = 0x14,
    SET_FILTERED_DSQM_VALUE        = 0x15,
    SET_DSQM_PARAM                 = 0x16,
    GET_DSQM_PARAM                 = 0x17,
    SET_ALIGN_PARAMETERS           = 0x18,
    GET_ALIGN_PARAMETERS           = 0x19
};

enum IBOC_DIAGNOSTICS_FUNC_CODES{
    //RESERVED  0x00 - 0x07
    GET_IBOC_COUNTERS        = 0x08,
    GET_AUDIO_FER_STATUS     = 0x09,
    GET_BER                  = 0x0A,
    ENABLE_BER               = 0x0B,
    RESET_MEASUREMENTS       = 0x0C,
    //RESERVED  0x0D
    SET_BER_PATTERN          = 0x0E,
    BER_TABLE_DOWNLOAD       = 0x0F,
    GET_BER_PATTERN_STATUS   = 0x10,
    GET_MRC_STATUS           = 0x11,
    SET_MRC_DEMOD_CNTRL      = 0x12
};

enum ACTIVE_RADIO_FUNC_CODES{
    GET_AR_MSG         = 0x01,
    GET_AR_MSG_STATUS  = 0x02,
    ALERT_TONE_ROUTING = 0x03
};

static void procGetStatus(HDR_instance_t* hdrInstance, U8* dataOut, U32* outLength);
static CMD_dispatch_rc_t procIbocCtrlCnfg(HDR_instance_t* hdrInstance, const U8* dataIn, U32 inLength, U8* dataOut, U32* outLength);
static CMD_dispatch_rc_t procIbocDiagnostics(HDR_instance_t* hdrInstance, const U8* dataIn, U8* dataOut, U32* outLength);

CMD_dispatch_rc_t IBOC_procHostCommand(HDR_instance_t* hdrInstance, CMD_opcode_t opCode, const U8* dataIn, U32 inLength, U8* dataOut, U32* outLength)
{
    CMD_dispatch_rc_t dispatch_rc = CMD_DISPATCH_OK;

    switch(opCode){
        case BAND_SELECT:
        {
            LOG(CMD,1U, "Received BAND_SELECT");
            dataOut[0] = dataIn[0];

            BBP_tune_select_t tuneSelect;

            tuneSelect.band = (HDR_tune_band_t)dataIn[0];
            tuneSelect.rfFreq = 0;

            (void)CMD_cb_bbp_set_tune_select(hdrInstance, &tuneSelect);

            *outLength = 1;
            break;
        }
        case GET_STATUS:
            //LOG(CMD,1U, "Received GET_STATUS");
            procGetStatus(hdrInstance, dataOut, outLength);
            break;
        case GET_QI:
            LOG(CMD,1U, "Received GET_QI");
            HDR_audio_quality_report_t audioQualityReport;
            (void)(*stOsal.osmemset)((void*)&audioQualityReport, (S8)0, (U32)sizeof(HDR_audio_quality_report_t));

            (void)HDR_get_audio_quality_report(hdrInstance, &audioQualityReport);
            *dataOut = (U8)(audioQualityReport.quality_indicator & 0xffU);
            *outLength = 1;
            break;
        case ACTIVE_RADIO:
        {
            U32 funcCode = dataIn[0];
            switch(funcCode){
                case (U32)GET_AR_MSG:
                {
                    LOG(CMD,1U, "Received ACTIVE_RADIO->GET_AR_MSG");
                    U32 offset = 0;
                    HDR_alert_message_t arMessage;

                    dataOut[offset] = (U8)GET_AR_MSG;
					offset++;

                    if(HDR_alert_get_message(hdrInstance, &arMessage) < 0){
                        *outLength = 1;
                        break;
                    }

                    U32 crc12Status = 0x0000;

                    if(arMessage.cnt_crc_pass == true){
                        crc12Status = 0xAAA;
                    }

                    dataOut[offset] = (U8)arMessage.text_encoding;
					offset++;
                    dataOut[offset] = (U8)(arMessage.payload_length & 0xffU);
					offset++;
                    dataOut[offset] = (U8)((arMessage.payload_length >> 8U) & 0xffU);
					offset++;
                    dataOut[offset] = (U8)(arMessage.payload_crc & 0xffU);
					offset++;
                    dataOut[offset] = (U8)(arMessage.cnt_length & 0xffU);
					offset++;
                    dataOut[offset] = (U8)arMessage.payload[0];
					offset++;
                    dataOut[offset] = (U8)(crc12Status & 0xffU);
					offset++;
                    dataOut[offset] = (U8)((crc12Status >> 8U) & 0xffU);
					offset++;
                    dataOut[offset] = (U8)(arMessage.text_length & 0xffU);
					offset++;
                    dataOut[offset] = (U8)((arMessage.text_length >> 8U) & 0xffU);
					offset++;
                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)arMessage.payload, arMessage.payload_length);
                    offset = (*stArith.u32add)(offset, arMessage.payload_length);
                    *outLength = offset;
                    break;
                }
                case (U32)GET_AR_MSG_STATUS:
                {
                    LOG(CMD,1U, "Received ACTIVE_RADIO->GET_AR_MSG_STATUS");
                    U32 offset = 0;
                    HDR_alerts_msg_status_t status;

                    dataOut[offset] = (U8)GET_AR_MSG_STATUS;
					offset++;

                    if(HDR_alert_get_message_status(hdrInstance, &status) < 0){
                        *outLength = 0;
                        break;
                    }

                    // Clear status bits as required by 2206 appendix R
                    (void)HDR_alert_clear_message_status(hdrInstance);

                    dataOut[offset] = (*stCast.booltou8)(status.frame_received);
                    dataOut[offset] |= (*stCast.booltou8)(status.frame0_available) << 1;
                    dataOut[offset] |= (*stCast.booltou8)(status.full_message) << 2;
					offset++;
                    dataOut[offset] = (U8)(status.frame_counter & 0xffU);
					offset++;
                    dataOut[offset] = (U8)(status.message_id & 0xffU);
					offset++;
                    dataOut[offset] = (U8)(status.payload_crc & 0xffU);
					offset++;
                    *outLength = offset;
                    break;
                }
                case (U32)ALERT_TONE_ROUTING:
                {
                    LOG(CMD,1U, "Received ACTIVE_RADIO->GET_AR_MSG_STATUS");
					U32 offset = 0;
                    dataOut[offset] = (U8)ALERT_TONE_ROUTING;
					offset++;
                    (void)CMD_cb_bbp_play_alert_tone(hdrInstance);
                    *outLength = offset;
                    break;
                }
                default:
                    LOG(CMD,1U, "ACTIVE_RADIO -> Received unsupported Func Code(0x%X) from host.", funcCode);
                    dispatch_rc = CMD_UNSUPPORTED_OPCODE;
                    break;
            }
            break;
        }
        case IBOC_CNTRL_CNFG:
            dispatch_rc = procIbocCtrlCnfg(hdrInstance, dataIn, inLength, dataOut, outLength);
            break;
        case ADVANCED_AUDIO_BLENDING:
        {
            LOG(CMD,1U, "Received ADVANCED_AUDIO_BLENDING");
            U32 offset = 0;
            dataOut[offset] = (U8)ADVANCED_AUDIO_BLENDING;
			offset++;

            HDR_test_audio_bw_status_t audioBwStatus;
            S32 rc = HDR_test_get_audio_bw_status(hdrInstance, &audioBwStatus);

            U32 startBw = 0;
            (void)HDR_blend_get_adv_param(hdrInstance, am_dig_audio_blend_start_bw, &startBw);

            U32 maxBw = 0;
            (void)HDR_blend_get_adv_param(hdrInstance, am_dig_audio_max_bw, &maxBw);

            if(rc < 0){
                *outLength = 0;
                break;
            }

            dataOut[offset] = (U8)(audioBwStatus.currentBw & 0xffU);
			offset++;
            dataOut[offset] = (U8)(audioBwStatus.currentSep & 0xffU);
			offset++;
            dataOut[offset] = (U8)(startBw & 0xffU);
			offset++;
            dataOut[offset] = (U8)(maxBw & 0xffU);
			offset++;
            dataOut[offset] = 0; // reserved
			offset++;
            dataOut[offset] = 0; // reserved
			offset++;

            *outLength = offset;

            break;
        }
        case IBOC_DIAGNOSTICS:
            dispatch_rc = procIbocDiagnostics(hdrInstance, dataIn, dataOut, outLength);
            break;
        default:
            LOG(CMD,1U, "Received unsupported OP Code(0x%X) from host.", opCode);
            dispatch_rc = CMD_UNSUPPORTED_OPCODE;
			break;
    }

    return dispatch_rc;
}

static CMD_dispatch_rc_t procIbocCtrlCnfg(HDR_instance_t* hdrInstance, const U8* dataIn, U32 inLength, U8* dataOut, U32* outLength)
{
	CMD_dispatch_rc_t dispatch_rc = CMD_DISPATCH_OK;

	if(dataIn == NULL) {
		(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "procIbocCtrlCnfg input data is null.\n");
		dispatch_rc = CMD_UNSPECIFIED_ERROR;
    }

    if(inLength <= (U32)0){
        dispatch_rc = CMD_UNSPECIFIED_ERROR;
    }

	if(dispatch_rc == CMD_DISPATCH_OK) {
    	U8 funcCode = dataIn[0];

	    switch(funcCode){
	        case (U8)GET_IBOC_CNFG:
	        {
	           LOG(CMD,128U, "received IBOC_CNTRL_CNFG->GET_IBOC_CNFG");
	           U32 offset = 0;
	           dataOut[offset] = (U8)GET_IBOC_CNFG;
			   offset++;
	           dataOut[offset] = dataIn[1]; // Configuration type(Not supported at the moment)
			   offset++;

	           BBP_iboc_config_t ibocConfig;
	           HDR_config_t hdrConfig;
#ifdef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
	           (void)CMD_cb_bbp_get_iboc_config(&ibocConfig, (BBP_config_select_t)dataIn[1]);
	           if(HDR_get_config(hdrInstance, &hdrConfig) != 0)
#else
	           if(CMD_cb_bbp_get_iboc_config(&ibocConfig, (BBP_config_select_t)dataIn[1]) != 0 || HDR_get_config(hdrInstance, &hdrConfig) != 0)
#endif
	           {
	               *outLength = 2;
	               break;
	           }

	           dataOut[offset] = 0; // reserved
			   offset++;
	           dataOut[offset] = (U8)(hdrConfig.blend_params.d2a_blend_holdoff & 0xffU);
			   offset++;
	           dataOut[offset] = (U8)((hdrConfig.blend_params.d2a_blend_holdoff >> 8U) & 0xffU);
			   offset++;

	           // Skip deprecated codec blend params
	           (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)1, (U32)112);
	           offset += (U32)112;

	           //0x73
	           dataOut[offset] = 0x00;
	           if(hdrConfig.blend_params.am_audio_invert_phase) {
	                dataOut[offset] = 0x01U;
	           }
	           if(hdrConfig.blend_params.fm_audio_invert_phase) {
	                dataOut[offset] |= 0x02U;
	           }
			   offset++;
	           (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)4); // reserved
	           offset += (U32)4;
	           U32 timeConst;
	           (void)HDR_get_dsqm_filt_time_const(hdrInstance, &timeConst);
	           dataOut[offset] = (U8)(timeConst & 0xffU);
			   offset++;
	           dataOut[offset] =  1; // EZ blend is always enabled
			   offset++;
	           dataOut[offset] = (*stCast.booltou8)(hdrConfig.blend_params.blend_decision);
			   offset++;
	           dataOut[offset] = (U8)(hdrConfig.blend_params.fm_cdno_blend_decision & 0xffU);
			   offset++;
	           dataOut[offset] = (U8)(hdrConfig.blend_params.am_cdno_blend_decision & 0xffU);
			   offset++;
	           dataOut[offset] = 0; // reserved
			   offset++;
	           dataOut[offset] = (U8)(ibocConfig.dsqm_seek_threshold & 0xffU);
			   offset++;
	           dataOut[offset] = (U8)((ibocConfig.dsqm_seek_threshold >> 8U) & 0xffU);
			   offset++;
#ifndef AVOID_QAC_1STEP_WARNING		// This comparison is always ''false'.
	           if(offset != (U32)130) {
					(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "GET_IBOC_CNFG offset is incorrect.\n");
					dispatch_rc = CMD_UNSPECIFIED_ERROR;
					break;
	           }
#endif
	           *outLength = offset;
	            break;
	        }
	        case (U8)SET_IBOC_CNFG:
	        {
	            LOG(CMD,128U, "received IBOC_CNTRL_CNFG->SET_IBOC_CNFG");

	            dataOut[0] = (U8)SET_IBOC_CNFG;
	            dataOut[1] = dataIn[1];

	            // Re-initializing blend(iboc) parameters
	            if(dataIn[1] == (U8)0x00){
	                *outLength = 1; // Not supported
	                break;
	            }

	            U32 offset = 2;
	            BBP_iboc_config_t ibocConfig;
	            U32 value;

	            offset++; // skip reserved
	            value = dataIn[offset];
				offset++;
	            value |= (U32)dataIn[offset] << 8;
				offset++;

	            U32 rc =0;
	            S32 rcD2A = HDR_blend_set_param(hdrInstance, d2a_blend_holdoff, value);
	            // rcD2A = -2 when this command is issued in FM or AM mode.
	            // checking if the command was sent in normal mode and  notifying the framework

	            if(rcD2A == -2){
	                (void)CMD_cb_write_protect_error("d2a blend holdoff");
	            }
	            // If the error is -2 or 0 then we force the rc to be 0
	            // rc is the combined return condition of all the functions in this 2206 commands.
	            // if rc is -1 from here on then the command should return an error condition and not return
	            // Byte 1 because that will only happen on an invalid value being sent by the host.

	            if((rcD2A == -2) || (rcD2A == 0)){
	                rc=0U;
	            }else{
	                rc=1U;
	            }

	            // Skip deprecated codec based blend
	            offset += 112U;

	            U32 phaseInversion = dataIn[offset];
				offset++;
	            rc |= (U32)(HDR_blend_set_param(hdrInstance, am_audio_invert_phase, (phaseInversion & 1U)) < 0);
	            rc |= (U32)(HDR_blend_set_param(hdrInstance, fm_audio_invert_phase, (phaseInversion >> 1U))< 0);

	            offset += (U32)4;// reserved
	            rc |= (U32)(HDR_set_dsqm_filt_time_const(hdrInstance, dataIn[offset]) < 0);
	            offset++;

	            offset++; // Skip deprecatd ez blend enabled

	            rc |= (U32)(HDR_blend_set_param(hdrInstance, blend_decision, dataIn[offset]) < 0);
	            offset++;

	            rc |= (U32)(HDR_blend_set_param(hdrInstance, fm_cdno_blend_decision, dataIn[offset]) < 0);
	            offset++;

	            rc |= (U32)(HDR_blend_set_param(hdrInstance, am_cdno_blend_decision, dataIn[offset]) < 0);
	            offset++;

	            offset += (U32)1;// reserved
	            ibocConfig.dsqm_seek_threshold = dataIn[offset];
				offset++;
	            ibocConfig.dsqm_seek_threshold |= (U32)dataIn[offset] << 8;
				offset++;
#ifndef AVOID_QAC_1STEP_WARNING		// This comparison is always ''false'.
				if(offset != (U32)130) {
					(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "SET_IBOC_CNFG offset is incorrect.\n");
					dispatch_rc = CMD_UNSPECIFIED_ERROR;
					break;
				}
#endif
	            // This returns the byte 1 only if all functions are processed correctly.
	            // For commands which dont comply with IDLE mode we notify the framework using the CMD_cb_write_protect_error callback.
	            // But we always return Byte 1 unless the command issued with a bad value.
	            if((CMD_cb_bbp_set_iboc_config(&ibocConfig) == 0) && (rc == 0U)){
	                *outLength = 2;
	            } else {
	                *outLength = 1;
	            }
	            break;
	        }
	        case (U8)SET_ALL_MPS_SPS_BLEND_PARAMS:
	        {
	            LOG(CMD,256U, "received IBOC_CNTRL_CNFG->SET_ALL_MPS_SPS_BLEND_PARAMS");

	            dataOut[0] = (U8)SET_ALL_MPS_SPS_BLEND_PARAMS;
	            dataOut[1] = dataIn[1];

	            // Byte 1:0x00: Re-initialize MPS and SPS blending configuration parameters (no further data is required)
	            if(dataIn[1] ==(U8)0x00){
	                *outLength = 1; // Not supported
	                break;
	            }

	            // Byte 1:0x01: Set MPS and SPS blending configuration parameter values as defined in subsequent data
	            U32 offset = 2;
	            *outLength = 1;

	            HDR_blend_params_t hdrBlendParams;
	            if(HDR_blend_get_all_params(hdrInstance, &hdrBlendParams) < 0){
	                *outLength = 1;
	                dispatch_rc = CMD_UNSPECIFIED_ERROR;
					break;
	            }

	            hdrBlendParams.fm_mps_blend_thresh = (HDR_blend_thresh_sel_t)dataIn[offset];
				offset++;
	            hdrBlendParams.fm_mps_audio_scaling = dataIn[offset];
				offset++;
	            hdrBlendParams.fm_mps_audio_scaling |= (U32)dataIn[offset] << 8;
				offset++;
	            hdrBlendParams.fm_mps_blend_rate = dataIn[offset];
				offset++;
	            hdrBlendParams.fm_mps_dig_audio_delay = dataIn[offset];
				offset++;
	            hdrBlendParams.fm_mps_dig_audio_delay |= (U32)dataIn[offset] << 8;
				offset++;
	            hdrBlendParams.fm_mps_dig_audio_delay += (U32)40000; // CDM delay is offset by 40000

	            offset+= (U32)2; //fm_mps_alt_dig_audio_delay

	            hdrBlendParams.fm_all_dig_blend_thresh = (HDR_blend_thresh_sel_t)dataIn[offset];
				offset++;
	            hdrBlendParams.fm_all_dig_audio_scaling = dataIn[offset];
				offset++;
	            hdrBlendParams.fm_all_dig_audio_scaling |= (U32)dataIn[offset] << 8;
				offset++;
	            hdrBlendParams.fm_all_dig_blend_rate = dataIn[offset];
				offset++;

	            hdrBlendParams.am_mps_blend_thresh = (HDR_blend_thresh_sel_t)dataIn[offset];
				offset++;
	            hdrBlendParams.am_mps_audio_scaling = dataIn[offset];
				offset++;
	            hdrBlendParams.am_mps_audio_scaling |= (U32)dataIn[offset] << 8;
				offset++;
	            hdrBlendParams.am_mps_blend_rate = dataIn[offset];
				offset++;
	            hdrBlendParams.am_mps_dig_audio_delay = dataIn[offset];
				offset++;
	            hdrBlendParams.am_mps_dig_audio_delay |= (U32)dataIn[offset] << 8;
				offset++;
	            offset += (U32)2; //am_mps_alt_dig_audio_delay

	            hdrBlendParams.am_all_dig_blend_thresh = (HDR_blend_thresh_sel_t)dataIn[offset];
				offset++;
	            hdrBlendParams.am_all_dig_audio_scaling = dataIn[offset];
				offset++;
	            hdrBlendParams.am_all_dig_audio_scaling |= (U32)dataIn[offset] << 8;
				offset++;
	            hdrBlendParams.am_all_dig_blend_rate = dataIn[offset];
				offset++;
#ifndef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
				if(offset != 26) {	// This comparison is always false.
					(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "SET_ALL_MPS_SPS_BLEND_PARAMS offset is incorrect.\n");
					dispatch_rc = CMD_UNSPECIFIED_ERROR;
					break;
				}
#endif
	            *outLength = offset;

	            if(HDR_blend_set_all_params(hdrInstance, &hdrBlendParams) == 0){
	                *outLength = 2;
	            } else {
	                *outLength = 1;
	            }
	            break;
	        }
	        case (U8)GET_ALL_MPS_SPS_BLEND_PARAMS:
	        {
	            LOG(CMD,256U, "received IBOC_CNTRL_CNFG->GET_ALL_MPS_SPS_BLEND_PARAMS");
	            U32 offset = 0;
	            dataOut[offset] = (U8)GET_ALL_MPS_SPS_BLEND_PARAMS;
				offset++;
	            dataOut[offset] = dataIn[1];
				offset++;

	            HDR_blend_params_t hdrBlendParams;

	            if(HDR_blend_get_all_params(hdrInstance, &hdrBlendParams) != 0){
	                *outLength = 1;
	                break;
	            }

	            dataOut[offset] = (U8)hdrBlendParams.fm_mps_blend_thresh;
				offset++;
	            dataOut[offset] = (U8)(hdrBlendParams.fm_mps_audio_scaling & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((hdrBlendParams.fm_mps_audio_scaling >> 8U) & 0xffU);
				offset++;
	            dataOut[offset] = (U8)(hdrBlendParams.fm_mps_blend_rate & 0xffU);
				offset++;

	            U32 mpsDigDelay = hdrBlendParams.fm_mps_dig_audio_delay - (U32)40000; // put the delay in range of the CDM slider

	            dataOut[offset] = (U8)(mpsDigDelay & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((mpsDigDelay >> 8U) & 0xffU);
				offset++;
	            dataOut[offset] = 0; //fm_mps_alt_dig_audio_delay
				offset++;
	            dataOut[offset] = 0; //fm_mps_alt_dig_audio_delay
				offset++;

	            dataOut[offset] = (U8)hdrBlendParams.fm_all_dig_blend_thresh;
				offset++;
	            dataOut[offset] = (U8)(hdrBlendParams.fm_all_dig_audio_scaling & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((hdrBlendParams.fm_all_dig_audio_scaling >> 8U) & 0xffU);
				offset++;
	            dataOut[offset] = (U8)(hdrBlendParams.fm_all_dig_blend_rate & 0xffU);
				offset++;

	            dataOut[offset] = (U8)hdrBlendParams.am_mps_blend_thresh;
				offset++;
	            dataOut[offset] = (U8)(hdrBlendParams.am_mps_audio_scaling & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((hdrBlendParams.am_mps_audio_scaling >> 8U) & 0xffU);
				offset++;
	            dataOut[offset] = (U8)(hdrBlendParams.am_mps_blend_rate & 0xffU);
				offset++;

	            dataOut[offset] = (U8)(hdrBlendParams.am_mps_dig_audio_delay & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((hdrBlendParams.am_mps_dig_audio_delay >> 8U) & 0xffU);
				offset++;
	            dataOut[offset] = 0; //am_mps_alt_dig_audio_delay
				offset++;
	            dataOut[offset] = 0; //am_mps_alt_dig_audio_delay
				offset++;

	            dataOut[offset] = (U8)hdrBlendParams.am_all_dig_blend_thresh;
				offset++;
	            dataOut[offset] = (U8)(hdrBlendParams.am_all_dig_audio_scaling & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((hdrBlendParams.am_all_dig_audio_scaling >> 8U) & 0xffU);
				offset++;
	            dataOut[offset] = (U8)(hdrBlendParams.am_all_dig_blend_rate & 0xffU);
				offset++;

#ifndef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
				if(offset != 26) {	// This comparison is always false.
					(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "GET_ALL_MPS_SPS_BLEND_PARAMS offset is incorrect.\n");
					dispatch_rc = CMD_UNSPECIFIED_ERROR;
					break;
				}
#endif
	            *outLength = offset;
	            break;
	        }
	        case (U8)SET_MPS_SPS_BLEND_PARAM:
	        {
	            LOG(CMD,512U, "received IBOC_CNTRL_CNFG->SET_MPS_SPS_BLEND_PARAMS");

	            dataOut[0] = (U8)SET_MPS_SPS_BLEND_PARAM;

	            // Table 5-17: IBOC_Cntrl_Cnfg Command (opcode 0x91) Function Definitions
	            // Function Code 0x0D
	            // Byte 1 = 0x00 Re-initialize the parameter designated by Byte 2
	            if(dataIn[1] == (U8)0x00){ // reinitialize param
	                // TODO
	                break;
	            }

	            // Byte 1 = 0x01 Set the parameter designated by Byte 2 to the value defined by Byte 3 (and Byte 4 for parameters that are two bytes in length)
	            S32 rc;
	            U32 value = dataIn[3];

	            switch(dataIn[2]){
	                case 0x00:
	                    rc = HDR_blend_set_param(hdrInstance, fm_mps_blend_thresh, value);
	                    break;
	                case 0x01:
	                    value = dataIn[3] | ((U32)dataIn[4] << 8);
	                    rc = HDR_blend_set_param(hdrInstance, fm_mps_audio_scaling, value);
	                    break;
	                case 0x03:
	                    rc = HDR_blend_set_param(hdrInstance, fm_mps_blend_rate, value);
	                    break;
	                case 0x04:
	                    value = (dataIn[3] | ((U32)dataIn[4] << 8)) + (U32)40000; // CDM delay is offset by 40000
	                    rc = HDR_blend_set_param(hdrInstance, fm_mps_dig_audio_delay, value);
	                    break;
	                case 0x06:
	                    // Alternative digital audio delay not supported
	                    rc = -1;
	                    break;
	                case 0x08:
	                    rc = HDR_blend_set_param(hdrInstance, fm_all_dig_blend_thresh, value);
	                    break;
	                case 0x09:
	                    value = dataIn[3] | ((U32)dataIn[4] << 8);
	                    rc = HDR_blend_set_param(hdrInstance, fm_all_dig_audio_scaling, value);
	                    break;
	                case 0x0B:
	                    rc = HDR_blend_set_param(hdrInstance, fm_all_dig_blend_rate, value);
	                    break;
	                case 0x0C:
	                    rc = HDR_blend_set_param(hdrInstance, am_mps_blend_thresh, value);
	                    break;
	                case 0x0D:
	                    value = dataIn[3] | ((U32)dataIn[4] << 8);
	                    rc = HDR_blend_set_param(hdrInstance, am_mps_audio_scaling, value);
	                    break;
	                case 0x0F:
	                    rc = HDR_blend_set_param(hdrInstance, am_mps_blend_rate, value);
	                    break;
	                case 0x10:
	                    value = dataIn[3] | ((U32)dataIn[4] << 8);
	                    rc = HDR_blend_set_param(hdrInstance, am_mps_dig_audio_delay, value);
	                    break;
	                case 0x12:
	                    // Alternative digital audio delay not supported
	                    rc = -1;
	                    break;
	                case 0x14:
	                    rc = HDR_blend_set_param(hdrInstance, am_all_dig_blend_thresh, value);
	                    break;
	                case 0x15:
	                    value = dataIn[3] | ((U32)dataIn[4] << 8);
	                    rc = HDR_blend_set_param(hdrInstance, am_all_dig_audio_scaling, value);
	                    break;
	                case 0x17:
	                    rc = HDR_blend_set_param(hdrInstance, am_all_dig_blend_rate, value);
	                    break;
	                default:
	                    LOG(CMD,512U, "Undefined Ez Blend Param offset.");
	                    rc = -1;
	                    break;
	            }

	            if(rc < 0){
	                dataOut[1] = 0x00; // failure
	            } else {
	                dataOut[1] = 0x01; // success
	            }
	            *outLength = 2;
	            break;
	        }
	        case (U8)GET_MPS_SPS_BLEND_PARAM:
	        {
	            LOG(CMD,512U, "received IBOC_CNTRL_CNFG->GET_MPS_SPS_BLEND_PARAMS");
	            dataOut[0] = (U8)GET_MPS_SPS_BLEND_PARAM;

	            U32 value = 0;
	            S32 rc = 0;
	            *outLength = 2;

	            switch(dataIn[1]){
	                case 0x00:
	                    rc = HDR_blend_get_param(hdrInstance, fm_mps_blend_thresh, &value);
	                    break;
	                case 0x01:
	                    rc = HDR_blend_get_param(hdrInstance, fm_mps_audio_scaling, &value);
	                    dataOut[2] = (U8)((value >> 8U) & 0xffU);
	                    *outLength += (U32)1;
	                    break;
	                case 0x03:
	                    rc = HDR_blend_get_param(hdrInstance, fm_mps_blend_rate, &value);
	                    break;
	                case 0x04:
	                    rc = HDR_blend_get_param(hdrInstance, fm_mps_dig_audio_delay, &value);
	                    value = (*stArith.u32sub)(value, 40000U); // put the delay in range of the CDM slider
	                    dataOut[2] = (U8)((value >> 8U) & 0xffU);
	                    *outLength += (U32)1;
	                    break;
	                case 0x06:
	                    // Alternative digital audio delay not supported
	                    rc = -1;
	                    break;
	                case 0x08:
	                    rc = HDR_blend_get_param(hdrInstance, fm_all_dig_blend_thresh, &value);
	                    break;
	                case 0x09:
	                    rc = HDR_blend_get_param(hdrInstance, fm_all_dig_audio_scaling, &value);
	                    dataOut[2] = (U8)((value >> 8U) & 0xffU);
	                    *outLength += (U32)1;
	                    break;
	                case 0x0B:
	                    rc = HDR_blend_get_param(hdrInstance, fm_all_dig_blend_rate, &value);
	                    break;
	                case 0x0C:
	                    rc = HDR_blend_get_param(hdrInstance, am_mps_blend_thresh, &value);
	                    break;
	                case 0x0D:
	                    rc = HDR_blend_get_param(hdrInstance, am_mps_audio_scaling, &value);
	                    dataOut[2] = (U8)((value >> 8U) & 0xffU);
	                    *outLength+= (U32)1;
	                    break;
	                case 0x0F:
	                    rc = HDR_blend_get_param(hdrInstance, am_mps_blend_rate, &value);
	                    break;
	                case 0x10:
	                    rc = HDR_blend_get_param(hdrInstance, am_mps_dig_audio_delay, &value);
	                    dataOut[2] = (U8)((value >> 8U) & 0xffU);
	                    *outLength += (U32)1;
	                    break;
	                case 0x12:
	                    // Alternative digital audio delay not supported
	                    rc = -1;
	                    break;
	                case 0x14:
	                    rc = HDR_blend_get_param(hdrInstance, am_all_dig_blend_thresh, &value);
	                    break;
	                case 0x15:
	                    rc = HDR_blend_get_param(hdrInstance, am_all_dig_audio_scaling, &value);
	                    dataOut[2] = (U8)((value >> 8U) & 0xffU);
	                    *outLength += (U32)1;
	                    break;
	                case 0x17:
	                    rc = HDR_blend_get_param(hdrInstance, am_all_dig_blend_rate, &value);
	                    break;
	                 default:
	                    LOG(CMD,1U, "Undefined Ez Blend Param offset.");
	                    *outLength = 1;
	                    break;
	            }

	            dataOut[1] = (U8)(value & 0xffU);

	            if(rc < 0){
	               *outLength = 1;
	            }
	            break;
	        }
	        case (U8)SET_ALL_ADV_AUDIO_BLEND_PARAMS:
	        {
	            LOG(CMD,1024U, "received IBOC_CNTRL_CNFG->SET_ALL_ADV_AUDIO_BLEND_PARAMS");

	            dataOut[0] = (U8)SET_ALL_ADV_AUDIO_BLEND_PARAMS;
	            dataOut[1] = dataIn[1];

	            if(dataIn[1] ==(U8)0x00){ // reinitialize params
	                // TODO
	                break;
	            }

	            U32 offset = 2;
	            *outLength = 2;
	            HDR_blend_adv_params_t advBlendParams;

	            advBlendParams.ramp_up_enabled = (HDBOOL)dataIn[offset];
				offset++;
	            advBlendParams.ramp_up_time = dataIn[offset];
				offset++;
	            advBlendParams.ramp_down_enabled = (HDBOOL)dataIn[offset];
				offset++;
	            advBlendParams.comfort_noise_enabled = (HDBOOL)dataIn[offset];
				offset++;
	            advBlendParams.ramp_down_time = dataIn[offset];
				offset++;
	            advBlendParams.comfort_noise_level = (S32)((S8)dataIn[offset]);
				offset++;

	            // 0x06
	            advBlendParams.am_enh_stream_holdoff_enabled = (HDBOOL)dataIn[offset];
				offset++;
	            offset += (U32)4; //reserved
	            advBlendParams.am_mps_enh_stream_holdoff_thresh = dataIn[offset];
				offset++;
	            advBlendParams.am_all_dig_enh_stream_holdoff_thresh = dataIn[offset];
				offset++;
	            offset += (U32)7; // reserved

	            // 0x14
	            advBlendParams.am_dig_audio_bw_mgmt_enabled = (HDBOOL)dataIn[offset];
				offset++;
	            advBlendParams.am_mono2stereo_enabled = (HDBOOL)dataIn[offset];
				offset++;
	            advBlendParams.am_mono2stereo_start_bw = dataIn[offset];
				offset++;
	            advBlendParams.am_mono2stereo_step_time = dataIn[offset];
				offset++;
	            advBlendParams.am_mono2stereo_max_sep = dataIn[offset];
				offset++;
	            advBlendParams.am_dig_audio_blend_start_bw = dataIn[offset];
				offset++;
	            advBlendParams.am_dig_audio_max_bw = dataIn[offset];
				offset++;

	            // 0x1B
	            advBlendParams.am_dig_audio_bw_step_up_size = dataIn[offset];
				offset++;
	            advBlendParams.am_dig_audio_bw_step_down_size =  dataIn[offset];
				offset++;
	            advBlendParams.am_dig_audio_bw_step_threshold = dataIn[offset];
				offset++;
	            advBlendParams.am_dig_audio_bw_step_time = dataIn[offset];
				offset++;
	            advBlendParams.am_dig_audio_bw_step_time |= (U32)dataIn[offset] << 8;
				offset++;
	            offset += (U32)12; // reserved

	            // 0x2C
	            offset += (U32)6; // reserved
#ifndef AVOID_QAC_1STEP_WARNING		// This comparison is always ''false'.
				if(offset != (U32)52) {
					(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "SET_ALL_ADV_AUDIO_BLEND_PARAMS offset is incorrect.\n");
					dispatch_rc = CMD_UNSPECIFIED_ERROR;
					break;
				}
#endif
	            if(HDR_blend_set_all_adv_params(hdrInstance, &advBlendParams) != 0){
	                *outLength = 1;
	            }
	            break;
	        }
	        case (U8)GET_ALL_ADV_AUDIO_BLEND_PARAMS:
	        {
	            LOG(CMD,1024U, "received IBOC_CNTRL_CNFG->GET_ALL_ADV_AUDIO_BLEND_PARAMS");
	            U32 offset = 0;
				U32 uitemp = 0;
	            dataOut[offset] = (U8)GET_ALL_ADV_AUDIO_BLEND_PARAMS;
				offset++;
	            dataOut[offset] = dataIn[1];
				offset++;

	            HDR_blend_adv_params_t advBlendParams;

	            S32 rc;
	            if((BBP_config_select_t)dataIn[1] == ACTIVE){
	                rc = HDR_blend_get_all_adv_params(hdrInstance, &advBlendParams);
	            } else {
	                rc = -1; // Not supported
	            }

	            if(rc != 0){
	                *outLength = 1;
	                break;
	            }

	            dataOut[offset] = (*stCast.booltou8)(advBlendParams.ramp_up_enabled);
				offset++;
	            dataOut[offset] = (U8)(advBlendParams.ramp_up_time & 0xffU);
				offset++;
	            dataOut[offset] = (*stCast.booltou8)(advBlendParams.ramp_down_enabled);
				offset++;
	            dataOut[offset] = (*stCast.booltou8)(advBlendParams.comfort_noise_enabled);
				offset++;
	            dataOut[offset] = (U8)(advBlendParams.ramp_down_time & 0xffU);
				offset++;
	            (void)(*stOsal.osmemcpy)((void*)&uitemp, (void*)&advBlendParams.comfort_noise_level, (U32)sizeof(S32));
	            dataOut[offset] = (U8)(uitemp & 0xffU);
				offset++;

	            // 0x06
	            dataOut[offset] = (*stCast.booltou8)(advBlendParams.am_enh_stream_holdoff_enabled);
				offset++;
	            offset += (U32)4; //reserved
	            dataOut[offset] = (U8)(advBlendParams.am_mps_enh_stream_holdoff_thresh & 0xffU);
				offset++;
	            dataOut[offset] = (U8)(advBlendParams.am_all_dig_enh_stream_holdoff_thresh & 0xffU);
				offset++;
	            offset += (U32)7; // reserved

	            // 0x14
	            dataOut[offset] = (*stCast.booltou8)(advBlendParams.am_dig_audio_bw_mgmt_enabled);
				offset++;
	            dataOut[offset] = (*stCast.booltou8)(advBlendParams.am_mono2stereo_enabled);
				offset++;
	            dataOut[offset] = (U8)(advBlendParams.am_mono2stereo_start_bw & 0xffU);
				offset++;
	            dataOut[offset] = (U8)(advBlendParams.am_mono2stereo_step_time & 0xffU);
				offset++;
	            dataOut[offset] = (U8)(advBlendParams.am_mono2stereo_max_sep & 0xffU);
				offset++;
	            dataOut[offset] = (U8)(advBlendParams.am_dig_audio_blend_start_bw & 0xffU);
				offset++;
	            dataOut[offset] = (U8)(advBlendParams.am_dig_audio_max_bw & 0xffU);
				offset++;

	            // 0x1B
	            dataOut[offset] = (U8)(advBlendParams.am_dig_audio_bw_step_up_size & 0xffU);
				offset++;
	            dataOut[offset] = (U8)(advBlendParams.am_dig_audio_bw_step_down_size & 0xffU);
				offset++;
	            dataOut[offset] = (U8)(advBlendParams.am_dig_audio_bw_step_threshold & 0xffU);
				offset++;
	            dataOut[offset] = (U8)(advBlendParams.am_dig_audio_bw_step_time & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((advBlendParams.am_dig_audio_bw_step_time >> 8U) & 0xffU);
				offset++;
	            offset += (U32)12; // reserved

	            // 0x2C
	            offset += (U32)6; // reserved
#ifndef AVOID_QAC_1STEP_WARNING		// This comparison is always ''false'.
				if(offset != (U32)52) {
					(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "GET_ALL_ADV_AUDIO_BLEND_PARAMS offset is incorrect.\n");
					dispatch_rc = CMD_UNSPECIFIED_ERROR;
					break;
				}
#endif
	            *outLength = offset;
	            break;
	        }
	        case (U8)SET_ADV_AUDIO_BLEND_PARAM:
	        {
	            LOG(CMD,2048U, "received IBOC_CNTRL_CNFG->SET_ADV_AUDIO_BLEND_PARAM");

	            dataOut[0] = (U8)SET_ADV_AUDIO_BLEND_PARAM;

	            // Table 5-17: IBOC_Cntrl_Cnfg Command (opcode 0x91) Function Definitions
	            // Function Code 0x11
	            // Byte 1 = 0x00 Re-initialize the parameter designated by Byte 2
	            // Byte 1 = 0x01 Set the parameter designated by Byte 2 to the value defined by Byte 3 (and Byte 4 for parameters that are two bytes in length)
	            // Byte 2: Designates the offset of the parameter to set. The offsets are specified in Table 9-10
	            if(dataIn[1] == (U8)0x00){ // reinitialize params
	                // TODO
	                break;
	            }

	            S32 rc;
	            // Byte 1 = 0x01 Set the parameter designated by Byte 2 to the value defined by Byte 3 (and Byte 4 for parameters that are two bytes in length)
#ifdef USE_HDRLIB_2ND_CHG_VER
				S32 value = (S8)dataIn[3];
#else
	            U32 value = dataIn[3];
#endif

	            switch(dataIn[2]){
	                case 0x00:
	                    rc = HDR_blend_set_adv_param(hdrInstance, ramp_up_enabled, value);
	                    break;
	                case 0x01:
	                    rc = HDR_blend_set_adv_param(hdrInstance, ramp_up_time, value);
	                    break;
	                case 0x02:
	                    rc = HDR_blend_set_adv_param(hdrInstance, ramp_down_enabled, value);
	                    break;
	                case 0x03:
	                    rc = HDR_blend_set_adv_param(hdrInstance, comfort_noise_enabled, value);
	                    break;
	                case 0x04:
	                    rc = HDR_blend_set_adv_param(hdrInstance, ramp_down_time, value);
	                    break;
	                case 0x05:
	                    rc = HDR_blend_set_adv_param(hdrInstance, comfort_noise_level, value);
	                    break;
	                case 0x06:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_enh_stream_holdoff_enabled, value);
	                    break;
	                case 0x0B:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_mps_enh_stream_holdoff_thresh, value);
	                    break;
	                case 0x0C:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_all_dig_enh_stream_holdoff_thresh, value);
	                    break;
	                case 0x14:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_dig_audio_bw_mgmt_enabled, value);
	                    break;
	                case 0x15:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_mono2stereo_enabled, value);
	                    break;
	                case 0x16:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_mono2stereo_start_bw, value);
	                    break;
	                case 0x17:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_mono2stereo_step_time, value);
	                    break;
	                case 0x18:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_mono2stereo_max_sep, value);
	                    break;
	                case 0x19:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_dig_audio_blend_start_bw, value);
	                    break;
	                case 0x1A:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_dig_audio_max_bw, value);
	                    break;
	                case 0x1B:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_dig_audio_bw_step_up_size, value);
	                    break;
	                case 0x1C:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_dig_audio_bw_step_down_size, value);
	                    break;
	                case 0x1D:
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_dig_audio_bw_step_threshold, value);
	                    break;
	                case 0x1E:
	                    value = dataIn[3] | ((U32)dataIn[4] << 8);
	                    rc = HDR_blend_set_adv_param(hdrInstance, am_dig_audio_bw_step_time, value);
	                    break;
	                 default:
	                    LOG(CMD,1U, "Undefined adv Blend Param offset.");
	                    rc = -1;
	                    *outLength = 1;
	                    break;
	            }

	            if(rc < 0){
	                dataOut[1] = 0x00; // failure
	            } else {
	                dataOut[1] = 0x01; // success
	            }
	            *outLength = 2;
	            break;
	        }
	        case (U8)GET_ADV_AUDIO_BLEND_PARAM:
	        {
	            LOG(CMD,2048U, "received IBOC_CNTRL_CNFG->GET_ADV_AUDIO_BLEND_PARAM");
	            dataOut[0] = (U8)GET_ADV_AUDIO_BLEND_PARAM;

	            U32 value = 0;
	            S32 rc = 0;
	            *outLength = 2;

	            switch(dataIn[1]){
	                case 0x00:
	                    rc = HDR_blend_get_adv_param(hdrInstance, ramp_up_enabled, &value);
	                    break;
	                case 0x01:
	                    rc = HDR_blend_get_adv_param(hdrInstance, ramp_up_time, &value);
	                    break;
	                case 0x02:
	                    rc = HDR_blend_get_adv_param(hdrInstance, ramp_down_enabled, &value);
	                    break;
	                case 0x03:
	                    rc = HDR_blend_get_adv_param(hdrInstance, comfort_noise_enabled, &value);
	                    break;
	                case 0x04:
	                    rc = HDR_blend_get_adv_param(hdrInstance, ramp_down_time, &value);
	                    break;
	                case 0x05:
	                    rc = HDR_blend_get_adv_param(hdrInstance, comfort_noise_level, &value);
	                    break;
	                case 0x06:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_enh_stream_holdoff_enabled, &value);
	                    break;
	                case 0x0B:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_mps_enh_stream_holdoff_thresh, &value);
	                    break;
	                case 0x0C:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_all_dig_enh_stream_holdoff_thresh, &value);
	                    break;
	                case 0x14:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_dig_audio_bw_mgmt_enabled, &value);
	                    break;
	                case 0x15:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_mono2stereo_enabled, &value);
	                    break;
	                case 0x16:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_mono2stereo_start_bw, &value);
	                    break;
	                case 0x17:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_mono2stereo_step_time, &value);
	                    break;
	                case 0x18:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_mono2stereo_max_sep, &value);
	                    break;
	                case 0x19:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_dig_audio_blend_start_bw, &value);
	                    break;
	                case 0x1A:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_dig_audio_max_bw, &value);
	                    break;
	                case 0x1B:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_dig_audio_bw_step_up_size, &value);
	                    break;
	                case 0x1C:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_dig_audio_bw_step_down_size, &value);
	                    break;
	                case 0x1D:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_dig_audio_bw_step_threshold, &value);
	                    break;
	                case 0x1E:
	                    rc = HDR_blend_get_adv_param(hdrInstance, am_dig_audio_bw_step_time, &value);
	                    dataOut[2] = (U8)((value >> 8U) & 0xffU);
	                    *outLength = 3;
	                    break;
	                case 0x2C:
	                    rc = -1; // deprecated
	                    break;
	                case 0x2D:
	                    rc = -1; // deprecated
	                    break;
	                case 0x2E:
	                    rc = -1; // deprecated
	                    break;
	                case 0x2F:
	                    rc = -1; // deprecated
	                    break;
	                case 0x30:
	                    rc = -1; // deprecated
	                    break;
	                case 0x31:
	                    rc = -1; // deprecated
	                    break;
	                 default:
	                    LOG(CMD,1U, "Undefined adv Blend Param offset.");
	                    *outLength = 1;
	                    break;
	            }

	            dataOut[1] = (U8)(value & 0xffU);

	            if(rc < 0){
	                *outLength = 1;
	            }
	            break;
	        }
	        case (U8)SET_MRC_CNFG:
	            LOG(CMD,4096U, "received IBOC_CNTRL_CNFG->SET_MRC_CNFG");
	            dataOut[0] = (U8)SET_MRC_CNFG;
	            *outLength = 2;

	            if(dataIn[1] == (U8)0x01){
	                (void)HDR_mrc_enable(hdrInstance);
	                dataOut[1] = 1;
	            } else {
	                (void)HDR_mrc_disable(hdrInstance);
	                dataOut[1] = 0;
	            }
	            break;
	        case (U8)GET_MRC_CNFG:
	            LOG(CMD,4095U, "received IBOC_CNTRL_CNFG->GET_MRC_CNFG");
	            dataOut[0] = (U8)GET_MRC_CNFG;
	            *outLength = 2;

	            HDBOOL mrcEnabled = false;
	            (void)HDR_mrc_enabled(hdrInstance, &mrcEnabled);

	            if(mrcEnabled == true){
	                dataOut[1] = 0x1;
	            } else {
	                dataOut[1] = 0x0;
	            }
	            break;
	        case (U8)SET_FILTERED_DSQM_VALUE:
	            LOG(CMD,8192U, "received IBOC_CNTRL_CNFG->SET_FILTERED_DSQM_VALUE");
	            dataOut[0] = (U8)SET_FILTERED_DSQM_VALUE;

	            U32 filtDsqm = dataIn[1] | ((U32)dataIn[2] << 8);
	            (void)HDR_set_filt_dsqm(hdrInstance, filtDsqm);

	            *outLength = 1;
	            break;
	        case (U8)SET_DSQM_PARAM:
	        {
	            LOG(CMD,8192U, "received IBOC_CNTRL_CNFG->SET_DSQM_PARAM");
	            dataOut[0] = (U8)SET_DSQM_PARAM;
	            S32 rc = -1;
	            if(dataIn[1] == (U8)0x00){ // Re-initialize
	                if(dataIn[2] == (U8)0x78){
	                    rc = CMD_cb_bbp_reset_dsqm_time_const();
	                } else {
	                    if(dataIn[2] == (U8)0x7E){
	                        rc = CMD_cb_bbp_reset_dsqm_seek_thresh();
	                    }
	                }
	            } else { // Set parameter
	                if(dataIn[2] == (U8)0x78){
	                    rc = HDR_set_dsqm_filt_time_const(hdrInstance, dataIn[3]);
	                } else {
	                    if(dataIn[2] == (U8)0x7E){
		                    U32 thresh = dataIn[3] | ((U32)dataIn[4] << 8);
		                    rc = CMD_cb_bbp_set_dsqm_seek_thresh(thresh);
	                    }
	                }
	            }
	            dataOut[1] = (U8)(rc == 0);
	            *outLength = 2;
	            break;
	        }
	        case (U8)GET_DSQM_PARAM:
	        {
	            LOG(CMD,8192U, "received IBOC_CNTRL_CNFG->GET_DSQM_PARAM");
	            dataOut[0] = (U8)GET_DSQM_PARAM;
	            S32 rc = -1;

	            U32 paramValue = 0;
	            if(dataIn[1] == (U8)0x78){
	                rc = HDR_get_dsqm_filt_time_const(hdrInstance, &paramValue);
	                dataOut[1] = (U8)(paramValue & 0xffU);
	                *outLength = 2;
	            } else {
	                if(dataIn[1] == (U8)0x7E){
	                    rc = CMD_cb_bbp_get_dsqm_seek_thresh(&paramValue);
	                    dataOut[1] = (U8)(paramValue & 0xffU);
	                    dataOut[2] = (U8)((paramValue >> 8U) & 0xffU);
	                    *outLength = 3;
	                }
	            }

	            if(rc != 0){
	              *outLength = 1;
	            }
	            break;
	        }
	        case (U8)SET_ALIGN_PARAMETERS:
	        {
	            LOG(CMD,16384U, "received IBOC_CNTRL_CNFG->SET_ALIGN_PARAMETERS");
	            dataOut[0] = (U8)SET_ALIGN_PARAMETERS;
	            dataOut[1] = 0x01;

	            CMD_auto_alignment_config_t autoAlignConfig;

				if((dataIn[1] & (U8)0x01) == 1U) {
					autoAlignConfig.am_auto_time_align_enabled = true;
				}else{
					autoAlignConfig.am_auto_time_align_enabled = false;
				}
				if(((dataIn[1] >> 1U) & (U8)0x1) == 1U) {
					autoAlignConfig.fm_auto_time_align_enabled = true;
				}else{
					autoAlignConfig.fm_auto_time_align_enabled = false;
				}

				if((dataIn[3] & (U8)0x1) == 1U) {
					autoAlignConfig.am_auto_level_align_enabled = true;
				}else{
					autoAlignConfig.am_auto_level_align_enabled = false;
				}
				if(((dataIn[3] >> 1U) & (U8)0x1) == 1U) {
					autoAlignConfig.fm_auto_level_align_enabled = true;
				}else{
					autoAlignConfig.fm_auto_level_align_enabled = false;
				}

	            if(CMD_cb_set_auto_alignment_config(hdrInstance, &autoAlignConfig) != 0){
	                dataOut[1] = 0x00;
	            }

	            *outLength = 2;
	            break;
	        }
	        case (U8)GET_ALIGN_PARAMETERS:
	        {
	            LOG(CMD,16384U, "received IBOC_CNTRL_CNFG->GET_ALIGN_PARAMETERS");
	            dataOut[0] = (U8)GET_ALIGN_PARAMETERS;

	            U32 offset = 1;

	            CMD_auto_alignment_config_t autoAlignConfig;
	            CB_auto_alignment_spec_t autoAlignSpec;

	            if(CMD_cb_get_auto_alignment_config(hdrInstance, &autoAlignConfig) < 0){
	                *outLength = 1;
	                dispatch_rc = CMD_UNSPECIFIED_ERROR;
					break;
	            }

	            if(CMD_cb_get_auto_alignment_spec(hdrInstance, &autoAlignSpec) < 0){
	                *outLength = 1;
	                dispatch_rc = CMD_UNSPECIFIED_ERROR;
					break;
	            }

	            dataOut[offset] = (*stCast.booltou8)(autoAlignConfig.am_auto_time_align_enabled);
	            dataOut[offset] |= (*stCast.booltou8)(autoAlignConfig.fm_auto_time_align_enabled) << 1;
				offset++;

	            dataOut[offset] = 0; // Byte 2 reserved
				offset++;

	            dataOut[offset] = (*stCast.booltou8)(autoAlignConfig.am_auto_level_align_enabled);
	            dataOut[offset] |= (*stCast.booltou8)(autoAlignConfig.fm_auto_level_align_enabled) << 1;
				offset++;

	            dataOut[offset] = (U8)(autoAlignSpec.amMaxPosRange & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((autoAlignSpec.amMaxPosRange >> 8U) & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((autoAlignSpec.amMaxPosRange >> 16U) & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((autoAlignSpec.amMaxPosRange >> 24U) & 0xffU);
				offset++;

	            dataOut[offset] = (U8)(autoAlignSpec.amMaxNegRange & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((autoAlignSpec.amMaxNegRange >> 8U) & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((autoAlignSpec.amMaxNegRange >> 16U) & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((autoAlignSpec.amMaxNegRange >> 24U) & 0xffU);
				offset++;

	            dataOut[offset] = (U8)(autoAlignSpec.fmMaxPosRange & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((autoAlignSpec.fmMaxPosRange >> 8U) & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((autoAlignSpec.fmMaxPosRange >> 16U) & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((autoAlignSpec.fmMaxPosRange >> 24U) & 0xffU);
				offset++;

	            dataOut[offset] = (U8)(autoAlignSpec.fmMaxNegRange & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((autoAlignSpec.fmMaxNegRange >> 8U) & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((autoAlignSpec.fmMaxNegRange >> 16U) & 0xffU);
				offset++;
	            dataOut[offset] = (U8)((autoAlignSpec.fmMaxNegRange >> 24U) & 0xffU);
				offset++;
#ifndef AVOID_QAC_1STEP_WARNING		// This comparison is always ''false'.
				if(offset != (U32)20) {
					(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "GET_ALIGN_PARAMETERS offset is incorrect.\n");
					dispatch_rc = CMD_UNSPECIFIED_ERROR;
					break;
				}
#endif
	            *outLength = offset;

	            break;
	        }
	        default:
	            LOG(CMD,128U, "Received UNKNOWN function (0x%X) for IBOC Control and Configuration host command.", funcCode);
	            dispatch_rc = CMD_UNSUPPORTED_OPCODE;
				break;
	    }
	}

    return dispatch_rc;
}

static void procGetStatus(HDR_instance_t *hdrInstance, U8* dataOut, U32 *outLength)
{
    BBP_tune_select_t tuneSelect;
    if(CMD_cb_bbp_get_tune_select(hdrInstance, &tuneSelect) != 0){
        *outLength = 1;
    }
	else {
	    U32 offset = 0;

	    dataOut[offset] = (U8)tuneSelect.band;
		offset++;
	    dataOut[offset] = (U8)HDR_get_primary_service_mode(hdrInstance);
		offset++;
	    dataOut[offset] = 0; // reserved
		offset++;
	    dataOut[offset] = (*stCast.booltou8)(HDR_hd_signal_acquired(hdrInstance));
		offset++;
	    dataOut[offset] = 0; // reserved
		offset++;
	    dataOut[offset] =(*stCast.booltou8)(HDR_sis_crc_ok(hdrInstance));
		offset++;
		if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
			dataOut[offset] = (*stCast.booltou8)(HDR_digital_audio_acquired(hdrInstance));
		} else {
			dataOut[offset] = 0;
		}
		offset++;
	    dataOut[offset] = 0; // reserved
		offset++;

	    U32 blend_control = 0;
	    (void)HDR_test_get_raw_tx_blend_control(hdrInstance, &blend_control);
	    dataOut[offset] = (U8)((blend_control & 2U) == 2U);
		offset++;

	    HDR_audio_codec_mode_t codecMode = HDR_AUDIO_CODEC_MODE0;
	    (void)HDR_get_codec_mode(hdrInstance, &codecMode);
	    dataOut[offset] = (U8)codecMode;
		offset++;

	    U32 tx_gain = 0;
	    (void)HDR_get_tx_dig_audio_gain(hdrInstance, &tx_gain);
	    dataOut[offset] = (U8)(tx_gain & 0xffU);
		offset++;

	    dataOut[offset] = (U8)(blend_control & 0xffU);
		offset++;

	    HDR_program_bitmap_t availablePrograms;
	    availablePrograms.all = 0;
	    (void)HDR_get_available_programs(hdrInstance, &availablePrograms);
	    dataOut[offset] = availablePrograms.all;
		offset++;


	    HDR_program_types_t programTypes;
	    (void)(*stOsal.osmemset)((void*)&programTypes, (S8)0, (U32)sizeof(HDR_program_types_t));
	    if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
	        (void)HDR_get_program_types(hdrInstance, &programTypes);
	    }
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)programTypes.value, HDR_MAX_NUM_PROGRAMS);
	    offset += (U32)8;

	    HDR_program_t playingProgram = HDR_PROGRAM_HD1;
	    if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
	        (void)HDR_get_playing_program(hdrInstance, &playingProgram);
	    }
	    dataOut[offset] = (U8)playingProgram;
		offset++;

	    HDR_audio_quality_report_t audioQualityReport;
	    (void)(*stOsal.osmemset)((void*)&audioQualityReport, (S8)0, (U32)sizeof(HDR_audio_quality_report_t));
	    if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
	        (void)HDR_get_audio_quality_report(hdrInstance, &audioQualityReport);
	    }
	    dataOut[offset] = (U8)(audioQualityReport.filt_quality_indicator & 0xffU);
		offset++;

	    dataOut[offset] = 0; // Conditional access no longer supported
		offset++;
	    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)9); // reserved
	    offset += (U32)9;

	    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)8); // SIS acq time not supported / Audio acq time not supported / reserved
	    offset += (U32)8;

	    dataOut[offset] = (U8)(audioQualityReport.quality_indicator & 0xffU);
		offset++;

	    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)63); // reserved
	    offset += (U32)63;

	    U32 raw_snr = 0;
	    (void)HDR_get_raw_snr(hdrInstance, &raw_snr);
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&raw_snr, (U32)4);
	    offset += (U32)4;

	    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)44); // Blend count not supported / reserved
	    offset += (U32)44;

	    HDR_test_ber_t ber;
	    (void)(*stOsal.osmemset)((void*)&ber, (S8)0, (U32)sizeof(HDR_test_ber_t));

	    if(HDR_test_ber_mode_enabled(hdrInstance) == true){
	        (void)HDR_test_get_ber(hdrInstance, &ber);
	    }

	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&ber.p4_bit_errors, (U32)4);
	    offset += (U32)4;
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&ber.p4_bits_tested, (U32)4);
	    offset += (U32)4;

	    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)0, (U32)32); // reserved
	    offset += (U32)32;

	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&audioQualityReport.frame_count, (U32)4);
	    offset += (U32)4;
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&audioQualityReport.core_errors, (U32)4);
	    offset += (U32)4;
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&audioQualityReport.enh_errors, (U32)4);
	    offset += (U32)4;

	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&ber.pids_block_errors, (U32)4);
	    offset += (U32)4;
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&ber.pids_blocks_tested, (U32)4);
	    offset += (U32)4;
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&ber.pids_bit_errors, (U32)4);
	    offset += (U32)4;
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&ber.pids_bits_tested, (U32)4);
	    offset += (U32)4;
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&ber.p3_bit_errors, (U32)4);
	    offset += (U32)4;
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&ber.p3_bits_tested, (U32)4);
	    offset += (U32)4;
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&ber.p2_bit_errors, (U32)4);
	    offset += (U32)4;
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&ber.p2_bits_tested, (U32)4);
	    offset += (U32)4;
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&ber.p1_bit_errors, (U32)4);
	    offset += (U32)4;
	    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&ber.p1_bits_tested, (U32)4);
	    offset += (U32)4;

#ifndef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
		if(offset != 245) {	// This comparison is always false.
			(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "procGetStatus offset is incorrect.\n");
			return;
		}
#endif
	    *outLength = offset;
	}
}

static CMD_dispatch_rc_t procIbocDiagnostics(HDR_instance_t* hdrInstance, const U8* dataIn, U8* dataOut, U32* outLength)
{
	CMD_dispatch_rc_t dispatch_rc = CMD_DISPATCH_OK;
    U32 funcCode = (U32)*dataIn; //BYTE 0 specifies the function code

    dataOut[0] = (U8)funcCode;
    *outLength = 1;

    switch(funcCode){
		case (U32)GET_IBOC_COUNTERS:
        {
            LOG(CMD,1U, "Received IBOC_DIAGNOSTICS->GET_IBOC_COUNTERS");
            U32 offset =0;
            dataOut[offset] = (U8)funcCode;
			offset++;
            dataOut[offset] = 0;
			offset++;
            dataOut[offset] = 0;//Bytes 1-2 SIS
			offset++;
            dataOut[offset] = 0;
			offset++;
            dataOut[offset] = 0;// Bytes 3-4 Audio Acquisition Time
			offset++;
            dataOut[offset] = 0;
			offset++;
            dataOut[offset] = 0;
			offset++;
            dataOut[offset] = 0;
			offset++;
            dataOut[offset] = 0;// Blend Count;
			offset++;
            *outLength = offset;
            break;

        }
        case (U32)GET_AUDIO_FER_STATUS:
        {
            LOG(CMD,1U, "Received IBOC_DIAGNOSTICS->GET_AUDIO_FER_STATUS");
            U32 offset =0;
            dataOut[offset] = (U8)funcCode;
			offset++;
            HDR_audio_quality_report_t audioQualityReport;
            (void)(*stOsal.osmemset)((void*)&audioQualityReport, (S8)0, (U32)sizeof(HDR_audio_quality_report_t));

			if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
				(void)HDR_get_audio_quality_report(hdrInstance, &audioQualityReport);
			}

            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&audioQualityReport.frame_count, 4);
            offset += (U32)4;
            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&audioQualityReport.core_errors, 4);
            offset += (U32)4;
            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&audioQualityReport.enh_errors, 4);
            offset += (U32)4;
            *outLength = offset;
            break;

        }
        case (U32)GET_BER:
        {
            LOG(CMD,1U, "Received IBOC_DIAGNOSTICS->GET_BER");
            HDR_test_ber_t ber;
            S32 rc = HDR_test_get_ber(hdrInstance, &ber);
            U32 offset = 0;
            if(rc < 0){
                *outLength = 0;
                dispatch_rc = CMD_UNSPECIFIED_ERROR;
				break;
            }

            dataOut[offset] = (U8)(funcCode & 0xffU);
			offset++;

            dataOut[offset] = (U8)(ber.pids_block_errors & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.pids_block_errors >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.pids_block_errors >> 16U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.pids_block_errors >> 24U) & 0xffU);
			offset++;

            dataOut[offset] = (U8)(ber.pids_blocks_tested & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.pids_blocks_tested >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.pids_blocks_tested >> 16U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.pids_blocks_tested >> 24U) & 0xffU);
			offset++;

            dataOut[offset] = (U8)(ber.pids_bit_errors & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.pids_bit_errors >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.pids_bit_errors >> 16U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.pids_bit_errors >> 24U) & 0xffU);
			offset++;

            dataOut[offset] = (U8)(ber.pids_bits_tested & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.pids_bits_tested >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.pids_bits_tested >> 16U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.pids_bits_tested >> 24U) & 0xffU);
			offset++;

            dataOut[offset] = (U8)(ber.p3_bit_errors & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p3_bit_errors >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p3_bit_errors >> 16U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p3_bit_errors >> 24U) & 0xffU);
			offset++;

            dataOut[offset] = (U8)(ber.p3_bits_tested & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p3_bits_tested >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p3_bits_tested >> 16U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p3_bits_tested >> 24U) & 0xffU);
			offset++;

            dataOut[offset] = (U8)(ber.p2_bit_errors & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p2_bit_errors >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p2_bit_errors >> 16U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p2_bit_errors >> 24U) & 0xffU);
			offset++;

            dataOut[offset] = (U8)(ber.p2_bits_tested & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p2_bits_tested >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p2_bits_tested >> 16U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p2_bits_tested >> 24U) & 0xffU);
			offset++;

            dataOut[offset] = (U8)(ber.p1_bit_errors & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p1_bit_errors >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p1_bit_errors >> 16U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p1_bit_errors >> 24U) & 0xffU);
			offset++;

            dataOut[offset] = (U8)(ber.p1_bits_tested & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p1_bits_tested >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p1_bits_tested >> 16U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p1_bits_tested >> 24U) & 0xffU);
			offset++;

            dataOut[offset] = (U8)(ber.p4_bit_errors & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p4_bit_errors >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p4_bit_errors >> 16U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p4_bit_errors >> 24U) & 0xffU);
			offset++;

            dataOut[offset] = (U8)(ber.p4_bits_tested & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p4_bits_tested >> 8U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p4_bits_tested >> 16U) & 0xffU);
			offset++;
            dataOut[offset] = (U8)((ber.p4_bits_tested >> 24U) & 0xffU);
			offset++;

            *outLength = offset;
            break;
        }
        case (U32)ENABLE_BER:
            LOG(CMD,1U, "Received IBOC_DIAGNOSTICS->ENABLE_BER");
            if(dataIn[1] == (U8)1){
                (void)HDR_test_enable_ber_mode(hdrInstance);
            } else {
                (void)HDR_test_disable_ber_mode(hdrInstance);
            }
            break;
        case (U32)RESET_MEASUREMENTS:
            LOG(CMD,1U, "Received IBOC_DIAGNOSTICS->RESET_MEASUREMENTS");
            if(hdrInstance->instance_type != HDR_DEMOD_ONLY_INSTANCE) {
                (void)HDR_test_reset_ber(hdrInstance);
            }
            if(hdrInstance->instance_type == HDR_FULL_INSTANCE) {
                (void)HDR_test_reset_audio_errors(hdrInstance);
            }
            break;
        case (U32)SET_BER_PATTERN:
            LOG(CMD,1U, "Received IBOC_DIAGNOSTICS->SET_BER_PATTERN");
            *outLength = 1;
            break;
        case (U32)BER_TABLE_DOWNLOAD:
            LOG(CMD,1U, "Received IBOC_DIAGNOSTICS->BER_TABLE_DOWNLOAD");
            *outLength = 1;
            break;
        case (U32)GET_BER_PATTERN_STATUS:
            LOG(CMD,1U, "Received IBOC_DIAGNOSTICS->GET_BER_PATTERN_STATUS");
            *outLength = 1;
            break;
        case (U32)GET_MRC_STATUS:
        {
            LOG(CMD,1U, "Received IBOC_DIAGNOSTICS->GET_MRC_STATUS");
            dataOut[0] = (U8)GET_MRC_STATUS;

            HDBOOL mrcEnabled = false;
            U32 offset = 1;

            S32 rc = HDR_mrc_enabled(hdrInstance, &mrcEnabled);

            if(rc == (S32)HDR_ERROR_INSTANCE_TYPE){
                dispatch_rc = CMD_UNSUPPORTED_INSTANCE;
				break;
            }

            dataOut[offset] = (*stCast.booltou8)(mrcEnabled);
			offset++;

            if(mrcEnabled == false){
                *outLength = offset;
                dispatch_rc = CMD_DISPATCH_OK;
				break;
            }

            HDR_instance_t* mrcSlaveInstance = CMD_cb_bbp_get_hdr_instance(2);
			if(mrcSlaveInstance == NULL) {
				(*pfnHdrLog)(eTAG_CDM, eLOG_WRN, "There is no MRC slave instance.\n");
				dispatch_rc = CMD_UNSUPPORTED_INSTANCE;
				break;
			}

            dataOut[offset] = (*stCast.booltou8)(HDR_demod_hd_signal_acquired(hdrInstance));
            dataOut[offset] |= (*stCast.booltou8)(HDR_demod_hd_signal_acquired(mrcSlaveInstance)) << 1;
			offset++;

            dataOut[offset] = HDR_get_mrc_demod_active_state(hdrInstance);
            dataOut[offset] |= (*stCast.booltou8)(HDR_test_mrc_demod_enabled(hdrInstance)) << 4;
            dataOut[offset] |= (*stCast.booltou8)(HDR_test_mrc_demod_enabled(mrcSlaveInstance)) << 5;
			offset++;

            U32 cdN0 = 0;
            (void)HDR_get_demod_cdno(hdrInstance, &cdN0);

            dataOut[offset] = (U8)(cdN0 & 0xffU);
			offset++;

            cdN0 = 0;
            (void)HDR_get_demod_cdno(mrcSlaveInstance, &cdN0);
            dataOut[offset] = (U8)(cdN0 & 0xffU);
			offset++;

            (void)HDR_get_cdno(hdrInstance, &cdN0);
            dataOut[offset] = (U8)(cdN0 & 0xffU);
			offset++;

            U32 filt_dsqm = 0;
            (void)HDR_get_demod_filt_dsqm(hdrInstance, &filt_dsqm);

            dataOut[offset] = (U8)(filt_dsqm & 0xffU);
			offset++;
            dataOut[offset] = (U8)((filt_dsqm >> 8U) & 0xffU);
			offset++;

            filt_dsqm = 0;
            (void)HDR_get_demod_filt_dsqm(mrcSlaveInstance, &filt_dsqm);
            dataOut[offset] = (U8)(filt_dsqm & 0xffU);
			offset++;
            dataOut[offset] = (U8)((filt_dsqm >> 8U) & 0xffU);
			offset++;

            filt_dsqm = 0;
            (void)HDR_get_filt_dsqm(hdrInstance, &filt_dsqm);

            dataOut[offset] = (U8)(filt_dsqm & 0xffU);
			offset++;
            dataOut[offset] = (U8)((filt_dsqm >> 8U) & 0xffU);
			offset++;

            S32 signal_offset = 0;
            (void)HDR_test_mrc_alignment_offset(hdrInstance, &signal_offset);
            signal_offset = signal_offset / 2160; // convert to offset measured in symbols
            if(signal_offset < 0){
                signal_offset = -signal_offset;
            }

            dataOut[offset] = (U8)(signal_offset);
			offset++;

            *outLength = offset;
            break;
        }
        case (U32)SET_MRC_DEMOD_CNTRL:
        {
            LOG(CMD,1U, "Received IBOC_DIAGNOSTICS->GET_MRC_STATUS");
            dataOut[0] = (U8)SET_MRC_DEMOD_CNTRL;
            *outLength = 1;

            HDBOOL mrcEnabled = false;
            S32 rc = HDR_mrc_enabled(hdrInstance, &mrcEnabled);

            if(rc == (S32)HDR_ERROR_INSTANCE_TYPE){
                dispatch_rc = CMD_UNSUPPORTED_INSTANCE;
				break;
            }

            if(mrcEnabled == false){
                dispatch_rc = CMD_DISPATCH_OK;
				break;
            }

            HDR_instance_t* mrcSlaveInstance = CMD_cb_bbp_get_hdr_instance(2);
			if(mrcSlaveInstance == NULL) {
				(*pfnHdrLog)(eTAG_CDM, eLOG_WRN, "There is no MRC slave instance.\n");
				dispatch_rc = CMD_UNSUPPORTED_INSTANCE;
				break;
			}

            if((dataIn[1] & (U8)0x01) != (U8)0){
                (void)HDR_test_mrc_demod_enable(hdrInstance);
            } else {
                (void)HDR_test_mrc_demod_disable(hdrInstance);
            }

            if((dataIn[1] & (U8)0x02) != (U8)0){
                (void)HDR_test_mrc_demod_enable(mrcSlaveInstance);
            } else {
                (void)HDR_test_mrc_demod_disable(mrcSlaveInstance);
            }
            break;
        }
        default:
            LOG(CMD,1U, "Received UNKNOWN function (0x%X) for IBOC_DIAGNOSTICS host command.", funcCode);
            dispatch_rc = CMD_UNSUPPORTED_OPCODE;
			break;
    }

    return dispatch_rc;
}


