/*******************************************************************************

*   FileName : max2175_core.c

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
#include <string.h>
#include <unistd.h>

#include "tcradio_types.h"
#include "tcradio_peri_config.h"
#include "tcradio_drv.h"
#include "max2175_hal.h"

#define	__MAX2175_DRIVER_VER_PREFIX__			'M'
#define	__MAX2175_DRIVER_VER_RELEASE_NUMBER__	0x02U
#define	__MAX2175_DRIVER_VER_MAJOR_NUMBER__		0x00U
#define	__MAX2175_DRIVER_VER_MINOR_NUMBER__		0x00U

stTUNER_DRV_CONFIG_t gMaximConf;

uint32 max2175_drv_initialized = 0;
uint32 max2175_drv_current_band[MAXIM_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};
uint32 max2175_drv_fm_frequency[MAXIM_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};
uint32 max2175_drv_am_frequency[MAXIM_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};

RET max2175_setTune(uint32 mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner);

void max2175_reset(void)
{
	if(pfnRadioGpioOpen != NULL)	(*pfnRadioGpioOpen)();
	if(pfnTunerPower != NULL)		(*pfnTunerPower)(ON);
	if(pfnAntPower != NULL)			(*pfnAntPower)(OFF);
	max2175_mwait(50);
	if(pfnAntPower != NULL)			(*pfnAntPower)(ON);
	max2175_mwait(20);
	if(pfnTunerReset != NULL)		(*pfnTunerReset)(ON);
	max2175_mwait(50);
	if(pfnTunerReset != NULL)		(*pfnTunerReset)(OFF);
}

RET max2175_initConfig(uint32 ntuners, uint32 xtal_opt)
{
	RET ret = eRET_OK;
	int tuner;

	/*
		Configure all the tuners in the system, set the ntuners (number of tuners) = to the tuners installed.
		Set the initial operating parameters for each tuner (this is required).
		One tuner is the master tuner (usually tuner 0) and provides the crystal reference to the slave tuners 1,2,3, etc.
	*/
	for (tuner = 0; tuner < ntuners; tuner++)
	{
		/*
			Initialize the tuners, needs to be done each time the tuners are powered up (or re-powered).
			Note: master tuner must be initialized before the other tuners using it's reference out as a reference source or connected
			references will not be present and the clock will not be there for I2C communications.

		*/
		MAXIM_DBG ("---> Init Tuner Config %d \n", tuner);
		if(xtal_opt == 1) {
			init(tuner, NORTH_AMERICA, 40.186125);
		}
		else {
			init(tuner, EUROPE, 36.864);
		}
		receive_mode(tuner, FM_HD_1p0);
		set_RF_frequency(tuner, 107.7);
	}
	return ret;
}

RET max2175_open(stTUNER_DRV_CONFIG_t type)
{
	RET ret = eRET_OK;

	MAXIM_DBG("%s:\n",__func__);

	if(max2175_drv_initialized != 0) {
		ret = eRET_NG_ALREADY_OPEN;
		return ret;
	}

	max2175_drv_initialized = 0;
	max2175_memset(max2175_drv_current_band, 0xff, sizeof(max2175_drv_current_band));
	max2175_memset(max2175_drv_fm_frequency, 0xff, sizeof(max2175_drv_fm_frequency));
	max2175_memset(max2175_drv_am_frequency, 0xff, sizeof(max2175_drv_am_frequency));

	gMaximConf = type;

	if(pfnI2cOpen != NULL && pfnI2cClose != NULL && pfnI2cTx != NULL && pfnI2cRx != NULL) {
		(*pfnI2cOpen)();
		max2175_reset();

		ret = max2175_initConfig(2, 1);
		if(ret == eRET_OK) {
			switch(type.numTuners) {
				case eTUNER_DRV_CONF_TYPE_QUAD:		max2175_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_QUATERNARY);
				case eTUNER_DRV_CONF_TYPE_TRIPLE:	max2175_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_TERTIARY);
				case eTUNER_DRV_CONF_TYPE_DUAL:		max2175_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_SECONDARY);
				default:							max2175_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_PRIMARY);	break;
			}
		}
	}
	else {
		ret = eRET_NG_NO_RSC;
	}

	if (ret == eRET_OK) {
		max2175_drv_initialized = 1;
	}
	else  {
		max2175_drv_initialized = 0;
		MAXIM_DBG("%s is failed[%d]\n",__func__, ret);
	}

	return ret;
}

RET max2175_close(void)
{
	RET ret = eRET_OK;
	MAXIM_DBG("[%s] \n", __func__);

	if(pfnI2cClose != NULL) {
		(*pfnI2cClose)();
	}
	max2175_drv_initialized = 0;
	max2175_memset(max2175_drv_current_band, 0xff, sizeof(max2175_drv_current_band));
	max2175_memset(max2175_drv_fm_frequency, 0xff, sizeof(max2175_drv_fm_frequency));
	max2175_memset(max2175_drv_am_frequency, 0xff, sizeof(max2175_drv_am_frequency));

	if(pfnTunerPower != NULL)		(*pfnTunerPower)(OFF);
	if(pfnAntPower != NULL)			(*pfnAntPower)(OFF);
	if(pfnTunerReset != NULL)		(*pfnTunerReset)(OFF);
	if(pfnRadioGpioClose != NULL)	(*pfnRadioGpioClose)();

	return ret;
}

RET max2175_setBand(uint32 mod_mode, uint32 ntuner)
{
	RET ret = eRET_OK;

	MAXIM_DBG("[%s] ", __func__);

	if(mod_mode == eTUNER_DRV_FM_MODE) {
		receive_mode(ntuner, FM_HD_1p0) ;
		if(ret == eRET_OK) {
			max2175_drv_current_band[ntuner] = eTUNER_DRV_FM_MODE;
		}
	}
	else if(mod_mode == eTUNER_DRV_AM_MODE) {
		receive_mode(ntuner, AM_NA_1p0) ;
		if(ret == eRET_OK) {
			max2175_drv_current_band[ntuner] = eTUNER_DRV_AM_MODE;
		}
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET max2175_setTune(uint32 mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner)
{
	RET ret = eRET_OK;
	double tfreq=0;

	MAXIM_DBG("[%s] ", __func__);

	tfreq = ((double)freq)/1000;

	if(mod_mode == eTUNER_DRV_FM_MODE) {
		if(max2175_drv_current_band[ntuner] != eTUNER_DRV_FM_MODE) {
			receive_mode(ntuner, FM_HD_1p0) ;
		}
		set_RF_frequency(ntuner, tfreq);
		if(ret == eRET_OK) {
			max2175_drv_current_band[ntuner] = eTUNER_DRV_FM_MODE;
			max2175_drv_fm_frequency[ntuner] = freq;
		}
	}
	else if(mod_mode == eTUNER_DRV_AM_MODE) {
		if(max2175_drv_current_band[ntuner] != eTUNER_DRV_AM_MODE) {
			receive_mode(ntuner, AM_NA_1p0) ;
		}
		set_RF_frequency(ntuner, tfreq);
		if(ret == eRET_OK) {
			max2175_drv_current_band[ntuner] = eTUNER_DRV_AM_MODE;
			max2175_drv_am_frequency[ntuner] = freq;
		}
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET max2175_getTune(uint32 *mod_mode, uint32 *curfreq, uint32 ntuner)
{
	RET ret = eRET_OK;

	MAXIM_DBG("[%s] \n", __func__);

	*mod_mode = max2175_drv_current_band[ntuner];
	if(*mod_mode == eTUNER_DRV_FM_MODE) {
		*curfreq = max2175_drv_fm_frequency[ntuner];
	}
	else if(*mod_mode == eTUNER_DRV_AM_MODE) {
		*curfreq = max2175_drv_am_frequency[ntuner];
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET max2175_getRFgain(int32 *rfdata, uint32 ntuner)

{
	RET ret = eRET_OK;

	MAXIM_DBG("[%s] \n", __func__);

	*rfdata = read_RSSI(ntuner);

	return ret;
}

RET max2175_setMute(uint32 fOnOff, uint32 ntuner)
{
	RET ret = eRET_OK;

	MAXIM_DBG("[%s] : fOnOff[%d] Not support.\n", __func__, fOnOff);

	return ret;
}

RET max2175_i2cTx(uint8 addr, uint8 *tx, uint32 len)
{
	RET ret = eRET_OK;

	MAXIM_DBG("[%s] : %02xh\n", __func__, addr);
	ret = (*pfnI2cTx)(addr, tx, len);

	return ret;
}

RET max2175_i2cRx(uint8 addr, uint8 *reg, uint32 reglen, uint8* rx, uint32 rxlen)
{
	RET ret = eRET_OK;

	MAXIM_DBG("[%s] : %02xh\n", __func__, addr);
	ret = (*pfnI2cRx)(addr, reg, reglen, rx, rxlen);

	return ret;
}

uint32 max2175_getVersion(void)
{
	return ((__MAX2175_DRIVER_VER_PREFIX__<<24) | (__MAX2175_DRIVER_VER_RELEASE_NUMBER__<<16) |
			(__MAX2175_DRIVER_VER_MAJOR_NUMBER__<<8) | (__MAX2175_DRIVER_VER_MINOR_NUMBER__<<0));
}

float32 max2175_getPreciseIqSampleRate(uint32 ntuner)
{
	float32 samplerate;
	samplerate = (float32)744187.5;
	return samplerate;
}

int32 max2175_getIqSampleRate(uint32 ntuner)
{
	int32 samplerate;
	samplerate = (int32)744187;
	return samplerate;
}

int32 max2175_getIqSamplingBit(uint32 ntuner)
{
	return (int32)20;
}

/*
이하 컴파일을 위한 함수들.
*/
//typedef enum Region { EUROPE_, NORTH_AMERICA_ } Region_t;
void init_tuner_MAX2175(int32 nTuner, Region_t area)
{
	MAXIM_DBG("################ %s : Not Used Function ################\n", __func__);
}

void Goto_RF_Frequency_MAX2175(int32 nTuner, double freq)
{
	MAXIM_DBG("################ %s : Not Used Function ################\n", __func__);
}

int32 mode(int32 nTuner, efsm_mode new_mode)
{
	MAXIM_DBG("################ %s : Not Used Function ################\n", __func__);
	return 0;
}

int32 dev_init_gpios(void)
{
	MAXIM_DBG("################ %s : Not Used Function ################\n", __func__);
	return 0;
}

#if 0
void dump_all_reg_val(int32 nTuner)
{
	MAXIM_DBG("################ %s : Not Used Function ################\n", __func__);
}

int32 read_register(int32 nTuner, U8 regindex)
{
	MAXIM_DBG("################ %s : Not Used Function ################\n", __func__);
	return 0;
}

int32 MAX2175_get_rssi(int32 nTuner, U8 band)
{
	MAXIM_DBG("################ %s : Not Used Function ################\n", __func__);
	return 0;
}

double read_RSSI(int nTuner)
{
	MAXIM_DBG("################ %s : Not Used Function ################\n", __func__);
	return 0;
}

void debug_app(int8 *argv[], int32 argc)
{
	MAXIM_DBG("################ %s : Not Used Function ################\n", __func__);
}
#endif
int dev_aout_alsa_open(void *arg,unsigned int	nSamplingRate)
{
	return 0;
}

int dev_aout_alsa_close(void *arg)
{
	return 0;
}

int dev_aout_alsa_start(void *arg)
{
	return 0;
}

int dev_aout_alsa_output(void *arg, unsigned char *pcm, unsigned int pcmlen, unsigned int sampling_frequecy)
{
	return 0;
}

int dev_aout_alsa_volume(void *arg,unsigned int volume)
{
	return 0;
}

int dev_i2c_init(unsigned char nTuner)
{
	return 0;
}

int dev_iq01_i2s_init(void)
{
	return 0;
}