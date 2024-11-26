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
 * file cmdSisPsd.c
 * brief Ibiquity command Processor Station Information Service (SIS) and Program Service Data (PSD) API
 */
#include <string.h>
#include "tchdr_common.h"			// For HDCDM_DBG
#include "tchdr_log.h"
#include "tchdr_cmdproc.h"
#include "tchdr_cmdopcode.h"
#include "tchdr_cmdsispsd.h"
#include "hdrSis.h"
#include "hdrPsd.h"
#include "hdrAas.h"

/**
 * Function Codes for OpCode 0x47, Ref: 2206 Table 9-16: Get_Ext_SIS_Data Command
 */
#define SIS_GET_STATION_MESSAGE             (0x01)    //Function Code for Get Station Message, see 2206 Table 9-16
#define SIS_GET_TIME_ZONE                   (0x02)
#define SIS_GET_LEAP_SECONDS                (0x03)
#define SIS_GET_UNIVERSAL_SHORT_NAME        (0x04)
#define SIS_GET_BASIC_DATA                  (0x05)
#define SIS_GET_SLOGAN                      (0x06)
#define SIS_GET_TX_EXCITER_CORE_VERSION     (0x07)
#define SIS_GET_TX_EXCITER_MANUF_VERSION    (0x08)
#define SIS_GET_TX_IMPORTER_CORE_VERSION    (0x09)
#define SIS_GET_TX_IMPORTER_MANUF_VERSION   (0x0A)

/**
 * Function Codes for OpCode 0x4B, Ref: 2206 Table 5-4: Get_Service_Info Command
 */
#define GET_SPECIFIC_AUDIO_PROGRAM_INFO                  (0x00)
#define GET_ALL_PROGRAMS_SOUND_PROCESSING		         (0x01)
#define GET_ALL_AUDIO_PROGRAMS_AND_DATA_SERVICES_LIST    (0x02)
#define GET_ALL_AUDIO_PROGRAMS_LIST	                     (0x03)
#define GET_ALL_DATA_SERVICES_LIST		                 (0x04)
#define GET_SPECIFIC_DATA_SERVICE_INFO                   (0x05)
#define GET_ALL_AUDIO_PROGRAMS_INFO		                 (0x06)
#define GET_ALL_DATA_SERVICE_INFO		                 (0x07)
#define GET_ALL_SERVICE_INFO		                     (0x08)

/**
 * Function Codes for OpCode 0x93, Ref: 2206 Table 5-4: PSD_Decode Command
 */
#define GET_PSD_DECODE             (0x02)
#define SET_PSD_CNFG_PARAM         (0x03)
#define GET_PSD_CNFG_PARAM         (0x04)

CMD_dispatch_rc_t SIS_procHostCommand(HDR_instance_t* hdrInstance, CMD_opcode_t opCode, const U8* dataIn,
                                      U8* dataOut, U32* outLength)
{
    U8 funcCode;

	if(dataIn == NULL) {
		(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "SIS_procHostCommand input data is null.\n");
		return CMD_UNSPECIFIED_ERROR;
    }

    funcCode = dataIn[0]; //BYTE 0 specifies the function code
    dataOut[0] = funcCode;
    *outLength = 1;

    switch(opCode){
        case SET_SIS_CNFG:
        {
            LOG(CMD,1U, "Received SET_SIS_CNFG");
            U32 i;
            HDR_sis_enabled_basic_types_t enabledTypes;
            enabledTypes.all = 0;

            for(i = 0; i < dataIn[1]; ++i){
                switch(dataIn[(U32)2 + i]){
                    case 0x00:
                        enabledTypes.stationId = 1;
                        break;
                    case 0x01:
                        enabledTypes.shortName = 1;
                        break;
                    case 0x02:
                        // reserved
                        break;
                    case 0x04:
                        enabledTypes.location = 1;
                        break;
                    default:
                        LOG(CMD,1U, "SET_SIS_CNFG: recevied undefined basic SIS type id");
                        break;
                }
            }

            (void)HDR_sis_enable_basic_types(hdrInstance, enabledTypes);
            *outLength = 0;
            break;
        }
        case GET_SIS_CNFG:
        {
            LOG(CMD,1U, "Received GET_SIS_CNFG");
            U32 count = 0, offset = 0;
            HDR_sis_enabled_basic_types_t enabledTypes;
            (void)HDR_sis_get_enabled_basic_types(hdrInstance, &enabledTypes);

            dataOut[offset] = (U8)GET_SIS_CNFG;
			offset++;

            offset++; // skip the number of types for now

            if(enabledTypes.stationId == (U8)1){
                ++count;
                dataOut[offset] = 0x00;
				offset++;
            }

            if(enabledTypes.shortName == (U8)1){
                ++count;
                dataOut[offset] = 0x01;
				offset++;
            }

            if(enabledTypes.location == (U8)1){
                ++count;
                dataOut[offset] = 0x04;
				offset++;
            }

            dataOut[1] = (U8)(count & 0xffU);

            *outLength += (U32)1 + count;
            break;
        }
        case GET_EXT_SIS_DATA:
            // Ref: 2206 Table 9-16: Get_Ext_SIS_Data  (0x47)
            //LOG(CMD,1U, "Received GET_EXT_SIS_DATA");
            switch (funcCode){
                case SIS_GET_STATION_MESSAGE:
                {
                    LOG(CMD,131072U, "received GET_EXT_SIS_DATA->SIS_GET_STATION_MESSAGE");

                    U32 offset = 0;

                    dataOut[offset] = SIS_GET_STATION_MESSAGE;
					offset++;

                    HDR_sis_station_msg_t stationMsg;

                    if((HDR_sis_get_station_message(hdrInstance, &stationMsg) == 0) && (stationMsg.status == HDR_SIS_NEW_DATA)){
                        dataOut[offset] = (U8)stationMsg.text_encoding;
						offset++;
                        dataOut[offset] = (*stCast.booltou8)(stationMsg.high_priority);
						offset++;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)stationMsg.text, stationMsg.length);
                        offset = (*stArith.u32add)(offset, stationMsg.length);
                        *outLength = offset;
                    }
                    break;
                }
                case SIS_GET_TIME_ZONE:
                {
                    LOG(CMD,131072U, "received GET_EXT_SIS_DATA->SIS_GET_TIME_ZONE");
                    U32 offset = 0;
                    HDR_sis_local_time_t local_time;
                    if(HDR_sis_get_local_time(hdrInstance, &local_time) == 0){
                        U32 uiUtcOffset = (U32)local_time.utc_offset & (U32)0x7FF;
                        local_time.utc_offset = (S32)uiUtcOffset; // Host only wants 11 bits

                        dataOut[offset] = SIS_GET_TIME_ZONE;
						offset++;
                        dataOut[offset] = (U8)local_time.utc_offset;
						offset++;
                        dataOut[offset] = (U8)((U32)local_time.utc_offset >> 8U);
						offset++;
                        dataOut[offset] = (U8)local_time.dst_schedule;
						offset++;
                        dataOut[offset] = (U8)local_time.dst_local;
						offset++;
                        dataOut[offset] = (*stCast.booltou8)(local_time.dst_in_effect);
						offset++;
                    }

                    *outLength = offset;
                    break;
                }
                case SIS_GET_LEAP_SECONDS:
                {
                    LOG(CMD,131072U, "received GET_EXT_SIS_DATA->SIS_GET_LEAP_SECONDS");
                    U32 offset = 0;
                    HDR_sis_leap_sec_t leap_sec;
                    if(HDR_sis_get_leap_sec(hdrInstance, &leap_sec) == 0){
                        dataOut[offset] = SIS_GET_LEAP_SECONDS;
						offset++;
                        dataOut[offset] = (U8)leap_sec.pending_offset;
						offset++;
                        dataOut[offset] = (U8)leap_sec.current_offset;
						offset++;
                        dataOut[offset] = (U8)((leap_sec.pending_offset_alfn >> 0U) & 0xffU);
						offset++;
                        dataOut[offset] = (U8)((leap_sec.pending_offset_alfn >> 8U) & 0xffU);
						offset++;
                        dataOut[offset] = (U8)((leap_sec.pending_offset_alfn >> 16U) & 0xffU);
						offset++;
                        dataOut[offset] = (U8)((leap_sec.pending_offset_alfn >> 24U) & 0xffU);
						offset++;
                    }
                    *outLength = offset;
                    break;
                }
                case SIS_GET_UNIVERSAL_SHORT_NAME:
                {
                    LOG(CMD,131072U, "received GET_EXT_SIS_DATA->SIS_GET_UNIVERSAL_SHORT_NAME");
                    HDR_sis_univ_name_t univName;
                    U32 offset = 0;
                    *outLength = 0;

                    if(HDR_sis_get_universal_name(hdrInstance, &univName) == 0){
                        dataOut[offset] = SIS_GET_UNIVERSAL_SHORT_NAME;
						offset++;
                        dataOut[offset] = (U8)univName.text_encoding;
						offset++;
                        dataOut[offset] = (*stCast.booltou8)(univName.append_fm);
						offset++;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)univName.text, univName.length);
                        offset = (*stArith.u32add)(offset, univName.length);
                        dataOut[offset] = (U8)'\0'; // null terminate the string
						offset++;
                        *outLength = offset;
                    }
                    break;
                }
                case SIS_GET_BASIC_DATA:
                {
                    LOG(CMD,131072U, "received GET_EXT_SIS_DATA->SIS_GET_BASIC_DATA");
                    HDR_sis_enabled_basic_types_t enabledTypes;
                    enabledTypes.all = 0;

                    U32 offset = 0;
                    U8* numTypes = NULL;

                    (void)HDR_sis_get_enabled_basic_types(hdrInstance, &enabledTypes);

                    dataOut[offset] = SIS_GET_BASIC_DATA;
					offset++;
                    dataOut[offset] = 0x28;
					offset++;
                    dataOut[offset] = 0x00;
					offset++;
                    dataOut[offset] = (U8)(HDR_sis_get_block_count(hdrInstance) & 0xffU);
					offset++;

                    HDR_sis_alfn_t alfn;

                    if(HDR_sis_get_alfn(hdrInstance, &alfn) != 0){
                        alfn.value = 0;
                        alfn.status = ALFN_ACQUIRING;
                    }

                    dataOut[offset] = (U8)alfn.status;
					offset++;
                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&alfn.value, (U32)sizeof(alfn.value));
                    offset += (U32)sizeof(alfn.value);

                    dataOut[offset] = (*stCast.booltou8)(HDR_sis_time_gps_locked(hdrInstance));
					offset++;

                    // save the location of Num_Types field; we'll update it as we go
                    numTypes = &dataOut[offset];
					offset++;
                    *numTypes = 0;

                    if(enabledTypes.stationId == (U8)1){
                        HDR_sis_station_id_t stationId;
                        (void)(*stOsal.osmemset)((void*)&stationId, (S8)0, (U32)sizeof(HDR_sis_station_id_t));
                        (void)HDR_sis_get_station_id(hdrInstance, &stationId);

                        U32 length = 0;

                        if((stationId.status == HDR_SIS_NEW_DATA) || (stationId.status == HDR_SIS_OLD_DATA)){
                            length = 4;
                        }

                        (*numTypes)++;
                        dataOut[offset] = 0x00; // station id
                        offset++;
                        dataOut[offset] = (U8)stationId.status;
						offset++;
                        dataOut[offset] = (U8)length;
                        offset++;
                        if(length > 0U) {
                        #ifdef USE_HDRLIB_3RD_CHG_VER
                            // Reverse byte order as required by 2206
                            U32 tempId = REVERSE_BYTES_32(stationId.all);
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&tempId, length);
						#else
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&stationId.all, length);
                        #endif
                        }
                        offset += length;
                    }

                    if(enabledTypes.shortName == (U8)1){
                        HDR_sis_short_name_t shortName;
                        (void)(*stOsal.osmemset)((void*)&shortName, (S8)0, (U32)sizeof(HDR_sis_short_name_t));

                        (void)HDR_sis_get_station_short_name(hdrInstance, &shortName);

                        (*numTypes)++;
                        dataOut[offset] = 0x01; // station short name
						offset++;

                        if((shortName.status != HDR_SIS_NO_DATA) && (shortName.status != HDR_SIS_ERROR)){
                            dataOut[offset] = (U8)shortName.status;
							offset++;
                            dataOut[offset] = (U8)(shortName.length & 0xffU);
							offset++;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)shortName.text, shortName.length);
                            offset = (*stArith.u32add)(offset, shortName.length);
                        } else {
                            // No short name; try universal name
                            HDR_sis_univ_name_t univName;
                            (void)(*stOsal.osmemset)((void*)&univName, (S8)0, (U32)sizeof(HDR_sis_univ_name_t));
                            (void)HDR_sis_get_universal_name(hdrInstance, &univName);

                            if(univName.length <= HDR_SIS_SHORT_NAME_MAX_LENGTH){
                                dataOut[offset] = (U8)univName.status;
                                offset++;
                                dataOut[offset] = (U8)(univName.length & 0xffU);
                                offset++;
                                if(univName.length > 0) {
                                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)univName.text, univName.length);
                                }
                                offset = (*stArith.u32add)(offset, univName.length);
                            }else {
                                dataOut[offset] = (U8)HDR_SIS_NO_DATA;
								offset++;
                                dataOut[offset] = 0;
								offset++;
                            }
                        }
                    }

                    if(enabledTypes.location == (U8)1){
                        U32 length = 0;
                        U32 locationWord = 0;
                        HDR_sis_station_location_t location = {0,};
                        U32 locationReadCount = 0;
                        HDR_sis_status_t status = HDR_SIS_NO_DATA;

                        (*numTypes)++;
                        (void)HDR_sis_get_station_location(hdrInstance, &location);

                        if((location.status == HDR_SIS_NEW_DATA) || (location.status == HDR_SIS_OLD_DATA)){
                            status = HDR_SIS_NEW_DATA; // New data until read account is 2 or greater
                            (void)HDR_sis_location_read_count(hdrInstance, &locationReadCount);

                            // Document RX_IDD_2206 Section 9.5.1.4
                            // Type_ID 0x04 - Station Location
                            // 2206 expects the location word to be in MS byte first order
                            // It's easier to contruct a LS byte order first and then convert
                            // The host expects altitude to be in units of (meters * 16) but the HD Library API provides just meters
                            // shift altitude 4 bits to left before adding to the location word to get the expected units
                            if((locationReadCount & (U32)0x1) == (U32)1){
                                // Give high portion when count is odd
                            #ifdef USE_HDRLIB_3RD_CHG_VER
                                locationWord = ((U32)1U << 31U) | ((((U32)location.latitude << 4U) | (((U32)location.altitude >> 8U) & 0x0FU)) << 5U);
							#else
                                locationWord = ((U32)1U << 31U) | ((((U32)location.latitude << 4U) | ((location.altitude >> 4U) & 0x0FU)) << 5U);
							#endif
                            } else {
                                // Give low portion when count is even
                                // Location fields are sign extended; make sure we account for that
                            #ifdef USE_HDRLIB_3RD_CHG_VER
                                locationWord = ((((U32)location.longitude << 4U) | (((U32)location.altitude >> 4U) & 0x0FU)) << 5U) & 0x7FFFFFFF;
							#else
                                locationWord = ((((U32)location.longitude << 4U) | ((((U32)location.altitude >> 4U) >> 4U) & 0x0FU)) << 5U) & (U32)0x7FFFFFFF;
							#endif
                            }

                            if(locationReadCount > (U32)2){
                                status = HDR_SIS_OLD_DATA;
                            }

                            length = 4;
                        }

                        // Always populate field information
                        dataOut[offset] = 0x04;
						offset++;
                        dataOut[offset] = (U8)status;
						offset++;
                        dataOut[offset] = (U8)(length & 0xffU);
						offset++;

                        // If there is data
                        if(length == (U32)4){
                            dataOut[offset] = (U8)((locationWord >> 24U) & 0xffU);
							offset++;
                            dataOut[offset] = (U8)((locationWord >> 16U) & 0xffU);
							offset++;
                            dataOut[offset] = (U8)((locationWord >> 8U) & 0xffU);
							offset++;
                            dataOut[offset] = (U8)(locationWord & 0xffU);
							offset++;
                        }
                    }
                    *outLength = offset;
                }
                break;
                case SIS_GET_SLOGAN:
                {
                    LOG(CMD,131072U, "received GET_EXT_SIS_DATA->SIS_GET_SLOGAN");
                    *outLength = 0;

                    HDR_sis_station_slogan_t slogan;
                    if(HDR_sis_get_station_slogan(hdrInstance, &slogan) == 0){
                        dataOut[0] = SIS_GET_SLOGAN;
                        dataOut[1] = (U8)slogan.text_encoding;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[2], (void*)slogan.text, slogan.length);
                        *outLength = slogan.length + (U32)2;
                    }
                }
                break;
                case SIS_GET_TX_EXCITER_CORE_VERSION:
                {
                    LOG(CMD,131072U, "received GET_EXT_SIS_DATA->SIS_GET_TX_EXCITER_CORE_VERSION");
                    HDR_sis_tx_ver_str_t coreVersion;
                    *outLength = 0;

                    if(HDR_sis_get_exciter_core_ver(hdrInstance, &coreVersion) == 0){
                        dataOut[0] = SIS_GET_TX_EXCITER_CORE_VERSION;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[1], (void*)coreVersion.string, coreVersion.length);
                        *outLength = (U32)1 + coreVersion.length;
                    }
                    break;
                }
                case SIS_GET_TX_EXCITER_MANUF_VERSION:
                {
                    LOG(CMD,131072U, "received GET_EXT_SIS_DATA->SIS_GET_TX_EXCITER_MANUF_VERSION");
                    HDR_sis_tx_manuf_ver_t manufVersion;
                    *outLength = 0;

                    if(HDR_sis_get_exciter_manuf_ver(hdrInstance, &manufVersion) == 0){
                        dataOut[0] = SIS_GET_TX_EXCITER_MANUF_VERSION;
                        dataOut[1] = (U8)manufVersion.left_most_mnf_id;
                        dataOut[2] = (U8)manufVersion.right_most_mnf_id;

                        (void)(*stOsal.osmemcpy)((void*)&dataOut[3], (void*)manufVersion.version_string.string, manufVersion.version_string.length);
                        *outLength = (U32)3 + manufVersion.version_string.length;
                    }
                    break;
                }
                case SIS_GET_TX_IMPORTER_CORE_VERSION:
                {
                    LOG(CMD,131072U, "received GET_EXT_SIS_DATA->SIS_GET_TX_IMPORTER_CORE_VERSION");
                    *outLength = 0;

                    HDR_sis_tx_ver_str_t coreVersion;

                    if(HDR_sis_get_importer_core_ver(hdrInstance, &coreVersion) == 0){
                        dataOut[0] = SIS_GET_TX_IMPORTER_CORE_VERSION;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[1], (void*)coreVersion.string, coreVersion.length);
                        *outLength = (U32)1 + coreVersion.length;
                    }
                    break;
                }
                case SIS_GET_TX_IMPORTER_MANUF_VERSION:
                {
                    LOG(CMD,131072U, "received GET_EXT_SIS_DATA->SIS_GET_TX_IMPORTER_MANUF_VERSION");
                    HDR_sis_tx_manuf_ver_t manufVersion;

                    if(HDR_sis_get_importer_manuf_ver(hdrInstance, &manufVersion) == 0){
                        dataOut[0] = SIS_GET_TX_IMPORTER_MANUF_VERSION;
                        dataOut[1] = (U8)manufVersion.left_most_mnf_id;
                        dataOut[2] = (U8)manufVersion.right_most_mnf_id;

                        (void)(*stOsal.osmemcpy)((void*)&dataOut[3], (void*)manufVersion.version_string.string, manufVersion.version_string.length);
                        *outLength = (U32)3 + manufVersion.version_string.length;
                    }
                    break;
                }
                default:
                    LOG(CMD,131072U, "GET_EXT_SIS_DATA: function code %x not recognised", funcCode);
                    *outLength = 0;
                break;
            }
            break;
        case FLUSH_SIS_DATA:
            LOG(CMD,131072U, "Received FLUSH_SIS_DATA");
            (void)HDR_sis_flush(hdrInstance);
            *outLength = 0;
            break;
        case GET_SERVICE_INFO:
            // See Table 5-4: Get_Service_Info Command (opcode 0x4B) Function Definitions
            //LOG(CMD,1U, "Received GET_SERVICE_INFO");
            switch (funcCode){
                case GET_SPECIFIC_AUDIO_PROGRAM_INFO:
                {
                    LOG(CMD,262144U, "received GET_SERVICE_INFO->GET_SPECIFIC_AUDIO_PROGRAM_INFO");

                    U32 offset = 0;
                    U32 programNumber = dataIn[1];
                    HDR_sis_program_info_t programInfo;
                    (void)(*stOsal.osmemset)((void*)&programInfo, (S8)0, (U32)sizeof(HDR_sis_program_info_t));

                    (void)HDR_sis_get_program_info(hdrInstance, (HDR_program_t)programNumber, &programInfo);

                    if(programInfo.status == HDR_SIS_NO_DATA){
                        *outLength = 0;
                        break;
                    }

                    dataOut[offset] = GET_SPECIFIC_AUDIO_PROGRAM_INFO;
					offset++;
                    dataOut[offset] = (U8)programNumber;
					offset++;
                    dataOut[offset] = (U8)programInfo.access;
					offset++;
                    dataOut[offset] = (U8)(programInfo.program_type & 0xffU);
					offset++;
                    dataOut[offset] = 0; // Reserved
					offset++;
                    dataOut[offset] = (U8)(programInfo.surround_sound & 0xffU);
					offset++;
                    dataOut[offset] = (U8)programInfo.status;
					offset++;

                    *outLength = offset;
                    break;
                }
                case GET_ALL_PROGRAMS_SOUND_PROCESSING:
                {
                    LOG(CMD,262144U, "received GET_SERVICE_INFO->GET_ALL_PROGRAMS_SOUND_PROCESSING");
                    U32 offset = 0;
                    U32 i;
                    HDR_sis_program_info_t programInfo;
                    (void)(*stOsal.osmemset)((void*)&programInfo, (S8)0, (U32)sizeof(HDR_sis_program_info_t));

                    dataOut[offset] = GET_ALL_PROGRAMS_SOUND_PROCESSING;
					offset++;
                    for(i = 0; i < HDR_MAX_NUM_PROGRAMS; ++i){
                        (void)HDR_sis_get_program_info(hdrInstance, (HDR_program_t)i, &programInfo);
                        if(programInfo.status != HDR_SIS_NO_DATA){
                            dataOut[offset] = (U8)(programInfo.surround_sound & 0xffU);
							offset++;
                        } else {
                            dataOut[offset] = 0xff;
							offset++;
                        }
                    }

                    // TODO BK - not sure why there are more than 8 programs
                    (void)(*stOsal.osmemset)((void*)&dataOut[offset], (S8)(-1), (U32)56);
                    offset += (U32)56;

                    *outLength = offset;
                    break;
                }
                case GET_ALL_AUDIO_PROGRAMS_AND_DATA_SERVICES_LIST:
                {
                    LOG(CMD,262144U, "received GET_SERVICE_INFO->GET_ALL_AUDIO_PROGRAMS_AND_DATA_SERVICES_LIST");
                    U32 i = 0, offset = 0;
                    HDR_sis_avail_programs_t programList;
                    HDR_sis_avail_data_services_t servicesList;

                    (void)HDR_sis_get_avail_programs_list(hdrInstance, &programList);

                    dataOut[offset] = GET_ALL_AUDIO_PROGRAMS_AND_DATA_SERVICES_LIST;
					offset++;
                    dataOut[offset] = (U8)(programList.program_count & 0xffU); // number of programs is in the first byte
					offset++;

                    for(i = 0; i < programList.program_count; ++i){
                        dataOut[offset] = (U8)programList.program[i].program_number;
						offset++;
                        dataOut[offset] = (U8)programList.program[i].status;
						offset++;
                    }

                    (void)HDR_sis_get_avail_data_serv_list(hdrInstance, &servicesList);

                    dataOut[offset] = (U8)(servicesList.service_count & 0xffU);
					offset++;

                    for(i = 0; i < servicesList.service_count; ++i){
                        dataOut[offset] = (U8)(servicesList.service[i].type & 0xffU);
						offset++;
                        dataOut[offset] = (U8)((servicesList.service[i].type >> 8U) & 0xffU);
						offset++;
                        dataOut[offset] = (U8)servicesList.service[i].status;
						offset++;
                    }
                    *outLength = offset;
                    break;
                }
                case GET_ALL_AUDIO_PROGRAMS_LIST:
                {
                    LOG(CMD,262144U, "received GET_SERVICE_INFO->GET_ALL_AUDIO_PROGRAMS_LIST");
                    U32 i = 0, offset = 0;
                    HDR_sis_avail_programs_t programList;

                    (void)HDR_sis_get_avail_programs_list(hdrInstance, &programList);

                    dataOut[offset] = GET_ALL_AUDIO_PROGRAMS_LIST;
					offset++;
                    dataOut[offset] = (U8)(programList.program_count & 0xffU); // number of programs is in the first byte
					offset++;

                    for(i = 0; i < programList.program_count; ++i){
                        dataOut[offset] = (U8)programList.program[i].program_number;
						offset++;
                        dataOut[offset] = (U8)programList.program[i].status;
						offset++;
                    }

                    *outLength = offset;
                    break;
                }
                case GET_ALL_DATA_SERVICES_LIST:
                {
                    LOG(CMD,262144U, "received GET_SERVICE_INFO->GET_ALL_DATA_SERVICES_LIST");
                    U32 i = 0, offset = 0;

                    HDR_sis_avail_data_services_t servicesList;

                    if(HDR_sis_get_avail_data_serv_list(hdrInstance, &servicesList) < 0){
                        servicesList.service_count = 0;
                    }

                    dataOut[offset] = GET_ALL_DATA_SERVICES_LIST;
					offset++;
                    dataOut[offset] = (U8)(servicesList.service_count & 0xffU);      // number of data services available is in the first byte
					offset++;
                    for(i = 0; i < servicesList.service_count; ++i){
                        dataOut[offset] = (U8)(servicesList.service[i].type & 0xffU);
						offset++;
                        dataOut[offset] = (U8)((servicesList.service[i].type >> 8U) & 0xffU);
						offset++;
                        dataOut[offset] = (U8)servicesList.service[i].status;
						offset++;
                    }
                    *outLength = offset;
                    break;
                }
                case GET_SPECIFIC_DATA_SERVICE_INFO:
                {
                    LOG(CMD,262144U, "received GET_SERVICE_INFO->GET_SPECIFIC_DATA_SERVICE_INFO");
                    U32 serviceType = 0,offset = 0;
                    HDR_sis_data_services_info_t serviceInfo;

                    serviceType = dataIn[1];
                    serviceType |= ((U32)dataIn[2] << 8);

                    (void)HDR_sis_get_data_services_type(hdrInstance, serviceType, &serviceInfo);

                    dataOut[offset] = GET_SPECIFIC_DATA_SERVICE_INFO;
					offset++;
                    dataOut[offset] = dataIn[1]; // Service type LSB
					offset++;
                    dataOut[offset] = dataIn[2]; // Service type MSB
					offset++;
                    dataOut[offset] = (U8)(serviceInfo.service_count & 0xffU);
					offset++;

                    U32 i;
                    for(i = 0; i < serviceInfo.service_count; ++i){
                        dataOut[offset] = (U8)(serviceInfo.service[i].service_type & 0xffU);
						offset++;
                        dataOut[offset] = (U8)(serviceInfo.service[i].service_type >> 8U);
						offset++;
                        dataOut[offset] = (U8)(serviceInfo.service[i].mime_type & 0xffU);
						offset++;
                        dataOut[offset] = (U8)(serviceInfo.service[i].mime_type >> 8U);
						offset++;
                        dataOut[offset] = (U8)serviceInfo.service[i].access;
						offset++;
                        dataOut[offset] = 0;
						offset++;
                        dataOut[offset] = (U8)serviceInfo.service[i].status;
						offset++;
                    }

                    *outLength = offset;
                    break;
                }
                case GET_ALL_AUDIO_PROGRAMS_INFO:
                {
                    LOG(CMD,262144U, "received GET_SERVICE_INFO->GET_ALL_AUDIO_PROGRAMS_INFO");
                    U32 offset = 0, i = 0;
                    HDR_sis_avail_programs_t programList;
                    HDR_sis_program_info_t programInfo;

                    (void)HDR_sis_get_avail_programs_list(hdrInstance, &programList);

                    dataOut[offset] = GET_ALL_AUDIO_PROGRAMS_INFO;
					offset++;
                    dataOut[offset] = (U8)(programList.program_count & 0xffU);
					offset++;

                    for(i = 0; i < programList.program_count; ++i){
                        (void)HDR_sis_get_program_info(hdrInstance, programList.program[i].program_number, &programInfo);

                        dataOut[offset] = (U8)programList.program[i].program_number;
						offset++;
                        dataOut[offset] = (U8)programInfo.access;
						offset++;
                        dataOut[offset] = (U8)(programInfo.program_type & 0xffU);
						offset++;
                        dataOut[offset] = 0; // Reserved
						offset++;
                        dataOut[offset] = (U8)(programInfo.surround_sound & 0xffU);
						offset++;
                        dataOut[offset] = (U8)programInfo.status;
						offset++;
                    }

                    *outLength = offset;
                    break;
                }
                case GET_ALL_DATA_SERVICE_INFO:
                {
                    LOG(CMD,262144U, "received GET_SERVICE_INFO->GET_ALL_DATA_SERVICE_INFO");
                    U32 i = 0, offset = 0;
                    HDR_sis_data_services_info_t servicesInfo;

                    if(HDR_sis_get_all_data_services(hdrInstance, &servicesInfo) < 0){
                        *outLength = 0;
                        break;
                    }

                    dataOut[offset] = GET_ALL_DATA_SERVICE_INFO;
					offset++;
                    dataOut[offset] = (U8)(servicesInfo.service_count & 0xffU);
					offset++;

                    for(i = 0; i < servicesInfo.service_count; ++i){
                        dataOut[offset] = (U8)(servicesInfo.service[i].service_type & 0xffU);
						offset++;
                        dataOut[offset] = (U8)(servicesInfo.service[i].service_type >> 8U);
						offset++;
                        dataOut[offset] = (U8)(servicesInfo.service[i].mime_type & 0xffU);
						offset++;
                        dataOut[offset] = (U8)(servicesInfo.service[i].mime_type >> 8U);
						offset++;
                        dataOut[offset] = (U8)servicesInfo.service[i].access;
						offset++;
                        dataOut[offset] = 0;
						offset++;
                        dataOut[offset] = (U8)servicesInfo.service[i].status;
						offset++;
                    }
                    *outLength = offset;
                    break;
                }
                case GET_ALL_SERVICE_INFO:
                {
                    LOG(CMD,262144U, "received GET_SERVICE_INFO->GET_ALL_SERVICE_INFO");
                    U32 i = 0, offset = 0;
                    HDR_sis_avail_programs_t programList;
                    HDR_sis_program_info_t programInfo;
                    HDR_sis_data_services_info_t servicesInfo;

                    (void)HDR_sis_get_avail_programs_list(hdrInstance, &programList);

                    dataOut[offset] = GET_ALL_SERVICE_INFO;
					offset++;
                    dataOut[offset] = (U8)(programList.program_count & 0xffU);
					offset++;

                    for(i = 0; i < programList.program_count; ++i){
                        (void)HDR_sis_get_program_info(hdrInstance, programList.program[i].program_number, &programInfo);

                        dataOut[offset] = (U8)programList.program[i].program_number;
						offset++;
                        dataOut[offset] = (U8)programInfo.access;
						offset++;
                        dataOut[offset] = (U8)(programInfo.program_type & 0xffU);
						offset++;
                        dataOut[offset] = 0; // Reserved
						offset++;
                        dataOut[offset] = (U8)(programInfo.surround_sound & 0xffU);
						offset++;
                        dataOut[offset] = (U8)programInfo.status;
						offset++;
                    }

                    if(HDR_sis_get_all_data_services(hdrInstance, &servicesInfo) < 0){
                        servicesInfo.service_count = 0;
                    }

                    dataOut[offset] = (U8)(servicesInfo.service_count & 0xffU);
					offset++;

                    for(i = 0; i < servicesInfo.service_count; ++i){
                        dataOut[offset] = (U8)(servicesInfo.service[i].service_type & 0xffU);
						offset++;
                        dataOut[offset] = (U8)(servicesInfo.service[i].service_type >> 8U);
						offset++;
                        dataOut[offset] = (U8)(servicesInfo.service[i].mime_type & 0xffU);
						offset++;
                        dataOut[offset] = (U8)(servicesInfo.service[i].mime_type >> 8U);
						offset++;
                        dataOut[offset] = (U8)servicesInfo.service[i].access;
						offset++;
                        dataOut[offset] = 0; // Reserved
						offset++;
                        dataOut[offset] = (U8)servicesInfo.service[i].status;
						offset++;
                    }
                    *outLength = offset;
                    break;
                }
                default:
                    LOG(CMD,262144U, "GET_SERVICE_INFO: function code 0x%x not recognised", funcCode);
                    return CMD_UNSUPPORTED_OPCODE;
					break;
            }
            break;
        case PSD_DECODE:
            switch (funcCode){
                case GET_PSD_DECODE:
                {
                    LOG(CMD,524288U, "received PSD_DECODE->GET_PSD_DECODE");
                    U32 offset = 0;

                    U8 program = (U8)HDR_PROGRAM_HD1;
                    HDR_psd_data_t psdData;
                    U32 programBitmask = dataIn[1];

                    HDR_psd_fields_t fieldBitmask;
                    fieldBitmask.all = dataIn[5];

                    U32 commentSubfields = dataIn[7];
                    U32 ufidSubfields = dataIn[8];
                    U32 commercialSubfields = dataIn[9];

                    HDR_psd_fields_t enabledFields;
					enabledFields.all = HDR_psd_get_enabled_fields(hdrInstance).all;

					(void)(*stOsal.osmemset)((void*)&psdData, (S8)0, (U32)sizeof(HDR_psd_data_t));

                    dataOut[offset] = GET_PSD_DECODE;
					offset++;

                    for(program = 0U; program < HDR_MAX_NUM_PROGRAMS; ++program){
                        if((programBitmask & ((U32)1 << program)) == 0U){
                            continue;
                        }

                        // Title
                        if((fieldBitmask.title == (U8)1) && (enabledFields.title == (U8)1)){
                            psdData.length = 0; // erase the old value
                            (void)HDR_psd_get_title(hdrInstance, (HDR_program_t)program, &psdData);

                            dataOut[offset] = program;
							offset++;
                            dataOut[offset] = 0x01;
							offset++;
                            dataOut[offset] = 0; // Field takes up 2 bytes
							offset++;

                            dataOut[offset] = 0; // Subfield
							offset++;
                            dataOut[offset] = 0; // byte 5 is reserved
							offset++;
                            dataOut[offset] = (U8)psdData.data_type;
                            offset++;
                            dataOut[offset] = (U8)psdData.length;
                            offset++;
                            if(psdData.length > 0) {
                                (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)psdData.data, psdData.length);
                            }
                            offset = (*stArith.u32add)(offset, psdData.length);
                        }

                        // Artist
                        if((fieldBitmask.artist == (U8)1) && (enabledFields.artist == (U8)1)){
                            psdData.length = 0; // erase the old value
                            (void)HDR_psd_get_artist(hdrInstance, (HDR_program_t)program, &psdData);

                            dataOut[offset] = program;
							offset++;
                            dataOut[offset] = 0x02;
							offset++;
                            dataOut[offset] = 0; // Field takes up 2 bytes
							offset++;

                            dataOut[offset] = 0; // Subfield
							offset++;
                            dataOut[offset] = 0; // byte 5 is reserved
                            offset++;
                            dataOut[offset] = (U8)psdData.data_type;
                            offset++;
                            dataOut[offset] = (U8)psdData.length;
                            offset++;
                            if(psdData.length > 0) {
                                (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)psdData.data, psdData.length);
                            }
                            offset = (*stArith.u32add)(offset, psdData.length);
                        }

                        // Album
                        if((fieldBitmask.album == (U8)1) && (enabledFields.album == (U8)1)){
                            psdData.length = 0; // erase the old value
                            (void)HDR_psd_get_album(hdrInstance, (HDR_program_t)program, &psdData);

                            dataOut[offset] = program;
							offset++;
                            dataOut[offset] = 0x04;
							offset++;
                            dataOut[offset] = 0; // Field takes up 2 bytes
							offset++;

                            dataOut[offset] = 0; // Subfield
							offset++;
                            dataOut[offset] = 0; // byte 5 is reserved
                            offset++;
                            dataOut[offset] = (U8)psdData.data_type;
                            offset++;
                            dataOut[offset] = (U8)psdData.length;
                            offset++;
                            if(psdData.length > 0) {
                                (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)psdData.data, psdData.length);
                            }
                            offset = (*stArith.u32add)(offset, psdData.length);
                        }

                        // Genre
                        if((fieldBitmask.genre == (U8)1) && (enabledFields.genre == (U8)1)){
                            psdData.length = 0; // erase the old value
                            (void)HDR_psd_get_genre(hdrInstance, (HDR_program_t)program, &psdData);

                            dataOut[offset] = program;
							offset++;
                            dataOut[offset] = 0x08;
							offset++;
                            dataOut[offset] = 0; // Field takes up 2 bytes
							offset++;

                            dataOut[offset] = 0; // Subfield
							offset++;
                            dataOut[offset] = 0; // byte 5 is reserved
							offset++;
                            dataOut[offset] = (U8)psdData.data_type;
                            offset++;
                            dataOut[offset] = (U8)psdData.length;
                            offset++;
                            if(psdData.length > 0) {
                                (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)psdData.data, psdData.length);
                            }
                            offset = (*stArith.u32add)(offset, psdData.length);
                        }

                        // Comment
                        if((fieldBitmask.comment == (U8)1) && (enabledFields.comment == (U8)1)){
                            U32 subfield;
                            for(subfield = 0; subfield < (U32)HDR_PSD_NUM_COMMENT_SUBFIELDS; subfield++){
                                if((commentSubfields & ((U32)1 << subfield)) == 0U){
                                    // subfield wasn't requested
                                    continue;
                                }

                                psdData.length = 0; // erase the old value
                                (void)HDR_psd_get_comment(hdrInstance, (HDR_program_t)program, (HDR_psd_comm_subfield_t)subfield, &psdData);

                                dataOut[offset] = program;
								offset++;
                                dataOut[offset] = 0x10;
								offset++;
                                dataOut[offset] = 0; // Field takes up 2 bytes
								offset++;

                                dataOut[offset] = (U8)1 << subfield; // Subfield
								offset++;
                                dataOut[offset] = 0; // byte 5 is reserved
								offset++;
                                dataOut[offset] = (U8)psdData.data_type;
                                offset++;
                                dataOut[offset] = (U8)psdData.length;
                                offset++;
                                if(psdData.length > 0) {
                                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)psdData.data, psdData.length);
                                }
                                offset = (*stArith.u32add)(offset, psdData.length);
                            }
                        }

                        // UFID
                        if((fieldBitmask.UFID == (U8)1) && (enabledFields.UFID == (U8)1)){
                            U32 n;
							U32 subfield;
                            for(n = 0; n < (U32)HDR_MAX_NUM_UFIDS; n++){
                                for(subfield = 0; subfield < (U32)HDR_PSD_NUM_UFID_SUBFIELDS; subfield++){
                                    if((ufidSubfields & ((U32)1 << subfield)) == (U32)0){
                                        // subfield wasn't requested
                                        continue;
                                    }

                                    psdData.length = 0; // erase the old value
                                    (void)HDR_psd_get_ufid(hdrInstance, (HDR_program_t)program, n, (HDR_psd_ufid_subfield_t)subfield, &psdData);

                                    dataOut[offset] = program;
									offset++;
                                    dataOut[offset] = 0x20;
									offset++;
                                    dataOut[offset] = 0; // Field takes up 2 bytes
									offset++;

                                    dataOut[offset] = (U8)1 << subfield; // Subfield
									offset++;
                                    dataOut[offset] = 0; // byte 5 is reserved
									offset++;
                                    dataOut[offset] = (U8)psdData.data_type;
                                    offset++;
                                    dataOut[offset] = (U8)psdData.length;
                                    offset++;
                                    if(psdData.length > 0) {
                                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)psdData.data, psdData.length);
                                    }
                                    offset = (*stArith.u32add)(offset, psdData.length);
                                }
                            }
                        }

                        // Commercial
                        if((fieldBitmask.commercial == (U8)1) && (enabledFields.commercial == (U8)1)){
                            U32 subfield;
                            for(subfield = 0; subfield < (U32)HDR_PSD_NUM_COMR_SUBFIELDS; subfield++){
                                if((commercialSubfields & ((U32)1 << subfield)) == 0U){
                                    // subfield wasn't requested
                                    continue;
                                }

                                psdData.length = 0; // erase the old value
                                (void)HDR_psd_get_commercial(hdrInstance, (HDR_program_t)program, (HDR_psd_comr_subfield_t)subfield, &psdData);

                                dataOut[offset] = program;
								offset++;
                                dataOut[offset] = 0x40;
								offset++;
                                dataOut[offset] = 0; // Field takes up 2 bytes
								offset++;

                                dataOut[offset] = (U8)1 << subfield; // Subfield
								offset++;
                                dataOut[offset] = 0; // byte 5 is reserved
								offset++;
                                dataOut[offset] = (U8)psdData.data_type;
                                offset++;
                                dataOut[offset] = (U8)psdData.length;
                                offset++;
                                if(psdData.length > 0) {
                                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)psdData.data, psdData.length);
                                }
                                offset = (*stArith.u32add)(offset, psdData.length);
                            }
                        }

                        // XHDR
                        if((fieldBitmask.XHDR == (U8)1) && (enabledFields.XHDR == (U8)1)){
                            U32 n;
                            for(n = 0; n < HDR_MAX_NUM_XHDRS; n++){
                                psdData.length = 0; // erase the old value
                                (void)HDR_psd_get_xhdr(hdrInstance, (HDR_program_t)program, n, &psdData);

                                dataOut[offset] = program;
								offset++;
                                dataOut[offset] = 0x80;
								offset++;
                                dataOut[offset] = 0; // Field takes up 2 bytes
								offset++;

                                dataOut[offset] = 0; // Subfield
								offset++;
                                dataOut[offset] = 0; // byte 5 is reserved
								offset++;
                                dataOut[offset] = (U8)psdData.data_type;
                                offset++;
                                dataOut[offset] = (U8)psdData.length;
                                offset++;
                                if(psdData.length > 0) {
                                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)psdData.data, psdData.length);
                                }
                                offset = (*stArith.u32add)(offset, psdData.length);
                            }
                        }

                        (void)HDR_psd_clear_changed_program(hdrInstance, (HDR_program_t)program);
                    }

                    *outLength = offset;
                    break;
                }
                case SET_PSD_CNFG_PARAM:
                {
                    LOG(CMD,524288U, "received PSD_DECODE->SET_PSD_CNFG_PARAM");
                    S32 rc = 0;
                    dataOut[0] = SET_PSD_CNFG_PARAM;

                    if(dataIn[1] == (U8)0x00){ // 0x00 - reset field specified by byte 2
                        if(dataIn[2] == (U8)0x01){
                            HDR_psd_fields_t enabled_fields;
                            enabled_fields.all = 0xFF;
                            (void)HDR_psd_enable_fields(hdrInstance, enabled_fields);
                        } else {
							U8 ucPsdField = dataIn[2] - 2U;
                            HDR_psd_length_config_t psd_field = (HDR_psd_length_config_t)ucPsdField;
                            rc = HDR_psd_reset_max_length(hdrInstance, psd_field);
                        }
                    } else {
                    	if(dataIn[1] == (U8)0x01){ // set field to value specified by byte3
	                        if(dataIn[2] == (U8)0x01){
	                            HDR_psd_fields_t enabled_fields;
	                            enabled_fields.all = dataIn[3];
	                            (void)HDR_psd_enable_fields(hdrInstance, enabled_fields);
	                        } else {
								U8 ucPsdField = dataIn[2] - (U8)2;
	                            HDR_psd_length_config_t psd_field = (HDR_psd_length_config_t)ucPsdField;
	                            rc = HDR_psd_set_max_length(hdrInstance, psd_field, dataIn[3]);
	                        }
                    	}
                    }

                    if(rc < 0){
                        dataOut[1] = 0;
                    } else {
                        dataOut[1] = 1;
                    }
                    *outLength = 2;
                    break;
                }
                case GET_PSD_CNFG_PARAM:
                {
                    LOG(CMD,524288U, "received PSD_DECODE->GET_PSD_CNFG_PARAM");

                    dataOut[0] = GET_PSD_CNFG_PARAM;

                    if(dataIn[1] == (U8)0x01){
                        dataOut[1] = HDR_psd_get_enabled_fields(hdrInstance).all;
                        dataOut[2] = 0;
                        *outLength = 3;
                    } else if((dataIn[1] > (U8)0x01) && (dataIn[1] <= (U8)0x0D)){
						U8 ucPsdField = dataIn[1] - 2U;
                        HDR_psd_length_config_t psd_field = (HDR_psd_length_config_t)ucPsdField;
                        dataOut[1] = (U8)(HDR_psd_get_max_length(hdrInstance, psd_field) & 0xffU);
                        *outLength = 2;
                    } else {
                        LOG(CMD,1U, "Invalid GET_PSD_CNFG_PARAM field ID.");
                        *outLength = 1;
                    }
                    break;
                }
                default:
                    LOG(CMD,524288U, "PSD_DECODE: function code 0x%x not recognised", funcCode);
                    return CMD_UNSUPPORTED_OPCODE;
					break;
            }
            break;
        default:
            LOG(CMD,1U, "Received unsupported SIS/PSD Function Code(0x%X) from host.", funcCode);
            return CMD_UNSUPPORTED_OPCODE;
			break;
    }
    return CMD_DISPATCH_OK;
}
