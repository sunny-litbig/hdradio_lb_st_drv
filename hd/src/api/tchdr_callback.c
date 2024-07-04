/*******************************************************************************

*   FileName : tchdr_callback.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio Framework callback functions

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
*		Include 			   *
****************************************************/
#include "tchdr_api.h"
#include "tchdr_callback.h"
#ifdef USE_TELECHIPS_EVB
#include "tcradio_types.h"
#include "tcradio_hal.h"
#endif

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

/***************************************************
*           Local constant definitions              *
****************************************************/

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition				*
****************************************************/
S32 tchdr_cb_getIqSampleRate(U32 ntuner)
{
    S32 srRet = (S32)eTC_HDR_BBSRC_UNKNOWN;

#ifdef USE_TELECHIPS_EVB
    S32 iqSR = tcradiohal_getIqSampleRate(ntuner);                // Get I/Q sample rate from tuner

    if(iqSR == 744187) {
        srRet = (S32)eTC_HDR_BBSRC_744_KHZ;
    }
    else if(iqSR == 650000) {
        srRet = (S32)eTC_HDR_BBSRC_650_KHZ;
    }
    else if(iqSR == 675000) {
        srRet = (S32)eTC_HDR_BBSRC_675_KHZ;
    }
    else {
        srRet = (S32)eTC_HDR_BBSRC_744_KHZ;                      // default value
    }
#else    /* #ifdef USE_TELECHIPS_EVB */


        ////////////////
        /////
        /////  TODO                                              // Replace with your tuner API.
        /////
        ////////////////


#endif    /* #ifdef USE_TELECHIPS_EVB */

    return srRet;
}

S32 tchdr_cb_setTune(eTC_HDR_BAND_t band, U32 freq, U32 instance_number)	// freq: Khz
{
    S32 ret = -1;

#ifdef USE_TELECHIPS_EVB
    U32 ntuner = instance_number;
    ret = tcradiohal_setTune((U32)band, freq, 0, ntuner);        // Set tuner band and frequency
#else    /* #ifdef USE_TELECHIPS_EVB */


        ////////////////
        /////
        /////  TODO                                              // Replace with your tuner API.
        /////
        ////////////////


#endif    /* #ifdef USE_TELECHIPS_EVB */

    return ret;
}

