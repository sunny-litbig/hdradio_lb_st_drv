/*******************************************************************************

*   FileName : si479xx_hal.c

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
*		Include 			   *
****************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "tcradio_types.h"
#include "tcradio_peri_config.h"
#include "tcradio_drv.h"
#include "si479xx_commanddefs.h"
#include "si479xx_hal.h"
#include "si479xx_core.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
uint8 firmware_buff[SI479XX_FW_IMAGE_BUFF_SIZE];
uint32 guid_table[MAX_IMAGE_COUNT];
uint32 fw_last_offset;
int32 fw_size;
uint8 file_open;
uint8 *si479xx_fw_buff;
uint32 si479xx_fw_buff_size;
int32 _fd_;
uint32 si479xx_1st_chipset_name = 0;
uint32 si479xx_2nd_chipset_name = 0;

stPOWER_UP_ARGS_t si479xx_powerup_args;

uint8 cmd_buff[CMD_BUFF_SIZE];
uint8 rsp_buff[RSQ_BUFF_SIZE];
uint8 fw_buff[1024];

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/

/***************************************************
*             Local preprocessor                   *
****************************************************/
#ifdef __ANDROID__
#define FWPATH "/vendor/etc/TccRadio/Firmware/"
#else
#define FWPATH "/usr/bin/"
#endif

#define	SI47951_ASIA_EU_BIN		(FWPATH"si47951_asia_eu_radio_sw12.bin")
#define	SI47951_NA_BIN			(FWPATH"si47951_na_radio_sw12.bin")
#define	SI47961_ASIA_EU_BIN		(FWPATH"si47961_asia_eu_radio_sw12.bin")
#define	SI47961_NA_BIN			(FWPATH"si47961_na_radio_sw12.bin")

#define	SI47951_AMFMWB_BIN_V3P0	(FWPATH"si4795x_amfmwb_sw1p2_v3p0.bin")
#define	SI47961_AMFMWB_BIN_V3P0	(FWPATH"si4796x_amfmwb_sw1p2_v3p0.bin")

#define	SI47951_AMFMDAB_BIN_V3P0 (FWPATH"si4795x_amfmdab_sw1p2_v3p0.bin")
#define	SI47961_AMFMDAB_BIN_V3P0 (FWPATH"si4796x_amfmdab_sw1p2_v3p0.bin")

#define	SI4795x_AMFMWB_BIN_V4P1	(FWPATH"si4795x_amfmwb_sw1p2p11_v4p1.bin")
#define	SI4796x_AMFMWB_BIN_V4P1	(FWPATH"si4796x_amfmwb_sw1p2p11_v4p1.bin")

#define	SI4795x_AMFMDAB_BIN_V4P1 (FWPATH"si4795x_amfmdab_sw1p2p5p11_v4p1.bin")
#define	SI4796x_AMFMDAB_BIN_V4P1 (FWPATH"si4796x_amfmdab_sw1p2p5p11_v4p1.bin")

#define	SI4795x_AMFMWB_BIN_V5P2	(FWPATH"si4795x_amfmwb_sw1p2p11_v5p2.bin")
#define	SI4796x_AMFMWB_BIN_V5P2	(FWPATH"si4796x_amfmwb_sw1p2p11_v5p2.bin")

#define	SI4795x_AMFMDAB_BIN_V5P2 (FWPATH"si4795x_amfmdab_sw1p2p5p11_v5p2.bin")
#define	SI4796x_AMFMDAB_BIN_V5P2 (FWPATH"si4796x_amfmdab_sw1p2p5p11_v5p2.bin")

#define MAX_NUM_OF_DAB_FREQ		(64)

/***************************************************
*           Local constant definitions              *
****************************************************/
static const uint32 dabFreqTable_initBandIII[] =
{
	174928000, 176640000, 178352000, 180064000,							/* 5A~ 5D */
	181936000, 183648000, 185360000, 187072000,							/* 6A~ 6D */
	188928000, 190640000, 192352000, 194064000,							/* 7A~ 7D */
	195936000, 197648000, 199360000, 201072000,							/* 8A~ 8D */
	202928000, 204640000, 206352000, 208064000,							/* 9A~ 9D */
	209936000, 211648000, 213360000, 215072000, 210096000,				/*10A~10D, 10N */
	216928000, 218640000, 220352000, 222064000, 217088000,				/*11A~11D, 11N */
	223936000, 225648000, 227360000, 229072000, 224096000,				/*12A~12D, 12N */
	230784000, 232496000, 234208000, 235776000, 237488000, 239200000	/*13A~13F */
};
static const uint32 dabFreqTableInitSize=41;
static uint32 dab01FreqTable_BandIII[MAX_NUM_OF_DAB_FREQ] = {0,};
static uint32 dab01FreqTableSize=0;
static uint32 dab23FreqTable_BandIII[MAX_NUM_OF_DAB_FREQ] = {0,};
static uint32 dab23FreqTableSize=0;

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/
RET si479xx_getPartInfo(uint32 ntuner, uint32 *name);
RET si479xx_waitForCTS(uint32 ntuner);

/***************************************************
*			function definition				*
****************************************************/
void si479xx_mssleep(uint32 cnt)
{
	usleep(cnt*1000);
}

uint32 si479xx_u8btou32b(uint8 src)
{
	uint32 ret;

	if(src & 0x80) {
		ret = src | 0xffffff00U;
	}
	else {
		ret = src & 0x000000ffU;
	}

	return ret;
}

void si479xx_16bto8b(uint8 *buf, uint16 data)
{
	*(buf+0) = (uint8)data;
	*(buf+1) = (uint8)(data >> 8);
}

void si479xx_32bto8b(uint8 *buf, uint32 data)
{
	*(buf+0) = (uint8)data;
	*(buf+1) = (uint8)(data >> 8);
	*(buf+2) = (uint8)(data >> 16);
	*(buf+3) = (uint8)(data >> 24);
}

void *si479xx_memset(void *pvdst, uint8 ubch, uint32 uilen)
{
	uint8 *pW;
	uint32 i;

	pW = (uint8 *)pvdst;

	if(uilen == 0)
		return (void *)pW;

	for(i=0; i<uilen; i++)
	{
		*pW++ = ubch;
	}
	return (void *)pW;
}

void *si479xx_memcpy(void *pvDst, void *pvSrc, uint32 uilen)
{
	uint32 i;
	uint8 *pW;
	uint8 *pR;

	pW = (uint8 *)pvDst;
	pR = (uint8 *)pvSrc;

	if(uilen == 0)
		return (void *)pW;

	for(i = 0; i < uilen; i++)
	{
		*pW++ = *pR++;
	}
	return (void *)pW;
}

void si479xx_clearCmdMemory(void)
{
    si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));
}

RET si479xx_cmdReadWrite(uint8 *tx, uint8 *rx, uint32 len, uint32 ntuner)
{
	RET ret = eRET_OK;

//	SILAB_DBG("pfnSpiTxRx Function [%p]\n", pfnSpiTxRx);

	if(pfnSpiTxRx != NULL) {
		if(ntuner <= 1) {
			ret = (*pfnSpiTxRx)(tx, rx, len, 0);
			if(ret != eRET_OK) {
				SILAB_ERR("[%s:%d]: ntuner[%d] tx/rx error ret %d\n", __func__, __LINE__, ntuner, ret);
				ret = eRET_NG_IO;
			}
		}
		else if(ntuner <= 3) {
			ret = (*pfnSpiTxRx)(tx, rx, len, 1);
			if(ret != eRET_OK) {
				SILAB_ERR("[%s:%d]: ntuner[%d] tx/rx error ret %d\n", __func__, __LINE__, ntuner, ret);
				ret = eRET_NG_IO;
			}
		}
		else {
			ret = eRET_NG_INVALID_PARAM;
		}
	}
	else {
		ret = eRET_NG_IO;
	}

	return ret;
}

void si479xx_setFirmwareImage(eFW_IMG_t type)
{
    fw_last_offset = 0;
    fw_size = 0;
    file_open = 0;
}

uint8 si479xx_getGuidTable(uint32 tunertype, uint32 ntuner)
{
	uint8 ret=0;

#if SILAB_FW_VER == 25
	switch(tunertype) {
		case eTUNER_DRV_CONF_TYPE_SINGLE:
			if(ntuner == eTUNER_DRV_ID_PRIMARY) {
				// SI47901 or SI47951
				switch(gSilabConf.area) {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[0] = 0x00000005;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801001;	// mcu.radio.eu.defaults.boot
						guid_table[2] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400001;	// swpkg01 : FM Phase Diversity
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[0] = 0x00000005;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801002;	// mcu.radio.na.defaults.boot
						guid_table[2] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400001;	// swpkg01 : FM Phase Diversity
						break;
					default:	// ASIA
						guid_table[0] = 0x00000005;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801003;	// mcu.radio.china.defaults.boot
						guid_table[2] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400001;	// swpkg01 : FM Phase Diversity
						break;
				}
				if(gSilabConf.fPhaseDiversity == 0) {
					ret = 3;
				}
				else {
					ret = 4;
				}
			}
			break;

		case eTUNER_DRV_CONF_TYPE_DUAL:
			if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY) {
				// SI47961
				switch(gSilabConf.area) {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[0] = 0x00000006;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801011;	// mcu.radio.eu.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[0] = 0x00000006;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801012;	// mcu.radio.na.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
					default:	// ASIA
						guid_table[0] = 0x00000006;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801013;	// mcu.radio.china.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
				}
				if(gSilabConf.fPhaseDiversity == 0) {
					ret = 3;
				}
				else {
					ret = 4;
				}
			}
			break;

		case eTUNER_DRV_CONF_TYPE_TRIPLE:
			if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY) {
				// SI47961
				switch(gSilabConf.area) {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[0] = 0x00000006;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801011;	// mcu.radio.eu.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[0] = 0x00000006;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801012;	// mcu.radio.na.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
					default:	// ASIA
						guid_table[0] = 0x00000006;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801013;	// mcu.radio.china.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
				}
				if(gSilabConf.fPhaseDiversity == 0) {
					ret = 3;
				}
				else {
					ret = 4;
				}
			}
			else if(ntuner == eTUNER_DRV_ID_TERTIARY) {
				// SI47901 or SI47951
				switch(gSilabConf.area) {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[0] = 0x00000005;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801001;	// mcu.radio.eu.defaults.boot
						guid_table[2] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400001;	// swpkg01 : FM Phase Diversity
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[0] = 0x00000005;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801002;	// mcu.radio.na.defaults.boot
						guid_table[2] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400001;	// swpkg01 : FM Phase Diversity
						break;
					default:	// ASIA
						guid_table[0] = 0x00000005;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801003;	// mcu.radio.china.defaults.boot
						guid_table[2] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400001;	// swpkg01 : FM Phase Diversity
						break;
				}
				if(gSilabConf.fPhaseDiversity == 0) {
					ret = 3;
				}
				else {
					ret = 4;
				}
			}
			break;

		case eTUNER_DRV_CONF_TYPE_QUAD:
			if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY || ntuner == eTUNER_DRV_ID_TERTIARY || ntuner == eTUNER_DRV_ID_QUATERNARY) {
				// SI47961
				switch(gSilabConf.area) {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[0] = 0x00000006;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801011;	// mcu.radio.eu.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[0] = 0x00000006;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801012;	// mcu.radio.na.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
					default:	// ASIA
						guid_table[0] = 0x00000006;	// mcu.radio_amfmwb.boot
						guid_table[1] = 0x00801013;	// mcu.radio.china.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
				}
				if(gSilabConf.fPhaseDiversity == 0) {
					ret = 3;
				}
				else {
					ret = 4;
				}
			}
			break;

		default:
			break;
	}

#elif SILAB_FW_VER == 30

	switch(tunertype) {
		case eTUNER_DRV_CONF_TYPE_SINGLE:
			if(ntuner == eTUNER_DRV_ID_PRIMARY) {
				// SI47901 or SI47951
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					guid_table[0] = 0x0000000b;	// mcu.radio_amfmdab.boot
				}
				else {
					guid_table[0] = 0x00000009;	// mcu.radio_amfmwb.boot
				}
				switch(gSilabConf.area) {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[1] = 0x00801001;	// mcu.radio.eu.defaults.boot
						guid_table[2] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400001;	// swpkg01 : FM Phase Diversity
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[1] = 0x00801002;	// mcu.radio.na.defaults.boot
						guid_table[2] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400001;	// swpkg01 : FM Phase Diversity
						break;
					default:	// ASIA
						guid_table[1] = 0x00801003;	// mcu.radio.china.defaults.boot
						guid_table[2] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400001;	// swpkg01 : FM Phase Diversity
						break;
				}
				if(gSilabConf.fPhaseDiversity == 0) {
					ret = 3;
				}
				else {
					ret = 4;
				}
			}
			break;

		case eTUNER_DRV_CONF_TYPE_DUAL:
			if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY) {
				// SI47961
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					guid_table[0] = 0x0000000c;	// mcu.radio_amfmdab.boot
				}
				else {
					guid_table[0] = 0x0000000a;	// mcu.radio_amfmwb.boot
				}
				switch(gSilabConf.area) {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[1] = 0x00801011;	// mcu.radio.eu.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[1] = 0x00801012;	// mcu.radio.na.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
					default:	// ASIA
						guid_table[1] = 0x00801013;	// mcu.radio.china.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
				}
				if(gSilabConf.fPhaseDiversity == 0) {
					ret = 3;
				}
				else {
					ret = 4;
				}
			}
			break;

		case eTUNER_DRV_CONF_TYPE_TRIPLE:
			if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY) {
				// SI47961
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					guid_table[0] = 0x0000000c;	// mcu.radio_amfmdab.boot
				}
				else {
					guid_table[0] = 0x0000000a;	// mcu.radio_amfmwb.boot
				}
				switch(gSilabConf.area) {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[1] = 0x00801011;	// mcu.radio.eu.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[1] = 0x00801012;	// mcu.radio.na.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
					default:	// ASIA
						guid_table[1] = 0x00801013;	// mcu.radio.china.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
				}
				if(gSilabConf.fPhaseDiversity == 0) {
					ret = 3;
				}
				else {
					ret = 4;
				}
			}
			else if(ntuner == eTUNER_DRV_ID_TERTIARY) {
				// SI47901 or SI47951
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					guid_table[0] = 0x0000000b;	// mcu.radio_amfmdab.boot
				}
				else {
					guid_table[0] = 0x00000009;	// mcu.radio_amfmwb.boot
				}
				switch(gSilabConf.area) {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[1] = 0x00801001;	// mcu.radio.eu.defaults.boot
						guid_table[2] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400001;	// swpkg01 : FM Phase Diversity
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[1] = 0x00801002;	// mcu.radio.na.defaults.boot
						guid_table[2] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400001;	// swpkg01 : FM Phase Diversity
						break;
					default:	// ASIA
						guid_table[1] = 0x00801003;	// mcu.radio.china.defaults.boot
						guid_table[2] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400001;	// swpkg01 : FM Phase Diversity
						break;
				}
				if(gSilabConf.fPhaseDiversity == 0) {
					ret = 3;
				}
				else {
					ret = 4;
				}
			}
			break;

		case eTUNER_DRV_CONF_TYPE_QUAD:
			if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY || ntuner == eTUNER_DRV_ID_TERTIARY || ntuner == eTUNER_DRV_ID_QUATERNARY) {
				// SI47961
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					guid_table[0] = 0x0000000c;	// mcu.radio_amfmdab.boot
				}
				else {
					guid_table[0] = 0x0000000a;	// mcu.radio_amfmwb.boot
				}
				switch(gSilabConf.area) {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[1] = 0x00801011;	// mcu.radio.eu.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[1] = 0x00801012;	// mcu.radio.na.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
					default:	// ASIA
						guid_table[1] = 0x00801013;	// mcu.radio.china.defaults.boot
						guid_table[2] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
						guid_table[3] = 0x00400101;	// swpkg01 : FM Phase Diversity
						break;
				}
				if(gSilabConf.fPhaseDiversity == 0) {
					ret = 3;
				}
				else {
					ret = 4;
				}
			}
			break;

		default:
			break;
	}

#elif SILAB_FW_VER == 40 || SILAB_FW_VER == 50

	switch(tunertype) {
		case eTUNER_DRV_CONF_TYPE_SINGLE:
			if(ntuner == eTUNER_DRV_ID_PRIMARY)
            {
				uint32 offset = 0;
				if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP || gSilabConf.sdr == eTUNER_SDR_DAB) {
					// SI47902 or SI47952
					guid_table[offset] = 0x0000000b;	// mcu.radio_amfmdab.boot
					offset++;
					guid_table[offset] = 0x00400011;	// swpkg11 : DRM I/Q output
					offset++;
				}
				else {
					// SI47901 or SI47951
					guid_table[offset] = 0x00000009;	// mcu.radio_amfmwb.boot
					offset++;
					guid_table[offset] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
					offset++;
				}
				switch(gSilabConf.area) {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[offset] = 0x00801001;	// mcu.radio.eu.defaults.boot
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[offset] = 0x00801002;	// mcu.radio.na.defaults.boot
						break;
					default:	// ASIA
						guid_table[offset] = 0x00801003;	// mcu.radio.china.defaults.boot
						break;
				}
				offset++;
				if(gSilabConf.fPhaseDiversity != 0) {
					guid_table[offset] = 0x00400001;		// swpkg01 : FM Phase Diversity
					offset++;
				}
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					guid_table[offset] = 0x00000011;		// hifi.advanced_tuner_amfmdab.boot
					offset++;
				#ifndef SI47952_DAB_ISSUE
					guid_table[offset] = 0x00400005;		// swpkg05 : DAB MRC/Digital Diversity/DAB Digital AGC
					offset++;
				#endif
				}
			#endif
				ret = offset;
			}
			break;

		case eTUNER_DRV_CONF_TYPE_DUAL:
			if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY)
            {
				uint32 offset = 0;
				if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP || gSilabConf.sdr == eTUNER_SDR_DAB) {
					// SI47962
					guid_table[offset] = 0x0000000c;	// mcu.radio_amfmdab.boot
					offset++;
					guid_table[offset] = 0x00400111;	// swpkg11 : DRM I/Q output
					offset++;
				}
				else {
					// SI47961
					guid_table[offset] = 0x0000000a;	// mcu.radio_amfmwb.boot
					offset++;
					guid_table[offset] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
					offset++;
				}
				switch(gSilabConf.area)
                {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[offset] = 0x00801011;	// mcu.radio.eu.defaults.boot
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[offset] = 0x00801012;	// mcu.radio.na.defaults.boot
						break;
					default:	// ASIA
						guid_table[offset] = 0x00801013;	// mcu.radio.china.defaults.boot
						break;
				}
				offset++;
				if(gSilabConf.fPhaseDiversity != 0) {
					guid_table[offset] = 0x00400101;		// swpkg01 : FM Phase Diversity
					offset++;
				}
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					guid_table[offset] = 0x00000021;		// hifi.advanced_tuner_amfmdab.boot
					offset++;
					guid_table[offset] = 0x00400105;		// swpkg05 : DAB MRC/Digital Diversity/DAB Digital AGC
					offset++;
				}
			#endif
				ret = offset;
			}
			break;

		case eTUNER_DRV_CONF_TYPE_TRIPLE:
			if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY)
            {
				uint32 offset = 0;
				if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP || gSilabConf.sdr == eTUNER_SDR_DAB) {
					// SI47962
					guid_table[offset] = 0x0000000c;	// mcu.radio_amfmdab.boot
					offset++;
					guid_table[offset] = 0x00400111;	// swpkg11 : DRM I/Q output
					offset++;
				}
				else {
					// SI47961
					guid_table[offset] = 0x0000000a;	// mcu.radio_amfmwb.boot
					offset++;
					guid_table[offset] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
					offset++;
				}
				switch(gSilabConf.area)
                {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[offset] = 0x00801011;	// mcu.radio.eu.defaults.boot
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[offset] = 0x00801012;	// mcu.radio.na.defaults.boot
						break;
					default:	// ASIA
						guid_table[offset] = 0x00801013;	// mcu.radio.china.defaults.boot
						break;
				}
				offset++;
				if(gSilabConf.fPhaseDiversity != 0) {
					guid_table[offset] = 0x00400101;		// swpkg01 : FM Phase Diversity
					offset++;
				}
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					guid_table[offset] = 0x00000021;		// hifi.advanced_tuner_amfmdab.boot
					offset++;
					guid_table[offset] = 0x00400105;		// swpkg05 : DAB MRC/Digital Diversity/DAB Digital AGC
					offset++;
				}
			#endif
				ret = offset;
			}
			else if(ntuner == eTUNER_DRV_ID_TERTIARY)
            {
				uint32 offset = 0;
				if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP || gSilabConf.sdr == eTUNER_SDR_DAB) {
					// SI47902 or SI47952
					guid_table[offset] = 0x0000000b;	// mcu.radio_amfmdab.boot
					offset++;
					guid_table[offset] = 0x00400011;	// swpkg11 : DRM I/Q output
					offset++;
				}
				else {
					// SI47901 or SI47951
					guid_table[offset] = 0x00000009;	// mcu.radio_amfmwb.boot
					offset++;
					guid_table[offset] = 0x00400002;	// swpkg02 : AM/FM HD Radio I/Q output
					offset++;
				}
				switch(gSilabConf.area) {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[offset] = 0x00801001;	// mcu.radio.eu.defaults.boot
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[offset] = 0x00801002;	// mcu.radio.na.defaults.boot
						break;
					default:	// ASIA
						guid_table[offset] = 0x00801003;	// mcu.radio.china.defaults.boot
						break;
				}
				offset++;
				if(gSilabConf.fPhaseDiversity != 0) {
					guid_table[offset] = 0x00400001;		// swpkg01 : FM Phase Diversity
					offset++;
				}
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					guid_table[offset] = 0x00000011;		// hifi.advanced_tuner_amfmdab.boot
					offset++;
				#ifndef SI47952_DAB_ISSUE
					guid_table[offset] = 0x00400005;		// swpkg05 : DAB MRC/Digital Diversity/DAB Digital AGC
					offset++;
				#endif
				}
			#endif
				ret = offset;
			}
			break;

		case eTUNER_DRV_CONF_TYPE_QUAD:
			if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY || ntuner == eTUNER_DRV_ID_TERTIARY || ntuner == eTUNER_DRV_ID_QUATERNARY)
            {
				uint32 offset = 0;
				if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP || gSilabConf.sdr == eTUNER_SDR_DAB) {
					// SI47962
					guid_table[offset] = 0x0000000c;	// mcu.radio_amfmdab.boot
					offset++;
					guid_table[offset] = 0x00400111;	// swpkg11 : DRM I/Q output
					offset++;
				}
				else {
					// SI47961
					guid_table[offset] = 0x0000000a;	// mcu.radio_amfmwb.boot
					offset++;
					guid_table[offset] = 0x00400102;	// swpkg02 : AM/FM HD Radio I/Q output
					offset++;
				}
				switch(gSilabConf.area)
                {
					case eTUNER_DRV_CONF_AREA_EU:
						guid_table[offset] = 0x00801011;	// mcu.radio.eu.defaults.boot
						break;
					case eTUNER_DRV_CONF_AREA_NA:
						guid_table[offset] = 0x00801012;	// mcu.radio.na.defaults.boot
						break;
					default:	// ASIA
						guid_table[offset] = 0x00801013;	// mcu.radio.china.defaults.boot
						break;
				}
				offset++;
				if(gSilabConf.fPhaseDiversity != 0) {
					guid_table[offset] = 0x00400101;		// swpkg01 : FM Phase Diversity
					offset++;
				}
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					guid_table[offset] = 0x00000021;		// hifi.advanced_tuner_amfmdab.boot
					offset++;
					guid_table[offset] = 0x00400105;		// swpkg05 : DAB MRC/Digital Diversity/DAB Digital AGC
					offset++;
				}
			#endif
				ret = offset;
			}
			break;

		default:
			break;
	}
#else
	#error This SILAB FW version is not availabled. (Available Version : V2p5 , V3p0, V4p0)
#endif

	return ret;
}

RET si479xx_loader_init(uint8 guid, uint32 *guidtable, uint32 ntuner)
{
    RET ret = eRET_OK;
    uint8 cmd_leng = 0;
    uint8 i;

    si479xx_clearCmdMemory();

    cmd_buff[0] = LOAD_INIT;
    cmd_buff[1] = guid;

    for (i=0; i<guid; i++)
    {
        si479xx_32bto8b(cmd_buff+4+i*4, *(guidtable+i));
    }

    cmd_leng = 4 + guid*4;

    ret = si479xx_command(cmd_leng, cmd_buff, 4, rsp_buff, ntuner);

    return ret;
}

void si479xx_allocBuffBlock(uint8 **buff, uint32 *buffer_size)
{
    *buff = firmware_buff;
	*buffer_size = 512 + 4;		 //the first 4 bytes are reserved for the command
}

RET si479xx_command_dualtuner(uint32 cmd_size, uint8 *cmd)
{
    RET ret = eRET_OK;

    if (cmd_size == 0)
        return eRET_NG_INVALID_PARAM;

    ret = si479xx_waitForCTS(eTUNER_DRV_ID_PRIMARY);
	if(ret != eRET_OK) {
		return ret;
	}
    ret = si479xx_waitForCTS(eTUNER_DRV_ID_TERTIARY);
	if(ret != eRET_OK) {
		return ret;
	}

    si479xx_cmdReadWrite(cmd, rsp_buff, cmd_size, eTUNER_DRV_ID_PRIMARY);
    si479xx_cmdReadWrite(cmd, rsp_buff, cmd_size, eTUNER_DRV_ID_TERTIARY);

    ret = si479xx_waitForCTS(eTUNER_DRV_ID_PRIMARY);
	if(ret != eRET_OK) {
		return ret;
	}
    ret = si479xx_waitForCTS(eTUNER_DRV_ID_TERTIARY);
	if(ret != eRET_OK) {
		return ret;
	}

    return ret;
}

int32 si479xx_ReadFileToBuffer(uint8 *buff, uint16 length, const char *path)
{
	int32 len=0;

	if (length == 0)
		return 0;

	if (file_open == 0)
	{
		_fd_ = open(path, O_RDONLY);

		if (_fd_ < 0) {
			SILAB_DBG("File Open Error!!!! : fd %d\n", _fd_);
			return -1;
		}
		else {
			SILAB_DBG("File open() success : fd %d\n", _fd_);
			fw_size = lseek(_fd_, 0, SEEK_END);
			if (fw_size < 0) {
				SILAB_DBG("Fail lseek()\n");\
				close(_fd_);
				return -1;
			}
			else {
			//	SILAB_DBG("binary file size : %d\n", fw_size);
				len = lseek(_fd_, 0, SEEK_SET);
			//	SILAB_DBG("lseek(fd, 0, SEEK_SET) : %d\n", len);
				if(len < 0) {
					SILAB_DBG("Fail lseek()\n");\
					close(_fd_);
					return -1;
				}
				else {
					file_open = 1;
				}
			}
		}
	}

	if (file_open == 1)
	{
		si479xx_memset(fw_buff, 0, sizeof(fw_buff));
	//	SILAB_DBG("ReadFileToBuffer : length[%d], fw_last_offset[%d]\n", length, fw_last_offset);
		len = lseek(_fd_, (off_t)fw_last_offset, SEEK_SET);
		if(len < 0) {
			SILAB_DBG("Fail lseek() : ret=%d, fw_last_offset=%d\n", len, fw_last_offset);
			close(_fd_);
			return -1;
		}
		else {
		//	SILAB_DBG("Success lseek() : ret=%d\n", len);
			if(length <= sizeof(fw_buff)) {
				len = read(_fd_, fw_buff, length);
				if(len < 0) {
					SILAB_DBG("Fail read() : ret=%d\n", len);
					close(_fd_);
					return -1;
				}
				else {
				//	SILAB_DBG("binary read length : %d\n", len);
					memcpy(buff, fw_buff, len);
					if (fw_last_offset + length >= fw_size ) {
						close(_fd_);
						SILAB_DBG("File close() success : fd %d\n", _fd_);
					}
				}
			}
			else {
				SILAB_DBG("The read length[%d] is larger than fw_buff[%d].\n", length, sizeof(fw_buff));
				close(_fd_);
				return -1;
			}
		}
	}

    return len;
}

int32 si479xx_getFirmwareSegment(uint8 *buff, uint16 length, const char *path)
{
	int32 act_data;

	if (length == 0)
		return 0;

	act_data = si479xx_ReadFileToBuffer(buff, length, path);

	return act_data;
}

const char* si79xx_getBootBinPath(eTUNER_DRV_CONF_TYPE_t tunertype, int32 ftripletuner)
{
	const char *binpath;

#if SILAB_FW_VER == 25
	if(gSilabConf.area == eTUNER_DRV_CONF_AREA_NA) {
		if(tunertype == eTUNER_DRV_CONF_TYPE_SINGLE) {
			binpath = SI47951_NA_BIN;
		}
		else if(tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE) {
			if(ftripletuner == 0) {
				binpath = SI47961_NA_BIN;
			}
			else {
				binpath = SI47951_NA_BIN;
			}
		}
		else {	// dual or quad
			binpath = SI47961_NA_BIN;
		}
    }
	else {	// asia or eu
		if(tunertype == eTUNER_DRV_CONF_TYPE_SINGLE) {
			binpath = SI47951_ASIA_EU_BIN;
		}
		else if(tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE) {
			if(ftripletuner == 0) {
				binpath = SI47961_ASIA_EU_BIN;
			}
			else {
				binpath = SI47951_ASIA_EU_BIN;
			}
		}
		else {	// dual or quad
			binpath = SI47961_ASIA_EU_BIN;
		}
	}
#elif SILAB_FW_VER == 30
	if(tunertype == eTUNER_DRV_CONF_TYPE_SINGLE) {
		if(gSilabConf.sdr == eTUNER_SDR_DAB) {
			binpath = SI47951_AMFMDAB_BIN_V3P0;
		}
		else {
			binpath = SI47951_AMFMWB_BIN_V3P0;
		}
	}
	else if(tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE) {
		if(ftripletuner == 0) {
			if(gSilabConf.sdr == eTUNER_SDR_DAB) {
				binpath = SI47961_AMFMDAB_BIN_V3P0;
			}
			else {
				binpath = SI47961_AMFMWB_BIN_V3P0;
			}
		}
		else {
			if(gSilabConf.sdr == eTUNER_SDR_DAB) {
				binpath = SI47951_AMFMDAB_BIN_V3P0;
			}
			else {
				binpath = SI47951_AMFMWB_BIN_V3P0;
			}
		}
	}
	else {	// dual or quad
		if(gSilabConf.sdr == eTUNER_SDR_DAB) {
			binpath = SI47961_AMFMDAB_BIN_V3P0;
		}
		else {
			binpath = SI47961_AMFMWB_BIN_V3P0;
		}
	}
#elif SILAB_FW_VER == 40
	if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP || gSilabConf.sdr == eTUNER_SDR_DAB) {
		if(tunertype == eTUNER_DRV_CONF_TYPE_SINGLE) {
			binpath = SI4795x_AMFMDAB_BIN_V4P1;
		}
		else if(tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE) {
			if(ftripletuner == 0) {
				binpath = SI4796x_AMFMDAB_BIN_V4P1;
			}
			else {
				binpath = SI4795x_AMFMDAB_BIN_V4P1;
			}
		}
		else {	// dual or quad
			binpath = SI4796x_AMFMDAB_BIN_V4P1;
		}
	}
	else {
		if(tunertype == eTUNER_DRV_CONF_TYPE_SINGLE) {
			binpath = SI4795x_AMFMWB_BIN_V4P1;
		}
		else if(tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE) {
			if(ftripletuner == 0) {
				binpath = SI4796x_AMFMWB_BIN_V4P1;
			}
			else {
				binpath = SI4795x_AMFMWB_BIN_V4P1;
			}
		}
		else {	// dual or quad
			binpath = SI4796x_AMFMWB_BIN_V4P1;
		}
	}
#elif SILAB_FW_VER == 50
		if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP || gSilabConf.sdr == eTUNER_SDR_DAB) {
			if(tunertype == eTUNER_DRV_CONF_TYPE_SINGLE) {
				binpath = SI4795x_AMFMDAB_BIN_V5P2;
			}
			else if(tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE) {
				if(ftripletuner == 0) {
					binpath = SI4796x_AMFMDAB_BIN_V5P2;
				}
				else {
					binpath = SI4795x_AMFMDAB_BIN_V5P2;
				}
			}
			else {	// dual or quad
				binpath = SI4796x_AMFMDAB_BIN_V5P2;
			}
		}
		else {
			if(tunertype == eTUNER_DRV_CONF_TYPE_SINGLE) {
				binpath = SI4795x_AMFMWB_BIN_V5P2;
			}
			else if(tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE) {
				if(ftripletuner == 0) {
					binpath = SI4796x_AMFMWB_BIN_V5P2;
				}
				else {
					binpath = SI4795x_AMFMWB_BIN_V5P2;
				}
			}
			else {	// dual or quad
				binpath = SI4796x_AMFMWB_BIN_V5P2;
			}
		}
#else
	#error This SILAB FW version is not availabled. (Available Version : V2p5 , V3p0, V4p0, v5p2)
#endif
	SILAB_DBG("bin path = %s\n", binpath);

	return binpath;
}

RET si479xx_host_load(eTUNER_DRV_CONF_TYPE_t tunertype, eFW_IMG_t image)
{
	RET ret = eRET_OK;
	const char *binpath;
	int32 read_length, ftriple=0;

triple_tuner_load:

    si479xx_setFirmwareImage(image);

    si479xx_allocBuffBlock(&si479xx_fw_buff, &si479xx_fw_buff_size);
//    si479xx_fw_buff_size = si479xx_fw_buff_size & 0xfffc;		// for codesonar

    si479xx_fw_buff[0] = HOST_LOAD;
    si479xx_fw_buff[1] = 0;

    //get firmware/bootloader segment
	binpath = si79xx_getBootBinPath(tunertype, ftriple);
    read_length = si479xx_getFirmwareSegment(si479xx_fw_buff+4, si479xx_fw_buff_size-4, binpath);

	if(read_length == -1) {
		 SILAB_ERR("==> Failed to load f/w from host!!!\n");
		return eRET_NG_INVALID_RESP;
	}

    while (read_length > 0)
    {
		if(read_length == -1) {
			 SILAB_ERR("==> Failed to load f/w from host because of wrong length!!!\n");
			return eRET_NG_INVALID_RESP;
		}

        si479xx_fw_buff[2] = (uint8)(read_length>>0);
        si479xx_fw_buff[3] = (uint8)(read_length>>8);

        //in fact, the read back length may not equal si479xx_fw_buff_size-4
		if (tunertype == eTUNER_DRV_CONF_TYPE_SINGLE || tunertype == eTUNER_DRV_CONF_TYPE_DUAL || tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE)
        {
			if (fw_last_offset + si479xx_fw_buff_size - 4 < fw_size) {
				if(ftriple) {
					ret = si479xx_command(read_length+4, si479xx_fw_buff, 0, NULL, eTUNER_DRV_ID_TERTIARY);
					if(ret != eRET_OK) {
						SILAB_ERR("==>[1] Failed to load f/w from host!!! ret[%d], fw_last_offset[%u], si479xx_fw_buff_size[%u], fw_size[%d]\n",
							ret, fw_last_offset, si479xx_fw_buff_size, fw_size);
						return ret;
					}
				}
				else {
					ret =si479xx_command(read_length+4, si479xx_fw_buff, 0, NULL, eTUNER_DRV_ID_PRIMARY);
					if(ret != eRET_OK) {
						SILAB_ERR("==>[2] Failed to load f/w from host!!! ret[%d], fw_last_offset[%u], si479xx_fw_buff_size[%u], fw_size[%d]\n",
							ret, fw_last_offset, si479xx_fw_buff_size, fw_size);
						return ret;
					}
				}
				fw_last_offset += si479xx_fw_buff_size-4;
			}
			else {
				read_length = fw_size - fw_last_offset;
				if(ftriple) {
					ret = si479xx_command(read_length+4, si479xx_fw_buff, 0, NULL, eTUNER_DRV_ID_TERTIARY);
					if(ret != eRET_OK) {
						SILAB_ERR("==>[3] Failed to load f/w from host!!! ret[%d], fw_last_offset[%u], si479xx_fw_buff_size[%u], fw_size[%d]\n",
							ret, fw_last_offset, si479xx_fw_buff_size, fw_size);
						return ret;
					}
				}
				else {
					ret = si479xx_command(read_length+4, si479xx_fw_buff, 0, NULL, eTUNER_DRV_ID_PRIMARY);
					if(ret != eRET_OK) {
						SILAB_ERR("==>[4] Failed to load f/w from host!!! ret[%d], fw_last_offset[%u], si479xx_fw_buff_size[%u], fw_size[%d]\n",
							ret, fw_last_offset, si479xx_fw_buff_size, fw_size);
						return ret;
					}
				}
				break;
			}
        }
        else if(tunertype == eTUNER_DRV_CONF_TYPE_QUAD)
        {

			if (fw_last_offset + si479xx_fw_buff_size - 4 < fw_size) {
				ret = si479xx_command_dualtuner(read_length+4, si479xx_fw_buff);
				if(ret != eRET_OK) {
					return ret;
				}
				fw_last_offset += si479xx_fw_buff_size-4;
			}
			else {
				read_length = fw_size - fw_last_offset;
				ret = si479xx_command_dualtuner(read_length+4, si479xx_fw_buff);
				if(ret != eRET_OK) {
					return ret;
				}
				break;
			}
        }
        read_length = si479xx_getFirmwareSegment(si479xx_fw_buff+4, si479xx_fw_buff_size-4, binpath);
    }

	if(tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE && ftriple == 0) {
		ftriple = 1;
		goto triple_tuner_load;
	}

    //boot command
    SILAB_DBG("==> si479x Host Load Success !!!!\n");
    return ret;
}

uint8 si479xx_getCommandRequestStatus(uint8 cmd)
{
	ePOWERUP_STATE_t state = POWERUP_APPLICATION;

	switch (cmd)
	{
		case POWER_UP:
			state = POWERUP_RESET;
			break;

		case GET_PART_INFO:
		case HOST_LOAD:
		case LOAD_INIT:
		case LOAD_CONFIG:
			state = POWERUP_BOOTLOADER;
			break;

		case BOOT:
			state = POWERUP_BOOTLOADER | POWERUP_BOOTREADY;
			break;

		case FLASH_LOAD:
		case PART_OVERRIDE:
			state = POWERUP_BOOTLOADER | POWERUP_BOOTREADY | POWERUP_APPLICATION;
			break;

		default:
			break;
	}

	return state;
}

uint8 si479xx_checkErrorCode(uint32 ntuner)
{
	uint32 i;
	uint8 cmd[CMD_BUFF_SIZE]={0,};

	cmd[0] = 0xef;
	si479xx_cmdReadWrite(cmd, rsp_buff, 6, ntuner);
	for(i=0; i<6; i++) {
		rsp_buff[i] = rsp_buff[i+1];
	}

	SILAB_ERR("[si479x SPI In %d] Len[%d] Data", ntuner, 5);
	for(i=0; i<5; i++) {
		printf("[0x%02x]", rsp_buff[i]);
	}
	printf("\n");
	SILAB_ERR("[%s] Error Code[%d] \n", __func__, rsp_buff[4]);
	return rsp_buff[4];
}

void si479xx_readReply(uint16 len, uint8 *buffer, uint32 ntuner)
{
	uint16 i;
	uint8 cmd[CMD_BUFF_SIZE]={0,};

	cmd[0] = READ_REPLY;
	si479xx_cmdReadWrite(cmd, buffer, len+1, ntuner);
	for(i=0; i<len; i++) {
		buffer[i] = buffer[i+1];
	}
}

void si479xx_readMore(uint16 len, uint8 *buffer, uint32 ntuner)
{
	uint16 i;
	uint8 cmd[CMD_BUFF_SIZE] = {0,};

	cmd[0] = READ_MORE;
	si479xx_cmdReadWrite(cmd, &buffer[0], len+1, ntuner);
	for(i=0; i<len; i++) {
		buffer[i] = buffer[i+1];
	}
}

RET si479xx_waitForSTC(uint32 ntuner)
{
	uint16 i = 1000;

	do
	{
		si479xx_readReply(4, rsp_buff, ntuner);

		if(ntuner == eTUNER_DRV_ID_SECONDARY || ntuner == eTUNER_DRV_ID_QUATERNARY) {
			if ((rsp_buff[1] & STCINT) == STCINT)
			{
				return eRET_OK;
			}
		}
		else {
			if ((rsp_buff[0] & STCINT) == STCINT)
			{
				return eRET_OK;
			}
		}

		if ((rsp_buff[0] & 0x60) != 0 ||	(rsp_buff[3] & 0x3f) != 0)
		{
			SILAB_ERR("[%s] eRET_NG_INVALID_RESP!!! ntuner[%d], rsp_buff=%02xh %02xh %02xh %02xh\n", __func__, ntuner, rsp_buff[0], rsp_buff[1], rsp_buff[2], rsp_buff[3]);
			return eRET_NG_INVALID_RESP;
		}

		si479xx_mssleep(1);
	}while (--i);

	SILAB_ERR("[%s] Time Out Error!!! ntuner[%d], rsp_buff=%02xh %02xh %02xh %02xh\n", __func__, ntuner, rsp_buff[0], rsp_buff[1], rsp_buff[2], rsp_buff[3]);
	return eRET_NG_TIMEOUT;
}

RET si479xx_waitForCTS(uint32 ntuner)
{
	uint16 i = 1000;

	do
	{
		si479xx_readReply(4, rsp_buff, ntuner);

		if ((rsp_buff[0] & CTS) == CTS)
		{
			return eRET_OK;
		}

		if ((rsp_buff[0] & 0x60) != 0 || (rsp_buff[3] & 0x3f) != 0)
		{
			SILAB_ERR("[%s] eRET_NG_INVALID_RESP!!! ntuner[%d], rsp_buff=%02xh %02xh %02xh %02xh\n", __func__, ntuner, rsp_buff[0], rsp_buff[1], rsp_buff[2], rsp_buff[3]);
			//si479xx_checkErrorCode(ntuner);
			return eRET_NG_INVALID_RESP;
		}

		si479xx_mssleep(1);
	}while (--i);

	SILAB_ERR("[%s] Time Out Error!!! ntuner[%d], rsp_buff=%02xh %02xh %02xh %02xh\n", __func__, ntuner, rsp_buff[0], rsp_buff[1], rsp_buff[2], rsp_buff[3]);
	return eRET_NG_TIMEOUT;
}

RET si479xx_checkPowerupStatus(uint8 state, uint32 ntuner)
{
	uint8 ts;

	si479xx_readReply(4, rsp_buff, ntuner);

	ts = 1 << ((rsp_buff[3] >> 6) & 0x03);

	if ((ts & state) == ts)
	{
		return eRET_OK;
	}
	else
	{
		uint32 i;
		SILAB_ERR("[si479x SPI In %d] Len[%d] Data", ntuner, 4);
		for( i=0; i<4; i++) {
			printf("[0x%02x]", rsp_buff[i]);
		}
		printf("\n");
		SILAB_ERR("[%s] STATE ERROR (state:0x%02x, ts:0x%02x) \n", __func__, state, ts);
		return eRET_NG_INVALID_STATUS;
	}
}

RET si479xx_checkApplicationState(uint32 ntuner)
{
	RET ret = eRET_OK;
	uint8 ts;

	si479xx_readReply(4, rsp_buff, ntuner);
	ts = 1 << ((rsp_buff[3] >> 6) & 0x03);
	if ((ts & POWERUP_APPLICATION) != ts) {
		ret = eRET_NG_INVALID_STATUS;
	}

	return ret;
}

RET si479xx_command(uint32 cmd_size, uint8 *cmd, uint32 reply_size, uint8 *reply, uint32 ntuner)
{
	RET ret = eRET_OK;
	uint8 state = 0;
	int32 i;

//	SILAB_DBG("[%s][%d/%d]\n", __func__, cmd_size, reply_size);

	if (cmd_size == 0)
		return eRET_NG_INVALID_PARAM;

	state = si479xx_getCommandRequestStatus(cmd[0]);

	ret = si479xx_checkPowerupStatus(state, ntuner);
	if(ret != eRET_OK) {
		SILAB_ERR("[Power Status error before command]: ");
		goto error_command;
	}

	ret = si479xx_waitForCTS(ntuner);
	if(ret != eRET_OK) {
		SILAB_ERR("[CTS error before command]: ");
		goto error_command;
	}

	ret = si479xx_cmdReadWrite(cmd, rsp_buff, cmd_size, ntuner);
	if(ret != eRET_OK) {
		SILAB_ERR("[Command write/read error]: ");
		goto error_command;
	}

	ret = si479xx_waitForCTS(ntuner);
	if(ret != eRET_OK) {
		SILAB_ERR("[CTS error after command]: ");
		goto error_command;
	}

	if(reply_size > 4) {
		si479xx_readReply(reply_size, reply, ntuner);
	}

error_command:
#ifdef SILAB_DEBUG
	if(cmd[0] != HOST_LOAD && (cmd[0] != TUNER_CMD_GATEWAY || cmd[4] != FM_RDS_STATUS || reply_size != 20)) {
		SILAB_PRINTF("[si479x SPI Out %d] Len[%d] Data", ntuner, cmd_size);
		for( i=0; i<cmd_size; i++) {
			SILAB_PRINTF("[0x%02x]", cmd[i]);
		}
		SILAB_PRINTF("\n\n");
	}
#else
	if(ret != eRET_OK) {
		if(cmd[0] != HOST_LOAD && (cmd[0] != TUNER_CMD_GATEWAY || cmd[4] != FM_RDS_STATUS || reply_size != 20)) {
			SILAB_PRINTF("[si479x SPI Out %d] Len[%d] Data", ntuner, cmd_size);
			for( i=0; i<cmd_size; i++) {
				SILAB_PRINTF("[0x%02x]", cmd[i]);
			}
			SILAB_PRINTF("\n\n");
		}
		else {
			uint32 disp_size = cmd_size;
			SILAB_PRINTF("[si479x SPI Out %d] Len[%d] Data", ntuner, disp_size);
			if(disp_size > 20) {	// 516
				disp_size = 20;		// 516
				for( i=0; i<disp_size; i++) {
					SILAB_PRINTF("[0x%02x]", cmd[i]);
				}
				SILAB_PRINTF("...\n\n");
			}
			else {
				for( i=0; i<disp_size; i++) {
					SILAB_PRINTF("[0x%02x]", cmd[i]);
				}
				SILAB_PRINTF("\n\n");
			}
		}
	}
#endif

	return ret;
}

/* boot command */
stPOWER_UP_ARGS_t * si479xx_get_power_up_args(eTUNER_DRV_CONF_TYPE_t tunertype, uint8 pd_mode, uint8 iqout, uint32 ntuner)
{
	si479xx_powerup_args.CTSIEN = 0;
	si479xx_powerup_args.VIO = 0;
	si479xx_powerup_args.XTAL_FREQ = 36864000;
	if(gSilabConf.audioSamplerate == 48000) {
		si479xx_powerup_args.AFS = 0;
	}
	else {
		si479xx_powerup_args.AFS = 1;
	}

#if SILAB_FW_VER == 40 || SILAB_FW_VER == 50
    if (ntuner == eTUNER_DRV_ID_PRIMARY)
	{
		si479xx_powerup_args.CLKO_CURRENT = 7;
		si479xx_powerup_args.CLKOUT = 1;
		si479xx_powerup_args.CLK_MODE = eCLK_XTAL;
		si479xx_powerup_args.TR_SIZE = 8;
		si479xx_powerup_args.CTUN = 21;
		si479xx_powerup_args.IBIAS = 67;

		si479xx_powerup_args.CHIPID = 0;

		if(iqout == 1)
        {
            si479xx_powerup_args.EZIQ_MASTER = 1;
			if(tunertype >= eTUNER_DRV_CONF_TYPE_DUAL) {
                si479xx_powerup_args.EZIQ_ENABLE = 4;
			}
			else {
				si479xx_powerup_args.EZIQ_ENABLE = 1;
			}
		}
		else
        {
            si479xx_powerup_args.EZIQ_MASTER = 1;
            si479xx_powerup_args.EZIQ_ENABLE = 0;
		}
	}
	else
	{
		si479xx_powerup_args.CLKO_CURRENT = 0;
		si479xx_powerup_args.CLKOUT = 0;
		si479xx_powerup_args.CLK_MODE = eCLK_SINGLE_DC;
		si479xx_powerup_args.TR_SIZE = 0;
		si479xx_powerup_args.CTUN = 0;
		si479xx_powerup_args.IBIAS = 0;

		si479xx_powerup_args.CHIPID = 1;

		if(iqout == 1)
        {
            si479xx_powerup_args.EZIQ_MASTER = 1;
			if(tunertype >= eTUNER_DRV_CONF_TYPE_QUAD) {
                si479xx_powerup_args.EZIQ_ENABLE = 4;
			}
			else {
				si479xx_powerup_args.EZIQ_ENABLE = 1;
			}
		}
		else
        {
            si479xx_powerup_args.EZIQ_MASTER = 1;
            si479xx_powerup_args.EZIQ_ENABLE = 0;
		}
	}
#else
	if(gSilabConf.sdr == eTUNER_SDR_DAB) {
		si479xx_powerup_args.DIGDAT = 1;	// If you use DAB, set to 1. Si47962 support DAB.
		si479xx_powerup_args.IQRATE = 0;
	#ifdef USE_DAB_IQ_CLKINV
		si479xx_powerup_args.IQEARLY = 1;
	#else
		si479xx_powerup_args.IQEARLY = 0;
	#endif
	}
	else {
   		si479xx_powerup_args.DIGDAT = 0;	// If you use HDR, set to 0. Si47961 do not support DAB.
   		si479xx_powerup_args.IQRATE = 1;
		si479xx_powerup_args.IQEARLY = 0;
	}

	if (ntuner == eTUNER_DRV_ID_PRIMARY)
	{
		si479xx_powerup_args.CLKO_CURRENT = 7;
		si479xx_powerup_args.CLKOUT = 1;
		si479xx_powerup_args.CLK_MODE = eCLK_XTAL;
		si479xx_powerup_args.TR_SIZE = 8;
		si479xx_powerup_args.CTUN = 21;
		si479xx_powerup_args.IBIAS = 67;

		if (pd_mode != 0) {
			si479xx_powerup_args.CHIPID = 0;
			si479xx_powerup_args.ICLINK = 17;
			si479xx_powerup_args.ISFSMODE = 1;		// deleted on the v2.4
		}
		else {
			si479xx_powerup_args.CHIPID = 0;
			si479xx_powerup_args.ICLINK = 0;
			si479xx_powerup_args.ISFSMODE = 0;		// deleted on the v2.4
		}

		if(iqout == 1) {
			if(tunertype >= eTUNER_DRV_CONF_TYPE_DUAL) {
				si479xx_powerup_args.IQCHANNELS = 1;
			}
			else {
				si479xx_powerup_args.IQCHANNELS = 0;
			}
			si479xx_powerup_args.IQFMT = 1;
			si479xx_powerup_args.IQMUX = 0;

			si479xx_powerup_args.IQSWAP1 = 0;		// deleted on the v2.4
			si479xx_powerup_args.IQSWAP0 = 0;
			si479xx_powerup_args.IQSLOT = 1;
		#ifdef USE_COMMON_IQ_CLOCK
			if(tunertype <= eTUNER_DRV_CONF_TYPE_DUAL) {
				si479xx_powerup_args.IQOUT = 1;
			}
			else {
				si479xx_powerup_args.IQOUT = 2;
			}
		#else
			si479xx_powerup_args.IQOUT = 1;
		#endif
		}
		else {
			si479xx_powerup_args.IQCHANNELS = 0;
			si479xx_powerup_args.IQFMT = 0;
			si479xx_powerup_args.IQMUX = 0;

			si479xx_powerup_args.IQSWAP1 = 0;		// deleted on the v2.4
			si479xx_powerup_args.IQSWAP0 = 0;
			si479xx_powerup_args.IQSLOT = 1;
			si479xx_powerup_args.IQOUT = 0;
		}
	}
	else
	{
		si479xx_powerup_args.CLKO_CURRENT = 0;
		si479xx_powerup_args.CLKOUT = 0;
		si479xx_powerup_args.CLK_MODE = eCLK_SINGLE_DC;
		si479xx_powerup_args.TR_SIZE = 0;
		si479xx_powerup_args.CTUN = 0;
		si479xx_powerup_args.IBIAS = 0;

		if (pd_mode != 0)
		{
			si479xx_powerup_args.CHIPID = 1;
			si479xx_powerup_args.ICLINK = 10;
			si479xx_powerup_args.ISFSMODE = 2;		// deleted on the v2.4
		}
		else
		{
			si479xx_powerup_args.CHIPID = 1;
			si479xx_powerup_args.ICLINK = 0;
			si479xx_powerup_args.ISFSMODE = 0;		// deleted on the v2.4
		}

		if(iqout == 1) {
			if(tunertype == eTUNER_DRV_CONF_TYPE_QUAD) {
				si479xx_powerup_args.IQCHANNELS = 1;
			}
			else {
				si479xx_powerup_args.IQCHANNELS = 0;
			}
			si479xx_powerup_args.IQFMT = 1;
			si479xx_powerup_args.IQMUX = 0;

			si479xx_powerup_args.IQSWAP1 = 0;		// deleted on the v2.4
			si479xx_powerup_args.IQSWAP0 = 0;
			si479xx_powerup_args.IQSLOT = 1;
			si479xx_powerup_args.IQOUT = 1;
		}
		else {
			si479xx_powerup_args.IQCHANNELS = 0;
			si479xx_powerup_args.IQFMT = 0;
			si479xx_powerup_args.IQMUX = 0;

			si479xx_powerup_args.IQSWAP1 = 0;		// deleted on the v2.4
			si479xx_powerup_args.IQSWAP0 = 0;
			si479xx_powerup_args.IQSLOT = 1;
			si479xx_powerup_args.IQOUT = 0;
		}
	}
#endif

	return &si479xx_powerup_args;
}


RET si479xx_boot(uint32 ntuner)
{
	RET ret = eRET_OK;

	SILAB_DBG("[%s] ", __func__);

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	cmd_buff[0] = BOOT;

	ret = si479xx_command(3, cmd_buff, 4, rsp_buff, ntuner);

	return ret;
}

RET si479xx_powerup(stPOWER_UP_ARGS_t *pPowerup_args, uint32 ntuner)
{
	RET ret = eRET_OK;

	if (pPowerup_args == NULL)
	{
		return eRET_NG_INVALID_PARAM;
	}

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	cmd_buff[0] = POWER_UP;

#if SILAB_FW_VER == 40 || SILAB_FW_VER == 50
	if (pPowerup_args->CTSIEN)			cmd_buff[1] |= (pPowerup_args->CTSIEN & 0x01) << 7;
	if (pPowerup_args->CLKO_CURRENT)	cmd_buff[1] |= (pPowerup_args->CLKO_CURRENT & 0x07) << 4;
	if (pPowerup_args->VIO)				cmd_buff[1] |= (pPowerup_args->VIO & 0x01) << 2;
	if (pPowerup_args->CLKOUT)			cmd_buff[1] |= (pPowerup_args->CLKOUT & 0x01) << 1;
	if (pPowerup_args->CLK_MODE)		cmd_buff[2] |= (pPowerup_args->CLK_MODE & 0x0f) << 4;
	if (pPowerup_args->TR_SIZE)			cmd_buff[2] |= (pPowerup_args->TR_SIZE & 0x0f);
	if (pPowerup_args->CTUN)			cmd_buff[3] |= (pPowerup_args->CTUN & 0x3f);

	cmd_buff[4] = (uint8) (pPowerup_args->XTAL_FREQ >> 0);
	cmd_buff[5] = (uint8) (pPowerup_args->XTAL_FREQ >> 8);
	cmd_buff[6] = (uint8) (pPowerup_args->XTAL_FREQ >> 16);
	cmd_buff[7] = (uint8) (pPowerup_args->XTAL_FREQ >> 24);
	cmd_buff[8] = pPowerup_args->IBIAS;

	if (pPowerup_args->AFS)				cmd_buff[9] |= ((pPowerup_args->AFS & 0x01) << 7);
	if (pPowerup_args->CHIPID)			cmd_buff[10] |= (pPowerup_args->CHIPID & 0x07);

	if (pPowerup_args->EZIQ_MASTER)		cmd_buff[24] |= ((pPowerup_args->EZIQ_MASTER & 0x01) << 4);
	if (pPowerup_args->EZIQ_ENABLE)		cmd_buff[24] |= (pPowerup_args->EZIQ_ENABLE & 0x07);

	ret = si479xx_command(25, cmd_buff, 6, rsp_buff, ntuner);
#else
	if (pPowerup_args->CTSIEN)			cmd_buff[1] |= (pPowerup_args->CTSIEN & 0x01) << 7;
	if (pPowerup_args->CLKO_CURRENT)	cmd_buff[1] |= (pPowerup_args->CLKO_CURRENT & 0x07) << 4;
	if (pPowerup_args->VIO)				cmd_buff[1] |= (pPowerup_args->VIO & 0x01) << 2;
	if (pPowerup_args->CLKOUT)			cmd_buff[1] |= (pPowerup_args->CLKOUT & 0x01) << 1;
	if (pPowerup_args->CLK_MODE)		cmd_buff[2] |= (pPowerup_args->CLK_MODE & 0x0f) << 4;
	if (pPowerup_args->TR_SIZE)			cmd_buff[2] |= (pPowerup_args->TR_SIZE & 0x0f);
	if (pPowerup_args->CTUN)			cmd_buff[3] |= (pPowerup_args->CTUN & 0x3f);

	cmd_buff[4] = (uint8) (pPowerup_args->XTAL_FREQ >> 0);
	cmd_buff[5] = (uint8) (pPowerup_args->XTAL_FREQ >> 8);
	cmd_buff[6] = (uint8) (pPowerup_args->XTAL_FREQ >> 16);
	cmd_buff[7] = (uint8) (pPowerup_args->XTAL_FREQ >> 24);
	cmd_buff[8] = pPowerup_args->IBIAS;

	if (pPowerup_args->AFS)				cmd_buff[9] |= ((pPowerup_args->AFS & 0x01) << 7);
	if (pPowerup_args->DIGDAT)			cmd_buff[9] |= ((pPowerup_args->DIGDAT & 0x01) << 4);
	if (pPowerup_args->CHIPID)			cmd_buff[10] |= (pPowerup_args->CHIPID & 0x07);
//	if (pPowerup_args->ISFSMODE)		cmd_buff[20] |= (pPowerup_args->ISFSMODE & 0x03);
	if (pPowerup_args->ICLINK)			cmd_buff[21] |= (pPowerup_args->ICLINK & 0x7F);
	if (pPowerup_args->IQCHANNELS)		cmd_buff[22] |= (pPowerup_args->IQCHANNELS & 0x03) << 6;
	if (pPowerup_args->IQFMT)			cmd_buff[22] |= (pPowerup_args->IQFMT & 0x03) << 4;
	if (pPowerup_args->IQMUX)			cmd_buff[22] |= (pPowerup_args->IQMUX & 0x01) << 3;
	if (pPowerup_args->IQRATE)			cmd_buff[22] |= (pPowerup_args->IQRATE & 0x07);
//	if (pPowerup_args->IQSWAP1)			cmd_buff[23] |= (pPowerup_args->IQSWAP1 & 0x01) << 7;
	if (pPowerup_args->IQSWAP0)			cmd_buff[23] |= (pPowerup_args->IQSWAP0 & 0x01) << 6;
	if (pPowerup_args->IQSLOT)			cmd_buff[23] |= (pPowerup_args->IQSLOT & 0x07) << 3;
	if (pPowerup_args->IQEARLY)			cmd_buff[23] |= (pPowerup_args->IQEARLY & 0x01) << 2;
	if (pPowerup_args->IQOUT)			cmd_buff[23] |= (pPowerup_args->IQOUT & 0x03);

	ret = si479xx_command(24, cmd_buff, 6, rsp_buff, ntuner);
#endif

	return ret;
}

RET si479xx_setDacOut(uint32 ntuner, uint32 enable)
{
	RET ret = eRET_OK;

	SILAB_DBG("[%s] ", __func__);

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	cmd_buff[0] = DAC_CONFIG;
	cmd_buff[1] = 0x10;

	if(ntuner == eTUNER_DRV_ID_SECONDARY || ntuner == eTUNER_DRV_ID_QUATERNARY) {
		if(enable) {
			cmd_buff[1] |= 0x03;
		}
	}
	else {
		if(enable) {
			cmd_buff[1] |= 0x02;
		}
	}

	ret = si479xx_command(2, cmd_buff, 4, rsp_buff, ntuner);

	return ret;
}

RET si479xx_I2S_AudioOut(uint32 ntuner)
{
	RET ret = eRET_OK;

	SILAB_DBG("[%s] ", __func__);

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	if((ntuner == eTUNER_DRV_ID_PRIMARY) || (ntuner == eTUNER_DRV_ID_TERTIARY)) {
		cmd_buff[0] = I2STX0_CONFIG;
	}
	else {
		cmd_buff[0] = I2STX1_CONFIG;
	}
	if(gSilabConf.audioSamplerate == 48000) {
		cmd_buff[1] = 0x06;		// SAMPLE_RATE : 48000 Samples/s
	}
	else {
		cmd_buff[1] = 0x12;		// SAMPLE_RATE : 44100 Samples/s
	}
#if SILAB_FW_VER == 25 || SILAB_FW_VER == 30
	if((ntuner == eTUNER_DRV_ID_PRIMARY) || (ntuner == eTUNER_DRV_ID_TERTIARY)) {
		cmd_buff[2] = 0x0C;		// PURPOSE
	}
	else {
		cmd_buff[2] = 0x09;		// PURPOSE
	}
	cmd_buff[3] = 0x1A;		// MASTER, CLKFS
#else
	cmd_buff[2] = 0x10;		// PURPOSE
	cmd_buff[3] = 0x3A;		// ENABLE, MASTER, CLKFS
#endif
	cmd_buff[4] = 0x0A;		// DATA0
	cmd_buff[5] = 0x21;		// Slot: 32bit, Sample: 16bit
	cmd_buff[6] = 0x00;		// FRAMING_MODE
	cmd_buff[7] = 0x00;
	cmd_buff[8] = 0x00;
	cmd_buff[9] = 0x00;
	cmd_buff[10] = 0x00;

#if SILAB_FW_VER == 25 || SILAB_FW_VER == 30
	ret = si479xx_command(11, cmd_buff, 4, rsp_buff, ntuner);
#else
	ret = si479xx_command(8, cmd_buff, 4, rsp_buff, ntuner);
#endif
	return ret;
}

RET si479xx_setI2sControl(uint32 ntuner, uint32 i2sport, uint32 enable)
{
	RET ret = eRET_OK;

	SILAB_DBG("[%s] ", __func__);

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	// I2S TX[0:4] = 0x00, 0x01, 0x02, 0x03, 0x04
	// I2S RX[0:4] = 0x08, 0x09, 0x0A, 0x0B, 0x0C

	cmd_buff[0] = I2S_CONTROL;

	switch(i2sport) {
		case 0x00:	cmd_buff[1] = 0x00;	break;
		case 0x01:	cmd_buff[1] = 0x10;	break;
		case 0x02:	cmd_buff[1] = 0x20;	break;
		case 0x03:	cmd_buff[1] = 0x30;	break;
		case 0x04:	cmd_buff[1] = 0x40;	break;
		case 0x08:	cmd_buff[1] = 0x80;	break;
		case 0x09:	cmd_buff[1] = 0x90;	break;
		case 0x0A:	cmd_buff[1] = 0xA0;	break;
		case 0x0B:	cmd_buff[1] = 0xB0;	break;
		case 0x0C:	cmd_buff[1] = 0xC0;	break;
		default:	ret = eRET_NG_INVALID_PARAM;	break;
	}

	cmd_buff[1] |= (enable != 0) ? 0x04:0x02;
	ret = si479xx_command(2, cmd_buff, 4, rsp_buff, ntuner);

	return ret;
}

RET si479xx_setProperty(uint8 group, uint8 number, uint8 hdata, uint8 ldata, uint32 ntuner)
{
	RET ret = eRET_OK;

	SILAB_DBG("[%s] ", __func__);

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	cmd_buff[0] = SET_PROPERTY;
	cmd_buff[1] = 0x00;
	cmd_buff[2] = number;
	cmd_buff[3] = group;
	cmd_buff[4] = ldata;
	cmd_buff[5] = hdata;

	ret = si479xx_command(6, cmd_buff, 4, rsp_buff, ntuner);

	return ret;
}

RET si479xx_setTunerProperty(uint8 group, uint8 index, uint8 hdata, uint8 ldata, uint32 ntuner)
{
	RET ret = eRET_OK;
	uint8 tunerIndex = 0x00;
    int i;

	SILAB_DBG("[%s] ", __func__);

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	if(ntuner == eTUNER_DRV_ID_SECONDARY || ntuner == eTUNER_DRV_ID_QUATERNARY) {
		tunerIndex = 0x01;
	}

	cmd_buff[0] = TUNER_CMD_GATEWAY;
	cmd_buff[1] = tunerIndex;
	cmd_buff[2] = 0x00;
	cmd_buff[3] = 0x00;
	cmd_buff[4] = SET_PROPERTY;
	cmd_buff[5] = 0x00;
	cmd_buff[6] = index;
	cmd_buff[7] = group;
	cmd_buff[8] = ldata;
	cmd_buff[9] = hdata;

	ret = si479xx_command(10, cmd_buff, 4, rsp_buff, ntuner);

#if 0
    SILAB_DBG("[si479x rsp_buff");
    for(i=0; i<4; i++) {
        SILAB_DBG("[0x%02x]", rsp_buff[i]);
    }
    SILAB_DBG("\n");
#endif

	return ret;
}

RET si479xx_setTunerCmdArray(uint8 *txbuf, uint32 txlen, uint32 ntuner)
{
	RET ret = eRET_OK;
	uint8 tunerIndex = 0x00;
	int32 i;

	SILAB_DBG("[%s] ", __func__);

	if(txlen > 250) {
		return eRET_NG_INVALID_PARAM;
	}

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	if(ntuner == eTUNER_DRV_ID_SECONDARY || ntuner == eTUNER_DRV_ID_QUATERNARY) {
		tunerIndex = 0x01;
	}

	cmd_buff[0] = TUNER_CMD_GATEWAY;
	cmd_buff[1] = tunerIndex;
	cmd_buff[2] = 0x00;
	cmd_buff[3] = 0x00;

	for(i=0; i<txlen; i++) {
		cmd_buff[4+i] = txbuf[i];
	}

	ret = si479xx_command(txlen+4, cmd_buff, 4, rsp_buff, ntuner);

	return ret;
}

RET si479xx_setTunerServo(uint8 id, uint8 hmin, uint8 lmin, uint8 hmax, uint8 lmax, uint8 hinit, uint8 linit, uint8 acc, uint32 ntuner, uint8 band)
{
	RET ret = eRET_OK;
	uint8 tunerIndex = 0x00;

	SILAB_DBG("[%s] ", __func__);

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	if(ntuner == eTUNER_DRV_ID_SECONDARY || ntuner == eTUNER_DRV_ID_QUATERNARY) {
		tunerIndex = 0x01;
	}

	cmd_buff[0] = TUNER_CMD_GATEWAY;
	cmd_buff[1] = tunerIndex;
	cmd_buff[2] = 0x00;
	cmd_buff[3] = 0x00;
	if(band == 1)
		cmd_buff[4] = AM_SET_SERVO;
	else
		cmd_buff[4] = FM_SET_SERVO;
	cmd_buff[5] = id;
	cmd_buff[6] = lmin;
	cmd_buff[7] = hmin;
	cmd_buff[8] = lmax;
	cmd_buff[9] = hmax;
	cmd_buff[10] = linit;
	cmd_buff[11] = hinit;
	cmd_buff[12] = acc;

	ret = si479xx_command(13, cmd_buff, 4, rsp_buff, ntuner);

	return ret;
}

uint8 si479xx_getError(uint32 ntuner)
{
	RET ret = eRET_OK;
	uint8 retval = 0;

	SILAB_DBG("[%s] ", __func__);

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	cmd_buff[0] = 0xEF;

	si479xx_cmdReadWrite(cmd_buff, rsp_buff, 6, ntuner);

	retval = rsp_buff[5];

	SILAB_ERR("[%s] rsp_buff = %02xh %02xh %02xh %02xh %02xh %02xh\n", __func__, rsp_buff[0], rsp_buff[1], rsp_buff[2], rsp_buff[3], rsp_buff[4], rsp_buff[5]);

	return retval;
}

uint8 si479xx_getMaxDabFreqCount(void)
{
	return (uint8)MAX_NUM_OF_DAB_FREQ;
}

uint8 si479xx_getCurrentDabFreqCount(uint32 ntuner)
{
	uint8 dabcount;

	if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY) {
		dabcount = (uint8)dab01FreqTableSize;
	}
	else if(ntuner == eTUNER_DRV_ID_TERTIARY || ntuner == eTUNER_DRV_ID_QUATERNARY){
		dabcount = (uint8)dab23FreqTableSize;
	}
	else {
		dabcount = 0;
	}

	return dabcount;
}

RET si479xx_setDabFreqListToReg(uint32 *dab_freq_table, uint8 num_freq, uint32 ntuner)
{
	RET ret = eRET_OK;
	int i;

	SILAB_DBG("[%s] ", __func__);

	if(dab_freq_table != NULL && ntuner < gSilabConf.numTuners) {
		if(num_freq <= MAX_NUM_OF_DAB_FREQ) {
		#if SILAB_FW_VER == 50
			if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY) {
				si479xx_memcpy(dab01FreqTable_BandIII, dab_freq_table, num_freq * 4);
				dab01FreqTableSize = num_freq;
				SILAB_DBG("Set DAB01 Freq List Count: %d\n", dab01FreqTableSize);
			}
			else {
				si479xx_memcpy(dab23FreqTable_BandIII, dab_freq_table, num_freq * 4);
				dab23FreqTableSize = num_freq;
				SILAB_DBG("Set DAB23 Freq List Count: %d\n", dab23FreqTableSize);
			}
		#else
			si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

			cmd_buff[0] = DAB_SET_FREQ_LIST;
			cmd_buff[1] = num_freq;
			cmd_buff[2] = 0x00;
			cmd_buff[3] = 0x00;
			for (i=0; i<num_freq; i++)
		    {
		        si479xx_32bto8b(cmd_buff+4+i*4, *(dab_freq_table+i));
		    }

			ret = si479xx_command(i*4+4, cmd_buff, 4, rsp_buff, ntuner);
			if(ret == eRET_OK) {
				if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY) {
					si479xx_memcpy(dab01FreqTable_BandIII, dab_freq_table, sizeof(dab01FreqTable_BandIII));
					dab01FreqTableSize = num_freq;
				}
				else {
					si479xx_memcpy(dab23FreqTable_BandIII, dab_freq_table, sizeof(dab23FreqTable_BandIII));
					dab23FreqTableSize = num_freq;
				}
			}
		#endif
		}
		else {
			ret = eRET_NG_INVALID_LENGTH;
		}
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

RET si479xx_getDabFreqListFromReg(uint32 *dab_freq_table, uint8 *num_freq, uint32 ntuner)
{
	RET ret = eRET_OK;
	int i;

	SILAB_DBG("[%s] ", __func__);

	if(dab_freq_table != NULL && ntuner < gSilabConf.numTuners) {
	#if SILAB_FW_VER == 50
		if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY) {
			si479xx_memcpy(dab_freq_table, dab01FreqTable_BandIII, dab01FreqTableSize * 4);
			*num_freq = (uint8)dab01FreqTableSize;
		}
		else {
			si479xx_memcpy(dab_freq_table, dab23FreqTable_BandIII, dab23FreqTableSize * 4);
			*num_freq = (uint8)dab23FreqTableSize;
		}
	#else
		si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

		cmd_buff[0] = DAB_GET_FREQ_LIST;
		cmd_buff[1] = 00;
		ret = si479xx_command(2, cmd_buff, 264, rsp_buff, ntuner);

		if(ret == eRET_OK) {
			if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY) {
				si479xx_memset(dab01FreqTable_BandIII, 0, sizeof(dab01FreqTable_BandIII));
				for(i=0; i<rsp_buff[4]; i++) {
					dab01FreqTable_BandIII[i] = ((uint32)rsp_buff[i*4+11]<<24) | ((uint32)rsp_buff[i*4+10]<<16) | ((uint32)rsp_buff[i*4+9]<<8) | (uint32)rsp_buff[i*4+8];
				}
				dab01FreqTableSize = rsp_buff[4];
				si479xx_memcpy(dab_freq_table, dab01FreqTable_BandIII, sizeof(dab01FreqTable_BandIII));
				*num_freq = (uint8)dab01FreqTableSize;
			}
			else {
				si479xx_memset(dab23FreqTable_BandIII, 0, sizeof(dab23FreqTable_BandIII));
				for(i=0; i<rsp_buff[4]; i++) {
					dab23FreqTable_BandIII[i] = ((uint32)rsp_buff[i*4+11]<<24) | ((uint32)rsp_buff[i*4+10]<<16) | ((uint32)rsp_buff[i*4+9]<<8) | (uint32)rsp_buff[i*4+8];
				}
				dab23FreqTableSize = rsp_buff[4];
				si479xx_memcpy(dab_freq_table, dab23FreqTable_BandIII, sizeof(dab23FreqTable_BandIII));
				*num_freq = (uint8)dab23FreqTableSize;
			}
		}
	#endif
	}
	else {
		ret = eRET_NG_INVALID_PARAM;
	}

	return ret;
}

uint32 si479xx_getDabFreqFromList(uint16 index, uint32 ntuner)
{
	uint32 dabfreq = 0;

	SILAB_DBG("[%s] ", __func__);

	if(ntuner < gSilabConf.numTuners) {
		if(ntuner == eTUNER_DRV_ID_PRIMARY || ntuner == eTUNER_DRV_ID_SECONDARY) {
			if((uint32)index < dab01FreqTableSize) {
				dabfreq = dab01FreqTable_BandIII[index];
			}
		}
		else {
			if((uint32)index < dab23FreqTableSize) {
				dabfreq = dab23FreqTable_BandIII[index];
			}
		}
	}

	return dabfreq;
}

static void si479xx_setConfigProperty(uint32 tunertype)
{
	// Antenna Configuration + Tuner RF Property
#if SILAB_FW_VER == 40 || SILAB_FW_VER == 50

	switch(tunertype) {
		case eTUNER_DRV_CONF_TYPE_TRIPLE:
			// Antenna Configuration Setting
			if(gSilabConf.sdr == eTUNER_SDR_DAB) {
				si479xx_setProperty(0x06, 0x04, 0x05, 0x15, eTUNER_DRV_ID_PRIMARY);
			}
			else {
				si479xx_setProperty(0x06, 0x04, 0x01, 0x05, eTUNER_DRV_ID_PRIMARY);
			}
			si479xx_setProperty(0x06, 0x05, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x06, 0x00, 0x10, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x07, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x08, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x09, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			break;

		case eTUNER_DRV_CONF_TYPE_QUAD:
			// Antenna Configuration Setting
			if(gSilabConf.sdr == eTUNER_SDR_DAB) {
				si479xx_setProperty(0x06, 0x04, 0x05, 0x15, eTUNER_DRV_ID_PRIMARY);
			}
			else {
				si479xx_setProperty(0x06, 0x04, 0x01, 0x05, eTUNER_DRV_ID_PRIMARY);
			}
			si479xx_setProperty(0x06, 0x05, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x06, 0x00, 0x10, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x07, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x08, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x09, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			break;
	}

	// To reduce AM audio noise level when there is no carrier. (Softmute is 12dB when RSSI is less than -4dB)
	uint8 txbuf[20] = {AM_SET_SERVO_TRANSFORM, 0x14, 0x02, 0x03, 0xFC, 0xFF, 0x0C, 0x00, 0x08, 0x00, 0x00, 0x00, 0x88, 0x13, 0xF4, 0x01, 0x00, 0x00, 0x00};
	si479xx_setTunerCmdArray(txbuf, 19, 0);	// AM_SET_SERVO_TRANSFORM
	si479xx_setTunerServo(0x03, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 1, 0, 1);	// AM_SET_SERVO

#else	// #if SILAB_FW_VER == 40

	switch(tunertype) {
		case eTUNER_DRV_CONF_TYPE_TRIPLE:
			// Antenna Configuration Setting
			si479xx_setProperty(0x06, 0x04, 0x01, 0x05, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x05, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x06, 0x00, 0x10, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x07, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x08, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x09, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			break;

		case eTUNER_DRV_CONF_TYPE_QUAD:
			// Antenna Configuration Setting
			si479xx_setProperty(0x06, 0x04, 0x01, 0x05, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x05, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x06, 0x00, 0x10, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x07, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x08, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			si479xx_setProperty(0x06, 0x09, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
			break;
	}

#endif	// #if SILAB_FW_VER == 40
}


void si479xx_setIqProperty(uint32 tunertype)
{
#if SILAB_FW_VER == 40 || SILAB_FW_VER == 50

	switch(tunertype) {
		case eTUNER_DRV_CONF_TYPE_SINGLE:
			// ZIF0/1 Output Setting
			if(gSilabConf.fIqOut) {
                // EZIQ_CONFIG
                si479xx_setProperty(0x0e, 0x00, 0x00, 0x00, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_FIXED_SETTINGS
	            si479xx_setProperty(0x0e, 0x01, 0x81, 0x42, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_OUTPUT_SOURCE
                si479xx_setProperty(0x0e, 0x02, 0x00, 0x80, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_ZIF_FM_CONFIG
				si479xx_setProperty(0x0e, 0x03, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
                si479xx_setProperty(0x0e, 0x04, 0x01, 0x42, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_ZIF_AM_CONFIG0
				si479xx_setProperty(0x0e, 0x05, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);

				if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP) {
					// EZIQ_ZIF_AM_CONFIG1
					SILAB_DBG("[EZIQ_ZIF_AM_CONFIG1 Set IQ samplerate 192kHz] \n");
					si479xx_setProperty(0x0e, 0x06, 0x01, 0x08, eTUNER_DRV_ID_PRIMARY);

					// DRM30_DIG_AGC_CONFIG
					SILAB_DBG("[DRM30_DIG_AGC_CONFIG AGC active] \n");
					si479xx_setProperty(0x4d, 0x00, 0x18, 0x00, eTUNER_DRV_ID_PRIMARY);
#if 0
					SILAB_DBG("[DRM30_DIG_AGC_SPEED change] \n");
					si479xx_setProperty(0x4d, 0x02, 0xff, 0xff, eTUNER_DRV_ID_PRIMARY);
#endif
				}
				else {
					// EZIQ_ZIF_AM_CONFIG1
					si479xx_setProperty(0x0e, 0x06, 0x01, 0x42, eTUNER_DRV_ID_PRIMARY);
				}

				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
				#ifdef USE_DAB_IQ_CLKINV
					// EZIQ_CONFIG
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x04, eTUNER_DRV_ID_PRIMARY);
				#endif
					// EZIQ_ZIF_DAB_CONFIG0
					si479xx_setProperty(0x0e, 0x07, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
					// EZIQ_ZIF_DAB_CONFIG1
					si479xx_setProperty(0x0e, 0x08, 0x01, 0x80, eTUNER_DRV_ID_PRIMARY);
				#ifdef USE_DAB_DIGITAL_AGC
				  #ifndef SI47952_DAB_ISSUE
					// DAB_DIG_AGC_CONFIG
					si479xx_setProperty(0x65, 0x00, 0x00, 0x10, eTUNER_DRV_ID_PRIMARY);
					si479xx_setProperty(0x65, 0x01, 0x18, 0x00, eTUNER_DRV_ID_PRIMARY);
					si479xx_setProperty(0x65, 0x02, 0x0c, 0x0c, eTUNER_DRV_ID_PRIMARY);
				  #endif
				#endif
				}
			}
			break;

		case eTUNER_DRV_CONF_TYPE_DUAL:
			// ZIF0/1 Output Setting
			if(gSilabConf.fIqOut) {
				// EZIQ_CONFIG
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x00, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_FIXED_SETTINGS
				si479xx_setProperty(0x0e, 0x01, 0x81, 0x42, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_OUTPUT_SOURCE
				si479xx_setProperty(0x0e, 0x02, 0x81, 0x80, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_ZIF_FM_CONFIG
				si479xx_setProperty(0x0e, 0x03, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
				si479xx_setProperty(0x0e, 0x04, 0x01, 0x42, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_ZIF_AM_CONFIG0
				si479xx_setProperty(0x0e, 0x05, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);

				if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP) {
					// EZIQ_ZIF_AM_CONFIG1
					SILAB_DBG("[EZIQ_ZIF_AM_CONFIG1 Set IQ samplerate 192kHz] \n");
					si479xx_setProperty(0x0e, 0x06, 0x01, 0x08, eTUNER_DRV_ID_PRIMARY);

					SILAB_DBG("[DRM30_DIG_AGC_CONFIG AGC active] \n");
					si479xx_setProperty(0x4d, 0x00, 0x18, 0x00, eTUNER_DRV_ID_PRIMARY);
#if 0
					SILAB_DBG("[DRM30_DIG_AGC_SPEED change] \n");
					si479xx_setProperty(0x4d, 0x02, 0xff, 0xff, eTUNER_DRV_ID_PRIMARY);
#endif
				}
				else {
					// EZIQ_ZIF_AM_CONFIG1
					si479xx_setProperty(0x0e, 0x06, 0x01, 0x42, eTUNER_DRV_ID_PRIMARY);
				}

				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
				#ifdef USE_DAB_IQ_CLKINV
					// EZIQ_CONFIG
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x04, eTUNER_DRV_ID_PRIMARY);
				#endif
					// EZIQ_ZIF_DAB_CONFIG0
					si479xx_setProperty(0x0e, 0x07, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
					// EZIQ_ZIF_DAB_CONFIG1
					si479xx_setProperty(0x0e, 0x08, 0x01, 0x80, eTUNER_DRV_ID_PRIMARY);
				#ifdef USE_DAB_DIGITAL_AGC
					// DAB_DIG_AGC_CONFIG
					si479xx_setProperty(0x65, 0x00, 0x00, 0x10, eTUNER_DRV_ID_PRIMARY);
					si479xx_setProperty(0x65, 0x01, 0x18, 0x00, eTUNER_DRV_ID_PRIMARY);
					si479xx_setProperty(0x65, 0x02, 0x0c, 0x0c, eTUNER_DRV_ID_PRIMARY);
				#endif
				}
			}
			break;

		case eTUNER_DRV_CONF_TYPE_TRIPLE:
			//si479xx_setDacOut(eTUNER_DRV_ID_TERTIARY, 0);

			// ZIF0/1 Output Setting
			if(gSilabConf.fIqOut) {
                // EZIQ_CONFIG
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x00, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_FIXED_SETTINGS
				si479xx_setProperty(0x0e, 0x01, 0x81, 0x42, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_OUTPUT_SOURCE
				si479xx_setProperty(0x0e, 0x02, 0x81, 0x80, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_ZIF_FM_CONFIG
				si479xx_setProperty(0x0e, 0x03, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
				si479xx_setProperty(0x0e, 0x04, 0x01, 0x42, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_ZIF_AM_CONFIG0
				si479xx_setProperty(0x0e, 0x05, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);

				if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP) {
					// EZIQ_ZIF_AM_CONFIG1 192KHz
					si479xx_setProperty(0x0e, 0x06, 0x01, 0x08, eTUNER_DRV_ID_PRIMARY);

					// DRM30_DIG_AGC_CONFIG AGC on
					si479xx_setProperty(0x4d, 0x00, 0x18, 0x00, eTUNER_DRV_ID_PRIMARY);
#if 0
					SILAB_DBG("[DRM30_DIG_AGC_SPEED change] \n");
					si479xx_setProperty(0x4d, 0x02, 0xff, 0xff, eTUNER_DRV_ID_PRIMARY);
#endif
				}
				else {
					// EZIQ_ZIF_AM_CONFIG1 744.1875KHz
					si479xx_setProperty(0x0e, 0x06, 0x01, 0x42, eTUNER_DRV_ID_PRIMARY);
				}

				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
				#ifdef USE_DAB_IQ_CLKINV
					// EZIQ_CONFIG
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x04, eTUNER_DRV_ID_PRIMARY);
				#endif
					// EZIQ_ZIF_DAB_CONFIG0
					si479xx_setProperty(0x0e, 0x07, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
					// EZIQ_ZIF_DAB_CONFIG1
					si479xx_setProperty(0x0e, 0x08, 0x01, 0x80, eTUNER_DRV_ID_PRIMARY);
				#ifdef USE_DAB_DIGITAL_AGC
					// DAB_DIG_AGC_CONFIG
					si479xx_setProperty(0x65, 0x00, 0x00, 0x10, eTUNER_DRV_ID_PRIMARY);
					si479xx_setProperty(0x65, 0x01, 0x18, 0x00, eTUNER_DRV_ID_PRIMARY);
					si479xx_setProperty(0x65, 0x02, 0x0c, 0x0c, eTUNER_DRV_ID_PRIMARY);
				#endif
				}

				// EZIQ_CONFIG
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x00, eTUNER_DRV_ID_TERTIARY);

				// EZIQ_FIXED_SETTINGS
				si479xx_setProperty(0x0e, 0x01, 0x81, 0x42, eTUNER_DRV_ID_TERTIARY);

				// EZIQ_OUTPUT_SOURCE
				si479xx_setProperty(0x0e, 0x02, 0x00, 0x80, eTUNER_DRV_ID_TERTIARY);

				// EZIQ_ZIF_FM_CONFIG
				si479xx_setProperty(0x0e, 0x03, 0x00, 0x01, eTUNER_DRV_ID_TERTIARY);
				si479xx_setProperty(0x0e, 0x04, 0x01, 0x42, eTUNER_DRV_ID_TERTIARY);

				// EZIQ_ZIF_AM_CONFIG0
				si479xx_setProperty(0x0e, 0x05, 0x00, 0x01, eTUNER_DRV_ID_TERTIARY);

				if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP) {
					// EZIQ_ZIF_AM_CONFIG1 192KHz
					si479xx_setProperty(0x0e, 0x06, 0x01, 0x08, eTUNER_DRV_ID_TERTIARY);

					// DRM30_DIG_AGC_CONFIG AGC on
					si479xx_setProperty(0x4d, 0x00, 0x18, 0x00, eTUNER_DRV_ID_TERTIARY);
#if 0
					SILAB_DBG("[DRM30_DIG_AGC_SPEED change] \n");
					si479xx_setProperty(0x4d, 0x02, 0xff, 0xff, eTUNER_DRV_ID_TERTIARY);
#endif
				}
				else {
					// EZIQ_ZIF_AM_CONFIG1 744.1875KHz
					si479xx_setProperty(0x0e, 0x06, 0x01, 0x42, eTUNER_DRV_ID_TERTIARY);
				}

				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
				#ifdef USE_DAB_IQ_CLKINV
					// EZIQ_CONFIG
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x04, eTUNER_DRV_ID_TERTIARY);
				#endif
					// EZIQ_ZIF_DAB_CONFIG0
					si479xx_setProperty(0x0e, 0x07, 0x00, 0x01, eTUNER_DRV_ID_TERTIARY);
					// EZIQ_ZIF_DAB_CONFIG1
					si479xx_setProperty(0x0e, 0x08, 0x01, 0x80, eTUNER_DRV_ID_TERTIARY);
				#ifdef USE_DAB_DIGITAL_AGC
				  #ifndef SI47952_DAB_ISSUE
					// DAB_DIG_AGC_CONFIG
					si479xx_setProperty(0x65, 0x00, 0x00, 0x10, eTUNER_DRV_ID_TERTIARY);
					si479xx_setProperty(0x65, 0x01, 0x18, 0x00, eTUNER_DRV_ID_TERTIARY);
					si479xx_setProperty(0x65, 0x02, 0x0c, 0x0c, eTUNER_DRV_ID_TERTIARY);
				  #endif
				#endif
				}
			}
			break;

		case eTUNER_DRV_CONF_TYPE_QUAD:
			//si479xx_setDacOut(eTUNER_DRV_ID_TERTIARY, 0);

			// ZIF0/1 Output Setting
			if(gSilabConf.fIqOut) {
                // EZIQ_CONFIG
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x00, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_FIXED_SETTINGS
				si479xx_setProperty(0x0e, 0x01, 0x81, 0x42, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_OUTPUT_SOURCE
				si479xx_setProperty(0x0e, 0x02, 0x81, 0x80, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_ZIF_FM_CONFIG
				si479xx_setProperty(0x0e, 0x03, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
				si479xx_setProperty(0x0e, 0x04, 0x01, 0x42, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_ZIF_AM_CONFIG0
				si479xx_setProperty(0x0e, 0x05, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);

				if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP) {
					// EZIQ_ZIF_AM_CONFIG1 192KHz
					si479xx_setProperty(0x0e, 0x06, 0x01, 0x08, eTUNER_DRV_ID_PRIMARY);

					// DRM30_DIG_AGC_CONFIG AGC on
					si479xx_setProperty(0x4d, 0x00, 0x18, 0x00, eTUNER_DRV_ID_PRIMARY);
#if 0
                    SILAB_DBG("[DRM30_DIG_AGC_SPEED change] \n");
					si479xx_setProperty(0x4d, 0x02, 0xff, 0xff, eTUNER_DRV_ID_PRIMARY);
#endif
				}
				else {
					// EZIQ_ZIF_AM_CONFIG1 744.1875KHz
					si479xx_setProperty(0x0e, 0x06, 0x01, 0x42, eTUNER_DRV_ID_PRIMARY);
				}

				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
				#ifdef USE_DAB_IQ_CLKINV
					// EZIQ_CONFIG
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x04, eTUNER_DRV_ID_PRIMARY);
				#endif
					// EZIQ_ZIF_DAB_CONFIG0
					si479xx_setProperty(0x0e, 0x07, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
					// EZIQ_ZIF_DAB_CONFIG1
					si479xx_setProperty(0x0e, 0x08, 0x01, 0x80, eTUNER_DRV_ID_PRIMARY);
				#ifdef USE_DAB_DIGITAL_AGC
					// DAB_DIG_AGC_CONFIG
					si479xx_setProperty(0x65, 0x00, 0x00, 0x10, eTUNER_DRV_ID_PRIMARY);
					si479xx_setProperty(0x65, 0x01, 0x18, 0x00, eTUNER_DRV_ID_PRIMARY);
					si479xx_setProperty(0x65, 0x02, 0x0c, 0x0c, eTUNER_DRV_ID_PRIMARY);
				#endif
				}

				// EZIQ_CONFIG
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x00, eTUNER_DRV_ID_TERTIARY);

				// EZIQ_FIXED_SETTINGS
				si479xx_setProperty(0x0e, 0x01, 0x81, 0x42, eTUNER_DRV_ID_TERTIARY);

				// EZIQ_OUTPUT_SOURCE
				si479xx_setProperty(0x0e, 0x02, 0x81, 0x80, eTUNER_DRV_ID_TERTIARY);

				// EZIQ_ZIF_FM_CONFIG
				si479xx_setProperty(0x0e, 0x03, 0x00, 0x01, eTUNER_DRV_ID_TERTIARY);
				si479xx_setProperty(0x0e, 0x04, 0x01, 0x42, eTUNER_DRV_ID_TERTIARY);

				// EZIQ_ZIF_AM_CONFIG0
				si479xx_setProperty(0x0e, 0x05, 0x00, 0x01, eTUNER_DRV_ID_TERTIARY);

				if(gSilabConf.sdr == eTUNER_SDR_DRM30 || gSilabConf.sdr == eTUNER_SDR_DRMP) {
					// EZIQ_ZIF_AM_CONFIG1 192KHz
					si479xx_setProperty(0x0e, 0x06, 0x01, 0x08, eTUNER_DRV_ID_TERTIARY);

					// DRM30_DIG_AGC_CONFIG AGC on
					si479xx_setProperty(0x4d, 0x00, 0x18, 0x00, eTUNER_DRV_ID_TERTIARY);
#if 0
					SILAB_DBG("[DRM30_DIG_AGC_SPEED change] \n");
					si479xx_setProperty(0x4d, 0x02, 0xff, 0xff, eTUNER_DRV_ID_TERTIARY);
#endif
				}
				else {
					// EZIQ_ZIF_AM_CONFIG1 744.1875KHz
					si479xx_setProperty(0x0e, 0x06, 0x01, 0x42, eTUNER_DRV_ID_TERTIARY);
				}

				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
				#ifdef USE_DAB_IQ_CLKINV
					// EZIQ_CONFIG
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x04, eTUNER_DRV_ID_TERTIARY);
				#endif
					// EZIQ_ZIF_DAB_CONFIG0
					si479xx_setProperty(0x0e, 0x07, 0x00, 0x01, eTUNER_DRV_ID_TERTIARY);
					// EZIQ_ZIF_DAB_CONFIG1
					si479xx_setProperty(0x0e, 0x08, 0x01, 0x80, eTUNER_DRV_ID_TERTIARY);
				#ifdef USE_DAB_DIGITAL_AGC
					// DAB_DIG_AGC_CONFIG
					si479xx_setProperty(0x65, 0x00, 0x00, 0x10, eTUNER_DRV_ID_TERTIARY);
					si479xx_setProperty(0x65, 0x01, 0x18, 0x00, eTUNER_DRV_ID_TERTIARY);
					si479xx_setProperty(0x65, 0x02, 0x0c, 0x0c, eTUNER_DRV_ID_TERTIARY);
				#endif
				}
			}
			break;
	}

#else	// #if SILAB_FW_VER == 40

	switch(tunertype) {
		case eTUNER_DRV_CONF_TYPE_SINGLE:
			// ZIF0/1 Output Setting
			if(gSilabConf.fIqOut) {
				si479xx_setProperty(0x09, 0x02, 0x04, 0x30, eTUNER_DRV_ID_PRIMARY);
				si479xx_setProperty(0x09, 0x01, 0x84, 0x00, eTUNER_DRV_ID_PRIMARY);
			}
			break;

		case eTUNER_DRV_CONF_TYPE_DUAL:
			// ZIF0/1 Output Setting
			if(gSilabConf.fIqOut) {
				si479xx_setProperty(0x09, 0x02, 0x04, 0x40, eTUNER_DRV_ID_PRIMARY);
				si479xx_setProperty(0x09, 0x01, 0x84, 0x00, eTUNER_DRV_ID_PRIMARY);
			}
			break;

		case eTUNER_DRV_CONF_TYPE_TRIPLE:
			//si479xx_setDacOut(eTUNER_DRV_ID_TERTIARY, 0);
			// ZIF0/1 Output Setting
			if(gSilabConf.fIqOut) {
				si479xx_setProperty(0x09, 0x02, 0x04, 0x40, eTUNER_DRV_ID_PRIMARY);
				si479xx_setProperty(0x09, 0x01, 0x84, 0x00, eTUNER_DRV_ID_PRIMARY);

				si479xx_setProperty(0x09, 0x02, 0x04, 0x30, eTUNER_DRV_ID_TERTIARY);
				si479xx_setProperty(0x09, 0x01, 0x84, 0x00, eTUNER_DRV_ID_TERTIARY);
			}
			break;

		case eTUNER_DRV_CONF_TYPE_QUAD:
			//si479xx_setDacOut(eTUNER_DRV_ID_TERTIARY, 0);
			// ZIF0/1 Output Setting
			if(gSilabConf.fIqOut) {
				si479xx_setProperty(0x09, 0x02, 0x04, 0x40, eTUNER_DRV_ID_PRIMARY);
				si479xx_setProperty(0x09, 0x01, 0x84, 0x00, eTUNER_DRV_ID_PRIMARY);

				si479xx_setProperty(0x09, 0x02, 0x04, 0x40, eTUNER_DRV_ID_TERTIARY);
				si479xx_setProperty(0x09, 0x01, 0x84, 0x00, eTUNER_DRV_ID_TERTIARY);
			}
			break;
	}

#endif	// #if SILAB_FW_VER == 40
}

RET si479xx_setIQTestPatternControl(uint32 fOnOff, uint32 sel)
{
	RET ret = eRET_OK;

#if SILAB_FW_VER == 40 || SILAB_FW_VER == 50
	if(fOnOff) {
		switch(gSilabConf.numTuners) {
			case eTUNER_DRV_CONF_TYPE_SINGLE:
				SILAB_INF("[Set IQ0 test pattern output] \n");
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					// DAB_DIG_AGC_CONFIG OFF
					si479xx_setProperty(0x65, 0x00, 0x00, 0x13, eTUNER_DRV_ID_PRIMARY);
				}
			#endif
				// EZIQ_CONFIG
			#ifdef USE_DAB_IQ_CLKINV
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x0c, eTUNER_DRV_ID_PRIMARY);
				}
				else {
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x08, eTUNER_DRV_ID_PRIMARY);
				}
			#else
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x08, eTUNER_DRV_ID_PRIMARY);
			#endif
				break;
			case eTUNER_DRV_CONF_TYPE_DUAL:
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					// DAB_DIG_AGC_CONFIG OFF
					si479xx_setProperty(0x65, 0x00, 0x00, 0x13, eTUNER_DRV_ID_PRIMARY);
				}
			#endif
				// EZIQ_CONFIG
			#ifdef USE_DAB_IQ_CLKINV
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x0c, eTUNER_DRV_ID_PRIMARY);
				}
				else {
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x08, eTUNER_DRV_ID_PRIMARY);
				}
			#else
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x08, eTUNER_DRV_ID_PRIMARY);
			#endif
				// EZIQ_OUTPUT_SOURCE
				if(sel) {
					SILAB_INF("[Set IQ1 test pattern output] \n");
					si479xx_setProperty(0x0e, 0x02, 0x80, 0x81, eTUNER_DRV_ID_PRIMARY);
				}
				else {
					SILAB_INF("[Set IQ0 test pattern output] \n");
					si479xx_setProperty(0x0e, 0x02, 0x81, 0x80, eTUNER_DRV_ID_PRIMARY);
				}
				break;
			case eTUNER_DRV_CONF_TYPE_TRIPLE:
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					// DAB_DIG_AGC_CONFIG OFF
					si479xx_setProperty(0x65, 0x00, 0x00, 0x13, eTUNER_DRV_ID_PRIMARY);
					si479xx_setProperty(0x65, 0x00, 0x00, 0x13, eTUNER_DRV_ID_TERTIARY);
				}
			#endif
				// EZIQ_CONFIG
			#ifdef USE_DAB_IQ_CLKINV
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x0c, eTUNER_DRV_ID_PRIMARY);
				}
				else {
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x08, eTUNER_DRV_ID_PRIMARY);
				}
			#else
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x08, eTUNER_DRV_ID_PRIMARY);
			#endif
				// EZIQ_OUTPUT_SOURCE
				if(sel) {
					SILAB_INF("[Set IQ1 test pattern output] \n");
					si479xx_setProperty(0x0e, 0x02, 0x80, 0x81, eTUNER_DRV_ID_PRIMARY);
				}
				else {
					SILAB_INF("[Set IQ0 test pattern output] \n");
					si479xx_setProperty(0x0e, 0x02, 0x81, 0x80, eTUNER_DRV_ID_PRIMARY);
				}

				SILAB_INF("[Set IQ2 test pattern output] \n");
				// EZIQ_CONFIG
			#ifdef USE_DAB_IQ_CLKINV
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x0c, eTUNER_DRV_ID_TERTIARY);
				}
				else {
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x08, eTUNER_DRV_ID_TERTIARY);
				}
			#else
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x08, eTUNER_DRV_ID_TERTIARY);
			#endif
				break;
			case eTUNER_DRV_CONF_TYPE_QUAD:
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					// DAB_DIG_AGC_CONFIG OFF
					si479xx_setProperty(0x65, 0x00, 0x00, 0x13, eTUNER_DRV_ID_PRIMARY);
					si479xx_setProperty(0x65, 0x00, 0x00, 0x13, eTUNER_DRV_ID_TERTIARY);
				}
			#endif
				// EZIQ_CONFIG
			#ifdef USE_DAB_IQ_CLKINV
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x0c, eTUNER_DRV_ID_PRIMARY);
				}
				else {
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x08, eTUNER_DRV_ID_PRIMARY);
				}
			#else
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x08, eTUNER_DRV_ID_PRIMARY);
			#endif
				// EZIQ_OUTPUT_SOURCE
				if(sel) {
					SILAB_INF("[Set IQ1 test pattern output] \n");
					si479xx_setProperty(0x0e, 0x02, 0x80, 0x81, eTUNER_DRV_ID_PRIMARY);
				}
				else {
					SILAB_INF("[Set IQ0 test pattern output] \n");
					si479xx_setProperty(0x0e, 0x02, 0x81, 0x80, eTUNER_DRV_ID_PRIMARY);
				}

				// EZIQ_CONFIG
			#ifdef USE_DAB_IQ_CLKINV
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x0c, eTUNER_DRV_ID_TERTIARY);
				}
				else {
					si479xx_setProperty(0x0e, 0x00, 0x00, 0x08, eTUNER_DRV_ID_TERTIARY);
				}
			#else
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x08, eTUNER_DRV_ID_TERTIARY);
			#endif
				// EZIQ_OUTPUT_SOURCE
				if(sel) {
					SILAB_INF("[Set IQ3 test pattern output] \n");
					si479xx_setProperty(0x0e, 0x02, 0x80, 0x81, eTUNER_DRV_ID_TERTIARY);
				}
				else {
					SILAB_INF("[Set IQ2 test pattern output] \n");
					si479xx_setProperty(0x0e, 0x02, 0x81, 0x80, eTUNER_DRV_ID_TERTIARY);
				}
				break;
			default:
				ret = eRET_NG_NO_RSC;
				break;
		}
	}
	else {
		switch(gSilabConf.numTuners) {
			case eTUNER_DRV_CONF_TYPE_SINGLE:
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					// DAB_DIG_AGC_CONFIG OFF
					si479xx_setProperty(0x65, 0x00, 0x00, 0x10, eTUNER_DRV_ID_PRIMARY);
				}
			#endif
				// EZIQ_CONFIG
                si479xx_setProperty(0x0e, 0x00, 0x00, 0x00, eTUNER_DRV_ID_PRIMARY);
				break;
			case eTUNER_DRV_CONF_TYPE_DUAL:
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					// DAB_DIG_AGC_CONFIG OFF
					si479xx_setProperty(0x65, 0x00, 0x00, 0x10, eTUNER_DRV_ID_PRIMARY);
				}
			#endif
				// EZIQ_CONFIG
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x00, eTUNER_DRV_ID_PRIMARY);
				// EZIQ_OUTPUT_SOURCE
				si479xx_setProperty(0x0e, 0x02, 0x81, 0x80, eTUNER_DRV_ID_PRIMARY);
				break;
			case eTUNER_DRV_CONF_TYPE_TRIPLE:
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					// DAB_DIG_AGC_CONFIG OFF
					si479xx_setProperty(0x65, 0x00, 0x00, 0x10, eTUNER_DRV_ID_PRIMARY);
					si479xx_setProperty(0x65, 0x00, 0x00, 0x10, eTUNER_DRV_ID_TERTIARY);
				}
			#endif
				// EZIQ_CONFIG
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x00, eTUNER_DRV_ID_PRIMARY);
				// EZIQ_OUTPUT_SOURCE
				si479xx_setProperty(0x0e, 0x02, 0x81, 0x80, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_CONFIG
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x00, eTUNER_DRV_ID_TERTIARY);
				break;
			case eTUNER_DRV_CONF_TYPE_QUAD:
			#ifdef USE_DAB_DIGITAL_AGC
				if(gSilabConf.sdr == eTUNER_SDR_DAB) {
					// DAB_DIG_AGC_CONFIG OFF
					si479xx_setProperty(0x65, 0x00, 0x00, 0x10, eTUNER_DRV_ID_PRIMARY);
					si479xx_setProperty(0x65, 0x00, 0x00, 0x10, eTUNER_DRV_ID_TERTIARY);
				}
			#endif
				// EZIQ_CONFIG
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x00, eTUNER_DRV_ID_PRIMARY);
				// EZIQ_OUTPUT_SOURCE
				si479xx_setProperty(0x0e, 0x02, 0x81, 0x80, eTUNER_DRV_ID_PRIMARY);

				// EZIQ_CONFIG
				si479xx_setProperty(0x0e, 0x00, 0x00, 0x00, eTUNER_DRV_ID_TERTIARY);
				// EZIQ_OUTPUT_SOURCE
				si479xx_setProperty(0x0e, 0x02, 0x81, 0x80, eTUNER_DRV_ID_TERTIARY);
				break;
			default:
				ret = eRET_NG_NO_RSC;
				break;
		}
	}

#else	// #if SILAB_FW_VER == 40

	if(fOnOff) {
		switch(gSilabConf.numTuners) {
			case eTUNER_DRV_CONF_TYPE_SINGLE:
				si479xx_setProperty(0x09, 0x02, 0x06, 0x30, eTUNER_DRV_ID_PRIMARY);
				break;
			case eTUNER_DRV_CONF_TYPE_DUAL:
				si479xx_setProperty(0x09, 0x02, 0x06, 0x00, eTUNER_DRV_ID_PRIMARY);
				break;
			case eTUNER_DRV_CONF_TYPE_TRIPLE:
				si479xx_setProperty(0x09, 0x02, 0x06, 0x00, eTUNER_DRV_ID_PRIMARY);
				si479xx_setProperty(0x09, 0x02, 0x06, 0x30, eTUNER_DRV_ID_TERTIARY);
				break;
			case eTUNER_DRV_CONF_TYPE_QUAD:
				si479xx_setProperty(0x09, 0x02, 0x06, 0x00, eTUNER_DRV_ID_PRIMARY);
				si479xx_setProperty(0x09, 0x02, 0x06, 0x00, eTUNER_DRV_ID_TERTIARY);
				break;
			default:
				ret = eRET_NG_NO_RSC;
				break;
		}
	}
	else {
		switch(gSilabConf.numTuners) {
			case eTUNER_DRV_CONF_TYPE_SINGLE:
				si479xx_setProperty(0x09, 0x02, 0x04, 0x30, eTUNER_DRV_ID_PRIMARY);
				break;
			case eTUNER_DRV_CONF_TYPE_DUAL:
				si479xx_setProperty(0x09, 0x02, 0x04, 0x40, eTUNER_DRV_ID_PRIMARY);
				break;
			case eTUNER_DRV_CONF_TYPE_TRIPLE:
				si479xx_setProperty(0x09, 0x02, 0x04, 0x40, eTUNER_DRV_ID_PRIMARY);
				si479xx_setProperty(0x09, 0x02, 0x04, 0x30, eTUNER_DRV_ID_TERTIARY);
				break;
			case eTUNER_DRV_CONF_TYPE_QUAD:
				si479xx_setProperty(0x09, 0x02, 0x04, 0x40, eTUNER_DRV_ID_PRIMARY);
				si479xx_setProperty(0x09, 0x02, 0x04, 0x40, eTUNER_DRV_ID_TERTIARY);
				break;
			default:
				ret = eRET_NG_NO_RSC;
				break;
		}
	}
#endif

	return ret;
}

RET si479xx_getPartInfo(uint32 ntuner, uint32 *name)
{
	int i;
	uint32 cmd_len=0, rx_len=0;
	RET ret = eRET_OK;

	if(name == NULL) {
		return eRET_NG_INVALID_PARAM;
	}

	*name = (uint32)0;

	si479xx_memset(cmd_buff, 0, sizeof(cmd_buff));

	cmd_buff[0] = GET_PART_INFO;
	cmd_buff[1] = 0x00;
	cmd_len = 2;
	rx_len = 19;

	ret = si479xx_command(cmd_len, cmd_buff, rx_len, rsp_buff, ntuner);

	if(ret == eRET_OK) {
		if(rsp_buff[7] == 0x96 || rsp_buff[7] == 0x00) { // Si4796x or Si4790x Family Series
			if(rsp_buff[8] >= 0x30 && rsp_buff[8] <= 0x39 && rsp_buff[9] >= 0x30 && rsp_buff[9] <= 0x39) {
				*name = 47900 + ((rsp_buff[8] & 0x0f) * 10) + (rsp_buff[9] & 0x0f);
			}
			else {
				*name = 0;	// Not supported tuner chipset
			}
		}
		else {
			*name = 0;	// Not supported tuner chipset
		}

	#if 0		// for debugging
		for(i=0; i<rx_len; i++) {
			printf("[%d]%02xh ", i, rsp_buff[i]);
		}
		printf("\n");
	#endif
	}

	return ret;
}

RET si479xx_bootup(uint32 tunertype)
{
	RET ret = eRET_OK;
	stPOWER_UP_ARGS_t * p_powerup_args = NULL;
	uint8 guid;
	SILAB_INF("Si479xx %s\n", SILAB_FW_VER_STRING);

	//power up command
	SILAB_DBG("[si479x Power Up] \n");
	p_powerup_args = si479xx_get_power_up_args(tunertype, 0, (uint8)gSilabConf.fIqOut, eTUNER_DRV_ID_PRIMARY);
	ret = si479xx_powerup(p_powerup_args, eTUNER_DRV_ID_PRIMARY);
	if(ret != eRET_OK) {
		SILAB_ERR("[si479x Power Up] Failed.\n");
		goto error_tuner_bootup;
	}

	if (tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE || tunertype == eTUNER_DRV_CONF_TYPE_QUAD) {
		p_powerup_args = si479xx_get_power_up_args(tunertype, 0, (uint8)gSilabConf.fIqOut, eTUNER_DRV_ID_TERTIARY);
		ret = si479xx_powerup(p_powerup_args, eTUNER_DRV_ID_TERTIARY);
		if(ret != eRET_OK) {
			SILAB_ERR("[si479x Power Up2] Failed.\n");
			goto error_tuner_bootup;
		}
	}

	si479xx_getPartInfo(eTUNER_DRV_ID_PRIMARY, &si479xx_1st_chipset_name);
	if(si479xx_1st_chipset_name != 0) {
		SILAB_INF("1st Tuner Chipset: Si%d\n", si479xx_1st_chipset_name);
	}
	if(gSilabConf.numTuners > eTUNER_DRV_CONF_TYPE_DUAL) {
		si479xx_getPartInfo(eTUNER_DRV_ID_TERTIARY, &si479xx_2nd_chipset_name);
		if(si479xx_2nd_chipset_name != 0) {
			SILAB_INF("2nd Tuner Chipset: Si%d\n", si479xx_2nd_chipset_name);
		}
	}

	//load init
	SILAB_DBG("[si479x Load Init] \n");
	guid = si479xx_getGuidTable(tunertype, eTUNER_DRV_ID_PRIMARY);
	ret = si479xx_loader_init(guid, guid_table, eTUNER_DRV_ID_PRIMARY);
	if(ret != eRET_OK) {
		SILAB_ERR("[si479x Load Init] Failed.\n");
		goto error_tuner_bootup;
	}

	if (tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE || tunertype == eTUNER_DRV_CONF_TYPE_QUAD) {
		guid = si479xx_getGuidTable(tunertype, eTUNER_DRV_ID_TERTIARY);
		ret = si479xx_loader_init(guid, guid_table, eTUNER_DRV_ID_TERTIARY);
		if(ret != eRET_OK) {
			SILAB_ERR("[si479x Load Init2] Failed.\n");
			goto error_tuner_bootup;
		}
	}

	//host load
	SILAB_DBG("[si479x Host Load] \n");
	ret = si479xx_host_load(tunertype, TUNER_FW);
	if(ret != eRET_OK) {
		SILAB_ERR("[si479x Host Load] Failed.\n");
		goto error_tuner_bootup;
	}

	// Delay for stability
	si479xx_mssleep(20);

	//boot
	ret = si479xx_boot(eTUNER_DRV_ID_PRIMARY);
	if(ret != eRET_OK) {
		SILAB_ERR("[si479x Boot] Failed.\n");
		goto error_tuner_bootup;
	}
	if (tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE || tunertype == eTUNER_DRV_CONF_TYPE_QUAD) {
		ret = si479xx_boot(eTUNER_DRV_ID_TERTIARY);
		if(ret != eRET_OK) {
			SILAB_ERR("[si479x Boot2] Failed.\n");
			goto error_tuner_bootup;
		}
	}

	// Delay for stability
	si479xx_mssleep(80);

	ret = si479xx_setDacOut(eTUNER_DRV_ID_PRIMARY, 1);
	if(ret != eRET_OK) {
		SILAB_ERR("Failed to set application command during bootup.\n");
		goto error_tuner_bootup;
	}

	if(gSilabConf.fExtAppCtrl != 0x11) {
		si479xx_I2S_AudioOut(eTUNER_DRV_ID_PRIMARY);
		si479xx_setI2sControl(eTUNER_DRV_ID_PRIMARY, 0x00, ON);
	}
	else {
		si479xx_I2S_AudioOut(eTUNER_DRV_ID_SECONDARY);
		si479xx_setI2sControl(eTUNER_DRV_ID_SECONDARY, 0x01, ON);
	}

	if(gSilabConf.fExtAppCtrl == 0x21) {
		si479xx_I2S_AudioOut(eTUNER_DRV_ID_TERTIARY);
		si479xx_setI2sControl(eTUNER_DRV_ID_TERTIARY, 0x00, ON);
	}

#ifdef USE_THIRD_TUNER_AS_SINGLE_TUNER
	if(gSilabConf.numTuners == eTUNER_DRV_CONF_TYPE_SINGLE) {
		si479xx_I2S_AudioOut(eTUNER_DRV_ID_TERTIARY);
		si479xx_setI2sControl(eTUNER_DRV_ID_TERTIARY, 0x00, ON);
	}
#endif

	si479xx_setConfigProperty(tunertype);

	if(gSilabConf.sdr == eTUNER_SDR_DAB) {
		si479xx_setDabFreqListToReg((uint32*)dabFreqTable_initBandIII, dabFreqTableInitSize, eTUNER_DRV_ID_PRIMARY);
		if (tunertype == eTUNER_DRV_CONF_TYPE_TRIPLE || tunertype == eTUNER_DRV_CONF_TYPE_QUAD) {
			si479xx_setDabFreqListToReg((uint32*)dabFreqTable_initBandIII, dabFreqTableInitSize, eTUNER_DRV_ID_TERTIARY);
		}
	}

//	You should control the CHANBW via servo command from FW v3.0
//	si479xx_setTunerServo(0x02, 0x00, 0x96, 0x00, 0x96, 0x00, 0x96, 0x01, eTUNER_DRV_ID_PRIMARY, 0);	// FM CHANBW
//	si479xx_setTunerCommand(0x29, 0x00, 0x00, 0x01, eTUNER_DRV_ID_PRIMARY);
//	si479xx_setTunerCommand(0x29, 0x05, 0x00, 0x96, eTUNER_DRV_ID_PRIMARY);
//	si479xx_setTunerCommand(0x29, 0x08, 0x00, 0x96, eTUNER_DRV_ID_PRIMARY);
//	si479xx_setTunerCommand(0x29, 0x0f, 0x00, 0x96, eTUNER_DRV_ID_PRIMARY);
//	si479xx_setTunerCommand(0x29, 0x10, 0x00, 0x96, eTUNER_DRV_ID_PRIMARY);

error_tuner_bootup:

	return ret;
}

