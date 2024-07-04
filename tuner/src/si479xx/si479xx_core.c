/*******************************************************************************

*   FileName : si479xx_core.c

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
#include "si479xx_commanddefs.h"
#include "si479xx_hal.h"
#include "si479xx_core.h"

#define	__SI479XX_DRIVER_VER_PREFIX__			'S'
#define	__SI479XX_DRIVER_VER_RELEASE_NUMBER__	0x02U
#define	__SI479XX_DRIVER_VER_MAJOR_NUMBER__		0x00U
#define	__SI479XX_DRIVER_VER_MINOR_NUMBER__		0x00U

stTUNER_DRV_CONFIG_t gSilabConf;

uint32 si479xx_drv_initialized = 0;
uint32 si479xx_drv_current_band[SILAB_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};
uint32 si479xx_drv_fm_frequency[SILAB_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};
uint32 si479xx_drv_am_frequency[SILAB_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};
uint32 si479xx_drv_dab_index[SILAB_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};

static void si479xx_openGpio(void);
static void si479xx_reset(void);
RET si479xx_setBand(uint32 mod_mode, uint32 ntuner);

/*================================================
		Internal Functions For Application Programming Interface
==================================================*/
RET si479xx_open(stTUNER_DRV_CONFIG_t type)
{
	RET ret = eRET_NG_NO_RSC;
	uint32 numTuners=0;
	uint32 retrycnt=3;

	SILAB_DBG("[%s] \n", __func__);

	if(si479xx_drv_initialized != 0) {
		ret = eRET_NG_ALREADY_OPEN;
		return ret;
	}

	si479xx_drv_initialized = 0;
	si479xx_memset(si479xx_drv_current_band, 0xff, sizeof(si479xx_drv_current_band));
	si479xx_memset(si479xx_drv_fm_frequency, 0xff, sizeof(si479xx_drv_fm_frequency));
	si479xx_memset(si479xx_drv_am_frequency, 0xff, sizeof(si479xx_drv_am_frequency));
	si479xx_memset(si479xx_drv_dab_index, 0xff, sizeof(si479xx_drv_dab_index));

	gSilabConf = type;

	if(pfnSpiOpen != NULL && pfnSpiClose != NULL && pfnSpiTxRx != NULL) {
		ret = (RET)(*pfnSpiOpen)(0);
		if(ret == eRET_OK || ret == eRET_NG_ALREADY_OPEN) {
		#ifdef USE_THIRD_TUNER_AS_SINGLE_TUNER
			if(gSilabConf.numTuners != eTUNER_DRV_CONF_TYPE_DUAL) {
				ret = (RET)(*pfnSpiOpen)(1);
			}
			if(gSilabConf.numTuners == eTUNER_DRV_CONF_TYPE_SINGLE) {
				numTuners = eTUNER_DRV_CONF_TYPE_TRIPLE;
			}
			else {
				numTuners = gSilabConf.numTuners;
			}
			if(ret == eRET_OK || ret == eRET_NG_ALREADY_OPEN) {
				si479xx_openGpio();
				do {
					si479xx_reset();
					ret = si479xx_bootup(numTuners);
					if(ret == eRET_OK) {
						switch(numTuners) {
							case eTUNER_DRV_CONF_TYPE_QUAD:		si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_QUATERNARY);
							case eTUNER_DRV_CONF_TYPE_TRIPLE:	si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_TERTIARY);
							case eTUNER_DRV_CONF_TYPE_DUAL:		si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_SECONDARY);
							default:							si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_PRIMARY);	break;
						}
						si479xx_setIqProperty(numTuners);
						retrycnt = 0;
					}
					else {
						if(retrycnt > 0) {
							retrycnt--;
							SILAB_ERR("Failed to boot tuner and Tuner boot retry!!! Retry Count[%u]\n", retrycnt);
						}
						else {
							SILAB_ERR("Failed to boot tuner!!! Check the tuner control signal.\n");
						}
					}
				}while(retrycnt>0);
			}
		#else
			if(gSilabConf.numTuners > eTUNER_DRV_CONF_TYPE_DUAL) {
				ret = (RET)(*pfnSpiOpen)(1);
			}
			if(ret == eRET_OK || ret == eRET_NG_ALREADY_OPEN) {
				si479xx_openGpio();
				do {
					si479xx_reset();
					ret = si479xx_bootup(gSilabConf.numTuners);
					if(ret == eRET_OK) {
						switch(type.numTuners) {
							case eTUNER_DRV_CONF_TYPE_QUAD:
								si479xx_setBand(type.initMode, eTUNER_DRV_ID_PRIMARY);
								si479xx_setBand(type.initMode, eTUNER_DRV_ID_SECONDARY);
								si479xx_setBand(type.initMode, eTUNER_DRV_ID_TERTIARY);
								si479xx_setBand(type.initMode, eTUNER_DRV_ID_QUATERNARY);
								si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_PRIMARY);
								si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_SECONDARY);
								si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_TERTIARY);
								si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_QUATERNARY);
								break;
							case eTUNER_DRV_CONF_TYPE_TRIPLE:
								si479xx_setBand(type.initMode, eTUNER_DRV_ID_PRIMARY);
								si479xx_setBand(type.initMode, eTUNER_DRV_ID_SECONDARY);
								si479xx_setBand(type.initMode, eTUNER_DRV_ID_TERTIARY);
								si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_PRIMARY);
								si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_SECONDARY);
								si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_TERTIARY);
								break;
							case eTUNER_DRV_CONF_TYPE_DUAL:
								si479xx_setBand(type.initMode, eTUNER_DRV_ID_PRIMARY);
								si479xx_setBand(type.initMode, eTUNER_DRV_ID_SECONDARY);
								si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_PRIMARY);
								si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_SECONDARY);
								break;
							default:
								si479xx_setBand(type.initMode, eTUNER_DRV_ID_PRIMARY);
								si479xx_setTune(type.initMode, type.initFreq, eTUNER_DRV_TUNE_NORMAL, eTUNER_DRV_ID_PRIMARY);
								break;
						}
						si479xx_setIqProperty(gSilabConf.numTuners);
						retrycnt = 0;
					}
					else {
						if(retrycnt > 0) {
							retrycnt--;
							SILAB_ERR("Failed to boot tuner and tuner boot retry!!! Retry Count[%u]\n", retrycnt);
						}
						else {
							SILAB_ERR("Failed to boot tuner!!! Check the tuner control signal.\n");
						}
					}
				}while(retrycnt>0);
			}
		#endif
		}
	}


	if (ret == eRET_OK) {
		si479xx_drv_initialized = 1;
		SILAB_DBG("[si479xx_open] Tuner open succeed.!!!\n");
	}
	else  {
		si479xx_drv_initialized = 0;
		SILAB_DBG("[si479xx_open] Failed to open tuner!!!\n");
	}

	return ret;
}

RET si479xx_close(void)
{
	RET ret = eRET_OK;

	if(pfnSpiClose != NULL) {
		(*pfnSpiClose)(0);
	#ifdef USE_THIRD_TUNER_AS_SINGLE_TUNER
		if(gSilabConf.numTuners != eTUNER_DRV_CONF_TYPE_DUAL) {
			(*pfnSpiClose)(1);
		}
	#else
		if(gSilabConf.numTuners > eTUNER_DRV_CONF_TYPE_DUAL) {
			(*pfnSpiClose)(1);
		}
	#endif
	}
	if(pfnTunerPower != NULL)	(*pfnTunerPower)(OFF);
	if(pfnAntPower != NULL)		(*pfnAntPower)(OFF);
	if(pfnTunerReset != NULL)	(*pfnTunerReset)(OFF);
	if(pfnRadioGpioClose != NULL)	(*pfnRadioGpioClose)();

	si479xx_drv_initialized = 0;
	si479xx_memset(si479xx_drv_current_band, 0xff, sizeof(si479xx_drv_current_band));
	si479xx_memset(si479xx_drv_fm_frequency, 0xff, sizeof(si479xx_drv_fm_frequency));
	si479xx_memset(si479xx_drv_am_frequency, 0xff, sizeof(si479xx_drv_am_frequency));
	si479xx_memset(si479xx_drv_dab_index, 0xff, sizeof(si479xx_drv_dab_index));

	return ret;
}

uint8 si479xx_getTunerCh(uint32 ntuner)
{
	uint8 ret = 0;

	if(ntuner == eTUNER_DRV_ID_SECONDARY || ntuner == eTUNER_DRV_ID_QUATERNARY) {
		ret = 0x01;
	}

	return ret;
}

uint8 si479xx_getMaxDabFreqList(void)
{
	return si479xx_getMaxDabFreqCount();
}

uint8 si479xx_getCurrentDabFreqList(uint32 ntuner)
{
	return si479xx_getCurrentDabFreqCount(ntuner);
}

RET si479xx_setDabFreqList(uint32 *dab_freq_hz_table, uint8 num_freq, uint32 ntuner)
{
	RET ret = eRET_OK;

	if(gSilabConf.sdr == eTUNER_SDR_DAB) {
		if(ntuner == eTUNER_DRV_ID_PRIMARY) {
			si479xx_setDabFreqListToReg(dab_freq_hz_table, num_freq, eTUNER_DRV_ID_PRIMARY);
		}
		else {
			ret = eRET_NG_INVALID_ID;
		}
	}
	else {
		ret = eRET_NG_NOT_SUPPORT;
	}

	return ret;
}

RET si479xx_getDabFreqList(uint32 *dab_freq_hz_table, uint8 *num_freq, uint32 ntuner)
{
	RET ret = eRET_OK;

	if(gSilabConf.sdr == eTUNER_SDR_DAB) {
		if(ntuner == eTUNER_DRV_ID_PRIMARY) {
			si479xx_getDabFreqListFromReg(dab_freq_hz_table, num_freq, eTUNER_DRV_ID_PRIMARY);
		}
		else if(ntuner == eTUNER_DRV_ID_TERTIARY) {
			si479xx_getDabFreqListFromReg(dab_freq_hz_table, num_freq, eTUNER_DRV_ID_TERTIARY);
		}
		else {
			ret = eRET_NG_INVALID_ID;
		}
	}
	else {
		ret = eRET_NG_NOT_SUPPORT;
	}

	return ret;
}

RET si479xx_setBand(uint32 mod_mode, uint32 ntuner)
{
	RET ret = eRET_OK;

	SILAB_DBG("[%s] ", __func__);

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

#ifdef USE_THIRD_TUNER_AS_SINGLE_TUNER
	if(gSilabConf.numTuners == eTUNER_DRV_CONF_TYPE_SINGLE) {
		if(ntuner == eTUNER_DRV_ID_PRIMARY) {
			ntuner = eTUNER_DRV_ID_TERTIARY;
		}
	}
#endif

	cmd_buff[0] = 0xC0;
	cmd_buff[1] = si479xx_getTunerCh(ntuner);
	cmd_buff[2] = 0x00;
	cmd_buff[3] = 0x00;

	cmd_buff[4] = SET_MODE;
	if(mod_mode == eTUNER_DRV_FM_MODE) {
		cmd_buff[5] = 0;
	}
	else if(mod_mode == eTUNER_DRV_AM_MODE) {
		cmd_buff[5] = 1;
	}
	else if(mod_mode == eTUNER_DRV_DAB_MODE) {
		if(gSilabConf.sdr == eTUNER_SDR_DAB) {
			cmd_buff[5] = 2;
		}
		else {
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else {
		cmd_buff[5] = 7;
	}

	if(ret == eRET_OK) {
		ret = si479xx_command(6, cmd_buff, 4, rsp_buff, ntuner);
		if(ret == eRET_OK) {
			//wait for the STC bit set
			if (cmd_buff[5] == eTUNER_DRV_FM_MODE || cmd_buff[5] == eTUNER_DRV_AM_MODE || cmd_buff[5] == eTUNER_DRV_DAB_MODE) {
				ret = si479xx_waitForSTC(ntuner);
			}

			if(ret == eRET_OK) {
				si479xx_drv_current_band[ntuner] = (uint32)cmd_buff[5];
				if(gSilabConf.sdr == eTUNER_SDR_HD) {
					if(mod_mode == eTUNER_DRV_FM_MODE) {
						// The analog audio be scaled down appropriately at the source (Tuner) by approx. -3.3dB only for FM in the TCC803x EVB.
						si479xx_setTunerProperty(0x07, 0x11, 0x0A, 0x60, eTUNER_DRV_ID_PRIMARY);
					}
					else if(mod_mode == eTUNER_DRV_AM_MODE) {
						// The analog audio be scaled down appropriately at the source (Tuner) by approx. 0.2dB only for AM in the TCC803x EVB.
						si479xx_setTunerProperty(0x07, 0x11, 0x0F, 0x40, eTUNER_DRV_ID_PRIMARY);
					}
					else {
						;
					}
				}
			}
		}
	}

	return ret;
}

RET si479xx_setTune(uint32 mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner)
{
	RET ret = eRET_OK;
	uint16 tfreq=0;

	SILAB_DBG("[%s] ", __func__);

#ifdef USE_THIRD_TUNER_AS_SINGLE_TUNER
	if(gSilabConf.numTuners == eTUNER_DRV_CONF_TYPE_SINGLE) {
		if(ntuner == eTUNER_DRV_ID_PRIMARY) {
			ntuner = eTUNER_DRV_ID_TERTIARY;
		}
	}
#endif

	if((gSilabConf.sdr != eTUNER_SDR_DAB && mod_mode == eTUNER_DRV_DAB_MODE) || (mod_mode > eTUNER_DRV_DAB_MODE)) {
		return eRET_NG_INVALID_PARAM;
	}

	if(si479xx_drv_current_band[ntuner] != mod_mode) {
		si479xx_setBand(mod_mode, ntuner);
	}

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	cmd_buff[0] = 0xC0;
	cmd_buff[1] = si479xx_getTunerCh(ntuner);
	cmd_buff[2] = 0x00;
	cmd_buff[3] = 0x00;

	if(mod_mode == eTUNER_DRV_AM_MODE) {
		cmd_buff[4] = AM_TUNE_FREQ;
		tfreq = (uint16)freq;
		cmd_buff[5] = (tune_mode & 0x07) << 4;
	}
	else if(mod_mode == eTUNER_DRV_FM_MODE) {
		cmd_buff[4] = FM_TUNE_FREQ;
		tfreq = (uint16)(freq/10);
		cmd_buff[5] = (tune_mode & 0x07) << 4;
	}
	else {
		cmd_buff[4] = DAB_TUNER_FREQ;
		tfreq = freq;	// It's an index, not frequency.
		cmd_buff[5] = 0x01;
	}

#if SILAB_FW_VER == 50
	if(mod_mode == eTUNER_DRV_DAB_MODE) {
		if((freq >= 0) && (freq <= si479xx_getMaxDabFreqCount())) {	// DAB Index Mode: Range 0 ~ 64
			si479xx_32bto8b(cmd_buff+8, si479xx_getDabFreqFromList(freq, ntuner));
		}
		else if((freq >= 170000000) && (freq <= 240000000)) {	// DAB Freq Mode: Range 170MHz ~ 240MHz
			si479xx_32bto8b(cmd_buff+8, freq);
		}
		else {
			ret = eRET_NG_INVALID_PARAM;
		}

		if(ret == eRET_OK) {
			ret = si479xx_command(12, cmd_buff, 4, rsp_buff, ntuner);
		}
	}
	else {
		si479xx_16bto8b(cmd_buff+6, tfreq);
		ret = si479xx_command(8, cmd_buff, 4, rsp_buff, ntuner);
	}
#else
	si479xx_16bto8b(cmd_buff+6, tfreq);
	ret = si479xx_command(8, cmd_buff, 4, rsp_buff, ntuner);
#endif

	if(ret == eRET_OK) {
		//wait for the STC bit set
		ret = si479xx_waitForSTC(ntuner);
		if(ret == eRET_OK) {
			if(mod_mode == eTUNER_DRV_AM_MODE) {
				si479xx_drv_am_frequency[ntuner] = freq;
			}
			else if(mod_mode == eTUNER_DRV_FM_MODE) {
				si479xx_drv_fm_frequency[ntuner] = freq;
			}
			else {
				si479xx_drv_dab_index[ntuner] = freq;
			}
		}
	}
	return ret;
}

RET si479xx_getTune(uint32 *mod_mode, uint32 *curfreq, uint32 ntuner)
{
	RET ret = eRET_OK;

	SILAB_DBG("[%s] ", __func__);

#ifdef USE_THIRD_TUNER_AS_SINGLE_TUNER
	if(gSilabConf.numTuners == eTUNER_DRV_CONF_TYPE_SINGLE) {
		if(ntuner == eTUNER_DRV_ID_PRIMARY) {
			ntuner = eTUNER_DRV_ID_TERTIARY;
		}
	}
#endif

	*mod_mode = si479xx_drv_current_band[ntuner];
	if(*mod_mode == eTUNER_DRV_AM_MODE) {
		*curfreq = si479xx_drv_am_frequency[ntuner];
	}
	else if(*mod_mode == eTUNER_DRV_FM_MODE) {
		*curfreq = si479xx_drv_fm_frequency[ntuner];
	}
	else {
		*curfreq = si479xx_drv_dab_index[ntuner];
	}

	return ret;
}

RET si479xx_getQuality(uint32 mod_mode, stSILAB_DRV_QUALITY_t *qdata, uint32 ntuner)
{
	RET ret = eRET_OK, i;

	SILAB_DBG("[%s] ", __func__);

#ifdef USE_THIRD_TUNER_AS_SINGLE_TUNER
	if(gSilabConf.numTuners == eTUNER_DRV_CONF_TYPE_SINGLE) {
		if(ntuner == eTUNER_DRV_ID_PRIMARY) {
			ntuner = eTUNER_DRV_ID_TERTIARY;
		}
	}
#endif

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));
	si479xx_memset(rsp_buff, 0, sizeof(rsp_buff));

	if(mod_mode == eTUNER_DRV_AM_MODE) {
		cmd_buff[0] = 0xC0;
		cmd_buff[1] = si479xx_getTunerCh(ntuner);
		cmd_buff[2] = 0x00;
		cmd_buff[3] = 0x00;
		cmd_buff[4] = AM_RSQ_STATUS;
		cmd_buff[5] = 0x01;
		ret = si479xx_command(6, cmd_buff, 34, rsp_buff, ntuner);

		qdata->am.Flag = rsp_buff[4];
		qdata->am.Valid = rsp_buff[5];
		qdata->am.Freq = ((uint32)rsp_buff[7]) << 8 | ((uint32)rsp_buff[6]);
		qdata->am.Rssi = si479xx_u8btou32b(rsp_buff[9]);
		qdata->am.Snr = si479xx_u8btou32b(rsp_buff[10]);
		qdata->am.Mod = rsp_buff[25];
		qdata->am.Offs = si479xx_u8btou32b(rsp_buff[8]);
		qdata->am.Hdlevel = rsp_buff[29];

	#if 0		// for debugging
		SILAB_DBG("======= Read AM_RSQ_STATUS =======\n");
		for(i=0; i<30; i++) {
			printf("[%d]%02xh ", i, rsp_buff[i+4]);
		}
		printf("\n");

		SILAB_DBG("FREQ[%d] FLAG[%02xh] VALID[%02xh] RSSI[%d] SNR[%d] MOD[%d] OFFS[%d] HDLEVEL[%d]\n",
							qdata->am.Freq, qdata->am.Flag, qdata->am.Valid, qdata->am.Rssi, qdata->am.Snr, qdata->am.Mod,
							qdata->am.Offs,	qdata->am.Hdlevel);
	#endif

	}
	else if(mod_mode == eTUNER_DRV_FM_MODE){
		cmd_buff[0] = 0xC0;
		cmd_buff[1] = si479xx_getTunerCh(ntuner);
		cmd_buff[2] = 0x00;
		cmd_buff[3] = 0x00;
		cmd_buff[4] = FM_RSQ_STATUS;
		cmd_buff[5] = 0x01;
		ret = si479xx_command(6, cmd_buff, 42, rsp_buff, ntuner);

		qdata->fm.Flag = rsp_buff[4];
		qdata->fm.Valid = rsp_buff[5];
		qdata->fm.Freq = ((uint32)rsp_buff[7]) << 8 | ((uint32)rsp_buff[6]);
		qdata->fm.Rssi = si479xx_u8btou32b(rsp_buff[9]);
		qdata->fm.Snr = si479xx_u8btou32b(rsp_buff[10]);
		qdata->fm.Dev = rsp_buff[20];
		qdata->fm.Offs = si479xx_u8btou32b(rsp_buff[8]);
		qdata->fm.Pilot = rsp_buff[26];
		qdata->fm.Mpth = rsp_buff[18];
		qdata->fm.Usn = rsp_buff[19];
		qdata->fm.Hdlevel = rsp_buff[27];

	#if 0		// for debugging
		SILAB_DBG("======= Read FM_RSQ_STATUS =======\n");
		for(i=0; i<38; i++) {
			printf("[%d]%02xh ", i, rsp_buff[i+4]);
		}
		printf("\n");

		SILAB_DBG("FREQ[%d] FLAG[%02xh] VALID[%02xh] RSSI[%d] SNR[%d] DEV[%d] OFFS[%d] PILOT[%d] MPTH[%d] USN[%d] HDLEVEL[%d]\n",
							qdata->fm.Freq, qdata->fm.Flag, qdata->fm.Valid, qdata->fm.Rssi, qdata->fm.Snr, qdata->fm.Dev,
							qdata->fm.Offs,	qdata->fm.Pilot, qdata->fm.Mpth, qdata->fm.Usn, qdata->fm.Hdlevel);
	#endif
	}
	else if(mod_mode == eTUNER_DRV_DAB_MODE){
		if(gSilabConf.sdr == eTUNER_SDR_DAB) {
			cmd_buff[0] = 0xC0;
			cmd_buff[1] = si479xx_getTunerCh(ntuner);
			cmd_buff[2] = 0x00;
			cmd_buff[3] = 0x00;
			cmd_buff[4] = DAB_RSQ_STATUS;
			cmd_buff[5] = 0x01;
			ret = si479xx_command(6, cmd_buff, 17, rsp_buff, ntuner);

			qdata->dab.Flag = rsp_buff[4];
			qdata->dab.Valid = rsp_buff[5];
			qdata->dab.Index = ((uint32)rsp_buff[7]) << 8 | ((uint32)rsp_buff[6]);
			qdata->dab.Freq = ((uint32)rsp_buff[11]) << 24 | ((uint32)rsp_buff[10]) << 16 | ((uint32)rsp_buff[9]) << 8 | ((uint32)rsp_buff[8]);
			qdata->dab.Sqi = si479xx_u8btou32b(rsp_buff[12]);
			qdata->dab.Rssi = si479xx_u8btou32b(rsp_buff[13]);
			qdata->dab.Detect = rsp_buff[14];
			qdata->dab.RssiAdj = si479xx_u8btou32b(rsp_buff[15]);
			qdata->dab.Dagc = si479xx_u8btou32b(rsp_buff[16]);
		#if 0		// for debugging
				SILAB_DBG("======= Read DAB_RSQ_STATUS =======\n");
				for(i=0; i<13; i++) {
					printf("[%d]%02xh ", i, rsp_buff[i+4]);
				}
				printf("\n");
				SILAB_DBG("FREQ[%d] INDEX[%d] VALID[0x%02x] RSSI[%d] SQI[%d] DETECT[%d] RSSIADJ[%d] DAGC[%d]\n",
									qdata->dab.Freq, qdata->dab.Index, qdata->dab.Flag, qdata->dab.Rssi, qdata->dab.Sqi, qdata->dab.Detect,
									qdata->dab.RssiAdj, qdata->dab.Dagc);
		#endif
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

RET si479xx_setMute(uint32 fOnOff, uint32 ntuner)
{
	RET ret = eRET_OK;

	SILAB_DBG("[%s] ", __func__);

#ifdef USE_THIRD_TUNER_AS_SINGLE_TUNER
	if(gSilabConf.numTuners == eTUNER_DRV_CONF_TYPE_SINGLE) {
		if(ntuner == eTUNER_DRV_ID_PRIMARY) {
			ntuner = eTUNER_DRV_ID_TERTIARY;
		}
	}
#endif

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	cmd_buff[0] = 0xC0;
	cmd_buff[1] = si479xx_getTunerCh(ntuner);
	cmd_buff[2] = 0x00;
	cmd_buff[3] = 0x00;

	cmd_buff[4] = SET_PROPERTY;
	cmd_buff[5] = 0x00;
	cmd_buff[6] = 0x01;
	cmd_buff[7] = 0x07;
	if(fOnOff == 1) {
		cmd_buff[8] = 0x0F;
	}
	else {
		cmd_buff[8] = 0x00;
	}
	cmd_buff[9] = 0x00;

	ret = si479xx_command(10, cmd_buff, 4, rsp_buff, ntuner);

	return ret;
}

RET si479xx_setRdsConfig(uint32 fOnOff, uint32 blethB, uint32 blethCD, uint32 ntuner)
{
	RET ret = eRET_OK;
	uint8 rdscfg=0;

	SILAB_DBG("[%s:%d] ", __func__, __LINE__);

#ifdef USE_THIRD_TUNER_AS_SINGLE_TUNER
	if(gSilabConf.numTuners == eTUNER_DRV_CONF_TYPE_SINGLE) {
		if(ntuner == eTUNER_DRV_ID_PRIMARY) {
			ntuner = eTUNER_DRV_ID_TERTIARY;
		}
	}
#endif

	if(fOnOff == 1) {
		rdscfg = 1;
		if(blethB < 4) {
			rdscfg |= (uint8)(blethB<<6);
		}
		if(blethCD < 4) {
			rdscfg |= (uint8)(blethCD<<4);
		}
		SILAB_DBG("[rdscfg:0x%02x]\n", rdscfg);
		ret = si479xx_setProperty(0x2a, 0x02, 0x00, rdscfg, ntuner);
	}
	else {
		ret = si479xx_setProperty(0x2a, 0x02, 0x00, 0x00, ntuner);
	}
	return ret;
}

RET si479xx_getRdsData(uint8 *rdsbuf, uint32 ntuner)
{
	RET ret = eRET_OK;
//	SILAB_DBG("[%s:%d] ntuner[%d]\n", __func__, __LINE__, ntuner);

#ifdef USE_THIRD_TUNER_AS_SINGLE_TUNER
	if(gSilabConf.numTuners == eTUNER_DRV_CONF_TYPE_SINGLE) {
		if(ntuner == eTUNER_DRV_ID_PRIMARY) {
			ntuner = eTUNER_DRV_ID_TERTIARY;
		}
	}
#endif

	if(rdsbuf != NULL) {
		si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

		cmd_buff[0] = 0xC0;
		cmd_buff[1] = si479xx_getTunerCh(ntuner);
		cmd_buff[2] = 0x00;
		cmd_buff[3] = 0x00;
		cmd_buff[4] = FM_RDS_STATUS;
		cmd_buff[5] = 0x01;

		ret = si479xx_command(6, cmd_buff, 20, rsp_buff, ntuner);

		if(ret == eRET_OK) {
			(void)si479xx_memcpy(rdsbuf, rsp_buff+4, 16);
		}
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET si479xx_setRdsClear(uint32 ntuner)
{
	RET ret = eRET_OK;
	SILAB_DBG("[%s:%d] ntuner[%d]\n", __func__, __LINE__, ntuner);

#ifdef USE_THIRD_TUNER_AS_SINGLE_TUNER
	if(gSilabConf.numTuners == eTUNER_DRV_CONF_TYPE_SINGLE) {
		if(ntuner == eTUNER_DRV_ID_PRIMARY) {
			ntuner = eTUNER_DRV_ID_TERTIARY;
		}
	}
#endif

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	cmd_buff[0] = 0xC0;
	cmd_buff[1] = si479xx_getTunerCh(ntuner);
	cmd_buff[2] = 0x00;
	cmd_buff[3] = 0x00;
	cmd_buff[4] = FM_RDS_STATUS;
	cmd_buff[5] = 0x03;

	ret = si479xx_command(6, cmd_buff, 4, rsp_buff, ntuner);

	return ret;
}

RET si479xx_cspiTxRxData(uint8 *tx, uint8 *rx, uint32 len, uint32 cs)
{
	RET ret = eRET_OK;
	SILAB_DBG("[%s] len=%d, cs=%d\n", __func__, len, cs);

#ifdef USE_THIRD_TUNER_AS_SINGLE_TUNER
	if(gSilabConf.numTuners == eTUNER_DRV_CONF_TYPE_SINGLE) {
		if(cs == 0) {
			cs = 1;
		}
		else {
			return eRET_NG_NOT_SUPPORT;
		}
	}
#endif

	ret = (*pfnSpiTxRx)(tx, rx, len, cs);
	return ret;
}

uint32 si479xx_getVersion(void)
{
	return (((uint32)__SI479XX_DRIVER_VER_PREFIX__<<24) | ((uint32)__SI479XX_DRIVER_VER_RELEASE_NUMBER__<<16) |
			((uint32)__SI479XX_DRIVER_VER_MAJOR_NUMBER__<<8) | ((uint32)__SI479XX_DRIVER_VER_MINOR_NUMBER__<<0));
}

float32 si479xx_getPreciseIqSampleRate(uint32 ntuner)
{
	float samplerate;
	if(gSilabConf.sdr == eTUNER_SDR_DRM30) {
	#if SILAB_FW_VER >= (40)
		if(si479xx_drv_current_band[ntuner] == eTUNER_DRV_AM_MODE) {
			samplerate = (float)192000.0;
		}
		else {
			samplerate = (float)744187.5;
		}
	#else
		samplerate = (float)744187.5;
	#endif
	}
	else if(gSilabConf.sdr == eTUNER_SDR_DRMP) {
	#if SILAB_FW_VER >= (40)
		if(si479xx_drv_current_band[ntuner] == eTUNER_DRV_AM_MODE) {
			samplerate = (float)192000.0;
		}
		else {
			samplerate = (float)744187.5;
		}
	#else
		samplerate = (float)744187.5;
	#endif
	}
	else if(gSilabConf.sdr == eTUNER_SDR_DAB) {
		if(si479xx_drv_current_band[ntuner] == eTUNER_DRV_DAB_MODE) {
			samplerate = (float)2048000.0;
		}
		else {
			samplerate = (float)744187.5;
		}
	}
	else {
		if(gSilabConf.fIqOut != (uint32)0) {
			samplerate = (int32)744187.5;			// default samplerate
		}
		else {
			samplerate = (float)0.0;
		}
	}
	return samplerate;
}

int32 si479xx_getIqSampleRate(uint32 ntuner)
{
	int32 samplerate;
	if(gSilabConf.sdr == eTUNER_SDR_DRM30) {
	#if SILAB_FW_VER >= (40)
		if(si479xx_drv_current_band[ntuner] == eTUNER_DRV_AM_MODE) {
			samplerate = (int32)192000;
		}
		else {
			samplerate = (int32)744187;
		}
	#else
		samplerate = (int32)744187;
	#endif
	}
	else if(gSilabConf.sdr == eTUNER_SDR_DRMP) {
	#if SILAB_FW_VER >= (40)
		if(si479xx_drv_current_band[ntuner] == eTUNER_DRV_AM_MODE) {
			samplerate = (int32)192000;
		}
		else {
			samplerate = (int32)744187;
		}
	#else
		samplerate = (int32)744187;
	#endif
	}
	else if(gSilabConf.sdr == eTUNER_SDR_DAB) {
		if(si479xx_drv_current_band[ntuner] == eTUNER_DRV_DAB_MODE) {
			samplerate = (int32)2048000;
		}
		else {
			samplerate = (int32)744187;
		}
	}
	else {
		if(gSilabConf.fIqOut != (uint32)0) {
			samplerate = (int32)744187;			// default samplerate
		}
		else {
			samplerate = (int32)-1;
		}
	}
	return samplerate;
}

int32 si479xx_getIqSamplingBit(uint32 ntuner)
{
	return (int32)16;
}

static void si479xx_openGpio(void)
{
	if(pfnRadioGpioOpen != NULL) {
		(*pfnRadioGpioOpen)();
	}
}

static void si479xx_reset(void)
{
	if(pfnTunerPower != NULL)		(*pfnTunerPower)(ON);
	if(pfnAntPower != NULL)			(*pfnAntPower)(ON);
	si479xx_mwait(20);
	if(pfnTunerReset != NULL)		(*pfnTunerReset)(OFF);
	si479xx_mwait(20);
	if(pfnTunerReset != NULL)		(*pfnTunerReset)(ON);
	si479xx_mwait(20);
}

uint8 si479xx_getErrorCodes(uint32 ntuner)
{
	return si479xx_getError(ntuner);
}

RET si479xx_setIQTestPattern(uint32 fOnOff, uint32 sel)
{
	return si479xx_setIQTestPatternControl(fOnOff, sel);
}

RET si479xx_getStatus(uint32 ntuner, uint32 cmd, uint8 *rx, uint8 len)
{
	int i;
	uint32 cmd_len=0, add_len=0;
	uint8 rcmd=(uint8)(cmd >> 8);
	uint8 id=(uint8)cmd;
	RET ret = eRET_OK;

	if(rx == NULL && len == 0) {
		return eRET_NG_INVALID_PARAM;
	}

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	switch(rcmd) {
		case 0x17:
		case 0x0D:
			cmd_len = 2;
			add_len = 0;
			cmd_buff[0] = rcmd;	// Command
			cmd_buff[1] = id;	// ID or TYPE or None
			break;

		default:
			cmd_len = 6;
			add_len = 4;
			cmd_buff[0] = 0xC0;
			cmd_buff[1] = si479xx_getTunerCh(ntuner);
			cmd_buff[2] = 0x00;
			cmd_buff[3] = 0x00;
			cmd_buff[4] = rcmd;	// Command
			cmd_buff[5] = id;	// ID or TYPE or None
			break;
	}

	ret = si479xx_command(cmd_len, cmd_buff, len + add_len, rsp_buff, ntuner);

	if(ret == eRET_OK) {
		si479xx_memcpy(rx, rsp_buff+ add_len, len);
	}

	return ret;
}

RET si479xx_setCommand(uint32 cmd_size, uint8 *cmd, uint32 reply_size, uint8 *reply, uint32 ntuner)
{
	RET ret;
	ret = si479xx_command(cmd_size, cmd, reply_size, reply, ntuner);
	return ret;
}

/*
	fOnOff - 0: Off, 1: On, 2: Freeze for test, 3: Fixed Gain for test
	target_power - When fOnOff is 1, target_power range is 0 dBFS to 32 dBFS. (default: 24dBFS)
	               When fOnOff is 3, The target_power is used as a fixed gain. The range is -128 dBr to 127 dBr.
	inc_ms - Digital AGC loop time constant to increase gain(ms) only for DAB. 4ms ~ 255ms (default: 12ms)
	dec_ms - Digital AGC loop time constant to decrease gain(ms) only for DAB. 4ms ~ 255ms (default: 12ms)
	ntuner - Primary(0) or Tertiary(2) tuner
*/
RET si479xx_setDigitalAGC(uint8 fOnOff, int8 target_power, uint8 inc_ms, uint8 dec_ms, uint32 ntuner)
{
	RET ret = eRET_OK;
#if SILAB_FW_VER >= (40)
	if(gSilabConf.sdr == eTUNER_SDR_DAB) {
		if(fOnOff == 0) {
			// Disable
			ret = si479xx_setProperty(0x65, 0x00, 0x00, 0x13, ntuner);
		}
		else if(fOnOff == 1) {
			// Enable
			if(target_power >= 0 && target_power <= 32) {
				if(inc_ms >= 4 && dec_ms >= 4) {
					ret = si479xx_setProperty(0x65, 0x00, 0x00, 0x10, ntuner);
					if(ret == eRET_OK) {
						ret = si479xx_setProperty(0x65, 0x01, target_power, 0x00, ntuner);
						if(ret == eRET_OK) {
							si479xx_setProperty(0x65, 0x02, inc_ms, dec_ms, ntuner);
						}
					}
				}
				else {
					ret = eRET_NG_INVALID_PARAM;
				}
			}
			else {
				ret = eRET_NG_INVALID_PARAM;
			}
		}
		else if(fOnOff == 2) {
			// Freeze
			ret = si479xx_setProperty(0x65, 0x00, 0x00, 0x11, ntuner);
		}
		else if(fOnOff == 3) {
			// Fixed Gain
			ret = si479xx_setProperty(0x65, 0x00, target_power, 0x12, ntuner);
		}
		else {
			// parameter error
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else if(gSilabConf.sdr == eTUNER_SDR_DRM30) {
		if(fOnOff == 0) {
			// Disable
			ret = si479xx_setProperty(0x4d, 0x00, 0x00, 0x03, ntuner);
		}
		else if(fOnOff == 1) {
			// Enable
			if(target_power >= 6 && target_power <= 64) {
				if(inc_ms >= 4 && dec_ms >= 4) {
					ret = si479xx_setProperty(0x4d, 0x00, 0x00, 0x00, ntuner);
					if(ret == eRET_OK) {
						ret = si479xx_setProperty(0x4d, 0x01, target_power, 0x00, ntuner);
						//si479xx_setProperty(0x4d, 0x02, inc_ms, dec_ms, ntuner);
					}
				}
				else {
					ret = eRET_NG_INVALID_PARAM;
				}
			}
			else {
				ret = eRET_NG_INVALID_PARAM;
			}
		}
		else if(fOnOff == 2) {
			// Freeze
			ret = si479xx_setProperty(0x4d, 0x00, 0x00, 0x01, ntuner);
		}
		else if(fOnOff == 3) {
			// Fixed Gain
			if(target_power >= -32 && target_power <= 31) {
				ret = si479xx_setProperty(0x4d, 0x00, target_power, 0x02, ntuner);
			}
			else {
				ret = eRET_NG_INVALID_PARAM;
			}
		}
		else {
			// parameter error
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else if(gSilabConf.sdr == eTUNER_SDR_HD) {
		if(fOnOff == 0) {
			// Disable
			ret = si479xx_setProperty(0x2d, 0x00, 0x00, 0x03, ntuner);
		}
		else if(fOnOff == 1) {
			// Enable
			if(target_power >= 6 && target_power <= 64) {
				if(inc_ms >= 4 && dec_ms >= 4) {
					ret = si479xx_setProperty(0x2d, 0x00, 0x00, 0x00, ntuner);
					if(ret == eRET_OK) {
						ret = si479xx_setProperty(0x2d, 0x01, target_power, 0x00, ntuner);
						//si479xx_setProperty(0x2d, 0x02, inc_ms, dec_ms, ntuner);
					}
				}
				else {
					ret = eRET_NG_INVALID_PARAM;
				}
			}
			else {
				ret = eRET_NG_INVALID_PARAM;
			}
		}
		else if(fOnOff == 2) {
			// Freeze
			ret = si479xx_setProperty(0x2d, 0x00, 0x00, 0x01, ntuner);
		}
		else if(fOnOff == 3) {
			// Fixed Gain
			if(target_power >= -32 && target_power <= 31) {
				ret = si479xx_setProperty(0x2d, 0x00, target_power, 0x02, ntuner);
			}
			else {
				ret = eRET_NG_INVALID_PARAM;
			}
		}
		else {
			// parameter error
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else {
		(void)(ret);
	}
#endif
	return ret;
}

RET si479xx_setPropertyOfPrimaryTuner(uint8 group, uint8 number, uint8 hdata, uint8 ldata)
{
	RET ret = eRET_OK;

	ret = si479xx_setProperty(group, number, hdata, ldata, 0);

	return ret;
}

#ifdef INCLUDE_ONE_TUNER_DRIVER
RET tunerdrv_open(stTUNER_DRV_CONFIG_t type)
{
	return si479xx_open(type);
}

RET tunerdrv_close(void)
{
	return si479xx_close();
}

RET tunerdrv_setDabFreqList(uint32 *dab_freq_hz_table, uint8 num_freq, uint32 ntuner)
{
	return si479xx_setDabFreqList(dab_freq_hz_table, num_freq, ntuner);
}

RET tunerdrv_getDabFreqList(uint32 *dab_freq_hz_table, uint8 *num_freq, uint32 ntuner)
{
	return si479xx_getDabFreqList(dab_freq_hz_table, num_freq, ntuner);
}

RET tunerdrv_setTune(uint32 mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner)
{
	return si479xx_setTune(mod_mode, freq, tune_mode, ntuner);
}

RET tunerdrv_getFreq(uint32 *mod_mode, uint32 *curfreq, uint32 ntuner)
{
	return si479xx_getTune(mod_mode, curfreq, ntuner);
}

RET tunerdrv_getQuality(uint32 mod_mode, void *qdata, uint32 ntuner)
{
	stSILAB_DRV_QUALITY_t *tempQ =  (stSILAB_DRV_QUALITY_t *)qdata;

	return si479xx_getQuality(mod_mode, tempQ, ntuner);
}

RET tunerdrv_setMute(uint32 fOnOff, uint32 ntuner)
{
	return si479xx_setMute(fOnOff, ntuner);
}

RET tunerdrv_i2cTx(uint8 addr, uint8 *tx, uint32 len)
{
	return eRET_NG_NOT_SUPPORT;
}

RET tunerdrv_i2cRx(uint8 addr, uint8 *reg, uint32 reglen, uint8* rx, uint32 rxlen)
{
	return eRET_NG_NOT_SUPPORT;
}

RET tunerdrv_cspiTxRxData(uint8 *tx, uint8 *rx, uint32 len, uint32 cs)
{
	return si479xx_cspiTxRxData(tx, rx, len, cs);
}

uint32 tunerdrv_getVersion(void)
{
	return si479xx_getVersion();
}
#endif

