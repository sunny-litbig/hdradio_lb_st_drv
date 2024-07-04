/*******************************************************************************

*   FileName : tcradio_callback.c

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
#include "tcradio_api.h"

pfnOnGetNotificationCallBack_t pfnOnGetNotificationCallBack;
pfnOnGetStationListCallBack_t pfnOnGetStationListCallBack;
pfnOnGetRdsDataCallBack_t pfnOnGetRdsDataCallBack;
pfnOnPrecheckSeekQual_t pfnOnPrecheckSeekQual;
pfnOnCheckSeekQual_t pfnOnCheckSeekQual;

/*==================================================
		Callback Function Configuration
====================================================*/
void tcradio_configOnGetNotificationCallBack(void(*pfnGetNotificationCallBack)(uint32 notifyID, uint32 *arg, void **pData, int32 errorCode))
{
	pfnOnGetNotificationCallBack = pfnGetNotificationCallBack;
}
void tcradio_configOnGetStationListCallBack(void(*pfnGetStationListCallBack)(uint32 totalnum, void *list, int32 errorCode))
{
	pfnOnGetStationListCallBack = pfnGetStationListCallBack;
}
void tcradio_configOnGetRdsDataCallBack(void(*pfnGetRdsDataCallBack)(void *rdsData, int32 errorCode))
{
	pfnOnGetRdsDataCallBack = pfnGetRdsDataCallBack;
}

void tcradio_configOnPrecheckSeekQual(int32(*pfnPrecheckSeekQual)(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata))
{
	pfnOnPrecheckSeekQual = pfnPrecheckSeekQual;
}

void tcradio_configOnCheckSeekQual(int32(*pfnCheckSeekQual)(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata))
{
	pfnOnCheckSeekQual = pfnCheckSeekQual;
}