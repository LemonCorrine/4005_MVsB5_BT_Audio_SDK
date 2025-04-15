#include <nds32_utils_math.h>
#include <string.h>
#include <math.h>
#include "roboeffect_api.h"
#include "user_defined_effect_api.h"
#include "user_effect_parameter.h"
#include "nn_denoise_api.h"
#include "main_task.h"

extern uint32_t get_user_effects_script_len_mic(void);
extern uint32_t get_user_effects_script_len_music(void);
extern uint32_t get_user_effects_script_len_hfp(void);
extern uint32_t get_user_effects_script_len_Karaoke(void);

extern const unsigned char user_effect_parameters_hfp_hfp[];
extern const unsigned char user_module_parameters_hfp_hfp[];
extern const unsigned char user_effect_parameters_mic_mic[];
extern const unsigned char user_module_parameters_mic_mic[];
extern const unsigned char user_effect_parameters_music_music[];
extern const unsigned char user_module_parameters_music_music[];
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

static const ROBOEFFECT_EFFECT_ADDR effect_addr[] = {
	//mic
	{
		.APP_SOURCE_GAIN_ADDR = MIC_gain_control0_ADDR,
		.MIC_SOURCE_GAIN_ADDR = MIC_mic_gain_ADDR,
	},

	//music
	{
		.APP_SOURCE_GAIN_ADDR = MUSIC_gain_control0_ADDR,
		.MIC_SOURCE_GAIN_ADDR = MUSIC_mic_gain_ADDR,
	},

	//hfp
	{
		.APP_SOURCE_GAIN_ADDR = HFP_music_gain_ADDR,
		.MIC_SOURCE_GAIN_ADDR = HFP_mic_gain_ADDR,
	},

	//HUNXIANG
	{
		.SILENCE_DETECTOR_ADDR = KARAOKE_silence_detector0_ADDR,
		.APP_SOURCE_GAIN_ADDR = KARAOKE_gain_control0_ADDR,
		.MIC_SOURCE_GAIN_ADDR = KARAOKE_gain_control1_ADDR,
		.DAC0_SINK_GAIN_ADDR = KARAOKE_gain_control0_ADDR,	//框图里面没有，暂时复用app source
		.APP_SINK_GAIN_ADDR = KARAOKE_gain_control10_ADDR,
	},

	//DIANYIN
	{
		.SILENCE_DETECTOR_ADDR = KARAOKE_silence_detector0_ADDR,
		.APP_SOURCE_GAIN_ADDR = KARAOKE_gain_control0_ADDR,
		.MIC_SOURCE_GAIN_ADDR = KARAOKE_gain_control1_ADDR,
		.APP_SINK_GAIN_ADDR = KARAOKE_gain_control10_ADDR,
	},

	//MOYIN
	{
		.SILENCE_DETECTOR_ADDR = KARAOKE_silence_detector0_ADDR,
		.APP_SOURCE_GAIN_ADDR = KARAOKE_gain_control0_ADDR,
		.MIC_SOURCE_GAIN_ADDR = KARAOKE_gain_control1_ADDR,
		.APP_SINK_GAIN_ADDR = KARAOKE_gain_control10_ADDR,
	},

	//HANMAI
	{
		.SILENCE_DETECTOR_ADDR = KARAOKE_silence_detector0_ADDR,
		.APP_SOURCE_GAIN_ADDR = KARAOKE_gain_control0_ADDR,
		.MIC_SOURCE_GAIN_ADDR = KARAOKE_gain_control1_ADDR,
		.APP_SINK_GAIN_ADDR = KARAOKE_gain_control10_ADDR,
	},

	//NANBIANNV
	{
		.SILENCE_DETECTOR_ADDR = KARAOKE_silence_detector0_ADDR,
		.APP_SOURCE_GAIN_ADDR = KARAOKE_gain_control0_ADDR,
		.MIC_SOURCE_GAIN_ADDR = KARAOKE_gain_control1_ADDR,
		.APP_SINK_GAIN_ADDR = KARAOKE_gain_control10_ADDR,
	},

	//NVBIANNAN
	{
		.SILENCE_DETECTOR_ADDR = KARAOKE_silence_detector0_ADDR,
		.APP_SOURCE_GAIN_ADDR = KARAOKE_gain_control0_ADDR,
		.MIC_SOURCE_GAIN_ADDR = KARAOKE_gain_control1_ADDR,
		.APP_SINK_GAIN_ADDR = KARAOKE_gain_control10_ADDR,
	},

	//WAWAYIN
	{
		.SILENCE_DETECTOR_ADDR = KARAOKE_silence_detector0_ADDR,
		.APP_SOURCE_GAIN_ADDR = KARAOKE_gain_control0_ADDR,
		.MIC_SOURCE_GAIN_ADDR = KARAOKE_gain_control1_ADDR,
		.APP_SINK_GAIN_ADDR = KARAOKE_gain_control10_ADDR,
	},
};

static const ROBOEFFECT_SOURCE_NUM roboeffect_source[] = {
  //{mic_source,			 	app_source,			 		remind_source},
	{MIC_SOURCE_MIC_SOURCE, 	MIC_SOURCE_APP_SOURCE, 		MIC_SOURCE_REMIND_SOURCE,	MIC_SOURCE_REC_SOURCE},		//mic
	{MUSIC_SOURCE_MIC_SOURCE,	MUSIC_SOURCE_APP_SOURCE, 	MUSIC_SOURCE_REMIND_SOURCE,	MUSIC_SOURCE_REC_SOURCE},	//music
	{HFP_SOURCE_MIC_SOURCE,		HFP_SOURCE_APP_SOURCE, 		HFP_SOURCE_REMIND_SOURCE},		//HFP
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE,	KARAOKE_SOURCE_REC_SOURCE},	//HUNXIANG
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE,	KARAOKE_SOURCE_REC_SOURCE},	//DIANYIN
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE,	KARAOKE_SOURCE_REC_SOURCE},	//MOYIN
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE,	KARAOKE_SOURCE_REC_SOURCE},	//HANMAI
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE,	KARAOKE_SOURCE_REC_SOURCE},	//NANBIANNV
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE,	KARAOKE_SOURCE_REC_SOURCE},	//NVBIANNAN
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE,	KARAOKE_SOURCE_REC_SOURCE},	//WAWAYIN
};

static const ROBOEFFECT_SINK_NUM roboeffect_sink[] = {
  //{dac0_sink,			 	 app_sink,				stereo_sink},
	{MIC_SINK_DAC0_SINK,	 MIC_SINK_APP_SINK,     MIC_SINK_STEREO_SINK,	MIC_SINK_REC_SINK},		//mic
	{MUSIC_SINK_DAC0_SINK,	 MUSIC_SINK_APP_SINK,	MUSIC_SINK_STEREO_SINK,	MUSIC_SINK_REC_SINK},	//music
	{HFP_SINK_DAC0_SINK,	 HFP_SINK_APP_SINK,		HFP_SINK_STEREO_SINK},		//HFP
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK,	KARAOKE_SINK_REC_SINK},	//HUNXIANG
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK,	KARAOKE_SINK_REC_SINK},	//DIANYIN
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK,	KARAOKE_SINK_REC_SINK},	//MOYIN
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK,	KARAOKE_SINK_REC_SINK},	//HANMAI
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK,	KARAOKE_SINK_REC_SINK},	//NANBIANNV
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK,	KARAOKE_SINK_REC_SINK},	//NVBIANNAN
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK,	KARAOKE_SINK_REC_SINK},	//WAWAYIN
};

static const ROBOEFFECT_EFFECT_PARA roboeffect_para[] = {
	//mic
	{(roboeffect_effect_list_info *)&user_effect_list_mic,	(roboeffect_effect_steps_table *)&user_effect_steps_mic,		(uint8_t *)user_effects_script_mic,
	 (uint8_t *)user_effect_parameters_mic_mic,				(uint8_t *)user_module_parameters_mic_mic,						get_user_effects_script_len_mic},
	 //music
	{(roboeffect_effect_list_info *)&user_effect_list_music,(roboeffect_effect_steps_table *)&user_effect_steps_music,		(uint8_t *)user_effects_script_music,
	 (uint8_t *)user_effect_parameters_music_music,			(uint8_t *)user_module_parameters_music_music,					get_user_effects_script_len_music},
	 //HFP
	{(roboeffect_effect_list_info *)&user_effect_list_hfp,	(roboeffect_effect_steps_table *)&user_effect_steps_hfp,		(uint8_t *)user_effects_script_hfp,
	 (uint8_t *)user_effect_parameters_hfp_hfp,				(uint8_t *)user_module_parameters_hfp_hfp,						get_user_effects_script_len_hfp},
	 //HUNXIANG
	{(roboeffect_effect_list_info *)&user_effect_list_Karaoke,(roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,	(uint8_t *)user_effects_script_Karaoke,
	 (uint8_t *)user_effect_parameters_Karaoke_HunXiang,	(uint8_t *)user_module_parameters_Karaoke_HunXiang,				get_user_effects_script_len_Karaoke},
	//DIANYIN
	{(roboeffect_effect_list_info *)&user_effect_list_Karaoke,(roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,	(uint8_t *)user_effects_script_Karaoke,
	 (uint8_t *)user_effect_parameters_Karaoke_DianYin,		(uint8_t *)user_module_parameters_Karaoke_DianYin,				get_user_effects_script_len_Karaoke},
	//MOYIN
	{(roboeffect_effect_list_info *)&user_effect_list_Karaoke,(roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,	(uint8_t *)user_effects_script_Karaoke,
	 (uint8_t *)user_effect_parameters_Karaoke_MoYin,		(uint8_t *)user_module_parameters_Karaoke_MoYin,				get_user_effects_script_len_Karaoke},
	//HANMAI
	{(roboeffect_effect_list_info *)&user_effect_list_Karaoke,(roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,	(uint8_t *)user_effects_script_Karaoke,
	 (uint8_t *)user_effect_parameters_Karaoke_HanMai,		(uint8_t *)user_module_parameters_Karaoke_HanMai,				get_user_effects_script_len_Karaoke},
	//NANBIANNV
	{(roboeffect_effect_list_info *)&user_effect_list_Karaoke,(roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,	(uint8_t *)user_effects_script_Karaoke,
	 (uint8_t *)user_effect_parameters_Karaoke_NanBianNv,	(uint8_t *)user_module_parameters_Karaoke_NanBianNv,			get_user_effects_script_len_Karaoke},
	//NVBIANNAN
	{(roboeffect_effect_list_info *)&user_effect_list_Karaoke,(roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,	(uint8_t *)user_effects_script_Karaoke,
	 (uint8_t *)user_effect_parameters_Karaoke_NvBianNan,	(uint8_t *)user_module_parameters_Karaoke_NvBianNan,			get_user_effects_script_len_Karaoke},
	//WAWAYIN
	{(roboeffect_effect_list_info *)&user_effect_list_Karaoke,(roboeffect_effect_steps_table *)&user_effect_steps_Karaoke,	(uint8_t *)user_effects_script_Karaoke,
	 (uint8_t *)user_effect_parameters_Karaoke_WaWaYin,	(uint8_t *)user_module_parameters_Karaoke_WaWaYin,			get_user_effects_script_len_Karaoke},
};

static const uint16_t roboeffectVolArr[CFG_PARA_MAX_VOLUME_NUM + 1] =
{
#if CFG_PARA_MAX_VOLUME_NUM == 32
	0xe3e0/*-72db*/,
	0xe75a/*-63db*/,	0xea20/*-56db*/,	0xecdc/*-49db*/,	0xeed0/*-44db*/,	0xf060/*-40db*/,	0xf1f0/*-36db*/,	0xf380/*-32db*/,	0xf4ac/*-29db*/,
	0xf5d8/*-26db*/,	0xf6a0/*-24db*/,	0xf768/*-22db*/,	0xf830/*-20db*/,	0xf894/*-19db*/,	0xf8f8/*-18db*/,	0xf95c/*-17db*/,	0xf9c0/*-16db*/,
	0xfa24/*-15db*/,	0xfa88/*-14db*/,	0xfaec/*-13db*/,	0xfb50/*-12db*/,	0xfbb4/*-11db*/,	0xfc18/*-10db*/,	0xfc7c/*-9db*/,		0xfce0/*-8db*/,
	0xfd44/*-7db*/,		0xfda8/*-6db*/,		0xfe0c/*-5db*/,		0xfe70/*-4db*/,		0xfed4/*-3db*/,		0xff38/*-2db*/,		0xff9c/*-1db*/,		0x0/*0db*/
#endif
#if CFG_PARA_MAX_VOLUME_NUM == 16
	0xe3e0/*-72db*/,
	0xea20/*-56db*/,	0xf060/*-40db*/,	0xf4ac/*-29db*/,	0xf6a0/*-24db*/,	0xf830/*-20db*/,	0xf95c/*-17db*/,	0xfa24/*-15db*/,	0xfaec/*-13db*/,
	0xfbb4/*-11db*/,	0xfc7c/*-9db*/,		0xfd44/*-7db*/,		0xfe0c/*-5db*/,		0xfed4/*-3db*/,		0xff38/*-2db*/,		0xff9c/*-1db*/,		0x0/*0db*/
#endif
};

static ReverbMaxUnit ReverbMaxParam = {0 , 0 , 0 ,0};
static roboeffect_effect_list_info local_effect_list;

void Roboeffect_GetAudioEffectMaxValue(void)
{
	if(AudioCore.Roboeffect.context_memory == NULL)
		return;
	ReverbUnit *Reverbparam;
	EchoUnit *Echoparam;
	Reverbparam = (ReverbUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory,
										effect_addr[AudioCore.Roboeffect.flow_chart_mode].REVERB_ADDR, 0xff);
	Echoparam = (EchoUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory,
										effect_addr[AudioCore.Roboeffect.flow_chart_mode].ECHO_ADDR, 0xff);

	ReverbMaxParam.max_reverb_wet_scale  = Reverbparam->wet_scale;

	ReverbMaxParam.max_reverb_roomsize   = Reverbparam->roomsize_scale;

	ReverbMaxParam.max_echo_delay        = Echoparam->delay;

	ReverbMaxParam.max_echo_depth        = Echoparam->attenuation;
}

#if defined(CFG_FUNC_MIC_TREB_BASS_EN) || defined(CFG_FUNC_MUSIC_TREB_BASS_EN)
void Roboeffect_EQ_Ajust(ROBOEFFECT_EFFECT_TYPE type,uint8_t BassGain, uint8_t TrebGain)
{
	uint8_t node;

	if(type == MIC_EQ)
		node = effect_addr[AudioCore.Roboeffect.flow_chart_mode].MIC_EQ_ADDR;
	else if(type == MUSIC_EQ)
		node = effect_addr[AudioCore.Roboeffect.flow_chart_mode].MUSIC_EQ_ADDR;

	if(AudioCore.Roboeffect.context_memory == NULL)
		return;
	if(node == 0)
		return;

	printf("node: %02X\n", node);
	int16_t param = 0;

//	EQUnit *EQparam;
//	EQparam = (EQUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, node, 0xff);
//	printf("Before,EQ BassGain:%d ,TrebGain:%d\n", EQparam->eq_params[0].gain, EQparam->eq_params[1].gain);
//	printf("Need,EQ BassGain:%d ,TrebGain:%d\n", BassTrebGainTable[BassGain], BassTrebGainTable[TrebGain]);

	param = BassTrebGainTable[BassGain];
	roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, node, 6, &param);
	roboeffect_update_local_params(node, 6, &param, 3);

	param = BassTrebGainTable[TrebGain];
	roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, node, 11, &param);
	roboeffect_update_local_params(node, 11, &param, 3);

	gCtrlVars.AutoRefresh = node;//AudioCore.Roboeffect.effect_addr;
}
#endif

void Roboeffect_ReverbStep_Ajust(uint8_t ReverbStep)
{
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	uint8_t Reverbnode = effect_addr[AudioCore.Roboeffect.flow_chart_mode].REVERB_ADDR;
	uint8_t Echonode = effect_addr[AudioCore.Roboeffect.flow_chart_mode].ECHO_ADDR;

	if(AudioCore.Roboeffect.context_memory == NULL)
		return;
	if(Reverbnode == 0 && Echonode == 0)
		return;

	printf("Reverbnode: %02X   Echonode:  %02X \n", Reverbnode, Echonode);
	uint16_t step;
	ReverbUnit *Reverbparam;
	EchoUnit *Echoparam;
	Reverbparam = (ReverbUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, Reverbnode, 0xff);
	Echoparam = (EchoUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, Echonode, 0xff);

	printf("Before,Reverb wet_scale:%d roomsize_scale:%d \n", Reverbparam->wet_scale, Reverbparam->roomsize_scale);
	printf("Before,Echo delay:%d ,attenuation:%d\n", Echoparam->delay, Echoparam->attenuation);

//	printf("Echo \nfc:0x%x \nattenuation:0x%x \ndelay:0x%x\n", Echoparam->fc, Echoparam->attenuation, Echoparam->delay);
//	printf("max_delay:0x%x \nmode:0x%x \ndry:0x%x \nwet:0x%x\n\n\n", Echoparam->max_delay, Echoparam->quality_mode, Echoparam->dry, Echoparam->wet);
//
//	printf("Reverb \ndry:0x%x \nwet:0x%x \nwidth:0x%x\n", Reverbparam->dry_scale, Reverbparam->wet_scale, Reverbparam->width_scale);
//	printf("roomsize:0x%x \ndamping:0x%x \nmono:0x%x\n", Reverbparam->roomsize_scale, Reverbparam->damping_scale, Reverbparam->mono);

	step = ReverbMaxParam.max_echo_delay  / MAX_MIC_REVB_STEP;
	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		Echoparam->delay = ReverbMaxParam.max_echo_delay ;
	}
	else
	{
		Echoparam->delay = ReverbStep * step;
	}

	step = ReverbMaxParam.max_echo_depth / MAX_MIC_REVB_STEP;
	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		Echoparam->attenuation = ReverbMaxParam.max_echo_depth;
	}
	else
	{
		Echoparam->attenuation = ReverbStep * step;
	}

	step = ReverbMaxParam.max_reverb_wet_scale / MAX_MIC_REVB_STEP;
	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		Reverbparam->wet_scale = ReverbMaxParam.max_reverb_wet_scale;
	}
	else
	{
		Reverbparam->wet_scale = ReverbStep * step;
	}

	step = ReverbMaxParam.max_reverb_roomsize / MAX_MIC_REVB_STEP;
	if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
	{
		Reverbparam->roomsize_scale = ReverbMaxParam.max_reverb_roomsize;
	}
	else
	{
		Reverbparam->roomsize_scale = ReverbStep * step;
	}

//	roboeffect_enable_effect(AudioCore.Roboeffect.context_memory, Reverbnode, 0);
	roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, Reverbnode, 0xff, (int16_t *)Reverbparam);
	roboeffect_enable_effect(AudioCore.Roboeffect.context_memory, Reverbnode, 1);

//	roboeffect_enable_effect(AudioCore.Roboeffect.context_memory, Echonode, 0);
	roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, Echonode, 0xff, (int16_t *)Echoparam);
	roboeffect_enable_effect(AudioCore.Roboeffect.context_memory, Echonode, 1);

	Reverbparam = (ReverbUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, Reverbnode, 0xff);
	Echoparam = (EchoUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, Echonode, 0xff);
	printf("After,Reverb wet_scale:%d roomsize_scale:%d \n", Reverbparam->wet_scale, Reverbparam->roomsize_scale);
	printf("After,Echo delay:%d ,attenuation:%d\n", Echoparam->delay, Echoparam->attenuation);
	gCtrlVars.AutoRefresh = 1;
#endif
}

uint16_t Roboeffect_SilenceDetector_Get(uint8_t node)
{
	if(AudioCore.Roboeffect.context_memory == NULL)
		return 0;
	SilenceDetectorUnit *SDParam;
	SDParam = (SilenceDetectorUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, node, 0xff);
	return SDParam->level;
}

uint16_t Roboeffect_GainControl_Get(uint8_t node)
{
	if(AudioCore.Roboeffect.context_memory == NULL)
		return 0;
	GainControlUnit *GainParam;
	GainParam = (GainControlUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, node, 0xff);
	return GainParam->gain;
}

void Roboeffect_GainControl_Set(uint8_t node, uint16_t gain)
{
	if(node < 0x81 || node > 0xfb)
	{
//		printf("Roboeffect_GainControl_Set node error:0x%x !!!\n", node);
		return;
	}
	if(AudioCore.Roboeffect.context_memory == NULL)
		return;

	int16_t param = gain;
	roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, node, 1, &param);
	roboeffect_update_local_params(node, 1, &param, 3);

	gCtrlVars.AutoRefresh = node;
}
void Roboeffect_GainMute_Set(uint8_t node, bool muteFlag)
{
	if(node < 0x81 || node > 0xfb)
	{
		return;
	}
	if(AudioCore.Roboeffect.context_memory == NULL)
		return;

	int16_t param = muteFlag;
	roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, node, 0, &param);
	roboeffect_update_local_params(node, 0, &param, 3);

//	gCtrlVars.AutoRefresh = node;
}

void Roboeffect_EQMode_Set(uint8_t EQMode)
{
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	if(AudioCore.Roboeffect.context_memory == NULL)
		return;
//	roboeffect_enable_effect(AudioCore.Roboeffect.context_memory, MUSIC_EQ_ADDR, 0);
    switch(EQMode)
	{
		case EQ_MODE_FLAT:
			roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, effect_addr[AudioCore.Roboeffect.flow_chart_mode].MUSIC_EQ_ADDR, 0xff, &Flat[0]);
			break;
		case EQ_MODE_CLASSIC:
			roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, effect_addr[AudioCore.Roboeffect.flow_chart_mode].MUSIC_EQ_ADDR, 0xff, &Classical[0]);
			break;
		case EQ_MODE_POP:
			roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, effect_addr[AudioCore.Roboeffect.flow_chart_mode].MUSIC_EQ_ADDR, 0xff, &Pop[0]);
			break;
		case EQ_MODE_ROCK:
			roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, effect_addr[AudioCore.Roboeffect.flow_chart_mode].MUSIC_EQ_ADDR, 0xff, &Rock[0]);
			break;
		case EQ_MODE_JAZZ:
			roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, effect_addr[AudioCore.Roboeffect.flow_chart_mode].MUSIC_EQ_ADDR, 0xff, &Jazz[0]);
			break;
		case EQ_MODE_VOCAL_BOOST:
			roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, effect_addr[AudioCore.Roboeffect.flow_chart_mode].MUSIC_EQ_ADDR, 0xff, &Vocal_Booster[0]);
			break;
	}
	printf("Roboeffect_EQMode_Set:%d\n", EQMode);
	roboeffect_enable_effect(AudioCore.Roboeffect.context_memory, effect_addr[AudioCore.Roboeffect.flow_chart_mode].MUSIC_EQ_ADDR, 1);
#endif
}

void Roboeffect_SourceGain_Update(uint8_t Index)
{
	if(AudioCore.Roboeffect.context_memory == NULL)
		return;

	switch(Index)
	{
		case APP_SOURCE_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].APP_SOURCE_GAIN_ADDR,
					roboeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
			break;
#ifdef CFG_RES_TWO_CHANNEL_I2S_EN
		case I2S_CH2_SOURCE_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].I2S2_SOURCE_GAIN_ADDR,
					roboeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[I2S_CH2_SOURCE_NUM]]);
			break;
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
		case REMIND_SOURCE_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].REMIND_SOURCE_GAIN_ADDR,
					roboeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]]);
			break;
#endif
		case MIC_SOURCE_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].MIC_SOURCE_GAIN_ADDR,
					roboeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
			break;

#ifdef CFG_FUNC_RECORDER_EN
		case PLAYBACK_SOURCE_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].REC_SOURCE_GAIN_ADDR,
					roboeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]]);

			break;
#endif
	}
}

void Roboeffect_SinkGain_Update(uint8_t Index)
{
	if(AudioCore.Roboeffect.context_memory == NULL)
		return;

	switch(Index)
	{
		case AUDIO_DAC0_SINK_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].DAC0_SINK_GAIN_ADDR,
					roboeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]]);
			break;
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
		case AUDIO_APP_SINK_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].APP_SINK_GAIN_ADDR,
					roboeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_APP_SINK_NUM]]);
			break;
#endif

#ifdef CFG_FUNC_RECORDER_EN
		case AUDIO_RECORDER_SINK_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].REC_SINK_GAIN_ADDR,
					roboeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]]);
			break;
#endif
#ifdef CFG_RES_TWO_CHANNEL_I2S_EN
		case I2S_CH2_SINK_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].I2S2_SINK_GAIN_ADDR,
					roboeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[I2S_CH2_SINK_NUM]]);
			break;
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
		case AUDIO_STEREO_SINK_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].STEREO_SINK_GAIN_ADDR,
					roboeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_STEREO_SINK_NUM]]);
			break;
#endif
		default:
			break;
	}
}

void Roboeffect_SinkMute_Set(bool muteFlag)
{
	if(AudioCore.Roboeffect.context_memory == NULL)
		return;

	uint8_t i;
	for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
	{
		switch(i)
		{
			case AUDIO_DAC0_SINK_NUM:
				Roboeffect_GainMute_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].DAC0_SINK_GAIN_ADDR, muteFlag);
				break;
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
			case AUDIO_APP_SINK_NUM:
				Roboeffect_GainMute_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].APP_SINK_GAIN_ADDR, muteFlag);
				break;
#endif

#ifdef CFG_FUNC_RECORDER_EN
			case AUDIO_RECORDER_SINK_NUM:
				Roboeffect_GainMute_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].REC_SINK_GAIN_ADDR, muteFlag);
				break;
#endif
#ifdef CFG_RES_TWO_CHANNEL_I2S_EN
			case I2S_CH2_SINK_NUM:
				Roboeffect_GainMute_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].I2S2_SINK_GAIN_ADDR, muteFlag);
				break;
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
			case AUDIO_STEREO_SINK_NUM:
				Roboeffect_GainMute_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].STEREO_SINK_GAIN_ADDR, muteFlag);
				break;
#endif
			default:
				break;
		}
	}

	gCtrlVars.AutoRefresh = 1;
}

void roboeffect_update_local_params(uint8_t addr, uint8_t param_index, int16_t *param_input, uint8_t param_len)
{
	uint8_t *params = AudioCore.Roboeffect.user_effect_parameters + 5;
	uint16_t data_len = *(uint16_t *)AudioCore.Roboeffect.user_effect_parameters - 5;
	uint8_t len = 0;

//	printf("input: 0x%x\n", *(uint16_t *)param_input);
	while(data_len)
	{
		if(*params == addr)
		{
			params += (param_index * 2 + 3);
//			printf("before: 0x%x\n", *(uint16_t *)params);
//			*(uint16_t *)params = *(uint16_t *)param_input;
			memcpy((uint16_t *)params, (uint16_t *)param_input, param_len - 1);
//			printf("addr:0x%x,index:%d,local:0x%x, len:%d\n", addr, param_index, *(uint16_t *)params, param_len);
			break;
		}
		else
		{
			params++;
			len = *params;
			params += (len + 1);
			data_len -= (len + 1);
		}
	}
}

void roboeffect_update_local_block_params(uint8_t addr)
{
	uint8_t *params = AudioCore.Roboeffect.user_effect_parameters + 5;
	uint16_t data_len = *(uint16_t *)AudioCore.Roboeffect.user_effect_parameters - 5;
	uint8_t len = 0;
	const uint8_t *p = (uint8_t *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, addr, 0xFF);
//	uint8_t i = 0;

	while(data_len)
	{
		if(*params == addr)
		{
			params++;
			len = *params;
			params+=2;
//			for(; i < len; i ++)
//			{
//				printf("0x%x, 0x%x\n", *(params + i), *(p + i));
//			}
			memcpy(params, p, len - 1);
			printf("addr:0x%x,param:0x%x, len:0x%x\n", addr, *(uint16_t *)params, len);
			break;
		}
		else
		{
			params++;
			len = *params;
			params += (len + 1);
			data_len -= (len + 1);
		}
	}
}

uint8_t AudioCoreSourceToRoboeffect(int8_t source)
{

	switch (source) {
		case MIC_SOURCE_NUM:
			return roboeffect_source[AudioCore.Roboeffect.flow_chart_mode].mic_source;
		case APP_SOURCE_NUM:
			return roboeffect_source[AudioCore.Roboeffect.flow_chart_mode].app_source;
		case REMIND_SOURCE_NUM:
			return roboeffect_source[AudioCore.Roboeffect.flow_chart_mode].remind_source;
		case PLAYBACK_SOURCE_NUM:
			return roboeffect_source[AudioCore.Roboeffect.flow_chart_mode].rec_source;
			break;
		default:
			// handle error
			return roboeffect_source[AudioCore.Roboeffect.flow_chart_mode].app_source;
	}
	return 0;
}

uint8_t AudioCoreSinkToRoboeffect(int8_t sink)
{
	switch (sink) {
		case AUDIO_DAC0_SINK_NUM:
			return roboeffect_sink[AudioCore.Roboeffect.flow_chart_mode].dac0_sink;
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
		case AUDIO_APP_SINK_NUM:
			return roboeffect_sink[AudioCore.Roboeffect.flow_chart_mode].app_sink;
#endif

#ifdef CFG_FUNC_RECORDER_EN
		case AUDIO_RECORDER_SINK_NUM:
			return roboeffect_sink[AudioCore.Roboeffect.flow_chart_mode].rec_sink;
#endif
#if defined(CFG_RES_AUDIO_I2SOUT_EN)
		case AUDIO_STEREO_SINK_NUM:
			return roboeffect_sink[AudioCore.Roboeffect.flow_chart_mode].stereo_sink;
#endif
		default:
			// handle error
			return roboeffect_sink[AudioCore.Roboeffect.flow_chart_mode].app_sink;
	}
	return 0;
}

uint16_t get_user_effect_parameters_len(uint8_t *user_effect_parameters)
{
	uint8_t b1 = user_effect_parameters[0];
	uint8_t b2 = user_effect_parameters[1];
    return (b2 << 8) | b1;
}

ROBOEFFECT_EFFECT_PARA * get_user_effect_parameters(ROBOEFFECT_EFFECT_MODE mode)
{
	return &roboeffect_para[mode];
}

roboeffect_effect_list_info *get_local_effect_list_buf(void)
{
	return &local_effect_list;
}

uint8_t get_roboeffect_addr(ROBOEFFECT_EFFECT_TYPE effect_name)
{
	switch(effect_name)
	{
		case APP_SOURCE_GAIN:
			return effect_addr[AudioCore.Roboeffect.flow_chart_mode].APP_SOURCE_GAIN_ADDR;
			break;
		case SILENCE_DETECTOR:
			return effect_addr[AudioCore.Roboeffect.flow_chart_mode].SILENCE_DETECTOR_ADDR;
			break;
		default:
			break;
	}

}

uint16_t get_roboeffectVolArr(uint8_t vol)
{
	return roboeffectVolArr[vol];
}
