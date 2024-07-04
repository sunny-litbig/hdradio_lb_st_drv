/*******************************************************************************

*   FileName : max2175_core.h

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
#ifndef _MAX2175_CORE_H_
#define _MAX2175_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

extern RET max2175_open(stTUNER_DRV_CONFIG_t type);
extern RET max2175_close(void);
extern RET max2175_setBand(uint32 mod_mode, uint32 ntuner);
extern RET max2175_setTune(uint32 mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner);
extern RET max2175_getTune(uint32 *mod_mode, uint32 *curfreq, uint32 ntuner);
extern RET max2175_getRFgain(int32 *rfdata, uint32 ntuner);
extern RET max2175_setMute(uint32 fOnOff, uint32 ntuner);
extern RET max2175_i2cTx(uint8 addr, uint8 *tx, uint32 len);
extern RET max2175_i2cRx(uint8 addr, uint8 *reg, uint32 reglen, uint8* rx, uint32 rxlen);
extern uint32 max2175_getVersion(void);
extern float32 max2175_getPreciseIqSampleRate(uint32 ntuner);
extern int32 max2175_getIqSampleRate(uint32 ntuner);
extern int32 max2175_getIqSamplingBit(uint32 ntuner);

#ifdef __cplusplus
}
#endif

#endif
