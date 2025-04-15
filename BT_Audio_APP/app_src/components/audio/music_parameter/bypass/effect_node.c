#include "user_effect_flow_bypass.h"
#include "user_effect_parameter.h"

extern uint32_t get_user_effects_script_len_bypass(void);
extern const unsigned char user_effect_parameters_bypass_bypass[];
extern const unsigned char user_module_parameters_bypass_bypass[];

static const ROBOEFFECT_EFFECT_PARA effect_para[] =
{
	{
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_bypass,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_bypass,
		.user_effects_script = (uint8_t *)user_effects_script_bypass,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_bypass_bypass,
		.user_module_parameters = (uint8_t *)user_module_parameters_bypass_bypass,
		.get_user_effects_script_len = get_user_effects_script_len_bypass,
	}
};

const ROBOEFFECT_EFFECT_PARA_TABLE bypass_node =
{
	//ROBOEFFECT effect ID 通过这个ID来搜索匹配
	.effect_id    = EFFECT_MODE_BYPASS,
	//该框图下面有1个音效
	.effect_id_count = 1,

	//ROBOEFFECT effect 音效地址映射
	.effect_addr =
	{

	},
	//ROBOEFFECT effect SOURCE映射
	.roboeffect_source =
	{
		.mic_source = BYPASS_SOURCE_MIC_SOURCE,
		.app_source = BYPASS_SOURCE_APP_SOURCE,
		.remind_source = BYPASS_SOURCE_REMIND_SOURCE,
		.rec_source = BYPASS_SOURCE_REC_SOURCE,
		.usb_source = AUDIOCORE_SOURCE_SINK_ERROR,
		.i2s_mix_source = AUDIOCORE_SOURCE_SINK_ERROR,
		.linein_mix_source = AUDIOCORE_SOURCE_SINK_ERROR,
	},

	//ROBOEFFECT effect SINK映射
	.roboeffect_sink =
	{
		.dac0_sink = BYPASS_SINK_DAC0_SINK,
		.app_sink = BYPASS_SINK_APP_SINK,
		.stereo_sink = BYPASS_SINK_STEREO_SINK,
		.rec_sink = BYPASS_SINK_REC_SINK,
		.i2s_mix_sink = AUDIOCORE_SOURCE_SINK_ERROR,
	},

	//ROBOEFFECT effect 参数
	.roboeffect_para = &effect_para[0],
};
