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
 * @file hdrPsd.h
 * @brief Program Service Data (PSD) API functions and definitions
 * @defgroup HdrPsd Program Service Data (PSD)
 * @brief Program Service Data (PSD) API
 *
 * @ingroup HdrApi
 * @{
 *
 * Program Service Data (PSD) consists of a general set of categories that describe the various
 * programming content, such as a song, talk show, advertisement, or announcement. For example,
 * the Title field can be used to describe the name of a song, topic of a talk show, advertisement,
 * or announcement.
 *
 * Accurate display of the Program Info fields for all of the programs is important so that songs
 * are clearly identified for the listener and so that songs may be tagged properly for later purchase
 * through the iTunes Tagging feature.
 *
 * The PSD fields include the following:
 *    - Title
 *	  - Artist
 *	  - Album
 *	  - Genre
 *	  - Comment
 *	  - Commercial
 *	  - Unique File Identifier (UFID)
 *
 * Program audio and associated PSD are transmitted synchronously, so receivers can acquire correlated
 * audio and data at the same time. PSD messages are continuously transmitted with the most recent message
 * transmitted repeatedly.
 *
 * PSD is formatted using a subset of the standard called ID3v2.3.0, where each PSD field corresponds to an ID3 tag.
 * Historically, ID3 has been used to allow textual information, such as artist, title, and genre information,
 * to co-exist within MPEG-3 (MP3) audio files. The HD Radio system uses ID3 to deliver Program Service Data along
 * with real-time broadcast audio.
 *
 * HD Radio Lib API provides a set of functions to aid designers in implementing the ID3 parsing method required
 * for displaying Program Service Data.
 *
 * In addition to PSD decode functions there are several configuration parameters that allow the user to
 * customize the response format of the PSD returned from the HD Radio library and to customize the PSD fields
 * that will generate a change flag.
 *
 * <b>For additional information see:</b><br>
 *    http://www.id3.org/id3v2.3.0#sec3.3 - details about ID3 Frame format <br>
 *    RX_IDD_2206 Section 5.5.9 PSD_decode(0x93)
 */
#ifndef HDR_PSD_H_
#define HDR_PSD_H_

#include "hdrAudio.h"

/**
 * @brief Supported PSD fields
 *
 * This structure contains a bitmap used to enable/disable PSD fields. Enabled fields will be
 * parsed by the HD Radio library and trigger the PSD change flag when new PSD is received.
 */
 typedef union HDR_psd_fields_t{
    struct{
        uint8_t title:1;      /**< Song title */
        uint8_t artist:1;     /**< Artist name */
        uint8_t album:1;      /**< Album name*/
        uint8_t genre:1;      /**< Genre */
        uint8_t comment:1;    /**< General comments */
        uint8_t UFID:1;       /**< Unique File Identifier */
        uint8_t commercial:1; /**< For advertising purposes */
        uint8_t XHDR:1;       /**< Image synchronization trigger */
    };
    uint8_t all; /**< Used to read/write all fields at once */
}HDR_psd_fields_t;

/**
 * @brief Retrieves PSD content change flags
 *
 * The change flag associated with a certain program is set whenever all of the following conditions are true
 * for that program:
 *    - The PSD Decode service must be activated in the software.
 *    - New PSD data is received: new PSD data means that the ID3 tag data has changed in some way
 *      compared to the previously received ID3 tag. A repeated ID3 tag will not be flagged as a change.
 *    - The change must have occurred in an enabled PSD field. For example, if the artist field is not enabled 
 *      and a new ID3 tag is received where only the artist field is different, the change flag will not be set.
 * The change flag(s) will remain set until they are cleared using function #HDR_psd_clear_changed_program().
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 *
 * @returns Bitfield data structure with flags set to 1 for programs for which PSD changed
 */
HDR_program_bitmap_t HDR_psd_get_changed_programs(HDR_instance_t* hdr_instance);

/**
 * @brief Clears PSD content change flag for the specified program
 *
 * The library has no way of knowing when the host is finshed processing all the PSD data and
 * no longer needs the flag, so it's up to the host to clear it.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] program_number: Clear flag for this program
 * @see #HDR_psd_get_changed_programs
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_psd_clear_changed_program(HDR_instance_t* hdr_instance, HDR_program_t program_number);

/**
 * @brief Specifies which PSD fields to process
 *
 * Information for enabled fields will be parsed and refreshed. Any new PSD for the field will trigger the
 * PSD change flag to be set. (see #HDR_psd_get_changed_programs())
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] enabled_fields: Specifies which PSD fields to enable
 * @return
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_psd_enable_fields(HDR_instance_t* hdr_instance, HDR_psd_fields_t enabled_fields);

/**
 * @brief Returns currently enabled PSD fields
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 *
 * @returns Currently enabled PSD fields
 */
HDR_psd_fields_t HDR_psd_get_enabled_fields(HDR_instance_t* hdr_instance);

/**
 * @brief List of PSD field/subfield length configuration parameters
 *
 * HD Radio lib can be configured to truncate the length of some PSD fields/subfields
 */
typedef enum HDR_psd_length_config_t{
    HDR_PSD_TITLE_LENGTH_CONFIG,
    HDR_PSD_ARTIST_LENGTH_CONFIG,
    HDR_PSD_ALBUM_LENGTH_CONFIG,
    HDR_PSD_GENRE_LENGTH_CONFIG,
    HDR_PSD_COMM_SHORT_CONTENT_LENGTH_CONFIG,
    HDR_PSD_COMM_ACTUAL_TEXT_LENGTH_CONFIG,
    HDR_PSD_UFID_OWNER_ID_LENGTH_CONFIG,
    HDR_PSD_COMR_PRICE_STRING_LENGTH_CONFIG,
    HDR_PSD_COMR_CONTACT_URL_LENGTH_CONFIG,
    HDR_PSD_COMR_SELLER_NAME_LENGTH_CONFIG,
    HDR_PSD_COMR_DESCRIPTION_LENGTH_CONFIG,
    HDR_PSD_XHDR_LENGTH_CONFIG,
    HDR_PSD_NUM_FIELD_CONFIG
}HDR_psd_length_config_t;

/**
 * @brief Truncates a specified PSD field/subfield to a user defined length
 *
 * The user defined length must be less than or equal to the maximum allowed length specified by
 * the table below. By default all fields are set to maximum value. The length specified does not
 * include null-terminating character for PSD fields that are strings.
 *
 * <b> PSD field max/min length: </b>
 *
 * PSD Field                         |  Length
 * ----------------------------------|----------
 * Title                             | 16 to 127
 * Artist                            | 16 to 127
 * Album                             | 16 to 127
 * Genre                             | 16 to 127
 * Comment Short Content Description | 16 to 127
 * Comment Actual Text               | 16 to 255
 * UFID Owner Identifier             | 16 to 255
 * Commercial Price String           | 16 to 127
 * Commercial Contact URL            | 16 to 127
 * Commercial Name of Seller         | 16 to 127
 * Commercial Description            | 16 to 255
 * XHDR                              | 16 to 127
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] config: Specifies the PSD field/subfield to be truncated (see #HDR_psd_length_config_t)
 * @param[in] length: New user defined maximum length of the field/subfield
 * @returns
 *     0 - Success <br>
 *    -1 - Invalid length for the specified field <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_psd_set_max_length(HDR_instance_t* hdr_instance, HDR_psd_length_config_t config, uint_t length);

/**
 * @brief Resets the PSD field/subfield to maximum length
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] config: Specifies the PSD field/subfield to be truncated (see #HDR_psd_length_config_t)
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_psd_reset_max_length(HDR_instance_t* hdr_instance, HDR_psd_length_config_t config);

/**
 * @brief Returns the currently set maximum field length.
 *
 * By default all fields are set to maximum value defined. (see #HDR_psd_set_max_length)
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] config: Specifies the PSD field/subfield  (see #HDR_psd_length_config_t)
 * @return Current maximum length of the requested field/subfield.
 */
uint_t HDR_psd_get_max_length(HDR_instance_t* hdr_instance, HDR_psd_length_config_t config);

/**
 * @brief Text encoding type for PSD message strings
 */
typedef enum HDR_psd_text_encoding_t{
    HDR_PSD_ISO_IEC_8859_1_1998 = 0,   /**< 8-bit unicode character  */
    HDR_PSD_ISO_IEC_10646_1_2000 = 1,  /**< 16-bit unicode character (Little-endian) */
    HDR_PSD_BINARY = 0xFF              /**< binary/no explicit type */
}HDR_psd_data_type_t;

/**
 * @brief Maximum number of characters allowed by the system for largest PSD field/subfield.
 *
 * The actual number can be configured by the user @see HDR_psd_set_max_length()
 */
#define HDR_PSD_MAX_FIELD_LENGTH   		(256)

/**
 * @brief Defines output data structure for PSD fields/subfields
 *
 * If PSD data type is a string, it will be null-terminated and length value will not include
 * the null character - matching output of strlen()
 */
typedef struct HDR_psd_data_t{
   char data[HDR_PSD_MAX_FIELD_LENGTH]; /**< PSD data buffer */
   uint_t length;                 /**< Data length(not including null character */
   HDR_psd_data_type_t data_type;       /**< Data type */
}HDR_psd_data_t;

/**
 * @brief Gets PSD title data
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] program_number: Specifies the program number requested
 * @param[in] title: Pointer to output data. Must be allocated by the caller. (see #HDR_psd_data_t)
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @returns
 *     0 - Success <br>
 *    -1 - The field is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_psd_get_title(HDR_instance_t* hdr_instance, HDR_program_t program_number, HDR_psd_data_t* title);

/**
 * @brief Gets PSD artist data
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] program_number: Specifies the program number requested
 * @param[in] artist: Pointer to output data. Must be allocated by the caller. (see #HDR_psd_data_t)
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @returns
 *     0 - Success <br>
 *    -1 - The field is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_psd_get_artist(HDR_instance_t* hdr_instance, HDR_program_t program_number, HDR_psd_data_t* artist);

/**
 * @brief Gets PSD album data
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] program_number: Specifies the program number requested
 * @param[in] album: Pointer to output data. Must be allocated by the caller. (see #HDR_psd_data_t)
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @returns
 *     0 - Success <br>
 *    -1 - The field is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_psd_get_album(HDR_instance_t* hdr_instance, HDR_program_t program_number, HDR_psd_data_t* album);

/**
 * @brief Gets PSD genre data
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] program_number: Specifies the program number requested
 * @param[in] genre: Pointer to output data. Must be allocated by the caller. (see #HDR_psd_data_t)
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @returns
 *     0 - Success <br>
 *    -1 - The field is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_psd_get_genre(HDR_instance_t* hdr_instance, HDR_program_t program_number, HDR_psd_data_t* genre);

/**
 * @brief List of supported comment subfields
 */
typedef enum HDR_psd_comm_subfield_t{
    HDR_PSD_COMMENT_LANGUAGE,
    HDR_PSD_COMMENT_SHORT_CONTENT,
    HDR_PSD_COMMENT_ACTUAL_TEXT,
    HDR_PSD_NUM_COMMENT_SUBFIELDS
}HDR_psd_comm_subfield_t;

/**
 * @brief Gets PSD comment field/subfield data
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] program_number: Specifies the program number requested
 * @param[in] subfield: Specified requested subfield (see #HDR_psd_comm_subfield_t)
 * @param[in] comment: Pointer to output data. Must be allocated by the caller. (see #HDR_psd_data_t)
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @returns
 *     0 - Success <br>
 *    -1 - The field is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_psd_get_comment(HDR_instance_t* hdr_instance, HDR_program_t program_number, HDR_psd_comm_subfield_t subfield, HDR_psd_data_t* comment);

/**
 * @brief A single UFID ID3 tag may contain up to four UFID frames
 *
 * This could occur if the broadcast supports multiple applications,
 * each with their own type of identifier.
 */
#define HDR_MAX_NUM_UFIDS        (4U)

/**
 * @brief List of supported UFID subfields
 */
typedef enum HDR_psd_ufid_subfield_t{
    HDR_PSD_UFID_OWNER_ID,
    HDR_PSD_UFID_FILE_ID,
    HDR_PSD_NUM_UFID_SUBFIELDS,
}HDR_psd_ufid_subfield_t;

/**
 * @brief Gets PSD ufid field/subfield data
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] program_number: Specifies the program number requested
 * @param[in] ufid_num: Specified UFID frame (see #HDR_MAX_NUM_UFIDS)
 * @param[in] subfield: Specified requested subfield (see #HDR_psd_ufid_subfield_t)
 * @param[in] ufid: Pointer to output data. Must be allocated by the caller (see #HDR_psd_data_t)
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @returns
 *     0 - Success <br>
 *    -1 - The field is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_psd_get_ufid(HDR_instance_t* hdr_instance, HDR_program_t program_number, uint_t ufid_num,
                     HDR_psd_ufid_subfield_t subfield, HDR_psd_data_t* ufid);

/**
 * @brief List of supported commercial subfields
 */
typedef enum HDR_psd_comr_subfield_t{
    HDR_PSD_COMR_PRICE_STRING,
    HDR_PSD_COMR_VALID_UNTIL,
    HDR_PSD_COMR_CONTACT_URL,
    HDR_PSD_COMR_RECEIVED_AS,
    HDR_PSD_COMR_SELLER_NAME,
    HDR_PSD_COMR_DESCRIPTION,
    HDR_PSD_NUM_COMR_SUBFIELDS
}HDR_psd_comr_subfield_t;

/**
 * @brief Gets PSD commercial field/subfield data
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] program_number: Specifies the program number requested
 * @param[in] subfield: Specified requested subfield (see #HDR_psd_comr_subfield_t)
 * @param[in] commercial: Pointer to output data. Must be allocated by the caller (see #HDR_psd_data_t)
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @returns
 *     0 - Success <br>
 *    -1 - The field is not enabled
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_psd_get_commercial(HDR_instance_t* hdr_instance, HDR_program_t program_number, HDR_psd_comr_subfield_t subfield, HDR_psd_data_t* commercial);

/**
 * @brief XHDR ID3 tag may contain up to four XHDR frames
 */
#define HDR_MAX_NUM_XHDRS       (4U)

/**
 * @brief Gets PSD ufid field/subfield data
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] program_number: Specifies the program number requested
 * @param[in] xhdr_num: Specified UFID frame (see #HDR_MAX_NUM_UFIDS)
 * @param[in] xhdr: Pointer to output data. Must be allocated by the caller (see #HDR_psd_data_t)
 *
 * <b>This function only applies if PSD decoding is both supported and activated.</b>
 *
 * @returns
 *     0 - Success <br>
 *    -1 - The field is not enabled
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_psd_get_xhdr(HDR_instance_t* hdr_instance, HDR_program_t program_number, uint_t xhdr_num, HDR_psd_data_t* xhdr);


#endif // HDR_PSD_H_

/** @} */ //doxygen end-bracket
