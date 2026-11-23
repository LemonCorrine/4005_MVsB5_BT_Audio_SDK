/***************************************************
 * @file     user_effect_flow_Karaoke.h  
 * @brief   auto generated  
 * @author  ACPWorkbench: 5.1.8
 * @version V1.2.0 
 * @Created 2026-04-22T17:53:40 

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_KARAOKE_H__
#define __USER_EFFECT_FLOW_KARAOKE_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define KARAOKE_ROBOEFFECT_LIB_VER "2.44.0"

typedef enum _KARAOKE_roboeffect_io_enum
{
    KARAOKE_SOURCE_APP_SOURCE,
    KARAOKE_SOURCE_USB_SOURCE,
    KARAOKE_SOURCE_LINEIN_MIX_SOURCE,
    KARAOKE_SOURCE_MIC_SOURCE,
    KARAOKE_SOURCE_I2S_MIX_SOURCE,
    KARAOKE_SOURCE_I2S_MIX2_SOURCE,
    KARAOKE_SOURCE_REMIND_SOURCE,
    KARAOKE_SOURCE_REC_SOURCE,

    KARAOKE_SINK_DAC0_SINK,
    KARAOKE_SINK_APP_SINK,
    KARAOKE_SINK_STEREO_SINK,
    KARAOKE_SINK_REC_SINK,
    KARAOKE_SINK_I2S_MIX_SINK,
    KARAOKE_SINK_SPDIF_SINK,
} KARAOKE_roboeffect_io_enum;


typedef enum _KARAOKE_roboeffect_effect_list_enum{

    KARAOKE_preGain_ADDR = 0x81,
    KARAOKE_silence_detector_music_ADDR = 0x82,
    KARAOKE_gain_control13_ADDR = 0x83,
    KARAOKE_downmix_2to10_ADDR = 0x84,
    KARAOKE_gain_control1_ADDR = 0x85,
    KARAOKE_noise_suppressor_expander1_ADDR = 0x86,
    KARAOKE_silence_detector0_ADDR = 0x87,
    KARAOKE_gain_control0_ADDR = 0x88,
    KARAOKE_noise_suppressor_expander0_ADDR = 0x89,
    KARAOKE_compander0_ADDR = 0x8a,
    KARAOKE_low_level_compressor0_ADDR = 0x8b,
    KARAOKE_virtual_bass0_ADDR = 0x8c,
    KARAOKE_3D_ADDR = 0x8d,
    KARAOKE_harmonic_exciter0_ADDR = 0x8e,
    KARAOKE_stereo_widener0_ADDR = 0x8f,
    KARAOKE_eq0_ADDR = 0x90,
    KARAOKE_drc1_ADDR = 0x91,
    KARAOKE_eq1_ADDR = 0x92,
    KARAOKE_freq_shifter0_ADDR = 0x93,
    KARAOKE_howling_suppressor0_ADDR = 0x94,
    KARAOKE_howling_suppressor_fine0_ADDR = 0x95,
    KARAOKE_gain_control7_ADDR = 0x96,
    KARAOKE_pitch_shifter0_ADDR = 0x97,
    KARAOKE_pitch_shifter_pro0_ADDR = 0x98,
    KARAOKE_upmix_1to20_ADDR = 0x99,
    KARAOKE_eq7_ADDR = 0x9a,
    KARAOKE_gain_control2_ADDR = 0x9b,
    KARAOKE_eq4_ADDR = 0x9c,
    KARAOKE_gain_control5_ADDR = 0x9d,
    KARAOKE_drc0_ADDR = 0x9e,
    KARAOKE_gain_control8_ADDR = 0x9f,
    KARAOKE_voice_changer0_ADDR = 0xa0,
    KARAOKE_voice_changer_pro0_ADDR = 0xa1,
    KARAOKE_gain_control12_ADDR = 0xa2,
    KARAOKE_auto_tune0_ADDR = 0xa3,
    KARAOKE_eq5_ADDR = 0xa4,
    KARAOKE_echo0_ADDR = 0xa5,
    KARAOKE_gain_control3_ADDR = 0xa6,
    KARAOKE_eq6_ADDR = 0xa7,
    KARAOKE_reverb0_ADDR = 0xa8,
    KARAOKE_reverb_plate0_ADDR = 0xa9,
    KARAOKE_gain_control4_ADDR = 0xaa,
    KARAOKE_gain_control6_ADDR = 0xab,
    KARAOKE_gain_control9_ADDR = 0xac,
    KARAOKE_eq9_ADDR = 0xad,
    KARAOKE_drc2_ADDR = 0xae,
    KARAOKE_gain_control10_ADDR = 0xaf,
    KARAOKE_gain_control11_ADDR = 0xb0,
    KARAOKE_gain_control14_ADDR = 0xb1,
    KARAOKE_COUNT_ADDR,

} KARAOKE_roboeffect_effect_list_enum;

extern const char chart_name_Karaoke[];

extern const unsigned char user_effects_script_Karaoke[];

extern roboeffect_effect_list_info user_effect_list_Karaoke;

extern const roboeffect_effect_steps_table user_effect_steps_Karaoke;

extern uint32_t get_user_effects_script_len_Karaoke(void);

extern char *parameter_group_name_Karaoke[7];
extern const unsigned char user_effect_parameters_Karaoke_HunXiang[];
extern const unsigned char user_module_parameters_Karaoke_HunXiang[];
extern const unsigned char user_effect_parameters_Karaoke_DianYin[];
extern const unsigned char user_module_parameters_Karaoke_DianYin[];
extern const unsigned char user_effect_parameters_Karaoke_MoYin[];
extern const unsigned char user_module_parameters_Karaoke_MoYin[];
extern const unsigned char user_effect_parameters_Karaoke_HanMai[];
extern const unsigned char user_module_parameters_Karaoke_HanMai[];
extern const unsigned char user_effect_parameters_Karaoke_NanBianNv[];
extern const unsigned char user_module_parameters_Karaoke_NanBianNv[];
extern const unsigned char user_effect_parameters_Karaoke_NvBianNan[];
extern const unsigned char user_module_parameters_Karaoke_NvBianNan[];
extern const unsigned char user_effect_parameters_Karaoke_WaWaYin[];
extern const unsigned char user_module_parameters_Karaoke_WaWaYin[];
#endif/*__USER_EFFECT_FLOW_KARAOKE_H__*/
