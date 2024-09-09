/*******************************************************************************

*   FileName : tchdr_api.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio Framework API functions and definitions

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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <malloc.h>
#include <sys/socket.h>

#include "tchdr_common.h"

#include "hdrBbSrc.h"
#include "hdrAudio.h"
#include "hdrBlend.h"

#include "tchdr_cbuffer.h"
#include "tchdr_cmdcallbacks.h"
#include "tchdr_bytestream.h"
#include "tchdr_msg.h"
#include "tchdr_api.h"
#include "tchdr_psd.h"
#include "tchdr_sis.h"
#include "tchdr_service.h"
#include "tchdr_framework.h"
#include "tchdr_bbinput.h"
#include "tchdr_audio.h"
#include "tchdr_fader.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
stThread_attr_t stTcHdrManagerThdAttr;
stThread_attr_t stBbInputThdAttr;
stThread_attr_t stAudioBlendingThdAttr;
stThread_attr_t stPrimaryHdThdAttr;
stThread_attr_t stMrcHdThdAttr;
stThread_attr_t stBackscanHdThdAttr;
stThread_attr_t stBsMrcHdThdAttr;
stThread_attr_t stAudioPlaybackAttr;
stThread_attr_t stAudioInputAttr;
stThread_attr_t stRfIqInputAttr;
stThread_attr_t stCmdProcThdAttr;
stThread_attr_t stLoggerThdAttr;
stThread_attr_t stTraceThdAttr;

stTC_HDR_CONF_t	stTcHdrConf;

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/
// telechips protection
extern S32 tcid_check(void);

/***************************************************
*             Local preprocessor                   *
****************************************************/

/***************************************************
*           Local constant definitions              *
****************************************************/
static U32 fHdrInitStatus = 0;
static stTC_HDR_THREAD_PR_t stTcHdrThreadPriority[eTHREAD_MAX] = {
		{1, MANAGER_THREAD_PRIORITY},		// manager
		{1, RF_IQ_INPUT_THREAD_PRIORITY},	// iqinput
		{1, AUDIO_INPUT_THREAD_PRIORITY},	// audinput
		{1, BB_INPUT_THREAD_PRIORITY},		// bbinput
		{1, HDR_EXEC_THREAD_PRIORITY},		// demod
		{1, HDR_BLENDING_THREAD_PRIORITY},	// blending
		{1, AUDIO_OUTPUT_THREAD_PRIORITY},	// audoutput
		{1, CMD_PROC_THREAD_PRIORITY},		// cmdproc
		{1, LOGGER_THREAD_PRIORITY}			// logger
};

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/
static HDR_instance_t *tchdr_getHdrInstanceAddr(eTC_HDR_ID_t id);
HDRET tchdr_resetBBP(U32 mainOfBs);
HDRET tchdr_setMRC(U32 fOnOff);

static HDRET tchdr_checkTune(stTCHDR_TUNE_t tune);

/***************************************************
*			function definition				*
****************************************************/
HDRET tchdr_convertHdrError(HDR_error_code_t error)
{
	HDRET ret;
	switch((S32)error) {
		case (S32)HDR_ERROR_INVAL_ARG:
			ret = (HDRET)eTC_HDR_RET_NG_DEMOD_INVALID_PARAMETERS;
			break;
		case (S32)HDR_ERROR_REACQ:
			ret = (HDRET)eTC_HDR_RET_NG_DEMOD_BUSY;
			break;
		case (S32)HDR_ERROR_IDLE:
			ret = (HDRET)eTC_HDR_RET_NG_DEMOD_IDLE;
			break;
		case (S32)HDR_NOT_INITIALIZED:
			ret = (HDRET)eTC_HDR_RET_NG_DEMOD_NOT_INIT;
			break;
		case (S32)HDR_ERROR_INSTANCE_TYPE:
			ret = (HDRET)eTC_HDR_RET_NG_DEMOD_INSTANCE_TYPE;
			break;
		default:
			ret = (HDRET)eTC_HDR_RET_NG_UNKNOWN;
			break;
	}
	return ret;
}

static HDR_instance_t *tchdr_getHdrInstanceAddr(eTC_HDR_ID_t id)
{
	stHDR_FRAMEWORK_DATA_t* frameworkData = tchdrfwk_getDataStructPtr();
	HDR_instance_t *hdrInstance=pNULL;

	if(frameworkData != NULL) {
		U32 currentHdrType = tchdrfwk_getHdrType();
		if(id == eTC_HDR_ID_MAIN) {
			hdrInstance = &frameworkData->hdrInstance[0];
		}
		else if(id == eTC_HDR_ID_MRC) {
			if((currentHdrType == (U32)HDR_1p0_MRC_CONFIG) || (currentHdrType == (U32)HDR_1p5_MRC_CONFIG) || (currentHdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
				hdrInstance = &frameworkData->hdrInstance[1];
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Not support to the MRC type.\n");
			}
		}
		else if(id == eTC_HDR_ID_BS) {
			if(currentHdrType == (U32)HDR_1p5_CONFIG) {
				 hdrInstance = &frameworkData->hdrInstance[1];
			}
			else if((currentHdrType == (U32)HDR_1p5_MRC_CONFIG) || (currentHdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
				 hdrInstance = &frameworkData->hdrInstance[2];
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Not support to the background-scan type.\n");
			}
		}
		else if(id == eTC_HDR_ID_BS_MRC) {
			if(currentHdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG) {
				hdrInstance = &frameworkData->hdrInstance[3];
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Not support to the BS MRC type.\n");
			}
		}
		else {
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to get instance because of invalid ID.\n");
		}
	}

	return hdrInstance;
}

static void tchdr_setHDRadioInitStatus(U32 sts)
{
	if(sts > 0U) {
		fHdrInitStatus = 1;
	}
	else {
		fHdrInitStatus = 0;
	}
}

static U32 tchdr_getHDRadioInitStatus(void)
{
	return fHdrInitStatus;
}

static HDRET tchdr_checkHDRadioInitStatus(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	if(tchdr_getHDRadioInitStatus() == 0U) {
		ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_INIT;
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] Not initialize HD Radio framwork\n", __func__, __LINE__);
	}
	return ret;
}

HDRET tchdr_getHDRadioOpenStatus(void)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if(tchdrsvc_getOpenStatus() == (U32)0) {
			ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_OPEN;
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d] HD Radio framework is not opened\n", __func__, __LINE__);
		}
	}
	return ret;
}

static HDRET tchdr_checkTune(stTCHDR_TUNE_t tune)
{
	HDRET ret;

	if(tune.band == HDR_BAND_FM) {		// FM
		if((tune.iqsr != HDR_BB_SRC_1024_KHZ) && (tune.iqsr != HDR_BB_SRC_675_KHZ) &&
			(tune.iqsr != HDR_BB_SRC_650_KHZ) && (tune.iqsr != HDR_BB_SRC_744_KHZ) &&
			(tune.iqsr != HDR_BB_SRC_768_KHZ))
		{
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Invalid FM sample rate\n");
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
		else {
			ret = (HDRET)eTC_HDR_RET_OK;
		}
    }
	else if(tune.band == HDR_BAND_AM) {	// AM
		if((tune.iqsr != HDR_BB_SRC_1024_KHZ) && (tune.iqsr != HDR_BB_SRC_675_KHZ) &&
			(tune.iqsr != HDR_BB_SRC_650_KHZ) && (tune.iqsr != HDR_BB_SRC_744_KHZ) &&
			(tune.iqsr != HDR_BB_SRC_768_KHZ))
		{
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Invalid AM sample rate\n");
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
		else {
			ret = (HDRET)eTC_HDR_RET_OK;
		}
    }
	else if(tune.band == HDR_BAND_IDLE) {
		ret = (HDRET)eTC_HDR_RET_OK;
	}
	else {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Invalid band\n");
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
    }

	return ret;
}

static HDRET tchdr_checkConfiguration(stTC_HDR_CONF_t conf)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTC_HDR_BBSRC_RATE_t samplerate_khz = conf.iq.maxSampleRate;

	if((samplerate_khz != eTC_HDR_BBSRC_675_KHZ) && (samplerate_khz != eTC_HDR_BBSRC_650_KHZ) &&
		(samplerate_khz != eTC_HDR_BBSRC_1024_KHZ) && (samplerate_khz != eTC_HDR_BBSRC_744_KHZ) &&
		(samplerate_khz != eTC_HDR_BBSRC_768_KHZ))
	{
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to set sample configuration. The input sample rate is out of range.\n");
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
	}

	if(conf.iq.samplingBit != (U32)16) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to set sample configuration. The input sampling bit is out of range.\n");
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if(conf.hdrType == eTC_HDR_TYPE_HD_1p0) {
			tchdrfwk_setHdrType(HDR_1p0_CONFIG);
			tchdrfwk_setNumOfHdrInstance(1);
		}
#if HDR_CONFIG == HDR_1p5_CONFIG
		else if(conf.hdrType == eTC_HDR_TYPE_HD_1p5) {
			tchdrfwk_setHdrType(HDR_1p5_CONFIG);
			tchdrfwk_setNumOfHdrInstance(2);
		}
#endif
#if HDR_CONFIG == HDR_1p0_MRC_CONFIG
		else if(conf.hdrType == eTC_HDR_TYPE_HD_1p0_MRC) {
			tchdrfwk_setHdrType(HDR_1p0_MRC_CONFIG);
			tchdrfwk_setNumOfHdrInstance(2);
		}
#endif
#if HDR_CONFIG == HDR_1p5_MRC_CONFIG
		else if(conf.hdrType == eTC_HDR_TYPE_HD_1p0_MRC) {
			tchdrfwk_setHdrType(HDR_1p0_MRC_CONFIG);
			tchdrfwk_setNumOfHdrInstance(2);
		}
		else if(conf.hdrType == eTC_HDR_TYPE_HD_1p5) {
			tchdrfwk_setHdrType(HDR_1p5_CONFIG);
			tchdrfwk_setNumOfHdrInstance(2);
		}
		else if(conf.hdrType == eTC_HDR_TYPE_HD_1p5_MRC) {
			tchdrfwk_setHdrType(HDR_1p5_MRC_CONFIG);
			tchdrfwk_setNumOfHdrInstance(3);
		}
#endif
#ifdef USE_HDRLIB_2ND_CHG_VER
#if HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG
		else if(conf.hdrType == eTC_HDR_TYPE_HD_1p0_MRC) {
			tchdrfwk_setHdrType(HDR_1p0_MRC_CONFIG);
			tchdrfwk_setNumOfHdrInstance(2);
		}
		else if(conf.hdrType == eTC_HDR_TYPE_HD_1p5) {
			tchdrfwk_setHdrType(HDR_1p5_CONFIG);
			tchdrfwk_setNumOfHdrInstance(2);
		}
		else if(conf.hdrType == eTC_HDR_TYPE_HD_1p5_MRC) {
			tchdrfwk_setHdrType(HDR_1p5_MRC_CONFIG);
			tchdrfwk_setNumOfHdrInstance(3);
		}
		else if(conf.hdrType == eTC_HDR_TYPE_HD_1p5_DUAL_MRC) {
			tchdrfwk_setHdrType(HDR_1p5_DUAL_MRC_CONFIG);
			tchdrfwk_setNumOfHdrInstance(4);
		}
#endif
#endif
		else {
			tchdrfwk_setHdrType(0);
			tchdrfwk_setNumOfHdrInstance(0);
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_TYPE;

			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "The input HD Radio type[%d] is out of range supported \n", (S32)conf.hdrType);
#if HDR_CONFIG == HDR_1p0_CONFIG
			(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "Currently, HD Radio library is only available in HD 1.0 \n");
			(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "This library can support only one instance \n");
#elif HDR_CONFIG == HDR_1p0_MRC_CONFIG
			(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "Currently, HD Radio library is available up to HD 1.0 + MRC \n");
			(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "This library can support two instances \n");
#elif HDR_CONFIG == HDR_1p5_CONFIG
			(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "Currently, HD Radio library is available up to HD 1.5 \n");
			(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "This library can support two instances \n");
#elif HDR_CONFIG == HDR_1p5_MRC_CONFIG
			(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "Currently, HD Radio library is available up to HD 1.5 + MRC \n");
			(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "This library can support three instances \n");
#elif HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG
			(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "Currently, HD Radio library is available up to HD 1.5 + DUAL MRC \n");
			(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "This library can support four instances \n");
#endif
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to set type configuration\n");
		}
	}

	return ret;
}

static HDRET tchdr_checkExeStatus(stTC_HDR_CONF_t conf)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(tcid_check() < 0) {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Unsupported AP Chipset!!!\n");
		ret = (HDRET)eTC_HDR_RET_NG_INIT;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
#if HDR_CONFIG == HDR_1p0_CONFIG
		if(conf.hdrType != eTC_HDR_TYPE_HD_1p0) {
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Unsupported type. Please check hdrType configuration\n");
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
#elif HDR_CONFIG == HDR_1p0_MRC_CONFIG
		if((conf.hdrType != eTC_HDR_TYPE_HD_1p0) && (conf.hdrType != eTC_HDR_TYPE_HD_1p0_MRC)) {
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Unsupported type. Please check hdrType configuration\n");
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
#elif HDR_CONFIG == HDR_1p5_CONFIG
		if((conf.hdrType != eTC_HDR_TYPE_HD_1p0) && (conf.hdrType != eTC_HDR_TYPE_HD_1p5)) {
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Unsupported type. Please check hdrType configuration\n");
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
#elif HDR_CONFIG == HDR_1p5_MRC_CONFIG
		if((conf.hdrType != eTC_HDR_TYPE_HD_1p0) && (conf.hdrType != eTC_HDR_TYPE_HD_1p0_MRC) &&
		   (conf.hdrType != eTC_HDR_TYPE_HD_1p5) && (conf.hdrType != eTC_HDR_TYPE_HD_1p5_MRC))
		{
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Unsupported type. Please check hdrType configuration\n");
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
#else
		if((conf.hdrType != eTC_HDR_TYPE_HD_1p0) && (conf.hdrType != eTC_HDR_TYPE_HD_1p0_MRC) &&
		   (conf.hdrType != eTC_HDR_TYPE_HD_1p5) && (conf.hdrType != eTC_HDR_TYPE_HD_1p5_MRC) &&
		   (conf.hdrType != eTC_HDR_TYPE_HD_1p5_DUAL_MRC))
		{
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Unsupported type. Please check hdrType configuration\n");
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
#endif
	}

	return ret;
}

static HDRET tchdr_createManagerThread(U32 minStacksize)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	// Create thread to execute the user command and to notify
	(void)(*stOsal.osmemset)((void*)&stTcHdrManagerThdAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
	if(stTcHdrThreadPriority[eTHREAD_MANAGER].policy != 0) {
		stTcHdrManagerThdAttr.policy    = HDR_FWRK_THREADS_POLICY;
		stTcHdrManagerThdAttr.priority  = stTcHdrThreadPriority[eTHREAD_MANAGER].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
	}
	else {
		stTcHdrManagerThdAttr.policy    = SCHED_OTHER;
		stTcHdrManagerThdAttr.nice      = stTcHdrThreadPriority[eTHREAD_MANAGER].priority;
	}
	stTcHdrManagerThdAttr.stack_size    = (*stArith.u32add)(minStacksize, 0x20000U);
	stTcHdrManagerThdAttr.thread_name   = HDR_MANAGER_THREAD_NAME;

	if((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdrsvc_mainThread, &stTcHdrManagerThdAttr, NULL) < 0){
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create manager thread\n");
		ret = (HDRET)eTC_HDR_RET_NG_INIT;
	}

	return ret;
}

static HDRET tchdr_createBbinputThread(U32 minStacksize)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	// Create thread to read samples continuously from the Input stream
	(void)(*stOsal.osmemset)((void*)&stBbInputThdAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
	if(stTcHdrThreadPriority[eTHREAD_BBINPUT].policy != 0) {
		stBbInputThdAttr.policy    = HDR_FWRK_THREADS_POLICY;
		stBbInputThdAttr.priority  = stTcHdrThreadPriority[eTHREAD_BBINPUT].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
	}
	else {
		stBbInputThdAttr.policy    = SCHED_OTHER;
		stBbInputThdAttr.nice      = stTcHdrThreadPriority[eTHREAD_BBINPUT].priority;
	}
    stBbInputThdAttr.stack_size    = (*stArith.u32add)(minStacksize, 0x20000U);
	stBbInputThdAttr.thread_name   = HDR_BBINPUT_THREAD_NAME;

	if((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdr_bbInputThread, &stBbInputThdAttr, NULL) < 0){
        (*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create base band input thread\n");
    	ret = (HDRET)eTC_HDR_RET_NG_INIT;
    }

	return ret;
}

static HDRET tchdr_createFrameworkThreads(U32 minStacksize)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 hdrType = tchdrfwk_getHdrType();
	U32 threadInstanceNum[MAX_NUM_OF_INSTANCES] = {0,};

	if((hdrType == (U32)HDR_1p0_CONFIG) || (hdrType == (U32)HDR_1p0_MRC_CONFIG) ||
		(hdrType == (U32)HDR_1p5_CONFIG) || (hdrType == (U32)HDR_1p5_MRC_CONFIG) ||
		(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG))
	{
		// Primary Thread
		(void)(*stOsal.osmemset)((void*)&stPrimaryHdThdAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
		if(stTcHdrThreadPriority[eTHREAD_DEMOD].policy != 0) {
			stPrimaryHdThdAttr.policy    = HDR_FWRK_THREADS_POLICY;
			stPrimaryHdThdAttr.priority  = stTcHdrThreadPriority[eTHREAD_DEMOD].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
		}
		else {
			stPrimaryHdThdAttr.policy    = SCHED_OTHER;
			stPrimaryHdThdAttr.nice      = stTcHdrThreadPriority[eTHREAD_DEMOD].priority;
		}
		stPrimaryHdThdAttr.stack_size    = (*stArith.u32add)(minStacksize, 0x20000);
		stPrimaryHdThdAttr.thread_name   = HDR_PRIMARY_THREAD_NAME;
		threadInstanceNum[0] = 0;	// Primary Demod Thread

		if ((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdr_mainThread, &stPrimaryHdThdAttr, &threadInstanceNum[0]) < 0) {
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create MAIN HDR Execution thread\n");
			ret = (HDRET)eTC_HDR_RET_NG_INIT;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_TYPE;
	}

	// MRC Thread
#if (HDR_CONFIG == HDR_1p0_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if((hdrType == (U32)HDR_1p0_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_MRC_CONFIG) ||
			(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG))
		{
			(void)(*stOsal.osmemset)((void*)&stMrcHdThdAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
			if(stTcHdrThreadPriority[eTHREAD_DEMOD].policy != 0) {
				stMrcHdThdAttr.policy    = HDR_FWRK_THREADS_POLICY;
				stMrcHdThdAttr.priority  = stTcHdrThreadPriority[eTHREAD_DEMOD].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
			}
			else {
				stMrcHdThdAttr.policy    = SCHED_OTHER;
				stMrcHdThdAttr.nice      = stTcHdrThreadPriority[eTHREAD_DEMOD].priority;
			}
	        stMrcHdThdAttr.stack_size    = (*stArith.u32add)(minStacksize, 0x20000U);
			stMrcHdThdAttr.thread_name   = HDR_MRC_THREAD_NAME;
			threadInstanceNum[1] = 1;	// MRC Demod Thread

	        if ((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdr_mrcThread, &stMrcHdThdAttr, &threadInstanceNum[1]) < 0){
	            (*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create MRC HDR Execution thread\n");
	            ret = (HDRET)eTC_HDR_RET_NG_INIT;
	        }
		}
	}
#endif

	// Background-scan Thread
#if (HDR_CONFIG == HDR_1p5_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if((hdrType == (U32)HDR_1p5_CONFIG) || (hdrType == (U32)HDR_1p5_MRC_CONFIG) ||
			(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG))
		{
			U32 tInstanceNum;
			(void)(*stOsal.osmemset)((void*)&stBackscanHdThdAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
			if(stTcHdrThreadPriority[eTHREAD_DEMOD].policy != 0) {
				stBackscanHdThdAttr.policy    = HDR_FWRK_THREADS_POLICY;
				stBackscanHdThdAttr.priority  = stTcHdrThreadPriority[eTHREAD_DEMOD].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
			}
			else {
				stBackscanHdThdAttr.policy    = SCHED_OTHER;
				stBackscanHdThdAttr.nice      = stTcHdrThreadPriority[eTHREAD_DEMOD].priority;
			}
	        stBackscanHdThdAttr.stack_size    = (*stArith.u32add)(minStacksize, 0x20000U);
			stBackscanHdThdAttr.thread_name   = HDR_BACKSCAN_THREAD_NAME;
			if(hdrType == (U32)HDR_1p5_CONFIG) {
				tInstanceNum = 1;
			}
			else {
				tInstanceNum = 2;
			}
			threadInstanceNum[tInstanceNum] = tInstanceNum;	// BackGround-Scan Demod Thread

	        if ((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdr_bsThread, &stBackscanHdThdAttr, &threadInstanceNum[tInstanceNum]) < 0){
	            (*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create BS HDR Execution thread\n");
	            ret = (HDRET)eTC_HDR_RET_NG_INIT;
	        }
		}
	}
#endif

	// BS MRC Thread
#if (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			if(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG) {
				U32 tInstanceNum;
				(void)(*stOsal.osmemset)((void*)&stBsMrcHdThdAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
				if(stTcHdrThreadPriority[eTHREAD_DEMOD].policy != 0) {
					stBsMrcHdThdAttr.policy	  = HDR_FWRK_THREADS_POLICY;
					stBsMrcHdThdAttr.priority  = stTcHdrThreadPriority[eTHREAD_DEMOD].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
				}
				else {
					stBsMrcHdThdAttr.policy	  = SCHED_OTHER;
					stBsMrcHdThdAttr.nice	  = stTcHdrThreadPriority[eTHREAD_DEMOD].priority;
				}
				stBsMrcHdThdAttr.stack_size	  = (*stArith.u32add)(minStacksize, 0x20000U);
				stBsMrcHdThdAttr.thread_name   = HDR_BSMRC_THREAD_NAME;
				tInstanceNum = 3;
				threadInstanceNum[tInstanceNum] = tInstanceNum; // BackGround-Scan Demod Thread

				if ((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdr_bsMrcThread, &stBsMrcHdThdAttr, &threadInstanceNum[tInstanceNum]) < 0){
					(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create BS HDR Execution thread\n");
					ret = (HDRET)eTC_HDR_RET_NG_INIT;
				}
			}
		}
#endif

	// Bleding Thread
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		(void)(*stOsal.osmemset)((void*)&stAudioBlendingThdAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
		if(stTcHdrThreadPriority[eTHREAD_BLENDING].policy != 0) {
			stAudioBlendingThdAttr.policy    = HDR_FWRK_THREADS_POLICY;
			stAudioBlendingThdAttr.priority  = stTcHdrThreadPriority[eTHREAD_BLENDING].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
		}
		else {
			stAudioBlendingThdAttr.policy    = SCHED_OTHER;
			stAudioBlendingThdAttr.nice      = stTcHdrThreadPriority[eTHREAD_BLENDING].priority;
		}
		stAudioBlendingThdAttr.stack_size    = (*stArith.u32add)(minStacksize, 0x20000U);
		stAudioBlendingThdAttr.thread_name   = HDR_BLENDING_THREAD_NAME;

		if ((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdr_audioBlendingThread, &stAudioBlendingThdAttr, NULL) < 0){
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create HDR audio blending thread\n");
			ret = (HDRET)eTC_HDR_RET_NG_INIT;
		}
	}

	return ret;
}

static HDRET tchdr_createInOutThreads(U32 minStacksize)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;


	(void)(*stOsal.osmemset)((void*)&stAudioPlaybackAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
	if(stTcHdrThreadPriority[eTHREAD_AUDOUTPUT].policy != 0) {
		stAudioPlaybackAttr.policy	  = HDR_FWRK_THREADS_POLICY;
		stAudioPlaybackAttr.priority  = stTcHdrThreadPriority[eTHREAD_AUDOUTPUT].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
	}
	else {
		stAudioPlaybackAttr.policy	  = SCHED_OTHER;
		stAudioPlaybackAttr.nice	  = stTcHdrThreadPriority[eTHREAD_AUDOUTPUT].priority;
	}
	stAudioPlaybackAttr.stack_size    = (*stArith.u32add)(minStacksize, 0x10000U);
	stAudioPlaybackAttr.thread_name   = HDR_AUDOUTPUT_THREAD_NAME;

	if ((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdr_audioPlaybackThread, &stAudioPlaybackAttr, NULL) < 0){
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create audio playback thread\n");
		ret = (HDRET)eTC_HDR_RET_NG_INIT;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		(void)(*stOsal.osmemset)((void*)&stAudioInputAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
		if(stTcHdrThreadPriority[eTHREAD_AUDINPUT].policy != 0) {
			stAudioInputAttr.policy    = HDR_FWRK_THREADS_POLICY;
			stAudioInputAttr.priority  = stTcHdrThreadPriority[eTHREAD_AUDINPUT].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
		}
		else {
			stAudioInputAttr.policy    = SCHED_OTHER;
			stAudioInputAttr.nice	   = stTcHdrThreadPriority[eTHREAD_AUDINPUT].priority;
		}
		stAudioInputAttr.stack_size    = (*stArith.u32add)(minStacksize, 0x14000U);
		stAudioInputAttr.thread_name   = HDR_AUDINPUT_THREAD_NAME;

		if ((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdr_audioInputThread, &stAudioInputAttr, NULL) < 0){
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create audio input thread\n");
			ret = (HDRET)eTC_HDR_RET_NG_INIT;
		}
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		(void)(*stOsal.osmemset)((void*)&stRfIqInputAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
		if(stTcHdrThreadPriority[eTHREAD_IQINPUT].policy != 0) {
			stRfIqInputAttr.policy	  = HDR_FWRK_THREADS_POLICY;
			stRfIqInputAttr.priority  = stTcHdrThreadPriority[eTHREAD_IQINPUT].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
		}
		else {
			stRfIqInputAttr.policy	  = SCHED_OTHER;
			stRfIqInputAttr.nice	  = stTcHdrThreadPriority[eTHREAD_IQINPUT].priority;
		}
		stRfIqInputAttr.stack_size    = (*stArith.u32add)(minStacksize, 0x14000U);
		stRfIqInputAttr.thread_name   = HDR_IQINPUT_THREAD_NAME;

		if ((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdr_rfIqInputThread, &stRfIqInputAttr, NULL) < 0){
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create RF IQ input thread\n");
			ret = (HDRET)eTC_HDR_RET_NG_INIT;
		}
	}

	return ret;
}

static HDRET tchdr_createXperiThreads(U32 minStacksize)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	(void)(*stOsal.osmemset)((void*)&stCmdProcThdAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
	if(stTcHdrThreadPriority[eTHREAD_CMDPROC].policy != 0) {
		stCmdProcThdAttr.policy    = HDR_FWRK_THREADS_POLICY;
		stCmdProcThdAttr.priority  = stTcHdrThreadPriority[eTHREAD_CMDPROC].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
	}
	else {
		stCmdProcThdAttr.policy    = SCHED_OTHER;
		stCmdProcThdAttr.nice	   = stTcHdrThreadPriority[eTHREAD_CMDPROC].priority;
	}
	stCmdProcThdAttr.stack_size    = (*stArith.u32add)(minStacksize, 0x10000U);
	stCmdProcThdAttr.thread_name   = HDR_CMDPROC_THREAD_NAME;

	if ((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdr_cmdProcThread, &stCmdProcThdAttr, NULL) < 0){
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create command processor thread\n");
		ret = (HDRET)eTC_HDR_RET_NG_INIT;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		// Create logger thread
		(void)(*stOsal.osmemset)((void*)&stLoggerThdAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
		if(stTcHdrThreadPriority[eTHREAD_LOGGER].policy != 0) {
			stLoggerThdAttr.policy	  = HDR_FWRK_THREADS_POLICY;
			stLoggerThdAttr.priority  = stTcHdrThreadPriority[eTHREAD_LOGGER].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
		}
		else {
			stLoggerThdAttr.policy	  = SCHED_OTHER;
			stLoggerThdAttr.nice	  = stTcHdrThreadPriority[eTHREAD_LOGGER].priority;
		}
		stLoggerThdAttr.stack_size    = minStacksize;
		stLoggerThdAttr.thread_name   = HDR_LOGGER_THREAD_NAME;

		if ((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdr_loggerReaderThread, &stLoggerThdAttr, NULL) < 0){
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create logger thread\n");
			ret = (HDRET)eTC_HDR_RET_NG_INIT;
		}
	}

#ifdef DEBUG_ENABLE_TRACE_THREAD
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		// Create Trace logger thread
		(void)(*stOsal.osmemset)((void*)&stTraceThdAttr, (S8)0x00, (U32)sizeof(stThread_attr_t));
		if(stTcHdrThreadPriority[eTHREAD_LOGGER].policy != 0) {
			stTraceThdAttr.policy	 = HDR_FWRK_THREADS_POLICY;
			stTraceThdAttr.priority  = stTcHdrThreadPriority[eTHREAD_LOGGER].priority + (S32)DEFAULT_THREAD_PRIORITY_OFFSET;
		}
		else {
			stTraceThdAttr.policy	 = SCHED_OTHER;
			stTraceThdAttr.nice 	 = stTcHdrThreadPriority[eTHREAD_LOGGER].priority;
		}
		stTraceThdAttr.stack_size    = minStacksize;
		stTraceThdAttr.thread_name   = HDR_TRACE_THREAD_NAME;

		if ((*stOsal.oscreateThread)((FUNC_PTR_T)&tchdr_traceReaderThread, &stTraceThdAttr, NULL) < 0){
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "could not create logger thread\n");
			ret = (HDRET)eTC_HDR_RET_NG_INIT;
		}
	}
#endif

	return ret;
}

static U32 tchdr_getMinStackSize(void)
{
	size_t defStacksize;
	struct rlimit rlim;
	pthread_attr_t attr;
	U32 address_bytes = (U32)sizeof(void*);
	U32 minStacksize;

#if 0 // for priority test
	S32 iOtherPrioMin, iOtherPrioMax, iRrPrioMin, iRrPrioMax, iFifoPrioMin, iFifoPrioMax;
	iSchedType = SCHED_OTHER;
	iOtherPrioMin = sched_get_priority_min(iSchedType);
	iOtherPrioMax = sched_get_priority_max(iSchedType);
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "SCHED_OTHER schedule priority range: Min.[%d] Max.[%d]\n", iOtherPrioMin, iOtherPrioMax);

	iSchedType = SCHED_RR;
	iRrPrioMin = sched_get_priority_min(iSchedType);
	iRrPrioMax = sched_get_priority_max(iSchedType);
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "SCHED_RR schedule priority range: Min.[%d] Max.[%d]\n", iRrPrioMin, iRrPrioMax);

	iSchedType = SCHED_FIFO;
	iFifoPrioMin = sched_get_priority_min(iSchedType);
	iFifoPrioMax = sched_get_priority_max(iSchedType);
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "SCHED_FIFO schedule priority range: Min.[%d] Max.[%d]\n", iFifoPrioMin, iFifoPrioMax);
#endif

	// need to include <bits/local_lim.h>
	//(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "Minimum stack size = %d\n", (S32)PTHREAD_STACK_MIN);	// Linux - 32bit system: 16384(16KB), 64bit system: 131072(128KB)

	(void)pthread_attr_init(&attr);
	(void)pthread_attr_getstacksize (&attr, (size_t*)&defStacksize);
	(void)getrlimit((S32)RLIMIT_STACK, &rlim);
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "rlim current stack size = %lu, max stack size = %lu\n", rlim.rlim_cur, rlim.rlim_max);
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Default stack size = %lu\n", defStacksize);				// Default stack size 32bit: 8MB(Max), 64bit: 8MB(Max)
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Address size = %d\n", address_bytes);					// 32bit: 4byte, 64bit: 8byte
	if(address_bytes >= 8U) {
		// 64bit system or more
		minStacksize = (U32)PTHREAD_64BIT_STACK_MIN;
	}
	else {
		// 32bit system or less
		minStacksize = (U32)PTHREAD_32BIT_STACK_MIN;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "Minimum stack size = %d\n", minStacksize);

	return minStacksize;
}

static HDRET tchdr_createAllThreads(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 minStacksize = tchdr_getMinStackSize();

	ret = tchdr_createManagerThread(minStacksize);

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdr_createBbinputThread(minStacksize);
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdr_createFrameworkThreads(minStacksize);
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdr_createInOutThreads(minStacksize);
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdr_createXperiThreads(minStacksize);
	}

	return ret;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*                                                                                                        */
/*                                 Telechips HD Radio API                                                 */
/*                                                                                                        */
////////////////////////////////////////////////////////////////////////////////////////////////////////////
const S8 *tchdr_getFrameworkVersionString(void)
{
	return TC_HDRADIO_FRAMEWORK_VERSION_STRING;
}

const S8 *tchdr_getLibraryVersionString(void)
{
	return HDR_get_version_string();
}

static void tchdr_getLibsVersionLog(void)
{
#ifdef USE_EVALUATION_MODE
	(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "TC HD Radio framework is a trial version\n");
#endif
	(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "TC-HDR Framework: %s\n", tchdr_getFrameworkVersionString());
	(*pfnHdrLog)(eTAG_SYS, eLOG_INF, "TC-HDR Core: %s\n", HDR_get_version_string());
}

static HDRET tchdr_initThreads(void)
{
	HDRET ret;

	ret = tchdraudinput_init();				// Audio I2S input thread

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdriqinput_init();			// I/Q I2S input thread
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdrbbinput_init();			// BBinput thread
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdrfwk_init();				// Primary, MRC, Background-scan, Blending thread
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdraudoutput_init();		// Audio output thread
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdrsvc_init();				// HDR manager(service) thread
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdr_createAllThreads();
	}

	return ret;
}

HDRET tchdr_init(stTC_HDR_CONF_t conf)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	ret = tchdr_checkExeStatus(conf);

	tchdr_getLibsVersionLog();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if(tchdr_getHDRadioInitStatus() > 0U) {
			ret = (HDRET)eTC_HDR_RET_NG_ALREADY_INIT;
		}
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdr_checkConfiguration(conf);
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdr_initThreads();
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		stTcHdrConf = conf;
		tchdr_setHDRadioInitStatus(1);
	}
	else {
		if(ret == (HDRET)eTC_HDR_RET_NG_ALREADY_INIT) {
			(*pfnHdrLog)(eTAG_SYS, eLOG_WRN, "HDR already initialized.\n");
		}
		else {
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to initialize HDR. ret[%d]\n", ret);
		}
	}

	return ret;
}

static void tchdr_deinitBbinputThread(void) // for HIS metric violation (HIS_CALLS)
{
	stBbInputThdAttr.thread_running = 0;
	if(tchdriqinput_getReadySemaValue() == 0) {
		tchdriqinput_ready();
	}
}

static void tchdr_deinitAudOutputThread(void) // for HIS metric violation (HIS_CALLS)
{
	stAudioPlaybackAttr.thread_running = 0;
	if(tchdraudoutput_getReadySemaValue() == 0) {
		tchdraudoutput_ready();
	}
}

static void tchdr_deinitFwkThreads(void)         // for HIS metric violation (HIS_CALLS)
{
	U32 hdrType = tchdrfwk_getHdrType();

	stBsMrcHdThdAttr.thread_running = 0;
#if (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
	if(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG) {
		if(tchdrbbinput_getReadySemaValue(3) == 0) {
			tchdrbbinput_ready(3);
		}
	}
#endif

	stBackscanHdThdAttr.thread_running = 0;
#if (HDR_CONFIG == HDR_1p5_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
	if((hdrType == (U32)HDR_1p5_CONFIG) || (hdrType == (U32)HDR_1p5_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
		U32 instanceNum = 2;
		if(hdrType == (U32)HDR_1p5_CONFIG) {
			instanceNum = 1;
		}
		if(tchdrbbinput_getReadySemaValue(instanceNum) == 0) {
			tchdrbbinput_ready(instanceNum);
		}
	}
#endif

	stMrcHdThdAttr.thread_running = 0;
#if (HDR_CONFIG == HDR_1p0_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
	if((hdrType == (U32)HDR_1p0_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
		if(tchdrbbinput_getReadySemaValue(1) == 0) {
			tchdrbbinput_ready(1);
		}
	}
#endif

	stPrimaryHdThdAttr.thread_running = 0;
	if(tchdrbbinput_getReadySemaValue(0) == 0) {
		tchdrbbinput_ready(0);
	}

	stAudioBlendingThdAttr.thread_running = 0;
	if(tchdraudinput_getReadySemaValue() == 0) {
		tchdraudinput_ready();
	}

	UNUSED(hdrType);
}

HDRET tchdr_deinit(void)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if(tchdrsvc_getOpenStatus() == (U32)0) {
			tchdr_setHDRadioInitStatus(0);

			stAudioInputAttr.thread_running = 0;
			stRfIqInputAttr.thread_running = 0;

			// bbinput thread
			tchdr_deinitBbinputThread();
			// primary, mrc, background-scan, blending audio  threads
			tchdr_deinitFwkThreads();
			// audio output thread
			tchdr_deinitAudOutputThread();

			stCmdProcThdAttr.thread_running = 0;
			stLoggerThdAttr.thread_running = 0;
			stTcHdrManagerThdAttr.thread_running = 0;

			stBbInputThdAttr.thread_id = 0;
			stBackscanHdThdAttr.thread_id = 0;
			stMrcHdThdAttr.thread_id = 0;
			stPrimaryHdThdAttr.thread_id = 0;
			stAudioPlaybackAttr.thread_id = 0;
			stCmdProcThdAttr.thread_id = 0;
			stLoggerThdAttr.thread_id = 0;
			stTcHdrManagerThdAttr.thread_id = 0;
			stAudioBlendingThdAttr.thread_id = 0;
			stAudioInputAttr.thread_id = 0;
			stRfIqInputAttr.thread_id = 0;

			//add "time over" with emergency by using timer in thread deinit
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_NOT_YET_CLOSE;
		}
	}

	return ret;
}

static HDR_bb_src_input_rate_t tchdr_convertHdrSrcRate(eTC_HDR_BBSRC_RATE_t input)
{
	HDR_bb_src_input_rate_t ret;
	switch(input) {
		case eTC_HDR_BBSRC_650_KHZ:
			ret = HDR_BB_SRC_650_KHZ;
			break;
		case eTC_HDR_BBSRC_675_KHZ:
			ret = HDR_BB_SRC_675_KHZ;
			break;
		case eTC_HDR_BBSRC_768_KHZ:
			ret = HDR_BB_SRC_768_KHZ;
			break;
		case eTC_HDR_BBSRC_744_KHZ:
			ret = HDR_BB_SRC_744_KHZ;
			break;
		case eTC_HDR_BBSRC_1024_KHZ:
			ret = HDR_BB_SRC_1024_KHZ;
			break;
		default:
			(*pfnHdrLog)(eTAG_SYS, eLOG_WRN, "The input samplerate is not supported. It is set to 744.1875Khz.\n");
			ret = HDR_BB_SRC_744_KHZ;
			break;
	}
	return ret;
}

static HDRET tchdr_convertTuneInfo(stTCHDR_TUNE_t *tuneInfo_output, stTC_HDR_TUNE_TO_t tuneInfo_input)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(tuneInfo_output != NULL) {
		if(tuneInfo_input.band == eTC_HDR_FM_BAND) {
			tuneInfo_output->band = HDR_BAND_FM;
			tuneInfo_output->freq = (U16)((tuneInfo_input.freq/10U) & 0x0ffffU);
			tuneInfo_output->iqsr = tchdr_convertHdrSrcRate(tuneInfo_input.iqsamplerate);
			ret = tchdr_checkTune(*tuneInfo_output);
		}
		else if(tuneInfo_input.band == eTC_HDR_AM_BAND) {
			tuneInfo_output->band = HDR_BAND_AM;
			tuneInfo_output->freq = (U16)(tuneInfo_input.freq & 0x0ffffU);
			tuneInfo_output->iqsr = tchdr_convertHdrSrcRate(tuneInfo_input.iqsamplerate);
			ret = tchdr_checkTune(*tuneInfo_output);
		}
		else {
			tuneInfo_output->band = HDR_BAND_IDLE;
			if(tuneInfo_input.freq > 30000U) {	// Ref. 30MHz
				tuneInfo_output->freq = (U16)((tuneInfo_input.freq/10U) & 0x0ffffU);
			}
			else {
				tuneInfo_output->freq = (U16)(tuneInfo_input.freq & 0x0ffffU);
			}
			tuneInfo_output->iqsr = tchdr_convertHdrSrcRate(tuneInfo_input.iqsamplerate);

			// If tune band is idle when HDR opens, it should be return as error.
			// Because exactly band should be set FM or AM for core library setting and execution.
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}

		if(tuneInfo_input.iqsamplerate >= eTC_HDR_BBSRC_UNKNOWN) {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	return ret;
}

HDRET tchdr_open(stTC_HDR_TUNE_INFO_t tuneInfo)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, "[%s:%d]:\n", __func__, __LINE__);

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if(tchdrsvc_getOpenStatus() == (U32)0) {
			U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
			stTCHDR_TUNE_INFO_t hdrTuneInfo = {{HDR_BAND_AM,0,HDR_BB_SRC_744_KHZ},{HDR_BAND_AM,0,HDR_BB_SRC_744_KHZ},
											  {HDR_BAND_AM,0,HDR_BB_SRC_744_KHZ},{HDR_BAND_AM,0,HDR_BB_SRC_744_KHZ}};

			ret = tchdr_convertTuneInfo(&hdrTuneInfo.mainInstance, tuneInfo.mainTuner);
			switch(stTcHdrConf.hdrType) {
				case eTC_HDR_TYPE_HD_1p5:
					if(ret == (HDRET)eTC_HDR_RET_OK) {
						ret = tchdr_convertTuneInfo(&hdrTuneInfo.bsInstance, tuneInfo.bsTuner);
					}
					break;

				case eTC_HDR_TYPE_HD_1p0:
					UNUSED(stTcHdrConf.hdrType);
					break;

				case eTC_HDR_TYPE_HD_1p5_MRC:
					if(ret == (HDRET)eTC_HDR_RET_OK) {
						ret = tchdr_convertTuneInfo(&hdrTuneInfo.mrcInstance, tuneInfo.mrcTuner);
						if((tuneInfo.mainTuner.band != tuneInfo.mrcTuner.band) || (tuneInfo.mainTuner.freq != tuneInfo.mrcTuner.freq) ||
							(tuneInfo.mainTuner.iqsamplerate != tuneInfo.mrcTuner.iqsamplerate))
						{
							ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
						}
					}

					if(ret == (HDRET)eTC_HDR_RET_OK) {
						ret = tchdr_convertTuneInfo(&hdrTuneInfo.bsInstance, tuneInfo.bsTuner);
					}
					break;

				case eTC_HDR_TYPE_HD_1p0_MRC:
					if(ret == (HDRET)eTC_HDR_RET_OK) {
						ret = tchdr_convertTuneInfo(&hdrTuneInfo.mrcInstance, tuneInfo.mrcTuner);
						if((tuneInfo.mainTuner.band != tuneInfo.mrcTuner.band) || (tuneInfo.mainTuner.freq != tuneInfo.mrcTuner.freq) ||
							(tuneInfo.mainTuner.iqsamplerate != tuneInfo.mrcTuner.iqsamplerate))
						{
							ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
						}
					}
					break;

				case eTC_HDR_TYPE_HD_1p5_DUAL_MRC:
					if(ret == (HDRET)eTC_HDR_RET_OK) {
						ret = tchdr_convertTuneInfo(&hdrTuneInfo.mrcInstance, tuneInfo.mrcTuner);
						if((tuneInfo.mainTuner.band != tuneInfo.mrcTuner.band) || (tuneInfo.mainTuner.freq != tuneInfo.mrcTuner.freq) ||
							(tuneInfo.mainTuner.iqsamplerate != tuneInfo.mrcTuner.iqsamplerate))
						{
							ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
						}
					}

					if(ret == (HDRET)eTC_HDR_RET_OK) {
						ret = tchdr_convertTuneInfo(&hdrTuneInfo.bsInstance, tuneInfo.bsTuner);
					}

					if(ret == (HDRET)eTC_HDR_RET_OK) {
						ret = tchdr_convertTuneInfo(&hdrTuneInfo.bsmrcInstance, tuneInfo.bsmrcTuner);
						if((tuneInfo.bsTuner.band != tuneInfo.bsmrcTuner.band) || (tuneInfo.bsTuner.freq != tuneInfo.bsmrcTuner.freq) ||
							(tuneInfo.bsTuner.iqsamplerate != tuneInfo.bsmrcTuner.iqsamplerate))
						{
							ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
						}
					}
					break;

				default:
					ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
					break;
			}

			if(ret == (HDRET)eTC_HDR_RET_OK) {
				(void)(*stOsal.osmemcpy)((void*)uiSendMsg, (void*)&hdrTuneInfo, (U32)sizeof(stTCHDR_TUNE_INFO_t));
				ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_OPEN, uiSendMsg, pNULL, 0);
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_ALREADY_OPEN;
			(*pfnHdrLog)(eTAG_SYS, eLOG_WRN, "[%s:%d] HD Radio framework is already opened\n", __func__, __LINE__);
		}
	}
	else {
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s:%d]: Failed to open the TC HD Radio!!\n", __func__, __LINE__);
	}

	return ret;
}

static void tchdr_closeThreads(void)	// for HIS metric violation (HIS_CALLS)
{
	(void)tchdriqinput_close();
	(void)tchdraudoutput_close();
	(void)tchdrfwk_close();
	(void)tchdrbbinput_close();
	(void)tchdraudinput_close();
}

HDRET tchdr_close(void)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		(void)tchdrsvc_mutexLock();
		tchdrsvc_setOpenStatus(0);
		tchdr_closeThreads();
		tchdrsvc_mutexUnlock();
	}

	return ret;
}

HDRET tchdr_setTune(eTC_HDR_ID_t id, stTC_HDR_TUNE_TO_t tuneTo)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		stTCHDR_TUNE_t tune;
		U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
		if(tuneTo.band == eTC_HDR_FM_BAND) {
			tune.band = HDR_BAND_FM;
			tune.freq = (U16)((tuneTo.freq/10U) & 0x0ffffU);
			tune.iqsr = tchdr_convertHdrSrcRate(tuneTo.iqsamplerate);
			ret = tchdr_checkTune(tune);
	    }
		else if(tuneTo.band == eTC_HDR_AM_BAND) {
			tune.band = HDR_BAND_AM;
			tune.freq = (U16)(tuneTo.freq & 0x0ffffU);
			tune.iqsr = tchdr_convertHdrSrcRate(tuneTo.iqsamplerate);
			ret = tchdr_checkTune(tune);
		}
		else if(tuneTo.band == eTC_HDR_IDLE_BAND) {
			tune.band = HDR_BAND_IDLE;
			tune.freq = (U16)(tuneTo.freq & 0x0ffffU);
			tune.iqsr = tchdr_convertHdrSrcRate(tuneTo.iqsamplerate);
			ret = tchdr_checkTune(tune);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}

		if(ret == (HDRET)eTC_HDR_RET_OK) {
			uiSendMsg[0] = (U32)id;
			(void)(*stOsal.osmemcpy)((void*)&uiSendMsg[1], (void*)&tune, (U32)sizeof(stTCHDR_TUNE_t));
			ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_SET_TUNE, uiSendMsg, pNULL, 0);
		#ifdef USE_ANALOG_AUDIO_MUTE_FOR_TUNE
			if(ret == eTC_HDR_RET_OK) {
				tchdrblending_setAAMute(true);
			}
		#endif
		}
	}

	return ret;
}

HDRET tchdr_setAudioMode(eTC_HDR_AUDIO_MODE_t audioMode)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};

		switch(audioMode) {
			case eTC_HDR_AUDIO_BLEND:
				uiSendMsg[0] = (U32)eHDR_AUDIO_BLEND;
				break;
			case eTC_HDR_AUDIO_ANALOG:
				uiSendMsg[0] = (U32)eHDR_AUDIO_ANALOG_ONLY;
				break;
			case eTC_HDR_AUDIO_DIGITAL:
				uiSendMsg[0] = (U32)eHDR_AUDIO_DIGITAL_ONLY;
				break;
			case eTC_HDR_AUDIO_SPLIT:
				uiSendMsg[0] = (U32)eHDR_AUDIO_ANALOG_SPLIT;
				break;
			default:
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
				break;
		}

		if(ret == (HDRET)eTC_HDR_RET_OK) {
			ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_SET_MAIN_AUDIO_MODE, uiSendMsg, pNULL, 0);
		}
	}

	return ret;
}

HDRET tchdr_setProgram(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t numOfProgram)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdr_getHdrInstanceAddr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			ret = HDR_set_playing_program(hdrInstance, (HDR_program_t)numOfProgram);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				if(ret == (HDRET)eTC_HDR_RET_NG_UNKNOWN) {
					ret = (HDRET)eTC_HDR_RET_NG_SET_PROGRAM;
				}
			}
			else {
				(void)tchdrsvc_setProgramNumber(hdrInstance, (HDR_program_t)numOfProgram);
			}
		}
	}
	return ret;
}

HDRET tchdr_getProgram(eTC_HDR_ID_t id, eTC_HDR_PROGRAM_t *numOfProgram)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdr_getHdrInstanceAddr(id);
		if(hdrInstance == pNULL) {
			ret = (HDRET)eTC_HDR_RET_NG_INSTANCE_INIT;
		}
		else if(numOfProgram == pNULL) {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
		}
		else if(id == eTC_HDR_ID_MAIN) {
			HDR_program_t pn;
			ret = HDR_get_playing_program(hdrInstance, &pn);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				if(ret == (HDRET)eTC_HDR_RET_NG_UNKNOWN) {
					ret = (HDRET)eTC_HDR_RET_NG_GET_PROGRAM;
				}
			}
			else {
				*numOfProgram = (eTC_HDR_PROGRAM_t)pn;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}
	return ret;
}

HDRET tchdr_getSignalStatus(eTC_HDR_ID_t id, stTC_HDR_SIGNAL_STATUS_t *dataOut)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		(void)tchdrsvc_mutexLock();
		if((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) {
			ret = tchdrsvc_getHdrSignalStatus(id, dataOut);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
		tchdrsvc_mutexUnlock();
	}

	return ret;
}

HDRET tchdr_getAllStatus(eTC_HDR_ID_t id, stTC_HDR_STATUS_t *dataOut)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		(void)tchdrsvc_mutexLock();
		if((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) {
			ret = tchdrsvc_getHdrStatus(id, dataOut);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
		tchdrsvc_mutexUnlock();
	}

	return ret;
}

HDRET tchdr_getProgramType(eTC_HDR_ID_t id, stTC_HDR_PTY_t *pty)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdr_getHdrInstanceAddr(id);
		if(hdrInstance == pNULL) {
			ret = (HDRET)eTC_HDR_RET_NG_INSTANCE_INIT;
		}
		else if(pty == pNULL) {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
		}
		else if((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) {
			HDR_program_types_t srcPty;
			ret = HDR_get_program_types(hdrInstance, &srcPty);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				if(ret == (HDRET)eTC_HDR_RET_NG_UNKNOWN) {
					ret = (HDRET)eTC_HDR_RET_NG_GET_DATA;
				}
			}
			else {
				U32 i;
				for(i=0; i<(U32)eTC_HDR_PROGRAM_MAX; i++) {
					pty->value[i] = srcPty.value[i];
				}
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}

	return ret;
}

HDRET tchdr_enablePsdNotification(eTC_HDR_ID_t id, U8 progBitmask, U8 psdBitmask, U32 fEn)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
		uiSendMsg[0] = (U32)id;
		uiSendMsg[1] = (U32)progBitmask & 0x000000ffU;
		uiSendMsg[2] = (U32)psdBitmask & 0x000000ffU;
		uiSendMsg[3] = fEn;
		ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_ENABLE_GET_PSD, uiSendMsg, pNULL, 0);
	}

	return ret;
}

HDRET tchdr_enableSisNotification(eTC_HDR_ID_t id, U32 sisBitmask, U32 fEn)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
		uiSendMsg[0] = (U32)id;
		uiSendMsg[1] = sisBitmask;
		uiSendMsg[2] = fEn;
		ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_ENABLE_GET_SIS, uiSendMsg, pNULL, 0);
	}

	return ret;
}

HDRET tchdr_enableLotNotification(eTC_HDR_ID_t id, U8 progBitmask, U32 fEn)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
		uiSendMsg[0] = (U32)id;
		uiSendMsg[1] = (U32)progBitmask & 0x000000ffU;
		uiSendMsg[2] = fEn;
		ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_ENABLE_GET_LOT, uiSendMsg, pNULL, 0);
	}

	return ret;
}

HDRET tchdr_enableAlertNotification(eTC_HDR_ID_t id, U32 fEn)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
		uiSendMsg[0] = (U32)id;
		uiSendMsg[1] = fEn;
		ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_ENABLE_GET_ALERT, uiSendMsg, pNULL, 0);
	}

	return ret;
}

HDRET tchdr_setAudioMute(U32 fOnOff)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
		if(fOnOff > (U32)0) {
			uiSendMsg[0] = 1;
		}
		ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_SET_MUTE, uiSendMsg, pNULL, 0);
	}

	return ret;
}

HDRET tchdr_setAudioCtrl(U32 fStartStop)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
		if(fStartStop > (U32)0) {
			uiSendMsg[0] = 1;
		}
		ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_SET_AUDIO_CTRL, uiSendMsg, pNULL, 0);
	}

	return ret;
}

HDRET tchdr_setAnalogAudioMute(U32 fOnOff)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		(void)tchdrsvc_mutexLock();
		tchdrfwk_setAnalogAudioMute(fOnOff);
		tchdrsvc_mutexUnlock();
	}

	return ret;
}

HDRET tchdr_setAudioMuteFader(U32 enable, U32 fadein_ms, U32 fadeout_ms)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		stFADER_PARAMS_t faderParams;
		(void)tchdrsvc_mutexLock();
		if(enable > (U32)0) {
			faderParams.bFaderEn = 1;
		}
		else {
			faderParams.bFaderEn = 0;
		}
		if(fadein_ms <= (U32)1000) {
			faderParams.mute.fadeinTime = fadein_ms;
		}
		else{
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		//	(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to set fadein_ms because It's over limit(0ms ~ 1000ms). It's set to 1000ms.\n");
		}
		if(fadeout_ms <= (U32)1000) {
			faderParams.mute.fadeoutTime = fadeout_ms;
		}
		else{
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		//	(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to set fadeout_ms because It's over limit(0ms ~ 1000ms). It's set to 1000ms.\n");
		}
		faderParams.samplingRate = 44100;

		if(ret == (HDRET)eTC_HDR_RET_OK) {
			ret = tchdrfader_setParams(faderParams);
		}
		tchdrsvc_mutexUnlock();
	}
	return ret;
}

HDRET tchdr_getAudioMuteFader(U32 *enable, U32 *fadein_ms, U32 *fadeout_ms)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if((enable != NULL) && (fadein_ms != NULL) && (fadeout_ms != NULL)) {
			stFADER_PARAMS_t faderParams;
			(void)tchdrsvc_mutexLock();
			tchdrfader_getParams(&faderParams);
			*enable = faderParams.bFaderEn;
			*fadein_ms = faderParams.mute.fadeinTime;
			*fadeout_ms = faderParams.mute.fadeoutTime;
			tchdrsvc_mutexUnlock();
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
		}
	}
	return ret;
}

HDRET tchdr_resetBBP(U32 mainOfBs)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
		if(mainOfBs > (U32)0) {
			uiSendMsg[0] = 1;
		}
		ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_RESET_BBP, uiSendMsg, pNULL, 0);
	}

	return ret;
}

static HDRET tchdr_enableMRC(const HDR_instance_t * hdr_instance, U32 fOnOff)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	S32 hdret;
	HDBOOL mrc_enabled;
	HDR_tune_band_t curBand = HDR_get_band_select(hdr_instance);

	if(curBand == HDR_BAND_FM) {
		hdret = HDR_mrc_enabled(hdr_instance, &mrc_enabled);
		if(hdret == 0) {
			if(fOnOff == (U32)1) { // ON
				if(mrc_enabled == false) {
					hdret = HDR_mrc_enable(hdr_instance);
					if(hdret != 0) {
						if(hdret == -1) {
							ret = (HDRET)eTC_HDR_RET_NG_RSC;
						}
						else {
							ret = tchdr_convertHdrError((HDR_error_code_t)hdret);
						}
					}
				}
			}
			else if(fOnOff == (U32)0) { // OFF
				if(mrc_enabled == true) {
					hdret = HDR_mrc_disable(hdr_instance);
					if(hdret != 0) {
						if(hdret == -1) {
							ret = (HDRET)eTC_HDR_RET_NG_RSC;
						}
						else {
							ret = tchdr_convertHdrError((HDR_error_code_t)hdret);
						}
					}
				}
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else if(hdret == -1) {
			ret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
		else {
			ret = tchdr_convertHdrError((HDR_error_code_t)hdret);
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_BAND;
	}

	return ret;
}

HDRET tchdr_setMRC(U32 fOnOff)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	U32 hdrType = tchdrfwk_getHdrType();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if((hdrType == (U32)HDR_1p0_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
			const HDR_instance_t *hdrMainInstance = tchdr_getHdrInstanceAddr(eTC_HDR_ID_MAIN);
			ret = tchdr_enableMRC(hdrMainInstance, fOnOff);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_TYPE;
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Not support to MRC feature in your configured HD Radio.\n");
		}
	}

	return ret;
}

HDRET tchdr_setBsMRC(U32 fOnOff)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	U32 hdrType = tchdrfwk_getHdrType();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG) {
			const HDR_instance_t *hdrBsInstance = tchdr_getHdrInstanceAddr(eTC_HDR_ID_BS);
			ret = tchdr_enableMRC(hdrBsInstance, fOnOff);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_TYPE;
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Not support to BS MRC feature in your configured HD Radio.\n");
		}
	}

	return ret;
}

HDRET tchdr_getAvailablePrograms(eTC_HDR_ID_t id, stTC_HDR_PROG_BITMAP_t *availablePrograms)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrInstance = tchdr_getHdrInstanceAddr(id);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		if(((id == eTC_HDR_ID_MAIN) || (id == eTC_HDR_ID_BS)) && (hdrInstance != pNULL)) {
			if(availablePrograms != NULL) {
				HDR_program_bitmap_t hdrAvailablePrograms;
				hdrAvailablePrograms.all = 0U;
				ret = HDR_get_available_programs(hdrInstance, &hdrAvailablePrograms);
				if(ret != 0) {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
					if(ret == (HDRET)eTC_HDR_RET_NG_UNKNOWN) {
						ret = (HDRET)eTC_HDR_RET_NG_GET_DATA;
					}
				}
				else {
					availablePrograms->all = hdrAvailablePrograms.all;
				}
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
			}
		}
	}
	return ret;
}

HDRET tchdr_setAudioResamplerSlips(U32 fInOut, F64 ppm, F64 *out_hz)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if(out_hz != NULL) {
			F64 *outhz = out_hz;
			if(fInOut == 0U) {		// 0 is input resampler
				*outhz = tchdraudinput_setResamplerSlips(ppm);
			}
		#ifdef USE_AUDIO_OUTPUT_RESAMPLER
			else if(fInOut == 1U) {	// 1 is output resampler
				*outhz = tchdraudoutput_setResamplerSlips(ppm);
			}
		#endif
			else {
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
		}
	}
	return ret;
}

HDRET tchdr_getAudioResamplerSlips(U32 fInOut, F64 *ppm, F64 *out_hz)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if((ppm != NULL) && (out_hz != NULL)) {
			if(fInOut == 0U) {		// 0 is input resampler
				tchdraudinput_getResamplerSlips(ppm, out_hz);
			}
		#ifdef USE_AUDIO_OUTPUT_RESAMPLER
			else if(fInOut == 1U) {	// 1 is output resampler
				tchdraudoutput_getResamplerSlips(ppm, out_hz);
			}
		#endif
			else {
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
		}
	}
	return ret;
}

HDRET tchdr_setDigitalAudioSlips(S32 clkOffset)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdrfwk_setExtnalClockOffset(clkOffset);
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			(*pfnHdrLog)(eTAG_BLD, eLOG_DBG, "Successfully set digital audio clock offset(0x%08x)\n", clkOffset);
		}
		else {
			(*pfnHdrLog)(eTAG_BLD, eLOG_ERR, "Faild to set digital audio clock offset\n");
		}
	}
	return ret;
}

HDRET tchdr_getDigitalAudioSlips(S32 *clkOffset)	// Q16.16 format
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if(clkOffset != NULL) {
			clkOffset = tchdrfwk_getExtnalClockOffset();
			(*pfnHdrLog)(eTAG_BLD, eLOG_DBG, "Digital audio clock offset is 0x%08x\n", clkOffset);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
		}
	}
	return ret;
}

HDRET tchdr_setBlendTransitionTime(U32 transition_time)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		const stHDR_FRAMEWORK_DATA_t* frameworkData = tchdrfwk_getDataStructPtr();
		ret = HDR_set_blend_transition_time(frameworkData->blendCrossfade, transition_time);
		if(ret != 0) {
			ret = (HDRET)eTC_HDR_RET_NG_SET_VALUE;
		}
	}
	return ret;
}

HDRET tchdr_setBlendAllParams(stTC_HDR_BLEND_PARAMS_t params)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrMainInstance = tchdr_getHdrInstanceAddr(eTC_HDR_ID_MAIN);
		if(hdrMainInstance != NULL) {
			HDR_blend_params_t userparams;
			(void)(*stOsal.osmemcpy)(&userparams, &params, (U32)sizeof(HDR_blend_params_t));
			userparams.fm_mps_dig_audio_delay = (*stArith.u32add)(userparams.fm_mps_dig_audio_delay, (U32)40000);
			ret = HDR_blend_set_all_params(hdrMainInstance, &userparams);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
				}
				else if(ret == -2) {
					(*pfnHdrLog)(eTAG_BLD, eLOG_ERR, "Faild to modify write-protected parameter(s) while not in INIT state or IDLE mode.\n");
					ret = (HDRET)eTC_HDR_RET_NG_SET_VALUE;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
					if(ret == (HDRET)eTC_HDR_RET_NG_UNKNOWN) {
						ret = (HDRET)eTC_HDR_RET_NG_SET_VALUE;
					}
				}
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_INSTANCE;
		}
	}
	return ret;
}

HDRET tchdr_getBlendAllParams(stTC_HDR_BLEND_PARAMS_t *params)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		if(params != NULL) {
			HDR_instance_t *hdrMainInstance = tchdr_getHdrInstanceAddr(eTC_HDR_ID_MAIN);
			if(hdrMainInstance != NULL) {
				HDR_blend_params_t blendParams;
				(void)(*stOsal.osmemset)((void*)&blendParams, (S8)0, (U32)sizeof(HDR_blend_params_t));
				ret = HDR_blend_get_all_params(hdrMainInstance, &blendParams);
				if(ret != 0) {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
					if(ret == (HDRET)eTC_HDR_RET_NG_UNKNOWN) {
						ret = (HDRET)eTC_HDR_RET_NG_GET_DATA;
					}
				}
				else {
					if(blendParams.fm_mps_dig_audio_delay >= 40000U) {
						blendParams.fm_mps_dig_audio_delay -= 40000U;
						(void)(*stOsal.osmemcpy)((void*)params, (void*)&blendParams, (U32)sizeof(stTC_HDR_BLEND_PARAMS_t));
					}
					else {
						ret = (HDRET)eTC_HDR_RET_NG_GET_DATA;
						(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Detected to get a wrong  fm_mps_dig_audio_delay value.\n");
					}
				}
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_NULL_INSTANCE;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
		}
	}
	return ret;
}

HDRET tchdr_setBlendParam(eTC_HDR_BLEND_PARAMS_t param, U32 param_value)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrMainInstance = tchdr_getHdrInstanceAddr(eTC_HDR_ID_MAIN);
		if(hdrMainInstance != NULL){
			switch(param) {
				case eBLEND_FM_MPS_BLEND_THRESH:
					//ret = HDR_blend_set_param(hdrMainInstance, fm_mps_blend_thresh, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_mps_blend_thresh), (U32)sizeof(((HDR_blend_params_t *) 0)->fm_mps_blend_thresh), param_value);
					break;
				case eBLEND_FM_ALL_DIG_BLEND_THRESH:
					//ret = HDR_blend_set_param(hdrMainInstance, fm_all_dig_blend_thresh, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_all_dig_blend_thresh), (U32)sizeof(((HDR_blend_params_t *) 0)->fm_all_dig_blend_thresh), param_value);
					break;
				case eBLEND_FM_MPS_AUDIO_SCALING:
					//ret = HDR_blend_set_param(hdrMainInstance, fm_mps_audio_scaling, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_mps_audio_scaling), (U32)sizeof(((HDR_blend_params_t *) 0)->fm_mps_audio_scaling), param_value);
					break;
				case eBLEND_FM_ALL_DAUD_SCALLING:
					//ret = HDR_blend_set_param(hdrMainInstance, fm_all_dig_audio_scaling, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_all_dig_audio_scaling), (U32)sizeof(((HDR_blend_params_t *) 0)->fm_all_dig_audio_scaling), param_value);
					break;
				case eBLEND_FM_MPS_BLEND_RATE:
					//ret = HDR_blend_set_param(hdrMainInstance, fm_mps_blend_rate, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_mps_blend_rate), (U32)sizeof(((HDR_blend_params_t *) 0)->fm_mps_blend_rate), param_value);
					break;
				case eBLEND_FM_ALL_DIG_BLEND_RATE:
					//ret = HDR_blend_set_param(hdrMainInstance, fm_all_dig_blend_rate, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_all_dig_blend_rate), (U32)sizeof(((HDR_blend_params_t *) 0)->fm_all_dig_blend_rate), param_value);
					break;
				case eBLEND_FM_MPS_DAUD_DELAY:
					//ret = HDR_blend_set_param(hdrMainInstance, fm_mps_dig_audio_delay, param_value+(U32)40000);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_mps_dig_audio_delay), (U32)sizeof(((HDR_blend_params_t *) 0)->fm_mps_dig_audio_delay), (*stArith.u32add)(param_value, (U32)40000));
					break;
				case eBLEND_AM_MPS_BLEND_THRESH:
					//ret = HDR_blend_set_param(hdrMainInstance, am_mps_blend_thresh, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_mps_blend_thresh), (U32)sizeof(((HDR_blend_params_t *) 0)->am_mps_blend_thresh), param_value);
					break;
				case eBLEND_AM_ALL_DIG_BLEND_THRESH:
					//ret = HDR_blend_set_param(hdrMainInstance, am_all_dig_blend_thresh, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_all_dig_blend_thresh), (U32)sizeof(((HDR_blend_params_t *) 0)->am_all_dig_blend_thresh), param_value);
					break;
				case eBLEND_AM_MPS_AUDIO_SCALING:
					//ret = HDR_blend_set_param(hdrMainInstance, am_mps_audio_scaling, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_mps_audio_scaling), (U32)sizeof(((HDR_blend_params_t *) 0)->am_mps_audio_scaling), param_value);
					break;
				case eBLEND_AM_ALL_DAUD_SCALING:
					//ret = HDR_blend_set_param(hdrMainInstance, am_all_dig_audio_scaling, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_all_dig_audio_scaling), (U32)sizeof(((HDR_blend_params_t *) 0)->am_all_dig_audio_scaling), param_value);
					break;
				case eBLEND_AM_MPS_DAUD_DELAY:
					//ret = HDR_blend_set_param(hdrMainInstance, am_mps_dig_audio_delay, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_mps_dig_audio_delay), (U32)sizeof(((HDR_blend_params_t *) 0)->am_mps_dig_audio_delay), param_value);
					break;
				case eBLEND_AM_MPS_BLEND_RATE:
					//ret = HDR_blend_set_param(hdrMainInstance, am_mps_blend_rate, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_mps_blend_rate), (U32)sizeof(((HDR_blend_params_t *) 0)->am_mps_blend_rate), param_value);
					break;
				case eBLEND_AM_ALL_DIG_BLEND_RATE:
					//ret = HDR_blend_set_param(hdrMainInstance, am_all_dig_blend_rate, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_all_dig_blend_rate), (U32)sizeof(((HDR_blend_params_t *) 0)->am_all_dig_blend_rate), param_value);
					break;
				case eBLEND_D2A_BLEND_HOLDOFF:
					//ret = HDR_blend_set_param(hdrMainInstance, d2a_blend_holdoff, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, d2a_blend_holdoff), (U32)sizeof(((HDR_blend_params_t *) 0)->d2a_blend_holdoff), param_value);
					break;
				case eBLEND_BLEND_DECISION:
					//ret = HDR_blend_set_param(hdrMainInstance, blend_decision, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, blend_decision), (U32)sizeof(((HDR_blend_params_t *) 0)->blend_decision), param_value);
					break;
				case eBLEND_FM_CDNO_BLEND_DECISION:
					//ret = HDR_blend_set_param(hdrMainInstance, fm_cdno_blend_decision, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_cdno_blend_decision), (U32)sizeof(((HDR_blend_params_t *) 0)->fm_cdno_blend_decision), param_value);
					break;
				case eBLEND_AM_CDNO_BLEND_DECISION:
					//ret = HDR_blend_set_param(hdrMainInstance, am_cdno_blend_decision, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_cdno_blend_decision), (U32)sizeof(((HDR_blend_params_t *) 0)->am_cdno_blend_decision), param_value);
					break;
				case eBLEND_FM_AUDIO_INVERT_PHASE:
					//ret = HDR_blend_set_param(hdrMainInstance, fm_audio_invert_phase, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_audio_invert_phase), (U32)sizeof(((HDR_blend_params_t *) 0)->fm_audio_invert_phase), param_value);
					break;
				case eBLEND_AM_AUDIO_INVERT_PHASE:
					//ret = HDR_blend_set_param(hdrMainInstance, am_audio_invert_phase, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_audio_invert_phase), (U32)sizeof(((HDR_blend_params_t *) 0)->am_audio_invert_phase), param_value);
					break;
				case eBLEND_DISABLE_AUDIO_SCALING:
					//ret = HDR_blend_set_param(hdrMainInstance, disable_audio_scaling, param_value);
					ret = HDR_blend_set_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, disable_audio_scaling), (U32)sizeof(((HDR_blend_params_t *) 0)->disable_audio_scaling), param_value);
					break;
				default:
					ret = (HDRET)eTC_HDR_RET_NG_SET_VALUE;
					break;
			}
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
				}
				else if(ret == -2) {
					(*pfnHdrLog)(eTAG_BLD, eLOG_ERR, "Faild to modify write-protected parameter while not in INIT state or IDLE mode.\n");
					ret = (HDRET)eTC_HDR_RET_NG_SET_VALUE;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
					if(ret == (HDRET)eTC_HDR_RET_NG_UNKNOWN) {
						ret = (HDRET)eTC_HDR_RET_NG_SET_VALUE;
					}
				}
			}
		} else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_INSTANCE;
		}
	}
	return ret;
}

HDRET tchdr_getBlendParam(eTC_HDR_BLEND_PARAMS_t param, U32 *param_value)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrMainInstance = tchdr_getHdrInstanceAddr(eTC_HDR_ID_MAIN);
		if(hdrMainInstance == NULL){
			ret = (HDRET)eTC_HDR_RET_NG_NULL_INSTANCE;
		} else if(param_value == NULL){
			ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
		} else {
			switch(param) {
				case eBLEND_FM_MPS_BLEND_THRESH:
					//ret = HDR_blend_get_param(hdrMainInstance, fm_mps_blend_thresh, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_mps_blend_thresh), (U32)sizeof(((HDR_blend_params_t *)0)->fm_mps_blend_thresh), param_value);
					break;
				case eBLEND_FM_ALL_DIG_BLEND_THRESH:
					//ret = HDR_blend_get_param(hdrMainInstance, fm_all_dig_blend_thresh, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_all_dig_blend_thresh), (U32)sizeof(((HDR_blend_params_t *)0)->fm_all_dig_blend_thresh), param_value);
					break;
				case eBLEND_FM_MPS_AUDIO_SCALING:
					//ret = HDR_blend_get_param(hdrMainInstance, fm_mps_audio_scaling, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_mps_audio_scaling), (U32)sizeof(((HDR_blend_params_t *)0)->fm_mps_audio_scaling), param_value);
					break;
				case eBLEND_FM_ALL_DAUD_SCALLING:
					//ret = HDR_blend_get_param(hdrMainInstance, fm_all_dig_audio_scaling, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_all_dig_audio_scaling), (U32)sizeof(((HDR_blend_params_t *)0)->fm_all_dig_audio_scaling), param_value);
					break;
				case eBLEND_FM_MPS_BLEND_RATE:
					//ret = HDR_blend_get_param(hdrMainInstance, fm_mps_blend_rate, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_mps_blend_rate), (U32)sizeof(((HDR_blend_params_t *)0)->fm_mps_blend_rate), param_value);
					break;
				case eBLEND_FM_ALL_DIG_BLEND_RATE:
					//ret = HDR_blend_get_param(hdrMainInstance, fm_all_dig_blend_rate, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_all_dig_blend_rate), (U32)sizeof(((HDR_blend_params_t *)0)->fm_all_dig_blend_rate), param_value);
					break;
				case eBLEND_FM_MPS_DAUD_DELAY:
					//ret = HDR_blend_get_param(hdrMainInstance, fm_mps_dig_audio_delay, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_mps_dig_audio_delay), (U32)sizeof(((HDR_blend_params_t *)0)->fm_mps_dig_audio_delay), param_value);
					*param_value = (*stArith.u32sub)(*param_value, (U32)40000);
					break;
				case eBLEND_AM_MPS_BLEND_THRESH:
					//ret = HDR_blend_get_param(hdrMainInstance, am_mps_blend_thresh, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_mps_blend_thresh), (U32)sizeof(((HDR_blend_params_t *)0)->am_mps_blend_thresh), param_value);
					break;
				case eBLEND_AM_ALL_DIG_BLEND_THRESH:
					//ret = HDR_blend_get_param(hdrMainInstance, am_all_dig_blend_thresh, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_all_dig_blend_thresh), (U32)sizeof(((HDR_blend_params_t *)0)->am_all_dig_blend_thresh), param_value);
					break;
				case eBLEND_AM_MPS_AUDIO_SCALING:
					//ret = HDR_blend_get_param(hdrMainInstance, am_mps_audio_scaling, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_mps_audio_scaling), (U32)sizeof(((HDR_blend_params_t *)0)->am_mps_audio_scaling), param_value);
					break;
				case eBLEND_AM_ALL_DAUD_SCALING:
					//ret = HDR_blend_get_param(hdrMainInstance, am_all_dig_audio_scaling, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_all_dig_audio_scaling), (U32)sizeof(((HDR_blend_params_t *)0)->am_all_dig_audio_scaling), param_value);
					break;
				case eBLEND_AM_MPS_DAUD_DELAY:
					//ret = HDR_blend_get_param(hdrMainInstance, am_mps_dig_audio_delay, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_mps_dig_audio_delay), (U32)sizeof(((HDR_blend_params_t *)0)->am_mps_dig_audio_delay), param_value);
					break;
				case eBLEND_AM_MPS_BLEND_RATE:
					//ret = HDR_blend_get_param(hdrMainInstance, am_mps_blend_rate, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_mps_blend_rate), (U32)sizeof(((HDR_blend_params_t *)0)->am_mps_blend_rate), param_value);
					break;
				case eBLEND_AM_ALL_DIG_BLEND_RATE:
					//ret = HDR_blend_get_param(hdrMainInstance, am_all_dig_blend_rate, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_all_dig_blend_rate), (U32)sizeof(((HDR_blend_params_t *)0)->am_all_dig_blend_rate), param_value);
					break;
				case eBLEND_D2A_BLEND_HOLDOFF:
					//ret = HDR_blend_get_param(hdrMainInstance, d2a_blend_holdoff, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, d2a_blend_holdoff), (U32)sizeof(((HDR_blend_params_t *)0)->d2a_blend_holdoff), param_value);
					break;
				case eBLEND_BLEND_DECISION:
					//ret = HDR_blend_get_param(hdrMainInstance, blend_decision, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, blend_decision), (U32)sizeof(((HDR_blend_params_t *)0)->blend_decision), param_value);
					break;
				case eBLEND_FM_CDNO_BLEND_DECISION:
					//ret = HDR_blend_get_param(hdrMainInstance, fm_cdno_blend_decision, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_cdno_blend_decision), (U32)sizeof(((HDR_blend_params_t *)0)->fm_cdno_blend_decision), param_value);
					break;
				case eBLEND_AM_CDNO_BLEND_DECISION:
					//ret = HDR_blend_get_param(hdrMainInstance, am_cdno_blend_decision, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_cdno_blend_decision), (U32)sizeof(((HDR_blend_params_t *)0)->am_cdno_blend_decision), param_value);
					break;
				case eBLEND_FM_AUDIO_INVERT_PHASE:
					//ret = HDR_blend_get_param(hdrMainInstance, fm_audio_invert_phase, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, fm_audio_invert_phase), (U32)sizeof(((HDR_blend_params_t *)0)->fm_audio_invert_phase), param_value);
					break;
				case eBLEND_AM_AUDIO_INVERT_PHASE:
					//ret = HDR_blend_get_param(hdrMainInstance, am_audio_invert_phase, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, am_audio_invert_phase), (U32)sizeof(((HDR_blend_params_t *)0)->am_audio_invert_phase), param_value);
					break;
				case eBLEND_DISABLE_AUDIO_SCALING:
					//ret = HDR_blend_get_param(hdrMainInstance, disable_audio_scaling, param_value);
					ret = HDR_blend_get_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_params_t, disable_audio_scaling), (U32)sizeof(((HDR_blend_params_t *)0)->disable_audio_scaling), param_value);
					break;
				default:
					ret = (HDRET)eTC_HDR_RET_NG_GET_DATA;
					break;
			}

			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				if(ret == (HDRET)eTC_HDR_RET_NG_UNKNOWN) {
					ret = (HDRET)eTC_HDR_RET_NG_GET_DATA;
				}
			}
		}
	}
	return ret;
}

HDRET tchdr_setBlendAllAdvParams(stTC_HDR_BLEND_ADV_PARAMS_t params)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrMainInstance = tchdr_getHdrInstanceAddr(eTC_HDR_ID_MAIN);
		if(hdrMainInstance != NULL){
			HDR_blend_adv_params_t hdrParams;
			(void)(*stOsal.osmemcpy)(&hdrParams, &params, (U32)sizeof(HDR_blend_adv_params_t));
			ret = HDR_blend_set_all_adv_params(hdrMainInstance, &hdrParams);
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
					if(ret == (HDRET)eTC_HDR_RET_NG_UNKNOWN) {
						ret = (HDRET)eTC_HDR_RET_NG_SET_VALUE;
					}
				}
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_INSTANCE;
		}
	}
	return ret;
}

HDRET tchdr_getBlendAllAdvParams(stTC_HDR_BLEND_ADV_PARAMS_t *params)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrMainInstance = tchdr_getHdrInstanceAddr(eTC_HDR_ID_MAIN);
		if(hdrMainInstance != NULL){
			HDR_blend_adv_params_t hdrParams;
			(void)(*stOsal.osmemset)((void*)&hdrParams, (S8)0, (U32)sizeof(HDR_blend_adv_params_t));
			ret = HDR_blend_get_all_adv_params(hdrMainInstance, &hdrParams);
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				if(ret == (HDRET)eTC_HDR_RET_NG_UNKNOWN) {
					ret = (HDRET)eTC_HDR_RET_NG_GET_DATA;
				}
			}
			else {
				(void)(*stOsal.osmemcpy)((void*)params, (void*)&hdrParams, (U32)sizeof(stTC_HDR_BLEND_ADV_PARAMS_t));
			}
		} else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_INSTANCE;
		}
	}
	return ret;
}

HDRET tchdr_setBlendAdvParam(eTC_HDR_BLEND_ADV_PARAMS_t param, U32 param_value)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	S32 set_value = (*stCast.u32tos32)(param_value);
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrMainInstance = tchdr_getHdrInstanceAddr(eTC_HDR_ID_MAIN);
		if(hdrMainInstance != NULL){
			switch(param) {
				case eBLEND_ADV_RAMP_UP_ENABLED:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, ramp_up_enabled, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, ramp_up_enabled), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->ramp_up_enabled), set_value);
					break;
				case eBLEND_ADV_RAMP_UP_TIME:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, ramp_up_time, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, ramp_up_time), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->ramp_up_time), set_value);
					break;
				case eBLEND_ADV_RAMP_DOWN_ENABLED:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, ramp_down_enabled, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, ramp_down_enabled), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->ramp_down_enabled), set_value);
					break;
				case eBLEND_ADV_RAMP_DOWN_TIME:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, ramp_down_time, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, ramp_down_time), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->ramp_down_time), set_value);
					break;
				case eBLEND_ADV_COMFORT_NOISE_ENABLED:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, comfort_noise_enabled, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, comfort_noise_enabled), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->comfort_noise_enabled), set_value);
					break;
				case eBLEND_ADV_COMFORT_NOISE_LEVEL:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, comfort_noise_level, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, comfort_noise_level), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->comfort_noise_level), set_value);
					break;
				case eBLEND_ADV_AM_ENH_STREAM_HOLDOFF_ENABLED:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_enh_stream_holdoff_enabled, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_enh_stream_holdoff_enabled), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_enh_stream_holdoff_enabled), set_value);
					break;
				case eBLEND_ADV_AM_MPS_ENH_STREAM_HOLDOFF_THRESH:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_mps_enh_stream_holdoff_thresh, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_mps_enh_stream_holdoff_thresh), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_mps_enh_stream_holdoff_thresh), set_value);
					break;
				case eBLEND_ADV_ALL_DIG_ENH_STREAM_HOLDOFF_THRESH:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_all_dig_enh_stream_holdoff_thresh, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_all_dig_enh_stream_holdoff_thresh), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_all_dig_enh_stream_holdoff_thresh), set_value);
					break;
				case eBLEND_ADV_AM_DAUD_BW_MGMT_ENABLED:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_dig_audio_bw_mgmt_enabled, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_bw_mgmt_enabled), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_bw_mgmt_enabled), set_value);
					break;
				case eBLEND_ADV_AM_DAUD_BLEND_START_BW:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_dig_audio_blend_start_bw, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_blend_start_bw), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_blend_start_bw), set_value);
					break;
				case eBLEND_ADV_AM_DAUD_MAX_BW:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_dig_audio_max_bw, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_max_bw), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_max_bw), set_value);
					break;
				case eBLEND_ADV_AM_DAUD_BW_STEP_TIME:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_dig_audio_bw_step_time, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_bw_step_time), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_bw_step_time), set_value);
					break;
				case eBLEND_ADV_AM_DAUD_BW_STEP_UP_SIZE:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_dig_audio_bw_step_up_size, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_bw_step_up_size), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_bw_step_up_size), set_value);
					break;
				case eBLEND_ADV_AM_DAUD_BW_STEP_DOWN_SIZE:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_dig_audio_bw_step_down_size, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_bw_step_down_size), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_bw_step_down_size), set_value);
					break;
				case eBLEND_ADV_AM_DAUD_BW_STEP_THRESHOLD:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_dig_audio_bw_step_threshold, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_bw_step_threshold), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_bw_step_threshold), set_value);
					break;
				case eBLEND_ADV_AM_MONO2STEREO_ENABLED:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_mono2stereo_enabled, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_mono2stereo_enabled), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_mono2stereo_enabled), set_value);
					break;
				case eBLEND_ADV_AM_MONO2STEREO_START_BW:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_mono2stereo_start_bw, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_mono2stereo_start_bw), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_mono2stereo_start_bw), set_value);
					break;
				case eBLEND_ADV_AM_MONO2STEREO_STEP_TIME:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_mono2stereo_step_time, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_mono2stereo_step_time), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_mono2stereo_step_time), set_value);
					break;
				case eBLEND_ADV_AM_MONO2STEREO_MAX_SEP:
					//ret = HDR_blend_set_adv_param(hdrMainInstance, am_mono2stereo_max_sep, param_value);
					ret = HDR_blend_set_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_mono2stereo_max_sep), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_mono2stereo_max_sep), set_value);
					break;
				default:
					ret = (HDRET)eTC_HDR_RET_NG_SET_VALUE;
					break;
			}
			if(ret != 0) {
				if(ret == -1) {
					ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
				}
				else {
					ret = tchdr_convertHdrError((HDR_error_code_t)ret);
					if(ret == (HDRET)eTC_HDR_RET_NG_UNKNOWN) {
						ret = (HDRET)eTC_HDR_RET_NG_SET_VALUE;
					}
				}
			}
		} else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_INSTANCE;
		}
	}
	return ret;
}

HDRET tchdr_getBlendAdvParam(eTC_HDR_BLEND_ADV_PARAMS_t param, U32 *param_value)
{
	HDRET ret = tchdr_checkHDRadioInitStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		HDR_instance_t *hdrMainInstance = tchdr_getHdrInstanceAddr(eTC_HDR_ID_MAIN);
		if(hdrMainInstance != NULL){
			switch(param) {
				case eBLEND_ADV_RAMP_UP_ENABLED:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, ramp_up_enabled, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, ramp_up_enabled), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->ramp_up_enabled), param_value);
					break;
				case eBLEND_ADV_RAMP_UP_TIME:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, ramp_up_time, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, ramp_up_time), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->ramp_up_time), param_value);
					break;
				case eBLEND_ADV_RAMP_DOWN_ENABLED:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, ramp_down_enabled, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, ramp_down_enabled), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->ramp_down_enabled), param_value);
					break;
				case eBLEND_ADV_RAMP_DOWN_TIME:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, ramp_down_time, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, ramp_down_time), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->ramp_down_time), param_value);
					break;
				case eBLEND_ADV_COMFORT_NOISE_ENABLED:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, comfort_noise_enabled, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, comfort_noise_enabled), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->comfort_noise_enabled), param_value);
					break;
				case eBLEND_ADV_COMFORT_NOISE_LEVEL:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, comfort_noise_level, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, comfort_noise_level), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->comfort_noise_level), param_value);
					break;
				case eBLEND_ADV_AM_ENH_STREAM_HOLDOFF_ENABLED:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_enh_stream_holdoff_enabled, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_enh_stream_holdoff_enabled), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_enh_stream_holdoff_enabled), param_value);
					break;
				case eBLEND_ADV_AM_MPS_ENH_STREAM_HOLDOFF_THRESH:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_mps_enh_stream_holdoff_thresh, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_mps_enh_stream_holdoff_thresh), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_mps_enh_stream_holdoff_thresh), param_value);
					break;
				case eBLEND_ADV_ALL_DIG_ENH_STREAM_HOLDOFF_THRESH:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_all_dig_enh_stream_holdoff_thresh, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_all_dig_enh_stream_holdoff_thresh), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_all_dig_enh_stream_holdoff_thresh), param_value);
					break;
				case eBLEND_ADV_AM_DAUD_BW_MGMT_ENABLED:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_dig_audio_bw_mgmt_enabled, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_bw_mgmt_enabled), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_bw_mgmt_enabled), param_value);
					break;
				case eBLEND_ADV_AM_DAUD_BLEND_START_BW:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_dig_audio_blend_start_bw, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_blend_start_bw), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_blend_start_bw), param_value);
					break;
				case eBLEND_ADV_AM_DAUD_MAX_BW:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_dig_audio_max_bw, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_max_bw), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_max_bw), param_value);
					break;
				case eBLEND_ADV_AM_DAUD_BW_STEP_TIME:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_dig_audio_bw_step_time, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_bw_step_time), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_bw_step_time), param_value);
					break;
				case eBLEND_ADV_AM_DAUD_BW_STEP_UP_SIZE:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_dig_audio_bw_step_up_size, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_bw_step_up_size), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_bw_step_up_size), param_value);
					break;
				case eBLEND_ADV_AM_DAUD_BW_STEP_DOWN_SIZE:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_dig_audio_bw_step_down_size, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_bw_step_down_size), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_bw_step_down_size), param_value);
					break;
				case eBLEND_ADV_AM_DAUD_BW_STEP_THRESHOLD:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_dig_audio_bw_step_threshold, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_dig_audio_bw_step_threshold), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_dig_audio_bw_step_threshold), param_value);
					break;
				case eBLEND_ADV_AM_MONO2STEREO_ENABLED:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_mono2stereo_enabled, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_mono2stereo_enabled), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_mono2stereo_enabled), param_value);
					break;
				case eBLEND_ADV_AM_MONO2STEREO_START_BW:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_mono2stereo_start_bw, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_mono2stereo_start_bw), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_mono2stereo_start_bw), param_value);
					break;
				case eBLEND_ADV_AM_MONO2STEREO_STEP_TIME:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_mono2stereo_step_time, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_mono2stereo_step_time), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_mono2stereo_step_time), param_value);
					break;
				case eBLEND_ADV_AM_MONO2STEREO_MAX_SEP:
					//ret = HDR_blend_get_adv_param(hdrMainInstance, am_mono2stereo_max_sep, param_value);
					ret = HDR_blend_get_adv_param_actual(hdrMainInstance, (U32)offsetof(HDR_blend_adv_params_t, am_mono2stereo_max_sep), (U32)sizeof(((HDR_blend_adv_params_t *) 0)->am_mono2stereo_max_sep), param_value);
					break;
				default:
					ret = (HDRET)eTC_HDR_RET_NG_GET_DATA;
					break;
			}
			if(ret != 0) {
				ret = tchdr_convertHdrError((HDR_error_code_t)ret);
				if(ret == (HDRET)eTC_HDR_RET_NG_UNKNOWN) {
					ret = (HDRET)eTC_HDR_RET_NG_GET_DATA;
				}
			}
		} else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_INSTANCE;
		}
	}
	return ret;
}

HDRET tchdr_setAutoAudioAlignEnable(U32 fEnable)
{
	S32 rc;
	HDRET ret = tchdr_getHDRadioOpenStatus();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		const stHDR_FRAMEWORK_DATA_t* frameworkData = tchdrfwk_getDataStructPtr();
		HDR_auto_align_config_t configParams;
		if(frameworkData != NULL) {
			if(fEnable == (U32)0) {
			    configParams.am_auto_time_align_enabled = false;
			    configParams.fm_auto_time_align_enabled = false;
			    configParams.am_auto_level_align_enabled = false;
			    configParams.fm_auto_level_align_enabled = false;
				configParams.apply_level_adjustment = false;
			    rc = HDR_auto_align_set_config(frameworkData->autoAlign, &configParams);
				if(rc < (S32)0) {
					ret = (HDRET)eTC_HDR_RET_NG_SET_VALUE;
				}
			}
			else if(fEnable == (U32)1) {
				configParams.am_auto_time_align_enabled = true;
			    configParams.fm_auto_time_align_enabled = true;
			    configParams.am_auto_level_align_enabled = true;
			    configParams.fm_auto_level_align_enabled = true;
				configParams.apply_level_adjustment = true;
			    rc = HDR_auto_align_set_config(frameworkData->autoAlign, &configParams);
				if(rc < (S32)0) {
					ret = (HDRET)eTC_HDR_RET_NG_SET_VALUE;
				}
			}
			else {
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
	}

	return ret;
}

HDRET tchdr_setThreadPriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t userprio)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(tchdr_getHDRadioInitStatus() == 0U) {
		if(userprio.policy != 0) {
			S32 iPrioMin = sched_get_priority_min(HDR_FWRK_THREADS_POLICY);
			S32 iPrioMax = sched_get_priority_max(HDR_FWRK_THREADS_POLICY);

			if((iPrioMin < 0) || (iPrioMax < 0)) {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Real-time schduling is not available on this system, so use priority cannot be set.\n");
				ret = (HDRET)eTC_HDR_RET_NG_GET_DATA;
			}
			else if((userprio.priority >= iPrioMin) && (userprio.priority <= iPrioMax))
			{
				if((thread >= eTHREAD_MANAGER) && (thread < eTHREAD_MAX)) {
					stTcHdrThreadPriority[thread] = userprio;
				}
				else {
					(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "The thread you entered is invalid.\n");
					ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
				}
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "The priority value you entered is out of range.\n");
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Real-time priority range is from %d to %d.\n", iPrioMin, iPrioMax);
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
		else {	// Normal Policy
			if((userprio.priority >= PTHREAD_MIN_NICE) && (userprio.priority <= PTHREAD_MAX_NICE))
			{
				if((thread >= eTHREAD_MANAGER) && (thread < eTHREAD_MAX)) {
					stTcHdrThreadPriority[thread] = userprio;
				}
				else {
					(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "The thread you entered is invalid.\n");
					ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
				}
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "The priority value you entered is out of range.\n");
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Other priority range is from %d to %d.\n", PTHREAD_MIN_NICE, PTHREAD_MAX_NICE);
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			}
		}
	}
	else {
		(*pfnHdrLog)(eTAG_SYS, eLOG_WRN, "TC HD Radio framework is already initialized. Set the thread priority before initializing.\n");
		ret = (HDRET)eTC_HDR_RET_NG_ALREADY_INIT;
	}

	return ret;
}

HDRET tchdr_getDefaultThreadPriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t *userprio)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(userprio != NULL) {
		switch(thread) {
			case eTHREAD_MANAGER:
				userprio->priority = MANAGER_THREAD_PRIORITY;
				break;
			case eTHREAD_IQINPUT:
				userprio->priority = RF_IQ_INPUT_THREAD_PRIORITY;
				break;
			case eTHREAD_AUDINPUT:
				userprio->priority = AUDIO_INPUT_THREAD_PRIORITY;
				break;
			case eTHREAD_BBINPUT:
				userprio->priority = BB_INPUT_THREAD_PRIORITY;
				break;
			case eTHREAD_DEMOD:
				userprio->priority = HDR_EXEC_THREAD_PRIORITY;
				break;
			case eTHREAD_BLENDING:
				userprio->priority = HDR_BLENDING_THREAD_PRIORITY;
				break;
			case eTHREAD_AUDOUTPUT:
				userprio->priority = AUDIO_OUTPUT_THREAD_PRIORITY;
				break;
			case eTHREAD_CMDPROC:
				userprio->priority = CMD_PROC_THREAD_PRIORITY;
				break;
			case eTHREAD_LOGGER:
				userprio->priority = LOGGER_THREAD_PRIORITY;
				break;
			default:
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
				break;
		}

		if(ret == (HDRET)eTC_HDR_RET_OK) {
			userprio->policy = 1;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	return ret;
}

HDRET tchdr_getDefaultThreadNicePriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t *userprio)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(userprio != NULL) {
		switch(thread) {
			case eTHREAD_MANAGER:
				userprio->priority = MANAGER_THREAD_PRIORITY_NICE;
				break;
			case eTHREAD_IQINPUT:
				userprio->priority = RF_IQ_INPUT_THREAD_PRIORITY_NICE;
				break;
			case eTHREAD_AUDINPUT:
				userprio->priority = AUDIO_INPUT_THREAD_PRIORITY_NICE;
				break;
			case eTHREAD_BBINPUT:
				userprio->priority = BB_INPUT_THREAD_PRIORITY_NICE;
				break;
			case eTHREAD_DEMOD:
				userprio->priority = HDR_EXEC_THREAD_PRIORITY_NICE;
				break;
			case eTHREAD_BLENDING:
				userprio->priority = HDR_BLENDING_THREAD_PRIORITY_NICE;
				break;
			case eTHREAD_AUDOUTPUT:
				userprio->priority = AUDIO_OUTPUT_THREAD_PRIORITY_NICE;
				break;
			case eTHREAD_CMDPROC:
				userprio->priority = CMD_PROC_THREAD_PRIORITY_NICE;
				break;
			case eTHREAD_LOGGER:
				userprio->priority = LOGGER_THREAD_PRIORITY_NICE;
				break;
			default:
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
				break;
		}

		if(ret == (HDRET)eTC_HDR_RET_OK) {
			userprio->policy = 0;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	return ret;
}

HDRET tchdr_getThreadPriority(eTC_HDR_THREAD_t thread, stTC_HDR_THREAD_PR_t *userprio)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if((thread >= eTHREAD_MANAGER) && (thread < eTHREAD_MAX)) {
		if(userprio != NULL) {
			userprio->policy = stTcHdrThreadPriority[thread].policy;
			userprio->priority = stTcHdrThreadPriority[thread].priority;
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
	}

	return ret;
}

HDRET tchdr_debugTcHdrFramework(U32 id, U32 numdbg, const U32 *parg)
{
	HDRET ret = tchdr_getHDRadioOpenStatus();
	if(ret == (HDRET)eTC_HDR_RET_OK) {
		U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
		uiSendMsg[0] = numdbg;

		switch (numdbg) {
			case (U32)eTCHDR_DEBUG_BBINPUT_BUFFER_RESET:
				// fMainOrBs: 0: main , 1: bs, sel: 0: driver&buffer 1: driver 2: buffer
				uiSendMsg[1] = id;
				uiSendMsg[2] = 2;
				ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_TEST, uiSendMsg, pNULL, 0);
				break;
			case (U32)eTCHDR_DEBUG_BBINPUT_DRIVER_RESET:
				// fMainOrBs: 0: main , 1: bs, sel: 0: driver&buffer 1: driver 2: buffer
				uiSendMsg[1] = id;
				uiSendMsg[2] = 1;
				ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_TEST, uiSendMsg, pNULL, 0);
				break;
			case (U32)eTCHDR_DEBUG_BBINPUT_DRV_BUF_RESET:
				// fMainOrBs: 0: main , 1: bs, sel: 0: driver&buffer 1: driver 2: buffer
				uiSendMsg[1] = id;
				uiSendMsg[2] = 0;
				ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_TEST, uiSendMsg, pNULL, 0);
				break;
			case (U32)eTCHDR_DEBUG_BBINPUT_IQ_DUMP:
				if(parg != NULL) {
					uiSendMsg[1] = parg[0];	// 0: dump stop, 1:dump start
					ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_TEST, uiSendMsg, pNULL, 0);
				}
				else {
					ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
				}
				break;
			case (U32)eTCHDR_DEBUG_AUDIO_OUTPUT_DUMP:
				if(parg != NULL) {
					uiSendMsg[1] = parg[0]; // 0: dump stop, 1:dump start
					ret = tchdrsvc_sendMessage(eTCHDR_SENDER_ID_APP, (U32)eTCHDR_SVC_CMD_TEST, uiSendMsg, pNULL, 0);
				}
				else {
					ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
				}
				break;
			default:
				ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Invalid Debug Number.\n");
				break;
		}
	}
	return ret;
}
