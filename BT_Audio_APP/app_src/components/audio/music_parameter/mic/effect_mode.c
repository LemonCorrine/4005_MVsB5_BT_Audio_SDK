#include "user_effect_flow_mic.h"
#include "user_effect_parameter.h"

static const AUDIOEFFECT_EFFECT_PARA effect_para[] =
{
	{
		.user_effect_name = (uint8_t *)"Mic",
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_mic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_mic,
		.user_effects_script = (uint8_t *)user_effects_script_mic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_mic_Mic,
		.user_module_parameters = (uint8_t *)user_module_parameters_mic_Mic,
		.get_user_effects_script_len = get_user_effects_script_len_mic,
	}
};

const AUDIOEFFECT_EFFECT_PARA_TABLE mic_mode =
{
	//ROBOEFFECT effect ID ͨ�����ID������ƥ��
	.effect_id    = EFFECT_MODE_MIC ,
	//�ÿ�ͼ������1����Ч
	.effect_id_count = 1,

	//ROBOEFFECT effect ��Ч��ַӳ��
	.effect_addr =
	{
		.APP_SOURCE_GAIN_ADDR = MIC_gain_control0_ADDR,
		.MIC_SOURCE_GAIN_ADDR = MIC_mic_gain_ADDR,
		.REMIND_SOURCE_GAIN_ADDR = MIC_remind_gain_control_ADDR,
		.SILENCE_DETECTOR_ADDR = MIC_silence_detector_mic_ADDR,
		.SILENCE_DETECTOR_MUSIC_ADDR = MIC_silence_detector_music_ADDR,
	},

	//ROBOEFFECT effect SOURCEӳ��
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

	//ROBOEFFECT effect SINKӳ��
	.audioeffect_sink =
	{
		.dac0_sink = MIC_SINK_DAC0_SINK,
		.app_sink = MIC_SINK_APP_SINK,
		.stereo_sink = MIC_SINK_STEREO_SINK,
		.rec_sink = MIC_SINK_REC_SINK,
		.i2s_mix_sink = AUDIOCORE_SOURCE_SINK_ERROR,
		.spdif_sink = MIC_SINK_SPDIF_SINK,
	},

	//ROBOEFFECT effect ����
	.audioeffect_para = (AUDIOEFFECT_EFFECT_PARA *)&effect_para[0],
};
