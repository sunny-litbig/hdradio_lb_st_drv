/*******************************************************************************

*   FileName : tcradio_rds_def.h

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
#ifndef __TCRADIO_RDS_DEF_H__
#define __TCRADIO_RDS_DEF_H__

/***************************************************
*				Include					*
****************************************************/
#ifdef __ANDROID__
#include <android/log.h>
#endif

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "DMBLog.h"

#define RADIO_RDS_DEBUG

#ifdef __ANDROID__

#define RDS_TAG				("[RADIO][RDS]")
#define RDS_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,RDS_TAG, __VA_ARGS__))
#ifdef RADIO_RDS_DEBUG
#define RDS_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,RDS_TAG, __VA_ARGS__))
#else
#define	RDS_DBG(...)
#endif

#else // #ifdef __ANDROID__

// #define RDS_ERR(...)		((void)printf("[ERROR][RADIO][RDS]: " __VA_ARGS__))
#define RDS_ERR(...)		((void)LB_PRINTF("[ERROR][RADIO][RDS]: " __VA_ARGS__))
#ifdef RADIO_RDS_DEBUG
// #define RDS_DBG(...)		((void)printf("[DEBUG][RADIO][RDS]: " __VA_ARGS__))
#define RDS_DBG(...)		((void)LB_PRINTF("[DEBUG][RADIO][RDS]: " __VA_ARGS__))
#else
#define	RDS_DBG(...)
#endif

#endif // #ifdef __ANDROID__

#define	RDS_THREAD_TIME_INTERVAL	20

#define setBit(x, pos)			((x)|=(unsigned char)(1<<(pos)))
#define	clrBit(x, pos)			((x)&=~((unsigned char)(1<<(pos))))
#define	valBit(x, pos)			((x)&(unsigned char)(1<<(pos)))
#define	mskBit(dst, mask, src)	((dst)=(unsigned char)((src)&(mask))|(unsigned char)((dst)&(~(mask))))

#define	MAX_AF		26		/* Current Station 1 + AF 25 */
#define	MAX_WAF		6		/* Weighted AF Number */
#define	MAX_FAF		15		/* Forbidden AF Number */

#define GRP_0A		0x00	/* 0A/0X	: PI, PTY, TP, PS, TA, DI, M/S, AF.     */
#define GRP_0B		0x08	/* 0B		: PI, PTY, TP, PS, TA, DI, M/S.         */
#define GRP_1A		0x10	/* 1A/1X	: PI, PTY, TP, PIN.                     */
#define GRP_1B		0x18	/* 1B		: PI, PTY, TP, PIN.                     */
#define GRP_2A		0x20	/* 2A/2X	: PI, PTY, TP, RT.                      */
#define GRP_2B		0x28	/* 2B		: PI, PTY, TP, RT.                      */
#define GRP_3A		0x30	/* 3A/3X	: PI, PTY, TP, not define yet.          */
#define GRP_3B		0x38	/* 3B		: PI, PTY, TP, not define yet.          */
#define GRP_4A		0x40	/* 4A/4X	: PI, PTY, TP, time and date.           */
#define GRP_5A		0x50	/* 5A/5X	: PI, PTY, TP, external data use.       */
#define GRP_5B		0x58	/* 5B		: PI, PTY, TP, external data use.       */
#define GRP_6A		0x60	/* 6A/5X	: PI, PTY, TP, internal application.    */
#define GRP_6B		0x68	/* 6B		: PI, PTY, TP, internal application.    */
#define GRP_7A		0x70	/* 7A		: PI, PTY, TP, radio messagery.         */
#define GRP_8A		0x80	/* 8A		: PI, PTY, TP, not complete yet.        */
#define GRP_9A		0x90	/* 9A		: PI, PTY, TP, emergency warning system.*/
#define GRP_14A		0xE0	/* 14A/15X	: PI, PTY, TP, EON */
#define GRP_14B		0xE8	/* 14B		: PI, PTY, TP, EON */
#define	GRP_15A		0xF0	/* 15A		: not used */
#define GRP_15B		0xF8	/* 15B		: PI, PTY, TP (2 times) */

// To tatpStatus
#define RDS_TA_VALUE    4    /*  7   6   5    4      3      2      1      0  */
#define RDS_TA_VALID    3    /*            |<----TA----->|<----TP----->| New */
#define RDS_TP_VALUE    2    /*  X   X   X  Value Control Value Control      */
#define RDS_TP_VALID    1
#define RDS_TATP_NEW    0

// To ptyStatus
#define	RDS_PTY_VALID	0x40

// To psStatus
#define RDS_PS_VALID	  3		/* PS  : PS available for appli.               */
#define RDS_PS_NEW        2		/* NEW : New PS stored and available for appli */

// To msStatus
#define RDS_MS_VALUE    3          /* 7   6   5   4    3     2      1      0 */
#define RDS_MS_CHANGE   2          /*               |<---------MS------->|   */
#define RDS_MS_CTRL     1          /* X   X   X   X  Value Change Control  X */

/* To extStatus */					/* PS_SEG EONB_S EONC_S RT CTB CTC  X  X */
#define RDS_PS_SEG_OK      7		/* PS segment availibility bit.*/
#define RDS_EONB_SEG_OK    6        /* EON block B segment availibility bit.*/
#define RDS_EONC_SEG_OK    5        /* EON block C segment availibility bit.*/
#define RDS_RT_SEG_OK      4		/* RT segment availibility bit.*/
#define RDS_CTB_SEG_OK     3		/* CT block B segment availibility bit.*/
#define RDS_CTC_SEG_OK     2		/* CT block C segment availibility bit.*/

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum
{
	eRDS_BLOCK_A		= 0,
	eRDS_BLOCK_B		= 1,
	eRDS_BLOCK_C		= 2,
	eRDS_BLOCK_c		= 3,
	eRDS_BLOCK_D		= 4
}eRDS_BLOCK_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct{
	int32 fThreadRunning;
	int32 fEnable;
	int32 rdsFetchCounter;
	eRDS_EVENT_t eMainMode;

	/* current frequency rds data */
	uint8 pih;
	uint8 pil;
	uint8 pty;
	uint8 psname[MAX_PS];

	uint8 af[MAX_AF];
	uint8 availableAf[MAX_AF];
	uint8 weightedAf[MAX_WAF];
	uint8 forbiddenAf[MAX_FAF];

	/* for rds parsing */
	uint8 blockAH;
	uint8 blockAL;
	uint8 blockBH;
	uint8 blockBL;
	uint8 blockCH;
	uint8 blockCL;
	uint8 blockDH;
	uint8 blockDL;

	uint8 group;
	uint8 piCounter;
	uint8 piStatus;
	uint8 tatpStatus;
	uint8 psStatus;
	uint8 psbuf[MAX_PS];
	uint8 ptyStatus;
	uint8 msStatus;
	uint8 extStatus;
}stRDS_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/
extern stRDS_t stRds;

/***************************************************
*			Function declaration				*
****************************************************/
extern void tcrds_fetchRdsDataHandler(void);

extern void tcrds_setEnable(int32 fonoff);

#ifdef __cplusplus
}
#endif

#endif
