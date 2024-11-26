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
 * @file hdrAlerts.h
 * @brief Emergency Alerts Service API
 * @defgroup HdrAlerts Emergency Alerts
 * @brief Emergency Alerts Service API
 *
 * @ingroup HdrApi
 * @{
 *
 * The HD Radio Emergency Alerts feature enables receivers with the ability to actively alert
 * the radio users to irregular situations that may be possibly life-threatening events. Such
 * alerting may be achieved by rendering the alert content from a currently tuned-in station.
 * The alerting may also be achieved by a breakthrough: that is, when the entertainment system
 * is configured to a different function such as multimedia playback or by means of a secondary
 * tuner pointing to a different station. In addition to alerting, the EA feature provides
 * time-critical information that may possibly include life-saving information; then, the EA
 * feature may also provide event follow-up information.
 *
 * <b>For additional information see:</b><br>
 *    [1] RX_IDD_2206_appendixR - Emergency Alerts - Revision<X>.pdf <br>
 *    [2] RX_TN_3325 - Emergency Alerts Feature Automotive Receiver Host Controller Processing Notes.doc <br>
 *    [3] SY_IRS_2255 - HD Radio System Broadcast Interface Requirements for the Emergency Alerts Features.doc
 */
#ifndef HDR_ALERTS_H_
#define HDR_ALERTS_H_

#include "hdrCore.h"

/**
 * @brief Maximum length of an alert message payload
 */
#ifdef USE_HDRLIB_3RD_CHG_VER
#define HDR_MAX_ALERT_PAYLOAD_LENGTH            (381)
#else
#define HDR_MAX_ALERT_PYALOAD_LENGTH            (381)
#endif

/**
 * @brief Text encoding type for alert message strings
 */
typedef enum HDR_alert_text_encoding_t{
    HDR_ALERT_ISO_IEC_8859_1_1998 = 0,  /**< 8-bit unicode char(default)  */
    HDR_ALERT_ISO_IEC_8859_1_1998C = 1  /**< ISO/IEC 8859-1:1998 compressed */
}HDR_alert_text_encoding_t;

/**
 * @brief Output structure for an HDR emergency alert message
 *
 * Payload includes message ID, message control data(CNT), and text string.
 *
 * <b> See reference [3] for more details. </b>
 */
typedef struct HDR_alert_message_t{
    uint_t payload_crc;                    /**< 7-bit payload CRC value */
    uint_t payload_length;                 /**< Total payload length (7 - 381 bytes) */
    uint_t cnt_length;                     /**< CNT length in bytes */
    bool cnt_crc_pass;                           /**< CNT CRC check status. 12-bit CRC that covers all of the CNT bits */
    HDR_alert_text_encoding_t text_encoding;     /**< Text Encoding of the message string */
    uint_t text_length;                          /**< The length of the text string portion of the payload */
    char* text_message;                          /**< Pointer to the text message of the emergency alert. Set to NULL if not applicable. */
#ifdef USE_HDRLIB_3RD_CHG_VER
	char payload[HDR_MAX_ALERT_PAYLOAD_LENGTH];  /**< Payload Length */
#else
    char payload[HDR_MAX_ALERT_PYALOAD_LENGTH];  /**< Payload Length */
#endif
}HDR_alert_message_t;

/**
 * @brief Retrieves the latest emergency alert message
 *
 * The status information will be cleared after the message is read.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param [out] message: Pointer to emergency alert message. Must be allocated by the caller.
 * @returns
 *     0    Success <br>
 *    -1    HDR emergency alerts service is not enabled <br>
 *    -2    No new message received <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_alert_get_message(HDR_instance_t* hdr_instance, HDR_alert_message_t* message);

/**
 * @brief Output structure for current status of message reception
 */
typedef struct HDR_alerts_msg_status_t {
    bool frame_received;        /**< Indicates whether any alert message frame(piece) has been received */
    bool frame0_available;      /**< Indicates whether frame 0 was received. Frame 0 contains information about the contents of the message */
    bool full_message;          /**< Indicates whether full message was received */
    uint_t frame_counter; /**< Total number of frames received */
    uint_t message_id;    /**< Unique Emergency Alert message ID. (Ranges from 0 to 255) */
    uint_t payload_crc;   /**< 7-bit payload CRC value */
}HDR_alerts_msg_status_t;

/**
 * @brief Provides current status of message reception
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @param[out] status: Pointer to emergency alert status. Must be allocated by the caller.
 * @returns
 *     0    Success <br>
 *    -1    HDR emergency alerts service is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_alert_get_message_status(HDR_instance_t* hdr_instance, HDR_alerts_msg_status_t* status);

/**
 * @brief Clears message status bits
 *
 * This function will clear status bits HDR_alerts_msg_status_t::frame_received,
 * HDR_alerts_msg_status_t::frame0_available, and HDR_alerts_msg_status_t::full_message.
 *
 * This may be useful when keeping track if the library is still receiving message frames.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance
 * @returns
 *     0    Success <br>
 *    -1    HDR emergency alerts service is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_alert_clear_message_status(HDR_instance_t* hdr_instance);

/**
 * @brief Generate emergency alerts audio tone pattern
 *
 * Generates the next frame of samples of emergency alerts tone sequence. When the sequence is finished,
 * the function will set parameter \c finished to true.
 *
 * @param hdr_instance: Pointer to the HDR Library instance
 * @param [in] num_samples: Specifies number of samples(32-bit) to copy to the output buffer
 * @param [out] pcm_output: Buffer storing output audio samples(32-bit). Must be allocated by the caller.
 * @param[out] finished: Signals the caller when the tone sequence is complete.
 *                       Must be allocated by the caller.
 * @returns
 *     0    Success <br>
 *    -1    HDR emergency alerts service is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_alert_get_tone_audio(HDR_instance_t* hdr_instance, HDR_pcm_stereo_t* pcm_output, uint_t num_samples, bool* finished);

#endif //HDR_ALERTS_H_

/** @} */
