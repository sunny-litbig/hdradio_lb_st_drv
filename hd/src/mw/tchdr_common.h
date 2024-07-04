/*******************************************************************************

*   FileName : tchdr_common.h

*   Copyright (c) Telechips Inc.

*   Description : TC HD Radio framework common header

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
#ifndef TCHDR_COMMON_H__
#define TCHDR_COMMON_H__

/***************************************************
*				Include					*
****************************************************/
#include <stdio.h>
#include <unistd.h>

#include "tchdr_systypes.h"

#include "hdrBasicTypes.h"
#include "hdrCore.h"

#include "tchdr_std.h"

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define	HDR_ERR_BUF_SIZE		(128)

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
extern stThread_attr_t stTcHdrManagerThdAttr;
extern stThread_attr_t stBbInputThdAttr;
extern stThread_attr_t stPrimaryHdThdAttr;
extern stThread_attr_t stMrcHdThdAttr;
extern stThread_attr_t stBackscanHdThdAttr;
extern stThread_attr_t stAudioPlaybackAttr;
extern stThread_attr_t stAudioBlendingThdAttr;
extern stThread_attr_t stAudioInputAttr;
extern stThread_attr_t stRfIqInputAttr;
extern stThread_attr_t stCmdProcThdAttr;
extern stThread_attr_t stLoggerThdAttr;
extern stThread_attr_t stTraceThdAttr;

/***************************************************
*			Function declaration				*
****************************************************/
extern HDRET tchdr_convertHdrError(HDR_error_code_t error);
extern HDRET tchdr_getHDRadioOpenStatus(void);

#ifdef __cplusplus
}
#endif

#endif
