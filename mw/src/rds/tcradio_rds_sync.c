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

#if 1   // LB RDS code for ST tuner
typedef struct
{
    uint8 err_flag;
    uint8 val_L;
    uint8 val_H;
} rds_block_type_t;

typedef struct
{
    rds_block_type_t blockA;
    rds_block_type_t blockB;
    rds_block_type_t blockC;
    rds_block_type_t blockD;
} rds_message_type_t;

typedef struct
{
    int32 num_msg;
    rds_message_type_t message[4];
} lb_rds_data_t;

#ifdef RDS_DBG_MSG
#define NUM_RDS_DBG_MSG 4
static rds_message_type_t dbg_rds_msg[NUM_RDS_DBG_MSG];;
uint32 dbg_rds_msg_print = 0;
#endif

static lb_rds_data_t rcv_rds_data;

static RET LBcreateRDSMessage(uint8 *buff, int32 NumValidBlock)
{
    RET ret = eRET_OK;
    uint32 temp_cnt, msg_cnt_temp;
    int32 block_id_temp, prev_block_id;

    if (buff == NULL)
        return eRET_NG_INVALID_PARAM;

    if (NumValidBlock == 0)
        return eRET_NG_INVALID_PARAM;

#ifdef RDS_DBG_MSG
    uint32 aaa = 0;

    if (rcv_rds_data.num_msg > 0)
    {
        for (temp_cnt = 0; temp_cnt < rcv_rds_data.num_msg; temp_cnt++)
        {
            for (aaa = 0; aaa < (NUM_RDS_DBG_MSG - 1); aaa ++)
            {
                memcpy(&dbg_rds_msg[aaa], &dbg_rds_msg[aaa + 1], sizeof(rds_message_type_t));
            }

            memcpy(&dbg_rds_msg[NUM_RDS_DBG_MSG - 1], &rcv_rds_data.message[temp_cnt], sizeof(rds_message_type_t));
        }
    }
#endif

    memset(&rcv_rds_data, 0, sizeof(lb_rds_data_t));

    for (temp_cnt = 0; temp_cnt < 4; temp_cnt++)
    {
        rcv_rds_data.message[temp_cnt].blockA.err_flag = 1;
        rcv_rds_data.message[temp_cnt].blockB.err_flag = 1;
        rcv_rds_data.message[temp_cnt].blockC.err_flag = 1;
        rcv_rds_data.message[temp_cnt].blockD.err_flag = 1;
    }

    temp_cnt = 0;
    msg_cnt_temp = 0;
    prev_block_id = -1;

    while (temp_cnt < NumValidBlock)
    {
        block_id_temp = buff[temp_cnt * 3] & 0x03;   // get block id

        if (block_id_temp <= prev_block_id)
        {
            msg_cnt_temp++;
        }

        if (block_id_temp == 0)
        {
             rcv_rds_data.message[msg_cnt_temp].blockA.err_flag = 0;
             rcv_rds_data.message[msg_cnt_temp].blockA.val_H = buff[(temp_cnt * 3) + 1];
             rcv_rds_data.message[msg_cnt_temp].blockA.val_L = buff[(temp_cnt * 3) + 2];
        }

        if (block_id_temp == 1)
        {
             rcv_rds_data.message[msg_cnt_temp].blockB.err_flag = 0;
             rcv_rds_data.message[msg_cnt_temp].blockB.val_H = buff[(temp_cnt * 3) + 1];
             rcv_rds_data.message[msg_cnt_temp].blockB.val_L = buff[(temp_cnt * 3) + 2];
        }

        if (block_id_temp == 2)
        {
             rcv_rds_data.message[msg_cnt_temp].blockC.err_flag = 0;
             rcv_rds_data.message[msg_cnt_temp].blockC.val_H = buff[(temp_cnt * 3) + 1];
             rcv_rds_data.message[msg_cnt_temp].blockC.val_L = buff[(temp_cnt * 3) + 2];
        }

        if (block_id_temp == 3)
        {
             rcv_rds_data.message[msg_cnt_temp].blockD.err_flag = 0;
             rcv_rds_data.message[msg_cnt_temp].blockD.val_H = buff[(temp_cnt * 3) + 1];
             rcv_rds_data.message[msg_cnt_temp].blockD.val_L = buff[(temp_cnt * 3) + 2];
        }

        prev_block_id = block_id_temp;
        temp_cnt++;
    }

    if (msg_cnt_temp < 2)
    {
        rcv_rds_data.num_msg = msg_cnt_temp + 1;
    }
    else
    {
        RDS_ERR("[%s] Invalid Received RDS Message : msg_cnt_temp = %d, NumValidBlock = %d\n", __func__, msg_cnt_temp, NumValidBlock);

        for (int temp_cnt = 0; temp_cnt < NumValidBlock; temp_cnt ++)
        {
            RDS_ERR("temp_cnt = %d, buffer header = %02x, BLOCKID = %x, DATA_H = %02x, DATA_L = %02x .\n",
                    temp_cnt, buff[temp_cnt * 3], (buff[temp_cnt * 3] & 0x03),
                    buff[(temp_cnt * 3) + 1], buff[(temp_cnt * 3) + 2]);
        }
        ret = eRET_NG_INVALID_PARAM;
    }

    return ret;
}

RET LBparsingRDSMessage(void)
{
    RET ret = eRET_OK;
    uint32 temp_cnt = 0;
    rds_message_type_t *p_msg;

    if (rcv_rds_data.num_msg <= 0)
        return eRET_NG_INVALID_PARAM;

    temp_cnt = 0;

    while (temp_cnt < rcv_rds_data.num_msg)
    {
        p_msg = &rcv_rds_data.message[temp_cnt];

        if (p_msg->blockA.err_flag == 0)
        {
            tcrds_extractBlocks(eRDS_BLOCK_A, p_msg->blockA.val_H, p_msg->blockA.val_L);
        }

        if (p_msg->blockB.err_flag == 0)
        {
            tcrds_extractBlocks(eRDS_BLOCK_B, p_msg->blockB.val_H, p_msg->blockB.val_L);

            if (p_msg->blockC.err_flag == 0)
            {
                if(stRds.group & 0x08)
                {
                    // Block C is PI code in Type B message.
                    tcrds_extractBlocks(eRDS_BLOCK_c, p_msg->blockC.val_H, p_msg->blockC.val_L);
                }
            }

            if (p_msg->blockD.err_flag == 0)
            {
                tcrds_extractBlocks(eRDS_BLOCK_D, p_msg->blockD.val_H, p_msg->blockD.val_L);
            }
        }

        temp_cnt ++;
    }

    return ret;
}

#ifdef RDS_DBG_MSG
void printf_dbg_rds_msg(uint8 *buff, int32 NumValidBlock)
{
    uint32 aaa = 0;
    rds_message_type_t *p_dbg_msg;

    RDS_ERR("Previous received (%d) DBG_SAVE_RDS_MSG.\n", NUM_RDS_DBG_MSG);
    for (aaa = 0; aaa < NUM_RDS_DBG_MSG; aaa ++)
    {
        p_dbg_msg = &dbg_rds_msg[aaa];

        RDS_ERR("DBG_SAVE_RDS_MSG[%d]\n", aaa);
        RDS_ERR("BLOCK_A : err = %d, val_H = 0x%02x, val_L = 0x%02x\n",
            p_dbg_msg->blockA.err_flag, p_dbg_msg->blockA.val_H, p_dbg_msg->blockA.val_L);
        RDS_ERR("BLOCK_B : err = %d, val_H = 0x%02x, val_L = 0x%02x\n",
            p_dbg_msg->blockB.err_flag, p_dbg_msg->blockB.val_H, p_dbg_msg->blockB.val_L);
        RDS_ERR("BLOCK_C : err = %d, val_H = 0x%02x, val_L = 0x%02x\n",
            p_dbg_msg->blockC.err_flag, p_dbg_msg->blockC.val_H, p_dbg_msg->blockC.val_L);
        RDS_ERR("BLOCK_D : err = %d, val_H = 0x%02x, val_L = 0x%02x\n",
            p_dbg_msg->blockD.err_flag, p_dbg_msg->blockD.val_H, p_dbg_msg->blockD.val_L);
    }

    uint32 temp_cnt = 0;
    rds_message_type_t *p_msg;
    RDS_ERR("[%s] NumValidBlock = %d\n", __func__, NumValidBlock);
    for (int temp_cnt = 0; temp_cnt < NumValidBlock; temp_cnt ++)
    {
        RDS_ERR("temp_cnt = %d, buffer header = %02x, BLOCKID = %x, DATA_H = %02x, DATA_L = %02x .\n",
                temp_cnt, buff[temp_cnt * 3], (buff[temp_cnt * 3] & 0x03),
                buff[(temp_cnt * 3) + 1], buff[(temp_cnt * 3) + 2]);
    }

    RDS_ERR("[%s] Number of Received RDS Message = %d\n", __func__, rcv_rds_data.num_msg);
    for (temp_cnt = 0; temp_cnt < rcv_rds_data.num_msg; temp_cnt++)
    {
        p_msg = &rcv_rds_data.message[temp_cnt];

        RDS_ERR("RDS MSG[%d], BLOCK_A : err = %d, val_H = 0x%02x, val_L = 0x%02x\n", temp_cnt,
            p_msg->blockA.err_flag, p_msg->blockA.val_H, p_msg->blockA.val_L);
        RDS_ERR("RDS MSG[%d], BLOCK_B : err = %d, val_H = 0x%02x, val_L = 0x%02x\n", temp_cnt,
            p_msg->blockB.err_flag, p_msg->blockB.val_H, p_msg->blockB.val_L);
        RDS_ERR("RDS MSG[%d], BLOCK_C : err = %d, val_H = 0x%02x, val_L = 0x%02x\n", temp_cnt,
            p_msg->blockC.err_flag, p_msg->blockC.val_H, p_msg->blockC.val_L);
        RDS_ERR("RDS MSG[%d], BLOCK_D : err = %d, val_H = 0x%02x, val_L = 0x%02x\n", temp_cnt,
            p_msg->blockD.err_flag, p_msg->blockD.val_H, p_msg->blockD.val_L);
    }
}
#endif

void tcrds_fetchRdsDataHandler(void)
{
	uint8 tempbuf[16 * 3] = {0,};
	uint32 NumValidBlock = 0;
    RET ret = eRET_OK;

	if (stRds.fEnable) {
		if (++stRds.rdsFetchCounter >= (80 / RDS_THREAD_TIME_INTERVAL)) {	// 80ms		// 1 Group(A+B+C+D) = 104bits = about 87.6ms
			tcradiohal_getRdsData(tempbuf, 0, &NumValidBlock);

            if (NumValidBlock > 0)
            {
                ret = LBcreateRDSMessage(tempbuf, NumValidBlock);

                if (ret == eRET_OK)
                {
                    // parsing routine call
                    ret = LBparsingRDSMessage();
                }
            }
            else
            {
#ifdef RDS_DBG_MSG
                RDS_DBG("Can't receive RDS Message. (%d).", NumValidBlock);
#endif
            }

#ifdef RDS_DBG_MSG
            if (dbg_rds_msg_print  == 1)
            {
                printf_dbg_rds_msg(tempbuf, NumValidBlock);
                dbg_rds_msg_print = 0;
            }
#endif
            stRds.rdsFetchCounter = 0;
		}
	}
}
#else
void tcrds_fetchRdsDataHandler(void)
{
	uint8 tempbuf[20]={0,}, blerA, blerB, blerC, blerD;
	uint32 i;

	if(stRds.fEnable) {
		if(++stRds.rdsFetchCounter >= (80 / RDS_THREAD_TIME_INTERVAL)) {	// 80ms		// 1 Group(A+B+C+D) = 104bits = about 87.6ms
			tcradiohal_getRdsData(tempbuf, 0, NULL);
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
#endif
