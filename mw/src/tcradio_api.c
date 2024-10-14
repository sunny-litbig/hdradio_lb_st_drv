/*******************************************************************************

*   FileName : tcradio_api.c

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
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "tcradio_types.h"
#include "tcradio_memory.h"
#include "tcradio_api.h"
#include "tcradio_msgq.h"
#include "tcradio_utils.h"
#include "tcradio_peri_config.h"
#include "tcradio_hal.h"
#include "tcradio_hal_config.h"
#include "tcradio_service.h"
#include "tcradio_sound.h"
#include "tcradio_rds_api.h"
#include "tcradio_rds_def.h"
#include "tcradio_bg.h"

#define __GET_DIV_4BYTE(X)		(int32)(((X)[3] << 24) | ((X)[2] << 16) | ((X)[1] << 8) | (X)[0])
#define __GET_DIV_U4BYTE(X)		(uint32)(((X)[3] << 24) | ((X)[2] << 16) | ((X)[1] << 8) | (X)[0])
#define __GET_DIV_2BYTE(X)		(int16)(((int16)(X)[1] << 8) | ((int16)(X)[0]&0x00ff))

#define __TOTAL_BUF_SIZE__		(1024*1024)
#define __READ_UNIT_SIZE__ 		(__TOTAL_BUF_SIZE__/8)

static uint32 fradioInit=0;
/*
init/deinit	: Init or Deinit Radio Service/Sound/RDS/HAL Thread, Message Queue and Mutex.
open/close	: Open or Close Radio Service/Sound/RDS/HAL(Tuner IC Initial)
*/
void setRadioInitStatus(uint32 sts)
{
	fradioInit = sts;
}

static int32 getRadioInitStatus(void)
{
	int32 ret = 0;
	if(fradioInit)
		ret = 1;
	return fradioInit;
}

static RET checkRadioInitStatus(void)
{
	RET ret = eRET_OK;
	if(getRadioInitStatus() == (int32)NULL) {
		ret = eRET_NG_NOT_INIT;
		RSRV_ERR("[%s:%d] Not initialize radio!!!\n", __func__, __LINE__);
	}
	return ret;
}

static int32 getRadioOpenStatus(void)
{
	int32 ret = 0;
	if(stRadioService.fRadioOpened)
		ret = 1;
	return ret;
}

static RET checkRadioOpenStatus(void)
{
	RET ret = checkRadioInitStatus();
	if(ret == eRET_OK) {
		if(getRadioOpenStatus() == (int32)NULL) {
			ret = eRET_NG_NOT_OPEN;
			RSRV_ERR("[%s:%d] Not open radio!!!\n", __func__, __LINE__);
		}
		else {
			ret = eRET_OK;
		}
	}
	return ret;
}

RET tcradio_init(void)
{
	RET ret = eRET_OK;

    initLogging();

	if(getRadioInitStatus() != 0) {
		ret = eRET_NG_ALREADY_INIT;
		return ret;
	}

	ret= tcradiohal_init();

	if(ret == eRET_OK) {
		ret = tcradioservice_init();
	}

	if(ret == eRET_OK) {
		ret = tcrds_init();
	}

	if(ret == eRET_OK) {
		ret = tcradiosound_init();
	}

	if(ret == eRET_OK) {
		setRadioInitStatus(1);
	}
	else {
		setRadioInitStatus(0);
		RSRV_ERR("[%s:%d] Failed to init TcRadio middleware!!!\n", __func__, __LINE__);
	}

	if(ret == eRET_OK) {
		ret = tcradiobg_init();
	}

	return ret;
}

RET tcradio_deinit(void)
{
	RET ret = checkRadioInitStatus();
	if(ret == eRET_OK) {
		ret = tcradioservice_sendMessage(eSENDER_ID_APP, eRADIO_CMD_DEINIT, pNULL, pNULL, (RET)NULL);
		setRadioInitStatus(0);
	}

	if(ret == eRET_OK) {
		ret = tcradiobg_sendMessage(eSENDER_ID_APP, eRADIO_BG_CMD_DEINIT, pNULL, pNULL, (RET)NULL);
	}

	return ret;
}

RET tcradio_open(stRADIO_CONFIG_t *config)
{
	RET ret = checkRadioInitStatus();
	uint32 uiBuf[MSGQ_DATA_LENGTH]={0,};
	if(ret == eRET_OK) {
		if(getRadioOpenStatus() == (int32)NULL) {
			tcradioservice_mutexLock();
			uiBuf[0] = (uint32)(config->area);
			uiBuf[1] = (uint32)(config->initMode);
			uiBuf[2] = (uint32)(config->initFreq);
			uiBuf[3] = (uint32)(config->numTuners);
			uiBuf[4] = (uint32)(config->fPhaseDiversity);
			uiBuf[5] = (uint32)(config->fIqOut);
			uiBuf[6] = (uint32)(config->audioSamplerate);
			uiBuf[7] = (uint32)(config->fExtAppCtrl);
			uiBuf[8] = (uint32)(config->sdr);
			uiBuf[9] = (uint32)(config->hdType);
			ret = tcradioservice_sendMessageWithoutMutex(eSENDER_ID_APP, eRADIO_CMD_OPEN, uiBuf, pNULL, (RET)NULL);
			tcradioservice_mutexUnlock();
		}
		else {
			ret = eRET_NG_ALREADY_OPEN;
			RSRV_WRN("[%s:%d] Already open radio\n", __func__, __LINE__);
		}
	}

#if 1
	if(ret == eRET_OK) {
		tcradiobg_mutexLock();
		ret = tcradiobg_sendMessageWithoutMutex(eSENDER_ID_APP, eRADIO_BG_CMD_OPEN, uiBuf, pNULL, (RET)NULL);
		tcradiobg_mutexUnlock();
	}
#endif
	return ret;
}

RET tcradio_close(void)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		ret = tcradioservice_close();
	}

	if(ret == eRET_OK) {
		ret = tcradiobg_close();
	}

	return ret;
}

RET tcradio_getOpenStatus(void)
{
	return checkRadioOpenStatus();
}

RET tcradio_setBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 start_freq, uint32 end_freq, uint32 step)
{
	RET ret = checkRadioInitStatus();
	if(ret == eRET_OK) {
		tcradioservice_mutexLock();
		ret = tcradioservice_setBandFreqConfig(mod_mode, start_freq, end_freq, step);
		tcradioservice_mutexUnlock();
	}
	return ret;
}

RET tcradio_getMaxDabFreqList(uint8 *maxNums)
{
	RET ret = checkRadioInitStatus();
	if(ret == eRET_OK) {
		ret = tcradiohal_getMaxDabFreqList(maxNums);
	}
	return ret;
}

RET tcradio_getCurrentDabFreqList(uint8 *curNums, uint32 ntuner)
{
	RET ret = checkRadioInitStatus();
	if(ret == eRET_OK) {
		ret = tcradiohal_getCurrentDabFreqList(curNums, ntuner);
	}
	return ret;
}

RET tcradio_setDabFreqList(uint32 *dab_freq_hz_table, uint8 num_freq, uint32 ntuner)
{
	RET ret = checkRadioInitStatus();
	uint32 uiBuf[MSGQ_DATA_LENGTH]={0,};
	void *pData[MSGQ_PDATA_LENGTH]={pNULL, };
	if(ret == eRET_OK) {
		tcradioservice_mutexLock();
		uiBuf[0] = num_freq;
		uiBuf[1] = ntuner;
		pData[0] = dab_freq_hz_table;
		ret = tcradioservice_sendMessageWithoutMutex(eSENDER_ID_APP, eRADIO_CMD_SET_DAB_FREQ_LIST, uiBuf, pData, (RET)NULL);
		tcradioservice_mutexUnlock();
	}
	return ret;
}

RET tcradio_getDabFreqList(uint32 *dab_freq_hz_table, uint8 *num_freq, uint32 ntuner)
{
	RET ret = checkRadioInitStatus();
	if(ret == eRET_OK) {
		ret = tcradiohal_getDabFreqList(dab_freq_hz_table, num_freq, ntuner);
	}
	return ret;
}

RET tcradio_setTune(eRADIO_MOD_MODE_t mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner)
{
	RET ret = checkRadioOpenStatus();
	uint32 uiBuf[MSGQ_DATA_LENGTH]={0,};
	if(ret == eRET_OK) {
		tcradioservice_mutexLock();
		if(ntuner < tcradioservice_getNumberOfTuners()) {
			ret = tcradioservice_checkValidFreq(mod_mode, freq);
			if(ret == eRET_OK) {
				uiBuf[0] = mod_mode;
				uiBuf[1] = freq;
				uiBuf[2] = tune_mode;
				uiBuf[3] = ntuner;
				ret = tcradioservice_sendMessageWithoutMutex(eSENDER_ID_APP, eRADIO_CMD_SET_TUNE, uiBuf, pNULL, (RET)NULL);
			}
		}
		else {
			ret = eRET_NG_INVALID_PARAM;
		}
		tcradioservice_mutexUnlock();
	}
	return ret;
}

RET tcradio_setSeek(eRADIO_SEEK_MODE_t seekcmd, uint32 *data)
{
	RET ret = checkRadioOpenStatus();
	uint32 uiBuf[MSGQ_DATA_LENGTH]={0,};
	if(ret == eRET_OK) {
		tcradioservice_mutexLock();
		uiBuf[0] = (uint32)seekcmd;
		if(data != NULL) {
			tcradio_memcpy(scanPiList, data, 128);
		}
		if(seekcmd < eRADIO_SEEK_END) {
			ret = tcradioservice_sendMessageWithoutMutex(eSENDER_ID_APP, eRADIO_CMD_SET_SEEK, uiBuf, pNULL, (RET)NULL);
		}
		else {
			ret = eRET_NG_INVALID_PARAM;
		}
		tcradioservice_mutexUnlock();
	}
	return ret;
}

RET tcradio_bgStart(eRADIO_MOD_MODE_t mod_mode)
{
	RET ret = checkRadioOpenStatus();
	uint32 uiBuf[MSGQ_DATA_LENGTH]={0,};
	if(ret == eRET_OK) {
		tcradiobg_mutexLock();
		uiBuf[0] = (uint32)eRADIO_SEEK_SCAN_STATION;
		uiBuf[1] = (uint32)mod_mode;
		ret = tcradiobg_sendMessageWithoutMutex(eSENDER_ID_APP, eRADIO_BG_CMD_START, uiBuf, pNULL, (RET)NULL);
		tcradiobg_mutexUnlock();
	}
	return ret;
}

RET tcradio_bgStop(void)
{
	RET ret = checkRadioOpenStatus();
	uint32 uiBuf[MSGQ_DATA_LENGTH]={0,};
	if(ret == eRET_OK) {
		tcradiobg_mutexLock();
		uiBuf[0] = (uint32)eRADIO_SEEK_STOP;
		ret = tcradiobg_sendMessageWithoutMutex(eSENDER_ID_APP, eRADIO_BG_CMD_STOP, uiBuf, pNULL, (RET)NULL);
		tcradiobg_mutexUnlock();
	}
	return ret;
}

RET tcradio_setTunerCommand(uint8 *txdata, uint32 txlen, uint8 *rxdata, uint32 rxlen, uint32 ntuner)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
	#if 0
		if(ntuner < tcradioservice_getNumberOfTuners()) {
			ret = tcradiohal_setTunerCommand(txdata, txlen, ntuner);
		}
		else {
			ret = eRET_NG_INVALID_PARAM;
		}
	#else
		ret = tcradiohal_setTunerCommand(txdata, txlen, rxdata, rxlen, ntuner);
	#endif
	}
	return ret;
}

RET tcradio_getTunerStatus(uint32 cmd, uint8 *buf, uint32 len, uint32 ntuner)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		ret = tcradiohal_getTunerStatus(cmd, buf, len, ntuner);
	}
	return ret;
}

RET tcradio_setIQTestPattern(uint32 fOnOff, uint32 sel)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		ret = tcradiohal_setIQTestPattern(fOnOff, sel);
	}
	return ret;
}

RET tcradio_setAudio(uint32 play)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		ret = tcradiosound_setAudio(play);
	}
	return ret;
}

RET tcradio_setAudioDevice(stRADIO_CONFIG_t *config, uint32 OnOff)
{
	RET ret;

	if(OnOff == 0) {
		ret = tcradiosound_close();
	}
	else {
		ret = checkRadioOpenStatus();
		if(ret == eRET_OK) {
#ifdef USE_HDRADIO
			if(config->sdr == eRADIO_SDR_HD) {
				ret = tcradiosound_open(config->audioSamplerate, eSOUND_SOURCE_SDR);
			}
			else {
				ret = tcradiosound_open(config->audioSamplerate, eSOUND_SOURCE_TC_I2S);
			}
#else
			ret = tcradiosound_open(config->audioSamplerate, eSOUND_SOURCE_TC_I2S);
#endif
		}
	}

	return ret;
}

RET tcradio_getTune(eRADIO_MOD_MODE_t *mod_mode, uint32 *freq, uint32 ntuner)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		if(ntuner < tcradioservice_getNumberOfTuners() || ntuner == 1) {
			ret = tcradiohal_getTune(mod_mode, freq, ntuner);
		}
		else {
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	return ret;
}

RET tcradio_getQuality(eRADIO_MOD_MODE_t mod_mode, stRADIO_QUALITY_t * qdata, uint32 ntuner)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		if(ntuner < tcradioservice_getNumberOfTuners() || ntuner == 1) {
			ret = tcradiohal_getQuality(mod_mode, (stTUNER_QUALITY_t *)qdata, ntuner);
		}
		else {
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	return ret;
}

RET tcradio_getBandFreqConfig(eRADIO_MOD_MODE_t mod_mode, uint32 *start_freq, uint32 *end_freq, uint32 *step)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		tcradioservice_mutexLock();
		ret = tcradioservice_getBandFreqConfig(mod_mode, start_freq, end_freq, step);
		tcradioservice_mutexUnlock();
	}
	return ret;
}

RET tcradio_spi_writeread(uint8 *tx, uint8 *rx, uint32 len, uint32 cs)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		ret = tcradiohal_spi_writeRead(tx, rx, len, cs);
	}
	return ret;
}

RET tcradio_i2c_write(uint8 addr, uint8 *tx, uint32 len)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		ret = tcradiohal_i2c_write(addr, tx, len);
	}
	return ret;
}

RET tcradio_i2c_read(uint8 addr, uint8 *reg, uint32 reglen, uint8* rx, uint32 rxlen)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		ret = tcradiohal_i2c_read(addr, reg, reglen, rx, rxlen);
	}
	return ret;
}

uint32 tcradio_getMiddlewareVersion(void)
{
	uint32 ret = 0;
	if(checkRadioInitStatus() == eRET_OK) {
		ret = (((uint32)__RADIO_MIDDLEWARE_VER_PREFIX__<<24) | ((uint32)__RADIO_MIDDLEWARE_VER_RELEASE_NUMBER__<<16) |
				((uint32)__RADIO_MIDDLEWARE_VER_MAJOR_NUMBER__<<8) | ((uint32)__RADIO_MIDDLEWARE_VER_MINOR_NUMBER__<<0));
	}
	return ret;
}

uint32 tcradio_getSdrDemodVersion(uint32 demod)
{
	uint32 ret = 0;
	if(checkRadioInitStatus() == eRET_OK) {
		if(demod == 0) {		// FM/AM SDR
			ret = 2;
		}
		else if(demod ==1) {	// HD-Radio SDR
			ret = 2;
		}
	}
	return ret;
}

uint32 tcradio_getDriverVersion(void)
{
	uint32 ret = 0;
	if(checkRadioInitStatus() == eRET_OK) {
		ret = tcradiohal_getTunerDrvVersion();
	}
	return ret;
}

uint32 tcradio_getHalVersion(void)
{
	uint32 ret = 0;
	if(checkRadioInitStatus() == eRET_OK) {
		ret = tcradiohal_getHalVersion();
	}
	return ret;
}

int32 tcradio_getTunerChip(void)
{
	int32 ret = checkRadioInitStatus();
	if(ret == eRET_OK) {
		ret = tcradiohal_getTunerChip();
	}
	return ret;
}

RET tcradio_setRdsEnable(uint32 onoff)
{
	RET ret = checkRadioOpenStatus();
	uint32 uiBuf[MSGQ_DATA_LENGTH]={0,};
	if(ret == eRET_OK) {
		if(onoff == OFF) {
			tcrds_close(eRADIO_ID_PRIMARY);
		}
		else {
			tcrds_open(eRADIO_ID_PRIMARY);
		}
	}
	return ret;
}

int32 tcradio_getRdsEnable(void)
{
	int32 ret = 0;
	if(checkRadioOpenStatus() == eRET_OK) {
		ret = tcrds_getEnable();
	}
	return ret;
}

RET tcradio_getRdsPi(uint16 *pi)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		if(tcrds_getEnable()) {
			*pi = tcrds_getPi();
		}
		else {
			ret = eRET_NG_NOT_ENABLE;
		}
	}
	return ret;
}

RET tcradio_getRdsPty(uint8 *pty)
{
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		if(tcrds_getEnable()) {
			*pty = tcrds_getPty();
		}
		else {
			ret = eRET_NG_NOT_ENABLE;
		}
	}
	return ret;
}

RET tcradio_getRdsPsn(uint8 *psn)
{
	int i;
	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		if(tcrds_getEnable()) {
			if(tcrds_getPsValid()) {
				for(i=0; i<MAX_PS; i++)
					*(psn+i) = tcrds_getPs(i);
			}
			else {
				ret = eRET_NG_INVALID_RESP;
			}
		}
		else {
			ret = eRET_NG_NOT_ENABLE;
		}
	}
	return ret;
}

RET tcradio_getRdsRT(uint8 *rt)
{
	int i;

	RET ret = checkRadioOpenStatus();
	if(ret == eRET_OK) {
		if(tcrds_getEnable()) {
			if(tcrds_getRTValid()) {
                tcrds_getRT(rt);
			}
			else {
				ret = eRET_NG_INVALID_RESP;
			}
		}
		else {
			ret = eRET_NG_NOT_ENABLE;
		}
	}
	return ret;
}

/********************************************************/
/*			Debugging Hidden Functions					*/
/********************************************************/
long long tcradio_getSystemTime(void)
{
 long long systime;
 struct timespec tspec;
 clock_gettime(CLOCK_MONOTONIC, &tspec);
 systime = (long long) tspec.tv_sec * 1000 + tspec.tv_nsec / 1000000;

 return systime;
}

#if 0
int32 tcradio_verifyIQ16BitTestPattern(const char *path)
{

	int32 _iqfd_, len=0, dump_size, fopened=0, errcnt=0, i, dump_offset, total_offset;
	int8 *dump_buf, comp_buf[4]={0,};
	uint16 cbuf[3]={0,};

	if (fopened == 0)
	{
		_iqfd_ = open(path, O_RDONLY);

		if (_iqfd_ < 0) {
			RSRV_ERR("File Open Error!!!! : fd %d\n", _iqfd_);
			return -1;
		}
		else {
			RSRV_DBG("File open() success : fd %d   ", _iqfd_);
			dump_size = lseek(_iqfd_, 0, SEEK_END);
			if (dump_size < 0) {
				RSRV_ERR("Failed to lseek()!!\n");\
				close(_iqfd_);
				return -1;
			}
			else {
				RSRV_DBG("binary file size : %d\n", dump_size);
				len = lseek(_iqfd_, 0, SEEK_SET);
				if(len < 0) {
					RSRV_ERR("Fail lseek()\n");\
					close(_iqfd_);
					return -1;
				}
				else {
					fopened = 1;
				}
			}
		}
	}

	if (fopened == 1)
	{
		dump_buf = (int8 *)malloc((sizeof(int8))*(dump_size));
		if(dump_buf != NULL) {
			len = read(_iqfd_, dump_buf, dump_size);
			if(len <= 0) {
				RSRV_ERR("Failed to read() : ret=%d\n", len);
				close(_iqfd_);
				tcradio_free(dump_buf);
				return -1;
			}
			RSRV_DBG("Success to read dump file!!! len[%d]\n", len);
			dump_offset = 0;
			total_offset = dump_size/4;
			RSRV_DBG("Verifying...\n");

			while(dump_offset < total_offset) {
			//	RSRV_DBG("%d\r", dump_offset);
				memcpy(comp_buf, dump_buf+dump_offset, 4);
				cbuf[1] = *(int16*)(dump_buf+dump_offset+2);
				cbuf[2] = *(int16*)(dump_buf+dump_offset);
			//	RSRV_DBG("offset=%d, [1]=%04xh, [2]=%04xh\n", dump_offset, cbuf[1], cbuf[2]);
				if(dump_offset != 0) {
					if(cbuf[0] == 0x7FFF) {
						if(cbuf[1] != 0x0000) {
							errcnt++;
							RSRV_ERR("\nIQ data error detected!!! cbuf[0]=%04xh, cbuf[1]=%04xh, error count[%d], offset[%d]\n", cbuf[0], cbuf[1], errcnt, dump_offset);
						}
					}
					else {
						if(cbuf[0]+1 != cbuf[1]) {
							errcnt++;
							RSRV_ERR("\nIQ data error detected!!! cbuf[0]=%04xh, cbuf[1]=%04xh, error count[%d], offset[%d]\n", cbuf[0], cbuf[1], errcnt, dump_offset);
						}
					}
				}

				if(cbuf[1] == 0x7FFF) {
					if(cbuf[2] != 0x0000) {
						errcnt++;
						RSRV_ERR("\nIQ data error detected!!! cbuf[1]=%04xh, cbuf[2]=%04xh, error count[%d], offset[%d]\n", cbuf[1], cbuf[2], errcnt, dump_offset);
					}
				}
				else {
					if(cbuf[1]+1 != cbuf[2]) {
						errcnt++;
						RSRV_ERR("\nIQ data error detected!!! cbuf[1]=%04xh, cbuf[2]=%04xh, error count[%d], offset[%d]\n", cbuf[1], cbuf[2], errcnt, dump_offset);
					}
				}

				cbuf[0] = cbuf[2];
				dump_offset += 4;
			}
		}
		else {
			RSRV_ERR("\nFailed to malloc dump buffer!!! dump_buf[%p]\n", dump_buf);
		}

		tcradio_free(dump_buf);
		close(_iqfd_);
	}

    return 0;
}

int32 tcradio_verifyIQ16BitTestPatternInXperiFormat(const char *path)
{

	int32 _iqfd_, len=0, dump_size, fopened=0, errcnt=0, i, j, dump_offset, total_offset, chunk_samples=2160, x;
	uint16 *dump_buf;
	uint16 temp_buf=0, fCheckFirstSample=0;

	if (fopened == 0)
	{
		_iqfd_ = open(path, O_RDONLY);

		if (_iqfd_ < 0) {
			RSRV_ERR("File Open Error!!!! : fd %d\n", _iqfd_);
			return -1;
		}
		else {
			RSRV_DBG("File open() success : fd %d   ", _iqfd_);
			dump_size = lseek(_iqfd_, 0, SEEK_END);
			if (dump_size < 0) {
				RSRV_ERR("Failed to lseek()!!\n");\
				close(_iqfd_);
				return -1;
			}
			else {
				RSRV_DBG("binary file size : %d\n", dump_size);
				len = lseek(_iqfd_, 0, SEEK_SET);
				if(len < 0) {
					RSRV_ERR("Fail lseek()\n");\
					close(_iqfd_);
					return -1;
				}
				else {
					fopened = 1;
				}
			}
		}
	}

	if (fopened == 1)
	{
		dump_buf = (int8 *)malloc((sizeof(int8))*(dump_size));
		if(dump_buf != NULL) {
			len = read(_iqfd_, (void*)dump_buf, dump_size);
			if(len <= 0) {
				RSRV_ERR("Failed to read() : ret=%d\n", len);
				close(_iqfd_);
				tcradio_free(dump_buf);
				return -1;
			}
			RSRV_DBG("Success to read dump file!!! len[%d]\n", len);
			dump_offset = 0;
			total_offset = dump_size/4;
			RSRV_DBG("Verifying...\n");

			for(x=0; x<2; x++) {
				fCheckFirstSample = 0;
				//x==0 Verify I, x==1 Verify Q
				for(i=x; i<total_offset/(chunk_samples*2); i+=2) {
					for(j=0; j<chunk_samples; j++) {
						unsigned int addr = (i*chunk_samples) + j;
						if(j == chunk_samples - 1) {
							temp_buf = *(dump_buf+addr);
							fCheckFirstSample = 1;
							break;
						}
						else {
							if(fCheckFirstSample) {
								if(temp_buf == 0x7ffff) {
									if(*(dump_buf+addr) != 0x0000) {
										errcnt++;
										RSRV_ERR("\ni) %s data error detected!!! buf[i]=%04xh, buf[i+1]=%04xh, addr[%d], error count[%d], offset[%d]\n", x==0 ? "I":"Q", temp_buf, *(dump_buf+addr), addr, errcnt, dump_offset);
									}
								}
								else {
									if(temp_buf+1 != *(dump_buf+addr)) {
										errcnt++;
										RSRV_ERR("\nii) %s data error detected!!! buf[i]=%04xh, buf[i+1]=%04xh, addr[%d], error count[%d], offset[%d]\n", x==0 ? "I":"Q", temp_buf, *(dump_buf+addr), addr, errcnt, dump_offset);
									}
								}
								fCheckFirstSample = 0;
							}
							else {
								if(*(dump_buf+addr) == 0x7fff) {
									if(*(dump_buf+addr+1) != 0x0000) {
										errcnt++;
										RSRV_ERR("\niii) %s data error detected!!! buf[i]=%04xh, buf[i+1]=%04xh, addr[%d], error count[%d], offset[%d]\n", x==0 ? "I":"Q", *(dump_buf+addr), *(dump_buf+addr+1), addr, errcnt, dump_offset);
									}
								}
								else {
									if(*(dump_buf+addr)+1 != *(dump_buf+addr+1)) {
										errcnt++;
										RSRV_ERR("\niv) %s data error detected!!! buf[i]=%04xh, buf[i+1]=%04xh, addr[%d], error count[%d], offset[%d]\n", x==0 ? "I":"Q", *(dump_buf+addr), *(dump_buf+addr+1), addr, errcnt, dump_offset);
									}
								}
							}
						}
					}
				}
			}
		}
		else {
			RSRV_ERR("\nFailed to malloc dump buffer!!! dump_buf[%p]\n", dump_buf);
		}

		tcradio_free(dump_buf);
		close(_iqfd_);
	}

    return 0;
}
#endif

RET tcradio_dumpIQ(uint32 nbit, uint32 bufsize_kbyte, uint32 readsize_byte, uint32 fbin, uint32 dumpsize_mbyte)
{
	RET ret = checkRadioOpenStatus();

	long long start=0, end=0;
	float res=0;

	int8 *tmpdat0=NULL,*tmpdat1=NULL,*tmpdat2=NULL, *tmpdat3=NULL;
	int8 *tmpdat4=NULL,*tmpdat5=NULL,*tmpdat6=NULL, *tmpdat7=NULL;
	int32 ret_iq01size=0, ret_iq23size=0, ret_size=0;
	uint32 gdumpFileSize, n=0, totalsize=0, progpercent;
	FILE *gfile0_IQ=NULL, *gfile1_IQ=NULL, *gfile2_IQ=NULL, *gfile3_IQ=NULL;
	FILE *gfile0_I=NULL, *gfile0_Q=NULL, *gfile1_I=NULL, *gfile1_Q=NULL, *gfile2_I=NULL, *gfile2_Q=NULL, *gfile3_I=NULL, *gfile3_Q=NULL;
	uint32 fverify = 0;
	uint32 tNumOfTuners = tcradioservice_getNumberOfTuners();

	if(ret != eRET_OK) {
		goto error_init;
	}

	if(pfnIQ01I2sOpen == NULL || pfnIQ01I2sClose == NULL || pfnIQ01I2sStart == NULL || pfnIQ01I2sStop == NULL || pfnIQ01I2sSetParams == NULL || pfnIQ01I2sRead == NULL || pfnIQ01I2sReadCh == NULL) {
		goto error_init;
	}

	if(tNumOfTuners > eRADIO_CONF_TYPE_DUAL) {
		if(pfnIQ23I2sOpen == NULL || pfnIQ23I2sClose == NULL || pfnIQ23I2sStart == NULL || pfnIQ23I2sStop == NULL || pfnIQ23I2sSetParams == NULL || pfnIQ23I2sRead == NULL || pfnIQ23I2sReadCh == NULL) {
			goto error_init;
		}
	}

//	tcradiohal_setTune(1, 530, 0, 0);	// for test // drm am agc comperation issue

	if(bufsize_kbyte >= 1024 && bufsize_kbyte <= 2048 && (nbit == 16 || nbit == 20)) {
		(*pfnIQ01I2sOpen)();
		(*pfnIQ01I2sSetParams)(4, nbit, tcradiohal_getIqSampleRate(eRADIO_ID_PRIMARY)/2, bufsize_kbyte, bufsize_kbyte*1024/64);
		(*pfnIQ01I2sStart)();
		if(tNumOfTuners > eRADIO_CONF_TYPE_DUAL) {
			(*pfnIQ23I2sOpen)();
			(*pfnIQ23I2sSetParams)(4, nbit, tcradiohal_getIqSampleRate(eRADIO_ID_TERTIARY)/2, bufsize_kbyte, bufsize_kbyte*1024/64);
			(*pfnIQ23I2sStart)();
		}


		if(readsize_byte <= 0) {
			// default dump(read) size
			readsize_byte = (bufsize_kbyte * 1024) / 8;
		}
		RSRV_DBG("\nread size = %d byte\n", readsize_byte);

		switch(tNumOfTuners) {
			case eRADIO_CONF_TYPE_QUAD:
				tmpdat6 = (int8 *)malloc((sizeof(int8))*(readsize_byte));
				tmpdat7 = (int8 *)malloc((sizeof(int8))*(readsize_byte));
				RSRV_DBG ("I/Q Dump buffer6[%p], buffer7[%p]\n", tmpdat6, tmpdat7);
			case eRADIO_CONF_TYPE_TRIPLE:
				tmpdat4 = (int8 *)malloc((sizeof(int8))*(readsize_byte));
				tmpdat5 = (int8 *)malloc((sizeof(int8))*(readsize_byte));
				RSRV_DBG ("I/Q Dump buffer4[%p], buffer5[%p]\n", tmpdat4, tmpdat5);
			case eRADIO_CONF_TYPE_DUAL:
				tmpdat2 = (int8 *)malloc((sizeof(int8))*(readsize_byte));
				tmpdat3 = (int8 *)malloc((sizeof(int8))*(readsize_byte));
				RSRV_DBG ("I/Q Dump buffer2[%p], buffer3[%p]\n", tmpdat2, tmpdat3);
			default:
				tmpdat0 = (int8 *)malloc((sizeof(int8))*(readsize_byte));
				tmpdat1 = (int8 *)malloc((sizeof(int8))*(readsize_byte));
				RSRV_DBG ("I/Q Dump buffer0[%p], buffer1[%p]\n", tmpdat0, tmpdat1);
				break;
		}

		if(tmpdat0 != NULL && tmpdat1 != NULL) {
			if(fbin == 2) {
				if(stRadioService.curBand == eRADIO_AM_MODE) {
					switch(tNumOfTuners) {
						case eRADIO_CONF_TYPE_QUAD:		gfile3_IQ = fopen ("/tmp/IQ3_AMrData_e1.bin", "w");
						case eRADIO_CONF_TYPE_TRIPLE:	gfile2_IQ = fopen ("/tmp/IQ2_AMrData_e1.bin", "w");
						case eRADIO_CONF_TYPE_DUAL:		gfile1_IQ = fopen ("/tmp/IQ1_AMrData_e1.bin", "w");
						default:						gfile0_IQ = fopen ("/tmp/IQ0_AMrData_e1.bin", "w");	break;
					}
				}
				else {
					switch(tNumOfTuners) {
						case eRADIO_CONF_TYPE_QUAD:		gfile3_IQ = fopen ("/tmp/IQ3_FMrData_e1.bin", "w");
						case eRADIO_CONF_TYPE_TRIPLE:	gfile2_IQ = fopen ("/tmp/IQ2_FMrData_e1.bin", "w");
						case eRADIO_CONF_TYPE_DUAL:		gfile1_IQ = fopen ("/tmp/IQ1_FMrData_e1.bin", "w");
						default:						gfile0_IQ = fopen ("/tmp/IQ0_FMrData_e1.bin", "w");	break;
					}
				}
			}
			else if(fbin) {
				switch(tNumOfTuners) {
					case eRADIO_CONF_TYPE_QUAD:
						gfile3_I = fopen ("/tmp/I3_data.bin", "w");
						gfile3_Q = fopen ("/tmp/Q3_data.bin", "w");
					case eRADIO_CONF_TYPE_TRIPLE:
						gfile2_I = fopen ("/tmp/I2_data.bin", "w");
						gfile2_Q = fopen ("/tmp/Q2_data.bin", "w");
					case eRADIO_CONF_TYPE_DUAL:
						gfile1_I = fopen ("/tmp/I1_data.bin", "w");
						gfile1_Q = fopen ("/tmp/Q1_data.bin", "w");
					default:
						gfile0_I = fopen ("/tmp/I0_data.bin", "w");
						gfile0_Q = fopen ("/tmp/Q0_data.bin", "w");
						break;
				}
			}
			else {
				switch(tNumOfTuners) {
					case eRADIO_CONF_TYPE_QUAD:	gfile3_IQ = fopen ("/tmp/IQ3_data.csv", "w");
					case eRADIO_CONF_TYPE_TRIPLE:	gfile2_IQ = fopen ("/tmp/IQ2_data.csv", "w");
					case eRADIO_CONF_TYPE_DUAL:	gfile1_IQ = fopen ("/tmp/IQ1_data.csv", "w");
					default:			gfile0_IQ = fopen ("/tmp/IQ0_data.csv", "w");	break;
				}
			}

			gdumpFileSize = 0;
			totalsize = dumpsize_mbyte * 1024 * 1024;

			start = tcradio_getSystemTime();
			do {
				ret_iq01size = (*pfnIQ01I2sReadCh)(tmpdat0, tmpdat1, tmpdat2, tmpdat3, readsize_byte);
				ret_size = ret_iq01size;

				if((ret_iq01size > 0) && (tNumOfTuners > eRADIO_CONF_TYPE_DUAL)) {
					ret_iq23size = (*pfnIQ23I2sReadCh)(tmpdat4, tmpdat5, tmpdat6, tmpdat7, readsize_byte);
					ret_size = ret_iq23size;
				}

				if(ret_size > 0) {
					if(fbin == 2) {
						if(nbit == 16) {
							int x;
							short temp;
							short *usTmpdat0 = (short*)tmpdat0;
							short *usTmpdat1 = (short*)tmpdat1;
							short *usTmpdat2 = (short*)tmpdat2;
							short *usTmpdat3 = (short*)tmpdat3;

							switch(tNumOfTuners) {
								case eRADIO_CONF_TYPE_QUAD:
									for(x=0; x<ret_size/2; x+=2) {
										temp = *(usTmpdat3+x+1);
										*(usTmpdat3+x+1) = *(usTmpdat3+x);
										*(usTmpdat3+x) = temp;
									}
								case eRADIO_CONF_TYPE_TRIPLE:
									for(x=0; x<ret_size/2; x+=2) {
										temp = *(usTmpdat2+x+1);
										*(usTmpdat2+x+1) = *(usTmpdat2+x);
										*(usTmpdat2+x) = temp;
									}
								case eRADIO_CONF_TYPE_DUAL:
									for(x=0; x<ret_size/2; x+=2) {
										temp = *(usTmpdat1+x+1);
										*(usTmpdat1+x+1) = *(usTmpdat1+x);
										*(usTmpdat1+x) = temp;
									}
								default:
									for(x=0; x<ret_size/2; x+=2) {
										temp = *(usTmpdat0+x+1);
										*(usTmpdat0+x+1) = *(usTmpdat0+x);
										*(usTmpdat0+x) = temp;
									}
									break;
							}
						}

						switch(tNumOfTuners) {
							case eRADIO_CONF_TYPE_QUAD:
								fwrite (tmpdat6, 1, ret_size, gfile3_IQ);
								fwrite (tmpdat7, 1, ret_size, gfile3_IQ);
							case eRADIO_CONF_TYPE_TRIPLE:
								fwrite (tmpdat4, 1, ret_size, gfile2_IQ);
								fwrite (tmpdat5, 1, ret_size, gfile2_IQ);
							case eRADIO_CONF_TYPE_DUAL:
								fwrite (tmpdat2, 1, ret_size, gfile1_IQ);
								fwrite (tmpdat3, 1, ret_size, gfile1_IQ);
							default:
								fwrite (tmpdat0, 1, ret_size, gfile0_IQ);
								fwrite (tmpdat1, 1, ret_size, gfile0_IQ);
								break;
						}
					}
					else if(fbin) {
						switch(tNumOfTuners) {
							case eRADIO_CONF_TYPE_QUAD:
								fwrite (tmpdat6, 1, ret_size, gfile3_I);
								fwrite (tmpdat7, 1, ret_size, gfile3_Q);
							case eRADIO_CONF_TYPE_TRIPLE:
								fwrite (tmpdat4, 1, ret_size, gfile2_I);
								fwrite (tmpdat5, 1, ret_size, gfile2_Q);
							case eRADIO_CONF_TYPE_DUAL:
								fwrite (tmpdat2, 1, ret_size, gfile1_I);
								fwrite (tmpdat3, 1, ret_size, gfile1_Q);
							default:
								fwrite (tmpdat0, 1, ret_size, gfile0_I);
								fwrite (tmpdat1, 1, ret_size, gfile0_Q);
								break;
						}
					}
					else {
						if(nbit == 16) {
							for(n = 0; n < readsize_byte; n+=4) {
								switch(tNumOfTuners) {
									case eRADIO_CONF_TYPE_QUAD:
										fprintf(gfile3_IQ, "%d,%d\n", *((int16*)(tmpdat6+n+2)), *((int16*)(tmpdat7+n+2)));
										fprintf(gfile3_IQ, "%d,%d\n", *((int16*)(tmpdat6+n)), *((int16*)(tmpdat7+n)));
									case eRADIO_CONF_TYPE_TRIPLE:
										fprintf(gfile2_IQ, "%d,%d\n", *((int16*)(tmpdat4+n+2)), *((int16*)(tmpdat5+n+2)));
										fprintf(gfile2_IQ, "%d,%d\n", *((int16*)(tmpdat4+n)), *((int16*)(tmpdat5+n)));
									case eRADIO_CONF_TYPE_DUAL:
										fprintf(gfile1_IQ, "%d,%d\n", *((int16*)(tmpdat2+n+2)), *((int16*)(tmpdat3+n+2)));
										fprintf(gfile1_IQ, "%d,%d\n", *((int16*)(tmpdat2+n)), *((int16*)(tmpdat3+n)));
									default:
										fprintf(gfile0_IQ, "%d,%d\n", *((int16*)(tmpdat0+n+2)), *((int16*)(tmpdat1+n+2)));
										fprintf(gfile0_IQ, "%d,%d\n", *((int16*)(tmpdat0+n)), *((int16*)(tmpdat1+n)));
										break;
								}
							}
						}
						else {
							for(n = 0; n < readsize_byte; n+=4) {
								switch(tNumOfTuners) {
									case eRADIO_CONF_TYPE_QUAD:
										fprintf(gfile3_IQ, "%d,%d\n", *((int32*)(tmpdat6+n)) >> 12, *((int32*)(tmpdat7+n)) >> 12);
									case eRADIO_CONF_TYPE_TRIPLE:
										fprintf(gfile2_IQ, "%d,%d\n", *((int32*)(tmpdat4+n)) >> 12, *((int32*)(tmpdat5+n)) >> 12);
									case eRADIO_CONF_TYPE_DUAL:
										fprintf(gfile1_IQ, "%d,%d\n", *((int32*)(tmpdat2+n)) >> 12, *((int32*)(tmpdat3+n)) >> 12);
									default:
										fprintf(gfile0_IQ, "%d,%d\n", *((int32*)(tmpdat0+n)) >> 12, *((int32*)(tmpdat1+n)) >> 12);
										break;
								}
							}
						}
					}
					gdumpFileSize += ret_size;
					RSRV_DBG("dump file size = %d\n", gdumpFileSize);
				}
				else {
					RSRV_ERR("I2S read iq01size[%d] iq23size[%d] error!!!!\n", ret_iq01size, ret_iq23size);
				}
			}while(gdumpFileSize <= totalsize);

			end = tcradio_getSystemTime();
			res = (float)(end - start)/1000;
			if(fbin && dumpsize_mbyte >= 100) {
				// Since the samplerate is incorrect due to file write time, samplerate is calculated only when dumping to a binary file.
				RSRV_DBG("\nI/Q dump time : %.3f sec, Samplerate : %.3f Khz\n", res, (float)(gdumpFileSize>>1)/res/1000);
			}

			if(fbin == 2) {
				switch(tNumOfTuners) {
					case eRADIO_CONF_TYPE_QUAD:		fclose (gfile3_IQ);
					case eRADIO_CONF_TYPE_TRIPLE:	fclose (gfile2_IQ);
					case eRADIO_CONF_TYPE_DUAL:		fclose (gfile1_IQ);
					default:						fclose (gfile0_IQ);	break;
				}
			}
			else if(fbin) {
				switch(tNumOfTuners) {
					case eRADIO_CONF_TYPE_QUAD:
						fclose (gfile3_I);
						fclose (gfile3_Q);
					case eRADIO_CONF_TYPE_TRIPLE:
						fclose (gfile2_I);
						fclose (gfile2_Q);
					case eRADIO_CONF_TYPE_DUAL:
						fclose (gfile1_I);
						fclose (gfile1_Q);
					default:
						fclose (gfile0_I);
						fclose (gfile0_Q);
						break;
				}
			}
			else {
				switch(tNumOfTuners) {
					case eRADIO_CONF_TYPE_QUAD:		fclose (gfile3_IQ);
					case eRADIO_CONF_TYPE_TRIPLE:		fclose (gfile2_IQ);
					case eRADIO_CONF_TYPE_DUAL:		fclose (gfile1_IQ);
					default:				fclose (gfile0_IQ);			break;
				}
			}

#if 0
			if(nbit == 16 && fbin == 2 && dumpsize_mbyte >= 40) {
				if(fbin == 2) {
					if(stRadioService.curBand == eRADIO_AM_MODE) {
						tcradio_verifyIQ16BitTestPatternInXperiFormat("/tmp/IQ0_AMrData_e1.bin");
					}
					else {
						tcradio_verifyIQ16BitTestPatternInXperiFormat("/tmp/IQ0_FMrData_e1.bin");
					}
				}
				else {
					tcradio_verifyIQ16BitTestPattern("/tmp/I0_data.bin");
				}
				RSRV_DBG("\nI/Q verification finished!!!\n");
			}
#endif

			RSRV_DBG ("I/Q Dump File Close success\n");
		}

		sync();

		gdumpFileSize = 0;

		(*pfnIQ01I2sStop)();
		(*pfnIQ01I2sClose)();

		if(tNumOfTuners > eRADIO_CONF_TYPE_DUAL) {
			(*pfnIQ23I2sStop)();
			(*pfnIQ23I2sClose)();
		}

		switch(tNumOfTuners) {
			case eRADIO_CONF_TYPE_QUAD:
				tcradio_free(tmpdat6);
				tcradio_free(tmpdat7);
			case eRADIO_CONF_TYPE_TRIPLE:
				tcradio_free(tmpdat4);
				tcradio_free(tmpdat5);
			case eRADIO_CONF_TYPE_DUAL:
				tcradio_free(tmpdat2);
				tcradio_free(tmpdat3);
			default:
				tcradio_free(tmpdat0);
				tcradio_free(tmpdat1);
				break;
		}
	}
	else {
		ret = eRET_NG_NOT_SUPPORT;
	}

error_init:

	return ret;
}
