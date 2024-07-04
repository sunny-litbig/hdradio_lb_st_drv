/*******************************************************************************

*   FileName : dev_spi.h

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device SPI header

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
#ifndef DEV_SPI_H__
#define DEV_SPI_H__

/***************************************************
*				Include					*
****************************************************/
#ifdef __ANDROID__
#include <android/log.h>
#endif

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

//#define SPI_DEBUG  -> please enable this code to print debug messages

#ifdef __ANDROID__

#define SPI_TAG				("[RADIO][SPI]")
#define SPI_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,SPI_TAG, __VA_ARGS__))
#ifdef SPI_DEBUG
#define SPI_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,SPI_TAG, __VA_ARGS__))
#else
#define	SPI_DBG(...)
#endif

#else /* #ifdef __ANDROID__ */

#define SPI_ERR(...)		((void)printf("[ERROR][RADIO][SPI]: " __VA_ARGS__))
#ifdef SPI_DEBUG
#define SPI_DBG(...)		((void)printf("[DEBUG][RADIO][SPI]: " __VA_ARGS__))
#else
#define	SPI_DBG(...)
#endif

#endif /* #ifdef __ANDROID__ */

#define TCCSPI_SPEED	(4000U * 1000U)	//(20000 * 1000)
#define cmd_buffer_size	(512U)
#define MaxSpiCh		(2U)

/***************************************************
*				Enumeration				*
****************************************************/

/***************************************************
*				Typedefs					*
****************************************************/

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern int32 dev_spi_open(uint8 ch);
extern int32 dev_spi_close(uint8 ch);
extern int32 dev_spi_txrx (uint8 * pBufIn, uint8 * pBufOut, uint32 size, uint8 ch);

#ifdef __cplusplus
}
#endif

#endif
