/*******************************************************************************

*   FileName : tchdr_sis.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio station information service header

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
#ifndef TCHDR_SIS_H__
#define TCHDR_SIS_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define TC_HDR_SIS_NUM_BASIC_TYPES   			(4U)
#define TC_HDR_SIS_SHORT_NAME_MAX_LEN	        (8U)
#define TC_HDR_SIS_STATION_MESSAGE_MAX_LEN 	    (191U)
#define TC_HDR_SIS_UNIV_NAME_MAX_LEN        	(13U)
#define TC_HDR_SIS_SLOGAN_MAX_LEN				(128U)
#define TC_HDR_SIS_MAX_NUM_DATA_SERVICES		(32U)
#define TC_HDR_MAX_NUM_SERVICE_DATA_TYPES		(511U)
#define TC_HDR_SIS_CORE_VER_STR_MAX_LEN			(17U)

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eTC_HDR_SIS_NO_DATA = 0,	   /* No data for requested type is available */
	eTC_HDR_SIS_OLD_DATA,		   /* Available, but old data; no new data received since last request */
	eTC_HDR_SIS_NEW_DATA,		   /* New data available */
	eTC_HDR_SIS_ERROR			   /* Error occurred during request processing */
}eTC_HDR_SIS_STATUS_t;

typedef enum {
	eTC_ALFN_VALID, 				/* Valid data received */
	eTC_ALFN_ACQUIRING, 			/**< Still waiting for data */
	eTC_ALFN_FAILURE,				/* Failure occurred  */
	eTC_ALFN_INVALID				/* Data received but was determined to be invalid */
}eTC_HDR_SIS_ALFN_STATUS_t;

typedef enum {
	eTC_HDR_SIS_ISO_IEC_8859_1_1998 = 0,  /* 8-bit unicode char  */
	eTC_HDR_SIS_ISO_IEC_10646_1_2000 = 4  /* 16-bit unicode char (Little-endian) */
}eTC_HDR_SIS_TEXT_ENCODING_t;

typedef enum {
	eTC_HDR_SIS_DST_SCHED_NONE,
	eTC_HDR_SIS_DST_SCHED_US_CAN,
	eTC_HDR_SIS_DST_SCHED_EU
}eTC_HDR_SIS_DST_SCHEDULE_t;

typedef enum {
	eTC_HDR_SIS_DST_NOT_PRACTICED,
	eTC_HDR_SIS_DST_PRACTICED
}eTC_HDR_SIS_DST_LOCAL_t;

typedef enum {
	eTC_HDR_SIS_ACCESS_PUBLIC = 0, 	/* Public, unrestricted */
	eTC_HDR_SIS_ACCESS_RESTRICTED, 	/* Restricted */
	eTC_HDR_MAX_SIS_ACCESS_TYPE
}eTC_HDR_SIS_ACCESS_TYPE_t;

typedef enum {
	eBITMASK_SIS_STATION_ID 			= 0x00000001,
	eBITMASK_SIS_SHORT_NAME 			= 0x00000002,
	eBITMASK_SIS_LOCATION				= 0x00000004,			// not yet used.
	eBITMASK_SIS_UNIVERSAL_SHORT_NAME	= 0x00000008,
	eBITMASK_SIS_SLOGAN					= 0x00000010,
	eBITMASK_SIS_STATION_MESSAGE		= 0x00000020,			// not yet used.
	eBITMASK_SIS_STATION_TIME_ZONE		= 0x00000040,			// not yet used.
	eBITMASK_SIS_LEAP_SECONDS			= 0x00000080,			// not yet used.
	eBITMASK_SIS_EXCITER_CORE_VERSION			= 0x00000100,	// not yet used.
	eBITMASK_SIS_EXCITER_MANUFACTURER_VERSION	= 0x00000200,	// not yet used.
	eBITMASK_SIS_IMPORTER_CORE_VERSION			= 0x00000400,	// not yet used.
	eBITMASK_SIS_IMPROTER_MANUFACTURER_VERISON	= 0x00000800	// not yet used.
}eTC_HDR_SIS_BITMASK_t;				// for Demo

/***************************************************
*				Typedefs					*
****************************************************/
typedef union {
	struct {
		U8 stationId:1;
		U8 shortName:1;
		U8 location:1;
	}type;
	U8 all;
}stTC_HDR_SIS_ENABLED_BASIC_TYPES_t;

typedef struct {
	U32 value; 							/* ALFN value */
	eTC_HDR_SIS_ALFN_STATUS_t status;	/* Update status */
}stTC_HDR_SIS_ALFN_t;

typedef struct {
	union {
		struct {
			U32 country_code:10;	/* Binary representation of ISO 3166-1-alpha-2 Country Names and Code Elements */
			U32 reserved:3;		 	/* Bits not used */
			U32 fcc_facility_id:19; /* Unique Facility ID assigned by the FCC (USA only) */
		}field;
		U32 all;
	}id;
	eTC_HDR_SIS_STATUS_t status; /**< Update status */
}stTC_HDR_SIS_STATION_ID_t;

typedef struct {
	S8 text[TC_HDR_SIS_SHORT_NAME_MAX_LEN];
	U32 length;
	eTC_HDR_SIS_STATUS_t status;
}stTC_HDR_SIS_SHORT_NAME_t;

typedef struct {
	S32 latitude;
	S32 longitude;
	U32 altitude;
	eTC_HDR_SIS_STATUS_t status; /* Update status */
}stTC_HDR_SIS_STATION_LOCATION_t;

typedef struct {
	S8 pending_offset;
	S8 current_offset;
	U32 pending_offset_alfn;
	eTC_HDR_SIS_STATUS_t status;
}stTC_HDR_SIS_LEAP_SEC_t;

typedef struct {
	S8 text[TC_HDR_SIS_STATION_MESSAGE_MAX_LEN];
	U32 length;
	eTC_HDR_SIS_TEXT_ENCODING_t text_encoding;
	HDBOOL high_priority;
	eTC_HDR_SIS_STATUS_t status;
}stTC_HDR_SIS_STATION_MSG_t;

typedef struct {
	S32 utc_offset;
	eTC_HDR_SIS_DST_SCHEDULE_t dst_schedule;
	eTC_HDR_SIS_DST_LOCAL_t dst_local;
	HDBOOL dst_in_effect;
	eTC_HDR_SIS_STATUS_t status;  /* Update status */
}stTC_HDR_SIS_LOCAL_TIME_t;

typedef struct {
	S8 text[TC_HDR_SIS_UNIV_NAME_MAX_LEN];  	/* Universal name buffer */
	U32 length;									/* Universal name length */
	eTC_HDR_SIS_TEXT_ENCODING_t text_encoding;	/* Text encoding */
	HDBOOL append_fm; 						  	/* The Append Byte indicates whether "-FM"
												   should be appended to the short station */
	eTC_HDR_SIS_STATUS_t status;				/* Update status */
}stTC_HDR_SIS_UNIV_NAME_t;

typedef struct {
	S8 text[TC_HDR_SIS_SLOGAN_MAX_LEN]; 		/* Slogan buffer */
	U32 length;									/* Slogan length */
	eTC_HDR_SIS_TEXT_ENCODING_t text_encoding;	/* Encoding type */
	eTC_HDR_SIS_STATUS_t status;				/* Update status */
}stTC_HDR_SIS_STATION_SLOGAN_t;

typedef struct {
	struct {
		eTC_HDR_PROGRAM_t program_number;	/* Program number */
		eTC_HDR_SIS_STATUS_t status;		/* Update status */
	}program[eTC_HDR_PROGRAM_MAX];
	U32 program_count;						/* Total number of available programs */
}stTC_HDR_SIS_AVAIL_PROGRAMS_t;

typedef struct {
	U32 program_type;		   			/* Program type (e.g., News, Talk, Information, etc.) */
	U32 surround_sound;		  			/* Applied Sound Experience */
	eTC_HDR_SIS_ACCESS_TYPE_t access;	/* Program permissions, as assigned by the broadcaster */
	eTC_HDR_SIS_STATUS_t status;		/* Update status */
}stTC_HDR_SIS_PROGRAM_INFO_t;

typedef struct {
	struct {
		U32 type;		   				/* Data service type id */
		eTC_HDR_SIS_STATUS_t status;	/* Data service status (freshness) */
	}service[TC_HDR_SIS_MAX_NUM_DATA_SERVICES];
	U32 service_count;	   				/* Number of available data services */
}stTC_HDR_SIS_AVAIL_DATA_SERVICES_t;

typedef struct {
	struct {
		U16 service_type;	  				/* Indicates the service data type (e.g., News, Traffic, Weather, etc.) */
		U16 mime_type; 		  				/* MIME type hash value specifying the data application program type. */
		eTC_HDR_SIS_ACCESS_TYPE_t access;	/* Program permissions, as assigned by the broadcaster.
											   (0 - Public / Unrestricted; 1 - Restricted) */
		U8 status; 			  				/* Update status 1 - old; 2 - new (updated) */
	}service[TC_HDR_SIS_MAX_NUM_DATA_SERVICES];
	U32 service_count;						/* Total number of data services available */
}stTC_HDR_SIS_DATA_SERVICES_INFO_t;

typedef struct {
	S8 verstr[TC_HDR_SIS_CORE_VER_STR_MAX_LEN]; /* Text string containing Exciter Core Version */
	U32 length;							   		/* String length */
}stTC_HDR_SIS_TX_VER_STR_t;

typedef struct {
	U8 right_most_mnf_id;		/* Rightmost Exciter Manufacturer ID Char */
	U8 left_most_mnf_id;		/* Leftmost Exciter Manufacturer ID Char */
	stTC_HDR_SIS_TX_VER_STR_t version_string;
}stTC_HDR_SIS_TX_MANUF_VER_t;

typedef struct {
	union{
		struct {
			U32 countryCode:10;
			U32 reserved:3;
			U32 facilityID:19;
		}type;
		U32 all;
	}stationID;

	struct {
		S8 text[TC_HDR_SIS_SHORT_NAME_MAX_LEN];
		U32 len;
	}shortName;

	struct {
		S8 text[TC_HDR_SIS_UNIV_NAME_MAX_LEN];
		U32 len;
		eTC_HDR_SIS_TEXT_ENCODING_t charType;
		U32 appendFm;
	}universalName;

	struct {
		S8 text[TC_HDR_SIS_SLOGAN_MAX_LEN];
		U32 len;
		eTC_HDR_SIS_TEXT_ENCODING_t charType;
	}slogan;
}stTC_HDR_SIS_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern HDRET tchdr_sis_acquired(eTC_HDR_ID_t id, HDBOOL *acquired);
extern HDRET tchdr_sis_crcOk(eTC_HDR_ID_t id, HDBOOL *crc);
extern HDRET tchdr_sis_enableBasicTypes(eTC_HDR_ID_t id, stTC_HDR_SIS_ENABLED_BASIC_TYPES_t types);
extern HDRET tchdr_sis_getEnabledBasicTypes(eTC_HDR_ID_t id, stTC_HDR_SIS_ENABLED_BASIC_TYPES_t* enabled_types);
extern HDRET tchdr_sis_getBlockCount(eTC_HDR_ID_t id, U32 *count);
extern HDRET tchdr_sis_timeGpsLocked(eTC_HDR_ID_t id, HDBOOL *locked);
extern HDRET tchdr_sis_getAlfn(eTC_HDR_ID_t id, stTC_HDR_SIS_ALFN_t* alfn);
extern HDRET tchdr_sis_getStationID(eTC_HDR_ID_t id, stTC_HDR_SIS_STATION_ID_t* station_id);
extern HDRET tchdr_sis_getStationShortName(eTC_HDR_ID_t id, stTC_HDR_SIS_SHORT_NAME_t* short_name);
extern HDRET tchdr_sis_getStationLocation(eTC_HDR_ID_t id, stTC_HDR_SIS_STATION_LOCATION_t* location);
extern HDRET tchdr_sis_getLeapSec(eTC_HDR_ID_t id, stTC_HDR_SIS_LEAP_SEC_t* leap_sec);
extern HDRET tchdr_sis_getStationMessage(eTC_HDR_ID_t id, stTC_HDR_SIS_STATION_MSG_t* station_msg);
extern HDRET tchdr_sis_getLocalTime(eTC_HDR_ID_t id, stTC_HDR_SIS_LOCAL_TIME_t* local_time);
extern HDRET tchdr_sis_getUniversalName(eTC_HDR_ID_t id, stTC_HDR_SIS_UNIV_NAME_t* univ_name);
extern HDRET tchdr_sis_getAvailProgramsList(eTC_HDR_ID_t id, stTC_HDR_SIS_AVAIL_PROGRAMS_t* available_programs);
extern HDRET tchdr_sis_getStationSlogan(eTC_HDR_ID_t id, stTC_HDR_SIS_STATION_SLOGAN_t* slogan);
extern HDRET tchdr_sis_getProgramInfo(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, stTC_HDR_SIS_PROGRAM_INFO_t* program_info);
extern HDRET tchdr_sis_getAvailDataServList(eTC_HDR_ID_t id, stTC_HDR_SIS_AVAIL_DATA_SERVICES_t* available_services);
extern HDRET tchdr_sis_getAllDataServices(eTC_HDR_ID_t id,  stTC_HDR_SIS_DATA_SERVICES_INFO_t* data_services_info);
extern HDRET tchdr_sis_getDataServicesType(eTC_HDR_ID_t id, U32 service_type, stTC_HDR_SIS_DATA_SERVICES_INFO_t* data_services_info);
extern HDRET tchdr_sis_getExciterCoreVer(eTC_HDR_ID_t id, stTC_HDR_SIS_TX_VER_STR_t* version_string);
extern HDRET tchdr_sis_getExciterManufVer(eTC_HDR_ID_t id, stTC_HDR_SIS_TX_MANUF_VER_t* version_struct);
extern HDRET tchdr_sis_getImporterCoreVer(eTC_HDR_ID_t id, stTC_HDR_SIS_TX_VER_STR_t* version_string);
extern HDRET tchdr_sis_getImporterManufVer(eTC_HDR_ID_t id, stTC_HDR_SIS_TX_MANUF_VER_t* version_struct);
extern HDRET tchdr_sis_flush(eTC_HDR_ID_t id);

#ifdef __cplusplus
}
#endif

#endif
