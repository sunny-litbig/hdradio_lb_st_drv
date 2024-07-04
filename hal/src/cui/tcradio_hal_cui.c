/*******************************************************************************

*   FileName : tcradio_hal_cui.cpp

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
*        Include 			   					   *
****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/prctl.h>

#include "tcradio_types.h"
#include "tcradio_hal.h"

/***************************************************
*        Global variable definitions               *
****************************************************/

/***************************************************
*        Imported variable declarations            *
****************************************************/

/***************************************************
*        Imported function declartions             *
****************************************************/

/***************************************************
*        Local preprocessor                        *
****************************************************/
#define MAX_SCMP_LEN			(10)
#define SCMP(x, y, len)			strncmp((char*)(x), (char*)(y), (size_t)len)

#define	RHAL_AUDIO_SAMPLERATE	44100		// or 48000

#define	RHAL_FM_INIT_FREQ		87500		// khz
#define	RHAL_FM_START_FREQ		64000		// khz
#define	RHAL_FM_END_FREQ		108000		// khz
#define	RHAL_MW_INIT_FREQ		530			// khz
#define	RHAL_MW_START_FREQ		520			// khz
#define	RHAL_MW_END_FREQ		1710		// khz
#define	RHAL_DAB_INIT_INDEX		0
#define	RHAL_DAB_START_INDEX	0
#define	RHAL_DAB_END_INDEX		40

#define	RHAL_DAB_MAX_NUM_OF_FREQ	64			// Base on silab tuner

/***************************************************
*        Local type definitions                    *
****************************************************/

/***************************************************
*        Local constant definitions                *
****************************************************/
static stTUNER_DRV_CONFIG_t tunerConf;
static eTUNER_DRV_MOD_MODE_t current_hal_band=eTUNER_DRV_FM_MODE;
static uint32 current_hal_frequency=0;

/***************************************************
*        Local function prototypes                 *
****************************************************/
int32 tcradiohalcui_operation(char *argv[], int argc);

/***************************************************
*        function definition			           *
****************************************************/
static void tcradiohalcui_exeHelpMsg(char *name)
{
    printf("------------------------------------------------------------\n"
           "|  %s [USAGE]\n"
           "|   p1 : radio standard\n"
           "|  example>\n"
           "|     $ %s fm            :(FM Radio)\n"
           "|     $ %s mw            :(AM(MW) Radio)\n"
           "|     $ %s hdr           :(HD Radio)\n"
           "|     $ %s drm30         :(DRM30 Radio)\n"
           "|     $ %s drmp          :(DRM+ Radio)\n"
           "|     $ %s dab           :(DAB Radio)\n"
           "------------------------------------------------------------\n"
            , name, name, name, name, name, name, name);
}

static void tcradiohalcui_cmdHelpMsg(void)
{
	printf("\n");
	printf("========================================================================\n");
	printf("====================    Telechips Radio HAL CUI     ====================\n");
	printf("========================================================================\n");
	printf("h               : Help Radio HAL CUI\n");
	printf("open p1 p2 p3 p4 p5 p6 : Open Radio HAL\n");
	printf("                      p1 : Area (aisa, eu, na)\n");
	printf("                      p2 : Number of tuner (1 ~ 3)\n");
	printf("                      p3 : Phase Diversity (0:Disable, 1:Enable)\n");
	printf("                      p4 : I/Q Output (0:Disable, 1:Enable)\n");
	printf("                      p5 : Audio Samplerate (44100 or 48000)\n");
	printf("                      p6 : Radio Standard\n");
	printf("                           0: AM/FM, 1: HDR, 2: DRM30, 3: DRM+, 4: DAB\n");
	printf("                           ex) open                  : Open radio HAL with default config\n");
	printf("                               open na 2 0 1 44100 1 : Open radio HAL with user config(HDR)\n");
	printf("close           : Close Radio HAL\n");
	printf("fm p1 p2		: Tune FM Freqneucy (freq unit : KHz)\n");
	printf("                  p1 : Tuner Demod number (0 - 3) \n");
	printf("                  p2 : Frequency \n");
	printf("                       ex)fm (=fm 0), fm 1, fm 2 97900 \n");
	printf("am p1 p2        : Tune AM Frequency (freq unit : KHz)\n");
	printf("                  p1 : Tuner Demod number (0 - 3) \n");
	printf("                  p2 : Frequency \n");
	printf("                       ex)am (=am 0), am 1, am 2 530 \n");
	printf("dab p1 p2		: Tune DAB Freqneucy \n");
	printf("                  p1 : Tuner Demod number (0 - 1) \n");
	printf("                  p2 : Index of DAB Frequency List\n");
	printf("dablist         : Det DAB frequency list \n");
	printf("t p1            : Get Current Tune Information \n");
	printf("                  p1 : Tuner Demod number (0 - 3) \n");
	printf("q p1            : Get Current Tune Quality Data\n");
	printf("                  p1 : Tuner Demod number (0 - 3) \n");
	printf("iqtest on p1    : IQ Test Pattern On \n");
	printf("                  p1 : Select IQ (0 - 1) \n");
	printf("iqtest off      : IQ Test Pattern Off \n");
	printf("exit            : Exit Radio HAL \n");
	printf("\n");
	printf("Enter Your Command!!! \n");
	printf("========================================================================\n");
	printf(">");
}

static RET tcradiohalcui_getTunerConfFromArg(int cnt, char **sz, stTUNER_DRV_CONFIG_t *conf)
{
	RET ret = eRET_OK;

	conf->numTuners = eTUNER_DRV_CONF_TYPE_DUAL;		// default: dual tuner
	conf->fPhaseDiversity = 0;							// disable
	conf->fIqOut = 1;									// enable
	conf->audioSamplerate = RHAL_AUDIO_SAMPLERATE;		// 44.1Khz
	conf->fExtAppCtrl = 0;								// disable (not used)

	if(cnt == 2) {
	    if (*(sz+1)) {
			if(SCMP(*(sz+1), "fm", MAX_SCMP_LEN)==0) {
				conf->area = eTUNER_DRV_CONF_AREA_ASIA;
				conf->initMode = eTUNER_DRV_FM_MODE;
				conf->initFreq = RHAL_FM_INIT_FREQ;
				conf->sdr = eTUNER_SDR_NONE;
				RHAL_DBG("Selected FM Radio.\n");
	 		}
			else if(SCMP(*(sz+1), "mw", MAX_SCMP_LEN)==0) {
				conf->area = eTUNER_DRV_CONF_AREA_ASIA;
				conf->initMode = eTUNER_DRV_AM_MODE;
				conf->initFreq = RHAL_MW_INIT_FREQ;
				conf->sdr = eTUNER_SDR_NONE;
				RHAL_DBG("Selected AM(MW) Radio.\n");
	 		}
			else if(SCMP(*(sz+1), "hdr", MAX_SCMP_LEN)==0) {
				conf->area = eTUNER_DRV_CONF_AREA_NA;
				conf->initMode = eTUNER_DRV_FM_MODE;
				conf->initFreq = RHAL_FM_INIT_FREQ;
				conf->sdr = eTUNER_SDR_HD;
				RHAL_DBG("Selected HD Radio.\n");
	 		}
			else if(SCMP(*(sz+1), "drm30", MAX_SCMP_LEN)==0) {
				conf->area = eTUNER_DRV_CONF_AREA_ASIA;
				conf->initMode = eTUNER_DRV_AM_MODE;
				conf->initFreq = RHAL_MW_INIT_FREQ;
				conf->sdr = eTUNER_SDR_DRM30;
				RHAL_DBG("Selected DRM30 Radio.\n");
	 		}
			else if(SCMP(*(sz+1), "drmp", MAX_SCMP_LEN)==0) {
				conf->area = eTUNER_DRV_CONF_AREA_ASIA;
				conf->initMode = eTUNER_DRV_FM_MODE;
				conf->initFreq = RHAL_FM_INIT_FREQ;
				conf->sdr = eTUNER_SDR_DRMP;
				RHAL_DBG("Selected DRM+ Radio.\n");
	 		}
			else if(SCMP(*(sz+1), "dab", MAX_SCMP_LEN)==0) {
				conf->area = eTUNER_DRV_CONF_AREA_EU;
				conf->initMode = eTUNER_DRV_DAB_MODE;
				conf->initFreq = RHAL_DAB_INIT_INDEX;
				conf->sdr = eTUNER_SDR_DAB;
				RHAL_DBG("Selected DAB Radio.\n");
	 		}
			else {
				ret = eRET_NG_INVALID_PARAM;
				RHAL_ERR("Invalid Argument: optarg[%s]\n", *(sz+1));
	 		}
	    }
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
		RHAL_ERR("Invalid Argument.\n");
	}
    return ret;
}

int32 main(int argc, char* argv[])
{
	RET ret;
	int i, j;
	char cmd[10][11] = {0,};
	char str[100] = {0,};
	char *targv[8];

	if(tcradiohalcui_getTunerConfFromArg(argc, argv, &tunerConf) < 0) {
		tcradiohalcui_exeHelpMsg(argv[0]);
		return -1;
	}

	ret = tcradiohal_init();
	if(ret == eRET_OK) {
		ret = tcradiohal_open(tunerConf);
		if(ret != eRET_OK) {
			RHAL_DBG("Failed to open radio HAL!!!\n");
		}
		else {
			current_hal_band = tunerConf.initMode;
			current_hal_frequency = tunerConf.initFreq;
			if(tunerConf.initMode == eTUNER_DRV_DAB_MODE) {
				RHAL_DBG("Radio HAL is opened. sdr[%d], mode[%d], index[%d]\n", tunerConf.sdr, tunerConf.initMode, tunerConf.initFreq);
			}
			else {
				RHAL_DBG("Radio HAL is opened. sdr[%d], mode[%d], freq[%d]\n", tunerConf.sdr, tunerConf.initMode, tunerConf.initFreq);
			}
		}
	}

	do
	{
		memset(cmd, 0, sizeof(cmd));
		fgets(str, sizeof(str), stdin);
		sscanf(str, "%10s %10s %10s %10s %10s %10s %10s %10s", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
		for(i=0;i<8;i++) {
			targv[i] = cmd[i];
			if(cmd[i][0] == 0) {
				break;
			}
		}

		if(tcradiohalcui_operation(targv, i) < 0) {
			break;
		}
		usleep(20*1000);
	}while(1);

	tcradiohal_deinit();

	usleep(100*1000);
	return 0;
}

int32 tcradiohalcui_operation(char *argv[], int argc)
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
		tcradiohalcui_cmdHelpMsg();
	}
	else if(SCMP(p0, "open", MAX_SCMP_LEN)==0) {
		if(argc > 1) {
			if(SCMP(p1, "asia", MAX_SCMP_LEN)==0) {
				tunerConf.area = eTUNER_DRV_CONF_AREA_ASIA;
			}
			else if(SCMP(p1, "na", MAX_SCMP_LEN)==0) {
				tunerConf.area = eTUNER_DRV_CONF_AREA_NA;
			}
			else {
				if(SCMP(p1, "eu", MAX_SCMP_LEN)!=0) {
					RHAL_DBG("The input is wrong and the area is set to EU!!\n");
				}
				tunerConf.area = eTUNER_DRV_CONF_AREA_EU;
			}
			if(argc > 2) {
				uint32 ntuners = (uint32)atoi(p2);
				tunerConf.numTuners = ntuners;
				if(argc > 3) {
					uint32 pd = (uint32)atoi(p3);
					if(pd) {
						tunerConf.fPhaseDiversity = ON;
					}
					else {
						tunerConf.fPhaseDiversity = OFF;
					}
					if(argc > 4) {
						uint32 iq = (uint32)atoi(p4);
						if(iq) {
							tunerConf.fIqOut = YES;
						}
						else {
							tunerConf.fIqOut = NO;
						}
						if(argc > 5) {
							uint32 samplerate = (uint32)atoi(p5);
							if(samplerate == 44100 || samplerate == 48000) {
								tunerConf.audioSamplerate = samplerate;
							}
							else {
								RHAL_DBG("The samplerate input is wrong and it is set to 44.1KHz!!\n");
								tunerConf.audioSamplerate = 44100;
							}
							if(argc > 6) {
								uint32 fsdr = (uint32)atoi(p6);
								if(fsdr == eTUNER_SDR_HD) {
									tunerConf.sdr = eTUNER_SDR_HD;
									tunerConf.initMode = eTUNER_DRV_FM_MODE;
									tunerConf.initFreq = RHAL_FM_INIT_FREQ;
								}
								else if(fsdr == eTUNER_SDR_DRM30) {
									tunerConf.sdr = eTUNER_SDR_DRM30;
									tunerConf.initMode = eTUNER_DRV_AM_MODE;
									tunerConf.initFreq = RHAL_MW_INIT_FREQ;
								}
								else if(fsdr == eTUNER_SDR_DRMP) {
									tunerConf.sdr = eTUNER_SDR_DRMP;
									tunerConf.initMode = eTUNER_DRV_FM_MODE;
									tunerConf.initFreq = RHAL_FM_INIT_FREQ;
								}
								else if(fsdr == eTUNER_SDR_DAB) {
									tunerConf.sdr = eTUNER_SDR_DAB;
									tunerConf.initMode = eTUNER_DRV_DAB_MODE;
									tunerConf.initFreq = RHAL_DAB_INIT_INDEX;
								}
								else {
									tunerConf.sdr = eTUNER_SDR_NONE;
								}
							}
						}
					}
				}
			}
		}
		ret = tcradiohal_open(tunerConf);
		if(ret != eRET_OK) {
			RHAL_DBG("Failed to open radio HAL!!!\n");
			err = -1;
		}
		else {
			current_hal_band = tunerConf.initMode;
			current_hal_frequency = tunerConf.initFreq;
			if(tunerConf.initMode == eTUNER_DRV_DAB_MODE) {
				RHAL_DBG("Radio HAL is opened. sdr[%d], mode[%d], index[%d]\n", tunerConf.sdr, tunerConf.initMode, tunerConf.initFreq);
			}
			else {
				RHAL_DBG("Radio HAL is opened. sdr[%d], mode[%d], freq[%d]\n", tunerConf.sdr, tunerConf.initMode, tunerConf.initFreq);
			}
		}
	}
	else if(SCMP(p0, "close", MAX_SCMP_LEN)==0) {
		ret = tcradiohal_close();
		if(ret != eRET_OK) {
			RHAL_DBG("Failed to close radio HAL!!!\n");
			err = -1;
		}
		else {
			RHAL_DBG("Radio HAL is closed.\n");
		}
	}
	else if(SCMP(p0, "fm", MAX_SCMP_LEN)==0) {
		uint32 freq = 0;
		uint32 ntuner = eTUNER_DRV_ID_PRIMARY;

		if(argc > 2) {
			ntuner = (uint32)atoi(p1);
			freq = (uint32)atoi(p2);
		}
		else {
			err = -1;
		}

		if(err >= 0) {
			// If the input frequency is out of range, an error occurs and the tuner does not respond for a few seconds.
			ret = tcradiohal_setTune(eTUNER_DRV_FM_MODE, freq, eTUNER_DRV_TUNE_NORMAL, ntuner);
			if(ret == eRET_OK) {
				current_hal_band = eTUNER_DRV_FM_MODE;
				current_hal_frequency = freq;
				RHAL_DBG("tcradiohal_setTune: FM %dkHz ntuner[%d]\n", freq, ntuner);
			}
		}
	}
	else if(SCMP(p0, "am", MAX_SCMP_LEN)==0) {
		uint32 freq = 0;
		uint32 ntuner = eTUNER_DRV_ID_PRIMARY;

		if(argc > 2) {
			ntuner = (uint32)atoi(p1);
			freq = (uint32)atoi(p2);
		}
		else {
			err = -1;
		}

		if(err >= 0) {
			// If the input frequency is out of range, an error occurs and the tuner does not respond for a few seconds.
			ret = tcradiohal_setTune(eTUNER_DRV_AM_MODE, freq, eTUNER_DRV_TUNE_NORMAL, ntuner);
			if(ret == eRET_OK) {
				current_hal_band = eTUNER_DRV_AM_MODE;
				current_hal_frequency = freq;
				RHAL_DBG("tcradiohal_setTune: AM %dkHz ntuner[%d]\n", freq, ntuner);
			}
		}
	}
	else if(SCMP(p0, "dab", MAX_SCMP_LEN)==0) {
		uint32 index = 0;
		uint32 ntuner = eTUNER_DRV_ID_PRIMARY;

		if(tunerConf.sdr == eTUNER_SDR_DAB) {
			if(argc > 2) {
				ntuner = (uint32)atoi(p1);
				index = (uint32)atoi(p2);
				if(ntuner > eTUNER_DRV_ID_SECONDARY) {
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

		if(err >= 0) {
			// If the input frequency is out of range, an error occurs and the tuner does not respond for a few seconds.
			ret = tcradiohal_setTune(eTUNER_DRV_DAB_MODE, index, eTUNER_DRV_TUNE_NORMAL, ntuner);
			if(ret == eRET_OK) {
				current_hal_band = eTUNER_DRV_DAB_MODE;
				current_hal_frequency = index;
				RHAL_DBG("tcradiohal_setTune: DAB index[%d] ntuner[%d]\n", index, ntuner);
			}
		}
	}
	else if(SCMP(p0, "t", MAX_SCMP_LEN)==0) {
		uint32 ntuner = eTUNER_DRV_ID_PRIMARY;
		uint32 mod_mode = (uint32)current_hal_band;
		uint32 tfreq;

		if(argc > 1) {
			ntuner = (uint32)atoi(p1);
		}

		ret = tcradiohal_getTune(&mod_mode, &tfreq, ntuner);

		if(ret == eRET_OK) {
			if(current_hal_band == eTUNER_DRV_DAB_MODE) {
				RHAL_DBG("tcradiohal_getTune: mode[%d] index[%d] ntuner[%d]\n", mod_mode, tfreq, ntuner);
			}
			else {
				RHAL_DBG("tcradiohal_getTune: mode[%d] freq[%d] ntuner[%d]\n", mod_mode, tfreq, ntuner);
			}
		}
	}
	else if(SCMP(p0, "q", MAX_SCMP_LEN)==0) {
		uint32 ntuner = eTUNER_DRV_ID_PRIMARY;
		eTUNER_DRV_MOD_MODE_t mod_mode = current_hal_band;
		stTUNER_QUALITY_t qdata;

		if(argc > 1) {
			ntuner = (uint32)atoi(p1);
		}

		ret = tcradiohal_getQuality((int32)mod_mode, &qdata, ntuner);

		if(ret == eRET_OK) {
			RHAL_DBG("==== Tuner[%d] Mode[%d] Quality ====\n", ntuner, mod_mode);
			if(qdata.type == eTUNER_IC_S0)
			{
				if(mod_mode == eTUNER_DRV_FM_MODE) {	// FM
					RHAL_DBG("RSSI = %d\n", qdata.qual.fm.Qvalue[0]);
					RHAL_DBG("SNR = %d\n", qdata.qual.fm.Qvalue[1]);
					RHAL_DBG("Deviation = %d\n", qdata.qual.fm.Qvalue[2]);
					RHAL_DBG("Offset = %d\n", qdata.qual.fm.Qvalue[3]);
					RHAL_DBG("Pilot = %d\n", qdata.qual.fm.Qvalue[4]);
					RHAL_DBG("MultiPath = %d\n", qdata.qual.fm.Qvalue[5]);
					RHAL_DBG("USN = %d\n", qdata.qual.fm.Qvalue[6]);
				}
				else if(mod_mode == eTUNER_DRV_AM_MODE) {	// AM
					RHAL_DBG("RSSI = %d\n", qdata.qual.am.Qvalue[0]);
					RHAL_DBG("SNR = %d\n", qdata.qual.am.Qvalue[1]);
					RHAL_DBG("Modulation = %d\n", qdata.qual.am.Qvalue[2]);
					RHAL_DBG("Offset = %d\n", qdata.qual.am.Qvalue[3]);
				}
				else if(mod_mode == eTUNER_DRV_DAB_MODE) {	// DAB
					RHAL_DBG("RSSI = %d\n", qdata.qual.dab.Qvalue[0]);
					RHAL_DBG("SQI = %d\n", qdata.qual.dab.Qvalue[1]);
					RHAL_DBG("DETECT = %d\n", qdata.qual.dab.Qvalue[2]);
					RHAL_DBG("RSSIADJ = %d\n", qdata.qual.dab.Qvalue[3]);
					RHAL_DBG("DAGC = %d\n", qdata.qual.dab.Qvalue[4]);
				}
				else {
					ret = eRET_NG_NOT_SUPPORT;
				}
			}
			else if(qdata.type == eTUNER_IC_X0)
			{
				if(mod_mode == eTUNER_DRV_FM_MODE) {	// FM
					RHAL_DBG("Status = %xh\n", qdata.qual.fm.Qvalue[0]);
					RHAL_DBG("RSSI = %d\n", qdata.qual.fm.Qvalue[1]);
					RHAL_DBG("USN = %d\n", qdata.qual.fm.Qvalue[2]);
					RHAL_DBG("MultiPath = %d\n", qdata.qual.fm.Qvalue[3]);
					RHAL_DBG("Offset = %d\n", qdata.qual.fm.Qvalue[4]);
					RHAL_DBG("Bandwidth = %d\n", qdata.qual.fm.Qvalue[5]);
					RHAL_DBG("Modulation = %d\n", qdata.qual.fm.Qvalue[6]);
				}
				else if(mod_mode == eTUNER_DRV_AM_MODE) {	// AM
					RHAL_DBG("Status = %xh\n", qdata.qual.am.Qvalue[0]);
					RHAL_DBG("RSSI = %d\n", qdata.qual.am.Qvalue[1]);
					RHAL_DBG("High Frequency Noise Detector = %d\n", qdata.qual.am.Qvalue[2]);
					RHAL_DBG("Co-Channel Detector = %d\n", qdata.qual.am.Qvalue[3]);
					RHAL_DBG("Offset = %d\n", qdata.qual.am.Qvalue[4]);
					RHAL_DBG("Bandwidth = %d\n", qdata.qual.am.Qvalue[5]);
					RHAL_DBG("Modulation = %d\n", qdata.qual.am.Qvalue[6]);
				}
				else {
					ret = eRET_NG_NOT_SUPPORT;
				}
			}
			else {
				if(mod_mode == eTUNER_DRV_FM_MODE) {	// FM
					RHAL_DBG("RSSI = %d\n", qdata.qual.fm.Qvalue[0]);
					RHAL_DBG("Modulation = %d\n", qdata.qual.fm.Qvalue[1]);
					RHAL_DBG("Offset = %d\n", qdata.qual.fm.Qvalue[2]);
					RHAL_DBG("HFN = %d\n", qdata.qual.fm.Qvalue[3]);
					RHAL_DBG("MultiPath = %d\n", qdata.qual.fm.Qvalue[4]);
					RHAL_DBG("Pilot = %d\n", qdata.qual.fm.Qvalue[5]);
				}
				else if(mod_mode == eTUNER_DRV_AM_MODE) {	// AM
					RHAL_DBG("RSSI = %d\n", qdata.qual.am.Qvalue[0]);
					RHAL_DBG("Modulation = %d\n", qdata.qual.am.Qvalue[1]);
					RHAL_DBG("Offset = %d\n", qdata.qual.am.Qvalue[2]);
				}
				else {
					ret = eRET_NG_NOT_SUPPORT;
				}
			}
		}
	}
	else if(SCMP(p0, "dablist", MAX_SCMP_LEN)==0) {
		if(argc == 1 && ret == eRET_OK) {
			uint32 receivedDabList[RHAL_DAB_MAX_NUM_OF_FREQ] = {0,};
			uint8 numDabList=0, i;
			ret = tcradiohal_getDabFreqList(receivedDabList, &numDabList, eTUNER_DRV_ID_PRIMARY);
			RHAL_DBG("The number of DAB Frequencies: %d \n", numDabList);
			for(i=0; i<numDabList; i++) {
				RHAL_DBG("DAB Freq[%d]: %d Hz\n", i, receivedDabList[i]);
			}
		}
	}
	else if(SCMP(p0, "iqtest", MAX_SCMP_LEN)==0) {
		if(argc == 2) {
			if(SCMP(p1, "off", MAX_SCMP_LEN)==0) {
				ret = tcradiohal_setIQTestPattern(OFF, 0);
				if(ret == eRET_OK) {
					RHAL_DBG("IQ Test Pattern Off!!\n");
				}
			}
			else {
				err = -1;
			}
		}
		else if(argc == 3) {
			if(SCMP(p1, "on", MAX_SCMP_LEN)==0) {
				int32 sel = (int32)atoi(p2);
				ret = tcradiohal_setIQTestPattern(ON, (uint32)sel);
				if(ret == eRET_OK) {
					RHAL_DBG("IQ Test Pattern On!!\n");
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
	else if(SCMP(p0, "pp", MAX_SCMP_LEN)==0) {
		if(argc == 5) {
			int32 group = (int32)atoi(p1);
			int32 number = (int32)atoi(p2);
			int32 hdata = (int32)atoi(p3);
			int32 ldata = (int32)atoi(p4);
			ret = tcradiohal_setPropertyOfPrimaryTuner((uint8)group, (uint8)number, (uint8)hdata, (uint8)ldata);
		}
		else {
			err = -1;
		}
	}
	else if(SCMP(p0, "drvtest", MAX_SCMP_LEN)==0) {
		tcradiohal_testReliability();
	}
	else if(SCMP(p0, "exit", MAX_SCMP_LEN)==0) {
		tcradiohal_close();
		exit = -1;
		RHAL_DBG("Exiting Radio HAL.\n");
	}
	else {
		err = -1;
	}

	if(err == -1) {
		RHAL_DBG(">>>>>>> Wrong Command... Please try again\n");
	}
	else if(err == -2) {
		RHAL_DBG(">>>>>>> Wrong Service Number... Please try again\n");
	}
	else if(err == 0) {
		if(ret != eRET_OK && exit == 0) {
			RHAL_DBG("Failed to set command!!! [ret = %d]\n", ret);
		}
	}
	else {
		;
	}

	return exit;
}

