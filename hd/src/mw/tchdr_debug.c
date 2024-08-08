/*******************************************************************************

*   FileName : tchdr_debug.c

*   Copyright (c) Telechips Inc.

*   Description : Debugging functions and definitions

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
*        Include                                   *
****************************************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "tchdr_common.h"

#include "DMBLog.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
void (*pfnHdrLog)(eLOG_TAG_t tag, eLOG_TYPE_t type, const S8 *format, ...) = &HDRLOG;

/***************************************************
*        Imported variable declarations            *
****************************************************/
#ifdef DEBUG_ENABLE_HDR_LOG
static eLOG_TYPE_t eDebugLogLevel = eLOG_DBG;
#else
static eLOG_TYPE_t eDebugLogLevel = eLOG_INF;
#endif

/***************************************************
*        Imported function declartions             *
****************************************************/

/***************************************************
*        Local preprocessor                        *
****************************************************/
#ifdef HDR_SYSTEM_IS_ANDROID	// for android debugging
#define  HDRLOGTAG "[HDR][FRWK]"
#define  LOGV(...) (__android_log_print(ANDROID_LOG_VERBOSE,HDRLOGTAG, __VA_ARGS__))
#define  LOGD(...) (__android_log_print(ANDROID_LOG_DEBUG,HDRLOGTAG, __VA_ARGS__))
#define  LOGI(...) (__android_log_print(ANDROID_LOG_INFO,HDRLOGTAG, __VA_ARGS__))
#define  LOGW(...) (__android_log_print(ANDROID_LOG_WARN,HDRLOGTAG, __VA_ARGS__))
#define  LOGE(...) (__android_log_print(ANDROID_LOG_ERROR,HDRLOGTAG, __VA_ARGS__))
#else	// for linux or others debugging
// #define  LOGV(...) ((void)printf("[VBS][HDR]" __VA_ARGS__))
// #define  LOGD(...) ((void)printf("[DBG][HDR]" __VA_ARGS__))
// #define  LOGI(...) ((void)printf("[INF][HDR]" __VA_ARGS__))
// #define  LOGW(...) ((void)printf("[WRN][HDR]" __VA_ARGS__))
// #define  LOGE(...) ((void)printf("[ERR][HDR]" __VA_ARGS__))
#define  LOGV(...) ((void)LB_PRINTF("[VBS][HDR]" __VA_ARGS__))
#define  LOGD(...) ((void)LB_PRINTF("[DBG][HDR]" __VA_ARGS__))
#define  LOGI(...) ((void)LB_PRINTF("[INF][HDR]" __VA_ARGS__))
#define  LOGW(...) ((void)LB_PRINTF("[WRN][HDR]" __VA_ARGS__))
#define  LOGE(...) ((void)LB_PRINTF("[ERR][HDR]" __VA_ARGS__))
#endif

#define MAX_LOG_LENGTH		(1024U)

/***************************************************
*        Local type definitions                    *
****************************************************/

/***************************************************
*        Local constant definitions                *
****************************************************/
static stLOG_CONF_t stLogConf[eTAG_MAX] = {
	{"", 1},		// None Module Tag
	{"SYS", 1},		// Manage
	{"IQIN", 1},	// IQ Input
	{"AIN", 1},		// Audio Input
	{"CORE", 1},	// HD Radio Core Library
	{"BBIN", 1},	// BaseBand Input
	{"PRI", 1},		// Primary Instance
	{"MRC", 1},		// MRC Instance
	{"BS", 1},		// Background Scan Instance
	{"BLD", 1},		// Audio Blending
	{"AOUT", 1},	// Audio Output
	{"CB", 0},		// Callback of the HD Radio Core, There are many logs with frequent output.
	{"CDM", 1},		// Command Processor for CDM I/F
	{"ETH", 1},		// Ethernet Socket for CDM I/F
	{"XLOG", 1}		// XPERI Internal Logger
};

/***************************************************
*        Local function prototypes                 *
****************************************************/

/***************************************************
*        function definition                       *
****************************************************/
HDRET tchdrlog_setLevel(eLOG_TYPE_t level)
{
	HDRET ret = (HDRET)eTC_HDR_RET_OK;

	if((level == eLOG_ERR) || (level == eLOG_WRN) || (level == eLOG_INF) ||(level == eLOG_DBG)) {
		eDebugLogLevel = level;
	}
	else {
		ret = (HDRET)eTC_HDR_RET_NG_INVALID_PARAMETERS;
	}

	return ret;
}

eLOG_TYPE_t tchdrlog_getLevel(void)
{
	return eDebugLogLevel;
}

void tchdrlog_setEnable(eLOG_TAG_t tag, U32 fOnOff)
{
	if(tag <= eTAG_XLOG) {
		if(fOnOff > 0U) {
			stLogConf[tag].enable = 1U;
		}
		else {
			stLogConf[tag].enable = 0U;
		}
	}
}

void HDRLOG(eLOG_TAG_t tag, eLOG_TYPE_t type, const S8 *format, ...)
{
#ifndef AVOID_MC2012_RULE_21P6_USING_PRINTF
	if(((U8)tag <= (U8)eTAG_XLOG) && (stLogConf[tag].enable != 0U) && (type <= eDebugLogLevel)) {
		// Coverity does not recognize S8 as char.
		char strLog[MAX_LOG_LENGTH] = {0,};
		const char *strModule = stLogConf[tag].tag_str;
		va_list arg;

		va_start(arg, format);
		(void)vsprintf(strLog, format, arg);
		va_end(arg);

		if(strModule[0] != '\0') {
			switch(type) {
				case eLOG_ERR:
					LOGE("[%s] %s", strModule, strLog);
					break;

				case eLOG_WRN:
					LOGW("[%s] %s", strModule, strLog);
					break;

				case eLOG_INF:
					LOGI("[%s] %s", strModule, strLog);
					break;

				case eLOG_DBG:
					LOGD("[%s] %s", strModule, strLog);
					break;

				default:
					LOGE("[LOG] Log type error!\n");
					break;
			}
		}
		else {
			(void)printf("%s", strLog);
		}
	}
#endif
}

