/*******************************************************************************

*   FileName : tcradio_peri_config.h

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device configuration header

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
#ifndef TCRADIO_PERI_CONFIG_H__
#define TCRADIO_PERI_CONFIG_H__

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
typedef int32 (*pfnI2cOpen_t)(void);
typedef int32 (*pfnI2cClose_t)(void);
typedef int32 (*pfnI2cTx_t)(uint8 i2c_addr, uint8 *data, uint32 datalen);
typedef int32 (*pfnI2cRx_t)(uint8 i2c_addr, uint8 *reg, uint32 reglen, uint8 *data, uint32 datalen);

typedef int32 (*pfnSpiOpen_t)(uint8 ch);
typedef int32 (*pfnSpiClose_t)(uint8 ch);
typedef int32 (*pfnSpiTxRx_t)(uint8 *pBufIn, uint8* pBufOut, uint32 size, uint8 ch);

typedef int32 (*pfnRadioGpioOpen_t)(void);
typedef int32 (*pfnRadioGpioClose_t)(void);
typedef int32 (*pfnTunerPower_t)(int32 onoff);
typedef int32 (*pfnAntPower_t)(int32 onoff);
typedef int32 (*pfnTunerReset_t)(int32 onoff);

typedef int32 (*pfnIQ01I2sOpen_t)(void);
typedef int32 (*pfnIQ01I2sClose_t)(void);
typedef int32 (*pfnIQ01I2sStop_t)(void);
typedef int32 (*pfnIQ01I2sStart_t)(void);
typedef int32 (*pfnIQ01I2sSetParams_t)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize);
typedef int32 (*pfnIQ01I2sStartWithParams_t)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize);
typedef int32 (*pfnIQ01I2sRead_t)(int8 *data, int32 readsize);
typedef int32 (*pfnIQ01I2sReadCh_t)(int8 *ch0, int8 *ch1, int8 *ch2, int8 *ch3, int32 readsize);

typedef int32 (*pfnIQ23I2sOpen_t)(void);
typedef int32 (*pfnIQ23I2sClose_t)(void);
typedef int32 (*pfnIQ23I2sStop_t)(void);
typedef int32 (*pfnIQ23I2sStart_t)(void);
typedef int32 (*pfnIQ23I2sSetParams_t)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize);
typedef int32 (*pfnIQ23I2sStartWithParams_t)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize);
typedef int32 (*pfnIQ23I2sRead_t)(int8 *data, int32 readsize);
typedef int32 (*pfnIQ23I2sReadCh_t)(int8 *ch0, int8 *ch1, int8 *ch2, int8 *ch3, int32 readsize);

typedef int32 (*pfnBlendAudioI2sOpen_t)(void);
typedef int32 (*pfnBlendAudioI2sClose_t)(void);
typedef int32 (*pfnBlendAudioI2sStop_t)(void);
typedef int32 (*pfnBlendAudioI2sStart_t)(void);
typedef int32 (*pfnBlendAudioI2sSetParams_t)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize);
typedef int32 (*pfnBlendAudioI2sStartWithParams_t)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize);
typedef int32 (*pfnBlendAudioI2sRead_t)(int8 *data, int32 readsize);

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern pfnI2cOpen_t pfnI2cOpen;
extern pfnI2cClose_t pfnI2cClose;
extern pfnI2cTx_t pfnI2cTx;
extern pfnI2cRx_t pfnI2cRx;

extern pfnSpiOpen_t pfnSpiOpen;
extern pfnSpiClose_t pfnSpiClose;
extern pfnSpiTxRx_t pfnSpiTxRx;

extern pfnRadioGpioOpen_t pfnRadioGpioOpen;
extern pfnRadioGpioClose_t pfnRadioGpioClose;
extern pfnTunerPower_t pfnTunerPower;
extern pfnAntPower_t pfnAntPower;
extern pfnTunerReset_t pfnTunerReset;

extern pfnIQ01I2sOpen_t pfnIQ01I2sOpen;
extern pfnIQ01I2sClose_t pfnIQ01I2sClose;
extern pfnIQ01I2sStop_t pfnIQ01I2sStop;
extern pfnIQ01I2sStartWithParams_t pfnIQ01I2sStartWithParams;
extern pfnIQ01I2sStart_t pfnIQ01I2sStart;
extern pfnIQ01I2sSetParams_t pfnIQ01I2sSetParams;
extern pfnIQ01I2sRead_t pfnIQ01I2sRead;
extern pfnIQ01I2sReadCh_t pfnIQ01I2sReadCh;

extern pfnIQ23I2sOpen_t pfnIQ23I2sOpen;
extern pfnIQ23I2sClose_t pfnIQ23I2sClose;
extern pfnIQ23I2sStop_t pfnIQ23I2sStop;
extern pfnIQ23I2sStartWithParams_t pfnIQ23I2sStartWithParams;
extern pfnIQ23I2sStart_t pfnIQ23I2sStart;
extern pfnIQ23I2sSetParams_t pfnIQ23I2sSetParams;
extern pfnIQ23I2sRead_t pfnIQ23I2sRead;
extern pfnIQ23I2sReadCh_t pfnIQ23I2sReadCh;

extern pfnBlendAudioI2sOpen_t pfnBlendAudioI2sOpen;
extern pfnBlendAudioI2sClose_t pfnBlendAudioI2sClose;
extern pfnBlendAudioI2sStop_t pfnBlendAudioI2sStop;
extern pfnBlendAudioI2sStartWithParams_t pfnBlendAudioI2sStartWithParams;
extern pfnBlendAudioI2sStart_t pfnBlendAudioI2sStart;
extern pfnBlendAudioI2sSetParams_t pfnBlendAudioI2sSetParams;
extern pfnBlendAudioI2sRead_t pfnBlendAudioI2sRead;

/***************************************************
*			Function declaration				*
****************************************************/
extern void tcradio_configTunerI2cDriver(int32(*pfnOpen)(void), int32(*pfnClose)(void), int32(*pfnTx)(uint8 i2c_addr, uint8 *data, uint32 datalen),
											int32(*pfnRx)(uint8 i2c_addr, uint8 *reg, uint32 reglen, uint8 *data, uint32 datalen));
extern void tcradio_configTunerSpiDriver(int32(*pfnOpen)(uint8 ch), int32(*pfnClose)(uint8 ch),
											int32(*pfnTxRx)(uint8 *pBufIn, uint8* pBufOut, uint32 size, uint8 ch));
extern void tcradio_configTunerGpioDriver(int32(*pfnGpioOpen)(void), int32(*pfnGpioClose)(void), int32(*pfnTunerPwr)(int32 onoff), int32(*pfnAntPwr)(int32 onoff), int32(*pfnTunerRst)(int32 onoff));
extern void tcradio_configTunerIQ01I2sDriver(int32(*pfnIQ01DrvOpen)(void), int32(*pfnIQ01DrvClose)(void), int32(*pfnIQ01DrvStop)(void),
												int32(*pfnIQ01DrvStart)(void), int32(*pfnIQ01DrvSetParams)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize), int32(*pfnIQ01DrvRead)(int8 *data, int32 readsize),
												int32(*pfnIQ01DrvReadCh)(int8 *ch0, int8 *ch1, int8 *ch2, int8 *ch3, int32 readsize), int32(*pfnIQ01DrvStartWithParams)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize));
extern void tcradio_configTunerIQ23I2sDriver(int32(*pfnIQ23DrvOpen)(void), int32(*pfnIQ23DrvClose)(void), int32(*pfnIQ23DrvStop)(void),
												int32(*pfnIQ23DrvStart)(void), int32(*pfnIQ23DrvSetParams)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize), int32(*pfnIQ23DrvRead)(int8 *data, int32 readsize),
												int32(*pfnIQ23DrvReadCh)(int8 *ch0, int8 *ch1, int8 *ch2, int8 *ch3, int32 readsize), int32(*pfnIQ23DrvStartWithParams)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize));
extern void tcradio_configTunerBlendAudioI2sDriver(int32(*pfnAudioDrvOpen)(void), int32(*pfnAudioDrvClose)(void), int32(*pfnAudioDrvStop)(void),
												int32(*pfnAudioDrvStart)(void), int32(*pfnAudioDrvSetParams)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize), int32(*pfnAudioDrvRead)(int8 *data, int32 readsize),
												int32(*pfnAudioDrvStartWithParams)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize));
extern void tcradio_configTunerPeriDrivers(void);
extern void tcradio_releaseTunerPeriDrivers(void);

#ifdef __cplusplus
}
#endif

#endif	// ifndef TCRADIO_PERI_CONFIG_H__
