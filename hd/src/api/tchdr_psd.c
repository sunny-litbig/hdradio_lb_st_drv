/*******************************************************************************

*   FileName : tchdr_psd.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio program service data API functions and definitions

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
#include "tchdr_common.h"

#include "hdrPsd.h"

#include "tchdr_api.h"
#include "tchdr_framework.h"
#include "tchdr_psd.h"

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
HDRET tchdr_psd_getChangedPrograms(eTC_HDR_ID_t id, stTC_HDR_PROG_BITMAP_t* bitmap)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			if(bitmap != NULL) {
				bitmap->all = HDR_psd_get_changed_programs(hdrInstance).all;
				ret = (HDRET)eTC_HDR_RET_OK;
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_clearChangedProgram(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_psd_clear_changed_program(hdrInstance, (HDR_program_t)program_number);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_enableFields(eTC_HDR_ID_t id, stTC_HDR_PSD_FIELDS_t enabled_fields)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_psd_fields_t fields;
			fields.all = enabled_fields.all;
			ret = HDR_psd_enable_fields(hdrInstance, fields);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_getEnabledFields(eTC_HDR_ID_t id, stTC_HDR_PSD_FIELDS_t *fields)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			if(fields != NULL) {
				fields->all = HDR_psd_get_enabled_fields(hdrInstance).all;
				ret = (HDRET)eTC_HDR_RET_OK;
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_setMaxLength(eTC_HDR_ID_t id, eTC_HDR_PSD_LENGTH_CONFIG_t config, U32 length)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_psd_set_max_length(hdrInstance, (HDR_psd_length_config_t)config, length);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_PSD_INVALID_LENGTH;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_resetMaxLength(eTC_HDR_ID_t id, eTC_HDR_PSD_LENGTH_CONFIG_t config)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_psd_reset_max_length(hdrInstance, (HDR_psd_length_config_t)config);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_getMaxLength(eTC_HDR_ID_t id, eTC_HDR_PSD_LENGTH_CONFIG_t config, U32 *max_length)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			if(max_length != NULL) {
				*max_length = HDR_psd_get_max_length(hdrInstance, (HDR_psd_length_config_t)config);
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	return ret;
}

HDRET tchdr_psd_getTitle(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, stTC_HDR_PSD_FORM_t* title)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_psd_data_t getTitle;
			(void)(*stOsal.osmemset)((void*)&getTitle, (S8)0, (U32)sizeof(HDR_psd_data_t));
			ret = HDR_psd_get_title(hdrInstance, (HDR_program_t)program_number, &getTitle);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)title, (void*)&getTitle, (U32)sizeof(stTC_HDR_PSD_FORM_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_getArtist(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, stTC_HDR_PSD_FORM_t* artist)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_psd_data_t getArtist;
			(void)(*stOsal.osmemset)((void*)&getArtist, (S8)0, (U32)sizeof(HDR_psd_data_t));
			ret = HDR_psd_get_artist(hdrInstance, (HDR_program_t)program_number, &getArtist);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)artist, (void*)&getArtist, (U32)sizeof(stTC_HDR_PSD_FORM_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_getAlbum(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, stTC_HDR_PSD_FORM_t* album)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_psd_data_t getAlbum;
			(void)(*stOsal.osmemset)((void*)&getAlbum, (S8)0, (U32)sizeof(HDR_psd_data_t));
			ret = HDR_psd_get_album(hdrInstance, (HDR_program_t)program_number, &getAlbum);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)album, (void*)&getAlbum, (U32)sizeof(stTC_HDR_PSD_FORM_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_getGenre(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, stTC_HDR_PSD_FORM_t* genre)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_psd_data_t getGenre;
			(void)(*stOsal.osmemset)((void*)&getGenre, (S8)0, (U32)sizeof(HDR_psd_data_t));
			ret = HDR_psd_get_genre(hdrInstance, (HDR_program_t)program_number, &getGenre);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)genre, (void*)&getGenre, (U32)sizeof(stTC_HDR_PSD_FORM_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_getComment(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, eTC_HDR_PSD_COMM_SUBFIELD_t subfield, stTC_HDR_PSD_FORM_t* comment)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_psd_data_t getComment;
			(void)(*stOsal.osmemset)((void*)&getComment, (S8)0, (U32)sizeof(HDR_psd_data_t));
			ret = HDR_psd_get_comment(hdrInstance, (HDR_program_t)program_number, (HDR_psd_comm_subfield_t)subfield, &getComment);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)comment, (void*)&getComment, (U32)sizeof(stTC_HDR_PSD_FORM_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_getUfid(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, U32 ufid_num, eTC_HDR_PSD_UFID_SUBFIELD_t subfield, stTC_HDR_PSD_FORM_t* ufid)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_psd_data_t getUfid;
			(void)(*stOsal.osmemset)((void*)&getUfid, (S8)0, (U32)sizeof(HDR_psd_data_t));
			ret = HDR_psd_get_ufid(hdrInstance, (HDR_program_t)program_number, ufid_num, (HDR_psd_ufid_subfield_t)subfield, &getUfid);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)ufid, (void*)&getUfid, (U32)sizeof(stTC_HDR_PSD_FORM_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_getCommercial(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, eTC_HDR_PSD_COMR_SUBFIELD_t subfield, stTC_HDR_PSD_FORM_t* commercial)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_psd_data_t getCommercial;
			(void)(*stOsal.osmemset)((void*)&getCommercial, (S8)0, (U32)sizeof(HDR_psd_data_t));
			ret = HDR_psd_get_commercial(hdrInstance, (HDR_program_t)program_number, (HDR_psd_comr_subfield_t)subfield, &getCommercial);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)commercial, (void*)&getCommercial, (U32)sizeof(stTC_HDR_PSD_FORM_t));
			}
		}
	}
	return ret;
}

HDRET tchdr_psd_getXhdr(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t program_number, U32 xhdr_num, stTC_HDR_PSD_FORM_t* xhdr)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdrfwk_getHdrInstancePtr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			HDR_psd_data_t getXhdr;
			(void)(*stOsal.osmemset)((void*)&getXhdr, (S8)0, (U32)sizeof(HDR_psd_data_t));
			ret = HDR_psd_get_xhdr(hdrInstance, (HDR_program_t)program_number, xhdr_num, &getXhdr);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_NOT_ENABLED;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)xhdr, (void*)&getXhdr, (U32)sizeof(stTC_HDR_PSD_FORM_t));
			}
		}
	}
	return ret;
}

