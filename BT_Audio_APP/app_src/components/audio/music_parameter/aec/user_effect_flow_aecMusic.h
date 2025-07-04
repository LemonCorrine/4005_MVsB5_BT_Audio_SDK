/***************************************************
 * @file     user_effect_flow_aecMusic.h                      
 * @brief   auto generated                          
 * @author  ACPWorkbench: 3.16.6                 
 * @version V1.2.0                                  
 * @Created 2025-03-11T17:47:34                                      

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_AECMUSIC_H__
#define __USER_EFFECT_FLOW_AECMUSIC_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define AECMUSIC_ROBOEFFECT_LIB_VER "2.28.0"

typedef enum _AECMUSIC_roboeffect_io_enum
{
    AECMUSIC_SOURCE_APP_SOURCE,
    AECMUSIC_SOURCE_I2S_MIX_SOURCE,
    AECMUSIC_SOURCE_REMIND_SOURCE,

    AECMUSIC_SINK_DAC0_SINK,
    AECMUSIC_SINK_AEC_REF_SINK,
    AECMUSIC_SINK_STEREO_SINK,
    AECMUSIC_SINK_I2S_MIX_SINK,
} AECMUSIC_roboeffect_io_enum;


typedef enum _AECMUSIC_roboeffect_effect_list_enum{

    AECMUSIC_preGain_ADDR = 0x81,
    AECMUSIC_gain_control0_ADDR,
    AECMUSIC_music_EQ_ADDR,
    AECMUSIC_drc0_ADDR,
    AECMUSIC_gain_control1_ADDR,
    AECMUSIC_gain_control3_ADDR,
    AECMUSIC_downmix_2to1_0_ADDR,
    AECMUSIC_remind_gain_control_ADDR,
    AECMUSIC_COUNT_ADDR,

} AECMUSIC_roboeffect_effect_list_enum;

extern const char chart_name_aecMusic[];

extern const unsigned char user_effects_script_aecMusic[];

extern roboeffect_effect_list_info user_effect_list_aecMusic;

extern const roboeffect_effect_steps_table user_effect_steps_aecMusic;

extern uint32_t get_user_effects_script_len_aecMusic(void);

extern char *parameter_group_name_aecMusic[7];
extern const unsigned char user_effect_parameters_aecMusic_AEC_Music[];
extern const unsigned char user_module_parameters_aecMusic_AEC_Music[];
extern const unsigned char user_effect_parameters_aecMusic_AEC_Music_A[];
extern const unsigned char user_module_parameters_aecMusic_AEC_Music_A[];
extern const unsigned char user_effect_parameters_aecMusic_AEC_Music_B[];
extern const unsigned char user_module_parameters_aecMusic_AEC_Music_B[];
extern const unsigned char user_effect_parameters_aecMusic_AEC_Music_C[];
extern const unsigned char user_module_parameters_aecMusic_AEC_Music_C[];
extern const unsigned char user_effect_parameters_aecMusic_AEC_Music_D[];
extern const unsigned char user_module_parameters_aecMusic_AEC_Music_D[];
extern const unsigned char user_effect_parameters_aecMusic_AEC_Music_E[];
extern const unsigned char user_module_parameters_aecMusic_AEC_Music_E[];
extern const unsigned char user_effect_parameters_aecMusic_AEC_Music_F[];
extern const unsigned char user_module_parameters_aecMusic_AEC_Music_F[];
#endif/*__USER_EFFECT_FLOW_AECMUSIC_H__*/
