#include "user_effect_flow_hfp.h"
#include "user_effect_parameter.h"

extern uint32_t get_user_effects_script_len_hfp(void);
extern const unsigned char user_effect_parameters_hfp_hfp[];
extern const unsigned char user_module_parameters_hfp_hfp[];

static const ROBOEFFECT_EFFECT_PARA effect_para[] =
{
	{
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_hfp,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_hfp,
		.user_effects_script = (uint8_t *)user_effects_script_hfp,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_hfp_hfp,
		.user_module_parameters = (uint8_t *)user_module_parameters_hfp_hfp,
		.get_user_effects_script_len = get_user_effects_script_len_hfp,
	}
};

const ROBOEFFECT_EFFECT_PARA_TABLE hfp_node =
{
	//ROBOEFFECT effect ID ͨ�����ID������ƥ��
	.effect_id    = EFFECT_MODE_HFP_AEC ,
	//�ÿ�ͼ������1����Ч
	.effect_id_count = 1,

	//ROBOEFFECT effect ��Ч��ַӳ��
	.effect_addr =
	{
		.APP_SOURCE_GAIN_ADDR = HFP_music_gain_ADDR,
		.MIC_SOURCE_GAIN_ADDR = HFP_mic_gain_ADDR,
	},

	//ROBOEFFECT effect SOURCEӳ��
	.roboeffect_source =
	{
		.mic_source = HFP_SOURCE_MIC_SOURCE,
		.app_source = HFP_SOURCE_APP_SOURCE,
		.remind_source = HFP_SOURCE_REMIND_SOURCE,
		.rec_source = HFP_SOURCE_REC_SOURCE,
		.usb_source = AUDIOCORE_SOURCE_SINK_ERROR,
		.i2s_mix_source = AUDIOCORE_SOURCE_SINK_ERROR,
		.linein_mix_source = AUDIOCORE_SOURCE_SINK_ERROR,
	},

	//ROBOEFFECT effect SINKӳ��
	.roboeffect_sink =
	{
		.dac0_sink = HFP_SINK_DAC0_SINK,
		.app_sink = HFP_SINK_APP_SINK,
		.stereo_sink = HFP_SINK_STEREO_SINK,
		.rec_sink = HFP_SINK_REC_SINK,
		.i2s_mix_sink = AUDIOCORE_SOURCE_SINK_ERROR,
	},

	//ROBOEFFECT effect ����
	.roboeffect_para = &effect_para[0],
};
