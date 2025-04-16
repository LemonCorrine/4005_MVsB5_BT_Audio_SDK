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

#if (CFG_PARA_MAX_VOLUME_NUM == 32)
static const int16_t VolumeTable[CFG_PARA_MAX_VOLUME_NUM + 1] = {
	-7200, -6300, -5600, -4900, -4400, -4000, -3600, -3200, -2900, -2600,
	-2400, -2200, -2000, -1900, -1800, -1700, -1600, -1500, -1400, -1300,
	-1200, -1100, -1000, -900, -800, -700, -600, -500, -400, -300,
	-200,  -100, 0
};
#elif (CFG_PARA_MAX_VOLUME_NUM == 16)
static const int16_t VolumeTable[CFG_PARA_MAX_VOLUME_NUM + 1] = {
	-7200, -5600, -4400, -3600, -2900, -2400, -2000, -1600, -1300, -1000,
	-800, -600, -400, -200, -100, 0
};
#endif


uint8_t AudioMusicVolSync(void)
{
	uint8_t 	refresh_addr = GetEffectControlIndex(MUSIC_VOLUME_ADJUST);
	uint8_t     vol = mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM];

	if(vol > CFG_PARA_MAX_VOLUME_NUM)
		vol = CFG_PARA_MAX_VOLUME_NUM;

	if(refresh_addr != 0)
		roboeffect_set_effect_parameter(AudioEffect.context_memory, refresh_addr, 1, (int16_t *)&VolumeTable[vol]);

	return refresh_addr;
}

uint8_t AudioMicVolSync(void)
{
	uint8_t 	refresh_addr = GetEffectControlIndex(MIC_VOLUME_ADJUST);
	uint8_t     vol = mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM];

	if(vol > CFG_PARA_MAX_VOLUME_NUM)
		vol = CFG_PARA_MAX_VOLUME_NUM;

	if(refresh_addr != 0)
		roboeffect_set_effect_parameter(AudioEffect.context_memory, refresh_addr, 1, (int16_t *)&VolumeTable[vol]);

	return refresh_addr;
}

uint8_t AudioRemindVolSync(void)
{
	uint8_t 	refresh_addr = GetEffectControlIndex(REMIND_VOLUME_ADJUST);
	uint8_t     vol = mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM];

	if(vol > CFG_PARA_MAX_VOLUME_NUM)
		vol = CFG_PARA_MAX_VOLUME_NUM;

	if(refresh_addr != 0)
		roboeffect_set_effect_parameter(AudioEffect.context_memory, refresh_addr, 1, (int16_t *)&VolumeTable[vol]);

	return refresh_addr;
}

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

	AudioMusicVolSync();
	AudioMicVolSync();
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	AudioEqSync();
#endif
	EffectModeMsgProcess(MSG_EFFECT_SYNC); //同步自动化代码中处理的参数

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
		AudioMusicVolUp();
		refresh_addr = AudioMusicVolSync();
		break;
	case MSG_MUSIC_VOLDOWN:
		AudioMusicVolDown();
		refresh_addr = AudioMusicVolSync();
		break;
#if CFG_RES_MIC_SELECT
	case MSG_MIC_VOLUP:
		AudioMicVolUp();
		refresh_addr = AudioMicVolSync();
		break;
	case MSG_MIC_VOLDOWN:
		AudioMicVolDown();
		refresh_addr = AudioMicVolSync();
		break;
#endif
	}

	return refresh_addr;
}
