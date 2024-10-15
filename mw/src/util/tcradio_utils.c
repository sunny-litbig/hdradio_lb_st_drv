/*******************************************************************************

*   FileName : tcradio_utils.c

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
#include "tcradio_api.h"

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
*           Local constant definitions             *
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
void tcradio_16bto8b(uint8 *buf, uint16 data)
{
	*(buf+0) = (uint8)data;
	*(buf+1) = (uint8)(data >> 8);
}

void tcradio_32bto8b(uint8 *buf, uint32 data)
{
	*(buf+0) = (uint8)data;
	*(buf+1) = (uint8)(data >> 8);
	*(buf+2) = (uint8)(data >> 16);
	*(buf+3) = (uint8)(data >> 24);
}

void *tcradio_memset(void *pdst, uint8 ch, uint32 len)
{
	uint8 *pW;
	uint32 i;

	pW = (uint8 *)pdst;

	if(len == 0) {
		return (void *)pW;
	}
	for(i=0; i<len; i++)
	{
		*pW++ = ch;
	}
	return (void *)pW;
}

void *tcradio_memcpy(void *pdst, void *psrc, uint32 len)
{
	uint32 i;
	uint8 *pW;
	uint8 *pR;

	pW = (uint8 *)pdst;
	pR = (uint8 *)psrc;

	if(len == 0) {
		return (void *)pW;
	}
	for(i = 0; i < len; i++)
	{
		*pW++ = *pR++;
	}
	return (void *)pW;
}

int8 *tcradio_strcpy(int8 *pdst, int8 *psrc)
{
	int8 *p;

	p = pdst;

	while(*psrc != 0x00)
	{
		*pdst++ = *psrc++;
	}
	*pdst++='\0';

	return(p);
}

int8 *tcradio_strncpy(int8 *pdst, int8 *psrc, uint32 len)
{
	int8 *p;

	p = pdst;

	while(len != NULL)
	{
		*pdst++ = *psrc++;
		len--;
	}
	*pdst++='\0';

	return(p);
}

uint32 tcradio_strlen(int8 *psbStr)
{
	uint32 len;

	len=0;
	while(*psbStr++ != '\0')
	{
		len++;
	}
	return len;
}

int32 tcradio_strcmp(int8 *psbStr1, int8 *psbStr2)
{
	while(*psbStr1 != '\0')
	{
		if(*psbStr1++ != *psbStr2++){
			psbStr1--;
			psbStr2--;
			return((*psbStr1) - (*psbStr2));
		}
	}

	return 0;
}

int32 tcradio_strncmp(int8 *psbStr1, int8 *psbStr2, uint32 len)
{
	while(len != NULL)
	{
		if(*psbStr1++ != *psbStr2++){
			psbStr1--;
			psbStr2--;
			return( (*psbStr1) - (*psbStr2) );
			}
        len--;
	}

	return 0;
}


int8 *tcradio_strcat(int8 *psbStr1, int8 *psbStr2)
{
	while( *psbStr1++ != 0 )
	{
		;
	}
	psbStr1--;
	while(*psbStr2 != '\0')
	{
		*psbStr1++ = *psbStr2++;
	}

	return psbStr1;
}

uint32 BS_Truncation(uint32 ulData, uint16 uiUnit)
{
	return (ulData - (ulData % uiUnit));
}