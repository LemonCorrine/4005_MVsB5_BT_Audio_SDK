#include <string.h>
#include <nds32_intrinsic.h>
#include "math.h"
#include "debug.h"
#include "app_config.h"
#include "bt_config.h"
#include "rtos_api.h"
#include "audio_effect_library.h"
#include "audio_core_api.h"
#include "main_task.h"
#include "audio_effect.h"
#include "remind_sound.h"
#include "ctrlvars.h"
#include "bt_manager.h"
#include "mode_task.h"
#include "bt_hf_api.h"
#include "user_effect_parameter.h"

#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN
extern uint32_t music_eq_mode_unit;
#endif

AudioEffectContext 	AudioEffect;

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
__attribute__((optimize("Og")))
void AudioMusicProcess(AudioCoreContext *pAudioCore)
{
	int16_t  s;
	uint16_t n = AudioCoreFrameSizeGet(AudioCore.CurrentMix);
//	PCM_DATA_TYPE *monitor_out	= NULL;
	PCM_DATA_TYPE *mic_in	= NULL;
#ifdef CFG_APP_USB_AUDIO_MODE_EN
	PCM_DATA_TYPE *usb_out	= NULL;
#endif

//	for(s = 0; s < AUDIO_CORE_SOURCE_MAX_NUM; s++)
//	{
//		if(AudioCoreSourceToRoboeffect(s) != AUDIOCORE_SOURCE_SINK_ERROR)
//		{
//			if(!pAudioCore->AudioSource[s].Active || mainAppCt.gSysVol.MuteFlag || pAudioCore->AudioSource[s].LeftMuteFlag)
//			{
//				memset(roboeffect_get_source_buffer(AudioEffect.context_memory, AudioCoreSourceToRoboeffect(s)),
//									0, roboeffect_get_buffer_size(AudioEffect.context_memory));
//			}
//		}
//	}

#ifdef CFG_APP_USB_AUDIO_MODE_EN
	if(pAudioCore->AudioSink[USB_AUDIO_SINK_NUM].Active == TRUE)
	{
		usb_out = pAudioCore->AudioSink[USB_AUDIO_SINK_NUM].PcmOutBuf;
	}
#endif

	if(pAudioCore->AudioSource[MIC_SOURCE_NUM].Active)
	{
		mic_in = pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;
		if(mic_in)
		{
			for(s = n-1; s >= 0; s--)
			{
				mic_in[s*2 + 0] = mic_in[s];
				mic_in[s*2 + 1] = mic_in[s];
			}
		}
	}

	roboeffect_apply(AudioEffect.context_memory);

#ifdef CFG_APP_USB_AUDIO_MODE_EN
	if(usb_out)
	{
		if(pAudioCore->AudioSink[USB_AUDIO_SINK_NUM].Channels == 1)
		{
			for(s = 0; s < n; s++)
			{
#ifdef CFG_AUDIO_WIDTH_24BIT
				usb_out[s] = __nds32__clips(((int32_t)usb_out[2*s] + (int32_t)usb_out[2*s + 1] +1 ) >>1, 24-1);
#else
				usb_out[s] = __nds32__clips(((int32_t)usb_out[2*s] + (int32_t)usb_out[2*s + 1] +1 ) >>1, 16-1);
#endif
			}
		}
	}
#endif
}

#if (defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT))
//#define BTHF_AUDIOEFFECT_BYPASS  //目的:便于调试,直接进行通路数据的搬运,不应用V3音效流程
void AudioEffectProcessBTHF(AudioCoreContext *pAudioCore)
{
#ifdef BTHF_AUDIOEFFECT_BYPASS //audio effect bypass
	uint16_t n = AudioCoreFrameSizeGet(AudioCore.CurrentMix);
#endif
	int16_t  s;

#ifdef BTHF_AUDIOEFFECT_BYPASS
	int16_t  *hf_mic_in     = pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;//蓝牙通话mic采样buffer
	int16_t  *hf_pcm_in     = pAudioCore->AudioSource[APP_SOURCE_NUM].PcmInBuf;//蓝牙通话下传buffer
//	int16_t  *hf_aec_in		= NULL;//pBuf->hf_aec_in;;//蓝牙通话下传delay buffer,专供aec算法处理
	int16_t  *hf_pcm_out    = pAudioCore->AudioSink[AUDIO_APP_SINK_NUM].PcmOutBuf;//蓝牙通话上传buffer
	int16_t  *hf_dac_out    = pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;//蓝牙通话DAC的buffer
//	int16_t  *hf_rec_out    = NULL;//pBuf->hf_rec_out;//蓝牙通话送录音的buffer
#endif

	for(s = 0; s < AUDIO_CORE_SOURCE_MAX_NUM; s++)
	{
		if(AudioCoreSourceToRoboeffect(s) != AUDIOCORE_SOURCE_SINK_ERROR)
		{
			if(!pAudioCore->AudioSource[s].Active || mainAppCt.gSysVol.MuteFlag || pAudioCore->AudioSource[s].LeftMuteFlag)
			{
				memset(roboeffect_get_source_buffer(AudioEffect.context_memory, AudioCoreSourceToRoboeffect(s)),
									0, roboeffect_get_buffer_size(AudioEffect.context_memory));
			}
		}
	}

#ifdef BTHF_AUDIOEFFECT_BYPASS //audio effect bypass
	for(s = n-1; s >= 0; s--)
	{
		hf_dac_out[s*2 + 0] = hf_pcm_in[s];
		hf_dac_out[s*2 + 1] = hf_pcm_in[s];
	}

	for(s = 0; s < n; s++)
	{//mic单声道
		hf_pcm_out[s] = hf_mic_in[s];
	}
#else
	roboeffect_apply(AudioEffect.context_memory);
#endif

}
#endif
#endif
/*
****************************************************************
* 无音效音频处理主函数
*
****************************************************************
*/
void AudioBypassProcess(AudioCoreContext *pAudioCore)
{
	int16_t  s;
	uint16_t n = AudioCoreFrameSizeGet(AudioCore.CurrentMix);
	PCM_DATA_TYPE *mic_pcm    	= NULL;//pBuf->mic_in;///mic input
	PCM_DATA_TYPE *monitor_out  = NULL;//pBuf->dac0_out;
	PCM_DATA_TYPE *music_pcm    = NULL;//pBuf->app_in;
#ifdef CFG_APP_USB_AUDIO_MODE_EN
	PCM_DATA_TYPE *usb_out  	= NULL;//pBuf->app_out;
#endif
#ifdef CFG_FUNC_REMIND_SOUND_EN
	PCM_DATA_TYPE *remind_in 	= NULL;
#endif
	PCM_DATA_TYPE *i2s_out      = NULL;//pBuf->i2s0_out;

	for(s = 0; s < AUDIO_CORE_SOURCE_MAX_NUM; s++)
	{
		if(AudioCoreSourceToRoboeffect(s) != AUDIOCORE_SOURCE_SINK_ERROR)
		{
			if(!pAudioCore->AudioSource[s].Active || mainAppCt.gSysVol.MuteFlag || pAudioCore->AudioSource[s].LeftMuteFlag)
			{
				memset(roboeffect_get_source_buffer(AudioEffect.context_memory, AudioCoreSourceToRoboeffect(s)),
									0, roboeffect_get_buffer_size(AudioEffect.context_memory));
			}
		}
	}

#ifdef CFG_RES_AUDIO_DAC0_EN
	if(pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Active == TRUE)   ////dac0 buff
	{
		monitor_out = pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
	}
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	if(pAudioCore->AudioSink[AUDIO_STEREO_SINK_NUM].Active == TRUE)
	{
		i2s_out = pAudioCore->AudioSink[AUDIO_STEREO_SINK_NUM].PcmOutBuf;
	}
#endif
#ifdef CFG_APP_USB_AUDIO_MODE_EN
	if(pAudioCore->AudioSink[USB_AUDIO_SINK_NUM].Active == TRUE)
	{
		usb_out = pAudioCore->AudioSink[USB_AUDIO_SINK_NUM].PcmOutBuf;
	}
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
	remind_in = pAudioCore->AudioSource[REMIND_SOURCE_NUM].PcmInBuf;
#endif
	music_pcm = pAudioCore->AudioSource[APP_SOURCE_NUM].PcmInBuf;
	mic_pcm = pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;

	if(mic_pcm)
	{
		for(s = n-1; s >= 0; s--)
		{
			mic_pcm[2 * s + 0] = mic_pcm[s];
			mic_pcm[2 * s + 1] = mic_pcm[s];
		}
//			AudioCoreAppSourceVolApply(MIC_SOURCE_NUM, (int16_t *)mic_pcm, n, 2);
	}
	if(monitor_out || i2s_out)
	{
		if(monitor_out == NULL)
			monitor_out = i2s_out;
	#ifdef CFG_FUNC_REMIND_SOUND_EN
		if(pAudioCore->AudioSource[REMIND_SOURCE_NUM].Active)
		{
			for(s = 0; s < n; s++)
			{
				monitor_out[2 * s + 0] = remind_in[2 * s + 0];
				monitor_out[2 * s + 1] = remind_in[2 * s + 1];
			}
		}
		else
	#endif
		{
			if(music_pcm)
			{
				if(mic_pcm)
				{
					for(s = 0; s < n; s++)
					{
	#ifdef CFG_AUDIO_WIDTH_24BIT
						monitor_out[2 * s + 0] = __nds32__clips(((int32_t)music_pcm[2 * s + 0] + (int32_t)mic_pcm[2 * s + 0] +1 ) >>1, 24-1);
						monitor_out[2 * s + 1] = __nds32__clips(((int32_t)music_pcm[2 * s + 1] + (int32_t)mic_pcm[2 * s + 1] +1 ) >>1, 24-1);
	#else
						monitor_out[2 * s + 0] = __nds32__clips(((int32_t)music_pcm[2 * s + 0] + (int32_t)mic_pcm[2 * s + 0] +1 ) >>1, 16-1);
						monitor_out[2 * s + 1] = __nds32__clips(((int32_t)music_pcm[2 * s + 1] + (int32_t)mic_pcm[2 * s + 1] +1 ) >>1, 16-1);
	#endif
					}
				}
				else
				{
					for(s = 0; s < n; s++)
					{
						monitor_out[2 * s + 0] = music_pcm[2 * s + 0];
						monitor_out[2 * s + 1] = music_pcm[2 * s + 1];
					}
				}
			}
		}

		if(i2s_out && i2s_out != monitor_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s_out[2*s + 0] = monitor_out[2*s + 0];
				i2s_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}
	}

#ifdef CFG_APP_USB_AUDIO_MODE_EN
	if(usb_out)
	{
		if(pAudioCore->AudioSink[USB_AUDIO_SINK_NUM].Channels == 1)
		{
			for(s = 0; s < n; s++)
			{
#ifdef CFG_AUDIO_WIDTH_24BIT
				usb_out[s] = __nds32__clips(((int32_t)mic_pcm[2 * s] + (int32_t)mic_pcm[2 * s + 1] +1 ) >>1, 24-1);
#else
				usb_out[s] = __nds32__clips(((int32_t)mic_pcm[2 * s] + (int32_t)mic_pcm[2 * s + 1] +1 ) >>1, 16-1);
#endif
			}
		}
		else
		{
			for(s = 0; s < n; s++)
			{
				usb_out[2 * s + 0] = mic_pcm[s];
				usb_out[2 * s + 1] = mic_pcm[s];
			}
		}
	}
#endif

}

//无App通路音频处理主函数
void AudioNoAppProcess(AudioCoreContext *pAudioCore)
{
	int16_t  s;
	uint16_t n = AudioCoreFrameSizeGet(AudioCore.CurrentMix);
	PCM_DATA_TYPE *mic_pcm    	= NULL;//pBuf->mic_in;///mic input
	PCM_DATA_TYPE *monitor_out    = NULL;//pBuf->dac0_out;
#ifdef	CFG_FUNC_REMIND_SOUND_EN
	PCM_DATA_TYPE *remind_in = NULL;
#endif
	PCM_DATA_TYPE *i2s_out       = NULL;//pBuf->i2s0_out;

	if(pAudioCore->AudioSource[MIC_SOURCE_NUM].Active == TRUE)////mic buff
	{
		mic_pcm = pAudioCore->AudioSource[MIC_SOURCE_NUM].PcmInBuf;//双mic输入
	}

    if(pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].Active == TRUE)   ////dac0 buff
	{
    	monitor_out = pAudioCore->AudioSink[AUDIO_DAC0_SINK_NUM].PcmOutBuf;
	}

#if defined(CFG_FUNC_REMIND_SOUND_EN)
	if(pAudioCore->AudioSource[REMIND_SOURCE_NUM].Active == TRUE)////remind buff
	{
		remind_in = pAudioCore->AudioSource[REMIND_SOURCE_NUM].PcmInBuf;
	}
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	if(pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].Active == TRUE)	////i2s buff
	{
		i2s_out = pAudioCore->AudioSink[AUDIO_I2SOUT_SINK_NUM].PcmOutBuf;
	}
#endif


    if(monitor_out)
	{
		memset(monitor_out, 0, roboeffect_get_buffer_size(AudioEffect.context_memory));
    }
    if(i2s_out)
	{
		memset(i2s_out, 0, roboeffect_get_buffer_size(AudioEffect.context_memory));
    }

	//DAC0立体声监听音效处理
	if(monitor_out || i2s_out)
	{
		if(monitor_out == NULL)
			monitor_out = i2s_out;
		for(s = 0; s < n; s++)
		{
		#if defined(CFG_FUNC_REMIND_SOUND_EN)
			if(remind_in)
			{
				monitor_out[2*s + 0] = remind_in[2*s + 0];
				monitor_out[2*s + 1] = remind_in[2*s + 1];
			}
			else
		#endif
			if(mic_pcm)
			{
				monitor_out[2*s + 0] = mic_pcm[2*s + 0];
				monitor_out[2*s + 1] = mic_pcm[2*s + 1];
			}
		}

		if(i2s_out && i2s_out != monitor_out)
		{
			for(s = 0; s < n; s++)
			{
				i2s_out[2*s + 0] = monitor_out[2*s + 0];
				i2s_out[2*s + 1] = monitor_out[2*s + 1];
			}
		}		
	}

}

