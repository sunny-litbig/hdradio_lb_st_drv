/****************************************************************************************
 *   FileName    : DBusMsgDef.h
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

#ifndef DBUS_MSG_DEF_H
#define DBUS_MSG_DEF_H

/*==================== LAUNCHER DBUS ====================*/
//#define	USE_HDR_ACTIVE_ON_SHOW_APP
#define USE_GENIVI_AUDIOMANAGER

#define LAUNCHER_PROCESS_DBUS_NAME                  "telechips.launcher.process"
#define LAUNCHER_PROCESS_OBJECT_PATH                "/telechips/launcher/process"

#define LAUNCHER_EVENT_INTERFACE                    "launcher.event"
// LAUNCHER SIGNAL EVENT DEFINES
#define SIGNAL_LAUNCHER_KEY_PRESSED                 "signal_launcher_key_pressed"
#define SIGNAL_LAUNCHER_KEY_LONG_PRESSED            "signal_launcher_key_long_pressed"
#define SIGNAL_LAUNCHER_KEY_LONG_LONG_PRESSED       "signal_launcher_key_long_long_pressed"
#define SIGNAL_LAUNCHER_KEY_RELEASED                "signal_launcher_key_released"
#define SIGNAL_LAUNCHER_KEY_CLICKED                 "signal_launcher_key_clicked"
#define SIGNAL_LAUNCHER_ENABLE_KEY_EVENT			"signal_launcher_enable_key_event"
#define SIGNAL_LAUNCHER_DISABLE_KEY_EVENT			"signal_launcher_disable_key_event"

typedef enum {
    SignalLauncherKeyPressed,
    SignalLauncherKeyLongPressed,
    SignalLauncherKeyLongLongPressed,
    SignalLauncherKeyReleased,
    SignalLauncherKeyClicked,
	SignalLauncherEnableKeyEvent,
	SignalLauncherDisableKeyEvent,
    TotalSignalLauncherEvents
} SignalLauncherEvent;
extern const char *g_signalLauncherEventNames[TotalSignalLauncherEvents];

// LAUNCHER METHOD EVENT DEFINES
#define METHOD_LAUNCHER_ACTIVE_APPLICATION          "method_launcher_active_application"
#define METHOD_LAUNCHER_START_APPLICATIONS          "method_launcher_start_applications"
#define METHOD_LAUNCHER_PREEMPT_KEY_EVENT			"method_launcher_preempt_key_event"
#define METHOD_LAUNCHER_RELEASE_PREEMPT_KEY_EVENT	"method_launcher_release_preempt_key_event"

typedef enum {
    MethodLauncherActiveApplication,
    MethodLauncherStartApplications,
	MethodLauncherPreemptKeyEvent,
	MethodLauncherReleasePreemptKeyEvent,
    TotalMethodLauncherEvents
} MethodLauncherEvent;
extern const char *g_methodLauncherEventNames[TotalMethodLauncherEvents];

/*==================== LAUNCHER DBUS END ====================*/

/*==================== MODEMANAGER DBUS START ====================*/
#define MODEMANAGER_PROCESS_DBUS_NAME					"telechips.mode.manager"
#define MODEMANAGER_PROCESS_OBJECT_PATH					"/telechips/mode/manager"
#define MODEMANAGER_EVENT_INTERFACE						"mode.manager"

#define CHANGE_MODE										"change_mode"
#define RELEASE_RESOURCE_DONE							"release_resource_done"
#define END_MODE										"end_mode"
#define MODE_ERROR_OCCURED								"mode_error_occured"

typedef enum{
	ChangeMode,
	ReleaseResource_Done,
	EndMode,
	ModeErrorOccured,
	TotalMethodModeManagerEvent
}MethodModeManagerEvent;
extern const char* g_methodModeManagerEventNames[TotalMethodModeManagerEvent];

#define CHANGED_MODE									"changed_mode"
#define RELEASE_RESOURCE								"release_resource"

typedef enum{
	ChangedMode,
	ReleaseResource,
	TotalSignalModeManagerEvent
}SignalModeManagerEvent;
extern const char* g_signalModeManagerEventNames[TotalSignalModeManagerEvent];

/*==================== MODEMANAGER DBUS END ====================*/


/*==================== RADIO DBUS ====================*/
#define RADIO_PROCESS_DBUS_NAME						"telechips.radio.process"
#define RADIO_PROCESS_OBJECT_PATH					"/telechips/radio/process"
#define RADIO_EVENT_INTERFACE						"radio.event"

#define METHOD_RADIO_CHANGE_TYPE					"method_radio_change_type"
#define METHOD_RADIO_SELECT_PRESET					"method_radio_select_preset"
#define METHOD_RADIO_TURN_ON						"method_radio_turn_on"
#define METHOD_RADIO_TUNER_OPEN						"method_radio_tuner_open"
#define METHOD_RADIO_TUNER_CLOSE					"method_radio_tuner_close"
#define METHOD_RADIO_PI_TUNE						"method_radio_pi_tune"

typedef enum{
	MethodRadioChangeType,
	MethodRadioSelectPreset,
	MethodRadioTurnOn,
	MethodRadioTunerOpen,
	MethodRadioTunerClose,
	MethodRadioPITune,
	TotalMethodRadioEvent
}MethodRadioEvent;
extern const char* g_methodRadioEventNames[TotalMethodRadioEvent];

#define SIGNAL_RADIO_AM_MODE						"signal_radio_am_mode"
#define SIGNAL_RADIO_FM_MODE						"signal_radio_fm_mode"
#define SIGNAL_RADIO_HDR_MODE						"signal_radio_hdr_mode"
#define SIGNAL_RADIO_AM_PRESET_LIST					"signal_radio_am_preset_list"
#define SIGNAL_RADIO_FM_PRESET_LIST					"signal_radio_fm_preset_list"
#define SIGNAL_RADIO_PRESET_CHANGED					"signal_radio_preset_changed"
#define SIGNAL_RADIO_FREQUENCY_CHANGED				"signal_radio_frequency_changed"

typedef enum{
	SignalRadioAMMode,
	SignalRadioFMMode,
	SignalRadioHDRMode,
	SignalRadioAMPresetList,
	SignalRadioFMPresetList,
	SignalRadioPresetChanged,
	SignalRadioFrequencyChanged,
	TotalSignalRadioEvent
}SignalRadioEvent;
extern const char* g_signalRadioEventNames[TotalSignalRadioEvent];

/*==================== RADIO DBUS ====================*/

/*==================== DAB DBUS ====================*/
/*==================== DAB DBUS ====================*/

#endif // DBUS_MSG_DEF_H

