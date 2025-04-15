/***************************************************
 * @file     Karaoke.h
 * @brief   auto generated                          
 * @author  ACPWorkbench: 3.6.11
 * @version V1.1.0                                  
 * @Created 2023-10-12T15:27:59

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_KARAOKE_H__
#define __USER_EFFECT_FLOW_KARAOKE_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define KARAOKE_ROBOEFFECT_LIB_VER "2.7.8"

typedef enum _KARAOKE_roboeffect_io_enum
{
    KARAOKE_SOURCE_APP_SOURCE,
    KARAOKE_SOURCE_MIC_SOURCE,
    KARAOKE_SOURCE_REMIND_SOURCE,
    KARAOKE_SOURCE_REC_SOURCE,

    KARAOKE_SINK_DAC0_SINK,
    KARAOKE_SINK_APP_SINK,
    KARAOKE_SINK_STEREO_SINK,
    KARAOKE_SINK_REC_SINK,
} KARAOKE_roboeffect_io_enum;


typedef enum _KARAOKE_roboeffect_effect_list_enum{
    KARAOKE_START_ADDR = 0x80,
    KARAOKE_gain_control0_ADDR,
    KARAOKE_noise_suppressor_expander0_ADDR,
    KARAOKE_compander0_ADDR,
    KARAOKE_low_level_compressor0_ADDR,
    KARAOKE_virtual_bass0_ADDR,
    KARAOKE_3D_ADDR,
    KARAOKE_harmonic_exciter0_ADDR,
    KARAOKE_stereo_widener0_ADDR,
    KARAOKE_eq0_ADDR,
    KARAOKE_drc1_ADDR,
    KARAOKE_eq1_ADDR,
    KARAOKE_downmix_2to10_ADDR,
    KARAOKE_gain_control1_ADDR,
    KARAOKE_noise_suppressor_expander1_ADDR,
    KARAOKE_silence_detector0_ADDR,
    KARAOKE_freq_shifter0_ADDR,
    KARAOKE_howling_suppressor0_ADDR,
    KARAOKE_howling_suppressor_fine0_ADDR,
    KARAOKE_gain_control7_ADDR,
    KARAOKE_pitch_shifter0_ADDR,
    KARAOKE_pitch_shifter_pro0_ADDR,
    KARAOKE_upmix_1to20_ADDR,
    KARAOKE_eq7_ADDR,
    KARAOKE_gain_control2_ADDR,
    KARAOKE_eq4_ADDR,
    KARAOKE_gain_control5_ADDR,
    KARAOKE_drc0_ADDR,
    KARAOKE_gain_control8_ADDR,
    KARAOKE_voice_changer0_ADDR,
    KARAOKE_voice_changer_pro0_ADDR,
    KARAOKE_gain_control12_ADDR,
    KARAOKE_auto_tune0_ADDR,
    KARAOKE_eq5_ADDR,
    KARAOKE_echo0_ADDR,
    KARAOKE_gain_control3_ADDR,
    KARAOKE_eq6_ADDR,
    KARAOKE_reverb0_ADDR,
    KARAOKE_reverb_plate0_ADDR,
    KARAOKE_gain_control4_ADDR,
    KARAOKE_gain_control6_ADDR,
    KARAOKE_gain_control9_ADDR,
    KARAOKE_eq9_ADDR,
    KARAOKE_drc2_ADDR,
    KARAOKE_gain_control10_ADDR,
    KARAOKE_gain_control11_ADDR,
    KARAOKE_gain_control14_ADDR,
    KARAOKE_COUNT_ADDR,

} KARAOKE_roboeffect_effect_list_enum;

extern const unsigned char user_effects_script_Karaoke[];

extern roboeffect_effect_list_info user_effect_list_Karaoke;

extern const roboeffect_effect_steps_table user_effect_steps_Karaoke;

#endif/*__USER_EFFECT_FLOW_KARAOKE_H__*/
