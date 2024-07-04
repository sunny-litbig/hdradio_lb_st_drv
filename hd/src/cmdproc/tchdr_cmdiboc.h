/*******************************************************************************
*
* (C) copyright 2003-2014, iBiquity Digital Corporation, U.S.A.
*
********************************************************************************

    This confidential and proprietary software may be used only as
    authorized by a licensing agreement from iBiquity Digital Corporation.
    In the event of publication, the following notice is applicable:

    The availability of this material does not provide any license
    by implication, or otherwise under any patent rights of iBiquity
    Digital Corporation or others covering any use of the
    contents herein.

    Any copies or derivative works must include this and all other
    proprietary notices.

        iBiquity Digital Corporation
        6711 Columbia Gateway Drive, Suite 500
        Columbia, MD USA 21046
*******************************************************************************/
/**
 * file	: cmdIboc.h
 * brief
 */
#ifndef IBOC_CONFIG_H_
#define IBOC_CONFIG_H_

#include "hdrCore.h"
#include "tchdr_cmdopcode.h"

CMD_dispatch_rc_t IBOC_procHostCommand(HDR_instance_t* hdrInstance, CMD_opcode_t opCode, const U8* dataIn, U32 inLength, U8* dataOut, U32 * outLength);

#endif /* CMD_IBOC_H_ */
