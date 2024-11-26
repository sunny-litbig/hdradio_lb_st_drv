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
 * file cmdAas.c
 * brief
 */
#include <string.h>
#include "tchdr_common.h"			// For HDCDM_DBG
#include "tchdr_log.h"
#include "tchdr_cmdproc.h"
#include "tchdr_cmdaas.h"
#include "tchdr_cmdcallbacks.h"
#include "hdrAas.h"
#include "hdrSig.h"

// Table K-2: AAS_Enable_Ports Command Functions  OpCode 0x41 - AAS_Enable_Ports
#define ENABLE_PORT							(0x05)
#define DISABLE_PORT						(0x07)

// Table K-2: AAS_Enable_Ports Command Functions  OpCode 0x42 - AAS_Get_Enabled_Ports
#define GET_ENABLED_PORTS					(0x05)
#define PORT_MODE_NON_ORDERED				(0x00)
#define PORT_MODE_ORDERED					(0x01)

//Table K-4: AAS_Get_Data Command Functions  OpCode 0x49 - AAS_Get_Data
#define GET_PORT_DATA						(0x05)
#define GET_SPECIFIC_PORT					(0xFE)

//Table K-6: AAS_Flush_Queue Command Functions  OpCode 0x4A - AAS_Flush_Queue
#define FLUSH_PORT_DATA						(0x05)
#define FLUSH_SPECIFIC					    (0xFE)

// Table K-9 AAS_Proc_LOT Command Functions OpCode 0x4C
#define GET_LOT_INFO                        (0x01)
#define ENABLE_LOT_REASSEMBLY               (0x02)
#define DISABLE_LOT_REASSEMBLY              (0x03)
#define GET_OBJECT_LIST                     (0x04)
#define GET_OBJECT_LIST_BY_NAME             (0x05)
#define GET_HEADER                          (0x06)
#define GET_BODY                            (0x07)
#define FLUSH_OBJECT                        (0x08)
#define GET_LOT_TIMEOUT                     (0x09)
#define GET_LOT_OPEN_PORTS                  (0x0A)

// Table L-1: Sig_Get_Data Command Functions OpCode 0x4D
#define SIG_GET_ALL_SERVICES_LIST           (0x00)
#define SIG_GET_AUDIO_SERVICES_LIST         (0x01)
#define SIG_GET_DATA_SERVICE_LIST           (0x02)
#define SIG_GET_SPECIFIC_SERVICE_INFO       (0x03)
#define SIG_GET_ALL_AUDIO_SERVICES_INFO     (0x04)
#define SIG_GET_ALL_DATA_SERVICES_INFO      (0x05)
#define SIG_GET_ALL_SERVICES_INFO           (0x06)
#define SIG_FLUSH_ALL_SERVICES_INFO         (0x07)

// Table K-7: AAS_Cntrl_Cnfg Command Functions OpCode 0x4E
#define SET_AAS_CNFG                        (0x01)
#define GET_AAS_CNFG                        (0x02)

CMD_dispatch_rc_t AAS_procHostCommand(HDR_instance_t* hdrInstance, CMD_opcode_t opCode, U8* dataIn, U32 inLength,
                                      U8* dataOut, U32 dataOutBufferSize, U32* outLength)
{
    if(dataIn == NULL) {
		(*pfnHdrLog)(eTAG_CDM, eLOG_ERR, "AAS_procHostCommand input data is null.\n");
		return CMD_UNSPECIFIED_ERROR;
    }

    U8 funcCode = dataIn[0]; //BYTE 0 specifies the function code
    *outLength = 0;

    switch(opCode){
        case AAS_ENABLE_PORTS:
            switch (funcCode){
                case ENABLE_PORT:
                {
                    HDR_aas_port_list_t portList;
                    U32 i;
                    const U8* ptr;

					LOG(CMD,32768U, "Received AAS_ENABLE_PORTS->AAS_ENABLE_PORTS");
					dataOut[0] = ENABLE_PORT;
					ptr = &dataIn[1];
					portList.num_ports = *ptr;
					ptr++;

                    for(i = 0; i < portList.num_ports; i++){
                        portList.port[i].number = *ptr;
						ptr++;
                        portList.port[i].number |= (U16)*ptr << 8;
						ptr++;
                        portList.port[i].mode = *ptr;
						ptr++;
                    }

                    (void)HDR_aas_enable_ports(hdrInstance, &portList);
                    *outLength = 1;
                    break;
                }
                case DISABLE_PORT:
                {
                    HDR_aas_port_list_t portList;
                    U32 i;
                    const U8* ptr;

					LOG(CMD,32768U, "Received AAS_ENABLE_PORTS->AAS_DISABLE_PORST");
					dataOut[0] = DISABLE_PORT;
					ptr = &dataIn[1];
					portList.num_ports = *ptr;
					ptr++;

                    for(i = 0; i < portList.num_ports; i++){
                        portList.port[i].number = *ptr;
						ptr++;
                        portList.port[i].number |= (U16)*ptr << 8;
						ptr++;
                    }

                    (void)HDR_aas_disable_ports(hdrInstance, &portList);

                    *outLength = 1;
                    break;
                }
                default:
                    LOG(CMD,32768U, "AAS_ENABLE_PORTS: function code 0x%x not recognised", funcCode);
                    return CMD_UNSUPPORTED_OPCODE;
					break;
            }
            break;
        case AAS_GET_ENABLED_PORTS:
            switch (funcCode){
                case GET_ENABLED_PORTS:
            	{
                    U32 i;
                    U8* ptr;
                    HDR_aas_port_list_t portList = {0,};

                    LOG(CMD,32768U, "Received AAS_GET_ENABLED_PORTS->AAS_GET_ENABLED_PORTS");
                    dataOut[0] = GET_ENABLED_PORTS;
                    ptr = &dataOut[1];

                    (void)HDR_aas_get_enabled_ports(hdrInstance, &portList);

                    *ptr = (U8)portList.num_ports;
                    ptr++;

                    for(i = 0; i < portList.num_ports; i++){
                        *ptr = (U8)(portList.port[i].number & 0xffU);
                        ptr++;
                        *ptr = (U8)((portList.port[i].number >> 8U) & 0xffU);
                        ptr++;
                        *ptr = portList.port[i].mode;
                        ptr++;
                    }

                    *outLength = 2U + ((U32)portList.num_ports * 3U);
                    break;
            	}
                 default:
                    LOG(CMD,1U, "AAS_GET_ENABLED_PORTS: function code 0x%x not recognised", funcCode);
                    return CMD_UNSUPPORTED_OPCODE;
					break;
            }
            break;
        case AAS_GET_DATA:
            #define AAS_PACKET_HEADER_SIZE    (10) // 2206 specified AAS packet header size

            switch (funcCode){
                case GET_PORT_DATA:
                {
                    U32 offset = 0;
                    U32 bytesWritten;
                    HDR_aas_packet_info_t packet_info;

                    LOG(CMD,65536U, "Received AAS_GET_DATA->GET_PORT_DATA");
                    dataOut[offset] = GET_PORT_DATA;
                    offset++;

                    if((inLength > (U32)1) && (dataIn[1] == (U8)GET_SPECIFIC_PORT)){
                        U32 portNumber = dataIn[2];  // LSB
                        portNumber |= ((U32)dataIn[3] << 8); // MSB
                        if(HDR_aas_get_port_data(hdrInstance, portNumber, &packet_info, &dataOut[AAS_PACKET_HEADER_SIZE], (*stArith.u32sub)(dataOutBufferSize, (U32)AAS_PACKET_HEADER_SIZE), &bytesWritten) < 0){
                            // There is no data available. Reset the output length; that's what the CDM expects
                            *outLength = 0;
                            break;
                        }
                    } else {
                        if(HDR_aas_get_next_port_data(hdrInstance, &packet_info, &dataOut[AAS_PACKET_HEADER_SIZE], (*stArith.u32sub)(dataOutBufferSize, (U32)AAS_PACKET_HEADER_SIZE), &bytesWritten) < 0){
                            // There is no data available. Reset the output length; that's what the CDM expects
                            *outLength = 0;
                            break;
                        } else {
						}
                    }

                    // Populate packet information
                    dataOut[offset] = (U8)(packet_info.num_packets_avail & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)((packet_info.num_packets_avail >> 8U) & 0xffU);
                    offset++;
                    dataOut[offset] = (*stCast.booltou8)(packet_info.overflow_status);
                    offset++;
                    dataOut[offset] = (U8)((packet_info.port_number >> 8U) & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)(packet_info.port_number & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)(packet_info.sequence_number & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)((packet_info.sequence_number >> 8U) & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)(packet_info.packet_length & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)((packet_info.packet_length >> 8U) & 0xffU);
                    offset++;

                    *outLength = (*stArith.u32add)(offset, bytesWritten);

                    break;
                }
                default:
                    LOG(CMD,65536U, "AAS_GET_DATA: function code 0x%x not recognised", funcCode);
                    return CMD_UNSUPPORTED_OPCODE;
					break;
            }
            break;
        case AAS_FLUSH_QUEUE:
            switch (funcCode){
                case FLUSH_PORT_DATA:
                    LOG(CMD,65536U, "Received AAS_FLUSH_QUEUE->AAS_FLUSH_PORT_DATA");
                    dataOut[0] = FLUSH_PORT_DATA;
                    if(dataIn[1] == (U8)FLUSH_SPECIFIC){
                        U32 portNumber = dataIn[2];  // LSB
                        portNumber |= ((U32)dataIn[3] << 8U); // MSB
                        (void)HDR_aas_flush_port(hdrInstance, portNumber);
                    } else {
                        (void)HDR_aas_flush_all_ports(hdrInstance);
                    }
                    *outLength = 1;
                    break;
                default:
                    LOG(CMD,1U, "AAS_FLUSH_QUEUE: function code 0x%x not recognised", funcCode);
                    return CMD_UNSUPPORTED_OPCODE;
					break;
            }
            break;
        case AAS_PROC_LOT:
            switch(funcCode){
                case GET_LOT_INFO:
                {
                    U32 lotPoolSize = 0;

					LOG(CMD,1U, "Received AAS_PROC_LOT->GET_LOT_INFO");

                    dataOut[0] = GET_LOT_INFO;

                    if(HDR_aas_get_lot_pool_size(hdrInstance, &lotPoolSize) < 0){
                        *outLength = 1;
                        break;
                    }

                    lotPoolSize /= (U32)1024; //convert to kilobytes

                    dataOut[1] = (U8)(lotPoolSize & 0xffU);
                    dataOut[2] = (U8)((lotPoolSize >> 8U) & 0xffU);
                    dataOut[3] = (U8)HDR_AAS_MAX_NUM_LOT_PORTS;
                    dataOut[4] = (U8)HDR_MAX_NUM_LOT_OBJECTS;
                    *outLength = 5;
                    break;
                }
                case ENABLE_LOT_REASSEMBLY:
                {
                    U32 serviceNumber = 0;
					U32 portNumber = 0;

					LOG(CMD,1U, "Received AAS_PROC_LOT->ENABLE_LOT_REASSEMBLY");

                    dataOut[0] = ENABLE_LOT_REASSEMBLY;
                    serviceNumber = dataIn[1] | ((U32)dataIn[2] << 8);
                    portNumber = dataIn[3] | ((U32)dataIn[4] << 8);

                    *outLength = 1; // Error

                    if(HDR_aas_enable_lot_reassembly(hdrInstance, serviceNumber, portNumber) == 0){
                        dataOut[1] = 0x00; // Success
                        *outLength = 2;
                    }
                    break;
                }
                case DISABLE_LOT_REASSEMBLY:
                {
                    U32 serviceNumber = 0;
					U32 portNumber = 0;

					LOG(CMD,1U, "Received AAS_PROC_LOT->DISABLE_LOT_REASSEMBLY");
                    dataOut[0] = DISABLE_LOT_REASSEMBLY;

                    serviceNumber = dataIn[1] | ((U32)dataIn[2] << 8);
                    portNumber = dataIn[3] | ((U32)dataIn[4] << 8);

                    if(HDR_aas_disable_lot_reassembly(hdrInstance, serviceNumber, portNumber) < 0){
                        dataOut[1] = 0xFF; // Error
                    } else {
                        dataOut[1] = 0x00; // Success
                    }
                    *outLength = 2;
                    break;
                }
                case GET_OBJECT_LIST:
                {
                    U32 serviceNumber = 0;
					U32 offset = 0;
					U32 i;

                    HDR_aas_lot_object_list_t objectList;
                    HDR_aas_lot_object_header_t objectHeader;

					LOG(CMD,1U, "Received AAS_PROC_LOT->GET_OBJECT_LIST");
                    serviceNumber = dataIn[1] | ((U32)dataIn[2] << 8);

                    dataOut[offset] = GET_OBJECT_LIST;
                    offset++;

                    if(HDR_aas_get_lot_object_list(hdrInstance, serviceNumber, &objectList) < 0){
                       *outLength = 1;
                        break;
                    }

                    dataOut[offset] = (*stCast.booltou8)(HDR_aas_lot_overflow(hdrInstance));
                    offset++;

                    for(i = 0; i < objectList.num_objects; ++i){
                        if(HDR_aas_get_lot_object_header(hdrInstance, objectList.item[i].port_number, objectList.item[i].lot_id, &objectHeader) < 0){
                            continue;
                        }

                        dataOut[offset] = objectList.item[i].complete;
                        offset++;
                        dataOut[offset] = (U8)(objectList.item[i].port_number & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectList.item[i].port_number >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(objectList.item[i].lot_id & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectList.item[i].lot_id >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(objectHeader.file_size & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectHeader.file_size >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectHeader.file_size >> 16U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectHeader.file_size >> 24U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(objectHeader.mime_hash & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectHeader.mime_hash >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectHeader.mime_hash >> 16U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectHeader.mime_hash >> 24U) & 0xffU);
                        offset++;
                    }

                    *outLength = offset;
                    break;
                }
                case GET_OBJECT_LIST_BY_NAME:
                {

                    U32 serviceNumber = 0;
					U32 offset = 0;
					U32 i;
                    const S8* filename;

                    HDR_aas_lot_object_list_t objectList;
                    HDR_aas_lot_object_header_t objectHeader;

					LOG(CMD,1U, "Received AAS_PROC_LOT->GET_OBJECT_LIST_BY_NAME");
                    serviceNumber = dataIn[1] | ((U32)dataIn[2] << 8);

                    dataIn[inLength] = (U8)'\0'; // Null-terminate the filename in case host didn't

                    filename = (S8*)&dataIn[3];

                    dataOut[offset] = GET_OBJECT_LIST_BY_NAME;
                    offset++;

                    if(HDR_aas_lot_get_object_list_by_name(hdrInstance, serviceNumber, filename, &objectList) < 0){
                        *outLength = 1;
                        break;
                    }

                    dataOut[offset] = (*stCast.booltou8)(HDR_aas_lot_overflow(hdrInstance));
                    offset++;

                    for(i = 0; i < objectList.num_objects; ++i){
                        if(HDR_aas_get_lot_object_header(hdrInstance, objectList.item[i].port_number, objectList.item[i].lot_id, &objectHeader) < 0){
                            continue;
                        }
                        dataOut[offset] = objectList.item[i].complete;
                        offset++;
                        dataOut[offset] = (U8)(objectList.item[i].port_number & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectList.item[i].port_number >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(objectList.item[i].lot_id & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectList.item[i].lot_id >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(objectHeader.file_size & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectHeader.file_size >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectHeader.file_size >> 16U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectHeader.file_size >> 24U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(objectHeader.mime_hash & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectHeader.mime_hash >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectHeader.mime_hash >> 16U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((objectHeader.mime_hash >> 24U) & 0xffU);
                        offset++;
                    }

                    *outLength = offset;
                    break;
                }
                case GET_HEADER:
                {
                    U32 offset = 0;
                    U32 lotId = 0, portNumber = 0;
                    HDR_aas_lot_object_header_t objectHeader;

                    LOG(CMD,1U, "Received AAS_PROC_LOT->GET_HEADER");
                    dataOut[offset] = GET_HEADER;
                    offset++;

                    portNumber = dataIn[1] | ((U32)dataIn[2] << 8);
                    lotId = dataIn[3] | ((U32)dataIn[4] << 8);

                    S32 rc = HDR_aas_get_lot_object_header(hdrInstance, portNumber, lotId, &objectHeader);

                    if(rc == -1){
                        // Error processing command
                        *outLength = 1;
                        break;
                    }

                    if(rc < 0){
                        // Port number of LOT ID do not refer to an object
                        dataOut[offset] = 0x02;
                        *outLength = 2;
                        break;
                    }

                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&objectHeader.discard_time, (U32)sizeof(U32));
                    offset += (U32)sizeof(U32);
                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&objectHeader.file_size, (U32)sizeof(U32));
                    offset += (U32)sizeof(U32);
                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&objectHeader.mime_hash, (U32)sizeof(U32));
                    offset += (U32)sizeof(U32);

                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)objectHeader.filename, objectHeader.filename_length);
                    offset += objectHeader.filename_length;

                    *outLength = offset;
                    break;
                }
                case GET_BODY:
                {
                    U32 lotId = 0;
                    U32 portNumber = 0;
                    U32 hostBufferSize = 0;
                    U32 bytesWritten = 0;
                    S32 rc = 0;

                    LOG(CMD,1U, "Received AAS_PROC_LOT->GET_BODY");

                    dataOut[0] = GET_BODY;

                    portNumber = dataIn[1] | ((U32)dataIn[2] << 8);
                    lotId = dataIn[3] | ((U32)dataIn[4] << 8);
                    hostBufferSize = dataIn[5] | ((U32)dataIn[6] << 8); // Includes 2 protocol bytes

                    if(hostBufferSize < (U32)2){
                        // Not enough buffer space for even the protocol bytes
                        *outLength = 1;
                        break;
                    }

                    rc = HDR_aas_get_lot_object_body(hdrInstance, portNumber, lotId, &dataOut[2], hostBufferSize - (U32)2, &bytesWritten);

                    if(rc < 0){ //Error
                        dataOut[1] = 0x02;
                        *outLength = 2;
                    } else {
                        dataOut[1] = (U8)rc;
                        *outLength = (*stArith.u32add)(2U, bytesWritten);
                    }
                    break;
                }
                case FLUSH_OBJECT:
                {
                    U32 lotId = 0;
					U32 portNumber = 0;

                    LOG(CMD,1U, "Received AAS_PROC_LOT->FLUSH_OBJECT");

                    portNumber = dataIn[1] | ((U32)dataIn[2] << 8);
                    lotId = dataIn[3] | ((U32)dataIn[4] << 8);

                    dataOut[0] = FLUSH_OBJECT;
                    *outLength = 1;
                    (void)HDR_aas_flush_lot_object(hdrInstance, portNumber, lotId);
                    break;
                }
#ifdef USE_HDRLIB_3RD_CHG_VER
                case GET_LOT_TIMEOUT:
                {
                    LOG(CMD,1U, "Received AAS_PROC_LOT->GET_LOT_TIMEOUT");
                    uint_t lotTimeout;
                    dataOut[0] = GET_LOT_TIMEOUT;

                    if(HDR_aas_get_lot_timeout(hdrInstance, &lotTimeout) < 0){
                        *outLength = 1;
                        break;
                    }
                    dataOut[1] = (uint8_t)lotTimeout;
                    dataOut[2] = (uint8_t)(lotTimeout >> 8);
                    *outLength = 3;
                    break;
                }
                case GET_LOT_OPEN_PORTS:
                {
                    LOG(CMD,1U, "Received AAS_PROC_LOT->GET_LOT_OPEN_PORTS");
                    dataOut[0] = GET_LOT_OPEN_PORTS;
                    uint_t i;
                    uint8_t* ptr = &dataOut[1];

                    HDR_aas_lot_port_list_t portList;
                    if(HDR_aas_get_lot_ports(hdrInstance, &portList) < 0){
                        *ptr = 0;
                        *outLength = 2;
                        break;
                    }
                    *ptr++ = (uint8_t)portList.num_ports;

                    for(i = 0; i < portList.num_ports; i++){
                        *ptr++ = (uint8_t)portList.port[i].number;
                        *ptr++ = (uint8_t)(portList.port[i].number >> 8);
                        *ptr++ = (uint8_t)portList.port[i].service;
                        *ptr++ = (uint8_t)(portList.port[i].service >> 8);
                    }

                    *outLength = 2U + portList.num_ports * 4U;
                    break;
                }
#endif	// #ifdef USE_HDRLIB_3RD_CHG_VER
                default:
                    LOG(CMD,1U, "AAS_PROC_LOT: function code 0x%x not recognised", funcCode);
                    return CMD_UNSUPPORTED_OPCODE;
					break;
            }
            break;
        case SIG_GET_DATA:
             switch (funcCode){
                case SIG_GET_ALL_SERVICES_LIST:
                {
                    U32 offset = 0;
                    U8 serviceType;	// HDR_sig_service_type_t
                    HDR_sig_service_list_t serviceList;

                    LOG(CMD,1U, "Received SIG_GET_DATA->SIG_GET_ALL_SERVICES_LIST");

                    dataOut[offset] = SIG_GET_ALL_SERVICES_LIST;
                    offset++;

                    for(serviceType = 0U; serviceType < (U8)HDR_SIG_NUM_SERVICE_TYPES; ++serviceType){
                        if(HDR_sig_get_service_list(hdrInstance, (HDR_sig_service_type_t)serviceType, &serviceList) < 0){
                            offset = 0;
                            break;
                        }

                        dataOut[offset] = (U8)(serviceList.num_services & 0xffU);
                        offset++;

                        U32 i;
                        for(i = 0; i < serviceList.num_services; ++i){
                            dataOut[offset] = serviceType;
                            offset++;
                            dataOut[offset] = (U8)(serviceList.item[i].service_number & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)((serviceList.item[i].service_number >> 8U) & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)serviceList.item[i].status;
                            offset++;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceList.item[i].receive_time, (U32)sizeof(serviceList.item[i].receive_time));
                            offset+=(U32)sizeof(serviceList.item[i].receive_time);
                        }
                    }

                    *outLength = offset;
                    break;
                }
                case SIG_GET_AUDIO_SERVICES_LIST:
                {
                    U32 offset = 0;
                    HDR_sig_service_list_t audioServiceList;

                    LOG(CMD,1U, "Received SIG_GET_DATA->SIG_GET_ALL_SERVICES_LIST");

                    dataOut[offset] = SIG_GET_AUDIO_SERVICES_LIST;
                    offset++;

                    if(HDR_sig_get_service_list(hdrInstance, HDR_SIG_AUDIO_SERVICE_TYPE, &audioServiceList) < 0){
                         *outLength = 0;
                         break;
                    }

                    dataOut[offset] = (U8)(audioServiceList.num_services & 0xffU);
                    offset++;

                    U32 i;
                    for(i = 0; i < audioServiceList.num_services; ++i){
                        dataOut[offset] = (U8)HDR_SIG_AUDIO_SERVICE_TYPE;
                        offset++;
                        dataOut[offset] = (U8)(audioServiceList.item[i].service_number & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((audioServiceList.item[i].service_number >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)audioServiceList.item[i].status;
                        offset++;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&audioServiceList.item[i].receive_time, (U32)sizeof(audioServiceList.item[i].receive_time));
                        offset+=(U32)sizeof(audioServiceList.item[i].receive_time);
                    }

                    *outLength = offset;
                    break;
                }
                case SIG_GET_DATA_SERVICE_LIST:
                {
                    U32 offset = 0;
                    HDR_sig_service_list_t dataServiceList;

                    LOG(CMD,1U, "Received SIG_GET_DATA->SIG_GET_ALL_SERVICES_LIST");

                    dataOut[offset] = SIG_GET_DATA_SERVICE_LIST;
                    offset++;

                    if(HDR_sig_get_service_list(hdrInstance, HDR_SIG_DATA_SERVICE_TYPE, &dataServiceList) < 0){
                        *outLength = 0;
                        break;
                    }

                    dataOut[offset] = (U8)(dataServiceList.num_services & 0xffU);
                    offset++;

                    U32 i;
                    for(i = 0; i < dataServiceList.num_services; ++i){
                        dataOut[offset] = (U8)HDR_SIG_DATA_SERVICE_TYPE;
                        offset++;
                        dataOut[offset] = (U8)(dataServiceList.item[i].service_number & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((dataServiceList.item[i].service_number >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)dataServiceList.item[i].status;
                        offset++;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&dataServiceList.item[i].receive_time, (U32)sizeof(dataServiceList.item[i].receive_time));
                        offset+=(U32)sizeof(dataServiceList.item[i].receive_time);
                    }

                    *outLength = offset;
                    break;
                }
                case SIG_GET_SPECIFIC_SERVICE_INFO:
                {
                    U32 i;
                    U32 offset = 0;
                    U32 serviceNumber = 0;

                    HDR_sig_service_info_t serviceInfo;
                    HDR_sig_service_component_t serviceComponent;

                    LOG(CMD,1U, "Received SIG_GET_DATA->SIG_GET_SPECIFIC_SERVICE_INFO");

                    serviceNumber = dataIn[1];
                    serviceNumber |= (U32)dataIn[2] << 8;

                    dataOut[offset] = SIG_GET_SPECIFIC_SERVICE_INFO;
                    offset++;

                    if(HDR_sig_get_service_info(hdrInstance, serviceNumber, &serviceInfo) < 0){
                       *outLength = 1;
                        break;
                    }

                    dataOut[offset] = (U8)(serviceInfo.service_number & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)((serviceInfo.service_number >> 8U) & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)(serviceInfo.priority & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)(serviceInfo.sequence_number & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)((serviceInfo.sequence_number >> 8U) & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)serviceInfo.status;
                    offset++;
                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceInfo.receive_time, (U32)sizeof(serviceInfo.receive_time));
                    offset += (U32)sizeof(serviceInfo.receive_time);
                    dataOut[offset] = (U8)(serviceInfo.provider_text_encoding & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)(serviceInfo.provider_name_length & 0xffU);
                    offset++;
                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)serviceInfo.provider_name, serviceInfo.provider_name_length);
                    offset = (*stArith.u32add)(offset, serviceInfo.provider_name_length);
                    dataOut[offset] = (U8)(serviceInfo.display_text_encoding & 0xffU);
                    offset++;
                    dataOut[offset] = (U8)(serviceInfo.display_name_length & 0xffU);
                    offset++;
                    (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)serviceInfo.display_name, serviceInfo.display_name_length);
                    offset = (*stArith.u32add)(offset, serviceInfo.display_name_length);
                    dataOut[offset] = (U8)(serviceInfo.num_components & 0xffU);
                    offset++;

                    for(i = 0; i < serviceInfo.num_components; ++i){
                        if(HDR_sig_get_service_component(hdrInstance, serviceInfo.service_number, i, &serviceComponent) < 0){
                            break;
                        }

                        dataOut[offset] = (U8)serviceComponent.component_type;
                        offset++;
                        dataOut[offset] = (U8)(serviceComponent.channel & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((serviceComponent.channel >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(serviceComponent.content_type & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((serviceComponent.content_type >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = 0; // reserved
                        offset++;
                        dataOut[offset] = (U8)(serviceComponent.processing & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(serviceComponent.priority & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(serviceComponent.access_rights & 0xffU);
                        offset++;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceComponent.mime_hash_value, 4);
                        offset += (U32)4;
                        dataOut[offset] = (*stArith.u8add)(8U, (U8)(serviceComponent.expanded_id_length  & 0xffU));
                        offset++;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceComponent.provider_id, 4);
                        offset += (U32)4;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceComponent.service_id, 4);
                        offset += (U32)4;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)serviceComponent.expanded_service_id, serviceComponent.expanded_id_length);
                        offset = (*stArith.u32add)(offset, serviceComponent.expanded_id_length);
                    }

                    *outLength = offset;
                    break;
                }
                case SIG_GET_ALL_AUDIO_SERVICES_INFO:
                {
                    U32 s, c;
                    U32 offset = 0;

                    HDR_sig_service_list_t audioServiceList;
                    HDR_sig_service_info_t serviceInfo;
                    HDR_sig_service_component_t serviceComponent;

                    LOG(CMD,1U, "Received SIG_GET_DATA->SIG_GET_ALL_AUDIO_SERVICES_INFO");

                    if(HDR_sig_get_service_list(hdrInstance, HDR_SIG_AUDIO_SERVICE_TYPE, &audioServiceList) < 0){
                        *outLength = 0;
                        break;
                    }

                    dataOut[offset] = SIG_GET_ALL_AUDIO_SERVICES_INFO;
                    offset++;
                    dataOut[offset] = (U8)(audioServiceList.num_services & 0xffU);
                    offset++;

                    for(s = 0; s < audioServiceList.num_services; ++s){
                        if(HDR_sig_get_service_info(hdrInstance, audioServiceList.item[s].service_number, &serviceInfo) < 0){
                           offset = 0;
                           break;
                        }

                        dataOut[offset] = (U8)(serviceInfo.service_number & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((serviceInfo.service_number >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(serviceInfo.priority & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(serviceInfo.sequence_number & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((serviceInfo.sequence_number >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)serviceInfo.status;
                        offset++;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceInfo.receive_time, (U32)sizeof(serviceInfo.receive_time));
                        offset += (U32)sizeof(serviceInfo.receive_time);
                        dataOut[offset] = (U8)(serviceInfo.provider_text_encoding & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(serviceInfo.provider_name_length & 0xffU);
                        offset++;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)serviceInfo.provider_name, serviceInfo.provider_name_length);
                        offset = (*stArith.u32add)(offset, serviceInfo.provider_name_length);
                        dataOut[offset] = (U8)(serviceInfo.display_text_encoding & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(serviceInfo.display_name_length & 0xffU);
                        offset++;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)serviceInfo.display_name, serviceInfo.display_name_length);
                        offset = (*stArith.u32add)(offset, serviceInfo.display_name_length);
                        dataOut[offset] = (U8)(serviceInfo.num_components & 0xffU);
                        offset++;

                        for(c = 0; c < serviceInfo.num_components; ++c){
                            if(HDR_sig_get_service_component(hdrInstance, serviceInfo.service_number, c, &serviceComponent) < 0){
                                break;
                            }

                            dataOut[offset] = (U8)serviceComponent.component_type;
                            offset++;
                            dataOut[offset] = (U8)(serviceComponent.channel & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)((serviceComponent.channel >> 8U) & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)(serviceComponent.content_type & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)((serviceComponent.content_type >> 8U) & 0xffU);
                            offset++;
                            dataOut[offset] = 0; // reserved
                            offset++;
                            dataOut[offset] = (U8)(serviceComponent.processing & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)(serviceComponent.priority & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)(serviceComponent.access_rights & 0xffU);
                            offset++;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceComponent.mime_hash_value, 4);
                            offset += (U32)4;
							dataOut[offset] = (*stArith.u8add)(8U, (U8)(serviceComponent.expanded_id_length  & 0xffU));
                            offset++;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceComponent.provider_id, 4);
                            offset += (U32)4;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceComponent.service_id, 4);
                            offset += (U32)4;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)serviceComponent.expanded_service_id, serviceComponent.expanded_id_length);
                            offset = (*stArith.u32add)(offset, serviceComponent.expanded_id_length);
                        }
                    }
                    *outLength = offset;
                    break;
                }
                case SIG_GET_ALL_DATA_SERVICES_INFO:
                {
                    U32 s, c;
                    U32 offset = 0;

                    HDR_sig_service_list_t dataServicesList;
                    HDR_sig_service_info_t serviceInfo;
                    HDR_sig_service_component_t serviceComponent;

                    LOG(CMD,1U, "Received SIG_GET_DATA->SIG_GET_ALL_DATA_SERVICES_INFO");

                    if(HDR_sig_get_service_list(hdrInstance, HDR_SIG_DATA_SERVICE_TYPE, &dataServicesList) < 0){
                        *outLength = 0;
                        break;
                    }

                    dataOut[offset] = SIG_GET_ALL_DATA_SERVICES_INFO;
                    offset++;
                    dataOut[offset] = (U8)(dataServicesList.num_services & 0xffU);
                    offset++;

                    for(s = 0; s < dataServicesList.num_services; ++s){
                        if(HDR_sig_get_service_info(hdrInstance, dataServicesList.item[s].service_number, &serviceInfo) < 0){
                           offset = 0;
                           break;
                        }

                        dataOut[offset] = (U8)(serviceInfo.service_number & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((serviceInfo.service_number >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(serviceInfo.priority & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(serviceInfo.sequence_number & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)((serviceInfo.sequence_number >> 8U) & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)serviceInfo.status;
                        offset++;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceInfo.receive_time, (U32)sizeof(serviceInfo.receive_time));
                        offset += (U32)sizeof(serviceInfo.receive_time);
                        dataOut[offset] = (U8)(serviceInfo.provider_text_encoding & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(serviceInfo.provider_name_length & 0xffU);
                        offset++;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)serviceInfo.provider_name, serviceInfo.provider_name_length);
                        offset = (*stArith.u32add)(offset, serviceInfo.provider_name_length);
                        dataOut[offset] = (U8)(serviceInfo.display_text_encoding & 0xffU);
                        offset++;
                        dataOut[offset] = (U8)(serviceInfo.display_name_length & 0xffU);
                        offset++;
                        (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)serviceInfo.display_name, serviceInfo.display_name_length);
                        offset = (*stArith.u32add)(offset, serviceInfo.display_name_length);
                        dataOut[offset] = (U8)(serviceInfo.num_components & 0xffU);
                        offset++;

                        for(c = 0; c < serviceInfo.num_components; ++c){
                            if(HDR_sig_get_service_component(hdrInstance, serviceInfo.service_number, c, &serviceComponent) < 0){
                                break;
                            }

                            dataOut[offset] = (U8)serviceComponent.component_type;
                            offset++;
                            dataOut[offset] = (U8)(serviceComponent.channel & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)((serviceComponent.channel >> 8U) & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)(serviceComponent.content_type & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)((serviceComponent.content_type >> 8U) & 0xffU);
                            offset++;
                            dataOut[offset] = 0; // reserved
                            offset++;
                            dataOut[offset] = (U8)(serviceComponent.processing & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)(serviceComponent.priority & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)(serviceComponent.access_rights & 0xffU);
                            offset++;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceComponent.mime_hash_value, 4);
                            offset += (U32)4;
                            dataOut[offset] = (*stArith.u8add)(8U, (U8)(serviceComponent.expanded_id_length & (U32)0x0ff));
                            offset++;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceComponent.provider_id, 4);
                            offset += (U32)4;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceComponent.service_id, 4);
                            offset += (U32)4;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)serviceComponent.expanded_service_id, serviceComponent.expanded_id_length);
                            offset = (*stArith.u32add)(offset, serviceComponent.expanded_id_length);
                       }
                    }

                    *outLength = offset;
                    break;
                }
                case SIG_GET_ALL_SERVICES_INFO:
                {
                    U32 s, c;
                    U32 offset = 0;

                    LOG(CMD,1U, "Received SIG_GET_DATA->SIG_GET_ALL_AUDIO_SERVICES_INFO");

                    dataOut[offset] = SIG_GET_ALL_SERVICES_INFO;
                    offset++;

                    U8 serviceType;	// HDR_sig_service_type_t
                    HDR_sig_service_list_t serviceList;
                    HDR_sig_service_info_t serviceInfo;
                    HDR_sig_service_component_t serviceComponent;

                    for(serviceType = 0U; serviceType < (U8)HDR_SIG_NUM_SERVICE_TYPES; ++serviceType){
                        if(HDR_sig_get_service_list(hdrInstance, (HDR_sig_service_type_t)serviceType, &serviceList) < 0){
                            *outLength = 0;
                            break;
                        }

                        dataOut[offset] = (U8)(serviceList.num_services & 0xffU);
                        offset++;

                        for(s = 0; s < serviceList.num_services; ++s){
                            if(HDR_sig_get_service_info(hdrInstance, serviceList.item[s].service_number, &serviceInfo) < 0){
                               offset = 0;
                               break;
                            }

                            dataOut[offset] = (U8)(serviceInfo.service_number & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)((serviceInfo.service_number >> 8U) & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)(serviceInfo.priority & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)(serviceInfo.sequence_number & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)((serviceInfo.sequence_number >> 8U) & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)serviceInfo.status;
                            offset++;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceInfo.receive_time, (U32)sizeof(serviceInfo.receive_time));
                            offset += (U32)sizeof(serviceInfo.receive_time);
                            dataOut[offset] = (U8)(serviceInfo.provider_text_encoding & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)(serviceInfo.provider_name_length & 0xffU);
                            offset++;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)serviceInfo.provider_name, serviceInfo.provider_name_length);
                            offset = (*stArith.u32add)(offset, serviceInfo.provider_name_length);
                            dataOut[offset] = (U8)(serviceInfo.display_text_encoding & 0xffU);
                            offset++;
                            dataOut[offset] = (U8)(serviceInfo.display_name_length & 0xffU);
                            offset++;
                            (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)serviceInfo.display_name, serviceInfo.display_name_length);
                            offset = (*stArith.u32add)(offset, serviceInfo.display_name_length);
                            dataOut[offset] = (U8)(serviceInfo.num_components & 0xffU);
                            offset++;

                            for(c = 0; c < serviceInfo.num_components; ++c){
                                if(HDR_sig_get_service_component(hdrInstance, serviceInfo.service_number, c, &serviceComponent) < 0){
                                    break;
                                }

                                dataOut[offset] = (U8)serviceComponent.component_type;
                                offset++;
                                dataOut[offset] = (U8)(serviceComponent.channel & 0xffU);
                                offset++;
                                dataOut[offset] = (U8)((serviceComponent.channel >> 8U) & 0xffU);
                                offset++;
                                dataOut[offset] = (U8)(serviceComponent.content_type & 0xffU);
                                offset++;
                                dataOut[offset] = (U8)((serviceComponent.content_type >> 8U) & 0xffU);
                                offset++;
                                dataOut[offset] = 0; // reserved
                                offset++;
                                dataOut[offset] = (U8)(serviceComponent.processing & 0xffU);
                                offset++;
                                dataOut[offset] = (U8)(serviceComponent.priority & 0xffU);
                                offset++;
                                dataOut[offset] = (U8)(serviceComponent.access_rights & 0xffU);
                                offset++;
                                (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceComponent.mime_hash_value, 4);
                                offset += (U32)4;
								dataOut[offset] = (*stArith.u8add)(8U, (U8)(serviceComponent.expanded_id_length  & 0xffU));
                                offset++;
                                (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceComponent.provider_id, 4);
                                offset += (U32)4;
                                (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)&serviceComponent.service_id, 4);
                                offset += (U32)4;
                                (void)(*stOsal.osmemcpy)((void*)&dataOut[offset], (void*)serviceComponent.expanded_service_id, serviceComponent.expanded_id_length);
                                offset = (*stArith.u32add)(offset, serviceComponent.expanded_id_length);
                            }
                        }
                    }

                    *outLength = offset;
                    break;
                }
            case SIG_FLUSH_ALL_SERVICES_INFO:
                LOG(CMD,1U, "Received SIG_GET_DATA->SIG_FLUSH_ALL_SERVICES_INFO");
                dataOut[0] = SIG_FLUSH_ALL_SERVICES_INFO;
                (void)HDR_sig_flush_all(hdrInstance);
                *outLength = 1;
                break;
            default:
                LOG(CMD,1U, "SIG_GET_DATA: function code 0x%x not recognised", funcCode);
                return CMD_UNSUPPORTED_OPCODE;
				break;
            }
            break;
        default:
            LOG(CMD,1U, "Received unsupported opCode (0x%X) from host.", opCode);
            return CMD_UNSUPPORTED_OPCODE;
			break;
    }

    return CMD_DISPATCH_OK;
}
