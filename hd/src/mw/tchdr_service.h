/*******************************************************************************

*   FileName : tchdr_service.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio framework service header

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
#ifndef TCHDR_SERVICE_H__
#define TCHDR_SERVICE_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
*				Enumeration				*
****************************************************/

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct{
	HDR_tune_band_t			band;	// Current Band (FM or AM)
	U16 					freq;	// Current Frequency (2byte)
	HDR_bb_src_input_rate_t	iqsr;	// IQ Sample Rate
}stTCHDR_TUNE_t;

typedef struct {
	stTCHDR_TUNE_t mainInstance;
	stTCHDR_TUNE_t mrcInstance;
	stTCHDR_TUNE_t bsInstance;
	stTCHDR_TUNE_t bsmrcInstance;
}stTCHDR_TUNE_INFO_t;

typedef struct{
	struct {
		U8 fNotify;
		U32 checkInterval;
		U8 reserved[3];
	}status;
	struct {
		U8 fNotify;
		U32 value[HDR_MAX_NUM_PROGRAMS];
		U32 checkInterval;	// 10ms x checkInterval
		U8 reserved[3];
	}pty;
	struct {
		U8 fNotify;			// force
		U8 progBitmask;		// HDR_program_t (refer to hdrAudio.h)
		U8 fieldBitmask;	// HDR_psd_fields_t (refer to hdrPsd.h)
		U32 checkInterval;	// 10ms x checkInterval
		U8 reserved[1];
	}psd;
	struct {
		U8 fNotify;
		U8 enDefaultType;	// HDR_sis_enabled_basic_types_t
		U32 fieldBitmask;
		U32 checkInterval;	// 10ms x checkInterval
		U8 reserved[2];
	}sis;
	struct {
		U8 fNotify;
		U32 progBitmask;
		U32 fieldBitmask;
		U8 reserved[3];
	}sig;
	struct {
		U8 fNotify;
		U32 progBitmask;
		U32 fieldBitmask;
		U8 reserved[3];
	}aas;
	struct {
		U8 fNotify;
		U8 progBitmask;
		U32 checkInterval;
		U8 reserved[2];
	}lot;
	struct {
		U8 fNotify;
		U32 checkInterval;
		U8 reserved[3];
	}alert;
}stTCHDR_NOTIFY_t;

typedef union {
    struct{
        U8 hdsig:1;	// HD Signal
        U8 hdaud:1;	// HD Audio
        U8 sis:1;	// SIS Signal
        U8 sisok:1;	// SIS CRC OK
    }status;
    U8 all;			// Used to read/write all fields at once
}stTCHDR_ACQSTATUS_t;

typedef struct{
	stTCHDR_ACQSTATUS_t acqStatus;
	U32 cdno;
	HDBOOL hybrid;
}stTCHDR_STATUS_t;

typedef struct{
	U32 fInit;
	U32 fOpen;
	U32 statusChkInterval;

	HDR_program_t mainHdrProgNum;
	HDR_program_t bsHdrProgNum;
	stTCHDR_STATUS_t mainSts;
	stTCHDR_STATUS_t bsSts;
	stTCHDR_NOTIFY_t mainHdr;
	stTCHDR_NOTIFY_t bsHdr;

	eTCHDR_SVC_CMD_t eEventMode;
}stTCHDR_SVC_t;

typedef struct{
	HDR_instance_t *hdrInstance;
	eTC_HDR_ID_t id;
	U32 ServIdx;
	U32 ServNum;
	U32 Program_number;
	U16 lotId;
	U16 port_number;
}stTCHDR_LOTPROC_PARAM_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern stTCHDR_TUNE_INFO_t stTcHdrTuneInfo;

/***************************************************
*			Function declaration				*
****************************************************/
extern void *tchdrsvc_mainThread(void* arg);
extern HDRET tchdrsvc_init(void);
extern HDRET tchdrsvc_deinit(void);
extern HDRET tchdrsvc_open(void);
extern HDRET tchdrsvc_close(void);
extern HDRET tchdrsvc_getHdrSignalStatus(eTC_HDR_ID_t id, stTC_HDR_SIGNAL_STATUS_t *dataOut);
extern HDRET tchdrsvc_getHdrStatus(eTC_HDR_ID_t id, stTC_HDR_STATUS_t *dataOut);
extern HDRET tchdrsvc_enableGetPsd(eTC_HDR_ID_t id, U8 progBitmask, U8 psdBitmask, U32 fEn);
extern HDRET tchdrsvc_enableGetSis(eTC_HDR_ID_t id, U32 sisBitmask, U32 fEn);
extern HDRET tchdrsvc_enableGetLot(eTC_HDR_ID_t id, U8 progBitmask, U32 fEn);
extern HDRET tchdrsvc_getSisBasicData(eTC_HDR_ID_t id, stTC_HDR_SIS_t *dataOut, U32 *status);

extern void tchdrsvc_setOpenStatus(U32 sts);
extern U32 tchdrsvc_getOpenStatus(void);
extern HDRET tchdr_getHdrIdFromInstanceNumber(eTC_HDR_ID_t *hdrID, U32 instanceNum);
extern HDRET tchdr_getHdrTuneInfoWithInstanceNumber(stTCHDR_TUNE_t *tuneTo, U32 instanceNum);
extern HDRET tchdrsvc_setProgramNumber(const HDR_instance_t *hdrInstance, HDR_program_t program);
extern HDRET tchdrsvc_getProgramNumber(const HDR_instance_t *hdrInstance, HDR_program_t *program);

#ifdef __cplusplus
}
#endif

#endif
