/***************************************************
 * @file     bypass.h                      
 * @brief   auto generated                          
 * @author  ACPWorkbench: 3.6.15                 
 * @version V1.1.0                                  
 * @Created 2023-10-27T11:18:31                                      

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_BYPASS_H__
#define __USER_EFFECT_FLOW_BYPASS_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define BYPASS_ROBOEFFECT_LIB_VER "2.7.11"

typedef enum _BYPASS_roboeffect_io_enum
{
    BYPASS_SOURCE_MIC_SOURCE,
    BYPASS_SOURCE_APP_SOURCE,
    BYPASS_SOURCE_REMIND_SOURCE,
    BYPASS_SOURCE_REC_SOURCE,

    BYPASS_SINK_APP_SINK,
    BYPASS_SINK_DAC0_SINK,
    BYPASS_SINK_STEREO_SINK,
    BYPASS_SINK_REC_SINK,
} BYPASS_roboeffect_io_enum;


typedef enum _BYPASS_roboeffect_effect_list_enum{
    BYPASS_START_ADDR = 0x80,
    BYPASS_mic_gain_ADDR,
    BYPASS_gain_control1_ADDR,
    BYPASS_gain_control0_ADDR,
    BYPASS_COUNT_ADDR,

} BYPASS_roboeffect_effect_list_enum;

extern const unsigned char user_effects_script_bypass[];

extern roboeffect_effect_list_info user_effect_list_bypass;

extern const roboeffect_effect_steps_table user_effect_steps_bypass;

#endif/*__USER_EFFECT_FLOW_BYPASS_H__*/
