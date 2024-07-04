/*******************************************************************************

*   FileName : tchdr_sig.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio service information guide API functions and definitions

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

#include "hdrSig.h"

#include "tchdr_api.h"
#include "tchdr_framework.h"
#include "tchdr_sig.h"

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
HDRET tchdr_sig_getServiceList(eTC_HDR_ID_t id, eTC_HDR_SIG_SERVICE_TYPE_t service_type, stTC_HDR_SIG_SERVICE_LIST_t* service_list)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sig_service_list_t getServiceList;
			(void)(*stOsal.osmemset)((void*)&getServiceList, (S8)0, (U32)sizeof(HDR_sig_service_list_t));
			ret = HDR_sig_get_service_list(hdrInstance, (HDR_sig_service_type_t)service_type, &getServiceList);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)service_list, (void*)&getServiceList, (U32)sizeof(stTC_HDR_SIG_SERVICE_LIST_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sig_getServiceInfo(eTC_HDR_ID_t id, U32 service_number, stTC_HDR_SIG_SERVICE_INFO_t* service_info)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sig_service_info_t getServiceInfo;
			(void)(*stOsal.osmemset)((void*)&getServiceInfo, (S8)0, (U32)sizeof(HDR_sig_service_info_t));
			ret = HDR_sig_get_service_info(hdrInstance, service_number, &getServiceInfo);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_SIG_NO_SERVICE;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)service_info, (void*)&getServiceInfo, (U32)sizeof(stTC_HDR_SIG_SERVICE_INFO_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sig_getServiceComponent(eTC_HDR_ID_t id, U32 service_number, U32 component_index, stTC_HDR_SIG_SERVICE_COMPONENT_t* component)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_sig_service_component_t getComponent;
			(void)(*stOsal.osmemset)((void*)&getComponent, (S8)0, (U32)sizeof(HDR_sig_service_component_t));
			ret = HDR_sig_get_service_component(hdrInstance, service_number, component_index, &getComponent);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_SIG_NO_SERVICE;
				}
				else if(ret == -2) {
					ret = (HDRET)eTC_HDR_RET_NG_SIG_NO_COMPONENT;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)component, (void*)&getComponent, (U32)sizeof(stTC_HDR_SIG_SERVICE_COMPONENT_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_sig_flushAll(eTC_HDR_ID_t id)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_sig_flush_all(hdrInstance);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
		}
	}
	return ret;
}

