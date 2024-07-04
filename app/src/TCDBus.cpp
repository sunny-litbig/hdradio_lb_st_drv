/****************************************************************************************
 *   FileName    : TCPlayerDBusManager.cpp
 *   Description :
 ****************************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved

This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited
to re-distribution in source or binary form is strictly prohibited.
This source code is provided AS IS and nothing contained in this source code
shall constitute any express or implied warranty of any kind, including without limitation,
any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent,
copyright or other third party intellectual property right.
No warranty is made, express or implied, regarding the informations accuracy,
completeness, or performance.
In no event shall Telechips be liable for any claim, damages or other liability arising from,
out of or in connection with this source code or the use in the source code.
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
between Telechips and Company.
*
****************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <dbus/dbus.h>
#include <vector>
#include "DBusMsgDef.h"
#include "TCDBusRawAPI.h"
#include "TCDBus.h"
#include "DBusInterface.h"

extern int g_appID;
extern int g_debug;

#define TC_RADIO_PRINTF(format, arg...) \
	if (g_debug) \
	{ \
        fprintf(stderr, "[TC RADIO] %s: " format "", __FUNCTION__, ##arg); \
    }

typedef void (*DBusMethodCallFunction)(DBusMessage *message);

static DBusMsgErrorCode OnReceivedDBusSignal(DBusMessage *message, const char *interface);
static DBusMsgErrorCode OnReceivedMethodCall(DBusMessage *message, const char *interface);
static void LauncherSignalDBusProcess(unsigned int id, DBusMessage *message);
static void ModeManagerSignalDBusProcess(unsigned int id, DBusMessage *message);

static void DBusMethodChangeType(DBusMessage *message);
static void DBusMethodSelectPreset(DBusMessage *message);
static void DBusMethodTurnOn(DBusMessage *message);
static void DBusMethodTunerOpen(DBusMessage *message);
static void DBusMethodTunerClose(DBusMessage *message);
static void DBusMethodPITune(DBusMessage *message);

static DBusMethodCallFunction dbusMethodProcess[TotalMethodRadioEvent] = {
	DBusMethodChangeType,
	DBusMethodSelectPreset,
	DBusMethodTurnOn,
	DBusMethodTunerOpen,
	DBusMethodTunerClose,
	DBusMethodPITune
};

void InitilaizeTCDBusManager(void)
{
    qDebug("InitilaizeTCDBusManager");
	SetDBusPrimaryOwner(RADIO_PROCESS_DBUS_NAME);
	SetCallBackFunctions(OnReceivedDBusSignal, OnReceivedMethodCall);
	AddSignalInterface(LAUNCHER_EVENT_INTERFACE);
	AddSignalInterface(MODEMANAGER_EVENT_INTERFACE);
	AddMethodInterface(RADIO_EVENT_INTERFACE);
    InitializeRawDBusConnection("TC RADIO DBUS");
}

void ReleaseTCDBusManager(void)
{
	ReleaseRawDBusConnection();
}

void SendDBusRadioAM(void)
{
    TC_RADIO_PRINTF("\n");

	DBusMessage *message;
	message = CreateDBusMsgSignal(RADIO_PROCESS_OBJECT_PATH, RADIO_EVENT_INTERFACE,
								  g_signalRadioEventNames[SignalRadioAMMode],
							  	  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
    		TC_RADIO_PRINTF("Signal Emit Completed\n");
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusRadioFM(void)
{
    TC_RADIO_PRINTF("\n");

	DBusMessage *message;
	message = CreateDBusMsgSignal(RADIO_PROCESS_OBJECT_PATH, RADIO_EVENT_INTERFACE,
								  g_signalRadioEventNames[SignalRadioFMMode],
							  	  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
    		TC_RADIO_PRINTF("Signal Emit Completed\n");
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusRadioHDR(int status)
{
    TC_RADIO_PRINTF("\n");

	DBusMessage *message;
	message = CreateDBusMsgSignal(RADIO_PROCESS_OBJECT_PATH, RADIO_EVENT_INTERFACE,
								  g_signalRadioEventNames[SignalRadioHDRMode],
								  DBUS_TYPE_INT32, &status,
							  	  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
    		TC_RADIO_PRINTF("Signal Emit Completed\n");
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusRadioFrequency(int freq)
{
    TC_RADIO_PRINTF("Frequency(%d)\n", freq);

	DBusMessage *message;
	message = CreateDBusMsgSignal(RADIO_PROCESS_OBJECT_PATH, RADIO_EVENT_INTERFACE,
								  g_signalRadioEventNames[SignalRadioFrequencyChanged],
								  DBUS_TYPE_INT32, &freq,
							  	  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
    		TC_RADIO_PRINTF("Signal Emit Completed\n");
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusAMPresetList(int *preset, int cnt)
{
    TC_RADIO_PRINTF("\n");
	DBusMessage *message;
	message = CreateDBusMsgSignal(RADIO_PROCESS_OBJECT_PATH, RADIO_EVENT_INTERFACE,
								  g_signalRadioEventNames[SignalRadioAMPresetList],
								  DBUS_TYPE_ARRAY, DBUS_TYPE_INT32, &preset, cnt,
							  	  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			for(int i = 0; i < cnt; i++)
			{
    			TC_RADIO_PRINTF("Preset No.%d - %d)\n", i, preset[i]);
			}
    		TC_RADIO_PRINTF("Signal Emit Completed\n");
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusFMPresetList(int *preset, int cnt)
{
    TC_RADIO_PRINTF("\n");
	DBusMessage *message;
	message = CreateDBusMsgSignal(RADIO_PROCESS_OBJECT_PATH, RADIO_EVENT_INTERFACE,
								  g_signalRadioEventNames[SignalRadioFMPresetList],
								  DBUS_TYPE_ARRAY, DBUS_TYPE_INT32, &preset, cnt,
							  	  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			for(int i = 0; i < cnt; i++)
			{
    			TC_RADIO_PRINTF("Preset No.%d - %d)\n", i, preset[i]);
			}
    		TC_RADIO_PRINTF("Signal Emit Completed\n");
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusPresetChanged(int freq, int index)
{
    TC_RADIO_PRINTF("\n");
	DBusMessage *message;
	message = CreateDBusMsgSignal(RADIO_PROCESS_OBJECT_PATH, RADIO_EVENT_INTERFACE,
								  g_signalRadioEventNames[SignalRadioPresetChanged],
								  DBUS_TYPE_INT32, &freq,
								  DBUS_TYPE_INT32, &index,
							  	  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
    		TC_RADIO_PRINTF("Freq(%d), Index(%d)\n", freq, index);
    		TC_RADIO_PRINTF("Signal Emit Completed\n");
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusActiveApplication(int app, int active)
{
    TC_RADIO_PRINTF("APPLICATION(%d), ACTIVE(%s)\n",
										 app, (active != 0) ? "TRUE" : "FALSE");
	DBusMessage *message;

	message = CreateDBusMsgMethodCall(LAUNCHER_PROCESS_DBUS_NAME, LAUNCHER_PROCESS_OBJECT_PATH,
									  LAUNCHER_EVENT_INTERFACE,
									  METHOD_LAUNCHER_ACTIVE_APPLICATION,
									  DBUS_TYPE_INT32, &app,
									  DBUS_TYPE_BOOLEAN, &active,
									  DBUS_TYPE_INVALID);
	if (message != NULL)
	{
		if (!SendDBusMessage(message, NULL))
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}

		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}

void SendDBusChangeMode(const char* mode, int app)
{
	DBusMessage *message;
	int response;

	message = CreateDBusMsgMethodCall(MODEMANAGER_PROCESS_DBUS_NAME, MODEMANAGER_PROCESS_OBJECT_PATH,
			MODEMANAGER_EVENT_INTERFACE,
			CHANGE_MODE,
			DBUS_TYPE_STRING, &mode,
			DBUS_TYPE_INT32, &app,
			DBUS_TYPE_INVALID);
	if (message != NULL)
	{
		DBusPendingCall* pending = NULL;
		if(SendDBusMessage(message, &pending))
		{
			if(pending != NULL )
			{
				if(GetArgumentFromDBusPendingCall(pending,
												  DBUS_TYPE_INT32, &response,
												  DBUS_TYPE_INVALID))
				{
					if(!response)
					{
						TC_RADIO_PRINTF("%s: reply : %s\n", __FUNCTION__, response ? "ACK" : "NACK");
					}
				}
				else
				{
					TC_RADIO_PRINTF("%s: GetArgmentFromDBusPendingCall return error\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
			}
		}
		else
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}

void SendDBusEndMode(const char* mode, int app)
{
	DBusMessage *message;

	message = CreateDBusMsgMethodCall(MODEMANAGER_PROCESS_DBUS_NAME, MODEMANAGER_PROCESS_OBJECT_PATH,
			MODEMANAGER_EVENT_INTERFACE,
			END_MODE,
			DBUS_TYPE_STRING, &mode,
			DBUS_TYPE_INT32, &app,
			DBUS_TYPE_INVALID);
	if (message != NULL)
	{
		if (!SendDBusMessage(message, NULL))
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}

		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}

void SendDBusReleaseResourceDone(int resources, int app)
{
	DBusMessage *message;

	message = CreateDBusMsgMethodCall(MODEMANAGER_PROCESS_DBUS_NAME, MODEMANAGER_PROCESS_OBJECT_PATH,
			MODEMANAGER_EVENT_INTERFACE,
			RELEASE_RESOURCE_DONE,
			DBUS_TYPE_INT32, &resources,
			DBUS_TYPE_INT32, &app,
			DBUS_TYPE_INVALID);
	if (message != NULL)
	{
		if (!SendDBusMessage(message, NULL))
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}

		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}

static DBusMsgErrorCode OnReceivedMethodCall(DBusMessage *message, const char *interface)
{
	DBusMsgErrorCode error = ErrorCodeNoError;

	if (interface != NULL &&
			strcmp(interface, RADIO_EVENT_INTERFACE) == 0)
	{
		unsigned int index;
		unsigned int stop = 0;
		for (index = MethodRadioChangeType; index < TotalMethodRadioEvent && !stop; index++)
		{
			if (dbus_message_is_method_call(message,
						RADIO_EVENT_INTERFACE,
						g_methodRadioEventNames[index]))
			{
				TC_RADIO_PRINTF("method : %d, %s \n", index, g_methodRadioEventNames[index]);
				dbusMethodProcess[index](message);
				stop = 1;
			}
		}
	}
	return error;
}


static void DBusMethodChangeType(DBusMessage *message)
{
	TC_RADIO_PRINTF("\n");

	if (message != NULL)
	{
		if (GetArgumentFromDBusMessage(message,
					DBUS_TYPE_INVALID))
		{
			TC_IF_MANAGER->onChangeType();
		}
		else
		{
			fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodSelectPreset(DBusMessage *message)
{
	TC_RADIO_PRINTF("\n");
	int idx;

	if (message != NULL)
	{
		if (GetArgumentFromDBusMessage(message,
					DBUS_TYPE_INT32, &idx,
					DBUS_TYPE_INVALID))
		{
			TC_IF_MANAGER->onSelectPreset(idx);
		}
		else
		{
			fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodTurnOn(DBusMessage *message)
{
	TC_RADIO_PRINTF("\n");

	if (message != NULL)
	{
		if (GetArgumentFromDBusMessage(message,
					DBUS_TYPE_INVALID))
		{
			SendDBusChangeMode("audioplaybg", g_appID);
		}
		else
		{
			fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodTunerOpen(DBusMessage *message)
{
	TC_RADIO_PRINTF("\n");

	if (message != NULL)
	{
		if (GetArgumentFromDBusMessage(message,
					DBUS_TYPE_INVALID))
		{
			TC_IF_MANAGER->onTunerExtOpen();
		}
		else
		{
			fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodTunerClose(DBusMessage *message)
{
	TC_RADIO_PRINTF("\n");

	if (message != NULL)
	{
		if (GetArgumentFromDBusMessage(message,
					DBUS_TYPE_INVALID))
		{
			TC_IF_MANAGER->onTunerExtClose();
		}
		else
		{
			fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodPITune(DBusMessage *message)
{
	TC_RADIO_PRINTF("\n");

	unsigned int *PI;
	unsigned int count;

	if (message != NULL)
	{
		if (GetArgumentFromDBusMessage(message,
					DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &PI, &count,
					DBUS_TYPE_INVALID))
		{
			TC_IF_MANAGER->seekPIList(PI, count);
		}
		else
		{
			fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static DBusMsgErrorCode OnReceivedDBusSignal(DBusMessage *message, const char *interface)
{
	DBusMsgErrorCode error = ErrorCodeNoError;

	if (message != NULL &&
		interface != NULL)
	{
		unsigned int index;
		unsigned int stop = 0;

		if (strcmp(interface, LAUNCHER_EVENT_INTERFACE) == 0)
		{
			for (index = SignalLauncherKeyPressed; index < TotalSignalLauncherEvents && !stop; index++)
			{
				if (dbus_message_is_signal(message,
										   LAUNCHER_EVENT_INTERFACE,
										   g_signalLauncherEventNames[index]))
				{
					LauncherSignalDBusProcess(index, message);
					stop = 1;
				}
			}
		}
		else if (strcmp(interface, MODEMANAGER_EVENT_INTERFACE) == 0)
		{
			for(index = ChangedMode; index < TotalSignalModeManagerEvent && !stop; index++)
			{
				if(dbus_message_is_signal(message,
										  MODEMANAGER_EVENT_INTERFACE,
										  g_signalModeManagerEventNames[index]))
				{
					ModeManagerSignalDBusProcess(index, message);
					stop = 1;
				}
			}
		}

		if (!stop)
		{
			error = ErrorCodeUnknown;
		}
	}

	return error;
}

static void LauncherSignalDBusProcess(unsigned int id, DBusMessage *message)
{
	if (id < TotalSignalLauncherEvents)
	{
		TC_RADIO_PRINTF("[RADIO]LauncherSignalDBusProcess %d\n", id);
        TC_RADIO_PRINTF("RECEIVED SIGNAL[%s(%d)]\n",
									  g_signalLauncherEventNames[id], id);

		if (message != NULL)
		{
			int value = -1;
			if (GetArgumentFromDBusMessage(message,
										   DBUS_TYPE_INT32, &value,
										   DBUS_TYPE_INVALID))
			{
				switch (id)
				{
					case SignalLauncherKeyPressed:
                        TC_IF_MANAGER->onKeyboardPressedEvent(value);
						break;
					case SignalLauncherKeyLongPressed:
                        TC_IF_MANAGER->onKeyboardLongPressedEvent(value);
						break;
					case SignalLauncherKeyLongLongPressed:
                        TC_IF_MANAGER->onKeyboardLongLongPressedEvent(value);
						break;
					case SignalLauncherKeyReleased:
                        TC_IF_MANAGER->onKeyboardReleasedEvent(value);
						break;
					case SignalLauncherKeyClicked:
                        TC_IF_MANAGER->onKeyboardClickedEvent(value);
						break;
					default:
						fprintf(stderr, "%s: unknown signal id(%d)\n", __FUNCTION__, id);
						break;
				}
			}
			else
			{
				fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
			}
		}
	}
	else
	{
        TC_RADIO_PRINTF("RECEIVED SIGNAL[%d]\n",
									  id);
	}
}

static void ModeManagerSignalDBusProcess(unsigned int id, DBusMessage *message)
{
	if (id < TotalSignalLauncherEvents)
	{
        TC_RADIO_PRINTF("RECEIVED SIGNAL[%s(%d)]\n",
									  g_signalModeManagerEventNames[id], id);
		if (message != NULL)
		{
			if (id == ChangedMode)
			{
				const char* mode;
				int app;
				if (GetArgumentFromDBusMessage(message,
											   DBUS_TYPE_STRING, &mode,
											   DBUS_TYPE_INT32, &app,
											   DBUS_TYPE_INVALID))
				{
					if((strcmp(mode, "view") == 0) && (app == g_appID))
                		TC_IF_MANAGER->onChangedMode("view");
					else if((strcmp(mode, "audioplay") == 0) && (app == g_appID))
						TC_IF_MANAGER->onChangedMode("audioplay");
					else if((strcmp(mode, "audioplaybg") == 0) && (app == g_appID))
						TC_IF_MANAGER->onChangedMode("audioplaybg");
				}
			}
			else if(id == ReleaseResource)
			{
				int resources;
				int app;
				if (GetArgumentFromDBusMessage(message,
											   DBUS_TYPE_INT32, &resources,
											   DBUS_TYPE_INT32, &app,
											   DBUS_TYPE_INVALID))
				{
					if(app == g_appID)
					{
						TC_IF_MANAGER->onReleaseResources(resources);
					}
				}
			}
		}
	}
}
