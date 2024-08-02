/*******************************************************************************

*   FileName : tcradio_service.h

*   Copyright (c) Telechips Inc.

*   Description :

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
#ifndef __RADIO_SERVICE_H__
#define __RADIO_SERVICE_H__

/***************************************************
*				Include					*
****************************************************/
#ifdef __ANDROID__
#include <android/log.h>
#endif

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "DMBLog.h"

#define RADIO_SRV_DEBUG

#ifdef __ANDROID__

#define RSRV_TAG			("[RADIO][SRV]")
#define RSRV_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,RSRV_TAG, __VA_ARGS__))
#define RSRV_WRN(...)		(__android_log_print(ANDROID_LOG_WARN,RSRV_TAG, __VA_ARGS__))
#define RSRV_INF(...)		(__android_log_print(ANDROID_LOG_INFO,RSRV_TAG, __VA_ARGS__))
#ifdef RADIO_SRV_DEBUG
#define RSRV_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,RSRV_TAG, __VA_ARGS__))
#else
#define	RSRV_DBG(...)
#endif

#else // #ifdef __ANDROID__

// #define RSRV_ERR(...)		((void)printf("[ERROR][RADIO][SRV]: " __VA_ARGS__))
// #define RSRV_WRN(...)		((void)printf("[WARN][RADIO][SRV]: " __VA_ARGS__))
// #define RSRV_INF(...)		((void)printf("[INFO][RADIO][SRV]: " __VA_ARGS__))
#define RSRV_ERR(...)		((void)LB_PRINTF("[ERROR][RADIO][SRV]: " __VA_ARGS__))
#define RSRV_WRN(...)		((void)LB_PRINTF("[WARN][RADIO][SRV]: " __VA_ARGS__))
#define RSRV_INF(...)		((void)LB_PRINTF("[INFO][RADIO][SRV]: " __VA_ARGS__))
#ifdef RADIO_SRV_DEBUG
// #define RSRV_DBG(...)		((void)printf("[DEBUG][RADIO][SRV]: " __VA_ARGS__))
#define RSRV_DBG(...)		((void)LB_PRINTF("[DEBUG][RADIO][SRV]: " __VA_ARGS__))
#else
#define	RSRV_DBG(...)
#endif

#endif // #ifdef __ANDROID__

#define	SERVICE_THREAD_TIME_INTERVAL	10

#define	__RADIO_MIDDLEWARE_VER_PREFIX__				'V'
#define	__RADIO_MIDDLEWARE_VER_RELEASE_NUMBER__		0x02
#define	__RADIO_MIDDLEWARE_VER_MAJOR_NUMBER__		0x00
#define	__RADIO_MIDDLEWARE_VER_MINOR_NUMBER__		0x00

#define _MaxPresetNum		18
#define	_AmFmDBSize			100

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eRADIO_STS_OK					= 0,	/* Job Complete & Notify */
	eRADIO_STS_ERROR				= 1,	/* Critical Error */
	eRADIO_STS_DOING				= 4,	/* Job Continue */
	eRADIO_STS_DOING_NOTIFY			= 5,	/* Job Continue & Notify Case*/
	eRADIO_STS_DOING_ERROR_NOTIFY	= 6,	/* Job COntinue & Error Notify */
	eRADIO_STS_OK_NOTIFY			= 7,	/* Job Complete & Notify Case */
	eRADIO_STS_WAIT					= 8,
	eRADIO_STS_DONE					= 9,
	eRADIO_STS_END
} eRADIO_STS_t;

typedef enum
{
	eRADIO_CMD_NULL			= 0,

	eRADIO_CMD_OPEN			= 1,
	eRADIO_CMD_DEINIT		= 4,
	eRADIO_CMD_SET_TUNE		= 7,
    eRADIO_CMD_SET_SEEK		= 8,
    eRADIO_CMD_SET_COEF		= 9,
    eRADIO_CMD_SET_DAB_FREQ_LIST = 10,

	eRADIO_CMD_GET_PRESET_LIST   = 132,
	eRADIO_CMD_GET_COEF			 = 133,
	eRADIO_CMD_GET_DAB_FREQ_LIST = 134,

	eRADIO_CMD_END
}eRADIO_CMD_t;

typedef enum
{
    eRADIO_EVT_NULL			= 0,

	eRADIO_EVT_OPEN			= 1,
	eRADIO_EVT_DEINIT		= 4,
	eRADIO_EVT_SET_TUNE		= 7,
    eRADIO_EVT_SET_SEEK		= 8,
    eRADIO_EVT_SET_COEF		= 9,
    eRADIO_EVT_SET_DAB_FREQ_LIST = 10,

	eRADIO_EVT_GET_PRESET_LIST   = 132,
	eRADIO_EVT_GET_COEF			 = 133,
	eRADIO_EVT_GET_DAB_FREQ_LIST = 134,

    eRADIO_EVT_END
}eRADIO_EVENT_t;

typedef enum{
	eSEEK_STEP_ON_AIR		= 0,
    eSEEK_STEP_START		= 1,
    eSEEK_STEP_SET_FREQ		= 2,
    eSEEK_STEP_PRECHK_QDATA	= 3,
    eSEEK_STEP_CHK_QDATA_STATUS	= 4,
    eSEEK_STEP_CHK_QDATA 	= 5,
    eSEEK_STEP_STOP			= 6,
    eSEEK_STEP_CHK_PI		= 7,
	eSEEK_STEP_END
}eSEEK_STEP_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct{
	int32 fThreadRunning;
	int32 fRadioOpened;
	eRADIO_EVENT_t eMainMode;
	eSEEK_STEP_t eSeekStep;
	eRADIO_SEEK_MODE_t eSeekMode;
	uint32 curSeekResult;

	uint32 curBand;		// eRADIO_MOD_MODE_t
	uint32 curFreq;
	uint32 curStartFreq;

	uint32 currentBandPresetFreq[eRADIO_DAB_MODE+1][_MaxPresetNum];
	uint32 currentBandFreq[eRADIO_DAB_MODE+1];	// 각 Band에 따른 Frequency

	struct {
		uint32 startFreq;
		uint32 endFreq;
		uint32 step;
	}fm;

	struct {
		uint32 startFreq;
		uint32 endFreq;
		uint32 step;
	}am;

	struct {
		uint32 startIndex;
		uint32 endIndex;
		uint32 step;
	}dab;

	stRADIO_QUALITY_t stSchQdata;
	uint32 scanPI;		// DAB Seamless Link
}stRADIO_SERVICE_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern stRADIO_SERVICE_t stRadioService;
extern stRADIO_CONFIG_t stTunerConfig;
extern stRADIO_LIST_t stRadioList[_MaxPresetNum];
extern uint32 scanPiList[128];

/***************************************************
*			Function declaration				*
****************************************************/
extern RET tcradioservice_init(void);
extern RET tcradioservice_close(void);

extern uint32 tcradioservice_getCurrentFrequency(void);
extern RET tcradioservice_setBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 start_freq, uint32	end_freq, uint32 step);
extern RET tcradioservice_getBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 *start_freq, uint32 *end_freq, uint32 *step);
extern uint32 tcradioservice_getNumberOfTuners(void);
extern RET tcradioservice_checkValidFreq(eRADIO_MOD_MODE_t mod_mode, uint32 freq);

#ifdef __cplusplus
}
#endif

#endif
