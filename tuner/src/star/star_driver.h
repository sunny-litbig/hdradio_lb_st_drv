//****************************************************************************************************************
//
// File name   :	star_driver.h  
// Author	   :	SZ team (Michael LIANG)
// Description :	ST Star Tuner(TDA7707 / TDA7708) driver header file
//
// Copyright (C) 2019-2020 STMicroelectronics - All Rights Reserved
//
// License terms: STMicroelectronics Proprietary in accordance with licensing
// terms accepted at time of software delivery 
// 
// THIS SOFTWARE IS DISTRIBUTED "AS IS," AND ALL WARRANTIES ARE DISCLAIMED, 
// INCLUDING MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// EVALUATION ONLY - NOT FOR USE IN PRODUCTION
//****************************************************************************************************************/


#ifndef H_STAR_DRIVER
#define H_STAR_DRIVER

#include "star_public.h"
#include "tcradio_types.h"
#include "tcradio_drv.h"

/* Star tuner Command Code list */

//Basic commands
#define CMD_CODE_TUNER_PING						0x00
//#define CMD_CODE_TUNER_READ		      				0x1E
#define CMD_CODE_TUNER_WRITE  					0x1F
#define CMD_CODE_TUNER_CHANGE_BAND 				0x0A
#define CMD_CODE_TUNER_TUNE						0x08
#define CMD_CODE_TUNER_SET_FM_PROC				0x11
#define CMD_CODE_TUNER_GET_RECEPTION_QUALITY 	0x12
#define CMD_CODE_TUNER_GET_CHANNEL_QUALITY	   	0x13
#define CMD_CODE_TUNER_MUTE						0x16
#define CMD_CODE_TUNER_FM_STEREO_MODE			0x17

//Seek commands
#define CMD_CODE_TUNER_SEEK_START        			0x26
#define CMD_CODE_TUNER_SEEK_END            			0x27
#define CMD_CODE_TUNER_GET_SEEK_STATUS 			0x28
#define CMD_CODE_TUNER_SET_SEEK_TRESHOLD 		0x29

//RDS commands
#define CMD_CODE_TUNER_RDSBUFFER_SET 			0x1A
#define CMD_CODE_TUNER_RDSBUFFER_READ 			0x1B

//AF check commands
#define CMD_CODE_TUNER_AF_START					0x20
#define CMD_CODE_TUNER_AF_END					0x21
#define CMD_CODE_TUNER_AF_CHECK          			0x22
#define CMD_CODE_TUNER_AF_SWITCH        			0x23
#define CMD_CODE_TUNER_GET_AF_QUALITY 			0x24

//Configuration commands
#define CMD_CODE_TUNER_SET_AUDIO_IF            		0x0D
#define CMD_CODE_TUNER_SET_BB_IF	               		0x04
#define CMD_CODE_TUNER_CONF_BB_SAI             		0x05
#define CMD_CODE_TUNER_CONF_JESD204          		0x06
#define CMD_CODE_TUNER_SET_AUDIO_IF            		0x0D
#define CMD_CODE_TUNER_SET_HD_BLEND          		0x14


//Tuner Injection side
#define INJECTION_SIDE_AUTO 				0x00
#define INJECTION_SIDE_LOW					0x01   /*Force low injection side */
#define INJECTION_SIDE_HIGH 				0x02   /*Force high injection side */

//Tuner Seek definations
#define SEEK_MANUAL						0x00
#define SEEK_AUTO   							0x01
#define SEEK_END_UNMUTED					0x00
#define SEEK_END_MUTED						0x01
#define SEEK_STATUS_SEEKOK  				0x80
#define SEEK_STATUS_FULL_CIRCLE 			0x40 
#define SEEK_KEEP_START_FREQ				0x01

#define SEEK_GET_QUALITY_CNT 				9	/*means 10 times for average signal quality in auto seek mode */


#ifdef TDA7708_TUNER
// TDA7708 seek default thresholds
#define SEEK_FM_FS_THRESHOLD    			0x0F		/* 15dBu */
#define SEEK_FM_DT_THRESHOLD    			0x1B		/* 5.3kHz */
#define SEEK_FM_MPXN_THRESHOLD 			0x60  		/* 38% */
#define SEEK_FM_MP_THRESHOLD  			0xA5		/* 65% */
#define SEEK_FM_SNR_THRESHOLD				0x4C		/* 30% */
#define SEEK_FM_ADJ_THRESHOLD 			0x64		/* 79% */
#define SEEK_FM_COCH_THRESHOLD			0xFF		/* 100% */

#define SEEK_AM_FS_THRESHOLD				0x14   		/* 20dBu */
#define SEEK_AM_DT_THRESHOLD   			0x28 		/* 0.8kHz */
#define SEEK_AM_SNR_THRESHOLD				0x66		/* 40% */
#define SEEK_AM_ADJ_THRESHOLD   			0x46		/* 55.5% */

#define SEEK_WX_FS_THRESHOLD				0x1C  		/* 28dBu */
#define SEEK_WX_DT_THRESHOLD				0x26		/* 1.5kHz */

#else
// TDA7707 seek default  thresholds
#define SEEK_FM_FS_THRESHOLD    			0x0F		/* 15dBu */
#define SEEK_FM_DT_THRESHOLD    			0x1B		/* 5.3kHz */
#define SEEK_FM_MPXN_THRESHOLD 			0x60  		/* 38% */
#define SEEK_FM_MP_THRESHOLD  			0xC1		/* 76% */
#define SEEK_FM_SNR_THRESHOLD				0x3F		/* 25% */
#define SEEK_FM_ADJ_THRESHOLD 			0x64		/* 79% */
#define SEEK_FM_COCH_THRESHOLD			0xFF		/* 100% */

#define SEEK_AM_FS_THRESHOLD				0x14   		/* 20dBu */
#define SEEK_AM_DT_THRESHOLD   			0x28 		/* 0.8kHz */
#define SEEK_AM_SNR_THRESHOLD				0x66		/* 40% */
#define SEEK_AM_ADJ_THRESHOLD   			0x46		/* 55.5% */

#define SEEK_WX_FS_THRESHOLD				0x1C  		/* 26dBu */
#define SEEK_WX_DT_THRESHOLD				0x26		/* 1.5kHz */
#endif

//definations for manual seek algo
#define SEEK_FM_FS_SCALMIN_MPXN 			0x0E
#define SEEK_FM_FS_SCALMAX_MPXN 			0x20
#define SEEK_FM_FS_SCALMIN_MP				0x18
#define SEEK_FM_FS_SCALMAX_MP				0x3C
#define SEEK_FM_FS_SCALMIN_ADJ			0x14
#define SEEK_FM_FS_SCALMAX_ADJ			0x28

#define SEEK_FM_FS_THRESHOLD_DELTA  		0x05
#define SEEK_FM_MPXN_FACTOR				0.4
#define SEEK_FM_MP_FACTOR					0.5
#define SEEK_FM_ADJ_FACTOR				0.7

#define SEEK_FM_WAIT_T1 					5
#define SEEK_FM_WAIT_T2					1
#define SEEK_FM_WAIT_T3					15
#define SEEK_FM_WAIT_T4					1
#define SEEK_FM_GET_QUALITY_CNT_N1 		2
#define SEEK_FM_GET_QUALITY_CNT_N2 		10

#define SEEK_AMWX_WAIT_T1 				70
#define SEEK_AMWX_WAIT_T2					1
#define SEEK_AMWX_GET_QUALITY_CNT_N1 	10


//RDS 
#define RDS_NORMALMODE_NRQST				0x08
#define RDS_CSR_ERRTHRESH					0x00  	/*ERRTHRESH bit 4~6:  0x6: ignore the filter, all blocks will be saved to buffer*/
#define RDSBUFFER_WORDS_MAXNUM			0x10

#define STAR_ADDRESS_SCSR0				0x20100
#define STAR_ADDRESS_SCSR1				0x20101
#define STAR_ADDRESS_SCSR2				0x20102
#define STAR_ADDRESS_PRECHECKING			0x1401E
#define STAR_ADDRESS_PRESETTING			0x14008
#define STAR_ADDRESS_CMDBUFFER			0x20180


typedef struct 
{
	tU8 fieldStrengthTH;
	tU8 detuningTH;
	tU8 snrTH;
	tU8 adjTH;
	tU8 mpTH;
	tU8 mpxNoiseTH;
	tU8 coChTH;
} FM_SeekTH;


typedef struct 
{
	tU8 fieldStrengthTH;
	tU8 detuningTH;
	tU8 snrTH;
	tU8 adjTH;
} AM_SeekTH;


typedef struct 
{
	tU8 fieldStrengthTH;
	tU8 detuningTH;
} WX_SeekTH;


union Tun_SeekTH
{
	FM_SeekTH	fmSeekTH;
	AM_SeekTH	amSeekTH;
	WX_SeekTH	wxSeekTH;
};


typedef struct 
{
	int AFnumber;
	int AFfrequency ;
	int AFquality_fs ;
} AF_Des;


typedef enum 
{
	RDSBUFFER_DISABLE = 0,
	RDSBUFFER_ENABLE,
}RDS_Action;


typedef struct
{
	int   validBlockNum;
	tU8 blockdata[RDSBUFFER_WORDS_MAXNUM * 3];
} RDS_Buffer;

typedef struct
{
	eTUNER_DRV_MOD_MODE_t	bandMode;		/* Band Mode */
	unsigned int maxFreq;		/* Max freq */
	unsigned int minFreq;		/* Min freq */
	int seekStep;		/* seek step */
	int tuneStep;	  	/* For step up/down */
	int tunerBandID;	/* Tuner band ID used in Tuner Star command 'change band'*/
	unsigned int bandFreq;		/* When change band, default tune to this default frequency. */
}stSTAR_BAND_INFO_t;

typedef union {
	struct {
		unsigned int FstRF;		// Field strength before AGC (8bit int signed [dBuV])
		unsigned int FstBB;		// Field strength after AGC (8bit int signed [dBuV])
		unsigned int Det;			// Detuning (8bit int unsigned)
		unsigned int Snr;			// FM RF carrier to noise (8bit int unsigned [%])
		unsigned int Adj;			// FM Adjacent channel noise (8bit int signed [%])
		unsigned int Dev;			// FM Deviation (7bit int unsigned)
		unsigned int Mpth;		// FM Multipath (8bit int unsigned [%])
		unsigned int MpxNoise;	// FM Mpx Noise (8bit int unsigned [%])
		unsigned int Stereo;		// FM Stereo/Mono (1: Stereo, 0: Mono)
		unsigned int CoCh;		// FM VPA Co-channel (8bit int unsigned [%], only for foreground channel)
	} fm;

	struct {
		unsigned int FstRF;		// Field strength before AGC (8bit int signed [dBuV])
		unsigned int FstBB;		// Field strength after AGC (8bit int signed [dBuV])
		unsigned int Det;			// Detuning (8bit int unsigned)
		unsigned int Snr;			// FM RF carrier to noise (8bit int unsigned [%])
		unsigned int Adj;			// FM Adjacent channel noise (8bit int signed [%])
		unsigned int Dev;			// FM Deviation (7bit int unsigned)
	} am;

	struct {
		unsigned int FstRF;		// Field strength before AGC (8bit int signed [dBm])
		unsigned int FstBB;		// Field strength after AGC (8bit int signed [dBm])
	} dab;

	struct {
		unsigned int FstRF;		// Field strength before AGC (8bit int signed [dBuV])
		unsigned int FstBB;		// Field strength after AGC (8bit int signed [dBuV])
	} wx;
}stSTAR_DRV_QUALITY_t;


#if 0   // not use
Tun_Status TUN_Set_Seek_Threshold (tU8 deviceAddress, int channelID, union Tun_SeekTH seekThr);
Tun_Status TUN_Seek_Sart (tU8 deviceAddress, int channelID, Seek_Direction seekDirection, tU32 seekStep);
Tun_Status TUN_Seek_Stop (tU8 deviceAddress, int channelID, tBool bUnmuteAudio);
Tun_Status TUN_Get_Seek_Status (tU8 deviceAddress, int channelID, Tun_SeekStatus *pSeekStatus);
Tun_Status TUN_Seek_Continue (tU8 deviceAddress, int channelID, Seek_Direction seekDirection);
Tun_Status TUN_Ping (tU8 deviceAddress);
Tun_Status TUN_Set_Stereo_Mode (tU8 deviceAddress, Stereo_Mode stereoMode);
Tun_Status TUN_Set_VPAMode(tU8 deviceAddress, Switch_Mode VPAmode);
Tun_Status TUN_Get_ReceptionQuality (tU8 deviceAddress, int channelID, union Tun_SignalQuality *pQuality);
Tun_Status TUN_conf_JESD204(tU8 deviceAddress, tU32 mode, tU32 config, tU32 testnum, tU32 testchar, tU32 ilam, tU32 ilak);
Tun_Status TUN_Set_Blend(tU8 deviceAddress, tU8 blendMode);
Tun_Status TUN_Set_RDS (tU8 deviceAddress, int channelID, RDS_Action rdsAction);
Tun_Status TUN_Read_RDS (tU8 deviceAddress, int channelID, RDS_Buffer *pRDSbuffer);

Tun_Status TUN_AF_Start (tU8 deviceAddress, int channelID, tU32 alterFreq, tU32 antSelection, AF_SignalQuality *pAFquality);
Tun_Status TUN_AF_End(tU8 deviceAddress, int channelID, tU32 freqAfterAFEnd, AF_SignalQuality *pAFquality);
Tun_Status TUN_AF_Check (tU8 deviceAddress, int channelID, tU32 frequency, AF_SignalQuality *pAFquality);
Tun_Status TUN_AF_Switch (tU8 deviceAddress, int channelID, tU32 frequency);
Tun_Status TUN_Get_AFquality (tU8 deviceAddress, int channelID, AF_SignalQuality *pAFquality);
Tun_Status TUN_Wait_Ready(tU8 deviceAddress, int channelID, int msTimeOut);
#endif
Tun_Status TUN_Cmd_Write(tU8 deviceAddress, tU32 regAddress, tU32 regData);
Tun_Status TUN_Change_Band (tU8 deviceAddress, int channelID, int bandMode, tU32 maxFreq, tU32 minFreq, int seekStep, int VPAMode);
Tun_Status TUN_Change_Frequency (tU8 deviceAddress, int channelID, tU32 frequency);
#if 0
Tun_Status TUN_Get_ChannelQuality (tU8 deviceAddress, int channelID, tBool bVPAMode, union Tun_SignalQuality *pQuality);
#else
Tun_Status TUN_Get_ChannelQuality (tU8 deviceAddress, int channelID, tBool bVPAMode, stSTAR_DRV_QUALITY_t *pQuality);
#endif
Tun_Status TUN_Mute (tU8 deviceAddress, Mute_Action muteAction);
Tun_Status TUN_conf_BB_SAI(tU8 deviceAddress, tU32 mode, tU32 config);
Tun_Status TUN_Set_Audio_IF(tU8 deviceAddress, tU8 SAIMode, tU32 SAIConfig);
Tun_Status TUN_Set_BB_IF(tU8 deviceAddress, tU32 BBConfig);
Tun_Status TUN_Get_TunedFreq(tU8 deviceAddress, int channelID, tU32 *pFreq);

#if 0
Tun_Status TUN_Download_BootCode(tU8 deviceAddress, Device_Type deviceType);
#else
Tun_Status TUN_Download_BootCode(tU8 deviceAddress);
#endif
Tun_Status TUN_Download_CustomizedCoeffs(tU8 deviceAddress);

/***************************************************
*           interface with tcradio_hal             *
****************************************************/

unsigned int star_getVersion(void);
float star_getPreciseIqSampleRate(unsigned int ntuner);
int star_getIqSampleRate(unsigned int ntuner);
int star_getIqSamplingBit(unsigned int ntuner);
int star_setTune(unsigned int mod_mode, unsigned int freq, unsigned int tune_mode, unsigned int ntuner);
int star_getTune(unsigned int *mod_mode, unsigned int *curfreq, unsigned int ntuner);
int star_getQuality(unsigned int mod_mode, stSTAR_DRV_QUALITY_t *qdata, unsigned int ntuner);
int star_setMute(unsigned int fOnOff, unsigned int ntuner);
int star_open(stTUNER_DRV_CONFIG_t type);
int star_close(void);
int star_setIQTestPattern(unsigned int fOnOff, unsigned int sel);

#endif /* H_STAR_DRIVER */


