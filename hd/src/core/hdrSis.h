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
 * @file hdrSis.h
 * @brief Station Information Services (SIS) API functions and definitions
 * @defgroup hdrSis Station Information Services (SIS)
 * @brief Station Information Services (SIS) API
 * @ingroup HdrApi
 * @{
 *
 * Station Information Service (SIS) data provides basic information about the station such
 * as call sign, as well as information not displayable to the consumer such as the station
 * identification number. The SIS information is common to all programs since it originates
 * at the Exciter/Exporter, regardless if the broadcast system includes an Importer for SPS
 * programs. SIS also provides information from an advanced data receiver that is seeking and
 * scanning for data services on the channel.
 *
 * <b>For additional information see:</b><br>
 *    iBiquity Digital Corporation, "HD Radio System Global Service Parameter Definitions," URL:
 *    http://hdradio.com/broadcasters/us-regulatory/nrsc-supplemental-information
 *
 *  Document RX_IDD_2206 - HD Radio Commercial Receiver Baseband Processor Command and Data Interface Definition - Section 9.5
 */
#ifndef HDR_SIS_H_
#define HDR_SIS_H_

#include "hdrAudio.h"

/**
 * @brief SIS Update status
 */
typedef enum HDR_sis_status_t{
    HDR_SIS_NO_DATA = 0,       /**< No data for requested type is available */
    HDR_SIS_OLD_DATA,          /**< Available, but old data; no new data received since last request */
    HDR_SIS_NEW_DATA,          /**< New data available */
    HDR_SIS_ERROR              /**< Error occurred during request processing */
}HDR_sis_status_t;

/**
 * @brief Returns status of SIS data reception
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle
 * @returns
 *     true  - Receiving SIS messages <br>
 *     false - Not receiving SIS messages
 */
bool HDR_sis_acquired(HDR_instance_t* hdr_instance);

/**
 * @brief Instantaneous indication of SIS CRC status.
 *
 * Updated each time a SIS frame is received
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle
 * @returns
 *     true  - last SIS message CRC was ok <br>
 *     false - last SIS message CRC failed
 */
bool HDR_sis_crc_ok(HDR_instance_t* hdr_instance);

/**
 * @brief Number of basic SIS data types
 */
#define HDR_SIS_NUM_BASIC_TYPES   (4U)

/**
 * @brief Structure used for enabling/disabling basic SIS data types.
 *
 * Station ID, Station Short Name, Station Long Name, and Station Location
 * are considered basic SIS data types. All others are extended SIS data.
 */
typedef union HDR_sis_enabled_basic_types_t{
    struct {
        uint8_t stationId:1; /**< Station id */
        uint8_t shortName:1; /**< Station Name (Short Form) */
        uint8_t location:1;	 /**< Station Location  */
    };
    uint8_t all; /**< Controls all bits at once */
}HDR_sis_enabled_basic_types_t;

/**
 * @brief Enables user specified SIS basic data types
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle
 * @param [in] types: bitmap list of basic SIS information to be enabled (see #HDR_sis_enabled_basic_types_t)
 * @returns
 *     0  - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_enable_basic_types(HDR_instance_t* hdr_instance, HDR_sis_enabled_basic_types_t types);

/**
 * @brief Gets the information about enabled basic SIS data types
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle
 * @param [out] enabled_types: Pointer to enabled types storage provided by the caller (see #HDR_sis_enabled_basic_types_t)
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_enabled_basic_types(HDR_instance_t* hdr_instance, HDR_sis_enabled_basic_types_t* enabled_types);

/**
 * @brief Returns the SIS block count
 *
 * Block is defined as a single SIS PDU received
 *
 * @param [in] hdr_instance: Pointer to the HD Radio Instance
 *
 * @returns SIS block count
 */
uint_t HDR_sis_get_block_count(HDR_instance_t* hdr_instance);

/**
 * @brief Return status of transmit-site a GPS lock.
 *
 * The ALFN can be used to provide accurate time.
 * @param [in] hdr_instance:  Pointer to the HD Radio Instance
 * @returns
 *     true  - time is locked to GPS <br>
 *     false - time is not locked to GPS
 */
bool HDR_sis_time_gps_locked(HDR_instance_t* hdr_instance);

/**
 * @brief Status of Absolute Layer 1 Frame Number (ALFN) data
 */
typedef enum{
    ALFN_VALID,         /**< Valid data received */
    ALFN_ACQUIRING,     /**< Still waiting for data */
    ALFN_FAILURE,       /**< Failure occurred  */
    ALFN_INVALID        /**< Data received but was determined to be invalid */
}HDR_sis_alfn_status_t;

/**
 * @brief Absolute Layer 1 Frame Number (ALFN) output data structure
 */
typedef struct {
    uint32_t value;                 /**< ALFN value */
    HDR_sis_alfn_status_t status;   /**< Update status */
}HDR_sis_alfn_t;

/**
 * @brief Retrieves Absolute Layer 1 Frame Number (ALFN) number.
 *
 * If a station is locked to GPS, the ALFN can be used to provide accurate time.
 *
 * @param [in] hdr_instance: Pointer to the HD Radio Instance
 * @param [out] alfn: Pointer to ALFN output data structure. Must be allocated by the caller (see #HDR_sis_alfn_t)
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_alfn(HDR_instance_t* hdr_instance, HDR_sis_alfn_t* alfn);

/**
 * @brief Station ID
 *
 * Country Code: Binary representation of ISO 3166-1-alpha-2 Country Names and Code Elements
 * FCC Facility ID: Unique Facility ID assigned by the FCC (USA only).
 */
typedef struct HDR_sis_station_id_t{
    union {
        struct {
#ifdef USE_HDRLIB_3RD_CHG_VER
            uint32_t fcc_facility_id:19; /**< Unique Facility ID assigned by the FCC (USA only) */
            uint32_t reserved:3;         /**< Bits not used */
            uint32_t country_code:10;    /**< Binary representation of ISO 3166-1-alpha-2 Country Names and Code Elements */
#else
			uint32_t country_code:10;    /**< Binary representation of ISO 3166-1-alpha-2 Country Names and Code Elements */
			uint32_t reserved:3;		 /**< Bits not used */
			uint32_t fcc_facility_id:19; /**< Unique Facility ID assigned by the FCC (USA only) */
#endif
        };
        uint32_t all;
    };
    HDR_sis_status_t status; /**< Update status */
}HDR_sis_station_id_t;

/**
 *  @brief Retrieves the Station ID
 *
 *  @param [in] hdr_instance: Pointer to the HD Radio Instance
 *  @param [out] station_id: Pointer to Station ID. Must be allocated by the caller (see #HDR_sis_station_id_t)
 *  @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_station_id(HDR_instance_t* hdr_instance, HDR_sis_station_id_t* station_id);

/**
 * @brief Defines the maximum station short name length
 */
#define HDR_SIS_SHORT_NAME_MAX_LENGTH       (8U)

/**
 * @brief SIS station short name output data structure
 *
 * The text is null-terminated and length value will not include
 * the null character - matching output of strlen()
 */
typedef struct {
    char text[HDR_SIS_SHORT_NAME_MAX_LENGTH]; /**< Short name */
    uint_t length;                      /**< Short name length */
    HDR_sis_status_t status;                  /**< Update status */
}HDR_sis_short_name_t;

/**
 * @brief Retrieves the Station Name (Short Form)
 *
 * The station name (short form) identifies the station call sign. If the broadcaster sends a callsign of 6
 * characters or less, the callsign may be read in the station name. However, if a broadcaster sends a callsign
 * greater than 6 characters in length, this field will not be populated. In such a case, the host must use
 * the HDR_sis_get_universal_name().
 *
 * @param [in] hdr_instance: Pointer to the HD Radio Instance
 * @param [out] short_name: Pointer to the station short name output. Must be allocated by the caller (see #HDR_sis_short_name_t)
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_station_short_name(HDR_instance_t* hdr_instance, HDR_sis_short_name_t* short_name);

/**
 * @brief Station Location output data structure
 *
 * This structure defines the absolute three-dimensional location of the feed point of the broadcast antenna.
 * Location information may be used by the receiver for position determination.
 */
typedef struct HDR_sis_station_location_t {
    /**
     * @brief Station Latitude (S19.13 format)
     *
     * Latitude and longitude are both in identical fractional format. The LSB is equal to 1/8192 degrees.
     * The sign bit indicates the hemisphere, where positive latitude values represent positions
     * north of the equator and negative values represent positions south of the equator. Permissible latitude
     * values are between -90 and +90. Anything outside of these ranges is invalid.
     */
    int32_t latitude;

    /**
     * @brief Station Longitude (S19.13 format)
     *
     * Latitude and longitude are both in identical fractional format. The LSB is equal to 1/8192 degrees.
     * Positive latitude values represent positions north of the equator. Positive longitudes are in the eastern
     * hemisphere. Permissible longitude values are between -180 and +180. Anything outside of these
     * ranges is invalid.
     */
    int32_t longitude;

    /**
     * @brief Altitude of the station
     *
     * Altitude is in units of meters with resolution of 16 meters
     */
    uint_t altitude;

    HDR_sis_status_t status; /**< Update status */
}HDR_sis_station_location_t;

/**
 * @brief Retrieves station location
 *
 * Location indicates the absolute three-dimensional location of the feed point of the broadcast antenna.
 * Such location information may be used by the receiver for position determination.
 *
 * @param [in] hdr_instance: Pointer to the HD Radio instance.
 * @param [out] location: Pointer to the station location output. Must be allocated by the caller (see #HDR_sis_station_location_t)
 * @return
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_station_location(HDR_instance_t* hdr_instance, HDR_sis_station_location_t* location);

/**
 * @brief Defines structure with data related to leap second
 *
 * A leap-second correction factor occasionally adjusts UTC by one second to keep it
 * synchronized with astronomical time. Since GPS time does not apply this correction, the
 * two standards have diverged slightly over the years. Receivers can calculate GPS
 * time using the ALFN and then use the correction to calculate UTC.
 */
typedef struct HDR_sis_leap_sec_t{
    int8_t pending_offset;          /**< Pending Leap second offset */
    int8_t current_offset;          /**< Current Leap second offset */
    uint32_t pending_offset_alfn;   /**< ALFN of pending leap second offset */
    HDR_sis_status_t status;        /**< Update status */
}HDR_sis_leap_sec_t;

/**
 * @brief Retrieves SIS leap second information
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * @param [out] leap_sec: Pointer to the output data structure. Must be allocated by the caller (see #HDR_sis_leap_sec_t)
 * @return
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_leap_sec(HDR_instance_t* hdr_instance, HDR_sis_leap_sec_t* leap_sec);

/**
 * @brief Text encoding type for SIS message strings
 */
typedef enum HDR_sis_text_encoding_t{
    HDR_SIS_ISO_IEC_8859_1_1998 = 0,  /**< 8-bit unicode char  */
    HDR_SIS_ISO_IEC_10646_1_2000 = 4  /**< 16-bit unicode char (Little-endian) */
}HDR_sis_text_encoding_t;

/**
 * @brief Defines the maximum allowable message size in bytes including the null-terminator
 */
#define HDR_SIS_STATION_MESSAGE_MAX_SIZE        (191U)

/**
 * @brief SIS station message output data structure
 *
 * The text is null-terminated and length value will not include
 * the null character - matching output of strlen()
 */
typedef struct HDR_sis_station_msg_t {
    char text[HDR_SIS_STATION_MESSAGE_MAX_SIZE]; /**< Message buffer */
    uint_t length;                         /**< Message length */
    HDR_sis_text_encoding_t text_encoding;       /**< Encoding type */
    bool high_priority;                          /**< Specifies whether the message was sent with high priority */
    HDR_sis_status_t status;                     /**< Update status */
}HDR_sis_station_msg_t;

/**
 * @brief Retrieves SIS station message
 *
 * Arbitrary text message (e.g., telephone number, URL, etc.).
 * High-priority messages should take precedence over all other SIS data and PSD.
 * The Station Message can contain a string of up to 190 8-bit characters or
 * 95 16-bit characters.
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle
 * @param [out] station_msg: Pointer to the incoming message. Must be allocated by the caller (see #HDR_sis_station_msg_t)
 * @return
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_station_message(HDR_instance_t* hdr_instance, HDR_sis_station_msg_t* station_msg);

/**
 * @brief Indicates the region of daylight savings time(DST) schedule
 */
typedef enum HDR_sis_dst_schedule_t{
    HDR_SIS_DST_SCHED_NONE,
    HDR_SIS_DST_SCHED_US_CAN,
    HDR_SIS_DST_SCHED_EU
}HDR_sis_dst_schedule_t;

/**
 * @brief Indicates whether daylight savings time(DST) is practiced locally
 */
typedef enum HDR_sis_dst_local_t{
    HDR_SIS_DST_NOT_PRACTICED,
    HDR_SIS_DST_PRACTICED
}HDR_sis_dst_local_t;

/**
 * @brief Defines structure for retrieving station local time data
 *
 * Local time zone and daylight savings time (DST) information, allowing receivers to
 * automatically calculate and display time of day.
 */
typedef struct HDR_sis_local_time_t{
    /**
     * @brief Time Zone value offset from UTC
     * Stores a signed integer in minutes relative to UTC, assuming DST is not in
     * effect
     */
    int32_t utc_offset;
    /**
     * @brief Indicates the period over which DST is in effect
     */
    HDR_sis_dst_schedule_t dst_schedule;

    /**
     * @brief Indicates whether DST is practiced locally
     */
    HDR_sis_dst_local_t dst_local;

    /**
     * @brief Indicates whether DST is in effect within a broad region
     * (i.e., country) at a particular time.
     */
    bool dst_in_effect;

    HDR_sis_status_t status;  /**< Update status */
}HDR_sis_local_time_t;

/**
 * @brief Retrieves the SIS local time
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * @param [out] local_time: Pointer to the output data. Must be allocated by the caller (see #HDR_sis_local_time_t)
 * @return
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_local_time(HDR_instance_t* hdr_instance, HDR_sis_local_time_t* local_time);

/**
 * @brief Defines the maximum station universal name size in bytes
 */
#define HDR_SIS_MAX_UNIV_NAME_LENGTH        (13U)

/**
 * @brief SIS station universal name output data structure
 *
 * The text is null-terminated and length value will not include
 * the null character - matching output of strlen()
 */
typedef struct{
    char text[HDR_SIS_MAX_UNIV_NAME_LENGTH];  /**< Universal name buffer */
    uint_t length;                      /**< Universal name length */
    HDR_sis_text_encoding_t text_encoding;    /**< Text encoding */
    bool append_fm;                           /**< The Append Byte indicates whether "-FM"
                                                   should be appended to the short station */
    HDR_sis_status_t status;                  /**< Update status */
}HDR_sis_univ_name_t;

/**
 * @brief Retrieves the station universal short name
 *
 * Alternative to basic short station name, for countries where different text-encoding schemes may be required.
 *
 * @param [in] hdr_instance  Pointer to the HD Radio instance.
 * @param [out] univ_name: Pointer to the station universal name output. Must be allocated by the caller (see #HDR_sis_univ_name_t)
 * @return
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_universal_name(HDR_instance_t* hdr_instance, HDR_sis_univ_name_t* univ_name);

/**
 * @brief Defines the maximum allowable message size in bytes
 */
#define HDR_SIS_SLOGAN_MAX_SIZE         (128U)

/**
 * @brief SIS station slogan output data structure
 *
 * The text is null-terminated and length value will not include
 * the null character - matching output of strlen()
 */
typedef struct {
    char text[HDR_SIS_SLOGAN_MAX_SIZE];           /**< Slogan buffer */
    uint_t length;                          /**< Slogan length */
    HDR_sis_text_encoding_t text_encoding;        /**< Encoding type */
    HDR_sis_status_t status;                      /**< Update status */
}HDR_sis_station_slogan_t;

/**
 * @brief Retrieves the station slogan.
 *
 * @param [in] hdr_instance: Pointer to the HD Radio instance.
 * @param [out] slogan: Pointer to the station universal name output. Must be allocated by the caller (see #HDR_sis_station_slogan_t)
 * @return
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_station_slogan(HDR_instance_t* hdr_instance, HDR_sis_station_slogan_t* slogan);

/**
 * @brief List of available audio programs reported by SIS.
 */
typedef struct{
    struct {
        HDR_program_t program_number; /**< Program number */
        HDR_sis_status_t status;      /**< Update status */
    }program[HDR_MAX_NUM_PROGRAMS];
    uint_t program_count;       /**< Total number of available programs */
}HDR_sis_avail_programs_t;

/**
 * @brief Retrieves the available audio programs list
 *
 * @param [in] hdr_instance: Pointer to the HD Radio instance
 * @param [out] available_programs: Pointer to the available programs data structure. Must be allocated by the caller (see #HDR_sis_avail_programs_t)
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_avail_programs_list(HDR_instance_t* hdr_instance, HDR_sis_avail_programs_t* available_programs);

/**
 * @brief SIS access types.
 *
 * Program permissions, as assigned by the broadcaster.
 */
typedef enum{
    HDR_SIS_ACCESS_PUBLIC = 0, /**< Public, unrestricted */
    HDR_SIS_ACCESS_RESTRICTED, /**< Restricted */
    HDR_MAX_SIS_ACCESS_TYPE
}HDR_sis_access_type_t;

/**
 * @brief  Audio Program Information reported by SIS
 */
typedef struct{
    uint_t program_type;           /**< Program type (e.g., News, Talk, Information, etc.) */
    uint_t surround_sound;         /**< Applied Sound Experience */
    HDR_sis_access_type_t access;  /**< Program permissions, as assigned by the broadcaster */
    HDR_sis_status_t status;       /**< Update status */
}HDR_sis_program_info_t;

/**
 * @brief Retrieves specified audio program information.
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * @param [in] program_number: Specifies the program number requested (see #HDR_program_t)
 * @param [out] program_info: Pointer to the output data. Must be allocated by the caller (see #HDR_sis_program_info_t)
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_program_info(HDR_instance_t* hdr_instance, HDR_program_t program_number, HDR_sis_program_info_t* program_info);

/**
 * @brief Maximum number of data service allowed in HD Radio
 */
#define HDR_SIS_MAX_NUM_DATA_SERVICES       (32U)

/**
 * @brief Services data type IDs range from 0 to 511
 */
#define HDR_MAX_NUM_SERVICE_DATA_TYPES      (511U)

/**
 * @brief List of the available data services reported by SIS
 */
typedef struct{
    struct {
        uint_t type;           /**< Data service type id */
        HDR_sis_status_t status;     /**< Data service status (freshness) */
    }service[HDR_SIS_MAX_NUM_DATA_SERVICES];
    uint_t service_count;      /**< Number of available data services */
}HDR_sis_avail_data_services_t;

/**
 * @brief Retrieves the available data services list
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * @param [out] available_services: Pointer to the output data. Must be allocated by the caller (see #HDR_sis_avail_data_services_t)
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_avail_data_serv_list(HDR_instance_t* hdr_instance, HDR_sis_avail_data_services_t* available_services);

/**
 * @brief Data services information structure
 */
typedef struct{
    struct {
        uint16_t service_type;        /**< Indicates the service data type (e.g., News, Traffic, Weather, etc.) */
        uint16_t mime_type;           /**< MIME type hash value specifying the data application program type. */
        HDR_sis_access_type_t access; /**< Program permissions, as assigned by the broadcaster.
                                            (0 - Public / Unrestricted; 1 - Restricted) */
        uint8_t status;               /**< Update status 1 - old; 2 - new (updated) */
    }service[HDR_SIS_MAX_NUM_DATA_SERVICES];
    uint_t service_count;       /**< Total number of data services available */
}HDR_sis_data_services_info_t;

/**
 * @brief Retrieves all data services information
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * @param [out] data_services_info: Pointer to the output data. Must be allocated by the caller (see #HDR_sis_data_services_info_t)
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_all_data_services(HDR_instance_t* hdr_instance,  HDR_sis_data_services_info_t* data_services_info);

/**
 * @brief Retrieves specified data service information
 *
 * Upon completion, this function will mark the data as old.
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle
 * @param [in] service_type: Service Type
 * @param [out] data_services_info: Pointer to the output data. Must be allocated by the caller (see #HDR_sis_data_services_info_t)
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_data_services_type(HDR_instance_t* hdr_instance, uint_t service_type, HDR_sis_data_services_info_t* data_services_info);

/**
 * @brief Exciter Core version maximum string length
 */
#define HDR_SIS_CORE_VER_STR_MAX_LEN    (17U)

/**
 * @brief Structure contains Exciter Core Version data
 * Text string contains ISO/IEC 8859-1 character codes within
 * the range of 32 to 126 only
 *
 * The string is null-terminated and length value will not include
 * the null character - matching output of strlen()
 */
typedef struct{
    char string[HDR_SIS_CORE_VER_STR_MAX_LEN]; /**< Text string containing Exciter Core Version */
    uint_t length;                             /**< String length */
}HDR_sis_tx_ver_str_t;

/**
 * @brief returns pointer to the exciter core version string.
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * @param [out] version_string: Pointer to the returned version string. Must be allocated by the caller (see #HDR_sis_tx_ver_str_t)
 * @returns
 *    0 - Success <br>
 *   -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_exciter_core_ver(HDR_instance_t* hdr_instance, HDR_sis_tx_ver_str_t* version_string);

/**
 * @brief Structure contains Exciter Core Version sring and manufacturer id
 *
 * Text string contains ISO/IEC 8859-1 character codes within
 * the range of 32 to 126 only
 */
typedef struct{
    uint8_t right_most_mnf_id;           /**< Rightmost Exciter Manufacturer ID Char */
    uint8_t left_most_mnf_id;            /**< Leftmost Exciter Manufacturer ID Char */
    HDR_sis_tx_ver_str_t version_string;
}HDR_sis_tx_manuf_ver_t;

/**
 * @brief returns pointer to the exciter manufacturer string.
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * @param [out] version_struct: Pointer to the returned manufacturer string. Must be allocated by the caller (see #HDR_sis_tx_manuf_ver_t)

 * @returns
 *    0 - Success <br>
 *   -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_exciter_manuf_ver(HDR_instance_t* hdr_instance, HDR_sis_tx_manuf_ver_t* version_struct);

/**
 * @brief returns pointer to the importer core version string.
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * @param [out] version_string: Pointer to the returned importer core version string. Must be allocated by the caller (see #HDR_sis_tx_ver_str_t)

 * @returns
 *    0 - Success <br>
 *   -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_importer_core_ver(HDR_instance_t* hdr_instance, HDR_sis_tx_ver_str_t* version_string);

/**
 * @brief returns pointer to the manufacturer version string.
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle.
 * @param [out] version_struct: Pointer to the returned manufacturer version string. Must be allocated by the caller (see #HDR_sis_tx_manuf_ver_t)
 *
 * @returns
 *    0 - Success <br>
 *   -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_get_importer_manuf_ver(HDR_instance_t* hdr_instance, HDR_sis_tx_manuf_ver_t* version_struct);

/**
 * @brief Flushes out existing sis data and starts collecting sis data afresh.
 *
 * It clears all data in the SIS output buffers.
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle
 *
 * @returns
 *    0 - Success <br>
 *   -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_flush(HDR_instance_t* hdr_instance);

/**
 * @brief Reports number station location reads by the user since new location data
 * was received
 *
 * This function is only used for 2206 Command that sends station location to the host.
 * The 2206 expects the data to be broken up into two 32-bit words with
 * status being set to new until both words are provided. Since command
 * processor has no memory to store state, this function helps it keep track
 * of what word was sent last.
 *
 * @param [in] hdr_instance: Pointer to the HDR Library instance handle
 * @param [out] count: Number of station location reads since last new location
 *                     was received
 * @returns
 *    0 - Success <br>
 *   -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_sis_location_read_count(HDR_instance_t* hdr_instance, uint_t * count);

#endif //HDR_SIS_H_

/** @} */ // doxygen end HdrSis

