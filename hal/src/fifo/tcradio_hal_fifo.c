/*******************************************************************************

*   FileName : tcradio_hal_fifo.c

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
#include <stdio.h>
#include <stdlib.h>

#include "tcradio_types.h"
#include "tcradio_peri_config.h"
#include "tcradio_hal.h"
#include "tcradio_hal_config.h"
#include "tcradio_hal_mutex.h"
#include "tcradio_hal_rbuf.h"
#include "tcradio_hal_fifo.h"
#include "tcradio_hal_utils.h"

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
static stBUFFER_RING_t gstAudioFifo[AUDIO_FIFO_NUM];
static char *gpAudioFifoBuffer[AUDIO_FIFO_NUM];

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition				*
****************************************************/
RET tcradiohal_audiofifo_init(int32 numOfAudfifo)
{
	RET ret = (RET)eRET_OK;
	int32 i = 0;

	if(numOfAudfifo > AUDIO_FIFO_NUM) {
		RAHAL_ERR("[%s:%d] Input parameter is invalid!!!\n", __func__, __LINE__);
		return (RET)eRET_NG_INVALID_PARAM;
	}

	for(i=0; i<numOfAudfifo; i++)
	{
		tcradio_hal_memset(&gstAudioFifo[i], 0, sizeof(stBUFFER_RING_t));
		gpAudioFifoBuffer[i] = (char*)malloc(AUDIO_FIFO_BUFFER_SIZE);

		if(gpAudioFifoBuffer[i] == NULL) {
			RAHAL_ERR("[%s:%d] It is not get memory allocation!!!\n", __func__, __LINE__);
			return (RET)eRET_NG_MALLOC;
		}

		tcradio_hal_memset(gpAudioFifoBuffer[i], 0, sizeof(AUDIO_FIFO_BUFFER_SIZE));
		ringbuffer_init(&gstAudioFifo[i], gpAudioFifoBuffer[i], AUDIO_FIFO_BUFFER_SIZE);
	}
	ret= tcradiohal_audiofifo_mutexInit();

	return ret;
}

RET tcradiohal_audiofifo_deinit(int32 numOfAudfifo)
{
	RET ret = (RET)eRET_OK;
	int32 i = 0;

	if(numOfAudfifo > AUDIO_FIFO_NUM) {
		RAHAL_ERR("[%s:%d] Input parameter is invalid!!!\n", __func__, __LINE__);
		return (RET)eRET_NG_INVALID_PARAM;
	}

	for(i=0; i<numOfAudfifo; i++)
	{
		if(gpAudioFifoBuffer[i] != NULL) {
			free(gpAudioFifoBuffer[i]);
			gpAudioFifoBuffer[i] = NULL;
		}
		tcradio_hal_memset(&gstAudioFifo[i], 0, sizeof(stBUFFER_RING_t));
	}
	ret= tcradiohal_audiofifo_mutexDeinit();

	return ret;
}

RET tcradiohal_audiofifo_reset(int32 audfifo_index)
{
	RET ret = (RET)eRET_OK;

	if(gpAudioFifoBuffer[audfifo_index] != NULL) {
		tcradiohal_audiofifo_mutexLock();
		ringbuffer_init(&gstAudioFifo[audfifo_index], gpAudioFifoBuffer[audfifo_index], AUDIO_FIFO_BUFFER_SIZE);
		tcradiohal_audiofifo_mutexUnlock();
	}
	else {
		RAHAL_ERR("[%s:%d] radio hal audio fifo was not initialize!!!\n", __func__, __LINE__);
	}

	return ret;
}

int tcradiohal_audiofifo_pushData(int32 audfifo_index, uint8 *pucData, uint32 uiSize)
{
	int write_byte=0;

	if(gpAudioFifoBuffer[audfifo_index] != NULL) {
		tcradiohal_audiofifo_mutexLock();
		write_byte = ringbuffer_setData(&gstAudioFifo[audfifo_index], (char *)pucData, uiSize);
		tcradiohal_audiofifo_mutexUnlock();
	}
	else {
		RAHAL_DBG("[%s:%d] radio hal audio fifo was not initialize!!!\n", __func__, __LINE__);
		return (RET)eRET_NG_NO_MEM;
	}

	return write_byte;
}

int tcradiohal_audiofifo_popData(int32 audfifo_index, uint8 *pucData, uint32 uiSize)
{
	int read_byte=0, read_size=0;

	if(gpAudioFifoBuffer[audfifo_index] != NULL) {
		tcradiohal_audiofifo_mutexLock();
	#if 0
		read_size = (int32)(gstAudioFifo[audfifo_index].wp - gstAudioFifo[audfifo_index].rp);
		if(read_size < 0) {
			read_size += (int32)(gstAudioFifo[audfifo_index].maxSize);
		}
		read_byte = ringbuffer_getDataEx(&gstAudioFifo[audfifo_index], (char *)pucData, (long)read_size);
	#else
		read_byte = ringbuffer_getDataEx(&gstAudioFifo[audfifo_index], (char *)pucData, (long)uiSize);
	#endif
		tcradiohal_audiofifo_mutexUnlock();
	}
	else {
		RAHAL_DBG("[%s:%d] radio hal audio fifo was not initialize!!!\n", __func__, __LINE__);
		return (RET)eRET_NG_NO_MEM;
	}

	return read_byte;
}

int tcradiohal_audiofifo_getBlankSize(int32 audfifo_index)
{
	int blank_size=0;

	if(gpAudioFifoBuffer[audfifo_index] != NULL) {
		tcradiohal_audiofifo_mutexLock();
		blank_size = ringbuffer_getBlankSize(&gstAudioFifo[audfifo_index]);
		tcradiohal_audiofifo_mutexUnlock();
	}
	else {
		RAHAL_DBG("[%s:%d] radio hal audio fifo was not initialize!!!\n", __func__, __LINE__);
		return (RET)eRET_NG_NO_MEM;
	}

	return blank_size;
}

