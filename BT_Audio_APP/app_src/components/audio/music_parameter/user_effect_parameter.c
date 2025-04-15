#include <nds32_utils_math.h>
#include <string.h>
#include <math.h>
#include "roboeffect_api.h"
#include "user_defined_effect_api.h"
#include "user_effect_parameter.h"
#include "nn_denoise_api.h"
#include "main_task.h"
#include "bt_config.h"

extern const AUDIOEFFECT_EFFECT_PARA_TABLE bypass_mode;
extern const AUDIOEFFECT_EFFECT_PARA_TABLE karaoke_mode;
extern const AUDIOEFFECT_EFFECT_PARA_TABLE mic_mode;
extern const AUDIOEFFECT_EFFECT_PARA_TABLE music_mode;
extern const AUDIOEFFECT_EFFECT_PARA_TABLE micusbAI_mode;
extern const AUDIOEFFECT_EFFECT_PARA_TABLE hfp_mode;

static const AUDIOEFFECT_EFFECT_PARA_TABLE *effect_para_table[] =
{
#ifdef CFG_FUNC_EFFECT_BYPASS_EN
	&bypass_mode,
#else
	#ifdef CFG_FUNC_MIC_KARAOKE_EN
		&karaoke_mode,
	#else
		&mic_mode,
		&music_mode,
	#endif
#endif
#if defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT)
	&hfp_mode,
#endif
#ifdef CFG_AI_DENOISE_EN
	&micusbAI_mode,
#endif
	NULL,
};

static const uint16_t audioeffectVolArr[CFG_PARA_MAX_VOLUME_NUM + 1] =
{
#if CFG_PARA_MAX_VOLUME_NUM == 32
	-7200/*-72db*/,
	-6300/*-63db*/,	-5600/*-56db*/,	-4900/*-49db*/,	-4400/*-44db*/,	-4000/*-40db*/,	-3600/*-36db*/,	-3200/*-32db*/,	-2900/*-29db*/,
	-2600/*-26db*/,	-2400/*-24db*/,	-2200/*-22db*/,	-2000/*-20db*/,	-1900/*-19db*/,	-1800/*-18db*/,	-1700/*-17db*/,	-1600/*-16db*/,
	-1500/*-15db*/,	-1400/*-14db*/,	-1300/*-13db*/,	-1200/*-12db*/,	-1100/*-11db*/,	-1000/*-10db*/,	-900/*-9db*/,	-800/*-8db*/,
	-700/*-7db*/,	-600/*-6db*/,	-500/*-5db*/,	-400/*-4db*/,	-300/*-3db*/,	-200/*-2db*/,	-100/*-1db*/,	0/*0db*/
#endif
#if CFG_PARA_MAX_VOLUME_NUM == 16
	-7200/*-72db*/,
	-5600/*-56db*/,	-4000/*-40db*/,	-2900/*-29db*/,	-2400/*-24db*/,	-2000/*-20db*/,	-1700/*-17db*/,	-1500/*-15db*/,	-1300/*-13db*/,
	-1100/*-11db*/,	-900/*-9db*/,	-700/*-7db*/,	-500/*-5db*/,	-300/*-3db*/,	-200/*-2db*/,	-100/*-1db*/,	0/*0db*/
#endif
};

#ifdef CFG_FUNC_MIC_KARAOKE_EN
static ReverbMaxUnit ReverbMaxParam = {0 , 0 , 0 ,0};
#endif

#ifdef CFG_EFFECT_PARAM_UPDATA_BY_ACPWORKBENCH
uint8_t EffectParamFlahBuf[1024 * CFG_EFFECT_PARAM_IN_FLASH_SIZE] ={0};//定义16K buf，读取存储参数
#endif

AUDIOEFFECT_EFFECT_PARA_TABLE * GetCurEffectParaMode(void)
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
	AUDIOEFFECT_EFFECT_PARA_TABLE *param = GetCurEffectParaMode();

	uint8_t index = mode - param->effect_id;

	if(index > param->effect_id_count)
		index = 0;

	return param->audioeffect_para + index;
}

void AudioEffect_GetAudioEffectMaxValue(void)
{
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	if(AudioEffect.context_memory == NULL)
		return;

	ReverbUnit *Reverbparam;
	EchoUnit *Echoparam;
	ReverbPlateUnit *ReverbPlateparam;

	if(AudioEffect_effectAddr_check(get_audioeffect_addr(REVERB)))
	{
		Reverbparam = (ReverbUnit *)roboeffect_get_effect_parameter(AudioEffect.context_memory,
											get_audioeffect_addr(REVERB), 0xff);
		ReverbMaxParam.max_reverb_wet_scale  = Reverbparam->wet_scale;
		ReverbMaxParam.max_reverb_roomsize   = Reverbparam->roomsize_scale;
	}
	if(AudioEffect_effectAddr_check(get_audioeffect_addr(ECHO)))
	{
		Echoparam = (EchoUnit *)roboeffect_get_effect_parameter(AudioEffect.context_memory,
											get_audioeffect_addr(ECHO), 0xff);
		ReverbMaxParam.max_echo_delay        = Echoparam->delay;
		ReverbMaxParam.max_echo_depth        = Echoparam->attenuation;
	}
	if(AudioEffect_effectAddr_check(get_audioeffect_addr(REVERBPLATE)))
	{
		ReverbPlateparam = (ReverbPlateUnit *)roboeffect_get_effect_parameter(AudioEffect.context_memory,
											get_audioeffect_addr(REVERBPLATE), 0xff);
		ReverbMaxParam.max_reverbplate_wetdrymix   = ReverbPlateparam->wetdrymix;
	}
#endif
}

#if defined(CFG_FUNC_MIC_TREB_BASS_EN) || defined(CFG_FUNC_MUSIC_TREB_BASS_EN)
void AudioEffect_EQ_Ajust(AUDIOEFFECT_EFFECT_NUM effect,uint8_t BassGain, uint8_t TrebGain)
{

	if(AudioEffect.context_memory == NULL)
		return;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(effect)))
		return;

	int16_t param = 0;
	uint8_t addr = get_audioeffect_addr(effect);

	param = BassTrebGainTable[BassGain];
	roboeffect_set_effect_parameter(AudioEffect.context_memory, addr, 6, &param);
	AudioEffect_update_local_params(addr, 6, &param, 2);

	param = BassTrebGainTable[TrebGain];
	roboeffect_set_effect_parameter(AudioEffect.context_memory, addr, 11, &param);
	AudioEffect_update_local_params(addr, 11, &param, 2);

}
#endif

void AudioEffect_ReverbStep_Ajust(uint8_t ReverbStep)
{
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	if(AudioEffect.context_memory == NULL)
		return;

	uint16_t step = 0;
	int16_t param  = 0;
	uint8_t Reverbaddr = get_audioeffect_addr(REVERB);
	uint8_t Echoaddr = get_audioeffect_addr(ECHO);

	if(AudioEffect_effectAddr_check(Echoaddr))
	{
		step = ReverbMaxParam.max_echo_delay * 100  / MAX_MIC_REVB_STEP;
		if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
		{
			param = ReverbMaxParam.max_echo_delay ;
		}
		else
		{
			param = ReverbStep * step / 100;
		}
		roboeffect_set_effect_parameter(AudioEffect.context_memory, Echoaddr, 2, &param);
		AudioEffect_update_local_params(Echoaddr, 2, &param, 2);

		step = ReverbMaxParam.max_echo_depth * 100 / MAX_MIC_REVB_STEP;
		if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
		{
			param = ReverbMaxParam.max_echo_depth;
		}
		else
		{
			param = ReverbStep * step / 100;
		}

		roboeffect_set_effect_parameter(AudioEffect.context_memory, Echoaddr, 1, &param);
		AudioEffect_update_local_params(Echoaddr, 1, &param, 2);
	}
	if(AudioEffect_effectAddr_check(Reverbaddr))
	{
		step = ReverbMaxParam.max_reverb_wet_scale * 100 / MAX_MIC_REVB_STEP;
		if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
		{
			param = ReverbMaxParam.max_reverb_wet_scale;
		}
		else
		{
			param = ReverbStep * step / 100;
		}
		roboeffect_set_effect_parameter(AudioEffect.context_memory, Reverbaddr, 1, &param);
		AudioEffect_update_local_params(Reverbaddr, 1, &param, 2);

		step = ReverbMaxParam.max_reverb_roomsize * 100 / MAX_MIC_REVB_STEP;
		if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
		{
			param = ReverbMaxParam.max_reverb_roomsize;
		}
		else
		{
			param = ReverbStep * step / 100;
		}
		roboeffect_set_effect_parameter(AudioEffect.context_memory, Reverbaddr, 3, &param);
		AudioEffect_update_local_params(Reverbaddr, 3, &param, 2);
	}
#endif
}

void AudioEffect_ReverbPlateStep_Ajust(uint8_t ReverbStep)
{
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	if(AudioEffect.context_memory == NULL)
		return;

	uint16_t step = 0;
	int16_t param  = 0;
	uint8_t ReverbPlateaddr = get_audioeffect_addr(REVERBPLATE);
	uint8_t Echoaddr = get_audioeffect_addr(ECHO);

	if(AudioEffect_effectAddr_check(Echoaddr))
	{
		step = ReverbMaxParam.max_echo_delay * 100  / MAX_MIC_REVB_STEP;
		if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
		{
			param = ReverbMaxParam.max_echo_delay ;
		}
		else
		{
			param = ReverbStep * step / 100;
		}
		roboeffect_set_effect_parameter(AudioEffect.context_memory, Echoaddr, 2, &param);
		AudioEffect_update_local_params(Echoaddr, 2, &param, 2);

		step = ReverbMaxParam.max_echo_depth * 100 / MAX_MIC_REVB_STEP;
		if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
		{
			param = ReverbMaxParam.max_echo_depth;
		}
		else
		{
			param = ReverbStep * step / 100;
		}
		roboeffect_set_effect_parameter(AudioEffect.context_memory, Echoaddr, 1, &param);
		AudioEffect_update_local_params(Echoaddr, 1, &param, 2);
	}
	if(AudioEffect_effectAddr_check(ReverbPlateaddr))
	{
		step = ReverbMaxParam.max_reverbplate_wetdrymix * 100 / MAX_MIC_REVB_STEP;
		if(ReverbStep >= (MAX_MIC_REVB_STEP-1))
		{
			param = ReverbMaxParam.max_reverbplate_wetdrymix;
		}
		else
		{
			param = ReverbStep * step / 100;
		}

		roboeffect_set_effect_parameter(AudioEffect.context_memory, ReverbPlateaddr, 6, &param);
		AudioEffect_update_local_params(ReverbPlateaddr, 6, &param, 2);
	}
#endif
}

int32_t AudioEffect_SilenceDetector_Get(AUDIOEFFECT_EFFECT_NUM effect)
{
	int32_t level;
	if(AudioEffect.context_memory == NULL)
		return -1;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(effect)))
		return -2;

	SilenceDetectorUnit *SDParam;
	SDParam = (SilenceDetectorUnit *)roboeffect_get_effect_parameter(AudioEffect.context_memory, get_audioeffect_addr(effect), 0xff);
	level = SDParam->level;
	return level;
}

uint16_t AudioEffect_GainControl_Get(AUDIOEFFECT_EFFECT_NUM effect)
{
	if(AudioEffect.context_memory == NULL)
		return 0;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(effect)))
		return 0;

	GainControlUnit *GainParam;
	GainParam = (GainControlUnit *)roboeffect_get_effect_parameter(AudioEffect.context_memory, get_audioeffect_addr(effect), 0xff);
	return GainParam->gain;
}

void AudioEffect_GainControl_Set(AUDIOEFFECT_EFFECT_NUM effect, uint16_t gain)
{
	if(AudioEffect.context_memory == NULL)
		return;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(effect)))
		return;

	int16_t param = gain;
	roboeffect_set_effect_parameter(AudioEffect.context_memory, get_audioeffect_addr(effect), 1, &param);
	AudioEffect_update_local_params(get_audioeffect_addr(effect), 1, &param, 2);
}
void AudioEffect_GainMute_Set(AUDIOEFFECT_EFFECT_NUM effect, bool muteFlag)
{
	if(AudioEffect.context_memory == NULL)
		return;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(effect)))
		return;

	int16_t param = muteFlag;
	roboeffect_set_effect_parameter(AudioEffect.context_memory, get_audioeffect_addr(effect), 0, &param);
	AudioEffect_update_local_params(get_audioeffect_addr(effect), 0, &param, 2);
}

void AudioEffect_EQMode_Set(uint8_t EQMode)
{
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	if(AudioEffect.context_memory == NULL)
		return;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(MUSIC_EQ)))
		return;

    switch(EQMode)
	{
		case EQ_MODE_FLAT:
			roboeffect_set_effect_parameter(AudioEffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 0xff, (int16_t *)&Flat[0]);
			break;
		case EQ_MODE_CLASSIC:
			roboeffect_set_effect_parameter(AudioEffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 0xff, (int16_t *)&Classical[0]);
			break;
		case EQ_MODE_POP:
			roboeffect_set_effect_parameter(AudioEffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 0xff, (int16_t *)&Pop[0]);
			break;
		case EQ_MODE_ROCK:
			roboeffect_set_effect_parameter(AudioEffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 0xff, (int16_t *)&Rock[0]);
			break;
		case EQ_MODE_JAZZ:
			roboeffect_set_effect_parameter(AudioEffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 0xff, (int16_t *)&Jazz[0]);
			break;
		case EQ_MODE_VOCAL_BOOST:
			roboeffect_set_effect_parameter(AudioEffect.context_memory, get_audioeffect_addr(MUSIC_EQ), 0xff, (int16_t *)&Vocal_Booster[0]);
			break;
	}
    DBG("AudioEffect_EQMode_Set:%d\n", EQMode);
#endif
}

void AudioEffect_SourceGain_Update(uint8_t source)
{
	if(AudioEffect.context_memory == NULL)
		return;

	switch(source)
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

void AudioEffect_SinkGain_Update(uint8_t sink)
{
	if(AudioEffect.context_memory == NULL)
		return;

	switch(sink)
	{
		case AUDIO_DAC0_SINK_NUM:
			AudioEffect_GainControl_Set(DAC0_SINK_GAIN,	audioeffectVolArr[mainAppCt.gSysVol.AudioSinkVol[AUDIO_DAC0_SINK_NUM]]);
			break;
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
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
	EffectValidParamUnit unit = AudioEffect_GetUserEffectValidParam(AudioEffect.user_effect_parameters);
	uint8_t *params = unit.params_first_address;
	uint16_t data_len = unit.data_len;
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
	EffectValidParamUnit unit = AudioEffect_GetUserEffectValidParam(AudioEffect.user_effect_parameters);
	uint8_t *params = unit.params_first_address;
	uint16_t data_len = unit.data_len;
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
	EffectValidParamUnit unit = AudioEffect_GetUserEffectValidParam(AudioEffect.user_effect_parameters);
	uint8_t *params = unit.params_first_address;
	uint16_t data_len = unit.data_len;
	uint8_t len = 0;
	const uint8_t *p = (uint8_t *)roboeffect_get_effect_parameter(AudioEffect.context_memory, addr, 0xFF);
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

uint8_t AudioEffect_effect_status_Get(AUDIOEFFECT_EFFECT_NUM effect)
{
	if(AudioEffect.context_memory == NULL)
		return 0;
	if(!AudioEffect_effectAddr_check(get_audioeffect_addr(effect)))
		return 0;

	return roboeffect_get_effect_status(AudioEffect.context_memory, get_audioeffect_addr(effect));
}

void AudioEffect_effect_enable(AUDIOEFFECT_EFFECT_NUM effect, uint8_t enable)
{
	AudioEffect.effect_addr = get_audioeffect_addr(effect);
	AudioEffect.effect_enable = enable;

	MessageContext msgSend;
	msgSend.msgId = MSG_EFFECTREINIT;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

uint8_t AudioCoreSourceToRoboeffect(int8_t source)
{
	AUDIOEFFECT_EFFECT_PARA_TABLE *param = GetCurEffectParaMode();

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
	AUDIOEFFECT_EFFECT_PARA_TABLE *param = GetCurEffectParaMode();

	switch (sink)
	{
		case AUDIO_DAC0_SINK_NUM:
			return param->audioeffect_sink.dac0_sink;
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
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

uint8_t get_audioeffect_addr(AUDIOEFFECT_EFFECT_NUM effect_name)
{
	AUDIOEFFECT_EFFECT_PARA_TABLE *param = GetCurEffectParaMode();
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
		case SILENCE_DETECTOR_MUSIC:
			addr = param->effect_addr.SILENCE_DETECTOR_MUSIC_ADDR;
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
	AudioEffect_GetAudioEffectMaxValue();
	AudioEffect_ReverbStep_Ajust(mainAppCt.ReverbStep);
#endif
}

bool AudioEffect_effectAddr_check(uint8_t addr)
{
	if(addr < 0x81 || addr > (AudioEffect.cur_effect_para->user_effect_list->count + 0x80))
		return FALSE;
//	if(!roboeffect_get_effect_status(AudioEffect.context_memory, addr))
//			return FALSE;
	return TRUE;
}

#ifdef CFG_EFFECT_PARAM_IN_FLASH_EN
uint16_t get_EffectParamFlash_WriteAddr(void)
{
	uint16_t offset = 1024 * CFG_EFFECT_PARAM_IN_FLASH_SIZE;
	uint16_t flashCnt = 0;
	for(uint8_t i = 0; i < ((EFFECT_MODE_COUNT - 1) * 2); i++)
	{
		SpiFlashRead(get_effect_data_addr() + i * 2, (uint8_t*)&flashCnt, 2, 1);
//		DBG("flashCnt = %d\n", flashCnt);
		offset = (flashCnt < offset) ? flashCnt : offset;
	}
	return offset;
}

void EffectParamFlashUpdata(void)
{
//	SPI_FLASH_ERR_CODE ret = 0;
	int32_t FlashAddr = get_effect_data_addr();
	uint16_t FlashWriteOffset = 0;
	uint16_t effectHwCfgOffset = 0;
	uint16_t effectParamOffset = 0;

	if (((SpiFlashRead(get_effect_data_addr() + ((mainAppCt.EffectMode - 1) * 4), (uint8_t*)&effectHwCfgOffset, 2 ,1)  == FLASH_NONE_ERR)
			&& (effectHwCfgOffset != 0xffff))
		&& ((SpiFlashRead(get_effect_data_addr() + ((mainAppCt.EffectMode - 1) * 4) + 2, (uint8_t*)&effectParamOffset, 2 ,1)  == FLASH_NONE_ERR)
				&& (effectParamOffset != 0xffff)))
	{
		DBG("EffectData FlashAdd = %lx\n", FlashAddr);
		DBG("effectHwCfgOffset = %d\n", effectHwCfgOffset);
		DBG("effectParamOffset = %d\n", effectParamOffset);
	}
	else
	{
		FlashWriteOffset = get_EffectParamFlash_WriteAddr();
		if (FlashWriteOffset < (((EFFECT_MODE_COUNT - 1) * 4) + sizeof(gCtrlVars.HwCt)
				+ get_user_effect_parameters_len(AudioEffect.user_effect_parameters)))
		{
			APP_DBG("Flash space is not enough!!!\n");
			return;
		}

		effectHwCfgOffset = FlashWriteOffset - sizeof(gCtrlVars.HwCt);
		effectParamOffset = effectHwCfgOffset - get_user_effect_parameters_len(AudioEffect.user_effect_parameters);

		DBG("EffectData FlashAdd = %lx\n", FlashAddr);
		DBG("FlashWriteOffset = %d\n", FlashWriteOffset);
		DBG("effectHwCfgOffset = %d\n", effectHwCfgOffset);
		DBG("effectParamOffset = %d\n", effectParamOffset);
	}

	//write data
	if(SpiFlashRead(FlashAddr, EffectParamFlahBuf, 1024 * CFG_EFFECT_PARAM_IN_FLASH_SIZE, 1) == FLASH_NONE_ERR)
	{
		for(uint8_t i = 0; i < CFG_EFFECT_PARAM_IN_FLASH_SIZE / 4; i ++)
		{
			SpiFlashErase(SECTOR_ERASE, (FlashAddr + 4096 * i) /4096 , 1);
		}

		if (effectHwCfgOffset - effectParamOffset == get_user_effect_parameters_len(AudioEffect.user_effect_parameters))
		{
			memcpy(&EffectParamFlahBuf[effectHwCfgOffset], (uint8_t*)&gCtrlVars.HwCt, sizeof(gCtrlVars.HwCt));
			memcpy(&EffectParamFlahBuf[effectParamOffset], (uint8_t*)AudioEffect.user_effect_parameters,
									get_user_effect_parameters_len(AudioEffect.user_effect_parameters));
		}
		else
		{
			APP_DBG("Please reburning flash image to erase flash effect param!!!\n");
			return;
		}

		memcpy(&EffectParamFlahBuf[(mainAppCt.EffectMode - 1) * 4], (uint8_t*)&effectHwCfgOffset, 2);
		memcpy(&EffectParamFlahBuf[((mainAppCt.EffectMode - 1) * 4) + 2], (uint8_t*)&effectParamOffset, 2);
	}
	else
	{
		APP_DBG("EffectParam read Flash Error!\n");
		return;
	}

	if (SpiFlashWrite(FlashAddr, EffectParamFlahBuf, 1024 * CFG_EFFECT_PARAM_IN_FLASH_SIZE, 1) == FLASH_NONE_ERR)
	{
		APP_DBG("EffectParamFlashUpdata ok!\n");
	}
	else
	{
		APP_DBG("EffectParamFlashUpdata Error!\n");
	}
}

bool AudioEffect_GetFlashHwCfg(uint8_t effectMode, HardwareConfigContext *hw_ct)
{
	uint16_t offset = 0;
	if (SpiFlashRead(get_effect_data_addr() + ((mainAppCt.EffectMode - 1) * 4), (uint8_t*)&offset, 2 ,1) == FLASH_NONE_ERR)
	{
		if ((offset != 0xffff) && (SpiFlashRead(get_effect_data_addr() + offset, (uint8_t*)hw_ct, sizeof(gCtrlVars.HwCt) ,1) == FLASH_NONE_ERR))
		{
			return TRUE;
		}
	}
	DBG("flash read HwCt err\n");
	return FALSE;
}

bool AudioEffect_GetFlashEffectParam(uint8_t effectMode,  uint8_t *effect_param)
{
	uint16_t effectHwCfgOffset = 0;
	uint16_t effectParamOffset = 0;
	if ((SpiFlashRead(get_effect_data_addr() + ((mainAppCt.EffectMode - 1) * 4), (uint8_t*)&effectHwCfgOffset, 2 ,1) == FLASH_NONE_ERR)
			&& (SpiFlashRead(get_effect_data_addr() + ((mainAppCt.EffectMode - 1) * 4) + 2, (uint8_t*)&effectParamOffset, 2 ,1) == FLASH_NONE_ERR)
			&& (effectHwCfgOffset - effectParamOffset == get_user_effect_parameters_len(effect_param)))
	{
		if (SpiFlashRead(get_effect_data_addr() + effectParamOffset, (uint8_t*)effect_param, get_user_effect_parameters_len(effect_param) ,1) == FLASH_NONE_ERR)
		{
			return TRUE;
		}
	}
	DBG("flash read EffectParam err\n");
	return FALSE;
}
#endif

//total data length  	---- 2 Bytes
//Effect Version		---- 3 Bytes
//Roboeffect Version  	---- 3 Bytes
//ACPWorkbench V3.8.15以后版本导出的参数增加了3字节的Roboeffect Version + 3rd part data
//使用的时候注意参数的版本，修改对应的偏移
EffectValidParamUnit AudioEffect_GetUserEffectValidParam(uint8_t *effect_param)
{
	EffectValidParamUnit unit;
	uint16_t third_data_len = *(uint16_t *)(effect_param + 8);

	unit.params_first_address = effect_param + 8 + third_data_len + 2;
	unit.data_len = *(uint16_t *)effect_param - 8 - third_data_len - 2;

	return unit;
}


