#include "user_effect_flow_MICUSBAI.h"
#include "user_effect_parameter.h"

extern uint32_t get_user_effects_script_len_MICUSBAI(void);
extern const unsigned char user_effect_parameters_MICUSBAI_MICUSBAI[];
extern const unsigned char user_module_parameters_MICUSBAI_MICUSBAI[];

static const AUDIOEFFECT_EFFECT_PARA effect_para[] =
{
	{
		.user_effect_name = (uint8_t *)"MICUSBAI",
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_MICUSBAI,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_MICUSBAI,
		.user_effects_script = (uint8_t *)user_effects_script_MICUSBAI,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_MICUSBAI_MICUSBAI,
		.user_module_parameters = (uint8_t *)user_module_parameters_MICUSBAI_MICUSBAI,
		.get_user_effects_script_len = get_user_effects_script_len_MICUSBAI,
	}
};

const AUDIOEFFECT_EFFECT_PARA_TABLE micusbAI_mode =
{
	//ROBOEFFECT effect ID ͨ�����ID������ƥ��
	.effect_id    = EFFECT_MODE_MICUSBAI ,
	//�ÿ�ͼ������1����Ч
	.effect_id_count = 1,

	//ROBOEFFECT effect ��Ч��ַӳ��

	//��һ�����ǰ���Ч��ͼ�е�һЩ��Ч��ӳ�����������ȥ����Ч�ĵ�ַ��������Ч�ˣ�ֱ����ӳ�����Ч����������Ч��������⣩

	.effect_addr =
	{
		//.APP_SOURCE_GAIN_ADDR = MIC_gain_control0_ADDR,
		//.MIC_SOURCE_GAIN_ADDR = MIC_mic_gain_ADDR,
		//.REMIND_SOURCE_GAIN_ADDR = MIC_remind_gain_control_ADDR,
//		.SILENCE_DETECTOR_ADDR = MIC_silence_detector_mic_ADDR,
//		.SILENCE_DETECTOR_MUSIC_ADDR = MIC_silence_detector_music_ADDR,
	},

	//ROBOEFFECT effect SOURCEӳ��
	.audioeffect_source =
	{
		.mic_source = MICUSBAI_SOURCE_MIC_SOURCE,
		.app_source = MICUSBAI_SOURCE_APP_SOURCE,
		.remind_source = AUDIOCORE_SOURCE_SINK_ERROR,
		.rec_source = AUDIOCORE_SOURCE_SINK_ERROR,
		.usb_source = AUDIOCORE_SOURCE_SINK_ERROR,
		.i2s_mix_source = AUDIOCORE_SOURCE_SINK_ERROR,
		.linein_mix_source = AUDIOCORE_SOURCE_SINK_ERROR,
	},

	//ROBOEFFECT effect SINKӳ��
	.audioeffect_sink =
	{
		.dac0_sink = MICUSBAI_SINK_DAC0_SINK,
		.app_sink = MICUSBAI_SINK_APP_SINK,
		.stereo_sink = AUDIOCORE_SOURCE_SINK_ERROR,
		.rec_sink = AUDIOCORE_SOURCE_SINK_ERROR,
		.i2s_mix_sink = AUDIOCORE_SOURCE_SINK_ERROR,
		.spdif_sink = AUDIOCORE_SOURCE_SINK_ERROR,
	},

	//ROBOEFFECT effect ����
	.audioeffect_para = (AUDIOEFFECT_EFFECT_PARA *)&effect_para[0],
};
