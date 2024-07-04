/*******************************************************************************

*   FileName : dev_gpio.c

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device GPIO functions and definitions

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
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include "tcradio_types.h"
#include "dev_gpio.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
static int32 gpio_fd = -1;

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

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/
#ifndef USE_RADIO_GPIO_DRV
static int32 readGpio(int32 fd, int8 *value);
static int32 writeGpio(int32 fd, const int8 *value);
static int32 exportGpio(const int8 *dir_name, const int8 *port_value);
static int32 setGpio(const int8 *ioPortNum, const int8 *ioDirName, const int8 *ioValue, const int8 *ioDirection, const int8 *ioHighLow);
#endif

/***************************************************
*			function definition				*
****************************************************/
#ifdef USE_RADIO_GPIO_DRV

int32 dev_rgpio_open(void)
{
    int32 ret = 0;
    int32 iBoardType;

    GPIO_DBG("[%s] open [%s]\n", __func__, DXB_CTRL_DEV_FILE);
    gpio_fd = open(DXB_CTRL_DEV_FILE, O_RDWR | O_NDELAY);
    if(gpio_fd < 0)
    {
		GPIO_ERR("[%s:%d]: Failed to open %s, [err]: %s\n ", __func__, __LINE__, DXB_CTRL_DEV_FILE, strerror(errno));
        return -1;
    }

    iBoardType = BOARD_AMFM_TUNER;
    GPIO_DBG("[%s] IOCTL_DXB_CTRL_SET_BOARD(%d)\n", __func__, iBoardType);
    ret = ioctl(gpio_fd, IOCTL_DXB_CTRL_SET_BOARD, &iBoardType);
    if(ret != 0)
    {
        close(gpio_fd);
        gpio_fd = -1;
		GPIO_ERR("[%s:%d]: Failed to set IOCTL(IOCTL_DXB_CTRL_SET_BOARD), [err]: %s\n ", __func__, __LINE__, strerror(errno));
        return -2;
    }

    return ret;
}

int32 dev_rgpio_close(void)
{
    int32 ret = 0;

    GPIO_DBG("[%s] close [%s]\n", __func__, DXB_CTRL_DEV_FILE);
    ret = close(gpio_fd);
	if(ret != 0) {
		GPIO_ERR("[%s:%d]: Failed to close %s, [err]: %s\n ", __func__, __LINE__, DXB_CTRL_DEV_FILE, strerror(errno));
	}
    gpio_fd = -1;

    return ret;
}

#else

int32 dev_rgpio_open(void)
{
    int32 ret = 0;

    gpio_fd = 0;

    return ret;
}

int32 dev_rgpio_close(void)
{
    int32 ret = 0;

    gpio_fd = -1;

    return ret;
}

int32 writeGpio(int32 fd, const int8 *value)
{
	ssize_t bytes_written;
	size_t write_size = strnlen(value, 64);
	int32 ret = -1;

	if(fd < 0) {
		GPIO_ERR("[%s:%d]: Invalid fd!!\n", __func__, __LINE__);
	}
	else {
		int8 tmp_value[65] = {0,};
		(void)strncpy(tmp_value, value, write_size);
		bytes_written = write(fd,(void*)tmp_value,write_size);
		if(bytes_written != (ssize_t)write_size){
			GPIO_ERR("[%s:%d]: Failed to gpio write, [err]: %s\n ", __func__, __LINE__, strerror(errno));
		}
		else {
			ret = 0;	// OK
		}
	}

	return ret;
}

int32 readGpio(int32 fd, int8 *value)
{
	ssize_t bytes_read;
	size_t read_size = 1;
	int32 ret = -1;

	if(fd < 0) {
		GPIO_ERR("[%s:%d]: Invalid fd!!\n", __func__, __LINE__);
	}
	else {
		bytes_read = read(fd,(void*)value,read_size);
		if(bytes_read != (ssize_t)read_size){
			GPIO_ERR("[%s:%d]: Failed to gpio read, [err]: %s\n ", __func__, __LINE__, strerror(errno));
		}
		else {
			ret = 0;	// OK
		}
	}

	return ret;
}

/* export gpio if directory not exsisting*/
int32 exportGpio(const int8 *dir_name, const int8 *port_value)
{
	struct stat s;
	int32 err = stat(dir_name, &s);
	int32 ret = -1;

	if(err == -1) {
		if(writeGpio(gpio_fd,port_value) < 0) {
			GPIO_ERR("[%s:%d]: Failed to gpio export, port_value[%s], [err]: %s\n", __func__, __LINE__, port_value, strerror(errno));
		}else {
			ret= 0;			// OK
		}
	}else {
		if(S_ISDIR(s.st_mode)) {
			ret = 0;		// OK
		}
		else {
			if(writeGpio(gpio_fd,port_value) < 0) {
				GPIO_ERR("[%s:%d]: Failed to gpio export, port_value[%s], [err]: %s\n", __func__, __LINE__, port_value, strerror(errno));
			}else {
				ret = 0;	// OK
			}
		}
	}

	return ret;
}

static int32 setGpio(const int8 *ioPortNum, const int8 *ioDirName, const int8 *ioValue, const int8 *ioDirection, const int8 *ioHighLow)
{
	int32 ret = -1;
	int32 gpio_ctrl_fd = -1;
	int32 gpio_ctrl_export_fd =-1;
	int8 direction_str[5];
	int8 valur_str[5];

	gpio_fd = open(GPIO_EXPORT_PATH, O_WRONLY);
	if( gpio_fd < 0 ) {
		GPIO_ERR("[%s:%d]: Failed to open[%s], gpio_fd[%d], [err]: %s\n", __func__, __LINE__, GPIO_EXPORT_PATH, gpio_fd, strerror(errno));
		goto END;
	}

	/* export gpio file if it is not present*/
	ret = exportGpio(ioDirName, ioPortNum);
	if(ret < 0) {
		goto END;
	}

	gpio_ctrl_export_fd = open(ioDirection, O_RDWR);
	if(gpio_ctrl_export_fd < 0) {
		GPIO_ERR("[%s:%d]: Failed to open[%s], gpio_ctrl_export_fd[%d], [err]: %s\n", __func__, __LINE__, ioDirection, gpio_ctrl_export_fd, strerror(errno));
		goto END;
	}

	/* open gpio value file */
	gpio_ctrl_fd = open(ioValue, O_RDWR);
	if(gpio_ctrl_fd < 0) {
		GPIO_ERR("[%s:%d]: Failed to open[%s], gpio_ctrl_fd[%d], [err]: %s\n", __func__, __LINE__, ioValue, gpio_ctrl_fd, strerror(errno));
		goto END;
	}

	/* setting gpio direction  */
	ret = writeGpio(gpio_ctrl_export_fd, "out");
	if(ret < 0) {
		goto END;
	}

	ret = writeGpio(gpio_ctrl_fd, ioHighLow);
	if(ret < 0) {
		goto END;
	}

	(void)readGpio(gpio_ctrl_export_fd, direction_str);
	(void)readGpio(gpio_ctrl_fd, valur_str);

END:
	if(gpio_fd >= 0){
		(void)close(gpio_fd);
		gpio_fd = -1;
	}

	if(gpio_ctrl_export_fd >= 0){
		(void)close(gpio_ctrl_export_fd);
		gpio_ctrl_export_fd = -1;
	}

	if(gpio_ctrl_fd >= 0){
		(void)close(gpio_ctrl_fd);
		gpio_ctrl_fd = -1;
	}

	return ret;
}
#endif

int32 setTunerPower(int32 onoff)
{
	int32 ret;

	(void)(onoff);
#if defined(TCC8030_BOARD) || defined(TCC8031_BOARD)
 #if 0
 // This pin is not power control, This is for tuner selection.
 #ifdef USE_RADIO_GPIO_DRV
	int32 iDeviceIdx=0;
	if(gpio_fd == (-1)) {
		dev_rgpio_open();
	}
	ret = ioctl(gpio_fd, IOCTL_DXB_CTRL_OFF, &iDeviceIdx);
 #else /* #ifdef USE_RADIO_GPIO_DRV */
	ret = setGpio(TUNPWR_EXPORT_VAL, TUNPWR_CTRL_DIR, TUNPWR_CTRL_VAL, TUNPWR_CTRL_DIRECTION, GPIO_LOW);
 #endif /* #ifdef USE_RADIO_GPIO_DRV */
	if(ret < 0){
		GPIO_ERR("[%s:%d]: Failed to control tuner power pin low!!\n ",__func__, __LINE__);
	}
 #else
 // This port is changed to be controlled directly by the RADIO_GPIO driver on the TCC803x board only.
 #endif
#elif defined(TCC897X_LCN20_BOARD) || defined(TCC897X_LCN30_BOARD) || defined(TCC802X_BOARD) || defined(TCC802X_EVM21_BOARD)
	if(onoff == 1) {
		ret = setGpio(TUNPWR_EXPORT_VAL, TUNPWR_CTRL_DIR, TUNPWR_CTRL_VAL, TUNPWR_CTRL_DIRECTION, GPIO_HIGH);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to control tuner power pin high!!\n ",__func__, __LINE__);
			return -1;
		}
	#if defined(TCC802X_EVM21_BOARD)
		ret = setGpio(LTUNPWR_EXPORT_VAL, LTUNPWR_CTRL_DIR, LTUNPWR_CTRL_VAL, LTUNPWR_CTRL_DIRECTION, GPIO_HIGH);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to control Ltuner power pin high!!\n ",__func__, __LINE__);
		}
	#endif
	}
	else if(onoff == 0) {
		ret = setGpio(TUNPWR_EXPORT_VAL, TUNPWR_CTRL_DIR, TUNPWR_CTRL_VAL, TUNPWR_CTRL_DIRECTION, GPIO_LOW);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to control tuner power pin low!!\n ",__func__, __LINE__);
			return -1;
		}
	#if defined(TCC802X_EVM21_BOARD)
		ret = setGpio(LTUNPWR_EXPORT_VAL, LTUNPWR_CTRL_DIR, LTUNPWR_CTRL_VAL, LTUNPWR_CTRL_DIRECTION, GPIO_LOW);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to control Ltuner power pin low!!\n ",__func__, __LINE__);
		}
	#endif
	}
	else {
		GPIO_ERR("[%s:%d]: Failed to control tuner power pin!! Invalid onoff parameter\n ",__func__, __LINE__);
		ret = -1;
	}
#else
	// On these boards, this signal is controlled by a switch(SDR_PWR_SEL).
	ret = 0;
#endif

	return ret;
}

int32 setAntPower(int32 onoff)
{
	int32 ret = 0;

	(void)(onoff);
#if defined(TCC897X_LCN20_BOARD) || defined(TCC897X_LCN30_BOARD) || defined(TCC802X_BOARD)
 #ifdef USE_RADIO_GPIO_DRV

	////////////////
	/////
	/////  TODO
	/////
	////////////////

 #else
	if(onoff == 1) {
		ret = setGpio(ANTPWR_EXPORT_VAL, ANTPWR_CTRL_DIR, ANTPWR_CTRL_VAL, ANTPWR_CTRL_DIRECTION, GPIO_HIGH);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to control tuner antenna power pin high!!\n ",__func__, __LINE__);
		}
	}
	else if(onoff == 0) {
		ret = setGpio(ANTPWR_EXPORT_VAL, ANTPWR_CTRL_DIR, ANTPWR_CTRL_VAL, ANTPWR_CTRL_DIRECTION, GPIO_LOW);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to control tuner antenna power pin low!!\n ",__func__, __LINE__);
		}
	}
	else {
		GPIO_ERR("[%s:%d]: Failed to control antenna power pin!! Invalid onoff parameter\n ",__func__, __LINE__);
		ret = -1;
	}
 #endif
#endif

	return ret;
}

int32 setTunerReset(int32 onoff)
{
	int32 ret;

	(void)(onoff);
#ifdef USE_RADIO_GPIO_DRV
	int32 iDeviceIdx=0;

    if(gpio_fd == (-1)) {
        dev_rgpio_open();
    }
	if(onoff == 1) {
        ret = ioctl(gpio_fd, IOCTL_DXB_CTRL_RESET_HIGH, &iDeviceIdx);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to tuner reset pin high!!\n ",__func__, __LINE__);
		}
		else {
			GPIO_DBG("[%s:%d]: Tuner reset pin is high\n", __func__, __LINE__);
		}
    }
    else if(onoff == 0) {
        ret = ioctl(gpio_fd, IOCTL_DXB_CTRL_RESET_LOW, &iDeviceIdx);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to tuner reset pin low!!\n ",__func__, __LINE__);
		}
		else {
			GPIO_DBG("[%s:%d]: Tuner reset pin is low\n", __func__, __LINE__);
		}
    }
	else {
		GPIO_ERR("[%s:%d]: Failed to control tuner reset pin!! Invalid onoff parameter\n ",__func__, __LINE__);
		ret = -1;
	}
#else
	if(onoff == 1) {
		ret = setGpio(TUNRST_EXPORT_VAL, TUNRST_CTRL_DIR, TUNRST_CTRL_VAL, TUNRST_CTRL_DIRECTION, GPIO_HIGH);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to tuner reset pin high!!\n ",__func__, __LINE__);
		}
		else {
			GPIO_DBG("[%s:%d]: Tuner reset pin[%s] is high\n", __func__, __LINE__, TUNRST_EXPORT_VAL);
		}
	}
	else if(onoff == 0) {
		ret = setGpio(TUNRST_EXPORT_VAL, TUNRST_CTRL_DIR, TUNRST_CTRL_VAL, TUNRST_CTRL_DIRECTION, GPIO_LOW);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to tuner reset pin low!!\n ",__func__, __LINE__);
		}
		else {
			GPIO_DBG("[%s:%d]: Tuner reset pin[%s] is low\n", __func__, __LINE__, TUNRST_EXPORT_VAL);
		}
	}
	else {
		GPIO_ERR("[%s:%d]: Failed to control tuner reset pin!! Invalid onoff parameter\n ",__func__, __LINE__);
		ret = -1;
	}
#endif

	return ret;
}

int32 setTunerSelect(int32 sel)
{
	int32 ret = 0;

	(void)(sel);
#if defined(TCC802X_EVM21_BOARD)
 #ifdef USE_RADIO_GPIO_DRV

	////////////////
	/////
	/////  TODO
	/////
	////////////////

 #else
	if(sel == 1) {
		ret = setGpio(TUNSEL_EXPORT_VAL, TUNSEL_CTRL_DIR, TUNSEL_CTRL_VAL, TUNSEL_CTRL_DIRECTION, GPIO_HIGH);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to tuner select pin high!!\n ",__func__, __LINE__);
		}
		else {
			GPIO_DBG("[%s:%d]: Tuner select pin[%s] is high\n", __func__, __LINE__, TUNSEL_EXPORT_VAL);
		}
	}
	else if(sel == 0) {
		ret = setGpio(TUNSEL_EXPORT_VAL, TUNSEL_CTRL_DIR, TUNSEL_CTRL_VAL, TUNSEL_CTRL_DIRECTION, GPIO_LOW);
		if(ret < 0){
	        GPIO_ERR("[%s:%d]: Failed to tuner select pin low!!\n ",__func__, __LINE__);
		}
		else {
			GPIO_DBG("[%s:%d]: Tuner select pin[%s] is low\n", __func__, __LINE__, TUNSEL_EXPORT_VAL);
		}
	}
	else {
		GPIO_ERR("[%s:%d]: Failed to control tuner select pin!! Invalid sel parameter\n ",__func__, __LINE__);
		ret = -1;
	}
 #endif
#endif
	return ret;
}

int32 setTestGpio1(int32 onoff)
{
	int32 ret = 0;

	(void)(onoff);
#if defined(TCC8030_BOARD) || defined(TCC8031_BOARD)
 #ifdef USE_RADIO_GPIO_DRV

	////////////////
	/////
	/////  TODO
	/////
	////////////////

 #else

	if(onoff == 1) {
		ret = setGpio(TUNINT1_EXPORT_VAL, TUNINT1_CTRL_DIR, TUNINT1_CTRL_VAL, TUNINT1_CTRL_DIRECTION, GPIO_HIGH);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to tuner reset pin high!!\n ",__func__, __LINE__);
		}
		else {
			GPIO_DBG("[%s:%d]: Tuner reset pin[%s] is high\n", __func__, __LINE__, TUNINT1_EXPORT_VAL);
		}
	}
	else if(onoff == 0) {
		ret = setGpio(TUNINT1_EXPORT_VAL, TUNINT1_CTRL_DIR, TUNINT1_CTRL_VAL, TUNINT1_CTRL_DIRECTION, GPIO_LOW);
		if(ret < 0){
			GPIO_ERR("[%s:%d]: Failed to tuner reset pin low!!\n ",__func__, __LINE__);
		}
		else {
			GPIO_DBG("[%s:%d]: Tuner reset pin[%s] is low\n", __func__, __LINE__, TUNINT1_EXPORT_VAL);
		}
	}
	else {
		GPIO_ERR("[%s:%d]: Failed to control test gpio1 pin!! Invalid onoff parameter\n ",__func__, __LINE__);
		ret = -1;
	}
 #endif
#endif

	return ret;
}
