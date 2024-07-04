/*******************************************************************************

*   FileName : tchdr_debug.h

*   Copyright (c) Telechips Inc.

*   Description : TC HD Radio framework debugging log APIs and definitions

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
#ifndef TCHDR_DEBUG_H__
#define TCHDR_DEBUG_H__

/***************************************************
*               Include                            *
****************************************************/

/***************************************************
*               Defines                            *
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
*               Enumeration                        *
****************************************************/
typedef enum {
	eLOG_ERR,
	eLOG_WRN,
	eLOG_INF,
	eLOG_DBG
}eLOG_TYPE_t;

typedef enum {
	eTAG_NOTAG		=0,		// None Module Tag
	eTAG_SYS		=1,		// Management + API
	eTAG_IQIN		=2,		// IQ Input
	eTAG_AIN		=3,		// Audio Input
	eTAG_CORE		=4,		// HD Radio Core Library
	eTAG_BBIN		=5,		// Base-Band Input
	eTAG_PRI		=6,		// Primary Demod Instance
	eTAG_MRC		=7,		// MRC Demod Instance
	eTAG_BS			=8,		// Background-scan Demod Instance
	eTAG_BSMRC		=9,		// BS MRC Demod Instance
	eTAG_BLD		=10,	// Audio Blending
	eTAG_AOUT		=11,	// Audio Output
	eTAG_CB			=12,	// Callback of the HD Radio Core, There are many logs with frequent output.
	eTAG_CDM		=13,	// Command Processor for CDM I/F
	eTAG_ETH		=14,	// Ethernet Socket for CDM I/F
	eTAG_XLOG		=15,	// XPERI Internal Logger
	eTAG_MAX
}eLOG_TAG_t;

/***************************************************
*               Typedefs                           *
****************************************************/
typedef struct {
	const S8 *tag_str;	// Tag String
	U8 enable;			// 0: Disable Log, 1: Enable Log
}stLOG_CONF_t;

/***************************************************
*               Constant definitions               *
****************************************************/

/***************************************************
*               Variable definitions               *
****************************************************/
extern void (*pfnHdrLog)(eLOG_TAG_t tag, eLOG_TYPE_t type, const S8 *format, ...);

/***************************************************
*               Function declaration               *
****************************************************/
extern HDRET tchdrlog_setLevel(eLOG_TYPE_t level);
extern eLOG_TYPE_t tchdrlog_getLevel(void);
extern void tchdrlog_setEnable(eLOG_TAG_t tag, U32 fOnOff);
extern void HDRLOG(eLOG_TAG_t tag, eLOG_TYPE_t type, const S8 *format, ...);

#ifdef __cplusplus
}
#endif

#endif
