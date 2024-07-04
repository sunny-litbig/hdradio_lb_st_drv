/*******************************************************************************

*   FileName : tcradio_hal_config.c

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
/***************************************************
*		Include 			   					*
****************************************************/
#include "tcradio_types.h"
#include "dev_iq_i2s.h"
#include "tcradio_peri_config.h"
#include "tcradio_hal.h"
#include "tcradio_hal_config.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
pfnAoutOpen_t pfnAoutOpen;
pfnAoutClose_t pfnAoutClose;
pfnAoutWrite_t pfnAoutWrite;
pfnAoutCmd_t pfnAoutCmd;
pfnAinOpen_t pfnAinOpen;
pfnAinClose_t pfnAinClose;
pfnAinRead_t pfnAinRead;
pfnAinCmd_t pfnAinCmd;

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
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition				*
****************************************************/
void tcradio_configAoutDriver(RET(*pfnOpen)(stAUDIO_CONFIG_t params), RET(*pfnClose)(void), RET(*pfnWrite)(uint8* pbuf, int32 frame_size), RET(*pfnCmd)(eAUDIO_DRV_CMD_t cmd, void* args))
{
	pfnAoutOpen = pfnOpen;
	pfnAoutClose = pfnClose;
	pfnAoutWrite = pfnWrite;
	pfnAoutCmd = pfnCmd;
}

void tcradio_configAinDriver(RET(*pfnOpen)(stAUDIO_CONFIG_t params), RET(*pfnClose)(void), RET(*pfnRead)(uint8* pbuf, int32 frame_size), RET(*pfnCmd)(eAUDIO_DRV_CMD_t cmd, void* args))
{
	pfnAinOpen = pfnOpen;
	pfnAinClose = pfnClose;
	pfnAinRead = pfnRead;
	pfnAinCmd = pfnCmd;
}

void tcradio_releaseAoutDriver(void)
{
	pfnAoutOpen = NULL;
	pfnAoutClose = NULL;
	pfnAoutWrite = NULL;
	pfnAoutCmd = NULL;
}

void tcradio_releaseAinDriver(void)
{
	pfnAinOpen = NULL;
	pfnAinClose = NULL;
	pfnAinRead = NULL;
	pfnAinCmd = NULL;
}

int32 tcradio_getS0TunerConfig(void)
{
	int32 ret=0;
#if defined(USE_S0_TUNER)
	ret = 1;
#endif
	return ret;
}

int32 tcradio_getT0TunerConfig(void)
{
	int32 ret=0;
#if defined(USE_T0_TUNER)
	ret = 1;
#endif
	return ret;
}

int32 tcradio_getX0TunerConfig(void)
{
	int32 ret=0;
#if defined(USE_X0_TUNER)
	ret = 1;
#endif
	return ret;
}

int32 tcradio_getM0TunerConfig(void)
{
	int32 ret=0;
#if defined(USE_M0_TUNER)
	ret = 1;
#endif
	return ret;
}

void tcradiohal_extraInit(void)
{
	//User Code...
}

void tcradiohal_extraDeinit(void)
{
	//User Code...
}

