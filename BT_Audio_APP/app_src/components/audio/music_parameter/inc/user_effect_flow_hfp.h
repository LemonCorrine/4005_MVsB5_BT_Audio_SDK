/***************************************************
 * @file     hfp.h                      
 * @brief   auto generated                          
 * @author  ACPWorkbench: 3.5.4                 
 * @version V1.1.0                                  
 * @Created 2023-09-12T17:03:56                                      

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_HFP_H__
#define __USER_EFFECT_FLOW_HFP_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define HFP_ROBOEFFECT_LIB_VER "2.7.4"

typedef enum _HFP_roboeffect_io_enum
{
    HFP_SOURCE_MIC_SOURCE,
    HFP_SOURCE_APP_SOURCE,
    HFP_SOURCE_REMIND_SOURCE,

    HFP_SINK_DAC0_SINK,
    HFP_SINK_APP_SINK,
    HFP_SINK_STEREO_SINK,
} HFP_roboeffect_io_enum;


typedef enum _HFP_roboeffect_effect_list_enum{
    HFP_START_ADDR = 0x80,
    HFP_aec0_ADDR,
    HFP_noise_suppressor_blue0_ADDR,
    HFP_mic_EQ_ADDR,
    HFP_mic_drc_ADDR,
    HFP_mic_gain_ADDR,
    HFP_music_gain_ADDR,
    HFP_music_preEQ_ADDR,
    HFP_music_drc_ADDR,
    HFP_upmix_1to20_ADDR,
    HFP_pcm_delay0_ADDR,
    HFP_COUNT_ADDR,

} HFP_roboeffect_effect_list_enum;

extern const unsigned char user_effects_script_hfp[];

extern roboeffect_effect_list_info user_effect_list_hfp;

extern const roboeffect_effect_steps_table user_effect_steps_hfp;

extern const unsigned char user_effect_parameters_hfp_hfp[];

extern const unsigned char user_module_parameters_hfp_hfp[];
#endif/*__USER_EFFECT_FLOW_HFP_H__*/
