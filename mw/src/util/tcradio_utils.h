/*******************************************************************************

*   FileName : tcradio_utils.h

*   Copyright (c) Telechips Inc.

*   Description :

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
#ifndef __TCRADIO_UTILS_H__
#define __TCRADIO_UTILS_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define	tcradio_ussleep(X)		usleep(X)
#define tcradio_mssleep(X)		usleep((X)*(1000))

//#define USEANSICOLOR
//////////////////////////////////////////////////////////////////////////////////
/* Uart print coloer */
//////////////////////////////////////////////////////////////////////////////////

#ifdef USEANSICOLOR
/* COLOR BOLD: 1, NORMAL 0, any is background color (40~47)
40(black), 41(red), 42(green), 43(yellow), 44(blue), 45(magenta), 46(cyan), 47(white) */

#define COLOR_BOLD      "1"
#define COLOR_NORMAL    "0"

#define COLOR1  "\033["COLOR_NORMAL";30m" /* Dark black */
#define COLOR2  "\033["COLOR_NORMAL";31m" /* Dark Red */
#define COLOR3  "\033["COLOR_NORMAL";32m" /* Dark Green */
#define COLOR4  "\033["COLOR_NORMAL";33m" /* brown */
#define COLOR5  "\033["COLOR_NORMAL";34m" /* Dark Blue */
#define COLOR6  "\033["COLOR_NORMAL";35m" /* Dark Magenta */
#define COLOR7  "\033["COLOR_NORMAL";36m" /* Dark Cyan */
#define COLOR8  "\033["COLOR_NORMAL";37m" /* Light Gray */

#define COLOR9  "\033["COLOR_BOLD";30m" /* Gray */
#define COLOR10 "\033["COLOR_BOLD";31m" /* Red */
#define COLOR11 "\033["COLOR_BOLD";32m" /* Green */
#define COLOR12 "\033["COLOR_BOLD";33m" /* Yellow */
#define COLOR13 "\033["COLOR_BOLD";34m" /* Blue */
#define COLOR14 "\033["COLOR_BOLD";35m" /* Magenta */
#define COLOR15 "\033["COLOR_BOLD";36m" /* Cyan */
#define COLOR16 "\033["COLOR_BOLD";37m" /* White */

#define COLOREND "\033[0m"
#define COLORFIN "\033[0m\n"
#else
#define COLOR_BOLD ""
#define COLOR_NORMAL ""
#define COLOR1  ""
#define COLOR2  ""
#define COLOR3  ""
#define COLOR4  ""
#define COLOR5  ""
#define COLOR6  ""
#define COLOR7  ""
#define COLOR8  ""
#define COLOR9  ""
#define COLOR10 ""
#define COLOR11 ""
#define COLOR12 ""
#define COLOR13 ""
#define COLOR14 ""
#define COLOR15 ""
#define COLOR16 ""
#define COLOREND ""
#define COLORFIN "\n"
#endif

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
extern void tcradio_16bto8b(uint8 *buf, uint16 data);
extern void tcradio_32bto8b(uint8 *buf, uint32 data);
extern void *tcradio_memset(void *pdst, uint8 ch, uint32 len);
extern void *tcradio_memcpy(void *pdst, void *psrc, uint32 len);
extern int8 *tcradio_strcpy(int8 *pdst, int8 *psrc);
extern int8 *tcradio_strncpy(int8 *pdst, int8 *psrc, uint32 len);
extern uint32 tcradio_strlen(int8 *psbStr);
extern int32 tcradio_strcmp(int8 *psbStr1, int8 *psbStr2);
extern int32 tcradio_strncmp(int8 *psbStr1, int8 *psbStr2, uint32 len);
extern int8 *tcradio_strcat(int8 *psbStr1, int8 *psbStr2);
extern uint32 BS_Truncation(uint32 ulData, uint16 uiUnit);

#ifdef __cplusplus
}
#endif

#endif

