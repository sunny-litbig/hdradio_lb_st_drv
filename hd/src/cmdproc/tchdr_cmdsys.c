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
 * file cmdSys.c
 */
#include <string.h>
#include "tchdr_common.h"			// To avoid redundant condition warnings of codesonar
#include "tchdr_log.h"
#include "tchdr_cmdopcode.h"
#include "hdrCore.h"
#include "hdrPsd.h"
#include "hdrTest.h"
#include "hdrAudio.h"
#include "hdrBlend.h"
#include "tchdr_std.h"
#include "tchdr_cmdtune.h"
#include "tchdr_cmdsys.h"
#include "tchdr_cmdcallbacks.h"

#if 0	// This enum is not used now. When using this, uncomment it.
enum{
    CMD_SYS_VALID = 0,
    CMD_SYS_INVALID = 1
};
#endif

/**
 * Table 5-12: Sys_Cntrl_Cnfg Command (opcode 0x83) Function Definitions
 */
enum SYS_CTRL_CONFIG_FUNC_CODES{
    //RESERVED 0x0
    GET_ACTIVATED_SERVICES  = 0x1,
    SET_ACTIVATED_SERVICES  = 0x2,
    GET_SUPPORTED_SERVICES  = 0x3,
    //RESERVED 0x4-0x7
    GET_CONFIG_INFO         = 0x8,
    GET_SYS_CONFIG          = 0x9,
    SET_SYS_CONFIG          = 0xA,
    //RESERVED 0xB
    SET_SYS_CONFIG_PARAM    = 0xC,
    GET_SYS_CONFIG_PARAM    = 0xD
};

enum SYS_DIAGNOSTICS_FUNC_CODES{
    // RESERVED  0x00
    SPLIT_DIGITAL_ANALOG_AUDIO = 0x01,
    // RESERVED  0x02 - 0x09
    SET_DIAGNOSTICS_TONE = 0x0A

};

static CMD_dispatch_rc_t procSysCtrlConfig(HDR_instance_t* hdrInstance, const U8* dataIn, U32 inLength, U8* dataOut, U32* outLength);

CMD_dispatch_rc_t SYS_procHostCommand(HDR_instance_t* hdrInstance, CMD_opcode_t opCode, const U8* dataIn, U32 inLength, U8* dataOut, U32* outLength)
{
    CMD_dispatch_rc_t dispatch_rc = CMD_DISPATCH_OK;

    switch(opCode){
        case SYS_VERSION:
    	{
			U32 strlength=0;
            LOG(CMD,32U, "Received SYS_VERSION");
            dataOut[0] = 0x01;
            strlength = (*stOsal.osstrlen)(HDR_get_version_string());
            (void)(*stOsal.osstrncpy)((S8*)&dataOut[1], HDR_get_version_string(), strlength);
            *outLength = (*stArith.u32add)(strlength, 2U); // add extra count for null-charecter
             break;
        }
        case SYS_TUNE:
            (void)SYS_procTune(hdrInstance, dataIn, inLength, dataOut, outLength);
            break;
        case SYS_CNTRL_CNFG:
            //LOG(CMD,32U, "Received SYS_CTRL_CONFIG");
            dispatch_rc = procSysCtrlConfig(hdrInstance, dataIn, inLength, dataOut, outLength);
            break;
        case SYS_AUDIO_CNTRL:
            LOG(CMD,32U, "Received SYS_AUDIO_CNTRL");
            *outLength = 0;
            break;
        case SYS_DIAGNOSTICS:
        {
            U32 funcCode = dataIn[0];
            switch(funcCode){
                case (U32)SPLIT_DIGITAL_ANALOG_AUDIO:
                    LOG(CMD,32U, "Received SYS_DIAGNOSTICS->SPLIT_DIGITAL_ANALOG_LEFT_RIGHT");

                    dataOut[0] = (U8)SPLIT_DIGITAL_ANALOG_AUDIO;

                    if(dataIn[1] == (U8)1){
                        (void)CMD_cb_bbp_enable_audio_split_mode(hdrInstance);
                    } else {
                        (void)CMD_cb_bbp_disable_audio_split_mode(hdrInstance);
                    }
                    *outLength = 1;
                    break;
                case (U32)SET_DIAGNOSTICS_TONE:
                    LOG(CMD,32U, "Received SYS_DIAGNOSTICS->SPLIT_DIGITAL_ANALOG_LEFT_RIGHT");
                    dataOut[0] = (U8)SET_DIAGNOSTICS_TONE;
                    // Not supported
                    *outLength = 1;
                    break;
                default:
                    LOG(CMD,32U, "Received UNKNOWN function (0x%X) for Sys_Diagnostics(0x8D) host command.", funcCode);
                    dispatch_rc = CMD_UNSUPPORTED_OPCODE;
                    break;
            }
            break;
        }
        default:
            LOG(CMD,32U, "Received unsupported OP Code(0x%X) from host.", opCode);
            dispatch_rc = CMD_UNSUPPORTED_OPCODE;
            break;
    }

    return dispatch_rc;
}

static CMD_dispatch_rc_t procSysCtrlConfig(HDR_instance_t* hdrInstance, const U8* dataIn, U32 inLength, U8* dataOut, U32* outLength)
{
    U32 funcCode;

	if((hdrInstance == NULL) || (dataIn == NULL)) {
		(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "procSysCtrlConfig input data is null.\n");
		return CMD_UNSPECIFIED_ERROR;
    }
    funcCode = (U32)*dataIn; //BYTE 0 specifies the function code

    switch(funcCode){
        case (U32)GET_ACTIVATED_SERVICES:
        {
            LOG(CMD,64U, "Received SysCntrlCnfg->GET_ACTIVATED_SERVICES");
            U32 offset = 0;

            dataOut[offset] = (U8)GET_ACTIVATED_SERVICES;
			offset++;

            BBP_services_t bbpServices;
#ifdef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
			(void)CMD_cb_bbp_get_activated_services(hdrInstance, &bbpServices);
#else
            if(CMD_cb_bbp_get_activated_services(hdrInstance, &bbpServices) != 0){
                *outLength = 1;
                break;
            }
#endif
            dataOut[offset] = bbpServices.byte0.all;
			offset++;
            dataOut[offset] = bbpServices.byte1.all;
			offset++;
            dataOut[offset] = bbpServices.byte2.all;
			offset++;
            dataOut[offset] = bbpServices.byte3.all;
			offset++;
            *outLength = offset;
            break;
        }
        case (U32)SET_ACTIVATED_SERVICES:
        {
            LOG(CMD,64U, "Received SysCntrlCnfg->SET_ACTIVATED_SERVICES");
            BBP_services_t bbpServices;

            dataOut[0] = (U8)SET_ACTIVATED_SERVICES;

            bbpServices.byte0.all = dataIn[1];
            bbpServices.byte1.all = dataIn[2];
            bbpServices.byte2.all = dataIn[3];
            bbpServices.byte3.all = dataIn[4];
#ifdef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
			(void)CMD_cb_bbp_set_activated_services(hdrInstance, &bbpServices);
			dataOut[1] = 0x01; // Valid
#else
            if(CMD_cb_bbp_set_activated_services(hdrInstance, &bbpServices) < 0){
                dataOut[1] = 0x00; // Invalid
            } else {
                dataOut[1] = 0x01; // Valid
            }
#endif
            *outLength = 2;
            break;
        }
        case (U32)GET_SUPPORTED_SERVICES:
        {
            LOG(CMD,64U, "Received SysCntrlCnfg->GET_ACTIVATED_SERVICES");
            U32 offset = 0;
            BBP_services_t bbpServices;
#ifdef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
			(void)CMD_cb_bbp_get_supported_services(&bbpServices);
#else
            if(CMD_cb_bbp_get_supported_services(&bbpServices) != 0){
                *outLength = 1;
                break;
            }
#endif
            dataOut[offset] = (U8)GET_SUPPORTED_SERVICES;
			offset++;
            dataOut[offset] = bbpServices.byte0.all;
			offset++;
            dataOut[offset] = bbpServices.byte1.all;
			offset++;
            dataOut[offset] = bbpServices.byte2.all;
			offset++;
            dataOut[offset] = bbpServices.byte3.all;
			offset++;
            *outLength = offset;
            break;
        }
        case (U32)GET_CONFIG_INFO:
        {
            LOG(CMD,64U, "Received SysCntrlCnfg->GET_CONFIG_INFO");
            U32 offset = 0;

            dataOut[offset] = (U8)GET_CONFIG_INFO;
			offset++;
            offset++; // Skip Configuration used

            BBP_sys_config_info_t configInfo;
            HDBOOL customExists = false;
			HDBOOL defaultExists;
#ifdef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
			(void)CMD_cb_bbp_get_sys_config_info(&configInfo, DEFAULT);
			defaultExists = true;
#else
            if(CMD_cb_bbp_get_sys_config_info(&configInfo, DEFAULT) == 0){
                defaultExists = true;
            } else {
				defaultExists = false;
            }
#endif
            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&configInfo, (U32)sizeof(BBP_sys_config_info_t));
            offset += (U32)sizeof(BBP_sys_config_info_t);
#ifdef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
			(void)CMD_cb_bbp_get_sys_config_info(&configInfo, CUSTOM);
			UNUSED(customExists);
#else
            if(CMD_cb_bbp_get_sys_config_info(&configInfo, CUSTOM) != 0){
                customExists = true;
            }
#endif
            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&configInfo, (U32)sizeof(BBP_sys_config_info_t));
            offset += (U32)sizeof(BBP_sys_config_info_t);
#ifndef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
            if(customExists != true && defaultExists != true){
                dataOut[1] = 0;
            }
#endif
            *outLength = offset;
            break;
        }
        case (U32)GET_SYS_CONFIG:
        {
			BBP_sys_config_t* sysConf = (BBP_sys_config_t*)(dataIn + 2);
            //LOG(CMD,64U, "Received SysCntrlCnfg->GET_SYS_CONFIG");
            dataOut[0] = (U8)GET_SYS_CONFIG;
            dataOut[1] = dataIn[1]; // Configuration type
#ifdef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
			(void)CMD_cb_bbp_get_sys_config(sysConf, (BBP_config_select_t)dataIn[1]);
#else
            if(CMD_cb_bbp_get_sys_config(sysConf, (BBP_config_select_t)dataIn[1]) != 0){
                *outLength = 1;
                break;
            }
#endif
            *outLength = (U32)2 + (U32)sizeof(BBP_sys_config_t);
            break;
        }
        case (U32)SET_SYS_CONFIG:
        {
            LOG(CMD,64U, "Received SysCntrlCnfg->SET_SYS_CONFIG");
            S32 rc;
            dataOut[0] = (U8)SET_SYS_CONFIG;

            switch(dataIn[1]){
                case 0x00: // Re-initialize actvie configuration to power-up state
                    rc = CMD_cb_bbp_reset_sys_config();
                    break;
                case 0x01: // Set active configuration to values define in subsequent data
                {
                	BBP_sys_config_t* sysConf = (BBP_sys_config_t*)(dataIn + 2);
                    rc = CMD_cb_bbp_set_sys_config(sysConf);
                }
                    break;
                default:
                    rc = -1;
                    break;
            }

            if(rc < 0){
                //If Byte 1 is not returned, then it indicates an error in the command
                *outLength = 1;
            } else {
                dataOut[1] = dataIn[1];
                *outLength = 2;
            }
            break;
        }
        case (U32)SET_SYS_CONFIG_PARAM:
        {
            LOG(CMD,64U, "Received SysCntrlCnfg->SET_SYS_CONFIG_PARAM");
            U32 paramValue;
            dataOut[0] = (U8)SET_SYS_CONFIG_PARAM;
            dataOut[1] = dataIn[1];

            if(inLength >= (U32)5){
                paramValue = ((U32)dataIn[2] << 24) | ((U32)dataIn[3] << 16) | ((U32)dataIn[4] << 8) | dataIn[5];
            } else {
                paramValue = dataIn[2];
            }

            if(CMD_cb_bbp_set_sys_config_param((BBP_sys_config_param_t)dataIn[1], paramValue) < 0){
                dataOut[2] = 0x00; // Invalid request
            } else {
                dataOut[2] = 0x01;
            }
            *outLength = 3;
             break;
        }
        case (U32)GET_SYS_CONFIG_PARAM:
        {
             LOG(CMD,64U, "Received SysCntrlCnfg->GET_SYS_CONFIG_PARAM");
             U32 value = 0;
             dataOut[0] = (U8)GET_SYS_CONFIG_PARAM;
             dataOut[1] = dataIn[1];
             if(CMD_cb_bbp_get_sys_config_param((BBP_sys_config_param_t)dataIn[1], &value) != 0){
                 *outLength = 1;
                 break;
             }
             (void)(*stOsal.osmemcpy)((void*)&dataOut[2], (void*)&value, (U32)sizeof(value));
             *outLength = 5;
             break;
        }
        default:
            LOG(CMD,64U, "Received UNKNOWN function (0x%X) for System Control Configuration host command.", funcCode);
            return CMD_UNSUPPORTED_OPCODE;
			break;
    }

    return CMD_DISPATCH_OK;
}
