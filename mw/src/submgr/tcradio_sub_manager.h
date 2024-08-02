/*******************************************************************************

*   FileName : tcradio_sub_manager.h

*   Copyright (c) Telechips Inc.

*   Description : Radio Sub-core Manager Header

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
#ifndef TCRADIO_SUB_MANAGER_H__
#define TCRADIO_SUB_MANAGER_H__

/***************************************************
*               Include                            *
****************************************************/
#ifdef __ANDROID__
#include <android/log.h>
#endif

/***************************************************
*               Defines                            *
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "DMBLog.h"

#define SRMGR_DEBUG

#ifdef __ANDROID__

#define SRMGR_TAG			("[RADIO][SUBMGR]")
#define SRMGR_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,SRMGR_TAG, __VA_ARGS__))
#define SRMGR_WRN(...)		(__android_log_print(ANDROID_LOG_WARN,SRMGR_TAG, __VA_ARGS__))
#define SRMGR_INF(...)		(__android_log_print(ANDROID_LOG_INFO,SRMGR_TAG, __VA_ARGS__))
#ifdef SRMGR_DEBUG
#define SRMGR_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,SRMGR_TAG, __VA_ARGS__))
#else
#define	SRMGR_DBG(...)
#endif

#else // #ifdef __ANDROID__

// #define SRMGR_ERR(...)		((void)printf("[ERROR][RADIO][SUBMGR]: " __VA_ARGS__))
#define SRMGR_ERR(...)		((void)LB_PRINTF("[ERROR][RADIO][SUBMGR]: " __VA_ARGS__))
// #define SRMGR_WRN(...) 		((void)printf("[WARN][RADIO][SUBMGR]: " __VA_ARGS__))
#define SRMGR_WRN(...) 		((void)LB_PRINTF("[WARN][RADIO][SUBMGR]: " __VA_ARGS__))
// #define SRMGR_INF(...) 		((void)printf("[INFO][RADIO][SUBMGR]: " __VA_ARGS__))
#define SRMGR_INF(...) 		((void)LB_PRINTF("[INFO][RADIO][SUBMGR]: " __VA_ARGS__))
#ifdef SRMGR_DEBUG
// #define SRMGR_DBG(...)		((void)printf("[DEBUG][RADIO][SUBMGR]: " __VA_ARGS__))
#define SRMGR_DBG(...)		((void)LB_PRINTF("[DEBUG][RADIO][SUBMGR]: " __VA_ARGS__))
#else
#define	SRMGR_DBG(...)
#endif

#endif // #ifdef __ANDROID__

#define	IVI_REQ_UCMD	(0x0100UL)

#define	SUB_RESP_UCMD	(0xB100UL)
#define	SUB_NOTI_UCMD	(0xA100UL)

/***************************************************
*               Enumeration                        *
****************************************************/
typedef enum
{
	eRMGR_IPC_CMD_Init					= 0x0000UL,
	eRMGR_IPC_CMD_Deinit				= 0x0001UL,
	eRMGR_IPC_CMD_Open					= 0x0002UL,
	eRMGR_IPC_CMD_Close					= 0x0003UL,
    eRMGR_IPC_CMD_SetBandFreqConfig		= 0x0004UL,
    eRMGR_IPC_CMD_SetTune				= 0x0005UL,
    eRMGR_IPC_CMD_SetSeek 				= 0x0006UL,
    eRMGR_IPC_CMD_SetAudio				= 0x0007UL,
    eRMGR_IPC_CMD_SetAudioDevice		= 0x0008UL,
    eRMGR_IPC_CMD_GetBandFreqConfig		= 0x0009UL,
    eRMGR_IPC_CMD_GetTune 				= 0x000AUL,
    eRMGR_IPC_CMD_GetQuality			= 0x000BUL,
	eRMGR_IPC_CMD_GetHdrAllStatus		= 0x0104UL,
	eRMGR_IPC_CMD_GetHdrSignalStatus 	= 0x0105UL,
	eRMGR_IPC_CMD_SetHdrProgram 		= 0x0106UL,
	eRMGR_IPC_CMD_SetHdrAudioMode 		= 0x0107UL,
	eRMGR_IPC_CMD_SetHdrMute 			= 0x0108UL,
	eRMGR_IPC_CMD_SetHdrAduioCtrl 		= 0x0109UL,
	eRMGR_IPC_CMD_SetHdrAudioMuteFader	= 0x010AUL,
	eRMGR_IPC_CMD_GetHdrAudioMuteFader	= 0x010BUL,
	eRMGR_IPC_CMD_SetHdrAutoAudioAlign	= 0x010CUL,
}eRMGR_IPC_CMD_t;

typedef enum
{
    eRMGR_IPC_RESP_Init					= 0x0000UL,
	eRMGR_IPC_RESP_Deinit				= 0x0001UL,
	eRMGR_IPC_RESP_Open					= 0x0002UL,
	eRMGR_IPC_RESP_Close				= 0x0003UL,
	eRMGR_IPC_RESP_SetBandFreqConfig	= 0x0004UL,
	eRMGR_IPC_RESP_SetTune				= 0x0005UL,
	eRMGR_IPC_RESP_SetSeek				= 0x0006UL,
	eRMGR_IPC_RESP_SetAudio				= 0x0007UL,
	eRMGR_IPC_RESP_SetAudioDevice		= 0x0008UL,
	eRMGR_IPC_RESP_GetBandFreqConfig	= 0x0009UL,
	eRMGR_IPC_RESP_GetTune				= 0x000AUL,
	eRMGR_IPC_RESP_GetQuality			= 0x000BUL,
	eRMGR_IPC_RESP_SetScanStationListResult = 0x000CUL,
	eRMGR_IPC_RESP_HdrOpen				= 0x0102UL,
	eRMGR_IPC_RESP_GetHdrAllStatus		= 0x0104UL,
	eRMGR_IPC_RESP_GetHdrSignalStatus	= 0x0105UL,
	eRMGR_IPC_RESP_SetHdrProgram		= 0x0106UL,
	eRMGR_IPC_RESP_SetHdrAudioMode		= 0x0107UL,
	eRMGR_IPC_RESP_SetHdrMute			= 0x0108UL,
	eRMGR_IPC_RESP_SetHdrAudioCtrl		= 0x0109UL,
	eRMGR_IPC_RESP_SetHdrAudioMuteFader	= 0x010AUL,
	eRMGR_IPC_RESP_GetHdrAudioMuteFader	= 0x010BUL,
	eRMGR_IPC_RESP_SetHdrAutoAudioAlign	= 0x010CUL,
}eRMGR_IPC_RESP_t;

typedef enum
{
	eRMGR_IPC_NOTI_HdrSignalStatus		= 0x0100UL,
	eRMGR_IPC_NOTI_HdrPSD				= 0x0101UL,
	eRMGR_IPC_NOTI_HdrSIS				= 0x0102UL,
	eRMGR_IPC_NOTI_HdrPTY				= 0x0103UL,
}eRMGR_IPC_NOTI_t;

/***************************************************
*               Typedefs                           *
****************************************************/

/***************************************************
*               Constant definitions               *
****************************************************/

/***************************************************
*               Variable definitions               *
****************************************************/

/***************************************************
*               Function declaration               *
****************************************************/


#ifdef __cplusplus
}
#endif

#endif
