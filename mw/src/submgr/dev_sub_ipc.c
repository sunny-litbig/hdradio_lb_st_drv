/*******************************************************************************

*   FileName : tcradio_sub_ipc.c

*   Copyright (c) Telechips Inc.

*   Description : C file standard form

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
*        Include                                   *
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

#include "dev_sub_ipc.h"

/***************************************************
*        Global variable definitions               *
****************************************************/

/***************************************************
*        Imported variable declarations            *
****************************************************/

/***************************************************
*        Imported function declartions             *
****************************************************/

/***************************************************
*        Local preprocessor                        *
****************************************************/
#define SDR_IPC_SUB_DEV			"/dev/tcc_sdr_ipc"
#define	SDR_IPC_HDR_ID			(0x01)
#define	SDR_IPC_DRM_ID			(0x02)
#define	SDR_IPC_DAB_ID			(0x03)

/***************************************************
*        Local type definitions                    *
****************************************************/

/***************************************************
*        Local constant definitions                *
****************************************************/
static int32_t ipc_dev_fd = -1;
static struct pollfd poll_evt;

/***************************************************
*        Local function prototypes                 *
****************************************************/

/***************************************************
*        function definition                       *
****************************************************/
int32_t dev_ipcSdrSubOpen(void)
{
	int32_t ret = 0;

	if(ipc_dev_fd >= 0) {
		SRIPC_ERR("[%s:%d]: SDR IPC device is already opened.\n", __func__, __LINE__);
	}
	else {
		ipc_dev_fd = open(SDR_IPC_SUB_DEV, O_RDWR | O_NDELAY);
		if(ipc_dev_fd < 0) {
			SRIPC_ERR("[%s:%d]: Failed to open sdr ipc. %s!!!\n", __func__, __LINE__, strerror(errno));
			ret = (int32_t)(-1);
		}
		else {
			int32_t bcast_id = SDR_IPC_HDR_ID;
			ret = ioctl(ipc_dev_fd, IOCTL_SDRPIC_SET_ID, bcast_id);
			if(ret < 0) {
				SRIPC_ERR("[%s:%d]: Failed to set sdr ipc ID[%d]. %s!!!\n", __func__, __LINE__, bcast_id, strerror(errno));
				ret = (int32_t)(-1);
			}
		}
	}
	return ret;
}

int32_t dev_ipcSdrSubClose(void)
{
	int32_t ret = 0, i;

	if(ipc_dev_fd == -1) {
		SRIPC_ERR("[%s:%d]: SDR IPC Device is already closed\n", __func__, __LINE__);
		return (0);
	}

	close(ipc_dev_fd);
	ipc_dev_fd = -1;

	return 0;
}

int32_t dev_ipcSdrSubRx(int8_t *rxbuf, int32_t rxsize)		// rxsize unit : byte
{
	int32_t ret = 0;

	if(ipc_dev_fd < 0) {
		SRIPC_ERR("[%s:%d]: SDR IPC device is not opened!!\n", __func__, __LINE__);
		return(-1);
	}

	if(rxbuf == NULL) {
		SRIPC_ERR("[%s:%d]: Rx buffer is null!!!\n", __func__, __LINE__);
		return(-1);
	}

	if(rxsize > SDR_IPC_MAX_RX_SIZE) {
		SRIPC_ERR("[%s:%d]: Rx buffer size is over!!!\n", __func__, __LINE__);
		return(-1);
	}

	memset(&poll_evt, 0, sizeof(poll_evt));
	poll_evt.fd = ipc_dev_fd;
	poll_evt.events = POLLIN;
	ret = poll(&poll_evt, 1u, 1000);
	if(ret > 0) {
		if(poll_evt.revents & POLLIN) {
			ret = read(ipc_dev_fd, rxbuf, rxsize);
		}
		else if(poll_evt.revents & POLLERR) {
			SRIPC_ERR("[%s:%d]: IPC device error!!\n", __func__, __LINE__);
			ret = -1;
		}
		else if(poll_evt.revents & POLLHUP) {
			SRIPC_ERR("[%s:%d]: IPC device is disconnected!!\n", __func__, __LINE__);
			ret = -1;
		}
		else if(poll_evt.revents & POLLNVAL) {
			SRIPC_ERR("[%s:%d]: IPC device invalid request error!!\n", __func__, __LINE__);
			ret = -1;
		}
		else {
			SRIPC_ERR("[%s:%d]: IPC device unknown return value[%d]!!\n", __func__, __LINE__, poll_evt.revents);
			ret = -1;
		}
	}
	else if(ret == 0) {
	//	SRIPC_DBG("[%s:%d]: Poll timeout!!! %s\n", __func__, __LINE__, strerror(errno));
	}
	else {
		SRIPC_ERR("[%s:%d]: Poll error!!! %s\n", __func__, __LINE__, strerror(errno));
	}

	return ret;
}

int32_t dev_ipcSdrSubTx(int8_t *txbuf, int32_t txsize)		// txsize unit : byte
{
	int32_t ret = 0;

	if(ipc_dev_fd < 0) {
		SRIPC_ERR("[%s:%d]: SDR IPC device is not opened!!\n", __func__, __LINE__);
		return(-1);
	}

	if(txbuf == NULL) {
		SRIPC_ERR("[%s:%d]: Tx buffer is null!!!\n", __func__, __LINE__);
		return(-1);
	}

	if(txsize > SDR_IPC_MAX_TX_SIZE) {
		SRIPC_ERR("[%s:%d]: Tx buffer size is over!!!\n", __func__, __LINE__);
		return(-1);
	}

	ret = write(ipc_dev_fd, txbuf, txsize);

	if(ret < 0) {
		SRIPC_ERR("[%s:%d]: SDR IPC write error!!! %s\n", __func__, __LINE__, strerror(errno));
	}

	return ret;
}

