//****************************************************************************************************************
//
// File name   :    star_driver.c 
// Author       :    SZ team (Michael LIANG)
// Description :    ST Star Tuner(TDA7707 / TDA7708) Tuner control drivers.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <errno.h>

#include "star_gpio.h"
#include "star_public.h"
#include "star_protocol.h"
#include "star_driver.h"

// Boot Code for Telechips HD Radio
#include "TDA7707_OM_v7.22.0.h"
#include "TDA7707_OM_v7.22.0.boot.h"

#define I2C_SLAVE_ADDRESS   0x61
#define STAR_MAX_TUNER_IC_NUM   (4)

#define    STAR_DRIVER_VER_PREFIX__            'T'
#define    STAR_DRIVER_VER_RELEASE_NUMBER__    0x01U
#define    STAR_DRIVER_VER_MAJOR_NUMBER__        0x00U
#define    STAR_DRIVER_VER_MINOR_NUMBER__        0x00U

stTUNER_DRV_CONFIG_t gStarConf;

unsigned int star_drv_initialized = 0;
unsigned int star_drv_current_band[STAR_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};
unsigned int star_drv_fm_frequency[STAR_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};
unsigned int star_drv_am_frequency[STAR_MAX_TUNER_IC_NUM] = {-1, -1, -1, -1};

stSTAR_BAND_INFO_t stAreaBands[eTUNER_DRV_CONF_AREA_MAX][2] =
{
    {   // ASIA
        /*          BandMode, MaxFreq, MinFreq, SeekStep, TuneStep, TunerBandID, BandFreq*/
        { eTUNER_DRV_FM_MODE,  108000,   87500,      100,      100,        0x01,    87500},
        { eTUNER_DRV_AM_MODE,    1620,     520,        9,        9,        0x06,      520}
    },
    {   // EU
        /*          BandMode, MaxFreq, MinFreq, SeekStep, TuneStep, TunerBandID, BandFreq*/
        { eTUNER_DRV_FM_MODE,  108000,   87500,      100,       50,        0x01,     87500},
        { eTUNER_DRV_AM_MODE,    1620,     522,        9,        9,        0x05,      522}
    },
    {   // NA
        /*          BandMode, MaxFreq, MinFreq, SeekStep, TuneStep, TunerBandID, BandFreq*/
        { eTUNER_DRV_FM_MODE,  107900,   87500,      200,      200,        0x01,    87500},
        { eTUNER_DRV_AM_MODE,    1710,     530,       10,       10,        0x06,      530}    /* 1720 -> 1710 TDA7707&7708 MCU guidelines*/
    }
};

/* STAR DRIVER DEBUG INFORM OPTIONS */
//#define TRACE_STAR_CMD_WRITE
//#define TRACE_STAR_CMD_CHANGE_BAND
//#define TRACE_STAR_CMD_TUNE
//#define TRACE_STAR_CMD_SEEK
//#define TRACE_STAR_CMD_PING
//#define TRACE_STAR_CMD_FM_STEREO_MODE
//#define TRACE_STAR_CMD_MUTE
//#define TRACE_STAR_CMD_SET_VPA
//#define TRACE_STAR_CMD_GET_RECEPTION_QUALITY
//#define TRACE_STAR_CMD_SET_BB_IF
//#define TRACE_STAR_CMD_CONF_BB_SAI
//#define TRACE_STAR_CMD_CONF_JESD204
//#define TRACE_STAR_CMD_RDS
//#define TRACE_STAR_TUN_WAIT_READY
//#define TRACE_STAR_TUN_GET_TUNER_BUSY_STATUS
//#define TRACE_STAR_TUN_GET_TUNEDFREQ

#if 0
/*************************************************************************************
Function        : Star_Get_TunerBusyStatus
Description    : Get Tuner busy status by retrieving the busy bit in the STAR_ADDRESS_SCSR0 of star tuner.

Parameters    : 
        deviceAddress :  star tuner I2C address.
        channelID : star tuner channel ID.
            1  :  foreground channel
            2  :  background channel
            
        pBusyStatus : tuner busy status will be saved to this momery that this pointer pointed.

Return Value    : Tun_Status
*************************************************************************************/
Tun_Status Star_Get_TunerBusyStatus(tU8 deviceAddress, int channelID, int *pBusyStatus)
{
    Tun_Status tunerStatus = RET_ERROR;
    int  dataSize = 3;
    tU8 data[dataSize];
    
    assert((channelID == 1) ||(channelID == 2));
    memset(data, 0x00 , dataSize);
    
#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_I2C_Direct_Read(deviceAddress,  STAR_ADDRESS_SCSR0, data, dataSize);

#ifdef TRACE_STAR_TUN_GET_TUNER_BUSY_STATUS    
    PRINTF("Star_I2C_Direct_Read = %s, deviceAddress = 0x%02x, RegAddress = 0x%06x, dataSize = %d", RetStatusName[tunerStatus].Name, deviceAddress, STAR_ADDRESS_SCSR0, dataSize);
#endif

    if (tunerStatus == RET_SUCCESS)
    {
        if (channelID == 1) 
        {
            *pBusyStatus = data[2] & 0x01;
        }
        else if (channelID == 2)
        {
            *pBusyStatus = (data[1] >> 4) & 0x01;
        }
        tunerStatus = RET_SUCCESS;
    }
#endif

    return tunerStatus; 
}


/*************************************************************************************
Function        : TUN_Set_Seek_Threshold
Description    : This command sets the thresholds for automatic seek stop for FM, AM and WX band.
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel

        seekThr :
            union Tun_SeekTH
            {
                FM_SeekTH    fmSeekTH;        - for FM seek thresholds
                AM_SeekTH    amSeekTH;        - for AM seek thresholds
                WX_SeekTH    wxSeekTH;        - for WX seek thresholds
            };

Return Value    : Tun_Status                    
*************************************************************************************/
Tun_Status TUN_Set_Seek_Threshold (tU8 deviceAddress, int channelID, union Tun_SeekTH seekThr)
{
    Tun_Status tunerStatus = RET_ERROR;

    int cmdID = CMD_CODE_TUNER_SET_SEEK_TRESHOLD;
    int cmdParamNum = 4;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    
    paramData[2] = channelID;
    paramData[4] = seekThr.fmSeekTH.fieldStrengthTH;
    paramData[5] = seekThr.fmSeekTH.detuningTH;
    paramData[6] = seekThr.fmSeekTH.mpTH;            /* used only for FM */
    paramData[7] = seekThr.fmSeekTH.mpxNoiseTH;        /* used only for FM */
    paramData[8] = seekThr.fmSeekTH.snrTH;            /* used only for FM, AM */
    paramData[9] = seekThr.fmSeekTH.adjTH;            /* used only for FM, AM */
    paramData[10] = seekThr.fmSeekTH.coChTH;            /* used only for FM VPA */

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_SEEK        
    PRINTF("Star_Command_Communicate = %s, cmdParamNum = %d, fsTH = 0x%02x, detTH = 0x%02x, mpTH = 0x%02x, mpxTH = 0x%02x, snrTH= 0x%02x, adjTH = 0x%02x", 
            RetStatusName[tunerStatus].Name, cmdParamNum, paramData[4], paramData[5], paramData[6], paramData[7], paramData[8], paramData[9]);
#endif
    
#else
#endif    

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Seek_Sart
Description    : This function initiates a full automatic seek procedure including quality measurement 
            and evaluation (automatic mode).
            Supported bands: FM, AM, WX, DAB (manual mode only).
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel

        seekDirection :  Seek up or Seek down
        seekStep :  seek step [kHz] (allowed for all bands) or   seek frequency [kHz] (for DAB B3 and DAB L-band)
                        
Return Value    : Tun_Status                    
*************************************************************************************/
Tun_Status TUN_Seek_Sart (tU8 deviceAddress, int channelID, Seek_Direction seekDirection, tU32 seekStep)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_SEEK_START;    
    int cmdParamNum = 3;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */
#ifdef FMAM_SEEK_MANUAL_MODE
    tU32 seekConfig =  SEEK_MANUAL;
#else
    tU32 seekConfig =  SEEK_AUTO;
#endif

    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);

    seekConfig |=  seekDirection << 1;
    seekConfig |=  SEEK_END_MUTED << 2;
    
#ifndef FMAM_SEEK_MANUAL_MODE
    seekConfig |=  SEEK_GET_QUALITY_CNT << 4;
#endif
    
    paramData[2] = channelID;
    paramData[4] = (seekConfig & 0xFF00) >> 8;
    paramData[5] = (seekConfig & 0xFF);
    paramData[8] = seekStep;    

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);
    
#ifdef TRACE_STAR_CMD_SEEK        
    PRINTF("Star_Command_Communicate = %s, cmdParamNum = %d", RetStatusName[tunerStatus].Name, cmdParamNum);
#endif

#else
#endif    
    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Seek_Stop
Description    : This function can be either used to leave Seek mode when a frequency with an acceptable quality is found, 
            or to terminate automatic seek mode.
            Supported bands: FM, AM, WX, DAB (manual mode only)
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel
                
        bUnmuteAudio :     Seek end audio status
                FALSE: exit seek and unmute (default)
                TRUE: exit seek and keep audio muted
                
Return Value    : Tun_Status                        
*************************************************************************************/
Tun_Status TUN_Seek_Stop (tU8 deviceAddress, int channelID, tBool bUnmuteAudio)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_SEEK_END;    
    int cmdParamNum = 2;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    
    paramData[2] = channelID;
    paramData[5] =  (bUnmuteAudio) ? SEEK_END_UNMUTED : SEEK_END_MUTED;

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);
    
#ifdef TRACE_STAR_CMD_SEEK        
    PRINTF("Star_Command_Communicate = %s, cmdParamNum = %d", RetStatusName[tunerStatus].Name, cmdParamNum);
#endif
    
#else
#endif    

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Get_Seek_Status
Description    : This function returns the status of a currently active seek operation together with 
            quality information of the most recently tested frequency.
            Seek status:
                frequency, station good or not, full cycle scanned or not.
            The quality information consists of:
                RF fieldstrength, baseband fieldstrength, Detune, Multipath, Adjacent Channel  
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel
            
        pSeekStatus: Return the seek status include signal quality.
                
Return Value    : Tun_Status                        
*************************************************************************************/
Tun_Status TUN_Get_Seek_Status (tU8 deviceAddress, int channelID, Tun_SeekStatus *pSeekStatus)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_GET_SEEK_STATUS;    
    int cmdParamNum = 1;
    int ansParmNum = 4;
    int realAnsParamNum = 0;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    
    paramData[2] = channelID;

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);
    
#ifdef TRACE_STAR_CMD_SEEK        
    PRINTF("Star_Command_Communicate = %s, answerData = 0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x   0x%02x 0x%02x 0x%02x   0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x", RetStatusName[tunerStatus].Name, 
        answerData[0], answerData[1], answerData[2],  answerData[3], answerData[4], answerData[5],
        answerData[6], answerData[7], answerData[8],  answerData[9], answerData[10], answerData[11],
        answerData[12], answerData[13], answerData[14], answerData[15], answerData[16], answerData[17]);
#endif
        
    if ((tunerStatus == RET_SUCCESS) && (realAnsParamNum >= 4))
    {
        pSeekStatus->stationFound = (answerData[3] & 0x80) ? TRUE : FALSE;
        pSeekStatus->fullCycleScanned = (answerData[3] & 0x40) ? TRUE : FALSE;
        pSeekStatus->frequency = (answerData[3] & 0x1F) << 16;
        pSeekStatus->frequency |= (answerData[4] & 0xFF) << 8;
        pSeekStatus->frequency |= (answerData[5] & 0xFF);
        
        pSeekStatus->quality.fmQuality.fieldStrength = answerData[7] ;    /* fstBB */
        pSeekStatus->quality.fmQuality.detuning = answerData[8];
        pSeekStatus->quality.fmQuality.mp = answerData[9];            /* available only for FM*/
        pSeekStatus->quality.fmQuality.mpxNoise = answerData[10];    /* available only for FM */
        pSeekStatus->quality.fmQuality.snr = answerData[11];            /* available only for FM */
        pSeekStatus->quality.fmQuality.adj = answerData[12];            /* available only for FM AM*/
    }
#else
#endif    

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Seek_Continue
Description    : for automatic seek mode, use this function to continue seek without changing the last seek frequency.
            Supported bands: FM, AM, WX, DAB (manual mode only).
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel

        seekDirection :  Seek up or Seek down
                        
Return Value    : Tun_Status                    
*************************************************************************************/
Tun_Status TUN_Seek_Continue (tU8 deviceAddress, int channelID, Seek_Direction seekDirection)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_SEEK_START;    
#ifdef FMAM_SEEK_MANUAL_MODE
    int cmdParamNum = 1;
#else
    int cmdParamNum = 2;
    tU32 seekConfig =  SEEK_AUTO;
#endif
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);

    paramData[2] = channelID;

#ifndef FMAM_SEEK_MANUAL_MODE
    seekConfig |=  seekDirection << 1;
    seekConfig |=  SEEK_END_MUTED << 2;
    seekConfig |=  SEEK_GET_QUALITY_CNT << 4;
    seekConfig |=  SEEK_KEEP_START_FREQ << 8;
    paramData[4] = (seekConfig & 0xFF00) >> 8;
    paramData[5] = (seekConfig & 0xFF);
#endif

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);
    
#ifdef TRACE_STAR_CMD_SEEK        
    PRINTF("Star_Command_Communicate = %s, cmdParamNum = %d", RetStatusName[tunerStatus].Name, cmdParamNum);
#endif
    
#else
#endif    

    return tunerStatus;
}


/*************************************************************************************
Function         : TUN_Ping
Description    : This function is used to verify the "health status¡± of the tuner. 
            The car-radio MCU needs to periodically verify that the tuner is still operating correctly and is not "stuck¡±
            (for example as a consequence of an ESD to the car-radio in case the layout does not sufficiently protect the tuner): 
            to do so, the present command can be sent with an arbitrary 24 bit word as its parameter. 
            If the tuner is still operating correctly, it replies with a bit-inversed version of the word sent by the MCU.
            
Parameters    :
        deviceAddress :  star tuner I2C address.
            
Return Value    : Tun_Status
*************************************************************************************/
Tun_Status TUN_Ping (tU8 deviceAddress)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_PING;    
    int cmdParamNum = 1;
    int ansParmNum = 1;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */
    static tU32 checkData = 0x0;
    tU32 biData = 0;

    checkData++;
    if (checkData > 0xFFFFFF) checkData = 0x0;
    biData = (~checkData) & 0xFFFFFF;

    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[0] = (checkData >> 16) & 0xFF;
    paramData[1] = (checkData >> 8) & 0xFF;
    paramData[2] = (checkData & 0xFF) ;
    
#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);
    
#ifdef TRACE_STAR_CMD_PING    
    PRINTF("Star_Command_Communicate = %s, cmdParamNum = %d, checkData = 0x%06x", RetStatusName[tunerStatus].Name, cmdParamNum, checkData);
#endif
    
    if (tunerStatus == RET_SUCCESS)
    {
        biData   = answerData[3] << 16;
        biData |= answerData[4] << 8;
        biData |= answerData[5];
        if  (biData != ( (~checkData) & 0xFFFFFF))  tunerStatus = RET_ERR_SYS_FAILURE;
    }
#else
#endif

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Set_Stereo_Mode
Description    : This function is used to force the mono or stereo operation mode of the tuner with FM mode
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        stereoMode :
            0x000000: auto mono-stereo reception
            0x000001: force mono reception
            (0x000002: force stereo reception, not used)
            
Return Value    : Tun_Status
*************************************************************************************/
Tun_Status TUN_Set_Stereo_Mode (tU8 deviceAddress, Stereo_Mode stereoMode)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_FM_STEREO_MODE;    
    int cmdParamNum = 1;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = stereoMode;

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);
    
#ifdef TRACE_STAR_CMD_FM_STEREO_MODE        
    PRINTF("Star_Command_Communicate = %s, cmdParamNum = %d, stereoMode = %d", RetStatusName[tunerStatus].Name, cmdParamNum, stereoMode);
#endif
    
#else
#endif    

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Mute
Description    : This function is used to mute or unmute the audio outputs
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        muteAction :
                0x000000: mute audio
                0x000003: unmute audio
Return Value    : Tun_Status        
*************************************************************************************/
Tun_Status TUN_Mute (tU8 deviceAddress, Mute_Action muteAction)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_MUTE;    
    int cmdParamNum = 1;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = muteAction;

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);
    
#ifdef TRACE_STAR_CMD_MUTE    
    PRINTF("Star_Command_Communicate = %s, cmdParamNum = %d, muteAction = %d", RetStatusName[tunerStatus].Name, cmdParamNum, muteAction);
#endif
    
#else
#endif    
    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Set_VPAMode
Description    : For FM reception, switches operation mode dynamically and seamlessly between phase diversity (VPA) and dual tuner.
            To switch to VPA operation, both channels have to be previously set to FM mode and tuned to the same frequency.
            
Parameters     :
        deviceAddress :  star tuner I2C address.
        VPAmode : FM Tuner proc mode,  
            1 : VPA mode    (VPA on)
            0 : dual tuner mode, (VPA off)
            
Return Value    : Tun_Status            
*************************************************************************************/
Tun_Status TUN_Set_VPAMode(tU8 deviceAddress, Switch_Mode VPAmode)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_SET_FM_PROC;    
    int cmdParamNum = 1;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = VPAmode;

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);
    
#ifdef TRACE_STAR_CMD_SET_VPA    
    PRINTF("Star_Command_Communicate = %s, cmdParamNum = %d, VPAmode = %d", RetStatusName[tunerStatus].Name, cmdParamNum, VPAmode);
#endif
    
#else
#endif    
    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Get_ReceptionQuality
Description    : This function returns the AM/FM/WX quality information of the foreground 
            or background channel during normal reception mode (thus not including Seek nor RDS AF check)
            after the algorithms that improve reception quality.
            consisting of: RF fieldstrength, baseband fieldstrength, Detune, Multipath, Adjacent Channel 
            and Stereo/Mono flag (a deviation indication is also returned in addition to the quality information).
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel

        pQuality : the pointer of the signal quality data will be stored.    
            union Tun_SignalQuality
            {
                FM_SignalQuality    fmQuality;
                AM_SignalQuality    amQuality;
                WX_SignalQuality    wxQuality;
            };
            
Return Value    : Tun_Status
*************************************************************************************/
Tun_Status TUN_Get_ReceptionQuality (tU8 deviceAddress, int channelID, union Tun_SignalQuality *pQuality)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_GET_RECEPTION_QUALITY;    
    int cmdParamNum = 1;
    int ansParmNum = 3;
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = channelID;

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);
    
#ifdef TRACE_STAR_CMD_GET_RECEPTION_QUALITY    
    PRINTF("Star_Command_Communicate = %s, answerData = 0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x   0x%02x 0x%02x 0x%02x   0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x ", RetStatusName[tunerStatus].Name, 
        answerData[0], answerData[1], answerData[2],  answerData[3], answerData[4], answerData[5], answerData[6], answerData[7],
         answerData[8],  answerData[9], answerData[10], answerData[11], answerData[12], answerData[13], answerData[14]);
#endif        
        
    if ((tunerStatus == RET_SUCCESS) && (realAnsParamNum >= 3))
    {
        pQuality->fmQuality.fstRF = answerData[3];            /*fstRF*/
        pQuality->fmQuality.fieldStrength = answerData[4] ;    /* fstBB */
        pQuality->fmQuality.detuning = answerData[5];
        pQuality->fmQuality.mp = answerData[6];                /* available only for FM*/
        pQuality->fmQuality.mpxNoise = answerData[7];        /* available only for FM */
        pQuality->fmQuality.snr = answerData[8];                /* available only for FM, AM */
        pQuality->fmQuality.adj = answerData[9];                /* available only for FM AM*/
        pQuality->fmQuality.coCh = answerData[10];                /* available only for FM , only BG for VPA mode*/
        pQuality->fmQuality.stereo= answerData[11] & 0x01;    /* available only for FM*/
        pQuality->fmQuality.deviation = (answerData[11] >> 1) & 0x7F;    /*deviation */
    }
#else
#endif    

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_conf_JESD204
Description    : This function is used to configure the JESD204 baseband interface.        
            This function must be calleded before calling TUN_Set_BB_IF.
            
Parameters    :
        deviceAddress :  Star tuner I2C address.
        mode  :    mode
        config : Configuration
        testnum : Test number
        testchar : Char to be sent as test char
        ilam : ILA M parameter
        ilak : ILA K parameter
            
Return Value    : Tun_Status                    
*************************************************************************************/
Tun_Status TUN_conf_JESD204(tU8 deviceAddress, tU32 mode, tU32 config, tU32 testnum, tU32 testchar, tU32 ilam, tU32 ilak)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_CONF_JESD204;
    int cmdParamNum = 6;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[0] = (mode >> 16) & 0xFF;
    paramData[1] = (mode >> 8) & 0xFF;
    paramData[2] = (mode & 0xFF);
    
    paramData[3] = (config >> 16) & 0xFF;
    paramData[4] = (config >> 8) & 0xFF;
    paramData[5] = (config & 0xFF);

    paramData[6] = (testnum >> 16) & 0xFF;
    paramData[7] = (testnum >> 8) & 0xFF;
    paramData[8] = (testnum & 0xFF);

    paramData[11] = testchar;

    paramData[14] = ilam;

    paramData[17] = ilak;

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_CONF_JESD204
    PRINTF("Star_Command_Communicate = %s, mode = 0x%06x, config = 0x%06x, testnum = 0x%06x, testchar = 0x%06x, ILAM = 0x%06x, ILAK = 0x%06x", RetStatusName[tunerStatus].Name, mode, config, testnum, testchar, ilam, ilak);
#endif

#else
#endif    

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Set_Blend
Description    : This function is used to set the audio blend operation mode for digital reception standards in case the I2S audio 
            from an external digital decoder is brought back into the TDA7707. In case of HD reception, 
            the output from the audio blend block is fed outside in an analog format through the on-board DACs and in digital format via I2S. 
            In case of any other standards (DAB and DRM) the output from the audio blend block is fed outside only in an analog format through the on-board DACs.
            The function sets the way audio is selected.
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        blendMode :  Blend mode
            00: automatic via GPIO
            01: force analog signal
            10:force digital source input (HD)
            11: force digital source input (non-HD)
                
Return Value    : None                        
*************************************************************************************/
Tun_Status TUN_Set_Blend(tU8 deviceAddress, tU8 blendMode)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_SET_HD_BLEND;
    int cmdParamNum = 1;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = blendMode;

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_CONF_BB_SAI
    PRINTF("Star_Command_Communicate = %s, blendMode = 0x%06x", RetStatusName[tunerStatus].Name, blendMode);
#endif

#else
#endif    

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Set_RDS
Description    : This function is used to enable or disable the RDS buffer feature of STAR tuner.
            Decoded blocks are stored in the DSP buffer according to quality criteria set this function.
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel

        rdsAction : Enable RDS Buffer or disable RDS buffer.
            
Return Value    : Tun_Status        
*************************************************************************************/
Tun_Status TUN_Set_RDS (tU8 deviceAddress, int channelID, RDS_Action rdsAction)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_RDSBUFFER_SET;
    int cmdParamNum = 3;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */
    tU32 rdsCSR;
    
    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);

    //Reset,   err filter,  polling mode, RDS, Enable
    rdsCSR   = rdsAction;                //Enable, disable
#ifdef RBDS_ENABLE
    rdsCSR |= 0x01 << 2;               //RBDS
#endif
    rdsCSR |= 0x01 << 9;               //Reset
    rdsCSR |= RDS_CSR_ERRTHRESH << 4;           //err filter    
    
    paramData[2] = channelID;
    paramData[3] = (rdsCSR >> 16) & 0xFF;
    paramData[4] = (rdsCSR >> 8) & 0xFF;
    paramData[5] = (rdsCSR & 0xFF);
    paramData[8] = RDS_NORMALMODE_NRQST;

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_RDS
    PRINTF("Star_Command_Communicate = %s, rdsAction = %d", RetStatusName[tunerStatus].Name, rdsAction);
#endif

#else
#endif    

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Read_RDS
Description    : This function returns the collected RDS blocks and clears the RDS buffer of a specific RDS module. 
            In case the RDS buffer is empty the RDS Read Notification Register is the only response.
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel
            
        pRDSbuffer :    the pointer of the RDS buffer data will be stored.
            {
                int   validBlockNum;
                tU8 blockdata[RDSBUFFER_WORDS_MAXNUM * 3];
            } RDS_Buffer;
        
Return Value    : Tun_Status            
*************************************************************************************/
Tun_Status TUN_Read_RDS (tU8 deviceAddress, int channelID, RDS_Buffer *pRDSbuffer)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_RDSBUFFER_READ;
    int cmdParamNum = 1;
    int ansParmNum = 16 + 1;             /* The maximum block num is 16 in the DSP RDS Buffer, plus 1 RNR */
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */
    
    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = channelID;

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, 0, answerData, TRUE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_RDS
    PRINTF("Star_Command_Communicate = %s", RetStatusName[tunerStatus].Name);
#endif

    if ((tunerStatus == RET_SUCCESS) && (realAnsParamNum > 1))
    {
        /* answerData[3], answerData[4], answerData[5]   :  Read Notification Register (RNR) */
        /*  DATARDY  : Bit 23
              SYNC   :  Bit 22
              BOFL : Bit21   (do not need check, as new data will overwrite the old data, that means the data in RDS buffer is newest even if it's overfolowed )
              BNE : Bit20
              TUNCH2 : Bit 1
              TUNCH1 : Bit 0
        */
        if ((answerData[3] & 0x40) &&    /* data not empty. and RNR SYN OK. do not check DATADY, BOFL, and BNE */
            (answerData[5] & channelID) )    /* Tun channel information */
        {
            pRDSbuffer->validBlockNum = realAnsParamNum - 1;
            memcpy(pRDSbuffer->blockdata, answerData + 6, 3 * (pRDSbuffer->validBlockNum));
        }
    }
    else
    {
        pRDSbuffer->validBlockNum = 0;
    }
#else
#endif

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_AF_Start
Description    : This function is used to start the verification of the quality of a series of AF frequencies. 
            It is always terminated by a function TUN_AF_End.
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel

        alterFreq : Alternative frequency (kHz)
        antSelection : Antenna selection for VPA mode (meaningless if not in VPA mode)
        
        pAFquality: the pointer of the AF quality data which will be stored.
            {
                tU8 fieldStrength;
                tU8 detuning;
                tU8 mp;
                tU8 mpxNoise;
                tU8 snr;
                tS8 adj;
            } AF_SignalQuality;

Return Value    : Tun_Status            
*************************************************************************************/
Tun_Status TUN_AF_Start (tU8 deviceAddress, int channelID, tU32 alterFreq, tU32 antSelection, AF_SignalQuality *pAFquality)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_AF_START;
    int cmdParamNum = 3;
    int ansParmNum = 3;             /* The maximum block num is 16 in the DSP RDS Buffer, plus 1 RNR */
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */
    
    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = channelID;
    paramData[3] = (alterFreq >> 16) & 0xFF;
    paramData[4] = (alterFreq >> 8) & 0xFF;
    paramData[5] = (alterFreq & 0xFF);
    paramData[8] = antSelection;

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, 0, answerData, TRUE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_RDS
    PRINTF("Star_Command_Communicate = %s", RetStatusName[tunerStatus].Name);
#endif

    if ((tunerStatus == RET_SUCCESS) && (realAnsParamNum >=3))
    {
        pAFquality->fieldStrength = answerData[4] ;        /* fstBB */
        pAFquality->detuning =answerData[5];            /* detuning */
        pAFquality->mp =  answerData[6];                /* available only for FM*/
        pAFquality->mpxNoise = answerData[7];            /* available only for FM */
        pAFquality->snr = answerData[8];                /* available only for FM, AM */
        pAFquality->adj = answerData[9];                /* available only for FM AM*/
    }
#else
#endif

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_AF_End
Description    : This function is used to stop the verification of the quality of a series of AF frequencies 
            previously initiated with function TUN_AF_Start. This functions can be used both to stop 
            the sequence and to abort it. The tuner is restored to the configuration (single channel 
            FM mode or VPA mode) it had prior to the activation of the AF start sequence. The 
            frequency at which reception must be restored can be chosen between the previous 
            reception frequency, the latest checked AF frequency or any other specified frequency.

Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel

        freqAfterAFEnd :Reception frequency after AF end (kHz)
        
        pAFquality: the pointer of the AF quality data which will be stored.
            {
                tU8 fieldStrength;
                tU8 detuning;
                tU8 mp;
                tU8 mpxNoise;
                tU8 snr;
                tS8 adj;
            } AF_SignalQuality;
            
Return Value    : Tun_Status            
*************************************************************************************/
Tun_Status TUN_AF_End(tU8 deviceAddress, int channelID, tU32 freqAfterAFEnd, AF_SignalQuality *pAFquality)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_AF_END;
    int cmdParamNum = 2;
    int ansParmNum = 3;             /* The maximum block num is 16 in the DSP RDS Buffer, plus 1 RNR */
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */
    
    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = channelID;
    paramData[3] = (freqAfterAFEnd >> 16) & 0xFF;
    paramData[4] = (freqAfterAFEnd >> 8) & 0xFF;
    paramData[5] = (freqAfterAFEnd & 0xFF);

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, 0, answerData, TRUE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_RDS
    PRINTF("Star_Command_Communicate = %s", RetStatusName[tunerStatus].Name);
#endif

    if ((tunerStatus == RET_SUCCESS) && (realAnsParamNum >=3))
    {
        pAFquality->fieldStrength = answerData[4] ;        /* fstBB */
        pAFquality->detuning =answerData[5];            /* detuning */
        pAFquality->mp =  answerData[6];                /* available only for FM*/
        pAFquality->mpxNoise = answerData[7];            /* available only for FM */
        pAFquality->snr = answerData[8];                /* available only for FM, AM */
        pAFquality->adj = answerData[9];                /* available only for FM AM*/
    }
#else
#endif

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_AF_Check
Description    : This function is used to perform a fast verification of the quality of one AF frequency.

Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel
            
        frequency : Alternative frequency (kHz)
            
        pAFquality: the pointer of the AF quality data which will be stored.
            {
                tU8 fieldStrength;
                tU8 detuning;
                tU8 mp;
                tU8 mpxNoise;
                tU8 snr;
                tS8 adj;
            } AF_SignalQuality;
            
Return Value    : Tun_Status                    
*************************************************************************************/
Tun_Status TUN_AF_Check (tU8 deviceAddress, int channelID, tU32 frequency, AF_SignalQuality *pAFquality)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_AF_CHECK;
    int cmdParamNum = 2;
    int ansParmNum = 3;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */
    
    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = channelID;
    paramData[3] = (frequency >> 16) & 0xFF;
    paramData[4] = (frequency >> 8) & 0xFF;
    paramData[5] = (frequency & 0xFF);

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, FALSE);

#ifdef TRACE_STAR_CMD_RDS
    PRINTF("Star_Command_Communicate = %s, frequency = %d", RetStatusName[tunerStatus].Name, frequency);
#endif

    if ((tunerStatus == RET_SUCCESS) && (realAnsParamNum >=3))
    {
        pAFquality->fieldStrength = answerData[4] ;        /* fstBB */
        pAFquality->detuning =answerData[5];            /* detuning */
        pAFquality->mp =  answerData[6];                /* available only for FM*/
        pAFquality->mpxNoise = answerData[7];            /* available only for FM */
        pAFquality->snr = answerData[8];                /* available only for FM, AM */
        pAFquality->adj = answerData[9];                /* available only for FM AM*/
    }

#else
#endif    

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_AF_Switch
Description    : This function is used to perform a fast tuning operation to an AF frequency. 
             At the end of the operation, the tuner goes into normal reception mode on said frequency. 
             This function furthermore resets the RDS buffer.
             
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel
            
        frequency : Alternative frequency (kHz)
            
Return Value    : Tun_Status                    
*************************************************************************************/
Tun_Status TUN_AF_Switch (tU8 deviceAddress, int channelID, tU32 frequency)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_AF_SWITCH;
    int cmdParamNum = 2;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */
    
    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = channelID;
    paramData[3] = (frequency >> 16) & 0xFF;
    paramData[4] = (frequency >> 8) & 0xFF;
    paramData[5] = (frequency & 0xFF);

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_RDS
    PRINTF("Star_Command_Communicate = %s, frequency = %d", RetStatusName[tunerStatus].Name, frequency);
#endif

#else
#endif    

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Get_AFquality
Description    : This function returns the FM quality information of the foreground or background channel 
            as a result of an RDS AF check operation. The quality information consists of:
            RF fieldstrength, baseband fieldstrength, Detune, Multipath, Adjacent Channel.
             
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel
            
        pAFquality: the pointer of the AF quality data which will be stored.
            {
                tU8 fieldStrength;
                tU8 detuning;
                tU8 mp;
                tU8 mpxNoise;
                tU8 snr;
                tS8 adj;
            } AF_SignalQuality;
            
Return Value    : Tun_Status                        
*************************************************************************************/
Tun_Status TUN_Get_AFquality (tU8 deviceAddress, int channelID, AF_SignalQuality *pAFquality)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_GET_AF_QUALITY;
    int cmdParamNum = 1;
    int ansParmNum = 3;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */
    
    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = channelID;

#ifdef STAR_COMM_BUS_I2C
    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_RDS
    PRINTF("Star_Command_Communicate = %s", RetStatusName[tunerStatus].Name);
#endif

    if ((tunerStatus == RET_SUCCESS) && (realAnsParamNum >=3))
    {
        pAFquality->fieldStrength = answerData[4] ;        /* fstBB */
        pAFquality->detuning =answerData[5];            /* detuning */
        pAFquality->mp =  answerData[6];                /* available only for FM*/
        pAFquality->mpxNoise = answerData[7];            /* available only for FM */
        pAFquality->snr = answerData[8];                /* available only for FM, AM */
        pAFquality->adj = answerData[9];                /* available only for FM AM*/
    }
#else
#endif    

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Wait_Ready
Description    : This function is used to wait till tuner ready before acceptting new command to execute.
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel
            
        msTimeOut : ms value for time out.
        
Return Value    : Tun_Status            
*************************************************************************************/
Tun_Status TUN_Wait_Ready(tU8 deviceAddress, int channelID, int msTimeOut)
{
    Tun_Status tunerStatus = RET_ERROR;
    struct timeval tvTimeOut;
    struct timeval tvTimeNow;

    const tU32 MILLION = 1000000;
    tU32 usVal;
    int busyStatus = 0;
    
    gettimeofday(&tvTimeOut, NULL);

    usVal = tvTimeOut.tv_usec +  msTimeOut * 1000;
    tvTimeOut.tv_sec += usVal / MILLION;
    tvTimeOut.tv_usec = usVal % MILLION;
    
    while (1)
    {
        tunerStatus = Star_Get_TunerBusyStatus(deviceAddress, channelID, &busyStatus);    
        if (tunerStatus == RET_ERROR) break;
        
#ifdef TRACE_STAR_TUN_WAIT_READY
        PRINTF("Star_Get_TunerBusyStatus = %s, busyStatus = %d", RetStatusName[tunerStatus].Name, busyStatus);
#endif
        if (busyStatus == 0)
        {
            tunerStatus = RET_SUCCESS;
            break;
        }

        gettimeofday(&tvTimeNow, NULL);
        if ((tvTimeNow.tv_sec > tvTimeOut.tv_sec) || 
           ((tvTimeNow.tv_sec == tvTimeOut.tv_sec) && (tvTimeNow.tv_usec > tvTimeOut.tv_usec)))
        {
            tunerStatus = RET_ERR_TIME_OUT;
            break;
        }

        msleep(1);
    }

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Get_TunedFreq
Description    : This function is used to get current frequency of the named tuner channel.
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel
            
        pFreq : The tuned frequecy of the tuner channel will be stored through this pointer.
        
Return Value    : Tun_Status            
*************************************************************************************/
Tun_Status TUN_Get_TunedFreq(tU8 deviceAddress, int channelID, tU32 *pFreq)
{
    Tun_Status tunerStatus = RET_ERROR;
    tU32 addSCSR = 0;
    int dataSize = 3;
    tU8 dataBuff[dataSize];

    assert((channelID == 1) ||(channelID == 2));
    memset(dataBuff, 0x00 , dataSize);
    
#ifdef STAR_COMM_BUS_I2C

    addSCSR = (channelID == 1) ? STAR_ADDRESS_SCSR1 : STAR_ADDRESS_SCSR2;

    tunerStatus = Star_I2C_Direct_Read(deviceAddress, addSCSR, dataBuff, dataSize); 
    if (tunerStatus == RET_SUCCESS) 
    {
        *pFreq = ((dataBuff[0] << 16) | (dataBuff[1] << 8) | dataBuff[2]) ;
    }

#ifdef TRACE_STAR_TUN_GET_TUNEDFREQ
    PRINTF("Star_I2C_Direct_Read = %s, freq = %d", RetStatusName[tunerStatus].Name, *pFreq);
#endif

#endif

    return tunerStatus; 
}
#endif


static uint32 star_u8btou32b(uint8 src)
{
    // convert 8bit to 32bit with expanding the sign bit.
	uint32 ret;

	if(src & 0x80)
    {
		ret = src | 0xffffff00U;
	}
	else
    {
		ret = src & 0x000000ffU;
	}

	return ret;
}


/*************************************************************************************
Function        : TUN_Cmd_Write
Description    : This function performs a write operation to tuner' register. 
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        regAddress :  STAR DSP memory Addr or non-DSP memory addr.
        regData  :  the data which need be written to tuner.
             
Return Value    : Tun_Status            
*************************************************************************************/
Tun_Status TUN_Cmd_Write(tU8 deviceAddress, tU32 regAddress, tU32 regData)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_WRITE;    
    int cmdParamNum = 3;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    memset(paramData, 0x00, cmdParamNum * 3);
    
    paramData[0] = (regAddress >> 16) & 0xFF;
    paramData[1] = (regAddress >> 8) & 0xFF;
    paramData[2] = (regAddress) & 0xFF;

    // If bit 19 is 1, then the address is in the DSP memory space and the data word has 24 bits.
    // if bit 19 is 0, then the address is elsewhere and the data word has 32 bits.
    if((regAddress & 0x080000) == 0)
    {
        cmdParamNum = 3;
        paramData[3] = (regData >> 24) & 0xFF;
        paramData[4] = (regData >> 16) & 0xFF;
        paramData[5] = (regData >> 8) & 0xFF;
        paramData[6] = (regData >> 16) & 0xFF;
        paramData[7] = (regData >> 8) & 0xFF;
        paramData[8] = (regData) & 0xFF;
    }
    else
    {
        cmdParamNum = 2;
        paramData[3] = (regData >> 16) & 0xFF;
        paramData[4] = (regData >> 8) & 0xFF;
        paramData[5] = (regData) & 0xFF;
    }

    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);
    
#ifdef TRACE_STAR_CMD_WRITE    
    //PRINTF("Star_Command_Communicate= %s, deviceAddress= 0x%02x, regAddress= 0x%06x, cmdParamNum= %d, data= 0x%06x", RetStatusName[tunerStatus].Name, deviceAddress, regAddress, cmdParamNum, regData);
    printf("[%s] Star_Command_Communicate = %d, regAddress= 0x%06x, cmdParamNum= %d, data= 0x%06x \n",
        __func__, tunerStatus, regAddress, cmdParamNum, regData);
#endif

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Change_Band
Description     : This function executes the entire band change procedure on a specified channel.
            Additionally specific processing features will be activated (e.g. VPA).
            For DAB and DRM the corresponding digital base band interface is activated when this command is invoked;
            This command additionally sets the band limits and step for Seek operations.
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel
            
        bandMode :    
            0x000000: Standby (no reception)
            0x000001: FM
            0x000002: Weather Band (WX)
            0x000003: DAB Band 3
            0x000004: DAB L-Band
            0x000005: AM EU
            0x000006: AM US
            0x000007: DRM30
            0x000008: DRM+
            0x000009: not used
            0x00000A: not used
            0x00000B: not used
            0x00000C: not used
            0x00000D: not used
            0x00000E: Reserved ¨C do not use
            0x00000F: Reserved ¨C do not use
            
        maxFreq  : Upper band limit [kHz]
        minFreq   : Lower band limit [kHz]
        VPAMode : VPA mode : 0 - off;  1 - on.
        
Return Value    : Tun_Status        
*************************************************************************************/
Tun_Status TUN_Change_Band (tU8 deviceAddress, int channelID, int bandMode, tU32 maxFreq, tU32 minFreq, int seekStep, int VPAMode)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_CHANGE_BAND;
    int cmdParamNum = 6;
    int ansParmNum = 0;                    /* if it's 0, means only return answer header and check sum */
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    
    paramData[2] = channelID;
    paramData[5] = bandMode;
#if 0
    if (bandMode == 1) paramData[8] = (tU8)VPAMode;
#else
    paramData[8] = 0x10; // Param. 3 Bit [4] : 1 ==> for HD and DRM30 reception, Bit [0] : 0 ==> no phase div.
#endif
    paramData[9] = (minFreq >> 16) & 0xFF;
    paramData[10] = (minFreq  >> 8) & 0xFF;
    paramData[11] = (minFreq & 0xFF);
    paramData[12] = (maxFreq  >> 16) & 0xFF;
    paramData[13] = (maxFreq >> 8) & 0xFF;
    paramData[14] = (maxFreq & 0xFF);
    paramData[15] = (seekStep >> 16) & 0xFF;
    paramData[16] = (seekStep >> 8) & 0xFF;
    paramData[17] = (seekStep & 0xFF);

    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_CHANGE_BAND                
    //PRINTF("Star_Command_Communicate = %s, channelID = %d, band = %d, freqMax = %d, freqMin = %d, VPA_enable = %d", RetStatusName[tunerStatus].Name, channelID, bandMode, maxFreq, minFreq, VPAMode);
    printf("[%s] Star_Command_Communicate = %d, channelID = %d, band = %d, freqMax = %d, freqMin = %d \n",
            __func__, tunerStatus, channelID, bandMode, maxFreq, minFreq);
#endif

    (void) VPAMode;

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Change_Frequency
Description    : This function tunes the corresponding channel to the specified frequency. 
            For AM/FM bands it additionally performs muting and unmuting (foreground tuner only) 
            and initializing weak signal processing and quality detectors.
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel
            
        frequency : Frequency [kHz]
Return Value    : Tun_Status            
*************************************************************************************/
Tun_Status TUN_Change_Frequency (tU8 deviceAddress, int channelID, tU32 frequency)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_TUNE;
    int cmdParamNum = 3;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    
    paramData[2] = channelID;
    paramData[3] = (frequency >> 16) & 0xFF;
    paramData[4] = (frequency >> 8) & 0xFF;
    paramData[5] = (frequency & 0xFF);
    paramData[8] = INJECTION_SIDE_AUTO;

    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_TUNE                
    //PRINTF("Star_Command_Communicate = %s, frequency = %d, deviceAddr = 0x%02x", RetStatusName[tunerStatus].Name, frequency, deviceAddress);
    printf("[%s] Star_Command_Communicate = %d, channelID = %d, frequency = %d \n", __func__, tunerStatus, channelID, frequency);
#endif

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Get_ChannelQuality
Description    : This function returns the AM/FM/WX quality information of the foreground 
            or background channel before WSP during normal reception mode (thus not including Seek nor RDS AF check)
            before the algorithms that improve reception quality.
            consisting of: RF fieldstrength, baseband fieldstrength, Detune, Multipath, Adjacent Channel 
            and Stereo/Mono flag (a deviation indication is also returned in addition to the quality information).
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        channelID :
            1  :  foreground channel
            2  :  background channel

        pQuality : the pointer of the signal quality data will be stored.    
            union Tun_SignalQuality
            {
                FM_SignalQuality    fmQuality;
                AM_SignalQuality    amQuality;
                WX_SignalQuality    wxQuality;
            };
            
Return Value    : Tun_Status
*************************************************************************************/
#if 0
Tun_Status TUN_Get_ChannelQuality (tU8 deviceAddress, int channelID, tBool bVPAMode, union Tun_SignalQuality *pQuality)
#else
Tun_Status TUN_Get_ChannelQuality (tU8 deviceAddress, int channelID, tBool bVPAMode, stSTAR_DRV_QUALITY_t *pQuality)
#endif
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_GET_CHANNEL_QUALITY;    
    int cmdParamNum = 1;
    int ansParmNum = 3;
    int ansParamOffset = 0;
    int realAnsParamNum;
    
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    if (bVPAMode)
    {
        cmdParamNum = 0;
        ansParmNum = 6;
        if (channelID == 2) ansParamOffset = 3 * 3;
    }
    
    assert((channelID == 1) ||(channelID == 2));
    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = channelID;

    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);
    
#ifdef TRACE_STAR_CMD_GET_RECEPTION_QUALITY    
    PRINTF("Star_Command_Communicate = %s, answerData = 0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x   0x%02x 0x%02x 0x%02x   0x%02x 0x%02x 0x%02x  0x%02x 0x%02x 0x%02x ", RetStatusName[tunerStatus].Name, 
        answerData[0], answerData[1], answerData[2],  answerData[3], answerData[4], answerData[5], answerData[6], answerData[7],
         answerData[8],  answerData[9], answerData[10], answerData[11], answerData[12], answerData[13], answerData[14]);
#endif        
        
    if ((tunerStatus == RET_SUCCESS) && (realAnsParamNum >= 3))
    {
#if 0   // ST original
        pQuality->fmQuality.fstRF = answerData[3 + ansParamOffset];            /*fstRF*/
        pQuality->fmQuality.fieldStrength = answerData[4 + ansParamOffset] ;    /* fstBB */
        pQuality->fmQuality.detuning = answerData[5 + ansParamOffset];
        pQuality->fmQuality.mp = answerData[6 + ansParamOffset];                /* available only for FM*/
        pQuality->fmQuality.mpxNoise = answerData[7 + ansParamOffset];        /* available only for FM */
        pQuality->fmQuality.snr = answerData[8 + ansParamOffset];                /* available only for FM, AM */
        pQuality->fmQuality.adj = answerData[9 + ansParamOffset];                /* available only for FM AM*/
        pQuality->fmQuality.stereo= answerData[11 + ansParamOffset] & 0x01;    /* available only for FM*/
        pQuality->fmQuality.deviation = (answerData[11 + ansParamOffset] >> 1) & 0x7F;    /*deviation */
#else   // modify for TC HDRadio
        pQuality->fm.FstRF = star_u8btou32b(answerData[3 + ansParamOffset]);	/*fstRF*/
        pQuality->fm.FstBB = star_u8btou32b(answerData[4 + ansParamOffset]);	/*fstBB*/
        pQuality->fm.Det = answerData[5 + ansParamOffset];
        pQuality->fm.Mpth = answerData[6 + ansParamOffset];					/* available only for FM*/
        pQuality->fm.MpxNoise = answerData[7 + ansParamOffset];				/* available only for FM */
        pQuality->fm.Snr = answerData[8 + ansParamOffset];					/* available only for FM, AM */
        pQuality->fm.Adj = star_u8btou32b(answerData[9 + ansParamOffset]);	/* available only for FM AM*/
        pQuality->fm.Stereo= answerData[11 + ansParamOffset] & 0x01;		/* available only for FM*/
        pQuality->fm.Dev = (answerData[11 + ansParamOffset] >> 1) & 0x7F;	/*deviation*/
#endif
    }


    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_conf_BB_SAI
Description    : This function is used to configure the baseband SAI interface.
            The SAI is configured either in MUX mode or AFE mode.
                I and Q data length is 16bit each. Data alignment in one frame is:
                Without status bits: I(16) + Q(16)
                With status bits: I(16) + S1(16) + Q(16) + S2(16)
            MUX mode:
                Symmetrical word clock - one sample delayed (I2S mode), start of new frame at falling edge
            AFE mode:
                Word clock is a one-bit clock burst, start of new frame at falling edge
                The content of the status bits is described in the command description manual.
            This function must be calleded before calling TUN_Set_BB_IF.
            
Parameters    :
        deviceAddress :  Star tuner I2C address.
        mode  :    SAI mode
        config :     Configuration
            
Return Value    : Tun_Status                    
*************************************************************************************/
Tun_Status TUN_conf_BB_SAI(tU8 deviceAddress, tU32 mode, tU32 config)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_CONF_BB_SAI;
    int cmdParamNum = 2;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = mode;
    paramData[3] = (config >> 16) & 0xFF;
    paramData[4] = (config >> 8) & 0xFF;
    paramData[5] = (config & 0xFF);

    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_CONF_BB_SAI
    //PRINTF("Star_Command_Communicate = %s, mode = 0x%06x, config = 0x%06x", RetStatusName[tunerStatus].Name, mode, config);
    printf("[%s] Star_Command_Communicate = %d, mode = 0x%06x, config = 0x%06x \n", __func__, tunerStatus, mode, config);
#endif

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Set_Audio_IF
Description    : This function is used to set the audio DAC and audio SAI input and output channel.
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        SAIMode :  Audio SAI config
            Bit [2] : audio SAI input enable/disable    0: off    1: on
            Bit [1] : audio SAI output enable/disable    0: off    1: on
            Bit [0] : audio DAC enable/disable        0: off    1: on
            
        SAIConfig:     
            Bit [10] : SAI Word Select Configuration
                0: SAI WS is 50% duty cycle (default)
                1: SAI WS is a single pulse signal
            Bit [9] :     SAI Audio Data Sign Extension
                0: SAI Audio Data Sign is not extended to MSB, filled with zeros
                1: SAI Audio Data Sign is extended to MSB (default)
            Bit [8] : SAI Audio WS Polarity
                0: SAI Audio WS Polarity is high (1 Right / 0 Left)
                1: SAI Audio WS Polarity is low (0 Right / 1 Left) (default)
            Bit [7] : SAI Audio Revert Data Sequence
                0: SAI Audio Data is not reverted (not twizzeld) (default)
                1: SAI Audio Data is reverted (twizzeld)
            Bit [6] : SAI Audio Data Delay
                0: SAI Audio Data is delayed vs. WS edge (I2S mode) (default)
                1: SAI Audio Data is not delayed vs. WS edge
            Bit [5] : SAI Audio Data Length
                0: SAI Audio Data length is 16L + 16R
                1: SAI Audio Data length is 32L + 32R (default)
            Bit [4] : SAI Audio Data Clock Polarity
                0: SAI Audio Clock is not inverted
                1: SAI Audio Clock is inverted (default)
            Bit [3:0] : 0x0
            
Return Value    : Tun_Status                    
*************************************************************************************/
Tun_Status TUN_Set_Audio_IF(tU8 deviceAddress, tU8 SAIMode, tU32 SAIConfig)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_SET_AUDIO_IF;
    int cmdParamNum = 2;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[2] = SAIMode;
    paramData[3] = (SAIConfig >> 16) & 0xFF;
    paramData[4] = (SAIConfig >> 8) & 0xFF;
    paramData[5] = (SAIConfig & 0xFF);

    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_CONF_BB_SAI
    //PRINTF("Star_Command_Communicate = %s, SAIMode = 0x%06x, SAIConfig = 0x%06x", RetStatusName[tunerStatus].Name, SAIMode, SAIConfig);
    printf("[%s] Star_Command_Communicate = %d, SAIMode = 0x%06x, SAIConfig = 0x%06x \n", __func__, tunerStatus, SAIMode, SAIConfig);
#endif

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Set_BB_IF
Description     : This function is used to configure the digital Base Band interface and maps the digital Base Band 
              and the digital Audio interfaces to I/Os. Not all combinations of BB interface and Audio interface are allowed, 
              details see the document of Star tuner MCU control guideline. 
            
Parameters    :
        deviceAddress :  star tuner I2C address.
        BBConfig
            Audio interface configuration and Base Band (IF) interface configuration
            
Return Value    : Tun_Status                    
*************************************************************************************/
Tun_Status TUN_Set_BB_IF(tU8 deviceAddress, tU32 BBConfig)
{
    Tun_Status tunerStatus = RET_ERROR;
    int cmdID = CMD_CODE_TUNER_SET_BB_IF;
    int cmdParamNum = 1;
    int ansParmNum = 0;                    /*if it's 0, means only return answer header and check sum*/
    int realAnsParamNum;
    tU8 paramData[cmdParamNum * 3];
    tU8 answerData[(ansParmNum + 2) * 3];    /* answer data include asnwer header, answer param and check sum */

    memset(paramData, 0x00, cmdParamNum * 3);
    paramData[0] = (BBConfig >> 16) & 0xFF;
    paramData[1] = (BBConfig >> 8) & 0xFF;
    paramData[2] = (BBConfig & 0xFF);

    tunerStatus = Star_Command_Communicate(deviceAddress, cmdID, cmdParamNum, paramData, ansParmNum, answerData, FALSE, &realAnsParamNum, TRUE);

#ifdef TRACE_STAR_CMD_SET_BB_IF            
    //PRINTF("Star_Command_Communicate = %s, BBConfig = %d", RetStatusName[tunerStatus].Name, BBConfig);
    printf("[%s] Star_Command_Communicate = %d, BBConfig = %d \n", __func__, tunerStatus, BBConfig);
#endif

    return tunerStatus;
}


/*************************************************************************************
Function        : TUN_Download_BootCode
Description    : Download the boot code data with direct write method

Parameters    : 
        deviceAddress :  star tuner I2C address.
        deviceType : device type

Return Value    : Tun_Status
*************************************************************************************/
#if 0
Tun_Status TUN_Download_BootCode(tU8 deviceAddress, Device_Type deviceType)
#else
Tun_Status TUN_Download_BootCode(tU8 deviceAddress)
#endif
{
    Tun_Status tunerStatus = RET_ERROR;
    tU32 addr;
    int byteNum, i = 0;        /* Normally bootcode size < 65k */
    tU8 * pBootCode = NULL;

#if 0
#if defined(TDA7708_TUNER)
    if (deviceType == DEV_TDA7708) pBootCode = CMOST_Firmware_TDA7708;
#elif defined(TDA7707_TUNER_VPA) || defined(TDA7707_TUNER_BACKGROUND)
    if (deviceType == DEV_TDA7707) pBootCode = CMOST_Firmware_TDA7707;
#endif

#if defined(STA710_TUNER)
    if (deviceType == DEV_STA710) pBootCode = CMOST_Firmware_STA710;
#endif
#else
    pBootCode = CMOST_Firmware;
#endif

    if (pBootCode)
    {
        while ( (*(pBootCode + i) != 0xFF) || (*(pBootCode + i + 1) != 0xFF) || (*(pBootCode + i + 2) != 0xFF))
        {
            addr = (*(pBootCode + i)) * 0x10000 + (*(pBootCode + i + 1)) * 0x100 + (*(pBootCode + i + 2));
            byteNum = (*(pBootCode + i + 3)) * 256 + (*(pBootCode + i + 4));   //bytes number (data)

            tunerStatus = Star_I2C_Direct_Write(deviceAddress, addr, pBootCode + i + 5, byteNum);
            if (tunerStatus != RET_SUCCESS) return tunerStatus;

            i += 5 + byteNum;
        }
    }
    
    return tunerStatus;
}

/*************************************************************************************
Function        : TUN_Download_CustomizedCoeffs
Description    : this function is used to write customized data in array(CUSTOM_ARRAY) to Star tuner by write mem command
            
Parameter     : 
        deviceAddress :  star tuner I2C address.

Return Value    : Tun_Status
*************************************************************************************/
Tun_Status TUN_Download_CustomizedCoeffs(tU8 deviceAddress)
{
    Tun_Status tunerStatus = RET_SUCCESS;

    tunerStatus = TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_saiConfig__7___syscoReg06High, 0x70409B);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_saiConfig__7___syscoReg06Low, 0x409B0A);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_saiConfig__7___syscoReg07High, 0x80C60A);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_saiConfig__7___syscoReg07Low, 0xC60A00);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_saiConfig__7___syscoReg08High, 0x00F000);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_saiConfig__7___syscoReg08Low, 0xF00000);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_saiConfig__7___saiBBReg00DisHigh, 0x810006);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_saiConfig__7___saiBBReg00DisLow, 0x000611);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_saiConfig__7___saiBBReg00EnHigh, 0x810016);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_saiConfig__7___saiBBReg00EnLow, 0x001611);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_saiConfig__7___flags, 0x000002);
        
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_srcConfig__7___srcBBReg00DisHigh, 0x010000);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_srcConfig__7___srcBBReg00DisLow, 0x000000);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_srcConfig__7___srcBB912Reg01High, 0x0D0E54);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_srcConfig__7___srcBB912Reg01Low, 0x0E5400);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_srcConfig__7___srcBB456Reg01High, 0x1A1CAA);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_srcConfig__7___srcBB456Reg01Low, 0x1CAA00);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_srcConfig__7___srcBBReg03High, 0xFF0100);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_srcConfig__7___srcBBReg03Low, 0x010000);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_srcConfig__7___srcBB912Reg00EnHigh, 0x000103);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_srcConfig__7___srcBB912Reg00EnLow, 0x0103F0);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_srcConfig__7___srcBB456Reg00EnHigh, 0x000303);
    tunerStatus |= TUN_Cmd_Write(deviceAddress, TDA7707_bbifY_srcConfig__7___srcBB456Reg00EnLow, 0x0303F0);

    return tunerStatus;
}

unsigned int star_getVersion(void)
{
    return (((unsigned int)STAR_DRIVER_VER_PREFIX__<<24) | ((unsigned int)STAR_DRIVER_VER_RELEASE_NUMBER__<<16) |
            ((unsigned int)STAR_DRIVER_VER_MAJOR_NUMBER__<<8) | ((unsigned int)STAR_DRIVER_VER_MINOR_NUMBER__<<0));
}

float star_getPreciseIqSampleRate(unsigned int ntuner)
{
    (void) ntuner;
    return 744187.0;
}

int star_getIqSampleRate(unsigned int ntuner)
{
    (void) ntuner;
    return 744187;
}

int star_getIqSamplingBit(unsigned int ntuner)
{
    (void) ntuner;
    return 16;
}

int star_setBand(int mod_mode, int channelID)
{
    Tun_Status tunerStatus = RET_SUCCESS;
    int bandMode;
    unsigned int maxFreq;
    unsigned int minFreq;
    unsigned int seekStep;
    unsigned int VPAMode = 0;
    
    bandMode = stAreaBands[gStarConf.area][mod_mode].tunerBandID;
    maxFreq = stAreaBands[gStarConf.area][mod_mode].maxFreq;
    minFreq = stAreaBands[gStarConf.area][mod_mode].minFreq;
    seekStep = stAreaBands[gStarConf.area][mod_mode].seekStep;

    tunerStatus = TUN_Change_Band(I2C_SLAVE_ADDRESS, channelID, bandMode, maxFreq, minFreq, seekStep, VPAMode);

    return tunerStatus;
}

int starhal_getTunerCh(unsigned int ntuner)
{
    int val = 1;

    if(ntuner == eTUNER_DRV_ID_SECONDARY || ntuner == eTUNER_DRV_ID_QUATERNARY)
        val = 2;

    return val;
}

int star_setTune(unsigned int mod_mode, unsigned int freq, unsigned int tune_mode, unsigned int ntuner)
{
    Tun_Status tunerStatus = RET_SUCCESS;

    int channelID;

    (void)tune_mode;

    if (mod_mode > eTUNER_DRV_AM_MODE)
        return eRET_NG_UNKNOWN;

    if (ntuner > eTUNER_DRV_ID_SECONDARY)
        return eRET_NG_UNKNOWN;

    channelID = starhal_getTunerCh(ntuner);

    if(star_drv_current_band[ntuner] != mod_mode)
    {
        tunerStatus = star_setBand(mod_mode, channelID);

        if (tunerStatus == RET_SUCCESS)
        {
            usleep(15*1000);    // LB wait 15 ms (wait > 10 ms @ 6.2. Band change sequence)
            star_drv_current_band[ntuner] = mod_mode;
        }
    }

    if (tunerStatus == RET_SUCCESS)
        tunerStatus = TUN_Change_Frequency(I2C_SLAVE_ADDRESS, channelID, freq);

    if (tunerStatus == RET_SUCCESS)
    {
        if(mod_mode == eTUNER_DRV_AM_MODE) {
            star_drv_am_frequency[ntuner] = freq;
        }
        else if(mod_mode == eTUNER_DRV_FM_MODE) {
            star_drv_fm_frequency[ntuner] = freq;
        }
        else
        {
            printf("[%s] Not supported mod_mode (%d).\n", __func__, mod_mode);
            return eRET_NG_UNKNOWN;
        }

        return eRET_OK;
    }
    else
        return eRET_NG_UNKNOWN;
}

int star_getTune(unsigned int *mod_mode, unsigned int *curfreq, unsigned int ntuner)
{
    RET ret = eRET_OK;

    *mod_mode = star_drv_current_band[ntuner];

    if(*mod_mode == eTUNER_DRV_AM_MODE) {
        *curfreq = star_drv_am_frequency[ntuner];
    }
    else if(*mod_mode == eTUNER_DRV_FM_MODE) {
        *curfreq = star_drv_fm_frequency[ntuner];
    }
#if 0
    else {
        *curfreq = star_drv_dab_frequency[ntuner];
    }
#else
    else {
        ret = eRET_NG_UNKNOWN;
    }
#endif

    return ret;
}

int star_getQuality(unsigned int mod_mode, stSTAR_DRV_QUALITY_t *qdata, unsigned int ntuner)
{
    Tun_Status tunerStatus = RET_SUCCESS;
    int channelID;

    (void)mod_mode;

    if (ntuner > eTUNER_DRV_ID_SECONDARY)
        return eRET_NG_UNKNOWN;

    channelID = starhal_getTunerCh(ntuner);

    tunerStatus = TUN_Get_ChannelQuality(I2C_SLAVE_ADDRESS, channelID, 0, qdata);

    if (tunerStatus == RET_SUCCESS)
        return eRET_OK;
    else
        return eRET_NG_UNKNOWN;
}

int star_setMute(unsigned int fOnOff, unsigned int ntuner)
{
    (void)fOnOff;
    (void)ntuner;
    return 0;
}

int star_open(stTUNER_DRV_CONFIG_t type)
{
    Tun_Status tunerStatus = RET_ERROR;
    unsigned char read_data[4];
    int ret = 0;

    gStarConf = type;

    star_drv_initialized = 0;

    memset(star_drv_current_band, 0xff, sizeof(star_drv_current_band));
    memset(star_drv_fm_frequency, 0xff, sizeof(star_drv_fm_frequency));
    memset(star_drv_am_frequency, 0xff, sizeof(star_drv_am_frequency));

    ret = star_i2c_open();

    if (ret < 0)
        return eRET_NG_UNKNOWN;

    star_tuner_reset();

    // for Tunner communication check
    memset(read_data, 0, 4);
    tunerStatus = Star_I2C_Direct_Read(I2C_SLAVE_ADDRESS, STAR_ADDRESS_PRECHECKING, read_data, 4);

    if (tunerStatus != RET_SUCCESS)
    {
        printf("STAR_ADDRESS_PRECHECKING Star_I2C_Direct_Read fail\n");
        return eRET_NG_UNKNOWN;
    }

    printf("STAR_ADDRESS_PRECHECKING read result : %02x %02x %02x %02x\n", read_data[0], read_data[1], read_data[2], read_data[3]);

    tunerStatus = TUN_Download_BootCode(I2C_SLAVE_ADDRESS);

    if (tunerStatus != RET_SUCCESS)
    {
        printf("TUN_Download_BootCode fail\n");
        return eRET_NG_UNKNOWN;
    }

    usleep(130 * 1000);  // LB wait 130 ms (120 ms + 10ms margin)

    memset(read_data, 0 ,4);
    tunerStatus = Star_I2C_Direct_Read(I2C_SLAVE_ADDRESS, STAR_ADDRESS_CMDBUFFER, read_data, 4);

    if (tunerStatus != RET_SUCCESS)
    {
        printf("STAR_ADDRESS_CMDBUFFER Star_I2C_Direct_Read fail\n");
        return eRET_NG_UNKNOWN;
    }

    printf("STAR_ADDRESS_CMDBUFFER read result : %02x %02x %02x %02x\n", read_data[0], read_data[1], read_data[2], read_data[3]);

    //if (read_data[0] == 0xAF && read_data[1] == 0xFE && read_data[2] == 0x42 && read_data[3] == 0x00)
    if ((read_data[0] != 0xAF) || (read_data[1] != 0xFE) || (read_data[2] != 0x42) || (read_data[3] != 0x00))
    {
        return eRET_NG_UNKNOWN;
    }

    tunerStatus = TUN_Download_CustomizedCoeffs(I2C_SLAVE_ADDRESS);

    if (tunerStatus != RET_SUCCESS)
    {
        printf("TUN_Download_CustomizedCoeffs fail\n");
        return eRET_NG_UNKNOWN;
    }
    else
    {
        printf("CustomizedCoeffs has been successfully downloaded.\n");
    }

    // BB SAI setting
    // 0x07 ==> Mode7
    // 0x01 ==> master "half cycle" mode (for TDA7707 single only case)
    tunerStatus = TUN_conf_BB_SAI(I2C_SLAVE_ADDRESS, 0x07, 0x01);

    if (tunerStatus != RET_SUCCESS)
    {
        printf("BB SAI setup fail. (%d)\n", tunerStatus);
        return eRET_NG_UNKNOWN;
    }
    else
    {
        printf("BB SAI setup is complete.\n");
    }

    // set BB IF
    // 0x04 ==> Audio interface mode 4
    // 0x05 ==> Base Band (IF) interface SAI BB split mode 
    tunerStatus = TUN_Set_BB_IF(I2C_SLAVE_ADDRESS, 0x040005);

    if (tunerStatus != RET_SUCCESS)
    {
        printf("BB IF setup fail. (%d)\n", tunerStatus);
        return eRET_NG_UNKNOWN;
    }
    else
    {
        printf("BB IF setup is complete.\n");
    }

    // set Audio IF
    // 0x03 ==> audio SAI output enable and DAC enable
    // Bit [23:12] 0xA57 ==> 44.1 kHz, Bit [11:0] 0x310 ==> Bit[5] 0: SAI Audio Data length is 16L + 16R 
    tunerStatus = TUN_Set_Audio_IF(I2C_SLAVE_ADDRESS, 0x03, 0xA57310);

    if (tunerStatus != RET_SUCCESS)
    {
        printf("Audio IF setup fai.l (%d)\n", tunerStatus);
        return eRET_NG_UNKNOWN;
    }
    else
    {
        printf("Audio IF setup is complete.\n");
    }

    // initial tune
    tunerStatus = star_setTune(type.initMode, type.initFreq, 0, eTUNER_DRV_ID_PRIMARY);
    tunerStatus |= star_setTune(type.initMode, type.initFreq, 0, eTUNER_DRV_ID_SECONDARY);
    
    if (tunerStatus != RET_SUCCESS)
    {
        printf("initial tune fail. (%d)\n", tunerStatus);
        return eRET_NG_UNKNOWN;
    }
    else
    {
        printf("initial tune success.\n");
        return eRET_OK;
    }
}

int star_close(void)
{
    int ret = 0;

    ret = star_i2c_close();

    if (ret < 0)
        return eRET_NG_UNKNOWN;
    
    star_tuner_reset();
    
    star_drv_initialized = 0;

    memset(star_drv_current_band, 0xff, sizeof(star_drv_current_band));
    memset(star_drv_fm_frequency, 0xff, sizeof(star_drv_fm_frequency));
    memset(star_drv_am_frequency, 0xff, sizeof(star_drv_am_frequency));

    return ret;
}

int star_setIQTestPattern(unsigned int fOnOff, unsigned int sel)
{
    (void)fOnOff;
    (void)sel;

    return 0;
}

