#include "user_effect_flow_karaoke.h"
#include "user_effect_parameter.h"

extern uint32_t get_user_effects_script_len_Karaoke(void);
extern const unsigned char user_effect_parameters_Karaoke_HunXiang[];
extern const unsigned char user_module_parameters_Karaoke_HunXiang[];
extern const unsigned char user_effect_parameters_Karaoke_DianYin[];
extern const unsigned char user_module_parameters_Karaoke_DianYin[];
extern const unsigned char user_effect_parameters_Karaoke_MoYin[];
extern const unsigned char user_module_parameters_Karaoke_MoYin[];
extern const unsigned char user_effect_parameters_Karaoke_HanMai[];
extern const unsigned char user_module_parameters_Karaoke_HanMai[];
extern const unsigned char user_effect_parameters_Karaoke_NanBianNv[];
extern const unsigned char user_module_parameters_Karaoke_NanBianNv[];
extern const unsigned char user_effect_parameters_Karaoke_NvBianNan[];
extern const unsigned char user_module_parameters_Karaoke_NvBianNan[];
extern const unsigned char user_effect_parameters_Karaoke_WaWaYin[];
extern const unsigned char user_module_parameters_Karaoke_WaWaYin[];

static const ROBOEFFECT_EFFECT_PARA effect_para[] =
{
	 //HUNXIANG
	{
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_Karaoke,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,
		.user_effects_script = (uint8_t *)user_effects_script_Karaoke,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_Karaoke_HunXiang,
		.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_HunXiang,
		.get_user_effects_script_len = get_user_effects_script_len_Karaoke
	},
	//DIANYIN
	{
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_Karaoke,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,
		.user_effects_script = (uint8_t *)user_effects_script_Karaoke,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_Karaoke_DianYin,
		.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_DianYin,
		.get_user_effects_script_len = get_user_effects_script_len_Karaoke
	},
	//MOYIN
	{
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_Karaoke,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,
		.user_effects_script = (uint8_t *)user_effects_script_Karaoke,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_Karaoke_MoYin,
		.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_MoYin,
		.get_user_effects_script_len = get_user_effects_script_len_Karaoke
	},
	//HANMAI
	{
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_Karaoke,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,
		.user_effects_script = (uint8_t *)user_effects_script_Karaoke,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_Karaoke_HanMai,
		.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_HanMai,
		.get_user_effects_script_len = get_user_effects_script_len_Karaoke
	},
	//NANBIANNV
	{
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_Karaoke,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,
		.user_effects_script = (uint8_t *)user_effects_script_Karaoke,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_Karaoke_NanBianNv,
		.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_NanBianNv,
		.get_user_effects_script_len = get_user_effects_script_len_Karaoke
	},
	//NVBIANNAN
	{
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_Karaoke,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,
		.user_effects_script = (uint8_t *)user_effects_script_Karaoke,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_Karaoke_NvBianNan,
		.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_NvBianNan,
		.get_user_effects_script_len = get_user_effects_script_len_Karaoke
	},
	//WAWAYIN
	{
		.user_effect_list = (roboeffect_effect_list_info *)&user_effect_list_Karaoke,
		.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,
		.user_effects_script = (uint8_t *)user_effects_script_Karaoke,
		.user_effect_parameters = (uint8_t *)user_effect_parameters_Karaoke_WaWaYin,
		.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_WaWaYin,
		.get_user_effects_script_len = get_user_effects_script_len_Karaoke
	}
};

const ROBOEFFECT_EFFECT_PARA_TABLE karaoke_node =
{
	//ROBOEFFECT effect ID 通过这个ID来搜索匹配
	.effect_id    = EFFECT_MODE_HunXiang ,
	//该框图下面有7个音效
	.effect_id_count = EFFECT_MODE_WaWaYin - EFFECT_MODE_HunXiang + 1,

	//ROBOEFFECT effect 音效地址映射
	.effect_addr =
	{
		.REVERB_ADDR = KARAOKE_reverb0_ADDR,
		.ECHO_ADDR = KARAOKE_echo0_ADDR,
		.SILENCE_DETECTOR_ADDR = KARAOKE_silence_detector0_ADDR,
		.VOICE_CHANGER_ADDR = KARAOKE_voice_changer0_ADDR,
		.APP_SOURCE_GAIN_ADDR = KARAOKE_gain_control0_ADDR,
		.MIC_SOURCE_GAIN_ADDR = KARAOKE_gain_control1_ADDR,
		.DAC0_SINK_GAIN_ADDR = KARAOKE_gain_control0_ADDR,	//框图里面没有，暂时复用app source
		.APP_SINK_GAIN_ADDR = KARAOKE_gain_control10_ADDR,
	},

	//ROBOEFFECT effect SOURCE映射
	.roboeffect_source =
	{
		.mic_source = KARAOKE_SOURCE_MIC_SOURCE,
		.app_source = KARAOKE_SOURCE_APP_SOURCE,
		.remind_source = KARAOKE_SOURCE_REMIND_SOURCE,
		.rec_source = KARAOKE_SOURCE_REC_SOURCE,
		.usb_source = KARAOKE_SOURCE_USB_SOURCE,
		.i2s_mix_source = KARAOKE_SOURCE_I2S_MIX_SOURCE,
		.linein_mix_source = KARAOKE_SOURCE_LINEIN_MIX_SOURCE,
	},

	//ROBOEFFECT effect SINK映射
	.roboeffect_sink =
	{
		.dac0_sink = KARAOKE_SINK_DAC0_SINK,
		.app_sink = KARAOKE_SINK_APP_SINK,
		.stereo_sink = KARAOKE_SINK_STEREO_SINK,
		.rec_sink = KARAOKE_SINK_REC_SINK,
		.i2s_mix_sink = KARAOKE_SINK_I2S_MIX_SINK,
	},

	//ROBOEFFECT effect 参数
	.roboeffect_para = &effect_para[0],
};
