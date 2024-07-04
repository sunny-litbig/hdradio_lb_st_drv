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
 * file cmdTune.h
 * brief Ibiquity Command Processor Tune API
 */
#ifndef CMD_TUNE_H_
#define CMD_TUNE_H_

typedef enum {
    TUNE_ALL                = 0x00,
    TUNE_RESERVED           = 0x00,
    TUNE_ANALOG             = 0x01,
    TUNE_DIGITAL            = 0x02,
    TUNE_HYBRID             = 0x03,
    TUNE_PENDING            = 0xFF
}HDR_tune_method_t;

typedef enum{
    TUNE_STEP_MANUAL        = 0x01
}HDR_tune_step_method_t;

typedef enum{
    DISTANCE = 0x00,
    LOCAL    = 0x01
}HDR_tune_distance_t;

typedef enum {
    TUNE_DOWN = 0,
    TUNE_UP
}HDR_tune_direction_t;

typedef enum {
    TUNE_MULTICAST_SKIP_SPS = 0x00,
    TUNE_MULTICAST_USE_SPS  = 0x01
}HDR_tune_multicast_t;

typedef struct{
    HDR_tune_band_t band;
    U16 rfFreq;
    U16 fmMinTuningFreq;
    U16 fmMaxTuningFreq;
    U8  fmFreqSpacing;
    U16 amMinTuningFreq;
    U16 amMaxTuningFreq;
    U8  amFreqSpacing;
}CMD_tune_params_t;

S32 SYS_procTune(HDR_instance_t *hdrInstance, const U8* dataIn, U32 inLength, U8* dataOut, U32 * outLength);

#endif //CMD_TUNE_H_
