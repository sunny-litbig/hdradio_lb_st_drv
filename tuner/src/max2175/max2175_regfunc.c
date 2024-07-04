/*******************************************************************************
* Copyright (C) 2014 Maxim Integrated Products, Inc., All rights Reserved.
* * This software is protected by copyright laws of the United States and
* of foreign countries. This material may also be protected by patent laws
* and technology transfer regulations of the United States and of foreign
* countries. This software is furnished under a license agreement and/or a
* nondisclosure agreement and may only be used or reproduced in accordance
* with the terms of those agreements. Dissemination of this information to
* any party or parties not specified in the license agreement and/or
* nondisclosure agreement is expressly prohibited.
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
* Products, Inc. Branding Policy.*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/


// This Class handles the IO functions for the derived classes.
// It reads and writes bytes to registers in the MAXIM IC as well as provides bitwise functions to
// manipulate single bits or groups of bits.
// Ver 3.0  3/12/2015
// Paul Nichol

/*
 * Notes on use:
 *
 * In the three functions to read and write registers: Write_Registers, write_register
 * and read_register, replace the UART.XXXX functions
 *
 * with your software platform's serial I2C functions.
 * The I2C Read and Write addresses are dependant on the state of the MAX2175's
 * ADDR1 and ADDR2 pins.
 * Write_Address	Read_Address	ADDR2	ADDR1
 *	0xC0			0XC1			0		0
 *	0xC2			0XC3			0		1
 *	0xC4			0XC5			1		0
 *	0XC6			0XC7			1		1
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tcradio_types.h"
//#include "MAX2175_Driver.h"
//#include "MAX2175_api.h"		/* 20160407 */
#include "tcradio_peri_config.h"
#include "tcradio_drv.h"
#include "max2175_hal.h"			/* 20160407 */

#define  I2C_DEV_ADDR_MAX2175_0  0xC0
#define  I2C_DEV_ADDR_MAX2175_1  0xC4//0xC2
//#define  I2C_DEV_ADDR_MAX2175_2  0xC4
//#define  I2C_DEV_ADDR_MAX2175_3  0xC6

#define I2C_DATA_BUF_SIZE 256

//int32 Reg_Array[256];

static RET dev_tuner_reg_write(uint8 nTuner, uint8 addr, uint8 *data, uint32 data_len);
static RET dev_tuner_reg_read(uint8 nTuner, uint8 addr, uint8 *data, uint32 data_len);


I2C_Addresses_t m_I2C_Address = 0;
uint32 cumulativeError = 0;

#if 1
RET dev_tuner_reg_write(uint8 nTuner, uint8 addr, uint8 *data, uint32 data_len)
{
	RET xerr;
	uint8 buf[I2C_DATA_BUF_SIZE]={0,};

	if(I2C_DATA_BUF_SIZE <= data_len)
		return eRET_NG_INVALID_LENGTH;

	buf[0] = addr;
	max2175_memcpy(buf+1, data, data_len);

	switch(nTuner){
	case 0:		xerr = (*pfnI2cTx)(I2C_DEV_ADDR_MAX2175_0, buf, data_len+1);	break;
	case 1:		xerr = (*pfnI2cTx)(I2C_DEV_ADDR_MAX2175_1, buf, data_len+1);	break;
//	case 2:		xerr = dev_i2c_write8(I2C_DEV_ADDR_MAX2175_2, addr, data, data_len);	break;
//	case 3:		xerr = dev_i2c_write8(I2C_DEV_ADDR_MAX2175_3, addr, data, data_len);	break;
	default:
		MAXIM_DBG("dev_tuner_reg_write : nTuner[%d] is invalid\n", nTuner);
		return eRET_NG_IO;
	}

	if(xerr != eRET_OK){
		MAXIM_DBG("%s : i2c write failed...\n", __func__);
		return eRET_NG_IO;
	}
	return eRET_OK;
}

RET dev_tuner_reg_read(uint8 nTuner, uint8 addr, uint8 *data, uint32 data_len)
{
	RET xerr;
	uint8 addr_buf[2]={0,};

	addr_buf[0] = addr;

	max2175_mwait(1);

	switch(nTuner){
	case 0:		xerr = (*pfnI2cRx)(I2C_DEV_ADDR_MAX2175_0, addr_buf, 1, data, data_len);	break;
	case 1:		xerr = (*pfnI2cRx)(I2C_DEV_ADDR_MAX2175_1, addr_buf, 1, data, data_len);	break;
//	case 2:		xerr = dev_i2c_read8(I2C_DEV_ADDR_MAX2175_2, addr, data, data_len);	break;
//	case 3:		xerr = dev_i2c_read8(I2C_DEV_ADDR_MAX2175_3, addr, data, data_len);	break;
	default:
		MAXIM_DBG("dev_tuner_reg_read : nTuner[%d] is invalid\n", nTuner);
		return eRET_NG_IO;
	}

	if(xerr != eRET_OK){
		MAXIM_DBG("%s : i2c write failed...\n", __func__);
		return eRET_NG_IO;
	}

	return eRET_OK;
}

#endif
/// <summary>
/// Sends a series of registers out in one I2C write sequence.
/// Note: This method is the faster than write_register because it sends all registers in a single
/// sequence instead of one at a time.  This method can only be used if sending consecutive register sequences.
///
/// write_register sends a single register by sending the ICaddr, RegIndex, RegValue char sequence.
/// This method sends all the registers by sending the sequence:
/// ICaddr, RegStartingIndex, RegVal0, RegVal1, RegVal2, RegVal3 ....
/// In this case the RegStartingIndex = 0, so we start writing at the first register.
/// Be cautious that you do not run off the end.  Setting Starting Index to 254 and sending 30 registers
/// will overflow.
/// Note2: C# is funny about bitwise manipulation with bytes (such as ands, ors, shifts), so I work with integer arrays
/// until the elements must be sent to my I-Squared-C function which takes bytes.
/// So that is why there is a conversion to a char array here.
/// </summary>
/// <param name="NumRegisters">Number of registers to send (1 to 256)</param>
/// <param name="StartingIndex">Index into the first register to write to in the sequence (0 to 255).</param>
RET write_registers(int32 nTuner, uint8 NumRegisters, uint8 StartingIndex)
{
	int32 i;
	RET xerr = eRET_OK;
	unsigned char* newArray = malloc(NumRegisters + 1);  //Create new array 1 element bigger.
	if(newArray == NULL)
		return eRET_NG_SYSCALL;

	if(cumulativeError> MAX_ERROR) {
		MAXIM_DBG("%s : Too many i2c failures...[%d] \n", __func__, cumulativeError);
		if(newArray != NULL) {
			free(newArray);
			newArray = NULL;
		}
		return eRET_NG_IO;
	}

    newArray[0] = StartingIndex;  //First char sent is 0, start writing at register 0.
    //Now copy the register values into the array, but starting at index 1 since the address occupies value 0;
    for ( i = 0; i < NumRegisters; i++)
        newArray[i + 1] = (char)regArray[i + StartingIndex];

    for ( i = StartingIndex; i < StartingIndex + NumRegisters; i++)
    {
	xerr = write_register(nTuner, i, regArray[i]);
	if(xerr != eRET_OK && cumulativeError> MAX_ERROR) {
		MAXIM_DBG("%s : Too many i2c failures, try again later...[%d] \n", __func__, cumulativeError);
		break;
	}
    }
	if(newArray != NULL) {
		free(newArray);
		newArray = NULL;
	}
	return xerr;
}

RET write_register(int32 nTuner, uint8 regindex, uint8 regval)
{
	uint8 data[2];
	uint8 readdata[2];
	RET xerr = eRET_OK;

	regArray[regindex] = regval;

	data[0] = (uint8)regindex;
	data[1] = (uint8)regArray[regindex] ;

	if(nTuner != 0 && regindex == 30) {
		data[1] |= 0x80;
	}

	if(nTuner == 0 && regindex == 56) {
		data[1] &= 0x1f;
		data[1] |= 0x20;
	}

	xerr = dev_tuner_reg_write(nTuner, data[0],&data[1],1);
//	usleep(500);
//	xerr |= dev_tuner_reg_read(nTuner, data[0], &readdata[1], 1);

	if(xerr!=eRET_OK){
		MAXIM_DBG("write_register: i2c write failed...\n");
		if(cumulativeError < 0xffffffff) {
			cumulativeError++;
		}
	}
	else {
		cumulativeError = 0;
	//	if(data[1] != readdata[1]) {
	//		MAXIM_DBG("Addr : %d(%xh), wdata : %xh , rdata : %xh <<<<<<<<<<<<<<<<<  write data and read data are different !!!! >>>>>>>>>>>>>>>>>>\n", data[0], data[0], data[1], readdata[1]);
	//	}
	}

	return xerr;
}

/// <summary>
/// Writes an array of bytes to the IC starting at the first register index: regindex.
/// </summary>
/// <param name="regindex">First register index to start writing at.</param>
/// <param name="regvalues">Array of bytes to write.</param>
void write_array(int32 nTuner, int32 regindex, uint8* regvalues, int32 length, int32 multiByteWrite)
{
	int32 i,j;
	RET xerr = eRET_OK;

	//multiByteWrite is where the serial sequence is sent all in one I2C write, for our debugging purposes
	//we wish to have an alternate method to send each register with address as opposed to sending in a burst.
	if (multiByteWrite){
		////Note: The right shifting of the address is because Maxim's UART uses a 7 bit addresses instead of 8 bits.

		xerr = dev_tuner_reg_write(nTuner, (uint8)regindex,regvalues,length);
		if(xerr!=eRET_OK){
			MAXIM_DBG("write_register: i2c write failed...\n");
		}
	}
	else
	{
		for (i = 0,j=regindex; i < length; i++,j++)
		{
			write_register(nTuner, j, regvalues[i]);
		}
	}
}


/// <summary>
/// Read a register from the IC at address regindex, return the value of that register.
/// The Index.  Used to read the entire valure of one register.
/// </summary>
/// <param name="regindex"></param>
/// <returns>Value read from the indexed register on the slave IC.</returns>
int32 read_register(int32 nTuner, uint8 regindex)
{
	uint8 dataRead = 0;
	uint32 bytesToRead = 1;
	RET xerr = eRET_OK;

	xerr = dev_tuner_reg_read(nTuner, (uint8)regindex, &dataRead, 1);
	if(xerr!=eRET_OK){
		MAXIM_DBG("read_register: i2c read failed...\n");
	}

	return dataRead;
}

/// <summary>
/// Overloaded ReadRegister.  This version takes just one argument.
/// The Index.  Used to read the entire value of one register.
/// </summary>
/// <param name="regindex"></param>
/// <returns>Value read from the indexed register on the slave IC.</returns>
uint8* read_registers(int32 nTuner, int32 regindex, int32 numberOfRegisters)
{
	uint8*  dataRead  = malloc(numberOfRegisters);
	if(dataRead == NULL)
		return NULL;

	RET xerr = eRET_OK;

	memset(dataRead,0x00,numberOfRegisters);

	xerr = dev_tuner_reg_read(nTuner, (uint8)regindex,dataRead,numberOfRegisters);
	if(xerr!=eRET_OK){
		MAXIM_DBG("read_register: i2c read failed...\n");
	}

	return dataRead;
}

RET read_tuner_registers(int32 nTuner, int32 addr, uint8 *datain, int32 numberOfRegisters)
{
	RET xerr = eRET_OK;

	if(datain == NULL)
		return eRET_NG_INVALID_PARAM;

	memset(datain, 0x00, numberOfRegisters);

	xerr = dev_tuner_reg_read((uint8)nTuner, (uint8)addr, datain,(uint32)numberOfRegisters);

	if(xerr!=eRET_OK){
		MAXIM_DBG("read_register: i2c read failed...\n");
	}

	return xerr;
}

/// <summary>
/// Sets a specific bit in a specified register high or low.
/// </summary>
/// <param name="regindex">Index to the register to set a bit in (0-255).</param>
/// <param name="bitindex">Index to the bit to set 0-7</param>
/// <param name="bitval">State to set the bit to 1=high, 0 = low.</param>
void set_bit(int32 nTuner, uint8 regindex, uint8 bitindex, uint8 bitval)
{
	int32 regval = 1 << bitindex;
	//First Set bit low at position: bitindex, if bitval = 0, you're done.
	regArray[regindex] = ((int)regArray[regindex]) & ~regval;
	//if bitval = 1, set the bit high at positon: bitindex.
	if (bitval == 1)
	{
		regArray[regindex] = ((int)regArray[regindex]) | regval;
	}
	BOOL resp = write_register(nTuner, regindex, regArray[regindex]);
	if (resp != eRET_OK)
	{
		//Error handling
		MAXIM_DBG("[%s:%d] Error!!!\n", __func__, __LINE__);
	}
}


/// <summary>
/// Sets a specific range of bits in a specified register to a specific value.
/// This function does not write the bits out to the serial bus but just sets the
/// register value.  upperbit must be > lowerbit.  The value must be able to fit
/// into the range defined by upperbit, lowerbit.   i.e. 15 can't go into 3 bits.
/// (set_bits(REG_NDIV,2,0,15) would overflow.
/// </summary>
/// <param name="regindex">Index to the register to set a bit in (0-255).</param>
/// <param name="upperbit">The index of the upper bit in the range (0-7).</param>
/// <param name="lowerbit">The index of the lower bit in the range (0-7).</param>
/// <param name="value">The value to insert into the bits referenced.</param>
void set_bits(int32 nTuner, uint8 regindex, uint8 upperbit, uint8 lowerbit, uint8 value)
{
	int32 mask = ((1 << (upperbit - lowerbit + 1)) - 1);
#if 0	// When you use, open.
	if (upperbit > 7)
	{
		//
	}

	if (value > mask)
	{
		//
	}
	if (lowerbit > upperbit)
	{
		//
	}
#endif
	mask = (mask << lowerbit);
	regArray[regindex] = (char)((regArray[regindex] & ~mask) | (value << lowerbit));

}



// <summary>
/// Sets a range of bits to a specified value in the Reg_Array array then sends the specified Array
/// member to the IC.
/// Example Usage: Register.Write_Register(6,3,15,2)
/// This would insert the value 15 into bits 6 through 3 in register 2 without
/// modifying the bits outside of that range:
///
///                           76543210    (bit numbers for reference)
///                           --------
/// Original register value:  10110010b
/// Value inserted:           01111000b
/// Resulting value:          11111010b   (15 inserted into bits 6 through 3)
/// </summary>
/// <param name="regindex">The Index of the register you wish to write to.</param>
/// <param name="UpperBit">The Upper bit of the range of bits to set.</param>
/// <param name="LowerBit">The Lower bit of the range of bits to set. </param>
/// <param name="value">The value to insert into the specified bit range.</param>
void write_bits(int32 nTuner, uint8 regindex, uint8 upperbit, uint8 lowerbit, uint8 value)
{
	int32 mask = ((1 << (upperbit - lowerbit + 1)) - 1);
#if 0	// When you use, open.
	if (upperbit > 7)
	{
		//
	}
	if (value > mask)
	{
		//
	}
	if (lowerbit > upperbit)
	{
		//
	}
#endif
	mask = (mask << lowerbit);
	regArray[regindex] = (char)((regArray[regindex] & ~mask) | (value << lowerbit));
	write_register(nTuner, regindex, regArray[regindex]);
	return;

}

BOOL write_bit(int32 nTuner, uint8 regindex, uint8 bitindex, uint8 bitvalue)
{
	int32 regval = 1 << bitindex;
	//First Set bit low at position: bitindex, if bitval = 0, you're done.
	regArray[regindex] = ((regArray[regindex]) & ~regval);
	//if bitval = 1, set the bit high at positon: bitindex.
	if (bitvalue == 1)
	{
		regArray[regindex] = ((regArray[regindex]) | regval);
	}
	BOOL resp = write_register(nTuner, regindex, regArray[regindex]);
	return resp;
}

/// <summary>
/// Reads a register variable in Reg_Array indexed by index and returns the value specified
/// by the bit range.
/// NOTE: This function does not read the IC but just the register variable!
///       See Read_Register if you wish to read the actual IC.
/// i.e.  Reg_Array[0] contains: '10010110b'  N = Read_Register(3,1,0)   N would = 6 after this call (bits 3 to 1).
/// </summary>
/// <param name="regindex">The Specific register to get a value from.</param>
/// <param name="upperbit">The Upper bit of the range of bits to get from the Indexed Register.</param>
/// <param name="lowerbit">The Lower bit of the range of bits to get from the Indexed Register.</param>
/// <returns></returns>
int32 get_bits(int32 nTuner, uint8 regindex, uint8 upperbit, uint8 lowerbit)
{
    //Create a mask where all bits upperbit through lowerbit are set high.
    int32 mask = ((1 << (upperbit - lowerbit + 1)) - 1);
#if 0	// When you use, open.
    if (upperbit > 7)
    {
        //
    }
    if (lowerbit > upperbit)
    {
        //
    }
#endif
    mask <<= lowerbit;
    int32 valueread = (regArray[regindex] & mask) >> lowerbit;
    return valueread;
}

/// <summary>
/// Reads a register variable in Reg_Array indexed by index and returns the bit value of the indexed bit.
/// NOTE: This function does not read the IC but just the register variable!
///       See Read_bit if you wish to read the actual IC.
/// </summary>
/// <param name="bitindex">The index of the bit you wish to retrieve the state of.</param>
/// <param name="regindex">The Specific register to get a value from.</param>
/// <returns>The indexed bit state</returns>
int32 get_bit(int32 nTuner, uint8 regindex, uint8 bitindex)
{
	int32 mask = (1 << bitindex);
#if 0	// When you use, open.
	if (bitindex > 7)
	{
		//
	}

	if (bitindex < 1)
	{
		//
	}
#endif
	int32 bitvalue = ((regArray[regindex] & mask) >> bitindex);

	return bitvalue;

}

/// <summary>
/// Reads a register (from the MAXIM IC) indexed by Index and returns the value specified
/// by the bit range.  See Write_Register for an explanation of bit ranges.
/// NOTE: This function does not read the IC but just the register variable!
///       See Read_Register if you wish to read the actual IC.
/// i.e.  The IC's Register 0 contains: '10010110b'  N = Read_Register(3,1,0)   N would = 6 after this call (bits 3 to 1).
/// </summary>
/// <param name="upperbit">The Upper bit of the range of bits to get from the Indexed Register.</param>
/// <param name="lowerbit">The Lower bit of the range of bits to get from the Indexed Register.</param>
/// <param name="regindex">The Specific register to get a value from.</param>
/// <returns></returns>
int32 read_bits(int32 nTuner, uint8 regindex, uint8 upperbit, uint8 lowerbit)
{
	int32 mask = ((1 << (upperbit - lowerbit + 1)) - 1);
#if 0	// When you use, open.
	if (upperbit > 7)
	{
		//error handler
	}
	if (lowerbit > upperbit)
	{
		//error handler
	}
#endif
	mask <<= lowerbit;
	int32 valueread = (read_register(nTuner, regindex) & mask) >> lowerbit;

	return valueread;
}


/// <summary>
/// Reads a register (from the MAXIM IC) indexed by Index and returns the a specific bit in the register.
/// by the bit range.  See Write_Register for an explanation of bit ranges.
/// NOTE: This function does not read the IC but just the register variable!
///       See Read_Register if you wish to read the actual IC.
/// i.e.  The IC's Register 0 contains: '10010110b'  N = Read_Register(3,1,0)   N would = 6 after this call (bits 3 to 1).
/// </summary>
/// <param name="regindex">The Upper bit of the range of bits to get from the Indexed Register.</param>
/// <param name="bitindex">The Lower bit of the range of bits to get from the Indexed Register.</param>
/// <returns>The value of the bit requested.</returns>
int32 read_bit(int32 nTuner, uint8 regindex, uint8 bitindex)
{
	int32 mask = 1 << bitindex;
#if 0	// When you use, open.
	if (bitindex > 7)
	{
		//error handler
	}
	if (bitindex < 0)
	{
		//error handler
	}
#endif
	int32 bitvalue = (read_register(nTuner, regindex) & mask) >> bitindex;

	return bitvalue;
}

