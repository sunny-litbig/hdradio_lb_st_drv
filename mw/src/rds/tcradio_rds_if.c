/*******************************************************************************

*   FileName : tcradio_rds_if.c

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
/***************************************************
*		Include 			   					*
****************************************************/
#include "tcradio_api.h"
#include "tcradio_service.h"
#include "tcradio_rds_api.h"
#include "tcradio_rds_def.h"

/***************************************************
*        Global variable definitions               *
****************************************************/

/***************************************************
*       Imported variable declarations             *
****************************************************/

/***************************************************
*        Imported function declartions              *
****************************************************/

/***************************************************
*             Local preprocessor                   *
****************************************************/

/***************************************************
*           Local constant definitions              *
****************************************************/

/***************************************************
*           Local type definitions                 *
****************************************************/

/***************************************************
*          Local function prototypes               *
****************************************************/

/***************************************************
*			function definition				*
****************************************************/
int32 tcrds_getEnable(void)
{
	return stRds.fEnable;
}

uint16 tcrds_getPi(void)
{
	return ((uint16)stRds.pih << 8 | (uint16)stRds.pil);
}

uint8 tcrds_getTaTp (void)
{
  uint8 tatpStatus = stRds.tatpStatus;

  clrBit(stRds.tatpStatus, RDS_TATP_NEW);	/* Info becomes unavailable by app.*/
  return(tatpStatus & 0x15);				/* Return TA, TP & availability.*/
}

uint8 tcrds_getPty (void)
{
  if ((stRds.ptyStatus & RDS_PTY_VALID) == 0)	/* If the PTY is not available.*/
    return(NO_PTY);								/* => return no PTY (0x00).*/

  return((stRds.ptyStatus & 0x1F));				/* Return PTY code value.*/
}

uint8 tcrds_getMsValid(void)
{													/* MS available only when */
  return(valBit(stRds.msStatus,RDS_MS_CHANGE));		/* there is a MS value change.*/
}

uint8 tcrds_getMs(void)
{
  clrBit(stRds.msStatus,RDS_MS_CHANGE);    			/* MS becomes unavailable by appli.*/
  return(valBit(stRds.msStatus,RDS_MS_VALUE));		/* Return MS flag value.*/
}

uint8 tcrds_getPsNew(void)
{
	return(valBit(stRds.psStatus,RDS_PS_NEW));
}

uint8 tcrds_getPsValid(void)
{
	return(valBit(stRds.psStatus,RDS_PS_VALID));
}

uint8 tcrds_getPs(uint8 tcrds_psidx)
{
	return(stRds.psname[tcrds_psidx]);
}

void tcrds_setResetPsNew(void)
{
	clrBit(stRds.psStatus,RDS_PS_NEW);
}

