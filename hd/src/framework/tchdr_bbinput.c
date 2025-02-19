/*******************************************************************************

*   FileName : tchdr_bbinput.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio Framework Baseband Input functions and definitions

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
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <unistd.h>

#include "tchdr_common.h"

#include "hdrBbSrc.h"
#include "hdrAudio.h"

#include "tchdr_bytestream.h"
#include "tchdr_cbuffer.h"
#include "tchdr_std.h"
#include "tchdr_msg.h"
#include "tchdr_api.h"
#include "tchdr_psd.h"
#include "tchdr_sis.h"
#include "tchdr_service.h"
#include "tchdr_framework.h"
#include "tchdr_cmdcallbacks.h"
#include "tchdr_bbinput.h"
#include "tchdr_callback_conf.h"
#include "tchdr_callback.h"
#include "tchdr_audio.h"
#include "tcaud_resampler.h"
#include "tcradio_types.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
static S8 *readIQ01;
static S8 *readIQ23;
static U32 fExtIq01DrvEn=0U;
static U32 fExtIq23DrvEn=0U;

int audio_in_out_count;

stCIRC_BUFF_t analogAudioBuffer;

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/

/***************************************************
*             Local preprocessor                   *
****************************************************/
#define BB_INPUT_CIRC_BUFFER_SIZE  (HDR_FM_BB_INPUT_BUFFER_SIZE * (U32)4)	// FM: 34560samples(34560x2byte) x 4 = 276480byte

// Maximum block size that can be read from a test vector file
#define MAX_READ_BLOCK_SIZE   (4500)

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/***************************************************
*           Local type definitions                 *
****************************************************/
typedef enum {
	FORMAT_E0,				// floating point format
	FORMAT_E1				// 16-bit fixed poing format
}eHDR_VECTOR_FORMAT_t;

// Baseband input control data structure
typedef struct {
	stTCHDR_TUNE_t tuneInfo;
    stHDR_BYTE_STREAM_t byteStream;
    eHDR_VECTOR_FORMAT_t vectorFormat;
    HDBOOL resamplerEnabled;
    S32 scaling;
    U8 iqReadBufferData[IQ_DRIVER_BUFFER_SIZE];
    stCIRC_BUFF_t iqReadBuffer;
	pthread_mutex_t iq_read_cb_mutex;
    S32	iqSampleBufferData[BB_INPUT_CIRC_BUFFER_SIZE];
    stCIRC_BUFF_t iqSampleBuffer;
    pthread_mutex_t iq_cb_mutex;
    U8 bb_src_memory[HDR_BB_SRC_MEM_SIZE];
    HDR_bb_src_t* hdrBbSrc;
}stHDR_BB_INPUT_CTRL_t;

typedef struct {
	S32 fIqInputThreadStart;
	S32 fAudioInputThreadStart;
	S32 fBbInputThreadStart;
	S32	fBandSwitching;
}stHDR_BB_INPUT_STATE_t;

typedef struct {
	U32 ftune;
	eTC_HDR_ID_t id;
	stTCHDR_TUNE_t inputTune;
}stHDR_BB_INPUT_TUNE_t;

/***************************************************
*           Local constant definitions              *
****************************************************/
static stHDR_BB_INPUT_CTRL_t stBbInputCtrl[NUM_HDR_INSTANCES];
static stHDR_BB_INPUT_STATE_t stBbInputState;

// The number of samples to read from the driver(file)		// The number of IQ Samples read from the I2S driver at once. (a chunk of I/Q samples)
static U32 bbReadBlockSize[MAX_NUM_OF_INSTANCES];			// FM: 2160 samples  AM: 4320 samples

// numInputSamplesNeeded and numOutputSamplesNeeded will only be different if BB SRC is used and input and output
// sample rates are not the same

// Total number of input samples needed to run one HDR_exec()
static U32 numInputSamplesNeeded[MAX_NUM_OF_INSTANCES];		//FM: 2160 x 16 =  34560   AM: 4320 x 8 = 34560

// Total number of output samples need to run one HDR_exec()
static U32 numOutputSamplesNeeded[MAX_NUM_OF_INSTANCES];	//FM: 2160 x 16 = 34560    AM: 270 x 8 = 2160

static eTCHDR_BBINPUT_CMD_t eBbinputEventMode;
static stHDR_BB_INPUT_TUNE_t stBbinputTune;

static S32 fBlendAudioDrvEn=0;

static pthread_mutex_t gTcHdrAAudBufMutex = PTHREAD_MUTEX_INITIALIZER;
static HDR_pcm_stereo_t  analogAudBufData[AUDIO_DRIVER_BUFFER_SIZE/4];

static S32 bbinput_reset_ready = 0;
static S32 bbinput_reset_counter = 0;

static stTC_AUDIO_RESAMPLER_t* tcAudioInputResampler;
static S8 tcAudioInputResamplerMem[TC_AUDIO_RESAMPLER_MEM_SIZE];
static F64 anaout_hz = (F64)TCHDR_SAMPLERATE;
static F64 anain_hz = (F64)TCHDR_SAMPLERATE;
static F64 anaout_ppm = (F64)TUNER_AUDIO_I2S_PPM;

static sem_t iqInputThreadSem;

#ifdef USE_EVALUATION_MODE
static struct timespec evalTimerStart;
static struct timespec evalTimerEnd;
static struct timespec evalTimerDiff;
static U32 evalAccumulatedTimerCount=0U;
static U32 evalMode=0U;
#endif

#ifdef DEBUG_IQ_BUF_FILE_DUMP
static U32 fIqdumpEnable=0U;
static FILE *gDumpIBuf=NULL;
static FILE *gDumpQBuf=NULL;
static FILE *gDumpIqBuf=NULL;
#endif

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
static struct timespec bbinput_ChkTimer, bbinput_ChkTimeNow, bbinput_ChkTimeDiff;
static U32 bbinput_AccumMs=0U, bbinput_LoopMs=0U, bbinput_DumpCount=0U;
static FILE *bbinput_DumpFile=NULL;

static struct timespec audinput_ChkTimer, audinput_ChkTimeNow, audinput_ChkTimeDiff;
static U32 audinput_AccumMs=0U, audinput_LoopMs=0U, audinput_DumpCount=0U;
static FILE *audinput_DumpFile=NULL;

static struct timespec iqinput_ChkTimer, iqinput_ChkTimeNow, iqinput_ChkTimeDiff;
static U32 iqinput_AccumMs=0U, iqinput_LoopMs=0U, iqinput_DumpCount=0U;
static FILE *iqinput_DumpFile=NULL;
#endif


/***************************************************
*          Local function prototypes               *
****************************************************/
static HDRET tchdrbbinput_getInstanceNumber(eTC_HDR_ID_t id, U32 *instanceNum);
static HDRET tchdriqinput_deinit(void);
static HDRET tchdriqinput_open(void);
static HDRET tchdriqinput_openManually(S32 fOpenIQ01, S32 fOpenIQ23);
static void tchdriqinput_closeManually(S32 fCloseIQ01, S32 fCloseIQ23);
static HDRET tchdriqinput_startManually(S32 fOpenIQ01, S32 fOpenIQ23);
static HDRET tchdriqinput_start(void);
static HDRET tchdriqinput_stop(void);

static void tchdraudinput_deinit(void);
static HDRET tchdraudinput_open(void);
static HDRET tchdraudinput_start(void);

static void tchdrbbinput_setHighestPriority(void);
static void tchdrbbinput_setOwnPriority(void);

static S32 tchdrbbinput_readTunerIqSamples(stHDR_BB_INPUT_CTRL_t* argStBbInputCtrl, int16c_t* outputBuffer, U32 numSamplesNeeded);
static HDRET tchdriqinput_readTunerIqSamples(U32 chunkSymbols01, U32 chunkSymbols23);
static F32 tchdrbbinput_getPreciseIqSampleRate(HDR_bb_src_input_rate_t eSamplerate);
static HDRET tchdriqinput_writeByteStreamIQbuffer(const S8 *pIq01Buf, S32 iq01Size, const S8 *pIq23Buf, S32 iq23Size, U32 numOfHdrInstances);
static void tchdriqinput_setIq01Enable(U32 en);
static void tchdriqinput_setIq23Enable(U32 en);
static U32 tchdriqinput_getIq01Enable(void);
static U32 tchdriqinput_getIq23Enable(void);
static HDRET tchdrbbinput_resetIqDrvAndBbinputSetting(eTC_HDR_ID_t hdrID, stTCHDR_TUNE_t inputTune);
static HDRET tchdrbbinput_setTune(eTC_HDR_ID_t id, stTCHDR_TUNE_t inputTune, U32 fNotify, U32 *uiSendMsg);

static eTCHDR_EVT_STS_t tchdrbbinput_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrbbinput_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrbbinput_event_set_tune(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrbbinput_event_delay_reset_main(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);
static eTCHDR_EVT_STS_t tchdrbbinput_event_test(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError);

/***************************************************
*			function definition					*
****************************************************/
static HDRET tchdrbbinput_getInstanceNumber(eTC_HDR_ID_t id, U32 *instanceNum)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if(instanceNum != NULL) {
		U32 currentHdrType = tchdrfwk_getHdrType();
		if(id == eTC_HDR_ID_MAIN) {
			*instanceNum = 0;
		}
		else if(id == eTC_HDR_ID_BS) {
			if(currentHdrType == (U32)HDR_1p5_CONFIG) {
				 *instanceNum = 1;
			}
			else if((currentHdrType == (U32)HDR_1p5_MRC_CONFIG) || (currentHdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
				 *instanceNum = 2;
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Not support the HDR BS ID.\n");
				ret = (HDRET)eTC_HDR_RET_NG_NOT_SUPPORT;
			}
		}
		else if(id == eTC_HDR_ID_MRC) {
			if((currentHdrType == (U32)HDR_1p0_MRC_CONFIG) || (currentHdrType == (U32)HDR_1p5_MRC_CONFIG) ||
				(currentHdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG))
			{
				 *instanceNum = 1;
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Not support the HDR MRC ID.\n");
				ret = (HDRET)eTC_HDR_RET_NG_NOT_SUPPORT;
			}
		}
		else if(id == eTC_HDR_ID_BS_MRC) {
			if(currentHdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG) {
				 *instanceNum = 3;
			}
			else {
				(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Not support the HDR BS MRC ID.\n");
				ret = (HDRET)eTC_HDR_RET_NG_NOT_SUPPORT;
			}
		}
		else {
			(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to get instance number because of invalid ID.\n");
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_NULL_POINTER_PARAMETERS;
	}

	return ret;
}

HDRET tchdrbbinput_init(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
    U32 i;
	U32 uiBufferSize  = BB_INPUT_CIRC_BUFFER_SIZE;
	const U32 numOfHdrInstances = tchdrfwk_getNumOfHdrInstance();

	stBbInputState.fBbInputThreadStart = 0;
	stBbInputState.fBandSwitching = 0;
	ret = tchdrbbinput_messageInit();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		(void)sem_init(&iqInputThreadSem, 0, 0);

	    for(i = 0; i < numOfHdrInstances; i++) {
			bbReadBlockSize[i] = 0;
			numInputSamplesNeeded[i] = 0;
			numOutputSamplesNeeded[i] = 0;

	        stBbInputCtrl[i].hdrBbSrc = HDR_bb_src_init((void*)(stBbInputCtrl[i].bb_src_memory));
	        if(stBbInputCtrl[i].hdrBbSrc == NULL){
				(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to initialize BB input resource[%d]\n", i);
	            ret = (HDRET)eTC_HDR_RET_NG_BB_SRC_INIT;
				break;
	        }

	        (*stCircFunc.cb_init)(&stBbInputCtrl[i].iqReadBuffer, (void*)(stBbInputCtrl[i].iqReadBufferData), IQ_DRIVER_BUFFER_SIZE, (U32)sizeof(U8));
	        (*stCircFunc.cb_init)(&stBbInputCtrl[i].iqSampleBuffer, (void*)(stBbInputCtrl[i].iqSampleBufferData), uiBufferSize, (U32)sizeof(int16c_t));
			(void)(*stOsal.mutexinit)(&stBbInputCtrl[i].iq_read_cb_mutex, NULL);
	        stBbInputCtrl[i].iqReadBuffer.mutex = &stBbInputCtrl[i].iq_read_cb_mutex;
			(void)(*stOsal.mutexinit)(&stBbInputCtrl[i].iq_cb_mutex, NULL);
	        stBbInputCtrl[i].iqSampleBuffer.mutex = &stBbInputCtrl[i].iq_cb_mutex;
	    }
	}
	else {
		(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to initialize BB input message\n");
		ret = (HDRET)eTC_HDR_RET_NG_INIT_MESSAGE_QUEUE;
	}

    return ret;
}

HDRET tchdrbbinput_deinit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 i;
	const U32 numOfHdrInstances = tchdrfwk_getNumOfHdrInstance();

	stBbInputState.fBbInputThreadStart = 0;
	stBbInputState.fBandSwitching = 0;
	(void)tchdrbbinput_messageDeinit();
	(void)sem_destroy(&iqInputThreadSem);

    for(i = 0; i < numOfHdrInstances; i++) {
		(void)(*stOsal.mutexdeinit)(&stBbInputCtrl[i].iq_cb_mutex);
		(void)(*stOsal.mutexdeinit)(&stBbInputCtrl[i].iq_read_cb_mutex);
    }
	return ret;
}

static F32 tchdrbbinput_getPreciseIqSampleRate(HDR_bb_src_input_rate_t eSamplerate)
{
	F32 ret;
	switch(eSamplerate) {
		case HDR_BB_SRC_650_KHZ:
			ret = 650.0f;
			break;
		case HDR_BB_SRC_675_KHZ:
			ret = 675.0f;
			break;
		case HDR_BB_SRC_744_KHZ:
			ret = 744.1875f;
			break;
		case HDR_BB_SRC_768_KHZ:
			ret = 768.0f;
			break;
		case HDR_BB_SRC_1024_KHZ:
			ret = 1024.0f;
			break;
		default:
			ret = 0.0f;
			break;
	}
	return ret;
}

static HDRET tchdrbbinput_setConditionsWithSamplerate(U32 instanceNum)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	F32 bbInputSampleRate;

	// Silab tuner FM/AM samplerate is 744.1875Khz. If you use another samplerate, The block size must be recalculated.
	bbInputSampleRate = tchdrbbinput_getPreciseIqSampleRate(stBbInputCtrl[instanceNum].tuneInfo.iqsr);
	if(bbInputSampleRate > 0.0f) {
		F32 tmpReadBlockSize;
		if(stBbInputCtrl[instanceNum].tuneInfo.band == HDR_BAND_AM) {
			// The float type does not use the 'equal' and 'not equal' operators.
			if(bbInputSampleRate < 46.51171875f) {
				stBbInputCtrl[instanceNum].resamplerEnabled = true;
				HDR_bb_src_reset(stBbInputCtrl[instanceNum].hdrBbSrc, stBbInputCtrl[instanceNum].tuneInfo.iqsr, stBbInputCtrl[instanceNum].tuneInfo.band);
			}
			else if(bbInputSampleRate > 46.51171875f) {
				stBbInputCtrl[instanceNum].resamplerEnabled = true;
				HDR_bb_src_reset(stBbInputCtrl[instanceNum].hdrBbSrc, stBbInputCtrl[instanceNum].tuneInfo.iqsr, stBbInputCtrl[instanceNum].tuneInfo.band);
			}
			else {
				stBbInputCtrl[instanceNum].resamplerEnabled = false;
			}
			tmpReadBlockSize = (bbInputSampleRate / 46.51171875f) * (F32)SYMBOL_SIZE_AM;
			bbReadBlockSize[instanceNum] = (*stCast.f32tou32)(tmpReadBlockSize);							// 4320samples at 744.1875Khz
			numInputSamplesNeeded[instanceNum] = bbReadBlockSize[instanceNum] * (U32)8;						// 34560 samples
			numOutputSamplesNeeded[instanceNum] = HDR_AM_BB_INPUT_BUFFER_SIZE;
		}
		else {    // FM or IDLE
			// The float type does not use the 'equal' and 'not equal' operators.
            if(bbInputSampleRate < 744.1875f) {
				stBbInputCtrl[instanceNum].resamplerEnabled = true;
				HDR_bb_src_reset(stBbInputCtrl[instanceNum].hdrBbSrc, stBbInputCtrl[instanceNum].tuneInfo.iqsr, stBbInputCtrl[instanceNum].tuneInfo.band);
			}
			else if(bbInputSampleRate > 744.1875f) {
				stBbInputCtrl[instanceNum].resamplerEnabled = true;
				HDR_bb_src_reset(stBbInputCtrl[instanceNum].hdrBbSrc, stBbInputCtrl[instanceNum].tuneInfo.iqsr, stBbInputCtrl[instanceNum].tuneInfo.band);
			}
			else {
				stBbInputCtrl[instanceNum].resamplerEnabled = false;
			}
			tmpReadBlockSize = (bbInputSampleRate / 744.1875f) * (F32)SYMBOL_SIZE_FM * 2.0f;	// 4320samples at 744.1875Khz
			bbReadBlockSize[instanceNum] = (*stCast.f32tou32)(tmpReadBlockSize);
			numInputSamplesNeeded[instanceNum] = bbReadBlockSize[instanceNum] * (U32)8;								// 34560 samples
            numOutputSamplesNeeded[instanceNum] = HDR_FM_BB_INPUT_BUFFER_SIZE;
        }
		(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "[Band:%d] bbInputSampleRate is [%f]kHz, BBInput[%d] resampler is %s.\n", stBbInputCtrl[instanceNum].tuneInfo.band, bbInputSampleRate, instanceNum, ((stBbInputCtrl[instanceNum].resamplerEnabled==true) ? "enabled" : "disabled"));
	}
	else {
		(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Not supported samplerate!\n");
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
	}

	return ret;
}

HDRET tchdrbbinput_open(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	const U32 numOfHdrInstances = tchdrfwk_getNumOfHdrInstance();
	U32 i;

	for(i = 0; i < numOfHdrInstances; i++) {
		// IQ output format of the current tuner is the 16bit fixed-point, so the default setting value is as follows.
		stBbInputCtrl[i].vectorFormat = FORMAT_E1;	// Do not change
        stBbInputCtrl[i].scaling = 1;				// Do not change

		ret = tchdrbbinput_setConditionsWithSamplerate(i);
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			break;
		}

		(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[i].iqReadBuffer);
		(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[i].iqSampleBuffer);
	}

	return ret;
}

HDRET tchdrbbinput_close(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 i;
	const U32 numOfHdrInstances = tchdrfwk_getNumOfHdrInstance();

	stBbInputState.fBbInputThreadStart = 0;
	stBbInputState.fBandSwitching = 0;

	for(i = 0; i < numOfHdrInstances; i++) {
        /* Comment : why is this here */
        HDR_bb_src_reset(stBbInputCtrl[i].hdrBbSrc,stBbInputCtrl[i].tuneInfo.iqsr, HDR_BAND_IDLE);

		(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[i].iqReadBuffer);
		(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[i].iqSampleBuffer);

		bbReadBlockSize[i] = 0;
		numInputSamplesNeeded[i] = 0;
		numOutputSamplesNeeded[i] = 0;
	}

	return ret;
}

S32 tchdriqinput_getReadySemaValue(void)
{
	S32 val;
	S32 rc;
	S32 ret;
	rc = sem_getvalue(&iqInputThreadSem, &val);
	if(rc == 0) {
		ret = val;
	}
	else {
		ret = rc;
	}
	return ret;
}

void tchdriqinput_ready(void)
{
	(void)sem_post(&iqInputThreadSem);
}

static S32 tchdrbbinput_setReacquire(HDR_instance_t* hdr_instance)
{
	S32 ret;
	stHDR_FRAMEWORK_DATA_t* frameworkData = tchdrfwk_getDataStructPtr();

	ret = HDR_reacquire(hdr_instance);
	if(ret == 0) {
		frameworkData->busyFlag[hdr_instance->instance_number] = true;
	}

	return ret;
}

static HDRET tchdrbbinput_reset(eTC_HDR_ID_t hdrID)	// 0: main , 1: mrc, 2: bs
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 hdrType = tchdrfwk_getHdrType();

	if(hdrID == eTC_HDR_ID_MAIN) {
	// [NOTE] If you modify this condition, you must also modify the same condition in #if above.
	#ifdef USE_RESET_IQ_WHEN_SWITCHING
		stBbInputState.fBbInputThreadStart = 0;
		stBbInputState.fAudioInputThreadStart = 0;
		stBbInputState.fIqInputThreadStart = 0;
		tchdraudinput_close();
		tchdriqinput_closeManually(1, 0);
		(*stCircFunc.cb_reset)(&stBbInputCtrl[0].iqReadBuffer);
		(*stCircFunc.cb_reset)(&stBbInputCtrl[0].iqSampleBuffer);
	#endif
        if(stBbInputCtrl[0].resamplerEnabled == true) {
        	/* Change : Perhaps find a better way of handling hte sample rate in the bb src reset */
        	HDR_bb_src_reset(stBbInputCtrl[0].hdrBbSrc, stBbInputCtrl[0].tuneInfo.iqsr, stBbInputCtrl[0].tuneInfo.band);
		}
	#ifdef USE_RESET_IQ_WHEN_SWITCHING
		tchdraudinput_open();
		tchdriqinput_openManually(1, 0);
		tchdraudinput_start();
		tchdriqinput_startManually(1, 0);
		stBbInputState.fAudioInputThreadStart = 1;
		stBbInputState.fIqInputThreadStart = 1;
	#endif
	}
	else if(hdrID == eTC_HDR_ID_MRC) {
#if (HDR_CONFIG == HDR_1p0_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
		if((hdrType == (U32)HDR_1p0_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_MRC_CONFIG) ||
			(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG))
		{
		#ifdef USE_RESET_IQ_WHEN_SWITCHING
			(*stCircFunc.cb_reset)(&stBbInputCtrl[1].iqReadBuffer);
			(*stCircFunc.cb_reset)(&stBbInputCtrl[1].iqSampleBuffer);
		#endif
			if(stBbInputCtrl[1].resamplerEnabled == true) {
	        	/* Change : Perhaps find a better way of handling hte sample rate in the bb src reset */
	        	HDR_bb_src_reset(stBbInputCtrl[1].hdrBbSrc, stBbInputCtrl[1].tuneInfo.iqsr, stBbInputCtrl[1].tuneInfo.band);
			}
		}
#else
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to reset bbinput. Check the HD Radio type.\n");
#endif
	}
	else if(hdrID == eTC_HDR_ID_BS) {
#if HDR_CONFIG == HDR_1p5_CONFIG
		if(hdrType == (U32)HDR_1p5_CONFIG) {
		// [NOTE] If you modify this condition, you must also modify the same condition in #elif below.
		#ifdef USE_RESET_IQ_WHEN_SWITCHING
			(*stCircFunc.cb_reset)(&stBbInputCtrl[1].iqReadBuffer);
			(*stCircFunc.cb_reset)(&stBbInputCtrl[1].iqSampleBuffer);
		#endif
            if(stBbInputCtrl[1].resamplerEnabled == true) {
	        	/* Change : Perhaps find a better way of handling hte sample rate in the bb src reset */
	        	HDR_bb_src_reset(stBbInputCtrl[1].hdrBbSrc, stBbInputCtrl[1].tuneInfo.iqsr, stBbInputCtrl[1].tuneInfo.band);
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to reset bbinput. Check the HD Radio type.\n");
		}
#elif (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
		U32 bsInsNum = 0;
		if(hdrType == (U32)HDR_1p5_CONFIG) {
			bsInsNum = 1;
		}
		else if((hdrType == (U32)HDR_1p5_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
			bsInsNum = 2;
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to reset bbinput. Check the HD Radio type.\n");
		}

		if(ret == (HDRET)eTC_HDR_RET_OK) {
		#ifdef USE_RESET_IQ_WHEN_SWITCHING
			(*stCircFunc.cb_reset)(&stBbInputCtrl[bsInsNum].iqReadBuffer);
			(*stCircFunc.cb_reset)(&stBbInputCtrl[bsInsNum].iqSampleBuffer);
		#endif
            if(stBbInputCtrl[bsInsNum].resamplerEnabled == true) {
	        	/* Change : Perhaps find a better way of handling hte sample rate in the bb src reset */
	        	HDR_bb_src_reset(stBbInputCtrl[bsInsNum].hdrBbSrc, stBbInputCtrl[bsInsNum].tuneInfo.iqsr, stBbInputCtrl[bsInsNum].tuneInfo.band);
			}
		}
#else
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to reset bbinput. Check the HD Radio type.\n");
#endif
	}
	else if(hdrID == eTC_HDR_ID_BS_MRC) {
#if (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
		if(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG) {
			if(stBbInputCtrl[3].resamplerEnabled == true) {
	        	/* Change : Perhaps find a better way of handling hte sample rate in the bb src reset */
	        	HDR_bb_src_reset(stBbInputCtrl[3].hdrBbSrc, stBbInputCtrl[3].tuneInfo.iqsr, stBbInputCtrl[3].tuneInfo.band);
			}
		}
#else
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to reset bbinput. Check the HD Radio type.\n");
#endif
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to reset bbinput. Check the HD Radio type.\n");
	}

	UNUSED(hdrType);
	return ret;
}

static void tchdrbbinput_resetPrimaryInputDriver(void)
{
	S32 audioBufferTotalSizeKbyte = (*stCast.u32tos32)(AUDIO_DRIVER_BUFFER_SIZE / 1024U); // (HDR_AUDIO_FRAME_SIZE*4*32/1024);
	S32 audioBufferPeriodSizeByte = (*stCast.u32tos32)(AUDIO_DRIVER_PERIOD_SIZE);

	// close
	if(pfnTcHdrBlendAudioStop != pNULL) {
		(void)(*pfnTcHdrBlendAudioStop)();
	}
	if(pfnTcHdrBlendAudioClose != pNULL) {
		(void)(*pfnTcHdrBlendAudioClose)();
	}
	tchdriqinput_closeManually(1, 0);

	// open
	(void)(*pfnTcHdrBlendAudioOpen)();
	(void)(*pfnTcHdrBlendAudioSetParams)(2, 16, 44100, audioBufferTotalSizeKbyte, audioBufferPeriodSizeByte);
	(void)tchdriqinput_openManually(1, 0);
	(void)tchdraudinput_start();
	(void)tchdriqinput_startManually(1, 0);
}

static void tchdrbbinput_resetIqInputBuffer(void)
{
	U32 hdrType = tchdrfwk_getHdrType();
	(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[0].iqReadBuffer);
	(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[0].iqSampleBuffer);
#if (HDR_CONFIG == HDR_1p0_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
	if((hdrType == (U32)HDR_1p0_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_MRC_CONFIG) ||
		(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG))
	{
		(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[1].iqReadBuffer);
		(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[1].iqSampleBuffer);
	}
#else
	UNUSED(hdrType);
#endif
}

static void tchdrbbinput_resetPrimaryInputBuffer(void)
{
	(void)(*stCircFunc.cb_reset)(&analogAudioBuffer);
	tchdrbbinput_resetIqInputBuffer();
}

static void tchdrbbinput_resetPrimaryInputDrvAndBuf(void)
{
	// close
	(void)tchdraudinput_close();
	tchdriqinput_closeManually(1, 0);

	tchdrbbinput_resetIqInputBuffer();

	// open
	(void)tchdraudinput_open();
	(void)tchdriqinput_openManually(1, 0);
	(void)tchdraudinput_start();
	(void)tchdriqinput_startManually(1, 0);
}

static HDRET tchdrbbinput_resetBuffersAndDrivers(U32 fMainOrBs, U32 sel)	// fMainOrBs: 0: main , 1: bs, sel: 0: driver&buffer 1: driver 2: buffer
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 hdrType = tchdrfwk_getHdrType();
	stHDR_FRAMEWORK_DATA_t* frameworkData = tchdrfwk_getDataStructPtr();

	if(fMainOrBs == (U32)0) {
		stBbInputState.fBbInputThreadStart = 0;
		stBbInputState.fAudioInputThreadStart = 0;
		stBbInputState.fIqInputThreadStart = 0;
		switch(sel) {
			case 1:	// reset drvier
				tchdrbbinput_resetPrimaryInputDriver();
				break;
			case 2:	// reset buffer
				tchdrbbinput_resetPrimaryInputBuffer();
				break;
			default: // reset driver and buffer
				tchdrbbinput_resetPrimaryInputDrvAndBuf();
				break;
		}
		stBbInputState.fAudioInputThreadStart = 1;
		stBbInputState.fIqInputThreadStart = 1;

		// Reacquisition for quick recovery
#if (HDR_CONFIG == HDR_1p0_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
		if((hdrType == (U32)HDR_1p0_CONFIG) || (hdrType == (U32)HDR_1p5_CONFIG)) {
			(void)tchdrbbinput_setReacquire(&frameworkData->hdrInstance[0]);
		}
		else {
			(void)tchdrbbinput_setReacquire(&frameworkData->hdrInstance[0]);
			(void)tchdrbbinput_setReacquire(&frameworkData->hdrInstance[1]);
		}
#else
		(void)tchdrbbinput_setReacquire(&frameworkData->hdrInstance[0]);
#endif
	}
	else {
#if HDR_CONFIG == HDR_1p5_CONFIG
		if(hdrType == (U32)HDR_1p5_CONFIG) {
			// [NOTE] If you modify this condition, you must also modify the same condition in #elif below.
			(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[1].iqReadBuffer);
			(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[1].iqSampleBuffer);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to reset bbinput. Check the HD Radio type.\n");
		}
#elif (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
		if(hdrType == (U32)HDR_1p5_CONFIG) {
			// [NOTE] If you modify this condition, you must also modify the same condition in #if above.
			(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[1].iqReadBuffer);
			(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[1].iqSampleBuffer);
		}
		else if(hdrType == (U32)HDR_1p5_MRC_CONFIG) {
			(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[2].iqReadBuffer);
			(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[2].iqSampleBuffer);
		}
		else if(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG) {
			(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[2].iqReadBuffer);
			(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[2].iqSampleBuffer);
			(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[3].iqReadBuffer);
			(void)(*stCircFunc.cb_reset)(&stBbInputCtrl[3].iqSampleBuffer);
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
			(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to reset bbinput. Check the HD Radio type.\n");
		}
#else
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to reset bbinput. Check the HD Radio type.\n");
#endif
	}

	UNUSED(hdrType);
	return ret;
}

static void tchdrbbinput_setEventMode(eTCHDR_BBINPUT_CMD_t evtmode)
{
	eBbinputEventMode = evtmode;
}

static eTCHDR_BBINPUT_CMD_t tchdrbbinput_getEventMode(void)
{
	return eBbinputEventMode;
}

static void tchdrbbinput_sendNotifyMessage(U32 eRcvID, U32 mode, const U32 *data, const void *pdata, S32 err)
{
	U32 txmode = (*stArith.u32add)(mode, (U32)TCHDR_BBP_NOTIFY_WT);

	switch(eRcvID) {
		case (U32)eTCHDR_SENDER_ID_APP:
			(void)tchdrapp_sendMessage(eTCHDR_SENDER_ID_BBINPUT, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_SERVICE:
			(void)tchdrsvc_sendMessage(eTCHDR_SENDER_ID_BBINPUT, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_AUDIO:
			(void)tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_BBINPUT, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_BBINPUT:
			(void)tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_BBINPUT, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MAIN:
			(void)tchdrmain_sendMessage(eTCHDR_SENDER_ID_BBINPUT, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_MRC:
			(void)tchdrmrc_sendMessage(eTCHDR_SENDER_ID_BBINPUT, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BS:
			(void)tchdrbs_sendMessage(eTCHDR_SENDER_ID_BBINPUT, txmode, data, pdata, err);
			break;
		case (U32)eTCHDR_SENDER_ID_HDR_BLENDING:
			(void)tchdrblending_sendMessage(eTCHDR_SENDER_ID_BBINPUT, txmode, data, pdata, err);
			break;
		default:
			UNUSED(0);
			(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, " %s : Invalid received ID[%d].\n", __func__, eRcvID);
			break;
	}
}

static void tchdrbbinput_tuneHandler(void)
{
	if(stBbinputTune.ftune > 0U) {
		U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};
		stBbinputTune.ftune = 0;
		(void)tchdrbbinput_setTune(stBbinputTune.id, stBbinputTune.inputTune, 1U, uiSendMsg);
	}
}

static void tchdrbbinput_messageParser(const stTcHdrMsgBuf_t *pstMsg)
{
	switch(pstMsg->uiMode) {
		// Command
		case (U32)eTCHDR_BBINPUT_CMD_OPEN:
		case (U32)eTCHDR_BBINPUT_CMD_CLOSE:
		case (U32)eTCHDR_BBINPUT_CMD_RESET_MAIN:
		case (U32)eTCHDR_BBINPUT_CMD_RESET_BS:
		case (U32)eTCHDR_BBINPUT_CMD_SET_TUNE:
		case (U32)eTCHDR_BBINPUT_CMD_DELAY_RESET_MAIN:
		case (U32)eTCHDR_BBINPUT_CMD_TEST:
			tchdrbbinput_setEventMode((eTCHDR_BBINPUT_CMD_t)pstMsg->uiMode);
			break;

		default:
			// Notification of the other thread: Do it here.
			switch(pstMsg->uiMode) {
				// HDR Audio Notification
				case (U32)eTCHDR_AUDIO_NOTIFY_RESET:
					(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Telechips HD Radio Audio Buffer Reset!!!\n");
					break;

				case (U32)eTCHDR_AUDIO_NOTIFY_MUTE:
					break;

				default:
					UNUSED(0);
					break;
			}
			break;
	}
}

static void tchdrbbinput_eventHandler(stTcHdrMsgBuf_t stRcvMsgQ)
{
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;
	eTCHDR_BBINPUT_CMD_t eNowEvtMode;
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 uiSendMsg[TCHDR_MSGQ_DATA_LENGTH] = {0,};

	eNowEvtMode = tchdrbbinput_getEventMode();

	switch(eNowEvtMode) {
		case eTCHDR_BBINPUT_CMD_OPEN:
			eEvtSt = tchdrbbinput_event_open(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BBINPUT_CMD_CLOSE:
			eEvtSt = tchdrbbinput_event_close(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BBINPUT_CMD_RESET_MAIN:
			ret = tchdrbbinput_reset(eTC_HDR_ID_MAIN);
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
			break;

		case eTCHDR_BBINPUT_CMD_RESET_BS:
			ret = tchdrbbinput_reset(eTC_HDR_ID_BS);
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
			break;

		case eTCHDR_BBINPUT_CMD_SET_TUNE:
			eEvtSt = tchdrbbinput_event_set_tune(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BBINPUT_CMD_DELAY_RESET_MAIN:
			eEvtSt = tchdrbbinput_event_delay_reset_main(stRcvMsgQ, uiSendMsg, &ret);
			break;

		case eTCHDR_BBINPUT_CMD_TEST:
			eEvtSt = tchdrbbinput_event_test(stRcvMsgQ, uiSendMsg, &ret);
			break;

		default:
			UNUSED(0);
			break;
	}

	tchdrbbinput_setEventMode(eTCHDR_BBINPUT_CMD_NULL);

	switch(eEvtSt) {
		/* Job Good Complete -> No Notify */
		case eTCHDR_EVT_STS_DONE:
			break;

		/* Job Good Complete -> Check Notify Case */
		case eTCHDR_EVT_STS_DONE_NOTIFY:
			tchdrbbinput_sendNotifyMessage(stRcvMsgQ.uiSender, (U32)eNowEvtMode, uiSendMsg, pNULL, ret);
			break;

		/* Return Error */
		default:
			UNUSED(0);
			break;
	}

	if(bbinput_reset_ready > 0) {
		if(bbinput_reset_counter == 0) {
		//	If the counter is 3, it will be about 130ms to 160ms. (Based on bbinput thread loop time)
			(*pfnHdrLog)(eTAG_BBIN, eLOG_INF, "Completed to recover from SDR driver error.\n");
			(void)tchdrbbinput_resetBuffersAndDrivers(0,0);
			bbinput_reset_ready = 0;
		}
		bbinput_reset_counter--;
	}
}

static void tchdrbbbinput_eventMessageProcess(stTcHdrMsgBuf_t *pstMsg)
{
	(void)tchdrbbinput_getMessage(pstMsg);
	if(pstMsg->fNewMsg == (U32)eTCHDR_NEW_MSG_EXIST) {
		tchdrbbinput_messageParser(pstMsg);
	}
	tchdrbbinput_eventHandler(*pstMsg);
}

// Input reader thread reads samples from the Input byteStream and stores them in the IQ sample buffer.
#if 1
void *tchdr_bbInputThread(void* arg)
{
    struct timespec timer;
	struct timespec timeNow;
	stTcHdrMsgBuf_t stRecivedMessage;
    S32 numSamplesRead = 0;
	S32 fIQPass = 0;
    U32 accumulatedSampleCount;
	const U32 numOfHdrInstances = tchdrfwk_getNumOfHdrInstance();
	static int16c_t readOutputBuffer[MAX_READ_BLOCK_SIZE];
    static int16c_t bbOutputBuffer[(SYMBOL_SIZE_FM*2U) + 80U];
    U32 numOutputSamples = 0;
	S32 numOutputIqSamples;
	S32 numOutputAudioFrames;
    U32 i;

	(*stOsal.setPostThreadAttr)(stBbInputThdAttr, eTAG_BBIN);	// Set thread nice & name
	(void)(*stOsal.osmemset)((void*)&stRecivedMessage, (S8)0, (U32)sizeof(stTcHdrMsgBuf_t));

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	bbinput_DumpFile = fopen(DUMP_PATH"bbinput_thread_status.csv", "w");
	clock_gettime(LINUX_CLOCK_TIMER_TYPE, &bbinput_ChkTimer);
#endif

    (void)clock_gettime(LINUX_CLOCK_TIMER_TYPE,&timer);		// Get current time.
    timer.tv_nsec += BBINPUT_START_NANO_DELAY;				// Start after BBINPUT_START_NANO_DELAY

    // Timed while loop executes at the rate set by the call to clock_nanosleep()
    stBbInputThdAttr.thread_running = 1;
    while (stBbInputThdAttr.thread_running > 0) {
		tchdrbbbinput_eventMessageProcess(&stRecivedMessage);

        accumulatedSampleCount = 0;
        while (accumulatedSampleCount < numInputSamplesNeeded[0]) {
			U32 openStatus = tchdrsvc_getOpenStatus();
			S32 availSize = (*stCircFunc.cb_availData)(&stBbInputCtrl[0].iqReadBuffer);
			if((openStatus == 1U) && (stBbInputState.fBbInputThreadStart == 1) && (availSize > (*stCast.u32tos32)(numInputSamplesNeeded[0])))
			{
	            for(i = 0; i < numOfHdrInstances; i++) {
	                if(stBbInputCtrl[i].resamplerEnabled == true) {
						numSamplesRead = tchdrbbinput_readTunerIqSamples(&stBbInputCtrl[i], readOutputBuffer, bbReadBlockSize[i]);
						if(numSamplesRead > 0) {
                    		HDR_bb_src_exec(stBbInputCtrl[i].hdrBbSrc, readOutputBuffer, bbReadBlockSize[i], bbOutputBuffer, &numOutputSamples);
						}
	                }
					else {
	                    numSamplesRead = tchdrbbinput_readTunerIqSamples(&stBbInputCtrl[i], bbOutputBuffer, bbReadBlockSize[i]);
						if(numSamplesRead > 0) {
	                    	numOutputSamples = (U32)numSamplesRead;
						#ifdef DEBUG_IQ_BUF_FILE_DUMP
							if(fIqdumpEnable && i == 0) {
								fwrite(bbOutputBuffer, 4, bbReadBlockSize[i], gDumpIqBuf);
							}
						#endif
						}
	                }

	                if (numSamplesRead < (*stCast.u32tos32)(bbReadBlockSize[i])) {
	                    (*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Failed to read samples of the instance[%d]. readSize[%d]\n", i, numSamplesRead);
	                }
					else {
		                if((*stCircFunc.cb_write)(&stBbInputCtrl[i].iqSampleBuffer, (void*)bbOutputBuffer, numOutputSamples) < 0) {
		                    (*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "IQ buffer %d overrun.\n", i);
		                    (void)(*stCircFunc.cb_reset)(&stBbInputCtrl[i].iqSampleBuffer);
		                }
						else {
							fIQPass = (*stArith.s32add)(fIQPass, 1);	// fIQPass++;
						}
					}
	            }

				if((fIQPass > 0) && (numSamplesRead > 0)) {
					fIQPass = 0;
            		accumulatedSampleCount += (U32)numSamplesRead;
				}
			}

			// Wait until next shot.
	        (void)clock_nanosleep(LINUX_CLOCK_TIMER_TYPE, TIMER_ABSTIME, &timer, NULL);

	        // Calculate next shot.
	        //timer.tv_nsec += bbReadTickPeriod + addBbinputLoopTime;
	        timer.tv_nsec = (*stArith.slongadd)(timer.tv_nsec, 2000000);	// 2msec

	        while (timer.tv_nsec >= NSEC_PER_SEC) {
	            timer.tv_nsec -= NSEC_PER_SEC;
	            timer.tv_sec++;
	        }

	        // If we fall too far behind, reset the timer
	        (void)clock_gettime(LINUX_CLOCK_TIMER_TYPE, &timeNow);
	        if(timeNow.tv_sec > timer.tv_sec){
	            timer = timeNow;
	        }
        }

		// The code below is necessary to prevent CPU high load at the init status.
		if(numInputSamplesNeeded[0] == (U32)0) {
			// HD radio framework is not opened.
			(void)usleep(2000);	// 2ms
		}

        // Schedule HDR_exec() for every 16 symbols of data that is available
		if(numOutputSamplesNeeded[0] > (U32)0)
		{
	        for(i = 0; i < numOfHdrInstances; i++){
				numOutputIqSamples = (*stCircFunc.cb_availData)(&stBbInputCtrl[i].iqSampleBuffer);
				numOutputAudioFrames = (*stCircFunc.cb_availData)(&analogAudioBuffer);
				if(numOutputIqSamples >= (S32)numOutputSamplesNeeded[i])
				{
                    if(i == (U32)0) {
						if(numOutputAudioFrames < (S32)HDR_AUDIO_FRAME_SIZE) {
							UNUSED(numOutputAudioFrames);
							(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "[BB Input] A warning [%d] less than the amount [2048] of required audio data!!!\n", numOutputAudioFrames);
						}
						tchdraudinput_ready();
                    }
					tchdrbbinput_ready(i);
				}
	        }
		}
#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &bbinput_ChkTimeNow);
		bbinput_ChkTimeDiff.tv_sec = bbinput_ChkTimeNow.tv_sec - bbinput_ChkTimer.tv_sec;
		bbinput_ChkTimeDiff.tv_nsec = bbinput_ChkTimeNow.tv_nsec - bbinput_ChkTimer.tv_nsec;
		if(bbinput_ChkTimeDiff.tv_nsec < 0) {
			bbinput_ChkTimeDiff.tv_sec--;
			bbinput_ChkTimeDiff.tv_nsec += 1e9;
		}
		bbinput_LoopMs = (bbinput_ChkTimeDiff.tv_sec*1000) + (bbinput_ChkTimeDiff.tv_nsec/1000000);
		if(bbinput_DumpFile != NULL) {
			S32 iq0Samples = (*stCircFunc.cb_availData)(&stBbInputCtrl[0].iqSampleBuffer);
			S32 audSamples = (*stCircFunc.cb_availData)(&analogAudioBuffer);
			fprintf(bbinput_DumpFile, "%d,%d,%d,%d,%d\n", bbinput_DumpCount++, bbinput_AccumMs+=bbinput_LoopMs, bbinput_LoopMs, iq0Samples, audSamples);
			fflush(bbinput_DumpFile);
		}
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &bbinput_ChkTimer);
#endif
    }

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	if(bbinput_DumpFile != NULL) {
		fclose(bbinput_DumpFile);
	}
#endif
	stBbInputThdAttr.thread_running = -1;
	(void)tchdrbbinput_deinit();
	(void)tchdriqinput_deinit();
	tchdraudinput_deinit();
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrBbInput Thread Sequence 03...\n");
	return pNULL;
}
#elif 0
// tell cpd to start ignoring code - CPD-OFF
void *tchdr_bbInputThread(void* arg)
{
	stTcHdrMsgBuf_t stRecivedMessage;
    S32 numSamplesRead = 0;
	const U32 numOfHdrInstances = tchdrfwk_getNumOfHdrInstance();

	(void)(*stOsal.osmemset)((void*)&stRecivedMessage, (S8)0, (U32)sizeof(stTcHdrMsgBuf_t));
    prctl(PR_SET_NAME,HDR_BBINPUT_THREAD_NAME,0,0,0);

    static int16c_t readOutputBuffer[MAX_READ_BLOCK_SIZE];
    static int16c_t bbOutputBuffer[SYMBOL_SIZE_FM*2 + 80];

    U32 numOutputSamples = 0;
	U32 numOutputBlocks = 0;
    U32 i = 0;
	U32 j = 0;

    stBbInputThdAttr.thread_running = 1;
    while (stBbInputThdAttr.thread_running > 0) {
		tchdrbbinput_getMessage(&stRecivedMessage);
		if(stRecivedMessage.fNewMsg == (U32)eTCHDR_NEW_MSG_EXIST) {
			tchdrbbinput_messageParser(&stRecivedMessage);
		}
		tchdrbbinput_eventHandler();

		if(tchdrsvc_getOpenStatus() && stBbInputState.fBbInputThreadStart) {
			sem_wait(&iqInputThreadSem);
			if(stBbInputThdAttr.thread_running != 1) {
				break;
			}

	        for(i = 0; i < numOfHdrInstances; i++) {
	            if(stBbInputCtrl[i].resamplerEnabled == true) {
					numSamplesRead = tchdrbbinput_readTunerIqSamples(&stBbInputCtrl[i], readOutputBuffer, bbReadBlockSize[i]);
					if(numSamplesRead > 0) {
						HDR_bb_src_exec(stBbInputCtrl[i].hdrBbSrc, readOutputBuffer, bbReadBlockSize[i], bbOutputBuffer, &numOutputSamples);
					}
	            }
				else {
	                numSamplesRead = tchdrbbinput_readTunerIqSamples(&stBbInputCtrl[i], bbOutputBuffer, bbReadBlockSize[i]);
					if(numSamplesRead > 0) {
						numOutputSamples = (U32)numSamplesRead;
					}
	            }
			#ifdef DEBUG_IQ_BUF_FILE_DUMP
				if(fIqdumpEnable && i == 0) {
					 if(stBbInputCtrl[i].resamplerEnabled == true) {
					 	fwrite(readOutputBuffer, 4, bbReadBlockSize[i], gDumpIqBuf);
					 }
					 else {
						fwrite(bbOutputBuffer, 4, bbReadBlockSize[i], gDumpIqBuf);
					 }
				}
			#endif

	            if (numSamplesRead < bbReadBlockSize[i]) {
	                (*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to read samples of the instance[%d]. readSize[%d]\n", i, numSamplesRead);
	            }
				else {
	                if((*stCircFunc.cb_write)(&stBbInputCtrl[i].iqSampleBuffer, (void*)bbOutputBuffer, (S32)numOutputSamples) < 0) {
	                    (*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "IQ buffer %d overrun.\n", i);
	                    (*stCircFunc.cb_reset)(&stBbInputCtrl[i].iqSampleBuffer);
	                }
				}

				// Schedule HDR_exec() for every 16 symbols of data that is available
				numOutputBlocks = (*stCircFunc.cb_availData)(&stBbInputCtrl[i].iqSampleBuffer) / numOutputSamplesNeeded[i];
		        for(j = 0; j < numOutputBlocks; j++) {
					S32 numOutputAudioFrames = (*stCircFunc.cb_availData)(&analogAudioBuffer);
	                if(i == (U32)0) {
						S32 diffMs = 0;
						if(numOutputAudioFrames < (S32)HDR_AUDIO_FRAME_SIZE) {
							(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "[BB Input] A warning [%d] less than the amount [2048] of required audio data!!!\n", numOutputAudioFrames);
						}
						tchdraudinput_ready();
	                }
					tchdrbbinput_ready(i);
		        }
	        }
		}
		else {
			usleep(5000);	// 5ms sleep
		}
    }
	stBbInputThdAttr.thread_running = -1;
	tchdrbbinput_deinit();
	tchdriqinput_deinit();
	tchdraudinput_deinit();
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrBbInput Thread Sequence 03...\n");
	return pNULL;
}
// resume CPD analysis - CPD-ON
#else
// tell cpd to start ignoring code - CPD-OFF
void *tchdr_bbInputThread(void* arg)
{
    struct timespec timer;
	struct timespec timeNow;
	stTcHdrMsgBuf_t stRecivedMessage;
    S32 numSamplesRead = 0;
	const U32 numOfHdrInstances = tchdrfwk_getNumOfHdrInstance();
	static int16c_t readOutputBuffer[MAX_READ_BLOCK_SIZE];
    static int16c_t bbOutputBuffer[(SYMBOL_SIZE_FM*2U) + 80U];
    U32 numOutputSamples = 0;
	S32 numOutputIqSamples;
	S32 numOutputAudioFrames;
    U32 i;

	(void)(*stOsal.osmemset)((void*)&stRecivedMessage, (S8)0, (U32)sizeof(stTcHdrMsgBuf_t));
    (void)prctl(PR_SET_NAME,HDR_BBINPUT_THREAD_NAME,0,0,0);

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	bbinput_DumpFile = fopen(DUMP_PATH"bbinput_thread_status.csv", "w");
	clock_gettime(LINUX_CLOCK_TIMER_TYPE, &bbinput_ChkTimer);
#endif

    (void)clock_gettime(LINUX_CLOCK_TIMER_TYPE,&timer);		// Get current time.
    timer.tv_nsec += BBINPUT_START_NANO_DELAY;				// Start after BBINPUT_START_NANO_DELAY

    // Timed while loop executes at the rate set by the call to clock_nanosleep()
    stBbInputThdAttr.thread_running = 1;
    while (stBbInputThdAttr.thread_running > 0) {
		U32 openStatus = tchdrsvc_getOpenStatus();

		//*****************************************************************//
		//                     - VERY IMPORTANT!!! -                       //
		// The message command of the BBInput thread has to be processed   //
		// every 34560 samples in FM and 2160 samples in AM.               //
		//*****************************************************************//
		(void)tchdrbbinput_getMessage(&stRecivedMessage);
		if(stRecivedMessage.fNewMsg == (U32)eTCHDR_NEW_MSG_EXIST) {
			tchdrbbinput_messageParser(&stRecivedMessage);
		}
		tchdrbbinput_eventHandler(stBbinputRcvMsgQ);

		if((openStatus == 1U) && (stBbInputState.fBbInputThreadStart == 1))
		{
			for(i = 0; i < numOfHdrInstances; i++)
			{
				if(numOutputSamplesNeeded[i] > 0U)
				{
					S32 availSize = (*stCircFunc.cb_availData)(&stBbInputCtrl[i].iqReadBuffer);
					if(availSize > bbReadBlockSize[i])
					{
		                if(stBbInputCtrl[i].resamplerEnabled == true)
						{
							numSamplesRead = tchdrbbinput_readTunerIqSamples(&stBbInputCtrl[i], readOutputBuffer, bbReadBlockSize[i]);
							if(numSamplesRead > 0)
							{
								HDR_bb_src_exec(stBbInputCtrl[i].hdrBbSrc, readOutputBuffer, bbReadBlockSize[i], bbOutputBuffer, &numOutputSamples);
							}
		                }
						else
						{
		                    numSamplesRead = tchdrbbinput_readTunerIqSamples(&stBbInputCtrl[i], bbOutputBuffer, bbReadBlockSize[i]);
							if(numSamplesRead > 0)
							{
								numOutputSamples = (U32)numSamplesRead;
							#ifdef DEBUG_IQ_BUF_FILE_DUMP
								if(fIqdumpEnable && i == 0) {
									fwrite(bbOutputBuffer, 4, bbReadBlockSize[i], gDumpIqBuf);
								}
							#endif
							}
		                }

		                if (numSamplesRead >= (S32)bbReadBlockSize[i])
						{
							if((*stCircFunc.cb_write)(&stBbInputCtrl[i].iqSampleBuffer, (void*)bbOutputBuffer, (S32)numOutputSamples) < 0)
							{
			                    (*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "IQ[%d] buffer overrun detected!\n", i);
			                }
		                }
						else
						{
			                (*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to read samples of the instance[%d]. readSize[%d]\n", i, numSamplesRead);
						}
					}
		        }
			}

			for(i = 0; i < numOfHdrInstances; i++)
			{
				if(numOutputSamplesNeeded[i] > 0U)
				{
					numOutputIqSamples = (*stCircFunc.cb_availData)(&stBbInputCtrl[i].iqSampleBuffer);
					if(numOutputIqSamples > (S32)numOutputSamplesNeeded[i])
					{
						tchdrbbinput_tuneHandler();
						if(i == 0U)
						{
							numOutputAudioFrames = (*stCircFunc.cb_availData)(&analogAudioBuffer);
							if(numOutputAudioFrames < (S32)HDR_AUDIO_FRAME_SIZE)
							{
								(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "The amount[%d] of input data is less than the 2048 samples of audio data required!\n", numOutputAudioFrames);
							}
							tchdraudinput_ready();
	                    }
						tchdrbbinput_ready(i);
					}
				}
			}
		}

		// Wait until next shot.
        (void)clock_nanosleep(LINUX_CLOCK_TIMER_TYPE, TIMER_ABSTIME, &timer, NULL);

        // Calculate next shot.
        //timer.tv_nsec += bbReadTickPeriod + addBbinputLoopTime;
        timer.tv_nsec += 2500000;	// 2msec

        while (timer.tv_nsec >= NSEC_PER_SEC) {
            timer.tv_nsec -= NSEC_PER_SEC;
            timer.tv_sec++;
        }

        // If we fall too far behind, reset the timer
        (void)clock_gettime(LINUX_CLOCK_TIMER_TYPE, &timeNow);
        if(timeNow.tv_sec > timer.tv_sec){
            timer = timeNow;
        }

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &bbinput_ChkTimeNow);
		bbinput_ChkTimeDiff.tv_sec = bbinput_ChkTimeNow.tv_sec - bbinput_ChkTimer.tv_sec;
		bbinput_ChkTimeDiff.tv_nsec = bbinput_ChkTimeNow.tv_nsec - bbinput_ChkTimer.tv_nsec;
		if(bbinput_ChkTimeDiff.tv_nsec < 0) {
			bbinput_ChkTimeDiff.tv_sec--;
			bbinput_ChkTimeDiff.tv_nsec += 1e9;
		}
		bbinput_LoopMs = (bbinput_ChkTimeDiff.tv_sec*1000) + (bbinput_ChkTimeDiff.tv_nsec/1000000);
		if(bbinput_DumpFile != NULL) {
			S32 iq0Samples = (*stCircFunc.cb_availData)(&stBbInputCtrl[0].iqSampleBuffer);
			S32 audSamples = (*stCircFunc.cb_availData)(&analogAudioBuffer);
			fprintf(bbinput_DumpFile, "%d,%d,%d,%d,%d\n", bbinput_DumpCount++, bbinput_AccumMs+=bbinput_LoopMs, bbinput_LoopMs, iq0Samples, audSamples);
			fflush(bbinput_DumpFile);
		}
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &bbinput_ChkTimer);
#endif
    }

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	if(bbinput_DumpFile != NULL) {
		fclose(bbinput_DumpFile);
	}
#endif
	stBbInputThdAttr.thread_running = -1;
	(void)tchdrbbinput_deinit();
	(void)tchdriqinput_deinit();
	tchdraudinput_deinit();
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrBbInput Thread Sequence 03...\n");
	return pNULL;
}
// resume CPD analysis - CPD-ON
#endif

S32 tchdrbbinput_getSamples(int16c_t* bbSamples, U32 numSamples, U32 instanceNum)
{
#ifndef IQ_PATTERN_CHECK_ENABLE
    return (*stCircFunc.cb_read)(&stBbInputCtrl[instanceNum].iqSampleBuffer, (void*)bbSamples, numSamples);
#else
	S32 ret;
    ret = (*stCircFunc.cb_read)(&stBbInputCtrl[instanceNum].iqSampleBuffer, (void*)bbSamples, numSamples);

	uint_t check_size;

    if(stBbInputCtrl[instanceNum].tuneInfo.band == HDR_BAND_AM)
	{
		check_size = HDR_AM_BB_INPUT_BUFFER_SIZE;
    }
	else if (stBbInputCtrl[instanceNum].tuneInfo.band == HDR_BAND_FM)
	{
		check_size = HDR_FM_BB_INPUT_BUFFER_SIZE;
	}
	else
	{
		check_size = 0;
	}


	if (numSamples != check_size)
	{
		(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "[IQ PATTERN CHECK] tchdrbbinput_getSamples numSamples = %d \n", numSamples);
	}
	else
	{
		int check_cnt = 0;
		uint_t check_val = TUNER_A_PATTERN_4BYTE;
		uint_t *check_p = bbSamples;

		for (int tmp_cnt = 0; tmp_cnt < check_size; tmp_cnt ++)
		{
			if (check_p[tmp_cnt] != check_val)
				check_cnt ++;
		}

		if (check_cnt != 0)
			(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "[IQ PATTERN CHECK] tchdrbbinput_getSamples check_cnt = %d check_size = %d \n", check_cnt, check_size);
	}
	
	return ret;
#endif
}

S32 tchdrbbinput_getSamplesValid(U32 instanceNum)
{
    return (*stCircFunc.cb_availData)(&stBbInputCtrl[instanceNum].iqSampleBuffer);
}

void tchdrbbinput_resetSamples(U32 instanceNum)
{
    (void)(*stCircFunc.cb_reset)(&stBbInputCtrl[instanceNum].iqSampleBuffer);
}

S32 tchdrbbinput_fillSamples(S8 value, U32 numElems, U32 instanceNum)
{
    return (*stCircFunc.cb_fill)(&stBbInputCtrl[instanceNum].iqSampleBuffer, value, numElems);
}

static void tchdrbbinput_packVectorE0Samples(HDR_tune_band_t band, F32 iqscaling, const S16 *iBuffer, const S16 *qBuffer, int16c_t *outBuffer, U32 numSamples)
{
	U32 n;
	F32 tmpIsamp = 0.0f;	// for calculation
	F32 tmpQsamp = 0.0f;	// for calculation
	S32 iTemp = 0;
	S32 qTemp = 0;

	// Pack samples as alternating I/Q
	for (n = 0; n < numSamples; n+=(U32)2) {
		tmpIsamp = (*stCast.s16tof32)(iBuffer[n]) * iqscaling;
		tmpQsamp = (*stCast.s16tof32)(qBuffer[n]) * iqscaling;
		iTemp = (*stCast.f32tos32)(tmpIsamp);
		qTemp = (*stCast.f32tos32)(tmpQsamp);

		iTemp = CLAMP(iTemp, -32768, 32767);
		qTemp = CLAMP(qTemp, -32768, 32767);

		if(band == HDR_BAND_FM) {
			outBuffer[n].re = (S16)qTemp;
			outBuffer[n].im = (S16)iTemp;
		}
		else {
			outBuffer[n].re = (S16)iTemp;
			outBuffer[n].im = (S16)qTemp;
		}

		tmpIsamp = (*stCast.s16tof32)(iBuffer[n+1U]) * iqscaling;
		tmpQsamp = (*stCast.s16tof32)(qBuffer[n+1U]) * iqscaling;
		iTemp = (*stCast.f32tos32)(tmpIsamp);
		qTemp = (*stCast.f32tos32)(tmpQsamp);

		iTemp = CLAMP(iTemp, -32768, 32767);
		qTemp = CLAMP(qTemp, -32768, 32767);

		if(band == HDR_BAND_FM) {
			outBuffer[n+(U32)1].re = (S16)qTemp;
			outBuffer[n+(U32)1].im = (S16)iTemp;
		}
		else {
			outBuffer[n+(U32)1].re = (S16)iTemp;
			outBuffer[n+(U32)1].im = (S16)qTemp;
		}
	}
}

static void tchdrbbinput_packVectorE1Samples(HDR_tune_band_t band, const S16 *iBuffer, const S16 *qBuffer, int16c_t *outBuffer, U32 numSamples)
{
	U32 n;

	// Pack samples as alternating I/Q
	if(band == HDR_BAND_FM) {
		for (n = 0; n < numSamples; n+=(U32)2) {
			outBuffer[n].re = qBuffer[n+1U];
			outBuffer[n].im = iBuffer[n+1U];

			outBuffer[n+(U32)1].re = qBuffer[n];
			outBuffer[n+(U32)1].im = iBuffer[n];
		}
	}
	else {
		for (n = 0; n < numSamples; n+=(U32)2) {
			outBuffer[n].re = iBuffer[n+1U];
			outBuffer[n].im = qBuffer[n+1U];

			outBuffer[n+(U32)1].re = iBuffer[n];
			outBuffer[n+(U32)1].im = qBuffer[n];
		}
	}
}

// Read baseband samples from a standard test vector file(2160I | 2160Q)
static S32 tchdrbbinput_readTunerIqSamples(stHDR_BB_INPUT_CTRL_t* argStBbInputCtrl, int16c_t* outputBuffer, U32 numSamplesNeeded)
{
	// Input I/Q I2S supports only 16bit slot size and 744.1875kHz sample rate.
	static S16 ISampBuffer[BB_INPUT_SYMBOL_SIZE] = {0,};
	static S16 QSampBuffer[BB_INPUT_SYMBOL_SIZE] = {0,};

	U32 sampleSize = 0;
	U32 bytesToRead;
	S32 ret=0;
	S32 rc0;
	S32 rc1;

	F32 iqscaling = 0.0f;	// for calculation

	if((argStBbInputCtrl == NULL) || (outputBuffer == NULL)) {
		(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "The buffer pointer parameters of tchdrbbinput_readTunerIqSamples() are NULL.\n");
		ret = -1;
	}
	else if(numSamplesNeeded > BB_INPUT_SYMBOL_SIZE) {
		(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "The numSamplesNeeded is larger than the internal buffer size.\n");
		ret = -1;
	}
	else {
		iqscaling = (*stCast.s32tof32)(argStBbInputCtrl->scaling);

		if(argStBbInputCtrl->vectorFormat == FORMAT_E0) {
			sampleSize = (U32)sizeof(F32);
		}
		else {
			sampleSize = (U32)sizeof(S16);
		}
	}

	if(ret == 0) {
		bytesToRead = numSamplesNeeded * sampleSize;

		rc0 = (*stCircFunc.cb_read)(&argStBbInputCtrl->iqReadBuffer, (void*)ISampBuffer, bytesToRead);
		rc1 = (*stCircFunc.cb_read)(&argStBbInputCtrl->iqReadBuffer, (void*)QSampBuffer, bytesToRead);

#ifdef IQ_PATTERN_CHECK_ENABLE
		if (rc0 != (BB_INPUT_SYMBOL_SIZE * 2))
		{
			(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "[IQ PATTERN CHECK] iqReadBuffer ISampBuffer return = %d \n", rc0);
		}
		else
		{
			int check_cnt = 0;
			S16 check_val = TUNER_A_PATTERN_I;

			for (int tmp_cnt = 0; tmp_cnt < BB_INPUT_SYMBOL_SIZE; tmp_cnt ++)
			{
				if (ISampBuffer[tmp_cnt] != check_val)
					check_cnt ++;
			}

			if (check_cnt != 0)
				(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "[IQ PATTERN CHECK] iqReadBuffer ISampBuffer check_cnt = %d \n", check_cnt);
		}

		if (rc1 != (BB_INPUT_SYMBOL_SIZE * 2))
		{
			(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "[IQ PATTERN CHECK] iqReadBuffer QSampBuffer return = %d \n", rc1);
		}
		else
		{
			int check_cnt = 0;
			S16 check_val = TUNER_A_PATTERN_Q;
			for (int tmp_cnt = 0; tmp_cnt < BB_INPUT_SYMBOL_SIZE; tmp_cnt ++)
			{
				if (QSampBuffer[tmp_cnt] != check_val)
					check_cnt ++;
			}

			if (check_cnt != 0)
				(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "[IQ PATTERN CHECK] iqReadBuffer QSampBuffer check_cnt = %d \n", check_cnt);
		}
#endif

		if((rc0 < (S32)bytesToRead) || (rc1 < (S32)bytesToRead)) {
			(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "IQ-samples read size is samll or not available. Iret[%d], Qret[%d]\n", rc0, rc1);
			ret = -1;
		}
	}

	if(ret == 0) {
		// Pack samples as alternating I/Q
		if(argStBbInputCtrl->vectorFormat == FORMAT_E0) {
			tchdrbbinput_packVectorE0Samples(argStBbInputCtrl->tuneInfo.band, iqscaling, ISampBuffer, QSampBuffer, outputBuffer, numSamplesNeeded);
		}
		else {
			// No matter what, the iqscaling is fixed at 1.0 in E1 vector format.
			tchdrbbinput_packVectorE1Samples(argStBbInputCtrl->tuneInfo.band, ISampBuffer, QSampBuffer, outputBuffer, numSamplesNeeded);

#ifdef IQ_PATTERN_CHECK_ENABLE
			if (numSamplesNeeded != BB_INPUT_SYMBOL_SIZE)
			{
				(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "[IQ PATTERN CHECK] tchdrbbinput_packVectorE1Samples numSamplesNeeded = %d \n", numSamplesNeeded);
			}
			else
			{
				int check_cnt = 0;
				uint_t check_val = TUNER_A_PATTERN_4BYTE;
				uint_t *check_p = outputBuffer;

				for (int tmp_cnt = 0; tmp_cnt < BB_INPUT_SYMBOL_SIZE; tmp_cnt ++)
				{
					if (check_p[tmp_cnt] != check_val)
						check_cnt ++;
				}

				if (check_cnt != 0)
					(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "[IQ PATTERN CHECK] tchdrbbinput_packVectorE1Samples check_cnt = %d \n", check_cnt);
			}
#endif
		}

		ret = (S32)numSamplesNeeded;
	}

    return ret;
}

static HDRET tchdriqinput_writeIQ01Buffer(const S8 *pIq01Buf, S32 iq01Size, U32 fBoth)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	S32 result;

	if(iq01Size > 0) {
		result = (*stCircFunc.cb_write)(&stBbInputCtrl[0].iqReadBuffer, &pIq01Buf[iq01Size*0], (*stCast.s32tou32)(iq01Size*2));
		if(result < 0) {
			UNUSED(result);
			(*pfnHdrLog)(eTAG_IQIN, eLOG_DBG, "IQ Read Buffer[0] Overrun!!!! Availiable Space[%d]\n", (*stCircFunc.cb_availSpace)(&stBbInputCtrl[0].iqReadBuffer));
		}

		if(fBoth > 0U) {
#if NUM_HDR_INSTANCES > 1
			result = (*stCircFunc.cb_write)(&stBbInputCtrl[1].iqReadBuffer, &pIq01Buf[iq01Size*2], (*stCast.s32tou32)(iq01Size*2));
			if(result < 0) {
				UNUSED(result);
				(*pfnHdrLog)(eTAG_IQIN, eLOG_DBG, "IQ Read Buffer[1] Overrun!!!! Availiable Space[%d]\n", (*stCircFunc.cb_availSpace)(&stBbInputCtrl[1].iqReadBuffer));
			}
#endif
		}
	}
	else {
		//To do: Add error handling later.
		if(iq01Size == SDR_DRV_ERRNO_XRUN) {
			ret = (HDRET)eTC_HDR_RET_NG_IQ01IN_XRUN;
		}
		else {
			UNUSED(ret);
			(*pfnHdrLog)(eTAG_IQIN, eLOG_DBG, "[%s:%d] Failed to read IQ01 driver. readSize[%d]\n", __func__, __LINE__, iq01Size);
			ret = (HDRET)eTC_HDR_RET_NG_READ_SIZE;
		}
	}

	return ret;
}

static HDRET tchdriqinput_writeIQ23Buffer(const S8 *pIq23Buf, S32 iq23Size, U32 fBoth)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

#if NUM_HDR_INSTANCES > 2
	if(iq23Size > 0) {
		S32 result;

		result = (*stCircFunc.cb_write)(&stBbInputCtrl[2].iqReadBuffer, &pIq23Buf[iq23Size*0], (*stCast.s32tou32)(iq23Size*2));
		if(result < 0) {
			UNUSED(result);
			(*pfnHdrLog)(eTAG_IQIN, eLOG_DBG, "IQ Read Buffer[2] Overrun!!!! Availiable Space[%d]\n", (*stCircFunc.cb_availSpace)(&stBbInputCtrl[2].iqReadBuffer));
		}

		if(fBoth > 0U) {
	#if NUM_HDR_INSTANCES > 3
			result = (*stCircFunc.cb_write)(&stBbInputCtrl[3].iqReadBuffer,&pIq23Buf[iq23Size*2], (*stCast.s32tou32)(iq23Size*2));
			if(result < 0) {
				UNUSED(result);
				(*pfnHdrLog)(eTAG_IQIN, eLOG_DBG, "IQ Read Buffer[3] Overrun!!!! Availiable Space[%d]\n", (*stCircFunc.cb_availSpace)(&stBbInputCtrl[3].iqReadBuffer));
			}
	#endif
		}
	}
	else {
		//To do: Add error handling later.
		if(iq23Size == SDR_DRV_ERRNO_XRUN) {
			ret = (HDRET)eTC_HDR_RET_NG_IQ23IN_XRUN;
		}
		else {
			UNUSED(ret);
			(*pfnHdrLog)(eTAG_IQIN, eLOG_DBG, "[%s:%d] Failed to read IQ23 driver. readSize[%d]\n", __func__, __LINE__, iq23Size);
			ret = (HDRET)eTC_HDR_RET_NG_READ_SIZE;
		}
	}
#else
	(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "IQ23 read buffer is invalid. Check HD Radio type.\n");
	ret = (HDRET)eTC_HDR_RET_NG_RSC;
	UNUSED(pIq23Buf);
	UNUSED(iq23Size);
	UNUSED(fBoth);
#endif

	return ret;
}

static HDRET tchdriqinput_writeByteStreamIQbuffer(const S8 *pIq01Buf, S32 iq01Size, const S8 *pIq23Buf, S32 iq23Size, U32 numOfHdrInstances)
{
	HDRET ret = (HDRET)eTC_HDR_RET_NG_INVALID_BUFFER_POINTER;

	if(numOfHdrInstances == 1U) {	// one Instance
		if(pIq01Buf != NULL) {
			ret = tchdriqinput_writeIQ01Buffer(pIq01Buf, iq01Size, 0);
		}
	}
#if NUM_HDR_INSTANCES > 1
	else if(numOfHdrInstances == 2U) {
		if(pIq01Buf != NULL) {
			ret = tchdriqinput_writeIQ01Buffer(pIq01Buf, iq01Size, 1);
		}
	}
#endif
#if NUM_HDR_INSTANCES > 2
	else if(numOfHdrInstances == 3U) {
		if((pIq01Buf != NULL) && (pIq23Buf != NULL)) {
			// How should I handle it when only one side is OK? We should think about it.
			ret = tchdriqinput_writeIQ01Buffer(pIq01Buf, iq01Size, 1);
			if(ret == (HDRET)eTC_HDR_RET_OK) {
				ret = tchdriqinput_writeIQ23Buffer(pIq23Buf, iq23Size, 0);
			}
		}
	}
#endif
#if NUM_HDR_INSTANCES > 3
	else if(numOfHdrInstances == 4U) {
		if(pIq01Buf != NULL && pIq23Buf != NULL) {
			ret = tchdriqinput_writeIQ01Buffer(pIq01Buf, iq01Size, 1);
			if(ret == (HDRET)eTC_HDR_RET_OK) {
				ret = tchdriqinput_writeIQ23Buffer(pIq23Buf, iq23Size, 1);
			}
		}
	}
#endif
	else {
		UNUSED(pIq23Buf);
		UNUSED(iq23Size);
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
	}

	return ret;
}

HDRET tchdraudinput_init(void)
{
	HDRET ret;

	stBbInputState.fAudioInputThreadStart = 0;
	(*stCircFunc.cb_init)(&analogAudioBuffer, (void*)analogAudBufData, AUDIO_DRIVER_BUFFER_SIZE/4U, (U32)sizeof(HDR_pcm_stereo_t));
	ret = (*stOsal.mutexinit)(&gTcHdrAAudBufMutex, NULL);
	analogAudioBuffer.mutex = &gTcHdrAAudBufMutex;
	tcAudioInputResampler = tcaudio_resampler_init((void*)tcAudioInputResamplerMem);
	if(tcAudioInputResampler == NULL) {
		(*pfnHdrLog)(eTAG_AIN, eLOG_ERR, "Failed to initialize external audio input resampler.\n");
		ret = (HDRET)eTC_HDR_RET_NG_AUD_RESAMPLER_INIT;
	}
	anaout_hz = anain_hz * (1. + (anaout_ppm / 1e6));
	(*pfnHdrLog)(eTAG_AIN, eLOG_DBG, "HDR audio input resampler slips: %fppm, in_hz: %fHz, out_hz: %fHz\n", anaout_ppm, anain_hz, anaout_hz);

	return ret;
}

static void tchdraudinput_deinit(void)
{
	stBbInputState.fAudioInputThreadStart = 0;
	(void)(*stCircFunc.cb_reset)(&analogAudioBuffer);
	analogAudioBuffer.mutex = pNULL;
	(void)(*stOsal.mutexdeinit)(&gTcHdrAAudBufMutex);
}

static void tchdraudinput_setEnable(S32 en)
{
	if(en > 0) {
		fBlendAudioDrvEn = 1;
	}
	else {
		fBlendAudioDrvEn = 0;
	}
}

static S32 tchdraudinput_getEnable(void)
{
	return fBlendAudioDrvEn;
}

static HDRET tchdraudinput_open(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	S32 bufferTotalSizeKbyte = (*stCast.u32tos32)(AUDIO_DRIVER_BUFFER_SIZE / 1024U); // (HDR_AUDIO_FRAME_SIZE*4*32/1024);
	S32 bufferPeriodSizeByte = (*stCast.u32tos32)(AUDIO_DRIVER_PERIOD_SIZE);

	if( (pfnTcHdrBlendAudioOpen != NULL) && (pfnTcHdrBlendAudioClose != NULL) && (pfnTcHdrBlendAudioStart !=NULL) &&
		(pfnTcHdrBlendAudioSetParams != NULL) && (pfnTcHdrBlendAudioStop != NULL) && (pfnTcHdrBlendAudioRead != NULL)) {
		ret = (*pfnTcHdrBlendAudioOpen)();
		if(ret == 0) {
			ret = (*pfnTcHdrBlendAudioSetParams)(2, 16, 44100, bufferTotalSizeKbyte, bufferPeriodSizeByte);
			if(ret < 0) {
				(void)(*pfnTcHdrBlendAudioClose)();
				ret = (HDRET)eTC_HDR_RET_NG_RSC;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
	}
	else {
		(*pfnHdrLog)(eTAG_AIN, eLOG_ERR, "The pointer of the blend audio I2S driver functions is null\n");
		ret = (HDRET)eTC_HDR_RET_NG_RSC;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		(void)(*stCircFunc.cb_reset)(&analogAudioBuffer);
		tchdraudinput_setEnable(1);
	}

	return ret;
}

HDRET tchdraudinput_close(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	stBbInputState.fAudioInputThreadStart = 0;
	tchdraudinput_setEnable(0);
	if(pfnTcHdrBlendAudioStop != pNULL) {
		(void)(*pfnTcHdrBlendAudioStop)();
	}
	if(pfnTcHdrBlendAudioClose != pNULL) {
		(void)(*pfnTcHdrBlendAudioClose)();
	}
	(void)(*stCircFunc.cb_reset)(&analogAudioBuffer);

	return ret;
}

static HDRET tchdraudinput_start(void)
{
	HDRET ret;

	if(pfnTcHdrBlendAudioStart != NULL) {
		ret = (*pfnTcHdrBlendAudioStart)();
		if(ret != 0) {
			ret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
		else {
			stBbInputState.fAudioInputThreadStart = 1;
		}
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_RSC;
		(*pfnHdrLog)(eTAG_AIN, eLOG_ERR, "The pointer of the blend audio I2S driver functions is null\n");
	}

	return ret;
}

static HDRET tchdraudinput_stop(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	stBbInputState.fAudioInputThreadStart = 0;
	(void)(*stCircFunc.cb_reset)(&analogAudioBuffer);
	if(pfnTcHdrBlendAudioStop != pNULL) {
		(void)(*pfnTcHdrBlendAudioStop)();
	}

	return ret;
}

U8 tchdraudinput_getTunedBand(U8 hdrInstance) {
    return (stBbInputCtrl[hdrInstance].tuneInfo.band == HDR_BAND_AM)?0x01:0x00;
}

U16 tchdraudinput_getTunedFreq(U8 hdrInstance) {
    return stBbInputCtrl[hdrInstance].tuneInfo.freq;
}

static void tchdrbbinput_syncTuneInfo(stTCHDR_TUNE_INFO_t tuneInfo)
{
/*
	stBbInputCtrl[x].tuneInfo and stBbpTuneSelect[x] and stTcHdrTuneInfo must be match each other!!!
*/
	U32 hdrType = tchdrfwk_getHdrType();

	stBbInputCtrl[0].tuneInfo = tuneInfo.mainInstance;

	stBbpTuneSelect[0].band = tuneInfo.mainInstance.band;
	stBbpTuneSelect[0].rfFreq = tuneInfo.mainInstance.freq;

#if HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG
	if(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG) {
		stBbInputCtrl[1].tuneInfo = tuneInfo.mrcInstance;
		stBbInputCtrl[2].tuneInfo = tuneInfo.bsInstance;
		stBbInputCtrl[3].tuneInfo = tuneInfo.bsmrcInstance;

		stBbpTuneSelect[1].band = tuneInfo.mrcInstance.band;
		stBbpTuneSelect[1].rfFreq = tuneInfo.mrcInstance.freq;
		stBbpTuneSelect[2].band = tuneInfo.bsInstance.band;
		stBbpTuneSelect[2].rfFreq = tuneInfo.bsInstance.freq;
		stBbpTuneSelect[3].band = tuneInfo.bsmrcInstance.band;
		stBbpTuneSelect[3].rfFreq = tuneInfo.bsmrcInstance.freq;
	}
	if(hdrType == (U32)HDR_1p5_MRC_CONFIG) {
		stBbInputCtrl[1].tuneInfo = tuneInfo.mrcInstance;
		stBbInputCtrl[2].tuneInfo = tuneInfo.bsInstance;

		stBbpTuneSelect[1].band = tuneInfo.mrcInstance.band;
		stBbpTuneSelect[1].rfFreq = tuneInfo.mrcInstance.freq;
		stBbpTuneSelect[2].band = tuneInfo.bsInstance.band;
		stBbpTuneSelect[2].rfFreq = tuneInfo.bsInstance.freq;
	}
	if(hdrType == (U32)HDR_1p5_CONFIG) {
		stBbInputCtrl[1].tuneInfo = tuneInfo.bsInstance;

		stBbpTuneSelect[1].band = tuneInfo.bsInstance.band;
		stBbpTuneSelect[1].rfFreq = tuneInfo.bsInstance.freq;
	}
	if(hdrType == (U32)HDR_1p0_MRC_CONFIG) {
		stBbInputCtrl[1].tuneInfo = tuneInfo.mrcInstance;

		stBbpTuneSelect[1].band = tuneInfo.mrcInstance.band;
		stBbpTuneSelect[1].rfFreq = tuneInfo.mrcInstance.freq;
	}
#elif HDR_CONFIG == HDR_1p5_MRC_CONFIG
	if(hdrType == (U32)HDR_1p5_MRC_CONFIG) {
		stBbInputCtrl[1].tuneInfo = tuneInfo.mrcInstance;
		stBbInputCtrl[2].tuneInfo = tuneInfo.bsInstance;

		stBbpTuneSelect[1].band = tuneInfo.mrcInstance.band;
		stBbpTuneSelect[1].rfFreq = tuneInfo.mrcInstance.freq;
		stBbpTuneSelect[2].band = tuneInfo.bsInstance.band;
		stBbpTuneSelect[2].rfFreq = tuneInfo.bsInstance.freq;
	}
	if(hdrType == (U32)HDR_1p5_CONFIG) {
		stBbInputCtrl[1].tuneInfo = tuneInfo.bsInstance;

		stBbpTuneSelect[1].band = tuneInfo.bsInstance.band;
		stBbpTuneSelect[1].rfFreq = tuneInfo.bsInstance.freq;
	}
	if(hdrType == (U32)HDR_1p0_MRC_CONFIG) {
		stBbInputCtrl[1].tuneInfo = tuneInfo.mrcInstance;

		stBbpTuneSelect[1].band = tuneInfo.mrcInstance.band;
		stBbpTuneSelect[1].rfFreq = tuneInfo.mrcInstance.freq;
	}
#elif HDR_CONFIG == HDR_1p5_CONFIG
	if(hdrType == (U32)HDR_1p5_CONFIG) {
		stBbInputCtrl[1].tuneInfo = tuneInfo.bsInstance;

		stBbpTuneSelect[1].band = tuneInfo.bsInstance.band;
		stBbpTuneSelect[1].rfFreq = tuneInfo.bsInstance.freq;
	}
#elif HDR_CONFIG == HDR_1p0_MRC_CONFIG
	if(hdrType == (U32)HDR_1p0_MRC_CONFIG) {
		stBbInputCtrl[1].tuneInfo = tuneInfo.mrcInstance;

		stBbpTuneSelect[1].band = tuneInfo.mrcInstance.band;
		stBbpTuneSelect[1].rfFreq = tuneInfo.mrcInstance.freq;
	}
#else // HDR_1p0_CONFIG
	UNUSED(hdrType);
#endif
}

static HDRET tchdrbbinput_setTuneMain(stTCHDR_TUNE_t inputTune, S32 fChgBand, S32 fChgFreq, S32 fChgSR)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 hdrType = tchdrfwk_getHdrType();
	U32 fMrcType = 0U;
	stHDR_FRAMEWORK_DATA_t* frameworkData = tchdrfwk_getDataStructPtr();

	if(fChgBand > 0) {
		if((hdrType == (U32)HDR_1p0_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
			fMrcType = 1;
			if(tchdrfwk_getMrcStatus()) {
				if(inputTune.band != HDR_BAND_FM) {
					ret = HDR_mrc_disable(&frameworkData->hdrInstance[0]);
					(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Disable MRC on main instance. ret[%d]\n", ret);
				}
			}
		}

		ret = HDR_set_band(&frameworkData->hdrInstance[0], stBbInputCtrl[0].tuneInfo.band);
		if(ret == (HDRET)eTC_HDR_RET_OK) {
			(void)tchdrbbinput_resetIqDrvAndBbinputSetting(eTC_HDR_ID_MAIN, inputTune);
			if((fMrcType == 1) && (inputTune.band == HDR_BAND_FM)) {
				HDR_tune_band_t curband;
				curband = HDR_get_band_select(&frameworkData->hdrInstance[1]);
				if(curband == HDR_BAND_FM) {
					ret = HDR_mrc_enable(&frameworkData->hdrInstance[0]);
					if(ret == (HDRET)eTC_HDR_RET_OK) {
						(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Enabled MRC on main instance.\n");
					}
					else {
						(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to enable MRC on main instance. ret[%d]\n", ret);
					}
					// If I did not call this HDR_reacquire API here, MRC don't work normally.
					(void)tchdrbbinput_setReacquire(&frameworkData->hdrInstance[0]);
				}
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_SET_BAND;
		}
	}
	else if((fChgFreq > 0) || (fChgSR > 0)) {
		HDR_program_t curPn = HDR_PROGRAM_HD1;
		if(fChgSR > 0) {
			if(fChgFreq <= 0) {
				(void)HDR_get_playing_program(&frameworkData->hdrInstance[0], &curPn);
			}
			(void)tchdrbbinput_resetIqDrvAndBbinputSetting(eTC_HDR_ID_MAIN, inputTune);
		}
	    ret = tchdrbbinput_setReacquire(&frameworkData->hdrInstance[0]);
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			ret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
		else {
			if(fChgFreq <= 0) {
				// Maintain the current program number when acquiring at the same frequency.
				(void)HDR_set_playing_program(&frameworkData->hdrInstance[0], curPn);
				(void)tchdrsvc_setProgramNumber(&frameworkData->hdrInstance[0], curPn);
			}
		}
	}
	else {
		HDR_program_t curPn = HDR_PROGRAM_HD1;
		(void)HDR_get_playing_program(&frameworkData->hdrInstance[0], &curPn);
		// Just call HDR_reacquire() because anything is not changed
	    ret = tchdrbbinput_setReacquire(&frameworkData->hdrInstance[0]);
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			ret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
		else {
			// Maintain the current program number when acquiring at the same frequency.
			(void)HDR_set_playing_program(&frameworkData->hdrInstance[0], curPn);
			(void)tchdrsvc_setProgramNumber(&frameworkData->hdrInstance[0], curPn);
		}
	}

	return ret;
}

static HDRET tchdrbbinput_setTuneMrc(stTCHDR_TUNE_t inputTune, S32 fChgBand, S32 fChgFreq, S32 fChgSR)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

#if (HDR_CONFIG == HDR_1p0_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
	U32 hdrType = tchdrfwk_getHdrType();
	stHDR_FRAMEWORK_DATA_t* frameworkData = tchdrfwk_getDataStructPtr();

	// Make sure that MRC instances are connected and enabled. FULL INSTANCE is connected to the DEMOD ONLY instance.
	if((hdrType == (U32)HDR_1p0_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
		if(inputTune.band == HDR_BAND_FM) {
			if((fChgBand > 0) || (fChgSR > 0)) {
				ret = tchdrbbinput_resetIqDrvAndBbinputSetting(eTC_HDR_ID_MRC, inputTune);
				if(fChgBand > 0 ) {
					HDR_tune_band_t curband;
					curband = HDR_get_band_select(&frameworkData->hdrInstance[0]);
					if(curband == HDR_BAND_FM) {
						HDBOOL fMrc = FALSE;
						ret = HDR_mrc_enabled(&frameworkData->hdrInstance[0], &fMrc);
						if(ret == (HDRET)eTC_HDR_RET_OK) {
							if(fMrc == FALSE) {
								ret = HDR_mrc_enable(&frameworkData->hdrInstance[0]);
								if(ret == (HDRET)eTC_HDR_RET_OK) {
									(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Enabled MRC on mrc instance.\n");
								}
								else {
									(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to enable MRC on mrc instance. ret[%d]\n", ret);
								}
							}
						}
						else {
							(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to check MRC status of main instance. ret[%d]\n", ret);
						}
					}
				}
			}
			else {
				// Do nothing because anything is not changed.
			}
		}
	}
#else
	ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
	UNUSED(inputTune);
	UNUSED(fChgBand);
	UNUSED(fChgSR);
#endif
	UNUSED(fChgFreq);

	return ret;
}

static HDRET tchdrbbinput_setTuneBs(stTCHDR_TUNE_t inputTune, S32 fChgBand, S32 fChgFreq, S32 fChgSR)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 hdrType = tchdrfwk_getHdrType();

#if (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
	stHDR_FRAMEWORK_DATA_t* frameworkData = tchdrfwk_getDataStructPtr();

	if(fChgBand > 0) {
	#if (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
		if(hdrType == (U32)HDR_1p5_CONFIG) {
			ret = HDR_set_band(&frameworkData->hdrInstance[1], stBbInputCtrl[1].tuneInfo.band);
			if(ret != 0) {
				ret = (HDRET)eTC_HDR_RET_NG_SET_BAND;
			}
		}
		else if((hdrType == (U32)HDR_1p5_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
			if(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG) {
				if(tchdrfwk_getBsMrcStatus()) {
					if(inputTune.band != HDR_BAND_FM) {
						ret = HDR_mrc_disable(&frameworkData->hdrInstance[2]);
						(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Disable MRC on bs instance. ret[%d]\n", ret);
					}
				}
			}
			ret = HDR_set_band(&frameworkData->hdrInstance[2], stBbInputCtrl[2].tuneInfo.band);
			if(ret != (HDRET)eTC_HDR_RET_OK) {
				ret = (HDRET)eTC_HDR_RET_NG_SET_BAND;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	#else // (HDR_CONFIG == HDR_1p5_CONFIG)
		if(hdrType == (U32)HDR_1p5_CONFIG) {
			ret = HDR_set_band(&frameworkData->hdrInstance[1], stBbInputCtrl[1].tuneInfo.band);
			if(ret != 0) {
				ret = (HDRET)eTC_HDR_RET_NG_SET_BAND;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	#endif

		if(ret == (HDRET)eTC_HDR_RET_OK) {
			(void)tchdrbbinput_resetIqDrvAndBbinputSetting(eTC_HDR_ID_BS, inputTune);
			if((hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG) && (inputTune.band == HDR_BAND_FM)) {
				HDR_tune_band_t curband;
				curband = HDR_get_band_select(&frameworkData->hdrInstance[3]);
				if(curband == HDR_BAND_FM) {
					ret = HDR_mrc_enable(&frameworkData->hdrInstance[2]);
					if(ret == (HDRET)eTC_HDR_RET_OK) {
						(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Enabled MRC on bs instance.\n");
					}
					else {
						(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to enable MRC on bs instance. ret[%d]\n", ret);
					}
					// If I did not call this HDR_reacquire API here, MRC don't work normally.
					(void)tchdrbbinput_setReacquire(&frameworkData->hdrInstance[2]);
				}
			}
		}
	}
	else {
		if(fChgSR > 0) {
			(void)tchdrbbinput_resetIqDrvAndBbinputSetting(eTC_HDR_ID_BS, inputTune);
		}

		// Just call HDR_reacquire() because anything is not changed
	#if (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
		if(hdrType == (U32)HDR_1p5_CONFIG) {
			ret = tchdrbbinput_setReacquire(&frameworkData->hdrInstance[1]);
			if(ret != (HDRET)eTC_HDR_RET_OK) {
				ret = (HDRET)eTC_HDR_RET_NG_RSC;
			}
		}
		else if((hdrType == (U32)HDR_1p5_MRC_CONFIG) || (hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
			ret = tchdrbbinput_setReacquire(&frameworkData->hdrInstance[2]);
			if(ret != (HDRET)eTC_HDR_RET_OK) {
				ret = (HDRET)eTC_HDR_RET_NG_RSC;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	#else // (HDR_CONFIG == HDR_1p5_CONFIG)
		if(hdrType == (U32)HDR_1p5_CONFIG) {
			ret = tchdrbbinput_setReacquire(&frameworkData->hdrInstance[1]);
			if(ret != (HDRET)eTC_HDR_RET_OK) {
				ret = (HDRET)eTC_HDR_RET_NG_RSC;
			}
		}
		else {
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	#endif
	}
#else
	ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
#endif

	UNUSED(hdrType);
	UNUSED(fChgFreq);
	UNUSED(inputTune);
	return ret;
}

static HDRET tchdrbbinput_setTuneBsMrc(stTCHDR_TUNE_t inputTune, S32 fChgBand, S32 fChgFreq, S32 fChgSR)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

#if HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG
	U32 hdrType = tchdrfwk_getHdrType();
	stHDR_FRAMEWORK_DATA_t* frameworkData = tchdrfwk_getDataStructPtr();

	// Make sure that MRC instances are connected and enabled. FULL INSTANCE is connected to the DEMOD ONLY instance.
	if(hdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG) {
		if(inputTune.band == HDR_BAND_FM) {
			if((fChgBand > 0) || (fChgSR > 0)) {
				ret = tchdrbbinput_resetIqDrvAndBbinputSetting(eTC_HDR_ID_BS_MRC, inputTune);
				if(fChgBand > 0 ) {
					HDR_tune_band_t curband;
					curband = HDR_get_band_select(&frameworkData->hdrInstance[2]);
					if(curband == HDR_BAND_FM) {
						HDBOOL fMrc = FALSE;
						ret = HDR_mrc_enabled(&frameworkData->hdrInstance[2], &fMrc);
						if(ret == (HDRET)eTC_HDR_RET_OK) {
							if(fMrc == FALSE) {
								ret = HDR_mrc_enable(&frameworkData->hdrInstance[2]);
								if(ret == (HDRET)eTC_HDR_RET_OK) {
									(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Enabled BS MRC on bsmrc instance.\n");
								}
								else {
									(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to enable BS MRC on bsmrc instance. ret[%d]\n", ret);
								}
							}
						}
						else {
							(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to check BS MRC status of bs instance. ret[%d]\n", ret);
						}
					}
				}
			}
			else {
				// Do nothing because anything is not changed.
			}
		}
	}
#else
	ret = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
	UNUSED(inputTune);
	UNUSED(fChgBand);
	UNUSED(fChgSR);
#endif
	UNUSED(fChgFreq);

	return ret;
}

static HDRET tchdrbbinput_setTuneProcess(eTC_HDR_ID_t id, stTCHDR_TUNE_t inputTune, S32 fChgBand, S32 fChgFreq, S32 fChgSR)
{
	HDRET ret;

	switch(id) {
		case eTC_HDR_ID_MAIN:
			ret = tchdrbbinput_setTuneMain(inputTune, fChgBand, fChgFreq, fChgSR);
			break;

		case eTC_HDR_ID_BS:
			ret =tchdrbbinput_setTuneBs(inputTune, fChgBand, fChgFreq, fChgSR);
			break;

		case eTC_HDR_ID_MRC:
			ret = tchdrbbinput_setTuneMrc(inputTune, fChgBand, fChgFreq, fChgSR);
			break;

		case eTC_HDR_ID_BS_MRC:
			ret = tchdrbbinput_setTuneBsMrc(inputTune, fChgBand, fChgFreq, fChgSR);
			break;

		default:
			/* Nothing To Do */
			break;
	}

	return ret;
}

static HDRET tchdrbbinput_setTune(eTC_HDR_ID_t id, stTCHDR_TUNE_t inputTune, U32 fNotify, U32 *uiSendMsg)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	S32 fChgBand = 0;
	S32 fChgSR = 0;
	S32 fChgFreq = 0;
	stTCHDR_TUNE_t *curTune;

	if(id == eTC_HDR_ID_MAIN) {
		curTune = &stTcHdrTuneInfo.mainInstance;
        if (curTune->band != inputTune.band) {
            (*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s][bbFreq][id == eTC_HDR_ID_MAIN] curTune->band [%s -> %s].\n"
                    , __func__, (curTune->band==HDR_BAND_AM?"AM":"FM"), (inputTune.band==HDR_BAND_AM?"AM":"FM"));
        }
        if (curTune->freq != inputTune.freq) {
            (*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "[%s][bbFreq][id == eTC_HDR_ID_MAIN] curTune->freq [%d -> %d].\n"
                    , __func__, curTune->freq, inputTune.freq);
        }
	}
	else if(id == eTC_HDR_ID_MRC) {
		curTune = &stTcHdrTuneInfo.mrcInstance;
	}
	else if(id == eTC_HDR_ID_BS) {
		curTune = &stTcHdrTuneInfo.bsInstance;
	}
	else if(id == eTC_HDR_ID_BS_MRC) {
		curTune = &stTcHdrTuneInfo.bsmrcInstance;
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK)
	{
		if(curTune->band != inputTune.band) {
			fChgBand = 1;
		}
		if(curTune->freq != inputTune.freq) {
			fChgFreq = 1;
		}
		if(curTune->iqsr != inputTune.iqsr) {
			fChgSR = 1;
		}

		*curTune = inputTune;
		tchdrbbinput_syncTuneInfo(stTcHdrTuneInfo);
		ret = tchdrbbinput_setTuneProcess(id, inputTune, fChgBand, fChgFreq, fChgSR);
	}

	if(uiSendMsg != NULL) {
		uiSendMsg[0] = (U32)id;
		uiSendMsg[1] = (U32)fChgBand;
		uiSendMsg[2] = (U32)fChgFreq;
		uiSendMsg[3] = (U32)fChgSR;
		uiSendMsg[4] = (U32)curTune->band;
		uiSendMsg[5] = (U32)curTune->freq;
		uiSendMsg[6] = (U32)curTune->iqsr;
		if(fNotify > 0U) {
			tchdrbbinput_sendNotifyMessage((U32)eTCHDR_SENDER_ID_BBINPUT, (U32)eTCHDR_BBINPUT_CMD_SET_TUNE, uiSendMsg, pNULL, ret);
		}
	}

	return ret;
}

static HDRET tchdrbbinput_openIQandAudio(void)
{
	HDRET ret;

    // struct timespec evalTimerStart;
    // struct timespec evalTimerEnd;
    // struct timespec evalTimerDiff;
    // clock_gettime(LINUX_CLOCK_TIMER_TYPE,&evalTimerStart);

	ret = tchdriqinput_open();

    // clock_gettime(LINUX_CLOCK_TIMER_TYPE,&evalTimerEnd);
    // evalTimerDiff.tv_sec = evalTimerEnd.tv_sec - evalTimerStart.tv_sec;
    // evalTimerDiff.tv_nsec = evalTimerEnd.tv_nsec - evalTimerStart.tv_nsec;
    // (*pfnHdrLog)(eTAG_SYS, eLOG_INF, ">>>>>>>>>>>>>>>>>>>> 1st tchdriqinput_open spends %d.%d msecs!!! <<<<<<<<<<<<<<<<<<<<\n"
    //         , evalTimerDiff.tv_nsec/1000000, evalTimerDiff.tv_nsec%1000000);

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdriqinput_close();
    // clock_gettime(LINUX_CLOCK_TIMER_TYPE,&evalTimerEnd);
    // evalTimerDiff.tv_sec = evalTimerEnd.tv_sec - evalTimerStart.tv_sec;
    // evalTimerDiff.tv_nsec = evalTimerEnd.tv_nsec - evalTimerStart.tv_nsec;
    // (*pfnHdrLog)(eTAG_SYS, eLOG_INF, ">>>>>>>>>>>>>>>>>>>> 2nd tchdriqinput_close spends %d.%d msecs!!! <<<<<<<<<<<<<<<<<<<<\n"
    //         , evalTimerDiff.tv_nsec/1000000, evalTimerDiff.tv_nsec%1000000);

		ret = tchdriqinput_open();
    // clock_gettime(LINUX_CLOCK_TIMER_TYPE,&evalTimerEnd);
    // evalTimerDiff.tv_sec = evalTimerEnd.tv_sec - evalTimerStart.tv_sec;
    // evalTimerDiff.tv_nsec = evalTimerEnd.tv_nsec - evalTimerStart.tv_nsec;
    // (*pfnHdrLog)(eTAG_SYS, eLOG_INF, ">>>>>>>>>>>>>>>>>>>> 3rd tchdriqinput_open spends %d.%d msecs!!! <<<<<<<<<<<<<<<<<<<<\n"
    //         , evalTimerDiff.tv_nsec/1000000, evalTimerDiff.tv_nsec%1000000);

    }

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdraudinput_open();
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdraudinput_start();
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdriqinput_start();
	}

	return ret;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
/*								Telechips HD Radio Service Event Functions							*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
static eTCHDR_EVT_STS_t tchdrbbinput_event_open(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	(void)(*stOsal.osmemcpy)((void*)&stTcHdrTuneInfo, (void*)stRcvMsgQ.uiData, (U32)sizeof(stTCHDR_TUNE_INFO_t));
	tchdrbbinput_syncTuneInfo(stTcHdrTuneInfo);

	ret = tchdrbbinput_open();

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdraudoutput_open();
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdrfwk_open();
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		ret = tchdrbbinput_openIQandAudio();
	}

	if(iError != NULL) {
		*iError = ret;
	}

	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrbbinput_event_close(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	return eEvtSt;
}

static HDRET tchdrbbinput_resetIqDrvAndBbinputSetting(eTC_HDR_ID_t hdrID, stTCHDR_TUNE_t inputTune)
{
	HDRET ret;
	U32 instanceNum = 0;

	ret = tchdrbbinput_getInstanceNumber(hdrID, &instanceNum);
	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Failed to get instance number from HD Radio ID[%d]. ret[%d]\n", (U32)hdrID, ret);
	}
	else {
		F32 bbInputSampleRate = tchdrbbinput_getPreciseIqSampleRate(inputTune.iqsr);
		if(bbInputSampleRate > 0.0f) {
			F32 tmpReadBlockSize;
			if(inputTune.band == HDR_BAND_AM) {
				// The float type should not be used the 'equal' and 'not equal' operators.
				if(bbInputSampleRate < 46.51171875f) {
					stBbInputCtrl[instanceNum].resamplerEnabled = true;
				}
				else if(bbInputSampleRate > 46.51171875f) {
					stBbInputCtrl[instanceNum].resamplerEnabled = true;
				}
				else {
					stBbInputCtrl[instanceNum].resamplerEnabled = false;
				}
				tmpReadBlockSize = (bbInputSampleRate / 46.51171875f) * (F32)SYMBOL_SIZE_AM;	// 4320samples at 744.1875Khz
				bbReadBlockSize[instanceNum] = (*stCast.f32tou32)(tmpReadBlockSize);
				numInputSamplesNeeded[instanceNum] = bbReadBlockSize[instanceNum] * (U32)8;		// 34560 samples
				numOutputSamplesNeeded[instanceNum] = HDR_AM_BB_INPUT_BUFFER_SIZE;
			}
			else {    // FM or IDLE
				// The float type should not be used the 'equal' and 'not equal' operators.
				if(bbInputSampleRate < 744.1875f) {
					stBbInputCtrl[instanceNum].resamplerEnabled = true;
				}
				else if(bbInputSampleRate > 744.1875f) {
					stBbInputCtrl[instanceNum].resamplerEnabled = true;
				}
				else {
					stBbInputCtrl[instanceNum].resamplerEnabled = false;
				}
				tmpReadBlockSize = (bbInputSampleRate / 744.1875f) * (F32)SYMBOL_SIZE_FM * 2.0f;	// 4320samples at 744.1875Khz
				bbReadBlockSize[instanceNum] = (*stCast.f32tou32)(tmpReadBlockSize);
				numInputSamplesNeeded[instanceNum] = bbReadBlockSize[instanceNum] * (U32)8;			// 34560 samples
	            numOutputSamplesNeeded[instanceNum] = HDR_FM_BB_INPUT_BUFFER_SIZE;
			}
			(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "[Band:%d] bbInputSampleRate is [%f]kHz, BBInput[%d] resampler is %s.\n", inputTune.band, bbInputSampleRate, instanceNum, ((stBbInputCtrl[instanceNum].resamplerEnabled==true) ? "enabled" : "disabled"));

			(void)tchdrbbinput_reset(hdrID);
		}
		else {
			(*pfnHdrLog)(eTAG_BBIN, eLOG_ERR, "Not supported IQ samplerate!!\n");
			ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
		}
	}

	return ret;
}

static eTCHDR_EVT_STS_t tchdrbbinput_event_set_tune(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
	stTCHDR_TUNE_t inputTune;
	U32 hdrID = stRcvMsgQ.uiData[0];

	if((hdrID == (U32)eTC_HDR_ID_MAIN) || (hdrID == (U32)eTC_HDR_ID_MRC) || (hdrID == (U32)eTC_HDR_ID_BS) || (hdrID == (U32)eTC_HDR_ID_BS_MRC)) {
		stBbinputTune.id = (eTC_HDR_ID_t)hdrID;
		(void)(*stOsal.osmemcpy)((void*)&inputTune, (void*)&stRcvMsgQ.uiData[1], (U32)sizeof(stTCHDR_TUNE_t));

		stBbinputTune.inputTune.band = inputTune.band;
		stBbinputTune.inputTune.freq = inputTune.freq;
		stBbinputTune.inputTune.iqsr = inputTune.iqsr;
		stBbinputTune.ftune = 1;

		ret = tchdrbbinput_setTune(stBbinputTune.id, stBbinputTune.inputTune, 0U, uiSendMsg);
		if(iError != NULL) {
			*iError = ret;
		}
	}
	else {
		if(iError != NULL) {
			*iError = (HDRET)eTC_HDR_RET_NG_INVALID_HDR_ID;
		}
	}

	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrbbinput_event_delay_reset_main(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	if(bbinput_reset_ready == 0) {
		bbinput_reset_ready = 1;
		bbinput_reset_counter = 3;
	}

	if(iError != NULL) {
		*iError = ret;
	}

	UNUSED(stRcvMsgQ);
	return eEvtSt;
}

static eTCHDR_EVT_STS_t tchdrbbinput_event_test(stTcHdrMsgBuf_t stRcvMsgQ, U32 *uiSendMsg, S32 *iError)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	eTCHDR_EVT_STS_t eEvtSt = eTCHDR_EVT_STS_DONE;

	if(uiSendMsg != NULL) { // Declarations and Definitions (MISRA C-2012 Rule 8.13)
		uiSendMsg[0] = 0;
	}

	U32 num_api = stRcvMsgQ.uiData[0];
	switch (num_api) {
		case (U32)eTCHDR_DEBUG_BBINPUT_BUFFER_RESET:
		case (U32)eTCHDR_DEBUG_BBINPUT_DRIVER_RESET:
		case (U32)eTCHDR_DEBUG_BBINPUT_DRV_BUF_RESET:
			ret = tchdrbbinput_resetBuffersAndDrivers(stRcvMsgQ.uiData[1], stRcvMsgQ.uiData[2]);
			break;
		case (U32)eTCHDR_DEBUG_BBINPUT_IQ_DUMP:
		#ifdef DEBUG_IQ_BUF_FILE_DUMP
			if(stRcvMsgQ.uiData[1]) {
				gDumpIBuf = fopen(DUMP_PATH"dump_i0.bin", "w");
				gDumpQBuf = fopen(DUMP_PATH"dump_q0.bin", "w");
				gDumpIqBuf = fopen(DUMP_PATH"dump_iq0.bin", "w");
				fIqdumpEnable = 1;
				(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Start to dump IQ data at /tmp folder!!!\n");
			}
			else {
				fIqdumpEnable = 0;
				if(gDumpIBuf != NULL)	fclose(gDumpIBuf);
				if(gDumpQBuf != NULL)	fclose(gDumpQBuf);
				if(gDumpIqBuf != NULL)	fclose(gDumpIqBuf);
				(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Finished to dump IQ data at /tmp folder!!!\n");
			}
		#endif
			break;
		default:
			UNUSED(0);
			break;
	}

	if(iError != NULL) {
		*iError = ret;
		if(ret != (HDRET)eTC_HDR_RET_OK) {
			eEvtSt = eTCHDR_EVT_STS_DONE_NOTIFY;
		}
	}

	return eEvtSt;
}

static int dump_flag = 0;
static FILE *giqfile;
//////////////////////////////////////////////////////////////////////////////////////////////////////
/*								Telechips HD Radio IQ Data Functions							*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
static HDRET tchdriqinput_readTunerIqSamples(U32 chunkSymbols01, U32 chunkSymbols23)
{
	HDRET ret = (HDRET)eTC_HDR_RET_NG_IQ_INPUT_DRIVER;
	S32 ret_size0;
	S32 ret_size1;
	U32 numOfHdrInstances = tchdrfwk_getNumOfHdrInstance();
	U32 chunkSizeIQ01 = (*stArith.u32mul)(chunkSymbols01, IQ_SAMPLE_SIZE);
	U32 chunkSizeIQ23 = (*stArith.u32mul)(chunkSymbols23, IQ_SAMPLE_SIZE);

#ifdef USE_EVALUATION_MODE
	if(evalMode != (U32)100) {
		clock_gettime(LINUX_CLOCK_TIMER_TYPE,&evalTimerEnd);
		evalTimerDiff.tv_sec = evalTimerEnd.tv_sec - evalTimerStart.tv_sec;
		evalTimerDiff.tv_nsec = evalTimerEnd.tv_nsec - evalTimerStart.tv_nsec;
		if(evalTimerDiff.tv_nsec < 0) {
			evalTimerDiff.tv_sec--;
			evalTimerDiff.tv_nsec += 1e9;
		}
		evalAccumulatedTimerCount += (evalTimerDiff.tv_sec*1000) + (evalTimerDiff.tv_nsec/1000000);
		if(evalAccumulatedTimerCount >= LIMIT_EVALUATION_TIME) {
			(*pfnHdrLog)(eTAG_SYS, eLOG_INF, ">>>>>>>>>>>>>>>>>>>> HD Radio stops because the evaluation time has expired!!! <<<<<<<<<<<<<<<<<<<<\n");
			evalMode = (U32)100;
		}
		clock_gettime(LINUX_CLOCK_TIMER_TYPE,&evalTimerStart);
	}
#endif

	if((numOfHdrInstances == (U32)3) || (numOfHdrInstances == (U32)4)) {
		if((pfnTcHdrIQ01Read != NULL) && (pfnTcHdrIQ23Read != NULL) && (readIQ01 != NULL) && (readIQ23 != NULL) && (chunkSymbols01 > 0U) && (chunkSymbols23 > 0U)) {
			ret_size0 = (*pfnTcHdrIQ01Read)(readIQ01, (*stCast.u32tos32)(chunkSizeIQ01));
			ret_size1 = (*pfnTcHdrIQ23Read)(readIQ23, (*stCast.u32tos32)(chunkSizeIQ23));
		#ifdef USE_EVALUATION_MODE
			if(evalMode != (U32)100) {
				ret = tchdriqinput_writeByteStreamIQbuffer(readIQ01, ret_size0, readIQ23, ret_size1, numOfHdrInstances);
			}
			else {
				ret = (HDRET)eTC_HDR_RET_OK;
			}
		#else
			ret = tchdriqinput_writeByteStreamIQbuffer(readIQ01, ret_size0, readIQ23, ret_size1, numOfHdrInstances);
		#endif
		}
	}
	else {	// one or two Instance
		if((pfnTcHdrIQ01Read != NULL) && (readIQ01 != NULL) && (chunkSymbols01 > 0U)) {
			ret_size0 = (*pfnTcHdrIQ01Read)(readIQ01, (*stCast.u32tos32)(chunkSizeIQ01));

#ifdef IQ_PATTERN_CHECK_ENABLE
		if (ret_size0 > 0)
		{
			int check_cnt = 0;

			if (ret_size0 != (BB_INPUT_SYMBOL_SIZE * 2))
			{
				(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "[IQ PATTERN CHECK] pfnTcHdrIQ01Read return = %d \n", ret_size0);
			}
			else
			{
				for (int tmp_cnt = 0; tmp_cnt < (BB_INPUT_SYMBOL_SIZE * 2); tmp_cnt += 2)
				{
					if (readIQ01[tmp_cnt] != TUNER_A_PATTERN_BYTE2 || readIQ01[tmp_cnt + 1] != TUNER_A_PATTERN_BYTE3)
					{
						check_cnt ++;
					}

					if (readIQ01[tmp_cnt + (BB_INPUT_SYMBOL_SIZE * 2)] != TUNER_A_PATTERN_BYTE0 || readIQ01[tmp_cnt + (BB_INPUT_SYMBOL_SIZE * 2) + 1] != TUNER_A_PATTERN_BYTE1)
					{
						check_cnt ++;
					}
				}

				if (check_cnt != 0)
					(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "[IQ PATTERN CHECK] pfnTcHdrIQ01Read check_cnt = %d \n", check_cnt);
			}
		}
#endif

		#ifdef DEBUG_IQ_BUF_FILE_DUMP
			if(fIqdumpEnable) {
				fwrite(readIQ01, 1, ret_size0, gDumpIBuf);
				fwrite(readIQ01+ret_size0, 1, ret_size0, gDumpQBuf);
			}
		#endif
		#ifdef USE_EVALUATION_MODE
			if(evalMode != (U32)100) {
				ret = tchdriqinput_writeByteStreamIQbuffer(readIQ01, ret_size0, readIQ23, 0, numOfHdrInstances);
			}
			else {
				ret = (HDRET)eTC_HDR_RET_OK;
			}
		#else
			ret = tchdriqinput_writeByteStreamIQbuffer(readIQ01, ret_size0, readIQ23, 0, numOfHdrInstances);
		#endif
		}
		else {
		}
	}

	return ret;
}

HDRET tchdriqinput_init(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 numOfInstances = tchdrfwk_getNumOfHdrInstance();

	stBbInputState.fIqInputThreadStart = 0;

	if(numOfInstances == (U32)2) {
		readIQ01 = (*stOsal.osmalloc)(IQ_CHUNK_MAX_SIZE*4U);
		if(readIQ01 == NULL) {
			(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to malloc buffer for readIQ01\n");
			ret = (HDRET)eTC_HDR_RET_NG_MALLOC;
		}
	}
	else if(numOfInstances == (U32)3) {
		readIQ01 = (*stOsal.osmalloc)(IQ_CHUNK_MAX_SIZE*4U);
		if(readIQ01 == NULL) {
			(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to malloc buffer for readIQ01\n");
			ret = (HDRET)eTC_HDR_RET_NG_MALLOC;
		}
		readIQ23 = (*stOsal.osmalloc)(IQ_CHUNK_MAX_SIZE*2U);
		if(readIQ23 == NULL) {
			(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to malloc buffer for readIQ23\n");
			ret = (HDRET)eTC_HDR_RET_NG_MALLOC;
		}
	}
	else if(numOfInstances == (U32)4) {
		readIQ01 = (*stOsal.osmalloc)(IQ_CHUNK_MAX_SIZE*4U);
		if(readIQ01 == NULL) {
			(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to malloc buffer for readIQ01\n");
			ret = (HDRET)eTC_HDR_RET_NG_MALLOC;
		}
		readIQ23 = (*stOsal.osmalloc)(IQ_CHUNK_MAX_SIZE*4U);
		if(readIQ23 == NULL) {
			(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to malloc buffer for readIQ23\n");
			ret = (HDRET)eTC_HDR_RET_NG_MALLOC;
		}
	}
	else {	// one Instance
		readIQ01 = (*stOsal.osmalloc)(IQ_CHUNK_MAX_SIZE*2U);
		if(readIQ01 == NULL) {
			(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to malloc buffer for readIQ01\n");
			ret = (HDRET)eTC_HDR_RET_NG_MALLOC;
		}
	}

	if(ret != (HDRET)eTC_HDR_RET_OK) {
		(*stOsal.osfree)((void*)readIQ01);
		(*stOsal.osfree)((void*)readIQ23);
		(*pfnHdrLog)(eTAG_SYS, eLOG_ERR, "Failed to initialize external IQ input buffer\n");
	}

	return ret;
}

static U32 tchdriqinput_getSamplerate(U32 ntuner)
{
	U32 samplerate = 0;

	samplerate = 372094;	// Because the L and R channels are either I or Q, I2S samplerate is the IQ samplerate divided by 2.

	UNUSED(ntuner);
	return samplerate;
}

static HDRET tchdriqinput_deinit(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 numOfInstances = tchdrfwk_getNumOfHdrInstance();

	stBbInputState.fIqInputThreadStart = 0;
	if((numOfInstances == (U32)1) || (numOfInstances == (U32)2)) {
		(*stOsal.osfree)((void*)readIQ01);
	}
	else {
		(*stOsal.osfree)((void*)readIQ01);
		(*stOsal.osfree)((void*)readIQ23);
	}

	return ret;
}

static void tchdriqinput_setIq01Enable(U32 en)
{
	if(en > 0U) {
		fExtIq01DrvEn = 1U;
	}
	else {
		fExtIq01DrvEn = 0U;
	}
}

static void tchdriqinput_setIq23Enable(U32 en)
{
	if(en > 0U) {
		fExtIq23DrvEn = 1U;
	}
	else {
		fExtIq23DrvEn = 0U;
	}
}

static U32 tchdriqinput_getIq01Enable(void)
{
	return fExtIq01DrvEn;
}

static U32 tchdriqinput_getIq23Enable(void)
{
	return fExtIq23DrvEn;
}

static HDRET tchdriqinput_openIQ01Drv(S32 nch, S32 nbit, S32 samplerate, S32 buffersize, S32 periodsize)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if((pfnTcHdrIQ01Open != NULL) && (pfnTcHdrIQ01Close != NULL) && (pfnTcHdrIQ01SetParams != NULL) && (pfnTcHdrIQ01Start != NULL) && (pfnTcHdrIQ01Stop != NULL) && (pfnTcHdrIQ01Read != NULL)) {
		ret = (*pfnTcHdrIQ01Open)();
		if(ret == 0) {
			ret = (*pfnTcHdrIQ01SetParams)(nch, nbit, samplerate, buffersize, periodsize);
			if(ret < 0) {
				(void)(*pfnTcHdrIQ01Close)();
				(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to set parameters of IQ01 driver\n");
				ret = (HDRET)eTC_HDR_RET_NG_RSC;
			}
		}
		else {
			(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to open IQ01 driver\n");
			ret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
	}
	else {
		(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "IQ01 driver functions are not registered\n");
		ret = (HDRET)eTC_HDR_RET_NG_RSC;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		tchdriqinput_setIq01Enable(1);
	}
	else {
		tchdriqinput_setIq01Enable(0);
	}

	return ret;
}

static HDRET tchdriqinput_openIQ23Drv(S32 nch, S32 nbit, S32 samplerate, S32 buffersize, S32 periodsize)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if((pfnTcHdrIQ23Open != NULL) && (pfnTcHdrIQ23Close != NULL) && (pfnTcHdrIQ23SetParams != NULL) && (pfnTcHdrIQ23Start != NULL) && (pfnTcHdrIQ23Stop != NULL) && (pfnTcHdrIQ23Read != NULL)) {
		ret = (*pfnTcHdrIQ23Open)();
		if(ret == 0) {
			ret = (*pfnTcHdrIQ23SetParams)(nch, nbit, samplerate, buffersize, periodsize);
			if(ret < 0) {
				(void)(*pfnTcHdrIQ23Close)();
				(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to set parameters of IQ23 driver\n");
				ret = (HDRET)eTC_HDR_RET_NG_RSC;
			}
		}
		else {
			(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to open IQ23 driver\n");
			ret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
	}
	else {
		(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "IQ23 driver functions are not registered\n");
		ret = (HDRET)eTC_HDR_RET_NG_RSC;
	}

	if(ret == (HDRET)eTC_HDR_RET_OK) {
		tchdriqinput_setIq23Enable(1);
	}
	else {
		tchdriqinput_setIq23Enable(0);
	}

	return ret;
}

static HDRET tchdriqinput_open(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	ret = tchdriqinput_openManually(1,1);

	return ret;
}

HDRET tchdriqinput_close(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 numOfInstances = tchdrfwk_getNumOfHdrInstance();

	stBbInputState.fIqInputThreadStart = 0;
	tchdriqinput_setIq01Enable(0);
	tchdriqinput_setIq23Enable(0);

	if((numOfInstances == (U32)1) || (numOfInstances == (U32)2)) {
		if(pfnTcHdrIQ01Stop != NULL) {
			(void)(*pfnTcHdrIQ01Stop)();
		}
		if(pfnTcHdrIQ01Close != NULL) {
			(void)(*pfnTcHdrIQ01Close)();
		}
	}
	else {
		if(pfnTcHdrIQ01Stop != NULL) {
			(void)(*pfnTcHdrIQ01Stop)();
		}
		if(pfnTcHdrIQ01Close != NULL) {
			(void)(*pfnTcHdrIQ01Close)();
		}
		if(pfnTcHdrIQ23Stop != NULL) {
			(void)(*pfnTcHdrIQ23Stop)();
		}
		if(pfnTcHdrIQ23Close != NULL) {
			(void)(*pfnTcHdrIQ23Close)();
		}
	}

	return ret;
}

static HDRET tchdriqinput_start(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	S32 IQ23_numOfChannels;
	U32 numOfInstances = tchdrfwk_getNumOfHdrInstance();

	stBbInputState.fIqInputThreadStart = 1;

	if(numOfInstances == (U32)3) {
		IQ23_numOfChannels = 2;
	}
	else if(numOfInstances == (U32)4) {
		IQ23_numOfChannels = 4;
	}
	else {	// one instance or two instance
		IQ23_numOfChannels = 0;
	}

	if(pfnTcHdrIQ01Start != NULL) {
		ret = (*pfnTcHdrIQ01Start)();
		if(ret != 0) {
			(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to start IQ01 driver\n");
			ret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
	}
	else {
		(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "IQ01 driver start functions are not registered\n");
		ret = (HDRET)eTC_HDR_RET_NG_RSC;
	}

	if((ret == (HDRET)eTC_HDR_RET_OK) && (IQ23_numOfChannels > 0)) {
		if(pfnTcHdrIQ23Start != NULL) {
			ret = (*pfnTcHdrIQ23Start)();
			if(ret != 0) {
				(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to start IQ23 driver\n");
				ret = (HDRET)eTC_HDR_RET_NG_RSC;
			}
		}
		else {
			(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "IQ23 driver start functions are not registered\n");
			ret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
	}

	return ret;
}

static HDRET tchdriqinput_stop(void)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	U32 numOfInstances = tchdrfwk_getNumOfHdrInstance();

	stBbInputState.fIqInputThreadStart = 0;
	tchdriqinput_setIq01Enable(0);
	tchdriqinput_setIq23Enable(0);

	if((numOfInstances == (U32)1) || (numOfInstances == (U32)2)) {
		if(pfnTcHdrIQ01Stop != NULL) {
			(void)(*pfnTcHdrIQ01Stop)();
		}
	}
	else {
		if(pfnTcHdrIQ01Stop != NULL) {
			(void)(*pfnTcHdrIQ01Stop)();
		}
		if(pfnTcHdrIQ23Stop != NULL) {
			(void)(*pfnTcHdrIQ23Stop)();
		}
	}

	return ret;
}

static void tchdrbbinput_setHighestPriority(void)
{
	S32 ret;
	S32 policy = 0;

     // We'll operate on the currently running thread.
     pthread_t this_thread = pthread_self();
	 //pthread_getname_np(this_thread, cName);
	 //(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "This thread name is %s\n", cName);

	 // struct sched_param is used to store the scheduling priority
     struct sched_param params;

     // We'll set the priority to the maximum.
     params.sched_priority = sched_get_priority_max(HDR_FWRK_THREADS_POLICY);

	 (*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Trying to set thread realtime highest prio = %d\n", params.sched_priority);

     // Attempt to set thread real-time priority to the SCHED_FIFO policy
     ret = pthread_setschedparam(this_thread, HDR_FWRK_THREADS_POLICY, &params);
     if (ret != 0) {
         (*pfnHdrLog)(eTAG_BBIN, eLOG_WRN, "Unsuccessful in setting thread realtime highest prio\n");
     }
     else {
		 // Now verify the change in thread priority
	     ret = pthread_getschedparam(this_thread, &policy, &params);
	     if (ret != 0) {
	         (*pfnHdrLog)(eTAG_BBIN, eLOG_WRN, "Couldn't retrieve real-time scheduling temp paramers\n");
	     }
     }

     // Print thread scheduling priority
     (*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Thread highest priority is %d\n", params.sched_priority);
}

static void tchdrbbinput_setOwnPriority(void)
{
	S32 ret;
	S32 policy = 0;

     // We'll operate on the currently running thread.
     pthread_t this_thread = pthread_self();
	 //pthread_getname_np(this_thread, cName);
	 //(*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "This thread name is %s\n", cName);

	 // struct sched_param is used to store the scheduling priority
     struct sched_param params;

     // We'll set the priority to the maximum.
     params.sched_priority = BB_INPUT_THREAD_PRIORITY;

	 (*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Trying to set thread realtime own prio = %d\n", params.sched_priority);

     // Attempt to set thread real-time priority to the SCHED_FIFO policy
     ret = pthread_setschedparam(this_thread, HDR_FWRK_THREADS_POLICY, &params);
     if (ret != 0) {
         (*pfnHdrLog)(eTAG_BBIN, eLOG_WRN, "Unsuccessful in setting thread realtime own prio\n");
     }
     else {
		 // Now verify the change in thread priority
	     ret = pthread_getschedparam(this_thread, &policy, &params);
	     if (ret != 0) {
	         (*pfnHdrLog)(eTAG_BBIN, eLOG_WRN, "Couldn't retrieve real-time scheduling own paramers\n");
	     }
     }

     // Print thread scheduling priority
     (*pfnHdrLog)(eTAG_BBIN, eLOG_DBG, "Thread own priority is %d\n", params.sched_priority);
}

static HDRET tchdriqinput_openManually(S32 fOpenIQ01, S32 fOpenIQ23)	// 0: not open, 1: open
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	S32 IQ01_numOfChannels;
	S32 IQ23_numOfChannels;
	S32 samplingBit = (S32)IQ_SAMPLE_SIZE * 8;
	S32 bufferTotalSizeKbyte = (*stCast.u32tos32)(IQ_DRIVER_BUFFER_SIZE / 1024U);
	S32 bufferPeriodSizeByte = (*stCast.u32tos32)(IQ_DRIVER_PERIOD_SIZE);
	U32 numOfInstances = tchdrfwk_getNumOfHdrInstance();

	if(numOfInstances == (U32)2) {
		IQ01_numOfChannels = 4;
		IQ23_numOfChannels = 0;
	}
	else if(numOfInstances == (U32)3) {
		IQ01_numOfChannels = 4;
		IQ23_numOfChannels = 2;
	}
	else if(numOfInstances == (U32)4) {
		IQ01_numOfChannels = 4;
		IQ23_numOfChannels = 4;
	}
	else {	// one Instance
		IQ01_numOfChannels = 2;
		IQ23_numOfChannels = 0;
	}

	if(fOpenIQ01 > 0) {
		ret = tchdriqinput_openIQ01Drv(IQ01_numOfChannels, samplingBit, (S32)tchdriqinput_getSamplerate(0), bufferTotalSizeKbyte, bufferPeriodSizeByte);
	}

	if((ret == (HDRET)eTC_HDR_RET_OK) && (IQ23_numOfChannels > 0) && (fOpenIQ23 > 0)) {
		ret = tchdriqinput_openIQ23Drv(IQ23_numOfChannels, samplingBit, (S32)tchdriqinput_getSamplerate(2), bufferTotalSizeKbyte, bufferPeriodSizeByte);
	}

	return ret;
}

static void tchdriqinput_closeManually(S32 fCloseIQ01, S32 fCloseIQ23)	// 0: not close, 1: close
{
	stBbInputState.fIqInputThreadStart = 0;
	if(fCloseIQ01 > (S32)0) {
		if(pfnTcHdrIQ01Stop != NULL) {
			(void)(*pfnTcHdrIQ01Stop)();
		}
		if(pfnTcHdrIQ01Close != NULL) {
			(void)(*pfnTcHdrIQ01Close)();
		}
	}
	if(fCloseIQ23 > (S32)0) {
		if(pfnTcHdrIQ23Stop != NULL) {
			(void)(*pfnTcHdrIQ23Stop)();
		}
		if(pfnTcHdrIQ23Close != NULL) {
			(void)(*pfnTcHdrIQ23Close)();
		}
	}
}

static HDRET tchdriqinput_startManually(S32 fOpenIQ01, S32 fOpenIQ23)	// 0: not open, 1: open
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;
	S32 IQ23_numOfChannels;
	U32 numOfInstances = tchdrfwk_getNumOfHdrInstance();

	stBbInputState.fIqInputThreadStart = 1;
	if(numOfInstances == (U32)3) {
		IQ23_numOfChannels = 2;
	}
	else if(numOfInstances == (U32)4) {
		IQ23_numOfChannels = 4;
	}
	else {	// one instance or two instance
		IQ23_numOfChannels = 0;
	}

	if(fOpenIQ01 > 0) {
		if(pfnTcHdrIQ01Start != NULL) {
			ret = (*pfnTcHdrIQ01Start)();
			if(ret != 0) {
				(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to start IQ01 driver\n");
				ret = (HDRET)eTC_HDR_RET_NG_RSC;
			}
		}
		else {
			(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "IQ01 driver start functions are not registered\n");
			ret = (HDRET)eTC_HDR_RET_NG_RSC;
		}
	}

	if(IQ23_numOfChannels > 0) {
		if((ret == (HDRET)eTC_HDR_RET_OK) && (fOpenIQ23 > 0)) {
			if(pfnTcHdrIQ23Start != NULL) {
				ret = (*pfnTcHdrIQ23Start)();
				if(ret != 0) {
					(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "Failed to start IQ23 driver\n");
					ret = (HDRET)eTC_HDR_RET_NG_RSC;
				}
			}
			else {
				(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "IQ23 driver start functions are not registered\n");
				ret = (HDRET)eTC_HDR_RET_NG_RSC;
			}
		}
	}
	else {
		(*pfnHdrLog)(eTAG_IQIN, eLOG_WRN, "It is not able to start because it does not use the IQ23 driver\n");
	}

	return ret;
}

F64 tchdraudinput_setResamplerSlips(F64 ppm)
{
	anaout_ppm = ppm;
	anaout_hz = anain_hz * (1. + (anaout_ppm / 1e6));
	(*pfnHdrLog)(eTAG_AIN, eLOG_DBG, "HDR audio input resampler slips: %fppm, in_hz: %fHz, out_hz: %fHz\n", anaout_ppm, anain_hz, anaout_hz);
	return anaout_hz;
}

void tchdraudinput_getResamplerSlips(F64 *ppm, F64 *out_hz)
{
	if((ppm != NULL) && (out_hz != NULL)) {
		*ppm = anaout_ppm;
		*out_hz = anaout_hz;
	}
}

// static count = 0;
static S32 tchdr_readAudioInputProcess(S16 *anaAudioInBuffer, S16 * anaAudioOutBuffer)
{
	S32 ret=0;
	static U32 prevInputSize=0;

    audio_in_out_count++; // +1
	ret= (*pfnTcHdrBlendAudioRead)((S8*)anaAudioInBuffer, (S32)HDR_AUDIO_FRAME_SIZE);
    audio_in_out_count++; // +2
    // (*pfnHdrLog)(eTAG_AIN, eLOG_ERR, "%s pfnTcHdrBlendAudioRead audioIn[%d]\n", __func__, audio_in_out_count);
    // FOR_TEST
    // if (count == 10000) {
    //     (void)usleep(5000000);	// 5000ms sleep
    // }
    // count++;
    // ~FOR_TEST
	if(ret > 0) {
		ret = tcaudio_resampler_exec(tcAudioInputResampler, anaAudioInBuffer, anaAudioOutBuffer, ret/4, anain_hz, anaout_hz);
		if(ret > 0) {
			if(prevInputSize != (*stCast.s32tou32)(ret)) {
				//(*pfnHdrLog)(eTAG_AIN, eLOG_DBG, "Changed to %d samples after going through audio resampler.\n", ret);
				prevInputSize = (*stCast.s32tou32)(ret);
			}
		#ifdef USE_EVALUATION_MODE
			if(evalMode != (U32)100) {
				ret = (*stCircFunc.cb_write)(&analogAudioBuffer, (void*)anaAudioOutBuffer, (U32)ret);
				if(ret < 0) {
					(*pfnHdrLog)(eTAG_AIN, eLOG_ERR, "Analog audio(FM/AM) input buffer overrun! Please check tuner audio I2S sample rate.\n");
				}
			}
		#else
			ret = (*stCircFunc.cb_write)(&analogAudioBuffer, (void*)anaAudioOutBuffer, (U32)ret);
			if(ret < 0) {
				(*pfnHdrLog)(eTAG_AIN, eLOG_ERR, "Analog audio(FM/AM) input buffer overrun! Please check tuner audio I2S sample rate.\n");
			}
		#endif
		}
		else {
			(*pfnHdrLog)(eTAG_AIN, eLOG_ERR, "Analog audio input resampler parameter error! [%d]\n", ret);
		}
	}
	else {
		if(ret == SDR_DRV_ERRNO_XRUN) {
			// RF IQ input buffer or audio input buffer overrun causes a shift between digital and analog audio.
			// Recovery function for DMA buffer overrun.
			(*pfnHdrLog)(eTAG_AIN, eLOG_WRN, "Analog audio input driver xrun!!!\n");
			(void)tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_AUDIN, (U32)eTCHDR_BBINPUT_CMD_DELAY_RESET_MAIN, pNULL, pNULL, 0);
		}
		else {
			(*pfnHdrLog)(eTAG_AIN, eLOG_ERR, "Analog audio input driver error[%d]!!!\n", ret);
		}
	}

	return ret;
}

void *tchdr_audioInputThread(void* arg)
{
	S32 ret=0;
	S16 anaAudioInBuffer[HDR_AUDIO_FRAME_SIZE];
	S16 anaAudioOutBuffer[HDR_AUDIO_FRAME_SIZE];

	(*stOsal.setPostThreadAttr)(stAudioInputAttr, eTAG_AIN);	// Set thread nice & name

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	audinput_DumpFile = fopen(DUMP_PATH"audinput_thread_status.csv", "w");
	clock_gettime(LINUX_CLOCK_TIMER_TYPE, &audinput_ChkTimer);
#endif

	stAudioInputAttr.thread_running = 1;
	while(stAudioInputAttr.thread_running > 0) {
		if((tchdrsvc_getOpenStatus() == (U32)1) && (stBbInputState.fAudioInputThreadStart == 1)) {
			ret = tchdr_readAudioInputProcess(anaAudioInBuffer, anaAudioOutBuffer);
		}
		else {
			(void)usleep(5000);	// 5ms sleep
		}
#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &audinput_ChkTimeNow);
		audinput_ChkTimeDiff.tv_sec = audinput_ChkTimeNow.tv_sec - audinput_ChkTimer.tv_sec;
		audinput_ChkTimeDiff.tv_nsec = audinput_ChkTimeNow.tv_nsec - audinput_ChkTimer.tv_nsec;
		if(audinput_ChkTimeDiff.tv_nsec < 0) {
			audinput_ChkTimeDiff.tv_sec--;
			audinput_ChkTimeDiff.tv_nsec += 1e9;
		}
		audinput_LoopMs = (audinput_ChkTimeDiff.tv_sec*1000) + (audinput_ChkTimeDiff.tv_nsec/1000000);
		if(audinput_DumpFile != NULL) {
			S32 frames = ret;
			S32 availspace = (*stCircFunc.cb_availData)(&analogAudioBuffer);
			S32 outputavailspace = (*stCircFunc.cb_availData)(&digitalAudioBuffer);
			fprintf(audinput_DumpFile, "%d,%d,%d,%d,%d,%f,%d\n", audinput_DumpCount++, audinput_AccumMs+=audinput_LoopMs, audinput_LoopMs, availspace, frames, anaout_hz, outputavailspace);
			fflush(audinput_DumpFile);
		}
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &audinput_ChkTimer);
#endif
	}

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	if(audinput_DumpFile != NULL) {
		fclose(audinput_DumpFile);
	}
#endif
	stAudioInputAttr.thread_running = -1;
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrAudInput Thread Sequence 01...\n");
	return pNULL;
}

static void tchdr_readIqPerocess(void)
{
	S32 ret;
	U32 chunksymbols01 = bbReadBlockSize[eTC_HDR_ID_MAIN];
	U32 chunksymbols23 = 0U;

#if (HDR_CONFIG == HDR_1p5_MRC_CONFIG) || (HDR_CONFIG == HDR_1p5_DUAL_MRC_CONFIG)
	U32 currentHdrType = tchdrfwk_getHdrType();
	if((currentHdrType == (U32)HDR_1p5_MRC_CONFIG) || (currentHdrType == (U32)HDR_1p5_DUAL_MRC_CONFIG)) {
		chunksymbols23 = bbReadBlockSize[eTC_HDR_ID_BS];
	}
#endif

	ret = tchdriqinput_readTunerIqSamples(chunksymbols01, chunksymbols23);
	if(ret == (HDRET)eTC_HDR_RET_OK) {		// To prevent it from becoming 1 when the framework is closed.
		stBbInputState.fBbInputThreadStart = 1;
		tchdriqinput_ready();
	}
	else {
		if(ret == (HDRET)eTC_HDR_RET_NG_IQ01IN_XRUN) {
			// RF IQ input buffer or audio input buffer overrun causes a shift between digital and analog audio.
			// Recovery function for DMA buffer overrun.
			(*pfnHdrLog)(eTAG_IQIN, eLOG_WRN, "IQ01 input driver xrun!!!\n");
			(void)tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_IQIN, (U32)eTCHDR_BBINPUT_CMD_DELAY_RESET_MAIN, pNULL, pNULL, 0);
		}
		else {
			(*pfnHdrLog)(eTAG_IQIN, eLOG_ERR, "IQ input driver error[%d]!!!\n", ret);
		}
	}
}

void *tchdr_rfIqInputThread(void* arg)
{
	(*stOsal.setPostThreadAttr)(stRfIqInputAttr, eTAG_IQIN);	// Set thread nice & name

#ifdef USE_EVALUATION_MODE
	clock_gettime(LINUX_CLOCK_TIMER_TYPE,&evalTimerStart);
#endif
#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	iqinput_DumpFile = fopen(DUMP_PATH"iqinput_thread_status.csv", "w");
	clock_gettime(LINUX_CLOCK_TIMER_TYPE, &iqinput_ChkTimer);
#endif

	stRfIqInputAttr.thread_running = 1;
	while(stRfIqInputAttr.thread_running > 0) {
		if((tchdrsvc_getOpenStatus() == (U32)1) && (stBbInputState.fIqInputThreadStart == 1)) {
			tchdr_readIqPerocess();
		}
		else {
			(void)usleep(5000);	// 5ms sleep
		}
#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &iqinput_ChkTimeNow);
		iqinput_ChkTimeDiff.tv_sec = iqinput_ChkTimeNow.tv_sec - iqinput_ChkTimer.tv_sec;
		iqinput_ChkTimeDiff.tv_nsec = iqinput_ChkTimeNow.tv_nsec - iqinput_ChkTimer.tv_nsec;
		if(iqinput_ChkTimeDiff.tv_nsec < 0) {
			iqinput_ChkTimeDiff.tv_sec--;
			iqinput_ChkTimeDiff.tv_nsec += 1e9;
		}
		iqinput_LoopMs = (iqinput_ChkTimeDiff.tv_sec*1000) + (iqinput_ChkTimeDiff.tv_nsec/1000000);
		if(iqinput_DumpFile != NULL) {
			S32 iq0Samples = (*stCircFunc.cb_availData)(&stBbInputCtrl[0].iqReadBuffer);
			fprintf(iqinput_DumpFile, "%d,%d,%d,%d\n", iqinput_DumpCount++, iqinput_AccumMs+=iqinput_LoopMs, iqinput_LoopMs, iq0Samples);
			fflush(iqinput_DumpFile);
		}
		clock_gettime(LINUX_CLOCK_TIMER_TYPE, &iqinput_ChkTimer);
#endif
	}

#ifdef DEBUG_ALL_THREAD_STATUS_DUMP
	if(iqinput_DumpFile != NULL) {
		fclose(iqinput_DumpFile);
	}
#endif
#ifdef DEBUG_IQ_BUF_FILE_DUMP
	if(gDumpIBuf != NULL)	fclose(gDumpIBuf);
	if(gDumpQBuf != NULL)	fclose(gDumpQBuf);
	if(gDumpIqBuf != NULL)	fclose(gDumpIqBuf);
#endif
	stRfIqInputAttr.thread_running = -1;
	if(arg != NULL) {
		// MISRA C-2012 Pointer Type Conversions (MISRA C-2012 Rule 11.5)
		// Note: Instead of MISRA C-2012 Rule 8.13 const warning, modify it to be Rule 11.5 Warning in DR.
		*(U8*)arg = 0;
	}
	(*pfnHdrLog)(eTAG_SYS, eLOG_DBG, ">>>>>>> Deinit TcHdrAudInput Thread Sequence 02...\n");
	return pNULL;
}
