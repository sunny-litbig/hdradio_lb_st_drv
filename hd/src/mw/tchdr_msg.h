/*******************************************************************************

*   FileName : tchdr_msg.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio framework message queue and mutex header

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
#ifndef TCHDR_MSGQ_H__
#define TCHDR_MSGQ_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define	TCHDR_MSGQ_SIZE			(32U)
#define	TCHDR_MSGQ_DATA_LENGTH	(32U)
#define	TCHDR_MSGQ_PDATA_LENGTH	(16U)

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum{
    eTCHDR_NEW_MSG_NULL 		= 0,
    eTCHDR_NEW_MSG_EXIST 		= 1
}eTCHDR_NEW_MSG_EXIST_t;

typedef enum{
	eTCHDR_SENDER_ID_APP		= 1,
	eTCHDR_SENDER_ID_SERVICE	= 2,
	eTCHDR_SENDER_ID_AUDIO		= 3,
	eTCHDR_SENDER_ID_AUDIN		= 4,
	eTCHDR_SENDER_ID_IQIN		= 5,
	eTCHDR_SENDER_ID_BBINPUT	= 6,
	eTCHDR_SENDER_ID_HDR_MAIN	= 7,
	eTCHDR_SENDER_ID_HDR_MRC	= 8,
	eTCHDR_SENDER_ID_HDR_BS		= 9,
	eTCHDR_SENDER_ID_HDR_BSMRC	= 10,
	eTCHDR_SENDER_ID_HDR_BLENDING = 11,
	eTCHDR_SENDER_ID_MAX
}eTCHDR_SENDER_ID_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct {
	U32 uiSender;
	U32 fNewMsg;
  	U32 uiMode;
   	U32 uiData[TCHDR_MSGQ_DATA_LENGTH];
	void *pData[TCHDR_MSGQ_PDATA_LENGTH];
	HDRET iError;
}stTcHdrMsgBuf_t;

typedef struct {
	U32 wp;					// message write pointer
	U32 rp;					// message read pointer
	stTcHdrMsgBuf_t msg[TCHDR_MSGQ_SIZE];	// message packet buffer
}stTcHdrMsgQueue_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern HDRET tchdrapp_messageInit(void);
extern HDRET tchdrapp_messageDeinit(void);
extern HDRET tchdrapp_mutexLock(void);
extern void tchdrapp_mutexUnlock(void);
extern HDRET tchdrapp_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError);
extern HDRET tchdrapp_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf);

extern HDRET tchdrsvc_messageInit(void);
extern HDRET tchdrsvc_messageDeinit(void);
extern HDRET tchdrsvc_mutexLock(void);
extern void tchdrsvc_mutexUnlock(void);
extern HDRET tchdrsvc_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError);
extern HDRET tchdrsvc_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf);

extern HDRET tchdrbbinput_messageInit(void);
extern HDRET tchdrbbinput_messageDeinit(void);
extern HDRET tchdrbbinput_mutexLock(void);
extern void tchdrbbinput_mutexUnlock(void);
extern HDRET tchdrbbinput_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError);
extern HDRET tchdrbbinput_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf);

extern HDRET tchdriqinput_messageInit(void);
extern HDRET tchdriqinput_messageDeinit(void);
extern HDRET tchdriqinput_mutexLock(void);
extern void tchdriqinput_mutexUnlock(void);

extern HDRET tchdraudinput_messageInit(void);
extern HDRET tchdraudinput_messageDeinit(void);
extern HDRET tchdraudinput_mutexLock(void);
extern void tchdraudinput_mutexUnlock(void);

extern HDRET tchdrmain_messageInit(void);
extern HDRET tchdrmain_messageDeinit(void);
extern HDRET tchdrmain_mutexLock(void);
extern void tchdrmain_mutexUnlock(void);
extern HDRET tchdrmain_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError);
extern HDRET tchdrmain_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf);

extern HDRET tchdrmrc_messageInit(void);
extern HDRET tchdrmrc_messageDeinit(void);
extern HDRET tchdrmrc_mutexLock(void);
extern void tchdrmrc_mutexUnlock(void);
extern HDRET tchdrmrc_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError);
extern HDRET tchdrmrc_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf);

extern HDRET tchdrbs_messageInit(void);
extern HDRET tchdrbs_messageDeinit(void);
extern HDRET tchdrbs_mutexLock(void);
extern void tchdrbs_mutexUnlock(void);
extern HDRET tchdrbs_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError);
extern HDRET tchdrbs_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf);

extern HDRET tchdrbsmrc_messageInit(void);
extern HDRET tchdrbsmrc_messageDeinit(void);
extern HDRET tchdrbsmrc_mutexLock(void);
extern void tchdrbsmrc_mutexUnlock(void);
extern HDRET tchdrbsmrc_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError);
extern HDRET tchdrbsmrc_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf);

extern HDRET tchdraudoutput_messageInit(void);
extern HDRET tchdraudoutput_messageDeinit(void);
extern HDRET tchdraudoutput_mutexLock(void);
extern void tchdraudoutput_mutexUnlock(void);
extern HDRET tchdraudoutput_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError);
extern HDRET tchdraudoutput_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf);

extern HDRET tchdrblending_messageInit(void);
extern HDRET tchdrblending_messageDeinit(void);
extern HDRET tchdrblending_mutexLock(void);
extern void tchdrblending_mutexUnlock(void);
extern HDRET tchdrblending_sendMessage(eTCHDR_SENDER_ID_t eSenderID, U32 uiMode, const U32 *uiData, const void *pData, HDRET iError);
extern HDRET tchdrblending_getMessage(stTcHdrMsgBuf_t *pstRsvMsgBuf);

#ifdef __cplusplus
}
#endif

#endif
