/***************************************************
 * @file     user_effect_flow_MICUSBAI.h                      
 * @brief   auto generated                          
 * @author  ACPWorkbench: 3.8.6                 
 * @version V1.1.0                                  
 * @Created 2024-02-28T10:51:58                                      

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_MICUSBAI_H__
#define __USER_EFFECT_FLOW_MICUSBAI_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define MICUSBAI_ROBOEFFECT_LIB_VER "2.15.0"

typedef enum _MICUSBAI_roboeffect_io_enum
{
    MICUSBAI_SOURCE_MIC_SOURCE,
    MICUSBAI_SOURCE_APP_SOURCE,

    MICUSBAI_SINK_DAC0_SINK,
    MICUSBAI_SINK_APP_SINK,
} MICUSBAI_roboeffect_io_enum;


typedef enum _MICUSBAI_roboeffect_effect_list_enum{
    MICUSBAI_START_ADDR = 0x80,
    MICUSBAI_downmix_2to1_0_ADDR,
    MICUSBAI_ai_denoise0_ADDR,
    MICUSBAI_upmix_1to2_0_ADDR,
    MICUSBAI_mic_eq0_ADDR,
    MICUSBAI_mic_ns_ADDR,
    MICUSBAI_mic_EQ_ADDR,
    MICUSBAI_mic_drc_ADDR,
    MICUSBAI_mic_gain_ADDR,
    MICUSBAI_COUNT_ADDR,

} MICUSBAI_roboeffect_effect_list_enum;

extern const unsigned char user_effects_script_MICUSBAI[];

extern roboeffect_effect_list_info user_effect_list_MICUSBAI;

extern const roboeffect_effect_steps_table user_effect_steps_MICUSBAI;

#endif/*__USER_EFFECT_FLOW_MICUSBAI_H__*/
