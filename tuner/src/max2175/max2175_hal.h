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
 * These are the constants used in MAX2175.c and register_functions.c
*/

#ifndef MAX2175_H
#define MAX2175_H

#ifdef __ANDROID__
#include <android/log.h>
#endif
#include <stdbool.h>
//#include "Maxim_IO.h"

//#include "MAX2175_api.h" //@AS
//#include "app_sys_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maxim Debug message control */
#define MAXIM_DEBUG
#ifdef __ANDROID__

#define MAXIM_TAG			("[RADIO][MAXIM]")
#define MAXIM_ERR(...)		(__android_log_print(ANDROID_LOG_ERROR,MAXIM_TAG, __VA_ARGS__))
#ifdef MAXIM_DEBUG
#define MAXIM_DBG(...)		(__android_log_print(ANDROID_LOG_DEBUG,MAXIM_TAG, __VA_ARGS__))
#else
#define	MAXIM_DBG(...)
#endif

#else // #ifdef __ANDROID__

#define MAXIM_ERR(...)		((void)printf("[ERROR][RADIO][MAXIM]: " __VA_ARGS__))
#ifdef MAXIM_DEBUG
#define MAXIM_DBG(...)		((void)printf("[DEBUG][RADIO][MAXIM]: " __VA_ARGS__))
#else
#define	MAXIM_DBG(...)
#endif

#endif // #ifdef __ANDROID__


#define max2175_mwait(X)	usleep((X)*(1000))

#define MAXIM_MAX_TUNER_IC_NUM		4

#define MAX_ERROR 	20

#define I2CADDRESS  0xc0

#define upper_range_err 2;
#define lower_range_err 3;
#define size_err 4;

//Register Name constants used to index the Reg_Array.
//Replaced with RA02 Names 1/26/2015
#define REG_CH_FSM_MODE  0
#define REG_NDIV  1
#define REG_RDIV_FRAC2  2
#define REG_FRAC1  3
#define REG_FRAC0  4
#define REG_BAND  5
#define REG_VCO  6
#define REG_NCO2  7
#define REG_NCO1  8
#define REG_NCO0  9
#define REG_ROPLL  10
#define REG_BANKS  11
#define REG_BBF_CTRL  12
#define REG_CTRL  13
#define REG_DET0  14
#define REG_DET1  15
#define REG_DET_HYST  16
#define REG_PGAAGC0  17
#define REG_PGAAGC1  18
#define REG_RFAGC0  19
#define REG_RFAGC1  20
#define REG_DAGC0  21
#define REG_DAGC1  22
#define REG_DAGC2  23
#define REG_DGCOMP_RFDLY  24
#define REG_DGCOMP_PGADLY  25
#define REG_DGCOMP_DLY  26
#define REG_DCOC0  27
#define REG_DCOC1  28
#define REG_I2S_CTRL0  29
#define REG_I2S_CTRL1  30
#define REG_I2S_CTRL2  31
#define REG_LTDET  32
#define REG_LTAGC0  33
#define REG_LTAGC1  34
#define REG_PLDET  35
#define REG_BBDCLIP_DET  36
#define REG_IQC0  37
#define REG_IQC1  38
#define REG_IQC2  39
#define REG_IQC_W0  40
#define REG_IQC_W1  41
#define REG_IQC_W2  42
#define REG_IQC_W3  43
#define REG_MANUAL_MODES  44
#define REG_HOLD_MODES  45
#define REG_PLD_NCO0  46
#define REG_PLD_NCO1  47
#define REG_DAGC_GAIN  48
#define REG_PGA_GAIN  49
#define REG_FM_RFGAIN  50
#define REG_AM_RFGAIN  51
#define REG_VHF_RFGAIN  52
#define REG_LB_RFGAIN  53
#define REG_PLL  54
#define REG_VAS  55
#define REG_ROM_ACCESS  56
#define REG_ROM_DATA  57
#define REG_ROM_READ  58
#define REG_VCO_STATUS  59
#define REG_VAS_STATUS  60
#define REG_AF_STATUS  61
#define REG_TSENSOR  62
#define REG_RFGAIN_STATUS  63
#define REG_DET_STATUS  64
#define REG_DAGC_STATUS  65
#define REG_SAT_STATUS0  66
#define REG_SAT_STATUS1  67
#define REG_PLD_PWR  68
#define REG_GEN_STATUS  69
#define REG_BIAS0  70
#define REG_BIAS1  71
#define REG_BIAS2  72
#define REG_BIAS3  73
#define REG_LDO0  74
#define REG_LDO1  75
#define REG_LDO2  76
#define REG_LDO3  77
#define REG_LDO4  78
#define REG_LDO5  79
#define REG_TRIM_AM  80
#define REG_TRIM_RO_TS  81
#define REG_TRIM_BIAS  82
#define REG_DFT  83
#define REG_CM_CTRL  84
#define REG_ENABLE0  85
#define REG_ENABLE1  86
#define REG_ENABLE2  87
#define REG_ENABLE3  88
#define REG_DCOC_DACI  89
#define REG_DCOC_DACQ  90
#define REG_DCOC_FFI0  91
#define REG_DCOC_FFI1  92
#define REG_DCOC_FFQ0  93
#define REG_DCOC_FFQ1  94
#define REG_AGC_DSPTH  95
#define REG_DAB_GAIN_OFFSET  96
#define REG_RESET0  97
#define REG_RESET1  98
#define REG_RESET2  99
#define REG_RESET3  100
#define REG_TEST0  101
#define REG_TEST1  102
#define REG_TEST2  103
#define REG_TEST3  104
#define REG_TEST4  105
#define REG_TEST5  106
#define REG_TEST6  107
#define REG_TEST7  108
#define REG_TEST8  109
#define REG_TEST9  110
#define REG_RFCTRL  111
#define REG_BBCTRL  112
#define REG_AFECTRL  113
#define REG_FIR_COEFF0  114
#define REG_FIR_COEFF1  115
#define REG_FIR_COEFF2  116
#define REG_FIR_COEFF_ADDR  117
#define REG_DSP_BANK0  118
#define REG_DSP_BANK1  119
#define REG_DSP_BANK2  120
#define REG_DSP_BANK3  121
#define REG_DSP_BANK4  122
#define REG_NOTCH0  123
#define REG_NOTCH1  124
#define REG_NOTCH2  125
#define REG_LVDS0  126
#define REG_LVDS1  127
#define REG_LVDS2  128
#define REG_LVDS3  129
#define REG_LVDS4  130
#define REG_LVDS5  131
#define REG_LVDS6  132
#define REG_LVDS7  133
#define REG_LVDS8  134
#define REG_LVDS9  135
#define REG_LVDS10  136
#define REG_LVDS11  137
#define REG_LVDS12  138
#define REG_LVDS13  139
#define REG_LVDS14  140
#define REG_LVDS15  141
#define REG_LVDS16  142
#define REG_LVDS17  143
#define REG_LVDS18  144
#define REG_LVDS19  145
#define REG_ADC0  146
#define REG_ADC1  147
#define REG_ADC2  148
#define REG_ADC3  149
#define REG_ADC4  150
#define REG_ADC5  151
#define REG_ADC6  152
#define REG_ADC7  153
#define REG_ADC8  154
#define REG_ADC9  155
#define REG_ADC10  156
#define REG_ADC11  157
#define REG_ADC12  158
#define REG_ADC13  159
#define REG_ADC14  160
#define REG_ADC15  161
#define REG_ADC16  162
#define REG_ADC17  163
#define REG_ADC18  164
#define REG_ADC19  165
#define REG_ADC20  166
#define REG_ADC21  167
#define REG_ADC22  168
#define REG_ADC23  169
#define REG_ADC24  170
#define REG_ADC25  171
#define REG_ADC26  172
#define REG_ADC27  173
#define REG_ADC28  174
#define REG_ADC29  175
#define REG_ADC30  176
#define REG_ADC31  177
#define REG_ADC32  178
#define REG_ADC33  179
#define REG_ADC34  180
#define REG_ADC35  181
#define REG_ADC36  182
#define REG_ADC37  183
#define REG_ADC38  184
#define REG_ADC39  185
#define REG_ADC40  186
#define REG_ADC41  187
#define REG_ADC42  188
#define REG_ADC43  189
#define REG_ADC44  190
#define REG_ADC45  191
#define REG_ADC46  192
#define REG_ADC47  193
#define REG_ADC48  194
#define REG_ADC49  195
#define REG_ADC50  196
#define REG_ADC51  197
#define REG_ADC52  198
#define REG_ADC53  199
#define REG_ADC54  200
#define REG_ADC55  201
#define REG_ADC56  202
#define REG_ADC57  203
#define REG_ADC58  204
#define REG_ADC59  205
#define REG_ADC60  206
#define REG_ADC61  207
#define REG_ADC62  208
#define REG_ADC63  209
#define REG_ADC64  210
#define REG_ADC65  211
#define REG_ADC66  212
#define REG_ADC67  213
#define REG_ADC68  214
#define REG_ADC69  215
#define REG_ADC70  216
#define REG_ADC71  217
#define REG_ADC72  218
#define REG_ADC73  219
#define REG_ADC74  220
#define REG_ADC75  221
#define REG_ADC76  222
#define REG_ADC77  223
#define REG_ADC78  224
#define REG_ADC79  225
#define REG_ADC80  226
#define REG_ADC81  227
#define REG_ADC82  228
#define REG_ADC83  229
#define REG_ADC84  230
#define REG_ADC85  231
#define REG_ADC86  232
#define REG_ADC87  233
#define REG_ADC88  234
#define REG_ADC89  235
#define REG_ADC90  236
#define REG_ADC91  237
#define REG_ADC92  238
#define REG_ADC93  239
#define REG_ADC94  240
#define REG_ADC95  241
#define REG_ADC96  242
#define REG_ADC97  243
#define REG_ADC98  244
#define REG_ADC99  245
#define REG_ADC100  246
#define REG_ADC101  247
#define REG_ADC102  248
#define REG_ADC103  249
#define REG_ADC104  250
#define REG_ADC105  251
#define REG_ADC106  252
#define REG_ADC107  253
#define REG_ADC108  254
#define REG_ADC109  255

#define CH_MSEL  0
#define EQ_MSEL  1
#define AA_MSEL  2   //Added for RA02

//Mode Constants, each constant has a corresponding register table that
//can be loaded by calling receive_modes().  NA sets are for North America.
//EU sets are for Europe.  Added RA02 Value 1/26/16
typedef enum
{
	FM_EU_1p0, FM_EU_1p1, FM_EU_1p2, FM_EU_1p3, FM_EU_1p7, FM_HD_4p0,
	FM_HD_4p3, FM_HD_4p7, FM_HD_5p0, FM_HD_5p1, FM_HD_5p2, FM_EU_2p0,
	FM_EU_2p1, FM_EU_2p2, AM_EU_1p0, AM_EU_1p1, AM_EU_1p2, AM_EU_1p3,
	AM_EU_1p7, AM_EU_2p0, AM_EU_2p1, AM_EU_2p2, MWAM_EU_1, MWAM_EU_2,
	SWAM_0, SWAM_1, SWAM_2, SWAM_3, SWAM_4, DAB_1_VHF, DAB_1_LB, DAB_2,
	DAB_1p1, DAB_1p2, DAB_1p3, DAB_1p7, FM_NA_1p0, FM_NA_1p1, FM_NA_1p2,
	FM_NA_2p0, FM_NA_3p0, FM_NA_3p1, WX_2p0, WX_1p0, WX_1p1, WX_1p2,
	FM_HD_1p0, FM_HD_1p1, FM_HD_1p2, FM_HD_3p0, FM_HD_3p1, AM_NA_1p0,
	AM_NA_1p1, AM_NA_1p2, AM_NA_2p0, AM_NA_3p0, AM_NA_3p1, MWAM_NA_1,
	FM_EU_1p0_FULL, FM_NA_1p0_FULL, UNKNOWN

} Receive_Modes;

/* Enumeration for io modes. */
typedef enum {
	IO_I2S = 0,    //I2S transmitter.
	IO_LVDS = 1     //JESD204B compliant transmitter
} IO_Select_Modes;



/* Enumeration for HSLS function. */
typedef enum
{
	LO_BELOW_DESIRED = 0,
	LO_ABOVE_DESIRED = 1
} HSLS;



/*
FSM Mode bit value enumeration - Used for MODE bits in register 0 (CH_FSM_MODE).
*/

typedef enum
{
	MODE_LOAD_TO_BUFFER,
	MODE_PRESET_TUNE,
	MODE_SEARCH,
	MODE_AF_UPDATE,
	MODE_JUMP_FAST_TUNE,
	MODE_CHECK,
	MODE_LOAD_AND_SWAP,
	MODE_END,
	MODE_BUFFER_PLUS_PRESET_TUNE,
	MODE_BUFFER_PLUS_SEARCH,
	MODE_BUFFER_PLUS_AF_UPDATE,
	MODE_BUFFER_PLUS_JUMP_FAST_TUNE,
	MODE_BUFFER_PLUS_CHECK,
	MODE_BUFFER_PLUS_LOAD_AND_SWAP,
	MODE_NO_ACTION,
	MODE_CURRENT_ACTION
} efsm_mode;

#if 0
/* Band Constants */
typedef enum
{
	AM = 0,
	FM = 1,
	VHF = 2,
	LBAND = 3
} eBand;
#endif

//typedef enum { EUROPE, NORTH_AMERICA } Region;
typedef enum Region { EUROPE, NORTH_AMERICA } Region_t;


//The MAX2175 has two possible I2C addresses based on the state of the ADDR pin.
typedef enum I2C_Addresses { C0 = 0xC0, C2 = 0xC2, C4 = 0xC4, C6 = 0xC6 }I2C_Addresses_t;


#define FIR_CHANNEL 0     /* Program Channel Filter Coefficient bank. */
#define FIR_EQ 1          /* Program EQ Filter Coefficient bank. */



#define bit_am_lo_en 7

#define  RSTB_DSP 7
#define  RSTB_PM  2

#define rstb_adc_all 1  //Reset ADC bit.

/* Bits used in setting the AM_HIZ_IN mode  */
#define AM_HIZ_IN 5
#define DCOC_DAC_2XGAIN 7
#define AMLNA_BIAS_MSB 1
#define AMLNA_BIAS_LSB 0
#define AM_LNA_IIP2_TRIM_MSB 5
#define AM_LNA_IIP2_TRIM_LSB 0


#define SEPARATE
#define DAGC
//#define FR_40MHZ


unsigned char regArray[256];


double Decim_Ratio_beforeNCO;

/* Variables to keep the BBF_BW values read from ROM in.  See receive_modes(), and Load_ROM() */
unsigned char ROM_BBF_BW_AM;
unsigned char ROM_BBF_BW_FM;
unsigned char ROM_BBF_BW_DAB;

/* Register Specific Functions */
BOOL write_register(int nTuner, unsigned char regindex, unsigned char regval);
RET write_registers(int nTuner, unsigned char NumRegisters, unsigned char StartingIndex);
int read_register(int nTuner, unsigned char regindex);
int read_bits(int nTuner, unsigned char regindex, unsigned char upperbit, unsigned char lowerbit);
int read_bit(int nTuner, unsigned char regindex, unsigned char bitindex);
BOOL write_bit(int nTuner, unsigned char regindex, unsigned char bitindex, unsigned char bitvalue);
void write_bits(int nTuner, unsigned char regindex, unsigned char upperbit, unsigned char lowerbit, unsigned char value);
void set_bits(int nTuner, unsigned char regindex, unsigned char upperbit, unsigned char lowerbit, unsigned char value);
void set_bit(int nTuner, unsigned char regindex, unsigned char bitindex, unsigned char bitvalue);
int get_bits(int nTuner, unsigned char regindex, unsigned char upperbit, unsigned char lowerbit);
int get_bit(int nTuner, unsigned char regindex, unsigned char bitindex);
bool wait_while_ch_fsm_busy(int nTuner);

void LoadADC_Registers(int nTuner);
void Load_FM_EU_1p0_FULL(int nTuner);
void Load_FM_NA_1p0_FULL(int nTuner);
void Load_FM_EU_1p0(int nTuner);
void Load_FM_EU_1p1(int nTuner);
void Load_FM_EU_1p2(int nTuner);
void Load_FM_EU_1p3(int nTuner);
void Load_FM_EU_1p7(int nTuner);
void Load_FM_HD_4p0(int nTuner);
void Load_FM_HD_4p3(int nTuner);
void Load_FM_HD_4p7(int nTuner);
void Load_FM_HD_5p0(int nTuner);
void Load_FM_HD_5p1(int nTuner);
void Load_FM_HD_5p2(int nTuner);
void Load_FM_EU_2p0(int nTuner);
void Load_FM_EU_2p1(int nTuner);
void Load_FM_EU_2p2(int nTuner);
void Load_AM_EU_1p0(int nTuner);
void Load_AM_EU_1p1(int nTuner);
void Load_AM_EU_1p2(int nTuner);
void Load_AM_EU_1p3(int nTuner);
void Load_AM_EU_1p7(int nTuner);
void Load_AM_EU_2p0(int nTuner);
void Load_AM_EU_2p1(int nTuner);
void Load_AM_EU_2p2(int nTuner);
void Load_MWAM_EU_1(int nTuner);
void Load_MWAM_EU_2(int nTuner);
void Load_SWAM_0(int nTuner);
void Load_SWAM_1(int nTuner);
void Load_SWAM_2(int nTuner);
void Load_SWAM_3(int nTuner);
void Load_SWAM_4(int nTuner);
void Load_DAB_1_VHF(int nTuner);
void Load_DAB_1_LB(int nTuner);
void Load_DAB_2(int nTuner);
void Load_DAB_1p1(int nTuner);
void Load_DAB_1p2(int nTuner);
void Load_DAB_1p3(int nTuner);
void Load_DAB_1p7(int nTuner);
void Load_FM_NA_1p0(int nTuner);
void Load_FM_NA_1p1(int nTuner);
void Load_FM_NA_1p2(int nTuner);
void Load_FM_NA_2p0(int nTuner);
void Load_FM_NA_3p0(int nTuner);
void Load_FM_NA_3p1(int nTuner);
void Load_WX_2p0(int nTuner);
void Load_WX_1p0(int nTuner);
void Load_WX_1p1(int nTuner);
void Load_WX_1p2(int nTuner);
void Load_FM_HD_1p0(int nTuner);
void Load_FM_HD_1p1(int nTuner);
void Load_FM_HD_1p2(int nTuner);
void Load_FM_HD_3p0(int nTuner);
void Load_FM_HD_3p1(int nTuner);
void Load_AM_NA_1p0(int nTuner);
void Load_AM_NA_1p1(int nTuner);
void Load_AM_NA_1p2(int nTuner);
void Load_AM_NA_2p0(int nTuner);
void Load_AM_NA_3p0(int nTuner);
void Load_AM_NA_3p1(int nTuner);
void Load_MWAM_NA_1(int nTuner);
long *Ch_Coeff_CH_FMEU();
long *Ch_Coeff_CH_AM();
long *Ch_Coeff_CH_MWAM();
long *Ch_Coeff_CH_DAB1();
long *Ch_Coeff_CH_DAB2();
long *Ch_Coeff_CH_FMNA();
long *Ch_Coeff_CH_FMHD();
//long *EQ_Coeff_EQ_FMEU1_RA02_m6dB();
long *EQ_Coeff_EQ_FMEU1();
//long *EQ_Coeff_EQ_FMHD4_RA02();
long *EQ_Coeff_EQ_FMHD4();
long *EQ_Coeff_EQ_FMHD5_RA02();
long *EQ_Coeff_EQ_FMEU2();
long *EQ_Coeff_EQ_FMNA1();
long *EQ_Coeff_EQ_FMNA2();
long *EQ_Coeff_EQ_FMNA30();
long *EQ_Coeff_EQ_FMNA31();
long *EQ_Coeff_EQ_FMHD1();
long *EQ_Coeff_EQ_FMHD30();
long *EQ_Coeff_EQ_FMHD31();
long *AA_Coeff_AA_FMHD();
long *AA_Coeff_AA_AM();
long *AA_Coeff_AA_MWAM();


Receive_Modes Current_Receive_Mode;   //The currently selected receive mode.
Region_t myRegion;



bool alias;
extern unsigned int cumulativeError;
void receive_mode(int nTuner, Receive_Modes rmode);
void load_from_ROM(int nTuner);
unsigned char read_rom(int nTuner, unsigned char rowindex);
unsigned char init_power_manager(int nTuner);
void program_bb_filter(int nTuner);
bool wait_for_pm_done(int nTuner);
void mswait(int ms);
void io_select(int nTuner, IO_Select_Modes iomode);
void LO_HSLS(int nTuner, HSLS LO_Pos);
//void init(Region area, double XtalFrequency);
void init(int nTuner, Region_t area, double XtalFrequency);
void Load_Filter_Coefficients_1bank(int nTuner, unsigned char M_SEL, unsigned char filter_bank, long *Coef_Array);
void LO_frequency(int nTuner, double desired_lo_frequency);
void perform_channel_FSM_action(int nTuner, efsm_mode FSM_Action);
bool set_fsm_mode(int nTuner, efsm_mode new_mode);
bool fsm_mode(int nTuner, efsm_mode new_mode);
void set_RF_frequency(int nTuner, double freq);
void RF_frequency(int nTuner, double desiredfrequency);
void NCO_frequency(int nTuner, double nco_frequency_desired);
double f_comparison(int nTuner);
double read_DAGC_Gain(int nTuner);
double read_DAGC_PWR(int nTuner);
double read_RSSI(int nTuner);


int MAX2175_get_rssi(int nTuner, unsigned char mod_mode);
void dump_all_reg_val(int nTuner);
int read_rssi(int nTuner);

////////////////////////////////////////////////////////////////////////


void init_tuner_MAX2175(int nTuner, Region_t area);
void Goto_RF_Frequency_MAX2175(int nTuner, double freq);
void NCO_Frequency(int nTuner, double nco_frequency_desired);
BOOL mode(int nTuner, efsm_mode new_mode);
void LO_Frequency(int nTuner, double desired_lo_frequency);
void Perform_Channel_FSM_Action(int nTuner, efsm_mode FSM_Action);
void Set_frequency_with_FSM(int nTuner, double freq, efsm_mode action);

void *max2175_memset(void *pvdst, uint8 ubch, uint32 uilen);
void *max2175_memcpy(void *pvDst, void *pvSrc, uint16 uiLength);

#ifdef __cplusplus
}
#endif

#endif
