/*******************************************************************************

*   FileName : tchdr_aas.h

*   Copyright (c) Telechips Inc.

*   Description : Advanced Application Service API header

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
#ifndef TCHDR_AAS_H__
#define TCHDR_AAS_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define TC_HDR_AAS_MAX_NUMBER_OF_PORTS			(255U)
#define TC_HDR_AAS_MAX_PACKET_SIZE				(4096U)
#define TC_HDR_AAS_MAX_NUM_LOT_PORTS   			(64U)
#define TC_HDR_MAX_NUM_LOT_OBJECTS     			(200U)
#define TC_HDR_AAS_LOT_MAX_FILENAME_LENGTH		(231U)

#define TC_HDR_AAS_LOT_MIME_HASH_NONE			(0x806FFF30U)
#define TC_HDR_AAS_LOT_MIME_HASH_TEXT_PLAIN		(0xBB492AACU)
#define TC_HDR_AAS_LOT_MIME_HASH_TEXT_ENRICHED	(0x7074b716U)
#define TC_HDR_AAS_LOT_MIME_HASH_IMAGE_GIF		(0x6E1D9F04U)
#define TC_HDR_AAS_LOT_MIME_HASH_IMAGE_PNG		(0x4F328CA0U)
#define TC_HDR_AAS_LOT_MIME_HASH_IMAGE_JPEG		(0x1E653E9CU)
#define TC_HDR_AAS_LOT_MIME_HASH_AUDIO_BASIC	(0x06362BAEU)
#define TC_HDR_AAS_LOT_MIME_HASH_VIDEO_MPEG		(0x761FB167U)

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eTC_HDR_AAS_PORT_NON_ORDERED = 0x00,  /* Do not order the packets(First-in-first-out) */
	eTC_HDR_AAS_PORT_ORDERED = 0x04 	  /* Order the packets based on the sequence number */
}eTC_HDR_AAS_PORT_MODE_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct {
	struct{
		U16 number;
		U8 mode;
	}port[TC_HDR_AAS_MAX_NUMBER_OF_PORTS];
	U8 num_ports;
}stTC_HDR_AAS_PORT_LIST_t;

typedef struct {
	U32 num_packets_avail;
	HDBOOL overflow_status;
	U32 port_number;
	U32 sequence_number;
	U32 packet_length;
	U32 num_bytes_unread;
}stTC_HDR_AAS_PACKET_INFO_t;

typedef struct {
	struct {
		U16 port_number;
		U16 lot_id;
		U8  complete;
	}item[TC_HDR_MAX_NUM_LOT_OBJECTS];
	U32 num_objects;
}stTC_HDR_AAS_LOT_OBJECT_LIST_t;

typedef struct {
	U32 discard_time;
	U32 file_size;
	U32 mime_hash;
	U8 filename[TC_HDR_AAS_LOT_MAX_FILENAME_LENGTH];
	U8 filename_length;
}stTC_HDR_AAS_LOT_OBJECT_HEADER_t;

typedef struct {
	U32 service_number;
	U32 app_mime_hash;
	stTC_HDR_AAS_LOT_OBJECT_HEADER_t header;
	U8* body;
	U32 body_bytes_written;
}stTC_HDR_LOT_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern HDRET tchdr_aas_enablePorts(eTC_HDR_ID_t id, const stTC_HDR_AAS_PORT_LIST_t* port_list);
extern HDRET tchdr_aas_disablePorts(eTC_HDR_ID_t id, const stTC_HDR_AAS_PORT_LIST_t* port_list);
extern HDRET tchdr_aas_disableAllPorts(eTC_HDR_ID_t id);
extern HDRET tchdr_aas_getEnabledPorts(eTC_HDR_ID_t id, stTC_HDR_AAS_PORT_LIST_t* port_list);
extern HDRET tchdr_aas_getNextPortData(eTC_HDR_ID_t id, stTC_HDR_AAS_PACKET_INFO_t* packet_info, U8* packet_buffer, U32 buffer_size, U32 * bytes_written);
extern HDRET tchdr_aas_getPortData(eTC_HDR_ID_t id, U32 port_number, stTC_HDR_AAS_PACKET_INFO_t* packet_info, U8* packet_buffer, U32 buffer_size, U32 * bytes_written);
extern HDRET tchdr_aas_flushPort(eTC_HDR_ID_t id, U32 port_number);
extern HDRET tchdr_aas_flushAllPorts(eTC_HDR_ID_t id);
extern HDRET tchdr_aas_getLotPoolSize(eTC_HDR_ID_t id, U32 * mem_pool_size);
extern HDRET tchdr_aas_getLotSpaceLeft(eTC_HDR_ID_t id, U32 *mem_left);
extern HDRET tchdr_aas_lotOverflow(eTC_HDR_ID_t id, HDBOOL *overFlow);
extern HDRET tchdr_aas_enableLotReassembly(eTC_HDR_ID_t id, U32 service_number, U32 port_number);
extern HDRET tchdr_aas_disableLotReassembly(eTC_HDR_ID_t id, U32 service_number, U32 port_number);
extern HDRET tchdr_aas_getLotObjectList(eTC_HDR_ID_t id, U32 service_number, stTC_HDR_AAS_LOT_OBJECT_LIST_t* object_list);
extern HDRET tchdr_aas_getLotObjectListByName(eTC_HDR_ID_t id, U32 service_number, const S8* filename, stTC_HDR_AAS_LOT_OBJECT_LIST_t* object_list);
extern HDRET tchdr_aas_getLotObjectHeader(eTC_HDR_ID_t id, U32 port_number, U32 object_lot_id, stTC_HDR_AAS_LOT_OBJECT_HEADER_t* object_header);
extern HDRET tchdr_aas_getLotObjectBody(eTC_HDR_ID_t id, U32 port_number, U32 object_lot_id, U8* buffer, U32 buffer_size, U32 * bytes_written);
extern HDRET tchdr_aas_flushLotObject(eTC_HDR_ID_t id, U32 port_number, U32 object_lot_id);

#ifdef __cplusplus
}
#endif

#endif
