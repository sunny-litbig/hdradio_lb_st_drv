/*******************************************************************************

*   FileName : tcradio_rds_parser.c

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
#define GROUP_0X	((stRds.group & 0xF0) == GRP_0A)
#define GROUP_0A	((stRds.group & 0xF8) == GRP_0A)
#define GROUP_0B	((stRds.group & 0xF8) == GRP_0B)
#define GROUP_2X	((stRds.group & 0xF0) == GRP_2A)
#define GROUP_2A	((stRds.group & 0xF8) == GRP_2A)
#define GROUP_2B	((stRds.group & 0xF8) == GRP_2B)
#define GROUP_4A	((stRds.group & 0xF8) == GRP_4A)
#define GROUP_14X	((stRds.group & 0xF0) == GRP_14A)
#define GROUP_14A	((stRds.group & 0xF8) == GRP_14A)
#define GROUP_14B	((stRds.group & 0xF8) == GRP_14B)
#define GROUP_15B	((stRds.group & 0xF8) == GRP_15B)

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
void tcrds_clearData(void)
{
	unsigned short i;

	/* RDS EXTRACTION CONTROL MANAGEMENT.......................................*/
	stRds.group       = 0;      /* Current extracted group code reset.*/
	stRds.extStatus   = 0;		/* RDS Data extraction general status register cleared.*/


	/* STORED RDS DATA INFORMATION INITIALIZATION..............................*/
	stRds.piCounter  = 0;		/* RDS PI code information availability cnt reset.*/
  								/* Force the RDS PI code to be not available.*/
	stRds.tatpStatus = 0;		/* RDS TA and TP info status/control reg. cleared.*/
	stRds.ptyStatus  = 0;		/* RDS PTY information status/control reg.  reset.*/

	if (stRds.psname[0] == 0xFF)
		stRds.psStatus = 0;		/* RDS PS information software control reg. reset.*/
	else {
		setBit(stRds.psStatus,RDS_PS_VALID);	/* Force PS to be available.*/
		setBit(stRds.psStatus,RDS_PS_NEW);		/* New PS available for appli.*/
	}

	for (i = 0; i < 8; i++)
		stRds.psbuf[i] = 0xFF;
}

void tcrds_extractTp(uint8 block_h, uint8 block_l)
{
	if (valBit(block_h, RDS_TP_VALUE) == valBit(stRds.tatpStatus, RDS_TP_VALUE)) {
		setBit(stRds.tatpStatus, RDS_TP_VALID);
	}
	else if (valBit(stRds.tatpStatus, RDS_TP_VALID)) {
		clrBit(stRds.tatpStatus, RDS_TP_VALID);
	}
    else {
		if(valBit(block_h, RDS_TP_VALUE)) {
			setBit(stRds.tatpStatus, RDS_TP_VALUE);
		}
		else {
			clrBit(stRds.tatpStatus, RDS_TP_VALUE);
		}
		setBit(stRds.tatpStatus, RDS_TP_VALID);
		setBit(stRds.tatpStatus, RDS_TATP_NEW);
	}
}

void tcrds_extractTa(uint8 block_h, uint8 block_l)
{
	if (valBit(block_l, RDS_TA_VALUE) == valBit(stRds.tatpStatus, RDS_TA_VALUE)) {
		setBit(stRds.tatpStatus, RDS_TA_VALID);
	}
	else if (valBit(stRds.tatpStatus, RDS_TA_VALID)) {
		clrBit(stRds.tatpStatus, RDS_TA_VALID);
	}
	else {
		if(valBit(block_l, RDS_TA_VALUE)) {
			setBit(stRds.tatpStatus, RDS_TA_VALUE);
		}
		else {
			clrBit(stRds.tatpStatus, RDS_TA_VALUE);
		}
		setBit(stRds.tatpStatus,RDS_TA_VALID);
		setBit(stRds.tatpStatus,RDS_TATP_NEW);
	}
}

void tcrds_extractPty(uint8 block_h, uint8 block_l)
{
	uint8 tcrds_ptycurval;                    /* RDS current PTY code information.*/

	tcrds_ptycurval = (block_l >> 5) | ((block_h << 3) & 0x18);

	if (tcrds_ptycurval == (stRds.ptyStatus & 0x1F)) {
		/* Stored & Current PTY are the same */
		stRds.ptyStatus |= RDS_PTY_VALID;
		stRds.pty = tcrds_ptycurval;
	}
	else {
		/* Stored & Current PTY are different.*/
		if (stRds.ptyStatus & RDS_PTY_VALID) {
			/* The previous PTY was valid.*/
			stRds.ptyStatus &= ~RDS_PTY_VALID;
		}
		/* If two continuous input PTYs are different from those stored, a new PTY code is stored */
		else {
			/* The previous PTY was not valid => current PTY and previous one */
			/* are different but the current one is stored and set valid.*/
			stRds.ptyStatus = tcrds_ptycurval;			/* New PTY value stored.*/
			stRds.ptyStatus &= ~RDS_PTY_VALID;			/* New stored data becomes valid.*/
		}
	}

	/* [Required Code] : Add code to reduce PTY check time in PTY search. */
}

void tcrds_extractPi(uint8 block_h, uint8 block_l)
{
	if ((block_h == NO_PI) || (block_l == NO_PI))
		return;

	/* The new PI is the same that the previous stored one.*/
	if ((stRds.pih == block_h) && (stRds.pil == block_l)) {
		if(stRds.piCounter < 10)
			stRds.piCounter++;
	}
	else {
		if (stRds.piCounter == 0) {
			stRds.pih = block_h;
			stRds.pil = block_l;
			stRds.piCounter++;
		}
		else {
			stRds.piCounter--;
		}
	}
}

void tcrds_extractPs(uint8 block_h, uint8 block_l)
{
	uint8 tcrds_psnflg, tcrds_psnseg, i;

	/* Add 4 to D1 and D0 to calculate D7 to D4 bit position. */
	tcrds_psnflg = (stRds.psStatus & 0x03) + 4;			/* Valid PS segment flag position.*/

	/* Because PS name is received by 2 bytes, segment addr * 2 value is PS name index. */
	tcrds_psnseg = (stRds.psStatus & 0x03) << 1;			/* PS index calculation.*/

	if ( (stRds.psbuf[tcrds_psnseg]   != block_h)		/* Different new and */
		|| (stRds.psbuf[tcrds_psnseg+1] != block_l) )	/* previous char.*/
	{
		if (valBit(stRds.psStatus, tcrds_psnflg))			/* PS segment Already received.*/
			stRds.psStatus &= 0x0F;						/* This is the 1st reception for a new PS.*/
														/* The active PS becomes not available.*/
		stRds.psbuf[tcrds_psnseg]   = block_h;			/* Extract character 1.*/
		stRds.psbuf[tcrds_psnseg+1] = block_l;			/* Extract character 2.*/
		setBit(stRds.psStatus, tcrds_psnflg);				/* 2 1st new available PSN char.*/
	}
	else												/* New and previous characters are equal.*/
		if (stRds.psStatus & 0xF0)						/* One of the PS segment is already available.*/
			setBit(stRds.psStatus, tcrds_psnflg);			/* 2 1st new available PSN char.*/

	clrBit(stRds.extStatus, RDS_PS_SEG_OK);				/* PS segment not more available.*/

	/* Storing of the active PS in the store one...............................*/
	if ((stRds.psStatus & 0xF0) == 0xF0)       /* All the PS char are available */
	{ /* Copy active PS buffer in PS Store buffer (seen by the application).   */
		stRds.psStatus &= 0x0F;        /* Active PS segments become not available.*/

		for (i=0 ; i<MAX_PS ; i++) {
			if ((stRds.psbuf[i] == 0xFF)	|| (stRds.psbuf[i] < 0x0F))	{		/* non-printable character */
				return;
			}
		}

		for (i=0 ; i<MAX_PS ; i++) {
			stRds.psname[i] = stRds.psbuf[i];
		}

		setBit(stRds.psStatus, RDS_PS_VALID);		/* PS available for appli.*/
		setBit(stRds.psStatus, RDS_PS_NEW);			/* New PS available for appli.*/
	}
}

void tcrds_extractMs(uint8 block_h, uint8 block_l)
{
	/* Stored & Current MS are the same.*/
	if (valBit(block_l, RDS_MS_VALUE) == valBit(stRds.msStatus, RDS_MS_VALUE)) {
		setBit(stRds.msStatus, RDS_MS_CTRL);     /* 2 consecutive same MS =>DataOK.*/
	}

	/* Stored & Current MS are different, The previous MS was valid.*/
	else if (valBit(stRds.msStatus, RDS_MS_CTRL)) {
		clrBit(stRds.msStatus, RDS_MS_CTRL);        /* The data become not valid.*/
	}

	/*	The previous MS was not valid => current MS and previous one
		are different but the current one is stored and set valid.*/
    else {
		if(valBit(block_l, RDS_MS_VALUE)) {
			setBit(stRds.msStatus, RDS_MS_VALUE);
		}
		else {
			clrBit(stRds.msStatus, RDS_MS_VALUE);
		}
		setBit(stRds.msStatus, RDS_MS_CTRL);    /* New stored data becomes valid.*/
		setBit(stRds.msStatus, RDS_MS_CHANGE);     /* There is a MS value change.*/
	}
}

void tcrds_extractAf(uint8 block_h, uint8 block_l)
{
}

void tcrds_extractBlockB(uint8 block_h, uint8 block_l)
{
	/* Extract Group Type Code */
	stRds.group = block_h & 0xF8;      /* block B upper : g3 g2 g1 g0 AB X  X  X */

	/* Common in all group */
	tcrds_extractTp(block_h, block_l);	/* Extract RDS TP information flag.*/
	tcrds_extractPty(block_h, block_l);	/* Extract RDS PTY information code.*/


	/*:::::::::: B Block Information From 0(A/B) or 15B :::::::::*/
	if (GROUP_0X || GROUP_15B) {
		tcrds_extractTa(block_h, block_l);	/* Extract RDS TA information flag.*/
		tcrds_extractMs(block_h, block_l);	/* Extract RDS MS information flag.*/
	}

	/* Extract Program Name Segment */
	if (GROUP_0X) {
		mskBit(stRds.psStatus, 0x03, block_l);  /* SR2 : X  X  X  X  X  X  c1 c0 */
		setBit(stRds.extStatus, RDS_PS_SEG_OK);            /* PS segment available.*/
	}

#if 0	// if use, open and make functions.
	/*:::::::::::::::: B BLOCK INFORMATION FROM A RT GROUP 2(A/B) :::::::::::::*/
    /* Extract current received text segment address.........................*/
	if (GROUP_2A) {
		tcrds_extractRtASegment(block_l);          /* SR2 : X  X  X  X  s3 s2 s1 s0 */
		setBit(stRds.extStatus, RDS_RT_SEG_OK);            /* RT segment available.*/
	}

	/*:::::::::::::::: B BLOCK INFORMATION FROM A CT GROUP 4A :::::::::::::::::*/
	if (GROUP_4A) {
		/* Extract current received CT MJD bit16 & bit15.........................*/
		tcrds_extractCt(block_l);        /* SR2 : X  X  X  X  X  X mjd16 mjd15 */
		setBit(stRds.extStatus,RDS_CTB_SEG_OK);              /* CT BlkB available.*/
	}

	/*:::::::::::::::: B BLOCK INFORMATION FROM A EON GROUP 14(A/B) :::::::::::*/
	if (GROUP_14A) {
		/* Extract current received variante code................................*/
		tcrds_extractEonVariante(block_l);         /* SR2 : X  X  X  X  v3 v2 v1 v0 */
		setBit(stRds.extStatus,RDS_EONB_SEG_OK);         /* EON segment available.*/
	}

	if (GROUP_14B) {
		/* Extract current received EON TA state.................................*/
		tcrds_extractEonTa(block_l);               /* SR2 : X  X  X  tp  ta X  X  X */
		setBit(stRds.extStatus,RDS_EONB_SEG_OK);         /* EON TA info available.*/
	}
#endif
}

void tcrds_extractBlockC (uint8 block_h, uint8 block_l)
{
	/*:::::::::::::::: C BLOCK INFORMATION FROM A GROUP 0A ::::::::::::::::::::*/
	if (GROUP_0A) {
		/* Extract Alternative Frequency from a 0A block.........................*/
		tcrds_extractAf(block_h, block_l);
	}

#if 0	// if use, open and make functions.
	/*:::::::::::::::: C BLOCK INFORMATION FROM A RT GROUP 2A :::::::::::::::::*/
	if ( GROUP_2A && valBit(stRds.extStatus, RDS_RT_SEG_OK) ) {
		/* Extract current received text character +0 and +1.....................*/
		tcrds_extractRtAData1(block_h, block_l);		/* (char+0)=SR3 / (char+1)=SR2 */
	}

	/*:::::::::::::::: C BLOCK INFORMATION FROM A CT GROUP 4A :::::::::::::::::*/
	if ( GROUP_4A && valBit(stRds.extStatus,RDS_CTB_SEG_OK) ) {
		/* Extract current received CT MJD bit 14..0 and Hour bit 4..............*/
		tcrds_extractCtData2(block_h, block_l);		/* MJD[14..0] and Hour[4] */
		clrBit(stRds.extStatus,RDS_CTB_SEG_OK);		/* CT Blk B not more available.*/
		setBit(stRds.extStatus,RDS_CTC_SEG_OK);		/* CT Blk C available.*/
	}

	/*:::::::::::::::: C BLOCK INFORMATION FROM A EON GROUP 14A :::::::::::::::*/
	if (GROUP_14A && valBit(stRds.extStatus,RDS_EONB_SEG_OK)) {
		/* Extract current received EON information..............................*/
		tcrds_extractEonInfo(block_h, block_l);
		clrBit(stRds.extStatus,RDS_EONB_SEG_OK);	/* Blk B segment not available.*/
		setBit(stRds.extStatus,RDS_EONC_SEG_OK);	/* Blk C segment available.*/
	}
#endif
}

void tcrds_extractBlockD(uint8 block_h, uint8 block_l)
{
	/*:::::::::::::::: D BLOCK INFORMATION FROM A GROUP O (A/B) :::::::::::::::*/
	if ( GROUP_0X && valBit(stRds.extStatus,RDS_PS_SEG_OK) ) {
		tcrds_extractPs(block_h,block_l);				/* Extract the active Program Station Name.*/
	}

#if 0	// if use, open and make functions.
	/*:::::::::::::::: D BLOCK INFORMATION FROM A RT GROUP 2(A/B) :::::::::::::*/
	if ( GROUP_2A && valBit(stRds.extStatus,RDS_RT_SEG_OK) ) {
		/* Extract current received text character +2 and +3.....................*/
		tcrds_extractRtData2(block_h,block_l);		/* (char+2)=SR3 / (char+3)=SR2 */
		clrBit(stRds.extStatus,RDS_RT_SEG_OK);		/* RT segment not more available.*/
	}

	/*:::::::::::::::: D BLOCK INFORMATION FROM A CT GROUP 4A :::::::::::::::::*/
	if ( GROUP_4A && valBit(stRds.extStatus,RDS_CTC_SEG_OK) ) {
		/* Extract current received CT and Hour bit 3..0, Minutes and Offset.....*/
		tcrds_extractCtData3(block_h, block_l);
		clrBit(stRds.extStatus,RDS_CTC_SEG_OK);		/* CT Blk B not more available.*/
	}

	/*:::::::::::::::: D BLOCK INFORMATION FROM A EON GROUP 14(A/B) :::::::::::*/
	if ( GROUP_14A && valBit(stRds.extStatus,RDS_EONC_SEG_OK) ) {
		/* Extract current received EON PI code..................................*/
		tcrds_extractEonPi(block_h,block_l);
		clrBit(stRds.extStatus,RDS_EONC_SEG_OK);	/* Blk C segment not available.*/
	}

	if ( GROUP_14B && valBit(stRds.extStatus,RDS_EONB_SEG_OK) ) {
		/* Extract current received EON TA state.................................*/
		tcrds_extractEonTaPi(block_h,block_l);
		clrBit(stRds.extStatus,RDS_EONB_SEG_OK);	/* EON TA info no more available.*/
	}
#endif
}

void tcrds_extractBlocks(uint8 block, uint8 block_h, uint8 block_l)
{
	switch (block) {
		case eRDS_BLOCK_A:		tcrds_extractPi(block_h, block_l);		break;
		case eRDS_BLOCK_B:		tcrds_extractBlockB(block_h, block_l);	break;
		case eRDS_BLOCK_C:		tcrds_extractBlockC(block_h, block_l);	break;
		case eRDS_BLOCK_c:		tcrds_extractPi(block_h, block_l);		break;
		case eRDS_BLOCK_D:		tcrds_extractBlockD(block_h, block_l);	break;
		default:														break;
	}
}

