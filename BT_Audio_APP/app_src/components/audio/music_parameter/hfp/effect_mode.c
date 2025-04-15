#include "user_effect_flow_hfp.h"
#include "user_effect_parameter.h"

extern uint32_t get_user_effects_script_len_hfp(void);
extern const unsigned char user_effect_parameters_hfp_hfp[];
extern const unsigned char user_module_parameters_hfp_hfp[];

static const AUDIOEFFECT_EFFECT_PARA effect_para[] =
{
	{
		.user_effect_name = (uint8_t *)"Hfp",
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_hfp,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_hfp,
		.user_effects_script = (uint8_t *)user_effects_script_hfp,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_hfp_hfp,
		.user_module_parameters = (uint8_t *)user_module_parameters_hfp_hfp,
		.get_user_effects_script_len = get_user_effects_script_len_hfp,
	}
};

const AUDIOEFFECT_EFFECT_PARA_TABLE hfp_mode =
{
	//ROBOEFFECT effect ID 通过这个ID来搜索匹配
	.effect_id    = EFFECT_MODE_HFP_AEC ,
	//该框图下面有1个音效
	.effect_id_count = 1,

	//ROBOEFFECT effect 音效地址映射
	.effect_addr =
	{
		.APP_SOURCE_GAIN_ADDR = HFP_music_gain_ADDR,
		.MIC_SOURCE_GAIN_ADDR = HFP_mic_gain_ADDR,
		.SILENCE_DETECTOR_ADDR = HFP_silence_detector_mic_ADDR,
		.SILENCE_DETECTOR_MUSIC_ADDR = HFP_silence_detector_music_ADDR,
	},

	//ROBOEFFECT effect SOURCE映射
	.audioeffect_source =
	{
		.mic_source = HFP_SOURCE_MIC_SOURCE,
		.app_source = HFP_SOURCE_APP_SOURCE,
		.remind_source = HFP_SOURCE_REMIND_SOURCE,
		.rec_source = HFP_SOURCE_REC_SOURCE,
		.usb_source = AUDIOCORE_SOURCE_SINK_ERROR,
		.i2s_mix_source = AUDIOCORE_SOURCE_SINK_ERROR,
		.linein_mix_source = AUDIOCORE_SOURCE_SINK_ERROR,
	},

	//ROBOEFFECT effect SINK映射
	.audioeffect_sink =
	{
		.dac0_sink = HFP_SINK_DAC0_SINK,
		.app_sink = HFP_SINK_APP_SINK,
		.stereo_sink = HFP_SINK_STEREO_SINK,
		.rec_sink = HFP_SINK_REC_SINK,
		.i2s_mix_sink = AUDIOCORE_SOURCE_SINK_ERROR,
		.spdif_sink = HFP_SINK_SPDIF_SINK,
	},

	//ROBOEFFECT effect 参数
	.audioeffect_para = (AUDIOEFFECT_EFFECT_PARA *)&effect_para[0],
};
