/***************************************************
 * @file     user_effect_flow_music.h  
 * @brief   auto generated  
 * @author  ACPWorkbench: 5.7.0
 * @version V1.2.0 
 * @Created 2026-07-20T10:16:38 

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_MUSIC_H__
#define __USER_EFFECT_FLOW_MUSIC_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define MUSIC_ROBOEFFECT_LIB_VER "2.47.1"

typedef enum _MUSIC_roboeffect_io_enum
{
    MUSIC_SOURCE_MIC_SOURCE,
    MUSIC_SOURCE_APP_SOURCE,
    MUSIC_SOURCE_REMIND_SOURCE,
    MUSIC_SOURCE_REC_SOURCE,

    MUSIC_SINK_DAC0_SINK,
    MUSIC_SINK_APP_SINK,
    MUSIC_SINK_STEREO_SINK,
    MUSIC_SINK_SPDIF_SINK,
    MUSIC_SINK_REC_SINK,
} MUSIC_roboeffect_io_enum;


typedef enum _MUSIC_roboeffect_effect_list_enum{

    MUSIC_upmix_1to2_0_ADDR = 0x81,
    MUSIC_silence_detector_mic_ADDR = 0x82,
    MUSIC_preGain_ADDR = 0x83,
    MUSIC_silence_detector_music_ADDR = 0x84,
    MUSIC_gain_control3_ADDR = 0x85,
    MUSIC_mic_eq0_ADDR = 0x86,
    MUSIC_mic_ns_ADDR = 0x87,
    MUSIC_mic_EQ_ADDR = 0x88,
    MUSIC_mic_drc_ADDR = 0x89,
    MUSIC_mic_gain_ADDR = 0x8a,
    MUSIC_gain_control0_ADDR = 0x8b,
    MUSIC_noise_suppressor_expander0_ADDR = 0x8c,
    MUSIC_gain_control2_ADDR = 0x8d,
    MUSIC_eq1_ADDR = 0x8e,
    MUSIC_compander_ADDR = 0x8f,
    MUSIC_low_level_compressor1_ADDR = 0x90,
    MUSIC_harmonic_exciter0_ADDR = 0x91,
    MUSIC_mvbass_ADDR = 0x92,
    MUSIC_3D_ADDR = 0x93,
    MUSIC_eq2_ADDR = 0x94,
    MUSIC_music_drc_ADDR = 0x95,
    MUSIC_music_EQ_ADDR = 0x96,
    MUSIC_gain_control1_ADDR = 0x97,
    MUSIC_eq0_ADDR = 0x98,
    MUSIC_low_level_compressor0_ADDR = 0x99,
    MUSIC_COUNT_ADDR,

} MUSIC_roboeffect_effect_list_enum;

extern const char chart_name_music[];

extern const unsigned char user_effects_script_music[];

extern roboeffect_effect_list_info user_effect_list_music;

extern const roboeffect_effect_steps_table user_effect_steps_music;

extern uint32_t get_user_effects_script_len_music(void);

extern char *parameter_group_name_music[1];
extern const unsigned char user_effect_parameters_music_Music[];
extern const unsigned char user_module_parameters_music_Music[];
#endif/*__USER_EFFECT_FLOW_MUSIC_H__*/
