/***************************************************
 * @file     user_effect_flow_uac.h  
 * @brief   auto generated  
 * @author  ACPWorkbench: 5.1.8
 * @version V1.2.0 
 * @Created 2026-04-22T17:53:43 

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_UAC_H__
#define __USER_EFFECT_FLOW_UAC_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define UAC_ROBOEFFECT_LIB_VER "2.44.0"

typedef enum _UAC_roboeffect_io_enum
{
    UAC_SOURCE_MIC_SOURCE,
    UAC_SOURCE_USB_HOST_SOURCE,
    UAC_SOURCE_APP_SOURCE,
    UAC_SOURCE_REMIND_SOURCE,
    UAC_SOURCE_REC_SOURCE,

    UAC_SINK_DAC0_SINK,
    UAC_SINK_APP_SINK,
    UAC_SINK_STEREO_SINK,
    UAC_SINK_SPDIF_SINK,
    UAC_SINK_REC_SINK,
    UAC_SINK_USB_HOST_SINK,
} UAC_roboeffect_io_enum;


typedef enum _UAC_roboeffect_effect_list_enum{

    UAC_upmix_1to2_0_ADDR = 0x81,
    UAC_silence_detector_mic_ADDR = 0x82,
    UAC_preGain_ADDR = 0x83,
    UAC_silence_detector_music_ADDR = 0x84,
    UAC_remind_gain_control_ADDR = 0x85,
    UAC_mic_eq0_ADDR = 0x86,
    UAC_mic_ns_ADDR = 0x87,
    UAC_mic_EQ_ADDR = 0x88,
    UAC_mic_drc_ADDR = 0x89,
    UAC_mic_gain_ADDR = 0x8a,
    UAC_gain_control0_ADDR = 0x8b,
    UAC_noise_suppressor_expander0_ADDR = 0x8c,
    UAC_gain_control2_ADDR = 0x8d,
    UAC_eq1_ADDR = 0x8e,
    UAC_compander_ADDR = 0x8f,
    UAC_low_level_compressor1_ADDR = 0x90,
    UAC_harmonic_exciter0_ADDR = 0x91,
    UAC_mvbass_ADDR = 0x92,
    UAC_3D_ADDR = 0x93,
    UAC_eq2_ADDR = 0x94,
    UAC_music_drc_ADDR = 0x95,
    UAC_music_EQ_ADDR = 0x96,
    UAC_gain_control1_ADDR = 0x97,
    UAC_eq0_ADDR = 0x98,
    UAC_low_level_compressor0_ADDR = 0x99,
    UAC_COUNT_ADDR,

} UAC_roboeffect_effect_list_enum;

extern const char chart_name_uac[];

extern const unsigned char user_effects_script_uac[];

extern roboeffect_effect_list_info user_effect_list_uac;

extern const roboeffect_effect_steps_table user_effect_steps_uac;

extern uint32_t get_user_effects_script_len_uac(void);

extern char *parameter_group_name_uac[1];
extern const unsigned char user_effect_parameters_uac_Uac[];
extern const unsigned char user_module_parameters_uac_Uac[];
#endif/*__USER_EFFECT_FLOW_UAC_H__*/
