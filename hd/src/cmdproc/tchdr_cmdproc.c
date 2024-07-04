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
 * file commandProcessor.c
 * brief CDM Command Processor Message Handler
 */
#include <string.h>
#include "tchdr_common.h"			// To avoid redundant condition warnings of codesonar
#include "tchdr_bytestream.h"
#include "tchdr_cmdproc.h"
#include "tchdr_cmdcallbacks.h"
#include "tchdr_log.h"
#include "tchdr_cmdopcode.h"
#include "tchdr_cmdsispsd.h"
#include "tchdr_cmdaas.h"
#include "tchdr_cmdsys.h"
#include "tchdr_cmdiboc.h"

#define CMD_WRITE_TIMEOUT_MS      (100)

#define MAX_MESSAGE_LENGTH        (2048U)
#define NUM_PROTOCOL_BYTES        (13U) // Packet header plus checksum (0 data)

#define MAX_PAYLOAD_SIZE          (MAX_MESSAGE_LENGTH - NUM_PROTOCOL_BYTES)

#define NUM_CMD_CONTROL_BYTES     (NUM_PROTOCOL_BYTES - 1U) // All protocol bytes excluding checksum
#define DATA_START_OFFSET         (NUM_PROTOCOL_BYTES - 1U)

#define DATA_OUT_BUFFER_SIZE      (4096 + 100)

// Group of messages sent from the host that can have more than one data segment
#define LONG_HOST_MSG_GROUP             case 0x84:

// For DTS internal use
#ifndef INTERNAL_COMMAND_PROC
    #define INTERNAL_COMMAND_PROC CMD_UNSUPPORTED_OPCODE;
#endif

typedef union {
    struct {
       U8 overallSuccess:1;
       U8 lmCountMismatch:1;
       U8 badHeader:1;
       U8 checksumFailure:1;
       U8 queueOverflow:1;
       U8 b5b6b7:3;
    }field;
    U8 all;
}CMD_RESP_STATUS_T;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

static COMMAND_PROC_CONFIG_T commandProcConfig;
static U8* rxMsgBuffer;
static U8* headerAsmBuffer;

static U8 dataOutputBuffer[DATA_OUT_BUFFER_SIZE];
static U32 dataOutLength = 0;
static U32 dataBytesSent = 0;

static U32 expectedMessageCount = 0;
static U32 expectedSegmentNumber = 0;
static CMD_opcode_t prevOpcode = (CMD_opcode_t)0;

static CMD_dispatch_rc_t dispatchMessage(HDR_instance_t* hdrInstance, CMD_opcode_t opCode, U8* dataIn, U32 inLength,
                                         U8* dataOut, U32* outLength);
static S32 waitForHostCommand(void);
static void sendErrorResponse(CMD_RESP_STATUS_T replyStatus);
static U8 computeChecksum(const U8* data, U32 length);

S32 commandProcessorInit(const COMMAND_PROC_CONFIG_T* config)
{
	S32 rc = 0;
    if((config == NULL) || (config->byteStream.isOpen == false)){
        rc = -1;
    }
	else{
	    (void)(*stOsal.osmemcpy)(&commandProcConfig, config, (U32)sizeof(COMMAND_PROC_CONFIG_T));

	    headerAsmBuffer = &commandProcConfig.msgBuffer[0];
	    rxMsgBuffer = &commandProcConfig.msgBuffer[12];
	}

    return rc;
}

#define CMD_HEADER_START_OFFSET         (0)
#define CMD_MESSAGE_COUNT_OFFSET        (2)
#define CMD_HEADER_INSTANCE_OFFSET      (3)
#define CMD_HEADER_MSG_LENGTH_OFFSET    (4)
#define CMD_HEADER_OPCODE_OFFSET        (6)
#define CMD_HEADER_STATUS_OFFSET        (7)
#define CMD_HEADER_NUM_SEGS_OFFSET      (8)
#define CMD_HEADER_SEG_NUM_OFFSET       (10)

//  2206 Message Header
//----------------------+------------+------------+
//   Field              |     Size   |
//----------------------+------------+------------+
// Start Pattern        |    2 Bytes |  0xA5A5    |
// Message Count        |    1 Byte  |  0 - 255   |
// Instance Number      |    1 Bytes |  0 - 2     |
// Message Length       |    2 Bytes |  0 - 2048  |
// Opcode               |    1 Byte  |  0 - 0xFF  |
// Status               |    1 Byte  |
// Number of Segments   |    2 Bytes |
// Segment Number       |    2 Bytes |
// Data                 |    N Bytes |
// Checksum             |    1 Byte  |
void commandProcessorExec(void)
{
	static S32 retry=0;
	S32 ret = waitForHostCommand();
	if(ret != 0){
		if(ret == -2) {		// failed to get TCP/IP socket
			if(retry > 10) {
				(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "Failed to run CDM thread!!!\n");
				stCmdProcThdAttr.thread_running = 0;
			}
			else {
				(void)sleep(1);
			}
			retry++;
		}
		else {
			retry = 0;
		}
		return;
	}

    U8* dataIn;
    U32 dataInLength;

    CMD_RESP_STATUS_T replyStatus;
    replyStatus.all = 1; // Start with overall reception status set to success

    U32 messageCount  = rxMsgBuffer[CMD_MESSAGE_COUNT_OFFSET];
    U32 instanceNum   = rxMsgBuffer[CMD_HEADER_INSTANCE_OFFSET];
    U32 messageLength = rxMsgBuffer[CMD_HEADER_MSG_LENGTH_OFFSET] |
                                 ((U32)rxMsgBuffer[CMD_HEADER_MSG_LENGTH_OFFSET + 1] << 8);
    U32 opCode        = rxMsgBuffer[CMD_HEADER_OPCODE_OFFSET];
    HDBOOL cmdAbort   = (HDBOOL)rxMsgBuffer[CMD_HEADER_STATUS_OFFSET];

    U32 numberOfSegments = rxMsgBuffer[CMD_HEADER_NUM_SEGS_OFFSET] |
                                    ((U32)rxMsgBuffer[CMD_HEADER_NUM_SEGS_OFFSET + 1] << 8);

    U32 segmentNumber = rxMsgBuffer[CMD_HEADER_SEG_NUM_OFFSET] |
                                 ((U32)rxMsgBuffer[CMD_HEADER_SEG_NUM_OFFSET + 1] << 8);

    U8 recvChecksum = *(rxMsgBuffer + messageLength - 1);
    U8 calcChecksum = computeChecksum(rxMsgBuffer, (*stArith.u32sub)(messageLength, 1U));

    if(recvChecksum != calcChecksum){
        replyStatus.all = 0;
        replyStatus.field.checksumFailure = 1;
        sendErrorResponse(replyStatus);
        return;
    }

    HDR_instance_t* hdrInstance = CMD_cb_bbp_get_hdr_instance(instanceNum);

    if(hdrInstance == NULL){
        replyStatus.field.b5b6b7 = 2;
        sendErrorResponse(replyStatus);
        return;
    }

    if(cmdAbort == true ){
        expectedSegmentNumber = 0;
        sendErrorResponse(replyStatus);
        return;
    }

    // Determine if the message from the host is of type that can have more than one data segment
    HDBOOL longHostMsg;
    switch(opCode){
        LONG_HOST_MSG_GROUP
            longHostMsg = true;
            break;
        default:
            longHostMsg = false;
            break;
    }

    // If the request is for a next segment(segmentNumber != 0) for large BBP response,
    // it's not an error.
    if((longHostMsg == false) && (numberOfSegments > (U32)1) && (segmentNumber == (U32)0)){
        // This opCode does not support long message from the host
        replyStatus.field.b5b6b7 = 5; // unexpected number of data segments
        sendErrorResponse(replyStatus);
        return;
    }

    if(prevOpcode != (CMD_opcode_t)opCode){
        expectedSegmentNumber = 0;
        prevOpcode = (CMD_opcode_t)opCode;
    }

    if(segmentNumber != expectedSegmentNumber){
        replyStatus.field.b5b6b7 = 1;
        sendErrorResponse(replyStatus);
        return;
    }

    if(messageCount != expectedMessageCount){
        replyStatus.field.lmCountMismatch = 1;
    }

    expectedMessageCount = ((messageCount + (U32)1) & (U32)0x00FF);

#ifndef AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
    if(CMD_cb_bbp_busy(hdrInstance) == true){
        // The bbp instance is busy and can't process the command
        replyStatus.field.b5b6b7 = 4;
        sendErrorResponse(replyStatus);
        expectedSegmentNumber = 0;
        return;
    }
#endif

    if(segmentNumber == (U32)0){
        dataIn = rxMsgBuffer + DATA_START_OFFSET;
        dataInLength = (*stArith.u32sub)(messageLength, (U32)NUM_PROTOCOL_BYTES);	// dataInLength = messageLength - (U32)NUM_PROTOCOL_BYTES;

        CMD_dispatch_rc_t rc = dispatchMessage(hdrInstance, (CMD_opcode_t)opCode, dataIn, dataInLength, dataOutputBuffer, &dataOutLength);

        switch(rc){
            case CMD_DISPATCH_OK:
                break;
            case CMD_UNSPECIFIED_ERROR:
                return;
				break;
            case CMD_UNSUPPORTED_OPCODE:
                replyStatus.field.b5b6b7 = 7;
                sendErrorResponse(replyStatus);
                return;
				break;
            case CMD_UNSUPPORTED_INSTANCE:
                return;
				break;
            default:
                return;
				break;
        }

        if(dataOutLength > (U32)MAX_PAYLOAD_SIZE ){
            numberOfSegments = (dataOutLength / MAX_PAYLOAD_SIZE) + 1U;
        } else {
            numberOfSegments = 1;
        }

        dataBytesSent = 0;
    }

    U32 numDataOutBytes = MIN(dataOutLength, MAX_PAYLOAD_SIZE);
    (void)(*stOsal.osmemcpy)((void*)(rxMsgBuffer + DATA_START_OFFSET), (void*)&dataOutputBuffer[dataBytesSent], numDataOutBytes);
    dataOutLength -= numDataOutBytes;
    dataBytesSent = (*stArith.u32add)(dataBytesSent, numDataOutBytes);

    messageLength = numDataOutBytes + (U32)NUM_PROTOCOL_BYTES;

    // Fill in the rest of the bytes
    rxMsgBuffer[CMD_HEADER_MSG_LENGTH_OFFSET] = (U8)(messageLength & 0xffU);
    rxMsgBuffer[CMD_HEADER_MSG_LENGTH_OFFSET + 1] = (U8)((messageLength >> 8U) & 0xffU);

    rxMsgBuffer[CMD_HEADER_STATUS_OFFSET] = replyStatus.all;
    rxMsgBuffer[CMD_HEADER_NUM_SEGS_OFFSET] = (U8)(numberOfSegments & 0xffU);
    rxMsgBuffer[CMD_HEADER_NUM_SEGS_OFFSET + 1] = (U8)((numberOfSegments >> 8U) & 0xffU);

    if((numberOfSegments > (U32)1) && (expectedSegmentNumber < (numberOfSegments - (U32)1))){
        // Next host message must specify the correct segment number
        expectedSegmentNumber = segmentNumber + (U32)1;
    } else{
        expectedSegmentNumber = 0;
    }

    rxMsgBuffer[messageLength - (U32)1] = computeChecksum(rxMsgBuffer, messageLength - (U32)1);
    rxMsgBuffer[messageLength] = 0; // Write zero at the end of the message as specified by 2206
	messageLength++;

    // Send the response back to the host
    (void)tchdr_bytestream_write(&commandProcConfig.byteStream, rxMsgBuffer, messageLength, CMD_WRITE_TIMEOUT_MS);
}

static CMD_dispatch_rc_t dispatchMessage(HDR_instance_t* hdrInstance, CMD_opcode_t opCode, U8* dataIn,
                                         U32 inLength, U8* dataOut, U32* outLength)
{
    CMD_dispatch_rc_t rc;

    switch((U8)opCode){
        SYS_CONFIG_GROUP
            rc = SYS_procHostCommand(hdrInstance, opCode, dataIn, inLength, dataOut, outLength);
            break;
        IBOC_MSG_CROUP
            rc = IBOC_procHostCommand(hdrInstance, opCode, dataIn, inLength, dataOut, outLength);
            break;
        SISPSD_MSG_GROUP
            rc = SIS_procHostCommand(hdrInstance, opCode, dataIn, dataOut, outLength);
            break;
        AAS_MSG_GROUP
            rc = AAS_procHostCommand(hdrInstance, opCode, dataIn, inLength, dataOut, DATA_OUT_BUFFER_SIZE, outLength);
            break;
        default:
            rc = CMD_UNSUPPORTED_OPCODE;
			break;
    }

    if(rc == CMD_UNSUPPORTED_OPCODE){
        rc = INTERNAL_COMMAND_PROC;
    }

    return rc;
}


static S32 waitForHostCommand(void)
{
    static U32 rxByteCount = 0;
    U32 bytesToRead;

	if(NUM_CMD_CONTROL_BYTES < rxByteCount) {
		return -1;
	}

	bytesToRead = NUM_CMD_CONTROL_BYTES - rxByteCount;

    // Read the first 12 bytes to look for the start pattern 0xA5A5 and message length
    S32 ret = tchdr_bytestream_read(&commandProcConfig.byteStream, headerAsmBuffer, bytesToRead);
    if(ret < 0) {
        return ret;
    }

    U32 i;
    for(i = 0; i < bytesToRead; ++i){
        switch(rxByteCount){
            case 0:
                if(headerAsmBuffer[i] == (U8)0xA5){
                    rxMsgBuffer[rxByteCount] = headerAsmBuffer[i];
					rxByteCount++;
                }
                break;
            case 1:
                if(headerAsmBuffer[i] == (U8)0xA5){
                    rxMsgBuffer[rxByteCount] = headerAsmBuffer[i];
					rxByteCount++;
                } else {
                    rxByteCount = 0;
                }
                break;
            default:
                rxMsgBuffer[rxByteCount] = headerAsmBuffer[i];
				rxByteCount++;
                break;
        }
    }

    if(rxByteCount < NUM_CMD_CONTROL_BYTES) {
        // Still waiting for header + control bytes
        return -1;
    }

    rxByteCount = 0;

    // Get the length of the remainder bytes to read
    U32 messageLength = (U32)rxMsgBuffer[CMD_HEADER_MSG_LENGTH_OFFSET] | ((U32)rxMsgBuffer[CMD_HEADER_MSG_LENGTH_OFFSET + 1] << 8);

    if((messageLength > (U32)MAX_MESSAGE_LENGTH) || (messageLength < (U32)NUM_PROTOCOL_BYTES)){
        LOG(CMD, "Invalid LM length received.", void);
        return -1;
    }

    // Get the rest of the message into the buffer
    if(tchdr_bytestream_read(&commandProcConfig.byteStream, &rxMsgBuffer[NUM_CMD_CONTROL_BYTES], messageLength - NUM_CMD_CONTROL_BYTES) < 0){
        return -1;
    }

    return 0;
}

static void sendErrorResponse(CMD_RESP_STATUS_T replyStatus)
{
    HDBOOL cmdAbort = (HDBOOL)rxMsgBuffer[CMD_HEADER_STATUS_OFFSET];

    U32 numberOfSegments = 0;
    if(cmdAbort == true){
        numberOfSegments = 1;
    }

    U32 messageLength = NUM_PROTOCOL_BYTES;

    rxMsgBuffer[CMD_HEADER_MSG_LENGTH_OFFSET] = (U8)(messageLength & 0xffU);
    rxMsgBuffer[CMD_HEADER_MSG_LENGTH_OFFSET + 1] = 0;
    rxMsgBuffer[CMD_HEADER_STATUS_OFFSET] = replyStatus.all;
    rxMsgBuffer[CMD_HEADER_NUM_SEGS_OFFSET] = (U8)(numberOfSegments & 0xffU);
    rxMsgBuffer[CMD_HEADER_NUM_SEGS_OFFSET + 1] = 0;

    rxMsgBuffer[NUM_PROTOCOL_BYTES - 1U] = computeChecksum(rxMsgBuffer, NUM_PROTOCOL_BYTES - 1U);
    rxMsgBuffer[messageLength] = 0; // Write zero at th
	messageLength++;

    // Send the error response to the host
    (void)tchdr_bytestream_write(&commandProcConfig.byteStream, rxMsgBuffer, messageLength, CMD_WRITE_TIMEOUT_MS);
}

static U8 computeChecksum(const U8* data, U32 length)
{
    const U8* ptr = data;

    U32 i;
    U8 sum;

	if(ptr == NULL) {
		(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "The data is null.\n");
		return 0;
	}

	if(length > (U32)MAX_MESSAGE_LENGTH) {
		(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "The length is invalid.\n");
		return 0;
	}

    sum = 0;
    for (i = 0; i < length; ++i){
        sum += *ptr; // let the sum overflow since we only need one byte
        ptr++;
    }

    return sum;
}
