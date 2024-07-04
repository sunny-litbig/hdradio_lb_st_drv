/*******************************************************************************

*   FileName : dev_i2c.c

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device I2C HAL functions and definitions

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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdbool.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "tcradio_types.h"
#include "dev_i2c.h"

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
#define MAX_I2C_BURST	(256U)
#if defined(TCC897X_LCN20_BOARD) || defined(TCC897X_LCN30_BOARD) || defined(TCC802X_BOARD) || defined(TCC802X_EVM21_BOARD)
#define I2C_DEV 		"/dev/i2c-3"
#else
#define I2C_DEV 		"/dev/i2c-1"
#endif

/***************************************************
*           Local constant definitions              *
****************************************************/
static int32 fd_i2c = -1;
static int32 dev_i2c_status = 0;

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/
int32 iic_write(uint8 slaveAddr, uint8 reg_addr, uint8* data, uint32 count);
int32 iic_read(uint8 slaveAddr, uint8 reg_addr, uint8* data, uint32 count);

/***************************************************
*			function definition				*
****************************************************/
int32 dev_i2c_open(void)
{
	if(dev_i2c_status != 0){
		I2C_ERR("[%s:%d]: I2C device already opened, fd[%i]: %s\n", __func__, __LINE__, fd_i2c, I2C_DEV);
	}
	else {
		fd_i2c = open(I2C_DEV, O_RDWR);
		if(fd_i2c < 0){
			I2C_ERR("[%s:%d]: Failed to open I2C device, fd[%i]: %s\n", __func__, __LINE__, fd_i2c, I2C_DEV);
		}
		else {
			dev_i2c_status = 1;
		}
	}

	return fd_i2c;
}

int32 dev_i2c_close(void)
{
	int32 ret = (RET)eRET_OK;
	int32 err;

	if(dev_i2c_status != 1){
		I2C_ERR("[%s:%d]: I2C device not opened, fd[%i]: %s\n", __func__, __LINE__, fd_i2c, I2C_DEV);
		ret = (RET)eRET_NG_NOT_OPEN;
	}
	else {
		err = close(fd_i2c);
		if(err < 0){
			I2C_ERR("[%s:%d]: Failed to close I2C device, fd[%i]: %s\n", __func__, __LINE__, err, I2C_DEV);
			ret = (RET)eRET_NG_IO;
		}
		else {
			dev_i2c_status = 0;
		}
	}

	return ret;
}

int32 dev_i2c_read8(uint8 dev_addr, uint8 reg_addr, uint8 *data, uint32 data_len)
{
	int32 ret;

	if(dev_i2c_status != 1){
		I2C_ERR("[%s:%d]: I2C device not opened, fd[%i]: %s\n", __func__, __LINE__, fd_i2c, I2C_DEV);
		ret = (RET)eRET_NG_NOT_OPEN;
	}
	else {
		ret = iic_read(dev_addr, reg_addr, data, data_len);
		if(ret != 0){
			I2C_ERR("[%s:%d]: Failed to read I2C, dev_addr[%02Xh], reg_addr[%02Xh], data_len[%d]\n", __func__, __LINE__, dev_addr, reg_addr, data_len);
		}
	}

	return ret;
}

int32 dev_i2c_write8(uint8 dev_addr, uint8 reg_addr, uint8 *data, uint32 data_len)
{
	int32 ret;

	if(dev_i2c_status != 1){
		I2C_ERR("[%s:%d]: I2C device not opened, fd[%i]: %s\n", __func__, __LINE__, fd_i2c, I2C_DEV);
		ret = (RET)eRET_NG_NOT_OPEN;
	}
	else {
		ret = iic_write(dev_addr, reg_addr, data, data_len);
		if(ret != 0){
			I2C_ERR("[%s:%d]: Failed to write I2C, dev_addr[%02Xh], reg_addr[%02Xh], data_len[%d]\n", __func__, __LINE__, dev_addr, reg_addr, data_len);
		}
	}

	return ret;
}

int32 iic_write(uint8 slaveAddr, uint8 reg_addr, uint8* data, uint32 count)
{
	uint8 buf[MAX_I2C_BURST+4U];
	uint32 length = 0;
	int32 ret = (RET)eRET_OK;
	int32 err;
	uint32 cnt = count;

	buf[0] = slaveAddr;
	buf[1] = reg_addr;

	err = ioctl(fd_i2c, I2C_SLAVE_FORCE, (buf[0] >> 1U));
	if(err < 0){
		perror("");
		I2C_ERR("[%s:%d]: Failed to I2C ioctl(I2C_SLAVE), fd[%d], slaveAddr[%02Xh]\n", __func__, __LINE__, fd_i2c, buf[0]);
		ret =  (RET)eRET_NG_IO;
	}
	else {
		while(cnt>0U){
			if(cnt  > MAX_I2C_BURST){
				(void)memcpy((void*)(&buf[2]), (void*)(&data[length]), MAX_I2C_BURST);
				length = MAX_I2C_BURST;
				cnt -= length;
			}else{
				(void)memcpy((void*)(&buf[2]), (void*)(&data[length]), cnt);
				length = cnt;
				cnt -= length;
			}

			err = (int32)write(fd_i2c, (void*)(&buf[1]), (size_t)length+1U);
			if(err < 0){
				I2C_ERR("[%s:%d]: Failed to wrtie I2C, fd[%d], slaveAddr[%02Xh], reg_addr[%02Xh], length[%d], [err]: %s\n", __func__, __LINE__, fd_i2c,slaveAddr, reg_addr, length, strerror(err));
				perror("");
				ret = (RET)eRET_NG_IO;
				break;
			}
		}
	}

	return ret;
}

int32 iic_read(uint8 slaveAddr, uint8 reg_addr, uint8* data, uint32 count)
{
	struct i2c_rdwr_ioctl_data rdwr;
	struct i2c_msg msg[2];
	int32 ret = (RET)eRET_OK;
	int32 err;
	uint8 regAddr = reg_addr;

	msg[ 0 ].addr  = (uint16)slaveAddr >> 1U;
	msg[ 0 ].flags = 0;
	msg[ 0 ].len   = 1;
	msg[ 0 ].buf   = &regAddr;

	if(count > MAX_I2C_BURST){
		I2C_ERR("[%s:%d]: Invalid length, reg_addr[%02Xh], count[%d]\n", __func__, __LINE__, reg_addr, count);
		ret = (RET)eRET_NG_INVALID_PARAM;
	}
	else {
		msg[ 1 ].addr  = (uint16)slaveAddr >> 1U;
		msg[ 1 ].flags = I2C_M_RD;
		msg[ 1 ].len   = (uint16)count ;
		msg[ 1 ].buf   = data;

		rdwr.msgs = msg;
		rdwr.nmsgs = 2;

		err = ioctl(fd_i2c, I2C_RDWR, &rdwr);
		if(err < 0){
			perror("");
			I2C_ERR("[%s:%d]: Failed to read I2C. [err]: %s.\n", __func__, __LINE__, strerror(err));
			ret = (RET)eRET_NG_IO;
		}
	}

	return ret;
}

int32 write_i2c(uint8 i2c_addr, uint8 *data, uint32 datalen)
{
	uint8 buf[MAX_I2C_BURST+4U];
	uint32 length =  0;
	int32 ret = (RET)eRET_OK;
	int32 err;
	uint32 dlen = datalen;

	if(dev_i2c_status != 1){
		I2C_ERR("[%s:%d]: I2C device not opened, fd[%i]: %s\n", __func__, __LINE__, fd_i2c, I2C_DEV);
		ret = (RET)eRET_NG_NOT_OPEN;
	}
	else {
		buf[0] = i2c_addr;

		err = ioctl(fd_i2c, I2C_SLAVE_FORCE, (buf[0] >> 1U));
		if(err < 0){
			perror("");
			I2C_ERR("[%s:%d]: Failed to write I2C ioctl, fd[%d], slaveAddr[%02Xh], [err]: %s\n", __func__, __LINE__, fd_i2c, buf[0], strerror(err));
			ret = (RET)eRET_NG_IO;
		}
		else {
			while(dlen>0U){

				if(dlen  > MAX_I2C_BURST){
					(void)memcpy((void*)(&buf[1]), (void*)(&data[length]), MAX_I2C_BURST);
					length = MAX_I2C_BURST;
					dlen -= length;
				}else{
					(void)memcpy((void*)(&buf[1]), (void*)(&data[length]), dlen);
					length = dlen;
					dlen -= length;
				}

				err = (int32)write(fd_i2c, (void*)(&buf[1]), (length));
				if(err < 0){
					perror("");
					I2C_ERR("[%s:%d]: Failed to I2C write, fd[%d], i2c_addr[%02Xh], reg_addr[%02Xh], length[%d], [err]: %s\n", __func__, __LINE__, fd_i2c, i2c_addr, buf[1], length, strerror(err));
					ret = (RET)eRET_NG_IO;
				}
			}
		}
	}

	return ret;
}

int32 read_i2c(uint8 i2c_addr, uint8 *reg, uint32 reglen, uint8 *data, uint32 datalen)
{
	struct i2c_rdwr_ioctl_data rdwr;
	struct i2c_msg msg[2];
	int32 ret = (RET)eRET_OK;
	int32 err;

	if((reg == NULL) || (data == NULL)) {
		I2C_ERR("[%s:%d]: Parameter is null\n", __func__, __LINE__);
		return (RET)eRET_NG_INVALID_PARAM;
	}

	if(dev_i2c_status != 1){
		I2C_ERR("[%s:%d]: I2C device not opened, fd[%i]: %s\n", __func__, __LINE__, fd_i2c, I2C_DEV);
		ret = (RET)eRET_NG_NOT_OPEN;
	}
	else {
		msg[ 0 ].addr  = (uint16)i2c_addr >> 1U;
		msg[ 0 ].flags = 0;
		msg[ 0 ].len   = (uint16)reglen;
		msg[ 0 ].buf   = reg;

		msg[ 1 ].addr  = (uint16)i2c_addr >> 1U;
		msg[ 1 ].flags = I2C_M_RD;
		msg[ 1 ].len   = (uint16)datalen ;
		msg[ 1 ].buf   = data;

		rdwr.msgs = msg;
		rdwr.nmsgs = 2;

		if(reglen > MAX_I2C_BURST){
			I2C_ERR("[%s:%d]: Invalid length, reg[%02Xh], reglen[%d]\n", __func__, __LINE__, *reg, reglen);
			ret = (RET)eRET_NG_INVALID_PARAM;
		}
		else {
			err = ioctl(fd_i2c, I2C_RDWR, &rdwr);
			if(err < 0){
				perror("");
				I2C_ERR("[%s:%d]: Failed to read I2C, [err]: %s\n", __func__, __LINE__, strerror(err));
				ret = (RET)eRET_NG_IO;
			}
		}
	}

	return ret;
}

