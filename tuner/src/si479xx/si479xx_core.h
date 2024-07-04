/*******************************************************************************

*   FileName : si479xx_core.h

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
#ifndef _SI479X_CORE_H_
#define _SI479X_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
	eTUNER_DRV_TUNE1_NORMAL		= 0,
	eTUNER_DRV_TUNE1_FAST		= 1,
	eTUNER_DRV_TUNE1_AFTUNE		= 2,
	eTUNER_DRV_TUNE1_AFCHECK	= 3,
	eTUNER_DRV_TUNE1_HDVALID	= 4
}eTUNER_DRV_TUNE1_MODE_t;

typedef union {
	struct {
		uint32 Flag;		// HDSEARCHDONE, BLEDPIN
		uint32 Valid;		// Valid Status
		uint32 Freq;		// Frequency
		uint32 Rssi;		// FM Received signal strength
		uint32 Snr;			// FM RF carrier to noise
		uint32 Dev;			// FM Deviation
		uint32 Offs;		// FM Offset
		uint32 Pilot;		// FM Pilot
		uint32 Mpth; 		// FM Multipath
		uint32 Usn;			// FM Ultra-sonic noise.
		uint32 Hdlevel;
	} fm;

	struct {
		uint32 Flag;		// HDSEARCHDONE, BLEDPIN
		uint32 Valid;		// Valid Status
		uint32 Freq;		// Frequency
		uint32 Rssi;		// AM Received signal strength
		uint32 Snr;			// AM RF carrier to noise
		uint32 Mod;			// AM Modulation
		uint32 Offs;		// AM Offset
		uint32 Hdlevel;
	} am;

	struct {
		uint32 Flag;		// unused
		uint32 Valid;
		uint32 Index;
		uint32 Freq;
		uint32 Sqi;
		uint32 Rssi;
		uint32 Detect;
		uint32 RssiAdj;
		uint32 Dagc;
	} dab;
}stSILAB_DRV_QUALITY_t;

extern stTUNER_DRV_CONFIG_t gSilabConf;

extern RET si479xx_open(stTUNER_DRV_CONFIG_t type);
extern RET si479xx_close(void);
extern uint8 si479xx_getMaxDabFreqList(void);
extern uint8 si479xx_getCurrentDabFreqList(uint32 ntuner);
extern RET si479xx_setDabFreqList(uint32 * dab_freq_hz_table, uint8 num_freq, uint32 ntuner);
extern RET si479xx_getDabFreqList(uint32 *dab_freq_hz_table, uint8 *num_freq, uint32 ntuner);
extern RET si479xx_setTune(uint32 mod_mode, uint32 freq, uint32 tune_mode, uint32 ntuner);
extern RET si479xx_getTune(uint32 *mod_mode, uint32 *curfreq, uint32 ntuner);
extern RET si479xx_getQuality(uint32 mod_mode, stSILAB_DRV_QUALITY_t *qdata, uint32 ntuner);
extern RET si479xx_setMute(uint32 fOnOff, uint32 ntuner);
extern RET si479xx_setRdsConfig(uint32 fOnOff, uint32 blethB, uint32 blethCD, uint32 ntuner);
extern RET si479xx_getRdsData(uint8 *rdsbuf, uint32 ntuner);
extern RET si479xx_setRdsClear(uint32 ntuner);
extern RET si479xx_cspiTxRxData(uint8 *tx, uint8 *rx, uint32 len, uint32 cs);
extern uint32 si479xx_getVersion(void);
extern float32 si479xx_getPreciseIqSampleRate(uint32 ntuner);
extern int32 si479xx_getIqSampleRate(uint32 ntuner);
extern int32 si479xx_getIqSamplingBit(uint32 ntuner);
extern RET si479xx_getStatus(uint32 ntuner, uint32 cmd, uint8 *rx, uint8 len);
extern RET si479xx_setIQTestPattern(uint32 fOnOff, uint32 sel);
extern RET si479xx_setCommand(uint32 cmd_size, uint8 *cmd, uint32 reply_size, uint8 *reply, uint32 ntuner);
extern RET si479xx_setDigitalAGC(uint8 fOnOff, int8 target_power, uint8 inc_ms, uint8 dec_ms, uint32 ntuner);  // Available from F/W v4.x.x.x.x
extern RET si479xx_setPropertyOfPrimaryTuner(uint8 group, uint8 number, uint8 hdata, uint8 ldata);

#ifdef __cplusplus
}
#endif

#endif
