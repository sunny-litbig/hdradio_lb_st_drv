//****************************************************************************************************************
//
// File name   :	radio_public.h  
// Author	   :	SZ team
// Description :	ST RADIO reference application software header file
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


#ifndef H_RADIO_PUBLIC_H
#define H_RADIO_PUBLIC_H

/*  Radio general definations  */
#ifndef	TRUE
#define TRUE        1
#endif

#ifndef FALSE
#define FALSE       0
#endif

#ifndef NULL
#define NULL        0
#endif

#ifndef INVALID
#define INVALID     0xFF
#endif

#define PRESET_MAX_NUM			6		/*Preset max num supported */

/* types general definations,  may need moved common header file */
typedef unsigned char       tU8;
typedef signed char         tS8;
typedef unsigned short      tU16;
typedef unsigned int        tU32;
typedef unsigned char       tBool;

// Error definations
typedef enum
{
	RET_SUCCESS               	= 0x0,	/* No error */
	RET_ERROR                   	,	/* Generic error */

	RET_ERR_CMD_ID 		,	/* Command ID error */
	RET_ERR_CMD_CHECKSUM ,	/* Command checksum error */
	RET_ERR_CMD_COLLISION ,	/* Command collision */
	RET_ERR_CMD_ANWSER_CHECKSUM ,	/* Command answer checksum error */
	RET_ERR_NO_DATA ,
	RET_ERR_PARMETER		,	/* Function parameter error */
	RET_ERR_TIME_OUT		,  	/* For time out case */
	RET_ERR_BOOTCODE		,	/* tuner bootcode download error */
	RET_ERR_SYS_FAILURE 	,	/* System failure, need reboot */
	RET_ERR_BUSY,
	RET_ERR_I2C_COMM,       // I2C read / write communication error
	
	RET_ERR_MAX_NUM
} Tun_Status;

#if 0
typedef enum 
{
	OFF = 0,
	ON = 1
}Switch_Mode;
#endif

/* Seek direction defination, please don't change the values. */
typedef enum _SEEK_DIRECTION_
{
	SEEK_UP = 0,
	SEEK_DOWN = 1
} Seek_Direction;

/* Audio mode: mono, stereo for Star tuner */
typedef enum 
{
	AUTO_MODE = 0,
	FORCE_MONO,
	FORECE_STEREO		/* For test, normally not used */
} Stereo_Mode;

/* Mute, unmute defination for Star tuner */
typedef enum 
{
	MUTE  		= 0x0,
	UNMUTE 	= 0x03
} Mute_Action;

typedef enum
{
	MODE_NONE = 0,
	MODE_FM = 1,
	MODE_WX,
	MODE_AM,
	MODE_DAB,
	MODE_ALL,
} Band_Mode;

typedef struct _FMAM_BAND_INFOR_
{
	char		bandName[6];	/* FM1, FM2,  WX, AMMW, AMLW, AMSW,  etc */	
	tU32  	maxFreq;		/* Max freq */
	tU32		minFreq;		/* Min freq */
	int  		seekStep;		/* seek step */
	int  		tuneStep;	  	/* For step up/down */
	Band_Mode bandMode;	/* Band Mode */
	
	int 		tunerBandID;	/* Band ID used in Tuner Star command 'change band'*/
	tU32  	bandFreq;		/* When change band, default tune to this default frequency. */
	tU32  	presetFreq[PRESET_MAX_NUM];	/* PreSet Freq list */
} FMAM_BandInfor;

typedef enum
{
	/* Normal reception mode,  
		- can check DSPStuck if needed. 
		- can do seek, tune, mute/unmute, VPA on/off, etc
		- can response RDS requests */
	RADIO_FMAM_IDLE,	


	/* Tuner works in seek mode, when other request received, 
	    will break the seek mode first (send breakSeek request to seek task, wait seek task till idle mode), 
	    then do the request. */
	RADIO_FMAM_SEEK,              /* FMAM Seek*/
	RADIO_FMAM_AUTO_STORE, /* FMAM auto store */

	RADIO_FMAM_LEARN,  /* FMAM learn */

	/* FMAM preset scan, tune the frequencies in the prestore list one by one, duration : for example 30s.
	    When other request received, directly do the request. (no break action) */
	RADIO_FMAM_PRESET_SCAN,  // preset scan,  same as the SCAN feature on CAP6+

	RADIO_FMAM_BG_SCAN,
	
	RADIO_FMAM_RDS_BUSY,
	
	RADIO_FMAM_WORKMODE_NUM
} Radio_FMAM_WorkMode;

/* For FM message information. */
typedef struct _FM_SIGNAL_INFOR_
{
	tS8  fstRF;
	tS8	fieldStrength; //FstBB
	tU8	detuning;
	tU8	snr;
	tS8	adj;
	tU8  deviation;
	tU8	mp; 
	tU8	mpxNoise;
	int	stereo;		/* 1 : Stereo */
	tU8  coCh;
}FM_SignalQuality;

typedef struct _AM_SIGNAL_INFOR_
{
	tS8  fstRF;
	tS8 	fieldStrength;
	tU8  detuning;
	tU8  snr;
	tS8  adj;
	tU8  deviation;
}AM_SignalQuality;

union Tun_SignalQuality
{
	FM_SignalQuality	fmQuality;
	AM_SignalQuality	amQuality;
//	WX_SignalQuality	wxQuality;
//	DAB_SignalQuality	dabQuality;
};

typedef struct _SEEK_STATUS_DES_
{
	tU32   frequency;
	tBool  stationFound;
	tBool  fullCycleScanned;
	union Tun_SignalQuality quality;
#ifdef RDS_ENABLE
	tU16	  PI;
	char	  PS[9];
	tBool TP;
	tBool TA;	
#endif
} Tun_SeekStatus;

#endif /* H_RADIO_PUBLIC_H */



