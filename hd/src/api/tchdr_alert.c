/*******************************************************************************

*   FileName : tchdr_alert.c

*   Copyright (c) Telechips Inc.

*   Description : Emergency Alerts Service API functions and definitions

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

#include "hdrAlerts.h"

#include "tchdr_api.h"
#include "tchdr_framework.h"
#include "tchdr_alert.h"

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
HDRET tchdr_alert_getMessage(eTC_HDR_ID_t id, stTC_HDR_ALERT_MESSAGE_t* message)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_alert_message_t alertMessage;
			(void)(*stOsal.osmemset)((void*)&alertMessage, (S8)0, (U32)sizeof(HDR_alert_message_t));
			ret = HDR_alert_get_message(hdrInstance, &alertMessage);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else if(ret == -2) {
					ret = (HDRET)eTC_HDR_RET_NG_ALERT_NO_NEW_MESSAGE;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				U32 msgOffset=0;
				(void)(*stOsal.osmemcpy)((void*)message, (void*)&alertMessage, (U32)sizeof(stTC_HDR_ALERT_MESSAGE_t));
				if(alertMessage.text_message != NULL) {
					msgOffset = (*stArith.u32add)(message->cnt_length, 1U);
					message->text_message = &message->payload[msgOffset];
				}
				else {
					message->text_message = NULL;
				}
			}
		}
	}
	return ret;
}

HDRET tchdr_alert_getMessageStatus(eTC_HDR_ID_t id, stTC_HDR_ALERTS_MSG_STATUS_t* status)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_alerts_msg_status_t alertStatus;
			(void)(*stOsal.osmemset)((void*)&alertStatus, (S8)0, (U32)sizeof(HDR_alerts_msg_status_t));
			ret = HDR_alert_get_message_status(hdrInstance, &alertStatus);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)status, (void*)&alertStatus, (U32)sizeof(stTC_HDR_ALERTS_MSG_STATUS_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_alert_clearMessageStatus(eTC_HDR_ID_t id)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_alert_clear_message_status(hdrInstance);
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

