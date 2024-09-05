/*******************************************************************************

*   FileName : dev_blend_audio_i2s.h

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device blend audio i2s header

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
#ifndef DEV_BLEND_AUDIO_I2S_H__
#define DEV_BLEND_AUDIO_I2S_H__

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

//#define BAI2S_DEBUG  enable this code to print debugging messages

#ifdef __ANDROID__

#define BAI2S_TAG			("[RADIO][BAI2S]")
#define BAI2S_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,BAI2S_TAG, __VA_ARGS__))
#ifdef BAI2S_DEBUG
#define BAI2S_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,BAI2S_TAG, __VA_ARGS__))
#else
#define	BAI2S_DBG(...)
#endif

#else /* #ifdef __ANDROID__ */

#define BAI2S_ERR(...)		((void)LB_PRINTF("[ERROR][RADIO][BAI2S]: " __VA_ARGS__))
#ifdef BAI2S_DEBUG
#define BAI2S_DBG(...)		((void)LB_PRINTF("[DEBUG][RADIO][BAI2S]: " __VA_ARGS__))
#else
#define	BAI2S_DBG(...)
#endif
// #define BAI2S_ERR(...)		((void)printf("[ERROR][RADIO][BAI2S]: " __VA_ARGS__))
// #ifdef BAI2S_DEBUG
// #define BAI2S_DBG(...)		((void)printf("[DEBUG][RADIO][BAI2S]: " __VA_ARGS__))
// #else
// #define	BAI2S_DBG(...)
// #endif

#endif /* #ifdef __ANDROID__ */

#define IOCTL_BLENDI2S_MAGIC       'S'
#define BLENDI2S_SET_PARAMS        _IO( IOCTL_BLENDI2S_MAGIC, 0)
#define BLENDI2S_TX_START          _IO( IOCTL_BLENDI2S_MAGIC, 1)
#define BLENDI2S_TX_STOP           _IO( IOCTL_BLENDI2S_MAGIC, 2)
#define BLENDI2S_RX_START          _IO( IOCTL_BLENDI2S_MAGIC, 3)
#define BLENDI2S_RX_STOP           _IO( IOCTL_BLENDI2S_MAGIC, 4)
#define BLENDI2S_AUDIO_MODE_RX_DAI _IO( IOCTL_BLENDI2S_MAGIC, 5)
#define BLENDI2S_GET_VALID_BYTES   _IO( IOCTL_BLENDI2S_MAGIC, 7)

#define MAX_BUFFER_SIZE (1024*1024*2)	//Bytes
#define MIN_BUFFER_SIZE (1024)			//Bytes
#define NUM_OF_AUDIO_CH 2

#define	MIN_BLEND_SAMPLERATE		(8000)		// Hz
#define	MAX_BLEND_SAMPLERATE		(192000)	// Hz

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	BLEND_AUDIO_I2S_BIT_POLARITY_POSITIVE_EDGE = 0,
	BLEND_AUDIO_I2S_BIT_POLARITY_NEGATIVE_EDGE = 1
} BLEND_AUDIO_I2S_POLARITY;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct BLEND_AUDIO_I2S_PARAM_ {
	uint32 eSampleRate;
	uint32 eRadioMode;
	uint32 eBitMode;
	uint32 eBitPolarity;
	uint32 eBufferSize;	// Min.1024byte Max.2Mbyte (2^n)
	uint32 eChannel;
	uint32 ePeriodSize;	// Radio: Min.512byte Max. 256Kbyte (2^n & multiples of 64), Audio: Min.256byte Max.256Kbyte (2^n & multiples of 32)
	uint32 Reserved3[3];
} BLEND_AUDIO_I2S_PARAM;

typedef struct BLEND_AUDIO_RX_PARAM_ {
    int8 *eBuf;
    uint32 eReadCount;
    int32 eIndex;
} BLEND_AUDIO_RX_PARAM;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern int32 dev_blend_audio_i2s_open(void);
extern int32 dev_blend_audio_i2s_close(void);
extern int32 dev_blend_audio_i2s_stop(void);
extern int32 dev_blend_audio_i2s_startWithParams(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize);
extern int32 dev_blend_audio_i2s_setParameters(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize);
extern int32 dev_blend_audio_i2s_start(void);
extern int32 dev_blend_audio_i2s_read(int8 *data, int32 readsize);
extern int32 dev_blend_audio_i2s_get_valid(uint32 *valid);

#ifdef __cplusplus
}
#endif

#endif
