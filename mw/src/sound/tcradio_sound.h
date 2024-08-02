/*******************************************************************************

*   FileName : tcradio_sound.h

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
#ifndef __TCRADIO_SOUND_H__
#define __TCRADIO_SOUND_H__

/***************************************************
*				Include					*
****************************************************/
#ifdef __ANDROID__
#include <android/log.h>
#endif

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "DMBLog.h"

#define RADIO_SND_DEBUG

#ifdef __ANDROID__

#define RSND_TAG			("[RADIO][SOUND]")
#define RSND_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,RSND_TAG, __VA_ARGS__))
#define RSND_INF(...)		(__android_log_print(ANDROID_LOG_INFO,RSND_TAG, __VA_ARGS__))
#define RSND_WRN(...)		(__android_log_print(ANDROID_LOG_WARN,RSND_TAG, __VA_ARGS__))
#ifdef RADIO_SND_DEBUG
#define RSND_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,RSND_TAG, __VA_ARGS__))
#else
#define	RSND_DBG(...)
#endif

#else // #ifdef __ANDROID__

// #define RSND_ERR(...)		((void)printf("[ERROR][RADIO][SOUND]: " __VA_ARGS__))
#define RSND_ERR(...)		((void)LB_PRINTF("[ERROR][RADIO][SOUND]: " __VA_ARGS__))
// #define RSND_INF(...)		((void)printf("[INFO][RADIO][SOUND]: " __VA_ARGS__))
#define RSND_INF(...)		((void)LB_PRINTF("[INFO][RADIO][SOUND]: " __VA_ARGS__))
// #define RSND_WRN(...)		((void)printf("[WARN][RADIO][SOUND]: " __VA_ARGS__))
#define RSND_WRN(...)		((void)LB_PRINTF("[WARN][RADIO][SOUND]: " __VA_ARGS__))
#ifdef RADIO_SND_DEBUG
// #define RSND_DBG(...)		((void)printf("[DEBUG][RADIO][SOUND]: " __VA_ARGS__))
#define RSND_DBG(...)		((void)LB_PRINTF("[DEBUG][RADIO][SOUND]: " __VA_ARGS__))
#else
#define	RSND_DBG(...)
#endif

#endif // #ifdef __ANDROID__

#define	SOUND_THREAD_TIME_INTERVAL	(10)

#define	SOUND_NUM_OF_CHANNEL		(2)
#define	SOUND_SAMPLERATE			(48000)
#ifdef USE_HDRADIO
#define	SOUND_BUFFER_SIZE			(8192)	// 8192	// 16384	// 2048
#define	SOUND_PERIOD_SIZE			(2048)	// 2048	// 4096		// 512
#else
#define	SOUND_BUFFER_SIZE			(16384)	// 8192	// 16384	// 2048
#define	SOUND_PERIOD_SIZE			(4096)	// 2048	// 4096		// 512
#endif

/* The Max. number of the audio fifo can be modified in 'tcradio_hal_fifo.h'. */
#define	ANA_AUD_FIFO_INDEX		0
#define	DIG_AUD_FIFO_INDEX		1

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eSOUND_STS_OK					= 0,	/* Job Complete & Notify */
	eSOUND_STS_ERROR				= 1,	/* Critical Error */
	eSOUND_STS_DOING				= 4,	/* Job Continue */
	eSOUND_STS_DOING_NOTIFY			= 5,	/* Job Continue & Notify Case*/
	eSOUND_STS_DOING_ERROR_NOTIFY	= 6,	/* Job COntinue & Error Notify */
	eSOUND_STS_OK_NOTIFY			= 7,	/* Job Complete & Notify Case */
	eSOUND_STS_WAIT					= 8,
	eSOUND_STS_DONE					= 9,
	eSOUND_STS_END
} eSOUND_STS_t;

typedef enum
{
	eSOUND_CMD_NULL			= 0,

	eSOUND_CMD_OPEN			= 1,
	eSOUND_CMD_CLOSE		= 2,
	eSOUND_CMD_PLAYBACK		= 5,
	eSOUND_CMD_PAUSE		= 6,
	eSOUND_CMD_RESET		= 7,

	eSOUND_CMD_END
}eSOUND_CMD_t;

typedef enum
{
    eSOUND_EVT_NULL			= 0,

	eSOUND_EVT_OPEN			= 1,
	eSOUND_EVT_CLOSE		= 2,
	eSOUND_EVT_PLAYBACK		= 5,
	eSOUND_EVT_PAUSE		= 6,
	eSOUND_EVT_RESET		= 7,

    eSOUND_EVT_END
}eSOUND_EVENT_t;

typedef enum
{
	eSOUND_NOTIFY_NULL		= 0,

	eSOUND_NOTIFY_OPEN		= 1,
	eSOUND_NOTIFY_CLOSE		= 2,
	eSOUND_NOTIFY_PLAYBACK	= 5,
	eSOUND_NOTIFY_PAUSE		= 6,
	eSOUND_NOTIFY_RESET		= 7,

	eSOUND_NOTIFY_END
}eSOUND_Notify_t;

typedef enum
{
	eSOUND_SOURCE_NULL		= 0,	// None
	eSOUND_SOURCE_I2S		= 1,	// External I2S using ALSA capture (Only AM/FM)
	eSOUND_SOURCE_TC_I2S	= 2,	// External I2S using TC driver (Only AM/FM)
	eSOUND_SOURCE_SDR		= 3		// Internal SDR(HDR/DRM), Analog AM/FM audio is external I2S controlled by TC driver
}eSOUND_SOURCE_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct{
	int32 fThreadRunning;
	uint32 fSoundInit;
	uint32 fSoundDriverOpen;
	uint32 fSoundPlaying;
	eSOUND_EVENT_t eMainMode;

	eSOUND_SOURCE_t soundSource;
}stRADIO_SOUND_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern RET tcradiosound_init(void);
extern RET tcradiosound_deinit(void);
extern RET tcradiosound_open(uint32 samplerate, eSOUND_SOURCE_t src);
extern RET tcradiosound_close(void);
extern RET tcradiosound_setAudio(uint32 fPalyback);
extern RET tcradiosound_reset(void);
extern void *tcradiosound_mainThread(void *arg);

#ifdef __cplusplus
}
#endif

#endif

