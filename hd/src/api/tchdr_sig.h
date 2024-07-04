/*******************************************************************************

*   FileName : tchdr_sig.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio service information guide header

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
#ifndef TCHDR_SIG_H__
#define TCHDR_SIG_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define TC_HDR_MAX_NUM_SIG_SERVICES_PER_STATION     (32U)		// Don't Change Value
#define TC_HDR_MAX_SERVICE_PROVIDER_NAME_LENGTH     (256U)		// Don't Change Value
#define TC_HDR_MAX_SERVICE_DISPLAY_NAME_LENGTH      (256U)		// Don't Change Value
#define TC_HDR_MAX_NUM_COMPONENTS_PER_SERVICE      	(32U)		// Don't Change Value
#define TC_HDR_MAX_EXPANDED_SERVICE_ID_SIZE        	(247U)		// Don't Change Value

#define TC_HDR_SIG_APP_MIME_HASH_STATION_LOGO		(0xD9C72536U)
#define TC_HDR_SIG_APP_MIME_HASH_ALBUM_ART			(0xBE4B7536U)

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
    eTC_HDR_SIG_NO_DATA,
    eTC_HDR_SIG_OLD_DATA,
    eTC_HDR_SIG_NEW_DATA,
    eTC_HDR_SIG_REPEAT_DATA
}eTC_HDR_SIG_STATUS_t;

typedef enum {
    eTC_HDR_SIG_AUDIO_COMPONENT,
    eTC_HDR_SIG_DATA_COMPONENT,
    eTC_HDR_SIG_NUM_COMPONENT_TYPES
}eTC_HDR_SIG_COMPONENT_TYPE_t;

typedef enum {
    eTC_HDR_SIG_AUDIO_SERVICE_TYPE,
    eTC_HDR_SIG_DATA_SERVICE_TYPE,
    eTC_HDR_SIG_NUM_SERVICE_TYPES
}eTC_HDR_SIG_SERVICE_TYPE_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct{
    struct {
        U32 service_number;
        U32 receive_time;
        eTC_HDR_SIG_STATUS_t status;
    }item[TC_HDR_MAX_NUM_SIG_SERVICES_PER_STATION];
    U32 num_services;
}stTC_HDR_SIG_SERVICE_LIST_t;

typedef struct {
    eTC_HDR_SIG_SERVICE_TYPE_t type;
    U32 service_number;
    U32 priority;
    U32 sequence_number;
    eTC_HDR_SIG_STATUS_t status;
    U32 receive_time;
    U32 provider_text_encoding;
    U32 provider_name_length;
    S8 provider_name[TC_HDR_MAX_SERVICE_PROVIDER_NAME_LENGTH];
    U32 display_text_encoding;
    U32 display_name_length;
    S8 display_name[TC_HDR_MAX_SERVICE_DISPLAY_NAME_LENGTH];
    U32 num_components;
}stTC_HDR_SIG_SERVICE_INFO_t;

typedef struct {
    eTC_HDR_SIG_COMPONENT_TYPE_t component_type;
    U32 component_number;
    U32 channel;
    U32 content_type;
    U32 processing;
    U32 priority;
    U32 access_rights;
    U32 mime_hash_value;
    U32 provider_id;
    U32 service_id;
    U32 expanded_id_length;
    U8 expanded_service_id[TC_HDR_MAX_EXPANDED_SERVICE_ID_SIZE];
}stTC_HDR_SIG_SERVICE_COMPONENT_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern HDRET tchdr_sig_getServiceList(eTC_HDR_ID_t id, eTC_HDR_SIG_SERVICE_TYPE_t service_type, stTC_HDR_SIG_SERVICE_LIST_t* service_list);
extern HDRET tchdr_sig_getServiceInfo(eTC_HDR_ID_t id, U32 service_number, stTC_HDR_SIG_SERVICE_INFO_t* service_info);
extern HDRET tchdr_sig_getServiceComponent(eTC_HDR_ID_t id, U32 service_number, U32 component_index, stTC_HDR_SIG_SERVICE_COMPONENT_t* component);
extern HDRET tchdr_sig_flushAll(eTC_HDR_ID_t id);

#ifdef __cplusplus
}
#endif

#endif
