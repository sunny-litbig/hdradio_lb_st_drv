/*******************************************************************************

*   FileName : tcradio_hdr_if.h

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
#ifndef __TCRADIO_HDR_IF_H__
#define __TCRADIO_HDR_IF_H__

/***************************************************
*				Include					*
****************************************************/
#ifdef __ANDROID__
#include <android/log.h>
#endif
#include "tcradio_api.h"
#include "tchdr_types.h"
#include "tchdr_api.h"
#include "tchdr_psd.h"
#include "tchdr_sis.h"
#include "tchdr_aas.h"
#include "tchdr_alert.h"
#include "tchdr_sig.h"

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

//#define HDR_SERVICE_DEBUG

#ifdef __ANDROID__

#define HDRS_TAG			("[RADIO][HDR-IF]")
#define HDRS_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,HDRS_TAG, __VA_ARGS__))
#ifdef HDR_SERVICE_DEBUG
#define HDRS_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,HDRS_TAG, __VA_ARGS__))
#else
#define	HDRS_DBG(...)
#endif

#else // #ifdef __ANDROID__

#define HDRS_ERR(...)		((void)printf("[ERROR][HAL][HDR-IF]: " __VA_ARGS__))
#ifdef HDR_SERVICE_DEBUG
#define HDRS_DBG(...)		((void)printf("[DEBUG][HAL][HDR-IF]: " __VA_ARGS__))
#else
#define	HDRS_DBG(...)
#endif

#endif // #ifdef __ANDROID__

/***************************************************
*				Enumeration				*
****************************************************/

/***************************************************
*				Typedefs					*
****************************************************/

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern const S8 *tcradio_getHdrFrameworkVersionString(void);
extern const S8 *tcradio_getHdrLibraryVersionString(void);
extern RET tcradio_getHdrAllStatus(eTC_HDR_ID_t id, stTC_HDR_STATUS_t *dataOut);
extern RET tcradio_getHdrSignalStatus(eTC_HDR_ID_t id, stTC_HDR_SIGNAL_STATUS_t *dataOut);
extern RET tcradio_getStationShortName(eTC_HDR_ID_t id, stTC_HDR_SIS_SHORT_NAME_t *dataOut);
extern RET tcradio_getProgramType(eTC_HDR_ID_t id, stTC_HDR_PTY_t *pty);
extern RET tcradio_setHdrProgram(uint32 num);
extern RET tcradio_setHdrPsdNotification(uint32 num, uint8 mask);
extern RET tcradio_setHdrAudioMode(uint32 audioMode);
extern RET tcradio_setHdrAutoAudioAlignEnable(U32 fEnable);
extern RET tcradio_setHdrThreadPriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t userprio);
extern void tcradio_getHdrDefaultThreadPriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t * userprio);
extern void tcradio_getHdrThreadPriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t * userprio);
extern RET tcradio_setTcHdrApiTest(uint32 num, uint32 *cmd);
extern RET tcradio_debugHdr(uint32 id, uint32 numdbg, uint32 *arg);

extern RET tcradioservice_setHdrMute(uint32 fOnOff);
extern RET tcradioservice_setHdrAudioCtrl(uint32 fStartStop);
extern RET tcradioservice_setHdrAudioMuteFader(unsigned int enable, unsigned int fadein_ms, unsigned int fadeout_ms);
extern RET tcradioservice_getHdrAudioMuteFader(unsigned int *enable, unsigned int *fadein_ms, unsigned int *fadeout_ms);
extern void tcradioservice_confHdrIQ01Drv(int(*open)(void), int(*close)(void), int(*setparams)(int nchannels, int nbit, int samplerate, int buffersize, int periodsize), int(*start)(void), int(*stop)(void), int(*read)(char *data, int readsize));
extern void tcradioservice_confHdrIQ23Drv(int(*open)(void), int(*close)(void), int(*setparams)(int nchannels, int nbit, int samplerate, int buffersize, int periodsize), int(*start)(void), int(*stop)(void), int(*read)(char *data, int readsize));
extern void tcradioservice_confHdrBlendAudioDrv(int(*open)(void), int(*close)(void), int(*setparams)(int nchannels, int nbit, int samplerate, int buffersize, int periodsize), int(*start)(void), int(*stop)(void), int(*read)(char *data, int readsize));
extern void tcradioservice_relHdrIQ01Drv(void);
extern void tcradioservice_relHdrIQ23Drv(void);
extern void tcradioservice_relHdrBlendAudioDrv(void);

extern eTC_HDR_BBSRC_RATE_t tcradioservice_getIqSampleRate(uint32 ntuner);
extern RET tcradioservice_initHdr(eRADIO_HD_TYPE_t type, eTC_HDR_BBSRC_RATE_t iqsamplerate, uint32 iqsamplebit);
extern RET tcradioservice_deinitHdr(void);
extern RET tcradioservice_openHdr(eRADIO_MOD_MODE_t init_band, uint32 init_freq, eTC_HDR_BBSRC_RATE_t iqsamplerate);
extern RET tcradioservice_closeHdr(void);

extern RET tcradioservice_setHdrTune(eTC_HDR_ID_t id, stTC_HDR_TUNE_TO_t tuneTo);

extern RET tcradioservice_getHdrSupport(void);
extern void tcradioservice_initHdrData(void);

extern RET tcradioservice_setHdrLotOpen(uint32 service, uint32 port);
extern RET tcradioservice_setHdrLotFlush(uint32 service, uint32 port);

extern RET tcradioservice_enableHdrSis(eTC_HDR_ID_t id, unsigned int sisBitmask, unsigned int fEn);
extern RET tcradioservice_enableHdrPsd(eTC_HDR_ID_t id, unsigned char progBitmask, unsigned char psdBitmask, unsigned int fEn);
extern RET tcradioservice_enableHdrLot(eTC_HDR_ID_t id, unsigned char progBitmask, unsigned int fEn);
extern RET tcradioservice_enableHdrAlert(eTC_HDR_ID_t id, unsigned int fEn);

extern RET tcradioservice_getHdrStatus(stTC_HDR_STATUS_t *sts);
#if 0
extern RET tcradioservice_getHdrSis(void);
extern RET tcradioservice_getHdrPsd(uint32 prgno);
#endif
extern RET tcradioservice_getHdrSig(void);
extern RET tcradioservice_getHdrAlertMessage(void);
extern RET tcradioservice_getHdrLot(void);
extern RET tcradioservice_setHdrProgramNumber(uint32 num);
extern int32 tcradioservice_getHdrProgramNumber(void);

#ifdef __cplusplus
}
#endif

#endif
