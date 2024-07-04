//****************************************************************************************************************
//
// File name   :	star_protocol.h  
// Author	   :	SZ team (Michael LIANG)
// Description :	ST Star Tuner(TDA7707 / TDA7708) command protocol layer header file
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


#ifndef H_STAR_PROTOCOL
#define H_STAR_PROTOCOL
#include "star_i2c.h"

Tun_Status Star_I2C_Direct_Write(int deviceAddress, tU32 regAddress, tU8 *pData, int dataSize);
Tun_Status Star_I2C_Direct_Read(int deviceAddress, tU32 regAddress, tU8* pData, int dataSize);
Tun_Status Star_Command_Communicate(int deviceAddress, tU8 cmdID, int paramNum, tU8 *pCmdData, tU8 ansParamNum,  tU8 *pAnswer, tBool bGetAnswerSizeFromHeader, int *pAnswerParamNum, tBool bCheckCheckSum);

#endif /* H_STAR_PROTOCOL */



