/***************************************************
 * @file     user_effect_flow_hfp.h  
 * @brief   auto generated  
 * @author  ACPWorkbench: 5.1.8
 * @version V1.2.0 
 * @Created 2026-04-22T17:53:40 

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_HFP_H__
#define __USER_EFFECT_FLOW_HFP_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define HFP_ROBOEFFECT_LIB_VER "2.44.0"

typedef enum _HFP_roboeffect_io_enum
{
    HFP_SOURCE_MIC_SOURCE,
    HFP_SOURCE_APP_SOURCE,
    HFP_SOURCE_REMIND_SOURCE,
    HFP_SOURCE_REC_SOURCE,

    HFP_SINK_DAC0_SINK,
    HFP_SINK_APP_SINK,
    HFP_SINK_STEREO_SINK,
    HFP_SINK_SPDIF_SINK,
    HFP_SINK_REC_SINK,
} HFP_roboeffect_io_enum;


typedef enum _HFP_roboeffect_effect_list_enum{

    HFP_silence_detector_mic_ADDR = 0x81,
    HFP_aec0_ADDR = 0x82,
    HFP_noise_suppressor_blue0_ADDR = 0x83,
    HFP_mic_EQ_ADDR = 0x84,
    HFP_mic_drc_ADDR = 0x85,
    HFP_mic_gain_ADDR = 0x86,
    HFP_preGain_ADDR = 0x87,
    HFP_silence_detector_music_ADDR = 0x88,
    HFP_music_preEQ_ADDR = 0x89,
    HFP_music_drc_ADDR = 0x8a,
    HFP_pcm_delay0_ADDR = 0x8b,
    HFP_music_gain_ADDR = 0x8c,
    HFP_upmix_1to2_0_ADDR = 0x8d,
    HFP_remind_gain_control_ADDR = 0x8e,
    HFP_COUNT_ADDR,

} HFP_roboeffect_effect_list_enum;

extern const char chart_name_hfp[];

extern const unsigned char user_effects_script_hfp[];

extern roboeffect_effect_list_info user_effect_list_hfp;

extern const roboeffect_effect_steps_table user_effect_steps_hfp;

extern uint32_t get_user_effects_script_len_hfp(void);

extern char *parameter_group_name_hfp[1];
extern const unsigned char user_effect_parameters_hfp_Hfp[];
extern const unsigned char user_module_parameters_hfp_Hfp[];
#endif/*__USER_EFFECT_FLOW_HFP_H__*/
