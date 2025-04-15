/**
 **************************************************************************************
 * @file    audio_core.c
 * @brief   audio core 
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include <nds32_intrinsic.h>
#include "main_task.h"
#include "audio_core_service.h"
#include "ctrlvars.h"
#include "audio_effect.h"
#include "mcu_circular_buf.h"
#include "beep.h"
#include "dma.h"

#ifdef CFG_APP_BT_MODE_EN
#include "bt_config.h"
#include "bt_play_api.h"
#include "bt_manager.h"
#if (BT_HFP_SUPPORT == ENABLE)
#include "bt_hf_api.h"
#endif
#endif
#include "dac_interface.h"
#include "audio_vol.h"
#include "user_effect_parameter.h"


/*******************************************************************************************************************************
 *
 *				 |***GetAdapter***|	  			 |***********CoreProcess***********|			  |***SetAdapter***|
 * ************	 ******************	 **********	 ***********************************  **********  ******************  **********
 *	SourceFIFO*->*SRCFIFO**SRAFIFO*->*PCMFrame*->*PreGainEffect**MixNet**GainEffect*->*PCMFrame*->*SRAFIFO**SRCFIFO*->*SinkFIFO*
 * ************  ******************	 **********	 ***********************************  **********  ******************  **********
 * 				 |*Context*|																			 |*Context*|
 *
 *******************************************************************************************************************************/

typedef enum
{
	AC_RUN_CHECK,//用于检测是否需要暂停任务，如果需要暂停任务，则停留再该状态
	AC_RUN_GET,
	AC_RUN_PROC,
	AC_RUN_PUT,
}AudioCoreRunState;

static AudioCoreRunState AudioState = AC_RUN_CHECK;
AudioCoreContext		AudioCore;

extern uint32_t gSysTick;
extern int32_t tws_get_pcm_delay(void);

void AudioCoreSourcePcmFormatConfig(uint8_t Index, uint16_t Format)
{
	if(Index < AUDIO_CORE_SOURCE_MAX_NUM)
	{
		AudioCore.AudioSource[Index].Channels = Format;
	}
}

void AudioCoreSourceEnable(uint8_t Index)
{
	if(Index < AUDIO_CORE_SOURCE_MAX_NUM)
	{
		AudioCore.AudioSource[Index].Enable = TRUE;
	}
}

void AudioCoreSourceDisable(uint8_t Index)
{
	if(Index < AUDIO_CORE_SOURCE_MAX_NUM)
	{
		AudioCore.AudioSource[Index].Enable = FALSE;
		//AudioCore.AudioSource[Index].LeftCurVol = 0;
		//AudioCore.AudioSource[Index].RightCurVol = 0;
	}
}

bool AudioCoreSourceIsEnable(uint8_t Index)
{
	return AudioCore.AudioSource[Index].Enable;
}

void AudioCoreSourceMute(uint8_t Index, bool IsLeftMute, bool IsRightMute)
{
	if(IsLeftMute)
	{
		AudioCore.AudioSource[Index].LeftMuteFlag = TRUE;
	}
	if(IsRightMute)
	{
		AudioCore.AudioSource[Index].RightMuteFlag = TRUE;
	}
}

void AudioCoreSourceUnmute(uint8_t Index, bool IsLeftUnmute, bool IsRightUnmute)
{
	if(IsLeftUnmute)
	{
		AudioCore.AudioSource[Index].LeftMuteFlag = FALSE;
		AudioCore.AudioSource[Index].LeftCurVol	  = 0; //淡入
	}
	if(IsRightUnmute)
	{
		AudioCore.AudioSource[Index].RightMuteFlag = FALSE;
		AudioCore.AudioSource[Index].RightCurVol   = 0; //淡入
	}
}

void AudioCoreSourceVolSet(uint8_t Index, uint16_t LeftVol, uint16_t RightVol)
{
	AudioEffect_SourceGain_Update(Index);
	AudioCore.AudioSource[Index].LeftVol = LeftVol;
	AudioCore.AudioSource[Index].RightVol = RightVol;
}

void AudioCoreSourceVolGet(uint8_t Index, uint16_t* LeftVol, uint16_t* RightVol)
{
	*LeftVol = AudioCore.AudioSource[Index].LeftCurVol;
	*RightVol = AudioCore.AudioSource[Index].RightCurVol;
}

void AudioCoreSourceConfig(uint8_t Index, AudioCoreSource* Source)
{
	memcpy(&AudioCore.AudioSource[Index], Source, sizeof(AudioCoreSource));
}

void AudioCoreSinkEnable(uint8_t Index)
{
	AudioCore.AudioSink[Index].Enable = TRUE;
}

void AudioCoreSinkDisable(uint8_t Index)
{
	AudioCore.AudioSink[Index].Enable = FALSE;
	AudioCore.AudioSink[Index].LeftCurVol = 0;
	AudioCore.AudioSink[Index].RightCurVol = 0;
}

bool AudioCoreSinkIsEnable(uint8_t Index)
{
	return AudioCore.AudioSink[Index].Enable;
}

void AudioCoreSinkMute(uint8_t Index, bool IsLeftMute, bool IsRightMute)
{
	/*if(IsLeftMute)
	{
		AudioCore.AudioSink[Index].LeftMuteFlag = TRUE;
	}
	if(IsRightMute)
	{
		AudioCore.AudioSink[Index].RightMuteFlag = TRUE;
	}
	*/
}

void AudioCoreSinkUnmute(uint8_t Index, bool IsLeftUnmute, bool IsRightUnmute)
{
	/*if(IsLeftUnmute)
	{
		AudioCore.AudioSink[Index].LeftMuteFlag = FALSE;
	}
	if(IsRightUnmute)
	{
		AudioCore.AudioSink[Index].RightMuteFlag = FALSE;
	}
	*/
}

void AudioCoreSinkVolSet(uint8_t Index, uint16_t LeftVol, uint16_t RightVol)
{
	AudioEffect_SinkGain_Update(Index);
	AudioCore.AudioSink[Index].LeftVol = LeftVol;
	AudioCore.AudioSink[Index].RightVol = RightVol;
}

void AudioCoreSinkVolGet(uint8_t Index, uint16_t* LeftVol, uint16_t* RightVol)
{
	*LeftVol = AudioCore.AudioSink[Index].LeftCurVol;
	*RightVol = AudioCore.AudioSink[Index].RightCurVol;
}

void AudioCoreSinkConfig(uint8_t Index, AudioCoreSink* Sink)
{
	memcpy(&AudioCore.AudioSink[Index], Sink, sizeof(AudioCoreSink));
}


void AudioCoreProcessConfig(AudioCoreProcessFunc AudioEffectProcess)
{
	AudioCore.AudioEffectProcess = AudioEffectProcess;
}

///**
// * @func        AudioCoreConfig
// * @brief       AudioCore参数块，本地化API
// * @param       AudioCoreContext *AudioCoreCt
// * @Output      None
// * @return      None
// * @Others      外部配置的参数块，复制一份到本地
// */
//void AudioCoreConfig(AudioCoreContext *AudioCoreCt)
//{
//	memcpy(&AudioCore, AudioCoreCt, sizeof(AudioCoreContext));
//}

bool AudioCoreInit(void)
{
	DBG("AudioCore init\n");
	return TRUE;
}

void AudioCoreDeinit(void)
{
	AudioState = AC_RUN_CHECK;
}

/**
 * @func        AudioCoreRun
 * @brief       音源拉流->音效处理+混音->推流
 * @param       None
 * @Output      None
 * @return      None
 * @Others      当前由audioCoreservice任务保障此功能有效持续。
 * Record
 */
extern uint32_t 	IsAudioCorePause;
extern uint32_t 	IsAudioCorePauseMsgSend;
extern bool AudioCoreSinkMuted(void);
void AudioProcessMain(void);
__attribute__((optimize("Og")))
void AudioCoreRun(void)
{
	bool ret;
	switch(AudioState)
	{
		case AC_RUN_CHECK:
			if(IsAudioCorePause == TRUE && ((!mainAppCt.gSysVol.MuteFlag) || AudioCoreSinkMuted()))
			{
				if(IsAudioCorePauseMsgSend == TRUE)
				{
					MessageContext		msgSend;
					msgSend.msgId		= MSG_AUDIO_CORE_HOLD;
					MessageSend(GetAudioCoreServiceMsgHandle(), &msgSend);

					IsAudioCorePauseMsgSend = FALSE;
				}
				return;
			}
		case AC_RUN_GET:
			AudioCoreIOLenProcess();
			ret = AudioCoreSourceSync();
			if(ret == FALSE)
			{
				return;
			}

		case AC_RUN_PROC:
			AudioProcessMain();
			AudioState = AC_RUN_PUT;

		case AC_RUN_PUT:
			ret = AudioCoreSinkSync();
			if(ret == FALSE)
			{
//				AudioCoreIOLenProcess();
				return;
			}
			AudioState = AC_RUN_CHECK;
			break;
		default:
			break;
	}
}

//音效处理函数，主入口
//将mic通路数据剥离出来统一处理
//mic通路数据和具体模式无关
//提示音通路无音效，剥离后在sink端混音。
void AudioProcessMain(void)
{
#ifdef CFG_FUNC_RECORDER_EN
	if(AudioCore.AudioSource[PLAYBACK_SOURCE_NUM].Enable == TRUE)
	{
		if(AudioCore.AudioSource[PLAYBACK_SOURCE_NUM].Channels == 1)
		{
			uint16_t i;
			for(i = SOURCEFRAME(PLAYBACK_SOURCE_NUM) * 2 - 1; i > 0; i--)
			{
				AudioCore.AudioSource[PLAYBACK_SOURCE_NUM].PcmInBuf[i] = AudioCore.AudioSource[PLAYBACK_SOURCE_NUM].PcmInBuf[i / 2];
			}
		}
	}
#endif
	if(AudioCore.AudioSource[APP_SOURCE_NUM].Active == TRUE)////music buff
	{
		#if (BT_HFP_SUPPORT == ENABLE) && defined(CFG_APP_BT_MODE_EN)
		if(GetSystemMode() != ModeBtHfPlay)
		#endif
		{
			if(AudioCore.AudioSource[APP_SOURCE_NUM].Channels == 1)
			{
				uint16_t i;
				for(i = SOURCEFRAME(APP_SOURCE_NUM) * 2 - 1; i > 0; i--)
				{
					AudioCore.AudioSource[APP_SOURCE_NUM].PcmInBuf[i] = AudioCore.AudioSource[APP_SOURCE_NUM].PcmInBuf[i / 2];
				}
			}
		}
	}	
		
#if defined(CFG_FUNC_REMIND_SOUND_EN)
	if(AudioCore.AudioSource[REMIND_SOURCE_NUM].Active == TRUE)////remind buff
	{
		if(AudioCore.AudioSource[REMIND_SOURCE_NUM].Channels == 1)
		{
			uint16_t i;
			for(i = SOURCEFRAME(REMIND_SOURCE_NUM) * 2 - 1; i > 0; i--)
			{
				AudioCore.AudioSource[REMIND_SOURCE_NUM].PcmInBuf[i] = AudioCore.AudioSource[REMIND_SOURCE_NUM].PcmInBuf[i / 2];
			}
		}
	}	
#endif


	if(AudioCore.AudioEffectProcess != NULL)
	{
		AudioCore.AudioEffectProcess((AudioCoreContext*)&AudioCore);
	}
	
    #ifdef CFG_FUNC_BEEP_EN
    if(AudioCore.AudioSink[AUDIO_DAC0_SINK_NUM].Active == TRUE)   ////dacx buff
	{
		Beep(AudioCore.AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf, SINKFRAME(AUDIO_DAC0_SINK_NUM));
	}
    #endif
}
