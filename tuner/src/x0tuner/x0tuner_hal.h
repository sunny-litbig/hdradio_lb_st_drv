/*******************************************************************************

*   FileName : x0tuner_hal.h

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
#ifndef __X0TUNER_HAL_H__
#define __X0TUNER_HAL_H__

#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define X0_DEBUG

#ifdef __ANDROID__

#define X0_TAG			("[RADIO][X0]")
#define X0_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,X0_TAG, __VA_ARGS__))
#ifdef X0_DEBUG
#define X0_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,X0_TAG, __VA_ARGS__))
#else
#define	X0_DBG(...)
#endif

#else // #ifdef __ANDROID__

#define X0_ERR(...)		((void)printf("[ERROR][RADIO][X0]: " __VA_ARGS__))
#ifdef X0_DEBUG
#define X0_DBG(...)		((void)printf("[DEBUG][RADIO][X0]: " __VA_ARGS__))
#else
#define	X0_DBG(...)
#endif

#endif // #ifdef __ANDROID__

#define x0tuner_mwait(X)			usleep((X)*(1000))

#define	X0_MAX_TUNER_IC_NUM		4

#define	SUB_LEN		3
#define	MAX_LEN		25
#define	PATCH_UNIT	(MAX_LEN-1)

#define LITHIO0_I2C_ADDR  			0xC8
#define	LITHIO1_I2C_ADDR			0xCA

#define  M_FM						0x20
#define  M_AM						0x21
#define  M_AUDIO					0x30
#define  M_APPL						0x40

//Write command
#define	Tune_To						0x01
#define	Set_Tune_Options			0x02
#define Set_BandWidth				0x0a
#define	Set_RFAGC					0x0b
#define Set_Antenna					0x0c
#define Set_MphSuppression			0x14
#define Set_ChannelEqualizer		0x16
#define Set_NoiseBlanker			0x17
#define Set_NoiseBlanker_Options	0x18
#define Set_DigitalRadio			0x1e
#define	Set_Deemphasis				0x1f
#define Set_StereoImprovement		0x20
#define Set_Highcut_Fix				0x21
#define Set_Lowcut_Fix				0x22
#define Set_LevelStep				0x26
#define Set_LevelOffset				0x27
#define Set_Softmute_Time			0x28
#define Set_Softmute_Mod			0x29
#define Set_Softmute_Level			0x2a
#define Set_Softmute_Noise			0x2b
#define Set_Softmute_Mph			0x2c
#define Set_Softmute_Max			0x2d
#define Set_Highcut_Time			0x32
#define Set_Highcut_Mod				0x33
#define Set_Highcut_Level			0x34
#define Set_Highcut_Noise			0x35
#define Set_Highcut_Mph				0x36
#define Set_Highcut_Max				0x37
#define Set_Highcut_Min				0x38
#define Set_Lowcut_Max				0x39
#define Set_Lowcut_Min				0x3a
#define Set_Highcut_Options			0x3b
#define Set_Stereo_Time				0x3c
#define Set_Stereo_Mod				0x3d
#define Set_Stereo_Level			0x3e
#define Set_Stereo_Noise			0x3f
#define Set_Stereo_Mph				0x40
#define Set_Stereo_Max				0x41
#define Set_Stereo_Min				0x42

#define Set_StHiBlend_Time			0x46
#define Set_StHiBlend_Mod			0x47
#define Set_StHiBlend_Level			0x48
#define Set_StHiBlend_Noise			0x49
#define Set_StHiBlend_Mph			0x4a
#define Set_StHiBlend_Max			0x4b
#define Set_StHiBlend_Min			0x4c

#define Set_Scaler					0x50
#define Set_RDS						0x51
#define Set_QualityStatus			0x52
#define Set_DR_Blend				0x53
#define Set_DR_Options				0x54
#define Set_Specials				0x55
#define Set_BandWidth_Options		0x56

#define Set_StBandBlend_Time		0x5a
#define Set_StBandBlend_Gain		0x5b
#define Set_StBandBlend_Bias		0x5c

#define Set_CoChannelDet			0x0e
#define Set_NoiseBlanker_Audio		0x18

//Write Audio command
#define Set_Volume					0x0a
#define Set_Mute		 			0x0b
#define Set_Input					0x0c
#define Set_Output_Source			0x0d
#define Set_Ana_Out					0x15
#define Set_Dig_IO					0x16
#define Set_Input_Scaler			0x17
#define Set_WaveGen					0x18

//Write APPL command
#define Set_OperationMode			0x01
#define Set_GPIO					0x03
#define Set_ReferenceClock			0x04
#define Set_Activate				0x05

//Read Command  - FM / AM
#define Get_Quality_Status 			0x80
#define Get_Quality_Data			0x81
#define Get_RDS_Status				0x82
#define Get_RDS_Data				0x83
#define Get_AGC						0x84
#define	Get_Signal_Status			0x85
#define	Get_Processing_Status		0x86
#define Get_Interface_Status		0x87

//Read APPL
#define	Get_Operation_Status		0x80
#define Get_GPIO_Status				0x81
#define	Get_Identification			0x82
#define Get_LastWrite				0x83

extern RET x0tunerhal_bootup(int32 ntuner);
extern RET x0tunerhal_tuneTo(int32 ntuner, uint32 mod_mode, uint32 freq, uint32 tune_mode);
extern RET x0tunerhal_getQualityData(int32 ntuner, uint32 mod_mode, stX0_DRV_QUALITY_t *qdata);
extern uint16 x0tunerhal_getInterfaceStatus(int32 ntuner);
extern RET x0tunerhal_setMute(int32 ntuner, uint32 fOnOff);

extern void *x0tunerhal_memset(void *pvdst, uint8 ubch, uint16 uilen);

#ifdef __cplusplus
}
#endif

#endif /* __X0TUNER_HAL_H__ */
