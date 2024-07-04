/*******************************************************************************

*   FileName : tchdr_ethernet.h

*   Copyright (c) Telechips Inc.

*   Description : TC HD Radio framework ethernet header

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
#ifndef TCHDR_ETHERNET_H__
#define TCHDR_ETHERNET_H__

/***************************************************
*				Include					*
****************************************************/

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
*				Enumeration				*
****************************************************/

/***************************************************
*				Typedefs					*
****************************************************/

/***************************************************
*			Constant definitions				*
****************************************************/

/***************************************************
*			Variable definitions				*
****************************************************/

/***************************************************
*			Function declaration				*
****************************************************/
/**
 * Opens(initializes) a Socket instance
 *
 * param port
 *     Selects the Ethernet Port number
 * param family
 *     Selects Socket Family
 * param type
 *     Selects UDP or TCP connection Type
 * return
 *     Returns assigned socket number or error if the operation failed.
 */
extern S32 tchdr_ethOpen(U32 port, U32 type);

extern S32 tchdr_ethWrite(U32 port, const U8 *buffer, S32 num_bytes);

extern S32 tchdr_ethRead(U32 port, U8* buffer, U32 length);

extern S32 tchdr_ethClose(U32 port);

extern S32 tchdr_ethReset(U32 port);

#ifdef __cplusplus
}
#endif

#endif /* TCHDR_ETHERNET_H__ */
