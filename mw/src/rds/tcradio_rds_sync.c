/*******************************************************************************

*   FileName : tcradio_rds_sync.c

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
#include "tcradio_hal.h"
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
extern void tcrds_extractBlocks(uint8 block, uint8 block_h, uint8 block_l);

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
void tcrds_fetchRdsDataHandler(void)
{
	uint8 tempbuf[20]={0,}, blerA, blerB, blerC, blerD;
	uint32 i;

	if(stRds.fEnable) {
		if(++stRds.rdsFetchCounter >= (80 / RDS_THREAD_TIME_INTERVAL)) {	// 80ms		// 1 Group(A+B+C+D) = 104bits = about 87.6ms
			tcradiohal_getRdsData(tempbuf, 0);
			if(tempbuf[6] > 0) {
				blerA = (tempbuf[7]>>6)&0x03;
				blerB = (tempbuf[7]>>4)&0x03;
				blerC = (tempbuf[7]>>2)&0x03;
				blerD = tempbuf[7]&0x03;
			#if 0
				RDS_DBG("===============================================================================\n");
				RDS_DBG("[RDS] Data In : ");
				for(i=0; i<16; i++) {
					printf("%02xh ", tempbuf[i]);
					if(i == 15) {
						printf("\n");
					}
				}
				RDS_DBG("TPPTYINT[%d], PIINT[%d], SYNCINT[%d], FIFOINT[%d]\n", (tempbuf[0]&0x10) ? 1:0, (tempbuf[0]&0x08) ? 1:0, (tempbuf[0]&0x02) ? 1:0, (tempbuf[0]&0x01) ? 1:0);
				RDS_DBG("TPPTYVALID[%d], PIVALID[%d], SYNC[%d], FIFOLOST[%d]\n", (tempbuf[1]&0x10) ? 1:0, (tempbuf[1]&0x08) ? 1:0, (tempbuf[1]&0x02) ? 1:0, (tempbuf[1]&0x01) ? 1:0);
				RDS_DBG("TP[%d], PTY[%02xh]\n", (tempbuf[2]&0x20) ? 1:0, tempbuf[2]&0x1f);
				RDS_DBG("PI[%02x%02xh], RDSFIFOUSED[%d], BLERA[%d], BLERB[%d], BLERC[%d], BLERD[%d]\n",
					     tempbuf[5], tempbuf[4], tempbuf[6], (tempbuf[7]>>6)&0x02, (tempbuf[7]>>4)&0x02, (tempbuf[7]>>2)&0x02, tempbuf[7]&0x02 );
				RDS_DBG("BlockA[%02x%02xh], BlockB[%02x%02xh], BlockC[%02x%02xh], BlockD[%02x%02xh]\n",
					          tempbuf[9], tempbuf[8], tempbuf[11], tempbuf[10], tempbuf[13], tempbuf[12], tempbuf[15], tempbuf[14]);
				RDS_DBG("===============================================================================\n");
			#endif
				/*=====================================================================================================*/
				/*                             Silab Tuner RDS Group Buffer Structure                                  */
				/*     [8]         [9]          [10]         [11]         [12]        [13]        [14]        [15]     */
				/* BlockA[7:0] BlockA[15:8] BlockB[7:0] BlockB[15:8] BlockC[7:0] BlockC[15:8] BlockD[7:0] BlockD[15:8] */
				/*=====================================================================================================*/
				if(blerA < 3) {
					tcrds_extractBlocks(eRDS_BLOCK_A, tempbuf[9], tempbuf[8]);
					if(blerB < 3) {
						tcrds_extractBlocks(eRDS_BLOCK_B, tempbuf[11], tempbuf[10]);
						if(blerC < 3) {
							if(stRds.group & 0x08) {
								tcrds_extractBlocks(eRDS_BLOCK_c, tempbuf[13], tempbuf[12]);
							}
							else {
								tcrds_extractBlocks(eRDS_BLOCK_C, tempbuf[13], tempbuf[12]);
							}
						}
						if(blerD < 3) {
							tcrds_extractBlocks(eRDS_BLOCK_D, tempbuf[15], tempbuf[14]);
						}
					}
				}
			#if 0
				RDS_DBG("PI=%04xh, PTY=%02x, PSNAME=", tcrds_getPi(), tcrds_getPty());
				for(i=0; i<MAX_PS; i++)
					printf("%c", tcrds_getPs(i));
				printf("\n");

			#endif
			}

			if(tempbuf[6] > 1) {
				stRds.rdsFetchCounter = 16;
			}
			else {
				stRds.rdsFetchCounter = 0;
			}
		}
	}
}

