/*******************************************************************************

*   FileName : tcradio_hal_utils.c

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
*		Include 			   *
****************************************************/
#include <unistd.h>

#include "tcradio_types.h"
#include "tcradio_hal_utils.h"

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
void tcradio_hal_ussleep(unsigned int cnt)
{
	usleep(cnt);
}

void tcradio_hal_mssleep(unsigned int cnt)
{
	usleep(cnt*1000);
}

void tcradio_hal_16bto8b(unsigned char *buf, unsigned short data)
{
	*(buf+0) = (unsigned char)data;
	*(buf+1) = (unsigned char)(data >> 8);
}

void tcradio_hal_32bto8b(unsigned char *buf, unsigned int data)
{
	*(buf+0) = (unsigned char)data;
	*(buf+1) = (unsigned char)(data >> 8);
	*(buf+2) = (unsigned char)(data >> 16);
	*(buf+3) = (unsigned char)(data >> 24);
}

void *tcradio_hal_memset(void *pdst, unsigned char ch, unsigned int len)
{
	unsigned char *pW;
	unsigned int i;

	pW = (unsigned char *)pdst;

	if(len == 0)
		return (void *)pW;

	for(i=0; i<len; i++)
	{
		*pW++ = ch;
	}
	return (void *)pW;
}

void *tcradio_hal_memcpy(void *pdst, void *psrc, unsigned int len)
{
	unsigned int i;
	unsigned char *pW;
	unsigned char *pR;

	pW = (unsigned char *)pdst;
	pR = (unsigned char *)psrc;

	if(len == 0)
		return (void *)pW;

	for(i = 0; i < len; i++)
	{
		*pW++ = *pR++;
	}
	return (void *)pW;
}

