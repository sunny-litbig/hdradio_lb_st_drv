/*******************************************************************************

*   FileName : tchdr_psd.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio program service data header

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
#ifndef TCHDR_PSD_H__
#define TCHDR_PSD_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define TC_HDR_MAX_NUM_XHDRS       		(4U)
#define TC_HDR_PSD_MAX_LEN				(256)
#define TC_HDR_MAX_NUM_UFIDS        	(4U)

/* 121 = Maximum(123)-(ParamId(1)+Len(1))*/
#define TC_HDR_MAX_LEN_XHDR_PARAM_VALUES		(121U)

/* OnlyBody Maximum is 123 > 122 = 61 x 2byte is Minimum parameter size(paramId(1)+Len(1)+Value(no-vel))) */
#define TC_HDR_MAX_NUM_XHDR_PARAM 		(61U)

#define TC_HDR_XHDR_MIME_HASH_SIZE		(4U)
#define	TC_HDR_XHDR_BODY_PARAM_ID_SIZE	(1U)
#define	TC_HDR_XHDR_BODY_LENGTH_SIZE	(1U)

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eTC_HDR_PSD_TITLE_LENGTH_CONFIG,
	eTC_HDR_PSD_ARTIST_LENGTH_CONFIG,
	eTC_HDR_PSD_ALBUM_LENGTH_CONFIG,
	eTC_HDR_PSD_GENRE_LENGTH_CONFIG,
	eTC_HDR_PSD_COMM_SHORT_CONTENT_LENGTH_CONFIG,
	eTC_HDR_PSD_COMM_ACTUAL_TEXT_LENGTH_CONFIG,
	eTC_HDR_PSD_UFID_OWNER_ID_LENGTH_CONFIG,
	eTC_HDR_PSD_COMR_PRICE_STRING_LENGTH_CONFIG,
	eTC_HDR_PSD_COMR_CONTACT_URL_LENGTH_CONFIG,
	eTC_HDR_PSD_COMR_SELLER_NAME_LENGTH_CONFIG,
	eTC_HDR_PSD_COMR_DESCRIPTION_LENGTH_CONFIG,
	eTC_HDR_PSD_XHDR_LENGTH_CONFIG,
	eTC_HDR_PSD_NUM_FIELD_CONFIG
}eTC_HDR_PSD_LENGTH_CONFIG_t;

typedef enum {
	eTC_HDR_PSD_ISO_IEC_8859_1_1998 = 0,   /**< 8-bit unicode character  */
	eTC_HDR_PSD_ISO_IEC_10646_1_2000 = 1,  /**< 16-bit unicode character (Little-endian) */
	eTC_HDR_PSD_BINARY = 0xFF			   /**< binary/no explicit type */
}eTC_HDR_PSD_CHAR_TYPE_t;

typedef enum {
	eTC_HDR_PSD_TITLE = 0,
	eTC_HDR_PSD_ARTIST,
	eTC_HDR_PSD_ALBUM,
	eTC_HDR_PSD_GENRE,
	eTC_HDR_PSD_COMMENT_LANGUAGE,
	eTC_HDR_PSD_COMMENT_SHORT_CONTENT,
	eTC_HDR_PSD_COMMENT_ACTUAL_TEXT,
	eTC_HDR_PSD_COMMERCIAL_PRICE_STRING,
	eTC_HDR_PSD_COMMERCIAL_VALID_UNTIL,
	eTC_HDR_PSD_COMMERCIAL_CONTACT_URL,
	eTC_HDR_PSD_COMMERCIAL_RECEIVED_AS,
	eTC_HDR_PSD_COMMERCIAL_SELLER_NAME,
	eTC_HDR_PSD_COMMERCIAL_DESCRIPTION,
	eTC_HDR_PSD_XHDR,
	eTC_HDR_PSD_MAX
}eTC_HDR_PSD_FIELD_t;			// for Demo

typedef enum {
	eTC_HDR_PSD_COMM_LANGUAGE,
	eTC_HDR_PSD_COMM_SHORT_CONTENT,
	eTC_HDR_PSD_COMM_ACTUAL_TEXT,
	eTC_HDR_PSD_NUM_COMM_SUBFIELDS
}eTC_HDR_PSD_COMM_SUBFIELD_t;

typedef enum {
	eTC_HDR_PSD_UFID_OWNER_ID,
	eTC_HDR_PSD_UFID_FILE_ID,
	eTC_HDR_PSD_NUM_UFID_SUBFIELDS
}eTC_HDR_PSD_UFID_SUBFIELD_t;

typedef enum {
	eTC_HDR_PSD_COMR_PRICE_STRING,
	eTC_HDR_PSD_COMR_VALID_UNTIL,
	eTC_HDR_PSD_COMR_CONTACT_URL,
	eTC_HDR_PSD_COMR_RECEIVED_AS,
	eTC_HDR_PSD_COMR_SELLER_NAME,
	eTC_HDR_PSD_COMR_DESCRIPTION,
	eTC_HDR_PSD_NUM_COMR_SUBFIELDS
}eTC_HDR_PSD_COMR_SUBFIELD_t;

typedef enum {
	eBITMASK_PSD_TITLE		= 0x01,
	eBITMASK_PSD_ARTIST		= 0x02,
	eBITMASK_PSD_ALBUM		= 0x04,
	eBITMASK_PSD_GENRE		= 0x08,
	eBITMASK_PSD_COMMENT	= 0x10,
	eBITMASK_PSD_COMMERCIAL	= 0x40,
	eBITMASK_PSD_XHDR		= 0X80
}eTC_HDR_PSD_BITMASK_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef union {
	struct{
		U8 title:1;	        /* Song title */
		U8 artist:1;	    /* Artist name */
		U8 album:1;	        /* Album name*/
		U8 genre:1;	        /* Genre */
		U8 comment:1;	    /* General comments */
		U8 UFID:1;	        /* Unique File Identifier */
		U8 commercial:1;	/* For advertising purposes */
		U8 XHDR:1;	        /* Image synchronization trigger */
	}field;
	U8 all;	                /* Used to read/write all fields at once */
}stTC_HDR_PSD_FIELDS_t;

typedef struct {
   S8 data[TC_HDR_PSD_MAX_LEN];
   U32 len;
   eTC_HDR_PSD_CHAR_TYPE_t charType;
}stTC_HDR_PSD_FORM_t;

typedef struct {
	/* Spec Value */
	U8 param_id;
	U8 length;
	U8 value[TC_HDR_MAX_LEN_XHDR_PARAM_VALUES];

	/* User Value */
	U16 lot_id;			// When param_id is 0, lot_id is valid.
}stTC_HDR_PSD_XHDR_PARAM_t;

typedef struct {
	/* Spec Value */
	U32 mime_hash;
	stTC_HDR_PSD_XHDR_PARAM_t params[TC_HDR_MAX_NUM_XHDR_PARAM];	// The XHDR's body is an array of parameter.

	/* User Value */
	U32 numParams;		// A number of valid params[] array.
	U8 program;
}stTC_HDR_PSD_XHDR_FRAME_t;

typedef struct {
	stTC_HDR_PSD_FORM_t title;
	stTC_HDR_PSD_FORM_t artist;
	stTC_HDR_PSD_FORM_t album;
	stTC_HDR_PSD_FORM_t genre;

	struct {
		stTC_HDR_PSD_FORM_t language;
		stTC_HDR_PSD_FORM_t shortContent;
		stTC_HDR_PSD_FORM_t actualText;
	}comment;

	struct {
		stTC_HDR_PSD_FORM_t priceString;
		stTC_HDR_PSD_FORM_t validUntil;
		stTC_HDR_PSD_FORM_t contactURL;
		stTC_HDR_PSD_FORM_t receivedAs;
		stTC_HDR_PSD_FORM_t sellerName;
		stTC_HDR_PSD_FORM_t description;
	}commercial;

	stTC_HDR_PSD_XHDR_FRAME_t xhdr;

}stTC_HDR_PSD_t;		// for Demo

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern HDRET tchdr_psd_getChangedPrograms(eTC_HDR_ID_t id, stTC_HDR_PROG_BITMAP_t* bitmap);
extern HDRET tchdr_psd_clearChangedProgram(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number);
extern HDRET tchdr_psd_enableFields(eTC_HDR_ID_t id, stTC_HDR_PSD_FIELDS_t enabled_fields);
extern HDRET tchdr_psd_getEnabledFields(eTC_HDR_ID_t id, stTC_HDR_PSD_FIELDS_t *fields);
extern HDRET tchdr_psd_setMaxLength(eTC_HDR_ID_t id, eTC_HDR_PSD_LENGTH_CONFIG_t config, U32 length);
extern HDRET tchdr_psd_resetMaxLength(eTC_HDR_ID_t id, eTC_HDR_PSD_LENGTH_CONFIG_t config);
extern HDRET tchdr_psd_getMaxLength(eTC_HDR_ID_t id, eTC_HDR_PSD_LENGTH_CONFIG_t config, U32 *max_length);
extern HDRET tchdr_psd_getTitle(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, stTC_HDR_PSD_FORM_t* title);
extern HDRET tchdr_psd_getArtist(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, stTC_HDR_PSD_FORM_t* artist);
extern HDRET tchdr_psd_getAlbum(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, stTC_HDR_PSD_FORM_t* album);
extern HDRET tchdr_psd_getGenre(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, stTC_HDR_PSD_FORM_t* genre);
extern HDRET tchdr_psd_getComment(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, eTC_HDR_PSD_COMM_SUBFIELD_t subfield, stTC_HDR_PSD_FORM_t* comment);
extern HDRET tchdr_psd_getUfid(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, U32 ufid_num, eTC_HDR_PSD_UFID_SUBFIELD_t subfield, stTC_HDR_PSD_FORM_t* ufid);
extern HDRET tchdr_psd_getCommercial(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, eTC_HDR_PSD_COMR_SUBFIELD_t subfield, stTC_HDR_PSD_FORM_t* commercial);
extern HDRET tchdr_psd_getXhdr(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, U32 xhdr_num, stTC_HDR_PSD_FORM_t* xhdr);

#ifdef __cplusplus
}
#endif

#endif

