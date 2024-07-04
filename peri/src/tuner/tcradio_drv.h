/*******************************************************************************

*   FileName : tcradio_drv.h

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device tuner header

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
#ifndef TCRADIO_DRV_H_
#define TCRADIO_DRV_H_

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
typedef enum {
    eTUNER_DRV_ID_PRIMARY		= 0,
    eTUNER_DRV_ID_SECONDARY		= 1,
    eTUNER_DRV_ID_TERTIARY		= 2,
    eTUNER_DRV_ID_QUATERNARY	= 3
}eTUNER_DRV_ID_t;

typedef enum {
	eTUNER_DRV_CONF_TYPE_SINGLE	= 1,
	eTUNER_DRV_CONF_TYPE_DUAL	= 2,
	eTUNER_DRV_CONF_TYPE_TRIPLE	= 3,
	eTUNER_DRV_CONF_TYPE_QUAD	= 4
}eTUNER_DRV_CONF_TYPE_t;

typedef enum {
	eTUNER_DRV_CONF_AREA_ASIA	= 0,
	eTUNER_DRV_CONF_AREA_EU		= 1,
	eTUNER_DRV_CONF_AREA_NA		= 2,
	eTUNER_DRV_CONF_AREA_MAX
} eTUNER_DRV_CONF_AREA_t;

typedef enum {
	eTUNER_DRV_FM_MODE 		= 0,	// FM : Frequency Modulation Mode
	eTUNER_DRV_AM_MODE		= 1,	// AM : Amplitude Modulation Mode
	eTUNER_DRV_DAB_MODE		= 2		// DAB : Digital Audio Broadcasting Mode
} eTUNER_DRV_MOD_MODE_t;

typedef enum{
	eTUNER_DRV_TUNE_NORMAL	= 0,	// Gerneric Tune
	eTUNER_DRV_TUNE_FAST	= 1,	// For Silab Device Tune
	eTUNER_DRV_TUNE_AFTUNE	= 2,	// For Silab Device Tune
	eTUNER_DRV_TUNE_AFCHECK	= 3		// For Silab Device Tune
}eTUNER_DRV_TUNE_MODE_t;

typedef enum {
	eTUNER_SDR_NONE			= 0,	// None SDR
	eTUNER_SDR_HD			= 1,	// HD Radio
	eTUNER_SDR_DRM30		= 2,    // DRM30 (Digital Radio Mondiale 30)
	eTUNER_SDR_DRMP			= 3,	// DRM+ (Digital Radio Mondiale Plus)
	eTUNER_SDR_DAB			= 4     // DAB (Digital Audio Broadcasting)
}eTUNER_DRV_SDR_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct {
	eTUNER_DRV_CONF_AREA_t area;	// Regional coefficient setting of tuner driver. Not a regional frequency configuration.
	eTUNER_DRV_MOD_MODE_t initMode;	// init Modulation Mode
	uint32 initFreq;				// init Frequency
	uint32 numTuners;				// Number of Tuners			// 1 ~ 4 (According to tuner H/W configuration)
	uint32 fPhaseDiversity;			// Phase Diversity Enable	// 0 : disable, 1 : enable (not used yet)
	uint32 fIqOut;					// IQ Out Enable			// 0 : disable, 1 : enable
	uint32 audioSamplerate; 		// Audio Samplerate			// 44100 : 44.1Khz, 48000 : 48Khz
	uint32 fExtAppCtrl;				// Control by external APP	// 0 : By radio APP, 1 : By external APP

	// For SDR
	eTUNER_DRV_SDR_t sdr;			// Software defined radio standards
	uint32 reserved[3];
}stTUNER_DRV_CONFIG_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/

#ifdef __cplusplus
}
#endif

#endif
