/*******************************************************************************

*   FileName : tcradio_msgq.h

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
#ifndef __TCRADIO_MSGQ_H__
#define __TCRADIO_MSGQ_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define	MSGQ_SIZE			32
#define	MSGQ_DATA_LENGTH	32
#define	MSGQ_PDATA_LENGTH	16

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum{
    eNEW_MSG_NULL 		= 0,
    eNEW_MSG_EXIST 		= 1
}eNEW_MSG_EXIST_t;

typedef enum{
	eSENDER_ID_NULL 	= 0,
   	eSENDER_ID_APP		= 1,	/* RADIO APPLICATION */
   	eSENDER_ID_SERVICE	= 2,	/* RADIO SERVICE */
	eSENDER_ID_RDS		= 3,	/* RADIO DATA SYSTEM (RDS)*/
   	eSENDER_ID_SOUND	= 4,	/* RADIO SOUND */
   	eSENDER_ID_BG	    = 5,	/* RADIO Background Scan */
   	eSENDER_ID_END
}eSENDER_ID_t;

/***************************************************
*				Typedefs					*
****************************************************/
typedef struct {
	uint32 uiSender;
	uint32 fNewMsg;
  	uint32 uiMode;
   	uint32 uiData[MSGQ_DATA_LENGTH];
	void *pData[MSGQ_PDATA_LENGTH];
	int32 iError;
}stMsgBuf_t;

typedef struct {
	uint32 wp;					// message write pointer
	uint32 rp;					// message read pointer
	stMsgBuf_t msg[MSGQ_SIZE];	// message packet buffer
}stMsgQueue_t;

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
extern RET tcradioservice_messageInit(void);
extern RET tcradioservice_messageDeinit(void);
extern RET tcradiodatasystem_messageInit(void);
extern RET tcradiodatasystem_messageDeinit(void);
extern RET tcradiosound_messageInit(void);
extern RET tcradiosound_messageDeinit(void);
extern void tcradioservice_mutexLock(void);
extern void tcradioservice_mutexUnlock(void);
extern RET tcradioservice_sendMessage(eSENDER_ID_t eSenderID, uint32 uiMode, uint32 *uiData, void *pData, int32 iError);
extern RET tcradioservice_sendMessageWithoutMutex(eSENDER_ID_t eSenderID, uint32 uiMode, uint32 *uiData, void *pData, int32 iError);
extern RET tcradioservice_getMessage(stMsgBuf_t *pstRsvMsgBuf);
extern void tcradioapp_mutexLock(void);
extern void tcradioapp_mutexUnlock(void);
extern RET tcradioapp_sendMessage(eSENDER_ID_t eSenderID, uint32 uiMode, uint32 *uiData, void *pData, int32 iError);
extern RET tcradioapp_getMessage(stMsgBuf_t *pstRsvMsgBuf);
extern void tcradiodatasystem_mutexLock(void);
extern void tcradiodatasystem_mutexUnlock(void);
extern RET tcradiodatasystem_sendMessage(eSENDER_ID_t eSenderID, uint32 uiMode, uint32 *uiData, void *pData, int32 iError);
extern RET tcradiodatasystem_getMessage(stMsgBuf_t *pstRsvMsgBuf);
extern void tcradiosound_mutexLock(void);
extern void tcradiosound_mutexUnlock(void);
extern RET tcradiosound_sendMessage(eSENDER_ID_t eSenderID, uint32 uiMode, uint32 *uiData, void *pData, int32 iError);
extern RET tcradiosound_getMessage(stMsgBuf_t *pstRsvMsgBuf);

extern RET tcradiobg_messageInit(void);
extern RET tcradiobg_messageDeinit(void);
extern void tcradiobg_mutexLock(void);
extern void tcradiobg_mutexUnlock(void);
extern RET tcradiobg_sendMessage(eSENDER_ID_t eSenderID, uint32 uiMode, uint32 *uiData, void *pData, int32 iError);
extern RET tcradiobg_sendMessageWithoutMutex(eSENDER_ID_t eSenderID, uint32 uiMode, uint32 *uiData, void *pData, int32 iError);
extern RET tcradiobg_getMessage(stMsgBuf_t *pstRsvMsgBuf);
#ifdef __cplusplus
}
#endif

#endif
