/*******************************************************************************

*   FileName : tchdr_bytestream.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio Framework Byte Stream functions and definitions

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
#ifndef TCHDR_BYTESTREAM_H__
#define TCHDR_BYTESTREAM_H__

/***************************************************
*				Include					*
****************************************************/
#include <stdio.h>
#include "hdrBasicTypes.h"

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define	HDR_ETH_ENABLED

/***************************************************
*				Enumeration				*
****************************************************/
/**
 * brief Supported device types that can be connected to this byte stream module
 */
typedef enum {
    eHDR_DEV_MEMORY	=1, /**< Standard memory buffer */
    eHDR_DEV_FILE,      /**< File I/O */
    eHDR_DEV_SPI,       /**< Serial Peripheral Interface(SPI) */
    eHDR_DEV_UART,      /**< Universal Asynchronous Receiver/Transmitter (UART) */
    eHDR_DEV_ETHER      /**< Ethernet */
}eHDR_DEV_TYPE_t;

/***************************************************
*				Typedefs					*
****************************************************/
/**
 * brief Byte stream control descriptor to connect to memory buffer
 */
typedef struct{
    U8* bufPtr;	/**< Start address of the memory buffer */
    U32 size;			/**< Memory buffer size */
    //Private data below; don't modify
    U32 rp;				/**< Next read position */
    U32 wp;				/**< Next write position */
}stHDR_MEM_DESC_t;

/**
 * brief Byte stream control descriptor to connect to FILE I/O
 */
typedef struct{
    FILE* fp;           /**< File handle */
    S8 filename[128]; /**< Filename */
    S8 mode[4];       /**< Open mode */
    HDBOOL loop;          /**< Automatically go to the beginning after EOF*/
}stHDR_FILEIO_DESC_t;

/**
 * brief Byte stream control descriptor to connect to UART
 */
typedef struct{
    U32 port;  /**< Local UART port */
    U32 rate;
}stHDR_UART_DESC_t;

/**
 * brief Byte stream control descriptor to connect to SPI
 */
typedef struct{
    U32 port;       /**< Local SPI port */
    U32 dmaChannel; /**< DMA channel to use for SPI */
    U32 mode;       /**< SPI mode (0 to 3) */
}stHDR_SPI_DESC_t;

/**
 * brief Byte stream descriptor to connect to Socket
 */
typedef struct{
	U32 port;
    U32 type;		//ie. UDP/TCP
}stHDR_SOCK_DESC_t;

/**
 * Union of all device descriptors
 */
typedef union{
    stHDR_MEM_DESC_t       mem;		/**< Memory buffer */
    stHDR_FILEIO_DESC_t    fileIo;	/**< File I/O */
    stHDR_UART_DESC_t      uart;	/**< UART */
    stHDR_SPI_DESC_t       spi;		/**< SPI */
    stHDR_SOCK_DESC_t      ether;
}stHDR_DEV_DESC_t;

/**
 * brief Defines byte stream object for a device
 */
typedef struct{
    eHDR_DEV_TYPE_t deviceType;		/**< Specifies device type of the byte stream */
    stHDR_DEV_DESC_t dev;			/**< Configuration related to the device */
    HDBOOL isOpen;
}stHDR_BYTE_STREAM_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
/**
 * brief Opens (initializes a byte stream)
 *
 * param byteStream: Byte stream handle. Must be configured before byte stream can be used.
 * return
 *     0 - Success <br>
 *    -1 - Failure
 */
S32 tchdr_bytestream_open(stHDR_BYTE_STREAM_t* byteStream);

/**
 * brief Closes the stream.
 *
 * param[in] byteStream: Byte stream handle.
 * return
 *     0 - Success <br>
 *    -1 - Failure
 */
S32 tchdr_bytestream_close(stHDR_BYTE_STREAM_t* byteStream);

/**
 * brief Reads a specified number of bytes from the byte stream
 *
 * param[in] byteStream: Byte stream handle.
 * param[out] buffer: Pointer to the buffer where to place read bytes
 * param[out] numberBytes: Number of bytes to read
 * return
 *   number of bytes read or -1 if error
 */
S32 tchdr_bytestream_read(stHDR_BYTE_STREAM_t* byteStream, U8* buffer, U32 numberBytes);

/**
 * brief Writes a specified number of bytes to the byte stream.
 *
 * param[in] byteStream: Byte stream handle
 * param[in] buffer: Pointer to the buffer to write bytes from
 * param[in] numberBytes: number of bytes to write
 * param[in] timeout: abort after timeout number of milliseconds
 * return
 *    number of bytes read or -1 if error
 */
S32 tchdr_bytestream_write(stHDR_BYTE_STREAM_t* byteStream, const U8* buffer, U32 numberBytes, S32 timeout);

/**
 * brief Returns number of available bytes that can be read from the byte stream
 *
 * param[in] byteStream: Byte stream handle
 * return Number of bytes available for reading
 */
U32 tchdr_bytestream_availBytes(const stHDR_BYTE_STREAM_t* byteStream);

/**
 * brief Space (in bytes) available for writing
 * May not apply to all Byte Stream devices
 *
 * param[in] byteStream: Byte stream handle
 * return  space available for writting
 */
U32 tchdr_bytestream_availSpace(const stHDR_BYTE_STREAM_t* byteStream);

/**
 * brief Resets the byte stream state and wipes away all data
 * param[in] byteStream: Byte stream handle
 * return
 *     0 - Success <br>
 *    -1 - Failure
 */
S32 tchdr_bytestream_reset(stHDR_BYTE_STREAM_t* byteStream);

#ifdef __cplusplus
}
#endif

#endif //TCHDR_BYTESTREAM_H__
