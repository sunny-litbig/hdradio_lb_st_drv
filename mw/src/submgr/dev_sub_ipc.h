/*******************************************************************************

*   FileName : tcradio_sub_ipc.h

*   Copyright (c) Telechips Inc.

*   Description : Header file standard form

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
#ifndef TCRADIO_SUB_IPC__H__
#define TCRADIO_SUB_IPC__H__

/***************************************************
*               Include                            *
****************************************************/
#ifdef __ANDROID__
#include <android/log.h>
#endif

/***************************************************
*               Defines                            *
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define SRIPC_DEBUG

#ifdef __ANDROID__

#define SIPC_TAG			("[RADIO][SUB-IPC]")
#define SRIPC_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,SIPC_TAG, __VA_ARGS__))
#ifdef SRIPC_DEBUG
#define SRIPC_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,SIPC_TAG, __VA_ARGS__))
#else
#define	SRIPC_DBG(...)
#endif

#else // #ifdef __ANDROID__

#define SRIPC_ERR(...)		((void)printf("[ERROR][RADIO][SUB-IPC]: " __VA_ARGS__))
#ifdef SRIPC_DEBUG
#define SRIPC_DBG(...)		((void)printf("[DEBUG][RADIO][SUB-IPC]: " __VA_ARGS__))
#else
#define SRIPC_DBG(...)
#endif

#endif // #ifdef __ANDROID__

#define SDR_IPC_MAX_RX_SIZE			(512*1024)
#define SDR_IPC_MAX_TX_SIZE			(512*1024)

// linux/tcc_sdr_ipc.h
#define SDRIPC_MAGIC        		('S')
#define IOCTL_SDRPIC_SET_ID     	_IO(SDRIPC_MAGIC, 0) /* range : 0 ~ 15 */

/***************************************************
*               Enumeration                        *
****************************************************/

/***************************************************
*               Typedefs                           *
****************************************************/

/***************************************************
*               Constant definitions               *
****************************************************/

/***************************************************
*               Variable definitions               *
****************************************************/

/***************************************************
*               Function declaration               *
****************************************************/
extern int32_t dev_ipcSdrSubOpen(void);
extern int32_t dev_ipcSdrSubClose(void);
extern int32_t dev_ipcSdrSubRx(int8_t *rxbuf, int32_t rxsize);
extern int32_t dev_ipcSdrSubTx(int8_t *txbuf, int32_t txsize);

#ifdef __cplusplus
}
#endif

#endif
