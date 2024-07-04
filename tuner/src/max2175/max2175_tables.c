/*******************************************************************************
* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.** The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
********************************************************************************/



/*
* This class contains the register sets used to program the registers by Receive Mode.
* The base register set is FM_EU_1.0  It contains all of the registers and is the first to be sent.
* All of the other register sets build on this FM_EU_1.0 set, and are just a subset of the registers.
* Only the registers that are different by mode.
* There are two main sets: EU (Europe) and NA (North America).
*/

/*
 * Version: 3.0
 * Release Date: 2/29/2016
 * Maxim Integrated Products
 * Author: Paul Nichol
 *
 * These are the setting tables for the various bands the MAX2175 can be set to.
*/
#include <stdio.h>

#include "tcradio_types.h"
#include "tcradio_peri_config.h"
#include "tcradio_drv.h"
#include "max2175_hal.h"


#define CH_MSEL 0
#define EQ_MSEL 1


/*
   <summary>
   Load the ADC registers and send them to the part.
   ADC-I channel and ADC-Q channel have to be sent out in seperate blocks or they do not load correctly.
   Updated from RA02_Power_Up_Sequence.doc 2/26/16

</summary>
*/
void LoadADC_Registers(int nTuner)
{
//	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	//Set 23 ADC-I Channel registers (146-168) and send out

	regArray[0x92] = 0x83;
	regArray[0x93] = 0x00;
	regArray[0x94] = 0xCF;
	regArray[0x95] = 0xB4;
	regArray[0x96] = 0x0F;
	regArray[0x97] = 0x2C;
	regArray[0x98] = 0x0C;
	regArray[0x99] = 0x49;
	regArray[0x9A] = 0x00;
	regArray[0x9B] = 0x00;
	regArray[0x9C] = 0x00;
	regArray[0x9D] = 0x8C;
	regArray[0x9E] = 0x02;
	regArray[0x9F] = 0x02;
	regArray[0xA0] = 0x00;
	regArray[0xA1] = 0x04;
	regArray[0xA2] = 0xEC;
	regArray[0xA3] = 0x82;
	regArray[0xA4] = 0x4B;
	regArray[0xA5] = 0xCC;
	regArray[0xA6] = 0x01;
	regArray[0xA7] = 0x88;
	regArray[0xA8] = 0x0C;

	// Now send out all the registers above Registers: 146-168
	write_registers(nTuner, 23, 0x92);

	//Set 23 ADC-Q Channel registers (201-223) and send out


	regArray[0xC9] = 0x83;
	regArray[0xCA] = 0x00;
	regArray[0xCB] = 0xCF;
	regArray[0xCC] = 0xB4;
	regArray[0xCD] = 0x0F;
	regArray[0xCE] = 0x2C;
	regArray[0xCF] = 0x0C;
	regArray[0xD0] = 0x49;
	regArray[0xD1] = 0x00;
	regArray[0xD2] = 0x00;
	regArray[0xD3] = 0x00;
	regArray[0xD4] = 0x8C;
	regArray[0xD5] = 0x02;
	regArray[0xD6] = 0x20;
	regArray[0xD7] = 0x33;
	regArray[0xD8] = 0x8C;
	regArray[0xD9] = 0x57;
	regArray[0xDA] = 0xD7;
	regArray[0xDB] = 0xD9;
	regArray[0xDC] = 0xB6;
	regArray[0xDD] = 0x65;
	regArray[0xDE] = 0x0E;
	regArray[0xDF] = 0x0C;

	// Now send out all the registers above.
	write_registers(nTuner, 23, 0xc9);
}


//These Tables were created from the Excel Register Table Revision: 130p7


// This should be sent twice during the init sequence only when init() is called
// for the European region.
void Load_FM_EU_1p0_FULL(int nTuner)
{
	RET xerr = eRET_OK;

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	regArray[1] = 0x15;
	regArray[2] = 0x04;
	regArray[3] = 0xB8;
	regArray[4] = 0xE3;
	regArray[5] = 0x35;
	regArray[6] = 0x18;
	regArray[7] = 0x7C;
	regArray[8] = 0x00;
	regArray[9] = 0x00;
	regArray[10] = 0x7D;
	regArray[11] = 0x40;
	regArray[12] = 0x08;
	regArray[13] = 0x80;
	regArray[14] = 0x77;
	regArray[15] = 0x88;
	regArray[16] = 0x71;
	regArray[17] = 0x61;
	regArray[18] = 0x61;
	regArray[19] = 0x61;
	regArray[20] = 0x61;
	regArray[21] = 0x5A;
	regArray[22] = 0x0F;
	regArray[23] = 0x34;
	regArray[24] = 0x40;
	regArray[25] = 0x20;
	regArray[26] = 0x88;
	regArray[27] = 0x33;
	regArray[28] = 0x02;

#ifdef SEPARATE
	regArray[29] = 0x00;
#else
    regArray[29] = 0x03;		/* 20160414 */
//    regArray[29] = 0x02;		/* 20160527 */
#endif

	regArray[30] = 0x01;
	regArray[31] = 0x00;
	regArray[32] = 0x65;
	regArray[33] = 0x9F;
	regArray[34] = 0x2B;
	regArray[35] = 0x80;
	regArray[36] = 0x00;
	regArray[37] = 0x95;
	regArray[38] = 0x04;
	regArray[39] = 0x37;
	regArray[40] = 0x00;
	regArray[41] = 0x00;
	regArray[42] = 0x00;
	regArray[43] = 0x00;
	regArray[44] = 0x00;
	regArray[45] = 0x00;
	regArray[46] = 0x00;
	regArray[47] = 0x00;
	regArray[48] = 0x40;
	regArray[49] = 0x4A;
	regArray[50] = 0x08;
	regArray[51] = 0xA8;
	regArray[52] = 0x0E;
	regArray[53] = 0x0E;
	regArray[54] = 0x37;
	regArray[55] = 0x7E;
	regArray[56] = 0x00;
	regArray[57] = 0x00;
	regArray[58] = 0x00;
	regArray[59] = 0x00;
	regArray[60] = 0x00;
	regArray[61] = 0x00;
	regArray[62] = 0x00;
	regArray[63] = 0x00;
	regArray[64] = 0x00;
	regArray[65] = 0x00;
	regArray[66] = 0x00;
	regArray[67] = 0x00;
	regArray[68] = 0x00;
	regArray[69] = 0x00;
	regArray[70] = 0xAB;
	regArray[71] = 0x5E;
	regArray[72] = 0xAA;
	regArray[73] = 0xAF;
	regArray[74] = 0xBB;
	regArray[75] = 0x5F;
	regArray[76] = 0x1B;
	regArray[77] = 0x3B;
	regArray[78] = 0x03;
	regArray[79] = 0x3B;
	regArray[80] = 0x64;
	regArray[81] = 0x40;
	regArray[82] = 0x60;
	regArray[83] = 0x00;
	regArray[84] = 0x2A;
	regArray[85] = 0xBF;
	regArray[86] = 0x3F;
	regArray[87] = 0xFF;
	regArray[88] = 0x99;
	regArray[89] = 0x00;
	regArray[90] = 0x00;
	regArray[91] = 0x00;
	regArray[92] = 0x00;
	regArray[93] = 0x00;
	regArray[94] = 0x00;
	regArray[95] = 0x0A;
	regArray[96] = 0x00;
	regArray[97] = 0xFF;
	regArray[98] = 0xFC;
	regArray[99] = 0xEF;
	regArray[100] = 0x1C;
	regArray[101] = 0x40;
	regArray[102] = 0x00;
	regArray[103] = 0x00;
	regArray[104] = 0x02;
	regArray[105] = 0x00;
	regArray[106] = 0x00;
	regArray[107] = 0xE0;
	regArray[108] = 0x00;
	regArray[109] = 0x00;
	regArray[110] = 0x00;
	regArray[111] = 0x00;
	regArray[112] = 0x00;
	regArray[113] = 0x00;
	regArray[114] = 0x00;
	regArray[115] = 0x00;
	regArray[116] = 0x00;
	regArray[117] = 0x00;
	regArray[118] = 0xAC;
	regArray[119] = 0x40;
	regArray[120] = 0x00;
	regArray[121] = 0x00;
	regArray[122] = 0x00;
	regArray[123] = 0x00;
	regArray[124] = 0x00;
	regArray[125] = 0x00;
	regArray[126] = 0x75;
	regArray[127] = 0x00;
	regArray[128] = 0x00;
	regArray[129] = 0x00;
	regArray[130] = 0x47;
	regArray[131] = 0x00;
	regArray[132] = 0x00;
	regArray[133] = 0x11;
	regArray[134] = 0x3F;
	regArray[135] = 0x22;
	regArray[136] = 0x00;
	regArray[137] = 0xF1;
	regArray[138] = 0x00;
	regArray[139] = 0x41;
	regArray[140] = 0x03;
	regArray[141] = 0xB0;
	regArray[142] = 0x00;
	regArray[143] = 0x00;
	regArray[144] = 0x00;
	regArray[145] = 0x1B;


	// Now send out all the registers above. Except register 0
	xerr = write_registers(nTuner, 145, 1);
	if(xerr != eRET_OK) {
		return;
	}

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 36;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());


	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
//	Load_Filter_Coefficients_1bank(EQ_MSEL, 0, EQ_Coeff_EQ_FMEU1_RA02_m6dB());
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMEU1());
}

//This is the full set for the FM_NA1.0 receive mode.
// This should be sent twice during the init sequence only when init() is called
// for the North American region.
void Load_FM_NA_1p0_FULL(int nTuner)
{
	RET xerr = eRET_OK;

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	regArray[1] = 0x13;
	regArray[2] = 0x08;
	regArray[3] = 0x8D;
	regArray[4] = 0xC0;
	regArray[5] = 0x35;
	regArray[6] = 0x18;
	regArray[7] = 0x7D;
	regArray[8] = 0x3F;
	regArray[9] = 0x7D;
	regArray[10] = 0x75;
	regArray[11] = 0x40;
	regArray[12] = 0x08;
	regArray[13] = 0x80;
	regArray[14] = 0x77;
	regArray[15] = 0x88;
	regArray[16] = 0x71;
	regArray[17] = 0x61;
	regArray[18] = 0x61;
	regArray[19] = 0x61;
	regArray[20] = 0x61;
	regArray[21] = 0x5C;
	regArray[22] = 0x0F;
	regArray[23] = 0x34;
	regArray[24] = 0x40;
	regArray[25] = 0x20;
	regArray[26] = 0x88;
	regArray[27] = 0x33;
	regArray[28] = 0x02;
#ifdef SEPARATE
	regArray[29] = 0x00;
#else
	regArray[29] = 0x03;	// I2S mode for combine mode 2016.7.26 masato
#endif
	regArray[30] = 0x01;
	regArray[31] = 0x00;
	regArray[32] = 0x65;
	regArray[33] = 0x9F;
	regArray[34] = 0x2B;
	regArray[35] = 0x80;
	regArray[36] = 0x00;
	regArray[37] = 0x95;
	regArray[38] = 0x04;
	regArray[39] = 0x37;
	regArray[40] = 0x00;
	regArray[41] = 0x00;
	regArray[42] = 0x00;
	regArray[43] = 0x00;
	regArray[44] = 0x00;
	regArray[45] = 0x00;
	regArray[46] = 0x00;
	regArray[47] = 0x00;
	regArray[48] = 0x40;
	regArray[49] = 0x4A;
	regArray[50] = 0x08;
	regArray[51] = 0xA8;
	regArray[52] = 0x0E;
	regArray[53] = 0x0E;
	regArray[54] = 0xB7;
	regArray[55] = 0x7E;
	regArray[56] = 0x00;
	regArray[57] = 0x00;
	regArray[58] = 0x00;
	regArray[59] = 0x00;
	regArray[60] = 0x00;
	regArray[61] = 0x00;
	regArray[62] = 0x00;
	regArray[63] = 0x00;
	regArray[64] = 0x00;
	regArray[65] = 0x00;
	regArray[66] = 0x00;
	regArray[67] = 0x00;
	regArray[68] = 0x00;
	regArray[69] = 0x00;
	regArray[70] = 0xAB;
	regArray[71] = 0x5E;
	regArray[72] = 0xAA;
	regArray[73] = 0xAF;
	regArray[74] = 0xBB;
	regArray[75] = 0x5F;
	regArray[76] = 0x1B;
	regArray[77] = 0x3B;
	regArray[78] = 0x03;
	regArray[79] = 0x3B;
	regArray[80] = 0x64;
	regArray[81] = 0x40;
	regArray[82] = 0x60;
	regArray[83] = 0x00;
	regArray[84] = 0x2A;
	regArray[85] = 0xBF;
	regArray[86] = 0x3F;
	regArray[87] = 0xFF;
	regArray[88] = 0x99;
	regArray[89] = 0x00;
	regArray[90] = 0x00;
	regArray[91] = 0x00;
	regArray[92] = 0x00;
	regArray[93] = 0x00;
	regArray[94] = 0x00;
	regArray[95] = 0x0A;
	regArray[96] = 0x00;
	regArray[97] = 0xFF;
	regArray[98] = 0xFC;
	regArray[99] = 0xEF;
	regArray[100] = 0x1C;
	regArray[101] = 0x40;
	regArray[102] = 0x00;
	regArray[103] = 0x00;
	regArray[104] = 0x02;
	regArray[105] = 0x00;
	regArray[106] = 0x00;
	regArray[107] = 0xE0;
	regArray[108] = 0x00;
	regArray[109] = 0x00;
	regArray[110] = 0x00;
	regArray[111] = 0x00;
	regArray[112] = 0x00;
	regArray[113] = 0x00;
	regArray[114] = 0x00;
	regArray[115] = 0x00;
	regArray[116] = 0x00;
	regArray[117] = 0x00;
	regArray[118] = 0xA6;
	regArray[119] = 0x40;
	regArray[120] = 0x00;
	regArray[121] = 0x00;
	regArray[122] = 0x00;
	regArray[123] = 0x00;
	regArray[124] = 0x00;
	regArray[125] = 0x00;
	regArray[126] = 0x75;
	regArray[127] = 0x00;
	regArray[128] = 0x00;
	regArray[129] = 0x00;
	regArray[130] = 0x35;
	regArray[131] = 0x00;
	regArray[132] = 0x00;
	regArray[133] = 0x11;
	regArray[134] = 0x3F;
	regArray[135] = 0x22;
	regArray[136] = 0x00;
	regArray[137] = 0xF1;
	regArray[138] = 0x00;
	regArray[139] = 0x41;
	regArray[140] = 0x03;
	regArray[141] = 0xB0;
	regArray[142] = 0x00;
	regArray[143] = 0x00;
	regArray[144] = 0x00;
	regArray[145] = 0x1B;


	// Now send out all the registers above. Except register 0
	xerr = write_registers(nTuner, 145, 1);
	if(xerr != eRET_OK) {
		return;
	}

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 27;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMNA());


	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMNA1());
}   //End Load_FM_NA_1p0_FULL()

	// This is the minimal set of registers that need to change when
	// switching between any two European receive modes. FM_EU_1p0
	// These are only the registers that differ from the default set:
	// Load_FM_EU_1p0_FULL. By sending only the registers that change between
	// One European modes and another, we send fewer registers and can change modes faster.
	// The subset is determined by examining all registers that change from the default set
	// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_EU_1p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x00);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7D);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x09);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0xAC);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 36;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
//	Load_Filter_Coefficients_1bank(EQ_MSEL, 0, EQ_Coeff_EQ_FMEU1_RA02_m6dB());
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMEU1());
}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_EU_1p1
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_EU_1p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x00);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x65);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x05);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0xAC);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 36;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
//	Load_Filter_Coefficients_1bank(EQ_MSEL, 0, EQ_Coeff_EQ_FMEU1_RA02_m6dB());
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMEU1());
}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_EU_1p2
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_EU_1p2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);
	if(cumulativeError> MAX_ERROR)
		return;
	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x00);
	write_register(nTuner, 9, 0x00);

#ifdef SEPARATE
	write_register(nTuner, 10, 0x73);
	write_register(nTuner, 11, 0x40);
#else
    write_register(nTuner, 10, 0x65);		/* 20160419 */
	write_register(nTuner, 11, 0x40);
#endif

	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);

#ifdef SEPARATE
	write_register(nTuner, 29, 0x00);
#else
    write_register(nTuner, 29, 0x03);		/* 20160414 */
//    write_register(nTuner, 29, 0x02);		/* 20160414 */
#endif

	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);

#ifdef DAGC
    write_register(nTuner, 88, 0x99);
#else
    write_register(nTuner, 88, 0x98);
#endif

    write_register(nTuner, 118, 0xAC);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 36;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
//	Load_Filter_Coefficients_1bank(EQ_MSEL, 0, EQ_Coeff_EQ_FMEU1_RA02_m6dB());
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMEU1());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_EU_1p3
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_EU_1p3(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x00);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7E);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x0C);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0xAC);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 36;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
//	Load_Filter_Coefficients_1bank(EQ_MSEL, 0, EQ_Coeff_EQ_FMEU1_RA02_m6dB());
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMEU1());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_EU_1p7
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_EU_1p7(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x5C);
	write_register(nTuner, 8, 0x00);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0xFD);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x0E);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0xAC);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 36;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
//	Load_Filter_Coefficients_1bank(EQ_MSEL, 0, EQ_Coeff_EQ_FMEU1_RA02_m6dB());
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMEU1());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_HD_4p0
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_HD_4p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x70);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7D);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x18);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x05);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0xB0);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 18;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
//	Load_Filter_Coefficients_1bank(EQ_MSEL, 0, EQ_Coeff_EQ_FMHD4_RA02());
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMHD4());

	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_FMHD());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_HD_4p3
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_HD_4p3(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x70);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7E);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x18);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x0B);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0xB0);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 18;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
//	Load_Filter_Coefficients_1bank(EQ_MSEL, 0, EQ_Coeff_EQ_FMHD4_RA02());
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMHD4());

	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_FMHD());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_HD_4p7
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_HD_4p7(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x5C);
	write_register(nTuner, 8, 0x70);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0xFD);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x18);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x08);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0xB0);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 18;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
//	Load_Filter_Coefficients_1bank(EQ_MSEL, 0, EQ_Coeff_EQ_FMHD4_RA02());
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMHD4());

	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_FMHD());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_HD_5p0
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_HD_5p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x70);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7A);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x18);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x07);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x94);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 24;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMHD5_RA02());

	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_FMHD());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_HD_5p1
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_HD_5p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x70);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x62);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x18);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x03);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x94);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 24;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMHD5_RA02());

	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_FMHD());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_HD_5p2
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_HD_5p2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x70);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7E);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x18);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x06);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x94);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 24;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMHD5_RA02());

	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_FMHD());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_EU_2p0
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_EU_2p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x71);
	write_register(nTuner, 9, 0xC7);
	write_register(nTuner, 10, 0x7A);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x48);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x09);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x8C);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x3F);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 32;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 1, EQ_Coeff_EQ_FMEU2());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_EU_2p1
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_EU_2p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x71);
	write_register(nTuner, 9, 0xC7);
	write_register(nTuner, 10, 0x62);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x48);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x05);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x8C);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x3F);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 32;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 1, EQ_Coeff_EQ_FMEU2());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. FM_EU_2p2
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_FM_EU_2p2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x71);
	write_register(nTuner, 9, 0xC7);
	write_register(nTuner, 10, 0x42);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x48);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 21, 0x5A);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x8C);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x3F);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 32;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMEU());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 1, EQ_Coeff_EQ_FMEU2());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. AM_EU_1p0
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_AM_EU_1p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x00);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x10);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7D);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x09);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x18);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. AM_EU_1p1
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_AM_EU_1p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x00);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x10);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x65);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x05);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x18);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. AM_EU_1p2
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_AM_EU_1p2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);
	if(cumulativeError> MAX_ERROR)
		return;
	write_register(nTuner, 1, 0x00);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x10);
	write_register(nTuner, 9, 0x00);

#ifdef SEPARATE
    write_register(nTuner, 10, 0x73);
#else
    write_register(nTuner, 10, 0x65);
#endif

    write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);

#ifdef SEPARATE
    write_register(nTuner, 29, 0x00);
#else
    write_register(nTuner, 29, 0x03);
#endif

    write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x18);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. AM_EU_1p3
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_AM_EU_1p3(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x00);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x10);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7E);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x0C);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x18);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. AM_EU_1p7
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_AM_EU_1p7(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x00);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x5F);
	write_register(nTuner, 8, 0x10);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0xFD);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x0E);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x18);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. AM_EU_2p0
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_AM_EU_2p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x00);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x10);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7A);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x09);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x12);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x3F);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. AM_EU_2p1
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_AM_EU_2p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x00);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x10);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x62);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x05);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x12);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x3F);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. AM_EU_2p2
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_AM_EU_2p2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x00);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x10);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x42);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x12);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x3F);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. MWAM_EU_1
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_MWAM_EU_1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x00);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x10);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7D);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x0A);
	write_register(nTuner, 120, 0x20);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x11);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_MWAM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_MWAM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. MWAM_EU_2
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_MWAM_EU_2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x00);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x10);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7A);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x03);
	write_register(nTuner, 120, 0xA0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x0F);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x0F);
	write_register(nTuner, 134, 0x2F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_MWAM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_MWAM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. SWAM_0
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_SWAM_0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x00);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x77);
	write_register(nTuner, 8, 0x86);
	write_register(nTuner, 9, 0x39);
	write_register(nTuner, 10, 0x73);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x18);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x18);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x47);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. SWAM_1
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_SWAM_1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x12);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x74);
	write_register(nTuner, 8, 0xE9);
	write_register(nTuner, 9, 0x16);
	write_register(nTuner, 10, 0x62);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x18);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x05);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0xBF);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x18);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x3F);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. SWAM_2
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_SWAM_2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x05);
	write_register(nTuner, 3, 0x55);
	write_register(nTuner, 4, 0x55);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x6E);
	write_register(nTuner, 8, 0x5B);
	write_register(nTuner, 9, 0x55);
	write_register(nTuner, 10, 0x62);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x18);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x03);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0xBF);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x12);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x2F);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. SWAM_3
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_SWAM_3(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x14);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x6D);
	write_register(nTuner, 8, 0x2E);
	write_register(nTuner, 9, 0x39);
	write_register(nTuner, 10, 0x62);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x18);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x04);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0xBF);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x12);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. SWAM_4
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_SWAM_4(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x11);
	write_register(nTuner, 2, 0x0C);
	write_register(nTuner, 3, 0x71);
	write_register(nTuner, 4, 0xC7);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 6, 0x18);
	write_register(nTuner, 7, 0x6A);
	write_register(nTuner, 8, 0xD4);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x65);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x18);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x41);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 21, 0x5C);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x04);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 51, 0xA8);
	write_register(nTuner, 54, 0x37);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0xBF);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x12);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. DAB_1_VHF
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_DAB_1_VHF(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x0D);
	write_register(nTuner, 3, 0x15);
	write_register(nTuner, 4, 0x55);
	write_register(nTuner, 5, 0x0A);
	write_register(nTuner, 6, 0xA0);
	write_register(nTuner, 7, 0x60);
	write_register(nTuner, 8, 0x00);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7D);
	write_register(nTuner, 11, 0x4A);
	write_register(nTuner, 12, 0x28);
	write_register(nTuner, 14, 0x43);
	write_register(nTuner, 15, 0xB5);
	write_register(nTuner, 16, 0x31);
	write_register(nTuner, 17, 0x9E);
	write_register(nTuner, 18, 0x68);
	write_register(nTuner, 19, 0x9E);
	write_register(nTuner, 20, 0x68);
	write_register(nTuner, 21, 0x58);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x3F);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x5A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xF8);
	write_register(nTuner, 54, 0x2F);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xAF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xF8);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x02);
	write_register(nTuner, 121, 0x40);
	write_register(nTuner, 130, 0x11);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_DAB1());



}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. DAB_1_LB
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_DAB_1_LB(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x0C);
	write_register(nTuner, 3, 0xCC);
	write_register(nTuner, 4, 0x71);
	write_register(nTuner, 5, 0x0F);
	write_register(nTuner, 6, 0xA0);
	write_register(nTuner, 7, 0x60);
	write_register(nTuner, 8, 0x00);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7D);
	write_register(nTuner, 11, 0x7A);
	write_register(nTuner, 12, 0x28);
	write_register(nTuner, 14, 0x43);
	write_register(nTuner, 15, 0xB5);
	write_register(nTuner, 16, 0x11);
	write_register(nTuner, 17, 0x9E);
	write_register(nTuner, 18, 0x68);
	write_register(nTuner, 19, 0x9E);
	write_register(nTuner, 20, 0x68);
	write_register(nTuner, 21, 0x58);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x3F);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x5A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xF8);
	write_register(nTuner, 54, 0x2F);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xBF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xF8);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x02);
	write_register(nTuner, 121, 0x40);
	write_register(nTuner, 130, 0x11);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_DAB1());



}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. DAB_2
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_DAB_2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x0D);
	write_register(nTuner, 3, 0x15);
	write_register(nTuner, 4, 0x55);
	write_register(nTuner, 5, 0x0A);
	write_register(nTuner, 6, 0xA0);
	write_register(nTuner, 7, 0x60);
	write_register(nTuner, 8, 0x00);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7A);
	write_register(nTuner, 11, 0x4F);
	write_register(nTuner, 12, 0x28);
	write_register(nTuner, 14, 0x43);
	write_register(nTuner, 15, 0xB5);
	write_register(nTuner, 16, 0x31);
	write_register(nTuner, 17, 0x9E);
	write_register(nTuner, 18, 0x68);
	write_register(nTuner, 19, 0x9E);
	write_register(nTuner, 20, 0x68);
	write_register(nTuner, 21, 0x58);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x3F);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x5A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xF8);
	write_register(nTuner, 54, 0x2F);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xAF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xF8);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x01);
	write_register(nTuner, 130, 0x0F);
	write_register(nTuner, 131, 0x01);
	write_register(nTuner, 133, 0x0F);
	write_register(nTuner, 134, 0x2F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #3
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 3, Ch_Coeff_CH_DAB2());



}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. DAB_1p1
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_DAB_1p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x0D);
	write_register(nTuner, 3, 0x15);
	write_register(nTuner, 4, 0x55);
	write_register(nTuner, 5, 0x0A);
	write_register(nTuner, 6, 0xA0);
	write_register(nTuner, 7, 0x60);
	write_register(nTuner, 8, 0x00);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7E);
	write_register(nTuner, 11, 0x4A);
	write_register(nTuner, 12, 0x28);
	write_register(nTuner, 14, 0x43);
	write_register(nTuner, 15, 0xB5);
	write_register(nTuner, 16, 0x31);
	write_register(nTuner, 17, 0x9E);
	write_register(nTuner, 18, 0x68);
	write_register(nTuner, 19, 0x9E);
	write_register(nTuner, 20, 0x68);
	write_register(nTuner, 21, 0x58);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x3F);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x5A);
	write_register(nTuner, 29, 0x01);
	write_register(nTuner, 30, 0x00);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xF8);
	write_register(nTuner, 54, 0x2F);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xAF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xF8);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x02);
	write_register(nTuner, 121, 0x40);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_DAB1());



}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. DAB_1p2
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_DAB_1p2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x0D);
	write_register(nTuner, 3, 0x15);
	write_register(nTuner, 4, 0x55);
	write_register(nTuner, 5, 0x0A);
	write_register(nTuner, 6, 0xA0);
	write_register(nTuner, 7, 0x40);
	write_register(nTuner, 8, 0x00);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7D);
	write_register(nTuner, 11, 0x4A);
	write_register(nTuner, 12, 0x28);
	write_register(nTuner, 14, 0x43);
	write_register(nTuner, 15, 0xB5);
	write_register(nTuner, 16, 0x31);
	write_register(nTuner, 17, 0x9E);
	write_register(nTuner, 18, 0x68);
	write_register(nTuner, 19, 0x9E);
	write_register(nTuner, 20, 0x68);
	write_register(nTuner, 21, 0x58);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x3F);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x5A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x00);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xF8);
	write_register(nTuner, 54, 0x2F);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xAF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xF8);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x02);
	write_register(nTuner, 121, 0x40);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_DAB1());



}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. DAB_1p3
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_DAB_1p3(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x0D);
	write_register(nTuner, 3, 0x15);
	write_register(nTuner, 4, 0x55);
	write_register(nTuner, 5, 0x0A);
	write_register(nTuner, 6, 0xA0);
	write_register(nTuner, 7, 0x60);
	write_register(nTuner, 8, 0x00);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0x7E);
	write_register(nTuner, 11, 0x4A);
	write_register(nTuner, 12, 0x28);
	write_register(nTuner, 14, 0x43);
	write_register(nTuner, 15, 0xB5);
	write_register(nTuner, 16, 0x31);
	write_register(nTuner, 17, 0x9E);
	write_register(nTuner, 18, 0x68);
	write_register(nTuner, 19, 0x9E);
	write_register(nTuner, 20, 0x68);
	write_register(nTuner, 21, 0x58);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x3F);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x5A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x0A);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xF8);
	write_register(nTuner, 54, 0x2F);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xAF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xF8);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x02);
	write_register(nTuner, 121, 0x40);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_DAB1());



}


// This is the minimal set of registers that need to change when
// switching between any two European receive modes. DAB_1p7
// These are only the registers that differ from the default set:
// Load_FM_EU_1p0_FULL. By sending only the registers that change between
// One European modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_EU_1p0_FULL) and determining all the registers that change per any new European receive mode.
void Load_DAB_1p7(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x0D);
	write_register(nTuner, 3, 0x15);
	write_register(nTuner, 4, 0x55);
	write_register(nTuner, 5, 0x0A);
	write_register(nTuner, 6, 0xA0);
	write_register(nTuner, 7, 0x40);
	write_register(nTuner, 8, 0x00);
	write_register(nTuner, 9, 0x00);
	write_register(nTuner, 10, 0xFD);
	write_register(nTuner, 11, 0x4A);
	write_register(nTuner, 12, 0x28);
	write_register(nTuner, 14, 0x43);
	write_register(nTuner, 15, 0xB5);
	write_register(nTuner, 16, 0x31);
	write_register(nTuner, 17, 0x9E);
	write_register(nTuner, 18, 0x68);
	write_register(nTuner, 19, 0x9E);
	write_register(nTuner, 20, 0x68);
	write_register(nTuner, 21, 0x58);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x3F);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0xAB);
	write_register(nTuner, 28, 0x5A);
	write_register(nTuner, 29, 0x00);
	write_register(nTuner, 30, 0x04);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 51, 0xF8);
	write_register(nTuner, 54, 0x2F);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 85, 0xAF);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xF8);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x02);
	write_register(nTuner, 121, 0x40);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 131, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_DAB1());



}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. FM_NA_1p0
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_FM_NA_1p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x08);
	write_register(nTuner, 3, 0x8D);
	write_register(nTuner, 4, 0xC0);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 7, 0x7D);
	write_register(nTuner, 8, 0x3F);
	write_register(nTuner, 9, 0x7D);
	/* as default value ROPLL Div_27 */
	write_register(nTuner, 10, 0x75);
	/* as default value ROPLL Div_12 */
//	write_register(nTuner, 10, 0x7B);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
//	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 88, 0x9F);
	write_register(nTuner, 118, 0xA6);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x35);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 27;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMNA());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMNA1());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. FM_NA_1p1
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_FM_NA_1p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x08);
	write_register(nTuner, 3, 0x8D);
	write_register(nTuner, 4, 0xC0);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 7, 0x5D);
	write_register(nTuner, 8, 0x3F);
	write_register(nTuner, 9, 0x7D);
	write_register(nTuner, 10, 0x65);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x02);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0xA6);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 27;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMNA());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMNA1());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. FM_NA_1p2
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_FM_NA_1p2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x08);
	write_register(nTuner, 3, 0x8D);
	write_register(nTuner, 4, 0xC0);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 7, 0x5D);
	write_register(nTuner, 8, 0x3F);
	write_register(nTuner, 9, 0x7D);
	write_register(nTuner, 10, 0x7D);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x06);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0xA6);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 27;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMNA());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMNA1());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. FM_NA_2p0
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_FM_NA_2p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x08);
	write_register(nTuner, 3, 0x8D);
	write_register(nTuner, 4, 0xC0);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x54);
	write_register(nTuner, 9, 0xA7);
	write_register(nTuner, 10, 0x55);
	write_register(nTuner, 11, 0x42);
	write_register(nTuner, 12, 0x48);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0xAC);
	write_register(nTuner, 119, 0xC0);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x6B);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 36;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_FMHD());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 1, EQ_Coeff_EQ_FMNA2());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. FM_NA_3p0
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_FM_NA_3p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x08);
	write_register(nTuner, 3, 0x8D);
	write_register(nTuner, 4, 0xC0);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0xD9);
	write_register(nTuner, 9, 0x67);
	write_register(nTuner, 10, 0x46);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0xC8);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x00);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x8C);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 32;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMNA());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #3
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 3, EQ_Coeff_EQ_FMNA30());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. FM_NA_3p1
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_FM_NA_3p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x13);
	write_register(nTuner, 2, 0x08);
	write_register(nTuner, 3, 0x8D);
	write_register(nTuner, 4, 0xC0);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0xF7);
	write_register(nTuner, 9, 0x47);
	write_register(nTuner, 10, 0x46);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0xC8);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x00);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xF7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0x8C);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 32;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMNA());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #3
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 3, EQ_Coeff_EQ_FMNA31());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. WX_2p0
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_WX_2p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x11);
	write_register(nTuner, 2, 0x09);
	write_register(nTuner, 3, 0x40);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0x2A);
	write_register(nTuner, 7, 0x7C);
	write_register(nTuner, 8, 0x54);
	write_register(nTuner, 9, 0xA7);
	write_register(nTuner, 10, 0x55);
	write_register(nTuner, 11, 0x42);
	write_register(nTuner, 12, 0x48);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0xAC);
	write_register(nTuner, 119, 0xC0);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x6B);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 36;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_FMHD());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 1, EQ_Coeff_EQ_FMNA2());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. WX_1p0
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_WX_1p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x11);
	write_register(nTuner, 2, 0x09);
	write_register(nTuner, 3, 0x40);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0x2A);
	write_register(nTuner, 7, 0x7D);
	write_register(nTuner, 8, 0x3F);
	write_register(nTuner, 9, 0x7D);
	write_register(nTuner, 10, 0x75);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0xA6);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x35);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 27;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMNA());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMNA1());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. WX_1p1
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_WX_1p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x11);
	write_register(nTuner, 2, 0x09);
	write_register(nTuner, 3, 0x40);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0x2A);
	write_register(nTuner, 7, 0x5D);
	write_register(nTuner, 8, 0x3F);
	write_register(nTuner, 9, 0x7D);
	write_register(nTuner, 10, 0x65);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x02);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0xA6);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 27;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMNA());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMNA1());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. WX_1p2
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_WX_1p2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x11);
	write_register(nTuner, 2, 0x09);
	write_register(nTuner, 3, 0x40);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0x2A);
	write_register(nTuner, 7, 0x5D);
	write_register(nTuner, 8, 0x3F);
	write_register(nTuner, 9, 0x7D);
	write_register(nTuner, 10, 0x7D);
	write_register(nTuner, 11, 0x40);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x0F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x06);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0x99);
	write_register(nTuner, 118, 0xA6);
	write_register(nTuner, 119, 0x40);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 27;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 0, Ch_Coeff_CH_FMNA());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #0
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 0, EQ_Coeff_EQ_FMNA1());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. FM_HD_1p0
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_FM_HD_1p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);
	if(cumulativeError> MAX_ERROR)
		return;
	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 7, 0x7B);
	write_register(nTuner, 8, 0x19);
	write_register(nTuner, 9, 0x17);
	/* as default value ROPLL Div_27 */
	write_register(nTuner, 10, 0x75);
	/* as default value ROPLL Div_12 */
//	write_register(nTuner, 10, 0x7B);
	write_register(nTuner, 11, 0x4A);
	write_register(nTuner, 12, 0x98);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
//	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 28, 0x00);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);

#ifdef DAGC
//	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 88, 0xBF);
#else
    write_register(nTuner, 88, 0x88);
#endif

    write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x0A);
	write_register(nTuner, 121, 0x64);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x35);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 27;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_FMHD());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 2, EQ_Coeff_EQ_FMHD1());

	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_FMHD());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. FM_HD_1p1
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_FM_HD_1p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 7, 0x5B);
	write_register(nTuner, 8, 0x19);
	write_register(nTuner, 9, 0x17);
	write_register(nTuner, 10, 0x65);
	write_register(nTuner, 11, 0x4A);
	write_register(nTuner, 12, 0x98);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x02);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x0A);
	write_register(nTuner, 121, 0x64);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 27;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_FMHD());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 2, EQ_Coeff_EQ_FMHD1());

	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_FMHD());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. FM_HD_1p2
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_FM_HD_1p2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 7, 0x5B);
	write_register(nTuner, 8, 0x19);
	write_register(nTuner, 9, 0x17);
	write_register(nTuner, 10, 0x7D);
	write_register(nTuner, 11, 0x4A);
	write_register(nTuner, 12, 0x98);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x06);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x0A);
	write_register(nTuner, 121, 0x64);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 27;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_FMHD());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 2, EQ_Coeff_EQ_FMHD1());

	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_FMHD());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. FM_HD_3p0
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_FM_HD_3p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 7, 0x7A);
	write_register(nTuner, 8, 0x63);
	write_register(nTuner, 9, 0x40);
	write_register(nTuner, 10, 0x46);
	write_register(nTuner, 11, 0x4A);
	write_register(nTuner, 12, 0xD8);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x00);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x08);
	write_register(nTuner, 121, 0xC4);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 32;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_FMHD());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #3
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 3, EQ_Coeff_EQ_FMHD30());

	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_FMHD());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. FM_HD_3p1
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_FM_HD_3p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x15);
	write_register(nTuner, 2, 0x04);
	write_register(nTuner, 3, 0xB8);
	write_register(nTuner, 4, 0xE3);
	write_register(nTuner, 5, 0x35);
	write_register(nTuner, 7, 0x7A);
	write_register(nTuner, 8, 0x98);
	write_register(nTuner, 9, 0x76);
	write_register(nTuner, 10, 0x46);
	write_register(nTuner, 11, 0x4A);
	write_register(nTuner, 12, 0xD8);
	write_register(nTuner, 14, 0x77);
	write_register(nTuner, 15, 0x88);
	write_register(nTuner, 16, 0x71);
	write_register(nTuner, 17, 0x61);
	write_register(nTuner, 18, 0x61);
	write_register(nTuner, 19, 0x61);
	write_register(nTuner, 20, 0x61);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x34);
	write_register(nTuner, 24, 0x40);
	write_register(nTuner, 26, 0x88);
	write_register(nTuner, 27, 0x33);
	write_register(nTuner, 28, 0x02);
	write_register(nTuner, 30, 0x00);
	write_register(nTuner, 37, 0x95);
	write_register(nTuner, 38, 0x04);
	write_register(nTuner, 39, 0x37);
	write_register(nTuner, 50, 0x08);
	write_register(nTuner, 54, 0xF7);
	write_register(nTuner, 55, 0x7E);
	write_register(nTuner, 86, 0x3F);
	write_register(nTuner, 87, 0xFF);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x08);
	write_register(nTuner, 121, 0xC4);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 32;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #2
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 2, Ch_Coeff_CH_FMHD());

	//Load the EQ Filter Coefficients into the IC's Channel Filter Bank #3
	Load_Filter_Coefficients_1bank(nTuner, EQ_MSEL, 3, EQ_Coeff_EQ_FMHD31());

	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_FMHD());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. AM_NA_1p0
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_AM_NA_1p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);
	if(cumulativeError> MAX_ERROR)
		return;
#if 1
	write_register(nTuner, 1, 0x12);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x23);
	write_register(nTuner, 9, 0xD7);
	write_register(nTuner, 10, 0x75);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0xA8);
	write_register(nTuner, 16, 0x51);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xB3);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
//	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 88, 0xFF);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x18);
	write_register(nTuner, 120, 0xC0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x35);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);
#elif 0
	write_register(nTuner, 1, 0x12);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x23);
	write_register(nTuner, 9, 0xD7);
	write_register(nTuner, 10, 0x75);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0xA8);
	write_register(nTuner, 16, 0x51);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
//	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 24, 0x25);
	write_register(nTuner, 26, 0x99);
//	write_register(nTuner, 27, 0xB3);
	write_register(nTuner, 27, 0xAA);
//	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 28, 0x9A);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
//	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 54, 0xAF);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
//	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 88, 0xFF);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x18);
	write_register(nTuner, 120, 0xC0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x35);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);
#else
	write_register(nTuner,1, 0x12);
	write_register(nTuner,2, 0x00);
	write_register(nTuner,3, 0x00);
	write_register(nTuner,4, 0x00);
	write_register(nTuner,5, 0xC0);
	write_register(nTuner,6, 0x18);
	write_register(nTuner,7, 0x7F);
	write_register(nTuner,8, 0x23);
	write_register(nTuner,9, 0xD7);
	write_register(nTuner,10, 0x75);
	write_register(nTuner,11, 0x45);
	write_register(nTuner,12, 0x08);
	write_register(nTuner,14, 0x70);
	write_register(nTuner,15, 0xA9);
	write_register(nTuner,16, 0x51);
	write_register(nTuner,17, 0xA1);
	write_register(nTuner,18, 0x74);
	write_register(nTuner,19, 0xA1);
	write_register(nTuner,20, 0x74);
	write_register(nTuner,21, 0x5C);
	write_register(nTuner,22, 0x2F);
	write_register(nTuner,23, 0x2F);
	write_register(nTuner,24, 0x25);
	write_register(nTuner,26, 0x99);
	write_register(nTuner,27, 0xAA);
	write_register(nTuner,28, 0x9A);
	write_register(nTuner,29, 0x00);
	write_register(nTuner,30, 0x01);
	write_register(nTuner,35, 0xC0);
	write_register(nTuner,36, 0x44);
	write_register(nTuner,37, 0x00);
	write_register(nTuner,38, 0x00);
	write_register(nTuner,39, 0x00);
	write_register(nTuner,50, 0xC8);
	write_register(nTuner,51, 0xA8);
	write_register(nTuner,54, 0xAF);
	write_register(nTuner,55, 0x72);
	write_register(nTuner,85, 0xBF);
	write_register(nTuner,86, 0x3C);
	write_register(nTuner,87, 0xF9);
	write_register(nTuner,88, 0xFF);
	write_register(nTuner,118, 0x00);
	write_register(nTuner,119, 0x18);
	write_register(nTuner,120, 0xC0);
	write_register(nTuner,121, 0x00);
	write_register(nTuner,130, 0x35);
	write_register(nTuner,131, 0x00);
	write_register(nTuner,133, 0x11);
	write_register(nTuner,134, 0x3F);
#endif

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. AM_NA_1p1
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_AM_NA_1p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x12);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 7, 0x5F);
	write_register(nTuner, 8, 0x23);
	write_register(nTuner, 9, 0xD7);
	write_register(nTuner, 10, 0x65);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0xA8);
	write_register(nTuner, 16, 0x51);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xB3);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 30, 0x02);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x18);
	write_register(nTuner, 120, 0xC0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. AM_NA_1p2
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_AM_NA_1p2(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x12);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 7, 0x5F);
	write_register(nTuner, 8, 0x23);
	write_register(nTuner, 9, 0xD7);
	write_register(nTuner, 10, 0x7D);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0xA8);
	write_register(nTuner, 16, 0x51);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xB3);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 30, 0x06);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x18);
	write_register(nTuner, 120, 0xC0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. AM_NA_2p0
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_AM_NA_2p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x12);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x23);
	write_register(nTuner, 9, 0xD7);
	write_register(nTuner, 10, 0x55);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0xA8);
	write_register(nTuner, 16, 0x51);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xB3);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x2C);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x6B);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. AM_NA_3p0
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_AM_NA_3p0(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x12);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x2B);
	write_register(nTuner, 9, 0x53);
	write_register(nTuner, 10, 0x46);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0xA8);
	write_register(nTuner, 16, 0x51);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xB3);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 30, 0x00);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x12);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. AM_NA_3p1
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_AM_NA_3p1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x12);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x33);
	write_register(nTuner, 9, 0x33);
	write_register(nTuner, 10, 0x46);
	write_register(nTuner, 11, 0x45);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0xA8);
	write_register(nTuner, 16, 0x51);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xB3);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 30, 0x00);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 54, 0xF7);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x12);
	write_register(nTuner, 120, 0xD0);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0x00);
	write_register(nTuner, 130, 0x00);
	write_register(nTuner, 133, 0x00);
	write_register(nTuner, 134, 0x20);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #1
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 1, Ch_Coeff_CH_AM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_AM());


}


// This is the minimal set of registers that need to change when
// switching between any two North American receive modes. MWAM_NA_1
// These are only the registers that differ from the default set:
// Load_FM_NA_1p0_FULL. By sending only the registers that change between
// One North Ameican modes and another, we send fewer registers and can change modes faster.
// The subset is determined by examining all registers that change from the default set
// (Load_FM_NA_1p0_FULL) and determining all the registers that change per any new North American receive mode.
void Load_MWAM_NA_1(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_register(nTuner, 1, 0x12);
	write_register(nTuner, 2, 0x00);
	write_register(nTuner, 3, 0x00);
	write_register(nTuner, 4, 0x00);
	write_register(nTuner, 5, 0xC0);
	write_register(nTuner, 7, 0x7F);
	write_register(nTuner, 8, 0x23);
	write_register(nTuner, 9, 0xD7);
	write_register(nTuner, 10, 0x7D);
	write_register(nTuner, 11, 0x4F);
	write_register(nTuner, 12, 0x08);
	write_register(nTuner, 14, 0x50);
	write_register(nTuner, 15, 0xA8);
	write_register(nTuner, 16, 0x51);
	write_register(nTuner, 17, 0xA1);
	write_register(nTuner, 18, 0x74);
	write_register(nTuner, 19, 0xA1);
	write_register(nTuner, 20, 0x74);
	write_register(nTuner, 22, 0x2F);
	write_register(nTuner, 23, 0x2F);
	write_register(nTuner, 24, 0x80);
	write_register(nTuner, 26, 0x99);
	write_register(nTuner, 27, 0xB3);
	write_register(nTuner, 28, 0x1A);
	write_register(nTuner, 30, 0x01);
	write_register(nTuner, 37, 0x00);
	write_register(nTuner, 38, 0x00);
	write_register(nTuner, 39, 0x00);
	write_register(nTuner, 50, 0xC8);
	write_register(nTuner, 54, 0xB7);
	write_register(nTuner, 55, 0x72);
	write_register(nTuner, 86, 0x3C);
	write_register(nTuner, 87, 0xF9);
	write_register(nTuner, 88, 0xB9);
	write_register(nTuner, 118, 0x00);
	write_register(nTuner, 119, 0x00);
	write_register(nTuner, 120, 0x00);
	write_register(nTuner, 121, 0x00);
	write_register(nTuner, 122, 0xA2);
	write_register(nTuner, 130, 0x11);
	write_register(nTuner, 133, 0x11);
	write_register(nTuner, 134, 0x3F);

	//Define the NCO's Dec_Ratio_beforeNCO which is used in tuning the NCO, different for each receive mode.
	Decim_Ratio_beforeNCO = 1;

	//Load the Channel Filter Coefficients into the IC's Channel Filter Bank #3
	Load_Filter_Coefficients_1bank(nTuner, CH_MSEL, 3, Ch_Coeff_CH_MWAM());


	//Load the AA Filter Coefficients into the IC's Channel Filter Bank
	Load_Filter_Coefficients_1bank(nTuner, AA_MSEL, 0, AA_Coeff_AA_MWAM());


}



// Define a Channel Filter Coefficient set named: CH_FMEU
// Channel Filter Coefficients are dependent on the receive_mode you choose.
long *Ch_Coeff_CH_FMEU()
{
	//Create and fill an array of coefficients  (24 coefficients per bank, this is one bank):
	static long ch_coeff[24];

//	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	ch_coeff[0] = 0x0;
	ch_coeff[1] = 0xffff;
	ch_coeff[2] = 0x1;
	ch_coeff[3] = 0x2;
	ch_coeff[4] = 0xfffa;
	ch_coeff[5] = 0xffff;
	ch_coeff[6] = 0x15;
	ch_coeff[7] = 0xffec;
	ch_coeff[8] = 0xffde;
	ch_coeff[9] = 0x54;
	ch_coeff[10] = 0xfff9;
	ch_coeff[11] = 0xff52;
	ch_coeff[12] = 0xb8;
	ch_coeff[13] = 0xa2;
	ch_coeff[14] = 0xfe0a;
	ch_coeff[15] = 0xaf;
	ch_coeff[16] = 0x2e3;
	ch_coeff[17] = 0xfc14;
	ch_coeff[18] = 0xfe89;
	ch_coeff[19] = 0x89d;
	ch_coeff[20] = 0xfa2e;
	ch_coeff[21] = 0xf30f;
	ch_coeff[22] = 0x25be;
	ch_coeff[23] = 0x4eb6;

	return ch_coeff;
}


// Define a Channel Filter Coefficient set named: CH_AM
// Channel Filter Coefficients are dependent on the receive_mode you choose.
long *Ch_Coeff_CH_AM()
{
	//Create and fill an array of coefficients  (24 coefficients per bank, this is one bank):
	static long ch_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	ch_coeff[0] = 0x0;
	ch_coeff[1] = 0x0;
	ch_coeff[2] = 0xffff;
	ch_coeff[3] = 0xfffc;
	ch_coeff[4] = 0xfff6;
	ch_coeff[5] = 0xffe9;
	ch_coeff[6] = 0xffd2;
	ch_coeff[7] = 0xffb1;
	ch_coeff[8] = 0xff84;
	ch_coeff[9] = 0xff50;
	ch_coeff[10] = 0xff1e;
	ch_coeff[11] = 0xff00;
	ch_coeff[12] = 0xff0b;
	ch_coeff[13] = 0xff59;
	ch_coeff[14] = 0x2;
	ch_coeff[15] = 0x117;
	ch_coeff[16] = 0x29e;
	ch_coeff[17] = 0x48a;
	ch_coeff[18] = 0x6bc;
	ch_coeff[19] = 0x903;
	ch_coeff[20] = 0xb25;
	ch_coeff[21] = 0xce4;
	ch_coeff[22] = 0xe09;
	ch_coeff[23] = 0xe6f;

	return ch_coeff;
}


// Define a Channel Filter Coefficient set named: CH_MWAM
// Channel Filter Coefficients are dependent on the receive_mode you choose.
long *Ch_Coeff_CH_MWAM()
{
	//Create and fill an array of coefficients  (24 coefficients per bank, this is one bank):
	static long ch_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	ch_coeff[0] = 0xfffe;
	ch_coeff[1] = 0xffff;
	ch_coeff[2] = 0x9;
	ch_coeff[3] = 0xfff5;
	ch_coeff[4] = 0xfff9;
	ch_coeff[5] = 0x26;
	ch_coeff[6] = 0xffd5;
	ch_coeff[7] = 0xfff1;
	ch_coeff[8] = 0x6f;
	ch_coeff[9] = 0xff76;
	ch_coeff[10] = 0x0;
	ch_coeff[11] = 0xf6;
	ch_coeff[12] = 0xfe98;
	ch_coeff[13] = 0x6c;
	ch_coeff[14] = 0x1b8;
	ch_coeff[15] = 0xfcd3;
	ch_coeff[16] = 0x1ce;
	ch_coeff[17] = 0x290;
	ch_coeff[18] = 0xf935;
	ch_coeff[19] = 0x5e6;
	ch_coeff[20] = 0x33d;
	ch_coeff[21] = 0xed68;
	ch_coeff[22] = 0x212d;
	ch_coeff[23] = 0x58d5;

	return ch_coeff;
}


// Define a Channel Filter Coefficient set named: CH_DAB1
// Channel Filter Coefficients are dependent on the receive_mode you choose.
long *Ch_Coeff_CH_DAB1()
{
	//Create and fill an array of coefficients  (24 coefficients per bank, this is one bank):
	static long ch_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	ch_coeff[0] = 0x1c;
	ch_coeff[1] = 0x7;
	ch_coeff[2] = 0xffcd;
	ch_coeff[3] = 0x56;
	ch_coeff[4] = 0xffa4;
	ch_coeff[5] = 0x33;
	ch_coeff[6] = 0x27;
	ch_coeff[7] = 0xff61;
	ch_coeff[8] = 0x10e;
	ch_coeff[9] = 0xfec0;
	ch_coeff[10] = 0x106;
	ch_coeff[11] = 0xffb8;
	ch_coeff[12] = 0xff1c;
	ch_coeff[13] = 0x23c;
	ch_coeff[14] = 0xfcb2;
	ch_coeff[15] = 0x39b;
	ch_coeff[16] = 0xfd4e;
	ch_coeff[17] = 0x55;
	ch_coeff[18] = 0x36a;
	ch_coeff[19] = 0xf7de;
	ch_coeff[20] = 0xd21;
	ch_coeff[21] = 0xee72;
	ch_coeff[22] = 0x1499;
	ch_coeff[23] = 0x6a51;

	return ch_coeff;
}


// Define a Channel Filter Coefficient set named: CH_DAB2
// Channel Filter Coefficients are dependent on the receive_mode you choose.
long *Ch_Coeff_CH_DAB2()
{
	//Create and fill an array of coefficients  (24 coefficients per bank, this is one bank):
	static long ch_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	ch_coeff[0] = 0x45;
	ch_coeff[1] = 0xffff;
	ch_coeff[2] = 0xffab;
	ch_coeff[3] = 0x76;
	ch_coeff[4] = 0xffd0;
	ch_coeff[5] = 0xff9a;
	ch_coeff[6] = 0xe3;
	ch_coeff[7] = 0xff34;
	ch_coeff[8] = 0xfff9;
	ch_coeff[9] = 0x12b;
	ch_coeff[10] = 0xfe37;
	ch_coeff[11] = 0x12a;
	ch_coeff[12] = 0xa7;
	ch_coeff[13] = 0xfd5c;
	ch_coeff[14] = 0x345;
	ch_coeff[15] = 0xfe81;
	ch_coeff[16] = 0xfdc2;
	ch_coeff[17] = 0x5d9;
	ch_coeff[18] = 0xf99d;
	ch_coeff[19] = 0x1bb;
	ch_coeff[20] = 0x802;
	ch_coeff[21] = 0xebf5;
	ch_coeff[22] = 0x1df8;
	ch_coeff[23] = 0x5e30;

	return ch_coeff;
}


// Define a Channel Filter Coefficient set named: CH_FMNA
// Channel Filter Coefficients are dependent on the receive_mode you choose.
long *Ch_Coeff_CH_FMNA()
{
	//Create and fill an array of coefficients  (24 coefficients per bank, this is one bank):
	static long ch_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	ch_coeff[0] = 0x1;
	ch_coeff[1] = 0x3;
	ch_coeff[2] = 0xfffe;
	ch_coeff[3] = 0xfff4;
	ch_coeff[4] = 0x0;
	ch_coeff[5] = 0x1f;
	ch_coeff[6] = 0xc;
	ch_coeff[7] = 0xffbc;
	ch_coeff[8] = 0xffd3;
	ch_coeff[9] = 0x7d;
	ch_coeff[10] = 0x75;
	ch_coeff[11] = 0xff33;
	ch_coeff[12] = 0xff01;
	ch_coeff[13] = 0x131;
	ch_coeff[14] = 0x1ef;
	ch_coeff[15] = 0xfe60;
	ch_coeff[16] = 0xfc7a;
	ch_coeff[17] = 0x20e;
	ch_coeff[18] = 0x656;
	ch_coeff[19] = 0xfd94;
	ch_coeff[20] = 0xf395;
	ch_coeff[21] = 0x2ab;
	ch_coeff[22] = 0x2857;
	ch_coeff[23] = 0x3d3f;

	return ch_coeff;
}


// Define a Channel Filter Coefficient set named: CH_FMHD
// Channel Filter Coefficients are dependent on the receive_mode you choose.
long *Ch_Coeff_CH_FMHD()
{
	//Create and fill an array of coefficients  (24 coefficients per bank, this is one bank):
	static long ch_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	ch_coeff[0] = 0x0;
	ch_coeff[1] = 0xfffe;
	ch_coeff[2] = 0x5;
	ch_coeff[3] = 0xfff8;
	ch_coeff[4] = 0x5;
	ch_coeff[5] = 0x8;
	ch_coeff[6] = 0xffdd;
	ch_coeff[7] = 0x3f;
	ch_coeff[8] = 0xffbd;
	ch_coeff[9] = 0x12;
	ch_coeff[10] = 0x5e;
	ch_coeff[11] = 0xff15;
	ch_coeff[12] = 0x13e;
	ch_coeff[13] = 0xff14;
	ch_coeff[14] = 0xffb7;
	ch_coeff[15] = 0x228;
	ch_coeff[16] = 0xfc24;
	ch_coeff[17] = 0x425;
	ch_coeff[18] = 0xfe29;
	ch_coeff[19] = 0xfc7f;
	ch_coeff[20] = 0xb2c;
	ch_coeff[21] = 0xecaf;
	ch_coeff[22] = 0x198f;
	ch_coeff[23] = 0x641a;

	return ch_coeff;
}


// Define a EQ Filter Coefficient set named: EQ_FMEU1_RA02_m6dB
// EQ Filter Coefficients are dependent on the receive_mode you choose.
//long *EQ_Coeff_EQ_FMEU1_RA02_m6dB()
long *EQ_Coeff_EQ_FMEU1()			/* 20160407 */
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long  eq_coeff[24];

//	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	eq_coeff[0] = 0x40;
	eq_coeff[1] = 0xffc6;
	eq_coeff[2] = 0xfffa;
	eq_coeff[3] = 0x2c;
	eq_coeff[4] = 0xd;
	eq_coeff[5] = 0xff90;
	eq_coeff[6] = 0x37;
	eq_coeff[7] = 0x6e;
	eq_coeff[8] = 0xffc0;
	eq_coeff[9] = 0xff5b;
	eq_coeff[10] = 0x6a;
	eq_coeff[11] = 0xf0;
	eq_coeff[12] = 0xff57;
	eq_coeff[13] = 0xfe94;
	eq_coeff[14] = 0x112;
	eq_coeff[15] = 0x252;
	eq_coeff[16] = 0xfe0c;
	eq_coeff[17] = 0xfc6a;
	eq_coeff[18] = 0x385;
	eq_coeff[19] = 0x553;
	eq_coeff[20] = 0xfa49;
	eq_coeff[21] = 0xf789;
	eq_coeff[22] = 0xb91;
	eq_coeff[23] = 0x1a10;

	return eq_coeff;
}


// Define a EQ Filter Coefficient set named: EQ_FMHD4_RA02
// EQ Filter Coefficients are dependent on the receive_mode you choose.
//long *EQ_Coeff_EQ_FMHD4_RA02()
long *EQ_Coeff_EQ_FMHD4()			/* 20160407 */
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long  eq_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	eq_coeff[0] = 0x47;
	eq_coeff[1] = 0xd1;
	eq_coeff[2] = 0xff0f;
	eq_coeff[3] = 0xff1e;
	eq_coeff[4] = 0xbd;
	eq_coeff[5] = 0x158;
	eq_coeff[6] = 0xff3f;
	eq_coeff[7] = 0xfe2d;
	eq_coeff[8] = 0x89;
	eq_coeff[9] = 0x251;
	eq_coeff[10] = 0x33;
	eq_coeff[11] = 0xfd22;
	eq_coeff[12] = 0xfeda;
	eq_coeff[13] = 0x370;
	eq_coeff[14] = 0x2c8;
	eq_coeff[15] = 0xfc9f;
	eq_coeff[16] = 0xfb1a;
	eq_coeff[17] = 0x2f5;
	eq_coeff[18] = 0x7db;
	eq_coeff[19] = 0xff31;
	eq_coeff[20] = 0xf388;
	eq_coeff[21] = 0xfbda;
	eq_coeff[22] = 0x18ac;
	eq_coeff[23] = 0x2912;

	return eq_coeff;
}


// Define a EQ Filter Coefficient set named: EQ_FMHD5_RA02
// EQ Filter Coefficients are dependent on the receive_mode you choose.
long *EQ_Coeff_EQ_FMHD5_RA02()
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long  eq_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	eq_coeff[0] = 0xffbd;
	eq_coeff[1] = 0xff41;
	eq_coeff[2] = 0x1a5;
	eq_coeff[3] = 0xff99;
	eq_coeff[4] = 0xfe68;
	eq_coeff[5] = 0x1a3;
	eq_coeff[6] = 0xde;
	eq_coeff[7] = 0xfd8b;
	eq_coeff[8] = 0x45;
	eq_coeff[9] = 0x2ff;
	eq_coeff[10] = 0xfdb3;
	eq_coeff[11] = 0xfdbd;
	eq_coeff[12] = 0x432;
	eq_coeff[13] = 0x91;
	eq_coeff[14] = 0xfa2c;
	eq_coeff[15] = 0x2ff;
	eq_coeff[16] = 0x672;
	eq_coeff[17] = 0xf7b0;
	eq_coeff[18] = 0xfcdf;
	eq_coeff[19] = 0xe5a;
	eq_coeff[20] = 0xfc1e;
	eq_coeff[21] = 0xea43;
	eq_coeff[22] = 0x1418;
	eq_coeff[23] = 0x399c;

	return eq_coeff;
}


// Define a EQ Filter Coefficient set named: EQ_FMEU2
// EQ Filter Coefficients are dependent on the receive_mode you choose.
long *EQ_Coeff_EQ_FMEU2()
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long  eq_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	eq_coeff[0] = 0x14;
	eq_coeff[1] = 0x3e;
	eq_coeff[2] = 0xff8a;
	eq_coeff[3] = 0xfffc;
	eq_coeff[4] = 0x4a;
	eq_coeff[5] = 0x6c;
	eq_coeff[6] = 0xffa5;
	eq_coeff[7] = 0xff51;
	eq_coeff[8] = 0x43;
	eq_coeff[9] = 0xe6;
	eq_coeff[10] = 0x3e;
	eq_coeff[11] = 0xfecc;
	eq_coeff[12] = 0xfef9;
	eq_coeff[13] = 0x17a;
	eq_coeff[14] = 0x215;
	eq_coeff[15] = 0xfeab;
	eq_coeff[16] = 0xfc58;
	eq_coeff[17] = 0x94;
	eq_coeff[18] = 0x5ce;
	eq_coeff[19] = 0x144;
	eq_coeff[20] = 0xf814;
	eq_coeff[21] = 0xfb3a;
	eq_coeff[22] = 0xc52;
	eq_coeff[23] = 0x173c;

	return eq_coeff;
}


// Define a EQ Filter Coefficient set named: EQ_FMNA1
// EQ Filter Coefficients are dependent on the receive_mode you choose.
long *EQ_Coeff_EQ_FMNA1()
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long  eq_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	eq_coeff[0] = 0xffd0;
	eq_coeff[1] = 0x82;
	eq_coeff[2] = 0xffee;
	eq_coeff[3] = 0xff93;
	eq_coeff[4] = 0xffbd;
	eq_coeff[5] = 0x14;
	eq_coeff[6] = 0x102;
	eq_coeff[7] = 0x39;
	eq_coeff[8] = 0xff01;
	eq_coeff[9] = 0xfec6;
	eq_coeff[10] = 0x50;
	eq_coeff[11] = 0x1ef;
	eq_coeff[12] = 0x139;
	eq_coeff[13] = 0xfea2;
	eq_coeff[14] = 0xfd05;
	eq_coeff[15] = 0xff24;
	eq_coeff[16] = 0x2f8;
	eq_coeff[17] = 0x404;
	eq_coeff[18] = 0xffe1;
	eq_coeff[19] = 0xfb45;
	eq_coeff[20] = 0xfaee;
	eq_coeff[21] = 0x1da;
	eq_coeff[22] = 0xbca;
	eq_coeff[23] = 0x108f;

	return eq_coeff;
}


// Define a EQ Filter Coefficient set named: EQ_FMNA2
// EQ Filter Coefficients are dependent on the receive_mode you choose.
long *EQ_Coeff_EQ_FMNA2()
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long  eq_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	eq_coeff[0] = 0xffca;
	eq_coeff[1] = 0xcc;
	eq_coeff[2] = 0xff5b;
	eq_coeff[3] = 0xff77;
	eq_coeff[4] = 0x8c;
	eq_coeff[5] = 0x9b;
	eq_coeff[6] = 0xffd2;
	eq_coeff[7] = 0xfed0;
	eq_coeff[8] = 0xc;
	eq_coeff[9] = 0x198;
	eq_coeff[10] = 0x51;
	eq_coeff[11] = 0xfdfc;
	eq_coeff[12] = 0xff1d;
	eq_coeff[13] = 0x20d;
	eq_coeff[14] = 0x205;
	eq_coeff[15] = 0xfe07;
	eq_coeff[16] = 0xfc5b;
	eq_coeff[17] = 0x134;
	eq_coeff[18] = 0x5d1;
	eq_coeff[19] = 0x65;
	eq_coeff[20] = 0xf7ff;
	eq_coeff[21] = 0xfc14;
	eq_coeff[22] = 0xca5;
	eq_coeff[23] = 0x17c9;

	return eq_coeff;
}


// Define a EQ Filter Coefficient set named: EQ_FMNA30
// EQ Filter Coefficients are dependent on the receive_mode you choose.
long *EQ_Coeff_EQ_FMNA30()
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long  eq_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	eq_coeff[0] = 0xffd0;
	eq_coeff[1] = 0x82;
	eq_coeff[2] = 0xffee;
	eq_coeff[3] = 0xff93;
	eq_coeff[4] = 0xffbd;
	eq_coeff[5] = 0x14;
	eq_coeff[6] = 0x102;
	eq_coeff[7] = 0x39;
	eq_coeff[8] = 0xff01;
	eq_coeff[9] = 0xfec6;
	eq_coeff[10] = 0x50;
	eq_coeff[11] = 0x1ef;
	eq_coeff[12] = 0x139;
	eq_coeff[13] = 0xfea2;
	eq_coeff[14] = 0xfd05;
	eq_coeff[15] = 0xff24;
	eq_coeff[16] = 0x2f8;
	eq_coeff[17] = 0x404;
	eq_coeff[18] = 0xffe1;
	eq_coeff[19] = 0xfb45;
	eq_coeff[20] = 0xfaee;
	eq_coeff[21] = 0x1da;
	eq_coeff[22] = 0xbca;
	eq_coeff[23] = 0x108f;

	return eq_coeff;
}


// Define a EQ Filter Coefficient set named: EQ_FMNA31
// EQ Filter Coefficients are dependent on the receive_mode you choose.
long *EQ_Coeff_EQ_FMNA31()
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long  eq_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	eq_coeff[0] = 0xffd0;
	eq_coeff[1] = 0x82;
	eq_coeff[2] = 0xffee;
	eq_coeff[3] = 0xff93;
	eq_coeff[4] = 0xffbd;
	eq_coeff[5] = 0x14;
	eq_coeff[6] = 0x102;
	eq_coeff[7] = 0x39;
	eq_coeff[8] = 0xff01;
	eq_coeff[9] = 0xfec6;
	eq_coeff[10] = 0x50;
	eq_coeff[11] = 0x1ef;
	eq_coeff[12] = 0x139;
	eq_coeff[13] = 0xfea2;
	eq_coeff[14] = 0xfd05;
	eq_coeff[15] = 0xff24;
	eq_coeff[16] = 0x2f8;
	eq_coeff[17] = 0x404;
	eq_coeff[18] = 0xffe1;
	eq_coeff[19] = 0xfb45;
	eq_coeff[20] = 0xfaee;
	eq_coeff[21] = 0x1da;
	eq_coeff[22] = 0xbca;
	eq_coeff[23] = 0x108f;

	return eq_coeff;
}


// Define a EQ Filter Coefficient set named: EQ_FMHD1
// EQ Filter Coefficients are dependent on the receive_mode you choose.
long *EQ_Coeff_EQ_FMHD1()
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long  eq_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	eq_coeff[0] = 0xffc7;
	eq_coeff[1] = 0x90;
	eq_coeff[2] = 0xff62;
	eq_coeff[3] = 0xc;
	eq_coeff[4] = 0xda;
	eq_coeff[5] = 0xfeda;
	eq_coeff[6] = 0x44;
	eq_coeff[7] = 0x119;
	eq_coeff[8] = 0xfea4;
	eq_coeff[9] = 0xffd3;
	eq_coeff[10] = 0x20b;
	eq_coeff[11] = 0xfe2c;
	eq_coeff[12] = 0xff18;
	eq_coeff[13] = 0x35c;
	eq_coeff[14] = 0xfde0;
	eq_coeff[15] = 0xfdad;
	eq_coeff[16] = 0x501;
	eq_coeff[17] = 0xfea5;
	eq_coeff[18] = 0xfa28;
	eq_coeff[19] = 0x7bb;
	eq_coeff[20] = 0x1ee;
	eq_coeff[21] = 0xf1b7;
	eq_coeff[22] = 0x89d;
	eq_coeff[23] = 0x2166;

	return eq_coeff;
}


// Define a EQ Filter Coefficient set named: EQ_FMHD30
// EQ Filter Coefficients are dependent on the receive_mode you choose.
long *EQ_Coeff_EQ_FMHD30()
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long  eq_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	eq_coeff[0] = 0xffc7;
	eq_coeff[1] = 0x90;
	eq_coeff[2] = 0xff62;
	eq_coeff[3] = 0xc;
	eq_coeff[4] = 0xda;
	eq_coeff[5] = 0xfeda;
	eq_coeff[6] = 0x44;
	eq_coeff[7] = 0x119;
	eq_coeff[8] = 0xfea4;
	eq_coeff[9] = 0xffd3;
	eq_coeff[10] = 0x20b;
	eq_coeff[11] = 0xfe2c;
	eq_coeff[12] = 0xff18;
	eq_coeff[13] = 0x35c;
	eq_coeff[14] = 0xfde0;
	eq_coeff[15] = 0xfdad;
	eq_coeff[16] = 0x501;
	eq_coeff[17] = 0xfea5;
	eq_coeff[18] = 0xfa28;
	eq_coeff[19] = 0x7bb;
	eq_coeff[20] = 0x1ee;
	eq_coeff[21] = 0xf1b7;
	eq_coeff[22] = 0x89d;
	eq_coeff[23] = 0x2166;

	return eq_coeff;
}


// Define a EQ Filter Coefficient set named: EQ_FMHD31
// EQ Filter Coefficients are dependent on the receive_mode you choose.
long *EQ_Coeff_EQ_FMHD31()
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long  eq_coeff[24];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	eq_coeff[0] = 0xffc7;
	eq_coeff[1] = 0x90;
	eq_coeff[2] = 0xff62;
	eq_coeff[3] = 0xc;
	eq_coeff[4] = 0xda;
	eq_coeff[5] = 0xfeda;
	eq_coeff[6] = 0x44;
	eq_coeff[7] = 0x119;
	eq_coeff[8] = 0xfea4;
	eq_coeff[9] = 0xffd3;
	eq_coeff[10] = 0x20b;
	eq_coeff[11] = 0xfe2c;
	eq_coeff[12] = 0xff18;
	eq_coeff[13] = 0x35c;
	eq_coeff[14] = 0xfde0;
	eq_coeff[15] = 0xfdad;
	eq_coeff[16] = 0x501;
	eq_coeff[17] = 0xfea5;
	eq_coeff[18] = 0xfa28;
	eq_coeff[19] = 0x7bb;
	eq_coeff[20] = 0x1ee;
	eq_coeff[21] = 0xf1b7;
	eq_coeff[22] = 0x89d;
	eq_coeff[23] = 0x2166;

	return eq_coeff;
}


// Define a AA Filter Coefficient set named: AA_FMHD
// AA Filter Coefficients are dependent on the receive_mode you choose.
long *AA_Coeff_AA_FMHD()
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long aa_coeff[12];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	aa_coeff[0] = 0x0;
	aa_coeff[1] = 0x0;
	aa_coeff[2] = 0x1;
	aa_coeff[3] = 0x1;
	aa_coeff[4] = 0x1;
	aa_coeff[5] = 0xfe;
	aa_coeff[6] = 0xfc;
	aa_coeff[7] = 0xfb;
	aa_coeff[8] = 0x1;
	aa_coeff[9] = 0xc;
	aa_coeff[10] = 0x19;
	aa_coeff[11] = 0x23;

	return aa_coeff;
}


// Define a AA Filter Coefficient set named: AA_AM
// AA Filter Coefficients are dependent on the receive_mode you choose.
long *AA_Coeff_AA_AM()
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long aa_coeff[12];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	aa_coeff[0] = 0x0;
	aa_coeff[1] = 0x0;
	aa_coeff[2] = 0x1;
	aa_coeff[3] = 0x1;
	aa_coeff[4] = 0x1;
	aa_coeff[5] = 0xfe;
	aa_coeff[6] = 0xfc;
	aa_coeff[7] = 0xfb;
	aa_coeff[8] = 0x1;
	aa_coeff[9] = 0xc;
	aa_coeff[10] = 0x19;
	aa_coeff[11] = 0x23;

	return aa_coeff;
}


// Define a AA Filter Coefficient set named: AA_MWAM
// AA Filter Coefficients are dependent on the receive_mode you choose.
long *AA_Coeff_AA_MWAM()
{
	//Create and fill an array of coefficients (24 coefficients per bank, this is one bank):
	static long aa_coeff[12];

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	aa_coeff[0] = 0x0;
	aa_coeff[1] = 0x0;
	aa_coeff[2] = 0x1;
	aa_coeff[3] = 0x1;
	aa_coeff[4] = 0x1;
	aa_coeff[5] = 0xfe;
	aa_coeff[6] = 0xfc;
	aa_coeff[7] = 0xfb;
	aa_coeff[8] = 0x1;
	aa_coeff[9] = 0xc;
	aa_coeff[10] = 0x19;
	aa_coeff[11] = 0x23;

	return aa_coeff;
}

