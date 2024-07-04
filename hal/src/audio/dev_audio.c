/*******************************************************************************

*   FileName : dev_audio.c

*   Copyright (c) Telechips Inc.

*   Description : ALSA device HAL functions and definitions

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
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "tcradio_types.h"
#include "tcradio_peri_config.h"
#include "tcradio_hal.h"
#include "tcradio_hal_config.h"
#include "dev_audio.h"

#ifndef __ANDROID__
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

/***************************************************
*           Local constant definitions              *
****************************************************/
static stAUDIO_ALSA_CTL_t stSoundRxCtl;
static stAUDIO_ALSA_CTL_t stSoundTxCtl;

#ifdef USE_PULSEAUDIO
static const int8 *device_tx = "pulsemedia";
#else
static const int8 *device_tx = "plug:tcc";
//static int8 *device_tx = "hw:0,0";	// for test
#endif

#if defined(TCC802X_BOARD)
static const int8 *device_rx = "hw:0,3";
#elif defined(TCC802X_EVM21_BOARD)
static const int8 *device_rx = "hw:0,3";
#else
static const int8 *device_rx = "hw:0,4";
#endif

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/
static int32 set_hwparams(void *arg, snd_pcm_access_t access);
static int32 set_swparams(void *arg, int32 direction);

/***************************************************
*			function definition				*
****************************************************/
RET dev_aout_open(stAUDIO_CONFIG_t params)
{
	RET ret = (RET)eRET_OK;
#ifdef TCC_AUDIO_DEVICE
	int32 iret = 0;
	stAUDIO_ALSA_CTL_t *ctl = &stSoundTxCtl;

	SND_DBG("[     ALSA     ] ALSA open function run : devicename[%s]\n", device_tx);

	if (ctl->h_alsa != NULL)
	{
		snd_pcm_drop(ctl->h_alsa);
		snd_pcm_close(ctl->h_alsa);
		ctl->h_alsa = NULL;
	}

	ctl->format			= SND_PCM_FORMAT_S16_LE;	/* sample format */
	ctl->rate			= params.samplerate;		/* stream rate */
	ctl->channels		= params.channels;			/* count of channels */
	ctl->buffer_time	= 0;						/* ring buffer length in us */
	ctl->period_time	= 0;						/* period time in us */
	ctl->nperiods		= 4;						/* number of periods */
	ctl->buffer_size	= params.buffersize;
	ctl->period_size	= params.periodsize;
	ctl->start_threshold = params.aout_startThd;	/* threshold frames when to start audio output. */

	snd_pcm_hw_params_alloca(&ctl->p_alsa_hw_params);
	snd_pcm_sw_params_alloca(&ctl->p_alsa_sw_params);

	iret = snd_pcm_open(&ctl->h_alsa, device_tx, SND_PCM_STREAM_PLAYBACK, 0);
	if(iret < 0) {
		SND_ERR("[%s:%d] Pcm Open Failed! : %s\n", __func__, __LINE__, snd_strerror(iret));
		ret = (RET)eRET_NG_NOT_SUPPORT;
		goto error_alsa;
	}

	iret = set_hwparams((void *)ctl, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(iret < 0) {
		SND_ERR("[%s:%d] Set H/W Parameter Failed! : %s\n", __func__, __LINE__, snd_strerror(iret));
		ret = (RET)eRET_NG_NOT_SUPPORT;
		goto error_alsa;
	}

	iret = set_swparams((void *)ctl, (int32)SND_PCM_STREAM_PLAYBACK);
	if(iret < 0) {
		SND_ERR("[%s:%d] Set S/W Parameter Failed! : %s\n", __func__, __LINE__, snd_strerror(iret));
		ret = (RET)eRET_NG_NOT_SUPPORT;
		goto error_alsa;
	}

error_alsa:

#else	/* #ifdef TCC_AUDIO_DEVICE */


	////////////////
	/////
	/////  TODO
	/////
	////////////////


#endif	/* #ifdef TCC_AUDIO_DEVICE */

	return ret;
}


RET dev_aout_write(uint8 *pbuf, int32 frame_size)
{
	RET ret = (RET)eRET_OK;
#ifdef TCC_AUDIO_DEVICE
	stAUDIO_ALSA_CTL_t *ctl = &stSoundTxCtl;
	int32 frames;
	uint8 *write_buf;
	snd_pcm_state_t state;
	snd_pcm_sframes_t avail;
	uint32 uiTimeWriteDone, uiTimeWriteStart;
	int32 retry_count=0;

	write_buf = pbuf;

	if((write_buf == NULL) || (frame_size <= 0)){
		SND_ERR("[%s:%d]:  pcm[0x%08x][pcmlen = %d]  !!\n", __func__, __LINE__, write_buf, frame_size);
		return eRET_NG_INVALID_PARAM;
	}
#if 0	// for codesonar : ctl always don't evaluates to false.
	if(ctl == NULL){
		SND_ERR("%s: null ctl  !!\n", __func__);
		return (RET)eRET_NG_INVALID_PARAM;
	}
#endif
	while((retry_count++<5) && (frame_size > 0))
	{
		frames = (int32)snd_pcm_writei(ctl->h_alsa, write_buf, (snd_pcm_uframes_t)frame_size);

		if(frames < 0)
		{
			state = snd_pcm_state(ctl->h_alsa);

			if(frames == -EPIPE)	/* EPIPE(32) : Broken pipe */
			{
				SND_ERR("[%s:%d] xrun. prepare[%d][%d], frame size[%d]\n", __func__, __LINE__, frames, state, frame_size);
				snd_pcm_prepare(ctl->h_alsa);
			}
			else
			{
				stAUDIO_CONFIG_t params;
				params.samplerate = ctl->rate;
				params.channels = ctl->channels;
				SND_ERR("[%s:%d] error ALSA write, run ALSA init. frame size[%d], state[%d]\n", __func__, __LINE__, frames, state);
				dev_aout_close();
				dev_aout_open(params);
			}
			frames = 0;

			if( retry_count==1)
			{
				/* fill zero */
				memset(write_buf, 0, frame_size*2*ctl->channels);
			}
		}

		frame_size -= frames;
		write_buf += frames*4;
	}

	if(frame_size > 0)
	{
		SND_ERR("[%s:%d]: ALSA write frame_size wrong[%d], retry_count[%d]\n",__func__, __LINE__, frame_size, retry_count);
		ret = (RET)eRET_NG_ALSA_UNDRRUN;
	}

#else	/* #ifdef TCC_AUDIO_DEVICE */


	////////////////
	/////
	/////  TODO
	/////
	////////////////


#endif	/* #ifdef TCC_AUDIO_DEVICE */

	return ret;
}

RET dev_aout_close(void)
{
	RET ret = (RET)eRET_OK;
#ifdef TCC_AUDIO_DEVICE
	stAUDIO_ALSA_CTL_t *ctl = &stSoundTxCtl;

	if (ctl->h_alsa != NULL)
	{
		snd_pcm_drop(ctl->h_alsa);
		snd_pcm_close(ctl->h_alsa);
		ctl->h_alsa = NULL;
	}

#else	/* #ifdef TCC_AUDIO_DEVICE */


	////////////////
	/////
	/////  TODO
	/////
	////////////////


#endif	/* #ifdef TCC_AUDIO_DEVICE */

	return ret;
}

RET dev_aout_command(eAUDIO_DRV_CMD_t cmd, void *args)
{
	RET ret = (RET)eRET_OK;
#ifdef TCC_AUDIO_DEVICE
	stAUDIO_ALSA_CTL_t *ctl = &stSoundTxCtl;

	if (ctl->h_alsa != NULL) {
		switch(cmd) {
			case eAUDIO_DRV_CMD_PREPARE:	snd_pcm_prepare(ctl->h_alsa);	break;
			case eAUDIO_DRV_CMD_DROP:		snd_pcm_drop(ctl->h_alsa);		break;
			case eAUDIO_DRV_CMD_DRAIN:		snd_pcm_drain(ctl->h_alsa);		break;
			case eAUDIO_DRV_CMD_RESET:		snd_pcm_reset(ctl->h_alsa);		break;
			case eAUDIO_DRV_CMD_START:		snd_pcm_start(ctl->h_alsa);		break;
			case eAUDIO_DRV_CMD_AVAIL:			ret = (RET)snd_pcm_avail(ctl->h_alsa);			break;
			case eAUDIO_DRV_CMD_AVAIL_UPDATE:	ret = (RET)snd_pcm_avail_update(ctl->h_alsa);	break;
			default:							ret = eRET_NG_NO_RSC;							break;
		}
	}
	else {
		ret = eRET_NG_NO_RSC;
	}

#else	/* #ifdef TCC_AUDIO_DEVICE */


	////////////////
	/////
	/////  TODO
	/////
	////////////////


#endif	/* #ifdef TCC_AUDIO_DEVICE */

	return ret;
}


RET dev_ain_open(stAUDIO_CONFIG_t params)
{
	RET ret = (RET)eRET_OK;
#ifdef TCC_AUDIO_DEVICE
	int32 iret = 0;
	stAUDIO_ALSA_CTL_t *ctl = &stSoundRxCtl;

	SND_DBG("[     ALSA     ] ALSA open function run : devicename[%s]\n", device_rx);

	if (ctl->h_alsa != NULL)
	{
		snd_pcm_drop(ctl->h_alsa);
		snd_pcm_close(ctl->h_alsa);
		ctl->h_alsa = NULL;
	}

	ctl->format			= SND_PCM_FORMAT_S16_LE;	/* sample format */
	ctl->rate			= params.samplerate;		/* stream rate */
	ctl->channels		= params.channels;			/* count of channels */
	ctl->buffer_time	= 0;						/* ring buffer length in us */
	ctl->period_time	= 0;						/* period time in us */
	ctl->nperiods		= 4;						/* number of periods */
	ctl->buffer_size	= params.buffersize;
	ctl->period_size	= params.periodsize;
	ctl->start_threshold = params.ain_startThd;		/* threshold frames when to start audio input. */

	snd_pcm_hw_params_alloca(&ctl->p_alsa_hw_params);
	snd_pcm_sw_params_alloca(&ctl->p_alsa_sw_params);

	iret = snd_pcm_open(&ctl->h_alsa, device_rx, SND_PCM_STREAM_CAPTURE, 0);
	if(iret < 0) {
		SND_ERR("[%s:%d] Capture Open Failed! : %s\n", __func__, __LINE__, snd_strerror(iret));
		ret = eRET_NG_NOT_SUPPORT;
		goto error_alsa;
	}

	iret = set_hwparams((void *)ctl, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(iret < 0) {
		SND_ERR("[%s:%d] Set H/W Parameter Failed! : %s\n", __func__, __LINE__, snd_strerror(iret));
		ret = (RET)eRET_NG_NOT_SUPPORT;
		goto error_alsa;
	}

	iret = set_swparams((void *)ctl, SND_PCM_STREAM_CAPTURE);
	if(iret < 0) {
		SND_ERR("[%s:%d] Set S/W Parameter Failed! : %s\n", __func__, __LINE__, snd_strerror(iret));
		ret = (RET)eRET_NG_NOT_SUPPORT;
		goto error_alsa;
	}

error_alsa:

#else	/* #ifdef TCC_AUDIO_DEVICE */


	////////////////
	/////
	/////  TODO
	/////
	////////////////


#endif	/* #ifdef TCC_AUDIO_DEVICE */

	return ret;
}


RET dev_ain_read(uint8 *pbuf, int32 frame_size)
{
	RET ret = (RET)eRET_OK;
#ifdef TCC_AUDIO_DEVICE
	stAUDIO_ALSA_CTL_t *ctl = &stSoundRxCtl;
	uint8 *read_buf;
	int32 frames=0, read_frames=0;

	read_buf = pbuf;

	if(read_buf == NULL|| frame_size <= 0) {
		SND_ERR("[%s:%d]:  pcm[0x%08x][pcmlen = %d]  !!\n", __func__, __LINE__, read_buf, frame_size);
		return eRET_NG_INVALID_PARAM;
	}
#if 0	// for codesonar : ctl always don't evaluates to false.
	if(ctl == NULL) {
		SND_ERR("%s: null ctl  !!\n",__func__);
		return eRET_NG_INVALID_PARAM;
	}
#endif
	while( frame_size > 0 )
	{
		frames = snd_pcm_wait(ctl->h_alsa, 5);
		if (frames < 0) {
			SND_ERR("[%s:%d]: snd_pcm_wait failed: %s\n", __func__, __LINE__, snd_strerror(frames));
			if(frames == -EPIPE)	/* EPIPE(32) : Broken pipe */
			{
				SND_ERR("[%s:%d] Alsa broken pipe!!, errno[%d], frame size[%d]\n", __func__, __LINE__, frames, frame_size);
				frames = snd_pcm_recover(ctl->h_alsa, frames, 0);
			}
			else {
				frames = 0;
			}
		}
		else {
			frames = (int32)snd_pcm_readi(ctl->h_alsa, read_buf, frame_size);

			if (frames == -EAGAIN) {	/* EAGAIN(11) : Try again */
				frames = 0;
			}
			else {
				if (frames < 0) {
					frames = snd_pcm_recover(ctl->h_alsa, frames, 0);
				}

				if (frames < 0) {
					SND_ERR("[%s:%d]: snd_pcm_readi failed: %s\n", __func__, __LINE__, snd_strerror(frames));
					frames = 0;
				}
			}
		}

		frame_size -= frames;
		read_frames += frames;
	//	read_buf += frames*4;	// The read_buf is the size to read once. So it does not increase an address.
	}

	if(frame_size > 0)
	{
		SND_ERR("[%s:%d]: ALSA read frame_size wrong[%d]\n", __func__, __LINE__, frame_size);
		ret = (RET)eRET_NG_ALSA_UNDRRUN;
	}

	if(ret == (RET)eRET_OK) {
		ret = (RET)read_frames;
	}

#else	/* #ifdef TCC_AUDIO_DEVICE */


	////////////////
	/////
	/////  TODO
	/////
	////////////////


#endif	/* #ifdef TCC_AUDIO_DEVICE */

	return ret;
}

RET dev_ain_close(void)
{
	RET ret = (RET)eRET_OK;
#ifdef TCC_AUDIO_DEVICE
	stAUDIO_ALSA_CTL_t *ctl = &stSoundRxCtl;

	if (ctl->h_alsa != NULL)
	{
		snd_pcm_drop(ctl->h_alsa);
		snd_pcm_close(ctl->h_alsa);
		ctl->h_alsa = NULL;
	}

#else	/* #ifdef TCC_AUDIO_DEVICE */


	////////////////
	/////
	/////  TODO
	/////
	////////////////


#endif	/* #ifdef TCC_AUDIO_DEVICE */

	return ret;
}

RET dev_ain_command(eAUDIO_DRV_CMD_t cmd, void *args)
{
	RET ret = (RET)eRET_OK;
#ifdef TCC_AUDIO_DEVICE
	stAUDIO_ALSA_CTL_t *ctl = &stSoundRxCtl;

	if (ctl->h_alsa != NULL) {
		switch(cmd) {
			case eAUDIO_DRV_CMD_PREPARE:	snd_pcm_prepare(ctl->h_alsa);	break;
			case eAUDIO_DRV_CMD_DROP:		snd_pcm_drop(ctl->h_alsa);		break;
			case eAUDIO_DRV_CMD_DRAIN:		snd_pcm_drain(ctl->h_alsa);		break;
			case eAUDIO_DRV_CMD_RESET:		snd_pcm_reset(ctl->h_alsa);		break;
			case eAUDIO_DRV_CMD_START:		snd_pcm_start(ctl->h_alsa);		break;
			case eAUDIO_DRV_CMD_AVAIL:			ret = (RET)snd_pcm_avail(ctl->h_alsa);			break;
			case eAUDIO_DRV_CMD_AVAIL_UPDATE:	ret = (RET)snd_pcm_avail_update(ctl->h_alsa);	break;
			default:							ret = eRET_NG_NO_RSC;							break;
		}
	}
	else {
		ret = eRET_NG_NO_RSC;
	}

#else	/* #ifdef TCC_AUDIO_DEVICE */


	////////////////
	/////
	/////  TODO
	/////
	////////////////


#endif	/* #ifdef TCC_AUDIO_DEVICE */

	return ret;
}

static int32 set_hwparams(void *arg, snd_pcm_access_t access)
{
	stAUDIO_ALSA_CTL_t *ctl = (stAUDIO_ALSA_CTL_t *)arg;

	uint32 rrate;
	int32  err;
	snd_pcm_uframes_t     period_size_min;
	snd_pcm_uframes_t     period_size_max;
	snd_pcm_uframes_t     buffer_size_min;
	snd_pcm_uframes_t     buffer_size_max;

	/* choose all parameters */
	err = snd_pcm_hw_params_any(ctl->h_alsa, ctl->p_alsa_hw_params);
	if (err < 0) {
		SND_ERR("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
		return err;
	}

	/* set the interleaved read/write format */
	err = snd_pcm_hw_params_set_access(ctl->h_alsa, ctl->p_alsa_hw_params, access);
	if (err < 0) {
		SND_ERR("Access type not available for playback: %s\n", snd_strerror(err));
		return err;
	}

	/* set the sample format */
	err = snd_pcm_hw_params_set_format(ctl->h_alsa, ctl->p_alsa_hw_params, ctl->format);
	if (err < 0) {
		SND_ERR("Sample format not available for playback: %s\n", snd_strerror(err));
		return err;
	}

	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(ctl->h_alsa, ctl->p_alsa_hw_params, ctl->channels);
	if (err < 0) {
		SND_ERR("Channels count (%i) not available for playbacks: %s\n", ctl->channels, snd_strerror(err));
		return err;
	}

	/* set the stream rate */
	rrate = ctl->rate;
	err = snd_pcm_hw_params_set_rate(ctl->h_alsa, ctl->p_alsa_hw_params, ctl->rate, 0);
	if (err < 0) {
		SND_ERR("Rate %iHz not available for playback: %s\n", ctl->rate, snd_strerror(err));
		return err;
	}

	if (rrate != ctl->rate) {
		SND_ERR("Rate doesn't match (requested %iHz, get %iHz, err %d)\n", ctl->rate, rrate, err);
		return -EINVAL;
	}

	SND_DBG("Rate set to %iHz (requested %iHz)\n", rrate, ctl->rate);
	/* set the buffer time */
	snd_pcm_hw_params_get_buffer_size_min(ctl->p_alsa_hw_params, &buffer_size_min);
	snd_pcm_hw_params_get_buffer_size_max(ctl->p_alsa_hw_params, &buffer_size_max);
	snd_pcm_hw_params_get_period_size_min(ctl->p_alsa_hw_params, &period_size_min, NULL);
	snd_pcm_hw_params_get_period_size_max(ctl->p_alsa_hw_params, &period_size_max, NULL);
	SND_DBG("Buffer size range from %lu to %lu\n",buffer_size_min, buffer_size_max);
	SND_DBG("Period size range from %lu to %lu\n",period_size_min, period_size_max);
	if (ctl->period_time > 0) {
		SND_DBG("Requested period time %u us\n", ctl->period_time);
		err = snd_pcm_hw_params_set_period_time_near(ctl->h_alsa, ctl->p_alsa_hw_params, &ctl->period_time, NULL);
		if (err < 0) {
			SND_ERR("Unable to set period time %u us for playback: %s\n", ctl->period_time, snd_strerror(err));
			return err;
		}
	}
	if (ctl->buffer_time > 0) {
		SND_DBG("Requested buffer time %u us\n", ctl->buffer_time);
		err = snd_pcm_hw_params_set_buffer_time_near(ctl->h_alsa, ctl->p_alsa_hw_params, &ctl->buffer_time, NULL);
		if (err < 0) {
			SND_ERR("Unable to set buffer time %u us for playback: %s\n", ctl->buffer_time, snd_strerror(err));
			return err;
		}
	}
	if ((ctl->buffer_time == 0) && (ctl->period_time == 0)) {
		if(ctl->buffer_size == 0) {
			ctl->buffer_size = buffer_size_max;
		}
		if (ctl->period_time == 0) {
			ctl->buffer_size = (ctl->buffer_size / ctl->nperiods) * ctl->nperiods;
		}
		SND_DBG("Using buffer size %lu\n", ctl->buffer_size);
		err = snd_pcm_hw_params_set_buffer_size_near(ctl->h_alsa, ctl->p_alsa_hw_params, &ctl->buffer_size);
		if (err < 0) {
			SND_ERR("Unable to set buffer size %lu for playback: %s\n", ctl->buffer_size, snd_strerror(err));
			return err;
		}
	}
	if ((ctl->buffer_time == 0) || (ctl->period_time == 0)) {
		SND_DBG("Periods = %u\n", ctl->nperiods);
		err = snd_pcm_hw_params_set_periods_near(ctl->h_alsa, ctl->p_alsa_hw_params, &ctl->nperiods, NULL);
		if (err < 0) {
			SND_ERR("Unable to set nperiods %u for playback: %s\n", ctl->nperiods, snd_strerror(err));
			return err;
		}
	}
	snd_pcm_hw_params_get_buffer_size(ctl->p_alsa_hw_params, &ctl->buffer_size);
	snd_pcm_hw_params_get_period_size(ctl->p_alsa_hw_params, &ctl->period_size, NULL);
	SND_DBG("was set period_size = %lu\n",ctl->period_size);
	SND_DBG("was set buffer_size = %lu\n",ctl->buffer_size);
	if (2*ctl->period_size > ctl->buffer_size) {
		SND_ERR("buffer too small, could not use\n");
		return err;
	}

	/* write the parameters to device */
	err = snd_pcm_hw_params(ctl->h_alsa, ctl->p_alsa_hw_params);
	if (err < 0) {
		SND_ERR("Unable to set hw params for playback: %s\n", snd_strerror(err));
		return err;
	}

	return 0;
}

static int32 set_swparams(void *arg, int32 direction)
{
	stAUDIO_ALSA_CTL_t *ctl = (stAUDIO_ALSA_CTL_t *)arg;

	int32 err;

	/* get the current swparams */
	err = snd_pcm_sw_params_current(ctl->h_alsa, ctl->p_alsa_sw_params);
	if (err < 0) {
		SND_ERR("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
		return err;
	}

	if(direction == SND_PCM_STREAM_CAPTURE)
	{
		/* start the transfer when a buffer is full */
		err = snd_pcm_sw_params_set_start_threshold(ctl->h_alsa, ctl->p_alsa_sw_params, ctl->start_threshold);
		if (err < 0) {
			SND_ERR("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
			return err;
		}
	}
	else
	{
		/* start the transfer when a buffer is full */
		err = snd_pcm_sw_params_set_start_threshold(ctl->h_alsa, ctl->p_alsa_sw_params, ctl->start_threshold);
		if (err < 0) {
			SND_ERR("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
			return err;
		}
	}

	/* allow the transfer when at least period_size frames can be processed */
	err = snd_pcm_sw_params_set_avail_min(ctl->h_alsa, ctl->p_alsa_sw_params, ctl->period_size);
	if (err < 0) {
		SND_ERR("Unable to set avail min for playback: %s\n", snd_strerror(err));
		return err;
	}

	/* write the parameters to the playback device */
	err = snd_pcm_sw_params(ctl->h_alsa, ctl->p_alsa_sw_params);
	if (err < 0) {
		SND_ERR("Unable to set sw params for playback: %s\n", snd_strerror(err));
		return err;
	}

	return 0;
}

#endif	/* #ifndef __ANDROID__ */