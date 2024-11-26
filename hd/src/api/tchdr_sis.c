/*******************************************************************************

*   FileName : tchdr_sis.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio station information service API functions and definitions

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
#include "tchdr_common.h"

#include "hdrSis.h"

#include "tchdr_api.h"
#include "tchdr_framework.h"
#include "tchdr_sis.h"

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
HDRET tchdr_sis_acquired(eTC_HDR_ID_t id, HDBOOL *acquired)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			if(acquired != NULL) {
				*acquired = HDR_sis_acquired(hdrInstance);
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	return ret;
}

HDRET tchdr_sis_crcOk(eTC_HDR_ID_t id, HDBOOL *crc)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			if(crc != NULL) {
				*crc = HDR_sis_crc_ok(hdrInstance);
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	return ret;
}

HDRET tchdr_sis_enableBasicTypes(eTC_HDR_ID_t id, stTC_HDR_SIS_ENABLED_BASIC_TYPES_t types)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		HDR_sis_enabled_basic_types_t sisTypes;
		sisTypes.all = types.all;
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_sis_enable_basic_types(hdrInstance, sisTypes);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getEnabledBasicTypes(eTC_HDR_ID_t id, stTC_HDR_SIS_ENABLED_BASIC_TYPES_t* enabled_types)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_enabled_basic_types_t getEnabledTypes;
			getEnabledTypes.all = 0;
			ret = HDR_sis_get_enabled_basic_types(hdrInstance, &getEnabledTypes);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				enabled_types->all = getEnabledTypes.all;
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getBlockCount(eTC_HDR_ID_t id, U32 *count)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			if(count != NULL) {
				*count = HDR_sis_get_block_count(hdrInstance);
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	return ret;
}

HDRET tchdr_sis_timeGpsLocked(eTC_HDR_ID_t id, HDBOOL *locked)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			if(locked != NULL) {
				*locked = HDR_sis_time_gps_locked(hdrInstance);
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	return ret;
}

HDRET tchdr_sis_getAlfn(eTC_HDR_ID_t id, stTC_HDR_SIS_ALFN_t* alfn)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_alfn_t getAlfn;
			getAlfn.value = 0U;
			getAlfn.status = ALFN_INVALID;
			ret = HDR_sis_get_alfn(hdrInstance, &getAlfn);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)alfn, (void*)&getAlfn, (U32)sizeof(stTC_HDR_SIS_ALFN_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getStationID(eTC_HDR_ID_t id, stTC_HDR_SIS_STATION_ID_t* station_id)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_station_id_t getStationID;
			getStationID.all = 0;
			getStationID.status = HDR_SIS_NO_DATA;
			ret = HDR_sis_get_station_id(hdrInstance, &getStationID);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
#ifdef USE_HDRLIB_3RD_CHG_VER
				station_id->id.field.fcc_facility_id = getStationID.all & 0x7FFFFU;
				station_id->id.field.reserved = (getStationID.all >> 19U) & 0x07U;
				station_id->id.field.country_code = (getStationID.all >> 22U) & 0x3FFU;
#else
				(void)(*stOsal.osmemcpy)((void*)station_id, (void*)&getStationID, (U32)sizeof(stTC_HDR_SIS_STATION_ID_t));
#endif
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getStationShortName(eTC_HDR_ID_t id, stTC_HDR_SIS_SHORT_NAME_t* short_name)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_short_name_t getShortName;
			(void)(*stOsal.osmemset)((void*)&getShortName, (S8)0, (U32)sizeof(HDR_sis_short_name_t));
			ret = HDR_sis_get_station_short_name(hdrInstance, &getShortName);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)short_name, (void*)&getShortName, (U32)sizeof(stTC_HDR_SIS_SHORT_NAME_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getStationLocation(eTC_HDR_ID_t id, stTC_HDR_SIS_STATION_LOCATION_t* location)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_station_location_t getLocation;
			(void)(*stOsal.osmemset)((void*)&getLocation, (S8)0, (U32)sizeof(HDR_sis_station_location_t));
			ret = HDR_sis_get_station_location(hdrInstance, &getLocation);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)location, (void*)&getLocation, (U32)sizeof(stTC_HDR_SIS_STATION_LOCATION_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getLeapSec(eTC_HDR_ID_t id, stTC_HDR_SIS_LEAP_SEC_t* leap_sec)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_leap_sec_t getLeapSec;
			(void)(*stOsal.osmemset)((void*)&getLeapSec, (S8)0, (U32)sizeof(HDR_sis_leap_sec_t));
			ret = HDR_sis_get_leap_sec(hdrInstance, &getLeapSec);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)leap_sec, (void*)&getLeapSec, (U32)sizeof(stTC_HDR_SIS_LEAP_SEC_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getStationMessage(eTC_HDR_ID_t id, stTC_HDR_SIS_STATION_MSG_t* station_msg)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_station_msg_t getStationMsg;
			(void)(*stOsal.osmemset)((void*)&getStationMsg, (S8)0, (U32)sizeof(HDR_sis_station_msg_t));
			ret = HDR_sis_get_station_message(hdrInstance, &getStationMsg);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)station_msg, (void*)&getStationMsg, (U32)sizeof(stTC_HDR_SIS_STATION_MSG_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getLocalTime(eTC_HDR_ID_t id, stTC_HDR_SIS_LOCAL_TIME_t* local_time)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_local_time_t getLocalTime;
			(void)(*stOsal.osmemset)((void*)&getLocalTime, (S8)0, (U32)sizeof(HDR_sis_local_time_t));
			ret = HDR_sis_get_local_time(hdrInstance, &getLocalTime);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)local_time, (void*)&getLocalTime, (U32)sizeof(stTC_HDR_SIS_LOCAL_TIME_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getUniversalName(eTC_HDR_ID_t id, stTC_HDR_SIS_UNIV_NAME_t* univ_name)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_univ_name_t getUnivName;
			(void)(*stOsal.osmemset)((void*)&getUnivName, (S8)0, (U32)sizeof(HDR_sis_univ_name_t));
			ret = HDR_sis_get_universal_name(hdrInstance, &getUnivName);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)univ_name, (void*)&getUnivName, (U32)sizeof(stTC_HDR_SIS_UNIV_NAME_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getAvailProgramsList(eTC_HDR_ID_t id, stTC_HDR_SIS_AVAIL_PROGRAMS_t* available_programs)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_avail_programs_t getAvailablePrograms;
			(void)(*stOsal.osmemset)((void*)&getAvailablePrograms, (S8)0, (U32)sizeof(HDR_sis_avail_programs_t));
			ret = HDR_sis_get_avail_programs_list(hdrInstance, &getAvailablePrograms);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)available_programs, (void*)&getAvailablePrograms, (U32)sizeof(stTC_HDR_SIS_AVAIL_PROGRAMS_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getStationSlogan(eTC_HDR_ID_t id, stTC_HDR_SIS_STATION_SLOGAN_t* slogan)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_station_slogan_t getSlogan;
			(void)(*stOsal.osmemset)((void*)&getSlogan, (S8)0, (U32)sizeof(HDR_sis_station_slogan_t));
			ret = HDR_sis_get_station_slogan(hdrInstance, &getSlogan);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)slogan, (void*)&getSlogan, (U32)sizeof(stTC_HDR_SIS_STATION_SLOGAN_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getProgramInfo(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, stTC_HDR_SIS_PROGRAM_INFO_t* program_info)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_program_info_t getProgramInfo;
			(void)(*stOsal.osmemset)((void*)&getProgramInfo, (S8)0, (U32)sizeof(HDR_sis_program_info_t));
			ret = HDR_sis_get_program_info(hdrInstance, (HDR_program_t)program_number, &getProgramInfo);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)program_info, (void*)&getProgramInfo, (U32)sizeof(stTC_HDR_SIS_PROGRAM_INFO_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getAvailDataServList(eTC_HDR_ID_t id, stTC_HDR_SIS_AVAIL_DATA_SERVICES_t* available_services)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_avail_data_services_t getAvailableServices;
			(void)(*stOsal.osmemset)((void*)&getAvailableServices, (S8)0, (U32)sizeof(HDR_sis_avail_data_services_t));
			ret = HDR_sis_get_avail_data_serv_list(hdrInstance, &getAvailableServices);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)available_services, (void*)&getAvailableServices, (U32)sizeof(stTC_HDR_SIS_AVAIL_DATA_SERVICES_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getAllDataServices(eTC_HDR_ID_t id,  stTC_HDR_SIS_DATA_SERVICES_INFO_t* data_services_info)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_data_services_info_t getDataServicesInfo = {0,};
			ret = HDR_sis_get_all_data_services(hdrInstance, &getDataServicesInfo);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)data_services_info, (void*)&getDataServicesInfo, (U32)sizeof(stTC_HDR_SIS_DATA_SERVICES_INFO_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getDataServicesType(eTC_HDR_ID_t id, U32 service_type, stTC_HDR_SIS_DATA_SERVICES_INFO_t* data_services_info)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_data_services_info_t getDataServicesInfo = {0,};
			ret = HDR_sis_get_data_services_type(hdrInstance, service_type, &getDataServicesInfo);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)data_services_info, (void*)&getDataServicesInfo, (U32)sizeof(stTC_HDR_SIS_DATA_SERVICES_INFO_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getExciterCoreVer(eTC_HDR_ID_t id, stTC_HDR_SIS_TX_VER_STR_t* version_string)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_tx_ver_str_t getVerString = {0,};
			ret = HDR_sis_get_exciter_core_ver(hdrInstance, &getVerString);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)version_string, (void*)&getVerString, (U32)sizeof(stTC_HDR_SIS_TX_VER_STR_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getExciterManufVer(eTC_HDR_ID_t id, stTC_HDR_SIS_TX_MANUF_VER_t* version_struct)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_tx_manuf_ver_t getVerStruct = {0,};
			ret = HDR_sis_get_exciter_manuf_ver(hdrInstance, &getVerStruct);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)version_struct, (void*)&getVerStruct, (U32)sizeof(stTC_HDR_SIS_TX_MANUF_VER_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getImporterCoreVer(eTC_HDR_ID_t id, stTC_HDR_SIS_TX_VER_STR_t* version_string)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_tx_ver_str_t getVerString = {0,};
			ret = HDR_sis_get_importer_core_ver(hdrInstance, &getVerString);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)version_string, (void*)&getVerString, (U32)sizeof(stTC_HDR_SIS_TX_VER_STR_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_getImporterManufVer(eTC_HDR_ID_t id, stTC_HDR_SIS_TX_MANUF_VER_t* version_struct)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sis_tx_manuf_ver_t getVerStruct = {0,};
			ret = HDR_sis_get_importer_manuf_ver(hdrInstance, &getVerStruct);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)version_struct, (void*)&getVerStruct, (U32)sizeof(stTC_HDR_SIS_TX_MANUF_VER_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sis_flush(eTC_HDR_ID_t id)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_sis_flush(hdrInstance);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
		}
	}
	return ret;
}

#if 0
HDRET tchdr_sis_locationReadCount(eTC_HDR_ID_t id, U32 * count)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_sis_location_read_count(hdrInstance, count);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
		}
	}
	return ret;
}
#endif

