/*******************************************************************************

*   FileName : dev_iq_i2s.h

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device IQ I2S header

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
#ifndef DEV_IQ_I2S_H__
#define DEV_IQ_I2S_H__

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

//#define IQI2S_DEBUG   <- enable this code to print debugging messages

#ifdef __ANDROID__

#define IQI2S_TAG			("[RADIO][IQI2S]")
#define IQI2S_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,IQI2S_TAG, __VA_ARGS__))
#ifdef IQI2S_DEBUG
#define IQI2S_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,IQI2S_TAG, __VA_ARGS__))
#else
#define	IQI2S_DBG(...)
#endif

#else /* #ifdef __ANDROID__ */

#define IQI2S_ERR(...)		((void)LB_PRINTF("[ERROR][RADIO][IQI2S]: " __VA_ARGS__))
#ifdef IQI2S_DEBUG
#define IQI2S_DBG(...)		((void)LB_PRINTF("[DEBUG][RADIO][IQI2S]: " __VA_ARGS__))
#else
#define	IQI2S_DBG(...)
#endif
// #define IQI2S_ERR(...)		((void)printf("[ERROR][RADIO][IQI2S]: " __VA_ARGS__))
// #ifdef IQI2S_DEBUG
// #define IQI2S_DBG(...)		((void)printf("[DEBUG][RADIO][IQI2S]: " __VA_ARGS__))
// #else
// #define	IQI2S_DBG(...)
// #endif

#endif /* #ifdef __ANDROID__ */

#define IOCTL_IQI2S_MAGIC       'S'
#define IQI2S_SET_PARAMS        _IO( IOCTL_IQI2S_MAGIC, 0)
#define IQI2S_TX_START          _IO( IOCTL_IQI2S_MAGIC, 1)
#define IQI2S_TX_STOP           _IO( IOCTL_IQI2S_MAGIC, 2)
#define IQI2S_RX_START          _IO( IOCTL_IQI2S_MAGIC, 3)
#define IQI2S_RX_STOP           _IO( IOCTL_IQI2S_MAGIC, 4)
#define IQI2S_RADIO_MODE_RX_DAI _IO( IOCTL_IQI2S_MAGIC, 5)
#define IQI2S_GET_VALID_BYTES   _IO( IOCTL_IQI2S_MAGIC, 7)

#define MAX_BUFFER_SIZE (1024*1024*2)	//Bytes
#define MIN_BUFFER_SIZE (1024)			//Bytes
#define CH_MAX 4

#define	MIN_IQ_SAMPLERATE		(96000)		// Hz
#define	MAX_IQ_SAMPLERATE		(1100000)	// Hz

/***************************************************
*				Enumeration				*
****************************************************/

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct IQ_I2S_PARAM_ {
   	uint32 eSampleRate;
	uint32 eRadioMode;
	uint32 eBitMode;
	uint32 eBitPolarity;
	uint32 eBufferSize;	// Min.1024byte Max.2Mbyte (2^n)
	uint32 eChannel;
	uint32 ePeriodSize;	// Radio: Min.512byte Max. 256Kbyte (2^n & multiples of 64), Audio: Min.256byte Max.256Kbyte (2^n & multiples of 32)
	uint32 Reserved3[3];
} IQ_I2S_PARAM;

typedef struct RADIO_IQ_RX_PARAM_ {
    int8 *eBuf;
    uint32 eReadCount;
    int32 eIndex;
} RADIO_IQ_RX_PARAM;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern int32 dev_iq01_i2s_open(void);
extern int32 dev_iq01_i2s_close(void);
extern int32 dev_iq01_i2s_stop(void);
extern int32 dev_iq01_i2s_startWithParams(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize);
extern int32 dev_iq01_i2s_setParameters(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize);
extern int32 dev_iq01_i2s_start(void);
extern int32 dev_iq01_i2s_read(int8 *data, int32 readsize);
extern int32 dev_iq01_i2s_read_ch(int8 *ch0, int8 *ch1, int8 *ch2, int8 *ch3, int32 readsize);
extern int32 dev_iq01_i2s_get_valid(uint32 *valid);

extern int32 dev_iq23_i2s_open(void);
extern int32 dev_iq23_i2s_close(void);
extern int32 dev_iq23_i2s_stop(void);
extern int32 dev_iq23_i2s_startWithParams(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize);
extern int32 dev_iq23_i2s_setParameters(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize);
extern int32 dev_iq23_i2s_start(void);
extern int32 dev_iq23_i2s_read(int8 *data, int32 readsize);
extern int32 dev_iq23_i2s_read_ch(int8 *ch0, int8 *ch1, int8 *ch2, int8 *ch3, int32 readsize);
extern int32 dev_iq23_i2s_get_valid(uint32 *valid);

#ifdef __cplusplus
}
#endif

#endif
