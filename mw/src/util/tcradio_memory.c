/*******************************************************************************

*   FileName : tcradio_memory.c

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
#include <stdlib.h>

#include "tcradio_api.h"
#include "tcradio_memory.h"

/***************************************************
*        Global variable definitions               *
****************************************************/
#ifdef	TCC_MEMORY_DEBUG
static uint32 gMemallocCnt = 0;
static uint32 gMemRemain = 0;
#endif

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
void* tcradio_malloc (uint32 iSize)
{
#ifdef TCC_LINUX_MEMORY_SYTEM
	void *ptr;
	ptr = malloc(iSize);
#ifdef TCC_MEMORY_DEBUG
	ALOGD("mAlloc, 0x%08x, size[%d], mem cnt [%d]\n", ptr, iSize, gMemallocCnt);
	gMemallocCnt++;
#endif

	return ptr;
#else
#endif
}

void* tcradio_calloc (uint32 isize_t, uint32 iSize)
{
#ifdef TCC_LINUX_MEMORY_SYTEM
	void *ptr;
	ptr = calloc(isize_t, iSize);
#ifdef TCC_MEMORY_DEBUG
	ALOGD("cAlloc, 0x%08x, size[%d], mem cnt [%d]\n", ptr, iSize, gMemallocCnt);
	gMemallocCnt++;
#endif
	return ptr;
#else
#endif
}

void* tcradio_realloc (void *p, uint32 iSize)
{
#ifdef TCC_LINUX_MEMORY_SYTEM
	void *ptr;
	ptr = realloc(p, iSize);
#ifdef TCC_MEMORY_DEBUG
	ALOGD("reAlloc, 0x%08x, size[%d], mem cnt [%d]\n", ptr, iSize, gMemallocCnt);
	gMemallocCnt++;

#endif
	return ptr;
#else
#endif
}

int tcradio_free(void *pvPtr)
{
#ifdef TCC_LINUX_MEMORY_SYTEM
#ifdef TCC_MEMORY_DEBUG
	gMemallocCnt--;
	ALOGD("MemFree, 0x%08x, mem cnt [%d]\n", pvPtr, gMemallocCnt);
#endif
	if(pvPtr != NULL) {
		free(pvPtr);
		pvPtr = NULL;
	}
#else
#endif

	return 0;
}

