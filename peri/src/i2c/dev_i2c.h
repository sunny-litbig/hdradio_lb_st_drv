/*******************************************************************************

*   FileName : dev_i2c.h

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device I2C header

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
#ifndef DEV_I2C_H__
#define DEV_I2C_H__

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

//#define I2C_DEBUG

#include "DMBLog.h"

#ifdef __ANDROID__

#define I2C_TAG				("[RADIO][I2C]")
#define I2C_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,I2C_TAG, __VA_ARGS__))
#ifdef I2C_DEBUG
#define I2C_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,I2C_TAG, __VA_ARGS__))
#else
#define	I2C_DBG(...)
#endif

#else // #ifdef __ANDROID__

#define I2C_ERR(...)		((void)LB_PRINTF("[ERROR][RADIO][I2C]: " __VA_ARGS__))
#ifdef I2C_DEBUG
#define I2C_DBG(...)		((void)LB_PRINTF("[DEBUG][RADIO][I2C]: " __VA_ARGS__))
#else
#define	I2C_DBG(...)
#endif
// #define I2C_ERR(...)		((void)printf("[ERROR][RADIO][I2C]: " __VA_ARGS__))
// #ifdef I2C_DEBUG
// #define I2C_DBG(...)		((void)printf("[DEBUG][RADIO][I2C]: " __VA_ARGS__))
// #else
// #define	I2C_DBG(...)
// #endif

#endif // #ifdef __ANDROID__

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
extern int32 dev_i2c_open(void);
extern int32 dev_i2c_close(void);
extern int32 dev_i2c_read8(uint8 dev_addr, uint8 reg_addr, uint8 *data, uint32 data_len);
extern int32 dev_i2c_write8(uint8 dev_addr, uint8 reg_addr, uint8 *data, uint32 data_len);
extern int32 write_i2c(uint8 i2c_addr, uint8 *data, uint32 datalen);
extern int32 read_i2c(uint8 i2c_addr, uint8 *reg, uint32 reglen, uint8 *data, uint32 datalen);
#ifdef __cplusplus
}
#endif

#endif /* DEV_I2C_H__ */
