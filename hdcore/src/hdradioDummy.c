/*******************************************************************************

*   FileName : hdradioDummy.c

*   Copyright (c) Telechips Inc.

*   Description : HD Radio Dummy Core Code

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
*        Include                                   *
****************************************************/
#include "hdrBasicTypes.h"
#include "hdrCore.h"
#include "hdrAas.h"
#include "hdrAlerts.h"
#include "hdrAudioResampler.h"
#include "hdrAutoAlign.h"
#include "hdrBbSrc.h"
#include "hdrBlend.h"
#include "hdrBlendCrossfade.h"
#include "hdrConfig.h"
#include "hdrPhy.h"
#include "hdrPsd.h"
#include "hdrSig.h"
#include "hdrSis.h"
#include "hdrTest.h"

/***************************************************
*        Global variable definitions               *
****************************************************/

/***************************************************
*        Imported variable declarations            *
****************************************************/

/***************************************************
*        Imported function declartions             *
****************************************************/

/***************************************************
*        Local preprocessor                        *
****************************************************/
#define HDRD_NUM_OF_INSTANCES		4
/***************************************************
*        Local type definitions                    *
****************************************************/

/***************************************************
*        Local constant definitions                *
****************************************************/
static HDR_tune_band_t curHdrBand[HDRD_NUM_OF_INSTANCES] = {HDR_BAND_IDLE, HDR_BAND_IDLE, HDR_BAND_IDLE, HDR_BAND_IDLE};

/***************************************************
*        Local function prototypes                 *
****************************************************/

/***************************************************
*        function definition                       *
****************************************************/
int_t HDR_aas_flush_lot_object(HDR_instance_t * hdr_instance, uint_t port_number, uint_t object_lot_id)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_mrc_enable(const HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}


int_t HDR_mrc_enabled(const HDR_instance_t * hdr_instance, bool * mrc_enabled)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}


int_t HDR_aas_flush_all_ports(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}


int_t HDR_psd_get_title(HDR_instance_t * hdr_instance, HDR_program_t program_number, HDR_psd_data_t * title)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_filt_dsqm(const HDR_instance_t * hdr_instance, uint_t * filt_dsqm)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_psd_get_ufid(HDR_instance_t * hdr_instance, HDR_program_t program_number, uint_t ufid_num, HDR_psd_ufid_subfield_t subfield, HDR_psd_data_t * ufid)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_station_location(HDR_instance_t * hdr_instance, HDR_sis_station_location_t * location)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_tx_dig_audio_gain(HDR_instance_t * hdr_instance, uint_t * tx_gain)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

bool HDR_hd_signal_acquired(const HDR_instance_t * hdr_instance)
{
	bool ret = false;

	return ret;
}

int_t HDR_blend_get_all_params(HDR_instance_t * hdr_instance, HDR_blend_params_t * blend_params)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_cdno(const HDR_instance_t * hdr_instance, uint_t * cdno)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_flush_port(HDR_instance_t * hdr_instance, uint_t port_number)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_audio_resampler_output(HDR_audio_resampler_t * audio_resampler, HDR_pcm_stereo_t * output)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_playing_program_type(HDR_instance_t * hdr_instance, uint_t * program_type)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_importer_manuf_ver(HDR_instance_t * hdr_instance, HDR_sis_tx_manuf_ver_t * version_struct)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_dsqm_filt_time_const(const HDR_instance_t * hdr_instance, uint_t * time_constant)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_blend_get_all_adv_params(HDR_instance_t * hdr_instance, HDR_blend_adv_params_t * adv_blend_params)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_test_enable_ber_mode(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

bool HDR_digital_audio_acquired(HDR_instance_t * hdr_instance)
{
	bool ret = false;

	return ret;
}

int_t HDR_psd_clear_changed_program(HDR_instance_t * hdr_instance, HDR_program_t program_number)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_psd_get_commercial(HDR_instance_t * hdr_instance, HDR_program_t program_number, HDR_psd_comr_subfield_t subfield, HDR_psd_data_t * commercial)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sig_flush_all(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_demod_raw_snr(const HDR_instance_t * hdr_instance, uint_t * raw_snr)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

uint_t HDR_aas_get_lot_space_left(HDR_instance_t * hdr_instance)
{
	uint_t ret = 0U;

	return ret;
}

int_t HDR_sis_get_alfn(HDR_instance_t * hdr_instance, HDR_sis_alfn_t * alfn)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_blend_set_all_adv_params(HDR_instance_t * hdr_instance, const HDR_blend_adv_params_t * params)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

HDR_tune_band_t HDR_get_band_select(const HDR_instance_t * hdr_instance)
{
	return curHdrBand[hdr_instance->instance_number];
}

bool HDR_test_ber_mode_enabled(HDR_instance_t * hdr_instance)
{
	bool ret = false;

	return ret;
}

int_t HDR_transport_task(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

HDR_program_bitmap_t HDR_psd_get_changed_programs(HDR_instance_t * hdr_instance)
{
	HDR_program_bitmap_t tBitmap;

	tBitmap.all = 0;

	return tBitmap;
}

const char* HDR_get_version_string(void)
{
#ifdef SYSTEM_IS_AARCH64
	return "HDR-E-2.6.10.X-DUMMY-64bit";
#else
	return "HDR-E-2.6.10.X-DUMMY-32bit";
#endif
}

int_t HDR_sis_get_data_services_type(HDR_instance_t * hdr_instance, uint_t service_type, HDR_sis_data_services_info_t * data_services_info)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_logger_disable_all(void)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_get_next_port_data(HDR_instance_t * hdr_instance, HDR_aas_packet_info_t * packet_info, uint8_t * packet_buffer, uint_t buffer_size, uint_t * bytes_written)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_test_get_raw_tx_blend_control(HDR_instance_t * hdr_instance, uint_t * blend_control)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_audio_resampler_reset(HDR_audio_resampler_t * audio_resampler)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_test_disable_ber_mode(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_exciter_core_ver(HDR_instance_t * hdr_instance, HDR_sis_tx_ver_str_t * version_string)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_demod_dsqm(const HDR_instance_t * hdr_instance, HDR_dsqm_t * dsqm)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

#ifdef USE_HDRLIB_2ND_CHG_VER
int32_t HDR_audio_resample_update_slips(HDR_audio_resampler_t* audio_resampler, int32_t bb_sample_slips, HDR_tune_band_t band, int32_t clk_offset, int32_t ppm_est);
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}
#else
int_t HDR_audio_resample_update_slips(HDR_audio_resampler_t * audio_resampler, int_t bb_sample_slips, HDR_tune_band_t band, int_t ppm_est)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}
#endif

int_t HDR_get_codec_mode(HDR_instance_t * hdr_instance, HDR_audio_codec_mode_t * codec_mode)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_alert_get_message_status(HDR_instance_t * hdr_instance, HDR_alerts_msg_status_t * status)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_auto_align_get_config(const HDR_auto_align_t * auto_align, HDR_auto_align_config_t * config)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_program_types(HDR_instance_t * hdr_instance, HDR_program_types_t * program_types)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_default_config(HDR_config_t * hdr_config)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_station_message(HDR_instance_t * hdr_instance, HDR_sis_station_msg_t * station_msg)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_avail_programs_list(HDR_instance_t * hdr_instance, HDR_sis_avail_programs_t * available_programs)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_bb_sample_slips(const HDR_instance_t * hdr_instance, HDR_bb_sample_slips_t * sample_slips)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_enable_lot_reassembly(HDR_instance_t * hdr_instance, uint_t service_number, uint_t port_number)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_hw_iso_enabled(const HDR_instance_t * hdr_instance, bool * hw_iso_enabled)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_set_dsqm_filt_time_const(const HDR_instance_t * hdr_instance, uint_t time_constant)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_auto_align_reset(HDR_auto_align_t * auto_align, HDR_tune_band_t band)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_importer_core_ver(HDR_instance_t * hdr_instance, HDR_sis_tx_ver_str_t * version_string)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_get_lot_object_header(HDR_instance_t * hdr_instance, uint_t port_number, uint_t object_lot_id, HDR_aas_lot_object_header_t * object_header)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_get_enabled_ports(HDR_instance_t * hdr_instance, HDR_aas_port_list_t * port_list)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

#ifdef USE_HDRLIB_2ND_CHG_VER
int_t HDR_baseband_input(const HDR_instance_t* hdr_instance, int16c_t* bb_input_samples, uint_t num_samples, uint32_t timestamp)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}
#else
int_t HDR_baseband_input(const HDR_instance_t * hdr_instance, int16c_t * bb_input_samples, uint_t num_samples)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}
#endif

int_t HDR_audio_decoder_task(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_blend_set_param_actual(HDR_instance_t * hdr_instance, uint_t offset, uint_t size, uint_t value)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_init(HDR_instance_t * hdr_instance, HDR_config_t * hdr_config, HDR_mem_usage_t * mem_usage)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_clock_offset(const HDR_instance_t * hdr_instance, int_t * ppm)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_leap_sec(HDR_instance_t * hdr_instance, HDR_sis_leap_sec_t * leap_sec)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_auto_align_set_config(HDR_auto_align_t * auto_align, const HDR_auto_align_config_t * config)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_demod_freq_offset(const HDR_instance_t * hdr_instance, int32_t * freq_offset)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_mrc_disable(const HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_test_reset_audio_errors(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_hybrid_program(HDR_instance_t * hdr_instance, bool * hybrid)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_all_data_services(HDR_instance_t * hdr_instance, HDR_sis_data_services_info_t * data_services_info)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

bool HDR_sis_time_gps_locked(HDR_instance_t * hdr_instance)
{
	bool ret = false;

	return ret;
}

int_t HDR_logger_enable(uint_t module, uint32_t group_mask)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

uint_t HDR_psd_get_max_length(HDR_instance_t * hdr_instance, HDR_psd_length_config_t config)
{
	uint_t ret = 0U;

	return ret;
}

int_t HDR_test_mrc_alignment_offset(HDR_instance_t * hdr_instance, int_t * offset)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_config(const HDR_instance_t * hdr_instance, HDR_config_t * hdr_config)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sig_get_service_list(HDR_instance_t * hdr_instance, HDR_sig_service_type_t service_type, HDR_sig_service_list_t * service_list)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_demod_cdno(const HDR_instance_t * hdr_instance, uint_t * cdno)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_set_blend_transition_time(HDR_blend_crossfade_t * blend_handle, uint_t transition_time)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_test_mrc_demod_disable(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_get_lot_pool_size(HDR_instance_t * hdr_instance, uint_t * mem_pool_size)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_psd_reset_max_length(HDR_instance_t * hdr_instance, HDR_psd_length_config_t config)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_blend_set_all_params(HDR_instance_t * hdr_instance, const HDR_blend_params_t * params)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_front_end_task(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_blend_align_progress_status(HDR_instance_t * hdr_instance, bool * align_status)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_blend_set_adv_param_actual(HDR_instance_t * hdr_instance, uint_t offset, uint_t size, uint_t value)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_psd_get_xhdr(HDR_instance_t * hdr_instance, HDR_program_t program_number, uint_t xhdr_num, HDR_psd_data_t * xhdr)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_set_filt_dsqm(const HDR_instance_t * hdr_instance, uint_t dsqm_value)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_blend_crossfade(HDR_blend_crossfade_t * blend_handle, HDR_pcm_stereo_t * dig, HDR_pcm_stereo_t * ana, bool blend_flag, HDR_pcm_stereo_t * * blended_audio)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_get_lot_object_body(HDR_instance_t * hdr_instance, uint_t port_number, uint_t object_lot_id, uint8_t * buffer, uint_t buffer_size, uint_t * bytes_written)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_get_port_data(HDR_instance_t * hdr_instance, uint_t port_number, HDR_aas_packet_info_t * packet_info, uint8_t * packet_buffer, uint_t buffer_size, uint_t * bytes_written)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_bit_proc_task(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_psd_get_genre(HDR_instance_t * hdr_instance, HDR_program_t program_number, HDR_psd_data_t * genre)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_audio_quality_report(HDR_instance_t * hdr_instance, HDR_audio_quality_report_t * report)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_test_reset_ber(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_flush(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

void HDR_bb_src_exec(HDR_bb_src_t * bb_src, int16c_t * bb_input_samples, uint_t num_input_samples, int16c_t * bb_output_samples, uint32_t * num_output_samples)
{
}

int_t HDR_test_get_audio_bw_status(HDR_instance_t * hdr_instance, HDR_test_audio_bw_status_t * audio_bw_status)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_disable_lot_reassembly(HDR_instance_t * hdr_instance, uint_t service_number, uint_t port_number)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_alert_get_tone_audio(HDR_instance_t * hdr_instance, HDR_pcm_stereo_t * pcm_output, uint_t num_samples, bool * finished)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

bool HDR_test_mrc_demod_enabled(HDR_instance_t * hdr_instance)
{
	bool ret = false;

	return ret;
}

int_t HDR_aas_disable_ports(HDR_instance_t * hdr_instance, const HDR_aas_port_list_t * port_list)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_psd_get_comment(HDR_instance_t * hdr_instance, HDR_program_t program_number, HDR_psd_comm_subfield_t subfield, HDR_psd_data_t * comment)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

uint8_t HDR_get_mrc_demod_active_state(HDR_instance_t * hdr_instance)
{
	uint8_t ret = 0U;

	return ret;
}

int_t HDR_blend_adjust_audio_delay(HDR_instance_t * hdr_instance, int_t sample_adjust)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sideband_inputs(const HDR_instance_t * hdr_instance, int16c_t * usb_in, int16c_t * lsb_in, uint_t num_samples)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_blend_get_adv_param_actual(HDR_instance_t * hdr_instance, uint_t offset, uint_t size, uint_t * output)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_raw_snr(const HDR_instance_t * hdr_instance, uint_t * raw_snr)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

HDR_auto_align_rc_t HDR_auto_align_exec(HDR_auto_align_t * auto_align, HDR_pcm_stereo_t * ana_audio, HDR_pcm_stereo_t * dig_audio, uint_t audio_quality, bool blend_flag, bool blend_alignInProg, int_t * time_offset, int_t * level_offset)
{
	HDR_auto_align_rc_t ret = HDR_AUTO_ALIGN_DISABLED;

	return ret;
}

int_t HDR_get_audio_output(HDR_instance_t * hdr_instance, HDR_pcm_stereo_t * pcm_output, uint_t num_samples, uint_t * audio_quality, bool * blend_flag)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_blend_get_param_actual(HDR_instance_t * hdr_instance, uint_t offset, uint_t size, uint_t * output)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

HDR_psd_fields_t HDR_psd_get_enabled_fields(HDR_instance_t * hdr_instance)
{
	HDR_psd_fields_t tPsdField;

	tPsdField.all = 0;

	return tPsdField;
}

int_t HDR_get_playing_program(HDR_instance_t * hdr_instance, HDR_program_t * program)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_get_lot_object_list(HDR_instance_t * hdr_instance, uint_t service_number, HDR_aas_lot_object_list_t * object_list)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_demod_task(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_psd_enable_fields(HDR_instance_t * hdr_instance, HDR_psd_fields_t enabled_fields)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_freq_offset(const HDR_instance_t * hdr_instance, int32_t * freq_offset)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sig_get_service_info(HDR_instance_t * hdr_instance, uint_t service_number, HDR_sig_service_info_t * service_info)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_enabled_basic_types(HDR_instance_t * hdr_instance, HDR_sis_enabled_basic_types_t * enabled_types)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_program_info(HDR_instance_t * hdr_instance, HDR_program_t program_number, HDR_sis_program_info_t * program_info)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_location_read_count(HDR_instance_t * hdr_instance, uint_t * count)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_available_programs(HDR_instance_t * hdr_instance, HDR_program_bitmap_t * avail_programs)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_blend_crossfade_reset(HDR_blend_crossfade_t * blend_handle)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sig_get_service_component(HDR_instance_t * hdr_instance, uint_t service_number, uint_t component_index, HDR_sig_service_component_t * component)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_station_id(HDR_instance_t * hdr_instance, HDR_sis_station_id_t * station_id)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_demod_clock_offset(const HDR_instance_t * hdr_instance, int_t * ppm)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_exciter_manuf_ver(HDR_instance_t * hdr_instance, HDR_sis_tx_manuf_ver_t * version_struct)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_get_demod_bb_sample_slips(const HDR_instance_t * hdr_instance, HDR_bb_sample_slips_t * sample_slips)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}


int_t HDR_audio_resampler_input(HDR_audio_resampler_t * audio_resampler, HDR_pcm_stereo_t * input)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

bool HDR_sis_crc_ok(HDR_instance_t * hdr_instance)
{
	bool ret = false;

	return ret;
}

HDR_audio_resampler_t* HDR_audio_resampler_init(void * audio_resampler_memory, HDR_audio_resampler_mode_t mode, void * mutex)
{
	HDR_audio_resampler_t* ret = audio_resampler_memory;

	return ret;
}

int_t HDR_set_band(HDR_instance_t * hdr_instance, HDR_tune_band_t band)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

void HDR_Logger_init(void * mutex)
{
}

int_t HDR_alert_get_message(HDR_instance_t * hdr_instance, HDR_alert_message_t * message)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_lot_get_object_list_by_name(HDR_instance_t * hdr_instance, uint_t service_number, const char * filename, HDR_aas_lot_object_list_t * object_list)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_alert_clear_message_status(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_station_short_name(HDR_instance_t * hdr_instance, HDR_sis_short_name_t * short_name)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_mrc_connect(HDR_instance_t * instance1, HDR_instance_t * instance2)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_enable_basic_types(HDR_instance_t * hdr_instance, HDR_sis_enabled_basic_types_t types)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

bool HDR_demod_hd_signal_acquired(const HDR_instance_t * hdr_instance)
{
	bool ret = false;

	return ret;
}

int_t HDR_get_dsqm(const HDR_instance_t * hdr_instance, HDR_dsqm_t * dsqm)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_reacquire(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_avail_data_serv_list(HDR_instance_t * hdr_instance, HDR_sis_avail_data_services_t * available_services)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_psd_set_max_length(HDR_instance_t * hdr_instance, HDR_psd_length_config_t config, uint_t length)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

uint_t HDR_sis_get_block_count(HDR_instance_t * hdr_instance)
{
	uint_t ret = 0U;

	return ret;
}

int_t HDR_test_get_ber(HDR_instance_t * hdr_instance, HDR_test_ber_t * ber)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_disable_all_ports(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_psd_get_artist(HDR_instance_t * hdr_instance, HDR_program_t program_number, HDR_psd_data_t * artist)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

HDR_psm_t HDR_get_primary_service_mode(const HDR_instance_t * hdr_instance)
{
	HDR_psm_t ret = HDR_FM_MP0;

	return ret;
}

int_t HDR_get_demod_filt_dsqm(const HDR_instance_t * hdr_instance, uint_t * filt_dsqm)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

HDR_bb_src_t* HDR_bb_src_init(void * bb_src_memory)
{
	HDR_bb_src_t* ret = bb_src_memory;

	return ret;
}

bool HDR_aas_lot_overflow(HDR_instance_t * hdr_instance)
{
	bool ret = false;

	return ret;
}

int_t HDR_psd_get_album(HDR_instance_t * hdr_instance, HDR_program_t program_number, HDR_psd_data_t * album)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

HDR_blend_crossfade_t* HDR_blend_crossfade_init(void * blend_mem)
{
	HDR_blend_crossfade_t* ret = blend_mem;

	return ret;
}

int_t HDR_set_playing_program(HDR_instance_t * hdr_instance, HDR_program_t program)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_station_slogan(HDR_instance_t * hdr_instance, HDR_sis_station_slogan_t * slogan)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

bool HDR_sis_acquired(HDR_instance_t * hdr_instance)
{
	bool ret = false;

	return ret;
}

int_t HDR_get_blend_control(HDR_instance_t * hdr_instance, HDR_blend_control_t * blend_control)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_universal_name(HDR_instance_t * hdr_instance, HDR_sis_univ_name_t * univ_name)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_sis_get_local_time(HDR_instance_t * hdr_instance, HDR_sis_local_time_t * local_time)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

HDR_auto_align_t* HDR_auto_align_init(void * auto_align_memory, uint_t size, const HDR_auto_align_config_t * config, HDR_audio_rate_t audio_rate)
{
	HDR_auto_align_t* ret = auto_align_memory;

	return ret;
}

int_t HDR_auto_align_get_status(const HDR_auto_align_t * auto_align, HDR_auto_align_status_t * status)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_aas_enable_ports(HDR_instance_t * hdr_instance, const HDR_aas_port_list_t * port_list)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_test_mrc_demod_enable(HDR_instance_t * hdr_instance)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

void HDR_bb_src_reset(HDR_bb_src_t * bb_src, HDR_bb_src_input_rate_t input_sample_rate, HDR_tune_band_t band)
{
}

void HDR_audio_resampler_set_src_offset(HDR_audio_resampler_t* audio_resampler, int32_t srcOffset)
{
}

uint32_t HDR_audio_resampler_avail_data(HDR_audio_resampler_t* audio_resampler)
{
	uint32_t ret = 0U;

	return ret;
}

void HDR_auto_align_set_holdoff(HDR_auto_align_t* auto_align, uint32_t holdoff)
{
}

int32_t HDR_bb_src_get_offset(HDR_bb_src_t* bb_src)
{
	int32_t ret = (int32_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_blend_set_adv_param_actual(HDR_instance_t* hdr_instance, uint_t offset, uint_t size, int_t value)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_blend_adjust_audio_delay(HDR_instance_t* hdr_instance, int_t sample_adjust, uint32_t* decodedAudioSamples)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int_t HDR_blend_set_flag(HDR_instance_t* hdr_instance, bool blendFlag)
{
	int_t ret = (int_t)HDR_NOT_INITIALIZED;

	return ret;
}

int HDR_get_mrc_instance(const HDR_instance_t* hdr_instance, HDR_instance_t** mrc_instance)
{
	int ret = (int)HDR_NOT_INITIALIZED;

	return ret;
}

/**********************************************************************************/
/****************************    UNUSED FUNCTIONS    ******************************/
/**********************************************************************************/
/* HDR_trace_enable */
/* HDR_get_space_used */
/* HDR_get_long_version_str */

/**********************************************************************************/
/**************************    CALLBACK FUNCTIONS    ******************************/
/**********************************************************************************/
//void (*post_task)(HDR_instance_t* hdr_instance, HDR_task_id_t task_id);
//int_t (*lock_mutex)(void* mutex, int_t timeout);
//void (*unlock_mutex)(void* mutex);
//int_t (*enter_critical_section)(const HDR_instance_t* hdr_instance);
//void (*exit_critical_section)(const HDR_instance_t* hdr_instance);
//void (*reacq_complete)(const HDR_instance_t* hdr_instance);
//int_t (*get_sys_time)(uint64_t* time_msec);
//void (*usleep)(uint_t usec);
//int_t (*queue_vit_job)(const HDR_instance_t* hdr_instance, int8_t* sd_in, uint32_t sd_count,
//                       uint32_t block_number, uint32_t frame_number, uint32_t timestamp,
//                       HDR_vit_code_rate_t code_rate, uint8_t* bits_out, HDR_vit_job_handle_t* vit_job);
//int_t (*query_vit_job)(const HDR_instance_t* hdr_instance, HDR_vit_job_handle_t vit_job, uint_t* error_count);
//int_t (*flush_vit_job)(const HDR_instance_t* hdr_instance, HDR_vit_job_handle_t vit_job);
//int_t (*mem_set_uncached)(const HDR_instance_t* hdr_instance, uint_t block_num);
//int_t (*mem_set_cached)(const HDR_instance_t* hdr_instance, uint_t block_num);
//void  (*error_handler)(const HDR_instance_t* hdr_instance, int_t error, intptr_t caller_func_addr, const char* func_name);
//bool  (*is_mps_alignment_finished)(const HDR_instance_t* hdr_instance);
//int_t (*logger_send)(uint8_t* data, uint_t length);

