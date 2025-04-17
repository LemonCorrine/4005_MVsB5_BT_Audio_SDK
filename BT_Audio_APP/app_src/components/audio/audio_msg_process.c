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
#include "audio_vol.h"

extern uint8_t GetEffectControlIndex(AUDIOEFFECT_EFFECT_CONTROL type);

extern int16_t EqMode_data_0[];
extern int16_t EqMode_data_1[];
extern int16_t EqMode_data_2[];
extern int16_t EqMode_data_3[];
extern int16_t EqMode_data_4[];
extern int16_t EqMode_data_5[];

extern uint8_t EffectModeMsgProcess(uint16_t Msg);

int16_t EqMode;
int16_t MicVolume = CFG_PARA_SYS_VOLUME_DEFAULT;
int16_t MusicVolume = CFG_PARA_SYS_VOLUME_DEFAULT;

uint8_t AudioEqSync(void)
{
	uint8_t refresh_addr = GetEffectControlIndex(EQ_MODE_ADJUST);

	if(refresh_addr)
	{
		if(EqMode == 0)
			roboeffect_set_effect_parameter(AudioEffect.context_memory, refresh_addr, 255, EqMode_data_0);
		else if(EqMode == 1)
			roboeffect_set_effect_parameter(AudioEffect.context_memory, refresh_addr, 255, EqMode_data_1);
		else if(EqMode == 2)
			roboeffect_set_effect_parameter(AudioEffect.context_memory, refresh_addr, 255, EqMode_data_2);
		else if(EqMode == 3)
			roboeffect_set_effect_parameter(AudioEffect.context_memory, refresh_addr, 255, EqMode_data_3);
		else if(EqMode == 4)
			roboeffect_set_effect_parameter(AudioEffect.context_memory, refresh_addr, 255, EqMode_data_4);
		else if(EqMode == 5)
			roboeffect_set_effect_parameter(AudioEffect.context_memory, refresh_addr, 255, EqMode_data_5);
		else
			refresh_addr = 0;
	}

	return refresh_addr;
}

void AudioEffectParamSync(void)
{
	if(AudioEffect.context_memory == NULL)
		return;

	AudioEffect_SourceGain_Update(APP_SOURCE_NUM, mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM]);
	AudioEffect_SourceGain_Update(MIC_SOURCE_NUM, mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	AudioEqSync();
#endif
	EffectModeMsgProcess(MSG_EFFECT_SYNC); //ͬ���Զ��������д���Ĳ���

	gCtrlVars.AutoRefresh = AutoRefresh_ALL_EFFECTS_PARA;

#ifdef CFG_FUNC_BREAKPOINT_EN
	BackupInfoUpdata(BACKUP_SYS_INFO);
#endif
}

uint8_t AudioCommonMsgProcess(uint16_t Msg)
{
	uint8_t 	refresh_addr = 0;

	switch(Msg)
	{
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	case MSG_EQ:
		if(++EqMode >= 6)
			EqMode = 0;
		refresh_addr = AudioEqSync();
		break;
#endif
	case MSG_3D:
		refresh_addr = GetEffectControlIndex(_3D_ENABLE);
		APP_DBG("MSG_3D  %d\n",refresh_addr);
		if(refresh_addr)
		{
			bool enable = roboeffect_get_effect_status(AudioEffect.context_memory, refresh_addr);
			AudioEffect_effect_enable(refresh_addr, !enable);
		}
		break;
	case MSG_MUSIC_VOLUP:
		refresh_addr = GetEffectControlIndex(MUSIC_VOLUME_ADJUST);
		if(refresh_addr)
		{
			AudioMusicVolUp();
		}
		break;
	case MSG_MUSIC_VOLDOWN:
		refresh_addr = GetEffectControlIndex(MUSIC_VOLUME_ADJUST);
		if(refresh_addr)
		{
			AudioMusicVolDown();
		}
		break;
#if CFG_RES_MIC_SELECT
	case MSG_MIC_VOLUP:
		refresh_addr = GetEffectControlIndex(MIC_VOLUME_ADJUST);
		if(refresh_addr)
		{
			AudioMicVolUp();
		}
		break;
	case MSG_MIC_VOLDOWN:
		refresh_addr = GetEffectControlIndex(MIC_VOLUME_ADJUST);
		if(refresh_addr)
		{
			AudioMicVolDown();
		}
		break;
#endif
	}

	return refresh_addr;
}
