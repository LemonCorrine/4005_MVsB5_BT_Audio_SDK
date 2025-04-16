#include <nds32_utils_math.h>
#include <string.h>
#include <math.h>
#include "roboeffect_api.h"
#include "user_defined_effect_api.h"
#include "user_effect_parameter.h"
#include "nn_denoise_api.h"
#include "main_task.h"
#include "bt_config.h"
#include "breakpoint.h"
#include "auto_gen_msg_process.h"

extern AUDIOEFFECT_SOURCE_SINK_NUM * get_user_effect_source_sink(void);
extern uint8_t AudioMicVolSync(void);
extern uint8_t AudioMusicVolSync(void);
extern uint8_t GetEffectControlIndex(AUDIOEFFECT_EFFECT_CONTROL type);

int16_t * AudioEffectGetAllParameter(AUDIOEFFECT_EFFECT_CONTROL effect)
{
	uint8_t addr;
	if(AudioEffect.context_memory == NULL)
		return NULL;
	addr = GetEffectControlIndex(effect);
	if(addr == 0)
		return NULL;

	return (int16_t *)roboeffect_get_effect_parameter(AudioEffect.context_memory, addr, 0xff);
}

void AudioEffect_GetAudioEffectValue(void)
{
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	param_echo *Echoparam = (param_echo *)AudioEffectGetAllParameter(ECHO_PARAM);
	if(Echoparam)
	{
		APP_DBG("echo fc:0x%x\n", Echoparam->cutoff_frequency);
		APP_DBG("echo delay:0x%x\n", Echoparam->delay);
		APP_DBG("echo dry:0x%x\n", Echoparam->dry);
		APP_DBG("echo attenuation:0x%x\n", Echoparam->attenuation);
		APP_DBG("echo max_delay:0x%x\n", Echoparam->max_delay);
		APP_DBG("echo quality_mode:0x%x\n", Echoparam->high_quality_enable);
		APP_DBG("echo wet:0x%x\n", Echoparam->wet);
	}
#endif
}

void AudioEffect_SourceGain_Update(uint8_t source)
{
	if(AudioEffect.context_memory == NULL)
		return;

	switch(source)
	{
		case APP_SOURCE_NUM:
			gCtrlVars.AutoRefresh = AudioMusicVolSync();
			break;
#ifdef CFG_FUNC_REMIND_SOUND_EN
		case REMIND_SOURCE_NUM:
			gCtrlVars.AutoRefresh = AudioRemindVolSync();
			break;
#endif
		case MIC_SOURCE_NUM:
			gCtrlVars.AutoRefresh = AudioMicVolSync();
			break;

#ifdef CFG_FUNC_RECORDER_EN
		case PLAYBACK_SOURCE_NUM:

			break;
#endif
#ifdef CFG_FUNC_MIC_KARAOKE_EN
#ifdef CFG_RES_AUDIO_I2S_MIX_IN_EN
		case I2S_MIX_SOURCE_NUM:
			break;
#endif
#ifdef CFG_FUNC_USB_AUDIO_MIX_MODE
			case USB_SOURCE_NUM:
				break;
#endif
#ifdef CFG_FUNC_LINEIN_MIX_MODE
			case LINEIN_MIX_SOURCE_NUM:
				break;
#endif
#endif
			default:
				break;
	}
#ifdef CFG_FUNC_BREAKPOINT_EN
	BackupInfoUpdata(BACKUP_SYS_INFO);
#endif
}

uint8_t AudioEffect_effect_status_Get(uint8_t effect_addr)
{
	if(AudioEffect.context_memory == NULL)
		return 0;
	if(!AudioEffect_effectAddr_check(effect_addr))
		return 0;

	return roboeffect_get_effect_status(AudioEffect.context_memory, effect_addr);
}

void AudioEffect_effect_enable(uint8_t effect_addr, uint8_t enable)
{
	AudioEffect.effect_addr = effect_addr;
	AudioEffect.effect_enable = enable;

	MessageContext msgSend;
	msgSend.msgId = MSG_EFFECTREINIT;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

uint8_t AudioCoreSourceToRoboeffect(int8_t source)
{
	AUDIOEFFECT_SOURCE_SINK_NUM *param = get_user_effect_source_sink();

	switch (source)
	{
		case MIC_SOURCE_NUM:
			return param->mic_source;
		case APP_SOURCE_NUM:
			return param->app_source;
		case REMIND_SOURCE_NUM:
			return param->remind_source;
		case PLAYBACK_SOURCE_NUM:
			return param->rec_source;
		case I2S_MIX_SOURCE_NUM:
			return param->i2s_mix_source;
		case I2S_MIX2_SOURCE_NUM:
			return param->i2s_mix2_source;
		case USB_SOURCE_NUM:
			return param->usb_source;
		case LINEIN_MIX_SOURCE_NUM:
			return param->linein_mix_source;
		default:
			break;// handle error
	}
	return AUDIOCORE_SOURCE_SINK_ERROR;
}

uint8_t AudioCoreSinkToRoboeffect(int8_t sink)
{
	AUDIOEFFECT_SOURCE_SINK_NUM *param = get_user_effect_source_sink();

	switch (sink)
	{
		case AUDIO_DAC0_SINK_NUM:
			return param->dac0_sink;
#if	(defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT)) || defined(CFG_APP_USB_AUDIO_MODE_EN)
		case AUDIO_APP_SINK_NUM:
			return param->app_sink;
#endif

#ifdef CFG_FUNC_RECORDER_EN
		case AUDIO_RECORDER_SINK_NUM:
			return param->rec_sink;
#endif
#if defined(CFG_RES_AUDIO_I2SOUT_EN) || defined(CFG_RES_AUDIO_I2S_MIX2_OUT_EN)
		case AUDIO_STEREO_SINK_NUM:
			return param->stereo_sink;
#endif
#ifdef CFG_RES_AUDIO_I2S_MIX_OUT_EN
		case AUDIO_I2S_MIX_OUT_SINK_NUM:
			return param->i2s_mix_sink;
#endif
#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
		case AUDIO_SPDIF_SINK_NUM:
			return param->spdif_sink;
#endif
		default:
			// handle error
			break;
	}
	return AUDIOCORE_SOURCE_SINK_ERROR;
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
//			DBG("addr:0x%x,param:0x%x, len:0x%x\n", addr, *(uint16_t *)params, len);
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

uint16_t get_user_effect_parameters_len(uint8_t *user_effect_parameters)
{
	uint8_t b1 = user_effect_parameters[0];
	uint8_t b2 = user_effect_parameters[1];
    return ((b2 << 8) | b1) + 2;
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

#ifdef CFG_EFFECT_PARAM_UPDATA_BY_ACPWORKBENCH
#define CFG_FLASH_SECTOR_SIZE		(4096)//4KB
bool AudioEffect_FlashWrite(uint32_t Addr, uint8_t *Buffer, uint32_t Length)
{
	static uint8_t EffectParamFlahBuf[CFG_FLASH_SECTOR_SIZE] ={0};
    uint32_t sectorIndex = (Addr - get_effect_data_addr()) / CFG_FLASH_SECTOR_SIZE;
    uint32_t spaceLen = get_effect_data_addr() + (sectorIndex + 1) * CFG_FLASH_SECTOR_SIZE - Addr;
    uint32_t writeLen = spaceLen >= Length ? Length : spaceLen;
    uint32_t writeOffset = Addr - get_effect_data_addr() - sectorIndex * CFG_FLASH_SECTOR_SIZE;
    if (SpiFlashRead(get_effect_data_addr() + sectorIndex * CFG_FLASH_SECTOR_SIZE, EffectParamFlahBuf, CFG_FLASH_SECTOR_SIZE, 1) == FLASH_NONE_ERR)
    {
        SpiFlashErase(SECTOR_ERASE, (get_effect_data_addr() + sectorIndex * CFG_FLASH_SECTOR_SIZE) / 4096, 1);
    }
    memcpy(&EffectParamFlahBuf[writeOffset], Buffer, writeLen);
    if (SpiFlashWrite(get_effect_data_addr() + sectorIndex * CFG_FLASH_SECTOR_SIZE, EffectParamFlahBuf, CFG_FLASH_SECTOR_SIZE, 1) != FLASH_NONE_ERR)
    {
        APP_DBG("AudioEffect_FlashWrite ERROR!\n");
    	return FALSE;
    }
    if (writeLen < Length)
	{
		AudioEffect_FlashWrite(Addr + writeLen, &Buffer[writeLen], Length - writeLen);
	}
    return TRUE;
}
#endif

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
    SpiFlashWrite(FlashAddr + (mainAppCt.EffectMode - 1) * 4, (uint8_t*)&effectHwCfgOffset, 2, 1);
    SpiFlashWrite(FlashAddr + (mainAppCt.EffectMode - 1) * 4 + 2, (uint8_t*)&effectParamOffset, 2, 1);
	if (AudioEffect_FlashWrite(FlashAddr + effectHwCfgOffset, (uint8_t*)&gCtrlVars.HwCt, sizeof(gCtrlVars.HwCt))
		&& AudioEffect_FlashWrite(FlashAddr + effectParamOffset, (uint8_t*)AudioEffect.user_effect_parameters,
				get_user_effect_parameters_len(AudioEffect.user_effect_parameters)))
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

bool AudioEffect_CheckFlashEffectParam(void)
{
	uint16_t flashParam[EFFECT_MODE_COUNT * 2];
	AUDIOEFFECT_EFFECT_PARA *para;
	uint8_t i, index;

	if ((SpiFlashRead(get_effect_data_addr(), (uint8_t*)flashParam, EFFECT_MODE_COUNT * 4 ,1) == FLASH_NONE_ERR))
	{
		for(i = 0; i < EFFECT_MODE_COUNT;)
		{
//			DBG("### %d - Hwcfg:%d, param:%d\n", i, flashParam[i], flashParam[i + 1]);
			para = get_user_effect_parameters(i);
			if((flashParam[i] != 0xffff) && (flashParam[i + 1] != 0xffff)
				&& (flashParam[i] - flashParam[i + 1] != get_user_effect_parameters_len(para->user_effect_parameters)))
			{
//				DBG("flash read effect_mode(%d)EffectParam err %d\n", i, get_user_effect_parameters_len(para->user_effect_parameters));
				DBG("EffectParam in flash changed, erase all\n");
				for(index = 0; index < ((1024 * CFG_EFFECT_PARAM_IN_FLASH_SIZE) / CFG_FLASH_SECTOR_SIZE); index++)
				{
					SpiFlashErase(SECTOR_ERASE, (get_effect_data_addr() + index * CFG_FLASH_SECTOR_SIZE) /4096 , 1);
				}
				return FALSE;
			}
			i += 2;
		}
	}
	return TRUE;
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


