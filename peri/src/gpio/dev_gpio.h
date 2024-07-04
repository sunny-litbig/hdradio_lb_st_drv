/*******************************************************************************

*   FileName : dev_gpio.h

*   Copyright (c) Telechips Inc.

*   Description : Peripheral device GPIO header

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
#ifndef DEV_GPIO_H__
#define DEV_GPIO_H__

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

//#define GPIO_DEBUG    <- enable this code to print debugging messages

#ifdef __ANDROID__

#define	USE_RADIO_GPIO_DRV
#define GPIO_TAG			("[RADIO][GPIO]")
#define GPIO_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,GPIO_TAG, __VA_ARGS__))
#ifdef GPIO_DEBUG
#define GPIO_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,GPIO_TAG, __VA_ARGS__))
#else
#define	GPIO_DBG(...)
#endif

#else /* #ifdef __ANDROID__ */

#define	USE_RADIO_GPIO_DRV
#define GPIO_ERR(...)		((void)printf("[ERROR][RADIO][GPIO]: " __VA_ARGS__))
#ifdef GPIO_DEBUG
#define GPIO_DBG(...)		((void)printf("[DEBUG][RADIO][GPIO]: " __VA_ARGS__))
#else
#define	GPIO_DBG(...)
#endif

#endif /* #ifdef __ANDROID__ */

#ifndef USE_RADIO_GPIO_DRV
#define GPIO_HIGH ("1")
#define GPIO_LOW  ("0")

#define  GPIO_EXPORT_PATH 			"/sys/class/gpio/export"

/* tcc8971 lcn2.0 */
#define GPIO_E20_EXPORT_VAL		"148"
#define GPIO_E20_DIR					"/sys/class/gpio/gpio148"
#define GPIO_E20_VAL					"/sys/class/gpio/gpio148/value"
#define GPIO_E20_DIRECTION 			"/sys/class/gpio/gpio148/direction"

#define GPIO_E22_EXPORT_VAL		"150"
#define GPIO_E22_DIR					"/sys/class/gpio/gpio150"
#define GPIO_E22_VAL					"/sys/class/gpio/gpio150/value"
#define GPIO_E22_DIRECTION 			"/sys/class/gpio/gpio150/direction"

#define GPIO_E29_EXPORT_VAL		"157"
#define GPIO_E29_DIR					"/sys/class/gpio/gpio157"
#define GPIO_E29_VAL					"/sys/class/gpio/gpio157/value"
#define GPIO_E29_DIRECTION 			"/sys/class/gpio/gpio157/direction"

/* tcc8971 lcn3.0 */
#define GPIO_G06_EXPORT_VAL		"198"
#define GPIO_G06_DIR					"/sys/class/gpio/gpio198"
#define GPIO_G06_VAL					"/sys/class/gpio/gpio198/value"
#define GPIO_G06_DIRECTION 			"/sys/class/gpio/gpio198/direction"

#define GPIO_A01_EXPORT_VAL		"1"
#define GPIO_A01_DIR					"/sys/class/gpio/gpio1"
#define GPIO_A01_VAL					"/sys/class/gpio/gpio1/value"
#define GPIO_A01_DIRECTION 			"/sys/class/gpio/gpio1/direction"

/* tcc8021 evm sv1.0 */
#define GPIO_B08_EXPORT_VAL		"25"	// Use GFB number for TCC802X
#define GPIO_B08_DIR					"/sys/class/gpio/gpio25"
#define GPIO_B08_VAL					"/sys/class/gpio/gpio25/value"
#define GPIO_B08_DIRECTION 			"/sys/class/gpio/gpio25/direction"

#define GPIO_K15_EXPORT_VAL		"210"	// Use GFB number for TCC802X
#define GPIO_K15_DIR					"/sys/class/gpio/gpio210"
#define GPIO_K15_VAL					"/sys/class/gpio/gpio210/value"
#define GPIO_K15_DIRECTION 			"/sys/class/gpio/gpio210/direction"

/* tcc8021 evm sv2.0 */
#define GPIO_G09_EXPORT_VAL		"172"	// Use GFB number for TCC802X
#define GPIO_G09_DIR					"/sys/class/gpio/gpio172"
#define GPIO_G09_VAL					"/sys/class/gpio/gpio172/value"
#define GPIO_G09_DIRECTION 			"/sys/class/gpio/gpio172/direction"

#define GPIO_G10_EXPORT_VAL		"173"	// Use GFB number for TCC802X
#define GPIO_G10_DIR					"/sys/class/gpio/gpio173"
#define GPIO_G10_VAL					"/sys/class/gpio/gpio173/value"
#define GPIO_G10_DIRECTION 			"/sys/class/gpio/gpio173/direction"

#define GPIO_G15_EXPORT_VAL		"178"	// Use GFB number for TCC802X
#define GPIO_G15_DIR					"/sys/class/gpio/gpio180"
#define GPIO_G15_VAL					"/sys/class/gpio/gpio180/value"
#define GPIO_G15_DIRECTION 			"/sys/class/gpio/gpio180/direction"

#define GPIO_G17_EXPORT_VAL		"180"	// Use GFB number for TCC802X
#define GPIO_G17_DIR					"/sys/class/gpio/gpio180"
#define GPIO_G17_VAL					"/sys/class/gpio/gpio180/value"
#define GPIO_G17_DIRECTION 			"/sys/class/gpio/gpio180/direction"

#if defined(TCC897X_LCN20_BOARD)
#define TUNRST_EXPORT_VAL			GPIO_E29_EXPORT_VAL
#define TUNRST_CTRL_DIR				GPIO_E29_DIR
#define TUNRST_CTRL_VAL				GPIO_E29_VAL
#define TUNRST_CTRL_DIRECTION 		GPIO_E29_DIRECTION

#define TUNPWR_EXPORT_VAL			GPIO_E20_EXPORT_VAL
#define TUNPWR_CTRL_DIR				GPIO_E20_DIR
#define TUNPWR_CTRL_VAL				GPIO_E20_VAL
#define TUNPWR_CTRL_DIRECTION 		GPIO_E20_DIRECTION
#elif defined(TCC897X_LCN30_BOARD)
#define TUNRST_EXPORT_VAL			GPIO_G06_EXPORT_VAL
#define TUNRST_CTRL_DIR				GPIO_G06_DIR
#define TUNRST_CTRL_VAL				GPIO_G06_VAL
#define TUNRST_CTRL_DIRECTION 		GPIO_G06_DIRECTION

#define TUNPWR_EXPORT_VAL			GPIO_A01_EXPORT_VAL	// not used
#define TUNPWR_CTRL_DIR				GPIO_A01_DIR
#define TUNPWR_CTRL_VAL				GPIO_A01_VAL
#define TUNPWR_CTRL_DIRECTION 		GPIO_A01_DIRECTION
#elif defined(TCC802X_BOARD)
#define TUNRST_EXPORT_VAL			GPIO_B08_EXPORT_VAL
#define TUNRST_CTRL_DIR				GPIO_B08_DIR
#define TUNRST_CTRL_VAL				GPIO_B08_VAL
#define TUNRST_CTRL_DIRECTION 		GPIO_B08_DIRECTION

#define TUNPWR_EXPORT_VAL			GPIO_K15_EXPORT_VAL
#define TUNPWR_CTRL_DIR				GPIO_K15_DIR
#define TUNPWR_CTRL_VAL				GPIO_K15_VAL
#define TUNPWR_CTRL_DIRECTION 		GPIO_K15_DIRECTION
#elif defined(TCC802X_EVM21_BOARD)
#define TUNRST_EXPORT_VAL			GPIO_G09_EXPORT_VAL
#define TUNRST_CTRL_DIR				GPIO_G09_DIR
#define TUNRST_CTRL_VAL				GPIO_G09_VAL
#define TUNRST_CTRL_DIRECTION 		GPIO_G09_DIRECTION

#define TUNPWR_EXPORT_VAL			GPIO_G17_EXPORT_VAL
#define TUNPWR_CTRL_DIR				GPIO_G17_DIR
#define TUNPWR_CTRL_VAL				GPIO_G17_VAL
#define TUNPWR_CTRL_DIRECTION 		GPIO_G17_DIRECTION

#define LTUNPWR_EXPORT_VAL			GPIO_G10_EXPORT_VAL
#define LTUNPWR_CTRL_DIR			GPIO_G10_DIR
#define LTUNPWR_CTRL_VAL			GPIO_G10_VAL
#define LTUNPWR_CTRL_DIRECTION 		GPIO_G10_DIRECTION

#define TUNSEL_EXPORT_VAL			GPIO_G15_EXPORT_VAL
#define TUNSEL_CTRL_DIR				GPIO_G15_DIR
#define TUNSEL_CTRL_VAL				GPIO_G15_VAL
#define TUNSEL_CTRL_DIRECTION 		GPIO_G15_DIRECTION
#elif defined(TCC8030_BOARD) || defined(TCC8031_BOARD) || defined(TCC8031P_MAIN_BOARD) || defined(TCC8034P_MAIN_BOARD)
#define GPIO_G00_EXPORT_VAL		"165"
#define GPIO_G00_DIR					"/sys/class/gpio/gpio165"
#define GPIO_G00_VAL					"/sys/class/gpio/gpio165/value"
#define GPIO_G00_DIRECTION 			"/sys/class/gpio/gpio165/direction"

#define GPIO_MA00_EXPORT_VAL	"207"
#define GPIO_MA00_DIR					"/sys/class/gpio/gpio207"
#define GPIO_MA00_VAL					"/sys/class/gpio/gpio207/value"
#define GPIO_MA00_DIRECTION 		"/sys/class/gpio/gpio207/direction"

#define GPIO_MA18_EXPORT_VAL	"225"
#define GPIO_MA18_DIR					"/sys/class/gpio/gpio225"
#define GPIO_MA18_VAL					"/sys/class/gpio/gpio225/value"
#define GPIO_MA18_DIRECTION 		"/sys/class/gpio/gpio225/direction"

#define TUNRST_EXPORT_VAL			GPIO_G00_EXPORT_VAL
#define TUNRST_CTRL_DIR				GPIO_G00_DIR
#define TUNRST_CTRL_VAL				GPIO_G00_VAL
#define TUNRST_CTRL_DIRECTION 		GPIO_G00_DIRECTION

#define TUNPWR_EXPORT_VAL			GPIO_MA00_EXPORT_VAL
#define TUNPWR_CTRL_DIR				GPIO_MA00_DIR
#define TUNPWR_CTRL_VAL				GPIO_MA00_VAL
#define TUNPWR_CTRL_DIRECTION 		GPIO_MA00_DIRECTION

#define TUNINT1_EXPORT_VAL			GPIO_MA18_EXPORT_VAL
#define TUNINT1_CTRL_DIR			GPIO_MA18_DIR
#define TUNINT1_CTRL_VAL			GPIO_MA18_VAL
#define TUNINT1_CTRL_DIRECTION 		GPIO_MA18_DIRECTION
#elif defined(TCC8059_MAIN_BOARD) || defined(TCC8059_SUB_BOARD)
#define GPIO_G00_EXPORT_VAL		"111"
#define GPIO_G00_DIR					"/sys/class/gpio/gpio111"
#define GPIO_G00_VAL					"/sys/class/gpio/gpio111/value"
#define GPIO_G00_DIRECTION 			"/sys/class/gpio/gpio111/direction"

#define GPIO_MA00_EXPORT_VAL	"153"
#define GPIO_MA00_DIR					"/sys/class/gpio/gpio153"
#define GPIO_MA00_VAL					"/sys/class/gpio/gpio153/value"
#define GPIO_MA00_DIRECTION 		"/sys/class/gpio/gpio153/direction"

#define GPIO_MA18_EXPORT_VAL	"171"
#define GPIO_MA18_DIR					"/sys/class/gpio/gpio171"
#define GPIO_MA18_VAL					"/sys/class/gpio/gpio171/value"
#define GPIO_MA18_DIRECTION 		"/sys/class/gpio/gpio171/direction"

#define TUNRST_EXPORT_VAL			GPIO_G00_EXPORT_VAL
#define TUNRST_CTRL_DIR				GPIO_G00_DIR
#define TUNRST_CTRL_VAL				GPIO_G00_VAL
#define TUNRST_CTRL_DIRECTION 		GPIO_G00_DIRECTION

#define TUNPWR_EXPORT_VAL			GPIO_MA00_EXPORT_VAL
#define TUNPWR_CTRL_DIR				GPIO_MA00_DIR
#define TUNPWR_CTRL_VAL				GPIO_MA00_VAL
#define TUNPWR_CTRL_DIRECTION 		GPIO_MA00_DIRECTION

#define TUNINT1_EXPORT_VAL			GPIO_MA18_EXPORT_VAL
#define TUNINT1_CTRL_DIR			GPIO_MA18_DIR
#define TUNINT1_CTRL_VAL			GPIO_MA18_VAL
#define TUNINT1_CTRL_DIRECTION 		GPIO_MA18_DIRECTION
#elif defined(TCC8050_MAIN_BOARD) || defined(TCC8050_SUB_BOARD) || defined(TCC8053_MAIN_BOARD) || defined(TCC8053_SUB_BOARD)
#define GPIO_G00_EXPORT_VAL		"111"
#define GPIO_G00_DIR					"/sys/class/gpio/gpio111"
#define GPIO_G00_VAL					"/sys/class/gpio/gpio111/value"
#define GPIO_G00_DIRECTION 			"/sys/class/gpio/gpio111/direction"

#define GPIO_MB04_EXPORT_VAL	"187"
#define GPIO_MB04_DIR					"/sys/class/gpio/gpio187"
#define GPIO_MB04_VAL					"/sys/class/gpio/gpio187/value"
#define GPIO_MB04_DIRECTION 		"/sys/class/gpio/gpio187/direction"

#define GPIO_MB22_EXPORT_VAL	"205"
#define GPIO_MB22_DIR					"/sys/class/gpio/gpio205"
#define GPIO_MB22_VAL					"/sys/class/gpio/gpio205/value"
#define GPIO_MB22_DIRECTION 		"/sys/class/gpio/gpio205/direction"

#define TUNRST_EXPORT_VAL			GPIO_G00_EXPORT_VAL
#define TUNRST_CTRL_DIR				GPIO_G00_DIR
#define TUNRST_CTRL_VAL				GPIO_G00_VAL
#define TUNRST_CTRL_DIRECTION 		GPIO_G00_DIRECTION

#define TUNPWR_EXPORT_VAL			GPIO_MB04_EXPORT_VAL
#define TUNPWR_CTRL_DIR				GPIO_MB04_DIR
#define TUNPWR_CTRL_VAL				GPIO_MB04_VAL
#define TUNPWR_CTRL_DIRECTION 		GPIO_MB04_DIRECTION

#define TUNINT1_EXPORT_VAL			GPIO_MB22_EXPORT_VAL
#define TUNINT1_CTRL_DIR			GPIO_MB22_DIR
#define TUNINT1_CTRL_VAL			GPIO_MB22_VAL
#define TUNINT1_CTRL_DIRECTION 		GPIO_MB22_DIRECTION
#else
	#error This TARGET BOARD is not supported. (Available Board : TCC897XLCN20(A) or TCC802X or TCC802X_EVM21 or TCC803X or TCC805x)
#endif

#define ANTPWR_EXPORT_VAL			GPIO_E22_EXPORT_VAL
#define ANTPWR_CTRL_DIR				GPIO_E22_DIR
#define ANTPWR_CTRL_VAL				GPIO_E22_VAL
#define ANTPWR_CTRL_DIRECTION 		GPIO_E22_DIRECTION

#endif	/* #ifndef USE_RADIO_GPIO_DRV */
/***************************************************
*				Enumeration				*
****************************************************/

/***************************************************
*				Typedefs					*
****************************************************/
#ifdef USE_RADIO_GPIO_DRV
typedef enum
{
	BOARD_DXB_UNDEFINED=0, /* don't not modify or remove this line. when add new device, add to below of this line */
	BOARD_DXB_TCC3171=15,
	BOARD_AMFM_TUNER=30,
	BOARD_MAX
}DXB_BOARD_TYPE;

#define DXB_CTRL_DEV_FILE           "/dev/tcc_dxb_ctrl"
#define DXB_CTRL_IOCTL_BASE         251
#define IOCTL_DXB_CTRL_OFF          _IO(DXB_CTRL_IOCTL_BASE, 1)
#define IOCTL_DXB_CTRL_ON           _IO(DXB_CTRL_IOCTL_BASE, 2)
#define IOCTL_DXB_CTRL_RESET        _IO(DXB_CTRL_IOCTL_BASE, 3)
#define IOCTL_DXB_CTRL_SET_BOARD    _IO(DXB_CTRL_IOCTL_BASE, 4)
#define IOCTL_DXB_CTRL_GET_CTLINFO  _IO(DXB_CTRL_IOCTL_BASE, 5)
#define IOCTL_DXB_CTRL_RF_PATH      _IO(DXB_CTRL_IOCTL_BASE, 6)
#define IOCTL_DXB_CTRL_SET_CTRLMODE _IO(DXB_CTRL_IOCTL_BASE, 7)
#define IOCTL_DXB_CTRL_RESET_LOW    _IO(DXB_CTRL_IOCTL_BASE, 8)
#define IOCTL_DXB_CTRL_RESET_HIGH   _IO(DXB_CTRL_IOCTL_BASE, 9)
#define IOCTL_DXB_CTRL_PURE_ON      _IO(DXB_CTRL_IOCTL_BASE, 10)
#define IOCTL_DXB_CTRL_PURE_OFF     _IO(DXB_CTRL_IOCTL_BASE, 11)
#endif

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern int32 dev_rgpio_open(void);
extern int32 dev_rgpio_close(void);
extern int32 setTunerPower(int32 onoff);
extern int32 setAntPower(int32 onoff);
extern int32 setTunerReset(int32 onoff);
extern int32 setTunerSelect(int32 sel);

extern int32 setTestGpio1(int32 onoff);			// for test
#ifdef __cplusplus
}
#endif

#endif
