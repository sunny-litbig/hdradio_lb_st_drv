/*******************************************************************************
*
* (C) copyright 2003-2015, iBiquity Digital Corporation, U.S.A.
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
 * @file hdrAas.h
 * @brief Advanced Application Service(AAS) API functions and definitions
 * @defgroup HdrAas Advanced Application Service(AAS)
 * @brief Advanced Application Service(AAS) API
 *
 * @ingroup HdrApi
 * @{
 *
 * The HD Radio system allows multiple services to share the broadcast capacity of a single station.
 * First generation (core) services include the Main Program Service (MPS) and the Station Information
 * Service (SIS). Second generation services, referred to as Advanced Application Services - or AAS,
 * include new information services providing, for example, multicast programming, electronic program
 * guides, navigation maps, traffic information, multimedia programming, large object transfer (LOT)
 * and other content. The AAS Framework provides a common infrastructure to support the developers
 * of these services.
 *
 * <b>Port Number Assignment:</b>
 *
 * Port Number     | Usage
 * ----------------|--------------------------
 * 0x0000 - 0x03FF | Reserved for System Use
 * 0x0400 - 0x50FF | Free for use by AAS
 * 0x5100 - 0x5100 | MPSD
 * 0x5101 - 0x51FF | Reserved for System Use
 * 0x5201 - 0x5207 | SPSD
 * 0x5208 - 0x52FF | Reserved for System Use
 * 0x5300 - 0x7CFF | Free for use by AAS
 * 0x7D00 - 0x7EFF | Invalid cannot be used
 * 0x7F00 - 0xFEFF | Free for use by AAS
 * 0xFF00 - 0xFFFF | Reserved for System Use
 *
 * <b>The Large Object Transfer (LOT):</b>
 *
 * The Large Object Transfer (LOT) feature allows to send objects (files) up to 2^32 - 1 bytes long,
 * limited only by the size of the LOT memory pool on the receive side. LOT addresses the problems of
 * sending an object larger than the maximum HD AAS packet size (#HDR_AAS_MAX_PACKET_SIZE), of losing AAS packets
 * due to severe interference and fading, and of sending descriptive information about an object. LOT
 * does not address the issue of sending a group of related objects. LOT operates by sending an object
 * broken up into two or more fragments, each of which is no larger than an HD AAS packet. To mitigate
 * the effect of packet loss, a fragment may be broadcast several times. To include descriptive
 * information about an object, a header is broadcasted with each object.
 *
 * The LOT processing function includes an internal timeout parameter. In previous versions, LOT Timeout was
 * configurable; but, that is no longer the case. LOT Timeout is fixed at 300 seconds.
 *
 * If HD Radio does not receive any LOT object fragments within the LOT Timeout period, then it is
 * assumed that no further fragments will be received for this object and it is automatically flushed.
 * Every fragment received for an object, restarts the timer. Note that the above applies only to incomplete
 * LOT objects; that is, while reassembly is still in progress. Once an object has been fully reassembled, the
 * timeout does not apply and the object will never be automatically flushed.
 *
 * After an LOT object is read or flushed, either via an API call function, or as described above, assembly of
 * another occurrence of the LOT object (with the same LOT ID / Port Number) will be disabled until after the LOT
 * Timeout Period expires.
 *
 * If a LOT timeout is in progress, the timer for a given LOT object is reset by any of the following actions:
 *      - The LOT object data port is disabled and subsequently re-enabled
 *      - LOT reassembly is disabled for the object data port and then subsequently re-enabled
 *      - A reacquisition or re-tune operation occurs
 *
 *  <b>For additional information see:</b><br>
 *    RX_IDD_2206_appendixK - Advanced Applications Services - Revision<X>.pdf
 */
#ifndef HDR_AAS_H_
#define HDR_AAS_H_

#include "hdrCore.h"

/**
 * @brief Defines the maximum number of AAS ports that can be enabled at one time
 */
#define HDR_AAS_MAX_NUMBER_OF_PORTS         (255U)

/**
 * @brief Maximum size of an AAS packet allowed by HD Radio
 */
#define HDR_AAS_MAX_PACKET_SIZE             (4096U)

/**
 * @brief AAS Port types, ordered or non-ordered.
 *
 * In ordered mode, data packets are ordered within the system before they are sent to
 * the caller and in non-ordered mode, data packets are sent to the caller as they arrive.
 * If the port is enabled in non-ordered mode, the caller may use the Sequence Number provided
 * with the packet to accomplish the same thing.
 */
typedef enum HDR_port_mode_t{
    HDR_PORT_NON_ORDERED = 0x00,  /**< Do not order the packets(First-in-first-out) */
    HDR_PORT_ORDERED = 0x04       /**< Order the packets based on the sequence number */
}HDR_port_mode_t;

/**
 * @brief Defines a structure used to specify a list of ports to be enabled/disabled.
 *
 * Used for enabling or disabling ports, as well as retrieving information
 * about currently enabled ports
 */
typedef struct HDR_aas_port_list_t{
    struct{
        uint16_t number;                /**< Port number */
        uint8_t mode;                   /**< Port mode (ordered or non-ordered).@see HDR_port_mode_t */
    }port[HDR_AAS_MAX_NUMBER_OF_PORTS]; /**< Stores the port list in array */
    uint8_t num_ports;					/**< Number of AAS ports in the list */
}HDR_aas_port_list_t;

/**
 * @brief Enables specified ports
 *
 * With this function, ports may be enabled in one of two modes. In ordered mode,
 * data packets will be sent in order determined by their sequence number
 * and in non-ordered mode data packets are sent as they arrive. If the port
 * is enabled in non-ordered mode, you may still use the Sequence Number to
 * re-order the packets. For most applications, non-ordered mode is recommended.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] port_list: Specifies the AAS ports to be enabled.
 * @returns
 *     0 - Success <br>
 *    -1 - At least one port was not enabled <br>
 *    -100 or less - Generic error(see #HDR_error_code_t)
 */
int_t HDR_aas_enable_ports(HDR_instance_t* hdr_instance, const HDR_aas_port_list_t* port_list);

/**
 * @brief Disables specified ports
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] port_list: Specifies the AAS ports to be disabled. Port mode is ingnored.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error(see #HDR_error_code_t)
 */
int_t HDR_aas_disable_ports(HDR_instance_t* hdr_instance, const HDR_aas_port_list_t* port_list);

/**
 * @brief Disables all enabled ports
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 *
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error(see #HDR_error_code_t)
 */
int_t HDR_aas_disable_all_ports(HDR_instance_t* hdr_instance);

/**
 * @brief Reports information about currently enabled AAS ports
 *
 * Information includes the number of ports enabled, enabled port numbers, and enabled
 * port mode(see #HDR_port_mode_t). The port list will include LOT ports enabled via
 * command HDR_aas_enable_lot_reassembly().
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] port_list: The information about the enabled AAS ports.
 *                         Must be allocated by the caller.
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error(see #HDR_error_code_t)
 * @see HDR_aas_enable_ports().
 */
int_t HDR_aas_get_enabled_ports(HDR_instance_t* hdr_instance, HDR_aas_port_list_t* port_list);

/**
 * @brief Information associated with a received packet
 */
typedef struct HDR_aas_packet_info_t{
    uint_t num_packets_avail;    /**< Number of packets still available on this port.(Not including this one) */
    bool overflow_status;              /**< Indicates whether the port queue overflowed. At least one(oldest) packet was deleted */
    uint_t port_number;          /**< Packet port number (16-bit number). */
    uint_t sequence_number;      /**< Packet sequence number. */
    uint_t packet_length;        /**< Packet length in bytes. */
    uint_t num_bytes_unread;     /**< Number of bytes left to read. Used when buffer provided is smaller than the packet length. */
}HDR_aas_packet_info_t;

/**
 * @brief Retrieves the next available packet from any enabled port.
 *
 * This function will look through enabled ports in the same order they were enabled(early enabled ports will always get priority),
 * and retrieve the first available packet. If the packet is too large for the buffer provided, the subsequent calls to this
 * function will return the rest of the packet data.
 *
 * The packets are sent in ordered or non-ordered mode, as determined by
 * the port mode. (see #HDR_port_mode_t)
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] packet_info: Pointer to the packet information structure. Must be allocated by the caller.
 * @param[out] packet_buffer: Pointer to the packet output buffer. Must be allocated by the caller.
 * @param[in] buffer_size: Size of the output buffer provided.
 * @param[out] bytes_written: Number of bytes written into the buffer.
 * @returns
 *     0 - Success <br>
 *    -1 - No data available <br>
 *    -2 - No ports enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_aas_get_next_port_data(HDR_instance_t* hdr_instance, HDR_aas_packet_info_t* packet_info,
                                 uint8_t* packet_buffer, uint_t buffer_size, uint_t * bytes_written);

/**
 * @brief Retrieves the next available packet from the specified port number
 *
 * The packets are sent in ordered or non-ordered format, as determined by
 * the port mode. (see #HDR_port_mode_t)
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] port_number: Port Number of requested data.
 * @param[out] packet_info: Pointer to the packet information structure. Must be allocated by the caller.
 * @param[out] packet_buffer: Pointer to the packet output buffer. Must be allocated by the caller.
 * @param[in] buffer_size: Size of the output buffer provided.
 * @param[out] bytes_written: Number of bytes written into the buffer.
 * @returns
 *     0 - Success <br>
 *    -1 - No data available <br>
 *    -2 - Port is not enabled <br>
 *    -3 - Packet is corrupted <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_aas_get_port_data(HDR_instance_t* hdr_instance, uint_t port_number, HDR_aas_packet_info_t* packet_info,
                            uint8_t* packet_buffer, uint_t buffer_size, uint_t * bytes_written);

/**
 * @brief Flushes all the data waiting on the specified port number
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] port_number: Specified port number to flush
 * @returns
 *     0 - Success <br>
 *    -1 - Port is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_aas_flush_port(HDR_instance_t* hdr_instance, uint_t port_number);

/**
 * @brief Flushes all the data waiting on all enabled ports.
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @returns
 *     0 - Success <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_aas_flush_all_ports(HDR_instance_t* hdr_instance);

/**
 *  Maximum number of ports that may be enabled for LOT
 */
#define HDR_AAS_MAX_NUM_LOT_PORTS   (64U)

/**
 *  Maximum number of concurrent LOT objects allowed
 */
#define HDR_MAX_NUM_LOT_OBJECTS     (200U)

/**
 * Maximum allowable file name length for LOT object
 */
#define HDR_AAS_LOT_MAX_FILENAME_LENGTH   (231U)

/**
 * @brief Returns total lot memory pool size(in bytes).
 *
 * <b>This function only applies if the LOT processing is both supported and activated.Otherwise, returns -1.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[out] mem_pool_size: Total pool size for LOT data
 * @returns
 *     0 - Success <br>
 *    -1 - LOT is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_aas_get_lot_pool_size(HDR_instance_t* hdr_instance, uint_t * mem_pool_size);

/**
 * @brief Returns lot memory(in bytes) left in the pool.
 *
 * <b>This function only applies if the LOT processing is both supported and activated.
 * Otherwise, returns 0.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @return LOT memory left
 */
uint_t HDR_aas_get_lot_space_left(HDR_instance_t* hdr_instance);

/**
 * @brief Detects if overflow condition occurred
 *
 * Overflow flag is triggered when the LOT memory pool has no memory left for new LOT files.
 * Complete LOT files are stored in memory until they are retrieved via
 * HDR_aas_get_lot_object_body() or flushed via HDR_aas_flush_lot_object().
 *
 * <b>This function only applies if the LOT processing is both supported and activated.
 * Otherwise, returns false.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @return
 *     true  - overflow occurred        <br>
 *     false - no overflow occurred
 */
bool HDR_aas_lot_overflow(HDR_instance_t* hdr_instance);

/**
 * @brief Enables the specified port number and initiates LOT reassembly of all LOT
 * objects received on that port.
 *
 * To associate one service number with multiple port numbers, this command should
 * be sent once for each port number with the same service number each time.
 *
 * <b>This function only applies if the LOT processing is both supported and activated.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] service_number: Specifies the service number that should be associated with the port number
 * @param[in] port_number: Specifies the port number to enable.
 * @return
 *     0 - Success <br>
 *    -1 - LOT is not enabled <br>
 *    -2 - Reserved port number requested <br>
 *    -3 - Maximum number of AAS ports already enabled <br>
 *    -4 - Maximum number of LOT ports already enabled <br>
 *    -5 - Specified Port Number is already open
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_aas_enable_lot_reassembly(HDR_instance_t* hdr_instance, uint_t service_number, uint_t port_number);

/**
 * @brief Stops LOT reassembly for a specified service number and port number, or all
 * enabled ports for that service number.
 *
 * If the Port Number is 0x0000, all Ports enabled for this Service Number will be disabled.
 * All LOT objects associated with that port/service number will be flushed.
 *
 * <b>This function only applies if the LOT processing is both supported and activated.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] service_number: Specifies the service number that should be disabled
 * @param[in] port_number: Specifies the port number to disable. Use port number 0 to
 *                          disable all ports associated with the specified service number
 * @return
 *     0 - Success <br>
 *    -1 - LOT is not enabled <br>
 *    -2 - Specified port number or service number not found <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_aas_disable_lot_reassembly(HDR_instance_t* hdr_instance, uint_t service_number, uint_t port_number);

/**
 * @brief List of all complete and incomplete LOT objects
 */
typedef struct HDR_aas_lot_object_list_t{
    struct {
        uint16_t port_number;       /**< Port number origin of the LOT object */
        uint16_t lot_id;            /**< LOT id number */
        uint8_t  complete;          /**< Flag indicating if the object is complete( 1 - complete) */
    }item[HDR_MAX_NUM_LOT_OBJECTS]; /**< LOT object list */
    uint_t num_objects;       /**< Number of LOT items */
}HDR_aas_lot_object_list_t;

/**
 * @brief Gets the current list of all complete and incomplete objects for the specified service number
 *
 * <b>This function only applies if the LOT processing is both supported and activated.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] service_number: Specifies the service number associated with the lot objects
 * @param[out] object_list: Stores the list of the LOT objects. Must be allocated by the caller.
 * @return
 *     0 - Success <br>
 *    -1 - LOT is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_aas_get_lot_object_list(HDR_instance_t* hdr_instance, uint_t service_number, HDR_aas_lot_object_list_t* object_list);

/**
 * @brief Gets the current list of all complete and incomplete objects for the service number
 *        whose file names match the requested file name
 *
 * <b>This function only applies if the LOT processing is both supported and activated.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] service_number: Specifies the service number associated with the lot objects
 * @param[in] filename: Specifies the Null-terminated filename of the objects
 *                      Filename string can't exceed 231 characters including the NULL characters
 * @param [out] object_list: Stores the list of complete and incomplete objects. Storage must be allocated by the caller.
 * @return
 *     0 - Success <br>
 *    -1 - LOT is not enabled <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_aas_lot_get_object_list_by_name(HDR_instance_t* hdr_instance, uint_t service_number, const char* filename, HDR_aas_lot_object_list_t* object_list);

/**
 * @brief LOT object header definition
 *
 *  The set of supported LOT file MIME types and their
 *  corresponding hash values are listed below:
 *
 *    MIME Types   | Four-Byte Hash Value
 * ----------------|----------------------
 *  none           |      0x806FFF30
 *  text/plain     |      0xBB492AAC
 *  text/enriched  |      0x7074B716
 *  image/gif      |      0x6E1D9F04
 *  image/jpeg     |      0x1E653E9C
 *  audio/basic    |      0x06362BAE
 *  video/mpeg     |      0x761FB167
 */
typedef struct HDR_aas_lot_object_header_t{
    uint32_t discard_time;                             /**< The year, month, day, hour, and minute, in Coordinated Universal Time (UTC),
                                                            after which the object may be discarded by the receiver. */
    uint32_t file_size;                                /**< Object Size(in bytes) */
    uint32_t mime_hash;                                /**< File MIME Type Hash Value */
    uint8_t filename[HDR_AAS_LOT_MAX_FILENAME_LENGTH]; /**< File Name(Maximum length is 231 bytes) */
    uint8_t filename_length;                           /**< Actual file name length */
}HDR_aas_lot_object_header_t;

/**
 * @brief Gets the header of an object
 *
 * <b>This function only applies if the LOT processing is both supported and activated.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] port_number: Specifies the port number of the object
 * @param[in] object_lot_id: Specifies the object LOT id
 * @param[out] object_header: Pointer to the structure, where the header should be stored. Storage must be allocated by the caller.
 * @return
 *     0 - Success <br>
 *    -1 - LOT is not enabled <br>
 *    -2 - Object not found or header wasn't received yet <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_aas_get_lot_object_header(HDR_instance_t* hdr_instance, uint_t port_number, uint_t object_lot_id, HDR_aas_lot_object_header_t* object_header);

/**
 * @brief Gets the next block of data from the body of the LOT object
 *
 * The caller may read the body in blocks as small as one byte, as determined by the \e buffer_size parameter.
 * For each execution, this function will write \e buffer_size bytes of the object if available. If this
 * function is executed and the last data byte of the object is read, then the output buffer may contain
 * fewer than \e buffer_size bytes, indicated by \e bytes_written being smaller than \e buffer_size. The next
 * execution, no bytes will be written and the return value will indicate end-of-file. In other words, one additional
 * call to HDR_aas_get_lot_object_body() is needed, after the last of the bytes are transferred, to get the return
 * value of 1(end-of-file). This last call will write zero bytes into the output buffer.
 *
 * <b>This function only applies if the LOT processing is both supported and activated.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] port_number: Specifies the port number of the object
 * @param[in] object_lot_id: Specifies the object LOT id.
 * @param[out] buffer: Pointer to the buffer where the object body should be stored. Storage must be provided by the caller.
 * @param[out] buffer_size: Size of the buffer
 * @param[out] bytes_written: Actual number of bytes written to the buffer. Storage must be provided by the caller.
 * @return
 *     0 - Not end-of-file <br>
 *     1 - End-of-file <br>
 *    -1 - LOT is not enabled <br>
 *    -2 - Object not found or is incomplete <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_aas_get_lot_object_body(HDR_instance_t* hdr_instance, uint_t port_number, uint_t object_lot_id,
                                  uint8_t* buffer, uint_t buffer_size, uint_t * bytes_written);

/**
 * @brief Flushes an object from LOT memory
 *
 * After object is flushed, LOT reassembly of an object with this lot id is blocked until the LOT Timeout
 * period (300 seconds) has expired.
 *
 * <b>This function only applies if the LOT processing is both supported and activated.</b>
 *
 * @param[in] hdr_instance: Pointer to the HDR Library instance handle
 * @param[in] port_number: Specifies the port number of the object
 * @param[in] object_lot_id: Specifies the object LOT id
 * @return
 *     0 - Success <br>
 *    -1 - LOT is not enabled <br>
 *    -2 - Object not found <br>
 *    -100 or less - Generic error.(see #HDR_error_code_t)
 */
int_t HDR_aas_flush_lot_object(HDR_instance_t* hdr_instance, uint_t port_number, uint_t object_lot_id);

#endif //HDR_AAS_H_

/** @} */ //doxygen end-bracket
