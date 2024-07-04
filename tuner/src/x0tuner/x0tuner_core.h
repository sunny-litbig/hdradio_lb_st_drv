/*******************************************************************************

*   FileName : x0tuner_core.h

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
#ifndef __X0TUNER_CORE_H__
#define __X0TUNER_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	eTUNER_DRV_TUNE2_NORMAL 	= 0,
	eTUNER_DRV_TUNE2_SEARCH		= 1,
	eTUNER_DRV_TUNE2_AFUPDATE	= 2,
	eTUNER_DRV_TUNE2_JUMP		= 3,
	eTUNER_DRV_TUNE2_CHECK		= 4,
	eTUNER_DRV_TUNE2_END		= 6
}eTUNER_DRV_TUNE2_MODE_t;

typedef union {
	struct {
		uint32 Status;		// Status
		uint32 Rssi;		// Level detector result	// -200 ... 1200 (*0.1dBuV) = -20 ... 120dBuV RF input level
		uint32 Usn;			// Ultra-sonic noise.		// 0 ... 1000 (*0.1%) = 0 ... 100% relative ultrasonic noise detector result.
		uint32 Mpth; 		// Multipath				// 0 ... 1000 (*0.1%) = 0 ... 100% relative wideband-AM multipath detector result.
		uint32 Offs;		// Offset					// -1200 ... 1200 (*0.1Khz) = -120 ... 120Khz radio frequency error
		uint32 Bwth;		// Bandwidth				// 560 ... 3110 (*0.1Khz) = IF bandwidth 56 ... 311 Khz
		uint32 Mod;			// Modulation				// 0 ... 1000 (*0.1%) = 0 ... 100% modulation = 0 ... 75Khz FM dev.
	} fm;

	struct {
		uint32 Status;		// Status
		uint32 Rssi;		// Level detector result 	// -200 ... 1200 (*0.1dBuV) = -20 ... 120dBuV RF input level
		uint32 Hfn;			// High frequency noise detector	// 0 ... 50000 (*0.1%) = 0 ... 5000% noise relative to wanted signal
														// 1000 = 100% is approximate equal noise and wanted signal
		uint32 Coch;		// Co-Channel detector		// 0 = no co-channel detected, 1 = co-channel detected (based on selected criteria)
		uint32 Offs;		// Offset					// -1200 ... 1200 (*0.1Khz) = -120 ... 120Khz radio frequency error
		uint32 Bwth;		// Bandwidth				// 30 ... 80 (*0.1Khz) = IF bandwidth 3 ... 8 Khz
		uint32 Mod;			// Modulation				// 0 ... 1000 (*0.1%) = 0 ... 100% AM modulation index
														// 1000 ... 2000 (*0.1%) = 100% ... 200% peak modulation range
	} am;
}stX0_DRV_QUALITY_t;

extern RET x0tuner_open(stTUNER_DRV_CONFIG_t type);
extern RET x0tuner_close(void);
extern RET x0tuner_setTune(uint32 mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner);
extern RET x0tuner_getTune(uint32 *mod_mode, uint32 *curfreq, uint32 ntuner);
extern RET x0tuner_getQuality(uint32 mod_mode, stX0_DRV_QUALITY_t *qdata, uint32 ntuner);
extern float32 x0tuner_getPreciseIqSampleRate(uint32 ntuner);
extern int32 x0tuner_getIqSampleRate(uint32 ntuner);
extern int32 x0tuner_getIqSamplingBit(uint32 ntuner);
extern RET x0tuner_setMute(uint32 fOnOff, uint32 ntuner);
extern RET x0tuner_i2cTx(uint8 addr, uint8 *tx, uint32 len);
extern RET x0tuner_i2cRx(uint8 addr, uint8 *reg, uint32 reglen, uint8* rx, uint32 rxlen);
extern uint32 x0tuner_getVersion(void);

#ifdef __cplusplus
}
#endif

#endif /* __X0TUNER_CORE_H__ */
