/*******************************************************************************

*   FileName : tcradio_sound.c

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
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/prctl.h>

#include "tcradio_api.h"
#include "tcradio_utils.h"
#include "tcradio_thread.h"
#include "tcradio_msgq.h"
#include "tcradio_hal_config.h"
//#include "tcradio_service.h"
#include "tcradio_sound.h"
#include "tcradio_hal_fifo.h"

#ifdef USE_HDRADIO
#include "tchdr_api.h"
#endif

/***************************************************
*        Global variable definitions               *
****************************************************/

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/

/***************************************************
*             Local preprocessor                   *
****************************************************/
//#define USE_ALSA_AIN_AS_INPUT_AUDIO

// #define DEBUG_TCRADIO_AUDIO_OUTPUT_DUMP
//#define DEBUG_TCRADIO_AUDIO_OUTPUT_AVAIL_DUMP	// Test on ALSA device hw0,0. It is incorrect on pulsmedia.

/***************************************************
*           Local constant definitions              *
****************************************************/
static pthread_t soundMainThreadID = (pthread_t)NULL;
static stRADIO_SOUND_t stRadioSound = {0, 0, 0, 0, eSOUND_EVT_NULL, eSOUND_SOURCE_NULL};
static stAUDIO_CONFIG_t stSoundConfig;
static uint8 soundBuffer[AUDIO_FIFO_BUFFER_SIZE];
static uint8 firstTime = 1;

#ifdef DEBUG_TCRADIO_AUDIO_OUTPUT_DUMP
static unsigned int dumpAudioCnt=0;
static FILE *gOutAudfile;
#endif
#ifdef DEBUG_TCRADIO_AUDIO_OUTPUT_AVAIL_DUMP
static struct timespec audChkTimer, audChkTimeNow, audChkTimeDiff;
static unsigned int audAccumMs=0, audLoopMs=0;
static unsigned int dumpAvailCnt=0;
static FILE *gAvailfile;
#endif
#if defined(USE_HDRADIO) && defined(USE_ALSA_AIN_AS_INPUT_AUDIO)
static uint8 analogBuffer[TC_HDR_AUDIO_FRAME_SIZE*4];
#endif

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/

/****************************************************
*			function definition						*
*****************************************************/

/************************************
*			External API  			*
*************************************/
static void tcradiosound_setPlayback(uint32 fPalyback)
{
	if(fPalyback == 0) {
		stRadioSound.fSoundPlaying= 0;
		RSND_DBG("Stop radio sound\n");
	}
	else {
	//	if(pfnAinCmd != NULL) {
	//		(*pfnAinCmd)(eAUDIO_DRV_CMD_PREPARE, pNULL);
	//	}
	//	if(pfnAoutCmd != NULL) {
	//		(*pfnAoutCmd)(eAUDIO_DRV_CMD_PREPARE, pNULL);
	//	}
		stRadioSound.fSoundPlaying= 1;
		RSND_DBG("Play radio sound\n");
	}
}

static uint32 tcradiosound_getPlayback(void)
{
	return stRadioSound.fSoundPlaying;
}

static void tcradiosound_setSoundSource(eSOUND_SOURCE_t src)
{
	stRadioSound.soundSource = src;
	if(stRadioSound.soundSource == eSOUND_SOURCE_I2S) {
		RSND_DBG("Sound source is the external I2S using ALSA capture\n");
	}
	else if(stRadioSound.soundSource == eSOUND_SOURCE_TC_I2S) {
		RSND_DBG("Sound source is the external I2S using TC driver\n");
	}
	else if(stRadioSound.soundSource == eSOUND_SOURCE_SDR) {
		RSND_DBG("Sound source is internal demod\n");
	}
	else {
		RSND_DBG("No sound source\n");
	}
}

static eSOUND_SOURCE_t tcradiosound_getSoundSource(void)
{
	return stRadioSound.soundSource;
}

RET tcradiosound_init(void)
{
	RET ret = eRET_OK;

	if(soundMainThreadID == (pthread_t)NULL) {
		ret = tcradio_createThread(&soundMainThreadID, &tcradiosound_mainThread, "TCRADIO_SOUND", eRADIO_SCHED_OTHER, 0, pNULL);
		if(ret != eRET_OK) {
			RSND_ERR("[%s:%d] Can not make radio sound thread!!!\n", __func__, __LINE__);
			ret = eRET_NG_CREATE_THREAD;
			return ret;
		}
	}
	else {
		RSND_WRN("[%s:%d] Already sound thread was created!!!\n", __func__, __LINE__);
	}

	ret = tcradiosound_messageInit();
	if(ret != eRET_OK) {
		return ret;
	}

	ret = tcradiohal_audiofifo_init(AUDIO_FIFO_NUM);
	if(ret != eRET_OK) {
		return ret;
	}

	return ret;
}

RET tcradiosound_deinit(void)
{
	RET ret;
	tcradiosound_setPlayback(0);			// stop
	stRadioSound.fThreadRunning = 0;
	soundMainThreadID = (pthread_t)NULL;
	ret = tcradio_joinThread(soundMainThreadID, (void**)NULL);
	return ret;
}

RET tcradiosound_open(uint32 samplerate, eSOUND_SOURCE_t src)
{
	RET ret = eRET_OK;
	int32 i;
	uint32 faudio = tcradiosound_getPlayback();

	tcradiosound_mutexLock();
	if(stRadioSound.fSoundDriverOpen  == 0) {
		stSoundConfig.samplerate = samplerate;
		for(i=ANA_AUD_FIFO_INDEX; i<AUDIO_FIFO_NUM; i++) {
			tcradiohal_audiofifo_reset(i);
		}
		tcradiosound_setPlayback(0);		// stop
		tcradiosound_setSoundSource(src);
		tcradio_memset(soundBuffer, 0, sizeof(soundBuffer));
		tcradiohal_audiofifo_pushData(ANA_AUD_FIFO_INDEX, (uint8*)soundBuffer, 2048*4);
		if(pfnAoutOpen != NULL) {
			ret = (*pfnAoutOpen)(stSoundConfig);
		}
		else {
			ret = eRET_NG_NO_RSC;
		}

		if(ret == eRET_OK) {
			if(tcradiosound_getSoundSource() == eSOUND_SOURCE_I2S)
			{
				if(pfnAinOpen != NULL) {
					ret = (*pfnAinOpen)(stSoundConfig);
					if(ret == eRET_OK) {
						stRadioSound.fSoundDriverOpen = 1;
					}
					else {
						stRadioSound.soundSource = eSOUND_SOURCE_TC_I2S;
					}
				}
				else if((pfnBlendAudioI2sOpen != NULL) && (pfnBlendAudioI2sSetParams != NULL) && (pfnBlendAudioI2sStart != NULL)) {
					ret = eRET_NG_NO_RSC;		// It's return value for eSOUND_SOURCE_I2S
					stRadioSound.soundSource = eSOUND_SOURCE_TC_I2S;
				}
				else {
					ret = eRET_NG_NO_RSC;
				}

				if(ret != eRET_OK) {
					if(tcradiosound_getSoundSource() == eSOUND_SOURCE_TC_I2S) {
						ret = (*pfnBlendAudioI2sOpen)();
						if(ret == eRET_OK) {
							ret = (*pfnBlendAudioI2sSetParams)(2, 16, 44100, 16, 1024);
							if(ret < eRET_OK) {
								(*pfnBlendAudioI2sClose)();
								ret = eRET_NG_NO_RSC;
							}
							else {
								tcradiosound_setSoundSource(eSOUND_SOURCE_TC_I2S);
								(*pfnBlendAudioI2sStart)();
								stRadioSound.fSoundDriverOpen = 1;
							}
						}
						else {
							stRadioSound.soundSource = eSOUND_SOURCE_I2S;
							ret = eRET_NG_NO_RSC;
						}
					}
				}
			}
			else if(tcradiosound_getSoundSource() == eSOUND_SOURCE_TC_I2S)
			{
				if((pfnBlendAudioI2sOpen != NULL) && (pfnBlendAudioI2sSetParams != NULL) && (pfnBlendAudioI2sStart != NULL)) {
					ret = (*pfnBlendAudioI2sOpen)();
					if(ret == eRET_OK) {
						ret = (*pfnBlendAudioI2sSetParams)(2, 16, 44100, 16, 1024);
						if(ret < eRET_OK) {
							(*pfnBlendAudioI2sClose)();
							ret = eRET_NG_NO_RSC;
						}
						else {
							(*pfnBlendAudioI2sStart)();
							stRadioSound.fSoundDriverOpen = 1;
						}
					}
					else {
						stRadioSound.soundSource = eSOUND_SOURCE_I2S;
						ret = eRET_NG_NO_RSC;
					}
				}
				else if(pfnAinOpen != NULL) {
					ret = eRET_NG_NO_RSC;	// It's return value for eSOUND_SOURCE_TC_I2S
					stRadioSound.soundSource = eSOUND_SOURCE_I2S;
				}
				else {
					ret = eRET_NG_NO_RSC;
				}

				if(ret != eRET_OK) {
					if(tcradiosound_getSoundSource() == eSOUND_SOURCE_I2S) {
						ret = (*pfnAinOpen)(stSoundConfig);
						if(ret == eRET_OK) {
							tcradiosound_setSoundSource(eSOUND_SOURCE_I2S);
							stRadioSound.fSoundDriverOpen = 1;
						}
						else {
							stRadioSound.soundSource = eSOUND_SOURCE_TC_I2S;
						}
					}
				}
			}
			else
			{
				stRadioSound.fSoundDriverOpen = 1;
			}
		}

		if(faudio) {
			tcradiosound_setPlayback(1);		// start
		}
	}
	else {
		RSND_WRN("[%s:%d] Already open audio for radio\n", __func__, __LINE__);
		ret = eRET_NG_ALREADY_OPEN;
	}

	if(stRadioSound.fSoundDriverOpen == 1) {
		firstTime = 1;
	}
	tcradiosound_mutexUnlock();

	return ret;
}

RET tcradiosound_close(void)
{
	RET ret = eRET_OK;

	if(stRadioSound.fSoundDriverOpen != 0) {
		tcradiosound_mutexLock();
		stRadioSound.fSoundDriverOpen = 0;
		tcradiohal_audiofifo_reset(ANA_AUD_FIFO_INDEX);

		if(pfnAoutClose != NULL) {
			(void)(*pfnAoutClose)();
		}
		else {
			ret = eRET_NG_NO_RSC;
		}

		if(tcradiosound_getSoundSource() == eSOUND_SOURCE_I2S)
		{
			if(pfnAinClose != NULL) {
				ret = (*pfnAinClose)();
			}
			else {
				ret = eRET_NG_NO_RSC;
			}
		}
		else if(tcradiosound_getSoundSource() == eSOUND_SOURCE_TC_I2S)
		{
			if(pfnBlendAudioI2sClose != NULL) {
				ret = (*pfnBlendAudioI2sClose)();
			}
			else {
				ret = eRET_NG_NO_RSC;
			}
		}

		firstTime = 0;
		RSND_DBG("Close audio for radio\n");
		tcradiosound_mutexUnlock();
	}
	else {
		RSND_WRN("[%s:%d] Not open audio for radio!!!\n", __func__, __LINE__);
		ret = eRET_NG_NOT_OPEN;
	}

	return ret;
}

RET tcradiosound_setAudio(uint32 fPalyback)
{
	RET ret = eRET_OK;

	tcradiosound_mutexLock();
	if(stRadioSound.fSoundDriverOpen  != 0) {
		tcradiosound_setPlayback(fPalyback);
	}
	else {
		ret = eRET_NG_NOT_OPEN;
	}
	tcradiosound_mutexUnlock();

	return ret;
}

RET tcradiosound_reset(void)
{
	RET ret = eRET_OK;
	uint32 faudio = tcradiosound_getPlayback();

	tcradiosound_mutexLock();
	if(stRadioSound.fSoundDriverOpen  != 0) {
		tcradiosound_setPlayback(0);	// stop
		tcradio_memset(soundBuffer, 0, sizeof(soundBuffer));
		if(pfnAoutCmd != NULL) {
			ret = (*pfnAoutCmd)(eAUDIO_DRV_CMD_PREPARE, pNULL);
		}
		else {
			ret = eRET_NG_IO;
		}

		if(ret == eRET_OK) {
			if(tcradiosound_getSoundSource() == eSOUND_SOURCE_I2S)
			{
				if(pfnAinCmd != NULL) {
					ret = (*pfnAinCmd)(eAUDIO_DRV_CMD_PREPARE, pNULL);
				}
			}
		}

		if(faudio) {
			tcradiosound_setPlayback(1);	// start
		}
	}
	else {
		ret = eRET_NG_NOT_OPEN;
	}
	tcradiosound_mutexUnlock();

	return ret;
}

#ifdef USE_HDRADIO
void tcradiosound_hdrAudioOutPCMCallback(void *pOutBuf, int32 frames, uint32 samplerate)
{
	tcradiohal_audiofifo_pushData(ANA_AUD_FIFO_INDEX, (uint8*)pOutBuf, frames*4);
}
#endif

static RET tcradiosound_playbackHandler(void)
{
	RET ret = eRET_NG_NO_RSC;
	int32 frames = 0, read_bytes = 0;

	tcradiosound_mutexLock();
	if(stRadioSound.fSoundDriverOpen > 0) {
		/****************************/
		/*	Radio Sound Playback	*/
		/****************************/
		if(tcradiosound_getSoundSource() == eSOUND_SOURCE_I2S) {
			if(pfnAinRead != NULL) {
				if(firstTime) {
					firstTime = 0;
					frames = 2048;
					tcradio_memset(soundBuffer, 0, 2048*4);
				}
				else {
					frames = (*pfnAinRead)(soundBuffer, 1024);
					if(frames > 0) {
						if(tcradiosound_getPlayback() == 0) {
							tcradio_memset(soundBuffer, 0, 1024*4);
						}
					}
				}
			}
		}
		else if(tcradiosound_getSoundSource() == eSOUND_SOURCE_TC_I2S) {
			if(pfnBlendAudioI2sRead != NULL) {
				frames = (*pfnBlendAudioI2sRead)(soundBuffer, 2048);
				if(frames > 0) {
					if(tcradiosound_getPlayback() == 0) {
						tcradio_memset(soundBuffer, 0, frames);
					}
				}
				frames >>= 2;
			}
		}
		else if(tcradiosound_getSoundSource() == eSOUND_SOURCE_SDR) {
	#ifdef USE_HDRADIO
		#ifdef USE_ALSA_AIN_AS_INPUT_AUDIO
			if(pfnAinRead != NULL) {
				frames = (*pfnAinRead)(analogBuffer, TC_HDR_AUDIO_FRAME_SIZE);
				if(frames > 0) {
					if(tcradiosound_getPlayback() == 0) {
						tcradio_memset(analogBuffer, 0, TC_HDR_AUDIO_FRAME_SIZE*4);
					}
				}
			}
		#else
			frames = tcradiohal_audiofifo_popData(ANA_AUD_FIFO_INDEX, soundBuffer, TC_HDR_AUDIO_FRAME_SIZE*4);
			frames = frames/4;
			if(frames > 0 && tcradiosound_getPlayback() == 0) {
				tcradio_memset(soundBuffer, 0, TC_HDR_AUDIO_FRAME_SIZE*4);
				frames = TC_HDR_AUDIO_FRAME_SIZE;
			}
		#endif
	#else
			if(tcradiosound_getPlayback() == 0) {
				tcradio_memset(soundBuffer, 0, 2048*4);
				frames = 2048;
			}
	#endif
		}
		else {
			tcradio_memset(soundBuffer, 0, 2048*4);
			frames = 2048;
		}

		if(frames > 0) {
		#ifdef __ANDROID__
			// pcm data unit of android is bytes. but linux is frames.
			frames <<= 2;
		#endif
			ret = eRET_OK;
		#ifdef DEBUG_TCRADIO_AUDIO_OUTPUT_AVAIL_DUMP
			clock_gettime(CLOCK_MONOTONIC, &audChkTimeNow);
			audChkTimeDiff.tv_sec = audChkTimeNow.tv_sec - audChkTimer.tv_sec;
			audChkTimeDiff.tv_nsec = audChkTimeNow.tv_nsec - audChkTimer.tv_nsec;
			if(audChkTimeDiff.tv_nsec < 0) {
				audChkTimeDiff.tv_sec--;
				audChkTimeDiff.tv_nsec += 1e9;
			}
			audLoopMs = (audChkTimeDiff.tv_sec*1000) + (audChkTimeDiff.tv_nsec/1000000);

			if(gAvailfile!=NULL) {
				dumpAvailCnt++;
				fprintf(gAvailfile, "%d,%d,%d,%d,%d,", frames, dumpAvailCnt, audAccumMs+=audLoopMs, audLoopMs, (*pfnAoutCmd)(eAUDIO_DRV_CMD_AVAIL, NULL));
			}
			clock_gettime(CLOCK_MONOTONIC, &audChkTimer);
		#endif
			if(pfnAoutWrite != NULL) {
				(*pfnAoutWrite)(soundBuffer, frames);

                if (aout1st_flag == 1)
                {
					RSND_DBG("1st. Audio Output \n");
                    aout1st_flag = 0;
                }
			}
		#ifdef DEBUG_TCRADIO_AUDIO_OUTPUT_AVAIL_DUMP
			if(gAvailfile!=NULL) {
				fprintf(gAvailfile, "%d\n", (*pfnAoutCmd)(eAUDIO_DRV_CMD_AVAIL, NULL));
				fflush(gAvailfile);
			}
		#endif
		#ifdef DEBUG_TCRADIO_AUDIO_OUTPUT_DUMP
	    #ifdef USE_HDRADIO
			if(++dumpAudioCnt <= 15500) {	// When read frame size is 2048, it's about 11min.
				if(dumpAudioCnt == 999) {
					RSND_DBG(">>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<< Started Audio Output Dump >>>>>><<<<<<<\n");
				}
				else if(dumpAudioCnt >= 1000 && dumpAudioCnt < 15500) {
					fwrite(soundBuffer, 4, TC_HDR_AUDIO_FRAME_SIZE, gOutAudfile);
				}
				else if(dumpAudioCnt == 15500) {
					if(gOutAudfile!=NULL)	fclose(gOutAudfile);
					RSND_DBG(">>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<< Finished Audio Output Dump >>>>>><<<<<<<\n");
				}
			}
        #endif
		#endif
		}
	}
	tcradiosound_mutexUnlock();

	return ret;
}

/************************************
*	   Main Thread & Handler		*
*************************************/
void *tcradiosound_mainThread(void *arg)
{
	RET ret = (RET)eRET_OK;
	stMsgBuf_t stRecivedMessage = {0,};
	pid_t tid = (pid_t)syscall(SYS_gettid);
	int _pret_ = setpriority((int)PRIO_PROCESS, (unsigned int)tid, -8);

	prctl(PR_SET_NAME, "TCRADIO_SOUND",0,0,0);
#ifdef DEBUG_TCRADIO_AUDIO_OUTPUT_AVAIL_DUMP
	gAvailfile = fopen ("/tmp/alsa_audio_avail.csv", "w");
	clock_gettime(CLOCK_MONOTONIC, &audChkTimer);
#endif
#ifdef DEBUG_TCRADIO_AUDIO_OUTPUT_DUMP
	gOutAudfile = fopen ("/tmp/alsa_audio_output.bin", "w");
#endif

	stSoundConfig.samplerate = SOUND_SAMPLERATE;
	stSoundConfig.channels = SOUND_NUM_OF_CHANNEL;
	stSoundConfig.buffersize = SOUND_BUFFER_SIZE;
	stSoundConfig.periodsize = SOUND_PERIOD_SIZE;
	stSoundConfig.aout_startThd = SOUND_BUFFER_SIZE;
	stSoundConfig.ain_startThd = 0;

	stRadioSound.fSoundDriverOpen = 0;
	stRadioSound.fThreadRunning = 1;

	while(stRadioSound.fThreadRunning > 0) {
		ret = tcradiosound_playbackHandler();
		if(ret != eRET_OK) {
			tcradio_mssleep(SOUND_THREAD_TIME_INTERVAL);
		}
		else {
			// for context switching
			tcradio_mssleep(1);
		}
	}

#ifdef DEBUG_TCRADIO_AUDIO_OUTPUT_AVAIL_DUMP
	if(gAvailfile!=NULL)	fclose(gAvailfile);
#endif
#ifdef DEBUG_TCRADIO_AUDIO_OUTPUT_DUMP
	if(gOutAudfile!=NULL)	fclose(gOutAudfile);
#endif
	stRadioSound.fThreadRunning = 0;
	tcradiosound_messageDeinit();
	tcradiohal_audiofifo_deinit(AUDIO_FIFO_NUM);
	tcradio_exitThread(pNULL);

	return (pNULL);
}
