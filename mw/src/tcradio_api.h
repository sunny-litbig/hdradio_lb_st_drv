/*******************************************************************************

*   FileName : tcradio_api.h

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
#ifndef __TCRADIO_API_H__
#define __TCRADIO_API_H__

/***************************************************
*				Include					*
****************************************************/
#include "tcradio_types.h"
#include "tcradio_peri_config.h"
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

#define RAPP_DEBUG

#ifdef __ANDROID__

#define RAPP_TAG			("[RADIO][APP]")
#define RAPP_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,RAPP_TAG, __VA_ARGS__))
#ifdef RAPP_DEBUG
#define RAPP_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,RAPP_TAG, __VA_ARGS__))
#else
#define	RAPP_DBG(...)
#endif

#else // #ifdef __ANDROID__

// #define RAPP_ERR(...)		((void)printf("[ERROR][RADIO][APP]: " __VA_ARGS__))
#define RAPP_ERR(...)		((void)LB_PRINTF("[ERROR][RADIO][APP]: " __VA_ARGS__))
#ifdef RAPP_DEBUG
// #define RAPP_DBG(...)		((void)printf("[DEBUG][RADIO][APP]: " __VA_ARGS__))
#define RAPP_DBG(...)		((void)LB_PRINTF("[DEBUG][RADIO][APP]: " __VA_ARGS__))
#else
#define	RAPP_DBG(...)
#endif

#endif // #ifdef __ANDROID__

#define BS_malloc(X) 	malloc((size_t)(X))
#define BS_free(X)		free((void *)(X))
#define BS_mwait(X)		usleep((X)*(1000))

#define	RADIO_QVALUE_COUNT		20		// Do not change

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum
{
	eRADIO_NOTIFY_NULL			= 0,
	eRADIO_NOTIFY_OPEN			= 1,
	eRADIO_NOTIFY_DEINIT		= 4,
    eRADIO_NOTIFY_SEEK_MODE		= 8,
	eRADIO_NOTIFY_TUNE			= 130,
	eRADIO_NOTIFY_DAB_FREQ_LIST	= 131,

	eRADIO_HD_NOTIFY_OPEN		= 201,
	eRADIO_HD_NOTIFY_AUDIO_MODE = 211,
	eRADIO_HD_NOTIFY_PROGRAM	= 213,

	eRADIO_HD_NOTIFY_STATUS		= 220,
	eRADIO_HD_NOTIFY_PSD		= 221,
	eRADIO_HD_NOTIFY_SIS		= 222,
	eRADIO_HD_NOTIFY_SIG		= 223,
	eRADIO_HD_NOTIFY_AAS		= 224,
	eRADIO_HD_NOTIFY_ALERT		= 225,
	eRADIO_HD_NOTIFY_LOT		= 226,

	eRADIO_HD_NOTIFY_SIGNAL_STATUS	= 290,
	eRADIO_HD_NOTIFY_PTY 			= 291,

}eRADIO_NOTIFY_t;

typedef enum {
	eRADIO_TYPE0	= 0,
	eRADIO_TYPE1	= 1,
	eRADIO_TYPE2	= 2,
	eRADIO_TYPE3	= 3
}eRADIO_TYPE_t;

typedef enum {
    eRADIO_ID_PRIMARY		= 0,
    eRADIO_ID_SECONDARY		= 1,
    eRADIO_ID_TERTIARY		= 2,
    eRADIO_ID_QUATERNARY	= 3
}eRADIO_ID_t;

typedef enum {
	eRADIO_CONF_TYPE_SINGLE	= 1,
	eRADIO_CONF_TYPE_DUAL	= 2,
	eRADIO_CONF_TYPE_TRIPLE	= 3,
	eRADIO_CONF_TYPE_QUAD	= 4
}eRADIO_CONF_TYPE_t;

typedef enum {
	eRADIO_CONF_AREA_ASIA	= 0,
	eRADIO_CONF_AREA_EU		= 1,
	eRADIO_CONF_AREA_NA		= 2
} eRADIO_CONF_AREA_t;

typedef enum {
	eRADIO_FM_MODE 	= 0,			// FM : Frequency Modulation Mode
	eRADIO_AM_MODE	= 1,			// AM : Amplitude Modulation Mode
	eRADIO_DAB_MODE = 2				// DAB: Digital Audio Broadcasting
} eRADIO_MOD_MODE_t;

typedef enum{
	eRADIO_TUNE_NORMAL	= 0,
	eRADIO_TUNE_FAST	= 1,
	eRADIO_TUNE_AFTUNE	= 2,
	eRADIO_TUNE_AFCHECK	= 3
}eRADIO_TUNE_MODE_t;

typedef enum{
	eRADIO_SEEK_STOP			= 0,	// Seek Stop
	eRADIO_SEEK_MAN_UP			= 1,	// Manual Up
	eRADIO_SEEK_MAN_DOWN		= 2,	// Manual Down
	eRADIO_SEEK_AUTO_UP			= 3,	// Scan the receivable station (upwards)
	eRADIO_SEEK_AUTO_DOWN		= 4,	// Scan the receivable station (downwards)
	eRADIO_SEEK_SCAN_STATION	= 5,	// Creat the receivable station list in the current waveband (max.100)
	eRADIO_SEEK_SCAN_PI			= 6,	// Scan for dab seamless linking
	eRADIO_SEEK_END
}eRADIO_SEEK_MODE_t;

typedef enum {
	eRADIO_SDR_NONE				= 0,	// None SDR
	eRADIO_SDR_HD				= 1,	// HD Radio
	eRADIO_SDR_DRM30			= 2,    // DRM30 (Digital Radio Mondiale 30)
	eRADIO_SDR_DRMP				= 3,    // DRM+ (Digital Radio Mondiale Plus)
	eRADIO_SDR_DAB				= 4,    // DAB (Digital Audio Broadcasting)
}eRADIO_SDR_STANDARD_t;

typedef enum {
	eRADIO_HD_TYPE_HD1p0		= 0,	// 1 Tuner
	eRADIO_HD_TYPE_HD1p5		= 1,	// 2 Tuner
	eRADIO_HD_TYPE_HD1p0_MRC	= 2,	// 2 Tuner
	eRADIO_HD_TYPE_HD1p5_MRC	= 3,	// 3 Tuner
	eRADIO_HD_TYPE_HD1p5_DUAL_MRC = 4	// 4 Tuner
}eRADIO_HD_TYPE_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct {				// This enum have to same with stTUNER_DRV_CONFIG_t.
	eRADIO_CONF_AREA_t area;	// Regional coefficient setting of tuner driver. Not a regional frequency configuration.
	eRADIO_MOD_MODE_t initMode;	// Init Modulataion Mode
	uint32 initFreq;			// Init Frequency
	uint32 numTuners;			// Number of Tuners 		// 1 ~ 4 (According to tuner H/W configuration)
	uint32 fPhaseDiversity;		// Phase Diversity Enable	// 0 : disable, 1 : enable (not used yet)
	uint32 fIqOut;				// IQ Out Enable			// 0 : disable, 1 : enable
	uint32 audioSamplerate; 	// Audio Samplerate			// 44100 : 44.1Khz, 48000 : 48Khz
	uint32 fExtAppCtrl;			// Control by external APP	// 0 : By radio APP, 1 : By external APP

	// For SDR
	eRADIO_SDR_STANDARD_t sdr;	// Software defined radio standards
	eRADIO_HD_TYPE_t hdType;	// This value is available only if sdr is HD Radio. An user has to set it to match numTuners.
	uint32 reserved;
}stRADIO_CONFIG_t;

typedef union {
	struct {
		uint32 Qvalue[RADIO_QVALUE_COUNT];
	}fm;

	struct {
		uint32 Qvalue[RADIO_QVALUE_COUNT];
	}am;

	struct {
		uint32 Qvalue[RADIO_QVALUE_COUNT];
	}dab;
}stRADIO_QVALUE_t;

typedef struct {
	uint32 type;
	stRADIO_QVALUE_t qual;
}stRADIO_QUALITY_t;

typedef union {
	struct {
		uint32 Rssi;		// FM Received signal strength
		uint32 Mpth;		// FM Multipath
		uint32 Hfn;			// FM High Frequency noise
		uint32 Mod;			// FM Modulation
		uint32 Pilot;		// FM Pilot
		uint32 Offs; 		// FM Offset
	} fm;

	struct {
		uint32 Rssi;		// AM Received signal strength
		uint32 Mod;			// AM Modulation
		uint32 Offs;		// AM Offset
	} am;
}stRADIO_TYPE0_QUALITY_t;

typedef union {
	struct {
		int32 Rssi;			// FM Received signal strength
		int32 Snr;			// FM RF carrier to noise
		uint32 Dev;			// FM Deviation
		int32 Offs;			// FM Offset
		uint32 Pilot;		// FM Pilot
		uint32 Mpth; 		// FM Multipath
		uint32 Usn;			// FM Ultra-sonic noise.
	} fm;

	struct {
		int32 Rssi;			// AM Received signal strength
		int32 Snr;			// AM RF carrier to noise
		uint32 Mod;			// AM Modulation
		int32 Offs;			// AM Offset
	} am;

	struct {
		int32 Rssi;
		int32 Sqi;
		uint32 Detect;
		int32 RssiAdj;
		int32 Dagc;
		uint32 Valid;
	} dab;
}stRADIO_TYPE1_QUALITY_t;

typedef union {
	struct {
		uint32 Status;
		int32 Rssi;			// Level detector result
		uint32 Usn;			// Ultra-sonic noise.
		uint32 Mpth; 		// Multipath
		int32 Offs;			// Offset
		uint32 Bwth;		// Bandwidth
		uint32 Mod;			// Modulation
	} fm;

	struct {
		uint32 Status;
		int32 Rssi;			// Level detector result
		uint32 Hfn;			// High frequency noise detector
		uint32 Coch;		// Co-Channel detector
		int32 Offs;			// Offset
		uint32 Bwth;		// Bandwidth
		uint32 Mod;			// Modulation

	} am;
}stRADIO_TYPE2_QUALITY_t;

typedef union {
	struct {
		int32 FstRF;		// Field strength before AGC (8bit int signed [dBuV])
		int32 FstBB;		// Field strength after AGC (8bit int signed [dBuV])
		uint32 Det;			// Detuning (8bit int unsigned)
		uint32 Snr;			// FM RF carrier to noise (8bit int unsigned [%])
		int32 Adj;			// FM Adjacent channel noise (8bit int signed [%])
		uint32 Dev;			// FM Deviation (7bit int unsigned)
		uint32 Mpth;		// FM Multipath (8bit int unsigned [%])
		uint32 MpxNoise;	// FM Mpx Noise (8bit int unsigned [%])
		uint32 Stereo;		// FM Stereo/Mono (1: Stereo, 0: Mono)
		uint32 CoCh;		// FM VPA Co-channel (8bit int unsigned [%], only for foreground channel)
	} fm;

	struct {
		int32 FstRF;		// Field strength before AGC (8bit int signed [dBuV])
		int32 FstBB;		// Field strength after AGC (8bit int signed [dBuV])
		uint32 Det;			// Detuning (8bit int unsigned)
		uint32 Snr;			// FM RF carrier to noise (8bit int unsigned [%])
		int32 Adj;			// FM Adjacent channel noise (8bit int signed [%])
		uint32 Dev;			// FM Deviation (7bit int unsigned)
	} am;

	struct {
		int32 FstRF;		// Field strength before AGC (8bit int signed [dBm])
		int32 FstBB;		// Field strength after AGC (8bit int signed [dBm])
	} dab;

	struct {
		int32 FstRF;		// Field strength before AGC (8bit int signed [dBuV])
		int32 FstBB;		// Field strength after AGC (8bit int signed [dBuV])
	} wx;
}stRADIO_TYPE3_QUALITY_t;

typedef struct {
	uint32 uiFreq;
	stRADIO_QUALITY_t stQdata;
}stRADIO_LIST_t;

typedef void(*pfnOnGetNotificationCallBack_t)(uint32 notifyID, uint32 *arg, void **pData, int32 errorCode);
typedef void(*pfnOnGetStationListCallBack_t)(uint32 totalnum, void *list, int32 errorCode);
typedef void(*pfnOnGetRdsDataCallBack_t)(void *rdsData, int32 errorCode);

typedef int32(*pfnOnPrecheckSeekQual_t)(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata, uint32 freq, uint32 ntuner);	// mod_mode : 0 = FM, 1 = AM	// return value  0 = OK, -1 = NG
typedef int32(*pfnOnCheckSeekQual_t)(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata, uint32 ntuner);		// mod_mode : 0 = FM, 1 = AM	// return value  0 = OK, -1 = NG

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern pfnOnGetNotificationCallBack_t pfnOnGetNotificationCallBack;
extern pfnOnGetStationListCallBack_t pfnOnGetStationListCallBack;
extern pfnOnGetRdsDataCallBack_t pfnOnGetRdsDataCallBack;

extern pfnOnPrecheckSeekQual_t pfnOnPrecheckSeekQual;
extern pfnOnCheckSeekQual_t pfnOnCheckSeekQual;

extern int aout1st_flag;
/***************************************************
*			Function declaration				*
****************************************************/
extern void tcradio_configOnGetNotificationCallBack(void(*pfnGetNotificationCallBack)(uint32 notifyID, uint32 *arg, void **pData, int32 errorCode));
extern void tcradio_configOnGetStationListCallBack(void(*pfnGetStationListCallBack)(uint32 totalnum, void *list, int32 errorCode));
extern void tcradio_configOnGetRdsDataCallBack(void(*pfnGetRdsDataCallBack)(void *rdsData, int32 errorCode));

extern void tcradio_configOnPrecheckSeekQual(int32(*pfnPrecheckSeekQual)(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata, uint32 freq, uint32 ntuner));
extern void tcradio_configOnCheckSeekQual(int32(*pfnCheckSeekQual)(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata, uint32 ntuner));

extern RET tcradio_init(void);
extern RET tcradio_deinit(void);
extern RET tcradio_open(stRADIO_CONFIG_t *config);
extern RET tcradio_close(void);
extern RET tcradio_getOpenStatus(void);

extern RET tcradio_setBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 start_freq, uint32  end_freq, uint32 step);
extern RET tcradio_getMaxDabFreqList(uint8 *maxNums);
extern RET tcradio_getCurrentDabFreqList(uint8 *curNums, uint32 ntuner);
extern RET tcradio_setDabFreqList(uint32 *dab_freq_hz_table, uint8 num_freq, uint32 ntuner);
extern RET tcradio_getDabFreqList(uint32 *dab_freq_hz_table, uint8 *num_freq, uint32 ntuner);
extern RET tcradio_setTune(eRADIO_MOD_MODE_t mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner);
extern RET tcradio_setSeek(eRADIO_SEEK_MODE_t seekcmd, uint32 *data);
extern RET tcradio_bgStart(eRADIO_MOD_MODE_t mod_mode);
extern RET tcradio_bgStop(void);
extern RET tcradio_setTunerCommand(uint8 *txdata, uint32 txlen, uint8 *rxdata, uint32 rxlen, uint32 ntuner);
extern RET tcradio_getTunerStatus(uint32 cmd, uint8 *buf, uint32 len, uint32 ntuner);
extern RET tcradio_setAudio(uint32 play);
extern RET tcradio_setAudioDevice(stRADIO_CONFIG_t *config, uint32 OnOff);


extern RET tcradio_getTune(eRADIO_MOD_MODE_t *mod_mode, uint32 *freq, uint32 ntuner);
extern RET tcradio_getQuality(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t * qdata, uint32 ntuner);
extern RET tcradio_getBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 *start_freq, uint32 *end_freq, uint32 *step);

extern RET tcradio_spi_writeread(uint8 *tx, uint8 *rx, uint32 len, uint32 cs);
extern RET tcradio_i2c_write(uint8 addr, uint8 *tx, uint32 len);
extern RET tcradio_i2c_read(uint8 addr, uint8 *reg, uint32 reglen, uint8* rx, uint32 rxlen);
extern uint32 tcradio_getMiddlewareVersion(void);
extern uint32 tcradio_getHalVersion(void);
extern uint32 tcradio_getDriverVersion(void);

extern RET tcradio_setRdsEnable(uint32 onoff);
extern int32 tcradio_getRdsEnable(void);
extern RET tcradio_getRdsPi(uint16 *pi);
extern RET tcradio_getRdsPty(uint8 *pty);
extern RET tcradio_getRdsPsn(uint8 *psn);
extern RET tcradio_getRdsRT(uint8 *rt);


/***************************************************
*					For Test					*
****************************************************/
extern RET tcradio_dumpIQ(uint32 nbit, uint32 bufsize_kbyte, uint32 readsize_byte, uint32 fbin, uint32 dumpsize_mbyte);
extern int32 tcradio_getTunerChip(void);
extern RET tcradio_setIQTestPattern(uint32 fOnOff, uint32 sel);

#ifdef __cplusplus
}
#endif

#endif
