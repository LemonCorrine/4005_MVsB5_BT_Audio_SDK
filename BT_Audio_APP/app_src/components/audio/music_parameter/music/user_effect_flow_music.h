/***************************************************
 * @file     user_effect_flow_music.h                      
 * @brief   auto generated                          
 * @author  ACPWorkbench: 3.13.1                 
 * @version V1.2.0                                  
 * @Created 2024-11-04T15:13:33                                      

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_MUSIC_H__
#define __USER_EFFECT_FLOW_MUSIC_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define MUSIC_ROBOEFFECT_LIB_VER "2.25.5"

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

    MUSIC_silence_detector_mic_ADDR = 0x81,
    MUSIC_mic_eq0_ADDR,
    MUSIC_mic_ns_ADDR,
    MUSIC_mic_EQ_ADDR,
    MUSIC_mic_drc_ADDR,
    MUSIC_mic_gain_ADDR,
    MUSIC_preGain_ADDR,
    MUSIC_silence_detector_music_ADDR,
    MUSIC_gain_control0_ADDR,
    MUSIC_noise_suppressor_expander0_ADDR,
    MUSIC_gain_control2_ADDR,
    MUSIC_eq1_ADDR,
    MUSIC_compander_ADDR,
    MUSIC_low_level_compressor1_ADDR,
    MUSIC_harmonic_exciter0_ADDR,
    MUSIC_mvbass_ADDR,
    MUSIC_3D_ADDR,
    MUSIC_eq2_ADDR,
    MUSIC_music_drc_ADDR,
    MUSIC_music_EQ_ADDR,
    MUSIC_gain_control1_ADDR,
    MUSIC_eq0_ADDR,
    MUSIC_low_level_compressor0_ADDR,
    MUSIC_gain_control3_ADDR,
    MUSIC_COUNT_ADDR,

} MUSIC_roboeffect_effect_list_enum;

extern const unsigned char user_effects_script_music[];

extern roboeffect_effect_list_info user_effect_list_music;

extern const roboeffect_effect_steps_table user_effect_steps_music;

extern uint32_t get_user_effects_script_len_music(void);

extern char *parameter_group_name_music[1];
extern const unsigned char user_effect_parameters_music_Music[];
extern const unsigned char user_module_parameters_music_Music[];
#endif/*__USER_EFFECT_FLOW_MUSIC_H__*/
