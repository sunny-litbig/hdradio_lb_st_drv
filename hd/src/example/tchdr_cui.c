/*******************************************************************************

*   FileName : tchdr_cui.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio CUI Main functions

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
*        Include                                   *
****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/prctl.h>

#if defined(USE_TELECHIPS_EVB) && defined(BUILD_HDR_EXAMPLE_CUI)
#include "tchdr_api.h"
#include "tchdr_cui_if.h"

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

/***************************************************
*        Local type definitions                    *
****************************************************/

/***************************************************
*        Local constant definitions                *
****************************************************/

/***************************************************
*        Local function prototypes                 *
****************************************************/
static S32 myatoi(const S8* str);
static void tchdradiocui_exeHelpMsg(S8 *name);
static void tchdradiocui_cmdHelpMsg(void);
static HDRET tchdradiocui_cmdProcessor(S8* const argv[], S32 argc);

/***************************************************
*        function definition                       *
****************************************************/
S32 main(S32 argc, S8* const argv[])
{
	HDRET hdret = (S32)eTC_HDR_RET_OK;
	S32 i;
	S8 cmd[10][11] = {0,};
	S8 str[100] = {0,};
	S8 *targv[10];

	if(tchdradiocui_getConfFromArg(argc, argv) < 0) {
		if(argv != NULL) {
			tchdradiocui_exeHelpMsg(argv[0]);
		}
		else {
			HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "Failed to get argument vector\n");
		}
		hdret = (S32)eTC_HDR_RET_NG_INVALID_PARAMETERS;
	}
	else {
		hdret = tchdradiocui_init(0);
		if(hdret == (HDRET)eTC_HDR_RET_OK) {
			hdret = tchdradiocui_open();
			if(hdret != (HDRET)eTC_HDR_RET_OK) {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "Failed to open HD Radio\n");
				hdret = (S32)eTC_HDR_RET_NG_INIT;
			}
		}
		else {
			HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "Failed to initialize HD Radio\n");
			hdret = (S32)eTC_HDR_RET_NG_INIT;
		}
	}

	while(hdret == (HDRET)eTC_HDR_RET_OK)
	{
		(void)memset(cmd, 0, sizeof(cmd));
		(void)fgets(str, (S32)sizeof(str), stdin);
		(void)sscanf(str, "%10s %10s %10s %10s %10s %10s %10s %10s", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
		for(i=0;i<8;i++) {
			targv[i] = cmd[i];
			if(cmd[i][0] == (S8)0) {
				break;
			}
		}

		if(tchdradiocui_cmdProcessor(targv, i) < 0) {
			break;
		}

		(void)usleep(20*1000);
	}

	(void)tchdradiocui_deinit();

	(void)usleep(100*1000);
	return hdret;
}

static S32 myatoi(const S8* str)
{
	S32 sign = 1;
	S32 value = 0;
	S64 sum = 0;

	if (*str == '-') {
		sign = -1;
		str++;
	}

	while (*str != '\0') {
		if((*str >= '0') && (*str <= '9')) {
			sum = ((S64)value * 10) + (*str - '0');
			if(sum < 0x7FFFFFFF) {
				value = (S32)sum;
			}
			else {
				value = 0x7FFFFFFF;
			}
			str++;
		}
		else {
			break;
		}
	}

	if(sign == -1) {
		value = value * sign;
	}

	return value;
}

static void tchdradiocui_exeHelpMsg(S8 *name)
{
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "------------------------------------------------------------\n"
                                 "|  %s [USAGE]\n"
                                 "|   p1 : HD Radio Type\n"
                                 "|  example>\n"
                                 "|     $ %s hd10     :HD1.0 Radio\n"
                                 "|     $ %s hd10mrc  :HD1.0+MRC Radio\n"
                                 "|     $ %s hd15     :HD1.5 Radio\n"
                                 "|     $ %s hd15mrc  :HD1.5+MRC Radio\n"
                                 "------------------------------------------------------------\n"
                                  , name, name, name, name, name);
}

static void tchdradiocui_cmdHelpMsg(void)
{
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "========================================================================\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "====================     Telechips HD Radio CUI     ====================\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "========================================================================\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "h               : Help HD Radio CUI\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "init            : Init HD Radio\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "deinit          : Deinit HD Radio\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "open            : Open HD Radio\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "close           : Close HD Radio\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "fm p1 p2	     : Tune FM Freqneucy (freq unit : KHz)\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                  p1 : HD Radio ID (main, mrc, bs) \n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                  p2 : Frequency (Khz)\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                       ex)fm main 97900 \n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "am p1 p2        : Tune AM Frequency (freq unit : KHz)\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                  p1 : HD Radio ID (main, mrc, bs) \n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                  p2 : Frequency (Khz)\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                       ex)am main 1120 \n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "mute on         : Mute on \n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "mute off        : Mute off \n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "auddev on       : Audio Device On \n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "auddev off      : Audio Device Off \n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "audmode p1      : Set HD Radio Main Audio Mode\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                  p1 : main audio mode (0:blend 1:analog 2:digital 3:split)\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                       ex) audmode 1 : set analog audio output mode\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "allsts p1       : Get HD Radio all status\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                  p1 : HD Radio ID (main, bs)\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                       ex) allsts main : get main hd radio all status\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "sig p1          : Get HD Radio signal status\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                  p1 : HD Radio ID (main, bs)\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                       ex) sig main : main hd radio signal status\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "prog p1         : Set HD Radio program\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                  p1 : HD Radio program number (0(HD1) - 7(HD8)\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                       ex) prog 3 : HD4 program\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "aaa on          : Enable Automatic Audio Alignment\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "aaa off         : Disalbe Automatic Audio Alignment\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "lot p1 p2       : LOT (Large Object Transfer)\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                  p1 : main/bs\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "                  p2 : on/off\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "ea on           : EA (Emergency Alert) On \n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "ea off          : EA (Emergency Alert) Off \n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "exit            : Exit HD Radio CUI\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Enter Your Command! \n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "========================================================================\n");
	HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, ">");
}

static HDRET tchdradiocui_cmdProcessor(S8* const argv[], S32 argc)
{
	const S8 *p0;
	const S8 *p1;
	const S8 *p2;
	const S8 *p3;
	const S8 *p4;
	const S8 *p5;
	const S8 *p6;
	const S8 *p7;
	U32 i;
	S32 err = 0;
	S32 hdret = 0;
	S32 fexit = 0;

	if(argv != NULL)
	{
		p0 = (0 < argc) ? argv[0] : NULL;
		p1 = (1 < argc) ? argv[1] : NULL;
		p2 = (2 < argc) ? argv[2] : NULL;
		p3 = (3 < argc) ? argv[3] : NULL;
		p4 = (4 < argc) ? argv[4] : NULL;
		p5 = (5 < argc) ? argv[5] : NULL;
		p6 = (6 < argc) ? argv[6] : NULL;
		p7 = (7 < argc) ? argv[7] : NULL;

		if(argc > 0)
		{
			if(SCMP(p0, "h", 2)==0) {
				tchdradiocui_cmdHelpMsg();
			}
			else if(SCMP(p0, "init", 5)==0) {
				hdret = tchdradiocui_init(0);
				if(hdret != (HDRET)eTC_HDR_RET_OK) {
					err = -1;
				}
				else {
					HDRCUI_MAIN_LOG(CUI_DBG_LOG, 1U, "Success. HD Radio initialized.\n");
				}
			}
			else if(SCMP(p0, "deinit", 7)==0) {
				hdret = tchdradiocui_deinit();
				if(hdret != (HDRET)eTC_HDR_RET_OK) {
					err = -1;
				}
				else {
					HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Success. HD Radio deinitialized.\n");
				}
			}
			else if(SCMP(p0, "open", 5)==0) {
				hdret = tchdradiocui_open();
				if(hdret != (HDRET)eTC_HDR_RET_OK) {
					err = -1;
				}
				else {
					HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Success. HD Radio opened.\n");
				}
			}
			else if(SCMP(p0, "close", 6)==0) {
				hdret = tchdradiocui_close();
				if(hdret != (HDRET)eTC_HDR_RET_OK) {
					err = -1;
				}
				else {
					HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Success. HD Radio closed.\n");
				}
			}
			else if(SCMP(p0, "fm", 3)==0) {
				if(argc == 3) {
					S32 freq = myatoi(p2);
					if(freq > 0) {
						if(SCMP(p1, "main", 5)==0) {
							hdret = tchdradiocui_setTune(eTC_HDR_FM_BAND, (U32)freq, eTC_HDR_ID_MAIN);
						}
						else if(SCMP(p1, "mrc", 4)==0) {
							hdret = tchdradiocui_setTune(eTC_HDR_FM_BAND, (U32)freq, eTC_HDR_ID_MRC);
						}
						else if(SCMP(p1, "bs", 3)==0) {
							hdret = tchdradiocui_setTune(eTC_HDR_FM_BAND, (U32)freq, eTC_HDR_ID_BS);
						}
						else if(SCMP(p1, "bsmrc", 6)==0) {
							hdret = tchdradiocui_setTune(eTC_HDR_FM_BAND, (U32)freq, eTC_HDR_ID_BS_MRC);
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
			else if(SCMP(p0, "am", 3)==0) {
				if(argc == 3) {
					S32 freq = myatoi(p2);
					if(freq > 0) {
						if(SCMP(p1, "main", 5)==0) {
							hdret = tchdradiocui_setTune(eTC_HDR_AM_BAND, (U32)freq, eTC_HDR_ID_MAIN);
						}
						else if(SCMP(p1, "mrc", 4)==0) {
							hdret = tchdradiocui_setTune(eTC_HDR_AM_BAND, (U32)freq, eTC_HDR_ID_MRC);
						}
						else if(SCMP(p1, "bs", 3)==0) {
							hdret = tchdradiocui_setTune(eTC_HDR_AM_BAND, (U32)freq, eTC_HDR_ID_BS);
						}
						else if(SCMP(p1, "bsmrc", 6)==0) {
							hdret = tchdradiocui_setTune(eTC_HDR_AM_BAND, (U32)freq, eTC_HDR_ID_BS_MRC);
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
			else if(SCMP(p0, "reacq", 6)==0) {
				if(argc == 2) {
					if(SCMP(p1, "main", 5)==0) {
						hdret = tchdradiocui_setReacquire(eTC_HDR_ID_MAIN);
					}
					else if(SCMP(p1, "mrc", 4)==0) {
						hdret = tchdradiocui_setReacquire(eTC_HDR_ID_MRC);
					}
					else if(SCMP(p1, "bs", 3)==0) {
						hdret = tchdradiocui_setReacquire(eTC_HDR_ID_BS);
					}
					else if(SCMP(p1, "bsmrc", 6)==0) {
						hdret = tchdradiocui_setReacquire(eTC_HDR_ID_BS_MRC);
					}
					else {
						err = -1;
					}
				}
				else {
					err = -1;
				}
			}
			else if(SCMP(p0, "mute", 5)==0) {
				if(argc == 2) {
					if(SCMP(p1, "on", 3)==0) {
						hdret = tchdr_setAudioMute(1);
					}
					else if(SCMP(p1, "off", 4)==0) {
						hdret = tchdr_setAudioMute(0);
					}
					else {
						err = -1;
					}
				}
				else {
					err = -1;
				}
			}
			else if(SCMP(p0, "auddev", 7)==0) {
				if(argc == 2) {
					if(SCMP(p1, "on", 3)==0) {
						hdret = tchdradiocui_setAudioDevice(44100, 1);
						if(hdret == (HDRET)eTC_HDR_RET_OK) {
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Success. Audio device opened.\n");
						}
					}
					else if(SCMP(p1, "off", 4)==0) {
						hdret = tchdradiocui_setAudioDevice(44100, 0);
						if(hdret == (HDRET)eTC_HDR_RET_OK) {
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Success. Audio device closed.\n");
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
			else if(SCMP(p0, "audmode", 8)==0) {
				if(argc == 2) {
					S32 mode = myatoi(p1);
					switch(mode) {
						case 0:
							hdret = tchdr_setAudioMode(eTC_HDR_AUDIO_BLEND);
							break;
						case 1:
							hdret = tchdr_setAudioMode(eTC_HDR_AUDIO_ANALOG);
							break;
						case 2:
							hdret = tchdr_setAudioMode(eTC_HDR_AUDIO_DIGITAL);
							break;
						case 3:
							hdret = tchdr_setAudioMode(eTC_HDR_AUDIO_SPLIT);
							break;
						default:
							hdret = (S32)eTC_HDR_RET_NG_INVALID_PARAMETERS;
							break;
					}
				}
			}
			else if(SCMP(p0, "allsts", 7)==0) {
				if(argc == 2) {
					eTC_HDR_ID_t hdrID = eTC_HDR_ID_MAIN;
					if(SCMP(p1, "main", 5)==0) {
						hdrID = eTC_HDR_ID_MAIN;
					}
					else if(SCMP(p1, "mrc", 4)==0) {
						hdrID = eTC_HDR_ID_MRC;
					}
					else if(SCMP(p1, "bs", 3)==0) {
						hdrID = eTC_HDR_ID_BS;
					}
					else if(SCMP(p1, "bsmrc", 4)==0) {
						hdrID = eTC_HDR_ID_BS_MRC;
					}
					else {
						err = -1;
					}

					if(err >= 0) {
						stTC_HDR_STATUS_t hdrStatus;
						hdret = tchdr_getAllStatus(hdrID, &hdrStatus);
						if(hdret == (HDRET)eTC_HDR_RET_OK) {
							U8 digital_audio_acquired=0;
							U8 sis_crc_ok=0;
							U8 sis_acquired=0;
							U8 hd_signal_acquired=0;

							if((hdrStatus.acqStatus & (U32)0x08) > (U32)0) {
								digital_audio_acquired = 1;
							}
							if((hdrStatus.acqStatus & (U32)0x04) > (U32)0) {
								sis_crc_ok = 1;
							}
							if((hdrStatus.acqStatus & (U32)0x02) > (U32)0) {
								sis_acquired = 1;
							}
							if((hdrStatus.acqStatus & (U32)0x01) > (U32)0) {
								hd_signal_acquired = 1;
							}
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "00) HDR ID: %d\n", (U32)hdrStatus.hdrID);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "01) Playing Program Number: %d\n", hdrStatus.curPN);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "02) Acquired Status: 0x%02x, digital_audio_acquired[%d]:sis_crc_ok[%d]:sis_acquired[%d]:hd_signal_acquired[%d]\n",
									hdrStatus.acqStatus, digital_audio_acquired, sis_crc_ok, sis_acquired, hd_signal_acquired);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "03) Digital Audio Quality Indicator: %d\n", hdrStatus.audioQualityIndicator);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "04) CNR: %d\n", hdrStatus.cnr);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "05) TX Digital Audio Gain: %d\n", hdrStatus.digitalAudioGain);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "06) TX Blend Control: %d\n", hdrStatus.blendControl);
							for(i=(U32)eTC_HDR_PROGRAM_HD1; i<(U32)eTC_HDR_PROGRAM_MAX; i++) {
								if(i==(U32)eTC_HDR_PROGRAM_HD1) {
									HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "07) Program Types: ");
								}
								HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "[PTY%d: %d] ", i, hdrStatus.pty[i]);
								if(i==((U32)eTC_HDR_PROGRAM_MAX-(U32)1)) {
									HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "\n");
								}
							}
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "08) Playing Program Type: %d\n", hdrStatus.curPty);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "09) Available Program Bitmap: 0x%02x\n", hdrStatus.pmap);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "10) Changed PSD Program Bitmap: 0x%02x\n", hdrStatus.chgPmap);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "11) Primary Service Mode(PSM): %d\n", hdrStatus.psm);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "12) Codec Mode: %d\n", hdrStatus.codecMode);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "13) Filtered Digital Siganal Quality Measurement(DSQM): %d\n", hdrStatus.dsqm);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "14) Hybrid Program Status: %d\n", hdrStatus.hybridProgram);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "15) Raw SNR: %d\n", hdrStatus.rawSnr);
						}
					}
				}
			}
			else if(SCMP(p0, "sig", 4)==0) {
				if(argc == 2) {
					eTC_HDR_ID_t hdrID = eTC_HDR_ID_MAIN;
					if(SCMP(p1, "main", 5)==0) {
						hdrID = eTC_HDR_ID_MAIN;
					}
					else if(SCMP(p1, "mrc", 4)==0) {
						hdrID = eTC_HDR_ID_MRC;
					}
					else if(SCMP(p1, "bs", 3)==0) {
						hdrID = eTC_HDR_ID_BS;
					}
					else if(SCMP(p1, "bsmrc", 6)==0) {
						hdrID = eTC_HDR_ID_BS_MRC;
					}
					else {
						err = -1;
					}

					if(err >= 0) {
						stTC_HDR_SIGNAL_STATUS_t hdrSigStatus;
						hdret = tchdr_getSignalStatus(hdrID, &hdrSigStatus);
						if(hdret == (HDRET)eTC_HDR_RET_OK) {
							U8 digital_audio_acquired=0;
							U8 sis_crc_ok=0;
							U8 sis_acquired=0;
							U8 hd_signal_acquired=0;

							if((hdrSigStatus.acqStatus & (U32)0x08) > (U32)0) {
								digital_audio_acquired = 1;
							}
							if((hdrSigStatus.acqStatus & (U32)0x04) > (U32)0) {
								sis_crc_ok = 1;
							}
							if((hdrSigStatus.acqStatus & (U32)0x02) > (U32)0) {
								sis_acquired = 1;
							}
							if((hdrSigStatus.acqStatus & (U32)0x01) > (U32)0) {
								hd_signal_acquired = 1;
							}
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "00) HDR ID: %d\n", (U32)hdrSigStatus.hdrID);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "01) Playing Program Number: %d\n", hdrSigStatus.curPN);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "02) Available Program Bitmap: %d\n", hdrSigStatus.pmap);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "03) Acquired Status: 0x%02x, digital_audio_acquired[%d]:sis_crc_ok[%d]:sis_acquired[%d]:hd_signal_acquired[%d]\n",
									hdrSigStatus.acqStatus, digital_audio_acquired, sis_crc_ok, sis_acquired, hd_signal_acquired);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "04) CNR: %d\n", hdrSigStatus.cnr);
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "05) Hybrid Program Status: %d\n", hdrSigStatus.hybridProgram);
						}
					}
				}
				else {
					err = -1;
				}
			}
			else if(SCMP(p0, "prog", 5)==0) {
				if(argc == 2) {
					S32 nProg = myatoi(p1);
					if((nProg >= (S32)eTC_HDR_PROGRAM_HD1) && (nProg < (S32)eTC_HDR_PROGRAM_MAX)) {
						hdret = tchdr_setProgram(eTC_HDR_ID_MAIN, (eTC_HDR_PROGRAM_t)nProg);
						if(hdret == (HDRET)eTC_HDR_RET_OK){
							U8 progBitmask = (0x01U << (U8)nProg) & 0x0FFU;
							hdret = tchdr_enablePsdNotification(eTC_HDR_ID_MAIN, progBitmask, PSD_BITMASK, 1U);
						}
					}
					else {
						err = -1;
					}
				}
			}
			else if(SCMP(p0, "aaa", 4)==0) {
				if(argc == 2) {
					if(SCMP(p1, "on", 3)==0) {
						hdret = tchdr_setAutoAudioAlignEnable(1U);
						if(hdret == (HDRET)eTC_HDR_RET_OK) {
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Success. Enable AAA.\n");
						}
					}
					else if(SCMP(p1, "off", 4)==0) {
						hdret = tchdr_setAutoAudioAlignEnable(0U);
						if(hdret == (HDRET)eTC_HDR_RET_OK) {
							HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Success. Disable AAA.\n");
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
			else if(SCMP(p0, "lot", 4) == 0) {
				if(argc == 3){
					if (SCMP(p1, "main", 5) == 0){
						if(SCMP(p2, "on", 3)==0) {
							hdret = tchdr_enableLotNotification(eTC_HDR_ID_MAIN, 0xff, 1);
							if(hdret == (HDRET)eTC_HDR_RET_OK) {
								HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U,"Large Object Transfer (Main) Service On !!\n");
							}
						}
						else if(SCMP(p2, "off", 4)==0){
							hdret = tchdr_enableLotNotification(eTC_HDR_ID_MAIN, 0xff, 0);
							if(hdret == (HDRET)eTC_HDR_RET_OK) {
								HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U,"Large Object Transfer (Main) Service Off !!\n");
							}
						}
						else {
							err = -1;
						}
					}
					else if (SCMP(p1, "bs", 3) == 0) {
						if(SCMP(p2, "on", 3)==0) {
							hdret = tchdr_enableLotNotification(eTC_HDR_ID_BS, 0xff, 1);
							if(hdret == (HDRET)eTC_HDR_RET_OK) {
								HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U,"Large Object Transfer (Background) Service On !!\n");
							}
						}
						else if(SCMP(p2, "off", 4)==0){
							hdret = tchdr_enableLotNotification(eTC_HDR_ID_BS, 0xff, 0);
							if(hdret == (HDRET)eTC_HDR_RET_OK) {
								HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U,"Large Object Transfer (Background) Service Off !!\n");
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

				if(err == -1){
					HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U,"lot [main/bs] [on/off]\n");
				}
			}
			else if (SCMP(p0, "ea", 3) == 0) {
				if(argc == 3){
					if (SCMP(p1, "main", 5) == 0){
						if (SCMP(p2, "on", 3) == 0){
							hdret = tchdr_enableAlertNotification(eTC_HDR_ID_MAIN, 1);
							if(hdret == (HDRET)eTC_HDR_RET_OK){
								HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Emergency Alert Main Service On !!\n");
							}
						}
						else if (SCMP(p2, "off", 4) == 0) {
							hdret = tchdr_enableAlertNotification(eTC_HDR_ID_MAIN, 0);
							if(hdret == (HDRET)eTC_HDR_RET_OK){
								HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Emergency Alert Main Service Off !!\n");
							}
						}
						else {
							err = -1;
						}
					}
					else if (SCMP(p1, "bs", 3) == 0) {
						if (SCMP(p2, "on", 3) == 0){
							hdret = tchdr_enableAlertNotification(eTC_HDR_ID_BS, 1);
							if(hdret == (HDRET)eTC_HDR_RET_OK){
								HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Emergency Alert Background Service On !!\n");
							}
						}
						else if (SCMP(p2, "off", 4) == 0) {
							hdret = tchdr_enableAlertNotification(eTC_HDR_ID_BS, 0);
							if(hdret == (HDRET)eTC_HDR_RET_OK){
								HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Emergency Alert Background Service Off !!\n");
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

				if(err == -1){
					HDRCUI_MAIN_LOG(CUI_ERR_LOG, 0U, "ea [main/bs] [on/off]\n");
				}
			}
			else if(SCMP(p0, "exit", 5)==0) {
				(void)tchdradiocui_close();
				fexit = -1;
				HDRCUI_MAIN_LOG(CUI_MSG_LOG, 0U, "Exiting HD Radio CUI.\n");
			}
			else {
				err = -1;
			}

			if(err == 0) {
				if(hdret != (HDRET)eTC_HDR_RET_OK) {
					HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "Failed to set command! [hdret = %d]\n", hdret);
				}
			}
			else {
				HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, ">>>>>>> Wrong Command... Please try again\n");
			}
		}
	}
	else {
		HDRCUI_MAIN_LOG(CUI_ERR_LOG, 1U, "[%s:%d] argument vector is null.\n", __func__, __LINE__);
	}

	return fexit;
}


#endif	/* #ifdef USE_TELECHIPS_EVB */
