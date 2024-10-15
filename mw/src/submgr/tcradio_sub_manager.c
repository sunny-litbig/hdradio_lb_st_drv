/*******************************************************************************

*   FileName : tcradio_sub_manager.c

*   Copyright (c) Telechips Inc.

*   Description : Radio Sub-core Manager

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

#include "dev_sub_ipc.h"
#include "tcradio_api.h"
#include "tcradio_rds_api.h"
#include "tcradio_sub_manager.h"

#ifdef USE_HDRADIO
#include "tcradio_hdr_if.h"
#endif

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
#define	IPC_SDR_SIGNATURE		(0xA5A7)
#define IPC_DATA_LEN			(1024)
#define	IPC_MIN_LEN				(6)

#define	RADIO_AUDIO_SAMPLERATE	(44100)		// or 48000

#define	FM_START_FREQ			(87500)		// khz
#define	FM_END_FREQ				(107900)	// khz
#define	FM_FREQ_STEP			(200)		// khz
#define	MW_START_FREQ			(530)		// khz
#define	MW_END_FREQ				(1710)		// khz
#define	MW_FREQ_STEP			(10)		// khz

#define	PSD_BITMASK				((uint8)0xDF)

/***************************************************
*        Local type definitions                    *
****************************************************/
typedef enum {
	eRMGR_FM_BAND	= 0,
	eRMGR_MW_BAND	= 1,
	eRMGR_DAB_BAND	= 4,
	eRMGR_MAX_BAND
}eRMGR_BAND_t;

typedef struct {
	uint16 fNewMsg;
	uint16 length;
  	uint16 upperCmd;
	uint16 lowerCmd;
   	uint8 data[IPC_DATA_LEN];
	uint16 crc;
}stSdrIpcBuf_t;

typedef struct{
	int32 fRunning;
	stRADIO_CONFIG_t radioMgrConf;

	uint32 curBand;		// eRMGR_BAND_t
	uint32 curFreq;

	uint32 fmFreq;
	uint32 mwFreq;

	stSdrIpcBuf_t stRxMsg;
	stSdrIpcBuf_t stTxMsg;

	stRADIO_QUALITY_t stSchQdata;
	uint32 scanPI;		// for DAB Seamless Link
}stRADIO_MGR_t;

/***************************************************
*        Local constant definitions                *
****************************************************/
static stRADIO_MGR_t stRadioMgr;

//static int32 fRunning=0;
//static uint32 current_frequency=0;
//static uint32 current_band=0;
//static uint32 fm_frequency=0;
//static uint32 mw_frequency=0;
//static stRADIO_CONFIG_t radioMgrConf;

//static stSdrIpcBuf_t stRxMsg;
//static stSdrIpcBuf_t stTxMsg;

static const uint16 crclut[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

/***************************************************
*        Local function prototypes                 *
****************************************************/
static int tcsubrmgr_precheckSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata);
static int tcsubrmgr_checkSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata);
static void tcsubrmgr_getNotificationCallBack(uint32 notifyID, uint32 *arg, void **pData, int32 errorCode);
static void tcsubrmgr_getStationListCallBack(uint32 totalnum, void *list, int32 errorCode);
static int32 tcsubrmgr_checkCRC16(uint8 *rxbuf);
static uint16 tcsubrmgr_makeCRC16(uint8 *rxbuf);


/***************************************************
*        function definition                       *
****************************************************/
static void tcsubrmgr_16bto8b(uint8 *buf, uint16 data)
{
	uint16 indata = data;
	if(buf != NULL) {
		*(buf+1) = (U8)indata;
		*(buf+0) = (U8)(indata >> 8U);
	}
}

static void tcsubrmgr_32bto8b(uint8 *buf, uint32 data)
{
	uint32 indata = data;
	if(buf != NULL) {
		*(buf+3) = (uint8)indata;
		*(buf+2) = (uint8)(indata >> 8);
		*(buf+1) = (uint8)(indata >> 16);
		*(buf+0) = (uint8)(indata >> 24);
	}
}

static uint32 tcsubrmgr_8bto32b(uint8 *data)
{
	uint32 ret = 0;
	if(data != NULL) {
		ret = (((uint32)data[0] << 24) | ((uint32)data[1] << 16) | ((uint32)data[2] << 8) | ((uint32)data[3] << 0));
	}
	return ret;
}

static uint16 tcsubrmgr_8bto16b(uint8 *data)
{
	uint16 ret = 0;
	if(data != NULL) {
		ret = (((uint16)data[0]) << 8 | ((uint16)data[1]));
	}
	return ret;
}

static RET tcsubrmgr_setSdrIpcMessage(stSdrIpcBuf_t txbuf)
{
	RET ret = eRET_OK;
	uint8 tempbuf[IPC_DATA_LEN+10] = {0,};
	uint32 len = 0;
	int32 crc_ret;
	uint32 i;

	if(txbuf.length >= (uint16)IPC_MIN_LEN) {
		txbuf.fNewMsg = IPC_SDR_SIGNATURE;
		tcsubrmgr_16bto8b(tempbuf, txbuf.fNewMsg);
		tcsubrmgr_16bto8b(tempbuf+2, txbuf.length);
		tcsubrmgr_16bto8b(tempbuf+4, txbuf.upperCmd);
		tcsubrmgr_16bto8b(tempbuf+6, txbuf.lowerCmd);
		for(i=0; i<(txbuf.length-6); i++)
		{
			tempbuf[i+8] = txbuf.data[i];
		}
		txbuf.crc = tcsubrmgr_makeCRC16(tempbuf);
		tcsubrmgr_16bto8b(tempbuf+2+txbuf.length, txbuf.crc);
		ret = dev_ipcSdrSubTx(tempbuf, txbuf.length+4);
		if(ret < 0) {
			SRMGR_ERR("[%s:%d]: Failed to write tx data in tcc_sdr_ipc device!!! ret[%d]\n", __func__, __LINE__, ret);
			ret = eRET_NG_IPC_DRV;
		}
	#ifdef SRMGR_DEBUG
		len = txbuf.length+4;
		SRMGR_DBG("[%s:%d]: LEN[%d] ", __func__, __LINE__, len);
		for(i=0; i<len; i++)
		{
			printf(" %02xh", tempbuf[i]);
		}
		printf("\n");
	#endif
	}
	else {
		ret = eRET_NG_INVALID_LENGTH;
		SRMGR_ERR("[%s:%d]: Failed to write!!! Invalid write length.\n", __func__, __LINE__);
	}

	return ret;
}

static void tcsubrmgr_getSdrIpcMessage(stSdrIpcBuf_t *rxbuf)
{
	uint8 tempbuf[IPC_DATA_LEN+10] = {0,};
	int32 len = 0;
	int32 crc_ret;
	uint32 i;

	if(rxbuf != NULL) {
		rxbuf->fNewMsg = 0;
		len = dev_ipcSdrSubRx(tempbuf, sizeof(tempbuf));

		if((len >= 10) && (len <= sizeof(tempbuf))) {
			crc_ret = tcsubrmgr_checkCRC16(tempbuf);
			if(crc_ret == 0) {
				rxbuf->fNewMsg = tcsubrmgr_8bto16b(tempbuf);
				rxbuf->length = tcsubrmgr_8bto16b(tempbuf+2);
				rxbuf->upperCmd = tcsubrmgr_8bto16b(tempbuf+4);
				rxbuf->lowerCmd = tcsubrmgr_8bto16b(tempbuf+6);
				rxbuf->crc = tcsubrmgr_8bto16b(tempbuf+(rxbuf->length)+2);
				for(i=0; i<(rxbuf->length-6); i++)
				{
					rxbuf->data[i] = tempbuf[i+8];
				}
			}
			else {
				SRMGR_ERR("[%s:%d]: tcc_sdr_ipc rx data crc error!!!\n", __func__, __LINE__);
			}
		}
		else {
			if(len != 0) {	// When poll timeout error, len is 0. This condition is to avoid unnecessary logs.
				SRMGR_ERR("[%s:%d]: Failed to receive rx data from tcc_sdr_ipc device[%d]!!!\n", __func__, __LINE__, len);
			}
		}
	#if 1
		if(len > 0) {
			SRMGR_DBG("[%s:%d]: LEN[%d] ", __func__, __LINE__, len);
			for(i=0; i<len; i++)
			{
				printf(" %02xh", tempbuf[i]);
			}
			printf("\n");
		}
	#endif
	}
	else {
		SRMGR_ERR("[%s:%d]: Ivalid input paramter!!!\n", __func__, __LINE__);
	}

}

static void tcsubrmgr_initValiable(void)
{
	memset(&stRadioMgr, 0x00, sizeof(stRadioMgr));
}

static void tcsubrmgr_setConf(stRADIO_CONFIG_t *conf)
{
	conf->numTuners = eRADIO_CONF_TYPE_DUAL;		// Default TC EVB tuner is dual.

	conf->fPhaseDiversity = NO;
	conf->fIqOut = YES;
	conf->audioSamplerate = RADIO_AUDIO_SAMPLERATE;
	conf->fExtAppCtrl = 0;

	conf->sdr = eRADIO_SDR_HD;
	conf->hdType = eRADIO_HD_TYPE_HD1p0;
	conf->area = eRADIO_CONF_AREA_NA;
	conf->initMode = eRADIO_FM_MODE;
	conf->initFreq = FM_START_FREQ;
}

static RET tcsubrmgr_init(void)
{
	RET ret;

	tcradio_configOnGetNotificationCallBack(tcsubrmgr_getNotificationCallBack);
	tcradio_configOnGetStationListCallBack(tcsubrmgr_getStationListCallBack);

	tcradio_configOnPrecheckSeekQual(tcsubrmgr_precheckSeekQual);
	tcradio_configOnCheckSeekQual(tcsubrmgr_checkSeekQual);

	stRadioMgr.fmFreq = FM_START_FREQ;
	stRadioMgr.mwFreq = MW_START_FREQ;
	stRadioMgr.curFreq = FM_START_FREQ;
	stRadioMgr.curBand = eRMGR_FM_BAND;

	ret = tcradio_init();
	if(ret != eRET_OK){
		if(ret == eRET_NG_ALREADY_INIT) {
			SRMGR_WRN("Already init tcradio!!![%d]\n", ret);
		}
		else {
			SRMGR_ERR("tcradio_init is failed!!![%d]\n", ret);
		}
	}

	if(ret == eRET_OK) {
		ret = tcradio_setBandFreqConfig(eRADIO_FM_MODE, FM_START_FREQ, FM_END_FREQ, FM_FREQ_STEP);			// FM Setting
		if(ret != eRET_OK) {
			SRMGR_ERR("tcradio_setBandFreqConfig is failed[FM:%d]. Set to default.\n", ret);
		}
		ret = tcradio_setBandFreqConfig(eRADIO_AM_MODE, MW_START_FREQ, MW_END_FREQ, MW_FREQ_STEP);			// AM Setting
		if(ret != eRET_OK) {
			SRMGR_ERR("tcradio_setBandFreqConfig is failed[AM:%d]. Set to default.\n", ret);
		}
	}

	return ret;
}

static void tcsubrmgr_eventHandler(stSdrIpcBuf_t rxbuf, stSdrIpcBuf_t *txbuf)
{
	uint16 fRadioReq = (rxbuf.upperCmd & (uint16)IVI_REQ_UCMD);
	int32 ret = eRET_NG_NOT_SUPPORT;
	int32 fresp = 0;

	if(fRadioReq == (uint16)IVI_REQ_UCMD) {
		switch(rxbuf.lowerCmd) {
			case eRMGR_IPC_CMD_Init:
				ret = tcsubrmgr_init();
				txbuf->length = (uint16)IPC_MIN_LEN + 2;
				txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
				txbuf->lowerCmd = rxbuf.lowerCmd;
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				if(ret != eRET_OK) {
					SRMGR_ERR("Failed to init radio!!! error[%d]\n", ret);
				}
				else {
					SRMGR_DBG("Radio init success.\n");
				}
				fresp = 1;
				break;
			case eRMGR_IPC_CMD_Deinit:	// callback
				ret = tcradio_deinit();
				if(ret != eRET_OK) {
					SRMGR_ERR("Failed to deinit radio!!! error[%d]\n", ret);
				}
				else {
					SRMGR_DBG("Radio deinit success.\n");
				}
				break;
			case eRMGR_IPC_CMD_Open:	// callback
			//	tcsubrmgr_setConf(&stRadioMgr.radioMgrConf);
				stRadioMgr.radioMgrConf.area = (eRADIO_CONF_AREA_t)rxbuf.data[0];
				stRadioMgr.radioMgrConf.initMode = (eRADIO_MOD_MODE_t)rxbuf.data[1];
				stRadioMgr.radioMgrConf.initFreq = tcsubrmgr_8bto32b(rxbuf.data+2);
				stRadioMgr.radioMgrConf.numTuners = (uint8)rxbuf.data[6];
				stRadioMgr.radioMgrConf.fPhaseDiversity = (uint8)rxbuf.data[7];
				stRadioMgr.radioMgrConf.fIqOut = (uint8)rxbuf.data[8];
				stRadioMgr.radioMgrConf.audioSamplerate = tcsubrmgr_8bto16b(rxbuf.data+9);
				stRadioMgr.radioMgrConf.fExtAppCtrl = (uint8)rxbuf.data[11];
				stRadioMgr.radioMgrConf.sdr = (uint8)rxbuf.data[12];
				stRadioMgr.radioMgrConf.hdType = (uint8)rxbuf.data[13];
				ret = tcradio_open(&stRadioMgr.radioMgrConf);
				if(ret != eRET_OK) {
					fresp = 1;
					txbuf->length = (uint16)IPC_MIN_LEN + 2;
					txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
					txbuf->lowerCmd = rxbuf.lowerCmd;
					tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
					SRMGR_ERR("Failed to open radio!!! error[%d]\n", ret);
				}
				else {
					SRMGR_DBG("Radio open success.\n");
				}
				break;
			case eRMGR_IPC_CMD_Close:
				ret = tcradio_close();
				txbuf->length = (uint16)IPC_MIN_LEN + 2;
				txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
				txbuf->lowerCmd = rxbuf.lowerCmd;
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				if(ret != eRET_OK) {
					SRMGR_ERR("Failed to close radio!!! error[%d]\n", ret);
				}
				else {
					SRMGR_DBG("Radio close success.\n");
				}
				fresp = 1;
				break;
		    case eRMGR_IPC_CMD_SetBandFreqConfig:
		    {
				eRADIO_MOD_MODE_t reqmode = (eRADIO_MOD_MODE_t)rxbuf.data[0];
				uint32 startfreq = tcsubrmgr_8bto32b(rxbuf.data+1);
				uint32 endfreq = tcsubrmgr_8bto32b(rxbuf.data+5);
				uint32 stepfreq = (uint32)rxbuf.data[9];
				ret = tcradio_setBandFreqConfig(reqmode, startfreq, endfreq, stepfreq);
				txbuf->length = (uint16)IPC_MIN_LEN + 2;
				txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
				txbuf->lowerCmd = rxbuf.lowerCmd;
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				if(ret != eRET_OK) {
					SRMGR_ERR("Failed to set radio command[%d]!!! error[%d]\n", rxbuf.lowerCmd, ret);
				}
				else {
					SRMGR_DBG("Radio SetBandFreqConfig[md:%d, sf:%dkHz, ef:%dkHz, step:%dkHz] success. LCMD[%04xh]\n",reqmode, startfreq, endfreq, stepfreq, rxbuf.lowerCmd);
				}
				fresp = 1;
		    }
				break;
		    case eRMGR_IPC_CMD_SetTune:	// callback
		    {
				eRADIO_MOD_MODE_t reqmode = (eRADIO_MOD_MODE_t)rxbuf.data[0];
				uint32 reqfreq = tcsubrmgr_8bto32b(rxbuf.data+1);
				uint32 reqtunemode = (uint32)rxbuf.data[5];
				uint32 reqtunerid = (uint32)rxbuf.data[6];
				ret = tcradio_setTune(reqmode, reqfreq, reqtunemode, reqtunerid);
				if(ret != eRET_OK) {
					fresp = 1;
					txbuf->length = (uint16)IPC_MIN_LEN + 2;
					txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
					txbuf->lowerCmd = rxbuf.lowerCmd;
					tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
					SRMGR_ERR("Failed to tune frequency!!! error[%d]\n", ret);
				}
				else {
					SRMGR_DBG("Radio tune frequency[%dkHz] success. LCMD[%04xh]\n", reqfreq, rxbuf.lowerCmd);
				}
		    }
				break;
		    case eRMGR_IPC_CMD_SetSeek:	// callback
		    {
				eRADIO_SEEK_MODE_t seekcmd = (eRADIO_SEEK_MODE_t)rxbuf.data[0];
				uint32 picode[128] = {0,};
				picode[0] = tcsubrmgr_8bto32b(rxbuf.data+1);
				ret = tcradio_setSeek(seekcmd, picode);
				if(ret != eRET_OK) {
					fresp = 1;
					txbuf->length = (uint16)IPC_MIN_LEN + 2;
					txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
					txbuf->lowerCmd = rxbuf.lowerCmd;
					tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
					SRMGR_ERR("Failed to set seek command!!! error[%d]\n", ret);
				}
				else {
					SRMGR_DBG("Radio Seek command[%d] success. LCMD[%04xh]\n", seekcmd, rxbuf.lowerCmd);
				}
		    }
				break;
		    case eRMGR_IPC_CMD_SetAudio:
				if(rxbuf.data[0] == 0) {
					ret = tcradio_setAudio((uint32)0);
				}
				else if(rxbuf.data[0] == 1) {
					ret = tcradio_setAudio((uint32)1);
				}
				else {
					ret = eRET_NG_INVALID_PARAM;
				}
				txbuf->length = (uint16)IPC_MIN_LEN + 2;
				txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
				txbuf->lowerCmd = rxbuf.lowerCmd;
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				if(ret != eRET_OK) {
					SRMGR_ERR("Failed to set audio command!!! error[%d]\n", ret);
				}
				else {
					SRMGR_DBG("Radio set audio command[%d] success. LCMD[%04xh]\n", rxbuf.data[0], rxbuf.lowerCmd);
				}
				fresp = 1;
				break;
		    case eRMGR_IPC_CMD_SetAudioDevice:
				if(rxbuf.data[0] == 0) {
					ret = tcradio_setAudioDevice(&stRadioMgr.radioMgrConf, (uint32)0);
				}
				else if(rxbuf.data[0] == 1) {
					ret = tcradio_setAudioDevice(&stRadioMgr.radioMgrConf, (uint32)1);
				}
				else {
					ret = eRET_NG_INVALID_PARAM;
				}
				txbuf->length = (uint16)IPC_MIN_LEN + 2;
				txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
				txbuf->lowerCmd = rxbuf.lowerCmd;
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				if(ret != eRET_OK) {
					SRMGR_ERR("Failed to set audio device command!!! error[%d]\n", ret);
				}
				else {
					SRMGR_DBG("Radio set audio device command[%d] success. LCMD[%04xh]\n", rxbuf.data[0], rxbuf.lowerCmd);
				}
				fresp = 1;
				break;
		    case eRMGR_IPC_CMD_GetBandFreqConfig:
		    {
				eRADIO_MOD_MODE_t reqmode = (eRADIO_MOD_MODE_t)rxbuf.data[0];
				uint32 startfreq;
				uint32 endfreq;
				uint32 stepfreq;
				ret = tcradio_getBandFreqConfig(reqmode, &startfreq, &endfreq, &stepfreq);
				txbuf->length = (uint16)IPC_MIN_LEN + 12;
				txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
				txbuf->lowerCmd = rxbuf.lowerCmd;
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				txbuf->data[2] = (uint8)reqmode;
				tcsubrmgr_32bto8b(txbuf->data+3, startfreq);
				tcsubrmgr_32bto8b(txbuf->data+7, endfreq);
				txbuf->data[11] = (uint8)stepfreq;
				if(ret != eRET_OK) {
					txbuf->length = (uint16)IPC_MIN_LEN + 2;
					SRMGR_ERR("Failed to set radio command[%d]!!! error[%d]\n", rxbuf.lowerCmd, ret);
				}
				else {
					SRMGR_DBG("Radio command[%d] success.\n", rxbuf.lowerCmd);
					SRMGR_DBG("Radio GetBandFreqConfig[md:%d, sf:%dkHz, ef:%dkHz, step:%dkHz] success. LCMD[%04xh]\n", reqmode, startfreq, endfreq, stepfreq, rxbuf.lowerCmd);
				}
				fresp = 1;
		    }
				break;
		    case eRMGR_IPC_CMD_GetTune:
		    {
				uint32 ntuner = (uint32)rxbuf.data[0];
				eRADIO_MOD_MODE_t curmode;
				uint32 curfreq;
				ret = tcradio_getTune(&curmode, &curfreq, ntuner);
				txbuf->length = (uint16)IPC_MIN_LEN + 8;
				txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
				txbuf->lowerCmd = rxbuf.lowerCmd;
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				txbuf->data[2] = ntuner;
				txbuf->data[3] = curmode;
				tcsubrmgr_32bto8b(txbuf->data+4, curfreq);
				if(ret != eRET_OK) {
					txbuf->length = (uint16)IPC_MIN_LEN + 2;
					SRMGR_ERR("Failed to set radio command[%d]!!! error[%d]\n", rxbuf.lowerCmd, ret);
				}
				else {
					SRMGR_DBG("Radio command[%d] success.\n", rxbuf.lowerCmd);
					SRMGR_DBG("Radio get tune command[id:%d, md:%d, cf:%dkHz] success. LCMD[%04xh]\n", ntuner, curmode, curfreq, rxbuf.lowerCmd);
				}
				fresp = 1;
		    }
				break;
			case eRMGR_IPC_CMD_GetHdrAllStatus:
			{
				eTC_HDR_ID_t hdrID = (eTC_HDR_ID_t)rxbuf.data[0];
				stTC_HDR_STATUS_t rcvStatus;
				ret = tcradio_getHdrAllStatus(hdrID, &rcvStatus);
				txbuf->length = (uint16)IPC_MIN_LEN + 29;
				txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
				txbuf->lowerCmd = rxbuf.lowerCmd;
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				if(ret != eRET_OK) {
					txbuf->length = (uint16)IPC_MIN_LEN + 2;
					SRMGR_ERR("Failed to set radio command[%d]!!! error[%d]\n", rxbuf.lowerCmd, ret);
				}
				else {
					txbuf->data[2] = (uint8)rcvStatus.hdrID;
					txbuf->data[3] = (uint8)rcvStatus.curPN;
					txbuf->data[4] = (uint8)rcvStatus.acqStatus;
					txbuf->data[5] = (uint8)rcvStatus.audioQualityIndicator;
					txbuf->data[6] = (uint8)rcvStatus.cnr;
					txbuf->data[7] = (uint8)rcvStatus.digitalAudioGain;
					txbuf->data[8] = (uint8)rcvStatus.blendControl;
					txbuf->data[9] = (uint8)rcvStatus.pty[0];
					txbuf->data[10] = (uint8)rcvStatus.pty[1];
					txbuf->data[11] = (uint8)rcvStatus.pty[2];
					txbuf->data[12] = (uint8)rcvStatus.pty[3];
					txbuf->data[13] = (uint8)rcvStatus.pty[4];
					txbuf->data[14] = (uint8)rcvStatus.pty[5];
					txbuf->data[15] = (uint8)rcvStatus.pty[6];
					txbuf->data[16] = (uint8)rcvStatus.pty[7];
					txbuf->data[17] = (uint8)rcvStatus.curPty;
					txbuf->data[18] = (uint8)rcvStatus.pmap;
					txbuf->data[19] = (uint8)rcvStatus.chgPmap;
					txbuf->data[20] = (uint8)rcvStatus.psm;
					txbuf->data[21] = (uint8)rcvStatus.codecMode;
					txbuf->data[22] = (uint8)rcvStatus.hybridProgram;
					tcsubrmgr_16bto8b(txbuf->data+23, (uint16)rcvStatus.dsqm);
					tcsubrmgr_32bto8b(txbuf->data+25, rcvStatus.rawSnr);
					SRMGR_DBG("Radio GetHdrAllStatus success. LCMD[%04xh]\n", rxbuf.lowerCmd);
				}
				fresp = 1;
			}
				break;
			case eRMGR_IPC_CMD_GetHdrSignalStatus:
			{
				eTC_HDR_ID_t hdrID = (eTC_HDR_ID_t)rxbuf.data[0];
				stTC_HDR_SIGNAL_STATUS_t rcvStatus;
				ret = tcradio_getHdrSignalStatus(hdrID, &rcvStatus);
				txbuf->length = (uint16)IPC_MIN_LEN + 8;
				txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
				txbuf->lowerCmd = rxbuf.lowerCmd;
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				if(ret != eRET_OK) {
					txbuf->length = (uint16)IPC_MIN_LEN + 2;
					SRMGR_ERR("Failed to set radio command[%d]!!! error[%d]\n", rxbuf.lowerCmd, ret);
				}
				else {
					txbuf->data[2] = (uint8)rcvStatus.hdrID;
					txbuf->data[3] = (uint8)rcvStatus.curPN;
					txbuf->data[4] = (uint8)rcvStatus.acqStatus;
					txbuf->data[5] = (uint8)rcvStatus.cnr;
					txbuf->data[6] = (uint8)rcvStatus.pmap;
					txbuf->data[7] = (uint8)rcvStatus.hybridProgram;
					SRMGR_DBG("Radio GetHdrSignalStatus success. LCMD[%04xh]\n", rxbuf.lowerCmd);
				}
				fresp = 1;
			}
				break;
			case eRMGR_IPC_CMD_SetHdrProgram:
			{
				eTC_HDR_ID_t hdrID = (eTC_HDR_ID_t)rxbuf.data[0];
				uint8 hdrPN = rxbuf.data[1];
				if(hdrID == eTC_HDR_ID_MAIN) {
					ret = tcradio_setHdrProgram((uint32)hdrPN);
					if(ret == eTC_HDR_RET_OK){
						ret = tcradio_setHdrPsdNotification((uint32)hdrPN,PSD_BITMASK);
					}
				}
				txbuf->length = (uint16)IPC_MIN_LEN + 4;
				txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
				txbuf->lowerCmd = rxbuf.lowerCmd;
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				if(ret != eRET_OK) {
					txbuf->length = (uint16)IPC_MIN_LEN + 2;
					SRMGR_ERR("Failed to set radio command[%d]!!! error[%d]\n", rxbuf.lowerCmd, ret);
				}
				else {
					txbuf->data[2] = (uint8)hdrID;
					txbuf->data[3] = hdrPN;
					SRMGR_DBG("Radio SetHdrProgram success. LCMD[%04xh]\n", rxbuf.lowerCmd);
				}
				fresp = 1;
			}
				break;
			case eRMGR_IPC_CMD_SetHdrAudioMode:	// callback
			{
				uint8 audmode = rxbuf.data[0];
				ret = tcradio_setHdrAudioMode((uint32)audmode);
				if(ret != eRET_OK) {
					fresp = 1;
					txbuf->length = (uint16)IPC_MIN_LEN + 2;
					txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
					txbuf->lowerCmd = rxbuf.lowerCmd;
					tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
					SRMGR_ERR("Failed to set radio command[%d]!!! error[%d]\n", rxbuf.lowerCmd, ret);
				}
				else {
					SRMGR_DBG("Radio SetHdrAudioMode[%d] success. LCMD[%04xh]\n", audmode, rxbuf.lowerCmd);
				}
			}
				break;
			case eRMGR_IPC_CMD_SetHdrMute:
			{
				uint8 mute = rxbuf.data[0];
				ret = tcradioservice_setHdrMute(mute);
				txbuf->length = (uint16)IPC_MIN_LEN + 3;
				txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
				txbuf->lowerCmd = rxbuf.lowerCmd;
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				if(ret != eRET_OK) {
					txbuf->length = (uint16)IPC_MIN_LEN + 2;
					SRMGR_ERR("Failed to set radio command[%d]!!! error[%d]\n", rxbuf.lowerCmd, ret);
				}
				else {
					txbuf->data[2] = mute;
					SRMGR_DBG("Radio SetHdrMute[%d] success. LCMD[%04xh]\n", mute, rxbuf.lowerCmd);
				}
				fresp = 1;
			}
				break;
			case eRMGR_IPC_CMD_SetHdrAduioCtrl:
			{
				uint8 audctrl = rxbuf.data[0];
				ret = tcradioservice_setHdrAudioCtrl(audctrl);
				txbuf->length = (uint16)IPC_MIN_LEN + 3;
				txbuf->upperCmd = (uint16)SUB_RESP_UCMD;
				txbuf->lowerCmd = rxbuf.lowerCmd;
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				if(ret != eRET_OK) {
					txbuf->length = (uint16)IPC_MIN_LEN + 2;
					SRMGR_ERR("Failed to set radio command[%d]!!! error[%d]\n", rxbuf.lowerCmd, ret);
				}
				else {
					txbuf->data[2] = audctrl;
					SRMGR_DBG("Radio SetHdrAduioCtrl[%d] success. LCMD[%04xh]\n", audctrl, rxbuf.lowerCmd);
				}
				fresp = 1;
			}
				break;
			case eRMGR_IPC_CMD_SetHdrAudioMuteFader:
				//todo
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				if(ret != eRET_OK) {
					SRMGR_ERR("Failed to set radio command[%d]!!! error[%d]\n", rxbuf.lowerCmd, ret);
				}
				else {
					SRMGR_DBG("Radio SetHdrAudioMuteFader success. LCMD[%04xh]\n", rxbuf.lowerCmd);
				}
				fresp = 1;
				break;
			case eRMGR_IPC_CMD_GetHdrAudioMuteFader:
				// todo
				tcsubrmgr_16bto8b(txbuf->data, (uint16)ret);
				if(ret != eRET_OK) {
					SRMGR_ERR("Failed to set radio command[%d]!!! error[%d]\n", rxbuf.lowerCmd, ret);
				}
				else {
					SRMGR_DBG("Radio GetHdrAudioMuteFader success. LCMD[%04xh]\n", rxbuf.lowerCmd);
				}
				fresp = 1;
				break;
			default:
				SRMGR_WRN("This command is not supported or not.  CMD[0x%04x 0x%04x]!!!\n", rxbuf.upperCmd, rxbuf.lowerCmd);
				break;
		}
	}
	else {
		SRMGR_WRN("This command is not supported or not.  CMD[0x%04x 0x%04x]!!!\n", rxbuf.upperCmd, rxbuf.lowerCmd);
	}

	if(fresp == 1) {
		tcsubrmgr_setSdrIpcMessage(*txbuf);
	}
}

static int tcsubrmgr_precheckSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata, uint32 freq, uint32 ntuner)
{
	int32 ret=-1, rssi=0;

	if(qdata.type == eRADIO_TYPE1)
	{
		stRADIO_TYPE1_QUALITY_t temp_qdata;
		memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(stRadioMgr.curBand == eRMGR_FM_BAND) {
			rssi = (int32)temp_qdata.fm.Rssi;
			if(rssi >= 20)
			{
				ret = 0;
			}
		}
		else if(stRadioMgr.curBand == eRMGR_MW_BAND) {
			rssi = (int32)temp_qdata.am.Rssi;
			if(rssi >= 38)
			{
				ret = 0;
			}
		}
		else if(stRadioMgr.curBand == eRMGR_DAB_BAND) {
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

		if(stRadioMgr.curBand == eRMGR_FM_BAND) {
			rssi = (int32)temp_qdata.fm.Rssi;
			if(rssi >= 280)
			{
				ret = 0;
			}
		}
		else if(stRadioMgr.curBand == eRMGR_MW_BAND){
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

	return ret;
}

static int tcsubrmgr_checkSeekQual(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t qdata)
{
	int32 ret=-1;
	int32 rssi=0, snr=0, offs=0, sqi=0;
	uint32 usn=0, mpth=0, noise=0, detect=0;

	if(qdata.type == eRADIO_TYPE1)
	{
		stRADIO_TYPE1_QUALITY_t temp_qdata;
		memcpy((void *)&temp_qdata, (void *)(&qdata.qual), sizeof(temp_qdata));

		if(stRadioMgr.curBand == eRMGR_FM_BAND) {
			rssi = (int32)temp_qdata.fm.Rssi;
			snr = (int32)temp_qdata.fm.Snr;
			offs = (int32)temp_qdata.fm.Offs;
			if((rssi >= 20) && (snr > 4) && (offs > -6) && (offs < 6))
			{
				ret = 0;
			}
		}
		else if(stRadioMgr.curBand == eRMGR_MW_BAND) {
			rssi = (int32)temp_qdata.am.Rssi;
			snr = (int32)temp_qdata.am.Snr;
			offs = (int32)temp_qdata.am.Offs;
			if((rssi >= 38) && (snr > 6) && (offs > -6) && (offs < 6))
			{
				ret = 0;
			}
		}
		else if(stRadioMgr.curBand == eRMGR_DAB_BAND) {
			rssi = (int32)temp_qdata.dab.Rssi;
			sqi = (int32)temp_qdata.dab.Sqi;
			detect = (int32)temp_qdata.dab.Detect;
			if((rssi >= -107) && (sqi > 8) && (detect > 0))
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

		if(stRadioMgr.curBand == eRMGR_FM_BAND) {
			rssi = (int32)temp_qdata.fm.Rssi;
			offs = (int32)temp_qdata.fm.Offs;
			usn = temp_qdata.fm.Usn;
			mpth = temp_qdata.fm.Mpth;
			if((rssi >= 280) && (offs > -100) && (offs < 100) && (usn < 120) && (mpth < 200))
			{
				ret = 0;
			}
		}
		else if(stRadioMgr.curBand == eRMGR_MW_BAND) {
			rssi = (int32)temp_qdata.am.Rssi;
			offs = (int32)temp_qdata.am.Offs;
			noise = temp_qdata.am.Hfn;
			if((rssi >= 630) && (offs > -50) && (offs < 50) && (noise < 100))
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

static void tcsubrmgr_getNotificationCallBack(uint32 notifyID, uint32 *arg, void **pData, int32 errorCode)
{
	uint32 i;

	switch(notifyID) {
		case eRADIO_NOTIFY_OPEN:
			stRadioMgr.stTxMsg.upperCmd = (uint16)SUB_RESP_UCMD;
			stRadioMgr.stTxMsg.lowerCmd = (uint16)eRMGR_IPC_CMD_Open;
			if(errorCode != eRET_OK && errorCode != eRET_NG_ALREADY_OPEN) {
				stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+2;
				SRMGR_ERR("eRADIO_NOTIFY_OPEN : Error[%d]!\n", errorCode);
			}
			else {
				SRMGR_DBG("eRADIO_NOTIFY_OPEN : Done! band[%d], freq[%d]\n", arg[0], arg[1]);
				if(arg[0] == eRADIO_AM_MODE) {
					// If you use SW or LW, more conditionals will be needed.
					stRadioMgr.curBand = eRMGR_MW_BAND;
					stRadioMgr.mwFreq = stRadioMgr.curFreq = arg[1];
				}
				else { // eRADIO_FM_MODE
					stRadioMgr.curBand = eRMGR_FM_BAND;
					stRadioMgr.fmFreq = stRadioMgr.curFreq = arg[1];
				}

				if(stRadioMgr.radioMgrConf.sdr == eRADIO_SDR_NONE) {
					tcradio_setAudioDevice(&stRadioMgr.radioMgrConf, ON);
					tcradio_setAudio(1);	// play
				}
				stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+7;
				stRadioMgr.stTxMsg.data[2] = (uint8)stRadioMgr.curBand;
				tcsubrmgr_32bto8b(stRadioMgr.stTxMsg.data+3, arg[1]);
			}
			tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data, (uint16)errorCode);
			tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);
			break;

		case eRADIO_NOTIFY_DEINIT:
			stRadioMgr.stTxMsg.upperCmd = (uint16)SUB_RESP_UCMD;
			stRadioMgr.stTxMsg.lowerCmd = (uint16)eRMGR_IPC_CMD_Deinit;
			stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+2;
			tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data, (uint16)errorCode);
			tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);
			if(errorCode != eRET_OK) {
				SRMGR_ERR("eRADIO_NOTIFY_DEINIT : Error[%d]!\n", errorCode);
			}
			else {
				SRMGR_DBG("eRADIO_NOTIFY_DEINIT : Done!\n");
			}
			break;

	    case eRADIO_NOTIFY_SEEK_MODE:
			stRadioMgr.stTxMsg.upperCmd = (uint16)SUB_RESP_UCMD;
			stRadioMgr.stTxMsg.lowerCmd = (uint16)eRMGR_IPC_CMD_SetSeek;
			tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data, errorCode);
			if(errorCode != eRET_OK) {
				SRMGR_ERR("eRADIO_NOTIFY_SET_SEEK : Error[%d]! band[%d], freq[%d], seekmode[%d]\n", errorCode,arg[0], arg[1], arg[2]);
			}
			else {
				stRadioMgr.curFreq = arg[1];
				SRMGR_DBG("eRADIO_NOTIFY_SET_SEEK : Done! band[%d], freq[%d], seekmode[%d]\n", arg[0], arg[1], arg[2]);
			}
			if(arg[2] == 3 || arg[2] == 4 || arg[2] == 5) {		// 3: Auto Up, 4: Auto Down, 5: Scan List
				if(arg[3] == eRADIO_TYPE1)
				{
					stRADIO_TYPE1_QUALITY_t qvalue;
					memcpy((void *)&qvalue, (void *)(arg+4), sizeof(qvalue));

					stRadioMgr.stTxMsg.data[2] = (uint8)arg[0];
					tcsubrmgr_32bto8b(stRadioMgr.stTxMsg.data+3, arg[1]);
					stRadioMgr.stTxMsg.data[7] = (uint8)arg[2];
					stRadioMgr.stTxMsg.data[8] = (uint8)arg[3];
					stRadioMgr.stTxMsg.data[9] = (uint8)0;

					if(arg[0] == eRADIO_AM_MODE) {	// AM
						stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+18;
						tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+10, (uint16)qvalue.am.Rssi);
						tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+12, (uint16)qvalue.am.Snr);
						tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+14, (uint16)qvalue.am.Mod);
						tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+16, (uint16)qvalue.am.Offs);
						tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);
						SRMGR_INF("RSSI = %d\n", qvalue.am.Rssi);
						SRMGR_INF("SNR = %d\n", qvalue.am.Snr);
						SRMGR_INF("Modulation = %d\n", qvalue.am.Mod);
						SRMGR_INF("Offset = %d\n", qvalue.am.Offs);
					}
					else if(arg[0] == eRADIO_FM_MODE) {	// FM
						stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+24;
						tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+10, (uint16)qvalue.fm.Rssi);
						tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+12, (uint16)qvalue.fm.Snr);
						tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+14, (uint16)qvalue.fm.Dev);
						tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+16, (uint16)qvalue.fm.Offs);
						tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+18, (uint16)qvalue.fm.Pilot);
						tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+20, (uint16)qvalue.fm.Mpth);
						tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+22, (uint16)qvalue.fm.Usn);
						tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);
						SRMGR_INF("RSSI = %d\n", qvalue.fm.Rssi);
						SRMGR_INF("SNR = %d\n", qvalue.fm.Snr);
						SRMGR_INF("Deviation = %d\n", qvalue.fm.Dev);
						SRMGR_INF("Offset = %d\n", qvalue.fm.Offs);
						SRMGR_INF("Pilot = %d\n", qvalue.fm.Pilot);
						SRMGR_INF("MultiPath = %d\n", qvalue.fm.Mpth);
						SRMGR_INF("USN = %d\n", qvalue.fm.Usn);
					}
					else {
						// other band.
						stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+10;
						tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);
					}
				}
			}

			if(arg[2] == eRADIO_SEEK_STOP) {
				stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+10;
				stRadioMgr.stTxMsg.data[2] = (uint8)arg[0];
				tcsubrmgr_32bto8b(stRadioMgr.stTxMsg.data+3, arg[1]);
				stRadioMgr.stTxMsg.data[7] = (uint8)arg[2];
				stRadioMgr.stTxMsg.data[8] = (uint8)eRADIO_TYPE1;
				stRadioMgr.stTxMsg.data[9] = (uint8)arg[3];
				tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);
				if(arg[3] == 0) {
					SRMGR_INF("Not seek or Can't found radio station!!! [%d]\n", arg[3]);
				}
				else {
					SRMGR_INF("Found radio station!!! [%d]\n", arg[3]);
				}

				if(arg[0] == eRADIO_FM_MODE) {
					stRadioMgr.curBand = eRMGR_FM_BAND;
					stRadioMgr.fmFreq = stRadioMgr.curFreq = arg[1];
				}
				else if(arg[0] == eRADIO_AM_MODE) {
					// If you use SW or LW, more conditionals will be needed.
					stRadioMgr.curBand = eRMGR_MW_BAND;
					stRadioMgr.mwFreq = stRadioMgr.curFreq = arg[1];
				}
				else {
					;	// Other user mode can be set.
				}
			}
			break;

		case eRADIO_NOTIFY_TUNE:
			stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+9;
			stRadioMgr.stTxMsg.upperCmd = (uint16)SUB_RESP_UCMD;
			stRadioMgr.stTxMsg.lowerCmd = (uint16)eRMGR_IPC_CMD_SetTune;
			tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data, errorCode);
			stRadioMgr.stTxMsg.data[2] = (uint8)arg[0];
			tcsubrmgr_32bto8b(stRadioMgr.stTxMsg.data+3, arg[1]);
			stRadioMgr.stTxMsg.data[7] = (uint8)arg[2];
			stRadioMgr.stTxMsg.data[8] = (uint8)arg[3];
			tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);
			if(errorCode != eRET_OK) {
				SRMGR_ERR("eRADIO_NOTIFY_GET_FREQ : Error[%d]! band[%d] freq[%d] seekmode[%d] ntuner[%d]\n", errorCode, arg[0], arg[1], arg[2], arg[3]);
			}
			else {
				SRMGR_DBG("eRADIO_NOTIFY_GET_FREQ : Done! band[%d] freq[%d] seekmode[%d] ntuner[%d]\n", arg[0], arg[1], arg[2], arg[3]);
			}

			if(arg[3] == eRADIO_ID_PRIMARY) {
				if(arg[0] == eRADIO_FM_MODE) {
					stRadioMgr.curBand = eRMGR_FM_BAND;
					stRadioMgr.fmFreq = stRadioMgr.curFreq = arg[1];
				}
				else if(arg[0] == eRADIO_AM_MODE) {
					// If you use SW or LW, more conditionals will be needed.
					stRadioMgr.curBand = eRMGR_MW_BAND;
					stRadioMgr.mwFreq = stRadioMgr.curFreq = arg[1];
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
			stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+2;
			stRadioMgr.stTxMsg.upperCmd = (uint16)SUB_RESP_UCMD;
			stRadioMgr.stTxMsg.lowerCmd = (uint16)eRMGR_IPC_RESP_HdrOpen;
			tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data, errorCode);
			if(errorCode != eRET_OK) {
				SRMGR_ERR("eRADIO_HD_NOTIFY_OPEN : Error[%d]!\n", errorCode);
			}
			else {
				SRMGR_DBG("eRADIO_HD_NOTIFY_OPEN : [%d] Done!\n", arg[0]);
				if(stRadioMgr.radioMgrConf.fExtAppCtrl==0) {
					tcradio_setAudioDevice(&stRadioMgr.radioMgrConf, ON);
				}
				tcradio_setAudio(1);	// play
			}
			tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);
			break;

		case eRADIO_HD_NOTIFY_AUDIO_MODE:
			stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+2;
			stRadioMgr.stTxMsg.upperCmd = (uint16)SUB_RESP_UCMD;
			stRadioMgr.stTxMsg.lowerCmd = (uint16)eRMGR_IPC_CMD_SetHdrAudioMode;
			tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data, errorCode);
			if(errorCode != eRET_OK) {
				SRMGR_ERR("eRADIO_HD_NOTIFY_AUDIO_MODE : Error[%d]!\n", errorCode);
			}
			else {
				stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+3;
				stRadioMgr.stTxMsg.data[2] = (uint8)arg[0];
				SRMGR_DBG("eRADIO_HD_NOTIFY_AUDIO_MODE : [%d] Done!\n", arg[0]);
			}
			tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);
			break;

		case eRADIO_HD_NOTIFY_PSD:
			if(errorCode != eRET_OK) {
				SRMGR_ERR("eRADIO_HD_NOTIFY_PSD : Error[%d]!\n", errorCode);
			}
			else {
				if(*(pData+0) != NULL) {
					unsigned int type = 0;
					stTC_HDR_PSD_t hdrPsd;
					memcpy((void*)&hdrPsd, *pData, sizeof(stTC_HDR_PSD_t));

					stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+522;
					stRadioMgr.stTxMsg.upperCmd = (uint16)SUB_NOTI_UCMD;
					stRadioMgr.stTxMsg.lowerCmd = (uint16)eRMGR_IPC_NOTI_HdrPSD;
					stRadioMgr.stTxMsg.data[0] = (uint8)arg[0];
					stRadioMgr.stTxMsg.data[1] = (uint8)arg[1];
					memcpy(stRadioMgr.stTxMsg.data+2, hdrPsd.title.data, hdrPsd.title.len);
					stRadioMgr.stTxMsg.data[130] = (uint8)hdrPsd.title.len;
					stRadioMgr.stTxMsg.data[131] = (uint8)hdrPsd.title.charType;
					memcpy(stRadioMgr.stTxMsg.data+132, hdrPsd.artist.data, hdrPsd.artist.len);
					stRadioMgr.stTxMsg.data[260] = (uint8)hdrPsd.artist.len;
					stRadioMgr.stTxMsg.data[261] = (uint8)hdrPsd.artist.charType;
					memcpy(stRadioMgr.stTxMsg.data+262, hdrPsd.album.data, hdrPsd.album.len);
					stRadioMgr.stTxMsg.data[390] = (uint8)hdrPsd.album.len;
					stRadioMgr.stTxMsg.data[391] = (uint8)hdrPsd.album.charType;
					memcpy(stRadioMgr.stTxMsg.data+392, hdrPsd.genre.data, hdrPsd.genre.len);
					stRadioMgr.stTxMsg.data[520] = (uint8)hdrPsd.genre.len;
					stRadioMgr.stTxMsg.data[521] = (uint8)hdrPsd.genre.charType;
					tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);

					SRMGR_DBG("-----------------------------------------------\n");
					SRMGR_DBG("eRADIO_HD_NOTIFY_PSD: HDR ID[%d], PN[%02d]\n", arg[0], arg[1]);
					for(type=eTC_HDR_PSD_TITLE; type<eTC_HDR_PSD_MAX; type++) {
						stTC_HDR_PSD_FORM_t *tempPsdData;
						switch(type) {
							case eTC_HDR_PSD_TITLE:						tempPsdData = &hdrPsd.title;					break;
							case eTC_HDR_PSD_ARTIST:					tempPsdData = &hdrPsd.artist;					break;
							case eTC_HDR_PSD_ALBUM:						tempPsdData = &hdrPsd.album;					break;
							case eTC_HDR_PSD_GENRE:						tempPsdData = &hdrPsd.genre;					break;
							case eTC_HDR_PSD_COMMENT_LANGUAGE:			tempPsdData = &hdrPsd.comment.language;			break;
							case eTC_HDR_PSD_COMMENT_SHORT_CONTENT:		tempPsdData = &hdrPsd.comment.shortContent;		break;
							case eTC_HDR_PSD_COMMENT_ACTUAL_TEXT:		tempPsdData = &hdrPsd.comment.actualText;		break;
							case eTC_HDR_PSD_COMMERCIAL_PRICE_STRING:	tempPsdData = &hdrPsd.commercial.priceString;	break;
							case eTC_HDR_PSD_COMMERCIAL_VALID_UNTIL:	tempPsdData = &hdrPsd.commercial.validUntil;	break;
							case eTC_HDR_PSD_COMMERCIAL_CONTACT_URL:	tempPsdData = &hdrPsd.commercial.contactURL;	break;
							case eTC_HDR_PSD_COMMERCIAL_RECEIVED_AS:	tempPsdData = &hdrPsd.commercial.receivedAs;	break;
							case eTC_HDR_PSD_COMMERCIAL_SELLER_NAME:	tempPsdData = &hdrPsd.commercial.sellerName;	break;
							case eTC_HDR_PSD_COMMERCIAL_DESCRIPTION:	tempPsdData = &hdrPsd.commercial.description;	break;
						}
						if(tempPsdData->len > 0) {
							SRMGR_DBG("PsdType[%d], CharType[0x%02x] DataLength[%d]\n", type, tempPsdData->charType, tempPsdData->len);
							SRMGR_DBG("");
							if(tempPsdData->len > 0) {
								for(i=0; i<tempPsdData->len; i++) {
									printf("%c", tempPsdData->data[i]);
								}
								printf("\n");
							}
						}
					}
				}
			}
			break;

		case eRADIO_HD_NOTIFY_SIS:
			if(errorCode != eRET_OK) {
				SRMGR_DBG("eRADIO_HD_NOTIFY_SIS : Error[%d]!\n", errorCode);
			}
			else {
				if(*(pData+0) != NULL) {
					stTC_HDR_SIS_t sisData;
					memcpy((void*)&sisData, *pData, sizeof(stTC_HDR_SIS_t));

					stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+160;
					stRadioMgr.stTxMsg.upperCmd = (uint16)SUB_NOTI_UCMD;
					stRadioMgr.stTxMsg.lowerCmd = (uint16)eRMGR_IPC_NOTI_HdrSIS;
					stRadioMgr.stTxMsg.data[0] = arg[0];
					tcsubrmgr_32bto8b(stRadioMgr.stTxMsg.data+1, sisData.stationID.all);
					memcpy(stRadioMgr.stTxMsg.data+5, sisData.shortName.text, sisData.shortName.len);
					stRadioMgr.stTxMsg.data[13] = sisData.shortName.len;
					memcpy(stRadioMgr.stTxMsg.data+14, sisData.universalName.text, sisData.universalName.len);
					stRadioMgr.stTxMsg.data[27] = sisData.universalName.len;
					stRadioMgr.stTxMsg.data[28] = sisData.universalName.charType;
					if(sisData.universalName.appendFm) {
						stRadioMgr.stTxMsg.data[29] = (uint8)1;
					}
					else {
						stRadioMgr.stTxMsg.data[29] = (uint8)0;
					}
					memcpy(stRadioMgr.stTxMsg.data+30, sisData.slogan.text, sisData.slogan.len);
					stRadioMgr.stTxMsg.data[158] = sisData.slogan.len;
					stRadioMgr.stTxMsg.data[159] = sisData.slogan.charType;
					tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);

					SRMGR_DBG("---------------------------------------------------------\n");
					SRMGR_DBG("Station Information Service Data: HDR ID[%d]\n", arg[0]);

					SRMGR_DBG("Station ID[%04xh]\n", sisData.stationID.all);
					SRMGR_DBG("Short Name Length[%d]\n", sisData.shortName.len);
					SRMGR_DBG("Short Name : ");
					if(sisData.shortName.len > 0) {
						for(i=0; i<sisData.shortName.len; i++) {
							printf("%c", sisData.shortName.text[i]);
						}
					}
					printf("\n");

					SRMGR_DBG("Universal Name Length[%d], charType[%d], appendFm[%d]\n", sisData.universalName.len, sisData.universalName.charType, sisData.universalName.appendFm);
					SRMGR_DBG("Universal Name : ");
					if(sisData.universalName.len > 0) {
						for(i=0; i<sisData.universalName.len; i++) {
							printf("%c", sisData.universalName.text[i]);
						}
					}
					printf("\n");

					SRMGR_DBG("Slogan Length[%d], charType[%d]\n", sisData.slogan.len, sisData.slogan.charType);
					SRMGR_DBG("Slogan : ");
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
				SRMGR_ERR("eRADIO_HD_NOTIFY_SIGNAL_STATUS : Error[%d]!\n", errorCode);
			}
			else {
				stTC_HDR_SIGNAL_STATUS_t hdrSigStatus;
				memcpy((void*)&hdrSigStatus, *(pData+0), sizeof(stTC_HDR_SIGNAL_STATUS_t));

				stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+6;
				stRadioMgr.stTxMsg.upperCmd = (uint16)SUB_NOTI_UCMD;
				stRadioMgr.stTxMsg.lowerCmd = (uint16)eRMGR_IPC_NOTI_HdrSignalStatus;
				stRadioMgr.stTxMsg.data[0] = (uint8)hdrSigStatus.hdrID;
				stRadioMgr.stTxMsg.data[1] = (uint8)hdrSigStatus.curPN;
				stRadioMgr.stTxMsg.data[2] = (uint8)hdrSigStatus.acqStatus;
				stRadioMgr.stTxMsg.data[3] = (uint8)hdrSigStatus.cnr;
				stRadioMgr.stTxMsg.data[4] = (uint8)hdrSigStatus.pmap;
				stRadioMgr.stTxMsg.data[5] = (uint8)hdrSigStatus.hybridProgram;
				tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);

				SRMGR_DBG("eRADIO_HD_NOTIFY_SIGNAL_STATUS :  ALL[%02xh], HDSIG[%d], HDAUD[%d], SIS[%d], SISOK[%d] Done!\n",
					hdrSigStatus.acqStatus, hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_HD_SIGNAL, hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_HD_AUDIO,
					hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_SIS, hdrSigStatus.acqStatus&eBITMASK_SIGNAL_STATUS_SIS_OK);
			}
			break;

		case eRADIO_HD_NOTIFY_PTY:
			if(errorCode != eRET_OK) {
				SRMGR_ERR("eRADIO_HD_NOTIFY_PTY : Error[%d]!\n", errorCode);
			}
			else {
				stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+3;
				stRadioMgr.stTxMsg.upperCmd = (uint16)SUB_NOTI_UCMD;
				stRadioMgr.stTxMsg.lowerCmd = (uint16)eRMGR_IPC_NOTI_HdrPTY;
				stRadioMgr.stTxMsg.data[0] = (uint8)arg[0];
				stRadioMgr.stTxMsg.data[1] = (uint8)arg[1];
				stRadioMgr.stTxMsg.data[2] = (uint8)arg[2];
				tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);
				SRMGR_DBG("eRADIO_HD_NOTIFY_PTY : ID[%d], Current ProgNum[%d], PTY[%d]\n", arg[0], arg[1], arg[2]);
			}
			break;
#endif	// #ifdef USE_HDRADIO

		default:
			break;
	}
}

static void tcsubrmgr_getStationListCallBack(uint32 totalnum, void *list, int32 errorCode)
{
	uint32 i;
	stRADIO_LIST_t *StationList = (stRADIO_LIST_t *)list;

	SRMGR_DBG("====== Total List Num[%d], List Addr[%p], errorCode[%d] =====\n", totalnum, list, errorCode);
	if(totalnum > 0) {
		for(i=0; i<totalnum; i++) {
			SRMGR_DBG("%d Station : %d Khz\n", i+1, (StationList+i)->uiFreq);
			stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+12;
			if((StationList+i)->stQdata.type == eRADIO_TYPE1)
			{
				stRADIO_TYPE1_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)((StationList+i)->stQdata.qual.fm.Qvalue), sizeof(qvalue));

				if(stRadioMgr.curBand == eRMGR_FM_BAND) {	// FM
					SRMGR_DBG("RSSI = %d\n", qvalue.fm.Rssi);
					SRMGR_DBG("SNR = %d\n", qvalue.fm.Snr);
					SRMGR_DBG("Deviation = %d\n", qvalue.fm.Dev);
					SRMGR_DBG("Offset = %d\n", qvalue.fm.Offs);
					SRMGR_DBG("Pilot = %d\n", qvalue.fm.Pilot);
					SRMGR_DBG("MultiPath = %d\n", qvalue.fm.Mpth);
					SRMGR_DBG("USN = %d\n", qvalue.fm.Usn);

					stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+26;
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+12, (uint16)qvalue.fm.Rssi);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+14, (uint16)qvalue.fm.Snr);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+16, (uint16)qvalue.fm.Dev);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+18, (uint16)qvalue.fm.Offs);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+20, (uint16)qvalue.fm.Pilot);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+22, (uint16)qvalue.fm.Mpth);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+24, (uint16)qvalue.fm.Usn);
				}
				else if(stRadioMgr.curBand == eRMGR_MW_BAND) {	// MW
					SRMGR_DBG("RSSI = %d\n", qvalue.am.Rssi);
					SRMGR_DBG("SNR = %d\n", qvalue.am.Snr);
					SRMGR_DBG("Modulation = %d\n", qvalue.am.Mod);
					SRMGR_DBG("Offset = %d\n", qvalue.am.Offs);

					stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+20;
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+12, (uint16)qvalue.am.Rssi);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+14, (uint16)qvalue.am.Snr);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+16, (uint16)qvalue.am.Mod);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+18, (uint16)qvalue.am.Offs);
				}
				else {
					;	// Other user bands can be set.
				}
			}
			else if((StationList+i)->stQdata.type == eRADIO_TYPE2)
			{
				stRADIO_TYPE2_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)((StationList+i)->stQdata.qual.fm.Qvalue), sizeof(qvalue));

				if(stRadioMgr.curBand == eRMGR_FM_BAND) {	// FM
					SRMGR_DBG("Status = %xh\n", qvalue.fm.Status);
					SRMGR_DBG("RSSI = %d\n", qvalue.fm.Rssi);
					SRMGR_DBG("USN = %d\n", qvalue.fm.Usn);
					SRMGR_DBG("MultiPath = %d\n", qvalue.fm.Mpth);
					SRMGR_DBG("Offset = %d\n", qvalue.fm.Offs);
					SRMGR_DBG("Bandwidth = %d\n", qvalue.fm.Bwth);
					SRMGR_DBG("Modulation = %d\n", qvalue.fm.Mod);

					stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+26;
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+12, (uint16)qvalue.fm.Status);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+14, (uint16)qvalue.fm.Rssi);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+16, (uint16)qvalue.fm.Usn);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+18, (uint16)qvalue.fm.Mpth);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+20, (uint16)qvalue.fm.Offs);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+22, (uint16)qvalue.fm.Bwth);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+24, (uint16)qvalue.fm.Mod);
				}
				else if(stRadioMgr.curBand == eRMGR_MW_BAND) {	// MW
					SRMGR_DBG("Status = %xh\n", qvalue.am.Status);
					SRMGR_DBG("RSSI = %d\n", qvalue.am.Rssi);
					SRMGR_DBG("High Frequency Noise Detector = %d\n", qvalue.am.Hfn);
					SRMGR_DBG("Co-Channel Detector = %d\n", qvalue.am.Coch);
					SRMGR_DBG("Offset = %d\n", qvalue.am.Offs);
					SRMGR_DBG("Bandwidth = %d\n", qvalue.am.Bwth);
					SRMGR_DBG("Modulation = %d\n", qvalue.am.Mod);

					stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+26;
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+12, (uint16)qvalue.am.Status);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+14, (uint16)qvalue.am.Rssi);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+16, (uint16)qvalue.am.Hfn);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+18, (uint16)qvalue.am.Coch);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+20, (uint16)qvalue.am.Offs);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+22, (uint16)qvalue.am.Bwth);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+24, (uint16)qvalue.am.Mod);
				}
				else {
					;	// Other user bands can be set.
				}
			}
			else {
				stRADIO_TYPE0_QUALITY_t qvalue;
				memcpy((void *)&qvalue, (void *)((StationList+i)->stQdata.qual.fm.Qvalue), sizeof(qvalue));

				if(stRadioMgr.curBand == eRMGR_FM_BAND) {	// FM
					SRMGR_DBG("RSSI = %d\n", qvalue.fm.Rssi);
					SRMGR_DBG("Modulation = %d\n", qvalue.fm.Mod);
					SRMGR_DBG("Offset = %d\n", qvalue.fm.Offs);
					SRMGR_DBG("HFN = %d\n", qvalue.fm.Hfn);
					SRMGR_DBG("MultiPath = %d\n", qvalue.fm.Mpth);
					SRMGR_DBG("Pilot = %d\n", qvalue.fm.Pilot);

					stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+24;
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+12, (uint16)qvalue.fm.Rssi);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+14, (uint16)qvalue.fm.Mod);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+16, (uint16)qvalue.fm.Offs);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+18, (uint16)qvalue.fm.Hfn);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+20, (uint16)qvalue.fm.Mpth);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+22, (uint16)qvalue.fm.Pilot);
				}
				else if(stRadioMgr.curBand == eRMGR_MW_BAND) {	// MW
					SRMGR_DBG("RSSI = %d\n", qvalue.am.Rssi);
					SRMGR_DBG("Modulation = %d\n", qvalue.am.Mod);
					SRMGR_DBG("Offset = %d\n", qvalue.am.Offs);

					stRadioMgr.stTxMsg.length = (uint16)IPC_MIN_LEN+18;
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+12, (uint16)qvalue.am.Rssi);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+14, (uint16)qvalue.am.Mod);
					tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+16, (uint16)qvalue.am.Offs);
				}
				else {
					;	// Other user bands can be set.
				}
			}

			stRadioMgr.stTxMsg.upperCmd = (uint16)SUB_RESP_UCMD;
			stRadioMgr.stTxMsg.lowerCmd = (uint16)eRMGR_IPC_RESP_SetScanStationListResult;
			tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data, errorCode);
			tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+2, (uint16)totalnum);
			tcsubrmgr_16bto8b(stRadioMgr.stTxMsg.data+4, (uint16)i);
			stRadioMgr.stTxMsg.data[6] = (uint8)stRadioMgr.curBand;
			tcsubrmgr_32bto8b(stRadioMgr.stTxMsg.data+7, (StationList+i)->uiFreq);
			stRadioMgr.stTxMsg.data[11] = (uint8)(StationList+i)->stQdata.type;
			tcsubrmgr_setSdrIpcMessage(stRadioMgr.stTxMsg);
		}
	}
	else {
		SRMGR_INF("Not available station list!!!");
	}
}

static int32 tcsubrmgr_checkCRC16(uint8 *rxbuf)
{
	int32 crc_error = 0;
	uint16 crc_val;
	uint16 crc = 0xffff;
	uint8 data;
	uint32 crc_index;
	uint32 i;

	crc_index = 2 + (((uint32)rxbuf[2]<<8) | (uint32)rxbuf[3]);

	crc_val = ((uint16)rxbuf[crc_index] << 8) | (uint16)rxbuf[crc_index+1];

	for(i=2; i < crc_index; i++)
	{
		data = rxbuf[i];
		crc = (crc << 8) ^ crclut[(crc >> 8) ^ data];
	}
	crc = ~crc;

	if( crc != crc_val) {
		crc_error = -1;
	}
	return crc_error;
}

static uint16 tcsubrmgr_makeCRC16(uint8 *rxbuf)
{
	uint16 crc = 0xffff;
	uint8 data;
	uint32 crc_index;
	uint32 i;

	crc_index = 2 + (((uint32)rxbuf[2]<<8) | (uint32)rxbuf[3]);

	for(i=2; i < crc_index; i++)
	{
		data = rxbuf[i];
		crc = (crc << 8) ^ crclut[(crc >> 8) ^ data];
	}
	crc = ~crc;

	return crc;
}

int32 main(int argc, char* argv[])
{
	int32 ret = 0;
	int32 i;

	tcsubrmgr_initValiable();
	for(i=0; i<10; i++)
	{
		// if not ok, retry 10 times
		ret = dev_ipcSdrSubOpen();
		if(ret == 0) {
			stRadioMgr.fRunning = 1;
			break;
		}
		usleep(200*1000);	// interval 200ms
	}

	if(ret != 0) {
		SRMGR_ERR("[%s:%d]: Failed to open tcc_sdr_ipc device!!!\n", __func__, __LINE__);
	}
	else {
		SRMGR_INF("Start Sub-Core HD Radio Manager...\n");
	}

	while(stRadioMgr.fRunning)
	{
		tcsubrmgr_getSdrIpcMessage(&stRadioMgr.stRxMsg);
		if(stRadioMgr.stRxMsg.fNewMsg == IPC_SDR_SIGNATURE) {
			tcsubrmgr_eventHandler(stRadioMgr.stRxMsg, &stRadioMgr.stTxMsg);
		}
	}

	dev_ipcSdrSubClose();
	stRadioMgr.fRunning = -1;

	return ret;
}

