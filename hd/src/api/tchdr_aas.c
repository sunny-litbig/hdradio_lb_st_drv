/*******************************************************************************

*   FileName : tchdr_aas.c

*   Copyright (c) Telechips Inc.

*   Description : Advanced Application Service API functions and definitions

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

#include "hdrAas.h"

#include "tchdr_api.h"
#include "tchdr_framework.h"
#include "tchdr_aas.h"

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
*              Local definitions                   *
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
static HDRET tchdr_aas_setPorts(eTC_HDR_ID_t id, const stTC_HDR_AAS_PORT_LIST_t* port_list, U32 fEn)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_aas_port_list_t portList;
			(void)(*stOsal.osmemcpy)(&portList, port_list, (U32)sizeof(HDR_aas_port_list_t));
			if(fEn > 0U) {
				ret = HDR_aas_enable_ports(hdrInstance, &portList);
			}
			else {
				ret = HDR_aas_disable_ports(hdrInstance, &portList);
			}
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_enablePorts(eTC_HDR_ID_t id, const stTC_HDR_AAS_PORT_LIST_t* port_list)
{
	HDRET ret;
	if(port_list != NULL) {
		ret = tchdr_aas_setPorts(id, port_list, 1U);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}
	return ret;
}

HDRET tchdr_aas_disablePorts(eTC_HDR_ID_t id, const stTC_HDR_AAS_PORT_LIST_t* port_list)
{
	HDRET ret;
	if(port_list != NULL) {
		ret = tchdr_aas_setPorts(id, port_list, 0U);
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}
	return ret;
}

HDRET tchdr_aas_disableAllPorts(eTC_HDR_ID_t id)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_aas_disable_all_ports(hdrInstance);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_getEnabledPorts(eTC_HDR_ID_t id, stTC_HDR_AAS_PORT_LIST_t* port_list)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_aas_port_list_t portList;
			(void)(*stOsal.osmemset)((void*)&portList, (S8)0, (U32)sizeof(HDR_aas_port_list_t));
			ret = HDR_aas_get_enabled_ports(hdrInstance, &portList);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)port_list, (void*)&portList, (U32)sizeof(stTC_HDR_AAS_PORT_LIST_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_getNextPortData(eTC_HDR_ID_t id, stTC_HDR_AAS_PACKET_INFO_t* packet_info, U8* packet_buffer, U32 buffer_size, U32 * bytes_written)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_aas_packet_info_t packetInfo;
			(void)(*stOsal.osmemset)((void*)&packetInfo, (S8)0, (U32)sizeof(HDR_aas_packet_info_t));
			ret = HDR_aas_get_next_port_data(hdrInstance, &packetInfo, packet_buffer, buffer_size, bytes_written);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_NO_DATA_AVAILABLE;
				}
				else if(ret == -2) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)packet_info, (void*)&packetInfo, (U32)sizeof(stTC_HDR_AAS_PACKET_INFO_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_getPortData(eTC_HDR_ID_t id, U32 port_number, stTC_HDR_AAS_PACKET_INFO_t* packet_info, U8* packet_buffer, U32 buffer_size, U32 * bytes_written)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_aas_packet_info_t packetInfo;
			(void)(*stOsal.osmemset)((void*)&packetInfo, (S8)0, (U32)sizeof(HDR_aas_packet_info_t));
			ret = HDR_aas_get_port_data(hdrInstance, port_number, &packetInfo, packet_buffer, buffer_size, bytes_written);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_NO_DATA_AVAILABLE;
				}
				else if(ret == -2) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else if(ret == -3) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_CORRUPTED_PACKET;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)packet_info, (void*)&packetInfo, (U32)sizeof(stTC_HDR_AAS_PACKET_INFO_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_flushPort(eTC_HDR_ID_t id, U32 port_number)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_aas_flush_port(hdrInstance, port_number);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_flushAllPorts(eTC_HDR_ID_t id)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_aas_flush_all_ports(hdrInstance);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_getLotPoolSize(eTC_HDR_ID_t id, U32 *mem_pool_size)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_aas_get_lot_pool_size(hdrInstance, mem_pool_size);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_getLotSpaceLeft(eTC_HDR_ID_t id, U32 *mem_left)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			if(mem_left != NULL) {
				*mem_left = HDR_aas_get_lot_space_left(hdrInstance);
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

HDRET tchdr_aas_lotOverflow(eTC_HDR_ID_t id, HDBOOL *overFlow)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			if(overFlow != NULL) {
				*overFlow = HDR_aas_lot_overflow(hdrInstance);
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

HDRET tchdr_aas_enableLotReassembly(eTC_HDR_ID_t id, U32 service_number, U32 port_number)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_aas_enable_lot_reassembly(hdrInstance, service_number, port_number);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else if(ret == -2) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_RESERVED_PORT_REQ;
				}
				else if(ret == -3) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_MAX_AAS_PORTS_ALREADY_ENABLED;
				}
				else if(ret == -4) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_MAX_LOT_PORTS_ALREADY_ENABLED;
				}
				else if(ret == -5) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_ALEADY_OPEN_PORT;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_disableLotReassembly(eTC_HDR_ID_t id, U32 service_number, U32 port_number)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_aas_disable_lot_reassembly(hdrInstance, service_number, port_number);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else if(ret == -2) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_NOT_FOUND_PORT_OR_SERVICE;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_getLotObjectList(eTC_HDR_ID_t id, U32 service_number, stTC_HDR_AAS_LOT_OBJECT_LIST_t* object_list)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_aas_lot_object_list_t objectList;
			(void)(*stOsal.osmemset)((void*)&objectList, (S8)0, (U32)sizeof(HDR_aas_lot_object_list_t));
			ret = HDR_aas_get_lot_object_list(hdrInstance, service_number, &objectList);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)object_list, (void*)&objectList, (U32)sizeof(stTC_HDR_AAS_LOT_OBJECT_LIST_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_getLotObjectListByName(eTC_HDR_ID_t id, U32 service_number, const S8* filename, stTC_HDR_AAS_LOT_OBJECT_LIST_t* object_list)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_aas_lot_object_list_t objectList;
			(void)(*stOsal.osmemset)((void*)&objectList, (S8)0, (U32)sizeof(HDR_aas_lot_object_list_t));
			ret = HDR_aas_lot_get_object_list_by_name(hdrInstance, service_number, filename, &objectList);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)object_list, (void*)&objectList, (U32)sizeof(stTC_HDR_AAS_LOT_OBJECT_LIST_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_getLotObjectHeader(eTC_HDR_ID_t id, U32 port_number, U32 object_lot_id, stTC_HDR_AAS_LOT_OBJECT_HEADER_t* object_header)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_aas_lot_object_header_t objectHeader;
			(void)(*stOsal.osmemset)((void*)&objectHeader, (S8)0, (U32)sizeof(HDR_aas_lot_object_header_t));
			ret = HDR_aas_get_lot_object_header(hdrInstance, port_number, object_lot_id, &objectHeader);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else if(ret == -2) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_NOT_FOUND_OBJECT;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)object_header, (void*)&objectHeader, (U32)sizeof(stTC_HDR_AAS_LOT_OBJECT_HEADER_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_getLotObjectBody(eTC_HDR_ID_t id, U32 port_number, U32 object_lot_id, U8* buffer, U32 buffer_size, U32 * bytes_written)
{
	S32 hdret;
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			hdret = HDR_aas_get_lot_object_body(hdrInstance, port_number, object_lot_id, buffer, buffer_size, bytes_written);
			if(hdret == 1) {
				ret = 1;
			}
			else if(hdret == 0) {
				ret = 0;
			}
			else if(hdret == -1) {
				ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
			}
			else if(hdret == -2) {
				ret = (HDRET)eTC_HDR_RET_NG_AAS_NOT_FOUND_OBJECT;
			}
			else {
				ret = tchdr_convertHdrError((HDR_error_code_t)hdret);
			}
		}
	}
	return ret;
}

HDRET tchdr_aas_flushLotObject(eTC_HDR_ID_t id, U32 port_number, U32 object_lot_id)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_aas_flush_lot_object(hdrInstance, port_number, object_lot_id);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else if(ret == -2) {
					ret = (HDRET)eTC_HDR_RET_NG_AAS_NOT_FOUND_OBJECT;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
		}
	}
	return ret;
}

