#include <nds32_utils_math.h>
#include <string.h>
#include <math.h>
#include "roboeffect_api.h"
#include "user_defined_effect_api.h"
#include "user_effect_parameter.h"
#include "nn_denoise_api.h"
#include "main_task.h"

extern const ROBOEFFECT_EFFECT_PARA_TABLE bypass_node;
extern const ROBOEFFECT_EFFECT_PARA_TABLE hfp_node;
extern const ROBOEFFECT_EFFECT_PARA_TABLE karaoke_node;
extern const ROBOEFFECT_EFFECT_PARA_TABLE mic_node;
extern const ROBOEFFECT_EFFECT_PARA_TABLE music_node;

static const ROBOEFFECT_EFFECT_PARA_TABLE *effect_para_table[] =
{
#ifdef CFG_FUNC_EFFECT_BYPASS_EN
	&bypass_node,
#else
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	&karaoke_node,
#else
	&hfp_node,
	&mic_node,
	&music_node,
#endif
#endif
	NULL,
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

ROBOEFFECT_EFFECT_PARA_TABLE * GetCurEffectParaNode(void)
{
	uint32_t i = 0;

	while(effect_para_table[i] != NULL)
	{
		if((mainAppCt.EffectMode >= effect_para_table[i]->effect_id ) &&
		   (mainAppCt.EffectMode < (effect_para_table[i]->effect_id + effect_para_table[i]->effect_id_count)))
			return (ROBOEFFECT_EFFECT_PARA_TABLE *)effect_para_table[i];
		else
			i++;
	}
	return (ROBOEFFECT_EFFECT_PARA_TABLE *)effect_para_table[0];
}

ROBOEFFECT_EFFECT_PARA * get_user_effect_parameters(uint8_t mode)
{
	ROBOEFFECT_EFFECT_PARA_TABLE *param = GetCurEffectParaNode();

	uint8_t index = mode - param->effect_id;

	if(index > param->effect_id_count)
		index = 0;

	DBG("EFFECT_MODE: %d\n",(int)param->effect_id + index);
	return param->roboeffect_para + index;
}

void Roboeffect_GetAudioEffectMaxValue(void)
{
	if(AudioCore.Roboeffect.context_memory == NULL)
		return;
	ReverbUnit *Reverbparam;
	EchoUnit *Echoparam;

	Reverbparam = (ReverbUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory,
										get_roboeffect_addr(REVERB), 0xff);
	Echoparam = (EchoUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory,
										get_roboeffect_addr(ECHO), 0xff);

	ReverbMaxParam.max_reverb_wet_scale  = Reverbparam->wet_scale;

	ReverbMaxParam.max_reverb_roomsize   = Reverbparam->roomsize_scale;

	ReverbMaxParam.max_echo_delay        = Echoparam->delay;

	ReverbMaxParam.max_echo_depth        = Echoparam->attenuation;
}

#if defined(CFG_FUNC_MIC_TREB_BASS_EN) || defined(CFG_FUNC_MUSIC_TREB_BASS_EN)
void Roboeffect_EQ_Ajust(ROBOEFFECT_EFFECT_TYPE type,uint8_t BassGain, uint8_t TrebGain)
{
	uint8_t node;

	node = get_roboeffect_addr(type);

	if(AudioCore.Roboeffect.context_memory == NULL)
		return;
	if(node == 0)
		return;

	DBG("node: %02X\n", node);
	int16_t param = 0;

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
	uint8_t Reverbnode = get_roboeffect_addr(REVERB);
	uint8_t Echonode = get_roboeffect_addr(ECHO);

	if(AudioCore.Roboeffect.context_memory == NULL)
		return;
	if(Reverbnode == 0 && Echonode == 0)
		return;

	uint16_t step;
	ReverbUnit *Reverbparam;
	EchoUnit *Echoparam;
	Reverbparam = (ReverbUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, Reverbnode, 0xff);
	Echoparam = (EchoUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, Echonode, 0xff);

	DBG("Before,Reverb wet_scale:%d roomsize_scale:%d \n", Reverbparam->wet_scale, Reverbparam->roomsize_scale);
	DBG("Before,Echo delay:%d ,attenuation:%d\n", Echoparam->delay, Echoparam->attenuation);

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

	roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, Reverbnode, 1, &Reverbparam->wet_scale);
	roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, Reverbnode, 3, &Reverbparam->roomsize_scale);
//	roboeffect_enable_effect(AudioCore.Roboeffect.context_memory, Reverbnode, 1);

	roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, Echonode, 1, &Echoparam->attenuation);
	roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, Echonode, 2, &Echoparam->delay);
//	roboeffect_enable_effect(AudioCore.Roboeffect.context_memory, Echonode, 1);

	Reverbparam = (ReverbUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, Reverbnode, 0xff);
	Echoparam = (EchoUnit *)roboeffect_get_effect_parameter(AudioCore.Roboeffect.context_memory, Echonode, 0xff);
	DBG("After,Reverb wet_scale:%d roomsize_scale:%d \n", Reverbparam->wet_scale, Reverbparam->roomsize_scale);
	DBG("After,Echo delay:%d ,attenuation:%d\n", Echoparam->delay, Echoparam->attenuation);
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
			roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, get_roboeffect_addr(MUSIC_EQ), 0xff, &Flat[0]);
			break;
		case EQ_MODE_CLASSIC:
			roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, get_roboeffect_addr(MUSIC_EQ), 0xff, &Classical[0]);
			break;
		case EQ_MODE_POP:
			roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, get_roboeffect_addr(MUSIC_EQ), 0xff, &Pop[0]);
			break;
		case EQ_MODE_ROCK:
			roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, get_roboeffect_addr(MUSIC_EQ), 0xff, &Rock[0]);
			break;
		case EQ_MODE_JAZZ:
			roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, get_roboeffect_addr(MUSIC_EQ), 0xff, &Jazz[0]);
			break;
		case EQ_MODE_VOCAL_BOOST:
			roboeffect_set_effect_parameter(AudioCore.Roboeffect.context_memory, get_roboeffect_addr(MUSIC_EQ), 0xff, &Vocal_Booster[0]);
			break;
	}
    DBG("Roboeffect_EQMode_Set:%d\n", EQMode);
	roboeffect_enable_effect(AudioCore.Roboeffect.context_memory, get_roboeffect_addr(MUSIC_EQ), 1);
#endif
}

void Roboeffect_SourceGain_Update(uint8_t Index)
{
	if(AudioCore.Roboeffect.context_memory == NULL)
		return;

	switch(Index)
	{
		case APP_SOURCE_NUM:
			Roboeffect_GainControl_Set(get_roboeffect_addr(APP_SOURCE_GAIN),
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
			Roboeffect_GainControl_Set(get_roboeffect_addr(REMIND_SOURCE_GAIN),
					roboeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]]);
			break;
#endif
		case MIC_SOURCE_NUM:
			Roboeffect_GainControl_Set(get_roboeffect_addr(MIC_SOURCE_GAIN),
					roboeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
			break;

#ifdef CFG_FUNC_RECORDER_EN
		case PLAYBACK_SOURCE_NUM:
			Roboeffect_GainControl_Set(get_roboeffect_addr(REC_SOURCE_GAIN),
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
			Roboeffect_GainControl_Set(get_roboeffect_addr(DAC0_SINK_GAIN),
					roboeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]]);
			break;
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
		case AUDIO_APP_SINK_NUM:
			Roboeffect_GainControl_Set(get_roboeffect_addr(APP_SINK_GAIN),
					roboeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_APP_SINK_NUM]]);
			break;
#endif

#ifdef CFG_FUNC_RECORDER_EN
		case AUDIO_RECORDER_SINK_NUM:
			Roboeffect_GainControl_Set(get_roboeffect_addr(REC_SINK_GAIN),
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
			Roboeffect_GainControl_Set(get_roboeffect_addr(STEREO_SINK_GAIN),
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
				Roboeffect_GainMute_Set(get_roboeffect_addr(DAC0_SINK_GAIN), muteFlag);
				break;
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
			case AUDIO_APP_SINK_NUM:
				Roboeffect_GainMute_Set(get_roboeffect_addr(APP_SINK_GAIN), muteFlag);
				break;
#endif

#ifdef CFG_FUNC_RECORDER_EN
			case AUDIO_RECORDER_SINK_NUM:
				Roboeffect_GainMute_Set(get_roboeffect_addr(REC_SINK_GAIN), muteFlag);
				break;
#endif
#ifdef CFG_RES_TWO_CHANNEL_I2S_EN
			case I2S_CH2_SINK_NUM:
				Roboeffect_GainMute_Set(effect_addr[AudioCore.Roboeffect.flow_chart_mode].I2S2_SINK_GAIN_ADDR, muteFlag);
				break;
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
			case AUDIO_STEREO_SINK_NUM:
				Roboeffect_GainMute_Set(get_roboeffect_addr(STEREO_SINK_GAIN), muteFlag);
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

//	DBG("input: 0x%x\n", *(uint16_t *)param_input);
	while(data_len)
	{
		if(*params == addr)
		{
			params += (param_index * 2 + 3);
//			DBG("before: 0x%x\n", *(uint16_t *)params);
//			*(uint16_t *)params = *(uint16_t *)param_input;
			memcpy((uint16_t *)params, (uint16_t *)param_input, param_len - 1);
//			DBG("addr:0x%x,index:%d,local:0x%x, len:%d\n", addr, param_index, *(uint16_t *)params, param_len);
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
//				DBG("0x%x, 0x%x\n", *(params + i), *(p + i));
//			}
			memcpy(params, p, len - 1);
			DBG("addr:0x%x,param:0x%x, len:0x%x\n", addr, *(uint16_t *)params, len);
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

uint8_t Roboeffect_effect_status_Get(ROBOEFFECT_EFFECT_TYPE type)
{
	return roboeffect_get_effect_status(AudioCore.Roboeffect.context_memory, get_roboeffect_addr(type));
}

void Roboeffect_effect_enable(ROBOEFFECT_EFFECT_TYPE type, uint8_t enable)
{
	AudioCore.Roboeffect.effect_addr = get_roboeffect_addr(type);
	AudioCore.Roboeffect.effect_enable = enable;

	MessageContext msgSend;
	msgSend.msgId = MSG_EFFECTREINIT;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

uint8_t AudioCoreSourceToRoboeffect(int8_t source)
{
	ROBOEFFECT_EFFECT_PARA_TABLE *param = GetCurEffectParaNode();

	switch (source)
	{
		case MIC_SOURCE_NUM:
			return param->roboeffect_source.mic_source;
		case APP_SOURCE_NUM:
			return param->roboeffect_source.app_source;
		case REMIND_SOURCE_NUM:
			return param->roboeffect_source.remind_source;
		case PLAYBACK_SOURCE_NUM:
			return param->roboeffect_source.rec_source;
		case I2S_MIX_SOURCE_NUM:
			return param->roboeffect_source.i2s_mix_source;
		case USB_SOURCE_NUM:
			return param->roboeffect_source.usb_source;
		case LINEIN_MIX_SOURCE_NUM:
			return param->roboeffect_source.linein_mix_source;
		default:
			break;// handle error
	}
	return AUDIOCORE_SOURCE_SINK_ERROR;
}

uint8_t AudioCoreSinkToRoboeffect(int8_t sink)
{
	ROBOEFFECT_EFFECT_PARA_TABLE *param = GetCurEffectParaNode();

	switch (sink)
	{
		case AUDIO_DAC0_SINK_NUM:
			return param->roboeffect_sink.dac0_sink;
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
		case AUDIO_APP_SINK_NUM:
			return param->roboeffect_sink.app_sink;
#endif

#ifdef CFG_FUNC_RECORDER_EN
		case AUDIO_RECORDER_SINK_NUM:
			return param->roboeffect_sink.rec_sink;
#endif
#if defined(CFG_RES_AUDIO_I2SOUT_EN)
		case AUDIO_STEREO_SINK_NUM:
			return param->roboeffect_sink.stereo_sink;
#endif
#ifdef CFG_RES_AUDIO_I2S_MIX_OUT_EN
		case AUDIO_I2S_MIX_OUT_SINK_NUM:
			return param->roboeffect_sink.i2s_mix_sink;
#endif
		default:
			// handle error
			break;
	}
	return AUDIOCORE_SOURCE_SINK_ERROR;
}

uint16_t get_user_effect_parameters_len(uint8_t *user_effect_parameters)
{
	uint8_t b1 = user_effect_parameters[0];
	uint8_t b2 = user_effect_parameters[1];
    return (b2 << 8) | b1;
}

roboeffect_effect_list_info *get_local_effect_list_buf(void)
{
	return &local_effect_list;
}

uint8_t get_roboeffect_addr(ROBOEFFECT_EFFECT_TYPE effect_name)
{
	ROBOEFFECT_EFFECT_PARA_TABLE *param = GetCurEffectParaNode();
	uint8_t addr = 0;

	switch(effect_name)
	{
		case MUSIC_EQ:
			addr = param->effect_addr.MUSIC_EQ_ADDR;
			break;
		case MIC_EQ:
			addr = param->effect_addr.MIC_EQ_ADDR;
			break;
		case REVERB:
			addr = param->effect_addr.REVERB_ADDR;
			break;
		case ECHO:
			addr = param->effect_addr.ECHO_ADDR;
			break;
		case SILENCE_DETECTOR:
			addr = param->effect_addr.SILENCE_DETECTOR_ADDR;
			break;
		case VOICE_CHANGER:
			addr = param->effect_addr.VOICE_CHANGER_ADDR;
			break;
		case APP_SOURCE_GAIN:
			addr = param->effect_addr.APP_SOURCE_GAIN_ADDR;
			break;
		case REMIND_SOURCE_GAIN:
			addr = param->effect_addr.REMIND_SOURCE_GAIN_ADDR;
			break;
		case MIC_SOURCE_GAIN:
			addr = param->effect_addr.MIC_SOURCE_GAIN_ADDR;
			break;
		case REC_SOURCE_GAIN:
			addr = param->effect_addr.REC_SOURCE_GAIN_ADDR;
			break;
		case DAC0_SINK_GAIN:
			addr = param->effect_addr.DAC0_SINK_GAIN_ADDR;
			break;
		case APP_SINK_GAIN:
			addr = param->effect_addr.APP_SINK_GAIN_ADDR;
			break;
		case STEREO_SINK_GAIN:
			addr = param->effect_addr.STEREO_SINK_GAIN_ADDR;
			break;
		case REC_SINK_GAIN:
			addr = param->effect_addr.REC_SINK_GAIN_ADDR;
			break;
		default:
			addr = 0;
			break;
	}
	return addr;
}

uint16_t get_roboeffectVolArr(uint8_t vol)
{
	return roboeffectVolArr[vol];
}
