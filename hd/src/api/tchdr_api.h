/*******************************************************************************

*   FileName : tchdr_api.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio Framework APIs header

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
#ifndef TCHDR_API_H__
#define TCHDR_API_H__

/***************************************************
*				Include					*
****************************************************/
#include <stdbool.h>
#include "tchdr_types.h"

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define	TC_HDR_AUDIO_FRAME_SIZE				(2048)	// Do not change value.

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eTC_HDR_TYPE_HD_1p0	= 1,			// HD 1.0
	eTC_HDR_TYPE_HD_1p5	= 2,			// HD 1.0 + Background Scan
	eTC_HDR_TYPE_HD_1p0_MRC	= 3,		// HD 1.0 + MRC
	eTC_HDR_TYPE_HD_1p5_MRC	= 4,		// HD 1.5 + MRC
	eTC_HDR_TYPE_HD_1p5_DUAL_MRC = 5	// HD 1.5 + Dual MRC
}eTC_HDR_TYPE_t;

typedef enum {
	eTC_HDR_ID_MAIN		= 0,
	eTC_HDR_ID_MRC		= 1,
	eTC_HDR_ID_BS		= 2,
	eTC_HDR_ID_BS_MRC	= 3
}eTC_HDR_ID_t;

typedef enum {
	eTC_HDR_FM_BAND		= 0,
	eTC_HDR_AM_BAND		= 1,
	eTC_HDR_IDLE_BAND	= 0xFF			// This setting is not available when open HDR
} eTC_HDR_BAND_t;

typedef enum {
    eTC_HDR_BBSRC_650_KHZ	= 0,		// Not supported yet
    eTC_HDR_BBSRC_675_KHZ	= 1,		// Not supported yet
    eTC_HDR_BBSRC_744_KHZ	= 2,		// 744.1875Khz
    eTC_HDR_BBSRC_768_KHZ	= 3,		// Not supported yet
    eTC_HDR_BBSRC_1024_KHZ	= 4,		// Not supported yet
    eTC_HDR_BBSRC_UNKNOWN	= 5
}eTC_HDR_BBSRC_RATE_t;

typedef enum {
	eBITMASK_SIGNAL_STATUS_HD_SIGNAL	= 0x01U,
	eBITMASK_SIGNAL_STATUS_SIS			= 0x02U,
	eBITMASK_SIGNAL_STATUS_SIS_OK		= 0x04U,
	eBITMASK_SIGNAL_STATUS_HD_AUDIO		= 0x08U
}eTC_HDR_SIGNAL_STATUS_t;

typedef enum {
	eTC_HDR_AUDIO_BLEND		= 0,
	eTC_HDR_AUDIO_ANALOG	= 1,
	eTC_HDR_AUDIO_DIGITAL	= 2,
	eTC_HDR_AUDIO_SPLIT		= 3		// For Tuning
}eTC_HDR_AUDIO_MODE_t;

typedef enum {
    eTC_HDR_PROGRAM_HD1 = 0,
    eTC_HDR_PROGRAM_HD2 = 1,
    eTC_HDR_PROGRAM_HD3 = 2,
    eTC_HDR_PROGRAM_HD4 = 3,
    eTC_HDR_PROGRAM_HD5 = 4,
    eTC_HDR_PROGRAM_HD6 = 5,
    eTC_HDR_PROGRAM_HD7 = 6,
    eTC_HDR_PROGRAM_HD8 = 7,
    eTC_HDR_PROGRAM_MAX = 8
}eTC_HDR_PROGRAM_t;

typedef enum
{
	eTC_HDR_NOTIFY_NULL				= 0,

	eTC_HDR_NOTIFY_OPEN				= 101,

	eTC_HDR_NOTIFY_AUDIO_MODE 		= 111,
	eTC_HDR_NOTIFY_TUNE 			= 112,
	eTC_HDR_NOTIFY_PROGRAM			= 113,		// For Demo
	eTC_HDR_NOTIFY_MUTE				= 114,
	eTC_HDR_NOTIFY_AUDIO_CTRL 		= 115,

	eTC_HDR_NOTIFY_PSD				= 121,		// For Demo
	eTC_HDR_NOTIFY_SIS				= 122,		// For Demo
	eTC_HDR_NOTIFY_ALERT			= 125,
	eTC_HDR_NOTIFY_LOT				= 126,

	eTC_HDR_NOTIFY_SIGNAL_STATUS	= 190,
	eTC_HDR_NOTIFY_PTY				= 191		// For Demo
}eTC_HDR_NOTIFY_t;

// For Blend
typedef enum{
    eBLEND_THRESH_FORCE_ANALOG = 0,
    eBLEND_THRESH_Q1_THRESH,
    eBLEND_THRESH_Q2_THRESH,
    eBLEND_THRESH_Q3_THRESH,
    eBLEND_THRESH_Q4_THRESH,
    eBLEND_THRESH_RESERVED1,
    eBLEND_THRESH_RESERVED2,
    eBLEND_THRESH_FORCE_DIGITAL,
    eBLEND_THRESH_TOTAL
}eTC_HDR_BLEND_THRESH_SEL_t;

typedef enum {
    eBLEND_FM_MPS_BLEND_THRESH = 0,
    eBLEND_FM_ALL_DIG_BLEND_THRESH,
    eBLEND_FM_MPS_AUDIO_SCALING,
    eBLEND_FM_ALL_DAUD_SCALLING,
    eBLEND_FM_MPS_BLEND_RATE,
    eBLEND_FM_ALL_DIG_BLEND_RATE,
    eBLEND_FM_MPS_DAUD_DELAY,
    eBLEND_AM_MPS_BLEND_THRESH,
    eBLEND_AM_ALL_DIG_BLEND_THRESH,
    eBLEND_AM_MPS_AUDIO_SCALING,
    eBLEND_AM_ALL_DAUD_SCALING,
    eBLEND_AM_MPS_DAUD_DELAY,
    eBLEND_AM_MPS_BLEND_RATE,
    eBLEND_AM_ALL_DIG_BLEND_RATE,
    eBLEND_D2A_BLEND_HOLDOFF,
    eBLEND_BLEND_DECISION,
    eBLEND_FM_CDNO_BLEND_DECISION,
    eBLEND_AM_CDNO_BLEND_DECISION,
    eBLEND_FM_AUDIO_INVERT_PHASE,
    eBLEND_AM_AUDIO_INVERT_PHASE,
    eBLEND_DISABLE_AUDIO_SCALING
}eTC_HDR_BLEND_PARAMS_t;

typedef enum {
    eBLEND_ADV_RAMP_UP_ENABLED = 0,
	eBLEND_ADV_RAMP_UP_TIME,
	eBLEND_ADV_RAMP_DOWN_ENABLED,
	eBLEND_ADV_RAMP_DOWN_TIME,
	eBLEND_ADV_COMFORT_NOISE_ENABLED,
	eBLEND_ADV_COMFORT_NOISE_LEVEL,
	eBLEND_ADV_AM_ENH_STREAM_HOLDOFF_ENABLED,
	eBLEND_ADV_AM_MPS_ENH_STREAM_HOLDOFF_THRESH,
	eBLEND_ADV_ALL_DIG_ENH_STREAM_HOLDOFF_THRESH,
	eBLEND_ADV_AM_DAUD_BW_MGMT_ENABLED,
	eBLEND_ADV_AM_DAUD_BLEND_START_BW,
	eBLEND_ADV_AM_DAUD_MAX_BW,
	eBLEND_ADV_AM_DAUD_BW_STEP_TIME,
	eBLEND_ADV_AM_DAUD_BW_STEP_UP_SIZE,
	eBLEND_ADV_AM_DAUD_BW_STEP_DOWN_SIZE,
	eBLEND_ADV_AM_DAUD_BW_STEP_THRESHOLD,
	eBLEND_ADV_AM_MONO2STEREO_ENABLED,
	eBLEND_ADV_AM_MONO2STEREO_START_BW,
	eBLEND_ADV_AM_MONO2STEREO_STEP_TIME,
	eBLEND_ADV_AM_MONO2STEREO_MAX_SEP
}eTC_HDR_BLEND_ADV_PARAMS_t;

typedef enum {
	eTHREAD_MANAGER   =0,	// Manager Thread
	eTHREAD_IQINPUT   =1,	// IQ Input Thread
	eTHREAD_AUDINPUT  =2,	// Audio Input Thread
	eTHREAD_BBINPUT   =3,	// Base-Band Input Thread
	eTHREAD_DEMOD     =4,	// Demodulation Thread
	eTHREAD_BLENDING  =5,	// Blending Audio Thread
	eTHREAD_AUDOUTPUT =6,	// Audio Output Thread
	eTHREAD_CMDPROC   =7,	// Command Process Thread for CDM I/F
	eTHREAD_LOGGER    =8,	// Logger Thread
	eTHREAD_MAX       =9
}eTC_HDR_THREAD_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct{
	S32 policy;			// Policy: 0:Normal Policy, 1:Real-Time Policy
	S32 priority;		// Real-Time Priority Range: 1(Low) ~ 99(High)
						// Normal Priority(NICE) Range: +19(Low) ~ -20(High)
}stTC_HDR_THREAD_PR_t;

typedef struct {
	U32						samplingBit;
	eTC_HDR_BBSRC_RATE_t	maxSampleRate;
}stTC_HDR_IQ_t;

typedef struct {
	eTC_HDR_TYPE_t	hdrType;
	stTC_HDR_IQ_t	iq;
	void			*reserved;		// not yet used.
}stTC_HDR_CONF_t;

typedef struct {
	eTC_HDR_BAND_t 			band;
	U32						freq;
	eTC_HDR_BBSRC_RATE_t	iqsamplerate;
}stTC_HDR_TUNE_TO_t;

typedef struct {
	stTC_HDR_TUNE_TO_t mainTuner;	// Primary
	stTC_HDR_TUNE_TO_t mrcTuner;	// Primary MRC
	stTC_HDR_TUNE_TO_t bsTuner;		// Background Scan/Search
	stTC_HDR_TUNE_TO_t bsmrcTuner;	// Background Scan/Search MRC
}stTC_HDR_TUNE_INFO_t;

typedef struct{
	eTC_HDR_ID_t hdrID;		// HD Radio ID
	U32 curPN;				// current program number
	U32 acqStatus;			// [3:0] [digital_audio_acquired : sis_crc_ok : sis_acquired : hd_signal_acquired]
	U32 cnr;				// carrier to noise ratio
	U32 pmap;				// audio available program bitmap
	U32 hybridProgram;		// hybrid program
} stTC_HDR_SIGNAL_STATUS_t;

typedef union {
    struct {
        U8 prog1:1;
        U8 prog2:1;
        U8 prog3:1;
        U8 prog4:1;
        U8 prog5:1;
        U8 prog6:1;
        U8 prog7:1;
        U8 prog8:1;
    }prog;
    U8 all;
}stTC_HDR_PROG_BITMAP_t;

typedef struct {
	eTC_HDR_ID_t hdrID;
	U32 curPN;
	U32 acqStatus;
	U32 audioQualityIndicator;
	U32 cnr;
	U32 digitalAudioGain;
	U32 blendControl;
	U32 pty[eTC_HDR_PROGRAM_MAX];
	U32 curPty;
	U32 pmap;
	U32 chgPmap;
	U32 psm;
	U32 codecMode;
	U32 hybridProgram;
	U32 dsqm;
	U32 rawSnr;
}stTC_HDR_STATUS_t;

typedef struct{
    U16 left;
    U16 right;
}stTC_HDR_PCM_t;

typedef struct{
    U32 value[eTC_HDR_PROGRAM_MAX];
}stTC_HDR_PTY_t;

typedef struct {
    eTC_HDR_BLEND_THRESH_SEL_t fm_mps_blend_thresh;
    eTC_HDR_BLEND_THRESH_SEL_t fm_all_dig_blend_thresh;
    U32 fm_mps_audio_scaling;
    U32 fm_all_dig_audio_scaling;
    U32 fm_mps_blend_rate;
    U32 fm_all_dig_blend_rate;
    U32 fm_mps_dig_audio_delay;
    eTC_HDR_BLEND_THRESH_SEL_t am_mps_blend_thresh;
    eTC_HDR_BLEND_THRESH_SEL_t am_all_dig_blend_thresh;
    U32 am_mps_audio_scaling;
    U32 am_all_dig_audio_scaling;
    U32 am_mps_dig_audio_delay;
    U32 am_mps_blend_rate;
    U32 am_all_dig_blend_rate;
    U32 d2a_blend_holdoff;
    HDBOOL blend_decision;
    U32 fm_cdno_blend_decision;
    U32 am_cdno_blend_decision;
    HDBOOL fm_audio_invert_phase;
    HDBOOL am_audio_invert_phase;
    HDBOOL disable_audio_scaling;
}stTC_HDR_BLEND_PARAMS_t;

typedef struct {
    HDBOOL ramp_up_enabled;
    U32 ramp_up_time;
    HDBOOL ramp_down_enabled;
    U32 ramp_down_time;
    HDBOOL comfort_noise_enabled;
    S32 comfort_noise_level;
    HDBOOL am_enh_stream_holdoff_enabled;
    U32 am_mps_enh_stream_holdoff_thresh;
    U32 am_all_dig_enh_stream_holdoff_thresh;
    HDBOOL am_dig_audio_bw_mgmt_enabled;
    U32 am_dig_audio_blend_start_bw;
    U32 am_dig_audio_max_bw;
    U32 am_dig_audio_bw_step_time;
    U32 am_dig_audio_bw_step_up_size;
    U32 am_dig_audio_bw_step_down_size;
    U32 am_dig_audio_bw_step_threshold;
    HDBOOL am_mono2stereo_enabled;
    U32 am_mono2stereo_start_bw;
    U32 am_mono2stereo_step_time;
    U32 am_mono2stereo_max_sep;
}stTC_HDR_BLEND_ADV_PARAMS_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern stTC_HDR_CONF_t	stTcHdrConf;

/***************************************************
*			Function declaration				*
****************************************************/
extern const S8 *tchdr_getFrameworkVersionString(void);
extern const S8 *tchdr_getLibraryVersionString(void);
extern HDRET tchdr_init(stTC_HDR_CONF_t conf);
extern HDRET tchdr_deinit(void);
extern HDRET tchdr_open(stTC_HDR_TUNE_INFO_t tuneInfo); // async
extern HDRET tchdr_close(void);
extern HDRET tchdr_setTune(eTC_HDR_ID_t id, stTC_HDR_TUNE_TO_t tuneTo);	// async
extern HDRET tchdr_setProgram(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t numOfProgram);
extern HDRET tchdr_getProgram(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t *numOfProgram);
extern HDRET tchdr_getAvailablePrograms(eTC_HDR_ID_t id, stTC_HDR_PROG_BITMAP_t *availablePrograms);
extern HDRET tchdr_getSignalStatus(eTC_HDR_ID_t id, stTC_HDR_SIGNAL_STATUS_t *dataOut);
extern HDRET tchdr_getAllStatus(eTC_HDR_ID_t id, stTC_HDR_STATUS_t *dataOut);
extern HDRET tchdr_setAudioMode(eTC_HDR_AUDIO_MODE_t audioMode);	// async
extern HDRET tchdr_setAudioMute(U32 fOnOff);	// async
extern HDRET tchdr_setAudioCtrl(U32 fStartStop);	// async
extern HDRET tchdr_setAnalogAudioMute(U32 fOnOff);
extern HDRET tchdr_setAudioMuteFader(U32 enable, U32 fadein_ms, U32 fadeout_ms);
extern HDRET tchdr_getAudioMuteFader(U32 *enable, U32 *fadein_ms, U32 *fadeout_ms);
extern HDRET tchdr_getProgramType(eTC_HDR_ID_t id, stTC_HDR_PTY_t *pty);

// Blend
extern HDRET tchdr_setBlendTransitionTime(U32 transition_time);
extern HDRET tchdr_setBlendAllParams(stTC_HDR_BLEND_PARAMS_t params);
extern HDRET tchdr_getBlendAllParams(stTC_HDR_BLEND_PARAMS_t *params);
extern HDRET tchdr_setBlendParam(eTC_HDR_BLEND_PARAMS_t param, U32 param_value);
extern HDRET tchdr_getBlendParam(eTC_HDR_BLEND_PARAMS_t param, U32 *param_value);
extern HDRET tchdr_setBlendAllAdvParams(stTC_HDR_BLEND_ADV_PARAMS_t params);
extern HDRET tchdr_getBlendAllAdvParams(stTC_HDR_BLEND_ADV_PARAMS_t *params);
extern HDRET tchdr_setBlendAdvParam(eTC_HDR_BLEND_ADV_PARAMS_t param, U32 param_value);
extern HDRET tchdr_getBlendAdvParam(eTC_HDR_BLEND_ADV_PARAMS_t param, U32 *param_value);

// AAA
extern HDRET tchdr_setAutoAudioAlignEnable(U32 fEnable);

// Thread Priority
// The tchdr_setThreadPriority() function must be set before initialization.
extern HDRET tchdr_setThreadPriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t userprio);
extern HDRET tchdr_getDefaultThreadPriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t *userprio);
extern HDRET tchdr_getDefaultThreadNicePriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t *userprio);
extern HDRET tchdr_getThreadPriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t *userprio);

// Config Callback Functions
extern void tchdr_configTunerIQ01Driver(S32(*pfnIQ01DrvOpen)(void),
												 S32(*pfnIQ01DrvClose)(void),
												 S32(*pfnIQ01DrvSetParams)(S32 nchannels, S32 nbit, S32 samplerate, S32 buffersize, S32 periodsize),
												 S32(*pfnIQ01DrvStart)(void),
												 S32(*pfnIQ01DrvStop)(void),
												 S32(*pfnIQ01DrvRead)(S8 *data, S32 readsize));
extern void tchdr_configTunerIQ23Driver(S32(*pfnIQ23DrvOpen)(void),
												 S32(*pfnIQ23DrvClose)(void),
												 S32(*pfnIQ23DrvSetParams)(S32 nchannels, S32 nbit, S32 samplerate, S32 buffersize, S32 periodsize),
												 S32(*pfnIQ23DrvStart)(void),
												 S32(*pfnIQ23DrvStop)(void),
												 S32(*pfnIQ23DrvRead)(S8 *data, S32 readsize));
extern void tchdr_configTunerBlendAudioDriver(S32(*pfnBlendAudioDrvOpen)(void),
										  		S32(*pfnBlendAudioDrvClose)(void),
										  		S32(*pfnBlendAudioDrvSetParams)(S32 nchannels, S32 nbit, S32 samplerate, S32 buffersize, S32 periodsize),
										  		S32(*pfnBlendAudioDrvStart)(void),
										  		S32(*pfnBlendAudioDrvStop)(void),
										  		S32(*pfnBlendAudioDrvRead)(S8 *data, S32 readsize));
extern void tchdr_configTcHdrNotificationCallBack(void(*pfnNotificationCallBack)(U32 notifyID, const U32 *pArg, void* const *pData, S32 errorCode));
extern void tchdr_configTcHdrAudioQueueCallBack(void(*pfnAudioQueueCallBack)(void *pOutBuf, S32 frames, U32 samplerate));

// for DEMO, These are not supported.
extern HDRET tchdr_enablePsdNotification(eTC_HDR_ID_t id, U8 progBitmask, U8 psdBitmask, U32 fEn); // async
extern HDRET tchdr_enableSisNotification(eTC_HDR_ID_t id, U32 sisBitmask, U32 fEn); // async
extern HDRET tchdr_enableLotNotification(eTC_HDR_ID_t id, U8 progBitmask, U32 fEn);
extern HDRET tchdr_enableAlertNotification(eTC_HDR_ID_t id, U32 fEn);

// for Debugging, Please do not use this.
extern HDRET tchdr_debugTcHdrFramework(U32 id, U32 numdbg, const U32 *parg);

#ifdef __cplusplus
}
#endif

#endif
