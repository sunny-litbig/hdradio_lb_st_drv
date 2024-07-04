/****************************************************************************************
 *   FileName    : DBusMsgDefNames.c
 *   Description : 
 ****************************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited 
to re-distribution in source or binary form is strictly prohibited.
This source code is provided ¡°AS IS¡± and nothing contained in this source code 
shall constitute any express or implied warranty of any kind, including without limitation, 
any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, 
copyright or other third party intellectual property right. 
No warranty is made, express or implied, regarding the information¡¯s accuracy, 
completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, 
out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************************/


#include "DBusMsgDef.h"

const char *g_signalLauncherEventNames[TotalSignalLauncherEvents] = {
    SIGNAL_LAUNCHER_KEY_PRESSED,
    SIGNAL_LAUNCHER_KEY_LONG_PRESSED,
    SIGNAL_LAUNCHER_KEY_LONG_LONG_PRESSED,
    SIGNAL_LAUNCHER_KEY_RELEASED,
    SIGNAL_LAUNCHER_KEY_CLICKED,
	SIGNAL_LAUNCHER_ENABLE_KEY_EVENT,
	SIGNAL_LAUNCHER_DISABLE_KEY_EVENT,
};

const char *g_methodLauncherEventNames[TotalMethodLauncherEvents] = {
    METHOD_LAUNCHER_ACTIVE_APPLICATION,
	METHOD_LAUNCHER_START_APPLICATIONS,
	METHOD_LAUNCHER_PREEMPT_KEY_EVENT,
	METHOD_LAUNCHER_RELEASE_PREEMPT_KEY_EVENT,
};

const char *g_methodModeManagerEventNames[TotalMethodModeManagerEvent] = {
	CHANGE_MODE,
	RELEASE_RESOURCE_DONE,
	END_MODE,
	MODE_ERROR_OCCURED
};

const char *g_signalModeManagerEventNames[TotalSignalModeManagerEvent] = {
	CHANGED_MODE,
	RELEASE_RESOURCE
};

const char* g_methodRadioEventNames[TotalMethodRadioEvent] = {
	METHOD_RADIO_CHANGE_TYPE,
	METHOD_RADIO_SELECT_PRESET,
	METHOD_RADIO_TURN_ON,
	METHOD_RADIO_TUNER_OPEN,
	METHOD_RADIO_TUNER_CLOSE,
	METHOD_RADIO_PI_TUNE
};
const char* g_signalRadioEventNames[TotalSignalRadioEvent] = {
	SIGNAL_RADIO_AM_MODE,
	SIGNAL_RADIO_FM_MODE,
	SIGNAL_RADIO_HDR_MODE,
	SIGNAL_RADIO_AM_PRESET_LIST,
	SIGNAL_RADIO_FM_PRESET_LIST,
	SIGNAL_RADIO_PRESET_CHANGED,
	SIGNAL_RADIO_FREQUENCY_CHANGED
};
