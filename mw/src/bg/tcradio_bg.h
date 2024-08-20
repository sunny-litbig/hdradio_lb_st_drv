/*******************************************************************************

*   FileName : tcradio_bg.h

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
#ifndef __RADIO_BG_H__
#define __RADIO_BG_H__

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

#define RADIO_BG_DEBUG

#ifdef __ANDROID__

#define RBG_TAG			("[RADIO][BG]")
#define RBG_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,RBG_TAG, __VA_ARGS__))
#define RBG_WRN(...)		(__android_log_print(ANDROID_LOG_WARN,RBG_TAG, __VA_ARGS__))
#define RBG_INF(...)		(__android_log_print(ANDROID_LOG_INFO,RBG_TAG, __VA_ARGS__))
#ifdef RADIO_BG_DEBUG
#define RBG_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,RBG_TAG, __VA_ARGS__))
#else
#define	RBG_DBG(...)
#endif

#else // #ifdef __ANDROID__

#define RBG_ERR(...)		((void)printf("[ERROR][RADIO][BG]: " __VA_ARGS__))
#define RBG_WRN(...)		((void)printf("[WARN][RADIO][BG]: " __VA_ARGS__))
#define RBG_INF(...)		((void)printf("[INFO][RADIO][BG]: " __VA_ARGS__))
#ifdef RADIO_BG_DEBUG 
#define RBG_DBG(...)		((void)printf("[DEBUG][RADIO][BG]: " __VA_ARGS__))
#else
#define	RBG_DBG(...)
#endif

#endif // #ifdef __ANDROID__

#define	BG_THREAD_TIME_INTERVAL 10
#define BG_WAIT_TIME_BGRESTART  1000    // 10sec.

#define	_AmFmDBSize			100
/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eRADIO_BG_STS_OK					= 0,	/* Job Complete & Notify */
	eRADIO_BG_STS_ERROR				    = 1,	/* Critical Error */
	eRADIO_BG_STS_DOING				    = 4,	/* Job Continue */
	eRADIO_BG_STS_DOING_NOTIFY			= 5,	/* Job Continue & Notify Case*/
	eRADIO_BG_STS_DOING_ERROR_NOTIFY	= 6,	/* Job COntinue & Error Notify */
	eRADIO_BG_STS_OK_NOTIFY			    = 7,	/* Job Complete & Notify Case */
	eRADIO_BG_STS_WAIT					= 8,
	eRADIO_BG_STS_DONE					= 9,
	eRADIO_BG_STS_END
} eRADIO_BG_STS_t;

typedef enum
{
	eRADIO_BG_CMD_NULL			= 0,

	eRADIO_BG_CMD_OPEN			= 1,
	eRADIO_BG_CMD_DEINIT		= 4,
	eRADIO_BG_CMD_SET_TUNE		= 7,
    eRADIO_BG_CMD_START		    = 8,
    eRADIO_BG_CMD_STOP		    = 11,

	eRADIO_BG_CMD_GET_PRESET_LIST   = 132,
	eRADIO_BG_CMD_GET_COEF			= 133,
	eRADIO_BG_CMD_GET_DAB_FREQ_LIST = 134,

	eRADIO_BG_CMD_END
}eRADIO_BG_CMD_t;

typedef enum
{
    eRADIO_BG_EVT_NULL			= 0,

	eRADIO_BG_EVT_OPEN			= 1,
	eRADIO_BG_EVT_DEINIT		= 4,
	eRADIO_BG_EVT_SET_TUNE		= 7,
    eRADIO_BG_EVT_SET_SEEK		= 8,
    eRADIO_BG_EVT_SET_COEF		= 9,
    eRADIO_BG_EVT_SET_DAB_FREQ_LIST = 10,
    eRADIO_BG_EVT_STOP          = 11,

	eRADIO_BG_EVT_GET_PRESET_LIST   = 132,
	eRADIO_BG_EVT_GET_COEF			 = 133,
	eRADIO_BG_EVT_GET_DAB_FREQ_LIST = 134,

    eRADIO_BG_EVT_END
}eRADIO_BG_EVENT_t;

typedef enum{
	eRADIO_BG_STATE_ON_AIR		    = 0,
    eRADIO_BG_STATE_START		        = 1,
    eRADIO_BG_STATE_SET_FREQ		    = 2,
    eRADIO_BG_STATE_PRECHK_QDATA	    = 3,
    eRADIO_BG_STATE_CHK_QDATA_STATUS	= 4,
    eRADIO_BG_STATE_CHK_QDATA 	    = 5,
    eRADIO_BG_STATE_STOP			    = 6,
    eRADIO_BG_STATE_WAIT			    = 7,
	eRADIO_BG_STATE_END
}eRADIO_BG_STATE_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct{
	int32 fBGThreadRunning;
	int32 fBGRadioOpened;
	eRADIO_BG_EVENT_t eMainMode;
	eRADIO_BG_STATE_t eBGState;
	eRADIO_SEEK_MODE_t eSeekMode;
	uint32 curBGResult;

	uint32 curBand;		// eRADIO_MOD_MODE_t
	uint32 curFreq;
	uint32 curStartFreq;

#if 0
	uint32 currentBandPresetFreq[eRADIO_DAB_MODE+1][_MaxPresetNum];
	uint32 currentBandFreq[eRADIO_DAB_MODE+1];	// 각 Band에 따른 Frequency
#endif

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

	stRADIO_QUALITY_t stSchQdata;
//	uint32 scanPI;		// DAB Seamless Link
}stRADIO_BG_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern stRADIO_BG_t stBGService;
//extern stRADIO_CONFIG_t stTunerConfig;
//extern uint32 scanPiList[128];

/***************************************************
*			Function declaration				*
****************************************************/
extern RET tcradiobg_init(void);
extern RET tcradiobg_close(void);
extern RET tcradiobg_checkValidFreq(eRADIO_MOD_MODE_t mod_mode, uint32 freq);

#if 0
extern uint32 tcradiobg_getCurrentFrequency(void);
extern RET tcradiobg_setBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 start_freq, uint32	end_freq, uint32 step);
extern RET tcradiobg_getBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 *start_freq, uint32 *end_freq, uint32 *step);
#endif

#ifdef __cplusplus
}
#endif

#endif
