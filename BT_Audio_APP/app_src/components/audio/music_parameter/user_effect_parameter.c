#include <nds32_utils_math.h>
#include <string.h>
#include <math.h>
#include "roboeffect_api.h"
#include "user_defined_effect_api.h"
#include "user_effect_parameter.h"
#include "nn_denoise_api.h"
#include "main_task.h"
#include "bt_config.h"

#ifdef CFG_FUNC_EFFECT_BYPASS_EN
extern const AUDIOEFFECT_EFFECT_PARA_TABLE bypass_node;
#else
#ifdef CFG_FUNC_MIC_KARAOKE_EN
extern const AUDIOEFFECT_EFFECT_PARA_TABLE karaoke_node;
#else
extern const AUDIOEFFECT_EFFECT_PARA_TABLE mic_node;
extern const AUDIOEFFECT_EFFECT_PARA_TABLE music_node;
#endif
#endif
#if defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)
extern const AUDIOEFFECT_EFFECT_PARA_TABLE hfp_node;
#endif

static const AUDIOEFFECT_EFFECT_PARA_TABLE *effect_para_table[] =
{
#ifdef CFG_FUNC_EFFECT_BYPASS_EN
	&bypass_node,
#else
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	&karaoke_node,
#else
	&mic_node,
	&music_node,
#endif
#endif
#if defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)
	&hfp_node,
#endif
	NULL,
};

static const uint16_t audioeffectVolArr[CFG_PARA_MAX_VOLUME_NUM + 1] =
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

#ifdef CFG_FUNC_MIC_KARAOKE_EN
static ReverbMaxUnit ReverbMaxParam = {0 , 0 , 0 ,0};
#endif

AUDIOEFFECT_EFFECT_PARA_TABLE * GetCurEffectParaNode(void)
{
	uint32_t i = 0;

	while(effect_para_table[i] != NULL)
	{
		if((mainAppCt.EffectMode >= effect_para_table[i]->effect_id ) &&
		   (mainAppCt.EffectMode < (effect_para_table[i]->effect_id + effect_para_table[i]->effect_id_count)))
			return (AUDIOEFFECT_EFFECT_PARA_TABLE *)effect_para_table[i];
		else
			i++;
	}
	return (AUDIOEFFECT_EFFECT_PARA_TABLE *)effect_para_table[0];
}

AUDIOEFFECT_EFFECT_PARA * get_user_effect_parameters(uint8_t mode)
{
	AUDIOEFFECT_EFFECT_PARA_TABLE *param = GetCurEffectParaNode();

	uint8_t index = mode - param->effect_id;

	if(index > param->effect_id_count)
		index = 0;

	return param->audioeffect_para + index;
}

void AudioEffect_GetAudioEffectMaxValue(void)
{
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	if(AudioCore.Audioeffect.context_memory == NULL)
		return;

	ReverbUnit *Reverbparam;
	EchoUnit *Echoparam;
	ReverbPlateUnit *ReverbPlateparam;

	if(AudioEffect_effectAddr_check(get_audioeffect_addr(REVERB)))
	{
		Reverbparam = (ReverbUnit *)roboeffect_get_effect_parameter(AudioCore.Audioeffect.context_memory,
											get_audioeffect_addr(REVERB), 0xff);
		ReverbMaxParam.max_reverb_wet_scale  = Reverbparam->wet_scale;
		ReverbMaxParam.max_reverb_roomsize   = Reverbparam->roomsize_scale;
	}
	if(AudioEffect_effectAddr_check(get_audioeffect_addr(ECHO)))
	{
		Echoparam = (EchoUnit *)roboeffect_get_effect_parameter(AudioCore.Audioeffect.context_memory,
											get_audioeffect_addr(ECHO), 0xff);
		ReverbMaxParam.max_echo_delay        = Echoparam->delay;
		ReverbMaxParam.max_echo_depth        = Echoparam->attenuation;
	}
	if(AudioEffect_effectAddr_check(get_audioeffect_addr(REVERBPLATE)))
	{
		ReverbPlateparam = (ReverbPlateUnit *)roboeffect_get_effect_parameter(AudioCore.Audioeffect.context_memory,
											get_audioeffect_addr(REVERBPLATE), 0xff);
		ReverbMaxParam.max_reverbplate_wetdrymix   = ReverbPlateparam->wetdrymix;
	}
#endif
}

#if defined(CFG_FUNC_MIC_TREB_BASS_EN) || defined(CFG_FUNC_MUSIC_TREB_BASS_EN)
void AudioEffect_EQ_Ajust(AUDIOEFFECT_EFFECT_TYPE type,uint8_t BassGain, uint8_t TrebGain)
{

	if(AudioCore.Audioeffect.context_memory == NULL)
		return;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(type)))
		return;

	int16_t param = 0;
	uint8_t node = get_audioeffect_addr(type);

	param = BassTrebGainTable[BassGain];
	roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, node, 6, &param);
	AudioEffect_update_local_params(node, 6, &param, 2);

	param = BassTrebGainTable[TrebGain];
	roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, node, 11, &param);
	AudioEffect_update_local_params(node, 11, &param, 2);

	gCtrlVars.AutoRefresh = node;//AudioCore.Audioeffect.effect_addr;
}
#endif

void AudioEffect_ReverbStep_Ajust(uint8_t ReverbStep)
{
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	if(AudioCore.Audioeffect.context_memory == NULL)
		return;

	uint16_t step;
	ReverbUnit *Reverbparam;
	EchoUnit *Echoparam;
	uint8_t Reverbnode = get_audioeffect_addr(REVERB);
	uint8_t Echonode = get_audioeffect_addr(ECHO);

	if(AudioEffect_effectAddr_check(Echonode))
	{
		Echoparam = (EchoUnit *)roboeffect_get_effect_parameter(AudioCore.Audioeffect.context_memory, Echonode, 0xff);
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

		roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, Echonode, 1, &Echoparam->attenuation);
		AudioEffect_update_local_params(Echonode, 1, &Echoparam->attenuation, 2);
		roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, Echonode, 2, &Echoparam->delay);
		AudioEffect_update_local_params(Echonode, 2, &Echoparam->delay, 2);
	}
	if(AudioEffect_effectAddr_check(Reverbnode))
	{
		Reverbparam = (ReverbUnit *)roboeffect_get_effect_parameter(AudioCore.Audioeffect.context_memory, Reverbnode, 0xff);
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

		roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, Reverbnode, 1, &Reverbparam->wet_scale);
		AudioEffect_update_local_params(Reverbnode, 1, &Reverbparam->wet_scale, 2);
		roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, Reverbnode, 3, &Reverbparam->roomsize_scale);
		AudioEffect_update_local_params(Reverbnode, 3, &Reverbparam->roomsize_scale, 2);
	}
	gCtrlVars.AutoRefresh = AutoRefresh_ALL_PARA;
#endif
}

void AudioEffect_ReverbPlateStep_Ajust(uint8_t ReverbStep)
{
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	if(AudioCore.Audioeffect.context_memory == NULL)
		return;

	uint16_t step;
	ReverbPlateUnit *ReverbPlateparam;
	EchoUnit *Echoparam;
	uint8_t ReverbPlatenode = get_audioeffect_addr(REVERBPLATE);
	uint8_t Echonode = get_audioeffect_addr(ECHO);

	if(AudioEffect_effectAddr_check(Echonode))
	{
		Echoparam = (EchoUnit *)roboeffect_get_effect_parameter(AudioCore.Audioeffect.context_memory, Echonode, 0xff);
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
		roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, Echonode, 1, &Echoparam->attenuation);
		AudioEffect_update_local_params(Echonode, 1, &Echoparam->attenuation, 2);
		roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, Echonode, 2, &Echoparam->delay);
		AudioEffect_update_local_params(Echonode, 2, &Echoparam->delay, 2);
	}
	if(AudioEffect_effectAddr_check(ReverbPlatenode))
	{
		ReverbPlateparam = (ReverbPlateUnit *)roboeffect_get_effect_parameter(AudioCore.Audioeffect.context_memory, ReverbPlatenode, 0xff);
		step = ReverbMaxParam.max_reverbplate_wetdrymix / MAX_MIC_REVB_STEP;
		if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
		{
			ReverbPlateparam->wetdrymix = ReverbMaxParam.max_reverbplate_wetdrymix;
		}
		else
		{
			ReverbPlateparam->wetdrymix = ReverbStep * step;
		}

		roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, ReverbPlatenode, 6, &ReverbPlateparam->wetdrymix);
		AudioEffect_update_local_params(ReverbPlatenode, 6, &ReverbPlateparam->wetdrymix, 2);
	}
	gCtrlVars.AutoRefresh = AutoRefresh_ALL_PARA;
#endif
}

uint16_t AudioEffect_SilenceDetector_Get(AUDIOEFFECT_EFFECT_TYPE type)
{
	if(AudioCore.Audioeffect.context_memory == NULL)
		return 0;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(type)))
		return 0;

	SilenceDetectorUnit *SDParam;
	SDParam = (SilenceDetectorUnit *)roboeffect_get_effect_parameter(AudioCore.Audioeffect.context_memory, get_audioeffect_addr(type), 0xff);
	return SDParam->level;
}

uint16_t AudioEffect_GainControl_Get(AUDIOEFFECT_EFFECT_TYPE type)
{
	if(AudioCore.Audioeffect.context_memory == NULL)
		return 0;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(type)))
		return 0;

	GainControlUnit *GainParam;
	GainParam = (GainControlUnit *)roboeffect_get_effect_parameter(AudioCore.Audioeffect.context_memory, get_audioeffect_addr(type), 0xff);
	return GainParam->gain;
}

void AudioEffect_GainControl_Set(AUDIOEFFECT_EFFECT_TYPE type, uint16_t gain)
{
	if(AudioCore.Audioeffect.context_memory == NULL)
		return;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(type)))
		return;

	int16_t param = gain;
	roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, get_audioeffect_addr(type), 1, &param);
	AudioEffect_update_local_params(get_audioeffect_addr(type), 1, &param, 2);

	gCtrlVars.AutoRefresh = get_audioeffect_addr(type);
}
void AudioEffect_GainMute_Set(AUDIOEFFECT_EFFECT_TYPE type, bool muteFlag)
{
	if(AudioCore.Audioeffect.context_memory == NULL)
		return;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(type)))
		return;

	int16_t param = muteFlag;
	roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, get_audioeffect_addr(type), 0, &param);
	AudioEffect_update_local_params(get_audioeffect_addr(type), 0, &param, 2);

//	gCtrlVars.AutoRefresh = node;
}

void AudioEffect_EQMode_Set(uint8_t EQMode)
{
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	if(AudioCore.Audioeffect.context_memory == NULL)
		return;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(MUSIC_EQ)))
		return;
//	AudioEffect_enable_effect(AudioCore.Audioeffect.context_memory, MUSIC_EQ_ADDR, 0);
    switch(EQMode)
	{
		case EQ_MODE_FLAT:
			roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 0xff, &Flat[0]);
			break;
		case EQ_MODE_CLASSIC:
			roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 0xff, &Classical[0]);
			break;
		case EQ_MODE_POP:
			roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 0xff, &Pop[0]);
			break;
		case EQ_MODE_ROCK:
			roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 0xff, &Rock[0]);
			break;
		case EQ_MODE_JAZZ:
			roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 0xff, &Jazz[0]);
			break;
		case EQ_MODE_VOCAL_BOOST:
			roboeffect_set_effect_parameter(AudioCore.Audioeffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 0xff, &Vocal_Booster[0]);
			break;
	}
    DBG("AudioEffect_EQMode_Set:%d\n", EQMode);
	roboeffect_enable_effect(AudioCore.Audioeffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 1);
#endif
}

void AudioEffect_SourceGain_Update(uint8_t Index)
{
	if(AudioCore.Audioeffect.context_memory == NULL)
		return;

	switch(Index)
	{
		case APP_SOURCE_NUM:
			AudioEffect_GainControl_Set(APP_SOURCE_GAIN, audioeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]]);
			break;
#ifdef CFG_FUNC_REMIND_SOUND_EN
		case REMIND_SOURCE_NUM:
			AudioEffect_GainControl_Set(REMIND_SOURCE_GAIN,	audioeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM]]);
			break;
#endif
		case MIC_SOURCE_NUM:
			AudioEffect_GainControl_Set(MIC_SOURCE_GAIN, audioeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
			break;

#ifdef CFG_FUNC_RECORDER_EN
		case PLAYBACK_SOURCE_NUM:
			AudioEffect_GainControl_Set(REC_SOURCE_GAIN, audioeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[PLAYBACK_SOURCE_NUM]]);

			break;
#endif
#ifdef CFG_FUNC_MIC_KARAOKE_EN
#ifdef CFG_RES_AUDIO_I2S_MIX_IN_EN
		case I2S_MIX_SOURCE_NUM:
			AudioEffect_GainControl_Set(I2S_MIX_SOURCE_GAIN, audioeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[I2S_MIX_SOURCE_NUM]]);
			break;
#endif
#ifdef CFG_FUNC_USB_AUDIO_MIX_MODE
			case USB_SOURCE_NUM:
				AudioEffect_GainControl_Set(USB_SOURCE_GAIN, audioeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[USB_SOURCE_NUM]]);
				break;
#endif
#ifdef CFG_FUNC_LINEIN_MIX_MODE
			case LINEIN_MIX_SOURCE_NUM:
				AudioEffect_GainControl_Set(LINEIN_MIX_SOURCE_GAIN,	audioeffectVolArr[mainAppCt.gSysVol.AudioSourceVol[LINEIN_MIX_SOURCE_NUM]]);
				break;
#endif
#endif
			default:
				break;
	}
}

void AudioEffect_SinkGain_Update(uint8_t Index)
{
	if(AudioCore.Audioeffect.context_memory == NULL)
		return;

	switch(Index)
	{
		case AUDIO_DAC0_SINK_NUM:
			AudioEffect_GainControl_Set(DAC0_SINK_GAIN,	audioeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]]);
			break;
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
		case AUDIO_APP_SINK_NUM:
			AudioEffect_GainControl_Set(APP_SINK_GAIN, audioeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_APP_SINK_NUM]]);
			break;
#endif

#ifdef CFG_FUNC_RECORDER_EN
		case AUDIO_RECORDER_SINK_NUM:
			AudioEffect_GainControl_Set(REC_SINK_GAIN, audioeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_RECORDER_SINK_NUM]]);
			break;
#endif
#ifdef CFG_RES_AUDIO_I2S_MIX_OUT_EN
			case AUDIO_I2S_MIX_OUT_SINK_NUM:
				AudioEffect_GainControl_Set(I2S_MIX_SINK_GAIN, audioeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_I2S_MIX_OUT_SINK_NUM]]);
			break;
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
		case AUDIO_STEREO_SINK_NUM:
			AudioEffect_GainControl_Set(STEREO_SINK_GAIN, audioeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_STEREO_SINK_NUM]]);
			break;
#endif
		default:
			break;
	}
}

void AudioEffect_update_local_params(uint8_t addr, uint8_t param_index, int16_t *param_input, uint8_t param_len)
{
	uint8_t *params = AudioCore.Audioeffect.user_effect_parameters + 5;
	uint16_t data_len = *(uint16_t *)AudioCore.Audioeffect.user_effect_parameters - 5;
	uint8_t len = 0;

//	DBG("input: 0x%x\n", *(uint16_t *)param_input);
	while(data_len)
	{
		if(*params == addr)
		{
			params += (param_index * 2 + 3);
//			DBG("before: 0x%x\n", *(uint16_t *)params);
//			*(uint16_t *)params = *(uint16_t *)param_input;
			memcpy((uint16_t *)params, (uint16_t *)param_input, param_len);
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

void AudioEffect_update_local_effect_status(uint8_t addr, uint8_t effect_enable)
{
	uint8_t *params = AudioCore.Audioeffect.user_effect_parameters + 5;
	uint16_t data_len = *(uint16_t *)AudioCore.Audioeffect.user_effect_parameters - 5;
	uint8_t len = 0;
	while(data_len)
	{
		if(*params == addr)
		{
			params += 2;
			*params = effect_enable;
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

void AudioEffect_update_local_block_params(uint8_t addr)
{
	uint8_t *params = AudioCore.Audioeffect.user_effect_parameters + 5;
	uint16_t data_len = *(uint16_t *)AudioCore.Audioeffect.user_effect_parameters - 5;
	uint8_t len = 0;
	const uint8_t *p = (uint8_t *)roboeffect_get_effect_parameter(AudioCore.Audioeffect.context_memory, addr, 0xFF);
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

uint8_t AudioEffect_effect_status_Get(AUDIOEFFECT_EFFECT_TYPE type)
{
	if(AudioCore.Audioeffect.context_memory == NULL)
		return 0;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(type)))
		return 0;

	return roboeffect_get_effect_status(AudioCore.Audioeffect.context_memory, get_audioeffect_addr(type));
}

void AudioEffect_effect_enable(AUDIOEFFECT_EFFECT_TYPE type, uint8_t enable)
{
	AudioCore.Audioeffect.effect_addr = get_audioeffect_addr(type);
	AudioCore.Audioeffect.effect_enable = enable;

	MessageContext msgSend;
	msgSend.msgId = MSG_EFFECTREINIT;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

uint8_t AudioCoreSourceToRoboeffect(int8_t source)
{
	AUDIOEFFECT_EFFECT_PARA_TABLE *param = GetCurEffectParaNode();

	switch (source)
	{
		case MIC_SOURCE_NUM:
			return param->audioeffect_source.mic_source;
		case APP_SOURCE_NUM:
			return param->audioeffect_source.app_source;
		case REMIND_SOURCE_NUM:
			return param->audioeffect_source.remind_source;
		case PLAYBACK_SOURCE_NUM:
			return param->audioeffect_source.rec_source;
		case I2S_MIX_SOURCE_NUM:
			return param->audioeffect_source.i2s_mix_source;
		case USB_SOURCE_NUM:
			return param->audioeffect_source.usb_source;
		case LINEIN_MIX_SOURCE_NUM:
			return param->audioeffect_source.linein_mix_source;
		default:
			break;// handle error
	}
	return AUDIOCORE_SOURCE_SINK_ERROR;
}

uint8_t AudioCoreSinkToRoboeffect(int8_t sink)
{
	AUDIOEFFECT_EFFECT_PARA_TABLE *param = GetCurEffectParaNode();

	switch (sink)
	{
		case AUDIO_DAC0_SINK_NUM:
			return param->audioeffect_sink.dac0_sink;
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
		case AUDIO_APP_SINK_NUM:
			return param->audioeffect_sink.app_sink;
#endif

#ifdef CFG_FUNC_RECORDER_EN
		case AUDIO_RECORDER_SINK_NUM:
			return param->audioeffect_sink.rec_sink;
#endif
#if defined(CFG_RES_AUDIO_I2SOUT_EN)
		case AUDIO_STEREO_SINK_NUM:
			return param->audioeffect_sink.stereo_sink;
#endif
#ifdef CFG_RES_AUDIO_I2S_MIX_OUT_EN
		case AUDIO_I2S_MIX_OUT_SINK_NUM:
			return param->audioeffect_sink.i2s_mix_sink;
#endif
#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
		case AUDIO_SPDIF_SINK_NUM:
			return param->audioeffect_sink.spdif_sink;
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
    return ((b2 << 8) | b1) + 2;
}

uint8_t get_audioeffect_addr(AUDIOEFFECT_EFFECT_TYPE effect_name)
{
	AUDIOEFFECT_EFFECT_PARA_TABLE *param = GetCurEffectParaNode();
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
		case REVERBPLATE:
			addr = param->effect_addr.REVERBPLATE_ADDR;
			break;
		case ECHO:
			addr = param->effect_addr.ECHO_ADDR;
			break;
		case VOCAL_CUT:
			addr = param->effect_addr.VOCAL_CUT_ADDR;
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

uint16_t get_audioeffectVolArr(uint8_t vol)
{
	return audioeffectVolArr[vol];
}

void AudioEffectParamSync(void)
{
#ifdef CFG_FUNC_MIC_TREB_BASS_EN
	AudioEffect_EQ_Ajust(MIC_EQ, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
#endif
#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
	AudioEffect_EQ_Ajust(MUSIC_EQ, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
#endif
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	AudioEffect_EQMode_Set(mainAppCt.EqMode);
#endif
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	AudioEffect_ReverbStep_Ajust(mainAppCt.ReverbStep);
#endif
}

bool AudioEffect_effectAddr_check(uint8_t node)
{
	if(node < 0x81 || node > (AudioCore.Audioeffect.user_effect_list->count + 0x80))
		return FALSE;
//	if(!roboeffect_get_effect_status(AudioCore.Audioeffect.context_memory, node))
//			return FALSE;
	return TRUE;
}
