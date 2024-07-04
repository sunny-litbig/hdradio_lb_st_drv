/*******************************************************************************

*   FileName : tcradio_hdr_if.c

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
#include "tcradio_api.h"
#include "tcradio_hal.h"
#include "tcradio_msgq.h"
#include "tcradio_utils.h"
#include "tcradio_service.h"
#ifdef USE_HDRADIO
#include "tcradio_hdr_if.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
stTC_HDR_CONF_t stHdrConf;

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/
extern void tcradiosound_hdrAudioOutPCMCallback(void *pOutBuf, int32 frames, uint32 samplerate);

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
void tcradioservice_getTcHdrNotificationCallBack(uint32 notifyID, uint32 *arg, void **pData, int32 errorCode);
void tcradioservice_audioQueueCallBack(void *pOutBuf, int32 frame, uint32 samplerate);

/***************************************************
*			function definition				*
****************************************************/
const S8 *tcradio_getHdrFrameworkVersionString(void)
{
	return tchdr_getFrameworkVersionString();
}

const S8 *tcradio_getHdrLibraryVersionString(void)
{
	return tchdr_getLibraryVersionString();
}

eTC_HDR_BBSRC_RATE_t tcradioservice_getIqSampleRate(uint32 ntuner)
{
	float srRet;
	int32 iqSR = tcradiohal_getIqSampleRate(ntuner);

	if(iqSR == 744187) {
		srRet = eTC_HDR_BBSRC_744_KHZ;
	}
	else if(iqSR == 650000) {
		srRet = eTC_HDR_BBSRC_650_KHZ;
	}
	else if(iqSR == 675000) {
		srRet = eTC_HDR_BBSRC_675_KHZ;
	}
	else {
		srRet = eTC_HDR_BBSRC_UNKNOWN;
	}

	return srRet;
}

void tcradioservice_confHdrIQ01Drv(int(*open)(void), int(*close)(void), int(*setparams)(int nchannels, int nbit, int samplerate, int buffersize, int periodsize), int(*start)(void), int(*stop)(void), int(*read)(char *data, int readsize))
{
#ifdef USE_HDRADIO
	tchdr_configTunerIQ01Driver(open, close, setparams, start, stop, read);
#endif
}

void tcradioservice_confHdrIQ23Drv(int(*open)(void), int(*close)(void), int(*setparams)(int nchannels, int nbit, int samplerate, int buffersize, int periodsize), int(*start)(void), int(*stop)(void), int(*read)(char *data, int readsize))
{
#ifdef USE_HDRADIO
	tchdr_configTunerIQ23Driver(open, close, setparams, start, stop, read);
#endif
}

void tcradioservice_confHdrBlendAudioDrv(int(*open)(void), int(*close)(void), int(*setparams)(int nchannels, int nbit, int samplerate, int buffersize, int periodsize), int(*start)(void), int(*stop)(void), int(*read)(char *data, int readsize))
{
#ifdef USE_HDRADIO
	tchdr_configTunerBlendAudioDriver(open, close, setparams, start, stop, read);
#endif
}

void tcradioservice_relHdrIQ01Drv(void)
{
#ifdef USE_HDRADIO
	tchdr_configTunerIQ01Driver(NULL, NULL, NULL, NULL, NULL, NULL);
#endif
}

void tcradioservice_relHdrIQ23Drv(void)
{
#ifdef USE_HDRADIO
	tchdr_configTunerIQ23Driver(NULL, NULL, NULL, NULL, NULL, NULL);
#endif
}

void tcradioservice_relHdrBlendAudioDrv(void)
{
#ifdef USE_HDRADIO
	tchdr_configTunerBlendAudioDriver(NULL, NULL, NULL, NULL, NULL, NULL);
#endif
}

RET tcradioservice_initHdr(eRADIO_HD_TYPE_t type, eTC_HDR_BBSRC_RATE_t iqsamplerate, uint32 iqsamplebit)
{
	RET ret = eRET_NG_NOT_SUPPORT;
	stTC_HDR_THREAD_PR_t userPriority;

#ifdef USE_HDRADIO
	tchdr_configTcHdrNotificationCallBack(tcradioservice_getTcHdrNotificationCallBack);
	tchdr_configTcHdrAudioQueueCallBack(tcradioservice_audioQueueCallBack);

	switch(type) {
		case eRADIO_HD_TYPE_HD1p0:			stHdrConf.hdrType = eTC_HDR_TYPE_HD_1p0;			break;
		case eRADIO_HD_TYPE_HD1p5:			stHdrConf.hdrType = eTC_HDR_TYPE_HD_1p5;			break;
		case eRADIO_HD_TYPE_HD1p0_MRC:		stHdrConf.hdrType = eTC_HDR_TYPE_HD_1p0_MRC;		break;
		case eRADIO_HD_TYPE_HD1p5_MRC:		stHdrConf.hdrType = eTC_HDR_TYPE_HD_1p5_MRC;		break;
		case eRADIO_HD_TYPE_HD1p5_DUAL_MRC:	stHdrConf.hdrType = eTC_HDR_TYPE_HD_1p5_DUAL_MRC;	break;
		default:							return eRET_NG_INVALID_PARAM;						break;
	}

	stHdrConf.iq.samplingBit = iqsamplebit;
	stHdrConf.iq.maxSampleRate = iqsamplerate;
//	stHdrConf.iq.maxSampleRate = eTC_HDR_BBSRC_744_KHZ;		// for silab tuner

#if 0	// for NICE Priority Test
	tchdr_getDefaultThreadNicePriority(eTHREAD_MANAGER, &userPriority);
	tchdr_setThreadPriority(eTHREAD_MANAGER, userPriority);
	tchdr_getDefaultThreadNicePriority(eTHREAD_IQINPUT, &userPriority);
	tchdr_setThreadPriority(eTHREAD_IQINPUT, userPriority);
	tchdr_getDefaultThreadNicePriority(eTHREAD_AUDINPUT, &userPriority);
	tchdr_setThreadPriority(eTHREAD_AUDINPUT, userPriority);
	tchdr_getDefaultThreadNicePriority(eTHREAD_BBINPUT, &userPriority);
	tchdr_setThreadPriority(eTHREAD_BBINPUT, userPriority);
	tchdr_getDefaultThreadNicePriority(eTHREAD_DEMOD, &userPriority);
	tchdr_setThreadPriority(eTHREAD_DEMOD, userPriority);
	tchdr_getDefaultThreadNicePriority(eTHREAD_BLENDING, &userPriority);
	tchdr_setThreadPriority(eTHREAD_BLENDING, userPriority);
	tchdr_getDefaultThreadNicePriority(eTHREAD_AUDOUTPUT, &userPriority);
	tchdr_setThreadPriority(eTHREAD_AUDOUTPUT, userPriority);
	tchdr_getDefaultThreadNicePriority(eTHREAD_CMDPROC, &userPriority);
	tchdr_setThreadPriority(eTHREAD_CMDPROC, userPriority);
	tchdr_getDefaultThreadNicePriority(eTHREAD_LOGGER, &userPriority);
	tchdr_setThreadPriority(eTHREAD_LOGGER, userPriority);
#endif

	ret = tchdr_init(stHdrConf);
#endif
	return ret;
}

RET tcradioservice_deinitHdr(void)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_deinit();
#endif
	return ret;
}

RET tcradioservice_openHdr(eRADIO_MOD_MODE_t init_band, uint32 init_freq, eTC_HDR_BBSRC_RATE_t iqsamplerate)
{
	RET ret = eRET_OK;

#ifdef USE_HDRADIO
	stTC_HDR_TUNE_INFO_t tuneInfo;
	float64 out_hz = (float64)0.0;

	if(init_band == eRADIO_AM_MODE) {
		tuneInfo.mainTuner.band = eTC_HDR_AM_BAND;
	}
	else {
		tuneInfo.mainTuner.band = eTC_HDR_FM_BAND;
	}
	tuneInfo.mainTuner.freq = init_freq;
	tuneInfo.mainTuner.iqsamplerate = iqsamplerate;

	tuneInfo.mrcTuner.band = tuneInfo.mainTuner.band;
	tuneInfo.mrcTuner.freq = tuneInfo.mainTuner.freq;
	tuneInfo.mrcTuner.iqsamplerate = tuneInfo.mainTuner.iqsamplerate;

	tuneInfo.bsTuner.band = tuneInfo.mainTuner.band;
	tuneInfo.bsTuner.freq = tuneInfo.mainTuner.freq;
	tuneInfo.bsTuner.iqsamplerate = tuneInfo.mainTuner.iqsamplerate;

	if(tcradiohal_getTunerChip() == eTUNER_IC_S0) {
		tchdr_setAudioResamplerSlips(0U, -0.0522655, &out_hz);
	}

	ret = tchdr_open(tuneInfo);
	if(ret != 0) {
		ret = eRET_NG_NO_RSC;
	}
#endif
	return ret;
}

RET tcradioservice_closeHdr(void)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_close();
#endif
	return ret;
}

RET tcradioservice_setHdrTune(eTC_HDR_ID_t id, stTC_HDR_TUNE_TO_t tuneTo)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_setTune(id, tuneTo);
#endif
	return ret;
}

RET tcradio_setHdrAudioMode(uint32 audioMode)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_setAudioMode(audioMode);
#endif
	return ret;
}

RET tcradio_setHdrProgram(uint32 num)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_setProgram(eTC_HDR_ID_MAIN, num);
	HDRS_DBG("Set Main Program: [%d]!\n", num);
#endif
	return ret;
}

RET tcradio_setHdrPsdNotification(uint32 num, uint8 mask)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_enablePsdNotification(eTC_HDR_ID_MAIN, (unsigned int)0x01<<num, mask, 1);
#endif
	return ret;
}

RET tcradioservice_setHdrLotOpen(uint32 service, uint32 port)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO

#endif
	return ret;
}

RET tcradioservice_setHdrLotFlush(uint32 service, uint32 port)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO

#endif
	return ret;
}

RET tcradioservice_getHdrStatus(stTC_HDR_STATUS_t *sts)
{
	RET ret = eRET_NG_NOT_SUPPORT;
	stTC_HDR_STATUS_t temp_hdr_status;

#ifdef USE_HDRADIO
	#if  0
	ret = tchdr_getSignalStatus(eTC_HDR_ID_t id, unsigned int *dataOut)
	ret = tchdradiohal_getHdrStatus(0, &hdrSts);
	if(ret == eRET_OK) {
		temp_hdr_status.cnr = hdrSts.mon_info.hdr.cn;
		temp_hdr_status.fhdaudio = hdrSts.mon_info.hdr.digital_audio_acquired;
		temp_hdr_status.fhdsignal = hdrSts.mon_info.hdr.hd_signal_acquired;
		temp_hdr_status.pmap = hdrSts.mon_info.hdr.audio_avaliable_program_bitmap;
		temp_hdr_status.fhybridprog = hdrSts.mon_info.hdr.hybridprogram;

		*sts = temp_hdr_status;
	}
	#endif
#endif
	return ret;
}

RET tcradioservice_enableHdrSis(eTC_HDR_ID_t id, unsigned int sisBitmask, unsigned int fEn)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	tchdr_enableSisNotification(id, sisBitmask, fEn);
#endif
	return ret;
}

RET tcradioservice_enableHdrPsd(eTC_HDR_ID_t id, unsigned char progBitmask, unsigned char psdBitmask, unsigned int fEn)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	tchdr_enablePsdNotification(id, progBitmask, psdBitmask, fEn);
#endif
	return ret;
}

RET tcradioservice_enableHdrLot(eTC_HDR_ID_t id, unsigned char progBitmask, unsigned int fEn)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_enableLotNotification(id, progBitmask, fEn);
#endif
	return ret;
}

RET tcradioservice_enableHdrAlert(eTC_HDR_ID_t id, unsigned int fEn)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_enableAlertNotification(id, fEn);
#endif
	return ret;
}

RET tcradioservice_getHdrSig(void)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO

#endif
	return ret;
}

RET tcradioservice_getHdrAlertMessage(void)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO

#endif
	return ret;
}

RET tcradioservice_getHdrLot(void)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO

#endif
	return ret;
}

RET tcradioservice_setHdrProgramNumber(uint32 num)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_setProgram(eTC_HDR_ID_MAIN, num);
#endif
	return ret;
}

int32 tcradioservice_getHdrProgramNumber(void)
{
	uint32 ret = -1;
#ifdef USE_HDRADIO
	uint32 pn;
	ret = tchdr_getProgram(eTC_HDR_ID_MAIN, &pn);
#endif
	return ret;
}

RET tcradioservice_getHdrSupport(void)	//  It is a function to be placed in the hd radio library.
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = eRET_OK;
#endif
	return ret;
}

RET tcradioservice_setHdrMute(uint32 fOnOff)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_setAudioMute(fOnOff);
#endif
	return ret;
}

RET tcradioservice_setHdrAudioCtrl(uint32 fStartStop)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_setAudioCtrl(fStartStop);
#endif
	return ret;
}

RET tcradioservice_setHdrAudioMuteFader(unsigned int enable, unsigned int fadein_ms, unsigned int fadeout_ms)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_setAudioMuteFader(enable, fadein_ms, fadeout_ms);
#endif
	return ret;
}

RET tcradioservice_getHdrAudioMuteFader(unsigned int *enable, unsigned int *fadein_ms, unsigned int *fadeout_ms)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_getAudioMuteFader(enable, fadein_ms, fadeout_ms);
#endif
	return ret;
}

RET tcradio_getHdrAllStatus(eTC_HDR_ID_t id, stTC_HDR_STATUS_t *dataOut)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_getAllStatus(id, dataOut);
#endif
	return ret;
}

RET tcradio_getHdrSignalStatus(eTC_HDR_ID_t id, stTC_HDR_SIGNAL_STATUS_t *dataOut)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_getSignalStatus(id, dataOut);
#endif
	return ret;
}

RET tcradio_getStationShortName(eTC_HDR_ID_t id, stTC_HDR_SIS_SHORT_NAME_t *dataOut)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_sis_getStationShortName(id, dataOut);
#endif
	return ret;
}

RET tcradio_getProgramType(eTC_HDR_ID_t id, stTC_HDR_PTY_t *pty)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_getProgramType(id, pty);
#endif
	return ret;
}

RET tcradio_setHdrAutoAudioAlignEnable(U32 fEnable)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_setAutoAudioAlignEnable(fEnable);
#endif
	return ret;
}

RET tcradio_setHdrThreadPriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t userprio)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_setThreadPriority(thread, userprio);
#endif
	return ret;
}

void tcradio_getHdrDefaultThreadPriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t *userprio)
{
#ifdef USE_HDRADIO
	(void)tchdr_getDefaultThreadPriority(thread, userprio);
#endif
}

void tcradio_getHdrThreadPriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t *userprio)
{
#ifdef USE_HDRADIO
	(void)tchdr_getThreadPriority(thread, userprio);
#endif
}

void tcradioservice_getTcHdrNotificationCallBack(uint32 notifyID, uint32 *arg, void **pData, int32 errorCode)
{
	uint32 i;

	switch(notifyID) {
		case eTC_HDR_NOTIFY_OPEN:
			if(errorCode != eRET_OK) {
			//	HDRS_DBG("eTC_HDR_NOTIFY_OPEN : Error[%d]!\n", errorCode);
			}
			else {
			//	HDRS_DBG("eTC_HDR_NOTIFY_OPEN : Done!\n");
				tchdr_enableSisNotification(eTC_HDR_ID_MAIN, eBITMASK_SIS_SHORT_NAME, 1);
			}
			tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_HD_NOTIFY_OPEN, arg, pData, errorCode);
			break;

		case eTC_HDR_NOTIFY_AUDIO_MODE:
			tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_HD_NOTIFY_AUDIO_MODE, arg, pData, errorCode);
			break;

		case eTC_HDR_NOTIFY_TUNE:
			if(errorCode != eRET_OK) {
				HDRS_DBG("eTC_HDR_NOTIFY_TUNE : Error[%d]!\n", errorCode);
			}
			else {
				HDRS_DBG("eTC_HDR_NOTIFY_TUNE : Done! id[%d], fChgBand[%d], fChgFreq[%d], fChgSR[%d]\n", arg[0], arg[1], arg[2], arg[3]);
			}
			break;

		case eTC_HDR_NOTIFY_MUTE:
			if(errorCode != eRET_OK) {
				HDRS_DBG("eTC_HDR_NOTIFY_MUTE : Error[%d]!\n", errorCode);
			}
			else {
				HDRS_DBG("eTC_HDR_NOTIFY_MUTE : %s!\n", arg[0]==0 ? "Mute off" : "Mute on");
			}
			break;

		case eTC_HDR_NOTIFY_AUDIO_CTRL:
			if(errorCode != eRET_OK) {
				HDRS_DBG("eTC_HDR_NOTIFY_AUDIO_CTRL : Error[%d]!\n", errorCode);
			}
			else {
				HDRS_DBG("eTC_HDR_NOTIFY_AUDIO_CTRL : %s!\n", arg[0]==0 ? "Stop" : "Start");
			}
			break;

		case eTC_HDR_NOTIFY_PSD:
			tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_HD_NOTIFY_PSD, arg, pData, errorCode);
			break;

		case eTC_HDR_NOTIFY_SIS:
			tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_HD_NOTIFY_SIS, arg, pData, errorCode);
			break;

		case eTC_HDR_NOTIFY_SIGNAL_STATUS:
			if(errorCode == eRET_OK) {
				stTC_HDR_SIGNAL_STATUS_t *sigSts = (stTC_HDR_SIGNAL_STATUS_t*)pData[0];
			}
			tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_HD_NOTIFY_SIGNAL_STATUS, arg, pData, errorCode);
			break;

		case eTC_HDR_NOTIFY_PTY:
			tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_HD_NOTIFY_PTY, arg, pData, errorCode);
			break;

		case eTC_HDR_NOTIFY_LOT:
			tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_HD_NOTIFY_LOT, arg, pData, errorCode);
			break;

		case eTC_HDR_NOTIFY_ALERT:
			tcradioapp_sendMessage(eSENDER_ID_SERVICE, eRADIO_HD_NOTIFY_ALERT, arg, pData, errorCode);
			break;

		default:
			break;
	}
}

void tcradioservice_audioQueueCallBack(void *pOutBuf, int32 frame, uint32 samplerate)
{
	tcradiosound_hdrAudioOutPCMCallback(pOutBuf, frame, samplerate);
}

RET tcradio_setTcHdrApiTest(uint32 num, uint32 *cmd)
{
	RET ret = eRET_NG_NOT_SUPPORT;
	stTC_HDR_SIG_SERVICE_LIST_t service_list;
	stTC_HDR_PSD_FORM_t psd_form;
#ifdef USE_HDRADIO
	switch(num) {
		case 0:	ret = tchdr_setProgram(eTC_HDR_ID_MAIN, num);	break;

		case 1:
			ret = tchdr_psd_getTitle(eTC_HDR_ID_MAIN, eTC_HDR_PROGRAM_HD1, &psd_form);
			ret |= tchdr_sig_getServiceList(eTC_HDR_ID_MAIN, eTC_HDR_SIG_AUDIO_COMPONENT, &service_list);
			break;

		case 2:
			ret = tchdr_setAudioCtrl(0);
			break;

		case 3:
			ret = tchdr_setAudioCtrl(1);
			break;

		default:
			break;
	}
#endif
	return ret;
}

RET tcradio_debugHdr(uint32 id, uint32 numdbg, uint32 *arg)
{
	RET ret = eRET_NG_NOT_SUPPORT;
#ifdef USE_HDRADIO
	ret = tchdr_debugTcHdrFramework(id, numdbg, arg);
#endif
	return ret;
}
#endif // USE_HDRADIO
