/*******************************************************************************

*   FileName : tchdr_alert.h

*   Copyright (c) Telechips Inc.

*   Description : Emergency Alerts Service API header

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
#ifndef TCHDR_ALERT_H__
#define TCHDR_ALERT_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define TC_HDR_MAX_ALERT_PAYLOAD_LENGTH            (381)

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eTC_HDR_ALERT_ISO_8859_1_1998 = 0,		/**< 8-bit unicode char(default)  */
	eTC_HDR_ALERT_ISO_8859_1_1998C = 1		/**< ISO/IEC 8859-1:1998 compressed */
}eTC_HDR_ALERT_TEXT_ENCODING_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct {
	U32 payload_crc;
	U32 payload_length;
	U32 cnt_length;
	HDBOOL cnt_crc_pass;
	eTC_HDR_ALERT_TEXT_ENCODING_t text_encoding;
	U32 text_length;
	S8* text_message; // A pointer to the message in the payload array. This string does not contain the last null byte.
	S8 payload[TC_HDR_MAX_ALERT_PAYLOAD_LENGTH];	// Message ID + Control String(CNT) + Alert Text Message
}stTC_HDR_ALERT_MESSAGE_t;

typedef struct {
	HDBOOL frame_received;
	HDBOOL frame0_available;
	HDBOOL full_message;
	U32 frame_counter;
	U32 message_id;
	U32 payload_crc;
}stTC_HDR_ALERTS_MSG_STATUS_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern HDRET tchdr_alert_getMessage(eTC_HDR_ID_t id, stTC_HDR_ALERT_MESSAGE_t* message);
extern HDRET tchdr_alert_getMessageStatus(eTC_HDR_ID_t id, stTC_HDR_ALERTS_MSG_STATUS_t* status);
extern HDRET tchdr_alert_clearMessageStatus(eTC_HDR_ID_t id);

#ifdef __cplusplus
}
#endif

#endif
