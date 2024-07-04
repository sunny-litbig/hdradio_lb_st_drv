/*******************************************************************************

*   FileName : tcradio_hal_rbuf.c

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
#include "tcradio_hal_rbuf.h"

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
static void *ringbuffer_memcpy(void *pDst, void *pSrc, unsigned int size)
{
	unsigned int i;
	unsigned char *pW;
	unsigned char *pR;

	pW = (unsigned char *)pDst;
	pR = (unsigned char *)pSrc;

	if(size == 0) {
		return (void *)pW;
	}

	for(i = 0; i < size; i++)
	{
		*pW++ = *pR++;
	}

	return (void *)pW;
}

void ringbuffer_init(stBUFFER_RING_t* stDst, char* pSrc, unsigned int size)
{
	stDst->wp = 0;		// write pointer = input = push
	stDst->rp =0;		// read pointer = output = pop
	stDst->maxSize = size;
	stDst->pBuf = pSrc;
	stDst->state = eBUFFER_EMPTY;
}

int ringbuffer_getBlankSize(stBUFFER_RING_t* stDst)
{
	unsigned int lRemainSize = 0;

	if(stDst->state == eBUFFER_EMPTY)
		return 0;

	if( stDst->rp < stDst->wp )	//	Tail(read point) < Head(write point)
	{
		 lRemainSize = stDst->wp - stDst->rp;
	}
	else
	{
		lRemainSize = (stDst->maxSize - stDst->rp) +  stDst->wp;
	}

	return lRemainSize;
}

int ringbuffer_getData(stBUFFER_RING_t* stDst, char * pSrc, unsigned int RequestSize)
{
	unsigned int lRemainSize = 0;
	int uiReadSize = 0;

	if(stDst->state == eBUFFER_EMPTY)
		return 0;

	uiReadSize = RequestSize;
	if( stDst->rp < stDst->wp )	//	Tail(read point) < Head(write point)
	{
		 lRemainSize = stDst->wp - stDst->rp;
		 if(lRemainSize >= RequestSize )
		 {
			ringbuffer_memcpy(pSrc,&stDst->pBuf[stDst->rp],RequestSize);
			stDst->rp += (unsigned int)RequestSize;
		 }
		 else
		 {
			return 0;	// short available data!
		 }
	}
	else
	{
		lRemainSize = (stDst->maxSize - stDst->rp) +  stDst->wp;
		if(lRemainSize >= RequestSize )
		{
			if( (stDst->maxSize - stDst->rp) >= RequestSize)
			{
				ringbuffer_memcpy(pSrc,&stDst->pBuf[stDst->rp],RequestSize);
				stDst->rp += (unsigned int)RequestSize;
			}
			else
			{
				ringbuffer_memcpy(pSrc, &stDst->pBuf[stDst->rp], (stDst->maxSize - stDst->rp));
				RequestSize -= (stDst->maxSize - stDst->rp);
				ringbuffer_memcpy(pSrc+(stDst->maxSize - stDst->rp), &stDst->pBuf[0], RequestSize);
				stDst->rp = (unsigned int)RequestSize;
			}
		 }
		 else
		 {
			return 0;	// short available data!
		 }
	}

	if(stDst->rp  == stDst->wp)
	{
		stDst->state = eBUFFER_EMPTY;
	}
	else
	{
		stDst->state = eBUFFER_NORMAL;
	}

	return uiReadSize;
}

int ringbuffer_setData(stBUFFER_RING_t* stDst, char * pSrc, unsigned int RequestSize)
{
	unsigned  int lEmptySize;

	if(stDst->state == eBUFFER_FULL)
		return 0;

	if(RequestSize > stDst->maxSize)
		return 0;

	if( stDst->rp >  stDst->wp )	//	Tail(read point) < Head(write point)
	{
		lEmptySize =  stDst->rp - stDst->wp;
		if( lEmptySize >= RequestSize )
		{
			ringbuffer_memcpy(&stDst->pBuf[stDst->wp], pSrc, RequestSize);
			stDst->wp+=(unsigned int)RequestSize;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		lEmptySize = (stDst->maxSize - stDst->wp) + stDst->rp;

		if( lEmptySize > RequestSize || (stDst->state == eBUFFER_EMPTY))
		{
			if( (stDst->maxSize - stDst->wp) >= RequestSize)
			{
				ringbuffer_memcpy(&stDst->pBuf[stDst->wp], pSrc, RequestSize);
				stDst->wp += (unsigned int)RequestSize;
			}
			else{
				ringbuffer_memcpy(&stDst->pBuf[stDst->wp], pSrc, (stDst->maxSize - stDst->wp));
				RequestSize -= (stDst->maxSize - stDst->wp);
				ringbuffer_memcpy(&stDst->pBuf[0], pSrc+(stDst->maxSize - stDst->wp), RequestSize);
				stDst->wp = (unsigned int)RequestSize;
			}
		}
		else
		{
			return 0;
		}
	}

	if(stDst->rp  == stDst->wp)
	{
		stDst->state = eBUFFER_FULL;
	}
	else
	{
		stDst->state = eBUFFER_NORMAL;
	}

	return RequestSize;
}

int ringbuffer_getDataEx(stBUFFER_RING_t* stDst, char * pSrc, unsigned int RequestSize)
{
	unsigned int lRemainSize = 0;
	int uiReadSize = 0;

	if(stDst->state == eBUFFER_EMPTY)
		return 0;

	if( stDst->rp < stDst->wp )	//	Tail(read point) < Head(write point)
	{
		 lRemainSize = stDst->wp - stDst->rp;
		 if( lRemainSize == 0 )
		 	return 0;

		 if(lRemainSize < RequestSize )
		 {
			RequestSize = lRemainSize;
		 }
		 uiReadSize = RequestSize;
		 ringbuffer_memcpy(pSrc, &stDst->pBuf[stDst->rp], RequestSize);
		 stDst->rp += (unsigned int)RequestSize;
	}
	else
	{
		lRemainSize = (stDst->maxSize - stDst->rp) +  stDst->wp;
		if( lRemainSize == 0 )
		 	return 0;

		if(lRemainSize < RequestSize )
		{
			RequestSize = lRemainSize;
		}

		uiReadSize = RequestSize;

		if( (stDst->maxSize - stDst->rp) >= RequestSize)
		{
			ringbuffer_memcpy(pSrc, &stDst->pBuf[stDst->rp], RequestSize);
			stDst->rp += (unsigned int)RequestSize;
		}
		else
		{
			ringbuffer_memcpy(pSrc, &stDst->pBuf[stDst->rp], (stDst->maxSize - stDst->rp));
			RequestSize -= (stDst->maxSize - stDst->rp);
			ringbuffer_memcpy(pSrc+(stDst->maxSize - stDst->rp), &stDst->pBuf[0], RequestSize);
			stDst->rp = (unsigned int)RequestSize;
		}
	}

	if(stDst->rp  == stDst->wp)
	{
		stDst->state = eBUFFER_EMPTY;
	}
	else
	{
		stDst->state = eBUFFER_NORMAL;
	}

	return uiReadSize;
}
