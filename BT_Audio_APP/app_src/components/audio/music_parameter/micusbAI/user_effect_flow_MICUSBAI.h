/***************************************************
 * @file     user_effect_flow_MICUSBAI.h                      
 * @brief   auto generated                          
 * @author  ACPWorkbench: 3.9.5                 
 * @version V1.2.0                                  
 * @Created 2024-04-01T13:47:44                                      

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_MICUSBAI_H__
#define __USER_EFFECT_FLOW_MICUSBAI_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define MICUSBAI_ROBOEFFECT_LIB_VER "2.17.7"

typedef enum _MICUSBAI_roboeffect_io_enum
{
    MICUSBAI_SOURCE_MIC_SOURCE,
    MICUSBAI_SOURCE_APP_SOURCE,

    MICUSBAI_SINK_DAC0_SINK,
    MICUSBAI_SINK_APP_SINK,
} MICUSBAI_roboeffect_io_enum;


typedef enum _MICUSBAI_roboeffect_effect_list_enum{
    MICUSBAI_downmix_2to1_0_ADDR = 0x81,
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

extern uint32_t get_user_effects_script_len_MICUSBAI(void);

extern char *parameter_group_name_MICUSBAI[1];
extern const unsigned char user_effect_parameters_MICUSBAI_MICUSBAI[];
extern const unsigned char user_module_parameters_MICUSBAI_MICUSBAI[];
#endif/*__USER_EFFECT_FLOW_MICUSBAI_H__*/
