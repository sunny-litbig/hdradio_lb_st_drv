/*******************************************************************************
*
* (C) copyright 2003-2014, iBiquity Digital Corporation, U.S.A.
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
 * file tchdr_cmdproc.h
 * brief
 */
#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "hdrCore.h"
#include "tchdr_bytestream.h"

// 2035 bytes for maximum length 2206 message
// 12 bytes for 0xA5A5 header
// 1 byte checksum
// Extra byte to terminate 2206 message with a zero
#define COMMAND_MESSAGE_BUFFER_SIZE   (2035 + 12 + 1 + 1)

typedef struct{
    stHDR_BYTE_STREAM_t byteStream;
    U8* msgBuffer;
}COMMAND_PROC_CONFIG_T;

S32 commandProcessorInit(const COMMAND_PROC_CONFIG_T* config);
void commandProcessorExec(void);

#endif //COMMAND_PROCESSOR_H
