#include "user_effect_flow_music.h"
#include "user_effect_parameter.h"

const AUDIOEFFECT_EFFECT_PARA music_effect_para =
{
	.user_effect_name = (uint8_t *)"Music",
	.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_music,
	.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_music,
	.user_effects_script = (uint8_t *)user_effects_script_music,
	.user_effect_parameters = (uint8_t *)user_effect_parameters_music_Music,
	.user_module_parameters = (uint8_t *)user_module_parameters_music_Music,
	.get_user_effects_script_len = get_user_effects_script_len_music,
};

const AUDIOEFFECT_SOURCE_SINK_NUM music_mode =
{
	//²»ÒªÉ¾³ý£¬source/sinkÄ¬ÈÏÖµ
	AUDIOEFFECT_SOURCE_SINK_DEFAULT_INIT,

	//ROBOEFFECT effect SOURCEÓ³Éä
	.mic_source = MUSIC_SOURCE_MIC_SOURCE,
	.app_source = MUSIC_SOURCE_APP_SOURCE,
	.remind_source = MUSIC_SOURCE_REMIND_SOURCE,
	.rec_source = MUSIC_SOURCE_REC_SOURCE,

	//ROBOEFFECT effect SINKÓ³Éä
	.dac0_sink = MUSIC_SINK_DAC0_SINK,
	.app_sink = MUSIC_SINK_APP_SINK,
	.stereo_sink = MUSIC_SINK_STEREO_SINK,
	.rec_sink = MUSIC_SINK_REC_SINK,
	.spdif_sink = MUSIC_SINK_SPDIF_SINK,
};

const uint8_t music_effect_ctrl[AUDIOEFFECT_EFFECT_CONTROL_MAX] =
{
	[EQ_MODE_ADJUST] = MUSIC_eq0_ADDR,
	[MUSIC_VOLUME_ADJUST] = MUSIC_gain_control0_ADDR,
	[MIC_VOLUME_ADJUST] = MUSIC_mic_gain_ADDR,
	[REMIND_VOLUME_ADJUST] = MUSIC_gain_control3_ADDR,
	[MIC_SILENCE_DETECTOR_PARAM] = MUSIC_silence_detector_mic_ADDR,
	[MUSIC_SILENCE_DETECTOR_PARAM] = MUSIC_silence_detector_music_ADDR,
	[APPMODE_PREGAIN] = MUSIC_preGain_ADDR,
};

