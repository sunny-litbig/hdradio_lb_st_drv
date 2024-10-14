/*******************************************************************************

*   FileName : tcradio_rds_api.h

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
#ifndef __TCRADIO_RDS_API_H__
#define __TCRADIO_RDS_API_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define	MAX_PS		8

#define NO_FREQ     0xFF
#define NO_PI       0x00
#define NO_PTY      0x00

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eRDS_STS_OK					= 0,	/* Job Complete & Notify */
	eRDS_STS_ERROR				= 1,	/* Critical Error */
	eRDS_STS_DOING				= 4,	/* Job Continue */
	eRDS_STS_DOING_NOTIFY		= 5,	/* Job Continue & Notify Case*/
	eRDS_STS_DOING_ERROR_NOTIFY	= 6,	/* Job COntinue & Error Notify */
	eRDS_STS_OK_NOTIFY			= 7,	/* Job Complete & Notify Case */
	eRDS_STS_WAIT				= 8,
	eRDS_STS_DONE				= 9,
	eRDS_STS_END
} eRDS_STS_t;

typedef enum
{
	eRDS_CMD_NULL		= 0,

	eRDS_CMD_DEINIT		= 2,
	eRDS_CMD_OPEN		= 3,
	eRDS_CMD_CLOSE		= 4,
	eRDS_CMD_RESET		= 5,

	eRDS_CMD_END
}eRDS_CMD_t;

typedef enum
{
    eRDS_EVT_NULL		= 0,

	eRDS_EVT_DEINIT		= 2,
	eRDS_EVT_OPEN		= 3,
	eRDS_EVT_CLOSE		= 4,
	eRDS_EVT_RESET		= 5,

    eRDS_EVT_END
}eRDS_EVENT_t;

typedef enum
{
	eRDS_NOTIFY_NULL	= 0,

	eRDS_NOTIFY_DEINIT	= 2,
	eRDS_NOTIFY_OPEN	= 3,
	eRDS_NOTIFY_CLOSE	= 4,
	eRDS_NOTIFY_RESET	= 5,

	eRDS_NOTIFY_END
}eRDS_Notify_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct{
	uint32 uiPi;
	uint32 uiFreq;
	int32 iQuality;
}stRDS_SCAN_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern RET tcrds_init(void);
extern RET tcrds_deinit(void);
extern RET tcrds_open(uint32 ntuner);
extern RET tcrds_close(uint32 ntuner);
extern int32 tcrds_getEnable(void);

extern uint16 tcrds_getPi(void);
extern uint8 tcrds_getPty (void);
extern uint8 tcrds_getPsValid(void);
extern uint8 tcrds_getPs(uint8 tcrds_psidx);
extern uint8 tcrds_getRTValid(void);
extern void tcrds_getRT(uint8 *rt);


#ifdef __cplusplus
}
#endif

#endif
