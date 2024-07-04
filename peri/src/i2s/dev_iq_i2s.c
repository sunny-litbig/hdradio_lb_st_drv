/*******************************************************************************

*   FileName : dev_iq_i2s.c

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device IQ I2S HAL functions and definitions

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
#include "dev_iq_i2s.h"

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
#define IQ01_I2S_DEV	"/dev/tcc-iq01"
#define	IQ23_I2S_DEV	"/dev/tcc-iq23"

#define	UINT32_NEGATIVE(x)	(~((uint32)(x))+1U)

/***************************************************
*           Local constant definitions              *
****************************************************/

static struct pollfd poll_evt;

static int8* iq01_buf[CH_MAX] = {NULL, NULL, NULL, NULL};
static RADIO_IQ_RX_PARAM g_iq01_rx_param[CH_MAX];
static IQ_I2S_PARAM g_iq01_i2s_param;
static int32 iq01_dev_fd = -1;
static int32 already_set_iq01_params = 0;
static int32 already_iq01_start = 0;

static int8* iq23_buf[CH_MAX] = {NULL, NULL, NULL, NULL};
static RADIO_IQ_RX_PARAM g_iq23_rx_param[CH_MAX];
static IQ_I2S_PARAM g_iq23_i2s_param;
static int32 iq23_dev_fd = -1;
static int32 already_set_iq23_params = 0;
static int32 already_iq23_start = 0;

/***************************************************
*           Local type definitions                 *
****************************************************/
//#define USE_IQ_I2S_RX_POLL     <- enable this code to print debugging messages

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition				*
****************************************************/
int32 dev_iq01_i2s_open(void)
{
	if(iq01_dev_fd >= 0){
		IQI2S_ERR("[%s:%d]: Device is already opened\n", __func__, __LINE__);
		return (0);
	}

	already_set_iq01_params = 0;

	iq01_dev_fd = open(IQ01_I2S_DEV, O_RDWR | O_NDELAY);
	if(iq01_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
		return (-1);
	}

	return 0;
}

int32 dev_iq01_i2s_close(void)
{
	int32 ret;
	uint32 i;

	if(iq01_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: Device is already closed\n", __func__, __LINE__);
		return (0);
	}

	if(already_iq01_start == 1){
		ret = ioctl(iq01_dev_fd, IQI2S_RX_STOP, &g_iq01_i2s_param.eChannel);
		if(ret != 0){
			IQI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
			return (-1);
		}
		already_iq01_start = 0;
	}

	(void)close(iq01_dev_fd);

	iq01_dev_fd = -1;
	already_set_iq01_params = 0;

	if(g_iq01_i2s_param.eChannel > 1U){
		for(i=0; i<g_iq01_i2s_param.eChannel; i++){
			if(iq01_buf[i] != NULL) {
				free((void*)(iq01_buf[i]));
				iq01_buf[i] = NULL;
			}
		}
	}

	return 0;
}

int32 dev_iq01_i2s_stop(void)
{

	int32 ret;

	if(iq01_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return (0);
	}

	ret = ioctl(iq01_dev_fd, IQI2S_RX_STOP, &g_iq01_i2s_param.eChannel);
	if(ret != 0){
		IQI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
		return (-1);
	}

	already_iq01_start = 0;

	return 0;
}

int32 dev_iq01_i2s_startWithParams(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize)		// samplerate : Hz, buffersize unit : kbyte, periodsize unit : byte
{
	int32 ret;
	uint32 i;
	int8 afile_name[CH_MAX][20];

	/* Slave and Radio mode is enabled by default*/

	IQI2S_DBG("[%s:%d]: ch[%d], bit[%d], samplerate[%d], total buffer size[%d], period size[%d]\n", __func__, __LINE__, nchannels, nbit, samplerate, buffersize, periodsize);

	if(iq01_dev_fd == -1) {
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return (0);
	}

	g_iq01_i2s_param.eRadioMode = 1;		// 0 : Audio I2S mode, 1 : Radio I/Q I2S mode

	if((samplerate >= (int32)MIN_IQ_SAMPLERATE) && (samplerate <= (int32)MAX_IQ_SAMPLERATE)) {
		g_iq01_i2s_param.eSampleRate = (uint32)samplerate;
	}
	else {
		IQI2S_ERR("[%s:%d]: Invalid samplerate[%d]!! Please insert from 160Khz to 1500Khz.\n", __func__, __LINE__, samplerate);
		return (-1);
	}

	g_iq01_i2s_param.eChannel = (uint32)nchannels;
	if((nchannels != 1) && (nchannels != 2) && (nchannels != 4)) {
		IQI2S_ERR("[%s:%d]: Invalid channel number!! Please insert from 0 to 4!\n", __func__, __LINE__);
		return (-1);
	}

	if(nbit <= 0) {
		IQI2S_ERR("[%s:%d]: Invalid BitMode!! Please insert more than 0!\n", __func__, __LINE__);
		return (-1);
	}
	else {
		g_iq01_i2s_param.eBitMode = (uint32)nbit;
	}

	if((buffersize >= ((int32)MIN_BUFFER_SIZE/1024)) && (buffersize <= ((int32)MAX_BUFFER_SIZE/1024))) {
		g_iq01_i2s_param.eBufferSize = (uint32)buffersize * 1024U;
	}
	else if(buffersize > ((int32)MAX_BUFFER_SIZE/1024)) {
		g_iq01_i2s_param.eBufferSize = MAX_BUFFER_SIZE;
	}
	else {
		g_iq01_i2s_param.eBufferSize = MIN_BUFFER_SIZE;
	}

	if(buffersize < 0){
		IQI2S_ERR("[%s:%d]: Invalid buffer size!! Please insert more than 0!\n", __func__, __LINE__);
		return (-1);
	}

	if(g_iq01_i2s_param.eChannel > 1U){
		for(i=0; i<g_iq01_i2s_param.eChannel; i++){
			(void)sprintf(afile_name[i], "%s_%d.txt", "read_test_0", i);
			if(iq01_buf[i] == NULL){
				iq01_buf[i] = (int8 *)malloc(g_iq01_i2s_param.eBufferSize);
			}
			if(iq01_buf[i] != NULL){
				(void)memset((void*)(iq01_buf[i]), 0, g_iq01_i2s_param.eBufferSize);
			}
		}
	}

	if((periodsize < 512) || (periodsize > (256*1024))) {
		IQI2S_ERR("[%s:%d]: Invalid buffer size!! The radio period size must be greater than 512byte and less than 256Kbytes.\n", __func__, __LINE__);
		return (-1);
	}

	g_iq01_i2s_param.ePeriodSize = (uint32)periodsize;

	if(already_set_iq01_params == 0){
		ret = ioctl(iq01_dev_fd, IQI2S_SET_PARAMS, &g_iq01_i2s_param);
		if(ret < 0){
			IQI2S_ERR("[%s:%d]: IQI2S_SET_PARAMS error!!: %s\n", __func__, __LINE__, strerror(errno));
			return(-1);
		}
		if(ret > 0){
			IQI2S_ERR("[%s:%d]: Invalid parameters and set with returned parameters.\n", __func__, __LINE__);
			ret = ioctl(iq01_dev_fd, IQI2S_SET_PARAMS, &g_iq01_i2s_param);
			if(ret != 0){
				IQI2S_ERR("[%s:%d]: IQI2S_SET_PARAMS 2nd error!!, %s\n", __func__, __LINE__, strerror(errno));
				return(-1);
			}
		}
		already_set_iq01_params = 1;
	}
	else {
		IQI2S_ERR("[%s:%d]: Device is already set parameters!!\n", __func__, __LINE__);
	}

	if(already_iq01_start == 0){
		ret = ioctl(iq01_dev_fd, IQI2S_RX_START, &g_iq01_i2s_param.eChannel);
		if(ret != 0){
			IQI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
			return (-1);
		}
		already_iq01_start = 1;
	}
	else {
		IQI2S_ERR("[%s:%d]: Device is already started!!\n", __func__, __LINE__);
	}

	return (0);
}

int32 dev_iq01_i2s_setParameters(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize)		// samplerate : Hz, buffersize unit : kbyte, periodsize unit : byte
{
	int32 ret;
	uint32 i;

	/* Slave and Radio mode is enabled by default*/

	IQI2S_DBG("[%s:%d]: ch[%d], bit[%d], samplerate[%d], total buffer size[%d], period size[%d]\n", __func__, __LINE__, nchannels, nbit, samplerate, buffersize, periodsize);

	if(iq01_dev_fd == -1) {
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return (0);
	}

	g_iq01_i2s_param.eRadioMode = 1;		// 0 : Audio I2S mode, 1 : Radio I/Q I2S mode

	if((samplerate >= (int32)MIN_IQ_SAMPLERATE) && (samplerate <= (int32)MAX_IQ_SAMPLERATE)) {
		g_iq01_i2s_param.eSampleRate = (uint32)samplerate;
	}
	else {
		IQI2S_ERR("[%s:%d]: Invalid samplerate[%d]!! Please insert from 160Khz to 1500Khz.\n", __func__, __LINE__, samplerate);
		return (-1);
	}

	g_iq01_i2s_param.eChannel = (uint32)nchannels;
	if((nchannels != 1) && (nchannels != 2) && (nchannels != 4)) {
		IQI2S_ERR("[%s:%d]: Invalid channel number!! Please insert from 0 to 4!\n", __func__, __LINE__);
		return (-1);
	}

	if(nbit <= 0) {
		IQI2S_ERR("[%s:%d]: Invalid BitMode!! Please insert more than 0!\n", __func__, __LINE__);
		return (-1);
	}
	else {
		g_iq01_i2s_param.eBitMode = (uint32)nbit;
	}

	if(buffersize < 0){
		IQI2S_ERR("[%s:%d]: Invalid buffer size!! Please insert more than 0!\n", __func__, __LINE__);
		return (-1);
	}

	if((buffersize >= ((int32)MIN_BUFFER_SIZE/1024)) && (buffersize <= ((int32)MAX_BUFFER_SIZE/1024))) {
		g_iq01_i2s_param.eBufferSize = (uint32)buffersize * 1024U;
	}
	else if(buffersize > ((int32)MAX_BUFFER_SIZE/1024)) {
		g_iq01_i2s_param.eBufferSize = MAX_BUFFER_SIZE;
	}
	else {
		g_iq01_i2s_param.eBufferSize = MIN_BUFFER_SIZE;
	}

	if(g_iq01_i2s_param.eChannel > 1U){
		for(i=0; i<g_iq01_i2s_param.eChannel; i++){
			if(iq01_buf[i] == NULL){
				iq01_buf[i] = (int8 *)malloc(g_iq01_i2s_param.eBufferSize);
			}
			if(iq01_buf[i] != NULL){
				(void)memset((void*)(iq01_buf[i]), 0, g_iq01_i2s_param.eBufferSize);
			}
		}
	}

	if((periodsize < 512) || (periodsize > (256*1024))) {
		IQI2S_ERR("[%s:%d]: Invalid buffer size!! The radio period size must be greater than 512byte and less than 256Kbytes.\n", __func__, __LINE__);
		return (-1);
	}

	g_iq01_i2s_param.ePeriodSize = (uint32)periodsize;

	if(already_set_iq01_params == 0){
		ret = ioctl(iq01_dev_fd, IQI2S_SET_PARAMS, &g_iq01_i2s_param);
		if(ret < 0){
			IQI2S_ERR("[%s:%d]: IQI2S_SET_PARAMS error!!: %s\n", __func__, __LINE__, strerror(errno));
			return(-1);
		}
		if(ret > 0){
			IQI2S_ERR("[%s:%d]: Invalid parameters and set with returned parameters.\n", __func__, __LINE__);
			ret = ioctl(iq01_dev_fd, IQI2S_SET_PARAMS, &g_iq01_i2s_param);
			if(ret != 0){
				IQI2S_ERR("[%s:%d]: IQI2S_SET_PARAMS 2nd error!!, %s\n", __func__, __LINE__, strerror(errno));
				return(-1);
			}
		}
		already_set_iq01_params = 1;
	}
	else {
		IQI2S_ERR("[%s:%d]: Device is already set parameters!!\n", __func__, __LINE__);
	}

	return (0);
}

int32 dev_iq01_i2s_start(void)
{
	int32 ret;

	IQI2S_DBG("[%s:%d]:\n", __func__, __LINE__);

	if(iq01_dev_fd == -1) {
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return (0);
	}

	if(already_set_iq01_params == 0) {
		IQI2S_ERR("[%s:%d]: Device is not set parameters!!\n", __func__, __LINE__);
		return (-1);
	}

	if(already_iq01_start == 0){
		ret = ioctl(iq01_dev_fd, IQI2S_RX_START, &g_iq01_i2s_param.eChannel);
		if(ret != 0){
			IQI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
			return (-1);
		}
		already_iq01_start = 1;
	}
	else {
		IQI2S_ERR("[%s:%d]: Device is already started!!\n", __func__, __LINE__);
	}

	return (0);
}

int32 dev_iq01_i2s_read(int8 *data, int32 readsize)		// readsize unit : byte
{
	uint32 i;
	int32 ch_readbyte[CH_MAX]={0,};

	if(iq01_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return(-1);
	}

	if(data == NULL){
		IQI2S_ERR("[%s:%d]: Device buffer null pointer error!!\n", __func__, __LINE__);
		return(-1);
	}

	if((readsize <= 0) || (readsize > (int32)g_iq01_i2s_param.eBufferSize)) {
		IQI2S_ERR("[%s:%d]: Read size error!!, readsize[%d bytes]\n", __func__, __LINE__, readsize);
		return (-1);
	}

	if(already_iq01_start == 0) {
		IQI2S_ERR("[%s:%d]: Device is not started!!\n", __func__, __LINE__);
		return(-1);
	}

#ifdef USE_IQ_I2S_RX_POLL
	int32 ret = 0;
	(void)memset((void*)&poll_evt, 0, sizeof(poll_evt));
	poll_evt.fd = iq01_dev_fd;
	poll_evt.events = POLLIN;
	ret = poll(&poll_evt, 1u, 1000);
	if(ret>0){
		if(poll_evt.revents & POLLERR){
			IQI2S_ERR("[%s:%d]: Poll error!!\n", __func__, __LINE__);
			return (-1);
		}else if(poll_evt.revents & POLLIN){
			if(g_iq01_i2s_param.eChannel > 1){
				for(i=0; i<g_iq01_i2s_param.eChannel; i++){
					g_iq01_rx_param[i].eBuf = iq01_buf[i];
					g_iq01_rx_param[i].eReadCount = readsize;
					g_iq01_rx_param[i].eIndex = i;

					ch_readbyte[i] = ioctl(iq01_dev_fd, IQI2S_RADIO_MODE_RX_DAI, &g_iq01_rx_param[i]);
					if(ch_readbyte[i] != readsize){
						if(ch_readbyte[i] >= 0) {
							IQI2S_ERR("[%s:%d]: DAI CH[%d], rxsize[%d] differs readsize[%d]\n", __func__, __LINE__, i, ch_readbyte[i], readsize);
						}
						else {
							if(g_iq01_rx_param[i].eReadCount == UINT32_NEGATIVE(EPIPE)) {	/* EPIPE(32) : Broken pipe */
								IQI2S_ERR("[%s:%d]: channel[%d] xrun\n", __func__, __LINE__, i);
								ch_readbyte[i] = -EPIPE;
							}
							else {
								IQI2S_ERR("[%s:%d]: %s\n", __func__, __LINE__, strerror(errno));
							}
						}
						return ch_readbyte[i];
					}
				}

				for(i=0; i< g_iq01_i2s_param.eChannel; i++){
					(void)memcpy((void*)(data+(i*readsize)), (void*)(iq01_buf[i]),ch_readbyte[i]);
				}
			}
		}
	}
#else
	if(g_iq01_i2s_param.eChannel > 1U){
		for(i=0; i<g_iq01_i2s_param.eChannel; i++){
			g_iq01_rx_param[i].eBuf = iq01_buf[i];
			g_iq01_rx_param[i].eReadCount = (uint32)readsize;
			g_iq01_rx_param[i].eIndex = (int32)i;

			ch_readbyte[i] = ioctl(iq01_dev_fd, IQI2S_RADIO_MODE_RX_DAI, &g_iq01_rx_param[i]);
			if(ch_readbyte[i] != readsize){
				if(ch_readbyte[i] >= 0) {
					IQI2S_ERR("[%s:%d]: DAI CH[%d], rxsize[%d] differs readsize[%d]\n", __func__, __LINE__, i, ch_readbyte[i], readsize);
				}
				else {
					if(g_iq01_rx_param[i].eReadCount == UINT32_NEGATIVE(EPIPE)) {	/* EPIPE(32) : Broken pipe */
						IQI2S_ERR("[%s:%d]: channel[%d] xrun\n", __func__, __LINE__, i);
						ch_readbyte[i] = -32;
					}
					else {
						IQI2S_ERR("[%s:%d]: %s\n", __func__, __LINE__, strerror(errno));
					}
				}
				return ch_readbyte[i];
			}
		}

		for(i=0; i< g_iq01_i2s_param.eChannel; i++){
			(void)memcpy((void*)(data+((int32)i*readsize)), (void*)iq01_buf[i], (size_t)ch_readbyte[i]);
		}
	}
#endif

	return ch_readbyte[0] ;
}

int32 dev_iq01_i2s_read_ch(int8 *ch0, int8 *ch1, int8 *ch2, int8 *ch3, int32 readsize)		// readsize unit : byte
{
	uint32 i;
	int32 ch_readbyte[CH_MAX]={0,};

	if(iq01_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return(-1);
	}

	if(ch0 == NULL){
		IQI2S_ERR("[%s:%d]: Device buffer null pointer error!!\n", __func__, __LINE__);
		return(-1);
	}

	if((readsize <= 0) || (readsize > (int32)g_iq01_i2s_param.eBufferSize)) {
		IQI2S_ERR("[%s:%d]: Read size error!!, readsize[%d bytes]\n", __func__, __LINE__, readsize);
		return (-1);
	}

	if(already_iq01_start == 0) {
		IQI2S_ERR("[%s:%d]: Device is not started!!\n", __func__, __LINE__);
		return(-1);
	}

#ifdef USE_IQ_I2S_RX_POLL
	int32 ret = 0;
	(void)memset((void*)&poll_evt, 0, sizeof(poll_evt));
	poll_evt.fd = iq01_dev_fd;
	poll_evt.events = POLLIN;
	ret = poll(&poll_evt, 1u, 1000);
	if(ret>0){
		if(poll_evt.revents & POLLERR){
			IQI2S_ERR("[%s:%d]: Poll error!!\n", __func__, __LINE__);
			return (-1);
		}else if(poll_evt.revents & POLLIN){
			if(g_iq01_i2s_param.eChannel > 1){
				for(i=0; i<g_iq01_i2s_param.eChannel; i++){
					g_iq01_rx_param[i].eBuf = iq01_buf[i];
					g_iq01_rx_param[i].eReadCount = readsize;
					g_iq01_rx_param[i].eIndex = i;

					ch_readbyte[i] = ioctl(iq01_dev_fd, IQI2S_RADIO_MODE_RX_DAI, &g_iq01_rx_param[i]);
					if(ch_readbyte[i] != readsize){
						if(ch_readbyte[i] >= 0) {
							IQI2S_ERR("[%s:%d]: DAI CH[%d], rxsize[%d] differs readsize[%d]\n", __func__, __LINE__, i, ch_readbyte[i], readsize);
						}
						else {
							if(g_iq01_rx_param[i].eReadCount == UINT32_NEGATIVE(EPIPE)) {	/* EPIPE(32) : Broken pipe */
								IQI2S_ERR("[%s:%d]: channel[%d] xrun\n", __func__, __LINE__, i);
								ch_readbyte[i] = -EPIPE;
							}
							else {
								IQI2S_ERR("[%s:%d]: %s\n", __func__, __LINE__, strerror(errno));
							}
						}
						return ch_readbyte[i];
					}
				}

				(void)memcpy((void*)ch0, (void*)(iq01_buf[0]), ch_readbyte[0]);
				if(ch1 != NULL) {
					(void)memcpy((void*)ch1, (void*)(iq01_buf[1]), ch_readbyte[1]);
				}
				if(ch2 != NULL) {
					(void)memcpy((void*)ch2, (void*)(iq01_buf[2]), ch_readbyte[2]);
				}
				if(ch3 != NULL) {
					(void)memcpy((void*)ch3, (void*)(iq01_buf[3]), ch_readbyte[3]);
				}
			}
		}
	}
#else
	if(g_iq01_i2s_param.eChannel > 1U){
		for(i=0; i<g_iq01_i2s_param.eChannel; i++){
			g_iq01_rx_param[i].eBuf = iq01_buf[i];
			g_iq01_rx_param[i].eReadCount = (uint32)readsize;
			g_iq01_rx_param[i].eIndex = (int32)i;

			ch_readbyte[i] = ioctl(iq01_dev_fd, IQI2S_RADIO_MODE_RX_DAI, &g_iq01_rx_param[i]);
			if(ch_readbyte[i] != readsize){
				if(ch_readbyte[i] >= 0) {
					IQI2S_ERR("[%s:%d]: DAI CH[%d], rxsize[%d] differs readsize[%d]\n", __func__, __LINE__, i, ch_readbyte[i], readsize);
				}
				else {
					if(g_iq01_rx_param[i].eReadCount == UINT32_NEGATIVE(EPIPE)) {	/* EPIPE(32) : Broken pipe */
						IQI2S_ERR("[%s:%d]: channel[%d] xrun\n", __func__, __LINE__, i);
						ch_readbyte[i] = -EPIPE;
					}
					else {
						IQI2S_ERR("[%s:%d]: %s\n", __func__, __LINE__, strerror(errno));
					}
				}
				return ch_readbyte[i];
			}
		}

		(void)memcpy((void*)ch0, (void*)iq01_buf[0], (size_t)ch_readbyte[0]);
		if(ch1 != NULL) {
			(void)memcpy((void*)ch1, (void*)iq01_buf[1], (size_t)ch_readbyte[1]);
		}
		if(ch2 != NULL) {
			(void)memcpy((void*)ch2, (void*)iq01_buf[2], (size_t)ch_readbyte[2]);
		}
		if(ch3 != NULL) {
			(void)memcpy((void*)ch3, (void*)iq01_buf[3], (size_t)ch_readbyte[3]);
		}
	}
#endif

	return ch_readbyte[0] ;
}

int32 dev_iq01_i2s_get_valid(uint32 *valid)
{
	int32 ret;

	if(iq01_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return(-1);
	}

	ret = ioctl(iq01_dev_fd, IQI2S_GET_VALID_BYTES, valid);
	if(ret < 0) {
		IQI2S_ERR("[%s:%d]: Faild to read valid bytes, %s\n", __func__, __LINE__, strerror(errno));
		return(-1);
	}
	else {
		ret = 0;
	}

	return ret;
}

int32 dev_iq23_i2s_open(void)
{
	if(iq23_dev_fd >= 0){
		IQI2S_ERR("[%s:%d]: Device is already opened\n", __func__, __LINE__);
		return (0);
	}

	already_set_iq23_params = 0;

	iq23_dev_fd = open(IQ23_I2S_DEV, O_RDWR | O_NDELAY);
	if(iq23_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
		return (-1);
	}

	return 0;
}

int32 dev_iq23_i2s_close(void)
{
	int32 ret;
	uint32 i;

	if(iq23_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: Device is already closed\n", __func__, __LINE__);
		return (0);
	}

	if(already_iq23_start == 1){
		ret = ioctl(iq23_dev_fd, IQI2S_RX_STOP, &g_iq23_i2s_param.eChannel);
		if(ret != 0){
			IQI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
			return (-1);
		}
		already_iq23_start = 0;
	}

	(void)close(iq23_dev_fd);

	iq23_dev_fd = -1;
	already_set_iq23_params = 0;

	if(g_iq23_i2s_param.eChannel > 1U){
		for(i=0; i<g_iq23_i2s_param.eChannel; i++){
			if(iq23_buf[i] != NULL) {
				free((void*)(iq23_buf[i]));
				iq23_buf[i] = NULL;
			}
		}
	}

	return 0;
}

int32 dev_iq23_i2s_stop(void)
{
	int32 ret;

	if(iq23_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return (0);
	}

	ret = ioctl(iq23_dev_fd, IQI2S_RX_STOP, &g_iq23_i2s_param.eChannel);
	if(ret != 0){
		IQI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
		return (-1);
	}

	already_iq23_start = 0;

	return 0;
}

int32 dev_iq23_i2s_startWithParams(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize)		// samplerate : Hz, buffersize unit : kbyte, periodsize unit : byte
{
	int32 ret;
	uint32 i;
	int8 afile_name[CH_MAX][20];

	/* Slave and Radio mode is enabled by default*/

	IQI2S_DBG("[%s:%d]: ch[%d], bit[%d], samplerate[%d], total buffer size[%d], period size[%d]\n", __func__, __LINE__, nchannels, nbit, samplerate, buffersize, periodsize);

	if(iq23_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return (0);
	}

	g_iq23_i2s_param.eRadioMode = 1;		// 0 : Audio I2S mode, 1 : Radio I/Q I2S mode

	if((samplerate >= (int32)MIN_IQ_SAMPLERATE) && (samplerate <= (int32)MAX_IQ_SAMPLERATE)) {
		g_iq23_i2s_param.eSampleRate = (uint32)samplerate;
	}
	else {
		IQI2S_ERR("[%s:%d]: Invalid samplerate[%d]!! Please insert from 160Khz to 1500Khz.\n", __func__, __LINE__, samplerate);
		return (-1);
	}

	g_iq23_i2s_param.eChannel = (uint32)nchannels;
	if((nchannels != 1) && (nchannels != 2) && (nchannels != 4)) {
		IQI2S_ERR("[%s:%d]: Invalid channel number!! Please insert from 0 to 4!\n", __func__, __LINE__);
		return (-1);
	}

	if(nbit <= 0) {
		IQI2S_ERR("[%s:%d]: Invalid BitMode!! Please insert more than 0!\n", __func__, __LINE__);
		return (-1);
	}
	else {
		g_iq23_i2s_param.eBitMode = (uint32)nbit;
	}

	if((buffersize >= ((int32)MIN_BUFFER_SIZE/1024)) && (buffersize <= ((int32)MAX_BUFFER_SIZE/1024))) {
		g_iq23_i2s_param.eBufferSize = (uint32)buffersize * 1024U;
	}
	else if(buffersize > ((int32)MAX_BUFFER_SIZE/1024)) {
		g_iq23_i2s_param.eBufferSize = MAX_BUFFER_SIZE;
	}
	else {
		g_iq23_i2s_param.eBufferSize = MIN_BUFFER_SIZE;
	}

	if(buffersize < 0){
		IQI2S_ERR("[%s:%d]: Invalid buffer size!! Please insert more than 0!\n", __func__, __LINE__);
		return (-1);
	}

	if(g_iq23_i2s_param.eChannel > 1U){
		for(i=0; i<g_iq23_i2s_param.eChannel; i++){
			(void)sprintf(afile_name[i], "%s_%d.txt", "read_test_0", i);
			if(iq23_buf[i] == NULL){
				iq23_buf[i] = (int8 *)malloc(g_iq23_i2s_param.eBufferSize);
			}
			if(iq23_buf[i] != NULL){
				(void)memset((void*)(iq23_buf[i]), 0, g_iq23_i2s_param.eBufferSize);
			}
		}
	}

	if((periodsize < 512) || (periodsize > (256*1024))) {
		IQI2S_ERR("[%s:%d]: Invalid buffer size!! The radio period size must be greater than 512byte and less than 256Kbytes.\n", __func__, __LINE__);
		return (-1);
	}

	g_iq23_i2s_param.ePeriodSize = (uint32)periodsize;

	if(already_set_iq23_params == 0){
		ret = ioctl(iq23_dev_fd, IQI2S_SET_PARAMS, &g_iq23_i2s_param);
		if(ret < 0){
			IQI2S_ERR("[%s:%d]: IQI2S_SET_PARAMS error!!, %s\n", __func__, __LINE__, strerror(errno));
			return(-1);
		}
		if(ret > 0){
			IQI2S_ERR("[%s:%d]: Invalid parameters and set with returned parameters.\n", __func__, __LINE__);
			ret = ioctl(iq23_dev_fd, IQI2S_SET_PARAMS, &g_iq23_i2s_param);
			if(ret != 0){
				IQI2S_ERR("[%s:%d]: IQI2S_SET_PARAMS 2nd error!!, %s\n", __func__, __LINE__, strerror(errno));
				return(-1);
			}
		}
		already_set_iq23_params = 1;
	}
	else {
		IQI2S_ERR("[%s:%d]: Device is already set parameters!!\n", __func__, __LINE__);
	}

	if(already_iq23_start == 0){
		ret = ioctl(iq23_dev_fd, IQI2S_RX_START, &g_iq23_i2s_param.eChannel);
		if(ret != 0){
			IQI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
			return (-1);
		}
		already_iq23_start = 1;
	}
	else {
		IQI2S_ERR("[%s:%d]: Device is already started!!\n", __func__, __LINE__);
	}

	return (0);
}

int32 dev_iq23_i2s_setParameters(int32 nchannels, int32 nbit, int32 samplerate, int32 buffersize, int32 periodsize)		// samplerate : Hz, buffersize unit : kbyte, periodsize unit : byte
{
	uint32 i;

	/* Slave and Radio mode is enabled by default*/

	IQI2S_DBG("[%s:%d]: ch[%d], bit[%d], samplerate[%d], total buffer size[%d], period size[%d]\n", __func__, __LINE__, nchannels, nbit, samplerate, buffersize, periodsize);

	if(iq23_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return (0);
	}

	g_iq23_i2s_param.eRadioMode = 1;		// 0 : Audio I2S mode, 1 : Radio I/Q I2S mode

	if((samplerate >= (int32)MIN_IQ_SAMPLERATE) && (samplerate <= (int32)MAX_IQ_SAMPLERATE)) {
		g_iq23_i2s_param.eSampleRate = (uint32)samplerate;
	}
	else {
		IQI2S_ERR("[%s:%d]: Invalid samplerate[%d]!! Please insert from 160Khz to 1500Khz.\n", __func__, __LINE__, samplerate);
		return (-1);
	}

	g_iq23_i2s_param.eChannel = (uint32)nchannels;
	if((nchannels != 1) && (nchannels != 2) && (nchannels != 4)) {
		IQI2S_ERR("[%s:%d]: Invalid channel number!! Please insert from 0 to 4!\n", __func__, __LINE__);
		return (-1);
	}

	if(nbit <= 0) {
		IQI2S_ERR("[%s:%d]: Invalid BitMode!! Please insert more than 0!\n", __func__, __LINE__);
		return (-1);
	}
	else {
		g_iq23_i2s_param.eBitMode = (uint32)nbit;
	}

	if(buffersize < 0){
		IQI2S_ERR("[%s:%d]: Invalid buffer size!! Please insert more than 0!\n", __func__, __LINE__);
		return (-1);
	}

	if((buffersize >= ((int32)MIN_BUFFER_SIZE/1024)) && (buffersize <= ((int32)MAX_BUFFER_SIZE/1024))) {
		g_iq23_i2s_param.eBufferSize = (uint32)buffersize * 1024U;
	}
	else if(buffersize > ((int32)MAX_BUFFER_SIZE/1024)) {
		g_iq23_i2s_param.eBufferSize = MAX_BUFFER_SIZE;
	}
	else {
		g_iq23_i2s_param.eBufferSize = MIN_BUFFER_SIZE;
	}

	if(g_iq23_i2s_param.eChannel > 1U){
		for(i=0; i<g_iq23_i2s_param.eChannel; i++){
			if(iq23_buf[i] == NULL){
				iq23_buf[i] = (int8 *)malloc(g_iq23_i2s_param.eBufferSize);
			}
			if(iq23_buf[i] != NULL){
				(void)memset((void*)iq23_buf[i], 0, g_iq23_i2s_param.eBufferSize);
			}
		}
	}

	if((periodsize < 512) || (periodsize > (256*1024))) {
		IQI2S_ERR("[%s:%d]: Invalid buffer size!! The radio period size must be greater than 512byte and less than 256Kbytes.\n", __func__, __LINE__);
		return (-1);
	}

	g_iq23_i2s_param.ePeriodSize = (uint32)periodsize;

	if(already_set_iq23_params == 0){
		int32 ret;
		ret = ioctl(iq23_dev_fd, IQI2S_SET_PARAMS, &g_iq23_i2s_param);
		if(ret < 0){
			IQI2S_ERR("[%s:%d]: IQI2S_SET_PARAMS error!!, %s\n", __func__, __LINE__, strerror(errno));
			return(-1);
		}
		if(ret > 0){
			IQI2S_ERR("[%s:%d]: Invalid parameters and set with returned parameters.\n", __func__, __LINE__);
			ret = ioctl(iq23_dev_fd, IQI2S_SET_PARAMS, &g_iq23_i2s_param);
			if(ret != 0){
				IQI2S_ERR("[%s:%d]: IQI2S_SET_PARAMS 2nd error!!, %s\n", __func__, __LINE__, strerror(errno));
				return(-1);
			}
		}
		already_set_iq23_params = 1;
	}
	else {
		IQI2S_ERR("[%s:%d]: Device is already set parameters!!\n", __func__, __LINE__);
	}

	return (0);
}

int32 dev_iq23_i2s_start(void)
{
	int32 ret;

	IQI2S_DBG("[%s:%d]:\n", __func__, __LINE__);

	if(iq23_dev_fd == -1) {
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return (0);
	}

	if(already_set_iq23_params == 0) {
		IQI2S_ERR("[%s:%d]: Device is not set parameters!!\n", __func__, __LINE__);
		return (-1);
	}

	if(already_iq23_start == 0){
		ret = ioctl(iq23_dev_fd, IQI2S_RX_START, &g_iq23_i2s_param.eChannel);
		if(ret != 0){
			IQI2S_ERR("[%s:%d]: %s!!\n", __func__, __LINE__, strerror(errno));
			return (-1);
		}
		already_iq23_start = 1;
	}
	else {
		IQI2S_ERR("[%s:%d]: Device is already started!!\n", __func__, __LINE__);
	}

	return (0);
}

int32 dev_iq23_i2s_read(int8 *data, int32 readsize)		// readsize unit : byte
{
	uint32 i;
	int32 ch_readbyte[CH_MAX]={0,};

	if(iq23_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return(-1);
	}

	if(data == NULL){
		IQI2S_ERR("[%s:%d]: Device buffer null pointer error!!\n", __func__, __LINE__);
		return(-1);
	}

	if((readsize <= 0) || (readsize > (int32)g_iq23_i2s_param.eBufferSize)) {
		IQI2S_ERR("[%s:%d]: Read size error!!, readsize[%d bytes]\n", __func__, __LINE__, readsize);
		return (-1);
	}

	if(already_iq23_start == 0) {
		IQI2S_ERR("[%s:%d]: Device is not started!!\n", __func__, __LINE__);
		return(-1);
	}

#ifdef USE_IQ_I2S_RX_POLL
	int32 ret = 0;
	(void)memset((void*)&poll_evt, 0, sizeof(poll_evt));
	poll_evt.fd = iq23_dev_fd;
	poll_evt.events = POLLIN;
	ret = poll(&poll_evt, 1u, 1000);
	if(ret>0){
		if(poll_evt.revents & POLLERR){
			IQI2S_ERR("[%s:%d]: Poll error!!\n", __func__, __LINE__);
			return (-1);
		}else if(poll_evt.revents & POLLIN){
			if(g_iq23_i2s_param.eChannel > 1){
				for(i=0; i<g_iq23_i2s_param.eChannel; i++){
					g_iq23_rx_param[i].eBuf = iq23_buf[i];
					g_iq23_rx_param[i].eReadCount = readsize;
					g_iq23_rx_param[i].eIndex = i;

					ch_readbyte[i] = ioctl(iq23_dev_fd, IQI2S_RADIO_MODE_RX_DAI, &g_iq23_rx_param[i]);
					if(ch_readbyte[i] != readsize){
						if(ch_readbyte[i] >= 0) {
							IQI2S_ERR("[%s:%d]: DAI CH[%d], rxsize[%d] differs readsize[%d]\n", __func__, __LINE__, i, ch_readbyte[i], readsize);
						}
						else {
							if(g_iq23_rx_param[i].eReadCount == UINT32_NEGATIVE(EPIPE)) {	/* EPIPE(32) : Broken pipe */
								IQI2S_ERR("[%s:%d]: channel[%d] xrun\n", __func__, __LINE__, i);
								ch_readbyte[i] = -EPIPE;
							}
							else {
								IQI2S_ERR("[%s:%d]: %s\n", __func__, __LINE__, strerror(errno));
							}
						}
						return ch_readbyte[i];
					}
				}

				for(i=0; i< g_iq23_i2s_param.eChannel; i++){
					(void)memcpy((void*)(data+(i*readsize)), (void*)(iq23_buf[i]), ch_readbyte[i]);
				}
			}
		}
	}
#else
	if(g_iq23_i2s_param.eChannel > 1U){
		for(i=0; i<g_iq23_i2s_param.eChannel; i++){
			g_iq23_rx_param[i].eBuf = iq23_buf[i];
			g_iq23_rx_param[i].eReadCount = (uint32)readsize;
			g_iq23_rx_param[i].eIndex = (int32)i;

			ch_readbyte[i] = ioctl(iq23_dev_fd, IQI2S_RADIO_MODE_RX_DAI, &g_iq23_rx_param[i]);
			if(ch_readbyte[i] != readsize){
				if(ch_readbyte[i] >= 0) {
					IQI2S_ERR("[%s:%d]: DAI CH[%d], rxsize[%d] differs readsize[%d]\n", __func__, __LINE__, i, ch_readbyte[i], readsize);
				}
				else {
					if(g_iq23_rx_param[i].eReadCount == UINT32_NEGATIVE(EPIPE)) {	/* EPIPE(32) : Broken pipe */
						IQI2S_ERR("[%s:%d]: channel[%d] xrun\n", __func__, __LINE__, i);
						ch_readbyte[i] = -EPIPE;
					}
					else {
						IQI2S_ERR("[%s:%d]: %s\n", __func__, __LINE__, strerror(errno));
					}
				}
				return ch_readbyte[i];
			}
		}

		for(i=0; i< g_iq23_i2s_param.eChannel; i++){
			(void)memcpy((void*)(data+((int32)i*readsize)), (void*)(iq23_buf[i]), (size_t)ch_readbyte[i]);
		}
	}
#endif

	return ch_readbyte[0] ;
}

int32 dev_iq23_i2s_read_ch(int8 *ch0, int8 *ch1, int8 *ch2, int8 *ch3, int32 readsize)		// readsize unit : byte
{
	uint32 i;
	int32 ch_readbyte[CH_MAX]={0,};

	if(iq23_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return(-1);
	}

	if(ch0 == NULL){
		IQI2S_ERR("[%s:%d]: Device buffer null pointer error!!\n", __func__, __LINE__);
		return(-1);
	}

	if((readsize <= 0) || (readsize > (int32)g_iq23_i2s_param.eBufferSize)) {
		IQI2S_ERR("[%s:%d]: Read size error!!, readsize[%d bytes]\n", __func__, __LINE__, readsize);
		return (-1);
	}

	if(already_iq23_start == 0) {
		IQI2S_ERR("[%s:%d]: Device is not started!!\n", __func__, __LINE__);
		return(-1);
	}

#ifdef USE_IQ_I2S_RX_POLL
	int32 ret = 0;
	(void)memset((void*)&poll_evt, 0, sizeof(poll_evt));
	poll_evt.fd = iq23_dev_fd;
	poll_evt.events = POLLIN;
	ret = poll(&poll_evt, 1u, 1000);
	if(ret>0){
		if(poll_evt.revents & POLLERR){
			IQI2S_ERR("[%s:%d]: Poll error!!\n", __func__, __LINE__);
			return (-1);
		}else if(poll_evt.revents & POLLIN){
			if(g_iq23_i2s_param.eChannel > 1){
				for(i=0; i<g_iq23_i2s_param.eChannel; i++){
					g_iq23_rx_param[i].eBuf = iq23_buf[i];
					g_iq23_rx_param[i].eReadCount = readsize;
					g_iq23_rx_param[i].eIndex = i;

					ch_readbyte[i] = ioctl(iq23_dev_fd, IQI2S_RADIO_MODE_RX_DAI, &g_iq23_rx_param[i]);
					if(ch_readbyte[i] != readsize){
						if(ch_readbyte[i] >= 0) {
							IQI2S_ERR("[%s:%d]: DAI CH[%d], rxsize[%d] differs readsize[%d]\n", __func__, __LINE__, i, ch_readbyte[i], readsize);
						}
						else {
							if(g_iq23_rx_param[i].eReadCount == UINT32_NEGATIVE(EPIPE)) {	/* EPIPE(32) : Broken pipe */
								IQI2S_ERR("[%s:%d]: channel[%d] xrun\n", __func__, __LINE__, i);
								ch_readbyte[i] = -EPIPE;
							}
							else {
								IQI2S_ERR("[%s:%d]: %s\n", __func__, __LINE__, strerror(errno));
							}
						}
						return ch_readbyte[i];
					}
				}
				(void)memcpy((void*)ch0, (void*)(iq23_buf[0]), (size_t)ch_readbyte[0]);
			if(ch1 != NULL)
				(void)memcpy((void*)ch1, (void*)(iq23_buf[1]), (size_t)ch_readbyte[1]);
			if(ch2 != NULL)
				(void)memcpy((void*)ch2, (void*)(iq23_buf[2]), (size_t)ch_readbyte[2]);
			if(ch3 != NULL)
				(void)memcpy((void*)ch3, (void*)(iq23_buf[3]), (size_t)ch_readbyte[3]);
			}
		}
	}
#else
	if(g_iq23_i2s_param.eChannel > 1U){
		for(i=0; i<g_iq23_i2s_param.eChannel; i++){
			g_iq23_rx_param[i].eBuf = iq23_buf[i];
			g_iq23_rx_param[i].eReadCount = (uint32)readsize;
			g_iq23_rx_param[i].eIndex = (int32)i;

			ch_readbyte[i] = ioctl(iq23_dev_fd, IQI2S_RADIO_MODE_RX_DAI, &g_iq23_rx_param[i]);
			if(ch_readbyte[i] != readsize){
				if(ch_readbyte[i] >= 0) {
					IQI2S_ERR("[%s:%d]: DAI CH[%d], rxsize[%d] differs readsize[%d]\n", __func__, __LINE__, i, ch_readbyte[i], readsize);
				}
				else {
					if(g_iq23_rx_param[i].eReadCount == UINT32_NEGATIVE(EPIPE)) {	/* EPIPE(32) : Broken pipe */
						IQI2S_ERR("[%s:%d]: channel[%d] xrun\n", __func__, __LINE__, i);
						ch_readbyte[i] = -EPIPE;
					}
					else {
						IQI2S_ERR("[%s:%d]: %s\n", __func__, __LINE__, strerror(errno));
					}
				}
				return ch_readbyte[i];
			}
		}
		(void)memcpy((void*)ch0, (void*)iq23_buf[0], (size_t)ch_readbyte[0]);
		if(ch1 != NULL){
			(void)memcpy((void*)ch1, (void*)iq23_buf[1], (size_t)ch_readbyte[1]);
		}
		if(ch2 != NULL) {
			(void)memcpy((void*)ch2, (void*)iq23_buf[2], (size_t)ch_readbyte[2]);
		}
		if(ch3 != NULL) {
			(void)memcpy((void*)ch3, (void*)iq23_buf[3], (size_t)ch_readbyte[3]);
		}
	}
#endif

	return ch_readbyte[0] ;
}

int32 dev_iq23_i2s_get_valid(uint32 *valid)
{
	int32 ret;

	if(iq23_dev_fd == -1){
		IQI2S_ERR("[%s:%d]: Device is not opened!!\n", __func__, __LINE__);
		return(-1);
	}

	ret = ioctl(iq23_dev_fd, IQI2S_GET_VALID_BYTES, valid);
	if(ret < 0) {
		IQI2S_ERR("[%s:%d]: Faild to read valid bytes, %s\n", __func__, __LINE__, strerror(errno));
		return(-1);
	}
	else {
		ret = 0;
	}

	return ret;
}

