/***************************************************
 * @file     mic.h                      
 * @brief   auto generated                          
 * @author  ACPWorkbench: 3.7.0                 
 * @version V1.1.0                                  
 * @Created 2023-12-01T13:40:02                                      

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_MIC_H__
#define __USER_EFFECT_FLOW_MIC_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define MIC_ROBOEFFECT_LIB_VER "2.12.0"

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
    MIC_START_ADDR = 0x80,
    MIC_mic_eq0_ADDR,
    MIC_mic_ns_ADDR,
    MIC_mic_EQ_ADDR,
    MIC_mic_drc_ADDR,
    MIC_mic_gain_ADDR,
    MIC_gain_control0_ADDR,
    MIC_noise_suppressor_expander0_ADDR,
    MIC_gain_control2_ADDR,
    MIC_eq1_ADDR,
    MIC_compander_ADDR,
    MIC_low_level_compressor1_ADDR,
    MIC_harmonic_exciter0_ADDR,
    MIC_mvbass_ADDR,
    MIC_3D_ADDR,
    MIC_eq2_ADDR,
    MIC_music_drc_ADDR,
    MIC_music_EQ_ADDR,
    MIC_gain_control1_ADDR,
    MIC_eq0_ADDR,
    MIC_low_level_compressor0_ADDR,
    MIC_remind_gain_control_ADDR,
    MIC_COUNT_ADDR,

} MIC_roboeffect_effect_list_enum;

extern const unsigned char user_effects_script_mic[];

extern roboeffect_effect_list_info user_effect_list_mic;

extern const roboeffect_effect_steps_table user_effect_steps_mic;

#endif/*__USER_EFFECT_FLOW_MIC_H__*/
