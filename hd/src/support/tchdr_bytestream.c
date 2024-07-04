/*******************************************************************************

*   FileName : tchdr_bytestream.c

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

/***************************************************
*		Include 			   					*
****************************************************/
#include <string.h>
#include <errno.h>

#include "tchdr_systypes.h"
#include "tchdr_common.h"
#include "tchdr_log.h"
#include "tchdr_bytestream.h"

#ifdef HDR_ETH_ENABLED
#include "tchdr_ethernet.h"
#endif
#ifdef UART_ENABLED
    #include "tchdr_uart.h"
#endif
#ifdef SPI_ENABLED
    #include "tchdr_spi.h"
#endif

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
/**
 * Note: for ARM UART, HDR_uartOpen() returns the assigned Port#
 */
S32 tchdr_bytestream_open(stHDR_BYTE_STREAM_t* byteStream)
{
    S32 rc = 0;

	if(byteStream != NULL) {
	    switch(byteStream->deviceType){
	    	case eHDR_DEV_ETHER:
				#ifdef HDR_ETH_ENABLED
	                rc = tchdr_ethOpen(byteStream->dev.ether.port, byteStream->dev.ether.type );
	            #else
					LOG(FWK, "Tried to open unsupported device type ETHER.", void);
					rc = -1;
				#endif
	   			break;
	    	case eHDR_DEV_MEMORY:
	            if(byteStream->dev.mem.bufPtr== NULL){
	                LOG(FWK, "NULL buffer pointer provided for BUFFER type byte stream.", void);
	                rc = -1;
	            }
	            byteStream->dev.mem.rp = 0;
	            byteStream->dev.mem.wp = 0;
	            break;
	        case eHDR_DEV_FILE:
	            byteStream->dev.fileIo.fp = fopen(byteStream->dev.fileIo.filename, byteStream->dev.fileIo.mode);
	            if (byteStream->dev.fileIo.fp == NULL) {
	                LOG(FWK, "Can't open input file %s, error: %s(%d)", byteStream->dev.fileIo.filename, strerror(errno), errno);
	                rc = -1;
	            }
	            break;
	        case eHDR_DEV_UART:
	            #ifdef UART_ENABLED
	        		rc = byteStream->dev.uart.port = HDR_uartOpen(byteStream->dev.uart.port, byteStream->dev.uart.rate, UART_B8S1P0, FLOW_CONTROL_DISABLED);
	            #else
	                LOG(FWK, "Tried to open unsupported device type UART.", void);
	                rc = -1;
	            #endif
	            break;
	        case eHDR_DEV_SPI:
	            #ifdef SPI_ENABLED
	                rc = HDR_spiOpen(byteStream->dev.spi.port, byteStream->dev.spi.dmaChannel, byteStream->dev.spi.mode);
	            #else
	                LOG(FWK, "Tried to open unsupported device type SPI.", void);
	                rc = -1;
	            #endif
	            break;
	        default:
	            LOG(FWK, "Byte stream tried to open unknown device type.", void);
	            rc = -1;
	            break;
	    }

		byteStream->isOpen = (rc >= 0);
	}
	else {
		rc = -1;
	}

    return rc;
}

S32 tchdr_bytestream_close(stHDR_BYTE_STREAM_t* byteStream)
{
    S32 rc = 0;
	S8 errBuf[HDR_ERR_BUF_SIZE]={0,};

	if(byteStream != NULL) {
	    switch(byteStream->deviceType){
	    	case eHDR_DEV_ETHER:
				#ifdef HDR_ETH_ENABLED
	    			rc = tchdr_ethClose(byteStream->dev.ether.port);
				#endif
				break;
	        case eHDR_DEV_MEMORY:
	             byteStream->dev.mem.bufPtr = NULL;
	             byteStream->dev.mem.size = 0;
	             byteStream->dev.mem.rp = 0;
	             byteStream->dev.mem.wp = 0;
	        	break;
	        case eHDR_DEV_FILE:
				rc = -1;
				if(byteStream->dev.fileIo.fp != NULL) {
					rc = fclose(byteStream->dev.fileIo.fp);
					if(rc < 0) {
						(void)strerror_r(rc, errBuf, sizeof(errBuf));
						(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to close stream file. %s\n", errBuf);
					}
				}
				else {
					(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "The file pointer to close is null.\n");
				}
	            break;
	        case eHDR_DEV_UART:
	            #ifdef UART_ENABLED
	                rc = HDR_uartClose(byteStream->dev.uart.port);
	            #else
	                (*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Tried to close unsupported device type UART.");
	                rc = -1;
	            #endif
	            break;
	        case eHDR_DEV_SPI:
	            #ifdef SPI_ENABLED
	               HDR_spiClose(byteStream->dev.spi.port);
	            #else
	               (*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Tried to close unsupported device type SPI.");
	               rc = -1;
	            #endif
	            break;
	        default:
	            (*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Byte stream tried to close unknown device type.");
	            rc = -1;
	            break;
	    }
	}
	else {
		rc = -1;
	}

    return rc;
}

S32 tchdr_bytestream_read(stHDR_BYTE_STREAM_t* byteStream, U8* buffer, U32 numberBytes)
{
    S32 rc = 0;
	ULONG rsize = 0;

	if((byteStream != NULL) && (buffer != NULL)) {
	    switch(byteStream->deviceType){
			case eHDR_DEV_ETHER:
				#ifdef HDR_ETH_ENABLED
	                rc = tchdr_ethRead(byteStream->dev.ether.port, buffer, numberBytes);
				#else
					LOG(FWK, "Tried to read from unsupported device type ETHER.", void);
					rc = -1;
				#endif
			break;
	        case eHDR_DEV_MEMORY:
	            if(byteStream->dev.mem.bufPtr == NULL){
	                LOG(FWK, "Buffer byte stream is not open. Not be able to read. Device[%d]\n", byteStream->deviceType);
	                rc = -1;
	            }

	            if(tchdr_bytestream_availBytes(byteStream) < numberBytes){
	                rc = -1;
	            }

				if(rc >= 0) {
		            U32 bytesRead;
		            if((byteStream->dev.mem.rp + numberBytes) > byteStream->dev.mem.size){		// we wrapped
		                (void)(*stOsal.osmemcpy)((void*)buffer, (void*)&byteStream->dev.mem.bufPtr[byteStream->dev.mem.rp],
		                       byteStream->dev.mem.size - byteStream->dev.mem.rp);
		                bytesRead = byteStream->dev.mem.size - byteStream->dev.mem.rp;
		                (void)(*stOsal.osmemcpy)((void*)&buffer[bytesRead], (void*)(byteStream->dev.mem.bufPtr), (*stArith.u32sub)(numberBytes, bytesRead));
		                byteStream->dev.mem.rp = (*stArith.u32sub)(numberBytes, bytesRead);
		            } else {
		                (void)(*stOsal.osmemcpy)((void*)buffer, (void*)&byteStream->dev.mem.bufPtr[byteStream->dev.mem.rp], numberBytes);
		                byteStream->dev.mem.rp = (*stArith.u32add)(byteStream->dev.mem.rp, numberBytes);
		            }
					rc = (S32)numberBytes;
				}
	        break;
	        case eHDR_DEV_FILE:
	            rsize = fread((void*)buffer, 1, numberBytes, byteStream->dev.fileIo.fp);
	            if(rsize < (ULONG)numberBytes){
	                if(byteStream->dev.fileIo.loop == true) {
	                    rewind(byteStream->dev.fileIo.fp);
	                } else {
	                    LOG(FWK, "Reached the end of the file.", void);
	                    rc = -1;
	                }
	            }
	            break;
	        case eHDR_DEV_UART:
	            #ifdef UART_ENABLED
	                rc = HDR_uartRead(byteStream->dev.uart.port, buffer, numberBytes, NO_TIMEOUT);
	            #else
	                LOG(FWK, "Tried to read from unsupported device type UART.", void);
	                rc = -1;
	            #endif
	            break;
	        case eHDR_DEV_SPI:
	            #ifdef SPI_ENABLED
	                rc = HDR_spiRead(byteStream->dev.spi.port, buffer, numberBytes, NO_TIMEOUT);

	            #else
	                LOG(FWK, "Tried to read from unsupported device type SPI.", void);
	                rc = -1;
	            #endif
	            break;
	        default:
	            LOG(FWK, "Byte stream tried to read from unknown device type.", void);
	            rc = -1;
				break;
	    }
	}
	else {
		rc = -1;
	}

    return rc;
}

S32 tchdr_bytestream_write(stHDR_BYTE_STREAM_t* byteStream, const U8* buffer, U32 numberBytes, S32 timeout)
{
	S32 rc = 0;

	if((byteStream != NULL) && (buffer != NULL)) {
	    switch(byteStream->deviceType){
			case eHDR_DEV_ETHER:
				#ifdef HDR_ETH_ENABLED
	                rc = tchdr_ethWrite(byteStream->dev.ether.port, buffer, (*stCast.u32tos32)(numberBytes));
				#else
					LOG(FWK, "Tried to write to unsupported device type ETHER.", void);
					rc = -1;
				#endif
			break;
	        case eHDR_DEV_MEMORY:
	        {
				U32 bytesWritten = 0;
	            if(byteStream->dev.mem.bufPtr != NULL){
		            if(tchdr_bytestream_availSpace(byteStream) >= numberBytes){
			            if((byteStream->dev.mem.wp + numberBytes) > byteStream->dev.mem.size){
			                (void)(*stOsal.osmemcpy)((void*)&byteStream->dev.mem.bufPtr[byteStream->dev.mem.wp], buffer,
			                       (*stArith.u32sub)(byteStream->dev.mem.size, byteStream->dev.mem.wp));
			                bytesWritten = (*stArith.u32sub)(byteStream->dev.mem.size, byteStream->dev.mem.wp);
			                (void)(*stOsal.osmemcpy)((void*)(byteStream->dev.mem.bufPtr), &buffer[bytesWritten], (*stArith.u32sub)(numberBytes, bytesWritten));
			                byteStream->dev.mem.wp = (*stArith.u32sub)(numberBytes, bytesWritten);
						//	LOG(FWK, "1Written Buffer!!! byteStream->dev.mem.bufPtr[%p] wp[%d] numberBytes[%d] \n", byteStream->dev.mem.bufPtr, byteStream->dev.mem.wp, numberBytes);
			            } else {
			                (void)(*stOsal.osmemcpy)((void*)&byteStream->dev.mem.bufPtr[byteStream->dev.mem.wp], buffer, numberBytes);
			                byteStream->dev.mem.wp = (*stArith.u32add)(byteStream->dev.mem.wp, numberBytes);
						//	LOG(FWK, "2Written Buffer!!! byteStream->dev.mem.bufPtr[%p] wp[%d] numberBytes[%d] \n", byteStream->dev.mem.bufPtr, byteStream->dev.mem.wp, numberBytes);
			            }
		            }
					else {
						rc = -1;
						(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Buffer is not have available buffer space.\n");
					}
	            }
				else {
					rc = -1;
	                (*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Buffer byte stream is not open. Not be able to write. Device[%d]\n", byteStream->deviceType);
				}
	        break;
        	}
	        case eHDR_DEV_FILE:
	            (void)fwrite(buffer, numberBytes, 1, byteStream->dev.fileIo.fp);
	            if(ferror(byteStream->dev.fileIo.fp) != 0){
	                rc = -1;
	            }
	            break;
	        case eHDR_DEV_UART:
	           #ifdef UART_ENABLED
	               rc = HDR_uartWrite(byteStream->dev.uart.port, buffer, numberBytes, timeout);
	           #else
	               LOG(FWK, "Tried to write to unsupported device type UART. timeout:%d", timeout);
	               rc = -1;
	           #endif
	            break;
	        case eHDR_DEV_SPI:
	            #ifdef SPI_ENABLED
	                rc = HDR_spiWrite(byteStream->dev.spi.port, buffer, numberBytes, timeout);
	            #else
	                LOG(FWK, "Tried to write unsupported device type SPI. timeout:%d", timeout);
	                rc = -1;
	            #endif
	            break;
	        default:
	            LOG(FWK, "Byte stream tried to write to unknown device type. timeout:%d", timeout);
	            rc = -1;
				UNUSED(timeout);
				break;
	    }
	}
	else {
		rc = -1;
	}

    return rc;
}

U32 tchdr_bytestream_availBytes(const stHDR_BYTE_STREAM_t* byteStream)
{
    U32 availData = 0;

    if(byteStream->deviceType == eHDR_DEV_MEMORY){
	    if(byteStream->dev.mem.wp == byteStream->dev.mem.rp){
	        availData = 0;
	    } else if(byteStream->dev.mem.wp > byteStream->dev.mem.rp){
	        availData = byteStream->dev.mem.wp - byteStream->dev.mem.rp;
	    } else {
	        availData = byteStream->dev.mem.size - byteStream->dev.mem.rp + byteStream->dev.mem.wp;
	    }
    }

    return availData;
}

U32 tchdr_bytestream_availSpace(const stHDR_BYTE_STREAM_t* byteStream)
{
    U32 availSpace = 0;

    if(byteStream->deviceType == eHDR_DEV_MEMORY){
	    if(byteStream->dev.mem.wp == byteStream->dev.mem.rp){
	        availSpace = byteStream->dev.mem.size;
	    }else if(byteStream->dev.mem.wp > byteStream->dev.mem.rp){
	        availSpace = byteStream->dev.mem.size - byteStream->dev.mem.wp + byteStream->dev.mem.rp;
	    } else {
	        availSpace = byteStream->dev.mem.rp - byteStream->dev.mem.wp;
	    }
    }

    return availSpace;
}

S32 tchdr_bytestream_reset(stHDR_BYTE_STREAM_t* byteStream)
{
    S32 rc = 0;

	if(byteStream != NULL) {
	    switch(byteStream->deviceType){
		    case eHDR_DEV_ETHER:
			    #ifdef HDR_ETH_ENABLED
				     rc = tchdr_ethReset(byteStream->dev.ether.port);
			    #endif
		    break;
	        case eHDR_DEV_MEMORY:
	             byteStream->dev.mem.rp = 0;
	             byteStream->dev.mem.wp = 0;
	        break;
	        case eHDR_DEV_FILE:
	            LOG(FWK,"Reset file stream is not handled.", void);
	            break;
	        case eHDR_DEV_UART:
	            #ifdef UART_ENABLED
	                rc = HDR_uartReset(byteStream->dev.uart.port);
	            #else
	                LOG(FWK, "Tried to reset unsupported device type UART.", void);
	                rc = -1;
	            #endif
	            break;
	        case eHDR_DEV_SPI:
	            #ifdef SPI_ENABLED
	               //rc = HDR_spiReset(byteStream->dev.spi.port);
	               LOG(FWK,"Reset spi stream is not supported.", void);
	            #else
	               LOG(FWK, "Tried to close unsupported device type SPI.", void);
	               rc = -1;
	            #endif
	            break;
			default:
				UNUSED(0);
	            break;
	    }
	}
	else {
		rc = -1;
	}

    return rc;
}
