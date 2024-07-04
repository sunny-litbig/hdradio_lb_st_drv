/*******************************************************************************
*
* (C) copyright 2003-2016, iBiquity Digital Corporation, U.S.A.
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
 * @file hdrSig.h
 * @brief Service Information Guide (SIG) API functions and definitions
 * @defgroup HdrSig Service Information Guide (SIG)
 * @brief Service Information Guide (SIG) API
 * @ingroup HdrApi
 * @{
 *
 * Service information protocol named the Service Information Guide (SIG) is used by the
 * HD Radio system for proper handling of the information regarding services supported by
 * the system. While the initial guide information regarding available services can be advertised
 * via the Station Information Service (SIS), SIS would typically require the additional
 * information provided via SIG in order to retrieve information about data services.
 *
 * SIG provides the host controller (HC) with a list of both the audio and data services available
 * from the broadcaster. For data services, the associated data may then be accessed using the
 * Advanced Application Services (AAS) feature. For data services, the specific port on which the
 * data may be accessed, is also provided by the SIG. These ports must be enabled using
 * the AAS commands or LOT commands if the data is transferred as a LOT object.
 *
 * <b>For additional information see:</b><br>
 *    RX_IDD_2206_appendixL - Service Information Guide - Revision<X>.pdf <br>
 *    iBiquity Digital Corporation, "HD Radio System Global Service Parameter Definitions," URL:
 *    http://hdradio.com/broadcasters/us-regulatory/nrsc-supplemental-information
 */
#ifndef HDR_SIG_H_
#define HDR_SIG_H_

#include "hdrCore.h"

/**
 * @brief Defines the updated status of service information
 */
typedef enum HDR_sig_status_t{
    HDR_SIG_NO_DATA,                 /**< No data available */
    HDR_SIG_OLD_DATA,                /**< Same information since last read */
    HDR_SIG_NEW_DATA,                /**< New information available */
    HDR_SIG_REPEAT_DATA              /**< Information updated but it's the same as previous */
}HDR_sig_status_t;

/**
 * @brief Defines possible service component types
 */
typedef enum HDR_sig_component_type_t{
    HDR_SIG_AUDIO_COMPONENT,      /**< Audio related component */
    HDR_SIG_DATA_COMPONENT,       /**< Data related component */
    HDR_SIG_NUM_COMPONENT_TYPES   /**< Number of component types */
}HDR_sig_component_type_t;

/**
 * @brief Defines possible SIG service types
 */
typedef enum HDR_sig_service_type_t{
    HDR_SIG_AUDIO_SERVICE_TYPE,       /**< Audio related service */
    HDR_SIG_DATA_SERVICE_TYPE,        /**< Data related service */
    HDR_SIG_NUM_SERVICE_TYPES         /**< Number of service types */
}HDR_sig_service_type_t;

/**
 * @brief Maximum number of services allowed per station
 */
#define HDR_MAX_NUM_SIG_SERVICES_PER_STATION     (32U)

/**
 * @brief Specifies a list of available SIG services
 *
 * Contains a list of available services and some additional information about the service.
 * Note: A change in receive time does not affect the status.
 */
typedef struct{
    struct {
        uint_t service_number;                  /**< Uniquely identifies a service */
        uint_t receive_time;                    /**< ALFN when the service was received */
        HDR_sig_status_t status;                /**< Update status */
    }item[HDR_MAX_NUM_SIG_SERVICES_PER_STATION];
    uint_t num_services;                        /**< Number of SIG services */
}HDR_sig_service_list_t;

/**
 * @brief Retrieves the list of available services of the type specified
 *
 * Each item in the list has a service number that uniquely identifies a SIG service
 *
 * @param [in] hdr_instance: Pointer to the current HDR instance.
 * @param [in] service_type: Specifies type(audio/data) of service (see #HDR_sig_service_type_t)
 * @param [out] service_list: Pointer to the audio services output data structure. Storage must be provided by the caller (see #HDR_sig_service_list_t)
 * @returns
 *     0  - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sig_get_service_list(HDR_instance_t* hdr_instance, HDR_sig_service_type_t service_type, HDR_sig_service_list_t* service_list);

/**
 * @brief Maximum length(bytes) of the service provider name including the null-terminator
 */
#define HDR_MAX_SERVICE_PROVIDER_NAME_LENGTH     (256)

/**
 * @brief Maximum length(bytes) of the service display name including the null-terminator
 */
#define HDR_MAX_SERVICE_DISPLAY_NAME_LENGTH      (256)

/**
 * @brief SIG service information output data structure
 *
 * Describes the SIG service information type and associated parameters
 */
typedef struct HDR_sig_service_info_t{
    HDR_sig_service_type_t type;                              /**< Identifies whether it is Audio or Data Service */
    uint_t service_number;                              /**< Unique number that Identifies a service */
    uint_t priority;                                    /**< Indicates the priority of the specified service. */
    uint_t sequence_number;                             /**< Indicates update number on the specified service. */
    HDR_sig_status_t status;                                  /**< Indicates the update status of the service information. */
    uint_t receive_time;                                /**< Indicates the precise time(ALFN) that the information was received. */
    uint_t provider_text_encoding;                      /**< Text Encoding type used for the Service Provider Name. */
    uint_t provider_name_length;                        /**< Length of the service provider name. */
    char provider_name[HDR_MAX_SERVICE_PROVIDER_NAME_LENGTH]; /**< Service provider name text. */
    uint_t display_text_encoding;                       /**< Text Encoding Type used for the Service Display Name. */
    uint_t display_name_length;                         /**< Length of service display name. */
    char display_name[HDR_MAX_SERVICE_DISPLAY_NAME_LENGTH];   /**< Service display Name Text */
    uint_t num_components;                              /**< Number of service  components */
}HDR_sig_service_info_t;

/**
 * @brief Retrieves service information specified by the service number id
 *
 * @param [in] hdr_instance: Pointer to the HD Radio Instance
 * @param [in] service_number: Service number id
 * @param [out] service_info: Service information (see #HDR_sig_service_info_t)
 * @returns
 *     0 - Success <br>
 *    -1 - No service found with the specified service number id
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sig_get_service_info(HDR_instance_t* hdr_instance, uint_t service_number, HDR_sig_service_info_t* service_info);

/**
 * @brief Maximum number of components allowed per service
 */
#define HDR_MAX_NUM_COMPONENTS_PER_SERVICE      (32U)

/**
 * @brief Maximum expanded service id size (bytes)
 */
#define HDR_MAX_EXPANDED_SERVICE_ID_SIZE        (247U)

/**
 * @brief Defines service component structure
 *
 * Each service has at least one component (component 0) which is the anchor component for the service.
 */
typedef struct HDR_sig_service_component_t{
    HDR_sig_component_type_t component_type;   /**< Component type, audio or data */
    uint_t component_number;             /**< Component id number within the service */
    uint_t channel;                      /**< Corresponds to audio program if component type is audio - or port number if data */
    uint_t content_type;                 /**< If audio defines audio content (e.g., News, Talk, Rock, etc.)
                                                    or if data defines service data type (e.g., News, Sports, Traffic, etc.). */
    /**
     * If Type of component is Audio, this specifies the Applied Sound Experience. The Applied Sound Experience field applies
     * further processing of the audio material beyond the channel-related audio encoding and decoding for transport purposes.
     *
     * If Type of component is Data, this field specifies the Data Processing method:  <br>
     *    0 - RLS Byte Streaming <br>
     *    1 - RLS Packet <br>
     *    2 - Reserved - Not Used <br>
     *    3 - LOT - Packet
     */
    uint_t processing;
    uint_t priority;                     /**< Indicates the priority of this component within the service record */
    uint_t access_rights;                /**< Data only Indication - indicates whether the Data Packet has not been scrambled or not*/
    uint_t mime_hash_value;              /**< This hash value indicates the application to which this service may be applied */
    uint_t provider_id;                  /**< Part of the unique ID provided to identify the source of this component. This parameter is applicable to data services only */
    uint_t service_id;                   /**< Part of the unique ID provided to identify this component. This parameter is applicable to data services only */
    uint_t expanded_id_length;           /**< Length of the entire Service Identifier. Includes Service Provider ID, Service ID and Expanded Service Identifier */

    /**
     * The Expanded Service Identifier is available when the Service Identifier information
     * requires more than the eight bytes available with the Service Provider ID and Service ID fields
     */
    uint8_t expanded_service_id[HDR_MAX_EXPANDED_SERVICE_ID_SIZE];
}HDR_sig_service_component_t;

/**
 * @brief Retrieves service component data
 *
 * The HD Radio Lib stores service components in a list as they arrive in the system. Therefore,
 * the component index may not match /c component_index field returned with the component data.
 *
 * @param [in] hdr_instance: Pointer to the HD Radio Instance
 * @param [in] service_number: Service number id
 * @param [in] component_index: Index of the service component as it's stored in memory. Not the same as component number within #HDR_sig_service_component_t
 * @param [out] component: Pointer to the service component output data structure. Storage must be provided by the caller (see #HDR_sig_service_component_t)
 * @returns
 *     0 - Success <br>
 *    -1 - No service found with the specified service number id <br>
 *    -2 - Service found but no component with that index <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sig_get_service_component(HDR_instance_t* hdr_instance, uint_t service_number, uint_t component_index,
                                  HDR_sig_service_component_t* component);

/**
 * @brief Flushes all information stored and restarts SIG information retrieval.
 *
 * @param [in] hdr_instance: Pointer to the HD Radio Instance
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sig_flush_all(HDR_instance_t* hdr_instance);

#endif /* HDR_SIG_H_ */

/** @} */ //doxygen end-bracket
