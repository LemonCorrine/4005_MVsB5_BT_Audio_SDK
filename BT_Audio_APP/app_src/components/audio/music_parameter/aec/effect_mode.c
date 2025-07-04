#include "user_effect_flow_aecMusic.h"
#include "user_effect_flow_aecMic.h"
#include "user_effect_parameter.h"

const AUDIOEFFECT_EFFECT_PARA aec_effect_para[MaxNet] =
{
	{
		.user_effect_name = (uint8_t *)"AEC_Music",
		.chart_name = (uint8_t *)chart_name_aecMusic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMusic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMusic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMusic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMusic_AEC_Music,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMusic_AEC_Music,
		.get_user_effects_script_len = get_user_effects_script_len_aecMusic,
	},
	{
		.user_effect_name = (uint8_t *)"AEC_Mic",
		.chart_name = (uint8_t *)chart_name_aecMic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMic_AEC_Music,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMic_AEC_Music,
		.get_user_effects_script_len = get_user_effects_script_len_aecMic,
	}
};

const AUDIOEFFECT_EFFECT_PARA aec_A_effect_para[MaxNet] =
{
	{
		.user_effect_name = (uint8_t *)"AEC_Music_A",
		.chart_name = (uint8_t *)chart_name_aecMusic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMusic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMusic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMusic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMusic_AEC_Music_A,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMusic_AEC_Music_A,
		.get_user_effects_script_len = get_user_effects_script_len_aecMusic,
	},
	{
		.user_effect_name = (uint8_t *)"AEC_Mic_A",
		.chart_name = (uint8_t *)chart_name_aecMic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMic_AEC_Music_A,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMic_AEC_Music_A,
		.get_user_effects_script_len = get_user_effects_script_len_aecMic,
	}
};

const AUDIOEFFECT_EFFECT_PARA aec_B_effect_para[MaxNet] =
{
	{
		.user_effect_name = (uint8_t *)"AEC_Music_B",
		.chart_name = (uint8_t *)chart_name_aecMusic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMusic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMusic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMusic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMusic_AEC_Music_B,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMusic_AEC_Music_B,
		.get_user_effects_script_len = get_user_effects_script_len_aecMusic,
	},
	{
		.user_effect_name = (uint8_t *)"AEC_Mic_B",
		.chart_name = (uint8_t *)chart_name_aecMic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMic_AEC_Music_B,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMic_AEC_Music_B,
		.get_user_effects_script_len = get_user_effects_script_len_aecMic,
	}
};

const AUDIOEFFECT_EFFECT_PARA aec_C_effect_para[MaxNet] =
{
	{
		.user_effect_name = (uint8_t *)"AEC_Music_C",
		.chart_name = (uint8_t *)chart_name_aecMusic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMusic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMusic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMusic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMusic_AEC_Music_C,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMusic_AEC_Music_C,
		.get_user_effects_script_len = get_user_effects_script_len_aecMusic,
	},
	{
		.user_effect_name = (uint8_t *)"AEC_Mic_C",
		.chart_name = (uint8_t *)chart_name_aecMic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMic_AEC_Music_C,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMic_AEC_Music_C,
		.get_user_effects_script_len = get_user_effects_script_len_aecMic,
	}
};

const AUDIOEFFECT_EFFECT_PARA aec_D_effect_para[MaxNet] =
{
	{
		.user_effect_name = (uint8_t *)"AEC_Music_D",
		.chart_name = (uint8_t *)chart_name_aecMusic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMusic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMusic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMusic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMusic_AEC_Music_D,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMusic_AEC_Music_D,
		.get_user_effects_script_len = get_user_effects_script_len_aecMusic,
	},
	{
		.user_effect_name = (uint8_t *)"AEC_Mic_D",
		.chart_name = (uint8_t *)chart_name_aecMic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMic_AEC_Music_D,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMic_AEC_Music_D,
		.get_user_effects_script_len = get_user_effects_script_len_aecMic,
	}
};

const AUDIOEFFECT_EFFECT_PARA aec_E_effect_para[MaxNet] =
{
	{
		.user_effect_name = (uint8_t *)"AEC_Music_E",
		.chart_name = (uint8_t *)chart_name_aecMusic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMusic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMusic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMusic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMusic_AEC_Music_E,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMusic_AEC_Music_E,
		.get_user_effects_script_len = get_user_effects_script_len_aecMusic,
	},
	{
		.user_effect_name = (uint8_t *)"AEC_Mic_E",
		.chart_name = (uint8_t *)chart_name_aecMic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMic_AEC_Music_E,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMic_AEC_Music_E,
		.get_user_effects_script_len = get_user_effects_script_len_aecMic,
	}
};

const AUDIOEFFECT_EFFECT_PARA aec_F_effect_para[MaxNet] =
{
	{
		.user_effect_name = (uint8_t *)"AEC_Music_F",
		.chart_name = (uint8_t *)chart_name_aecMusic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMusic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMusic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMusic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMusic_AEC_Music_F,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMusic_AEC_Music_F,
		.get_user_effects_script_len = get_user_effects_script_len_aecMusic,
	},
	{
		.user_effect_name = (uint8_t *)"AEC_Mic_F",
		.chart_name = (uint8_t *)chart_name_aecMic,
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_aecMic,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_aecMic,
		.user_effects_script = (uint8_t *)user_effects_script_aecMic,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_aecMic_AEC_Music_F,
		.user_module_parameters = (uint8_t *)user_module_parameters_aecMic_AEC_Music_F,
		.get_user_effects_script_len = get_user_effects_script_len_aecMic,
	}
};

const AUDIOEFFECT_SOURCE_SINK_NUM aec_mode =
{
	//²»ÒªÉ¾³ý£¬source/sinkÄ¬ÈÏÖµ
	AUDIOEFFECT_SOURCE_SINK_DEFAULT_INIT,

	//ROBOEFFECT effect SOURCEÓ³Éä
	.mic_source = AUDIOEFFECT_SET_SOURCESINKNUM(AECNet, AECMIC_SOURCE_MIC_SOURCE),
	.app_source = AUDIOEFFECT_SET_SOURCESINKNUM(DefaultNet, AECMUSIC_SOURCE_APP_SOURCE),
	.remind_source = AUDIOEFFECT_SET_SOURCESINKNUM(DefaultNet, AECMUSIC_SOURCE_REMIND_SOURCE),
	.aec_ref_source = AUDIOEFFECT_SET_SOURCESINKNUM(AECNet, AECMIC_SOURCE_AEC_REF_SOURCE),
	.i2s_mix_source = AUDIOEFFECT_SET_SOURCESINKNUM(DefaultNet, AECMUSIC_SOURCE_I2S_MIX_SOURCE),

	//ROBOEFFECT effect SINKÓ³Éä
	.dac0_sink = AUDIOEFFECT_SET_SOURCESINKNUM(DefaultNet, AECMUSIC_SINK_DAC0_SINK),
	.stereo_sink = AUDIOEFFECT_SET_SOURCESINKNUM(DefaultNet, AECMUSIC_SINK_STEREO_SINK),
	.i2s_mix_sink = AUDIOEFFECT_SET_SOURCESINKNUM(DefaultNet, AECMUSIC_SINK_I2S_MIX_SINK),
	.app_sink = AUDIOEFFECT_SET_SOURCESINKNUM(AECNet, AECMIC_SINK_APP_SINK),
	.aec_ref_sink = AUDIOEFFECT_SET_SOURCESINKNUM(DefaultNet, AECMUSIC_SINK_AEC_REF_SINK),
};

const uint16_t aec_effect_ctrl[AUDIOEFFECT_EFFECT_CONTROL_MAX] =
{
	[EQ_MODE_ADJUST] = AUDIOEFFECT_SET_EFFECTADDR(DefaultNet, AECMUSIC_music_EQ_ADDR),
	[MUSIC_VOLUME_ADJUST] = AUDIOEFFECT_SET_EFFECTADDR(DefaultNet, AECMUSIC_gain_control0_ADDR),
	[MIC_VOLUME_ADJUST] = AUDIOEFFECT_SET_EFFECTADDR(AECNet, AECMIC_gain_control0_ADDR),
	[REMIND_VOLUME_ADJUST] = AUDIOEFFECT_SET_EFFECTADDR(DefaultNet, AECMUSIC_remind_gain_control_ADDR),
	[APPMODE_PREGAIN] = AUDIOEFFECT_SET_EFFECTADDR(DefaultNet, AECMUSIC_preGain_ADDR),
};
