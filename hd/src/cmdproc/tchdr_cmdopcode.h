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
 * file	: opcodeDefs.h
 * brief
 */
#ifndef OPCODE_DEFS_H_
#define OPCODE_DEFS_H_

//Never use Case 0.
#define SYS_CONFIG_GROUP        case 0x80: case 0x82: case 0x83: case 0x84: \
                                case 0x8B: case 0x8C: case 0x8D:

#define IBOC_MSG_CROUP          case 0x02: case 0x20: case 0x21: case 0x60: \
                                case 0x91: case 0x99: case 0x9D: case 0xA0: \
                                case 0xA1: case 0xA2:

#define SISPSD_MSG_GROUP	    case 0x44: case 0x45: case 0x46: \
                                case 0x47: case 0x48: case 0x4B: case 0x93:

#define AAS_MSG_GROUP			case 0x41: case 0x42: case 0x49: case 0x4A: \
                                case 0x4C: case 0x4D:

#define INTERNAL_MSG_GROUP

/**
 * opCodes defined in RX_IDD_2206 - HD Radio Commercial Receiver BBP Command and Data Interface Definition - Revision 10.pdf
 */
typedef enum{
    BAND_SELECT = 0x02,
    GET_STATUS = 0x20,
    GET_QI = 0x21,
    AAS_ENABLE_PORTS = 0x41,
    AAS_GET_ENABLED_PORTS = 0x42,
    FLUSH_PSD_QUEUE = 0x44,
    SET_SIS_CNFG = 0x45,
    GET_SIS_CNFG = 0x46,
    GET_EXT_SIS_DATA = 0x47,
    FLUSH_SIS_DATA = 0x48,
    AAS_GET_DATA = 0x49,
    AAS_FLUSH_QUEUE = 0x4A,
    GET_SERVICE_INFO = 0x4B,
    AAS_PROC_LOT = 0x4C,
    SIG_GET_DATA = 0x4D,
    ACTIVE_RADIO = 0x60,
    CA_CNTRL = 0x75,
    SYS_VERSION = 0x80,
    SYS_TUNE = 0x82,
    SYS_CNTRL_CNFG = 0x83,
    SYS_NVM_ACCESS = 0x84,
    SYS_AUDIO_CNTRL = 0x8B,
    AIM_CNTRL = 0x8C,
    SYS_DIAGNOSTICS = 0x8D,
    IBOC_CNTRL_CNFG = 0x91,
    PSD_DECODE = 0x93,
    ADVANCED_AUDIO_BLENDING = 0x99,
    IBOC_DIAGNOSTICS = 0x9D,
    ANALOG_AM_DEMOD = 0xA0,
    ANALOG_FM_DEMOD = 0xA1,
    RBDS_CNTRL = 0xA2
}CMD_opcode_t;

typedef enum {
    CMD_DISPATCH_OK = 0,
    CMD_UNSPECIFIED_ERROR  = -1,
    CMD_UNSUPPORTED_OPCODE = -2,
    CMD_UNSUPPORTED_INSTANCE = -3
}CMD_dispatch_rc_t;

#endif /* OPCODE_DEFS_H_ */
