/*******************************************************************************

*   FileName : tcradio_peri_config.c

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device configuration functions and definitions

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
#include "tcradio_types.h"
#include "tcradio_peri_config.h"
#include "dev_gpio.h"
#include "dev_i2c.h"
#include "dev_iq_i2s.h"
#include "dev_spi.h"
#include "dev_blend_audio_i2s.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
pfnI2cOpen_t pfnI2cOpen;
pfnI2cClose_t pfnI2cClose;
pfnI2cTx_t pfnI2cTx;
pfnI2cRx_t pfnI2cRx;

pfnSpiOpen_t pfnSpiOpen;
pfnSpiClose_t pfnSpiClose;
pfnSpiTxRx_t pfnSpiTxRx;

pfnRadioGpioOpen_t pfnRadioGpioOpen;
pfnRadioGpioClose_t pfnRadioGpioClose;
pfnTunerPower_t pfnTunerPower;
pfnAntPower_t pfnAntPower;
pfnTunerReset_t	pfnTunerReset;

pfnIQ01I2sOpen_t pfnIQ01I2sOpen;
pfnIQ01I2sClose_t pfnIQ01I2sClose;
pfnIQ01I2sStop_t pfnIQ01I2sStop;
pfnIQ01I2sStartWithParams_t pfnIQ01I2sStartWithParams;
pfnIQ01I2sStart_t pfnIQ01I2sStart;
pfnIQ01I2sSetParams_t pfnIQ01I2sSetParams;
pfnIQ01I2sRead_t pfnIQ01I2sRead;
pfnIQ01I2sReadCh_t pfnIQ01I2sReadCh;

pfnIQ23I2sOpen_t pfnIQ23I2sOpen;
pfnIQ23I2sClose_t pfnIQ23I2sClose;
pfnIQ23I2sStop_t pfnIQ23I2sStop;
pfnIQ23I2sStartWithParams_t pfnIQ23I2sStartWithParams;
pfnIQ23I2sStart_t pfnIQ23I2sStart;
pfnIQ23I2sSetParams_t pfnIQ23I2sSetParams;
pfnIQ23I2sRead_t pfnIQ23I2sRead;
pfnIQ23I2sReadCh_t pfnIQ23I2sReadCh;

pfnBlendAudioI2sOpen_t pfnBlendAudioI2sOpen;
pfnBlendAudioI2sClose_t pfnBlendAudioI2sClose;
pfnBlendAudioI2sStop_t pfnBlendAudioI2sStop;
pfnBlendAudioI2sStartWithParams_t pfnBlendAudioI2sStartWithParams;
pfnBlendAudioI2sStart_t pfnBlendAudioI2sStart;
pfnBlendAudioI2sSetParams_t pfnBlendAudioI2sSetParams;
pfnBlendAudioI2sRead_t pfnBlendAudioI2sRead;

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
*           Local constant definitions              *
****************************************************/

/***************************************************
*          Local type definitions                  *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition				*
****************************************************/
void tcradio_configTunerI2cDriver(int32(*pfnOpen)(void), int32(*pfnClose)(void), int32(*pfnTx)(uint8 i2c_addr, uint8 *data, uint32 datalen),
											int32(*pfnRx)(uint8 i2c_addr, uint8 *reg, uint32 reglen, uint8 *data, uint32 datalen))
{
	pfnI2cOpen = pfnOpen;
	pfnI2cClose = pfnClose;
	pfnI2cTx = pfnTx;
	pfnI2cRx = pfnRx;
}

void tcradio_configTunerSpiDriver(int32(*pfnOpen)(uint8 ch), int32(*pfnClose)(uint8 ch),
											int32(*pfnTxRx)(uint8 *pBufIn, uint8* pBufOut, uint32 size, uint8 ch))
{
	pfnSpiOpen = pfnOpen;
	pfnSpiClose = pfnClose;
	pfnSpiTxRx = pfnTxRx;
}

void tcradio_configTunerGpioDriver(int32(*pfnGpioOpen)(void), int32(*pfnGpioClose)(void), int32(*pfnTunerPwr)(int32 onoff), int32(*pfnAntPwr)(int32 onoff), int32(*pfnTunerRst)(int32 onoff))
{
	pfnRadioGpioOpen = pfnGpioOpen;
	pfnRadioGpioClose = pfnGpioClose;
	pfnTunerPower = pfnTunerPwr;
	pfnAntPower = pfnAntPwr;
	pfnTunerReset = pfnTunerRst;
}

void tcradio_configTunerIQ01I2sDriver(int32(*pfnIQ01DrvOpen)(void), int32(*pfnIQ01DrvClose)(void), int32(*pfnIQ01DrvStop)(void),
												int32(*pfnIQ01DrvStart)(void), int32(*pfnIQ01DrvSetParams)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize), int32(*pfnIQ01DrvRead)(int8 *data, int32 readsize),
												int32(*pfnIQ01DrvReadCh)(int8 *ch0, int8 *ch1, int8 *ch2, int8 *ch3, int32 readsize), int32(*pfnIQ01DrvStartWithParams)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize))
{
	pfnIQ01I2sOpen =pfnIQ01DrvOpen;
	pfnIQ01I2sClose =pfnIQ01DrvClose;
	pfnIQ01I2sStop =pfnIQ01DrvStop;
	pfnIQ01I2sStart =pfnIQ01DrvStart;
	pfnIQ01I2sSetParams =pfnIQ01DrvSetParams;
	pfnIQ01I2sRead =pfnIQ01DrvRead;
	pfnIQ01I2sReadCh =pfnIQ01DrvReadCh;
	pfnIQ01I2sStartWithParams =pfnIQ01DrvStartWithParams;
}

void tcradio_configTunerIQ23I2sDriver(int32(*pfnIQ23DrvOpen)(void), int32(*pfnIQ23DrvClose)(void), int32(*pfnIQ23DrvStop)(void),
												int32(*pfnIQ23DrvStart)(void), int32(*pfnIQ23DrvSetParams)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize), int32(*pfnIQ23DrvRead)(int8 *data, int32 readsize),
												int32(*pfnIQ23DrvReadCh)(int8 *ch0, int8 *ch1, int8 *ch2, int8 *ch3, int32 readsize), int32(*pfnIQ23DrvStartWithParams)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize))
{
	pfnIQ23I2sOpen =pfnIQ23DrvOpen;
	pfnIQ23I2sClose =pfnIQ23DrvClose;
	pfnIQ23I2sStop =pfnIQ23DrvStop;
	pfnIQ23I2sStart =pfnIQ23DrvStart;
	pfnIQ23I2sSetParams =pfnIQ23DrvSetParams;
	pfnIQ23I2sRead =pfnIQ23DrvRead;
	pfnIQ23I2sReadCh =pfnIQ23DrvReadCh;
	pfnIQ23I2sStartWithParams =pfnIQ23DrvStartWithParams;
}

void tcradio_configTunerBlendAudioI2sDriver(int32(*pfnAudioDrvOpen)(void), int32(*pfnAudioDrvClose)(void), int32(*pfnAudioDrvStop)(void),
												int32(*pfnAudioDrvStart)(void), int32(*pfnAudioDrvSetParams)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize), int32(*pfnAudioDrvRead)(int8 *data, int32 readsize),
												int32(*pfnAudioDrvStartWithParams)(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize))
{
	pfnBlendAudioI2sOpen =pfnAudioDrvOpen;
	pfnBlendAudioI2sClose =pfnAudioDrvClose;
	pfnBlendAudioI2sStop =pfnAudioDrvStop;
	pfnBlendAudioI2sStart =pfnAudioDrvStart;
	pfnBlendAudioI2sSetParams =pfnAudioDrvSetParams;
	pfnBlendAudioI2sRead =pfnAudioDrvRead;
	pfnBlendAudioI2sStartWithParams =pfnAudioDrvStartWithParams;
}

void tcradio_configTunerPeriDrivers(void)
{
	// You can set only the peri drivers you want.
	tcradio_configTunerI2cDriver(&dev_i2c_open, &dev_i2c_close, &write_i2c, &read_i2c);
	tcradio_configTunerSpiDriver(&dev_spi_open, &dev_spi_close, &dev_spi_txrx);
	tcradio_configTunerGpioDriver(&dev_rgpio_open, &dev_rgpio_close, &setTunerPower, &setAntPower, &setTunerReset);
	tcradio_configTunerIQ01I2sDriver(&dev_iq01_i2s_open, &dev_iq01_i2s_close, &dev_iq01_i2s_stop, &dev_iq01_i2s_start, &dev_iq01_i2s_setParameters, &dev_iq01_i2s_read, &dev_iq01_i2s_read_ch, &dev_iq01_i2s_startWithParams);
	tcradio_configTunerIQ23I2sDriver(&dev_iq23_i2s_open, &dev_iq23_i2s_close, &dev_iq23_i2s_stop, &dev_iq23_i2s_start, &dev_iq23_i2s_setParameters, &dev_iq23_i2s_read, &dev_iq23_i2s_read_ch, &dev_iq23_i2s_startWithParams);
	tcradio_configTunerBlendAudioI2sDriver(&dev_blend_audio_i2s_open, &dev_blend_audio_i2s_close, &dev_blend_audio_i2s_stop, &dev_blend_audio_i2s_start, &dev_blend_audio_i2s_setParameters, &dev_blend_audio_i2s_read, &dev_blend_audio_i2s_startWithParams);
}

void tcradio_releaseTunerPeriDrivers(void)
{
	// Release the set peri drivers.
	pfnIQ01I2sOpen = NULL;
	pfnIQ01I2sClose = NULL;
	pfnIQ01I2sStop = NULL;
	pfnIQ01I2sStart = NULL;
	pfnIQ01I2sSetParams = NULL;
	pfnIQ01I2sStartWithParams = NULL;
	pfnIQ01I2sRead = NULL;
	pfnIQ01I2sReadCh = NULL;

	pfnIQ23I2sOpen = NULL;
	pfnIQ23I2sClose = NULL;
	pfnIQ23I2sStop = NULL;
	pfnIQ23I2sStart = NULL;
	pfnIQ23I2sSetParams = NULL;
	pfnIQ23I2sStartWithParams = NULL;
	pfnIQ23I2sRead = NULL;
	pfnIQ23I2sReadCh = NULL;

	pfnBlendAudioI2sOpen = NULL;
	pfnBlendAudioI2sClose = NULL;
	pfnBlendAudioI2sStop = NULL;
	pfnBlendAudioI2sStart = NULL;
	pfnBlendAudioI2sSetParams = NULL;
	pfnBlendAudioI2sStartWithParams = NULL;
	pfnBlendAudioI2sRead =NULL;

	pfnI2cOpen = NULL;
	pfnI2cClose = NULL;
	pfnI2cTx = NULL;
	pfnI2cRx = NULL;

	pfnSpiOpen = NULL;
	pfnSpiClose = NULL;
	pfnSpiTxRx = NULL;

	pfnRadioGpioOpen = NULL;
	pfnRadioGpioClose = NULL;
	pfnTunerPower = NULL;
	pfnAntPower = NULL;
	pfnTunerReset = NULL;
}

