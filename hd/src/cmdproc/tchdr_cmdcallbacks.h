/*******************************************************************************
*
* (C) copyright 2003-2014, iBiquity Digital Corporation, U.S.A.
*
********************************************************************************

    This confidential and proprietary software may be used only as
    authorized by a licensing agreement from iBiquity Digital Corporation.
    In the event of publication, the following notice is applicable:

    The availability of this material does not provide any license
    by implication, or otherwise under any patent rights of iBiquity
    Digital Corporation or others covering any use of the
    contents herein.

    Any copies or derivative works must include this and all other
    proprietary notices.

        iBiquity Digital Corporation
        6711 Columbia Gateway Drive, Suite 500
        Columbia, MD USA 21046
*******************************************************************************/
/**
 * file	: cmdCallbacks.h
 * brief
 */
#ifndef CMD_CALLBACKS_H_
#define CMD_CALLBACKS_H_

#include "hdrAudio.h"
#include "hdrBlend.h"

#ifdef USE_HDRLIB_2ND_CHG_VER
typedef union PACKED_STRUCTURE{
    struct PACKED_STRUCTURE{
        U8     digital_fm_1:   1;
        U8     digital_am_1:   1;
        U8     analog_fm:      1;
        U8     analog_am:      1;
        U8     digital_fm_2:   1;
        U8     digital_am_2:   1;
        U8     digital_fm_3:   1;
        U8     digital_am_3:   1;
    }bit;
	U8 all;
} RX_SW_Cnfg_Byte0;

typedef union PACKED_STRUCTURE{
    struct PACKED_STRUCTURE{
        U8     res1:           1;
        U8     res2:           1;
        U8     res3:          	1;
        U8     res4:           1;
        U8     aux1_audio_ip:  1;
        U8     aux2_audio_ip:  1;
        U8     digital_fm_4:   1;
        U8     digital_am_4:   1;
    }bit;
    U8 all;
} RX_SW_Cnfg_Byte1;

typedef union PACKED_STRUCTURE{
    struct PACKED_STRUCTURE{
        U8     inst1_cap_0:    1;
        U8     inst1_cap_1:    1;
        U8     inst2_cap_0:    1;
        U8     inst2_cap_1:    1;
        U8     inst3_cap_0:    1;
        U8     inst3_cap_1:    1;
        U8     cond_access:    1;
        U8     res1:           1;
    }bit;
    U8 all;
} RX_SW_Cnfg_Byte2;

typedef union PACKED_STRUCTURE{
    struct PACKED_STRUCTURE{
        U8     psd_decode:     1;
        U8     blend_on_chip:  1;
        U8     tagging:        1;
        U8     emerg_alerts:   1;
        U8     lot_offchip:    1;
        U8     auto_alignment: 1;
        U8     inst4_cap_0:    1;
        U8     inst4_cap_1:    1;
    }bit;
    U8 all;
} RX_SW_Cnfg_Byte3;

#else    // #ifdef USE_HDRLIB_3RD_CHG_VER

/**
 * 2206 Figure 5-4: RX_SW_Cnfg Bitmap:
 * - Sys_Cntrl_Cnfg (0x83) -> Get_Supported_Services (0x03)
 * - Sys_Cntrl_Cnfg (0x83) -> Set_Activated_Services (0x02)
 */
typedef union PACKED_STRUCTURE{
    struct PACKED_STRUCTURE{
        U8     digital_fm_1:   1;
        U8     digital_am_1:   1;
        U8     analog_fm:      1;
        U8     digital_fm_2:   1;
        U8     digital_am_2:   1;
        U8     digital_fm_3:   1;
        U8     digital_am_3:   1;
    }bit;
	U8 all;
} RX_SW_Cnfg_Byte0;

typedef union PACKED_STRUCTURE{
    struct PACKED_STRUCTURE{
        U8     rbds:           1;
        U8     rbds_decode:    1;
        U8     aim:           	1;
        U8     res2:           1;
        U8     aux1_audio_ip:  1;
        U8     res3:           1;
        U8     res4:           1;
        U8     res5:           1;
    }bit;
    U8 all;
} RX_SW_Cnfg_Byte1;

typedef union PACKED_STRUCTURE{
    struct PACKED_STRUCTURE{
        U8     inst1_cap_0:    1;
        U8     inst1_cap_1:    1;
        U8     inst2_cap_0:    1;
        U8     inst2_cap_1:    1;
        U8     inst3_cap_0:    1;
        U8     inst3_cap_1:    1;
        U8     cond_access:    1;
        U8     res1:           1;
    }bit;
    U8 all;
} RX_SW_Cnfg_Byte2;

typedef union PACKED_STRUCTURE{
    struct PACKED_STRUCTURE{
        U8     psd_decode:     1;
        U8     res1:           1;
        U8     tagging:        1;
        U8     active_radio:   1;
        U8     lot_offchip:    1;
        U8     auto_alignment: 1;
        U8     res3:           1;
        U8     res4:           1;
    }bit;
    U8 all;
} RX_SW_Cnfg_Byte3;
#endif

typedef struct PACKED_STRUCTURE BBP_services_t
{
    RX_SW_Cnfg_Byte0    byte0;
    RX_SW_Cnfg_Byte1    byte1;
    RX_SW_Cnfg_Byte2    byte2;
    RX_SW_Cnfg_Byte3    byte3;
}BBP_services_t;

typedef struct{
    HDR_tune_band_t band;
    U16 rfFreq;
}BBP_tune_select_t;

typedef struct {
    U32 fmMinTuningFreq;
    U32 fmMaxTuningFreq;
    U32 fmFreqSpacing;
    U32 amMinTuningFreq;
    U32 amMaxTuningFreq;
    U32 amFreqSpacing;
}BBP_tune_params_t;

typedef struct {
    /**
     * Used to set the digital signal quality seek threshold.
     */
     U32 dsqm_seek_threshold;
}BBP_iboc_config_t;

/**
 * Ref: 2206 Table 5-12: Configuration Information Format:
 * - Sys_Cntrl_Cnfg (0x83 -> Get_Cnfg_Info(0x08)
 */
typedef struct PACKED_STRUCTURE
{
    U8 month;
    U8 day;
    U8 year;
    U8 hour;
    U8 minute;
    S8 text[59];
}BBP_sys_config_info_t;

typedef enum{
    ACTIVE  = 0x0,
    DEFAULT = 0x1,
    CUSTOM  = 0x2,
    NUM_CONFIGS
}BBP_config_select_t;

/**
 * enums represent Offsets in Table 9-1
 */
typedef enum {
    SPDIF_ENABLE		= 0x27,
    AWSx256_ENABLE		= 0x28,
    I2S_MSTRSLV_CNTRL	= 0x3E,
    UART1_CLK_RATE		= 0x46,
    UART2_CLK_RATE		= 0x4A
}BBP_sys_config_param_t;

/**
 * Table 9-1: System Level Configuration Parameters Sys_Cntrl_Cnfg Command (0x83)For Tuner Instance #1
 * This structure implements Table 9-1  - the contents of the data fields of the logical messages associated
 * with the system-level configuration parameters. aligned everything on 32-bit boundary
 * System Configuration Parameters - xxx240 Bytes
 * These parameters are associated with the following function codes:
 * - Sys_Cntrl_Cnfg -> Get_Sys_Config (0x09),
 * - Sys_Cntrl_Cnfg -> Set_Sys_Config (0x0A),
 * - Sys_Cntrl_Cnfg -> Set_Sys_Cnfg_Param (0x0C),
 * - Sys_Cntrl_Cnfg -> Get_Sys_Cnfg_Param (0x0D)
 * These parameters may only be modified during idle mode (before receiving Band_Select or Sys_Tune
 * commands at start up.
 * Reserved Parameters should not be modified at any time, and their values should not be set to 0x00.
 */
typedef struct PACKED_STRUCTURE
{
    //***************************************************************************************************
    // PART 1 - 0x83 - General System Configuration Parameters - 122 Bytes
    //***************************************************************************************************
    // configuration state
    U32 state;                     // Offset 0x00 -> Reserved
    // configuration storage device
    // flash  : 0x10
    // eeprom : 0x20
    U8 storage_device;                    // Offset 0x04 -> Storage Device Type
    U8 flash_size;                        // Offset 0x05 -> Flash Memory Size
    U8 eeprom_size;                       // Offset 0x06 -> EEPROM Memory Size
    U8 storage_device_installed;          // Offset 0x07 -> Reserved
    BBP_services_t supported_services;      // Offset 0x08 -> Services Supported
    BBP_services_t activated_services;      // Offset 0x0C -> Services Activated
    U16 versionUpper;            // Offset 0x10 -> Reserved
    U16 versionLower;            // Offset 0x12 -> Reserved
    // I2S / baseband input
    U8 bb_src_mode;                       // Offset 0x14 -> Baseband Sample Rate
    U8 bb_src_mode_alternate;             // Offset 0x15 -> Alternate Baseband Sample Rate
    U8 bb_src_flags;                      // Offset 0x16 -> SRC Switching Mode
    U8 bb_src_reserved_0;                 // Offset 0x17 -> Reserved
    U8 bb_iq_swap;                        // Offset 0x18 -> Reserved
    U8 bb_iq_negate;                      // Offset 0x19 -> IQ Negate
    U8 bb_iq_shift;                       // Offset 0x1A -> IQ Shift
    U8 bb_flip_msb;                       // Offset 0x1B -> MSB Invert
    U8 bb_fe_enable;                      // Offset 0x1C -> Reserved
    U8 bb_iso_ctr_bypass;                 // Offset 0x1D -> Reserved
    U8 bb_iso_sb_bypass;                  // Offset 0x1E -> Reserved
    U8 bb_input_mode;                     // Offset 0x1F -> IQ Multiplex
    U8 bb_software_gain_ctrl;             // Offset 0x20 -> Reserved
    U8 bb_iq_shift_bit_left;              // Offset 0x21 -> IQ Shift Direction
    U8 bb_iq_dig_shift_bit;               // Offset 0x22 -> IQ Digital Shift Bits
    U8 bb_iq_anlg_shift_bit;              // Offset 0x23 -> IQ Analog Shift Bits
    // isolation filter
    U8 bb_iso_bands_enable;               // Offset 0x24 -> Reserved
    U8 bb_iso_reserved_0;                 // Offset 0x25 -> Reserved
    U8 bb_fe_swap;                        // Offset 0x26 -> Baseband Input Port
    U8 antenna_diversity;                 // Offset 0x27 -> Digital Antenna Diversity
    U16 tuner_hw;                         // Offset 0x28 -> Reserved
    U16 saturn_hw;                        // Offset 0x2A -> Reserved
    U16 board_id;                         // Offset 0x2C -> Reserved
    U8  hw_reserved_0;                    // Offset 0x2E -> Reserved
    U8  hw_reserved_1;                    // Offset 0x2F -> Reserved
    U8 ahb_clk;                           // Offset 0x30 -> Reserved
    U8 pclk;                              // Offset 0x31 -> Reserved
    U8 clk_half_rate;                     // Offset 0x32 -> Reserved
    U8 clk_reserved1;                     // Offset 0x33 -> Reserved
    U32 clk_reserved_0;                   // Offset 0x34 -> Reserved
    // audio interface select
    U8 i2s_data_out_select;                // Offset 0x38 -> I2S Audio Out Select
    U8 i2s_aes_spdif_on;                   // Offset 0x39 -> S/PDIF Enable
    U8 i2s_awsx256_on;                     // Offset 0x3A -> I2S AWSx256 Enable
    U8 i2s_abck_freq;                      // Offset 0x3B -> I2S Bit Clock Frequency
    U8 i2s_adapt_data_out_enable;          // Offset 0x3C -> I2S Audio Data Out Enable
    U8 i2s_32bit_wr_enable;                // Offset 0x3D -> Reserved
    U8 i2s_input_mode;                     // Offset 0x3E -> I2S Master/Slave Mode Control (Byte 1)
    U8 i2s_output_mode;                    // Offset 0x3F -> I2S Master/Slave Mode Control (Byte 2)
    U8 i2s_input_dst;                      // Offset 0x40 -> I2S Master/Slave Mode Control (Byte 3)
    U8 i2s_output_src;                     // Offset 0x41 -> I2S Master/Slave Mode Control (Byte 4)
    U8 i2s_audio_output_mode;              // Offset 0x42 -> I2S Audio Output Mode
    U8 asrc_mode;                          // Offset 0x43 -> Audio Output Re-sample Rate

    // Peripherals
    U8 iic_1_rate;                         // Offset 0x44 -> I2C 1 Clock Rate
    U8 iic_1_use_dma;                      // Offset 0x45 -> Reserved
    U8 iic_1_slave_addr;                   // Offset 0x46 -> I2C 1 slave address - 7 bits
    U8 iic_1_reserved_1;                   // Offset 0x47 -> Reserved

    U8 iic_2_rate;                         // Offset 0x48 -> I2C 2 Clock Rate
    U8 iic_2_use_dma;                      // Offset 0x49 -> Reserved
    U8 iic_2_slave_addr;                   // Offset 0x4A -> I2C 2 slave address - 7 bits
    U8 iic_2_reserved_1;                   // 0ffset 0x4B -> Reserved

    U8 spi_1_rate;                         // Offset 0x4C -> SPI 1 Clock Rate
    U8 spi_1_config;                       // Offset 0x4D -> SPI 1 Configuration
    U8 spi_1_use_rx_dma;                   // Offset 0x4E -> Reserved
    U8 spi_1_use_tx_dma;                   // Offset 0x4F -> Reserved

    U8 spi_2_rate;               // Offset 0x50 -> SPI 2 Clock Rate
    U8 spi_2_config;             // Offset 0x51 -> SPI 2 Configuration
    U8 spi_2_use_dma;            // Offset 0x52 -> Reserved
    U8 spi_2_reserved_0;         // Offset 0x53 -> Reserved

    U8 spi_3_rate;               // Offset 0x54 -> SPI 3 Clock Rate
    U8 spi_3_config;             // Offset 0x55 -> SPI 3 Configuration
    U8 spi_3_use_dma;            // Offset 0x56 -> Reserved
    U8 spi_3_reserved_0;         // Offset 0x57 -> Reserved

    U8 uart_1_rate;              // Offset 0x58 -> UART 1 Clock Rate
    U8 uart_1_config;            // Offset 0x59 -> UART 1 Configuration
    U8 uart_1_use_dma;           // Offset 0x5A -> Reserved
    U8 uart_1_cmd_proc;          // Offset 0x5B -> Reserved

    U8 uart_2_rate;              // Offset 0x5C -> UART 2 Clock Rate
    U8 uart_2_config;            // Offset 0x5D -> UART 2 Configuration
    U8 uart_2_use_dma;           // Offset 0x5E -> Reserved
    U8 uart_2_cmd_proc;          // Offset 0x5F -> Reserved
   // drive strength controls
    U32 drive_strength_ctrl;       // Offset 0x60 -> Reserved
    // watchdog timer
    U16 wdt_timeout_range;       // Offset 0x64 -> Reserved
                                            // timeout range = 2^(16 + sysConfigHandle->wdt_timeout_range)
    S8 wdt_enable;                        // Offset 0x66 -> Reserved - watchdog enable
    S8 wdt_reserved;                      // Offset 0x67 -> Reserved
    // clock gating enable/disable bitmap
    // bit[0] - isolation filter
    // bit[1] - general purpose timer #1
    // bit[2] - general purpose timer #2
    // bit[3] - uart #1
    // bit[4] - uart #2
    // bit[5] - iic #1
    // bit[6] - iic #2
    // bit[7] - spi #1
    // bit[8] - spi #2
    // bit[9] - spi #3
    // bit[10] - audio input
    // bit[11] - audio output
    // bit[12] - audio resampler
    // bit[13] - audio tracking
    // bit[14] - aes (spdif out)
    // bit[15] - aws x 256 (D/A)
    // bit[16] - Viterbi
    // bit[17] - Watchdog
    U32 clk_gating_cntrl_bitmap;   // Offset 0x68 -> Reserved

    // peripheral reset bitmap
    // software controllable resets on peripheral bus
    // bit[0] - isolaiton filter
    // bit[1] - general purpose timer #1
    // bit[2] - general purpose timer #2
    // bit[3] - uart #1
    // bit[4] - uart #2
    // bit[5] - iic #1
    // bit[6] - iic #2
    // bit[7] - spi #1
    // bit[8] - spi #2
    // bit[9] - spi #3
    // bit[10] - audio input
    // bit[11] - audio output
    // bit[12] - audio resampler
    // bit[13] - base band input
    // bit[14] - audio tracking
    // bit[15] - AES
    // bit[16] - SDRam
    // bit[17] - Vectra
    // bit[18] - Viterbi
    // bit[19] - Watchdog
    U32 reset_bitmap;              // Offset 0x6C -> Reserved

    U8 hd_blend_to;                // Offset 0x70 -> Blend-To-Source Select
    U8 hd_transition_time;         // Offset 0x71 -> Blend Transition Time
    U8 hd_sps_mute_time;           // Offset 0x72 -> Reserved
    U8 hd_sps_direct_tune_enable;  // Offset 0x73 -> Reserved

    U8 flash_sector_size;          // Offset 0x74 -> Reserved - 4k or 64k
    U8 tune_list_num_checks;       // Offset 0x75 -> Reserved
    U8 tune_list_initial_wait;     // Offset 0x76 -> Reserved - time to wait before initial check of hmi data
    U8 tune_list_update_interval;  // Offset 0x77 -> Reserved - check for hmi updates once every x seconds
    U16 tune_list_write_interval;  // Offset 0x78 -> Reserved - write hmi to flash/eeprom once every x mins

    //***************************************************************************************************
    // PART 2 - 0x83 - System Config Params Applicable only to BBP Chips with Integrated Tuner Control - 118 Bytes
    //***************************************************************************************************
    U16 tune_list_reserved1;       // Offset 0x7A -> Reserved
    U16 validFMThreshold;          // Offset 0x7C -> Distance FM Seek Threshold (bytes 125-126)
    U16 invalidFMThreshold;        // Offset 0x7E -> Reserved (bytes 127-128)
    U16 validAMThreshold;          // Offset 0x80 -> Distance AM Seek Threshold(bytes 129-130)
    U16 invalidAMThreshold;        // Offset 0x82 -> Reserved (bytes 131-132)
    S16 validFMRFThreshold;               // Offset 0x84 -> Distance FM RF Level Threshold (bytes 133-134)
    S16 validAMRFThreshold;               // Offset 0x86 -> Distance AM RF Level Threshold (bytes 135-136)

    U16 validLocalFMThreshold;     // Offset 0x88 -> Local FM Seek Threshold (bytes 137-138)
    U16 validLocalAMThreshold;     // Offset 0x8A -> Local AM Seek Threshold (bytes 139-140)
    S16 validLocalFMRFThreshold;          // Offset 0x8C -> Local FM RF Level Threshold (bytes 141-142)
    S16 validLocalAMRFThreshold;          // Offset 0x8E -> Local AM RF Level Threshold (bytes 143-144)

    U16 seekStrengthReserved0;     // Offset 0x90 -> Reserved
    U16 seekStrengthReserved1;     // Offset 0x92 -> Reserved

    U16 seekFMInitialCheckWait;    // Offset 0x94 -> Seek FM Initial Check Wait
                                            // number of msecs to wait before initial analog strength check
    U16 seekAMInitialCheckWait;    // Offset 0x96 -> Seek AM Initial Check Wait

    U16 seekFMTimeout;             // Offset 0x98 -> Seek FM Timeout
                                            // maximum number of msecs to wait before deeming a station as not valid
    U16 seekAMTimeout;             // Offset 0x9A -> Seek AM Timeout

    U8 seekFMCheckInterval;        // Offset 0x9C -> Seek FM Check Interval
                                            // number of msecs to wait between analog strength checks
    U8 seekAMCheckInterval;        // Offset 0x9D -> Seek AM Check Interval
    U8 seekReserved1;              // Offset 0x9E -> Reserved
    U8 seekReserved2;              // Offset 0x9F -> Reserved

    U16 seekFMHDInitialCheckWait;  // Offset 0xA0 -> Seek FM HD Initial Check Wait
    U16 seekAMHDInitialCheckWait;  // Offset 0xA2 -> Seek AM HD Initial Check Wait

    U16 seekFMHDTimeout;           // Offset 0xA4 -> Seek FM HD Timeout
    U16 seekAMHDTimeout;           // Offset 0xA6 -> Seek AM HD Timeout

    U8 scanValidStationWait;       // Offset 0xA8 -> Scan Valid Frequency Wait
                                            // number of seconds to pause on a valid station when scanning.  Allow
    U8 scanReserved1[11];          // Offset 0xA9 -> 0xB3 Reserved
                                            // genre and other hmi tune list table fields to be updated.
    U8 acgThresholdEstimation;     // Offset 0xB4 -> Reserved
    U8 power_reserved_0;           // Offset 0xB5 -> Reserved
    U8 powerFMCdNo;                // Offset 0xB6 -> Reserved
    U8 powerAMCdNo;                // Offset 0xB7 -> Reserved

    U32 sysConfigReserved13;       // Offset 0xB8 -> Reserved
    U32 sysConfigReserved12;       // Offset 0xBC -> Reserved
    U32 sysConfigReserved11;       // Offset 0xC0 -> Reserved

    U32 sysConfigReserved10;       // Offset 0xC4 -> Reserved
    U32 sysConfigReserved9;        // Offset 0xC8 -> Reserved
    U32 sysConfigReserved8;        // Offset 0xCC -> Reserved
    U32 sysConfigReserved7;        // Offset 0xD0 -> Reserved
    U32 sysConfigReserved6;        // Offset 0xD4 -> Reserved
    U32 sysConfigReserved5;        // Offset 0xD8 -> Reserved

    U8   aim_h2d_cntrFlags;      // Offset 0xDC -> Reserved
    U8   aim_dac_resolution;     // Offset 0xDD -> Antenna Tuning DAC Resolution
    U8   aim_h2d_enaAIM;         // Offset 0xDE -> AIM Loop EnaDis - Host-To-Dsp flag to enable/disable
    U8   aim_h2d_antenna_select; // Offset 0xDF -> Diversity Select
                                            // bit0-2: FM antenna select; bit3: LNA bypass/impedance; bit4-7: AM antenna select

    U16  aim_h2d_Q;                // Offset 0xE0 -> FM Q Factor
                                            // Offset 0xE1 -> AM Q Factor
    U16  aim_h2d_manual_set_dac_value;// Offset 0xE2 -> Antenna Tuning Value

    // antenna settings
    U8   fm_start;               // Offset 0xE4 -> FM Start Channel
    U8   fm_stop;                // Offset 0xE5 -> FM Stop Channel
    U16  fm_start_setting;       // Offset 0xE6 -> FM Start Setting
    U16  fm_stop_setting;        // Offset 0xE8 -> FM Stop Setting

    U8   am_start;               // Offset 0xEA -> AM Start Channel
    U8   am_stop;                // Offset 0xEB -> AM Stop Channel
    U16  am_start_setting;       // Offset 0xEC -> AM Start Setting
    U16  am_stop_setting;        // Offset 0xEE -> AM Stop Setting

    // WARNING: Do not add additional bytes to SYS_CFG_INFO_t because
    //          it will change the set and get configuration commands!
}BBP_sys_config_t;

extern BBP_tune_select_t stBbpTuneSelect[MAX_NUM_OF_INSTANCES];

HDR_instance_t* CMD_cb_bbp_get_hdr_instance(U32 instanceNum);

HDBOOL CMD_cb_bbp_busy(HDR_instance_t* hdr_instance);

S32 CMD_cb_bbp_play_alert_tone(HDR_instance_t* hdr_instance);

S32 CMD_cb_bbp_set_playing_program(HDR_instance_t* hdr_instance, HDR_program_t program);

S32 CMD_cb_bbp_get_tune_select(HDR_instance_t* hdr_instance, BBP_tune_select_t* tune_select);
S32 CMD_cb_bbp_set_tune_select(HDR_instance_t* hdr_instance, const BBP_tune_select_t* tune_select);

S32 CMD_cb_bbp_get_tune_params(BBP_tune_params_t* tune_params);
S32 CMD_cb_bbp_set_tune_params(const BBP_tune_params_t* tune_params);

S32 CMD_cb_bbp_get_iboc_config(BBP_iboc_config_t* iboc_config, BBP_config_select_t config_select);
S32 CMD_cb_bbp_set_iboc_config(const BBP_iboc_config_t* iboc_config);
S32 CMD_cb_bbp_reset_iboc_config(void);

S32 CMD_cb_bbp_reset_dsqm_time_const(void);
S32 CMD_cb_bbp_set_dsqm_seek_thresh(U32 thresh);
S32 CMD_cb_bbp_get_dsqm_seek_thresh(U32 * thresh);
S32 CMD_cb_bbp_reset_dsqm_seek_thresh(void);

#if 0	// There is not defined within this project.
S32 CMD_cb_bbp_get_blend_config(HDR_instance_t* hdr_instance, HDR_blend_params_t* blend_config, BBP_config_select_t config_select);
S32 CMD_cb_bbp_get_adv_blend_config(HDR_instance_t* hdr_instance, HDR_blend_adv_params_t* adv_blend_config, BBP_config_select_t config_select);
#endif

S32 CMD_cb_digital_audio_acquired(HDR_instance_t* hdr_instance, HDBOOL* audio_acquired);


S32 CMD_cb_bbp_get_activated_services(HDR_instance_t* hdr_instance, BBP_services_t* bbp_services);
S32 CMD_cb_bbp_set_activated_services(HDR_instance_t* hdr_instance, const BBP_services_t* bbp_services);
S32 CMD_cb_bbp_get_supported_services(BBP_services_t* bbp_services);

S32 CMD_cb_bbp_get_sys_config_info(BBP_sys_config_info_t* sys_config_info, BBP_config_select_t config_select);

S32 CMD_cb_bbp_get_sys_config(BBP_sys_config_t* sys_config, BBP_config_select_t config_select);
S32 CMD_cb_bbp_set_sys_config(const BBP_sys_config_t* sys_config);
S32 CMD_cb_bbp_reset_sys_config(void);

S32 CMD_cb_bbp_get_sys_config_param(BBP_sys_config_param_t param, U32 * value);
S32 CMD_cb_bbp_set_sys_config_param(BBP_sys_config_param_t param, U32 value);

S32 CMD_cb_bbp_enable_audio_split_mode(HDR_instance_t* hdr_instance);
S32 CMD_cb_bbp_disable_audio_split_mode(HDR_instance_t* hdr_instance);

typedef struct {
    HDBOOL am_auto_time_align_enabled;
    HDBOOL fm_auto_time_align_enabled;
    HDBOOL am_auto_level_align_enabled;
    HDBOOL fm_auto_level_align_enabled;
#ifdef USE_HDRLIB_3RD_CHG_VER
    HDBOOL am_auto_level_correction_enabled;
    HDBOOL fm_auto_level_correction_enabled;
#else
    HDBOOL apply_level_adjustment;
#endif
}CMD_auto_alignment_config_t;

S32 CMD_cb_set_auto_alignment_config(HDR_instance_t* hdr_instance, const CMD_auto_alignment_config_t* config);
S32 CMD_cb_get_auto_alignment_config(HDR_instance_t* hdr_instance, CMD_auto_alignment_config_t* config);

typedef struct {
    U32 amMaxPosRange;
    U32 amMaxNegRange;
    U32 fmMaxPosRange;
    U32 fmMaxNegRange;
}CB_auto_alignment_spec_t;

S32 CMD_cb_get_auto_alignment_spec(HDR_instance_t* hdr_instance, CB_auto_alignment_spec_t* spec);

typedef struct{
    HDBOOL alignmentFound;
    HDBOOL digAudioAvailable;
    HDBOOL phaseInverted;
    S32 alignmentOffset;
    S32 levelOffset;
    F32 confidenceLevel;
}CMD_auto_alignment_status_t;

void BBP_sys_init_default_config(void);
S32 CMD_cb_get_auto_alignment_status(HDR_instance_t* hdr_instance, CMD_auto_alignment_status_t* status);
S32 CMD_cb_get_auto_alignment_confidence_level(HDR_instance_t* hdr_instance, HDR_tune_band_t band, U32* level);
S32 CMD_cb_set_auto_alignment_confidence_level(HDR_instance_t* hdr_instance, HDR_tune_band_t band, U32 level);

S32 CMD_cb_write_protect_error(const S8* parameter_name);

#endif //CMD_CALLBACKS_H_
