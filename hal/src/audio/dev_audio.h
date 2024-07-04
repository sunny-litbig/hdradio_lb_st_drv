/*******************************************************************************

*   FileName : dev_audio.h

*   Copyright (c) Telechips Inc.

*   Description : ALSA device HAL header

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
#ifndef DEV_AUDIO_H__
#define DEV_AUDIO_H__

#define TCC_AUDIO_DEVICE
/***************************************************
*				Include					*
****************************************************/
#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef TCC_AUDIO_DEVICE
 #ifdef __ANDROID__
  #include "tinyalsa/asoundlib.h"
  #include "system/audio.h"
 #else
  #include "alsa/asoundlib.h"
 #endif
#endif
/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define SND_DEBUG

#ifdef __ANDROID__

#define SND_TAG				("[RADIO][TALSA]")
#define SND_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,SND_TAG, __VA_ARGS__))
#ifdef SND_DEBUG
#define SND_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,SND_TAG, __VA_ARGS__))
#else
#define	SND_DBG(...)
#endif

#else // #ifdef __ANDROID__

#define SND_ERR(...)		((void)printf("[ERROR][RADIO][ALSA]: " __VA_ARGS__))
#ifdef SND_DEBUG
#define SND_DBG(...)		((void)printf("[DEBUG][RADIO][ALSA]: " __VA_ARGS__))
#else
#define	SND_DBG(...)
#endif

#endif // #ifdef __ANDROID__

/***************************************************
*				Enumeration				*
****************************************************/

/***************************************************
*				Typedefs					*
****************************************************/
#ifdef __ANDROID__
typedef struct {
 	uint32       			rate;				/* stream rate */
	uint32       			channels;			/* count of channels */
 	uint32       			buffer_time;		/* ring buffer length in us */
 	uint32       			period_time;		/* period time in us */
 	uint32       			nperiods;			/* number of periods */
} stAUDIO_ALSA_CTL_t;
#else	// linux or others OS
typedef struct {
	snd_pcm_t				*h_alsa;
	snd_pcm_hw_params_t		*p_alsa_hw_params;
	snd_pcm_sw_params_t		*p_alsa_sw_params;
	snd_pcm_status_t		*p_alsa_status;
	snd_pcm_format_t   		format; 			/* sample format */
 	uint32       			rate;				/* stream rate */
	uint32       			channels;			/* count of channels */
 	uint32       			buffer_time;		/* ring buffer length in us */
 	uint32       			period_time;		/* period time in us */
 	uint32       			nperiods;			/* number of periods */
 	snd_pcm_uframes_t  		buffer_size;
 	snd_pcm_uframes_t  		period_size;
	snd_pcm_uframes_t		start_threshold;
} stAUDIO_ALSA_CTL_t;
#endif

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern RET dev_aout_open(stAUDIO_CONFIG_t params);
extern RET dev_aout_write(uint8 *pbuf, int32 frame_size);
extern RET dev_aout_close(void);
extern RET dev_aout_command(eAUDIO_DRV_CMD_t cmd, void *args);

extern RET dev_ain_open(stAUDIO_CONFIG_t params);
extern RET dev_ain_read(uint8 *pbuf, int32 frame_size);
extern RET dev_ain_close(void);
extern RET dev_ain_command(eAUDIO_DRV_CMD_t cmd, void *args);

#ifdef __cplusplus
}
#endif

#endif

