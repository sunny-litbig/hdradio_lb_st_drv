/*******************************************************************************

*   FileName : tcradio_cui.cpp

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
/***************************************************
*		Include 			   					*
****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/prctl.h>

#include "tcradio_api.h"
#include "tcradio_rds_api.h"

#ifdef USE_HDRADIO
#include "tcradio_hdr_if.h"
#endif

/***************************************************
*        Global variable definitions               *
****************************************************/

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/

/***************************************************
*             Local preprocessor                   *
****************************************************/
#define	MAX_SCMP_LEN			(10)
#define SCMP(x, y, len)			strncmp((char*)(x), (char*)(y), (size_t)len)

#define	RADIO_AUDIO_SAMPLERATE	(44100)		// or 48000

#define	FM_START_FREQ			(87500)		// khz
#define	FM_END_FREQ				(107900)	// khz
#define	FM_FREQ_STEP			(200)		// khz
#define	MW_START_FREQ			(530)		// khz
#define	MW_END_FREQ				(1710)		// khz
#define	MW_FREQ_STEP			(10)		// khz
#define	DAB_START_INDEX			(0)
#define	DAB_END_INDEX			(40)
#define	DAB_INDEX_STEP			(1)
#define	PSD_BITMASK 			((uint8)0xDF)

#define	RHAL_DAB_MAX_NUM_OF_FREQ	(64)	// Base on silab tuner

/***************************************************
*           Local type definitions                 *
****************************************************/
typedef enum {
	eRADIO_FM_BAND	= 0,
	eRADIO_MW_BAND	= 1,
	eRADIO_DAB_BAND = 4,
	eRADIO_MAX_BAND
}eRADIO_BAND_t;

/***************************************************
*           Local constant definitions              *
****************************************************/
static uint32 current_frequency=0;
static uint32 current_band=0;
static uint32 fm_frequency=0;
static uint32 mw_frequency=0;
static uint32 dab_freqIndex=0;
static uint32 fOperation=0;
static stRADIO_CONFIG_t radioConf;

/***************************************************
*          Local function prototypes               *
****************************************************/
void tcradiocui_setRadioConfig(stRADIO_CONFIG_t *config);
int tcradiocui_precheckSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata, uint32 freq, uint32 ntuner);
int tcradiocui_checkSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata);
void tcradiocui_getNotificationCallBack(uint32 notifyID, uint32 *arg, void **pData, int32 errorCode);
void tcradiocui_getStationListCallBack(uint32 totalnum, void * list, int32 errorCode);
RET tcradiocui_initTcRadioService(void);
int tcradiocui_operation(char *argv[], int argc);
void tcradiocui_help(void);

/***************************************************
*			function definition				*
****************************************************/
static void helpMsg(char *name)
{
    printf("------------------------------------------------------------\n"
           "|  %s [USAGE]\n"
           "|   p1 : radio standard\n"
           "|  example>\n"
           "|     $ %s          :AM/FM Radio\n"
           "|     $ %s hd10     :HD Radio 1.0\n"
           "|     $ %s hd10mrc  :HD Radio 1.0+MRC\n"
           "|     $ %s hd15     :HD Radio 1.5\n"
           "|     $ %s hd15mrc  :HD Radio 1.5+MRC\n"
           "|     $ %s drm30    :DRM30 Radio (only for IQ)\n"
           "|     $ %s drmp     :DRM+ Radio  (only for IQ)\n"
           "|     $ %s dab      :DAB Radio   (only for IQ)\n"
           "------------------------------------------------------------\n"
            , name, name, name, name, name, name, name, name, name);
}

static RET getSdrFromArg(int cnt, char **sz, stRADIO_CONFIG_t *conf)
{
	RET ret = eRET_OK;
	if(cnt == 1) {
		conf->numTuners = eRADIO_CONF_TYPE_DUAL;
		conf->sdr = eRADIO_SDR_NONE;
		RAPP_DBG("Selected AM/FM Radio.\n");
	}
	else if(cnt == 2) {
	    if (*(sz+1)) {
			if(SCMP(*(sz+1), "hd10", MAX_SCMP_LEN)==0) {
				conf->numTuners = eRADIO_CONF_TYPE_DUAL;
				conf->sdr = eRADIO_SDR_HD;
				conf->hdType = eRADIO_HD_TYPE_HD1p0;
				RAPP_DBG("Selected HD1.0 Radio.\n");
	 		}
			else if(SCMP(*(sz+1), "hd10mrc", MAX_SCMP_LEN)==0) {
				conf->numTuners = eRADIO_CONF_TYPE_DUAL;
				conf->sdr = eRADIO_SDR_HD;
				conf->hdType = eRADIO_HD_TYPE_HD1p0_MRC;
				RAPP_DBG("Selected HD1.0+MRC Radio.\n");
	 		}
			else if(SCMP(*(sz+1), "hd15", MAX_SCMP_LEN)==0) {
				conf->numTuners = eRADIO_CONF_TYPE_DUAL;
				conf->sdr = eRADIO_SDR_HD;
				conf->hdType = eRADIO_HD_TYPE_HD1p5;
				RAPP_DBG("Selected HD1.5 Radio.\n");
	 		}
			else if(SCMP(*(sz+1), "hd15mrc", MAX_SCMP_LEN)==0) {
				conf->numTuners = eRADIO_CONF_TYPE_TRIPLE;
				conf->sdr = eRADIO_SDR_HD;
				conf->hdType = eRADIO_HD_TYPE_HD1p5_MRC;
				RAPP_DBG("Selected HD1.5+MRC Radio.\n");
	 		}
			else if(SCMP(*(sz+1), "hd15dmrc", MAX_SCMP_LEN)==0) {
				conf->numTuners = eRADIO_CONF_TYPE_QUAD;
				conf->sdr = eRADIO_SDR_HD;
				conf->hdType = eRADIO_HD_TYPE_HD1p5_DUAL_MRC;
				RAPP_DBG("Selected HD1.5+DualMRC Radio.\n");
	 		}
			else if(SCMP(*(sz+1), "drm30", MAX_SCMP_LEN)==0) {
				conf->numTuners = eRADIO_CONF_TYPE_DUAL;
				conf->sdr = eRADIO_SDR_DRM30;
				RAPP_DBG("Selected DRM30 Radio.\n");
	 		}
			else if(SCMP(*(sz+1), "drmp", MAX_SCMP_LEN)==0) {
				conf->numTuners = eRADIO_CONF_TYPE_DUAL;
				conf->sdr = eRADIO_SDR_DRMP;
				RAPP_DBG("Selected DRM+ Radio.\n");
	 		}
			else if(SCMP(*(sz+1), "dab", MAX_SCMP_LEN)==0) {
				conf->numTuners = eRADIO_CONF_TYPE_DUAL;
				conf->sdr = eRADIO_SDR_DAB;
				RAPP_DBG("Selected DAB Radio.\n");
	 		}
			else {
				ret = eRET_NG_INVALID_PARAM;
				RAPP_ERR("Invalid Argument: optarg[%s]\n", *(sz+1));
	 		}
	    }
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
		RAPP_ERR("Too many Arguments\n");
	}
    return ret;
}

int32 main(int argc, char* argv[])
{
	RET ret;
	int i, j;
	char cmd[11][11] = {0,};
	char str[100] = {0,};
	char *targv[11];

#ifdef USE_CUI_DEBUG
	struct rlimit corelimit;

	// for coredump
    getrlimit(RLIMIT_CORE, &corelimit);
	RAPP_DBG("Default Core Limit value is : %lld\n", (long long int)corelimit.rlim_cur);
    // Call setrlimit() to set the core filesize limits
	if (corelimit.rlim_cur < 200*1024*1024) {
		corelimit.rlim_cur = RLIM_INFINITY;
		corelimit.rlim_max = RLIM_INFINITY;
		setrlimit(RLIMIT_CORE, &corelimit);
		getrlimit(RLIMIT_CORE, &corelimit);
		RAPP_DBG("Set Soft CoreFile Size: %lld", (long long int)corelimit.rlim_cur);
		RAPP_DBG("Set Hard CoreFile Size: %lld", (long long int)corelimit.rlim_max);
	}

	// Debug code to ensure core dumps are enabled for signals that normally produce a core dump, eg: SEGV
	/* Get the Dumpable state */
    ret = prctl( PR_GET_DUMPABLE, 0, 0, 0, 0 );
    if (ret != 1)
    	/* Set the Dumpable state */
    	ret = prctl( PR_SET_DUMPABLE, 1, 0, 0, 0 );
    ret = prctl( PR_GET_DUMPABLE, 0, 0, 0, 0 );
    RAPP_DBG( "PR_GET_DUMPABLE: %d\n", ret );
#endif

	if(getSdrFromArg(argc, argv, &radioConf) < 0) {
		helpMsg(argv[0]);
		return -1;
	}

	ret = tcradiocui_initTcRadioService();
	if(ret == eRET_OK) {
		tcradiocui_setRadioConfig(&radioConf);
		ret = tcradio_open(&radioConf);
		if(ret != eRET_OK) {
			RAPP_DBG("Failed to open radio service application!!!\n");
		}
		else {
			RAPP_DBG("Radio service application is opening.\n");
		}
	}

	do
	{
		memset(cmd, 0, sizeof(cmd));
		fgets(str, sizeof(str), stdin);
		sscanf(str, "%10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7], cmd[8], cmd[9], cmd[10]);
		for(i=0;i<11;i++) {
			targv[i] = cmd[i];
			if(cmd[i][0] == NULL) {
				break;
			}
		}

		if(tcradiocui_operation(targv, i) < 0) {
			break;
		}
		usleep(20*1000);
	}while(1);

	tcradio_deinit();

	usleep(100*1000);
	return 0;
}

void tcradiocui_setRadioConfig(stRADIO_CONFIG_t *config)
{
//  config->numTuners is set by the user execution command. Don't set here.
//	config->numTuners = eRADIO_CONF_TYPE_SINGLE;	// Set it accoring to tuner H/W configuration. If not, it is not work.
//	config->numTuners = eRADIO_CONF_TYPE_DUAL;
//	config->numTuners = eRADIO_CONF_TYPE_TRIPLE;

	config->fPhaseDiversity = NO;
	config->fIqOut = YES;
	config->audioSamplerate = RADIO_AUDIO_SAMPLERATE;
	config->fExtAppCtrl = 0;

	if(config->sdr == eRADIO_SDR_HD) {
		config->area = eRADIO_CONF_AREA_NA;				// This area setting releated to the tuner property.
		config->initMode = eRADIO_FM_MODE;
		config->initFreq = FM_START_FREQ;
	// config->hdType is set by the user execution command. Don't set here.
	}
	else if(config->sdr == eRADIO_SDR_DRM30) {
		config->area = eRADIO_CONF_AREA_ASIA;			// This area setting releated to the tuner property.
		config->initMode = eRADIO_AM_MODE;
		config->initFreq = MW_START_FREQ;
	}
	else if(config->sdr == eRADIO_SDR_DRMP) {
		config->area = eRADIO_CONF_AREA_ASIA;			// This area setting releated to the tuner property.
		config->initMode = eRADIO_FM_MODE;
		config->initFreq = FM_START_FREQ;
	}
	else if(config->sdr == eRADIO_SDR_DAB) {
		config->area = eRADIO_CONF_AREA_EU;				// This area setting releated to the tuner property.
		config->initMode = eRADIO_DAB_MODE;
		config->initFreq = DAB_START_INDEX;
	}
	else {
		config->sdr = eRADIO_SDR_NONE;
		config->area = eRADIO_CONF_AREA_ASIA;			// This area setting releated to the tuner property.
		config->initMode = eRADIO_FM_MODE;
		config->initFreq = FM_START_FREQ;
	}
}

RET tcradiocui_initTcRadioService(void)
{
	RET ret;

	tcradio_configOnGetNotificationCallBack(tcradiocui_getNotificationCallBack);
	tcradio_configOnGetStationListCallBack(tcradiocui_getStationListCallBack);

	tcradio_configOnPrecheckSeekQual(tcradiocui_precheckSeekQual);
	tcradio_configOnCheckSeekQual(tcradiocui_checkSeekQual);

	fm_frequency = FM_START_FREQ;
	mw_frequency = MW_START_FREQ;
	dab_freqIndex = DAB_START_INDEX;
	current_frequency = FM_START_FREQ;
	current_band = eRADIO_FM_BAND;

	ret = tcradio_init();
	if(ret != eRET_OK){
		if(ret == eRET_NG_ALREADY_INIT) {
			RAPP_DBG("Already init tcradio!!![%d]\n", ret);
		}
		else {
			RAPP_DBG("tcradio_init is failed!!![%d]\n", ret);
		}
	}

	if(ret == eRET_OK) {
		ret = tcradio_setBandFreqConfig(eRADIO_FM_MODE, FM_START_FREQ, FM_END_FREQ, FM_FREQ_STEP);			// FM Setting
		if(ret != eRET_OK) {
			RAPP_DBG("tcradio_setBandFreqConfig is failed[FM:%d]. Set to default.\n", ret);
		}
		ret = tcradio_setBandFreqConfig(eRADIO_AM_MODE, MW_START_FREQ, MW_END_FREQ, MW_FREQ_STEP);			// AM Setting
		if(ret != eRET_OK) {
			RAPP_DBG("tcradio_setBandFreqConfig is failed[AM:%d]. Set to default.\n", ret);
		}
		ret = tcradio_setBandFreqConfig(eRADIO_DAB_MODE, DAB_START_INDEX, DAB_END_INDEX, DAB_INDEX_STEP);	// DAB Setting
		if(ret != eRET_OK) {
			RAPP_DBG("tcradio_setBandFreqConfig is failed[AM:%d]. Set to default.\n", ret);
		}
	}

	return ret;
}

int tcradiocui_precheckSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata, uint32 freq, uint32 ntuner)
{
	int32 ret=-1, rssi=0;

	if(qdata.type == eRADIO_TYPE1)
	{
		stRADIO_TYPE1_QUALITY_t temp_qdata;
		memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(current_band == eRADIO_FM_BAND) {
			rssi = (int32)temp_qdata.fm.Rssi;
			if(rssi >= 20)
			{
				ret = 0;
			}
		}
		else if(current_band == eRADIO_MW_BAND) {
			rssi = (int32)temp_qdata.am.Rssi;
			if(rssi >= 38)
			{
				ret = 0;
			}
		}
		else if(current_band == eRADIO_DAB_BAND) {
			rssi = (int32)temp_qdata.dab.Rssi;
			if(rssi >= -107)
			{
				ret = 0;
			}
		}
		else {
			;	// Other user bands can be set.
		}
	}
	else if(qdata.type == eRADIO_TYPE2)
	{
		stRADIO_TYPE2_QUALITY_t temp_qdata;
		memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(current_band == eRADIO_FM_BAND) {
			rssi = (int32)temp_qdata.fm.Rssi;
			if(rssi >= 280)
			{
				ret = 0;
			}
		}
		else if(current_band == eRADIO_MW_BAND){
			rssi = (int32)temp_qdata.am.Rssi;
			if(rssi >= 630)
			{
				ret = 0;
			}
		}
		else {
			;	// Other user bands can be set.
		}
	}
	else if(qdata.type == eRADIO_TYPE3)
	{
		stRADIO_TYPE3_QUALITY_t temp_qdata;
		memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(current_band == eRADIO_FM_BAND) {
			rssi = (int32)temp_qdata.fm.FstBB;
			if(rssi >= 20)
			{
				ret = 0;
			}
		}
		else if(current_band == eRADIO_MW_BAND) {
			rssi = (int32)temp_qdata.am.FstBB;
			if(rssi >= 38)
			{
				ret = 0;
			}
		}
		else if(current_band == eRADIO_DAB_BAND) {
			rssi = (int32)temp_qdata.dab.FstBB;
			if(rssi >= -107)
			{
				ret = 0;
			}
		}
		else {
			;	// Other user bands can be set.
		}
	}

	return ret;
}

int tcradiocui_checkSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata)
{
	int32 ret=-1;
	int32 rssi=0, snr=0, offs=0, sqi=0;
	uint32 usn=0, mpth=0, noise=0, detect=0;

	if(qdata.type == eRADIO_TYPE1)
	{
		stRADIO_TYPE1_QUALITY_t temp_qdata;
		memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(current_band == eRADIO_FM_BAND) {
			rssi = (int32)temp_qdata.fm.Rssi;
			snr = (int32)temp_qdata.fm.Snr;
			offs = (int32)temp_qdata.fm.Offs;
			if(rssi >= 20 && snr > 4 && offs > -6 && offs < 6)
			{
				ret = 0;
			}
		}
		else if(current_band == eRADIO_MW_BAND) {
			rssi = (int32)temp_qdata.am.Rssi;
			snr = (int32)temp_qdata.am.Snr;
			offs = (int32)temp_qdata.am.Offs;
			if(rssi >= 38 && snr > 6 && offs > -6 && offs < 6)
			{
				ret = 0;
			}
		}
		else if(current_band == eRADIO_DAB_BAND) {
			rssi = (int32)temp_qdata.dab.Rssi;
			sqi = (int32)temp_qdata.dab.Sqi;
			detect = (int32)temp_qdata.dab.Detect;
			if(rssi >= -107 && sqi > 8 && detect > 0)
			{
				ret = 0;
			}
		}
		else {
			;	// Other user bands can be set.
		}
	}
	else if(qdata.type == eRADIO_TYPE2)
	{
		stRADIO_TYPE2_QUALITY_t temp_qdata;
		memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(current_band == eRADIO_FM_BAND) {
			rssi = (int32)temp_qdata.fm.Rssi;
			offs = (int32)temp_qdata.fm.Offs;
			usn = temp_qdata.fm.Usn;
			mpth = temp_qdata.fm.Mpth;
			if(rssi >= 280 && offs > -100 && offs < 100 && usn < 120 && mpth < 200)
			{
				ret = 0;
			}
		}
		else if(current_band == eRADIO_MW_BAND) {
			rssi = (int32)temp_qdata.am.Rssi;
			offs = (int32)temp_qdata.am.Offs;
			noise = temp_qdata.am.Hfn;
			if(rssi >= 630 && offs > -50 && offs < 50 && noise < 100)
			{
				ret = 0;
			}
		}
		else {
			;	// Other user bands can be set.
		}
	}
	else if(qdata.type == eRADIO_TYPE3)
	{
		stRADIO_TYPE3_QUALITY_t temp_qdata;
		memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(current_band == eRADIO_FM_BAND) {
			rssi = (int32)temp_qdata.fm.FstBB;
			snr = (int32)temp_qdata.fm.Snr;
			if(rssi >= 20 && snr > 100)
			{
				ret = 0;
			}
		}
		else if(current_band == eRADIO_MW_BAND) {
			rssi = (int32)temp_qdata.am.FstBB;
			snr = (int32)temp_qdata.am.Snr;
			if(rssi >= 38 && snr > 100)
			{
				ret = 0;
			}
		}
		else if(current_band == eRADIO_DAB_BAND) {
			rssi = (int32)temp_qdata.dab.FstBB;
			if(rssi >= -107)
			{
				ret = 0;
			}
		}
		else {
			;	// Other user bands can be set.
		}
	}

	return ret;
}

void tcradiocui_getNotificationCallBack(uint32 notifyID, uint32 *arg, void **pData, int32 errorCode)
{
	uint32 i;

	switch(notifyID) {
		case eRADIO_NOTIFY_OPEN:
			if(errorCode != eRET_OK && errorCode != eRET_NG_ALREADY_OPEN) {
				RAPP_ERR("eRADIO_NOTIFY_OPEN : Error[%d]!\n", errorCode);
			}
			else {
				RAPP_DBG("eRADIO_NOTIFY_OPEN : Done! band[%d], freq[%d]\n", arg[0], arg[1]);
				if(arg[0] == eRADIO_AM_MODE) {
					// If you use SW or LW, more conditionals will be needed.
					current_band = eRADIO_MW_BAND;
					mw_frequency = current_frequency = arg[1];
				}
				else if(arg[0] == eRADIO_DAB_MODE) {
					current_band = eRADIO_DAB_BAND;
					dab_freqIndex = current_frequency = arg[1];
				}
				else { // eRADIO_FM_MODE
					current_band = eRADIO_FM_BAND;
					fm_frequency = current_frequency = arg[1];
				}

				if(radioConf.sdr == eRADIO_SDR_NONE) {
					tcradio_setAudioDevice(&radioConf, ON);
					tcradio_setAudio(1);	// play
				}
			}
			break;

		case eRADIO_NOTIFY_DEINIT:
			if(errorCode != eRET_OK) {
				RAPP_ERR("eRADIO_NOTIFY_DEINIT : Error[%d]!\n", errorCode);
			}
			else {
				RAPP_DBG("eRADIO_NOTIFY_DEINIT : Done!\n");
			}
			break;

	    case eRADIO_NOTIFY_SEEK_MODE:
			if(errorCode != eRET_OK) {
				RAPP_ERR("eRADIO_NOTIFY_SET_SEEK : Error[%d]! band[%d], freq[%d], seekmode[%d]\n", errorCode,arg[0], arg[1], arg[2]);
			}
			else {
				current_frequency = arg[1];
				RAPP_DBG("eRADIO_NOTIFY_SET_SEEK : Done! band[%d], freq[%d], seekmode[%d]\n", arg[0], arg[1], arg[2]);
#ifdef USE_HDRADIO
				tcradio_setHdrPsdNotification(0, PSD_BITMASK);
#endif
			}
			if(arg[2] == 3 || arg[2] == 4 || arg[2] == 5) {		// 3: Auto Up, 4: Auto Down, 5: Scan List
				if(arg[3] == eRADIO_TYPE1)
				{
					stRADIO_TYPE1_QUALITY_t qvalue;
					memcpy((void *)&qvalue, (void *)(arg+4), sizeof(qvalue));

					if(arg[0] == eRADIO_AM_MODE) {	// AM
						RAPP_DBG("RSSI = %d\n", qvalue.am.Rssi);
						RAPP_DBG("SNR = %d\n", qvalue.am.Snr);
						RAPP_DBG("Modulation = %d\n", qvalue.am.Mod);
						RAPP_DBG("Offset = %d\n", qvalue.am.Offs);
					}
					else if(arg[0] == eRADIO_FM_MODE) {	// FM
						RAPP_DBG("RSSI = %d\n", qvalue.fm.Rssi);
						RAPP_DBG("SNR = %d\n", qvalue.fm.Snr);
						RAPP_DBG("Deviation = %d\n", qvalue.fm.Dev);
						RAPP_DBG("Offset = %d\n", qvalue.fm.Offs);
						RAPP_DBG("Pilot = %d\n", qvalue.fm.Pilot);
						RAPP_DBG("MultiPath = %d\n", qvalue.fm.Mpth);
						RAPP_DBG("USN = %d\n", qvalue.fm.Usn);
					}
					else if(arg[0] == eRADIO_DAB_MODE) {	// DAB
						RAPP_DBG("RSSI = %d\n", qvalue.dab.Rssi);
						RAPP_DBG("SQI = %d\n", qvalue.dab.Sqi);
						RAPP_DBG("DETECT = %d\n", qvalue.dab.Detect);
						RAPP_DBG("RSSIADJ = %d\n", qvalue.dab.RssiAdj);
						RAPP_DBG("DAGC = %d\n", qvalue.dab.Dagc);
						RAPP_DBG("VALID = %d\n", qvalue.dab.Valid);
					}
					else {
						// other band.
					}
				}
				else if(arg[3] == eRADIO_TYPE2)
				{
					stRADIO_TYPE2_QUALITY_t qvalue;
					memcpy((void *)&qvalue, (void *)(arg+4), sizeof(qvalue));

					if(arg[0] == eRADIO_AM_MODE) {	// AM
						RAPP_DBG("Status = %xh\n", qvalue.am.Status);
						RAPP_DBG("RSSI = %d\n", qvalue.am.Rssi);
						RAPP_DBG("High Frequency Noise Detector = %d\n", qvalue.am.Hfn);
						RAPP_DBG("Co-Channel Detector = %d\n", qvalue.am.Coch);
						RAPP_DBG("Offset = %d\n", qvalue.am.Offs);
						RAPP_DBG("Bandwidth = %d\n", qvalue.am.Bwth);
						RAPP_DBG("Modulation = %d\n", qvalue.am.Mod);
					}
					else if(arg[0] == eRADIO_FM_MODE) {	// FM
						RAPP_DBG("Status = %xh\n", qvalue.fm.Status);
						RAPP_DBG("RSSI = %d\n", qvalue.fm.Rssi);
						RAPP_DBG("USN = %d\n", qvalue.fm.Usn);
						RAPP_DBG("MultiPath = %d\n", qvalue.fm.Mpth);
						RAPP_DBG("Offset = %d\n", qvalue.fm.Offs);
						RAPP_DBG("Bandwidth = %d\n", qvalue.fm.Bwth);
						RAPP_DBG("Modulation = %d\n", qvalue.fm.Mod);
					}
					else {
						// DAB quality but not support.
					}
				}
				else if(arg[3] == eRADIO_TYPE3)
				{
					stRADIO_TYPE3_QUALITY_t qvalue;
					memcpy((void *)&qvalue, (void *)(arg+4), sizeof(qvalue));

					if(arg[0] == eRADIO_AM_MODE) {	// AM
						RAPP_DBG("FstRF = %d\n", qvalue.am.FstRF);
						RAPP_DBG("FstBB = %d\n", qvalue.am.FstBB);
						RAPP_DBG("Detuning = %d\n", qvalue.am.Det);
						RAPP_DBG("SNR = %d\n", qvalue.am.Snr);
						RAPP_DBG("Adjacent = %d\n", qvalue.am.Adj);
						RAPP_DBG("Deviation = %d\n", qvalue.am.Dev);
					}
					else if(arg[0] == eRADIO_FM_MODE) {	// FM
						RAPP_DBG("FstRF = %d\n", qvalue.fm.FstRF);
						RAPP_DBG("FstBB = %d\n", qvalue.fm.FstBB);
						RAPP_DBG("Detuning = %d\n", qvalue.fm.Det);
						RAPP_DBG("SNR = %d\n", qvalue.fm.Snr);
						RAPP_DBG("Adjacent = %d\n", qvalue.fm.Adj);
						RAPP_DBG("Deviation = %d\n", qvalue.fm.Dev);
						RAPP_DBG("Multipath = %d\n", qvalue.fm.Mpth);
						RAPP_DBG("MpxNoise = %d\n", qvalue.fm.MpxNoise);
						RAPP_DBG("Stereo = %d\n", qvalue.fm.Stereo);
					}
					else if(arg[0] == eRADIO_DAB_MODE) {	// DAB
						RAPP_DBG("FstRF = %d\n", qvalue.dab.FstRF);
						RAPP_DBG("FstBB = %d\n", qvalue.dab.FstBB);
					}
					else {
						// other band.
					}
				}
				else {
					stRADIO_TYPE0_QUALITY_t qvalue;
					memcpy((void *)&qvalue, (void *)(arg+4), sizeof(qvalue));

					if(arg[0] == eRADIO_AM_MODE) {	// AM
						RAPP_DBG("RSSI = %d\n", qvalue.am.Rssi);
						RAPP_DBG("Modulation = %d\n", qvalue.am.Mod);
						RAPP_DBG("Offset = %d\n", qvalue.am.Offs);
					}
					else if(arg[0] == eRADIO_FM_MODE) {	// FM
						RAPP_DBG("RSSI = %d\n", qvalue.fm.Rssi);
						RAPP_DBG("Modulation = %d\n", qvalue.fm.Mod);
						RAPP_DBG("Offset = %d\n", qvalue.fm.Offs);
						RAPP_DBG("HFN = %d\n", qvalue.fm.Hfn);
						RAPP_DBG("MultiPath = %d\n", qvalue.fm.Mpth);
						RAPP_DBG("Pilot = %d\n", qvalue.fm.Pilot);
					}
					else {
						// DAB quality but not support.
					}
				}
			}

			if(arg[2] == eRADIO_SEEK_STOP) {
				if(arg[3] == 0) {
					RAPP_DBG("Not seek or Can't found radio station!!! [%d]\n", arg[3]);
				}
				else {
					RAPP_DBG("Found radio station!!! [%d]\n", arg[3]);
				}

				if(arg[0] == eRADIO_FM_MODE) {
					current_band = eRADIO_FM_BAND;
					fm_frequency = current_frequency = arg[1];
				}
				else if(arg[0] == eRADIO_AM_MODE) {
					// If you use SW or LW, more conditionals will be needed.
					current_band = eRADIO_MW_BAND;
					mw_frequency = current_frequency = arg[1];
				}
				else if(arg[0] == eRADIO_DAB_MODE) {
					current_band = eRADIO_DAB_BAND;
					dab_freqIndex = current_frequency = arg[1];
				}
				else {
					;	// Other user mode can be set.
				}
			}
			break;

		case eRADIO_NOTIFY_TUNE:
			if(errorCode != eRET_OK) {
				RAPP_ERR("eRADIO_NOTIFY_GET_FREQ : Error[%d]! band[%d] freq[%d] seekmode[%d] ntuner[%d]\n", errorCode, arg[0], arg[1], arg[2], arg[3]);
			}
			else {
				RAPP_DBG("eRADIO_NOTIFY_GET_FREQ : Done! band[%d] freq[%d] seekmode[%d] ntuner[%d]\n", arg[0], arg[1], arg[2], arg[3]);
#ifdef USE_HDRADIO
				if(arg[3] == eRADIO_ID_PRIMARY) {
					tcradio_setHdrPsdNotification(0, PSD_BITMASK);
				}
#endif
			}

			if(arg[3] == eRADIO_ID_PRIMARY) {
				if(arg[0] == eRADIO_FM_MODE) {
					current_band = eRADIO_FM_BAND;
					fm_frequency = current_frequency = arg[1];
				}
				else if(arg[0] == eRADIO_AM_MODE) {
					// If you use SW or LW, more conditionals will be needed.
					current_band = eRADIO_MW_BAND;
					mw_frequency = current_frequency = arg[1];
				}
				else if(arg[0] == eRADIO_DAB_MODE) {
					current_band = eRADIO_DAB_BAND;
					dab_freqIndex = current_frequency = arg[1];
				}
				else {
					;	// Other user mode can be set.
				}
			}

			break;

#ifdef USE_HDRADIO
		/****************************/
		//	HD Radio Notifications	//
		/****************************/
		case eRADIO_HD_NOTIFY_OPEN:
			if(errorCode != eRET_OK) {
				RAPP_DBG("eRADIO_HD_NOTIFY_OPEN : Error[%d]!\n", errorCode);
			}
			else {
				RAPP_DBG("eRADIO_HD_NOTIFY_OPEN : [%d] Done!\n", arg[0]);
				if(radioConf.fExtAppCtrl==0) {
					tcradio_setAudioDevice(&radioConf, ON);
				}
				tcradio_setAudio(1);	// play
				tcradio_setHdrPsdNotification(0, PSD_BITMASK);
			}
			break;

		case eRADIO_HD_NOTIFY_AUDIO_MODE:
			if(errorCode != eRET_OK) {
				RAPP_DBG("eRADIO_HD_NOTIFY_AUDIO_MODE : Error[%d]!\n", errorCode);
			}
			else {
				RAPP_DBG("eRADIO_HD_NOTIFY_AUDIO_MODE : [%d] Done!\n", arg[0]);
			}
			break;

		case eRADIO_HD_NOTIFY_PROGRAM:
			if(errorCode != eRET_OK) {
				RAPP_DBG("eRADIO_HD_NOTIFY_PROGRAM : Error[%d]!\n", errorCode);
			}
			else {
				RAPP_DBG("eRADIO_HD_NOTIFY_PROGRAM : ID[%d], Program[%d] Done!\n", arg[0], arg[1]);
			}
			break;

		case eRADIO_HD_NOTIFY_PSD:
			if(errorCode != eRET_OK) {
				RAPP_DBG("eRADIO_HD_NOTIFY_PSD : Error[%d]!\n", errorCode);
			}
			else {
				if(*(pData+0) != NULL) {
					unsigned int type = 0;
					stTC_HDR_PSD_t hdrPsd;
					memcpy((void*)&hdrPsd, *pData, sizeof(stTC_HDR_PSD_t));
					RAPP_DBG("-----------------------------------------------\n");
					RAPP_DBG("eRADIO_HD_NOTIFY_PSD : Done! HDR ID[%d], PN[%02d] \n", arg[0], arg[1]);

					stTC_HDR_PSD_XHDR_FRAME_t *tempXhdrData=NULL;
					stTC_HDR_PSD_FORM_t *tempPsdData=NULL;

					for(type=eTC_HDR_PSD_TITLE; type <= eTC_HDR_PSD_COMMENT_ACTUAL_TEXT; type++) {
						switch(type) {
							case eTC_HDR_PSD_TITLE:                     tempPsdData = &hdrPsd.title;                    break;
							case eTC_HDR_PSD_ARTIST:                    tempPsdData = &hdrPsd.artist;                   break;
							case eTC_HDR_PSD_ALBUM:                     tempPsdData = &hdrPsd.album;                    break;
							case eTC_HDR_PSD_GENRE:                     tempPsdData = &hdrPsd.genre;                    break;
							case eTC_HDR_PSD_COMMENT_LANGUAGE:          tempPsdData = &hdrPsd.comment.language;         break;
							case eTC_HDR_PSD_COMMENT_SHORT_CONTENT:     tempPsdData = &hdrPsd.comment.shortContent;     break;
							case eTC_HDR_PSD_COMMENT_ACTUAL_TEXT:       tempPsdData = &hdrPsd.comment.actualText;       break;
						}

						if(tempPsdData->len > 0) {
							RAPP_DBG("PsdType[%d], CharType[0x%02x], DataLength[%d]\n", type, tempPsdData->charType, tempPsdData->len);
							RAPP_DBG("");

							for(i=0; i<tempPsdData->len; i++) {
							    printf("%c", tempPsdData->data[i]);
							}
							printf("\n");
						}
					}

					tempPsdData = &hdrPsd.commercial.receivedAs;
					if(tempPsdData->len > 0) {
						RAPP_DBG("PsdType[Received AS], CharType[0x%02x], DataLength[%d], AS[0x%02x]\n",
						tempPsdData->charType, tempPsdData->len, tempPsdData->data[0]);

						S8 As = tempPsdData->data[0];

						if(As == 0x00) {
							for(type=eTC_HDR_PSD_COMMERCIAL_PRICE_STRING; type <= eTC_HDR_PSD_COMMERCIAL_DESCRIPTION; type++) {
								switch(type) {
									case eTC_HDR_PSD_COMMERCIAL_PRICE_STRING:   tempPsdData = &hdrPsd.commercial.priceString;   break;
									case eTC_HDR_PSD_COMMERCIAL_VALID_UNTIL:    tempPsdData = &hdrPsd.commercial.validUntil;    break;
									case eTC_HDR_PSD_COMMERCIAL_CONTACT_URL:    tempPsdData = &hdrPsd.commercial.contactURL;    break;
									case eTC_HDR_PSD_COMMERCIAL_SELLER_NAME:    tempPsdData = &hdrPsd.commercial.sellerName;    break;
									case eTC_HDR_PSD_COMMERCIAL_DESCRIPTION:    tempPsdData = &hdrPsd.commercial.description;   break;
								}

								if((tempPsdData->len > 0) && (type != eTC_HDR_PSD_COMMERCIAL_RECEIVED_AS)) {
									RAPP_DBG("PsdType[%d], CharType[0x%02x], DataLength[%d]\n", type, tempPsdData->charType, tempPsdData->len);
									RAPP_DBG("");

									for(i=0; i<tempPsdData->len; i++) {
										printf("%c", tempPsdData->data[i]);
									}
									printf("\n");
								}
							}
						}else{
							// PSD Spec: "0x01 ~ 0xFF" Receivers shall not display the Commercial frame.
						}
					}

					tempXhdrData = &hdrPsd.xhdr;
					if(tempXhdrData != NULL) {
						if(tempXhdrData->numParams > 0){
							RAPP_DBG("PsdType[%d] numParams[%d]\n", eTC_HDR_PSD_XHDR, tempXhdrData->numParams);

							for(U32 num = 0; num < tempXhdrData->numParams; num++) {
								if(tempXhdrData->params[num].param_id == 0) {
									RAPP_DBG("XHDR ParamID[0x%02X], MIME[0x%X], lot id[%d]\n", tempXhdrData->params[num].param_id, tempXhdrData->mime_hash, tempXhdrData->params[num].lot_id);
								} else if(tempXhdrData->params[num].param_id == 1) {
									RAPP_DBG("XHDR ParamID[0x%02X], MIME[0x%X]\n", tempXhdrData->params[num].param_id, tempXhdrData->mime_hash);
								} else if(tempXhdrData->params[num].param_id == 2) {
									RAPP_DBG("XHDR ParamID[0x%02X], MIME[0x%X]\n", tempXhdrData->params[num].param_id, tempXhdrData->mime_hash);
								} else {
									RAPP_DBG("XHDR ParamID[0x%02X], MIME[0x%X], value[0x", tempXhdrData->params[num].param_id, tempXhdrData->mime_hash);
									for(U32 idx = 0; idx < tempXhdrData->params[num].length; idx++) {
										printf("%X", tempXhdrData->params[num].value[idx]);
									}
									printf("]\n");
								}
							}
						}
					}
				}
			}
			break;

		case eRADIO_HD_NOTIFY_SIS:
			if(errorCode != eRET_OK) {
				RAPP_DBG("eRADIO_HD_NOTIFY_SIS : Error[%d]!\n", errorCode);
			}
			else {
					if(*(pData+0) != NULL) {
					int i;
					stTC_HDR_SIS_t sisData;
					memcpy((void*)&sisData, *pData, sizeof(stTC_HDR_SIS_t));

					RAPP_DBG("---------------------------------------------------------\n");
					RAPP_DBG("Station Information Service Data: HDR ID[%d]\n", arg[0]);

					RAPP_DBG("Station ID[%04xh]\n", sisData.stationID.all);
					RAPP_DBG("Short Name Length[%d]\n", sisData.shortName.len);
					RAPP_DBG("Short Name : ");
					if(sisData.shortName.len > 0) {
						for(i=0; i<sisData.shortName.len; i++) {
							printf("%c", sisData.shortName.text[i]);
						}
					}
					printf("\n");

					RAPP_DBG("Universal Name Length[%d], charType[%d], appendFm[%d]\n", sisData.universalName.len, sisData.universalName.charType, sisData.universalName.appendFm);
					RAPP_DBG("Universal Name : ");
					if(sisData.universalName.len > 0) {
						for(i=0; i<sisData.universalName.len; i++) {
							printf("%c", sisData.universalName.text[i]);
						}
					}
					printf("\n");

					RAPP_DBG("Slogan Length[%d], charType[%d]\n", sisData.slogan.len, sisData.slogan.charType);
					RAPP_DBG("Slogan : ");
					if(sisData.slogan.len > 0) {
						for(i=0; i<sisData.slogan.len; i++) {
							printf("%c", sisData.slogan.text[i]);
						}
					}
					printf("\n");
				}
			}
			break;

		case eRADIO_HD_NOTIFY_SIGNAL_STATUS:
			if(errorCode != eRET_OK) {
				RAPP_DBG("eRADIO_HD_NOTIFY_SIGNAL_STATUS : Error[%d]!\n", errorCode);
			}
			else {
				stTC_HDR_SIGNAL_STATUS_t hdrSigStatus;
				memcpy((void*)&hdrSigStatus, *(pData+0), sizeof(stTC_HDR_SIGNAL_STATUS_t));
				RAPP_DBG("eRADIO_HD_NOTIFY_SIGNAL_STATUS : ID[%d], ALL[%02xh], HDSIG[%d], HDAUD[%d], SIS[%d], SISOK[%d] Done!\n",
					hdrSigStatus.hdrID , hdrSigStatus.acqStatus, hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_HD_SIGNAL, hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_HD_AUDIO,
					hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_SIS, hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_SIS_OK);
			}
			break;

		case eRADIO_HD_NOTIFY_PTY:
			if(errorCode != eRET_OK) {
				RAPP_DBG("eRADIO_HD_NOTIFY_PTY : Error[%d]!\n", errorCode);
			}
			else {
				RAPP_DBG("eRADIO_HD_NOTIFY_PTY : ID[%d], Current ProgNum[%d], PTY[%d]\n", arg[0], arg[1], arg[2]);
			}
			break;

		case eRADIO_HD_NOTIFY_LOT:
			if(errorCode != eRET_OK) {
				RAPP_DBG("eRADIO_HD_NOTIFY_LOT : not found [%d]!\n", errorCode);
			}
			else {
				if(*(pData) != NULL){
					S32 fret = 0;
					stTC_HDR_LOT_t lotData;

					memcpy((void*)&lotData, *(pData+0), sizeof(stTC_HDR_LOT_t));

					RAPP_DBG("--------------------------------------------------------- LOT\n");
					RAPP_DBG("eRADIO_HD_NOTIFY_LOT : Done! Service Number[%d], File Name[%s], app_mime_hash[0x%X], mimhash[0x%X]\n",
						lotData.service_number, lotData.header.filename,lotData.app_mime_hash,lotData.header.mime_hash);

					RAPP_DBG("HdrID [%d], PortNum[%d], LotID[%d], ByteWritten[%d], Service Number[%d], Dtime[%d], ProNum[%d]\n",
						arg[0],arg[1],arg[2],arg[3],arg[4],arg[5],arg[6]);

					if(lotData.app_mime_hash == TC_HDR_SIG_APP_MIME_HASH_ALBUM_ART){
						RAPP_DBG("* APP MIME : Primary Image\n");
					}else{
						RAPP_DBG("* APP MIME : Station Logo\n");
					}

					if(lotData.header.mime_hash == TC_HDR_AAS_LOT_MIME_HASH_IMAGE_PNG){
						RAPP_DBG("* FILE MIME : Ext ( PNG )\n");
					}else if(lotData.header.mime_hash == TC_HDR_AAS_LOT_MIME_HASH_IMAGE_JPEG){
						RAPP_DBG("* FILE MIME : Ext ( JPG )\n");
					}else {
						RAPP_DBG("* FILE MIME : Ext ( Other )\n");
					}

					/* The image data is in "lotData.body" and the size is "lotData.body_bytes_written". */
				}
			}
			break;

		case eRADIO_HD_NOTIFY_ALERT:
			if(errorCode != eRET_OK) {
				RAPP_DBG("eRADIO_HD_NOTIFY_ALERT : Error[%d]!\n", errorCode);
			}
			else {
				if(*(pData) != NULL) {
					U8 alert_message[TC_HDR_MAX_ALERT_PAYLOAD_LENGTH] = {0,};
					stTC_HDR_ALERT_MESSAGE_t msg;
					memcpy((void*)&msg, *(pData+0), sizeof(stTC_HDR_ALERT_MESSAGE_t));
					if(msg.text_message != NULL) {
						(void)memcpy((void*)alert_message, msg.text_message, msg.text_length);
					}

					RAPP_DBG("--------------------------------------------------------- EA\n");
					RAPP_DBG("eRADIO_HD_NOTIFY_ALERT : Done! HID[%u], text len[%d], payload len[%d]\n",
						arg[0], msg.text_length, msg.payload_length);
					RAPP_DBG("Alert Message	: %s\n", alert_message);
				}
			}
			break;
#endif	// #ifdef USE_HDRADIO

		default:
			break;
	}
}

void tcradiocui_getStationListCallBack(uint32 totalnum, void *list, int32 errorCode)
{
	uint32 i;
	stRADIO_LIST_t *StationList = (stRADIO_LIST_t *)list;

	RAPP_DBG("====== Total List Num[%d], List Addr[%p], errorCode[%d] =====\n", totalnum, list, errorCode);
	if(totalnum > 0) {
		for(i=0; i<totalnum; i++) {
			RAPP_DBG("%d Station : %d Khz\n", i+1, (StationList+i)->uiFreq);
			if((StationList+i)->stQdata.type == eRADIO_TYPE1)
			{
				stRADIO_TYPE1_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)((StationList+i)->stQdata.qual.fm.Qvalue), sizeof(qvalue));

				if(current_band == eRADIO_FM_BAND) {	// FM
					RAPP_DBG("RSSI = %d\n", qvalue.fm.Rssi);
					RAPP_DBG("SNR = %d\n", qvalue.fm.Snr);
					RAPP_DBG("Deviation = %d\n", qvalue.fm.Dev);
					RAPP_DBG("Offset = %d\n", qvalue.fm.Offs);
					RAPP_DBG("Pilot = %d\n", qvalue.fm.Pilot);
					RAPP_DBG("MultiPath = %d\n", qvalue.fm.Mpth);
					RAPP_DBG("USN = %d\n", qvalue.fm.Usn);
				}
				else if(current_band == eRADIO_MW_BAND) {	// MW
					RAPP_DBG("RSSI = %d\n", qvalue.am.Rssi);
					RAPP_DBG("SNR = %d\n", qvalue.am.Snr);
					RAPP_DBG("Modulation = %d\n", qvalue.am.Mod);
					RAPP_DBG("Offset = %d\n", qvalue.am.Offs);
				}
				else if(current_band == eRADIO_DAB_BAND) {
					// DAB
					RAPP_DBG("RSSI = %d\n", qvalue.dab.Rssi);
					RAPP_DBG("SQI = %d\n", qvalue.dab.Sqi);
					RAPP_DBG("DETECT = %d\n", qvalue.dab.Detect);
					RAPP_DBG("RSSIADJ = %d\n", qvalue.dab.RssiAdj);
					RAPP_DBG("DAGC = %d\n", qvalue.dab.Dagc);
					RAPP_DBG("VALID = %d\n", qvalue.dab.Valid);
				}
				else {
					;	// Other user bands can be set.
				}
			}
			else if((StationList+i)->stQdata.type == eRADIO_TYPE2)
			{
				stRADIO_TYPE2_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)((StationList+i)->stQdata.qual.fm.Qvalue), sizeof(qvalue));

				if(current_band == eRADIO_FM_BAND) {	// FM
					RAPP_DBG("Status = %xh\n", qvalue.fm.Status);
					RAPP_DBG("RSSI = %d\n", qvalue.fm.Rssi);
					RAPP_DBG("USN = %d\n", qvalue.fm.Usn);
					RAPP_DBG("MultiPath = %d\n", qvalue.fm.Mpth);
					RAPP_DBG("Offset = %d\n", qvalue.fm.Offs);
					RAPP_DBG("Bandwidth = %d\n", qvalue.fm.Bwth);
					RAPP_DBG("Modulation = %d\n", qvalue.fm.Mod);
				}
				else if(current_band == eRADIO_MW_BAND) {	// MW
					RAPP_DBG("Status = %xh\n", qvalue.am.Status);
					RAPP_DBG("RSSI = %d\n", qvalue.am.Rssi);
					RAPP_DBG("High Frequency Noise Detector = %d\n", qvalue.am.Hfn);
					RAPP_DBG("Co-Channel Detector = %d\n", qvalue.am.Coch);
					RAPP_DBG("Offset = %d\n", qvalue.am.Offs);
					RAPP_DBG("Bandwidth = %d\n", qvalue.am.Bwth);
					RAPP_DBG("Modulation = %d\n", qvalue.am.Mod);
				}
				else {
					;	// Other user bands can be set.
				}
			}
			else if((StationList+i)->stQdata.type == eRADIO_TYPE3)
			{
				stRADIO_TYPE3_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)((StationList+i)->stQdata.qual.fm.Qvalue), sizeof(qvalue));

					if(current_band == eRADIO_AM_MODE) {	// AM
						RAPP_DBG("FstRF = %d\n", qvalue.am.FstRF);
						RAPP_DBG("FstBB = %d\n", qvalue.am.FstBB);
						RAPP_DBG("Detuning = %d\n", qvalue.am.Det);
						RAPP_DBG("SNR = %d\n", qvalue.am.Snr);
						RAPP_DBG("Adjacent = %d\n", qvalue.am.Adj);
						RAPP_DBG("Deviation = %d\n", qvalue.am.Dev);
					}
					else if(current_band == eRADIO_FM_MODE) {	// FM
						RAPP_DBG("FstRF = %d\n", qvalue.fm.FstRF);
						RAPP_DBG("FstBB = %d\n", qvalue.fm.FstBB);
						RAPP_DBG("Detuning = %d\n", qvalue.fm.Det);
						RAPP_DBG("SNR = %d\n", qvalue.fm.Snr);
						RAPP_DBG("Adjacent = %d\n", qvalue.fm.Adj);
						RAPP_DBG("Deviation = %d\n", qvalue.fm.Dev);
						RAPP_DBG("Multipath = %d\n", qvalue.fm.Mpth);
						RAPP_DBG("MpxNoise = %d\n", qvalue.fm.MpxNoise);
						RAPP_DBG("Stereo = %d\n", qvalue.fm.Stereo);
					}
					else if(current_band == eRADIO_DAB_MODE) {	// DAB
						RAPP_DBG("FstRF = %d\n", qvalue.dab.FstRF);
						RAPP_DBG("FstBB = %d\n", qvalue.dab.FstBB);
					}
					else {
						// other band.
					}
				}
			else {
				stRADIO_TYPE0_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)((StationList+i)->stQdata.qual.fm.Qvalue), sizeof(qvalue));

				if(current_band == eRADIO_FM_BAND) {	// FM
					RAPP_DBG("RSSI = %d\n", qvalue.fm.Rssi);
					RAPP_DBG("Modulation = %d\n", qvalue.fm.Mod);
					RAPP_DBG("Offset = %d\n", qvalue.fm.Offs);
					RAPP_DBG("HFN = %d\n", qvalue.fm.Hfn);
					RAPP_DBG("MultiPath = %d\n", qvalue.fm.Mpth);
					RAPP_DBG("Pilot = %d\n", qvalue.fm.Pilot);
				}
				else if(current_band == eRADIO_MW_BAND) {	// MW
					RAPP_DBG("RSSI = %d\n", qvalue.am.Rssi);
					RAPP_DBG("Modulation = %d\n", qvalue.am.Mod);
					RAPP_DBG("Offset = %d\n", qvalue.am.Offs);
				}
				else {
					;	// Other user bands can be set.
				}
			}
		}
	}
	else {
		RAPP_ERR("Not available station list!!!");
	}
}

int32 tcradiocui_operation(char *argv[], int argc)
{
	char *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7;
	int i, err = 0, ret = 0, exit = 0;

	p0 = (0 < argc) ? argv[0] : NULL;
	p1 = (1 < argc) ? argv[1] : NULL;
	p2 = (2 < argc) ? argv[2] : NULL;
	p3 = (3 < argc) ? argv[3] : NULL;
	p4 = (4 < argc) ? argv[4] : NULL;
	p5 = (5 < argc) ? argv[5] : NULL;
	p6 = (6 < argc) ? argv[6] : NULL;
	p7 = (7 < argc) ? argv[7] : NULL;

	if(argc < 1) {
		printf("Input Command. If you need help, enter 'h'.\n");
		printf(">");
		return exit;
	}

	if(SCMP(p0, "h", MAX_SCMP_LEN)==0) {
		tcradiocui_help();
	}
	else if(SCMP(p0, "init", MAX_SCMP_LEN)==0) {
		ret = tcradiocui_initTcRadioService();
		if(ret != eRET_OK) {
			err = -1;
		}
		RAPP_DBG("Init radio service application.\n");
	}
	else if(SCMP(p0, "deinit", MAX_SCMP_LEN)==0) {
		ret = tcradio_deinit();
		if(ret != eRET_OK) {
			if(ret == eRET_NG_NOT_SUPPORT) {
				RAPP_DBG("This cui command is not support yet!!!\n");
			}
			if(ret == eRET_NG_NOT_INIT) {
				RAPP_DBG("Not to init radio service application!!!\n");
			}
			RAPP_DBG("Failed to deinit radio service application!!!\n");
			err = -1;
		}
		else {
			RAPP_DBG("Radio service application is deinit.\n");
		}
	}
	else if(SCMP(p0, "open", MAX_SCMP_LEN)==0) {
		tcradiocui_setRadioConfig(&radioConf);
		if(argc > 1) {
			if(SCMP(p1, "asia", MAX_SCMP_LEN)==0) {
				radioConf.area = eRADIO_CONF_AREA_ASIA;
			}
			else if(SCMP(p1, "na", MAX_SCMP_LEN)==0) {
				radioConf.area = eRADIO_CONF_AREA_NA;
			}
			else {
				if(SCMP(p1, "eu", MAX_SCMP_LEN)!=0) {
					RAPP_DBG("The input is wrong and the area is set to EU!!\n");
				}
				radioConf.area = eRADIO_CONF_AREA_EU;
			}
			if(argc > 2) {
				uint32 ntuners = (uint32)atoi(p2);
				radioConf.numTuners = ntuners;
				if(argc > 3) {
					uint32 pd = (uint32)atoi(p3);
					if(pd) {
						radioConf.fPhaseDiversity = ON;
					}
					else {
						radioConf.fPhaseDiversity = OFF;
					}
					if(argc > 4) {
						uint32 iq = (uint32)atoi(p4);
						if(iq) {
							radioConf.fIqOut = YES;
						}
						else {
							radioConf.fIqOut = NO;
						}
						if(argc > 5) {
							uint32 samplerate = (uint32)atoi(p5);
							if(samplerate == 44100 || samplerate == 48000) {
								radioConf.audioSamplerate = samplerate;
							}
							else {
								RAPP_DBG("The samplerate input is wrong and it is set to 44.1KHz!!\n");
								radioConf.audioSamplerate = 44100;
							}
							if(argc > 6) {
								uint32 fsdr = (uint32)atoi(p6);
								if(fsdr == eRADIO_SDR_HD) {
									radioConf.sdr = eRADIO_SDR_HD;
									radioConf.initMode = eRADIO_FM_MODE;
									radioConf.initFreq = FM_START_FREQ;
								}
								else if(fsdr == eRADIO_SDR_DRM30) {
									radioConf.sdr = eRADIO_SDR_DRM30;
									radioConf.initMode = eRADIO_AM_MODE;
									radioConf.initFreq = MW_START_FREQ;
								}
								else if(fsdr == eRADIO_SDR_DRMP) {
									radioConf.sdr = eRADIO_SDR_DRMP;
									radioConf.initMode = eRADIO_FM_MODE;
									radioConf.initFreq = FM_START_FREQ;
								}
								else if(fsdr == eRADIO_SDR_DAB) {
									radioConf.sdr = eRADIO_SDR_DAB;
									radioConf.initMode = eRADIO_DAB_MODE;
									radioConf.initFreq = DAB_START_INDEX;
								}
								else {
									radioConf.sdr = eRADIO_SDR_NONE;
								}
								if(argc > 7) {
									uint32 hdrType = (uint32)atoi(p7);
									radioConf.hdType = (eRADIO_HD_TYPE_t)hdrType;
								}
							}
							else {
								radioConf.sdr = eRADIO_SDR_HD;
								if(radioConf.numTuners == eRADIO_CONF_TYPE_DUAL) {
								//	radioConf.hdType = eRADIO_HD_TYPE_HD1p0_MRC;
									radioConf.hdType = eRADIO_HD_TYPE_HD1p5;
								}
								else if(radioConf.numTuners == eRADIO_CONF_TYPE_TRIPLE) {
									radioConf.hdType = eRADIO_HD_TYPE_HD1p5_MRC;
								}
								else if(radioConf.numTuners == eRADIO_CONF_TYPE_QUAD) {
									radioConf.hdType = eRADIO_HD_TYPE_HD1p5_DUAL_MRC;
								}
								else {
									radioConf.hdType = eRADIO_HD_TYPE_HD1p0;
								}
							}
						}
					}
				}
			}
		}
		ret = tcradio_open(&radioConf);
		if(ret != eRET_OK) {
			if(ret == eRET_NG_NOT_INIT) {
				RAPP_DBG("Not to init radio service application!!!\n");
			}
			RAPP_DBG("Failed to open radio service application!!!\n");
			err = -1;
		}
		else {
			RAPP_DBG("Radio service application is opening.\n");
		}
	}
	else if(SCMP(p0, "close", MAX_SCMP_LEN)==0) {
		ret = tcradio_close();
		if(ret != eRET_OK) {
			if(ret == eRET_NG_NOT_INIT) {
				RAPP_DBG("Not to init radio service application!!!\n");
			}
			RAPP_DBG("Failed to close radio service application!!!\n");
			err = -1;
		}
		else {
			RAPP_DBG("Radio service application is closed.\n");
		}
	}
	else if(SCMP(p0, "fm", MAX_SCMP_LEN)==0) {
		uint32 freq = 0;
		uint32 ntuner = eRADIO_ID_PRIMARY;

		if(argc == 1) {
			freq = fm_frequency;
		}
		else if(argc > 2) {
			ntuner = (uint32)atoi(p1);
			freq = (uint32)atoi(p2);
		}
		else {
			err = -1;
		}

		if(err >= 0) {
			ret = tcradio_setTune(eRADIO_FM_MODE, freq, eRADIO_TUNE_NORMAL, ntuner);
		}
	}
	else if(SCMP(p0, "am", MAX_SCMP_LEN)==0) {
		uint32 freq = 0;
		uint32 ntuner = eRADIO_ID_PRIMARY;

		if(argc == 1) {
			freq = mw_frequency;
		}
		else if(argc > 2) {
			ntuner = (uint32)atoi(p1);
			freq = (uint32)atoi(p2);
		}
		else {
			err = -1;
		}

		if(err >= 0) {
			ret = tcradio_setTune(eRADIO_AM_MODE, freq, eRADIO_TUNE_NORMAL, ntuner);
		}
	}
	else if(SCMP(p0, "dab", MAX_SCMP_LEN)==0) {
		uint32 index = 0;
		uint32 ntuner = eRADIO_ID_PRIMARY;

		if(radioConf.sdr == eRADIO_SDR_DAB) {
			if(argc == 1) {
				index = dab_freqIndex;
			}
			else if(argc > 2) {
				ntuner = (uint32)atoi(p1);
				index = (uint32)atoi(p2);
			}
			else {
				err = -1;
			}
		}
		else {
			err = -1;
		}

		if(err >= 0) {
			ret = tcradio_setTune(eRADIO_DAB_MODE, index, eRADIO_TUNE_NORMAL, ntuner);
		}
	}
	else if(SCMP(p0, "up", MAX_SCMP_LEN)==0) {
		ret = tcradio_setSeek(eRADIO_SEEK_AUTO_UP, NULL);
		RAPP_DBG("Wait... This operation takes time...\n");
	}
	else if(SCMP(p0, "dn", MAX_SCMP_LEN)==0) {
		ret = tcradio_setSeek(eRADIO_SEEK_AUTO_DOWN, NULL);
		RAPP_DBG("Wait... This operation takes time...\n");
	}
	else if(SCMP(p0, "u", MAX_SCMP_LEN)==0) {
		ret = tcradio_setSeek(eRADIO_SEEK_MAN_UP, NULL);
	}
	else if(SCMP(p0, "d", MAX_SCMP_LEN)==0) {
		ret = tcradio_setSeek(eRADIO_SEEK_MAN_DOWN, NULL);
	}
	else if(SCMP(p0, "list", MAX_SCMP_LEN)==0) {
		ret = tcradio_setSeek(eRADIO_SEEK_SCAN_STATION, NULL);
		RAPP_DBG("Wait... This operation takes time...\n");
	}
	else if(SCMP(p0, "s", MAX_SCMP_LEN)==0) {
		ret = tcradio_setSeek(eRADIO_SEEK_STOP, NULL);
	}
	else if(SCMP(p0, "bf", MAX_SCMP_LEN)==0) {
		if(argc > 1) {
			if(argc == 2) {
				if(SCMP(p1, "fm", MAX_SCMP_LEN)==0) {
					uint32 sfreq=0, efreq=0, step=0;
					ret = tcradio_getBandFreqConfig(eRADIO_FM_MODE, &sfreq, &efreq, &step);
					if(ret == eRET_OK) {
						RAPP_DBG("tcradio_getBandFreqConfig [FM] : startFreq[%d], endFreq[%d], step[%d]\n", sfreq, efreq, step);
					}
				}
				else if(SCMP(p1, "am", MAX_SCMP_LEN)==0) {
					uint32 sfreq=0, efreq=0, step=0;
					ret = tcradio_getBandFreqConfig(eRADIO_AM_MODE, &sfreq, &efreq, &step);
					if(ret == eRET_OK) {
						RAPP_DBG("tcradio_getBandFreqConfig [AM] : startFreq[%d], endFreq[%d], step[%d]\n", sfreq, efreq, step);
					}
				}
				else if(SCMP(p1, "dab", MAX_SCMP_LEN)==0) {
					uint32 sfreq=0, efreq=0, step=0;
					ret = tcradio_getBandFreqConfig(eRADIO_DAB_MODE, &sfreq, &efreq, &step);
					if(ret == eRET_OK) {
						RAPP_DBG("tcradio_getBandFreqConfig [DAB] : startIndex[%d], endIndex[%d], step[%d]\n", sfreq, efreq, step);
					}
				}
				else {
					err = -1;
				}
			}
			else if(argc == 5) {
				uint32 start_freq = (uint32)atoi(p2);
				uint32 end_freq = (uint32)atoi(p3);
				uint32 step = (uint32)atoi(p4);

				if(SCMP(p1, "fm", MAX_SCMP_LEN)==0) {
					ret = tcradio_setBandFreqConfig(eRADIO_FM_MODE, start_freq, end_freq, step);
					if(ret == eRET_OK) {
						RAPP_DBG("tcradio_setBandFreqConfig [FM] : startFreq[%d], endFreq[%d], step[%d]\n", start_freq, end_freq, step);
					}
				}
				else if(SCMP(p1, "am", MAX_SCMP_LEN)==0) {
					ret = tcradio_setBandFreqConfig(eRADIO_AM_MODE, start_freq, end_freq, step);
					if(ret == eRET_OK) {
						RAPP_DBG("tcradio_setBandFreqConfig [AM] : startFreq[%d], endFreq[%d], step[%d]\n", start_freq, end_freq, step);
					}
				}
				else if(SCMP(p1, "dab", MAX_SCMP_LEN)==0) {
					uint8 numOfdabBandIII;
					tcradio_getCurrentDabFreqList(&numOfdabBandIII, eRADIO_ID_PRIMARY);
					if(start_freq < numOfdabBandIII && end_freq < numOfdabBandIII) {
						ret = tcradio_setBandFreqConfig(eRADIO_DAB_MODE, start_freq, end_freq, step);
						if(ret == eRET_OK) {
							RAPP_DBG("tcradio_setBandFreqConfig [DAB] : startIndex[%d], endIndex[%d], step[%d]\n", start_freq, end_freq, step);
						}
					}
					else {
						ret = eRET_NG_INVALID_PARAM;
					}
				}
				else {
					err = -1;
				}
			}
			else {
				err = -1;
				RAPP_DBG("More input parameters are required!!!\n");
			}
		}
		else {
			err = -1;
		}
	}
	else if(SCMP(p0, "t", MAX_SCMP_LEN)==0) {
		uint32 ntuner = eRADIO_ID_PRIMARY;
		eRADIO_MOD_MODE_t mod_mode;
		uint32 tfreq;

		if(argc > 1) {
			ntuner = (uint32)atoi(p1);
		}

		ret = tcradio_getTune(&mod_mode, &tfreq, ntuner);

		if(ret == eRET_OK) {
			RAPP_DBG("tcradio_getTune : Done! band[%d] freq[%d] ntuner[%d]\n", mod_mode, tfreq, ntuner);
		}
	}
	else if(SCMP(p0, "q", MAX_SCMP_LEN)==0) {
		uint32 ntuner = eRADIO_ID_PRIMARY;
		eRADIO_MOD_MODE_t mod_mode;
		stRADIO_QUALITY_t qdata;

		if(argc > 1) {
			ntuner = (uint32)atoi(p1);
		}

		if(current_band == eRADIO_FM_BAND) {
			ret = tcradio_getQuality(eRADIO_FM_MODE, &qdata, ntuner);
		}
		else if(current_band == eRADIO_MW_BAND) {
			ret = tcradio_getQuality(eRADIO_AM_MODE, &qdata, ntuner);
		}
		else if(current_band == eRADIO_DAB_BAND) {
			ret = tcradio_getQuality(eRADIO_DAB_MODE, &qdata, ntuner);
		}
		else {
			ret = eRET_NG_NOT_SUPPORT;
		}

		if(ret == eRET_OK) {
			RAPP_DBG("==== Tuner[%d] Quality ====\n", ntuner);
			if(qdata.type == eRADIO_TYPE1)
			{
				stRADIO_TYPE1_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)(&qdata.qual), sizeof(qvalue));

				if(current_band == eRADIO_FM_BAND) {	// FM
					RAPP_DBG("RSSI = %d\n", qvalue.fm.Rssi);
					RAPP_DBG("SNR = %d\n", qvalue.fm.Snr);
					RAPP_DBG("Deviation = %d\n", qvalue.fm.Dev);
					RAPP_DBG("Offset = %d\n", qvalue.fm.Offs);
					RAPP_DBG("Pilot = %d\n", qvalue.fm.Pilot);
					RAPP_DBG("MultiPath = %d\n", qvalue.fm.Mpth);
					RAPP_DBG("USN = %d\n", qvalue.fm.Usn);
				}
				else if(current_band == eRADIO_MW_BAND) {	// AM
					RAPP_DBG("RSSI = %d\n", qvalue.am.Rssi);
					RAPP_DBG("SNR = %d\n", qvalue.am.Snr);
					RAPP_DBG("Modulation = %d\n", qvalue.am.Mod);
					RAPP_DBG("Offset = %d\n", qvalue.am.Offs);
				}
				else if(current_band == eRADIO_DAB_BAND) {	// DAB
					RAPP_DBG("RSSI = %d\n", qvalue.dab.Rssi);
					RAPP_DBG("SQI = %d\n", qvalue.dab.Sqi);
					RAPP_DBG("DETECT = %d\n", qvalue.dab.Detect);
					RAPP_DBG("RSSIADJ = %d\n", qvalue.dab.RssiAdj);
					RAPP_DBG("DAGC = %d\n", qvalue.dab.Dagc);
					RAPP_DBG("VALID = 0x%02x\n", qvalue.dab.Valid);
				}
				else {
					ret = eRET_NG_NOT_SUPPORT;
				}
			}
			else if(qdata.type == eRADIO_TYPE2)
			{
				stRADIO_TYPE2_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)(&qdata.qual), sizeof(qvalue));

				if(current_band == eRADIO_FM_BAND) {	// FM
					RAPP_DBG("Status = %xh\n", qvalue.fm.Status);
					RAPP_DBG("RSSI = %d\n", qvalue.fm.Rssi);
					RAPP_DBG("USN = %d\n", qvalue.fm.Usn);
					RAPP_DBG("MultiPath = %d\n", qvalue.fm.Mpth);
					RAPP_DBG("Offset = %d\n", qvalue.fm.Offs);
					RAPP_DBG("Bandwidth = %d\n", qvalue.fm.Bwth);
					RAPP_DBG("Modulation = %d\n", qvalue.fm.Mod);
				}
				else if(current_band == eRADIO_MW_BAND) {	// AM
					RAPP_DBG("Status = %xh\n", qvalue.am.Status);
					RAPP_DBG("RSSI = %d\n", qvalue.am.Rssi);
					RAPP_DBG("High Frequency Noise Detector = %d\n", qvalue.am.Hfn);
					RAPP_DBG("Co-Channel Detector = %d\n", qvalue.am.Coch);
					RAPP_DBG("Offset = %d\n", qvalue.am.Offs);
					RAPP_DBG("Bandwidth = %d\n", qvalue.am.Bwth);
					RAPP_DBG("Modulation = %d\n", qvalue.am.Mod);
				}
				else {
					ret = eRET_NG_NOT_SUPPORT;
				}
			}
			else if(qdata.type == eRADIO_TYPE3)
			{
				stRADIO_TYPE3_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)(&qdata.qual), sizeof(qvalue));

				if(current_band == eRADIO_AM_MODE) {	// AM
					RAPP_DBG("FstRF = %d dBuV\n", qvalue.am.FstRF);
					RAPP_DBG("FstBB = %d dBuV\n", qvalue.am.FstBB);
					RAPP_DBG("Detuning = %d\n", qvalue.am.Det);
					RAPP_DBG("SNR = %d \%\n", qvalue.am.Snr);
					RAPP_DBG("Adjacent = %d \%\n", qvalue.am.Adj);
					RAPP_DBG("Deviation = %d\n", qvalue.am.Dev);
				}
				else if(current_band == eRADIO_FM_MODE) {	// FM
					RAPP_DBG("FstRF = %d dBuV\n", qvalue.fm.FstRF);
					RAPP_DBG("FstBB = %d dBuV\n", qvalue.fm.FstBB);
					RAPP_DBG("Detuning = %d\n", qvalue.fm.Det);
					RAPP_DBG("SNR = %d \%\n", qvalue.fm.Snr);
					RAPP_DBG("Adjacent = %d \%\n", qvalue.fm.Adj);
					RAPP_DBG("Deviation = %d\n", qvalue.fm.Dev);
					RAPP_DBG("Multipath = %d \%\n", qvalue.fm.Mpth);
					RAPP_DBG("MpxNoise = %d \%\n", qvalue.fm.MpxNoise);
					RAPP_DBG("Stereo = %d\n", qvalue.fm.Stereo);
					RAPP_DBG("CoCh = %d \%\n", qvalue.fm.CoCh);
				}
				else if(current_band == eRADIO_DAB_MODE) {	// DAB
					RAPP_DBG("FstRF = %d dBm\n", qvalue.dab.FstRF);
					RAPP_DBG("FstBB = %d dBm\n", qvalue.dab.FstBB);
				}
				else {
					ret = eRET_NG_NOT_SUPPORT;
				}
			}
			else {
				stRADIO_TYPE0_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)(&qdata.qual), sizeof(qvalue));

				if(current_band == eRADIO_FM_BAND) {	// FM
					RAPP_DBG("RSSI = %d\n", qvalue.fm.Rssi);
					RAPP_DBG("Modulation = %d\n", qvalue.fm.Mod);
					RAPP_DBG("Offset = %d\n", qvalue.fm.Offs);
					RAPP_DBG("HFN = %d\n", qvalue.fm.Hfn);
					RAPP_DBG("MultiPath = %d\n", qvalue.fm.Mpth);
					RAPP_DBG("Pilot = %d\n", qvalue.fm.Pilot);
				}
				else if(current_band == eRADIO_MW_BAND) {	// AM
					RAPP_DBG("RSSI = %d\n", qvalue.am.Rssi);
					RAPP_DBG("Modulation = %d\n", qvalue.am.Mod);
					RAPP_DBG("Offset = %d\n", qvalue.am.Offs);
				}
				else {
					ret = eRET_NG_NOT_SUPPORT;
				}
			}
		}
	}
	else if(SCMP(p0, "v", MAX_SCMP_LEN)==0) {
		RAPP_DBG("Middleware Version : %c%d.%d.%d \n", tcradio_getMiddlewareVersion()>>24, tcradio_getMiddlewareVersion()>>16 & 0x00ff,
													   tcradio_getMiddlewareVersion()>>8 & 0x00ff, tcradio_getMiddlewareVersion() & 0x00ff);
		RAPP_DBG("HAL Version : %c%d.%d.%d \n", tcradio_getHalVersion()>>24, tcradio_getHalVersion()>>16 & 0x00ff,
												   tcradio_getHalVersion()>>8 & 0x00ff, tcradio_getHalVersion() & 0x00ff);
		RAPP_DBG("Driver Version : %c%d.%d.%d \n", tcradio_getDriverVersion()>>24, tcradio_getDriverVersion()>>16 & 0x00ff,
												   tcradio_getDriverVersion()>>8 & 0x00ff, tcradio_getDriverVersion() & 0x00ff);
	}
	else if(SCMP(p0, "mute", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			if(SCMP(p1, "on", MAX_SCMP_LEN)==0) {
				ret = tcradio_setAudio(0);
				if(ret == eRET_OK) {
					RAPP_DBG("Mute On!!\n");
				}
			}
			else if(SCMP(p1, "off", MAX_SCMP_LEN)==0) {
				ret = tcradio_setAudio(1);
				if(ret == eRET_OK) {
					RAPP_DBG("Mute Off!!\n");
				}
			}
			else {
				err = -1;
			}
		}
		else {
			err = -1;
		}
	}
	else if(SCMP(p0, "audio", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			if(SCMP(p1, "on", MAX_SCMP_LEN)==0) {
				ret = tcradio_setAudioDevice(&radioConf, ON);
				if(ret == eRET_OK) {
					RAPP_DBG("The audio device turned on to successfully!!\n");
				}
			}
			else if(SCMP(p1, "off", MAX_SCMP_LEN)==0) {
				ret = tcradio_setAudioDevice(&radioConf, OFF);
				if(ret == eRET_OK) {
					RAPP_DBG("The audio device turned off to successfully!!\n");
				}
			}
			else {
				err = -1;
			}
		}
		else {
			err = -1;
		}
	}
	else if(SCMP(p0, "rds", MAX_SCMP_LEN)==0) {
		if(argc > 1) {
			if(SCMP(p1, "on", MAX_SCMP_LEN)==0) {
				ret = tcradio_setRdsEnable(1);
				if(ret == eRET_OK) {
					RAPP_DBG("Rds is enabled!!!\n");
				}
			}
			else if(SCMP(p1, "off", MAX_SCMP_LEN)==0) {
				ret = tcradio_setRdsEnable(0);
				if(ret == eRET_OK) {
					RAPP_DBG("Rds is disabled!!!\n");
				}
			}
			else if(SCMP(p1, "pi", MAX_SCMP_LEN)==0) {
				uint16 pi=0;
				ret = tcradio_getRdsPi(&pi);
				if(ret == eRET_OK) {
					RAPP_DBG("Current PI = %04xh\n", pi);
				}
			}
			else if(SCMP(p1, "pty", MAX_SCMP_LEN)==0) {
				uint8 pty=0;
				ret = tcradio_getRdsPty(&pty);
				if(ret == eRET_OK) {
					RAPP_DBG("Current PTY = %02xh\n", pty);
				}
			}
			else if(SCMP(p1, "psn", MAX_SCMP_LEN)==0) {
				uint8 psn[MAX_PS+1] = {0, };
				ret = tcradio_getRdsPsn(psn);
				if(ret == eRET_OK) {
					RAPP_DBG("Current PS Name = %s\n", psn);
				}
			}
			else if(SCMP(p1, "piscan", MAX_SCMP_LEN)==0) {
				if(argc == 3) {
					uint32 pilist[128] = {0,};
					pilist[0] = (uint32)strtol(p2, NULL, 16);
					RAPP_DBG("pi scan list [0]=%xh\n", pilist[0]);
					if(pilist[0] > 0 && pilist[0] < 0x10000)
					{
						tcradio_setSeek(eRADIO_SEEK_SCAN_PI, pilist);
					}
				}
				else if(argc == 4) {
					uint32 pilist[128] = {0,};
					pilist[0] = (uint32)strtol(p2, NULL, 16);
					pilist[1] = (uint32)strtol(p3, NULL, 16);
					RAPP_DBG("pi scan list [0]=%xh [1]=%xh\n", pilist[0], pilist[1]);
					if( (pilist[0] > 0 && pilist[0] < 0x10000) &&
						(pilist[1] > 0 && pilist[1] < 0x10000) )
					{
						tcradio_setSeek(eRADIO_SEEK_SCAN_PI, pilist);
					}
				}
				else if(argc == 5) {
					uint32 pilist[128] = {0,};
					pilist[0] = (uint32)strtol(p2, NULL, 16);
					pilist[1] = (uint32)strtol(p3, NULL, 16);
					pilist[2] = (uint32)strtol(p4, NULL, 16);
					RAPP_DBG("pi scan list [0]=%xh [1]=%xh [2]=%xh\n", pilist[0], pilist[1], pilist[2]);
					if( (pilist[0] > 0 && pilist[0] < 0x10000) &&
						(pilist[1] > 0 && pilist[1] < 0x10000) &&
						(pilist[2] > 0 && pilist[2] < 0x10000) )
					{
						tcradio_setSeek(eRADIO_SEEK_SCAN_PI, pilist);
					}
				}
				else {
					err = -1;
				}
			}
			else {
				err = -1;
			}
		}
		else {
			err = -1;
		}
	}
	else if(SCMP(p0, "dump", MAX_SCMP_LEN)==0) {	// only for test
		if(argc == 2) {
			int32 bit = (int32)atoi(p1);
			ret = tcradio_dumpIQ(bit, 1024, NULL, NULL, 2);
		}
		else if(argc == 3) {
			int32 bit = (int32)atoi(p1);
			int32 bufsize = (int32)atoi(p2);
			ret = tcradio_dumpIQ(bit, bufsize, NULL, NULL, 2);
		}
		else if(argc == 4) {
			int32 bit = (int32)atoi(p1);
			int32 bufsize = (int32)atoi(p2);
			uint32 comp = (uint32)atoi(p3);
			ret = tcradio_dumpIQ(bit, bufsize, NULL, comp, 2);
		}
		else if(argc == 5) {
			int32 bit = (int32)atoi(p1);
			int32 bufsize = (int32)atoi(p2);
			uint32 comp = (uint32)atoi(p3);
			int32 dumpsize = (uint32)atoi(p4);
			ret = tcradio_dumpIQ(bit, bufsize, NULL, comp, dumpsize);
		}
		else if(argc == 6) {
			int32 bit = (int32)atoi(p1);
			int32 bufsize = (int32)atoi(p2);
			uint32 comp = (uint32)atoi(p3);
			int32 dumpsize = (uint32)atoi(p4);
			int32 readsize = (uint32)atoi(p5);
			ret = tcradio_dumpIQ(bit, bufsize, readsize, comp, dumpsize);
		}
		else {
			err = -1;
		}

		if(ret != eRET_OK) {
			err = -1;
		}

		if(err == 0) {
			RAPP_DBG("Dump tuner IQ data.\n");
		}
		else {
			RAPP_DBG("Failed to dump tuner IQ data.\n");
		}
	}
	else if(SCMP(p0, "spi0", MAX_SCMP_LEN)==0) {	// only for spi test
		uint8 rxdata[256] = {0,};
		if(argc == 3) {
			uint8 txdata[256] = {0,};
			txdata[0] = (uint8)strtol(p1, NULL, 16);
			txdata[1] = (uint8)strtol(p2, NULL, 16);
			ret = tcradio_setTunerCommand(txdata, 2, rxdata, 4, eRADIO_ID_PRIMARY);
			if(ret != eRET_OK) {
				RAPP_DBG("Failed to tx data!!\n");
			}
			else {
				RAPP_DBG("txdata = %xh %xh\n", txdata[0], txdata[1]);
			}
		}
		else if(argc == 4) {
			uint8 txdata[256] = {0,};
			txdata[0] = (uint8)strtol(p1, NULL, 16);
			txdata[1] = (uint8)strtol(p2, NULL, 16);
			txdata[2] = (uint8)strtol(p3, NULL, 16);
			ret = tcradio_setTunerCommand(txdata, 3, rxdata, 4, eRADIO_ID_PRIMARY);
			if(ret != eRET_OK) {
				RAPP_DBG("Failed to tx data!!\n");
			}
			else {
				RAPP_DBG("txdata = %xh %xh %xh\n", txdata[0], txdata[1], txdata[2]);
			}
		}
		else if(argc == 5) {
			uint8 txdata[256] = {0,};
			txdata[0] = (uint8)strtol(p1, NULL, 16);
			txdata[1] = (uint8)strtol(p2, NULL, 16);
			txdata[2] = (uint8)strtol(p3, NULL, 16);
			txdata[3] = (uint8)strtol(p4, NULL, 16);
			ret = tcradio_setTunerCommand(txdata, 4, rxdata, 4, eRADIO_ID_PRIMARY);
			if(ret != eRET_OK) {
				RAPP_DBG("Failed to tx data!!\n");
			}
			else {
				RAPP_DBG("txdata = %xh %xh %xh %xh\n", txdata[0], txdata[1], txdata[2], txdata[3]);
			}
		}
		else if(argc == 6) {
			uint8 txdata[256] = {0,};
			txdata[0] = (uint8)strtol(p1, NULL, 16);
			txdata[1] = (uint8)strtol(p2, NULL, 16);
			txdata[2] = (uint8)strtol(p3, NULL, 16);
			txdata[3] = (uint8)strtol(p4, NULL, 16);
			txdata[4] = (uint8)strtol(p5, NULL, 16);
			ret = tcradio_setTunerCommand(txdata, 5, rxdata, 4, eRADIO_ID_PRIMARY);
			if(ret != eRET_OK) {
				RAPP_DBG("Failed to tx data!!\n");
			}
			else {
				RAPP_DBG("txdata = %xh %xh %xh %xh %xh\n", txdata[0], txdata[1], txdata[2], txdata[3], txdata[4]);
			}
		}
		else if(argc == 7) {
			uint8 txdata[256] = {0,};
			txdata[0] = (uint8)strtol(p1, NULL, 16);
			txdata[1] = (uint8)strtol(p2, NULL, 16);
			txdata[2] = (uint8)strtol(p3, NULL, 16);
			txdata[3] = (uint8)strtol(p4, NULL, 16);
			txdata[4] = (uint8)strtol(p5, NULL, 16);
			txdata[5] = (uint8)strtol(p6, NULL, 16);
			ret = tcradio_setTunerCommand(txdata, 6, rxdata, 4, eRADIO_ID_PRIMARY);
			if(ret != eRET_OK) {
				RAPP_DBG("Failed to tx data!!\n");
			}
			else {
				RAPP_DBG("txdata = %xh %xh %xh %xh %xh %xh\n", txdata[0], txdata[1], txdata[2], txdata[3], txdata[4], txdata[5]);
			}
		}
		else {
			err = -1;
		}
	}
	else if(SCMP(p0, "spi1", MAX_SCMP_LEN)==0) {	// only for spi test
		uint8 rxdata[256] = {0,};
		if(argc == 3) {
			uint8 txdata[256] = {0,};
			txdata[0] = (uint8)strtol(p1, NULL, 16);
			txdata[1] = (uint8)strtol(p2, NULL, 16);
			ret = tcradio_setTunerCommand(txdata, 2, rxdata, 4, eRADIO_ID_TERTIARY);
			if(ret != eRET_OK) {
				RAPP_DBG("Failed to tx data!!\n");
			}
			else {
				RAPP_DBG("txdata = %xh %xh\n", txdata[0], txdata[1]);
			}
		}
		else if(argc == 4) {
			uint8 txdata[256] = {0,};
			txdata[0] = (uint8)strtol(p1, NULL, 16);
			txdata[1] = (uint8)strtol(p2, NULL, 16);
			txdata[2] = (uint8)strtol(p3, NULL, 16);
			ret = tcradio_setTunerCommand(txdata, 3, rxdata, 4, eRADIO_ID_TERTIARY);
			if(ret != eRET_OK) {
				RAPP_DBG("Failed to tx data!!\n");
			}
			else {
				RAPP_DBG("txdata = %xh %xh %xh\n", txdata[0], txdata[1], txdata[2]);
			}
		}
		else if(argc == 5) {
			uint8 txdata[256] = {0,};
			txdata[0] = (uint8)strtol(p1, NULL, 16);
			txdata[1] = (uint8)strtol(p2, NULL, 16);
			txdata[2] = (uint8)strtol(p3, NULL, 16);
			txdata[3] = (uint8)strtol(p4, NULL, 16);
			ret = tcradio_setTunerCommand(txdata, 4, rxdata, 4, eRADIO_ID_TERTIARY);
			if(ret != eRET_OK) {
				RAPP_DBG("Failed to tx data!!\n");
			}
			else {
				RAPP_DBG("txdata = %xh %xh %xh %xh\n", txdata[0], txdata[1], txdata[2], txdata[3]);
			}
		}
		else if(argc == 6) {
			uint8 txdata[256] = {0,};
			txdata[0] = (uint8)strtol(p1, NULL, 16);
			txdata[1] = (uint8)strtol(p2, NULL, 16);
			txdata[2] = (uint8)strtol(p3, NULL, 16);
			txdata[3] = (uint8)strtol(p4, NULL, 16);
			txdata[4] = (uint8)strtol(p5, NULL, 16);
			ret = tcradio_setTunerCommand(txdata, 5, rxdata, 4, eRADIO_ID_TERTIARY);
			if(ret != eRET_OK) {
				RAPP_DBG("Failed to tx data!!\n");
			}
			else {
				RAPP_DBG("txdata = %xh %xh %xh %xh %xh\n", txdata[0], txdata[1], txdata[2], txdata[3], txdata[4]);
			}
		}
		else if(argc == 7) {
			uint8 txdata[256] = {0,};
			txdata[0] = (uint8)strtol(p1, NULL, 16);
			txdata[1] = (uint8)strtol(p2, NULL, 16);
			txdata[2] = (uint8)strtol(p3, NULL, 16);
			txdata[3] = (uint8)strtol(p4, NULL, 16);
			txdata[4] = (uint8)strtol(p5, NULL, 16);
			txdata[5] = (uint8)strtol(p6, NULL, 16);
			ret = tcradio_setTunerCommand(txdata, 6, rxdata, 4, eRADIO_ID_TERTIARY);
			if(ret != eRET_OK) {
				RAPP_DBG("Failed to tx data!!\n");
			}
			else {
				RAPP_DBG("txdata = %xh %xh %xh %xh %xh %xh\n", txdata[0], txdata[1], txdata[2], txdata[3], txdata[4], txdata[5]);
			}
		}
		else {
			err = -1;
		}
	}
	else if(SCMP(p0, "sts", MAX_SCMP_LEN)==0) {
		// This is test command to confirm tuner status registers.
		uint8 rxdata[256] = {0,};
		uint32 cmd = 0;
		uint32 len = 0;
		int32 ntuner = 0;

		if(argc == 3) {
			ntuner = (int32)atoi(p2);
			if(tcradio_getTunerChip() == eRADIO_TYPE1) {
				if(SCMP(p1, "agc", MAX_SCMP_LEN)==0) {
					cmd = 0x1700;
					len = 25;
				}
				else if(SCMP(p1, "servo", MAX_SCMP_LEN)==0) {
					if(current_band == eRADIO_FM_BAND) {
						cmd = 0x3700;
						len = 14;
					}
					else if(current_band == eRADIO_MW_BAND) {
						cmd = 0x4700;
						len = 13;
					}
					else {
						err = -1;
					}
				}
				else {
					err = -1;
				}
			}
			else {
				RAPP_DBG("Not support this tuner chip!!!\n");
				err = -1;
			}
		}
		else if(argc == 4) {
			// Users should confirm the receive data length in the spec and input it.
			cmd = (uint32)strtol(p1, NULL, 16);
			len = (int32)atoi(p2);
			ntuner = (int32)atoi(p3);
			if(tcradio_getTunerChip() != eRADIO_TYPE1) {
				RAPP_DBG("Not support this tuner chip!!!\n");
				err = -1;
			}
		}
		else {
			err = -1;
		}

		if(err == 0) {
			ret = tcradio_getTunerStatus(cmd, rxdata, len, ntuner);
		#ifdef RAPP_DEBUG
			if(ret == eRET_OK) {
				int i;
				RAPP_DBG("[Tuner Status] CMD[%02xh] ", cmd);
				for(i=0; i<len; i++) {
					printf("[%02d]%02xh ", i, rxdata[i]);
				}
				printf("\n");
			}
		#endif
		}
	}
#ifdef USE_HDRADIO
	else if(SCMP(p0, "hdaudio", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			int32 mode = (int32)atoi(p1);
			switch(mode) {
				case 0: ret = tcradio_setHdrAudioMode(eTC_HDR_AUDIO_BLEND);		break;
				case 1: ret = tcradio_setHdrAudioMode(eTC_HDR_AUDIO_ANALOG);	break;
				case 2: ret = tcradio_setHdrAudioMode(eTC_HDR_AUDIO_DIGITAL);	break;
				case 3: ret = tcradio_setHdrAudioMode(eTC_HDR_AUDIO_SPLIT);		break;
				default: ret = eRET_NG_INVALID_PARAM;							break;
			}
		}
	}
	else if(SCMP(p0, "hdsts", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			int32 id = (int32)atoi(p1);
			if(id >= eTC_HDR_ID_MAIN && id <= eTC_HDR_ID_BS) {
				stTC_HDR_STATUS_t hdrStatus;
				ret = tcradio_getHdrAllStatus((eTC_HDR_ID_t)id, &hdrStatus);
				if(ret == 0) {
					int i=0;
					RAPP_DBG("00) HDR ID: %d\n", hdrStatus.hdrID);
					RAPP_DBG("01) Playing Program Number: %d\n", hdrStatus.curPN);
					RAPP_DBG("02) Acquired Status: 0x%02x, digital_audio_acquired[%d]:sis_crc_ok[%d]:sis_acquired[%d]:hd_signal_acquired[%d]\n",
							hdrStatus.acqStatus, hdrStatus.acqStatus&0x08 ? 1:0, hdrStatus.acqStatus&0x04 ? 1:0, hdrStatus.acqStatus&0x02 ? 1:0, hdrStatus.acqStatus&0x01 ? 1:0);
					RAPP_DBG("03) Digital Audio Quality Indicator: %d\n", hdrStatus.audioQualityIndicator);
					RAPP_DBG("04) CNR: %d\n", hdrStatus.cnr);
					RAPP_DBG("05) TX Digital Audio Gain: %d\n", hdrStatus.digitalAudioGain);
					RAPP_DBG("06) TX Blend Control: %d\n", hdrStatus.blendControl);
					for(i=0; i<8; i++) {
						if(i==0) RAPP_DBG("07) Program Types: ");
						printf("[PTY%d: %d] ", i, hdrStatus.pty[i]);
						if(i==7) printf("\n");
					}
					RAPP_DBG("08) Playing Program Type: %d\n", hdrStatus.curPty);
					RAPP_DBG("09) Available Program Bitmap: 0x%02x\n", hdrStatus.pmap);
					RAPP_DBG("10) Changed PSD Program Bitmap: 0x%02x\n", hdrStatus.chgPmap);
					RAPP_DBG("11) Primary Service Mode(PSM): %d\n", hdrStatus.psm);
					RAPP_DBG("12) Codec Mode: %d\n", hdrStatus.codecMode);
					RAPP_DBG("13) Filtered Digital Siganal Quality Measurement(DSQM): %d\n", hdrStatus.dsqm);
					RAPP_DBG("14) Hybrid Program Status: %d\n", hdrStatus.hybridProgram);
					RAPP_DBG("15) Raw SNR: %d\n", hdrStatus.rawSnr);
				}
			}
			else {
				err = -1;
			}
		}
	}
	else if(SCMP(p0, "hdsig", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			int32 id = (int32)atoi(p1);
			if(id >= eTC_HDR_ID_MAIN && id <= eTC_HDR_ID_BS) {
				stTC_HDR_SIGNAL_STATUS_t hdrSigStatus;
				ret = tcradio_getHdrSignalStatus((eTC_HDR_ID_t)id, &hdrSigStatus);
				if(ret == 0) {
					RAPP_DBG("00) HDR ID: %d\n", hdrSigStatus.hdrID);
					RAPP_DBG("01) Playing Program Number: %d\n", hdrSigStatus.curPN);
					RAPP_DBG("02) Available Program Bitmap: %d\n", hdrSigStatus.pmap);
					RAPP_DBG("03) Acquired Status: 0x%02x, digital_audio_acquired[%d]:sis_crc_ok[%d]:sis_acquired[%d]:hd_signal_acquired[%d]\n",
							hdrSigStatus.acqStatus, hdrSigStatus.acqStatus&0x08 ? 1:0, hdrSigStatus.acqStatus&0x04 ? 1:0, hdrSigStatus.acqStatus&0x02 ? 1:0, hdrSigStatus.acqStatus&0x01 ? 1:0);
					RAPP_DBG("04) CNR: %d\n", hdrSigStatus.cnr);
					RAPP_DBG("05) Hybrid Program Status: %d\n", hdrSigStatus.hybridProgram);
				}
			}
			else {
				err = -1;
			}
		}
	}
	else if(SCMP(p0, "hdprog", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			int32 nProg = (int32)atoi(p1);
			if(nProg >= 0 && nProg < 8) {
				ret = tcradio_setHdrProgram(nProg);
				if(ret == eTC_HDR_RET_OK){
					ret = tcradio_setHdrPsdNotification(nProg, PSD_BITMASK);
				}
			}
			else {
				err = -1;
			}
		}
	}
	else if(SCMP(p0, "hdtest", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			int32 numCmd = (int32)atoi(p1);
			ret = tcradio_setTcHdrApiTest(numCmd, (uint32*)pNULL);
		}
	}
	else if(SCMP(p0, "hdinit", MAX_SCMP_LEN)==0) {
		if(argc == 1) {
			ret = tcradioservice_initHdr(radioConf.hdType, eTC_HDR_BBSRC_744_KHZ, 16);
		}
	}
	else if(SCMP(p0, "hdopen", MAX_SCMP_LEN)==0) {
		if(argc == 1) {
			if(current_band == eRADIO_FM_BAND) {
				ret = tcradioservice_openHdr(eRADIO_FM_MODE, current_frequency, eTC_HDR_BBSRC_744_KHZ);
			}
			else if(current_band == eRADIO_MW_BAND) {
				ret = tcradioservice_openHdr(eRADIO_AM_MODE, current_frequency, eTC_HDR_BBSRC_744_KHZ);
			}
			else {
				ret = eRET_NG_NOT_SUPPORT;
			}
		}
	}
	else if(SCMP(p0, "hdclose", MAX_SCMP_LEN)==0) {
		if(argc == 1) {
			tcradio_setAudioDevice(&radioConf, OFF);
			ret = tcradioservice_closeHdr();
		}
	}
	else if(SCMP(p0, "hddeinit", MAX_SCMP_LEN)==0) {
		if(argc == 1) {
			ret = tcradioservice_deinitHdr();
		}
	}
	else if(SCMP(p0, "hdaaa", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			uint32 fEn = (uint32)atoi(p1);
			ret = tcradio_setHdrAutoAudioAlignEnable(fEn);
		}
	}
	else if(SCMP(p0, "hdsetprio", MAX_SCMP_LEN)==0) {
		if(argc == 3) {
			eTC_HDR_THREAD_t selthread;
			stTC_HDR_THREAD_PR_t userprio;
			userprio.policy = (int32)atoi(p1);
			userprio.priority = (int32)atoi(p2);
			if(SCMP(p1, "iqinput", MAX_SCMP_LEN)==0) {
				selthread = eTHREAD_IQINPUT;
			}
			else if(SCMP(p1, "audinput", MAX_SCMP_LEN)==0) {
				selthread = eTHREAD_AUDINPUT;
			}
			else if(SCMP(p1, "bbinput", MAX_SCMP_LEN)==0) {
				selthread = eTHREAD_BBINPUT;
			}
			else if(SCMP(p1, "demod", MAX_SCMP_LEN)==0) {
				selthread = eTHREAD_DEMOD;
			}
			else if(SCMP(p1, "blending", MAX_SCMP_LEN)==0) {
				selthread = eTHREAD_BLENDING;
			}
			else if(SCMP(p1, "audoutput", MAX_SCMP_LEN)==0) {
				selthread = eTHREAD_AUDOUTPUT;
			}
			else if(SCMP(p1, "manager", MAX_SCMP_LEN)==0) {
				selthread = eTHREAD_MANAGER;
			}
			else if(SCMP(p1, "cmdproc", MAX_SCMP_LEN)==0) {
				selthread = eTHREAD_CMDPROC;
			}
			else if(SCMP(p1, "logger", MAX_SCMP_LEN)==0) {
				selthread = eTHREAD_LOGGER;
			}
			else {
				err = -1;
			}

			if(err >= 0) {
				ret = tcradio_setHdrThreadPriority(selthread, userprio);
			}
		}
		else {
			err = -1;
		}
	}
	else if(SCMP(p0, "hdgetdprio", MAX_SCMP_LEN)==0) {
		if(argc == 1) {
			stTC_HDR_THREAD_PR_t userprio;
			tcradio_getHdrDefaultThreadPriority(eTHREAD_MANAGER, &userprio);
			RAPP_DBG("Manager thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrDefaultThreadPriority(eTHREAD_IQINPUT, &userprio);
			RAPP_DBG("IQ input thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrDefaultThreadPriority(eTHREAD_AUDINPUT, &userprio);
			RAPP_DBG("Audio input thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrDefaultThreadPriority(eTHREAD_BBINPUT, &userprio);
			RAPP_DBG("BBinput thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrDefaultThreadPriority(eTHREAD_DEMOD, &userprio);
			RAPP_DBG("Demod thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrDefaultThreadPriority(eTHREAD_BLENDING, &userprio);
			RAPP_DBG("Audio blending thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrDefaultThreadPriority(eTHREAD_AUDOUTPUT, &userprio);
			RAPP_DBG("Audio output thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrDefaultThreadPriority(eTHREAD_CMDPROC, &userprio);
			RAPP_DBG("Command Process thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrDefaultThreadPriority(eTHREAD_LOGGER, &userprio);
			RAPP_DBG("Logger thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
		}
	}
	else if(SCMP(p0, "hdgetprio", MAX_SCMP_LEN)==0) {
		if(argc == 1) {
			stTC_HDR_THREAD_PR_t userprio;
			tcradio_getHdrThreadPriority(eTHREAD_MANAGER, &userprio);
			RAPP_DBG("Manager thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrThreadPriority(eTHREAD_IQINPUT, &userprio);
			RAPP_DBG("IQ input thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrThreadPriority(eTHREAD_AUDINPUT, &userprio);
			RAPP_DBG("Audio input thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrThreadPriority(eTHREAD_BBINPUT, &userprio);
			RAPP_DBG("BBinput thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrThreadPriority(eTHREAD_DEMOD, &userprio);
			RAPP_DBG("Demod thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrThreadPriority(eTHREAD_BLENDING, &userprio);
			RAPP_DBG("Audio blending thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrThreadPriority(eTHREAD_AUDOUTPUT, &userprio);
			RAPP_DBG("Audio output thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrThreadPriority(eTHREAD_CMDPROC, &userprio);
			RAPP_DBG("Command Process thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
			tcradio_getHdrThreadPriority(eTHREAD_LOGGER, &userprio);
			RAPP_DBG("Logger thread policy: [%d], priority: [%d]\n",userprio.policy, userprio.priority);
		}
	}
	else if(SCMP(p0, "hdmute", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			if(SCMP(p1, "on", MAX_SCMP_LEN)==0) {
				ret = tcradioservice_setHdrMute(ON);
				if(ret == eRET_OK) {
					RAPP_DBG("Mute On!!\n");
				}
			}
			else if(SCMP(p1, "off", MAX_SCMP_LEN)==0) {
				ret = tcradioservice_setHdrMute(OFF);
				if(ret == eRET_OK) {
					RAPP_DBG("Mute Off!!\n");
				}
			}
			else {
				err = -1;
			}
		}
		else {
			err = -1;
		}
	}
	else if(SCMP(p0, "hdfader", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			if(SCMP(p1, "get", MAX_SCMP_LEN)==0) {
				uint32 en;
				uint32 inms;
				uint32 outms;
				ret = tcradioservice_getHdrAudioMuteFader(&en, &inms, &outms);
				if(ret == eRET_OK) {
					RAPP_DBG("Get fader parameters!! enable[%d], fade-in[%d]ms, fade-out[%d]ms\n", en, inms, outms);
				}
			}
			else {
				err = -1;
			}
		}
		else if(argc == 5) {
			if(SCMP(p1, "set", MAX_SCMP_LEN)==0) {
				int32 en = (int32)atoi(p2);
				int32 inms = (int32)atoi(p3);
				int32 outms = (int32)atoi(p4);
				ret = tcradioservice_setHdrAudioMuteFader((uint32)en, (uint32)inms, (uint32)outms);
				if(ret == eRET_OK) {
					RAPP_DBG("Set fader parameters!!\n");
				}
			}
			else {
				err = -1;
			}
		}
		else {
			err = -1;
		}
	}
	else if(SCMP(p0, "hdbufreset", MAX_SCMP_LEN)==0) {
		if(argc == 1) {
			uint32 numdbg = 0, pArg[2]={0,};
			ret = tcradio_debugHdr(0, numdbg, pArg);
		}
	}
	else if(SCMP(p0, "hddrvreset", MAX_SCMP_LEN)==0) {
		if(argc == 1) {
			uint32 numdbg = 1, pArg[2]={0,};
			ret = tcradio_debugHdr(0, numdbg, pArg);
		}
	}
	else if(SCMP(p0, "hdbbreset", MAX_SCMP_LEN)==0) {
		if(argc == 1) {
			uint32 numdbg = 2, pArg[2]={0,};
			ret = tcradio_debugHdr(0, numdbg, pArg);
		}
	}
	else if(SCMP(p0, "hdbufdump", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			int32 fOnOff = (int32)atoi(p1);
			uint32 numdbg = 3, pArg[2]={0,};
			if(fOnOff) {
				pArg[0] = 1;
			}
			else {
				pArg[0] = 0;
			}
			ret = tcradio_debugHdr(0, numdbg, pArg);
		}
	}
	else if(SCMP(p0, "hdadump", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			int32 fOnOff = (int32)atoi(p1);
			uint32 numdbg = 4, pArg[2]={0,};
			if(fOnOff) {
				pArg[0] = 1;
			}
			else {
				pArg[0] = 0;
			}
			ret = tcradio_debugHdr(0, numdbg, pArg);
		}
	}
#endif
	else if(SCMP(p0, "dablist", MAX_SCMP_LEN)==0) {
		if(argc == 1 && ret == eRET_OK) {
			uint32 receivedDabList[RHAL_DAB_MAX_NUM_OF_FREQ] = {0,};
			uint8 numDabList=0, i;
			ret = tcradio_getDabFreqList(receivedDabList, &numDabList, eRADIO_ID_PRIMARY);
			RAPP_DBG("The number of DAB Frequencies: %d \n", numDabList);
			for(i=0; i<numDabList; i++) {
				RAPP_DBG("DAB Freq[%d]: %d Hz\n", i, receivedDabList[i]);
			}
		}
	}
	else if(SCMP(p0, "iqtest", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			if(SCMP(p1, "off", MAX_SCMP_LEN)==0) {
				ret = tcradio_setIQTestPattern(OFF, 0);
				if(ret == eRET_OK) {
					RAPP_DBG("IQ Test Pattern Off!!\n");
				}
			}
			else {
				err = -1;
			}
		}
		else if(argc == 3) {
			if(SCMP(p1, "on", MAX_SCMP_LEN)==0) {
				int32 sel = (int32)atoi(p2);
				ret = tcradio_setIQTestPattern(ON, (uint32)sel);
				if(ret == eRET_OK) {
					RAPP_DBG("IQ Test Pattern On!!\n");
				}
			}
			else {
				err = -1;
			}
		}
		else {
			err = -1;
		}
	}
#ifdef USE_HDRADIO
	else if(SCMP(p0, "hdlot", MAX_SCMP_LEN) == 0) {
		if(argc == 3){
			if (SCMP(p1, "main", MAX_SCMP_LEN) == 0){
				if(SCMP(p2, "on", MAX_SCMP_LEN)==0) {
					ret = tcradioservice_enableHdrLot(eTC_HDR_ID_MAIN, 0xff, 1);
					if(ret == (HDRET)eRET_OK) {
						RAPP_DBG("Large Object Transfer (Main) Service On !!\n");
					}
				}
				else if(SCMP(p2, "off", MAX_SCMP_LEN)==0){
					ret = tcradioservice_enableHdrLot(eTC_HDR_ID_MAIN, 0xff, 0);
					if(ret == (HDRET)eRET_OK) {
						RAPP_DBG("Large Object Transfer (Main) Service Off !!\n");
					}
				} else {
					err = -1;
				}
			} else if (SCMP(p1, "bs", MAX_SCMP_LEN) == 0) {
				if(SCMP(p2, "on", MAX_SCMP_LEN)==0) {
					ret = tcradioservice_enableHdrLot(eTC_HDR_ID_BS, 0xff, 1);
					if(ret == (HDRET)eRET_OK) {
						RAPP_DBG("Large Object Transfer (Background) Service On !!\n");
					}
				}
				else if(SCMP(p2, "off", MAX_SCMP_LEN)==0){
					ret = tcradioservice_enableHdrLot(eTC_HDR_ID_BS, 0xff, 0);
					if(ret == (HDRET)eRET_OK) {
						RAPP_DBG("Large Object Transfer (Background) Service Off !!\n");
					}
				} else {
					err = -1;
				}
			} else {
				err = -1;
			}
		} else {
			err = -1;
		}

		if(err == -1){
			RAPP_DBG("lot [main/bs] [on/off]\n");
		}
	}
	else if (SCMP(p0, "hdea", MAX_SCMP_LEN) == 0) {
		if(argc == 3){
			if(SCMP(p1, "main", MAX_SCMP_LEN) == 0){
				if (SCMP(p2, "on", MAX_SCMP_LEN) == 0){
					ret = tcradioservice_enableHdrAlert(eTC_HDR_ID_MAIN, 1);
					if(ret == eRET_OK){
						RAPP_DBG("Emergency Alert Main Service On !!\n");
					}
				} else if (SCMP(p2, "off", MAX_SCMP_LEN) == 0) {
					ret = tcradioservice_enableHdrAlert(eTC_HDR_ID_MAIN, 0);
					if(ret == eRET_OK){
						RAPP_DBG("Emergency Alert Main Service Off !!\n");
					}
				} else {
					err = -1;
				}

			}else if(SCMP(p1, "bs", MAX_SCMP_LEN) == 0){
				if (SCMP(p2, "on", MAX_SCMP_LEN) == 0){
					ret = tcradioservice_enableHdrAlert(eTC_HDR_ID_BS, 1);
					if(ret == eRET_OK){
						RAPP_DBG("Emergency Alert Background Service On !!\n");
					}
				} else if (SCMP(p2, "off", MAX_SCMP_LEN) == 0) {
					ret = tcradioservice_enableHdrAlert(eTC_HDR_ID_BS, 0);
					if(ret == eRET_OK){
						RAPP_DBG("Emergency Alert Background Service Off !!\n");
					}
				} else {
					err = -1;
				}
			}else{
				err = -1;
			}
		}else{
			err = -1;
		}

		if(err == -1){
			RAPP_DBG("ea [main/bs] [on/off]\n");
		}
	}
#endif
	else if(SCMP(p0, "exit", MAX_SCMP_LEN)==0) {
		tcradio_close();
		exit = -1;
		RAPP_DBG("Exiting Radio Service Application.\n");
	}
    else {
		err = -1;
	}

	if(err == -1) {
		RAPP_DBG(">>>>>>> Wrong Command... Please try again\n");
	}
	else if(err == -2) {
		RAPP_DBG(">>>>>>> Wrong Service Number... Please try again\n");
	}
	else if(err == 0) {
		if(ret != eRET_OK && exit == 0) {
			RAPP_DBG("Failed to set command!!! [ret = %d]\n", ret);
		}
	}
	else {
		;
	}

	return exit;
}

void tcradiocui_help(void)
{
	printf("\n");
	printf("========================================================================\n");
	printf("===============    Telechips Radio Service CUI App     =================\n");
	printf("========================================================================\n");
	printf("h               : Help Menu \n");
	printf("init            : Init Radio Service\n");
	printf("deinit          : Deinit Radio Service\n");
	printf("open p1 p2 p3 p4 p5 : Open Radio Service\n");
	printf("                      p1 : Area (aisa, eu, na)\n");
	printf("                      p2 : Number of tuner (1 ~ 3)\n");
	printf("                      p3 : Phase Diversity (0:Disable, 1:Enable)\n");
	printf("                      p4 : I/Q Output (0:Disable, 1:Enable)\n");
	printf("                      p5 : Audio Samplerate (44100 or 48000)\n");
	printf("                           ex) open                : Open radio service with default config\n");
	printf("                               open na 2 0 1 44100 : Open radio service with user input\n");
	printf("close           : Close Radio Service\n");
	printf("fm p1 p2		: Tune FM Freqneucy (freq unit : KHz)\n");
	printf("                  p1 : Tuner Demod number (0 - 3) \n");
	printf("                  p2 : Frequency \n");
	printf("                       ex)fm (=fm 0), fm 1, fm 2 99100 \n");
	printf("am p1 p2        : Tune AM Frequency (freq unit : KHz)\n");
	printf("                  p1 : Tuner Demod number (0 - 3) \n");
	printf("                  p2 : Frequency \n");
	printf("                       ex)am (=am 0), am 1, am 2 530 \n");
	printf("u               : Frequency Up (Primary Demod)\n");
	printf("d               : Frequency Down (Primary Demod)\n");
	printf("up              : Auto Seek Up (Primary Demod)\n");
	printf("dn              : Auto Seek Down (Primary Demod)\n");
	printf("list            : Scan Station List (Primary Demod)\n");
	printf("s               : Seek Stop \n");
	printf("bf p1 p2 p3 p4  : Set/Get Band Frequency Configuration \n");
	printf("                  p1 : Band (fm or am or dab)\n");
	printf("                  p2 : Start Frequency (freq unit : KHz) \n");
	printf("                  p3 : End Frequency (freq unit : KHz) \n");
	printf("                  p4 : Frequency Step (freq unit : KHz) \n");
	printf("                       ex) bf fm                  : Get FM Configuration \n");
	printf("                           bf fm 87500 108000 100 : Set FM Configuration \n");
	printf("                           bf am 530 1710 10      : Set AM Configuration \n");
	printf("t p1            : Get Current Tune Information \n");
	printf("                  p1 : Tuner Demod number (0 - 3) \n");
	printf("q p1            : Get Current Tune Quality Data\n");
	printf("                  p1 : Tuner Demod number (0 - 3) \n");
	printf("v               : Middleware Version \n");
	printf("mute on         : Mute On \n");
	printf("mute off        : Mute Off \n");
	printf("audio on        : Audio Device On \n");
	printf("audio off       : Audio Device Off \n");
	printf("rds on          : RDS On \n");
	printf("rds off         : RDS Off \n");
	printf("rds pi          : RDS Program ID \n");
	printf("rds pty         : RDS Program Type \n");
	printf("rds psn         : RDS Program Station Name \n");
	printf("rds piscan p1 p2 p3 : RDS PI Scan \n");
	printf("                      p1-p3 : PI List Input (unit : hex) \n");
	printf("                              ex)rds piscan e001 e002 e003 ,rds piscan e001 0 0 \n");
	printf("exit            : Exit Radio Service \n");
	printf("dump p1 p2 p3 p4 p5 : Dump I/Q for test (dump directory: /tmp/I*_data.bin or /tmp/Q*_data.bin or /tmp/IQ*_data.txt)\n");
	printf("                  p1 : Bit mode (16, 20, 24, 30, 32, 40, 48, 60, 64, 80 bit)\n");
	printf("                  p2 : I/Q buffer size (Unit : Kbyte),(1024Kbyte <= size <= 2048Kbyte)\n");
	printf("                  p3 : Output type(0:csv file, 1:bin file)\n");
	printf("                  p4 : I/Q total dump size(Unit : Mbyte)\n");
	printf("                  p5 : I/Q read size(Unit : byte), (0 < size <= 2048Kbyte, default size: I/Q buffer size / 8)\n");
	printf("                       ex) dump 16 : Dump by default(=dump 16 1024 0)\n");
	printf("                       ex) dump 16 1024 1 : Dump to bin file with 16bit 1024Kbyte setting on device 0\n");
	printf("dab p1 p2		: Tune DAB Freqneucy \n");
	printf("                  p1 : Tuner Demod number (0 - 1) \n");
	printf("                  p2 : Index of DAB Frequency List\n");
	printf("dablist         : Det DAB frequency list \n");
	printf("iqtest on p1    : IQ Test Pattern On \n");
	printf("                  p1 : Select IQ (0 - 1) \n");
	printf("iqtest off      : IQ Test Pattern Off \n");
#ifdef USE_HDRADIO
	printf("hdlot p1 p2       : LOT (Large Object Transfer)\n");
	printf("                  p1 : main/bs\n");
	printf("                  p2 : on/off \n");
	printf("hdea p1 p2        : EA (Emergency Alert) \n");
	printf("                  p1 : main/bs \n");
	printf("                  p2 : on/off \n");
	printf("hdaudio p1      : Set HD Radio Main Audio Mode\n");
	printf("                  p1 : main audio mode (0:blend 1:analog 2:digital 3:split)\n");
	printf("                       ex) hdaudio 1 : set analog audio mode\n");
	printf("hdsts p1        : Get HD Radio all status\n");
	printf("                  p1 : HD Radio ID (0:main 2:bs)\n");
	printf("                       ex) hdsts 0 : get main hd radio all status\n");
	printf("hdsig p1        : Get HD Radio signal status\n");
	printf("                  p1 : HD Radio ID (0:main 2:bs)\n");
	printf("                       ex) hdsig 0 : main hd radio signal status\n");
	printf("hdprog p1       : Set HD Radio program\n");
	printf("                  p1 : HD Radio program number (0(HD1) - 7(HD8)\n");
	printf("                       ex) hdprog 3 : HD4 program\n");
#endif
	printf("\n");
	printf("Enter Your Command!!! \n");
	printf("========================================================================\n");
	printf(">");
}

