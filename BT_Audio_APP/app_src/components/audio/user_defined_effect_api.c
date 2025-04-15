/**
 **************************************************************************************
 * @file    user_defined_api.c
 * @brief   interface for user defined effect algorithm
 *
 * @author  Castle Cai
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <nds32_utils_math.h>
#include <string.h>
#include <math.h>
#include "roboeffect_api.h"
#include "user_defined_effect_api.h"
#include "nn_denoise_api.h"
#include "main_task.h"

const ROBOEFFECT_EFFECT_ADDR effect_addr[] = {
  //{music_EQ ,mic_EQ ,REVERB  ,ECHO  ,silence  ,APP_source  ,remind  ,mic  ,DAC0  ,APP_sink  ,stereo},
    {0x0      ,0x0    ,0x0     ,0x0   ,0x0      ,0x86        ,0x0     ,0x85 ,0x0   ,0x0       ,0x0},//mic
    {0x0      ,0x0    ,0x0     ,0x0   ,0x0      ,0x86        ,0x0     ,0x85 ,0x0   ,0x0       ,0x0},//music
    {0x0      ,0x0    ,0x0     ,0x0   ,0x0      ,0x86        ,0x0     ,0x85 ,0x0   ,0x0       ,0x0},//HFP
    {0x0      ,0x0    ,0x0     ,0x0   ,0x8F     ,0x81        ,0x0     ,0x8D ,0x0   ,0xAC      ,0x0},//HUNXIANG
    {0x0      ,0x0    ,0x0     ,0x0   ,0x8F     ,0x81        ,0x0     ,0x8D ,0x0   ,0xAC      ,0x0},//DIANYIN
    {0x0      ,0x0    ,0x0     ,0x0   ,0x8F     ,0x81        ,0x0     ,0x8D ,0x0   ,0xAC      ,0x0},//MOYIN
    {0x0      ,0x0    ,0x0     ,0x0   ,0x8F     ,0x81        ,0x0     ,0x8D ,0x0   ,0xAC      ,0x0},//HANMAI
    {0x0      ,0x0    ,0x0     ,0x0   ,0x8F     ,0x81        ,0x0     ,0x8D ,0x0   ,0xAC      ,0x0},//NANBIANNV
    {0x0      ,0x0    ,0x0     ,0x0   ,0x8F     ,0x81        ,0x0     ,0x8D ,0x0   ,0xAC      ,0x0},//NVBIANNAN
    {0x0      ,0x0    ,0x0     ,0x0   ,0x8F     ,0x81        ,0x0     ,0x8D ,0x0   ,0xAC      ,0x0},//WAWAYIN
};
const ROBOEFFECT_SOURCE_NUM roboeffect_source[] = {
  //{mic_source,			 	app_source,			 		remind_source},
	{MIC_SOURCE_MIC_SOURCE, 	MIC_SOURCE_APP_SOURCE, 		MIC_SOURCE_REMIND_SOURCE},		//mic
	{MUSIC_SOURCE_MIC_SOURCE,	MUSIC_SOURCE_APP_SOURCE, 	MUSIC_SOURCE_REMIND_SOURCE},	//music
	{HFP_SOURCE_MIC_SOURCE,		HFP_SOURCE_APP_SOURCE, 		HFP_SOURCE_REMIND_SOURCE},		//HFP
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE},	//HUNXIANG
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE},	//DIANYIN
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE},	//MOYIN
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE},	//HANMAI
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE},	//NANBIANNV
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE},	//NVBIANNAN
	{KARAOKE_SOURCE_MIC_SOURCE, KARAOKE_SOURCE_APP_SOURCE, 	KARAOKE_SOURCE_REMIND_SOURCE},	//WAWAYIN
};
const ROBOEFFECT_SINK_NUM roboeffect_sink[] = {
  //{dac0_sink,			 	 app_sink,				stereo_sink},
	{MIC_SINK_DAC0_SINK,	 MIC_SINK_APP_SINK,     MIC_SINK_STEREO_SINK},		//mic
	{MUSIC_SINK_DAC0_SINK,	 MUSIC_SINK_APP_SINK,	MUSIC_SINK_STEREO_SINK},	//music
	{HFP_SINK_DAC0_SINK,	 HFP_SINK_APP_SINK,		HFP_SINK_STEREO_SINK},		//HFP
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK},	//HUNXIANG
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK},	//DIANYIN
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK},	//MOYIN
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK},	//HANMAI
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK},	//NANBIANNV
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK},	//NVBIANNAN
	{KARAOKE_SINK_DAC0_SINK, KARAOKE_SINK_APP_SINK,	KARAOKE_SINK_STEREO_SINK},	//WAWAYIN
};

const uint16_t roboeffectVolArr[CFG_PARA_MAX_VOLUME_NUM + 1] =
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

ReverbMaxUnit ReverbMaxParam = {0 , 0 , 0 ,0};
roboeffect_effect_list_info local_effect_list;
/**
 * ai denoise begin
*/
bool roboeffect_ai_denoise_init_if(void *node)
{
	uint8_t *context_ptr, *scratch;
	roboeffect_user_defined_effect_info info;
	ai_denoise_struct *ai_info;

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct
	*/
	ai_info = info.context_memory;

	/**
	 * clear user defined algorithm context
	*/
	memset(ai_info, 0x00, sizeof(ai_denoise_struct) + AI_MEMORY_SIZE + AI_RAM_SIZE);

	/**
	 *get user defined algorithm context
	*/
	context_ptr = ((uint8_t*)info.context_memory) + sizeof(ai_denoise_struct);

	/**
	 * get scratch memory
	*/
	scratch = info.scratch_memory;

	/**
	 * initialize 
	*/
	ai_info->p_nn_denoise = nn_denoise_init(context_ptr, scratch, info.sample_rate, 10, NN_MODE_DENOISE_M3);
	nn_denoise_set_ram(ai_info->p_nn_denoise, context_ptr + AI_MEMORY_SIZE);

	/**
	 * set default parameters
	*/
	nn_set_max_suppress(ai_info->p_nn_denoise, (float)info.parameters[0]);

	return TRUE;
}

bool roboeffect_ai_denoise_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len)
{
//	int ret;
	uint8_t method_flag = 0;
	roboeffect_user_defined_effect_info info;
	ai_denoise_struct *ai_info;

	/**
	 * check parameters and update to effect instance
	*/
	if(ROBOEFFECT_ERROR_OK > roboeffect_user_defined_params_check(node, new_param, param_num, len, &method_flag))
	{
		return FALSE;
	}

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct
	*/
	ai_info = info.context_memory;

	if((method_flag & METHOD_CFG_1) && info.is_active)
	{
		// printf("call METHOD_CFG_1 func: %d\n", info.parameters[0]);
		nn_set_max_suppress(ai_info->p_nn_denoise, (float)info.parameters[0]);
	}

	return TRUE;
}


bool roboeffect_ai_denoise_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n)
{
//	int i;
	roboeffect_user_defined_effect_info info;
	ai_denoise_struct *ai_info;
	
	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct
	*/
	ai_info = info.context_memory;
	
	/**
	 * apply data
	*/
	nds32_convert_q15_f32(pcm_in1, ai_info->float_buffer_in, n);
	nn_denoise_process(ai_info->p_nn_denoise, ai_info->float_buffer_in, ai_info->float_buffer_out);
	nds32_convert_f32_q15(ai_info->float_buffer_out, pcm_out, n);

	return TRUE;
}


int32_t roboeffect_ai_denoise_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters)
{
	return AI_SCRATCH_SIZE;
}

int32_t roboeffect_user_gain_scratch_size(uint32_t sample_rate, uint16_t frame_size, uint16_t width, uint8_t ch_num, int16_t *parameters)
{
	return 16;
}

/**
 * user gain begin
*/
bool roboeffect_user_gain_init_if(void *node)
{
//	uint8_t *context_ptr;
	roboeffect_user_defined_effect_info info;
	user_gain_struct *gain_info;

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct context
	*/
	gain_info = info.context_memory;

	/**
	 * dummy initialize the context of the user_gain
	*/
	gain_info->data_a = 0xAB;
	gain_info->data_b = 0xCD;


	return TRUE;
}

bool roboeffect_user_gain_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len)
{
//	int ret;
	uint8_t method_flag = 0;
	roboeffect_user_defined_effect_info info;
//	user_gain_struct *gain_info;

	/**
	 * check parameters and update to effect instance
	*/
	if(ROBOEFFECT_ERROR_OK < roboeffect_user_defined_params_check(node, new_param, param_num, len, &method_flag))
	{
		return FALSE;
	}

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct context
	*/
//	gain_info = info.context_memory;

	if((method_flag & METHOD_CFG_1) && info.is_active)
	{

	}

	return TRUE;
}

#define SCALING_Q12_MAX (0x1000)
static inline uint32_t db_to_scaling(float db, uint32_t scaling_max)
{
	return (uint32_t)roundf(powf(10.0f,((float)db/20.0f)) * scaling_max);
}

bool roboeffect_user_gain_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n)
{
//	int i, s;
	int s;
	int32_t pregain, *pcm_in_24 = (int32_t*)pcm_in1, *pcm_out_24 = (int32_t*)pcm_out;
	roboeffect_user_defined_effect_info info;
//	user_gain_struct *gain_info;

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct context
	*/
//	gain_info = info.context_memory;
	
	/**
	 * apply data
	*/

	//info.parameters[0] is a dB, convert it to Q12.
	
	pregain = db_to_scaling(((float)info.parameters[0])/100, SCALING_Q12_MAX);

	if(info.width == BITS_24)
	{
		for(s = 0; s < n; s++)
		{
			if(info.ch_num == 2)
			{
				pcm_out_24[2 * s + 0] = __nds32__clips((((int64_t)pcm_in_24[2 * s + 0] * pregain + 2048) >> 12), (24)-1);
				pcm_out_24[2 * s + 1] = __nds32__clips((((int64_t)pcm_in_24[2 * s + 1] * pregain + 2048) >> 12), (24)-1);
			}
			else
			{
				pcm_out_24[s] = __nds32__clips((((int64_t)pcm_in_24[s] * pregain + 2048) >> 12), (24)-1);
			}
		}
	}
	else if(info.width == BITS_16)
	{
		for(s = 0; s < n; s++)
		{
			if(info.ch_num == 2)
			{
				pcm_out[2 * s + 0] = __nds32__clips((((int32_t)pcm_in1[2 * s + 0] * pregain + 2048) >> 12), (16)-1);
				pcm_out[2 * s + 1] = __nds32__clips((((int32_t)pcm_in1[2 * s + 1] * pregain + 2048) >> 12), (16)-1);
			}
			else
			{
				pcm_out[s] = __nds32__clips((((int32_t)pcm_in1[s] * pregain + 2048) >> 12), (16)-1);
			}
		}
	}

	return TRUE;
}


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
void Roboeffect_EQ_Ajust(uint8_t node, uint8_t BassGain, uint8_t TrebGain)
{
	if(AudioCore.Roboeffect.context_memory == NULL)
		return;
	if(node == 0)
		return;

	printf("node: %02X\n", node);
	EQUnit *EQparam;
//	uint8_t i;

	EQparam = (EQUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, node, 0xff);

//	printf("EQ pregain:0x%x \ncalculation_type:0x%x\n", EQparam->pregain, EQparam->calculation_type);
//	for(i = 0; i < 10; i++)
//	{
//		printf("enable:%x \ntype:0x%x\n", EQparam->eq_params[i].enable, EQparam->eq_params[i].type);
//		printf("f0:0x%x \nQ:0x%x \ngain:0x%x\n", EQparam->eq_params[i].f0, EQparam->eq_params[i].Q, EQparam->eq_params[i].gain);
//	}
	printf("Before,EQ BassGain:%d ,TrebGain:%d\n", EQparam->eq_params[0].gain, EQparam->eq_params[1].gain);
	printf("Need,EQ BassGain:%d ,TrebGain:%d\n", BassTrebGainTable[BassGain], BassTrebGainTable[TrebGain]);

	EQparam->eq_params[0].gain = BassTrebGainTable[BassGain];
	EQparam->eq_params[1].gain = BassTrebGainTable[TrebGain];

//	roboeffect_enable_effect(AudioCore.Roboeffect.context_memory, node, 0);
	roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, node, 0xff, (int16_t *)EQparam);
    roboeffect_enable_effect(AudioCore.Roboeffect.context_memory, node, 1);

	EQparam = (EQUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, node, 0xff);

	printf("Later,EQ BassGain:%d ,TrebGain:%d\n", EQparam->eq_params[0].gain, EQparam->eq_params[1].gain);
	if(EQparam->eq_params[0].gain == BassTrebGainTable[BassGain])
	{
		printf("EQ BassGain update ok\n");
	}
	if(EQparam->eq_params[1].gain == BassTrebGainTable[TrebGain])
	{
		printf("EQ TrebGain update ok\n");
	}
	gCtrlVars.AutoRefresh = 1;
}
#endif

void Roboeffect_ReverbStep_Ajust(uint8_t Reverbnode, uint8_t Echonode, uint8_t ReverbStep)
{
#ifdef CFG_FUNC_MIC_KARAOKE_EN
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

	GainControlUnit *GainParam;
	GainParam = (GainControlUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, node, 0xff);
	GainParam->gain = roboeffectVolArr[gain];
	roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, node, 0xff, (int16_t *)GainParam);

//	printf("Roboeffect_GainControl_Set node:0x%x gain:0x%x\n", node, GainParam->gain);
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
										mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
			break;
#ifdef CFG_RES_TWO_CHANNEL_I2S_EN
		case I2S_CH2_SOURCE_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].I2S2_SOURCE_GAIN_ADDR,
										mainAppCt.gSysVol.AudioSourceVol[I2S_CH2_SOURCE_NUM]);
			break;
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
		case REMIND_SOURCE_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].REMIND_SOURCE_GAIN_ADDR,
										mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]);
			break;
#endif
		case MIC_SOURCE_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].MIC_SOURCE_GAIN_ADDR,
										mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
			break;
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
										mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]);
			break;
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
		case AUDIO_APP_SINK_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].APP_SINK_GAIN_ADDR,
										mainAppCt.gSysVol.AudioSinkVol[AUDIO_APP_SINK_NUM]);
			break;
#endif
#ifdef CFG_RES_TWO_CHANNEL_I2S_EN
		case I2S_CH2_SINK_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].I2S2_SINK_GAIN_ADDR,
										mainAppCt.gSysVol.AudioSinkVol[I2S_CH2_SINK_NUM]);
			break;
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
		case AUDIO_STEREO_SINK_NUM:
			Roboeffect_GainControl_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].STEREO_SINK_GAIN_ADDR,
										mainAppCt.gSysVol.AudioSinkVol[AUDIO_STEREO_SINK_NUM]);
			break;
#endif
		default:
			break;
	}
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

#if 0
/**
 * howling_supressor_fine begin
*/
bool roboeffect_howling_suppressor_fine_init_if(void *node)
{
	uint8_t *context_ptr;
	roboeffect_user_defined_effect_info info;
	HowlingFineContext *hfc;

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct context
	*/
	hfc = info.context_memory;

	/**
	 * dummy initialize the context of the user_gain
	*/
	if(HOWLING_SUPPRESSOR_FINE_ERROR_OK == howling_suppressor_fine_init(hfc, info.sample_rate))
		return TRUE;
	else
		return FALSE;

	return TRUE;
}

bool roboeffect_howling_suppressor_fine_config_if(void *node, int16_t *new_param, uint8_t param_num, uint8_t len)
{
	int ret;
	uint8_t method_flag = 0;
	roboeffect_user_defined_effect_info info;
	HowlingFineContext *hfc;

	/**
	 * check parameters and update to effect instance
	*/
	if(ROBOEFFECT_ERROR_OK < roboeffect_user_defined_params_check(node, new_param, param_num, len, &method_flag))
	{
		return FALSE;
	}

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct context
	*/
	hfc = info.context_memory;

	if((method_flag & METHOD_CFG_1) && info.is_active)
	{

	}

	return TRUE;
}

bool roboeffect_howling_suppressor_fine_apply_if(void *node, int16_t *pcm_in1, int16_t *pcm_in2, int16_t *pcm_out, int32_t n)
{
	int i, s;
	int32_t pregain, *pcm_in_24 = (int32_t*)pcm_in1, *pcm_out_24 = (int32_t*)pcm_out;
	roboeffect_user_defined_effect_info info;
	HowlingFineContext *hfc;

	/**
	 * get information struct from instance
	*/
	roboeffect_user_defined_get_info(node, &info);

	/**
	 * get user defined struct context
	*/
	hfc = info.context_memory;
	
	/**
	 * apply data
	*/
	// printf("%d, %d, %d, %d\n", info.parameters[0], info.parameters[1], (int32_t)info.parameters[0] * 1024 / 100, (int32_t)info.parameters[1] * 1024 / 100);
	howling_suppressor_fine_apply(hfc, pcm_in1, pcm_out, n, (int32_t)info.parameters[0] * 1024 / 100, (int32_t)info.parameters[1] * 1024 / 100);


	return TRUE;
}

#endif
