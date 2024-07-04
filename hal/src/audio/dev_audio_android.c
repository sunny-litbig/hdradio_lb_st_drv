/*******************************************************************************

*   FileName : dev_audio_android.c

*   Copyright (c) Telechips Inc.

*   Description : Tiny-ALSA device HAL functions and definitions

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
*		Include 				        		   *
****************************************************/
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tcradio_types.h"
#include "tcradio_peri_config.h"
#include "tcradio_hal.h"
#include "tcradio_hal_config.h"
#include "dev_audio.h"

#ifdef __ANDROID__
/***************************************************
*        Global variable definitions               *
****************************************************/

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions             *
****************************************************/

/***************************************************
*             Local preprocessor                   *
****************************************************/
#define PCM_CARD				(0U)
#define DEVICE_PCM0				(0U)
#define DEVICE_SPDIF			(2U)
#define DEVICE_PCM1				(1U)

#define PCM_OUT_DEVICE			DEVICE_PCM0
#define PCM_IN_DEVICE			DEVICE_PCM0

#define PCM_OPEN_RETRY_COUNT	(500U)

//#define USE_DUMP

/***************************************************
*           Local constant definitions             *
****************************************************/
static struct pcm_config pcm_config = {
    .channels = 2,
    .rate = 44100,
    .period_size = 2048,
    .period_count = 4,
    .format = PCM_FORMAT_S16_LE,
    .start_threshold = 0,
    .stop_threshold = 0,
    .silence_threshold = 0,
    .avail_min = 0,
};

struct pcm *pcm_in;
struct pcm *pcm_out;

#ifdef USE_DUMP
FILE *wfp;
#endif

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition				       *
****************************************************/
#ifdef TCC_AUDIO_DEVICE
static struct pcm *open_pcm_device(const int32 card, const int32 device, struct pcm_config *config, uint32 direction)
{
    struct pcm *pcm;
    int32 max_try_time = PCM_OPEN_RETRY_COUNT;

    do
    {
        pcm = pcm_open(card, device, direction, config);

        if (pcm == NULL)
        {
            RHAL_DBG("%s : %d pcm (%d:%d) is null, try to reopen.. try_count = %d, status = %s", __func__,__LINE__, card, device, max_try_time, pcm_get_error(pcm));
            usleep(100*1000);
            continue;
        }else{
			RHAL_DBG("%s : %d pcm (%d:%d) is OK, try to reopen.. try_count = %d, status = %s", __func__,__LINE__, card, device, max_try_time, pcm_get_error(pcm));
        }

        if (!pcm_is_ready(pcm))
        {
            RHAL_DBG("%s : %d pcm (%d:%d) is not ready, try to reopen.. try_count = %d, status = %s", __func__,__LINE__, card, device, max_try_time, pcm_get_error(pcm));
            pcm_close(pcm);
            pcm = NULL;
            usleep(100*1000);
            continue;
        }else{
			RHAL_DBG("%s : %d pcm (%d:%d) is ready, try to reopen.. try_count = %d, status = %s", __func__,__LINE__, card, device, max_try_time, pcm_get_error(pcm));
        }
        break;
    } while (max_try_time--);

    if (pcm == NULL || !pcm_is_ready(pcm))
    {
        RHAL_DBG("%s : finally cannot get pcm.. return NULL", __func__);
        return NULL;
    }

    return pcm;
}
/////////////////////////////////////////////////////////////////////////////////////////

#endif //TCC_AUDIO_DEVICE

RET dev_aout_open(stAUDIO_CONFIG_t params)
{
	RET ret = eRET_OK;

#ifdef TCC_AUDIO_DEVICE
	RHAL_DBG("%s ",__func__);

	pcm_config.format		= PCM_FORMAT_S16_LE;		/* sample format */
	pcm_config.rate			= params.samplerate;		/* stream rate */
	pcm_config.channels		= params.channels;			/* count of channels */
	pcm_config.period_count		= 4;					/* number of periods */
	pcm_config.period_size	= params.periodsize;
	pcm_config.start_threshold = params.aout_startThd;	/* threshold frames when to start audio output. */

	RHAL_DBG("================[INFO]================");
	RHAL_DBG("format: %d",pcm_config.format);
	RHAL_DBG("rate: %d",pcm_config.rate);
	RHAL_DBG("channels: %d",pcm_config.channels);
	RHAL_DBG("period_count: %d",pcm_config.period_count);
	RHAL_DBG("period_size: %d",pcm_config.period_size);
	RHAL_DBG("start_threshold: %d",pcm_config.start_threshold);
	RHAL_DBG(" ");
	RHAL_DBG("buffersize: %d",params.buffersize);
	RHAL_DBG("================[INFO]================");

	pcm_out = open_pcm_device(PCM_CARD, PCM_OUT_DEVICE, &pcm_config, PCM_OUT);
#endif // TCC_AUDIO_DEVICE

	////////////////
	/////
	/////  TODO
	/////
	////////////////
#ifdef USE_DUMP
	wfp = fopen("/data/vendor/seed/pcm.bin", "wb");
#endif //USE_DUMP
	return ret;
}


RET dev_aout_write(uint8 *pbuf, int32 frame_size)
{
	RET ret = eRET_OK;

#ifdef TCC_AUDIO_DEVICE
	ssize_t n = 0;

	if(pcm_out){
		n = pcm_write(pcm_out, (int8 *)pbuf, frame_size);
	    if (n < 0)
	    {
	        RHAL_DBG("%s : spk out pcm read fail!!.. %zd write %d", __func__, n, frame_size);
	        return n;
	    }
	}else{
	}


#endif // TCC_AUDIO_DEVICE

	////////////////
	/////
	/////  TODO
	/////
	////////////////

	return ret;
}

RET dev_aout_close(void)
{
	RET ret = eRET_OK;

#ifdef TCC_AUDIO_DEVICE
	RHAL_DBG("%s ",__func__);

	if (pcm_out)
    {
        pcm_close(pcm_out);
		pcm_out = NULL;
    }
#endif // TCC_AUDIO_DEVICE

	////////////////
	/////
	/////  TODO
	/////
	////////////////

	return ret;
}


RET dev_aout_command(eAUDIO_DRV_CMD_t cmd, void *args)
{
	RET ret = eRET_OK;

#ifdef TCC_AUDIO_DEVICE
	RHAL_DBG("%s -> %d",__func__,cmd);

	// See the "external/tinyalsa/pcm.c"

	if (pcm_out != NULL) {
		switch(cmd) {
			case eAUDIO_DRV_CMD_PREPARE:
				pcm_prepare(pcm_out);
				break;

			case eAUDIO_DRV_CMD_DROP:
				pcm_stop(pcm_out);
				break;

			case eAUDIO_DRV_CMD_START:
				pcm_start(pcm_out);
				break;

			case eAUDIO_DRV_CMD_AVAIL_UPDATE:
				ret = (RET)pcm_avail_update(pcm_out);
				break;

			default:
				ret = eRET_NG_NO_RSC;
				break;
		}
	} else {
		ret = eRET_NG_NO_RSC;
	}
#endif // TCC_AUDIO_DEVICE


	////////////////
	/////
	/////  TODO
	/////
	////////////////

	return ret;
}


RET dev_ain_open(stAUDIO_CONFIG_t params)
{
	RET ret = eRET_OK;

#ifdef TCC_AUDIO_DEVICE
	RHAL_DBG("%s ",__func__);

	pcm_config.format		= PCM_FORMAT_S16_LE;		/* sample format */
	pcm_config.rate			= params.samplerate;		/* stream rate */
	pcm_config.channels		= params.channels;			/* count of channels */
	//pcm_config.buffer_time	= 0;					/* ring buffer length in us */
	//pcm_config.period_time	= 0;					/* period time in us */
	pcm_config.period_count		= 4;					/* number of periods */
	//pcm_config.buffer_size	= params.buffersize;
	pcm_config.period_size	= params.periodsize;
	pcm_config.start_threshold = params.aout_startThd;	/* threshold frames when to start audio output. */

	RHAL_DBG("================[INFO]================");
	RHAL_DBG("format: %d",pcm_config.format);
	RHAL_DBG("rate: %d",pcm_config.rate);
	RHAL_DBG("channels: %d",pcm_config.channels);
	RHAL_DBG("period_count: %d",pcm_config.period_count);
	RHAL_DBG("period_size: %d",pcm_config.period_size);
	RHAL_DBG("start_threshold: %d",pcm_config.start_threshold);
	RHAL_DBG(" ");
	RHAL_DBG("buffersize: %d",params.buffersize);
	RHAL_DBG("================[INFO]================");

	pcm_in = open_pcm_device(PCM_CARD, PCM_IN_DEVICE, &pcm_config, PCM_IN);
#endif // TCC_AUDIO_DEVICE


	////////////////
	/////
	/////  TODO
	/////
	////////////////

	return ret;
}


RET dev_ain_read(uint8 *pbuf, int32 frame_size)
{
	RET ret = eRET_OK;

#ifdef TCC_AUDIO_DEVICE
	//RHAL_DBG("%s frame_size %d",__func__,frame_size);

	ssize_t n = 0;

	if(pcm_in){
		n = pcm_read(pcm_in, (int8 *)pbuf, frame_size);
	    if (n < 0)
	    {
	        RHAL_DBG("%s : spk out pcm read fail!!.. %zd read size %d", __func__, n,frame_size);
	        return n;
	    }
	}
#endif //TCC_AUDIO_DEVICE


	////////////////
	/////
	/////  TODO
	/////
	////////////////

	return ret;
}

RET dev_ain_close(void)
{
	RET ret = eRET_OK;

#ifdef TCC_AUDIO_DEVICE
	RHAL_DBG("%s ",__func__);

	if (pcm_in)
    {
        pcm_close(pcm_in);
		pcm_in = NULL;
    }
#endif //TCC_AUDIO_DEVICE


	////////////////
	/////
	/////  TODO
	/////
	////////////////

	return ret;
}

RET dev_ain_command(eAUDIO_DRV_CMD_t cmd, void *args)
{
	RET ret = eRET_OK;

#ifdef TCC_AUDIO_DEVICE
	RHAL_DBG("%s -> %d",__func__,cmd);

	// See the "external/tinyalsa/pcm.c"

	if (pcm_in != NULL) {
		switch(cmd) {
			case eAUDIO_DRV_CMD_PREPARE:
				pcm_prepare(pcm_in);
				break;
			case eAUDIO_DRV_CMD_DROP:
				pcm_stop(pcm_in);
				break;

			case eAUDIO_DRV_CMD_START:
				pcm_start(pcm_in);
				break;

			case eAUDIO_DRV_CMD_AVAIL_UPDATE:
				ret = (RET)pcm_avail_update(pcm_in);
				break;

			default:
				ret = eRET_NG_NO_RSC;
				break;
		}
	} else {
		ret = eRET_NG_NO_RSC;
	}
#endif //TCC_AUDIO_DEVICE


	////////////////
	/////
	/////  TODO
	/////
	////////////////

	return ret;
}

#endif	/* #ifdef __ANDROID__ */