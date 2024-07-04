/*******************************************************************************

*   FileName : x0tuner_hal.c

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
#include <stdio.h>
//#include <string.h>
#include <sys/types.h>
//#include <fcntl.h>
#include <unistd.h>

#include "tcradio_types.h"
#include "tcradio_peri_config.h"
#include "tcradio_drv.h"
#include "x0tuner_core.h"
#include "x0tuner_hal.h"

extern const size_t PatchSize;
extern const uint8 *pPatchBytes;
extern const size_t LutSize;
extern const uint8 *pLutBytes;

extern stTUNER_DRV_CONFIG_t gX0Conf;

void x0tunerhal_setTune(uint32 mod);

uint32 x0tunerhal_u16btou32b(uint16 src)
{
	uint32 ret;

	if(src & 0x8000) {
		ret = src | 0xffffff00U;
	}
	else {
		ret = src & 0x0000ffffU;
	}

	return ret;
}

void x0tunerhal_32bto8b(uint8 *mem, uint32 data)
{
	mem[0] = (uint8)((data >> 24) & 0x000000FFUL);
	mem[1] = (uint8)((data >> 16) & 0x000000FFUL);
	mem[2] = (uint8)((data >> 8) & 0x000000FFUL);
	mem[3] = (uint8)((data >> 0) & 0x000000FFUL);
}

uint32 x0tunerhal_8bto32b(uint8 *data)
{
	return (((uint32)data[0] << 24) | ((uint32)data[1] << 16) | ((uint32)data[2] << 8) | ((uint32)data[3] << 0));
}

void x0tunerhal_16bto8b(uint8 *mem, uint16 data)
{
	mem[0] = (uint8)((data >> 8) & 0x00FF);
	mem[1] = (uint8)(data & 0x00FF);
}

uint16 x0tunerhal_8bto16b(uint8 *data)
{
	return (((uint16)data[0]) << 8 | ((uint16)data[1]));
}

void *x0tunerhal_memcpy(void *pvDst, void *pvSrc, uint16 uiLength)
{
	uint16 i;
	uint8 *pW;
	uint8 *pR;

	pW = (uint8 *)pvDst;
	pR = (uint8 *)pvSrc;

	if(uiLength == 0)
		return (void *)pW;

	for(i = 0; i < uiLength; i++)
	{
		*pW++ = *pR++;
	}
	return (void *)pW;
}

void *x0tunerhal_memset(void *pvdst, uint8 ubch, uint16 uilen)
{
	uint8 *pW;
	uint16 i;

	pW = (uint8 *)pvdst;

	if(uilen == 0)
		return (void *)pW;

	for(i=0; i<uilen; i++)
	{
		*pW++ = ubch;
	}
	return (void *)pW;
}

RET x0tunerhal_readRegister(int32 ntuner, uint8 module, uint8 cmd, uint8 index, uint8 *data, uint32 len)
{
	RET ret = eRET_NG_IO;
	uint8 tbuf[MAX_LEN] = {0,};
	uint32 i;

	tbuf[0] = module;
	tbuf[1] = cmd;
	tbuf[2] = index;

	if(ntuner != 0) {
		ret = (*pfnI2cRx)(LITHIO1_I2C_ADDR, tbuf, SUB_LEN, data, len);
	}
	else {
		ret = (*pfnI2cRx)(LITHIO0_I2C_ADDR, tbuf, SUB_LEN, data, len);
	}

	X0_DBG("[%s] : module[%02xh] cmd[%02xh] index[%02xh] len[%d] => data ", __func__, tbuf[0], tbuf[1], tbuf[2], len);
	for(i=0; i<len; i++) {
		X0_DBG("[%02xh] ", data[i]);
	}
	X0_DBG("\n");

	return ret;
}

RET x0tunerhal_writeRawData(int32 ntuner, uint8 *data, uint32 len)
{
	RET ret = eRET_OK;

	if(ntuner != 0) {
		ret = (*pfnI2cTx)(LITHIO1_I2C_ADDR, data, len);
	}
	else {
		ret = (*pfnI2cTx)(LITHIO0_I2C_ADDR, data, len);
	}

	if(ret < 0) {
		ret = eRET_NG_IO;
	}

	return ret;
}
RET x0tunerhal_write8Register(int32 ntuner, uint8 module, uint8 cmd, uint8 index, uint8 *data, uint32 len)
{
	RET ret = eRET_NG_IO;
	uint8 tbuf[MAX_LEN] = {0,};
	uint32 i;

	if(len <= (MAX_LEN-3)) {
		tbuf[0] = module;
		tbuf[1] = cmd;
		tbuf[2] = index;

		x0tunerhal_memcpy(tbuf+3, data, len);
		if(ntuner != 0) {
			ret = (*pfnI2cTx)(LITHIO1_I2C_ADDR, tbuf, len+3);
		}
		else {
			ret = (*pfnI2cTx)(LITHIO0_I2C_ADDR, tbuf, len+3);
		}

		X0_DBG("[%s] : module[%02xh] cmd[%02xh] index[%02xh] len[%d] => data ", __func__, tbuf[0], tbuf[1], tbuf[2], len);
		for(i=0; i<len; i++) {
			X0_DBG("[%02xh] ", tbuf[i+3]);
		}
		X0_DBG("\n");
	}
	else {
		ret = eRET_NG_INVALID_LENGTH;
	}

	return ret;
}

RET x0tunerhal_WriteData(uint8 m_mode, uint8 m_cmd, uint8 m_index, uint16 data1, uint16 data2, uint16 data3, uint16 data4, uint16 data5, uint16 data6, uint8 cnt)
{
	RET ret = eRET_OK;
	uint8 i2cWrdataBuf[15];
	uint32 i;

	i2cWrdataBuf[0] = m_mode;
	i2cWrdataBuf[1] = m_cmd;
	i2cWrdataBuf[2] = m_index;  // START INDEX

	i2cWrdataBuf[3] = (uint8)((data1 & 0xff00) >> 8);
	i2cWrdataBuf[4] = (uint8)((data1 & 0x00ff));
	i2cWrdataBuf[5] = (uint8)((data2 & 0xff00) >> 8);
	i2cWrdataBuf[6] = (uint8)((data2 & 0x00ff));
	i2cWrdataBuf[7] = (uint8)((data3 & 0xff00) >> 8);
	i2cWrdataBuf[8] = (uint8)((data3 & 0x00ff));
	i2cWrdataBuf[9] = (uint8)((data4 & 0xff00) >> 8);
	i2cWrdataBuf[10] = (uint8)((data4 & 0x00ff));
	i2cWrdataBuf[11] = (uint8)((data5 & 0xff00) >> 8);
	i2cWrdataBuf[12] = (uint8)((data5 & 0x00ff));
	i2cWrdataBuf[13] = (uint8)((data6 & 0xff00) >> 8);
	i2cWrdataBuf[14] = (uint8)((data6 & 0x00ff));

	ret = (*pfnI2cTx)(LITHIO0_I2C_ADDR, i2cWrdataBuf, cnt+3);

#if 0
	X0_DBG("[%s] : module[%02xh] cmd[%02xh] index[%02xh] len[%d] => data ", __func__, m_mode, m_cmd, m_index, cnt);
	for(i=0; i<cnt; i++) {
		X0_DBG("[%02xh] ", i2cWrdataBuf[i+3]);
	}
	X0_DBG("\n");
#endif

	return ret;
}

RET x0tunerhal_writePatch(int32 ntuner)
{
	RET ret = eRET_OK;
	uint8 tbuf[MAX_LEN+100] = {0,};
	uint32 i, rest;

	tbuf[0] = 0x1C;
	tbuf[1] = 0x00;
	tbuf[2] = 0x00;
	x0tunerhal_writeRawData(ntuner, tbuf, 3);
	tbuf[2] = 0x74;
	x0tunerhal_writeRawData(ntuner, tbuf, 3);

//------------------------------------------------------------
	x0tunerhal_memset(tbuf, 0, sizeof(tbuf));
	tbuf[0] = 0x1B;
	for(i=0; i<PatchSize/PATCH_UNIT; i++) {
		x0tunerhal_memcpy(tbuf+1, (void*)pPatchBytes+(i*PATCH_UNIT), PATCH_UNIT);
		x0tunerhal_writeRawData(ntuner, tbuf, PATCH_UNIT+1);
	}
	rest = PatchSize% PATCH_UNIT;
	if(rest != 0) {
		x0tunerhal_memcpy(tbuf+1, (void*)pPatchBytes+(i*PATCH_UNIT), rest);
		x0tunerhal_writeRawData(ntuner, tbuf, rest+1);
	}

//------------------------------------------------------------
	tbuf[0] = 0x1C;
	tbuf[1] = 0x00;
	tbuf[2] = 0x00;
	x0tunerhal_writeRawData(ntuner, tbuf, 3);
	tbuf[2] = 0x75;
	x0tunerhal_writeRawData(ntuner, tbuf, 3);

//------------------------------------------------------------
	tbuf[0] = 0x1B;
	x0tunerhal_memcpy(tbuf+1, (void*)pLutBytes, LutSize);
	x0tunerhal_writeRawData(ntuner, tbuf, (uint32)LutSize+1);

//------------------------------------------------------------
	tbuf[0] = 0x1C;
	tbuf[1] = 0x00;
	tbuf[2] = 0x00;
	x0tunerhal_writeRawData(ntuner, tbuf, 3);

	return ret;
}

// 4.12 APPL cmd 128 Get_Operation_Status
uint16 x0tunerhal_getOperationStatus(int32 ntuner)
{
	uint8 tbuf[MAX_LEN] = {0,};
	uint16 status;

	x0tunerhal_readRegister(ntuner, M_APPL, Get_Operation_Status, 1, tbuf, 2);
	status = x0tunerhal_8bto16b(tbuf);
	return status;
}

RET x0tunerhal_start(int32 ntuner)
{
	RET ret = eRET_OK;
	uint8 tbuf[3] = {0x14, 0x00, 0x01};

	ret = x0tunerhal_writeRawData(ntuner, tbuf, 3);

	return ret;
}

RET x0tunerhal_setReferenceClock(int32 ntuner, uint32 clock, uint16 type)
{
	RET ret = eRET_OK;
	uint8 tbuf[MAX_LEN] = {0,};

	if(type < 2) {
		if(clock == 4000000 || clock == 9216000 || clock == 12000000 || clock == 55466670) {
			x0tunerhal_32bto8b(tbuf, clock);
			x0tunerhal_16bto8b(tbuf+4, type);
			ret = x0tunerhal_write8Register(ntuner, M_APPL, Set_ReferenceClock, 1, tbuf, 6);
		}
		else {
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET x0tunerhal_activate(int32 ntuner)
{
	RET ret = eRET_OK;
	uint8 tbuf[2] = {0x00, 0x01};

	ret = x0tunerhal_write8Register(ntuner, M_APPL, Set_Activate, 1, tbuf, 2);

	return ret;
}

RET x0tunerhal_bootup(int32 ntuner)
{
	RET ret = eRET_OK;
	uint32 trycnt=0;

	while(x0tunerhal_getOperationStatus(ntuner) != 0)
	{
		if(pfnTunerPower != NULL) {
			(*pfnTunerPower)(OFF);
			x0tuner_mwait(20);
			(*pfnTunerPower)(ON);
		}
		x0tuner_mwait(20);

		if(++trycnt > 10) {
			ret = eRET_NG_NO_RSC;
			break;
		}
	}

	if(ret == eRET_OK) {
		ret = x0tunerhal_writePatch(ntuner);
	}

	if(ret == eRET_OK) {
		ret = x0tunerhal_start(ntuner);
	}

	if(ret == eRET_OK) {
		do {
			x0tuner_mwait(50);
			if(trycnt++ > 10) {
				ret = eRET_NG_TIMEOUT;
				break;
			}
		}while(x0tunerhal_getOperationStatus(ntuner) != 1);
	}

	if(ret == eRET_OK) {
		ret = x0tunerhal_setReferenceClock(ntuner, 9216000, 0);
	}

	if(ret == eRET_OK) {
		ret = x0tunerhal_activate(ntuner);
	}

	if(ret == eRET_OK) {
		do {
			x0tuner_mwait(100);
			if(++trycnt > 10) {
				ret = eRET_NG_TIMEOUT;
				break;
			}
		}while(x0tunerhal_getOperationStatus(ntuner) != 2);
	}

	return ret;
}

RET x0tunerhal_tuneTo(int32 ntuner, uint32 mod_mode, uint32 freq, uint32 tune_mode)
{
	RET ret = eRET_OK;
	uint8 tbuf[4] = {0, };

	tbuf[1] = (uint8)tune_mode + 1;

	if (((freq >= 144 && freq <=288) || (freq >= 522 && freq <= 1710) || (freq >= 2300 && freq <= 27000)) &&
		tbuf[1] > 0 && tbuf[1] <= 7 && tbuf[1] != 6)
	{
		x0tunerhal_16bto8b(tbuf+2, freq);
		ret = x0tunerhal_write8Register(ntuner, M_AM, Tune_To, 1, tbuf, 4);
		x0tunerhal_setTune(eTUNER_DRV_AM_MODE);
	}
	else if (freq >= 65000 && freq <=108000 && tbuf[1] > 0 && tbuf[1] <= 7 && tbuf[1] != 6)
	{
		x0tunerhal_16bto8b(tbuf+2, (uint16)(freq/10));
		ret = x0tunerhal_write8Register(ntuner, M_FM, Tune_To, 1, tbuf, 4);
		x0tunerhal_setTune(eTUNER_DRV_FM_MODE);
	}
	else
	{
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET x0tunerhal_setMute(int32 ntuner, uint32 fOnOff)
{
	RET ret = eRET_OK;

	if(fOnOff == 0) {
		ret = x0tunerhal_WriteData (M_AUDIO, Set_Mute, 1, 0, 0, 0, 0, 0, 0, 2);
	}
	else {
		ret = x0tunerhal_WriteData (M_AUDIO, Set_Mute, 1, 1, 0, 0, 0, 0, 0, 2);
	}

	return ret;
}

void x0tunerhal_setTune(uint32 mod)
{

	if (mod == eTUNER_DRV_FM_MODE)	// FM
	{
		x0tunerhal_WriteData (M_FM, Set_ChannelEqualizer, 1, 1, 0, 0, 0, 0, 0, 2);    //channel equalize
		x0tunerhal_WriteData (M_FM, Set_Softmute_Time, 1, 120, 500, 10, 20, 0, 0, 8);
		x0tunerhal_WriteData (M_FM, Set_Softmute_Mph, 1, 0, 200, 1000, 0, 60, 0, 10);
		x0tunerhal_WriteData (M_FM, Set_Softmute_Noise, 1, 0, 200, 1000, 0, 60, 0, 10);
		x0tunerhal_WriteData (M_FM, Set_Highcut_Time, 1, 200, 2000, 10, 80, 0, 0, 8);
		x0tunerhal_WriteData (M_FM, Set_Highcut_Mph, 1, 3, 120, 160, 00, 0, 0, 6);
		x0tunerhal_WriteData (M_FM, Set_Highcut_Noise, 1, 3, 150, 200, 00, 0, 0, 6);
		x0tunerhal_WriteData (M_FM, Set_Highcut_Max, 1, 2400, 0, 0, 00, 0, 0, 2);
		x0tunerhal_WriteData (M_FM, Set_Lowcut_Max, 1, 100, 0, 0, 00, 0, 0, 2);
		x0tunerhal_WriteData (M_FM, Set_Stereo_Level, 1, 0, 460, 240, 00, 0, 0, 6);
		x0tunerhal_WriteData (M_FM, Set_Stereo_Time, 1, 200, 4000, 20, 80, 0, 0, 8);
		x0tunerhal_WriteData (M_FM, Set_Stereo_Mph, 1, 0, 100, 150, 0, 0, 0, 6);
		x0tunerhal_WriteData (M_FM, Set_Stereo_Noise, 1, 0, 120, 160, 0, 0, 0, 6);
		x0tunerhal_WriteData (M_FM, Set_StHiBlend_Mph, 1, 3, 80, 140, 0, 0, 0, 6);
		x0tunerhal_WriteData (M_FM, Set_StHiBlend_Noise, 1, 3, 80, 140, 0, 0, 0, 6);
		x0tunerhal_WriteData (M_FM, Set_QualityStatus, 1, 200, 0, 0, 0, 0, 0, 4);

		x0tunerhal_WriteData (M_FM, Set_DR_Blend, 1, 3, 50, 50, 0, 0, 0, 8);
		x0tunerhal_WriteData (M_FM, Set_DR_Options, 1, 0, 8706, 4112, 0, 0, 0, 6);
		if(gX0Conf.fIqOut) {
			x0tunerhal_WriteData (M_FM, Set_DigitalRadio, 1, 1, 0, 0, 0, 0, 0, 2);
		}
	}
	else{	// AM
		x0tunerhal_WriteData (M_AM, Set_Highcut_Level, 1, 2, 470, 200, 0, 0, 0, 6);
		x0tunerhal_WriteData (M_AM, Set_QualityStatus, 1, 200, 0, 0, 0, 0, 0, 4);

		x0tunerhal_WriteData (M_AM, Set_DR_Blend, 1, 3, 50, 50, 0, 0, 0, 8);
		x0tunerhal_WriteData (M_AM, Set_DR_Options, 1, 0, 8706, 4112, 0, 0, 0, 6);
		if(gX0Conf.fIqOut) {
			x0tunerhal_WriteData (M_AM, Set_DigitalRadio, 1, 1, 0, 0, 0, 0, 0, 2);
		}
	}

	x0tunerhal_WriteData (M_APPL, Set_GPIO, 1, 0, 33, 3, 0, 0, 0, 6);
	x0tunerhal_WriteData (M_APPL, Set_GPIO, 1, 1, 33, 0, 0, 0, 0, 6);
	x0tunerhal_WriteData (M_APPL, Set_GPIO, 1, 2, 33, 0, 0, 0, 0, 6);
	x0tunerhal_WriteData (M_APPL, Set_GPIO, 1, 3, 33, 0, 0, 0, 0, 6);

	x0tunerhal_WriteData (M_APPL, Set_GPIO, 1, 0, M_FM, 3, 0, 0, 0, 6);
	x0tunerhal_WriteData (M_APPL, Set_GPIO, 1, 1, M_FM, 0, 0, 0, 0, 6);
	x0tunerhal_WriteData (M_APPL, Set_GPIO, 1, 2, M_FM, 0, 0, 0, 0, 6);
	x0tunerhal_WriteData (M_APPL, Set_GPIO, 1, 3, M_FM, 0, 0, 0, 0, 6);

//	x0tunerhal_WriteData (M_AUDIO, Set_Input, 1, 32, 0, 0, 0, 0, 0, 2);	// test
//	x0tunerhal_WriteData (M_AUDIO, Set_Ana_Out, 1, 128, 0, 0, 0, 0, 0, 4);	// test

	if(gX0Conf.audioSamplerate == 48000) {
		x0tunerhal_WriteData (M_AUDIO, Set_Dig_IO, 1, 32, 2, 16, 256, 4800, 0, 10);
		x0tunerhal_WriteData (M_AUDIO, Set_Dig_IO, 1, 33, 2, 16, 256, 4800, 0, 10);
	}
	else {
		x0tunerhal_WriteData (M_AUDIO, Set_Dig_IO, 1, 32, 2, 16, 256, 4410, 0, 10);
		x0tunerhal_WriteData (M_AUDIO, Set_Dig_IO, 1, 33, 2, 16, 256, 4410, 0, 10);
	}

//	x0tunerhal_WriteData (M_AUDIO, Set_Mute, 1, 0, 0, 0, 0, 0, 0, 2);
	x0tunerhal_WriteData (M_AUDIO, Set_Volume, 1, 0, 0, 0, 0, 0, 0, 2);

//	x0tunerhal_WriteData (M_AUDIO, Set_WaveGen, 1, 6, 0, 0, 400, 0, 1000, 12);	// test

	x0tunerhal_WriteData (M_AUDIO, Set_Output_Source, 1, 33, 224, 0, 0, 0, 0, 4);
	x0tunerhal_WriteData (M_AUDIO, Set_Output_Source, 1, 128, 224, 0, 0, 0, 0, 4);
}

RET x0tunerhal_getQualityData(int32 ntuner, uint32 mod_mode, stX0_DRV_QUALITY_t *qdata)
{
	RET ret = eRET_NG_TIMEOUT;
	uint8 tbuf[MAX_LEN] = {0,};

	x0tunerhal_memset(qdata, 0, sizeof(qdata));

	if(mod_mode == eTUNER_DRV_AM_MODE) {
		ret = x0tunerhal_readRegister(ntuner, M_AM, Get_Quality_Data, 1, tbuf, 14);
		qdata->am.Status = x0tunerhal_8bto16b(tbuf);
		qdata->am.Rssi = x0tunerhal_u16btou32b(x0tunerhal_8bto16b(tbuf+2));
		qdata->am.Hfn = x0tunerhal_8bto16b(tbuf+4);
		qdata->am.Coch = x0tunerhal_8bto16b(tbuf+6);
		qdata->am.Offs = x0tunerhal_u16btou32b(x0tunerhal_8bto16b(tbuf+8));
		qdata->am.Bwth = x0tunerhal_8bto16b(tbuf+10);
		qdata->am.Mod = x0tunerhal_8bto16b(tbuf+12);
	}
	else {
		ret = x0tunerhal_readRegister(ntuner, M_FM, Get_Quality_Data, 1, tbuf, 14);
		qdata->fm.Status = x0tunerhal_8bto16b(tbuf);
		qdata->fm.Rssi = x0tunerhal_u16btou32b(x0tunerhal_8bto16b(tbuf+2));
		qdata->fm.Usn = x0tunerhal_8bto16b(tbuf+4);
		qdata->fm.Mpth = x0tunerhal_8bto16b(tbuf+6);
		qdata->fm.Offs = x0tunerhal_u16btou32b(x0tunerhal_8bto16b(tbuf+8));
		qdata->fm.Bwth = x0tunerhal_8bto16b(tbuf+10);
		qdata->fm.Mod = x0tunerhal_8bto16b(tbuf+12);
	}

	return ret;
}

uint16 x0tunerhal_getInterfaceStatus(int32 ntuner)
{
	uint8 tbuf[MAX_LEN] = {0,};
	uint16 samplerate;

	x0tunerhal_readRegister(ntuner, M_APPL, Get_Interface_Status, 1, tbuf, 2);

	samplerate = x0tunerhal_8bto16b(tbuf);
	return samplerate;
}

