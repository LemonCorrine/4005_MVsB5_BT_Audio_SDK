/***************************************************
 * @file     user_effect_flow_mic.h  
 * @brief   auto generated  
 * @author  ACPWorkbench: 5.1.8
 * @version V1.2.0 
 * @Created 2026-04-22T17:53:41 

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_MIC_H__
#define __USER_EFFECT_FLOW_MIC_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define MIC_ROBOEFFECT_LIB_VER "2.44.0"

typedef enum _MIC_roboeffect_io_enum
{
    MIC_SOURCE_MIC_SOURCE,
    MIC_SOURCE_APP_SOURCE,
    MIC_SOURCE_REMIND_SOURCE,
    MIC_SOURCE_REC_SOURCE,

    MIC_SINK_DAC0_SINK,
    MIC_SINK_APP_SINK,
    MIC_SINK_STEREO_SINK,
    MIC_SINK_SPDIF_SINK,
    MIC_SINK_REC_SINK,
} MIC_roboeffect_io_enum;


typedef enum _MIC_roboeffect_effect_list_enum{

    MIC_upmix_1to2_0_ADDR = 0x81,
    MIC_silence_detector_mic_ADDR = 0x82,
    MIC_preGain_ADDR = 0x83,
    MIC_silence_detector_music_ADDR = 0x84,
    MIC_remind_gain_control_ADDR = 0x85,
    MIC_mic_eq0_ADDR = 0x86,
    MIC_mic_ns_ADDR = 0x87,
    MIC_mic_EQ_ADDR = 0x88,
    MIC_mic_drc_ADDR = 0x89,
    MIC_mic_gain_ADDR = 0x8a,
    MIC_gain_control0_ADDR = 0x8b,
    MIC_noise_suppressor_expander0_ADDR = 0x8c,
    MIC_gain_control2_ADDR = 0x8d,
    MIC_eq1_ADDR = 0x8e,
    MIC_compander_ADDR = 0x8f,
    MIC_low_level_compressor1_ADDR = 0x90,
    MIC_harmonic_exciter0_ADDR = 0x91,
    MIC_mvbass_ADDR = 0x92,
    MIC_3D_ADDR = 0x93,
    MIC_eq2_ADDR = 0x94,
    MIC_music_drc_ADDR = 0x95,
    MIC_music_EQ_ADDR = 0x96,
    MIC_gain_control1_ADDR = 0x97,
    MIC_eq0_ADDR = 0x98,
    MIC_low_level_compressor0_ADDR = 0x99,
    MIC_COUNT_ADDR,

} MIC_roboeffect_effect_list_enum;

extern const char chart_name_mic[];

extern const unsigned char user_effects_script_mic[];

extern roboeffect_effect_list_info user_effect_list_mic;

extern const roboeffect_effect_steps_table user_effect_steps_mic;

extern uint32_t get_user_effects_script_len_mic(void);

extern char *parameter_group_name_mic[1];
extern const unsigned char user_effect_parameters_mic_Mic[];
extern const unsigned char user_module_parameters_mic_Mic[];
#endif/*__USER_EFFECT_FLOW_MIC_H__*/
