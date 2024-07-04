/*******************************************************************************

*   FileName : tchdr_systypes.h

*   Copyright (c) Telechips Inc.

*   Description : HD Radio variables type definitions for internal framework

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
#ifndef TCHDR_SYSTYPES_H__
#define TCHDR_SYSTYPES_H__

#ifdef __ANDROID__
#define HDR_SYSTEM_IS_ANDROID
#else
#define HDR_SYSTEM_IS_LINUX
#endif

#ifdef __aarch64__
#define HDR_64BIT_SYSTEM
#else
#define HDR_32BIT_SYSTEM
#endif

/***************************************************
*				Include					*
****************************************************/
#ifdef HDR_SYSTEM_IS_ANDROID
#include <android/log.h>
#endif
#include "tchdr_types.h"

// Modify the debug options in this file.
#include "tchdr_debug.h"

/***************************************************
*				Defines					*
****************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define HDRADIO_FRAMEWORK_VER				"1.6.0"

#define PTHREAD_32BIT_STACK_MIN				((U32)16*(U32)1024)     // 32bit system's pthread minimum stack size
#define PTHREAD_64BIT_STACK_MIN				((U32)128*(U32)1024)	// 64bit system's pthread minimum stack size

#define PTHREAD_MAX_NICE                    (19)    // Based on the priority value in the API
#define PTHREAD_MIN_NICE                    (-20)   // Based on the priority value in the API

// It should match the HDR core library version used when building.
// Linux: It should be the same as the version set in CORE_HD_LIB in the Makefile.am file.
#define	HDRLIB_CUR_VER						(0x2A3)	// Current HD Core LIB version
// Do not modify the definition of the following change point version.
#define HDRLIB_1ST_CHG_VER					(0x263)	// 1st change point. Change some HD Core Callback Functions
#define HDRLIB_2ND_CHG_VER					(0x2A3)	// 2nd change point. Modify or add some HD Core Library APIs and Add HD configuration type

#if (HDRLIB_CUR_VER >= HDRLIB_1ST_CHG_VER)
#define	USE_CHANGED_HDRLIB_CB
#endif
#if (HDRLIB_CUR_VER >= HDRLIB_2ND_CHG_VER)
#define	USE_HDRLIB_2ND_CHG_VER
#endif

/** Common configurations supported by HD Radio */
#define HDR_1p0_CONFIG						(1U)		/**< One full HD Radio instance */
#define HDR_1p0_MRC_CONFIG					(2U)		/**< MRC configuration(1 full instance + 1 demod only instance) */
#define HDR_1p5_CONFIG						(3U)		/**< One full instance + 1 data-only instance */
#define HDR_1p5_MRC_CONFIG					(4U)		/**< 2 MRC instances + 1 data-only instance */
#define HDR_1p5_DUAL_MRC_CONFIG				(5U)		/**< 2 MRC instances + 2 MRC Data instances */

/**< Specify compiled HD Radio configuration for distribution */
//#define HDR_CONFIG	(HDR_1p5_DUAL_MRC_CONFIG)		// Not compatible with higher version
#define HDR_CONFIG	(HDR_1p5_MRC_CONFIG)			// Not compatible with higher version
//#define HDR_CONFIG	(HDR_1p5_CONFIG)				// Not compatible with higher version
//#define HDR_CONFIG	(HDR_1p0_MRC_CONFIG)			// Not compatible with higher version
//#define HDR_CONFIG	(HDR_1p0_CONFIG)				// Not compatible with higher version

#if HDR_CONFIG == HDR_1p0_CONFIG
	#define NUM_HDR_INSTANCES				(1U)
	#define TC_HDRADIO_FRAMEWORK_VERSION_STRING			("HDR_1.0_"HDRADIO_FRAMEWORK_VER)
#elif HDR_CONFIG == HDR_1p0_MRC_CONFIG
	#define NUM_HDR_INSTANCES				(2U)
	#define TC_HDRADIO_FRAMEWORK_VERSION_STRING			("HDR_1.0MRC_"HDRADIO_FRAMEWORK_VER)
#elif HDR_CONFIG == HDR_1p5_CONFIG
	#define NUM_HDR_INSTANCES				(2U)
	#define TC_HDRADIO_FRAMEWORK_VERSION_STRING			("HDR_1.5_"HDRADIO_FRAMEWORK_VER)
#elif HDR_CONFIG == HDR_1p5_MRC_CONFIG
	#define NUM_HDR_INSTANCES				(3U)
	#define TC_HDRADIO_FRAMEWORK_VERSION_STRING			("HDR_1.5MRC_"HDRADIO_FRAMEWORK_VER)
#else
	#define NUM_HDR_INSTANCES				(4U)
	#define TC_HDRADIO_FRAMEWORK_VERSION_STRING			("HDR_1.5DUALMRC_"HDRADIO_FRAMEWORK_VER)
#endif

#define MAX_NUM_OF_INSTANCES				(4U)		// Maximum number of HDR instances
#define LIMIT_EVALUATION_TIME				(1800000)	// (unit: ms) 1h:(3600000), 30m:(1800000)

// Tuner I2S clock error ppm between I/Q and analog audio
#define	SI479XX_TUNER_I2S_PPM				(-0.0522655)	// Silab Tuner Si479xx family chipset I2S clock error ppm
#define TDA7707_TUNER_I2S_PPM				(0.0)			// ST Tuner TDA770x family chipset I2S clock error ppm
#define TUNER_AUDIO_I2S_PPM					(0.0)			// Default setting value

#ifdef HDR_SYSTEM_IS_ANDROID
#define DUMP_PATH "/mnt/"
#else
#define DUMP_PATH "/tmp/"
#endif

// For Debugging
//#define DEBUG_ENABLE_HDR_LOG
//#define DEBUG_TCDAT_TEST_DUMP                    	// Enable digital-analog audio tracker
//#define DEBUG_IQ_BUF_FILE_DUMP                   	// Dump the I, Q, IQ buffers at bbinput thread
//#deinfe DEBUG_ENABLE_TRACE_THREAD                	// Enable trace thread for XPERI debugging
//#define DEBUG_ALL_THREAD_STATUS_DUMP             	// Dump the loop time and buffer usage for each thread to '/tmp' as 'csv' file

// For HD Radio Options
//#define USE_EVALUATION_MODE                      	// Evaluation mode that only works while LIMIT_EVALUATION_TIME.
//#define DISABLE_TCHDR_DATA_API              	   	// DISABLE THE DATA ACCESS FROM TC API FOR XPERI TESTING.
#define USE_AUTO_AUDIO_ALIGN                       	// Enable auto audio alignment(AAA)
//#define USE_RESET_IQ_WHEN_SWITCHING              	// Reset IQ driver and buffer when the frequency or the band is switched.
#define USE_AUDIO_BUFFER_FILL_ZERO_BEFORE_START    	// At the start of audio, the audio output buffer is filled with zeros and started.
//#define USE_AUTO_MUTE_WHEN_ONLY_ANALOG_AUDIO_MODE	// If current program is all-digital program and force-analog is requested, play mute. for JVCK.
#define USE_AUDIO_OUTPUT_RESAMPLER                 	// Use an audio resampler developed internally.
//#define USE_ANALOG_AUDIO_MUTE_FOR_TUNE           	// Use analog audio mute when tune band/frequency

// For Coverity
//#define AVOID_MC2012_RULE_21P6_USING_PRINTF

// For HD Radio Timer
#define	LINUX_CLOCK_TIMER_TYPE		(CLOCK_MONOTONIC)

// For HD Radio Audio Samplerate
#define	TCHDR_SAMPLERATE			(44100)

// For Codesonar
#define	AVOID_CODESONAR_REDUNDANT_CONDITION_WARNING
#define AVOID_QAC_1STEP_WARNING

// For Thread Start Delay
#define	BBINPUT_START_NANO_DELAY	(0)				// (EXTERNAL_INPUT_NANO_DELAY+100000000)	// + 100ms

// HD Radio Framework Notification Event Value Weight
#define	TCHDR_SVC_NOTIFY_WT			(100U)
#define	TCHDR_BBP_NOTIFY_WT			(200U)
#define	TCHDR_MAIN_NOTIFY_WT		(300U)
#define	TCHDR_MRC_NOTIFY_WT			(400U)
#define	TCHDR_BS_NOTIFY_WT			(500U)
#define TCHDR_BSMRC_NOTIFY_WT		(600U)
#define	TCHDR_AUD_NOTIFY_WT			(700U)
#define	TCHDR_BLENDING_NOTIFY_WT	(800U)

// HD Radio Framework Thread Priority: Use the API priority value[Range: 1(Low) ~ 99(High)]
// top PR = -1 - userpriority
// kernel PR = 100 - userpriority
#define DEFAULT_THREAD_PRIORITY_OFFSET (0)   // offset from max priority.  Unless set otherwise,
                                             // threads will use scheduling (priority - DEFAULT_THREAD_PRIORITY_OFFSET)
#define MANAGER_THREAD_PRIORITY  	  (1)    // for HDR manager thread scheduling priority (99 - 98)
#define BB_INPUT_THREAD_PRIORITY      (88)   // for bb input read thread scheduling priority (99 - 11)
#define HDR_EXEC_THREAD_PRIORITY      (74)   // for HDR exec thread scheduling priority (99 - 25)
#define HDR_BLENDING_THREAD_PRIORITY  (68)   // for HDR audio blending thread scheduling priority (99 - 31)
#define AUDIO_OUTPUT_THREAD_PRIORITY  (80)   // for audio playback thread scheduling priority (99 - 19)
#define CMD_PROC_THREAD_PRIORITY      (95)   // for command processor thread scheduling priority (99 - 4)
#define LOGGER_THREAD_PRIORITY        (49)   // for logger thread scheduling priority (99 - 50)
#define AUDIO_INPUT_THREAD_PRIORITY   (87)   // for audio input thread scheduling priority (99 - 12)
#define RF_IQ_INPUT_THREAD_PRIORITY   (88)   // for RF IQ input thread scheduling priority (99 - 11)

// When scheduler policy is sched_other, this definition is available.
#define	MANAGER_THREAD_PRIORITY_NICE        (-15)
#define BB_INPUT_THREAD_PRIORITY_NICE       (-19)
#define HDR_EXEC_THREAD_PRIORITY_NICE       (-17)
#define HDR_BLENDING_THREAD_PRIORITY_NICE   (-16)
#define AUDIO_OUTPUT_THREAD_PRIORITY_NICE   (-18)
#define CMD_PROC_THREAD_PRIORITY_NICE       (-15)
#define LOGGER_THREAD_PRIORITY_NICE         (-14)
#define AUDIO_INPUT_THREAD_PRIORITY_NICE    (-18)
#define RF_IQ_INPUT_THREAD_PRIORITY_NICE    (-19)

#define HDR_LIB_MEM_SIZE			(1024 * 1024)

/* Tuner IQ a sample byte */
#define IQ_SAMPLE_SIZE				(2U)	// unit: byte

/* Tuner IQ chunk size */
// When the maximum IQ sample rate is 744.1875kHz, it has the largest chunk value (4320 * 2bytes) in AM mode.
#define IQ_CHUNK_MAX_SIZE			(SYMBOL_SIZE_FM * 2U * IQ_SAMPLE_SIZE)	// unit: byte

/* Tuner driver buffer size for Radio */
// IQ input I2S driver buffer size: Max 2Mbytes, You should set to 2^n(Multiples of 64).
#define IQ_DRIVER_BUFFER_SIZE		(512U * 1024U)						// unit: byte	// 128 // If less than 128, overflow occurs when switching between hdclose and hdopen. To use it, reduce the driver buffer size, but do not reduce the iqinput buffer.
#define IQ_DRIVER_PERIOD_SIZE		(IQ_DRIVER_BUFFER_SIZE / 16U)			// unit: byte
// Audio Input I2S driver buffer size: Max 2Mbytes, You should set to 2^n(Multiples of 32).
#define	AUDIO_DRIVER_BUFFER_SIZE	(32U * 1024U)						// unit: byte	// 16
#define	AUDIO_DRIVER_PERIOD_SIZE	(AUDIO_DRIVER_BUFFER_SIZE / 64U)		// unit: byte

// HD Radio Framework Thread Name
#define	HDR_FWRK_THREADS_POLICY				(SCHED_FIFO)
// HD Radio Framework Thread Name
#define HDR_MANAGER_THREAD_NAME				(const S8*)("TcHdrManager")
#define HDR_BBINPUT_THREAD_NAME				(const S8*)("TcHdrBbInput")
#define HDR_PRIMARY_THREAD_NAME				(const S8*)("TcHdrPrimary")
#define HDR_MRC_THREAD_NAME					(const S8*)("TcHdrMrc")
#define HDR_BACKSCAN_THREAD_NAME			(const S8*)("TcHdrBackscan")
#define HDR_BSMRC_THREAD_NAME				(const S8*)("TcHdrBsMrc")
#define HDR_BLENDING_THREAD_NAME			(const S8*)("TcHdrBlending")
#define HDR_AUDOUTPUT_THREAD_NAME			(const S8*)("TcHdrAudOutput")
#define HDR_AUDINPUT_THREAD_NAME			(const S8*)("TcHdrAudInput")
#define HDR_IQINPUT_THREAD_NAME				(const S8*)("TcHdrIqInput")
#define HDR_CMDPROC_THREAD_NAME				(const S8*)("TcHdrCmdProc")
#define HDR_CMDPROC_PORTLISTENER_TREAD_NAME	(const S8*)("TcHdrCmdListen")	// not used yet
#define	HDR_LOGGER_THREAD_NAME				(const S8*)("TcHdrLogReader")
#define HDR_LOGGER_PORTLISTENER_TREAD_NAME	(const S8*)("TcHdrLogListen")	// not used yet
#define	HDR_TRACE_THREAD_NAME				(const S8*)("TcHdrTrace")

#define NSEC_PER_SEC 						(1000000000)

#define	SDR_DRV_ERRNO_XRUN					(-32)

#define UNUSED(x) 							(void)(x)

/***************************************************
*				Enumeration				*
****************************************************/
typedef enum {
	eTCHDR_DEBUG_BBINPUT_BUFFER_RESET	= 0U,
	eTCHDR_DEBUG_BBINPUT_DRIVER_RESET	= 1U,
	eTCHDR_DEBUG_BBINPUT_DRV_BUF_RESET	= 2U,
	eTCHDR_DEBUG_BBINPUT_IQ_DUMP		= 3U,
	eTCHDR_DEBUG_AUDIO_OUTPUT_DUMP		= 4U,
	eTCHDR_DEBUG_END					= 5U
}eTCHDR_DEBUG_t;

typedef enum {
	eTCHDR_EVT_STS_DONE					= 0,	/* Job Complete */
	eTCHDR_EVT_STS_DONE_NOTIFY			= 1,	/* Job Complete & Notify */
	eTCHDR_EVT_STS_END					= 2
} eTCHDR_EVT_STS_t;

typedef enum
{
	eTCHDR_SVC_CMD_NULL				= 0U,

	eTCHDR_SVC_CMD_OPEN				= 1U,
	eTCHDR_SVC_CMD_CLOSE			= 2U,

	eTCHDR_SVC_CMD_RESET_BBP		= 10U,
	eTCHDR_SVC_CMD_SET_MAIN_AUDIO_MODE	= 11U,
	eTCHDR_SVC_CMD_SET_TUNE			= 12U,
	eTCHDR_SVC_CMD_SET_PROGRAM		= 13U,
	eTCHDR_SVC_CMD_SET_MUTE			= 14U,
	eTCHDR_SVC_CMD_SET_AUDIO_CTRL 	= 15U,

	eTCHDR_SVC_CMD_GET_STATUS		= 20U,

	eTCHDR_SVC_CMD_ENABLE_GET_PSD	= 21U,
	eTCHDR_SVC_CMD_ENABLE_GET_SIS	= 22U,
	eTCHDR_SVC_CMD_ENABLE_GET_SIG	= 23U,
	eTCHDR_SVC_CMD_ENABLE_GET_AAS	= 24U,
	eTCHDR_SVC_CMD_ENABLE_GET_ALERT	= 25U,
	eTCHDR_SVC_CMD_ENABLE_GET_LOT	= 26U,

	eTCHDR_SVC_CMD_TEST				= 99U,

	eTCHDR_SVC_CMD_END				= 100U
}eTCHDR_SVC_CMD_t;

typedef enum
{
	eTCHDR_BBINPUT_CMD_NULL			= 0U,

	eTCHDR_BBINPUT_CMD_OPEN			= 1U,
	eTCHDR_BBINPUT_CMD_CLOSE		= 2U,
	eTCHDR_BBINPUT_CMD_RESET_MAIN	= 3U,
	eTCHDR_BBINPUT_CMD_RESET_BS		= 4U,
	eTCHDR_BBINPUT_CMD_SET_TUNE		= 5U,
	eTCHDR_BBINPUT_CMD_DELAY_RESET_MAIN	=6U,

	eTCHDR_BBINPUT_CMD_TEST			= 99U,

	eTCHDR_BBINPUT_CMD_END			= 100U
}eTCHDR_BBINPUT_CMD_t;

typedef enum
{
	eTCHDR_MAIN_CMD_NULL		= 0U,

	eTCHDR_MAIN_CMD_OPEN		= 1U,
	eTCHDR_MAIN_CMD_CLOSE		= 2U,
	eTCHDR_MAIN_CMD_RESET		= 3U,
	eTCHDR_MAIN_CMD_SET_TUNE	= 5U,

	eTCHDR_MAIN_CMD_DUMMY		= 255U,

	eTCHDR_MAIN_CMD_END			= 256U
}eTCHDR_MAIN_CMD_t;

typedef enum
{
	eTCHDR_MRC_CMD_NULL		= 0U,

	eTCHDR_MRC_CMD_OPEN		= 1U,
	eTCHDR_MRC_CMD_CLOSE	= 2U,
	eTCHDR_MRC_CMD_RESET	= 3U,
	eTCHDR_MRC_CMD_SET_TUNE	= 4U,

	eTCHDR_MRC_CMD_DUMMY	= 255U,

	eTCHDR_MRC_CMD_END		= 256U
}eTCHDR_MRC_CMD_t;

typedef enum
{
	eTCHDR_BS_CMD_NULL		= 0U,

	eTCHDR_BS_CMD_OPEN		= 1U,
	eTCHDR_BS_CMD_CLOSE		= 2U,
	eTCHDR_BS_CMD_RESET		= 3U,
	eTCHDR_BS_CMD_SET_TUNE	= 4U,

	eTCHDR_BS_CMD_DUMMY		= 255U,

	eTCHDR_BS_CMD_END		= 256U
}eTCHDR_BS_CMD_t;

typedef enum
{
	eTCHDR_BSMRC_CMD_NULL		= 0U,

	eTCHDR_BSMRC_CMD_OPEN		= 1U,
	eTCHDR_BSMRC_CMD_CLOSE		= 2U,
	eTCHDR_BSMRC_CMD_RESET		= 3U,
	eTCHDR_BSMRC_CMD_SET_TUNE	= 4U,

	eTCHDR_BSMRC_CMD_DUMMY		= 255U,

	eTCHDR_BSMRC_CMD_END		= 256U
}eTCHDR_BSMRC_CMD_t;

typedef enum
{
	eTCHDR_AUDIO_CMD_NULL	= 0U,

	eTCHDR_AUDIO_CMD_OPEN	= 1U,
	eTCHDR_AUDIO_CMD_CLOSE	= 2U,
	eTCHDR_AUDIO_CMD_START	= 3U,
	eTCHDR_AUDIO_CMD_STOP	= 4U,
	eTCHDR_AUDIO_CMD_RESET	= 5U,
	eTCHDR_AUDIO_CMD_MUTE	= 6U,

	eTCHDR_AUDIO_CMD_TEST	= 99U,

	eTCHDR_AUDIO_CMD_END	= 256U
}eTCHDR_AUDIO_CMD_t;

typedef enum
{
	eTCHDR_BLENDING_CMD_NULL		= 0U,

	eTCHDR_BLENDING_CMD_OPEN		= 1U,
	eTCHDR_BLENDING_CMD_CLOSE		= 2U,
	eTCHDR_BLENDING_CMD_RESET		= 3U,
	eTCHDR_BLENDING_CMD_AUDIO_MODE	= 4U,

	eTCHDR_BLENDING_CMD_DUMMY		= 255U,

	eTCHDR_BLENDING_CMD_END			= 256U
}eTCHDR_BLENDING_CMD_t;

typedef enum {
// HDR Service Notification
// It should be the same as the eTC_HDR_NOTIFY_t typedef. Make sure to check when adding or deleting.
	eTCHDR_SVC_NOTIFY_OPEN			= ((U32)eTCHDR_SVC_CMD_OPEN + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_CLOSE			= ((U32)eTCHDR_SVC_CMD_CLOSE + TCHDR_SVC_NOTIFY_WT),

	eTCHDR_SVC_NOTIFY_RESET_BBP		= ((U32)eTCHDR_SVC_CMD_RESET_BBP + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_AUDIO_MODE	= ((U32)eTCHDR_SVC_CMD_SET_MAIN_AUDIO_MODE + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_TUNE			= ((U32)eTCHDR_SVC_CMD_SET_TUNE + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_PROGRAM		= ((U32)eTCHDR_SVC_CMD_SET_PROGRAM + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_MUTE			= ((U32)eTCHDR_SVC_CMD_SET_MUTE + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_AUDIO_CTRL	= ((U32)eTCHDR_SVC_CMD_SET_AUDIO_CTRL + TCHDR_SVC_NOTIFY_WT),

	eTCHDR_SVC_NOTIFY_STATUS		= ((U32)eTCHDR_SVC_CMD_GET_STATUS + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_PSD			= ((U32)eTCHDR_SVC_CMD_ENABLE_GET_PSD + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_SIS			= ((U32)eTCHDR_SVC_CMD_ENABLE_GET_SIS + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_SIG			= ((U32)eTCHDR_SVC_CMD_ENABLE_GET_SIG + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_AAS			= ((U32)eTCHDR_SVC_CMD_ENABLE_GET_AAS + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_LOT			= ((U32)eTCHDR_SVC_CMD_ENABLE_GET_LOT + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_ALERT			= ((U32)eTCHDR_SVC_CMD_ENABLE_GET_ALERT + TCHDR_SVC_NOTIFY_WT),

	eTCHDR_SVC_NOTIFY_SIGNAL		= ((U32)90U + TCHDR_SVC_NOTIFY_WT),
	eTCHDR_SVC_NOTIFY_PTY			= ((U32)91U + TCHDR_SVC_NOTIFY_WT),

	eTCHDR_SVC_NOTIFY_TEST			= ((U32)eTCHDR_SVC_CMD_TEST + TCHDR_SVC_NOTIFY_WT),

// HDR BBinput Notification
	eTCHDR_BBINPUT_NOTIFY_OPEN			= ((U32)eTCHDR_BBINPUT_CMD_OPEN + TCHDR_BBP_NOTIFY_WT),
	eTCHDR_BBINPUT_NOTIFY_CLOSE			= ((U32)eTCHDR_BBINPUT_CMD_CLOSE + TCHDR_BBP_NOTIFY_WT),
	eTCHDR_BBINPUT_NOTIFY_RESET_MAIN	= ((U32)eTCHDR_BBINPUT_CMD_RESET_MAIN + TCHDR_BBP_NOTIFY_WT),
	eTCHDR_BBINPUT_NOTIFY_RESET_BS		= ((U32)eTCHDR_BBINPUT_CMD_RESET_BS + TCHDR_BBP_NOTIFY_WT),
	eTCHDR_BBINPUT_NOTIFY_TUNE			= ((U32)eTCHDR_BBINPUT_CMD_SET_TUNE + TCHDR_BBP_NOTIFY_WT),

// HDR Main Notification
	eTCHDR_MAIN_NOTIFY_OPEN				= ((U32)eTCHDR_MAIN_CMD_OPEN + TCHDR_MAIN_NOTIFY_WT),
	eTCHDR_MAIN_NOTIFY_CLOSE			= ((U32)eTCHDR_MAIN_CMD_CLOSE + TCHDR_MAIN_NOTIFY_WT),
	eTCHDR_MAIN_NOTIFY_RESET			= ((U32)eTCHDR_MAIN_CMD_RESET + TCHDR_MAIN_NOTIFY_WT),
	eTCHDR_MAIN_NOTIFY_TUNE				= ((U32)eTCHDR_MAIN_CMD_SET_TUNE + TCHDR_MAIN_NOTIFY_WT),
	eTCHDR_MAIN_NOTIFY_DUMMY			= ((U32)eTCHDR_MAIN_CMD_DUMMY + TCHDR_MAIN_NOTIFY_WT),

// HDR MRC Notification
	eTCHDR_MRC_NOTIFY_OPEN				= ((U32)eTCHDR_MRC_CMD_OPEN + TCHDR_MRC_NOTIFY_WT),
	eTCHDR_MRC_NOTIFY_CLOSE				= ((U32)eTCHDR_MRC_CMD_CLOSE + TCHDR_MRC_NOTIFY_WT),
	eTCHDR_MRC_NOTIFY_RESET				= ((U32)eTCHDR_MRC_CMD_RESET + TCHDR_MRC_NOTIFY_WT),
	eTCHDR_MRC_NOTIFY_TUNE				= ((U32)eTCHDR_MRC_CMD_SET_TUNE + TCHDR_MRC_NOTIFY_WT),
	eTCHDR_MRC_NOTIFY_DUMMY				= ((U32)eTCHDR_MRC_CMD_DUMMY + TCHDR_MRC_NOTIFY_WT),

// HDR BS Notification
	eTCHDR_BS_NOTIFY_OPEN				= ((U32)eTCHDR_BS_CMD_OPEN + TCHDR_BS_NOTIFY_WT),
	eTCHDR_BS_NOTIFY_CLOSE				= ((U32)eTCHDR_BS_CMD_CLOSE + TCHDR_BS_NOTIFY_WT),
	eTCHDR_BS_NOTIFY_RESET				= ((U32)eTCHDR_BS_CMD_RESET + TCHDR_BS_NOTIFY_WT),
	eTCHDR_BS_NOTIFY_TUNE				= ((U32)eTCHDR_BS_CMD_SET_TUNE + TCHDR_BS_NOTIFY_WT),
	eTCHDR_BS_NOTIFY_DUMMY				= ((U32)eTCHDR_BS_CMD_DUMMY + TCHDR_BS_NOTIFY_WT),

// HDR BS MRC Notification
	eTCHDR_BSMRC_NOTIFY_OPEN			= ((U32)eTCHDR_BSMRC_CMD_OPEN + TCHDR_BSMRC_NOTIFY_WT),
	eTCHDR_BSMRC_NOTIFY_CLOSE			= ((U32)eTCHDR_BSMRC_CMD_CLOSE + TCHDR_BSMRC_NOTIFY_WT),
	eTCHDR_BSMRC_NOTIFY_RESET			= ((U32)eTCHDR_BSMRC_CMD_RESET + TCHDR_BSMRC_NOTIFY_WT),
	eTCHDR_BSMRC_NOTIFY_TUNE			= ((U32)eTCHDR_BSMRC_CMD_SET_TUNE + TCHDR_BSMRC_NOTIFY_WT),
	eTCHDR_BSMRC_NOTIFY_DUMMY			= ((U32)eTCHDR_BSMRC_CMD_DUMMY + TCHDR_BSMRC_NOTIFY_WT),

// HDR Audio Notification
	eTCHDR_AUDIO_NOTIFY_OPEN			= ((U32)eTCHDR_AUDIO_CMD_OPEN + TCHDR_AUD_NOTIFY_WT),
	eTCHDR_AUDIO_NOTIFY_CLOSE			= ((U32)eTCHDR_AUDIO_CMD_CLOSE + TCHDR_AUD_NOTIFY_WT),
	eTCHDR_AUDIO_NOTIFY_START			= ((U32)eTCHDR_AUDIO_CMD_START + TCHDR_AUD_NOTIFY_WT),
	eTCHDR_AUDIO_NOTIFY_STOP			= ((U32)eTCHDR_AUDIO_CMD_STOP + TCHDR_AUD_NOTIFY_WT),
	eTCHDR_AUDIO_NOTIFY_RESET			= ((U32)eTCHDR_AUDIO_CMD_CLOSE + TCHDR_AUD_NOTIFY_WT),
	eTCHDR_AUDIO_NOTIFY_MUTE			= ((U32)eTCHDR_AUDIO_CMD_MUTE + TCHDR_AUD_NOTIFY_WT),

// HDR Blending Notification
	eTCHDR_BLENDING_NOTIFY_OPEN			= ((U32)eTCHDR_BLENDING_CMD_OPEN + TCHDR_BLENDING_NOTIFY_WT),
	eTCHDR_BLENDING_NOTIFY_CLOSE		= ((U32)eTCHDR_BLENDING_CMD_CLOSE + TCHDR_BLENDING_NOTIFY_WT),
	eTCHDR_BLENDING_NOTIFY_RESET		= ((U32)eTCHDR_BLENDING_CMD_RESET + TCHDR_BLENDING_NOTIFY_WT),
	eTCHDR_BLENDING_NOTIFY_AUDIO_MODE	= ((U32)eTCHDR_BLENDING_CMD_AUDIO_MODE + TCHDR_BLENDING_NOTIFY_WT),
	eTCHDR_BLENDING_NOTIFY_DUMMY		= ((U32)eTCHDR_BLENDING_CMD_DUMMY + TCHDR_BLENDING_NOTIFY_WT)

}eTCHDR_MW_NOTIFY_t;

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

#ifdef __cplusplus
}
#endif

#endif
