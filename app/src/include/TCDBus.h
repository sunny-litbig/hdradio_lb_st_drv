/****************************************************************************************
 *   FileName    : TCPlayerDBusManager.h
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

#ifndef TC_DBUS_MANAGER_H
#define TC_DBUS_MANAGER_H

void InitilaizeTCDBusManager(void);
void ReleaseTCDBusManager(void);

void SendDBusRadioAM(void);
void SendDBusRadioFM(void);
void SendDBusRadioHDR(int status);
void SendDBusRadioFrequency(int freq);
void SendDBusAMPresetList(int *preset, int cnt);
void SendDBusFMPresetList(int *preset, int cnt);
void SendDBusPresetChanged(int freq, int index);

void SendDBusActiveApplication(int app, int active);

void SendDBusChangeMode(const char* mode, int app);
void SendDBusEndMode(const char* mode, int app);
void SendDBusReleaseResourceDone(int resources, int app);

#endif // TC_DBUS_MANAGER_H
