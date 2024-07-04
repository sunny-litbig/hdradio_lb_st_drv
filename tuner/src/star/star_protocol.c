//****************************************************************************************************************
//
// File name   :    star_protocol.c 
// Author       :    SZ team (Michael LIANG)
// Description :    ST Star Tuner(TDA7707 / TDA7708) Tuner command protocol layer.
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

#include "star_public.h"
#include "star_protocol.h"

/*************************************************************************************
Function        : Command_CalculateCheckSum
Description    : Calculate the checksum for command access, and set CheckSum, as it's command usage,
              one word data is always 3 bytes.
              
Parameters    :  
            wordNum :  the parameter number + 1 (command header),   one word means 3 bytes.
                that is the number of the words which need to do summing.
                
            pData : the data buffer which need to calculate the check sum
        
Return Value    : the check sum value in tU32.
*************************************************************************************/
static tU32 Command_CalculateCheckSum(tU8 *pData, int wordNum)
{
    tU32  checkSum = 0;
    
    for (int i = 0; i <  wordNum * 3; i++)   checkSum += (tU32)(*pData++) << ((2 - i%3) * 8);     
    
    return checkSum & 0x00FFFFFF;
}


/*******************************************************************************************
Function        : Command_Send
Description    : send command to Star tuner

Parameters    :
            deviceAddress : device I2C address
            
            cmdID :     command ID   (command ID will be put inside the command header)
            
            paramNum : parameter numbers  (one parameter = 3 bytes)
            
            pData : the pointer of the buffer which will be sent to tuner.
             
Return Value    : Tun_Status            
*********************************************************************************************/
static Tun_Status Command_Send(tU32 deviceAddress, tU8 cmdID, int paramNum, tU8* pData)
{
/*    
    |command header    | command header    | Parameter1    |Parameter2    |....         | CheckSum    |
    |   Part1            |    part2        |            |            |        |            |
    | device address    |                |            |            |        |            |
    | + Fixed address    |                |            |            |        |            |
    |   0xF20018        |                |            |            |        |            |
    |    4 bytes        |    3 bytes        |3 bytes        |3 bytes        |...        | 3bytes        |
                    |                                                |        
                    |<---         to calculate check sum                         --->    |

*/
    Tun_Status tunerStatus = RET_ERROR;
    tU32 checkSum;
    int ret, byteNum = paramNum * 3 + 9;   /* Commander header (not include device address)+ checksum*/
    tU8  dataBuffer[byteNum];

    memset(dataBuffer, 0x00, byteNum);
    
    dataBuffer[0] = 0xF2;
    dataBuffer[1] = 0x01;
    dataBuffer[2] = 0x80;    
    
    dataBuffer[4] = cmdID;
    dataBuffer[5] = paramNum & 0xFF;

    memcpy(dataBuffer + 6, pData, paramNum * 3);

    /* Check sum : command header part2 + parameters */
    checkSum = Command_CalculateCheckSum(dataBuffer + 3, paramNum + 1);
    
    dataBuffer[byteNum - 3] = (checkSum >> 16) & 0xFF;
    dataBuffer[byteNum - 2] = (checkSum >> 8) & 0xFF;
    dataBuffer[byteNum - 1] = checkSum & 0xFF;

#if 0
    ret = i2c_writing(deviceAddress, dataBuffer, byteNum);
#else   // LB i2c
    ret = star_i2c_write(deviceAddress, dataBuffer, byteNum);
#endif

    if (ret >= 0) tunerStatus = RET_SUCCESS;

    return tunerStatus;
}


/*************************************************************************************
Function        : Command_GetAnswer
Description    : Request and recieve command answer from Star tuner
              The caller need allocate enough data buffer to read out the command answer buffer.

Parameters    :
            deviceAddress : device I2C address
            
            ansParamNum : the command answer word number which need to be read out from tuner.
            
            pAnswer : the memory pointer which the command answer data will be stored to.
            
            bGetAnswerSizeFromHeader : if true, will get the answer size from answer header.
            
            pAnswerParamNum : the real answer parameter number got from tuner.
        
Return Value    : Tun_Status        
*************************************************************************************/
static Tun_Status Command_GetAnswer(int deviceAddress, int ansParamNum, tU8 *pAnswer, tBool bGetAnswerSizeFromHeader, int *pAnswerParamNum)
{
    Tun_Status tunerStatus = RET_ERROR;
    tU8 cmdAddr[3] = {0x72, 0x01, 0x80};
    int  ret = 0;
    
#ifdef TRACE_STAR_CMD_PROTOCOL
    PRINTF("ansParamNum:%d, bGetAnswerSizeFromHeader:%d", ansParamNum, bGetAnswerSizeFromHeader);
#endif

    if (bGetAnswerSizeFromHeader)
    {
        /* read the answer header first*/
#if 0
        ret = i2c_reading(deviceAddress, cmdAddr, 3, pAnswer, 3);
#else   // LB i2c
        ret = star_i2c_read(deviceAddress, cmdAddr, 3, pAnswer, 3);
#endif
        if (ret >= 0)
        {
            /* get the number of parameters */
            int answerParmNum = *(pAnswer + 2);
            *pAnswerParamNum = answerParmNum;

            /* read out answer parameters and checksum */
#if 0
            ret = i2c_read_bytes(deviceAddress, pAnswer + 3, answerParmNum * 3 + 3);
#else   // LB i2c
            ret = star_i2c_read_byte_only(deviceAddress, pAnswer + 3, answerParmNum * 3 + 3);
#endif
        }
    }
    else
    {
        /* Read (parameter num + 1 answer header + 1 checksum) words  */
#if 0
        ret = i2c_reading(deviceAddress, cmdAddr, 3, pAnswer, (ansParamNum + 2) * 3);
#else   // LB i2c
        ret = star_i2c_read(deviceAddress, cmdAddr, 3, pAnswer, (ansParamNum + 2) * 3);
#endif
        if (ret >= 0) *pAnswerParamNum = ansParamNum;
    }

    if (ret >= 0) tunerStatus = RET_SUCCESS;
    
    return tunerStatus;
}


/*************************************************************************************
Function        : Star_Command_Communicate
Description    : Send command, request and read command answer from Star tuner, follow star command protocol.

Parameters    :
            deviceAddress : device I2C address
            
            cmdID :     command ID   
            
            paramNum : parameter numbers  (one parameter = 3 bytes)
            
            pCmdData : the pointer of the command data buffer which will be sent to tuner.
             
            ansParamNum : the command answer word number which need to be read out from tuner.
            
            pAnswer : the memory pointer which the command answer data will be stored to.
            
            bGetAnswerSizeFromHeader : if true, will get the answer size from answer header.
            
            pAnswerParamNum : the real answer parameter number got from tuner.
            
            bCheckCheckSum : if ture, will check the command answer checksum, will return RET_ERR_CMD_CHECKSUM if error happens.
            
Return Value    : Tun_Status        
*************************************************************************************/
Tun_Status Star_Command_Communicate(int deviceAddress, tU8 cmdID, int paramNum, tU8 *pCmdData, tU8 ansParamNum,  tU8 *pAnswer, tBool bGetAnswerSizeFromHeader, int *pAnswerParamNum, tBool bCheckCheckSum)
{
    Tun_Status tunerStatus = RET_ERROR;
    
    tunerStatus = Command_Send(deviceAddress, cmdID, paramNum, pCmdData);
    
#ifdef TRACE_STAR_CMD_PROTOCOL
    PRINTF("Command_Send = %s, cmdID = %d, paramNum = %d, ansParamNum = %d, GetAnswerSizeFromHeader = %d", RetStatusName[tunerStatus].Name, cmdID, paramNum, ansParamNum, bGetAnswerSizeFromHeader);
#endif

    if (tunerStatus == RET_SUCCESS)
    {
        //Wait 90us,    ~= 4 DSP audio cycle, need pay attention with high speed I2C communication
        usleep(7*1000); // LB check and change 6 ms + 1ms margin
        tunerStatus = Command_GetAnswer(deviceAddress, ansParamNum, pAnswer, bGetAnswerSizeFromHeader, pAnswerParamNum);
        
#ifdef TRACE_STAR_CMD_PROTOCOL
        PRINTF("Command_GetAnswer = %s, *pAnswerParamNum = %d", RetStatusName[tunerStatus].Name, *pAnswerParamNum);
#endif
        
        if (tunerStatus == RET_SUCCESS) 
        {
            if ((*pAnswer) & 0x20) return RET_ERR_CMD_COLLISION;
            if ((*pAnswer) & 0x40) return RET_ERR_CMD_ID;
            if ((*pAnswer) & 0x80) return RET_ERR_CMD_CHECKSUM;
            
            if (bCheckCheckSum)
            {
                /*    answer structure
                |answer header    | Parameter1        | Parameter2    |Parameter3    |....         | CheckSum    |
                |    3bytes        |    3 bytes        |3 bytes        |3 bytes        |...        | 3bytes        |    */
                
                tU32 checkSumInAnswer;
                int    offsetCheckSum = *(pAnswer + 2) * 3 + 3;
                
                /*  checksum in answer */
                checkSumInAnswer   = (tU32)(*(pAnswer + offsetCheckSum))     << 16;    
                checkSumInAnswer |= (tU32)(*(pAnswer + offsetCheckSum + 1)) << 8;
                checkSumInAnswer |= (tU32)(*(pAnswer + offsetCheckSum + 2));

                if (checkSumInAnswer != Command_CalculateCheckSum(pAnswer, *(pAnswer + 2) + 1)) tunerStatus = RET_ERR_CMD_ANWSER_CHECKSUM;
            }
        }
    }

    return tunerStatus;
}

/*******************************************************************************************
Function        : Star_I2C_Direct_Write
Description    : I2C direct write to Star tuner, followed tuner' direct wirite protocol

Parameters    :
            deviceAddress : device I2C address
            
            regAddress :     star tuner's regester address
            
            pData : the pointer of the buffer which will be sent to tuner.

            dataSize : byte numuber will be send
             
Return Value    : Tun_Status            
*********************************************************************************************/
Tun_Status Star_I2C_Direct_Write(int deviceAddress, tU32 regAddress, tU8 *pData, int dataSize)
{
    Tun_Status tunerStatus = RET_ERROR;
    tU8 dataBuffer[3 + dataSize];
    int ret;
    
    dataBuffer[0] = ((regAddress >> 16) & 0x0F) | 0xE0;
    dataBuffer[1] = (regAddress >> 8) & 0xFF;
    dataBuffer[2] = (regAddress) & 0xFF;
    
    memcpy(dataBuffer + 3,  pData,  dataSize);
    
#if 0
    ret = i2c_writing(deviceAddress, dataBuffer, 3 + dataSize);
#else   // LB i2c
    ret = star_i2c_write(deviceAddress, dataBuffer, 3 + dataSize);
#endif

#ifdef TRACE_STAR_CMD_PROTOCOL
    PRINTF("i2c_writing = %d, dataBuffer = 0x%02x%02x%02x ~~, regAddress = 0x%06x, size = %d", ret, dataBuffer[0], dataBuffer[1], dataBuffer[2], regAddress, dataSize);
#endif

    if (ret >= 0) tunerStatus = RET_SUCCESS;

    return tunerStatus;
}

/*******************************************************************************************
Function        : Star_I2C_Direct_Read
Description    : I2C direct read from Star tuner, followed tuner' direct wirite protocol

Parameters    :
            deviceAddress : device I2C address
            
            regAddress :     star tuner's regester address
            
            pData : the pointer of the buffer which will be saved to with data from tuner .

            dataSize : byte numuber will be send
             
Return Value    : Tun_Status            
*********************************************************************************************/
Tun_Status Star_I2C_Direct_Read(int deviceAddress, tU32 regAddress, tU8* pData, int dataSize)
{
    Tun_Status tunerStatus = RET_ERROR;
    tU8 dataBuffer[3];
    int ret; 

    dataBuffer[0] = ((regAddress >> 16) & 0x0F) | 0x60;
    dataBuffer[1] = (regAddress >> 8) & 0xFF;
    dataBuffer[2] = (regAddress) & 0xFF;
    
#if 0
    ret =  i2c_reading(deviceAddress, dataBuffer, 3, pData, dataSize);
#else   // LB i2c
    ret = star_i2c_read((unsigned int)deviceAddress, dataBuffer, 3, pData, (unsigned short)dataSize);
#endif

#ifdef TRACE_STAR_CMD_PROTOCOL
    PRINTF("i2c_reading = %d, write buffer = 0x%02x%02x%02x, read out data size = %d, out data = 0x%02x%02x%02x%02x", ret, dataBuffer[0], dataBuffer[1], dataBuffer[2], dataSize, *pData, *(pData + 1), *(pData + 2), *(pData + 3));
#endif

    if (ret >= 0) tunerStatus = RET_SUCCESS;
    
    return tunerStatus;
}

