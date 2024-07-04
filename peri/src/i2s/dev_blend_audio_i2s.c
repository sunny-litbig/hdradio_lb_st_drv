/*******************************************************************************

*   FileName : dev_blend_audio_i2s.c

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device blend audio i2s functions and definitions

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
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/poll.h>
#include <errno.h>

#include "tcradio_types.h"
#include "dev_blend_audio_i2s.h"

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
#define HDR_BLEND_AUDIO_I2S_DEV	"/dev/tcc-hdr-blend"

#define	UINT32_NEGATIVE(x)	(~((uint32)(x))+1U)

/***************************************************
*           Local constant definitions              *
****************************************************/
static struct pollfd poll_evt;

static int8* blend_audio_buf = NULL;
static BLEND_AUDIO_RX_PARAM g_blend_audio_rx_param;
static BLEND_AUDIO_I2S_PARAM g_blend_audio_i2s_param;
static int32 blend_audio_dev_fd = -1;
static int32 already_set_blend_audio_params = 0;
static int32 already_blend_audio_start = 0;

/***************************************************
*           Local type definitions                 *
****************************************************/
//#define	USE_AUDIO_I2S_RX_POLL  <- enable this code to print debugging messages

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition				*
****************************************************/
int32 dev_blend_audio_i2s_open(void)
{
	if(blend_audio_dev_fd >= 0){
		BAI2S_ERR("[%s:%d]: Device is already opened\n", __func__, __LINE__);
		return (0);
	}

	already_set_blend_audio_params = 0;

	blend_audio_dev_fd = open(HDR_BLEND_AUDIO_I2S_DEV, O_RDWR | O_NDELAY);
	if(blend_audio_dev_fd == -1){
		BAI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
		return (-1);
	}

	return 0;
}

int32 dev_blend_audio_i2s_close(void)
{
	int32 ret;

	if(blend_audio_dev_fd == -1){
		BAI2S_ERR("[%s:%d]: Device is already closed\n", __func__, __LINE__);
		return (0);
	}

	if(already_blend_audio_start == 1){
		ret = ioctl(blend_audio_dev_fd, BLENDI2S_RX_STOP, &g_blend_audio_i2s_param.eChannel);
		if(ret != 0){
			BAI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
			return (-1);
		}
		already_blend_audio_start = 0;
	}

	(void)close(blend_audio_dev_fd);

	blend_audio_dev_fd = -1;
	already_set_blend_audio_params = 0;

	if(blend_audio_buf != NULL) {
		free((void*)blend_audio_buf);
		blend_audio_buf = NULL;
	}

	return 0;
}

int32 dev_blend_audio_i2s_stop(void)
{

	int32 ret;

	if(blend_audio_dev_fd == -1){
		BAI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return (0);
	}

	ret = ioctl(blend_audio_dev_fd, BLENDI2S_RX_STOP, &g_blend_audio_i2s_param.eChannel);
	if(ret != 0){
		BAI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
		return (-1);
	}

	already_blend_audio_start = 0;

	return 0;
}

int32 dev_blend_audio_i2s_startWithParams(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize)		// samplerate : Hz, buffersize unit : kbyte, periodsize unit : byte
{
	int32 ret;
	/* Slave and Radio mode is enabled by default*/

	BAI2S_DBG("[%s:%d]: ch[%d], bit[%d], samplerate[%d], total buffer size[%d], period size[%d]\n", __func__, __LINE__, nchannels, nbit, samplerate, buffersize, periodsize);

	if(blend_audio_dev_fd == -1) {
		BAI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return (0);
	}

	g_blend_audio_i2s_param.eRadioMode = 0;		// 0 : Audio I2S mode, 1 : Radio I/Q I2S mode

	if((samplerate >= MIN_BLEND_SAMPLERATE) && (samplerate <= MAX_BLEND_SAMPLERATE))
	{
		g_blend_audio_i2s_param.eSampleRate = (uint32)samplerate;
	}
	else {
		BAI2S_ERR("[%s:%d]: Invalid samplerate[%d]!! Please insert from 8Khz to 192Khz.\n", __func__, __LINE__, samplerate);
		return (-1);
	}

	if(nchannels == NUM_OF_AUDIO_CH) {
		g_blend_audio_i2s_param.eChannel = (uint32)nchannels;
	}
	else {
		BAI2S_ERR("[%s:%d]: Invalid channel number!! Please insert 2ch(L&R).\n", __func__, __LINE__);
		return (-1);
	}

	if((nbit == 16) || (nbit == 24)) {
		g_blend_audio_i2s_param.eBitMode = (uint32)nbit;
	}
	else {
		BAI2S_ERR("[%s:%d]: Invalid BitMode!! Please insert 16bit or 24bit(32bit LE align).\n", __func__, __LINE__);
		return (-1);
	}

	if((buffersize >= ((int32)MIN_BUFFER_SIZE/1024)) && (buffersize <= ((int32)MAX_BUFFER_SIZE/1024))) {
		g_blend_audio_i2s_param.eBufferSize = (uint32)buffersize * 1024U;
	}
	else if(buffersize > ((int32)MAX_BUFFER_SIZE/1024)) {
		g_blend_audio_i2s_param.eBufferSize = MAX_BUFFER_SIZE;
	}
	else {
		g_blend_audio_i2s_param.eBufferSize = MIN_BUFFER_SIZE;
	}

	if(buffersize < 0){
		BAI2S_ERR("[%s:%d]: Invalid buffer size!! Please insert more than 0!\n", __func__, __LINE__);
		return (-1);
	}

	if(blend_audio_buf == NULL){
		blend_audio_buf = (int8 *)malloc(g_blend_audio_i2s_param.eBufferSize);
	}
	if(blend_audio_buf != NULL){
		(void)memset((void*)blend_audio_buf, 0, g_blend_audio_i2s_param.eBufferSize);
	}

	if((periodsize < 256) || (periodsize > (256*1024))) {
		BAI2S_ERR("[%s:%d]: Invalid buffer size!! The audio period size must be greater than 256byte and less than 256Kbytes.\n", __func__, __LINE__);
		return (-1);
	}

	g_blend_audio_i2s_param.ePeriodSize = (uint32)periodsize;

	if(already_set_blend_audio_params == 0){
		ret = ioctl(blend_audio_dev_fd, BLENDI2S_SET_PARAMS, &g_blend_audio_i2s_param);
		if(ret < 0){
			BAI2S_ERR("[%s:%d]: BLENDI2S_SET_PARAMS error!!: %s\n", __func__, __LINE__, strerror(errno));
			return(-1);
		}
		if(ret > 0){
			BAI2S_ERR("[%s:%d]: Invalid parameters and set with returned parameters.\n", __func__, __LINE__);
			ret = ioctl(blend_audio_dev_fd, BLENDI2S_SET_PARAMS, &g_blend_audio_i2s_param);
			if(ret != 0){
				BAI2S_ERR("[%s:%d]: BLENDI2S_SET_PARAMS 2nd error!!, %s\n", __func__, __LINE__, strerror(errno));
				return(-1);
			}
		}
		already_set_blend_audio_params = 1;
	}
	else {
		BAI2S_ERR("[%s:%d]: Device is already set parameters!!\n", __func__, __LINE__);
	}

	if(already_blend_audio_start == 0){
		ret = ioctl(blend_audio_dev_fd, BLENDI2S_RX_START, &g_blend_audio_i2s_param.eChannel);
		if(ret != 0){
			BAI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
			return (-1);
		}
		already_blend_audio_start = 1;
	}
	else {
		BAI2S_ERR("[%s:%d]: Device is already started!!\n", __func__, __LINE__);
	}

	return (0);
}

int32 dev_blend_audio_i2s_setParameters(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize)		// samplerate : Hz, buffersize unit : kbyte, periodsize unit : byte
{
	int32 ret;
	/* Slave and Radio mode is enabled by default*/

	BAI2S_DBG("[%s:%d]: ch[%d], bit[%d], samplerate[%d], total buffer size[%d], period size[%d]\n", __func__, __LINE__, nchannels, nbit, samplerate, buffersize, periodsize);

	if(blend_audio_dev_fd == -1) {
		BAI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return (0);
	}

	g_blend_audio_i2s_param.eRadioMode = 0;		// 0 : Audio I2S mode, 1 : Radio I/Q I2S mode

	if((samplerate >= MIN_BLEND_SAMPLERATE) && (samplerate <= MAX_BLEND_SAMPLERATE))
	{
		g_blend_audio_i2s_param.eSampleRate = (uint32)samplerate;
	}
	else {
		BAI2S_ERR("[%s:%d]: Invalid samplerate[%d]!! Please insert from 8Khz to 192Khz.\n", __func__, __LINE__, samplerate);
		return (-1);
	}

	if(nchannels == NUM_OF_AUDIO_CH) {
		g_blend_audio_i2s_param.eChannel = (uint32)nchannels;
	}
	else {
		BAI2S_ERR("[%s:%d]: Invalid channel number!! Please insert 2ch(L&R).\n", __func__, __LINE__);
		return (-1);
	}

	if((nbit == 16) || (nbit == 24)) {
		g_blend_audio_i2s_param.eBitMode = (uint32)nbit;
	}
	else {
		BAI2S_ERR("[%s:%d]: Invalid BitMode!! Please insert 16bit or 24bit(32bit LE align).\n", __func__, __LINE__);
		return (-1);
	}

	if(buffersize < 0){
		BAI2S_ERR("[%s:%d]: Invalid buffer size!! Please insert more than 0!\n", __func__, __LINE__);
		return (-1);
	}

	if((buffersize >= ((int32)MIN_BUFFER_SIZE/1024)) && (buffersize <= ((int32)MAX_BUFFER_SIZE/1024))) {
		g_blend_audio_i2s_param.eBufferSize = (uint32)buffersize * 1024U;
	}
	else if(buffersize > ((int32)MAX_BUFFER_SIZE/1024)) {
		g_blend_audio_i2s_param.eBufferSize = MAX_BUFFER_SIZE;
	}
	else {
		g_blend_audio_i2s_param.eBufferSize = MIN_BUFFER_SIZE;
	}

	if(blend_audio_buf == NULL){
		blend_audio_buf = (int8 *)malloc(g_blend_audio_i2s_param.eBufferSize);
	}

	if(blend_audio_buf != NULL){
		(void)memset((void*)blend_audio_buf, 0, g_blend_audio_i2s_param.eBufferSize);
	}

	if((periodsize < 256) || (periodsize > (256*1024))) {
		BAI2S_ERR("[%s:%d]: Invalid buffer size!! The audio period size must be greater than 256byte and less than 256Kbytes.\n", __func__, __LINE__);
		return (-1);
	}

	g_blend_audio_i2s_param.ePeriodSize = (uint32)periodsize;

	if(already_set_blend_audio_params == 0){
		ret = ioctl(blend_audio_dev_fd, BLENDI2S_SET_PARAMS, &g_blend_audio_i2s_param);
		if(ret < 0){
			BAI2S_ERR("[%s:%d]: BLENDI2S_SET_PARAMS error!!: %s\n", __func__, __LINE__, strerror(errno));
			return(-1);
		}
		if(ret > 0){
			BAI2S_ERR("[%s:%d]: Invalid parameters and set with returned parameters.\n", __func__, __LINE__);
			ret = ioctl(blend_audio_dev_fd, BLENDI2S_SET_PARAMS, &g_blend_audio_i2s_param);
			if(ret != 0){
				BAI2S_ERR("[%s:%d]: BLENDI2S_SET_PARAMS 2nd error!!, %s\n", __func__, __LINE__, strerror(errno));
				return(-1);
			}
		}
		already_set_blend_audio_params = 1;
	}
	else {
		BAI2S_ERR("[%s:%d]: Device is already set parameters!!\n", __func__, __LINE__);
	}

	return (0);
}

int32 dev_blend_audio_i2s_start(void)
{
	int32 ret;
	/* Slave and Radio mode is enabled by default*/

	BAI2S_DBG("[%s:%d]: \n", __func__, __LINE__);

	if(blend_audio_dev_fd == -1) {
		BAI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return (0);
	}

	if(already_set_blend_audio_params == 0) {
		BAI2S_ERR("[%s:%d]: Device is not set parameters!!\n", __func__, __LINE__);
		return (-1);
	}

	if(already_blend_audio_start == 0){
		ret = ioctl(blend_audio_dev_fd, BLENDI2S_RX_START, &g_blend_audio_i2s_param.eChannel);
		if(ret != 0){
			BAI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
			return (-1);
		}
		already_blend_audio_start = 1;
	}
	else {
		BAI2S_ERR("[%s:%d]: Device is already started!!\n", __func__, __LINE__);
	}

	return (0);
}

int32 dev_blend_audio_i2s_read(int8 *data, int32 readsize)		// readsize unit : byte
{
	int32 ch_readbyte;

	if(blend_audio_dev_fd == -1){
		BAI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return(-1);
	}

	if(data == NULL){
		BAI2S_ERR("[%s:%d]: Device buffer null pointer error!!\n", __func__, __LINE__);
		return(-1);
	}

	if((readsize <= 0) || (readsize > (int32)g_blend_audio_i2s_param.eBufferSize)) {
		BAI2S_ERR("[%s:%d]: Read size error!!, readsize[%d bytes]\n", __func__, __LINE__, readsize);
		return (-1);
	}

	if(already_blend_audio_start == 0) {
		BAI2S_ERR("[%s:%d]: Device is not started!!\n", __func__, __LINE__);
		return(-1);
	}

#ifdef USE_AUDIO_I2S_RX_POLL
	int32 ret;
	(void)memset((void*)&poll_evt, 0, sizeof(poll_evt));
	poll_evt.fd = blend_audio_dev_fd;
	poll_evt.events = POLLIN;
	ret = poll(&poll_evt, 1u, 1000);
	if(ret>0){
		if(poll_evt.revents & POLLERR){
			BAI2S_ERR("[%s:%d]: Poll error!!\n", __func__, __LINE__);
			ret = -1;
			return (-1);
		}else if(poll_evt.revents & POLLIN){
			g_blend_audio_rx_param.eBuf = blend_audio_buf;
			g_blend_audio_rx_param.eReadCount = readsize;
			g_blend_audio_rx_param.eIndex = 0;

			ch_readbyte = ioctl(blend_audio_dev_fd, BLENDI2S_AUDIO_MODE_RX_DAI, &g_blend_audio_rx_param);
			if(ch_readbyte != readsize){
				if(ch_readbyte >= 0) {
					BAI2S_ERR("[%s:%d]: rxsize[%d] differs readsize[%d]\n", __func__, __LINE__, ch_readbyte, readsize);
				}
				else {
					if(g_blend_audio_rx_param.eReadCount == UINT32_NEGATIVE(EPIPE)) {	/* EPIPE(32) : Broken pipe */
						BAI2S_ERR("[%s:%d]: xrun\n", __func__, __LINE__);
						ch_readbyte = -EPIPE;
					}
					else {
						BAI2S_ERR("[%s:%d]: %s\n", __func__, __LINE__, strerror(errno));
					}
				}
			}
			else {
				(void)memcpy((void*)data, (void*)blend_audio_buf, ch_readbyte);
			}
		}
	}
#else
	g_blend_audio_rx_param.eBuf = blend_audio_buf;
	g_blend_audio_rx_param.eReadCount = (uint32)readsize;
	g_blend_audio_rx_param.eIndex = 0;

	ch_readbyte = ioctl(blend_audio_dev_fd, BLENDI2S_AUDIO_MODE_RX_DAI, &g_blend_audio_rx_param);
	if(ch_readbyte == readsize) {
		(void)memcpy((void*)data, (void*)blend_audio_buf, (size_t)ch_readbyte);
	}
	else {
		if(ch_readbyte >= 0) {
			BAI2S_ERR("[%s:%d]: rxsize[%d] differs readsize[%d]\n", __func__, __LINE__, ch_readbyte, readsize);
		}
		else {
			if(g_blend_audio_rx_param.eReadCount == UINT32_NEGATIVE(EPIPE)) {	/* EPIPE(32) : Broken pipe */
				BAI2S_ERR("[%s:%d]: xrun\n", __func__, __LINE__);
				ch_readbyte = -EPIPE;
			}
			else {
				BAI2S_ERR("[%s:%d]: %s\n", __func__, __LINE__, strerror(errno));
			}
		}
	}
#endif
	return ch_readbyte ;
}

int32 dev_blend_audio_i2s_get_valid(uint32 *valid)
{
	int32 ret;

	if(blend_audio_dev_fd == -1){
		BAI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return(-1);
	}

	ret = ioctl(blend_audio_dev_fd, BLENDI2S_GET_VALID_BYTES, valid);
	if(ret < 0) {
		BAI2S_ERR("[%s:%d]: Faild to read valid bytes, %s\n", __func__, __LINE__, strerror(errno));
		return(-1);
	}

	return ret;
}

