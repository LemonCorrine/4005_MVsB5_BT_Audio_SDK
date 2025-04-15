#include "user_effect_flow_mic.h"
#include "user_effect_parameter.h"

extern uint32_t get_user_effects_script_len_mic(void);
extern const unsigned char user_effect_parameters_mic_mic[];
extern const unsigned char user_module_parameters_mic_mic[];

static const AUDIOEFFECT_EFFECT_PARA effect_para[] =
{
	{
		.user_effect_name = (uint8_t *)"Mic",
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_mic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_mic,
		.user_effects_script = (uint8_t *)user_effects_script_mic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_mic_mic,
		.user_module_parameters = (uint8_t *)user_module_parameters_mic_mic,
		.get_user_effects_script_len = get_user_effects_script_len_mic,
	}
};

const AUDIOEFFECT_EFFECT_PARA_TABLE mic_node =
{
	//ROBOEFFECT effect ID 通过这个ID来搜索匹配
	.effect_id    = EFFECT_MODE_MIC ,
	//该框图下面有1个音效
	.effect_id_count = 1,

	//ROBOEFFECT effect 音效地址映射
	.effect_addr =
	{
		.APP_SOURCE_GAIN_ADDR = MIC_gain_control0_ADDR,
		.MIC_SOURCE_GAIN_ADDR = MIC_mic_gain_ADDR,
		.REMIND_SOURCE_GAIN_ADDR = MIC_remind_gain_control_ADDR,
	},

	//ROBOEFFECT effect SOURCE映射
	.audioeffect_source =
	{
		.mic_source = MIC_SOURCE_MIC_SOURCE,
		.app_source = MIC_SOURCE_APP_SOURCE,
		.remind_source = MIC_SOURCE_REMIND_SOURCE,
		.rec_source = MIC_SOURCE_REC_SOURCE,
		.usb_source = AUDIOCORE_SOURCE_SINK_ERROR,
		.i2s_mix_source = AUDIOCORE_SOURCE_SINK_ERROR,
		.linein_mix_source = AUDIOCORE_SOURCE_SINK_ERROR,
	},

	//ROBOEFFECT effect SINK映射
	.audioeffect_sink =
	{
		.dac0_sink = MIC_SINK_DAC0_SINK,
		.app_sink = MIC_SINK_APP_SINK,
		.stereo_sink = MIC_SINK_STEREO_SINK,
		.rec_sink = MIC_SINK_REC_SINK,
		.i2s_mix_sink = AUDIOCORE_SOURCE_SINK_ERROR,
		.spdif_sink = MIC_SINK_SPDIF_SINK,
	},

	//ROBOEFFECT effect 参数
	.audioeffect_para = (AUDIOEFFECT_EFFECT_PARA *)&effect_para[0],
};
