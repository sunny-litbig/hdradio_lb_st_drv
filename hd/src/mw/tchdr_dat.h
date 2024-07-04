/*******************************************************************************

*   FileName : tchdr_dat.h

*   Copyright (c) Telechips Inc.

*   Description : Dynamic Audio Tracking APIs and definitions

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
#ifndef TCHDR_DAT_H__
#define TCHDR_DAT_H__

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

/***************************************************
*               Typedefs                           *
****************************************************/
typedef struct {
	U32 ret;
	U32 state;
	U32 quality_score;
	S32 init_delay;
	F64 drift_ppm;
}stDAT_RESULT_t;

/***************************************************
*               Constant definitions               *
****************************************************/

/***************************************************
*               Variable definitions               *
****************************************************/

/***************************************************
*               Function declaration               *
****************************************************/
extern U32 tchdr_datInit(U32 slot);
extern stDAT_RESULT_t tchdr_datProc(U32 slot, S16 * _digital, S16 *  _analog, U32 _samples);
extern void tchdr_datReinit(U32 slot);
extern void tchdr_datDeinit(U32 slot);

#ifdef __cplusplus
}
#endif

#endif