/*******************************************************************************

*   FileName : tcradio_hal_config.h

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
#ifndef __TCRADIO_HAL_CONFIG_H__
#define __TCRADIO_HAL_CONFIG_H__

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
	eAUDIO_DRV_CMD_PREPARE	=0,
	eAUDIO_DRV_CMD_DROP,
	eAUDIO_DRV_CMD_DRAIN,
	eAUDIO_DRV_CMD_RESET,
	eAUDIO_DRV_CMD_START,
	eAUDIO_DRV_CMD_AVAIL,
	eAUDIO_DRV_CMD_AVAIL_UPDATE,
	eAUDIO_DRV_CMD_VOLUP,
	eAUDIO_DRV_CMD_VOLDN
}eAUDIO_DRV_CMD_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct {
	uint32 samplerate;
	uint32 channels;
	uint32 buffersize;
	uint32 periodsize;
	uint32 aout_startThd;
	uint32 ain_startThd;
}stAUDIO_CONFIG_t;

typedef RET (*pfnAoutOpen_t)(stAUDIO_CONFIG_t params);
typedef RET (*pfnAoutClose_t)(void);
typedef RET (*pfnAoutWrite_t)(uint8 *pbuf, int32 frame_size);
typedef RET (*pfnAoutCmd_t)(eAUDIO_DRV_CMD_t cmd, void *args);
typedef RET (*pfnAinOpen_t)(stAUDIO_CONFIG_t params);
typedef RET (*pfnAinClose_t)(void);
typedef RET (*pfnAinRead_t)(uint8 *pbuf, int32 frame_size);
typedef RET (*pfnAinCmd_t)(eAUDIO_DRV_CMD_t cmd, void *args);

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern pfnAoutOpen_t pfnAoutOpen;
extern pfnAoutClose_t pfnAoutClose;
extern pfnAoutWrite_t pfnAoutWrite;
extern pfnAoutCmd_t pfnAoutCmd;
extern pfnAinOpen_t pfnAinOpen;
extern pfnAinClose_t pfnAinClose;
extern pfnAinRead_t pfnAinRead;
extern pfnAinCmd_t pfnAinCmd;

/***************************************************
*			Function declaration				*
****************************************************/
extern void tcradio_configAoutDriver(RET(*pfnOpen)(stAUDIO_CONFIG_t params), RET(*pfnClose)(void), RET(*pfnWrite)(uint8* pbuf, int32 frame_size), RET(*pfnCmd)(eAUDIO_DRV_CMD_t cmd, void* args));
extern void tcradio_configAinDriver(RET(*pfnOpen)(stAUDIO_CONFIG_t params), RET(*pfnClose)(void), RET(*pfnRead)(uint8* pbuf, int32 frame_size), RET(*pfnCmd)(eAUDIO_DRV_CMD_t cmd, void* args));
extern void tcradio_releaseAoutDriver(void);
extern void tcradio_releaseAinDriver(void);
extern int32 tcradio_getS0TunerConfig(void);
extern int32 tcradio_getX0TunerConfig(void);
extern int32 tcradio_getM0TunerConfig(void);
extern void tcradiohal_extraInit(void);
extern void tcradiohal_extraDeinit(void);
#ifdef __cplusplus
}
#endif

#endif	// #ifndef __RADIO_HAL_CONFIG_H__

