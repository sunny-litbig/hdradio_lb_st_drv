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
 * Version: 3.0
 * Release Date: 2/29/2016
 * Maxim Integrated Products
 * Author: Paul Nichol
 *
 * This is the top level code for the MAX2175 driver.
*/

/* ###########################
 *    Using this driver:
 * ############################
 *
 * In the source file register_functions.c, in the function: write_register()
 * replace the call to the serial driver:
 *  "return write_I2C(I2CADDRESS, 2, data)"
 * with your function to write to your systems I-squared-C serial bus.
 * Next, in the same file, find the function read_register(),
 * and replace the call to the serial driver:
 * "read_I2C(I2CADDRESS, regindex, 1, newregarray)"
 * with the call to your systems I-squared-C serial bus.
 *
 * When initializing your system, call: init(Region area, double XtalFrequency)
 * Region is North America or Europe, and XtalFrequency is the reference you will use
 * for the MAX2175 in MHz.
 * If you are designing for both regions, be sure and call init() with the new region.
 *
 * Call:  io_select() to select the appropriate digital output format for your system.
 *
 * Call Receive_mode() to choose the appropriate mode.   AM, FM, DAB, etc.
 * The Receive modes are listed in Table_Sets.cs under: public enum Receive_Modes.
 *
 * If using the FM band, call:  LO_HSLS() to position the LO Above or Below the
 * desired.   When changing LO_HSLS(), do this before changing RF_Frequency and after
 * changing receive_mode.
 *
 *
 * Call Goto_RF_Frequency(frequency in MHz) with the desired frequency.
 *
 * Within the same band, you can tune to different stations by calling Goto_RF_Frequency().
 *
 * To change to a different band i.e. FM to AM:
 *
 * Call Receive_Mode(newmode).
 *
 * Call LO_HSLS() if necessary.
 *
 * Call Goto_RF_Frequency()
 *
 * And, Please, Your constructive feedback would be greatly appreciated!
 * Paul.Nichol@maximintegrated.com
 */

#include <stdio.h>
#include <stdlib.h>   /* for wait function */
#include <math.h>     /* for frequency calculations */
#include <string.h>
#include <unistd.h>

#include "tcradio_types.h"
#include "tcradio_drv.h"
#include "max2175_hal.h"

/*----------------------------------------------------------------------------------*/

/* Band Constants */
typedef enum
{
    __AM__ = 0,
    __FM__ = 1,
    __VHF__ = 2,
    __LBAND__ = 3
} eBand;


double xtal_frequency = 0;  /* Crystal Frequency used. */
int am_hiz = true;  /*  Note: Set this true if using HIGH-Z input for AM, otherwise, it defaults to 50 ohm. 2-26-16 */


//Set the default FSM Mode:
efsm_mode Current_FSM_Mode = MODE_BUFFER_PLUS_JUMP_FAST_TUNE;

void *max2175_memcpy(void *pvDst, void *pvSrc, uint16 uiLength)
{
	uint16 i;
	uint8 *pW;
	uint8 *pR;

	pW = (uint8 *)pvDst;
	pR = (uint8 *)pvSrc;

	if(uiLength == 0)
		return (void *)pW;

	for(i = 0; i < uiLength; i++)
	{
		*pW++ = *pR++;
	}
	return (void *)pW;
}

void *max2175_memset(void *pvdst, uint8 ubch, uint32 uilen)
{
	uint8 *pW;
	uint32 i;

	pW = (uint8 *)pvdst;

	if(uilen == 0)
		return (void *)pW;

	for(i=0; i<uilen; i++)
	{
		*pW++ = ubch;
	}
	return (void *)pW;
}

int MAX2175_get_rssi(int nTuner, unsigned char band)
{
	float rssi = 0;
	float pga_gainf = 0;
	float DigGainf = 0;
	int ret = 0;

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	if(band == eTUNER_DRV_FM_MODE){ // FM
		int pga_gain = read_bits(nTuner, 46,4,0);
		int fm_lna_gain = read_bit(nTuner, 47,0);
		int fm_attn = read_bits(nTuner, 47,3,1);
		int DigGain  =  read_bits(nTuner, 44,5,0);
		int RF_Gain = read_bits(nTuner, 47,3,0);

		ret = ((band<<20)|(RF_Gain<<16)|(pga_gain<<8)|DigGain);

	}else if(band == eTUNER_DRV_AM_MODE){ // AM
		int pga_gain = read_bits(nTuner, 46,4,0);
		int am_lna_gain = read_bit(nTuner, 48,0);
		int am_attn = read_bits(nTuner, 48,3,1);
		int DigGain  =  read_bits(nTuner, 44,5,0);
		int RF_Gain = read_bits(nTuner, 48,3,0);

		ret = ((band<<20)|(RF_Gain<<16)|(pga_gain<<8)|(DigGain));

	}

	return ret;
}

/*
	Initialize the MAX2175 into a known operational state.
	This method must aloways be called at system power up and before the MAX2175 is
	ready to tune and output data.
	If the MAX2175 loses power, inti() must be called again once power is restored.
	area: Operating region.  Europe or North America.</param>
	XtalFrequency: The desired Crystal Reference Frequency in MHz.
*/
// void init(Region area, double XtalFrequency)
void init(int nTuner, Region_t area, double XtalFrequency)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d][%f]\n", __func__, __LINE__, XtalFrequency);

	myRegion = area;
	xtal_frequency = XtalFrequency;

	/*
	There are two default register sets one for Europe and one for North America.
	Call the appropriate set first to initialize the IC correctly.
	*/

	/*
		rewrite R0x01-R0x83 registers a second time.  This ensures that all internal regulators have ramped up and settled.
	*/
	if (area == EUROPE)
	{
		receive_mode(nTuner, FM_EU_1p0_FULL);
		mswait(5);
		receive_mode(nTuner, FM_EU_1p0_FULL);
	}
	else
	{
		receive_mode(nTuner, FM_NA_1p0_FULL);
		mswait(5);
		receive_mode(nTuner, FM_NA_1p0_FULL);
	}

	if(cumulativeError > MAX_ERROR)
		return;

	/* Reset ADC and its registers */
	write_bit(nTuner, REG_RESET2, rstb_adc_all, 0);
	mswait(1);
	write_bit(nTuner, REG_RESET2, rstb_adc_all, 1);

	/* Load the values into the ADC Registers (registers 159-255): */
	LoadADC_Registers(nTuner);

	/*
	Initialize the power management state machine.
	If you have a way to track errors, you can look at success.
	If the part is functioning correctly, success should be true.
	*/
	bool success = init_power_manager(nTuner);

	/*
	Copy the Factory programmed ROM correction factors into the appropriate registers.
	This function only needs to be called during this init() function.
	*/
	load_from_ROM(nTuner);

	/* Set this depending on your desired digital output interface. */
	io_select(nTuner, IO_I2S);

//	printf("MAX2175 CS2:BEFORE RESET CONTROL\n");
//	dump_all_reg_val();

//	dev_Reset_CtrlGpio();		/* add 20160412 */
//	GPIO_SLEEP(200);

//	printf("\n\nMAX2175 CS2:AFTER INIT FUNCTION\n");
//	dump_all_reg_val();


	/*
	  After you have called this init sequence, it is now time to:
	  1) Call: receive_mode() to set the setting correctly for the receive mode desired.
	  2) Call: HSLS to set the LO above or below the desired frequency (Applies to FM Mode).
	  3) Call: RF_frequency() to tune to the correct frequency.
	  4) Call: perform_channel_FSM_action() to put the state machine in the desired state.
	*/
}


/*
     Initializes the power manager state machine.  Must be done at start-up (init())

     Returns: True if state machine settled correctly.
 */
 unsigned char init_power_manager(int nTuner)
 {
//	printf("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	 write_bit(nTuner, REG_RESET2, RSTB_PM, 0);
	 mswait(1);
	 write_bit(nTuner, REG_RESET2, RSTB_PM, 1);
	 return wait_for_pm_done(nTuner);
 }


/*
	Sets the mode the part operates in.  i.e. FM, AM, FM_HD, DAB etc.
	Sends out the minimal set of registers to change between modes.
	This function also copies ROM BBF values to the appropriate registers based on the mode you choose.
	You can remove the regions or bands you do no use from this function if you wish.
	Since FM_EU_1p0_FULL or FM_NA_1p0_FULL will always be called during init(), it must remain.
*/
void receive_mode(int nTuner, Receive_Modes rmode)
 {
//	 sendval(rmode);

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	 Current_Receive_Mode = rmode;

	 /*
		The full register set of 145 registers only needs to be loaded during init() or unless you are changing regions
		from North American receive modes to European receive modes, or from European receive modes to North American receive modes.
		The when changing receive modes within a region, you call the smaller register sets:
		See: "Beginning of reduced register sets" and any set below that in the list.
		This will reduce the time it takes to switch receive modes.
	 */
	 switch (rmode)
	 {
	 case FM_EU_1p0_FULL:   /* Note to save serial writes, only call this at init()  */
	 	 printf("MAX2175 CS2:#############################################\n");
         printf("MAX2175 CS2:###      FM_EU_1p0_FULL - mode / 36MHz    ###\n");
         printf("MAX2175 CS2:#############################################\n");
		 Load_FM_EU_1p0_FULL(nTuner);
		 break;
	 case FM_NA_1p0_FULL:   /* Note to save serial writes, only call this at init()  */
	 	 printf("MAX2175 CS2:#############################################\n");
         printf("MAX2175 CS2:###      FM_NA_1p0_FULL - mode / 40MHz    ###\n");
         printf("MAX2175 CS2:#############################################\n");
		 Load_FM_NA_1p0_FULL(nTuner);
		 break;
	 case FM_EU_1p0:        /* Beginning of reduced register sets: */
		 Load_FM_EU_1p0(nTuner);
		 break;
	 case FM_EU_1p1:
		 Load_FM_EU_1p1(nTuner);
		 break;
	 case FM_EU_1p2:
        printf("MAX2175 CS2:######################################\n");
        printf("MAX2175 CS2:###      FM_EU_1p2 mode / 36MHz    ###\n");
        printf("MAX2175 CS2:######################################\n");
		Load_FM_EU_1p2(nTuner);
		break;
	 case FM_HD_4p0:
		 Load_FM_HD_4p0(nTuner);
		 break;
	 case FM_EU_2p0:
		 Load_FM_EU_2p0(nTuner);
		 break;
	 case FM_EU_2p1:
		 Load_FM_EU_2p1(nTuner);
		 break;
	 case FM_EU_2p2:
		 Load_FM_EU_2p2(nTuner);
		 break;
	 case AM_EU_1p0:
		 Load_AM_EU_1p0(nTuner);
		 break;
	 case AM_EU_1p1:
		 Load_AM_EU_1p1(nTuner);
		 break;
	 case AM_EU_1p2:
         printf("MAX2175 CS2:#######################################\n");
         printf("MAX2175 CS2:###      AM_EU_1p2 mode / 40MHz     ###\n");
         printf("MAX2175 CS2:#######################################\n");
         Load_AM_EU_1p2(nTuner);
		 break;
	 case AM_EU_2p0:
		 Load_AM_EU_2p0(nTuner);
		 break;
	 case AM_EU_2p1:
		 Load_AM_EU_2p1(nTuner);
		 break;
	 case AM_EU_2p2:
		 Load_AM_EU_2p2(nTuner);
		 break;
	 case MWAM_EU_1:
		 Load_MWAM_EU_1(nTuner);
		 break;
	 case MWAM_EU_2:
		 Load_MWAM_EU_2(nTuner);
		 break;
	 case SWAM_0:
		 Load_SWAM_0(nTuner);
		 break;
	 case DAB_1_VHF:
		 Load_DAB_1_VHF(nTuner);
		 break;
	 case DAB_1_LB:
		 Load_DAB_1_LB(nTuner);
		 break;
	 case DAB_2:
		 Load_DAB_2(nTuner);
		 break;
	 case DAB_1p1:
		 Load_DAB_1p1(nTuner);
		 break;
	 case DAB_1p2:
		 Load_DAB_1p2(nTuner);
		 break;
	 case FM_NA_1p0:
		 Load_FM_NA_1p0(nTuner);
		 break;
	 case FM_NA_1p1:
		 Load_FM_NA_1p1(nTuner);
		 break;
	 case FM_NA_1p2:
		 Load_FM_NA_1p2(nTuner);
		 break;
	 case FM_NA_2p0:
		 Load_FM_NA_2p0(nTuner);
		 break;
	 case FM_NA_3p0:
		 Load_FM_NA_3p0(nTuner);
		 break;
	 case FM_NA_3p1:
		 Load_FM_NA_3p1(nTuner);
		 break;
	 case WX_2p0:
		 Load_WX_2p0(nTuner);
		 break;
	 case WX_1p0:
		 Load_WX_1p0(nTuner);
		 break;
	 case WX_1p1:
		 Load_WX_1p1(nTuner);
		 break;
	 case WX_1p2:
		 Load_WX_1p2(nTuner);
		 break;
	 case FM_HD_1p0:
         printf("MAX2175 CS2:######################################\n");
         printf("MAX2175 CS2:###      FM HD 1.0 mode / 40MHz    ###\n");
         printf("MAX2175 CS2:######################################\n");
		 Load_FM_HD_1p0(nTuner);
		 break;
	 case FM_HD_1p1:
		 Load_FM_HD_1p1(nTuner);
		 break;
	 case FM_HD_1p2:
		 Load_FM_HD_1p2(nTuner);
		 break;
	 case FM_HD_3p0:
		 Load_FM_HD_3p0(nTuner);
		 break;
	 case FM_HD_3p1:
		 Load_FM_HD_3p1(nTuner);
		 break;
	 case AM_NA_1p0:
         printf("MAX2175 CS2:#######################################\n");
         printf("MAX2175 CS2:###      AM_NA_1p0 mode / 40MHz     ###\n");
         printf("MAX2175 CS2:#######################################\n");
		 Load_AM_NA_1p0(nTuner);
		 break;
	 case AM_NA_1p1:
		 Load_AM_NA_1p1(nTuner);
		 break;
	 case AM_NA_1p2:
		 Load_AM_NA_1p2(nTuner);
		 break;
	 case AM_NA_2p0:
		 Load_AM_NA_2p0(nTuner);
		 break;
	 case AM_NA_3p0:
		 Load_AM_NA_3p0(nTuner);
		 break;
	 case AM_NA_3p1:
		 Load_AM_NA_3p1(nTuner);
		 break;
	 case MWAM_NA_1:
		 Load_MWAM_NA_1(nTuner);
		 break;

	 }

	if(cumulativeError > MAX_ERROR)
		return;

	 /*  If the flag is set to go into AM High Impedance Input, */
	 if (am_hiz)
	 {
		 write_bit(nTuner, REG_FM_RFGAIN, AM_HIZ_IN,1);
		 write_bit(nTuner, REG_DCOC_DACQ, DCOC_DAC_2XGAIN, 1);
		 write_bits(nTuner, REG_BIAS3, AMLNA_BIAS_MSB, AMLNA_BIAS_LSB, 2);
		 write_bits(nTuner, REG_TRIM_AM, AM_LNA_IIP2_TRIM_MSB, AM_LNA_IIP2_TRIM_LSB, 33);

	 }
//	write_bit(nTuner, REG_FM_RFGAIN, 4,1);	// for test
	 load_from_ROM(nTuner);

	 /*
	 For each mode FM/AM/DAB, there is a different BB Filter trim value saved in ROM.
	 When you switch to a different band, the appropriate filter trim from ROM value needs to be
	 loaded into the registers.
	 */
	 program_bb_filter(nTuner);
 }


/*
	Copy ROM Base Band Filter trim values into the appropriate registers for the current band.
	The bits BBF_BW[3:0] in the BBF_CTRL register must have the appropriate ROM value copied
	into them as you change modes.
	The ROM is read for the BBF_BW values for the three bands AM, FM and DAB during the init() method.
	The procedure below determines which band you are in based on the current register values and
	copies the appropriate ROM value into the BBF_BW[3:0] bits in the BBF_CTRL register.
*/
void program_bb_filter(int nTuner)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);


	/*
	The Band register bits 1 and 0 are the RF_BAND[1:0] bits.
	If these are set to 00b, you are in AM band.
	*/
	if (get_bits(nTuner, REG_BAND, 1, 0) == __AM__)
	{
		/* In an AM mode. Copy BBF_BW_AM value read at init from ROM into the BBF_BW Bits. */
		write_bits(nTuner, REG_BBF_CTRL, 3, 0, ROM_BBF_BW_AM);
	}
	else
	{
		/*
		Bits 5,4 of the BBF_CTRL register are the BBF_MODE[1:0] bits.  Use these to determine if you are in
		FM or DAB modes.   if the bits are 00b or 01b, you are in FM mode, otherwise you are in DAB mode.
		*/
		if (get_bits(nTuner, REG_BBF_CTRL, 5, 4) < 2)
		{
			/* In a FM mode. Copy BBF_BW_FM value read at init from ROM into the BBF_BW Bits. */
			write_bits(nTuner, REG_BBF_CTRL, 3, 0, ROM_BBF_BW_FM);

		}
		else
		{
			/* In a DAB mode. Copy BBF_BW_DAB value read at init from ROM into the BBF_BW Bits. */
			write_bits(nTuner, REG_BBF_CTRL, 3, 0, ROM_BBF_BW_DAB);

		}
	}
}


/*
	Load the Channel FIR Coefficient memory bank from a specified coefficient table.
	The Channel Filter Coefficient memory is made up of 4 banks 0-3 (or A-D) of 24 locations totalling 96.
	The EQ Filter Coefficient memory is also made up of 4 banks or 24 locations, but only bank 0 and 1 are used.
	Once they are loaded into memory, they can be switched by setting FIR_BANK[1:0] or EQFILT_BANK[1:0] to 0,1,2 or 3.
	The FIR_BANK and EQFILT_BANK value is set when the receive mode register table is loaded.
	The coefficient banks are split into named tables so you can create your own customized and table and load it if necessary.

	M_SEL: Selects which major grouping of coefficient tables to load (0=Channel or 1=EQ).</param>
	filter_bank: Which memory bank to load the 24 coefficients into.</param>
	Coef_Array: The array of 24 (or 12 for AA Filters) coefficeints.</param>

	Upated to RA02 writing methods 2-25-16 (On the RA02 the order of the registers has changed so that all registers related
	to FIR coefficients can be done in one sequential write.  The address is after the coefficients instead of before).

*/
void Load_Filter_Coefficients_1bank(int nTuner, unsigned char M_SEL, unsigned char filter_bank, long * Coef_Array)
{
	int upper_address = 0;
	unsigned char coef_addr = 0;
	int i;

	//MSEL is two bits long.
	#define FIR_COEFF_MSEL_MSB 5  /* Upper bit of MESL  */
	#define FIR_COEFF_MSEL_LSB 4  /* Lower bit of MSEL  */
	#define  FIR_COEFF_WR  7    /* Bit location, FIR Coefficient Write Bit. */

//	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	/*
	MSEL sets which bank of FIR filters registers to write into,
	0 = Channel Filter FIR Bank, 1 = EQ Filter FIR Bank.
	*/
	write_bits(nTuner, REG_FIR_COEFF0, FIR_COEFF_MSEL_MSB, FIR_COEFF_MSEL_LSB, M_SEL);


	//The AA Filters only have 12 addresses per bank and only 1 bank.
	//The EQ and FIR filters have 24 address per bank and 4 banks.
	if (M_SEL == 2)
	{
		upper_address = 11;  //Address range 0-11 (12 total)
		filter_bank = 0;     //Only 1 bank so addresses only go from 0-11.
	}
	else
	{
		upper_address = 23;  //Address range 0-23 (24 total)
	}

	/*
	The Filter banks are seperated into 4 banks with 24 filter values per bank.
	Address 0-23 is bank 0 (or A), 24-47 is bank 1 (or B),  48-71 is bank 2 (or C)
	and 72-95 is bank 3 (or D).  The exception to this is the AA filter.  It only has
	one bank of 12 values (address of 0-11).


	Loop through each of the 12/24 addresses, copying each coefficient into memory for 1 bank only specified by filter_bank.
	Filter Coefficients as indexed by filter_bank value.
	**The register address: i = 0-23 for filter_bank 0, 24-47 for filter_bank 1, 48-71 for filter_bank 2,
	and 72-95 for filter_bank 3.

	The FIR coefficients are 16 bits and are split into 2 registers to program a single memory location.
	The next 8 bits (FIR_COEFF[15:8]) go into register FIR_COEFF1[7:0]
	The lower 8 bits (FIR_COEFF[7:0]) go into register FIR_COEFF2[7,0]
	*/

	/* Set the write bit high.  Note: This bit is set only in the register array and is not written to the part until later.
	The actual writing happens when write_registers gets called. */
	set_bit(nTuner, REG_FIR_COEFF_ADDR, FIR_COEFF_WR, 1);

	/* Now write the remaining 23 elements 1-23: */
	for (i = 0; i <= upper_address; i++)
	{
		coef_addr = i + (filter_bank * 24);  /*  see note ** above for bank offsets. */


		set_bits(nTuner, REG_FIR_COEFF1, 7, 0, (unsigned char)((*(Coef_Array + i) & 0x00ff00) >> 8));   /* Copy upper 8 bits of coefficient into REG_FIR_COEFF1 */
		set_bits(nTuner, REG_FIR_COEFF2, 7, 0, (unsigned char)(*(Coef_Array + i) & 0x0000ff));        /* Copy lower 8 bits of coefficient into REG_FIR_COEFF2 */
		/*
		  Set the address of the FIR memory location to write into:
		  Bits 6-0 are the FIR_COEFF_ADDR[6:0] bits.
		  When the address is written, and the FIR_COEFF_WR bit is high, the coeffecients will be written to memory.
		*/
		set_bits(nTuner, REG_FIR_COEFF_ADDR, 6, 0, coef_addr);
		/*
		Now, write three registers.  The first two are the coefficent registers, the third is the coefficient address.
		Write them sequentially ending with the address.
		Bits 6-0 are the FIR_COEFF_ADDR[6:0] bits.
		Note: Writing to the register with the Write bit high, latches in the two coefficient registers into memory.
		*/
		write_registers(nTuner, 3, REG_FIR_COEFF1);
	}

	/* Disable write mode (set Write bit Low): */
	write_bit(nTuner, REG_FIR_COEFF_ADDR, FIR_COEFF_WR, 0);

	return;
}


/*
	Calculates the Comparison Frequency in MHz.
	The Comparison Frequency is the crystal frequency scaled by the Reference Divider going into the
	PLL phase-frequency detector.  This frequency is compared with the LO frequency scaled by the
	PLL_Divider and N Divider.  The comparison ultimately results in the tune voltage that adjusts the
	LO to match these two inputs into the Phase-Frequency detector.
	Since the R-Divider can only divide by 1 or 2, the comparison frequency is either the crystal frequency
	or the crystal frequency divided by 2.   Used in the LO frequency calculations.
*/
double f_comparison(int nTuner)
{
	/*
	   The Rdivider is mapped as follows:
	     00 = Decimate by 1
	     01 = Decimate by 2
	     10 = Reserved
	     11 = Reserved"
	   So we just examine the LSB (bit 4) of the RDIV register bits to determine the decimation factor.
	   if bit 4 = 0, divide by 1, if 1, divide by 2:
	*/
	double rdiv = 1 << get_bit(nTuner, REG_RDIV_FRAC2, 4);

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	return xtal_frequency / rdiv;
}


/*
	<summary>
    This method reads the ROM table programmed at factory test within the MAX2175.
    The values read are then transferred into the registers in the MAX2175 during init().
    This happens during the init() method.   The three BBF_BW values read are later used
    if the receive_mode is changed.
	</summary>
*/
void load_from_ROM(int nTuner)
{
	unsigned char rom_data = 0;

//	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	/* Read ROM table row 0: */
	rom_data = read_rom(nTuner, 0);

	/*
	The lower 4 bits in row 0 contain the BBF_BW_AM value.  The bit value is saved in the variable:
	ROM_BBF_BW_AM, which will be copied over to
	the BBF_CTRL register bits:  BBF_BW[3:0] if you change to an AM band.
	*/
	ROM_BBF_BW_AM = (rom_data & 0x0f);

	/*
	The upper 4 bits of rom row 0 contain the ADC_TS_TRIM bits.
	move these bits into register TRIM_RO_TS at location: ADC_TS_TRIM[3:0]
	*/
	write_bits(nTuner, REG_TRIM_RO_TS, 3, 0, (rom_data >> 4) & 0xf);


	/* Note: ROM row 1 is band dependant so it will be loaded while tuning. */
	rom_data = read_rom(nTuner, 1);

	/*
	The lower 4 bits in row 1 contain the BBF_BW_FM value.  The bit value is saved in the variable:
	ROM_BBF_BW_FM, which will be copied over to
	the BBF_CTRL register bits:  BBF_BW[3:0] if you change to an FM band.
	*/
	ROM_BBF_BW_FM = (rom_data & 0x0f);


	/*
	The upper 4 bits in row 1 contain the BBF_BW_DAB value.  The bit value is saved in the variable:
	ROM_BBF_BW_DAB, which will be copied over to
	the BBF_CTRL register bits:  BBF_BW[3:0] if you change to an DAB band.
	*/
	ROM_BBF_BW_DAB = (rom_data & 0xf0) >> 4;



	rom_data = read_rom(nTuner, 2);

	/* Write the BIAS_TRIM ROM value to the lower 4 bits of register REG_TRIM_BIAS  */
	write_bits(nTuner, REG_TRIM_BIAS, 4, 0, rom_data & 0x1f);


	/* Write the RO_TRIM ROM value to the lower bits <5:4> of register REG_TRIM_RO_TS  */
	write_bits(nTuner, REG_TRIM_RO_TS, 5, 4, rom_data >> 6);


	/*
	Note the AM_LNA_IIP2_TRIM has not been implemented at this time.  This code is to be updated and used at the point that it is.
	Read bit ranges: AM_LNA_IIP2_TRIM_50[3:0]	AM_LNA_IIP2_TRIM_HIZ[3:0] from row 3.
	Apply an equation and place into register TRIM_AM[5:0]
	rom_data = read_rom(3);
	some equation on rom_data goes here.  Yet to be determined.
	rom_data = rom_data / 2;   //dummy equation
	write_bits(REG_TRIM_AM, 5, 0, rom_data);
	*/
}


/*
	<summary>
	Reads one row from the ROM table programmed in the MAX2175.
	</summary>

	rowindex: The index into the row to be read.
	<returns>The 8 bit ROM contents for the specified row.</returns>

*/
unsigned char read_rom(int nTuner, unsigned char rowindex)
{
	/*
	Set the ROM address to read from.  Note: bit 4 is the ROM_WR bit.
	This bit must always be low when reading the ROM so it masked (0xff) out to always
	be zero (which it is by default in the register sets).
	*/
//	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_bits(nTuner, REG_ROM_ACCESS, 4, 0, rowindex & 0xff);   /* RTA[3:0] (ROM Table Address)  */
	unsigned char rom_data = read_register(nTuner, REG_ROM_READ);
	write_bits(nTuner, REG_ROM_ACCESS, 4, 0, 0);				 /* Return the RTA to 0. */
	return rom_data;
}


 /*
 This is to create a delay of N ms.
 Either use a loop based on your clock rate, or a built in timer function in your code.
 */
void mswait(int ms)
{
#if 0
	/*
	Here is how you might implement this with clock cycles in microprocessor device.
	A interval timer might also be available in your hardware.
	*/
	clock_t start, finish;
	double duration;
	long count = 0;
	start = clock();

	clock_t end = clock() + ms;

	while (clock() < end)
		;

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;

	//	 char buf[50];
	//	 sprintf_s(buf, 50, "%2.1f seconds\n", duration);
#else
	usleep(ms * 1000);
#endif
}


 /*
	<summary>
     waits for the power manager state machine to signal that it is
     done.  This should take around 2 ms according to simulations.
     Called during init() routine only.
     Note: It takes 24 clock cycles or so to read the PM_DONE bit.
     By the time read back finishes, the done bit should be high
     so 50 iteration should be more than enough to wait 2ms.
     If you have a very fast clock cycle, you may need to increase the
     number of iterations.
     </summary>
     <returns>true if power manager finished, false if timed out waiting.</returns>
 */
bool wait_for_pm_done(int nTuner)
{
	 int PM_DONE = 7;  /* the bit location for PM_DONE */

	 bool done = false;
	 long iterations = 50;   /* See Note above. */

//	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	 while ((done == false) && (iterations-- > 0))
	 {
		 if (read_bit(nTuner, REG_GEN_STATUS, PM_DONE) > 0)
		 {
			 done = true;
		 }
	 }
	 return done;

 }


/*
Set the LO above the desired or below the desired frequency.
LO_Pos: LO_ABOVE_DESIRED or LO_BELOW_DESIRED
Note: In VHF band, HSLS bit is reversed from other bands.
*/
void LO_HSLS(int nTuner, HSLS LO_Pos)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	if (LO_Pos == LO_BELOW_DESIRED)
	{
		if (get_bits(nTuner, REG_BAND, 1, 0) == (unsigned char)__VHF__)
		{
			write_bit(nTuner, REG_BAND, 4, 1);
		}
		else
		{
			write_bit(nTuner, REG_BAND, 4, 0);
		}
	}
	else
	{
		if (get_bits(nTuner, REG_BAND, 1, 0) == (unsigned char)__VHF__)
		{
			write_bit(nTuner, REG_BAND, 4, 0);
		}
		else
		{
			write_bit(nTuner, REG_BAND, 4, 1);
		}
	}
}


/*
	Select appropriate digital output interface standard.
	There are two possible modes I2S or LVDS (JESD204B LVDS).
	Call this after receive mode is set, since there is a number of I2S/LVDS registers
	that will get programmed as part of Receive Mode set up.
	iomode: IO_I2S or IO_LVDS
*/
void io_select(int nTuner, IO_Select_Modes iomode)
{
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	write_bit(nTuner, REG_CTRL, 3, (unsigned char)iomode);   /* io_select */
	if (iomode == IO_LVDS)
	{
		write_bits(nTuner, REG_TEST3, 2, 0, 1);      /* i2s_test */
		write_bits(nTuner, REG_ENABLE0, 7, 6, 1);    /* roi2s_clk_en = bit 7 = 0, rolvds_clk_en=bit 6 = 1 */
		write_bit(nTuner, REG_RESET2, 4, 0);         /* rstb_jesd = bit 4 */
		mswait(1);
		write_bit(nTuner, REG_RESET2, 4, 1);         /* rstb_jesd = bit 4 */
	}
	else
	{
//		write_bits(nTuner, REG_TEST3, 2, 0, 2);      /* i2s_test */
		write_bits(nTuner, REG_TEST3, 3, 0, 2);      /* i2s_test */	/* 20160421 */
		write_bits(nTuner, REG_ENABLE0, 7, 6, 2);    /* roi2s_clk_en = bit 7 = 1, rolvds_clk_en=bit 6 = 0 */
		write_bit(nTuner, REG_RESET2, 5, 0);         /* rstb_i2s = bit 4 */
		mswait(1);
		write_bit(nTuner, REG_RESET2, 5, 1);         /*  rstb_i2s = bit 4 */
	}
}


/*
	Set the MAX2150 to the desired RF Frequency.
	Note: A Channel FSM action must happen after calling this function (perform_FSM
	For this reason, there is Goto_RF_frequency() which both calls this function, and calls Perform_FSM_Action.
	Only use this by itself if you wish to set the RF Frequency and perform a FSM_Action later.

	desiredfrequency: The desired RF frequency in MHz.</param>
*/
void RF_frequency(int nTuner, double desiredfrequency)
{
	/* Set the HSLS bit in the MAX2175: */
	HSLS LO_Pos = (HSLS)get_bit(nTuner, REG_BAND, 4);

	MAXIM_DBG("MAX2175 CS2:[%s:%d:%4.2f]\n", __func__, __LINE__, desiredfrequency);

	if (get_bits(nTuner, REG_BAND, 1, 0) == (unsigned char)__AM__)
	{
		/* In an AM mode. Copy BBF_BW_AM value read at init from ROM into the BBF_BW Bits. */
		NCO_frequency(nTuner, desiredfrequency);
	}
	else
	{
		/*
		Bits 5,4 of the BBF_CTRL register are the BBF_MODE[1:0] bits.  Use these to determine if you are in
		FM or DAB modes.   if the bits are 00b or 01b, you are in FM mode, otherwise you are in DAB mode.
		*/
		if (get_bits(nTuner, REG_BBF_CTRL, 5, 4) == 0)
		{
			/* In a FM/WX mode.  */

			if (get_bits(nTuner, REG_BAND, 1, 0) == (unsigned char)__VHF__)
			{
				if (LO_Pos == LO_ABOVE_DESIRED)  /* LO below Desired */
				{
					LO_frequency(nTuner, desiredfrequency + 0.128);
				}
				else                           /* LO above Desired */
				{
					LO_frequency(nTuner, desiredfrequency - 0.128);
				}
			}
			else
			{
				if (LO_Pos == LO_BELOW_DESIRED)  /* LO below Desired */
				{
					LO_frequency(nTuner, desiredfrequency + 0.128);
				}
				else                           /* LO above Desired */
				{
					LO_frequency(nTuner, desiredfrequency - 0.128);
				}
			}
			NCO_frequency(nTuner, 0.128);
		}
		else if (get_bits(nTuner, REG_BBF_CTRL, 5, 4) == 1)
		{
			/* In a FM HD mode. */

			if (get_bits(nTuner, REG_BAND, 1, 0) == (unsigned char)__VHF__)
			{
				if (LO_Pos == LO_BELOW_DESIRED)  /* Examine HSLS bit, LO below Desired */
				{
					LO_frequency(nTuner, desiredfrequency - 0.228);
				}
				else                           /* LO above Desired */
				{
					LO_frequency(nTuner, desiredfrequency + 0.228);
				}
			}
			else
			{
				if (LO_Pos == LO_BELOW_DESIRED)  /* Examine HSLS bit, LO below Desired */
				{
					LO_frequency(nTuner, desiredfrequency + 0.228);
				}
				else                           /* LO above Desired */
				{
					LO_frequency(nTuner, desiredfrequency - 0.228);
				}
			}
			NCO_frequency(nTuner, 0.228);
		}
		else
		{
			/* In a DAB mode. */
			LO_frequency(nTuner, desiredfrequency);
		}
	}
}


 /*
	When programming the RF_frequency, LO_Frequecy or NCO_Frequency, the Frequency State Machine
	(or FSM) must be initiated by writing to the mode register.  This latches the tuning values
	into the buffer or primary register depending on your mode of operation.
	Goto_RF_frequency calls this automatically after setting the RF_frequency.
 */
void perform_channel_FSM_action(int nTuner, efsm_mode FSM_Action)
 {
	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	 /*  Keep track of current FSM Mode, use the current mode if FSM_Action has not changed. */
	 if (FSM_Action == MODE_CURRENT_ACTION)
	 {
		 FSM_Action = Current_FSM_Mode;
	 }
	 else
	 {
		 Current_FSM_Mode = FSM_Action;
	 }

	 //Perform one or two FSM mode commands.
	 switch (FSM_Action)
	 {
	 case MODE_NO_ACTION:
	 {
		 /* Take no FSM Action. */
		 return;
	 }
	 case MODE_LOAD_TO_BUFFER:
		 fsm_mode(nTuner, MODE_LOAD_TO_BUFFER);
		 break;
	 case MODE_PRESET_TUNE:
	 {
		 fsm_mode(nTuner, MODE_PRESET_TUNE);
		 break;
	 }
	 case MODE_SEARCH:
	 {
		 fsm_mode(nTuner, MODE_SEARCH);
		 break;
	 }
	 case MODE_AF_UPDATE:
	 {
		 fsm_mode(nTuner, MODE_AF_UPDATE);
		 break;
	 }
	 case MODE_JUMP_FAST_TUNE:
		 fsm_mode(nTuner, MODE_JUMP_FAST_TUNE);
		 break;
	 case MODE_CHECK:
	 {
		 fsm_mode(nTuner, MODE_CHECK);
		 break;
	 }
	 case MODE_LOAD_AND_SWAP:
	 {
		 fsm_mode(nTuner, MODE_LOAD_AND_SWAP);
		 break;
	 }
	 case MODE_END:
	 {
		 fsm_mode(nTuner, MODE_END);
		 break;
	 }
	 case MODE_BUFFER_PLUS_PRESET_TUNE:
	 {
		 fsm_mode(nTuner, MODE_LOAD_TO_BUFFER);
		 fsm_mode(nTuner, MODE_PRESET_TUNE);
		 break;
	 }
	 case MODE_BUFFER_PLUS_SEARCH:
	 {
		 fsm_mode(nTuner, MODE_LOAD_TO_BUFFER);
		 fsm_mode(nTuner, MODE_SEARCH);
		 break;
	 }
	 case MODE_BUFFER_PLUS_AF_UPDATE:
	 {
		 fsm_mode(nTuner, MODE_LOAD_TO_BUFFER);
		 fsm_mode(nTuner, MODE_AF_UPDATE);
		 break;
	 }
	 case MODE_BUFFER_PLUS_JUMP_FAST_TUNE:
	 {
		 fsm_mode(nTuner, MODE_LOAD_TO_BUFFER);
		 fsm_mode(nTuner, MODE_JUMP_FAST_TUNE);
		 break;
	 }
	 case MODE_BUFFER_PLUS_CHECK:
	 {
		 fsm_mode(nTuner, MODE_LOAD_TO_BUFFER);
		 fsm_mode(nTuner, MODE_CHECK);
		 break;
	 }
	 case MODE_BUFFER_PLUS_LOAD_AND_SWAP:
	 {
		 fsm_mode(nTuner, MODE_LOAD_TO_BUFFER);
		 fsm_mode(nTuner, MODE_LOAD_AND_SWAP);
		 break;
	 }
	 default:
	 {
		 fsm_mode(nTuner, MODE_NO_ACTION);
		 break;
	 }
	 }

 }


/*
	Set the Numerically Controlled Oscillator's frequency registers (in MHz).
	Note: This does not immediately cause the NCO to change frequency but just programs the registers.
	The MODE bits must be set to cause the NCO to change frequency.
	This function is called by RF_frequency().
	NOTE: If called independently of the Goto_RF_frequency() function, the FSM state machine must be programmed
	accordingly.  Use: Call perform_channel_FSM_action().

	nco_frequency_desired: The desired NCO frequency in MHz.</param>
*/
void NCO_frequency(int nTuner, double nco_frequency_desired)
{
	/* Decim_Ratio_beforeNCO */
	nco_frequency_desired = -nco_frequency_desired;
	double ADC_SampleRate = xtal_frequency;
	double clock_rate = ADC_SampleRate / Decim_Ratio_beforeNCO;
	double nco_value_desired = 0.0;
	double nco_frequency_desired_ABS = 0.0;
	int regNCO = 0;  /* Value to be put into the AM_PLD_NCO bits of the PLD_NCO register after calculation. */

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	if (nco_frequency_desired < 0)
		nco_frequency_desired_ABS = -nco_frequency_desired;
	else
		nco_frequency_desired_ABS = nco_frequency_desired;

	if (nco_frequency_desired_ABS < clock_rate / 2.0)
	{
		nco_value_desired = 2.0 * nco_frequency_desired / clock_rate;
		alias = false;
	}
	else
	{
		if (nco_frequency_desired < 0)
		{
			nco_value_desired = -2.0 * (clock_rate - nco_frequency_desired_ABS) / clock_rate;
		}
		else
		{
			nco_value_desired = 2.0 * (clock_rate - nco_frequency_desired_ABS) / clock_rate;
		}
		alias = true;
	}

	if (nco_frequency_desired < 0)
	{
		/* pow(2.0, 21.0) sets the MSB bit if the NCO register value (regNCO).  The MSB is the sign bit in
		   regNCO. */
		regNCO = (int)(pow(2.0, 21.0) + floor((nco_value_desired * pow(2.0, 20.0)) + 0.50));
	}
	else
	{
		regNCO = (int)floor((nco_value_desired * pow(2.0, 20.0)) + 0.50);
	}

	/* Make sure the fsm is not busy.  Trying to set registers while it is busy would result failed writes. */
	wait_while_ch_fsm_busy(nTuner);

	/* write_bits for first register, since it only fills the first 5 LSB's */
	/* write_register for others, since it fills the whole register (a little faster). */
	set_bits(nTuner, REG_NCO2, 4, 0, (regNCO & 0x1f0000) >> 16);	/* NCO[20:16] */
	set_bits(nTuner, REG_NCO1, 7, 0, (regNCO & 0xff00) >> 8);       /* NCO[15:8]  */
	set_bits(nTuner, REG_NCO0, 7, 0, regNCO & 0xff);                /* NCO[7:0]   */
	write_registers(nTuner, 3, REG_NCO2);  //send all 3 NCO registers.
}


/*
	Change FSM mode and wait for the FSM (Finite State Machine) to finish.
	Called within RF_frequency().

	new_mode: one of the enumerated frequency state machine modes.

*/
bool fsm_mode(int nTuner, efsm_mode new_mode)
{
//	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);
	/*
	Attempting to set the state machine while it is busy can result in invalid states, so
	wait until the machine is no longer busy settling on a state before changing to a new state:
	*/
	wait_while_ch_fsm_busy(nTuner);



	write_bits(nTuner, REG_CH_FSM_MODE, 2, 0, (int)new_mode);
	/*
	Now we must wait for the CH_FSM_BUSY bit to go low indicating that the FSM state
	machine has finished changing mode.  This can take 50uS or 50ms.
	So to prevent having to read the BUSY bit continuously from the MAX2175 registers,
	we will add a delay here if the FSM state machine will take 1ms or longer to settle.
	Here the time is rounded up to the nearest ms.
	If your timer supports shorter times, you can modify the code below with more accurate times shown in the comments.
	*/
	if (get_bits(nTuner, REG_BAND, 1, 0) == (unsigned char)__AM__)
	{
		/* In an AM mode. */
		switch ((int)new_mode)
		{
		case 1:
			/* 51.1ms */
			mswait(52);
			break;
		case 2:
			/* 1.15ms */
			mswait(2);
			break;
		case 3:
			/* Not used */
			break;
		case 4:
			/* 1.25ms */
			mswait(2);
			break;
		case 5:
			/*  not used */
			break;
		case 6:
			/* 50us  */
			mswait(1);
			break;
		case 7:
			/* 100us */
			mswait(1);
			break;
		}
	}
	else
	{
		/*
		Bits 5,4 of the BBF_CTRL register are the BBF_MODE[1:0] bits.  Use these to determine if you are in
		FM or DAB modes.   if the bits are 00b or 01b, you are in FM mode, otherwise you are in DAB mode.
		*/
		if (get_bits(nTuner, REG_BBF_CTRL, 5, 4) < 2)
		{
			/*  IN FM Mode */
			switch ((int)new_mode)
			{
			case 1:
				/* 51.1ms */
				mswait(52);
				break;
			case 2:
				/* 1.15ms */
				mswait(2);
				break;
			case 3:
				/* 3.3ms */
				mswait(4);
				break;
			case 4:
				/* 1.25ms */
				mswait(2);
				break;
			case 5:
				/* 1.15ms */
				mswait(2);
				break;
			case 6:
				/* 50us */
				mswait(1);
				break;
			case 7:
				/* 100us */
				mswait(1);
				break;
			}
		}
		else
		{
			//In a DAB mode.
			switch ((int)new_mode)
			{
			case 1:
				/* 51.5ms */
				mswait(52);
				break;
			case 2:
				/* 200uS */
				mswait(1);
				break;
			case 3:
				/* 50us */
				mswait(1);
				break;
			case 4:
				/* 3ms */
				mswait(3);
				break;
			case 5:
				/* not used */
				break;
			case 6:
				/* 50us */
				mswait(1);
				break;
			case 7:
				/* 100us */
				mswait(1);
				break;
			}
		}
	}

	wait_while_ch_fsm_busy(nTuner);
	return true;
}


/*
	<summary>
	CH_FSM_BUSY is a bit that indicates that the channel FSM state machine
	is currently busy.  Attempting to set tuning registgers while the state machine
	is busy, may result in lost register changes.
	This function waits for the the CH_FSM_BUSY bit to go low indicating that the
	channel FSM state machine has finished.   To check this bit, the function has
	to read the GEN_STATUS register.  This takes a relatively long time since
	25 serial clock cycles have to happen to read this register.  Unless the processor
	clock is extremely fast, this should happen in only one or two reads.
	</summary>
*/
bool wait_while_ch_fsm_busy(int nTuner)
{
	const int BIT_CH_FSM_BUSY = 1;
	bool busy = true;

//	printf("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	if (read_bit(nTuner, REG_GEN_STATUS, BIT_CH_FSM_BUSY) == 0)
	{
		busy = false;
	}

	if (busy == true)
	{
		mswait(50);
		if (read_bit(nTuner, REG_GEN_STATUS, BIT_CH_FSM_BUSY) == 0)
		{
			busy = false;
		}
		if (busy == true)
		{
			mswait(50);
			if (read_bit(nTuner, REG_GEN_STATUS, BIT_CH_FSM_BUSY) == 0)
			{
				busy = false;
			}
		}

	}

	return busy;

}


/*
	Directly tune to a specified RF_frequency.
	The FSM Mode is set to load the buffer, then do a JUMP and Fast Tune.

	<param name="freq">The Desired Frequency in MHz.</param>
*/
void set_RF_frequency(int nTuner, double freq)
{
//	printf("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	RF_frequency(nTuner, freq);
	perform_channel_FSM_action(nTuner, MODE_CURRENT_ACTION);

	max2175_mwait(200);

//	printf("\n\nMAX2175 CS2:AFTER SET FREQUENCY\n");
//	dump_all_reg_val();
}


/*
	Set the MAX2175 to the desired LO frequency in MHz.
	This function is called by Goto_RF_frequency().
	NOTE: If called independently of the Goto_RF_frequency() function, the FSM state machine must be programmed
	accordingly.  Use: Call perform_channel_FSM_action().
	desired_lo_frequency: The desired frequency in MHz.</param>
*/
void LO_frequency(int nTuner, double desired_lo_frequency)
{
	double n_plus_f_desired = 0;
	int n_val_desired = 0;
	int f_val_desired = 0;
	double lo_div = 0;
	double pll_div = 0;
	int loband_bits = 0;
	int vcodiv_bits = 0;
	double lo_multiplier = 0;

	double fcomp = f_comparison(nTuner);

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	if (get_bits(nTuner, REG_BAND, 1, 0) == (unsigned char)__AM__)  /* Examine RF_Band bits to see what mode we are in. */
	{
		if (get_bit(nTuner, REG_BAND, bit_am_lo_en) == 0)
		{
			/*
			   lo mult = 8 / 128 = 1/16
			*/
			lo_div = 128;
			pll_div = 8;
		//	loband_bits = 0;	// already had same value. (for codesonar)
		//	vcodiv_bits = 0;	// already had same value. (for codesonar)

			lo_multiplier = pll_div / lo_div;

		}
	}
	else if (get_bits(nTuner, REG_BAND, 1, 0) == (unsigned char)__FM__)  //examine RF_Band bits to see what mode we are in.
	{
		if (desired_lo_frequency <= 74.7)
		{
			/*
			   lo mult =  6 / 4*24 = 1/16
			*/
			lo_div = 24;
			pll_div = 6;
		//	loband_bits = 0;	// already had same value. (for codesonar)
		//	vcodiv_bits = 0;	// already had same value. (for codesonar)
		}
		else if ((desired_lo_frequency > 74.7) && (desired_lo_frequency <= 110.0))
		{
			/*
			   lo mult =  8 / 4 * 16 = 1/8
			*/
			lo_div = 16;
			pll_div = 8;
			loband_bits = 1;
		//	vcodiv_bits = 0;	// already had same value. (for codesonar)
		}
		else
		{
			/*
			   lo mult = 6 / 4 * 12 = 1/8
			*/
			lo_div = 12;
			pll_div = 6;
			loband_bits = 1;
			vcodiv_bits = 3;
		}
		lo_multiplier = pll_div / (4 * lo_div);
	}
	else if (get_bits(nTuner, REG_BAND, 1, 0) == (unsigned char)__VHF__)  /* Examine RF_Band bits to see what mode we are in. */
	{
		/* Frequency dependent items */
		if (desired_lo_frequency <= 210.0e6)
		{
			/*
				lo mult = 8 / 4 * 8 = 1/4
			*/
			lo_div = 8;
			pll_div = 8;
			loband_bits = 2;
			vcodiv_bits = 2;
		}
		else
		{
			/*
			   lo mult = 6 / 4 * 6 = 1/4
			*/
			lo_div = 6;
			pll_div = 6;
			loband_bits = 2;
			vcodiv_bits = 1;
		}
		lo_multiplier = pll_div / (4 * lo_div);
	}
	else if (get_bits(nTuner, REG_BAND, 1, 0) == (unsigned char)__LBAND__)  /* examine RF_Band bits to see what mode we are in. */
	{
		/*
		   lo mult = 8 / 1 * 4 = 2
		*/
		lo_div = 1;
		pll_div = 8;
		loband_bits = 3;
		vcodiv_bits = 2;

		lo_multiplier = pll_div / (4 * lo_div);
	}

	if(lo_multiplier > 0) {
		/* calculate the N divider + the fractional divider part  i.e. 30.125 */
		n_plus_f_desired = desired_lo_frequency / fcomp / lo_multiplier;
		/* Seperate out the N-Divider portion: */
		n_val_desired = (int)floor(n_plus_f_desired);
		/* Seperate out the fractional portion, multiply by 2^20 to get the fractional multiplier.  */
		f_val_desired = (int)floor((n_plus_f_desired - n_val_desired) * pow(2.0, 20.0));

		/* Make sure the fsm is not busy.  Trying to set registers while it is busy could result in writes that fail. */
		wait_while_ch_fsm_busy(nTuner);


		/* Write the values calculated above out to the appropriate register. */
		set_bits(nTuner, REG_BAND, 3, 2, loband_bits);
		set_bits(nTuner, REG_VCO, 7, 6, vcodiv_bits);

		set_bits(nTuner, REG_NDIV, 7, 0, n_val_desired);
		set_bits(nTuner, REG_RDIV_FRAC2, 3, 0, (f_val_desired >> 16) & 0x1f);
		set_bits(nTuner, REG_FRAC1, 7, 0, (f_val_desired >> 8) & 0xff);
		set_bits(nTuner, REG_FRAC0, 7, 0, f_val_desired & 0xff);
		write_registers(nTuner, 6, REG_NDIV);  /* write all 6 registers starting at register 1 (REG_NDIV) */
	}
}

/*
	Read the PGA gain determined by the PGA AGC.
	The bit: MANUAL_PGAAGC (bit 2) in the register MANUAL_MODES must be set to
	0 for the Auto AGC to be enabled.
	If the auto AGC is enabled, reading the bits PGA_GAIN[4:0] of the PGA_GAIN register will
	return the PGA gain set by the AGC.
	If the MANUAL_PGAAGC bit is set to 1, gain will not change automatically
	but will be set by the bits PGA_GAIN[4:0] in the PGA_GAIN register.
	Gain ranges from -10dB to +10dB.
	Read the PGA_GAIN register to get the bit value from bits
	PGA_GAIN[4:0].   A bit value of 0 = -10 dB of gain.
	A bit value of 20 = +10dB of gain.  Each bit represents 1 dB of gain.
	Bit values > 20 are reserved and result in 10dB of gain.
	</summary>
*/
int read_PGA_Gain(int nTuner)
{
	int gain = -10 + (int)read_bits(nTuner, REG_PGA_GAIN, 4, 0);

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	if (gain > 10)
	{
		gain = 10;
	}

	return gain;
}

/*
	Calculate the current RF Gain for the current band.
	Set the MANUAL_RFAGC to 0 for this method.
	If the bit MANUAL_RFAGC (MANUAL_MODES register) is set to 0, the gain is set automatically by the RFAGC state machine.
	If the Manual_RFAGC bit is set to 1,  the gain can only be set manually by programming the gain registers and will
	not automatically be set by the RFAGC.  The same register you read in this method is used set the gain in the manual mode.
	The RF Gain can be calculated by reading the associated register for the band you are currently in and
	performing the calculations in this method.

	Returns: Gain in dB
*/
int read_RF_Gain(int nTuner)
{
	eBand rfBand = (eBand)get_bits(nTuner, REG_BAND, 1, 0);
	int gain = 0;

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	switch (rfBand)
	{
	case __AM__:
	{
		gain = 3 * read_bits(nTuner, REG_AM_RFGAIN, 3, 0) - 13;
		break;
	}
	case __FM__:
	{
		gain = 3 * read_bits(nTuner, REG_FM_RFGAIN, 3, 0) - 5;
		break;
	}

	case __VHF__:
	{
		int gainbits = read_bits(nTuner, REG_VHF_RFGAIN, 5, 4);
		int attenbits = read_bits(nTuner, REG_VHF_RFGAIN, 3, 0);

		switch (gainbits)
		{
		case 0:
			if (attenbits < 4)
				gain = -20;
			else
				gain = -20 + 3 * (attenbits - 4);
			break;
		case 1:
			if (attenbits < 10)
				gain = 16;
			else
				gain = 16 + 3 * (attenbits - 10);
			break;
		case 2:
			if (attenbits < 12)
				gain = 34;
			else
				gain = 34 + 3 * (attenbits - 12);
			break;
		case 3:
			if (attenbits < 12)
				gain = 46;
			else
				gain = 46 + 3 * (attenbits - 12);
			break;
		}
		break;
	}
	case __LBAND__:
	{
		int gainbits = read_bits(nTuner, REG_LB_RFGAIN, 5, 4);
		int attenbits = read_bits(nTuner, REG_LB_RFGAIN, 3, 0);

		switch (gainbits)
		{
		case 0:
			if (attenbits < 4)
				gain = -20;
			else
				gain = -20 + 3 * (attenbits - 4);
			break;
		case 1:
			if (attenbits < 10)
				gain = 16;
			else
				gain = 16 + 3 * (attenbits - 10);
			break;
		case 2:
			if (attenbits < 12)
				gain = 34;
			else
				gain = 34 + 3 * (attenbits - 12);
			break;
		case 3:
			if (attenbits < 12)
				gain = 46;
			else
				gain = 46 + 3 * (attenbits - 12);
			break;
		}
		break;
	}
	}

	return gain;
}


/// <summary>
/// Reads the Digital AGC Power from the part.
/// </summary>
/// <returns></returns>
double read_DAGC_PWR(int nTuner)
{
	int val = 0;
	double dbFS = 0;
	val = read_bits(nTuner, REG_DAGC_STATUS, 5, 0);
	double power = dbFS - 1.5 * (63 - val);

	MAXIM_DBG("MAX2175 CS2:[%s:%d], DAGC_PWR[%xh, %ddB]\n", __func__, __LINE__, val, power);

	return power;
}

/// <summary>
/// Read the Digital AGC Gain as determined by the part.
/// This value is a derivation from the value in the DAGC_GAIN register
/// </summary>
/// <returns>Gain in dB</returns>
double read_DAGC_Gain(int nTuner)
{
	double gain = 1.5 * read_bits(nTuner, REG_DAGC_GAIN, 5, 0);

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	return gain;
}



/*
	Calculate Input power based on the measured BB signal level.
	Pdbfs: Power in dB full scale for the base band IC.
*/
double	am_rssi_val;
double	maxim_rssi_tmp;
double read_RSSI(int nTuner)
{
	double rssi_Offset = 0;   /* Gain Factor. */
	int rssi_const = 0;
	int DUAL_FM = 2;
	int FM_HIZ_IN = 4;

	MAXIM_DBG("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	if (get_bits(nTuner, REG_BAND, 1, 0) == (unsigned char)__FM__)
	{
		rssi_const = -18;
		if (read_bit(nTuner, REG_FM_RFGAIN, FM_HIZ_IN) == 1)
		{
			rssi_Offset = 3.6;
		}
		else if (read_bit(nTuner, REG_CTRL, DUAL_FM) == 1)
		{
			rssi_Offset = 1.9;

		}
		else
		{
			rssi_Offset = 0;
		}
	}
	else
	{
		rssi_const = -18;   //TBD
		rssi_Offset = 0;
	}


	double rssi = read_DAGC_PWR(nTuner) - read_RF_Gain(nTuner) - read_PGA_Gain(nTuner) - read_DAGC_Gain(nTuner) - rssi_Offset - rssi_const;

	am_rssi_val = maxim_rssi_tmp = rssi;

	return rssi;

}

int gain(int ntuner)
{
	int temp_readback;
	int fm_temp_comp;
	int rf_band;
	int gain1_readback;
	int gain2_readback;
	int gain_readback;

	int dagc_gain;
	int temp_gain;

	int am_hiz=0;
	int fm_dsp_loss=12930;
	int am_dsp_loss=13570;

	rf_band = get_bits(ntuner, REG_BAND, 1, 0);
	if (rf_band != (unsigned char)__AM__) {
		if (read_bit(ntuner, REG_TSENSOR, 5) != 0)		{
			write_bit(ntuner, REG_TSENSOR, 5, 0);
		}

	}
	temp_readback = read_bits(ntuner, REG_TSENSOR, 4, 0);

	if (rf_band == (unsigned char)__FM__) {
		fm_temp_comp = (6 * (16 - temp_readback)) * - 18;
		gain1_readback = read_bit(ntuner, REG_FM_RFGAIN, 4);
		gain2_readback = read_bit(ntuner, REG_CTRL, 2);

		printf("----------------FM Bits Readback-------------------\n");
		printf("dagc_pwr readback = %d\n", read_bits(ntuner, REG_DAGC_STATUS, 5, 0));
		printf("gain, dagc_gain readback = %d\n", read_bits(ntuner, REG_DAGC_GAIN, 5, 0));
		printf("gain, pga_gain readback = %d\n", read_bits(ntuner, REG_PGA_GAIN, 4, 0));


		printf("gain, gain1_readback = %d\n", gain1_readback);
		printf("gain, gain2_readback = %d\n", gain2_readback);



		printf("----------------FM Power Readback-------------------\n");
		printf("gain, tuner->dsp_loss = %d\n", fm_dsp_loss);
		printf("gain, fm_temp_comp = %d\n", fm_temp_comp);
		printf("gain, dagc_gain = %d\n", (1500 * read_bits(ntuner, REG_DAGC_GAIN, 5, 0)));
		printf("gain, pga_gain = %d\n", (1000 * ((int)read_bits(ntuner, REG_PGA_GAIN, 4, 0) - 10)));
		printf("gain, fm_rf_gain = %d\n", 3000 * read_bits(ntuner, REG_FM_RFGAIN, 3, 0) - 5000);



		temp_gain = ((1500 * read_bits(ntuner, 48, 5, 0)) + (1000 * ((int)read_bits(ntuner, 49, 4, 0) - 10)) + (3000 * read_bits(ntuner, 50, 3, 0) - 5000)
			+ (gain1_readback * ((3600 * (1 - gain2_readback) + (1900 * gain2_readback))))
			+ (1700 * gain2_readback) - fm_dsp_loss - 13000 + 3000 +
			(fm_temp_comp * gain1_readback))/100;

		return temp_gain;

	} else if (rf_band == (unsigned char)__AM__) {



		printf("----------------AM Bits Readback-------------------\n");
		printf("dagc_pwr readback = %d\n", read_bits(ntuner, 65, 5, 0));
		printf("gain, rf_gain readback = %d\n", read_bits(ntuner, 51, 3, 0));
		printf("gain, dagc_gain readback = %d\n", read_bits(ntuner, 48, 5, 0));
		printf("gain, pga_gain readback = %d\n", read_bits(ntuner, 49, 4, 0));


		printf("gain, tuner->am_hiz = %d", am_hiz);



		printf("----------------AM Power Readback-------------------\n");
		printf("gain, tuner->dsp_loss = %d\n", am_dsp_loss);
		printf("gain, dagc_gain = %d\n", (1500 * read_bits(ntuner, 48, 5, 0)));
		printf("gain, pga_gain = %d\n", (1000 * ((int)read_bits(ntuner, 49, 4, 0) - 10)));
		printf("gain, fm_rf_gain = %d\n", 3000 * read_bits(ntuner, 51, 3, 0) - 13000);


		return ( ((1500 * read_bits(ntuner, 48, 5, 0)) + (1000 * ((int)read_bits(ntuner, 49, 4, 0) - 10)) + (3000 * read_bits(ntuner, 51, 3, 0) - 13000) +
			(am_hiz * -4000) - am_dsp_loss - 13000
			- (-9000 * am_hiz - (-2000 * (1 - am_hiz)))) )/100;

	} else if (rf_band == (unsigned char)__VHF__) {

		int VHF_TempCo = (6 * (16 - temp_readback)) * -44;

		gain_readback = read_bits(ntuner, 52, 5, 0);


		if (gain_readback > 50){
			temp_gain = 55000 + (gain_readback - 63) * 3000;
		}
		else if (gain_readback < 50 && gain_readback > 35){
			temp_gain = 43000 + (gain_readback - 47) * 3000;
		}
		else if (gain_readback < 35 && gain_readback > 20){
			temp_gain = 31000 + (gain_readback - 31) * 3000;
		}
		else if (gain_readback <= 15 && gain_readback >= 9){
			temp_gain = 13000 + (gain_readback - 15) * 3000;
		}
		else if (gain_readback < 9){
			temp_gain = -5000 + (gain_readback - 9) * 1500;
		}
		else {
			temp_gain = 0;
		}

		printf("----------------VHF Bits Readback-------------------\n");
		printf("dagc_pwr readback = %d\n", read_bits(ntuner, 65, 5, 0));
		printf("gain, rf_gain readback = %d\n", gain_readback);
		printf("gain, dagc_gain readback = %d\n", read_bits(ntuner, 48, 5, 0));
		printf("gain, pga_gain readback = %d\n", read_bits(ntuner, 49, 4, 0));


		printf("----------------VHF Power Readback-------------------\n");
		printf("gain, tuner->dsp_loss = %d\n", fm_dsp_loss);
		printf("gain, VHF_TempCo = %d\n", VHF_TempCo);
		printf("gain, dagc_gain = %d\n", (1500 * read_bits(ntuner, 48, 5, 0)));
		printf("gain, pga_gain = %d\n", (1000 * ((int)read_bits(ntuner, 49, 4, 0) - 10)));
		printf("gain, rf_gain = %d\n", temp_gain);

		/*  dagc_gain, pga_gain, rf_gain */

		return  ((1500 * read_bits(ntuner, 48, 5, 0))
			+ (1000 * ((int)read_bits(ntuner, 49, 4, 0) - 10)) + temp_gain - fm_dsp_loss - 13000 - 3000 - VHF_TempCo)/100;

	} else if (rf_band == (unsigned char)__LBAND__) {

		int LB_TempCo = (6 * (16 - temp_readback)) * -46;

		gain_readback = read_bits(ntuner, 53, 5, 0);


		if (gain_readback > 50) {
			temp_gain = 55000 + (gain_readback - 63) * 3000;
		}
		else if (gain_readback < 50 && gain_readback > 35) {
			temp_gain = 43000 + (gain_readback - 47) * 3000;
		}
		else if (gain_readback < 35 && gain_readback > 20) {
			temp_gain = 31000 + (gain_readback - 31) * 3000;
		}
		else if (gain_readback <= 15 && gain_readback >= 10) {
			temp_gain = 13000 + (gain_readback - 15) * 3000;
		}
		else if (gain_readback < 10) {
			temp_gain = -2000;
		}
		else
		{
			temp_gain = 0;
		}

		printf("----------------LBAND Bits Readback-------------------\n");
		printf("dagc_pwr readback = %d\n", read_bits(ntuner, 65, 5, 0));
		printf("gain, rf_gain readback = %d\n", gain_readback);
		printf("gain, dagc_gain readback = %d\n", read_bits(ntuner, 48, 5, 0));
		printf("gain, pga_gain readback = %d\n", read_bits(ntuner, 49, 4, 0));


		printf("----------------LBAND Power Readback-------------------\n");
		printf("gain, tuner->dsp_loss = %d\n", fm_dsp_loss);
		printf("gain, LB_TempCo = %d\n", LB_TempCo);
		printf("gain, dagc_gain = %d\n", (1500 * read_bits(ntuner, 48, 5, 0)));
		printf("gain, pga_gain = %d\n", (1000 * ((int)read_bits(ntuner, 49, 4, 0) - 10)));
		printf("gain, rf_gain = %d\n", temp_gain);

		return  ((1500 * read_bits(ntuner, 48, 5, 0)) + (1000 * ((int)read_bits(ntuner, 49, 4, 0) - 10)) + temp_gain - fm_dsp_loss - 13000 - 9000 - LB_TempCo)/100;
	}
	else {
		return 0;
	}

}

int read_rssi(int nTuner)
{
	int power;
	int gainread;
	gainread = gain(nTuner);
	power = (-15 * (63 - read_bits(nTuner, REG_DAGC_STATUS, 5, 0)));
	printf("read_rssi, power = %d, gain = %d, rssi = %d\n", power, gainread, power-gainread);
	return power - gainread;

}

void dump_all_reg_val(int nTuner)
{
#if 0
	int i;

	printf("MAX2175 CS2:[%s:%d]\n", __func__, __LINE__);

	for(i=0;i<=255;i++){
		if(i<0x6b||i>0x6f)
			printf("MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n",i,read_register(nTuner, i));
		mswait(10);
	}

	printf("MAX2175 CS2: REG=0x%02x,Val=0x%02x\n",0x6b,read_register(nTuner, 0x6b));
	mswait(10);
	printf("MAX2175 CS2: REG=0x%02x,Val=0x%02x\n",0x6c,read_register(nTuner, 0x6c));
	mswait(10);
	printf("MAX2175 CS2: REG=0x%02x,Val=0x%02x\n",0x6d,read_register(nTuner, 0x6d));
	mswait(10);
	printf("MAX2175 CS2: REG=0x%02x,Val=0x%02x\n",0x6e,read_register(nTuner, 0x6e));
	mswait(10);
	printf("MAX2175 CS2: REG=0x%02x,Val=0x%02x\n",0x6f,read_register(nTuner, 0x6f));
	mswait(10);
#else
	int i, reg_value;
	FILE *gfile_reg;
	gfile_reg = fopen ("/tmp/max2175reg.txt", "w");

	if(gfile_reg <= 0) {
		printf("MAX2175 CS2 : Can't make /tmp/max2175reg.txt file!!!\n");
		return;
	}

	for(i=0;i<=255;i++){
		if(i<0x6b||i>0x6f) {
			reg_value = read_register(nTuner, i);
			printf("MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n",i,reg_value);
			fprintf(gfile_reg, "MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n", i, reg_value);
		}
		mswait(10);
	}
	reg_value = read_register(nTuner, 0x6b);
	printf("MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n",0x6b,reg_value);
	fprintf(gfile_reg, "MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n", 0x6b, reg_value);
	mswait(10);
	reg_value = read_register(nTuner, 0x6c);
	printf("MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n",0x6c,reg_value);
	fprintf(gfile_reg, "MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n", 0x6c, reg_value);
	mswait(10);
	reg_value = read_register(nTuner, 0x6d);
	printf("MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n",0x6d,reg_value);
	fprintf(gfile_reg, "MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n", 0x6d, reg_value);
	mswait(10);
	reg_value = read_register(nTuner, 0x6e);
	printf("MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n",0x6e,reg_value);
	fprintf(gfile_reg, "MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n", 0x6e, reg_value);
	mswait(10);
	reg_value = read_register(nTuner, 0x6f);
	printf("MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n",0x6f,reg_value);
	fprintf(gfile_reg, "MAX2175 CS2 : REG=0x%02x,Val=0x%02x\n", 0x6f, reg_value);
	mswait(10);

	fclose(gfile_reg);
	gfile_reg = NULL;
	sync();
	printf ("MAX2175 Register Dump Success!!\n");
#endif
}


