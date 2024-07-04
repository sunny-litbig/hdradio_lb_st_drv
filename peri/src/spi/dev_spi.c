/*******************************************************************************

*   FileName : dev_spi.c

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device SPI functions and definitions

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
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <linux/spi/spidev.h>

#include "tcradio_types.h"
#include "dev_spi.h"

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
#define SPI0_DEV	"/dev/spidev0.0"
#define SPI1_DEV	"/dev/spidev0.1"

/***************************************************
*           Local constant definitions              *
****************************************************/
static int32 ghSPI[MaxSpiCh] = {-1, -1};

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition				*
****************************************************/
int32 dev_spi_open(uint8 ch)
{
	uint8 spi_mode = 0;
	uint32 spi_max_clock;
	int32 fd;

	if( ch >= MaxSpiCh ) {
		SPI_ERR("[%s:%d]: Tccspi channel invalid!!, ch[%d]\n", __func__, __LINE__, ch);
		return (RET)eRET_NG_INVALID_PARAM;
	}

	if (ch == 0U) {
		if(ghSPI[ch] < 0) {
			fd = open(SPI0_DEV, O_RDWR | O_NDELAY);
		 	ghSPI[ch] = fd;
		}
		else {
			SPI_ERR("[%s:%d]: Tccspi channel already opened!!, ch[%d]\n", __func__, __LINE__, ch);
			return (RET)eRET_OK;
		}
	}
	else {
		if(ghSPI[ch] < 0) {
			fd = open(SPI1_DEV, O_RDWR | O_NDELAY);
		 	ghSPI[ch] = fd;
		}
		else {
			SPI_ERR("[%s:%d]: Tccspi channel already opened!!, ch[%d]\n", __func__, __LINE__, ch);
			return (RET)eRET_OK;
		}
	}

	if (ghSPI[ch] < 0)
	{
		SPI_ERR("[%s:%d]: Invalid handle value!!, ch[%d], ghSPI[%d], [err]: %s\n", __func__, __LINE__, ch, ghSPI[ch], strerror(errno));
		return (RET)eRET_NG_NO_FILE;
	}
	else
	{
		//SET SPI ch using IOCTL
		(void)ioctl(ghSPI[ch], SPI_IOC_RD_MODE, &spi_mode);
		SPI_DBG("[%s:%d]: Tccspi ch[%d] RD clock mode[%d]\n", __func__, __LINE__, ch, spi_mode);
		spi_mode |= (uint8)SPI_MODE_3;
		(void)ioctl(ghSPI[ch], SPI_IOC_WR_MODE, &spi_mode);
		SPI_DBG("[%s:%d]: Tccspi ch[%d] WR clock mode[%d]\n", __func__, __LINE__, ch, spi_mode);

		spi_max_clock = TCCSPI_SPEED;
		(void)ioctl(ghSPI[ch], SPI_IOC_WR_MAX_SPEED_HZ, &spi_max_clock);
		(void)ioctl(ghSPI[ch], SPI_IOC_RD_MAX_SPEED_HZ, &spi_max_clock);
		SPI_DBG("[%s:%d]: Tccspi ch[%d] max clock[%d Hz]\n", __func__, __LINE__, ch, spi_max_clock);
	}
	return (RET)eRET_OK;
}

int32 dev_spi_close(uint8 ch)

{
	int32 dret;
	RET ret = (RET)eRET_OK;

	if( ch >= MaxSpiCh ) {
		SPI_ERR("[%s:%d]: Tccspi channel invalid!!, ch[%d]\n", __func__, __LINE__, ch);
		ret = (RET)eRET_NG_INVALID_PARAM;
	}
	else {
		if(ghSPI[ch] >= 0) {
			dret = close(ghSPI[ch]);
			ghSPI[ch] = -1;
			if(dret < 0) {
				SPI_ERR("[%s:%d]: Failed to close tccspi!!, ch[%d], ghSPI[%d], [err]: %s\n", __func__, __LINE__, ch, ghSPI[ch], strerror(errno));
				ret = (RET)eRET_NG_UNKNOWN;
			}
		}
		else {
			ret = (RET)eRET_NG_NOT_OPEN;
		}
	}
	return ret;
}

int32 dev_spi_txrx (uint8 * pBufIn, uint8 * pBufOut, uint32 size, uint8 ch)
{
	int32 dret;
	RET ret = (RET)eRET_OK;
	struct spi_ioc_transfer msg;

	(void)memset((void*)&msg, 0x00, sizeof(msg));
	msg.tx_buf = (__u64)pBufIn;
	msg.rx_buf = (__u64)pBufOut;
	msg.len = size;
	msg.speed_hz = TCCSPI_SPEED;
	msg.bits_per_word = 8;

	if( ch >= MaxSpiCh )
	{
		SPI_ERR("[%s:%d]: Tccspi channel invalid!!, ch[%d]\n", __func__, __LINE__, ch);
		ret = (RET)eRET_NG_INVALID_PARAM;
	}
	else {
		dret = ioctl(ghSPI[ch], SPI_IOC_MESSAGE(1), &msg);
		if (dret != (int32)size)
		{
			SPI_ERR("[%s:%d]: error: %s, ch[%d], size[%d]\n", __func__, __LINE__, strerror(errno), ch, size);
			ret = (RET)eRET_NG_IO;
		}
	}
	return ret;
}

