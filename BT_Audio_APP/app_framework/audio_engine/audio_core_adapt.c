/*
 * audio_core_adapt.c
 *
 *  Created on: Mar 11, 2021
 *      Author: piwang
 */


#include "string.h"
#include "resampler_polyphase.h"
#include "clk.h"
#include "rtos_api.h"
#include "debug.h"
#include "app_config.h"
#include "audio_core_api.h"
#include "audio_core_adapt.h"
#include "dac.h"
#include "mode_task.h"
#include "audio_vol.h"
#include "main_task.h"
#include "ctrlvars.h"
#include "user_effect_parameter.h"

#ifdef BT_AUDIO_AAC_ENABLE
#include "bt_play_api.h"
#include "bt_manager.h"
#endif

extern bool GIE_STATE_GET(void);
extern uint16_t AudioDAC0DataLenGet(void);
#define AUDIO_CORE_DEBUG 	//强化配置安全检测和定位


RESAMPLER_POLYPHASE_SRC_RATIO GetRatioEnum(uint32_t Scale1000)
{
	switch(Scale1000)
	{
		case (1000 * 6):
			return RESAMPLER_POLYPHASE_SRC_RATIO_6_1;
		case (1000 * 441 / 80):
			return RESAMPLER_POLYPHASE_SRC_RATIO_441_80;
		case (1000 * 640 / 147):
			return RESAMPLER_POLYPHASE_SRC_RATIO_640_147;
		case (1000 * 4):
			return RESAMPLER_POLYPHASE_SRC_RATIO_4_1;
		case (1000 * 147 / 40):
			return RESAMPLER_POLYPHASE_SRC_RATIO_147_40;
		case (1000 * 3):
			return RESAMPLER_POLYPHASE_SRC_RATIO_3_1;
		case (1000 * 441 / 160):
			return RESAMPLER_POLYPHASE_SRC_RATIO_441_160;
		case (1000 * 320 / 147):
			return RESAMPLER_POLYPHASE_SRC_RATIO_320_147;
		case (1000 * 2):
			return RESAMPLER_POLYPHASE_SRC_RATIO_2_1;
		case (1000 * 147 / 80):
			return RESAMPLER_POLYPHASE_SRC_RATIO_147_80;
		case (1000 * 3 / 2):
			return RESAMPLER_POLYPHASE_SRC_RATIO_3_2;
		case (1000 * 441 / 320):
			return RESAMPLER_POLYPHASE_SRC_RATIO_441_320;
		case (1000 * 4 / 3):
			return RESAMPLER_POLYPHASE_SRC_RATIO_4_3;
		case (1000 * 160 / 147):
			return RESAMPLER_POLYPHASE_SRC_RATIO_160_147;
/*****************************************************************/
		case (1000 * 147 / 160):
			return RESAMPLER_POLYPHASE_SRC_RATIO_147_160;
		case (1000 * 3 / 4):
			return RESAMPLER_POLYPHASE_SRC_RATIO_3_4;
		case (1000 * 80 / 147):
			return RESAMPLER_POLYPHASE_SRC_RATIO_80_147;
		case (1000 / 2):
			return RESAMPLER_POLYPHASE_SRC_RATIO_1_2;
		case (1000 * 147 / 320):
			return RESAMPLER_POLYPHASE_SRC_RATIO_147_320;
		case (1000 * 160 / 441):
			return RESAMPLER_POLYPHASE_SRC_RATIO_160_441;
		case (1000 / 3):
			return RESAMPLER_POLYPHASE_SRC_RATIO_1_3;
		case (1000 * 40 / 147):
			return RESAMPLER_POLYPHASE_SRC_RATIO_40_147;
		case (1000 / 4):
			return RESAMPLER_POLYPHASE_SRC_RATIO_1_4;
		case (1000 * 147 / 640):
			return RESAMPLER_POLYPHASE_SRC_RATIO_147_640;
		case (1000 * 320 / 441):
			return RESAMPLER_POLYPHASE_SRC_RATIO_320_441;
		case (1000 * 2 / 3):
			return RESAMPLER_POLYPHASE_SRC_RATIO_2_3;
		default:
#ifdef AUDIO_CORE_DEBUG
			if(Scale1000 != 1000)
			{
				DBG("SRC Samplerate Error！\n");
			}
#endif
			return RESAMPLER_POLYPHASE_SRC_RATIO_UNSUPPORTED;
	}
}
//根据转采样比例，输入数据和输出空间，获取合适的输入长度，返回 SRC_INPUT_MIN~MAX_FRAME_SAMPLES
uint16_t	SRCValidLenGet(uint32_t SampleRateIn, uint32_t AudioLen, uint32_t SampleRateOut, uint32_t SpaceLen)
{
	uint16_t ValidData = 0;
	uint16_t ValidSpace = sizeof(AudioCore.AdaptOut) / 4;
	if(SpaceLen >= SRC_OUPUT_MIN && AudioLen >= (SampleRateIn) *(SRC_OUPUT_MIN - SRC_OUPUT_JITTER)/(SampleRateOut))
	{
		if(SpaceLen < ValidSpace)
		{
			ValidSpace = SpaceLen;
		}
		ValidData = SampleRateIn *(ValidSpace - SRC_OUPUT_JITTER)/(SampleRateOut);
		if(ValidData > AudioLen)
		{
			ValidData = AudioLen;
		}

		if(ValidData < SRC_INPUT_MIN)
		{
			ValidData = 0;
		}
		else if(ValidData > MAX_FRAME_SAMPLES)
		{
			ValidData = MAX_FRAME_SAMPLES;
		}
	}
	return ValidData;
}

//使用此宏的参数 注意不要与中断变参 耦合
#define SRAValidLenGet(AudioLen, SpaceLen)	(((AudioLen) >= SRA_BLOCK && (SpaceLen) >= (SRA_BLOCK + SRA_MAX_CHG)) ? SRA_BLOCK : 0)

//1 Low 0 High -1
#define ADJLEVEL(LEVEL, LOW, HIGH)			((LEVEL) < (LOW) ? 1 : ((LEVEL) > ( HIGH)? (-1) : (0)))

uint32_t Min3(uint32_t x, uint32_t y, uint32_t z)
{
	uint32_t Val;
	if(x > y)
	{
		Val = y;
	}
	else
	{
		Val = x;
	}
	if(Val > z)
	{
		Val = z;
	}
	return Val;
}

//sign 1:正向微调; -1:反向微调
void ClkAdjust(CLK_ADJUST_ADAPTER * AdjAdapter, uint32_t ValidDepth, int8_t sign)
{
	uint16_t AverageLevel = AdjAdapter->TotalNum / AdjAdapter->Count;

	//水位超 mix要加快 div要降低
	int16_t	AdjustVal = ADJLEVEL(AdjAdapter->TotalNum / AdjAdapter->Count,
							(ValidDepth * AdjAdapter->LowLevelCent) / 100,
							(ValidDepth * AdjAdapter->HighLevelCent) / 100);
	if(!AdjustVal)//
	{
		if(AverageLevel > AdjAdapter->LastLevel)
		{
			if(AdjAdapter->RiseTimes > 0)
			{
				AdjAdapter->RiseTimes++;
				if(AdjAdapter->RiseTimes >= ADJUST_SHRESHOLD)
				{
					AdjustVal = -1;
				}
			}
			else
			{
				AdjAdapter->RiseTimes = 1;
			}
		}
		else if(AverageLevel < AdjAdapter->LastLevel)
		{
			if(AdjAdapter->RiseTimes < 0)
			{
				AdjAdapter->RiseTimes--;
				if(AdjAdapter->RiseTimes <= -ADJUST_SHRESHOLD)
				{
					AdjustVal = 1;
				}
			}
			else
			{
				AdjAdapter->RiseTimes = -1;
			}
		}
	}
	AdjustVal += AdjAdapter->AdjustVal; //历史值。
	AdjustVal += ADJLEVEL(AdjustVal, -ADJUST_DIV_MAX, ADJUST_DIV_MAX);//钳位 分频
	if(AdjAdapter->AdjustVal != AdjustVal)
	{
		AdjAdapter->AdjustVal = AdjustVal;
		AdjustVal *= sign;
		if(AdjustVal >= 0)
		{
			Clock_AudioPllClockAdjust(PLL_CLK_1, 0, AdjustVal);
			Clock_AudioPllClockAdjust(PLL_CLK_2, 0, AdjustVal);
		}
		else
		{
			Clock_AudioPllClockAdjust(PLL_CLK_1, 1, -AdjustVal);
			Clock_AudioPllClockAdjust(PLL_CLK_2, 1, -AdjustVal);
		}
	}
	AdjAdapter->LastLevel = AverageLevel;
	AdjAdapter->TotalNum = 0;
	AdjAdapter->Count = 0;
}

bool AudioCoreSourceInit(AudioCoreIO * AudioIO, uint8_t Index)
{
	uint32_t	UsedSize;// byte
	void		* AdaptBuf;
	AudioCoreSource * Source = &AudioCore.AudioSource[Index];
#ifdef AUDIO_CORE_DEBUG
	if(AudioCoreSourceIsInit(Index)
			|| AudioIO == NULL
			|| AudioIO->Channels > 2
			|| AudioIO->Adapt > SRC_ADJUST
			|| AudioIO->Net >= MaxNet
			|| AudioIO->DataIOFunc == NULL
			|| AudioIO->LenGetFunc == NULL)
	{
		DBG("Soure init error!\n");
		return FALSE;
	}
#endif
	Source->Channels = AudioIO->Channels;
	Source->Adapt = AudioIO->Adapt;
	Source->Net = AudioIO->Net;
	Source->Sync = AudioIO->Sync;
	Source->DataGetFunc = (AudioCoreDataGetFunc)(AudioIO->DataIOFunc);
	Source->DataLenFunc = (AudioCoreSpaceLenFunc)AudioIO->LenGetFunc;
#ifdef	CFG_AUDIO_WIDTH_24BIT
	Source->BitWidth = AudioIO->IOBitWidth;
	Source->BitWidthConvFlag = AudioIO->IOBitWidthConvFlag;
#endif
	Source->PcmBufFlag = 0;
	switch(AudioIO->Adapt)
	{
		case	STD:
			Source->PcmInBuf = roboeffect_get_source_buffer(AudioEffect[AudioCore.AudioSource[Index].Net].context_memory, AudioCoreSourceToRoboeffect(Index));
			if(Source->PcmInBuf == NULL)
			{
				Source->PcmInBuf = (PCM_DATA_TYPE *)osPortMalloc(SOURCEFRAME(Index) * sizeof(PCM_DATA_TYPE) * 2);
				if(Source->PcmInBuf == NULL)
				{
					DBG("PcmInBuf NULL error!\n");
					return FALSE;
				}
				Source->PcmBufFlag = 1;
			}
			Source->AdaptBuf = NULL;
			Source->AdjAdapter = NULL;
			Source->SrcAdapter = NULL;
			break;

		case   SRC_ONLY:
		{
			SRC_ADAPTER * SrcAdapter;
			UsedSize = SRC_FIFO_SIZE(SOURCEFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels;
			AdaptBuf = osPortMalloc(UsedSize);
			if(AdaptBuf == NULL
#ifdef AUDIO_CORE_DEBUG
					|| AudioIO->SampleRate == 0
#endif
					)
			{
				DBG("Src set error!\n");
				return FALSE;
			}
			memset(AdaptBuf, 0, UsedSize);
			Source->PcmInBuf = roboeffect_get_source_buffer(AudioEffect[AudioCore.AudioSource[Index].Net].context_memory, AudioCoreSourceToRoboeffect(Index));
			if(Source->PcmInBuf == NULL)
			{
				Source->PcmInBuf = (PCM_DATA_TYPE *)osPortMalloc(SOURCEFRAME(Index) * sizeof(PCM_DATA_TYPE) * 2);
				if(Source->PcmInBuf == NULL)
				{
					DBG("PcmInBuf NULL error!\n");
					osPortFree(AdaptBuf);
					return FALSE;
				}
				Source->PcmBufFlag = 1;
			}
			SrcAdapter = (SRC_ADAPTER *)osPortMalloc(sizeof(SRC_ADAPTER));
			if(SrcAdapter == NULL)
			{
				osPortFree(AdaptBuf);
				return FALSE;
			}
			memset(SrcAdapter, 0, sizeof(SRC_ADAPTER));

			SrcAdapter->SampleRate = AudioIO->SampleRate;
			if(SrcAdapter->SampleRate != AudioCore.SampleRate[AudioIO->Net])
			{
				resampler_polyphase_init(&SrcAdapter->SrcCt, AudioIO->Channels, GetRatioEnum(1000 * AudioCore.SampleRate[AudioIO->Net] / SrcAdapter->SampleRate));
			}
			MCUCircular_Config(&SrcAdapter->SrcBufHandler, AdaptBuf, SRC_FIFO_SIZE(SOURCEFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels);
			Source->AdaptBuf = AdaptBuf;
			Source->SrcAdapter = SrcAdapter;
			Source->AdjAdapter = NULL;
			break;
		}

		case SRA_ONLY:
		{
			SRA_ADAPTER *AdjAdapter;
			UsedSize = SRA_FIFO_SIZE(SOURCEFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels;
			AdaptBuf = osPortMalloc(UsedSize);
			if(AdaptBuf == NULL
#ifdef AUDIO_CORE_DEBUG
					|| AudioIO->HighLevelCent == 0
					|| AudioIO->Depth == 0
#endif
				)
			{
				DBG("Sra set error!\n");
				return FALSE;
			}
			memset(AdaptBuf, 0, UsedSize);
			Source->PcmInBuf = roboeffect_get_source_buffer(AudioEffect[AudioCore.AudioSource[Index].Net].context_memory, AudioCoreSourceToRoboeffect(Index));
			if(Source->PcmInBuf == NULL)
			{
				Source->PcmInBuf = (PCM_DATA_TYPE *)osPortMalloc(SOURCEFRAME(Index) * sizeof(PCM_DATA_TYPE) * 2);
				if(Source->PcmInBuf == NULL)
				{
					DBG("PcmInBuf NULL error!\n");
					osPortFree(AdaptBuf);
					return FALSE;
				}
				Source->PcmBufFlag = 1;
			}
			AdjAdapter = (SRA_ADAPTER *)osPortMalloc(sizeof(SRA_ADAPTER));
			if(AdjAdapter == NULL)
			{
				osPortFree(AdaptBuf);
				return FALSE;
			}
			memset(AdjAdapter, 0, sizeof(SRA_ADAPTER));

			//sra_init(&AdjAdapter->SraCt, AudioIO->Channels);
			resampler_farrow_init(&AdjAdapter->SraResFarCt, AudioIO->Channels, POLYNOMIAL_ORDER);
			MCUCircular_Config(&AdjAdapter->SraBufHandler, AdaptBuf, SRA_FIFO_SIZE(SOURCEFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels);
			AdjAdapter->HighLevelCent = AudioIO->HighLevelCent;
			AdjAdapter->LowLevelCent = AudioIO->LowLevelCent;
			AdjAdapter->Depth = AudioIO->Depth;
			Source->AdaptBuf = AdaptBuf;
			Source->AdjAdapter = (void *)AdjAdapter;
			Source->SrcAdapter = NULL;
			break;
		}

		case CLK_ADJUST_ONLY:
		{
			CLK_ADJUST_ADAPTER *AdjAdapter;
			Source->PcmInBuf = roboeffect_get_source_buffer(AudioEffect[AudioCore.AudioSource[Index].Net].context_memory, AudioCoreSourceToRoboeffect(Index));
			if(Source->PcmInBuf == NULL)
			{
				Source->PcmInBuf = (PCM_DATA_TYPE *)osPortMalloc(SOURCEFRAME(Index) * sizeof(PCM_DATA_TYPE) * 2);
				if(Source->PcmInBuf == NULL)
				{
					DBG("PcmInBuf NULL error!\n");
					return FALSE;
				}
				Source->PcmBufFlag = 1;
			}
			if(Source->PcmInBuf  == NULL
#ifdef AUDIO_CORE_DEBUG
					|| AudioIO->HighLevelCent == 0
					|| AudioIO->Depth == 0
#endif
									)
			{
				DBG("Clkadj set error!\n");
				return FALSE;
			}

			AdjAdapter = (CLK_ADJUST_ADAPTER *)osPortMalloc(sizeof(CLK_ADJUST_ADAPTER));
			if(AdjAdapter == NULL)
			{
//				osPortFree(Source->PcmInBuf);
				return FALSE;
			}
			memset(AdjAdapter, 0, sizeof(CLK_ADJUST_ADAPTER));

			AdjAdapter->HighLevelCent = AudioIO->HighLevelCent;
			AdjAdapter->LowLevelCent = AudioIO->LowLevelCent;
			AdjAdapter->Depth = AudioIO->Depth;
			Source->AdaptBuf = NULL;
			Source->AdjAdapter = (void *)AdjAdapter;
			Source->SrcAdapter = NULL;
			break;
		}

		case SRC_SRA:
		{
			SRA_ADAPTER *AdjAdapter;
			SRC_ADAPTER *SrcAdapter;
			UsedSize = SRA_FIFO_SIZE(SOURCEFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels + SRC_FIFO_SIZE(SOURCEFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels;
			AdaptBuf = osPortMalloc(UsedSize);
			if(AdaptBuf == NULL
#ifdef AUDIO_CORE_DEBUG
					|| AudioIO->SampleRate == 0
					|| AudioIO->HighLevelCent == 0
					|| AudioIO->Depth == 0
#endif
							)
			{
				DBG("Srcsra set error!\n");
				return FALSE;
			}
			memset(AdaptBuf, 0, UsedSize);
			Source->PcmInBuf = roboeffect_get_source_buffer(AudioEffect[AudioCore.AudioSource[Index].Net].context_memory, AudioCoreSourceToRoboeffect(Index));
			if(Source->PcmInBuf == NULL)
			{
				Source->PcmInBuf = (PCM_DATA_TYPE *)osPortMalloc(SOURCEFRAME(Index) * sizeof(PCM_DATA_TYPE) * 2);
				if(Source->PcmInBuf == NULL)
				{
					DBG("PcmInBuf NULL error!\n");
					osPortFree(AdaptBuf);
					return FALSE;
				}
				Source->PcmBufFlag = 1;
			}
			Source->AdjAdapter = osPortMalloc(sizeof(SRA_ADAPTER) + sizeof(SRC_ADAPTER));
			if(Source->AdjAdapter == NULL)
			{
				osPortFree(AdaptBuf);
				return FALSE;
			}
			memset(Source->AdjAdapter, 0, sizeof(SRA_ADAPTER) + sizeof(SRC_ADAPTER));
			AdjAdapter = (SRA_ADAPTER *)Source->AdjAdapter;
			SrcAdapter = (SRC_ADAPTER *)(Source->AdjAdapter + sizeof(SRA_ADAPTER));


			SrcAdapter->SampleRate = AudioIO->SampleRate;
			if(SrcAdapter->SampleRate != AudioCore.SampleRate[AudioIO->Net])
			{
				resampler_polyphase_init(&SrcAdapter->SrcCt, AudioIO->Channels, GetRatioEnum(1000 * AudioCore.SampleRate[AudioIO->Net] / SrcAdapter->SampleRate));
			}
			MCUCircular_Config(&SrcAdapter->SrcBufHandler,
					AdaptBuf + SRA_FIFO_SIZE(SOURCEFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels,
					SRC_FIFO_SIZE(SOURCEFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels);
			//sra_init(&AdjAdapter->SraCt, AudioIO->Channels);
			resampler_farrow_init(&AdjAdapter->SraResFarCt, AudioIO->Channels, POLYNOMIAL_ORDER);
			MCUCircular_Config(&AdjAdapter->SraBufHandler, AdaptBuf, SRA_FIFO_SIZE(SOURCEFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels);
			AdjAdapter->HighLevelCent = AudioIO->HighLevelCent;
			AdjAdapter->LowLevelCent = AudioIO->LowLevelCent;
			AdjAdapter->Depth = AudioIO->Depth;
			Source->AdaptBuf = AdaptBuf;
			Source->SrcAdapter = SrcAdapter;
			Source->AdjAdapter = (void *)AdjAdapter;

			break;
		}

		case SRC_ADJUST:
		{
			SRC_ADAPTER *SrcAdapter;
			CLK_ADJUST_ADAPTER *AdjAdapter;
			UsedSize = SRC_FIFO_SIZE(SOURCEFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels;
			AdaptBuf = osPortMalloc(UsedSize);
			if(AdaptBuf == NULL
#ifdef AUDIO_CORE_DEBUG
					|| AudioIO->SampleRate == 0
					|| AudioIO->HighLevelCent == 0
					|| AudioIO->Depth == 0
#endif
							)
			{
				DBG("Srcadj set error!\n");
				return FALSE;
			}
			memset(AdaptBuf, 0, UsedSize);
			Source->PcmInBuf = roboeffect_get_source_buffer(AudioEffect[AudioCore.AudioSource[Index].Net].context_memory, AudioCoreSourceToRoboeffect(Index));
			if(Source->PcmInBuf == NULL)
			{
				Source->PcmInBuf = (PCM_DATA_TYPE *)osPortMalloc(SOURCEFRAME(Index) * sizeof(PCM_DATA_TYPE) * 2);
				if(Source->PcmInBuf == NULL)
				{
					DBG("PcmInBuf NULL error!\n");
					osPortFree(AdaptBuf);
					return FALSE;
				}
				Source->PcmBufFlag = 1;
			}
			Source->AdjAdapter = osPortMalloc(sizeof(CLK_ADJUST_ADAPTER) + sizeof(SRC_ADAPTER));
			if(Source->AdjAdapter == NULL)
			{
				osPortFree(AdaptBuf);
				return FALSE;
			}
			memset(Source->AdjAdapter, 0, sizeof(CLK_ADJUST_ADAPTER) + sizeof(SRC_ADAPTER));

			SrcAdapter = (SRC_ADAPTER *)(Source->AdjAdapter + sizeof(CLK_ADJUST_ADAPTER));
			SrcAdapter->SampleRate = AudioIO->SampleRate;
			if(SrcAdapter->SampleRate != AudioCore.SampleRate[AudioIO->Net])
			{
				resampler_polyphase_init(&SrcAdapter->SrcCt, AudioIO->Channels, GetRatioEnum(1000 * AudioCore.SampleRate[AudioIO->Net] / SrcAdapter->SampleRate));
			}
			MCUCircular_Config(&SrcAdapter->SrcBufHandler, AdaptBuf, SRC_FIFO_SIZE(SOURCEFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels);

			AdjAdapter = (CLK_ADJUST_ADAPTER *)Source->AdjAdapter;
			AdjAdapter->HighLevelCent = AudioIO->HighLevelCent;
			AdjAdapter->LowLevelCent = AudioIO->LowLevelCent;
			AdjAdapter->Depth = AudioIO->Depth;
			Source->AdaptBuf = AdaptBuf;
			Source->SrcAdapter = SrcAdapter;
			Source->AdjAdapter = (void *)AdjAdapter;
			break;
		}
	}
	Source->InitFlag = TRUE;
	return TRUE;
}

void AudioCoreSourceDeinit(uint8_t Index)
{
	AudioCoreSource * Source = &AudioCore.AudioSource[Index];
#ifdef AUDIO_CORE_DEBUG
	if(!AudioCoreSourceIsInit(Index))
		return;
#endif
	Source->Enable = FALSE;
	Source->Sync = FALSE;
	SOURCE_BIT_DIS(AudioCore.FrameReady, Index);
	Source->DataGetFunc = NULL;
	Source->DataLenFunc = NULL;
	if(Source->PcmBufFlag)
		osPortFree(Source->PcmInBuf);
	Source->PcmBufFlag = 0;
	Source->PcmInBuf = NULL;
	switch(Source->Adapt)
	{
		case   	SRC_ONLY:
			osPortFree(Source->AdaptBuf);
			Source->AdaptBuf = NULL;
			osPortFree(Source->SrcAdapter);
			Source->SrcAdapter = NULL;
			break;
		case	SRA_ONLY:
			osPortFree(Source->AdaptBuf);
			Source->AdaptBuf = NULL;
			osPortFree(Source->AdjAdapter);
			Source->AdjAdapter = NULL;
			break;
		case 	CLK_ADJUST_ONLY:
			osPortFree(Source->AdjAdapter);
			Source->AdjAdapter = NULL;
			break;
		case 	SRC_SRA:
		case 	SRC_ADJUST:
			osPortFree(Source->AdaptBuf);
			Source->AdaptBuf = NULL;
			osPortFree(Source->AdjAdapter);
			Source->SrcAdapter = NULL;
			Source->AdjAdapter = NULL;
			break;
		default:
			break;
	}
	Source->InitFlag = FALSE;
}

bool AudioCoreSinkInit(AudioCoreIO * AudioIO, uint8_t Index)
{
	uint32_t	UsedSize;// byte
	void		* AdaptBuf;
	AudioCoreSink *Sink = &AudioCore.AudioSink[Index];
#ifdef AUDIO_CORE_DEBUG
	if(AudioCoreSinkIsInit(Index)
			|| AudioIO == NULL
			|| AudioIO->Channels > 2
			|| AudioIO->Depth <= AudioCoreFrameSizeGet(AudioCore.AudioSource[Index].Net)
			|| AudioIO->Adapt > SRC_ADJUST
			|| AudioIO->Net >= MaxNet
			|| AudioIO->DataIOFunc == NULL
			|| AudioIO->LenGetFunc ==NULL)
	{
		DBG("Sink para error!\n");
		return FALSE;
	}
#endif
	Sink->Channels = AudioIO->Channels;
	Sink->Adapt = AudioIO->Adapt;
	Sink->Net = AudioIO->Net;
	Sink->Sync = AudioIO->Sync;
	Sink->Depth = AudioIO->Depth;
	Sink->DataSetFunc = (AudioCoreDataSetFunc)AudioIO->DataIOFunc;
	Sink->SpaceLenFunc = (AudioCoreSpaceLenFunc)AudioIO->LenGetFunc;

#ifdef	CFG_AUDIO_WIDTH_24BIT
	Sink->BitWidth = AudioIO->IOBitWidth;
	Sink->BitWidthConvFlag = AudioIO->IOBitWidthConvFlag;
#endif
	Sink->PcmBufFlag = 0;
	switch(AudioIO->Adapt)
	{
		case	STD:
			Sink->PcmOutBuf = roboeffect_get_sink_buffer(AudioEffect[AudioCore.AudioSink[Index].Net].context_memory, AudioCoreSinkToRoboeffect(Index));
			if(Sink->PcmOutBuf == NULL)
			{
				Sink->PcmOutBuf = (PCM_DATA_TYPE *)osPortMalloc(SINKFRAME(Index) * sizeof(PCM_DATA_TYPE) * 2);
				if(Sink->PcmOutBuf == NULL)
				{
					return FALSE;
				}
				Sink->PcmBufFlag = 1;
			}
			Sink->AdaptBuf = NULL;
			Sink->AdjAdapter = NULL;
			Sink->SrcAdapter = NULL;
			break;

		case   SRC_ONLY:
		{
			SRC_ADAPTER * SrcAdapter;
			UsedSize = SRC_FIFO_SIZE(SINKFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels;
			if(AudioIO->Resident)
			{
				AdaptBuf = osPortMalloc(UsedSize);
			}
			else
			{
				AdaptBuf = osPortMalloc(UsedSize);
			}
			if(AdaptBuf == NULL
#ifdef AUDIO_CORE_DEBUG
					|| AudioIO->SampleRate == 0
#endif
					)
			{
				DBG("SRC set error!\n");
				return FALSE;
			}
			memset(AdaptBuf, 0, UsedSize);
			Sink->PcmOutBuf = roboeffect_get_sink_buffer(AudioEffect[AudioCore.AudioSink[Index].Net].context_memory, AudioCoreSinkToRoboeffect(Index));
			if(Sink->PcmOutBuf == NULL)
			{
				Sink->PcmOutBuf = (PCM_DATA_TYPE *)osPortMalloc(SINKFRAME(Index) * sizeof(PCM_DATA_TYPE) * 2);
				if(Sink->PcmOutBuf == NULL)
				{
					osPortFree(AdaptBuf);
					return FALSE;
				}
				Sink->PcmBufFlag = 1;
			}
			SrcAdapter = (SRC_ADAPTER *)osPortMalloc(sizeof(SRC_ADAPTER));
			if(SrcAdapter == NULL)
			{
				osPortFree(AdaptBuf);
				return FALSE;
			}
			memset(SrcAdapter, 0, sizeof(SRC_ADAPTER));

			SrcAdapter->SampleRate = AudioIO->SampleRate;
			if(SrcAdapter->SampleRate != AudioCore.SampleRate[AudioIO->Net])
			{
				resampler_polyphase_init(&SrcAdapter->SrcCt, AudioIO->Channels, GetRatioEnum(1000 * SrcAdapter->SampleRate / AudioCore.SampleRate[AudioIO->Net]));
			}
			MCUCircular_Config(&SrcAdapter->SrcBufHandler, AdaptBuf, SRC_FIFO_SIZE(SINKFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels);
			Sink->AdaptBuf = AdaptBuf;
			Sink->SrcAdapter = SrcAdapter;
			Sink->AdjAdapter = NULL;
			break;
		}

		case SRA_ONLY:
		{
			SRA_ADAPTER *AdjAdapter;
			UsedSize = SRA_FIFO_SIZE(SINKFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels;
			if(AudioIO->Resident)
			{
				AdaptBuf = osPortMalloc(UsedSize);
			}
			else
			{
				AdaptBuf = osPortMalloc(UsedSize);
			}
			if(AdaptBuf == NULL
#ifdef AUDIO_CORE_DEBUG
					|| AudioIO->HighLevelCent == 0
					|| AudioIO->Depth == 0
#endif
							)
			{
				DBG("SRA set error!\n");
				return FALSE;
			}
			memset(AdaptBuf, 0, UsedSize);
			Sink->PcmOutBuf = roboeffect_get_sink_buffer(AudioEffect[AudioCore.AudioSink[Index].Net].context_memory, AudioCoreSinkToRoboeffect(Index));
			if(Sink->PcmOutBuf == NULL)
			{
				Sink->PcmOutBuf = (PCM_DATA_TYPE *)osPortMalloc(SINKFRAME(Index) * sizeof(PCM_DATA_TYPE) * 2);
				if(Sink->PcmOutBuf == NULL)
				{
					osPortFree(AdaptBuf);
					return FALSE;
				}
				Sink->PcmBufFlag = 1;
			}
			AdjAdapter = (SRA_ADAPTER *)osPortMalloc(sizeof(SRA_ADAPTER));
			if(AdjAdapter == NULL)
			{
				osPortFree(AdaptBuf);
				return FALSE;
			}
			memset(AdjAdapter, 0, sizeof(SRA_ADAPTER));

			//sra_init(&AdjAdapter->SraCt, AudioIO->Channels);
			resampler_farrow_init(&AdjAdapter->SraResFarCt, AudioIO->Channels, POLYNOMIAL_ORDER);
			MCUCircular_Config(&AdjAdapter->SraBufHandler, AdaptBuf, SRA_FIFO_SIZE(SINKFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels);
			AdjAdapter->HighLevelCent = AudioIO->HighLevelCent;
			AdjAdapter->LowLevelCent = AudioIO->LowLevelCent;
			AdjAdapter->Depth = AudioIO->Depth;
			Sink->AdaptBuf = AdaptBuf;
			Sink->AdjAdapter = (void *)AdjAdapter;
			Sink->SrcAdapter = NULL;
			break;
		}

		case CLK_ADJUST_ONLY:
		{
			CLK_ADJUST_ADAPTER *AdjAdapter;
			Sink->PcmOutBuf = roboeffect_get_sink_buffer(AudioEffect[AudioCore.AudioSink[Index].Net].context_memory, AudioCoreSinkToRoboeffect(Index));
			if(Sink->PcmOutBuf == NULL)
			{
				Sink->PcmOutBuf = (PCM_DATA_TYPE *)osPortMalloc(SINKFRAME(Index) * sizeof(PCM_DATA_TYPE) * 2);
				if(Sink->PcmOutBuf == NULL)
				{
					return FALSE;
				}
				Sink->PcmBufFlag = 1;
			}
			if(Sink->PcmOutBuf == NULL
#ifdef AUDIO_CORE_DEBUG
					|| AudioIO->HighLevelCent == 0
					|| AudioIO->Depth == 0
#endif
									)
			{
				DBG("Clkadj set error!\n");
				return FALSE;
			}

			AdjAdapter = (CLK_ADJUST_ADAPTER *)osPortMalloc(sizeof(CLK_ADJUST_ADAPTER));
			if(AdjAdapter == NULL)
			{
//				osPortFree(Sink->PcmOutBuf);
				return FALSE;
			}
			memset(AdjAdapter, 0, sizeof(CLK_ADJUST_ADAPTER));

			AdjAdapter->HighLevelCent = AudioIO->HighLevelCent;
			AdjAdapter->LowLevelCent = AudioIO->LowLevelCent;
			AdjAdapter->Depth = AudioIO->Depth;
			Sink->AdaptBuf = NULL;
			Sink->AdjAdapter = (void *)AdjAdapter;
			Sink->SrcAdapter = NULL;
			break;
		}

		case SRC_SRA:
		{
			SRC_ADAPTER *SrcAdapter;
			SRA_ADAPTER *AdjAdapter;
			UsedSize = SRA_FIFO_SIZE(SINKFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels
					 + SRC_FIFO_SIZE(SINKFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels;
			if(AudioIO->Resident)
			{
				AdaptBuf = osPortMalloc(UsedSize);
			}
			else
			{
				AdaptBuf = osPortMalloc(UsedSize);
			}
			if(AdaptBuf == NULL
#ifdef AUDIO_CORE_DEBUG
					|| AudioIO->SampleRate == 0
					|| AudioIO->HighLevelCent == 0
					|| AudioIO->Depth == 0
#endif
							)
			{
				DBG("Srcsra set error!\n");
				return FALSE;
			}
			memset(AdaptBuf, 0, UsedSize);
			Sink->PcmOutBuf = roboeffect_get_sink_buffer(AudioEffect[AudioCore.AudioSink[Index].Net].context_memory, AudioCoreSinkToRoboeffect(Index));
			if(Sink->PcmOutBuf == NULL)
			{
				Sink->PcmOutBuf = (PCM_DATA_TYPE *)osPortMalloc(SINKFRAME(Index) * sizeof(PCM_DATA_TYPE) * 2);
				if(Sink->PcmOutBuf == NULL)
				{
					osPortFree(AdaptBuf);
					return FALSE;
				}
				Sink->PcmBufFlag = 1;
			}
			Sink->AdjAdapter = osPortMalloc(sizeof(SRA_ADAPTER) + sizeof(SRC_ADAPTER));
			if(Sink->AdjAdapter == NULL)
			{
//				osPortFree(Sink->PcmOutBuf);
				return FALSE;
			}
			memset(Sink->AdjAdapter, 0, sizeof(SRA_ADAPTER) + sizeof(SRC_ADAPTER));
			AdjAdapter = (SRA_ADAPTER *)Sink->AdjAdapter;
			SrcAdapter = (SRC_ADAPTER *)(Sink->AdjAdapter + sizeof(SRA_ADAPTER));

			//sra_init(&AdjAdapter->SraCt, AudioIO->Channels);
			resampler_farrow_init(&AdjAdapter->SraResFarCt, AudioIO->Channels, POLYNOMIAL_ORDER);
			MCUCircular_Config(&AdjAdapter->SraBufHandler, AdaptBuf, SRA_FIFO_SIZE(SINKFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels);
			AdjAdapter->HighLevelCent = AudioIO->HighLevelCent;
			AdjAdapter->LowLevelCent = AudioIO->LowLevelCent;
			AdjAdapter->Depth = AudioIO->Depth;
			Sink->AdjAdapter = (void *)AdjAdapter;

			SrcAdapter->SampleRate = AudioIO->SampleRate;
			if(SrcAdapter->SampleRate != AudioCore.SampleRate[AudioIO->Net])
			{
				resampler_polyphase_init(&SrcAdapter->SrcCt, AudioIO->Channels, GetRatioEnum(1000 * SrcAdapter->SampleRate / AudioCore.SampleRate[AudioIO->Net]));
			}
			MCUCircular_Config(&SrcAdapter->SrcBufHandler,
					AdaptBuf + SRA_FIFO_SIZE(SINKFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels,
								SRC_FIFO_SIZE(SINKFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels);
			Sink->AdaptBuf = AdaptBuf;
			Sink->SrcAdapter = SrcAdapter;
			break;
		}

		case SRC_ADJUST:
		{
			CLK_ADJUST_ADAPTER *AdjAdapter;
			SRC_ADAPTER *SrcAdapter;
			UsedSize = SRC_FIFO_SIZE(SINKFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels;
			if(AudioIO->Resident)
			{
				AdaptBuf = osPortMalloc(UsedSize);
			}
			else
			{
				AdaptBuf = osPortMalloc(UsedSize);
			}
			if(AdaptBuf == NULL
#ifdef AUDIO_CORE_DEBUG
					|| AudioIO->SampleRate == 0
					|| AudioIO->HighLevelCent == 0
					|| AudioIO->Depth == 0
#endif
								)
			{
				DBG("Srcadj set error!\n");
				return FALSE;
			}
			memset(AdaptBuf, 0, UsedSize);
			Sink->PcmOutBuf = roboeffect_get_sink_buffer(AudioEffect[AudioCore.AudioSink[Index].Net].context_memory, AudioCoreSinkToRoboeffect(Index));
			if(Sink->PcmOutBuf == NULL)
			{
				Sink->PcmOutBuf = (PCM_DATA_TYPE *)osPortMalloc(SINKFRAME(Index) * sizeof(PCM_DATA_TYPE) * 2);
				if(Sink->PcmOutBuf == NULL)
				{
					osPortFree(AdaptBuf);
					return FALSE;
				}
				Sink->PcmBufFlag = 1;
			}
			Sink->AdjAdapter = osPortMalloc(sizeof(CLK_ADJUST_ADAPTER) + sizeof(SRC_ADAPTER));
			if(Sink->AdjAdapter == NULL)
			{
				osPortFree(AdaptBuf);
				return FALSE;
			}
			memset(Sink->AdjAdapter, 0, sizeof(CLK_ADJUST_ADAPTER) + sizeof(SRC_ADAPTER));
			AdjAdapter = (CLK_ADJUST_ADAPTER *)Sink->AdjAdapter;
			SrcAdapter = (SRC_ADAPTER *)((uint8_t*)Sink->AdjAdapter + sizeof(CLK_ADJUST_ADAPTER));


			AdjAdapter->HighLevelCent = AudioIO->HighLevelCent;
			AdjAdapter->LowLevelCent = AudioIO->LowLevelCent;
			AdjAdapter->Depth = AudioIO->Depth;
			Sink->AdjAdapter = (void *)AdjAdapter;

			SrcAdapter->SampleRate = AudioIO->SampleRate;
			if(SrcAdapter->SampleRate != AudioCore.SampleRate[AudioIO->Net])
			{
				resampler_polyphase_init(&SrcAdapter->SrcCt, AudioIO->Channels, GetRatioEnum(1000 * SrcAdapter->SampleRate / AudioCore.SampleRate[AudioIO->Net]));
			}
			MCUCircular_Config(&SrcAdapter->SrcBufHandler, AdaptBuf, SRC_FIFO_SIZE(SINKFRAME(Index)) * sizeof(PCM_DATA_TYPE) * AudioIO->Channels);
			Sink->AdaptBuf = AdaptBuf;
			Sink->SrcAdapter = SrcAdapter;
			break;
		}
	}

	Sink->InitFlag = TRUE;
	return TRUE;
}

void AudioCoreSinkDeinit(uint8_t Index)
{

	AudioCoreSink * Sink = &AudioCore.AudioSink[Index];
#ifdef AUDIO_CORE_DEBUG
	if(!AudioCoreSinkIsInit(Index))
		return;
#endif
	Sink->Enable = FALSE;
	Sink->Sync = FALSE;
	SINK_BIT_DIS(AudioCore.FrameReady, Index);
	Sink->DataSetFunc = NULL;
	Sink->SpaceLenFunc = NULL;
	if(Sink->PcmBufFlag)
		osPortFree(Sink->PcmOutBuf);
	Sink->PcmBufFlag = 0;
	Sink->PcmOutBuf = NULL;
	switch(Sink->Adapt)
	{
		case   	SRC_ONLY:
			osPortFree(Sink->AdaptBuf);
			Sink->AdaptBuf = NULL;
			osPortFree(Sink->SrcAdapter);
			Sink->SrcAdapter = NULL;
			break;
		case	SRA_ONLY:
			osPortFree(Sink->AdaptBuf);
			Sink->AdaptBuf = NULL;
			osPortFree(Sink->AdjAdapter);
			Sink->AdjAdapter = NULL;
			break;
		case 	CLK_ADJUST_ONLY:
			osPortFree(Sink->AdjAdapter);
			Sink->AdjAdapter = NULL;
			break;
		case 	SRC_SRA:
		case 	SRC_ADJUST:
			osPortFree(Sink->AdaptBuf);
			Sink->AdaptBuf = NULL;
			osPortFree(Sink->AdjAdapter);
			Sink->SrcAdapter = NULL;
			Sink->AdjAdapter = NULL;
			break;
		default:
			break;
	}

	Sink->InitFlag = FALSE;
}

#ifdef CFG_AUDIO_WIDTH_24BIT
void AudioCorePcmDataBitWidthConv(void *PcmBuf,uint16_t dataSize,PCM_DATA_WIDTH BitWidth)
{
	uint32_t n;
	int32_t	*PcmBuf32 = (int32_t *)PcmBuf;
	int16_t	*PcmBuf16 = (int16_t *)PcmBuf;

	if(BitWidth == PCM_DATA_16BIT_WIDTH) //16bit转成24bit
	{
		PcmBuf16 += (dataSize / sizeof(int16_t));
		memcpy(PcmBuf16 ,PcmBuf, dataSize);
		for(n=0; n < dataSize / sizeof(int16_t); n++)
		{
			PcmBuf32[n] = PcmBuf16[n] << 8;
		}
	}
	else	//24bit转成16bit
	{
		for(n=0; n < dataSize / sizeof(int32_t); n++)
		{
			PcmBuf16[n] = __nds32__clips(((int32_t)PcmBuf32[n] + 128) >> 8, (16) - 1);
		}
	}
}
#endif

bool AudioCoreDataSpaceCheck(void)
{
	uint16_t Index;
	bool	 flag;
	AudioCoreSource *Source;
	AudioCoreSink *Sink;

	flag = FALSE;
	for(Index = 0; Index < AUDIO_CORE_SOURCE_MAX_NUM; Index++)
	{
		Source = &AudioCore.AudioSource[Index];
		if(Source->Enable
#ifdef AUDIO_CORE_DEBUG
			&& Source->DataGetFunc
			&& Source->DataLenFunc
#endif
			)
		{
			if(Index == REMIND_SOURCE_NUM) 	//提示音解码在DataLenFunc里进行，屏蔽这个通道防止解码器重入
				continue;
			if(Source->DataLenFunc() < SOURCEFRAME(Index))
			{
				return FALSE;
			}
			flag = TRUE; //Source有数据
		}
	}

	if(flag)	//所有的Source都有数据
	{
		flag = FALSE;
		for(Index = 0; Index < AUDIO_CORE_SINK_MAX_NUM; Index++)
		{
			Sink = &AudioCore.AudioSink[Index];
			if(Sink->Enable
	#ifdef AUDIO_CORE_DEBUG
				&& Sink->DataSetFunc
				&& Sink->SpaceLenFunc)
	#endif
			{
				if(Sink->SpaceLenFunc() < SINKFRAME(Index))
				{
					return FALSE;
				}
				flag = TRUE; //sink有剩余空间
			}
		}
	}

	return flag;
}

void AudioCoreIOLenProcess(void)
{
	uint16_t Index;
	uint32_t SrcAudioLen, SraAudioLen;//Sample
	AudioCoreSource *Source;
	AudioCoreSink *Sink;
	for(Index = 0; Index < AUDIO_CORE_SOURCE_MAX_NUM; Index++)
	{
		Source = &AudioCore.AudioSource[Index];
		if(Source->Enable
#ifdef AUDIO_CORE_DEBUG
			&& Source->DataGetFunc
			&& Source->DataLenFunc
#endif
			)
		{

			switch(Source->Adapt)
			{
				case STD:
				case CLK_ADJUST_ONLY:
					if(Source->DataLenFunc() >= SOURCEFRAME(Index))
					{
						SOURCE_BIT_EN(AudioCore.FrameReady, Index);
					}
					else
					{
						SOURCE_BIT_DIS(AudioCore.FrameReady, Index);
					}
					break;
				case SRC_ONLY:
				case SRC_ADJUST:
				{
					SRC_ADAPTER * SrcAdapter = Source->SrcAdapter;
					uint32_t     PcmDataLen = 	2 * Source->Channels;
#ifdef CFG_AUDIO_WIDTH_24BIT
					if(Source->BitWidth == PCM_DATA_24BIT_WIDTH)
						PcmDataLen *= 2;
#endif
					do{
#ifdef CFG_FUNC_MIXER_SRC_EN
						if(SrcAdapter->SampleRate != AudioCore.SampleRate[AudioCore.AudioSource[Index].Net])
						{
							SrcAudioLen = SRCValidLenGet(SrcAdapter->SampleRate,
												Source->DataLenFunc(),
												AudioCore.SampleRate[AudioCore.AudioSource[Index].Net],
												MCUCircular_GetSpaceLen(&SrcAdapter->SrcBufHandler) / PcmDataLen - 1);
							if(SrcAudioLen)
							{
								SrcAudioLen = Source->DataGetFunc(AudioCore.AdaptIn, SrcAudioLen);
						#ifdef CFG_AUDIO_WIDTH_24BIT
							if(Source->BitWidth == PCM_DATA_24BIT_WIDTH)
								SrcAudioLen = resampler_polyphase_apply24(&SrcAdapter->SrcCt, (int32_t *)AudioCore.AdaptIn, (int32_t *)AudioCore.AdaptOut, SrcAudioLen);
							else
						#endif
								SrcAudioLen = resampler_polyphase_apply(&SrcAdapter->SrcCt, (int16_t *)AudioCore.AdaptIn, (int16_t *)AudioCore.AdaptOut, SrcAudioLen);
								MCUCircular_PutData(&SrcAdapter->SrcBufHandler, AudioCore.AdaptOut, SrcAudioLen * PcmDataLen);
							}
						}
						else
#endif
						{
							SrcAudioLen = Min3(Source->DataLenFunc(),
												MCUCircular_GetSpaceLen(&SrcAdapter->SrcBufHandler) / PcmDataLen - 1,
												sizeof(AudioCore.AdaptOut)/PcmDataLen);
							if(SrcAudioLen)
							{
								SrcAudioLen = Source->DataGetFunc(AudioCore.AdaptOut, SrcAudioLen);
								MCUCircular_PutData(&SrcAdapter->SrcBufHandler, AudioCore.AdaptOut, SrcAudioLen * PcmDataLen);
							}
						}
						if(MCUCircular_GetDataLen(&SrcAdapter->SrcBufHandler) / (PcmDataLen) >= SOURCEFRAME(Index))
						{
							SOURCE_BIT_EN(AudioCore.FrameReady, Index);
						}
						else
						{
							SOURCE_BIT_DIS(AudioCore.FrameReady, Index);
						}
					}while(!SOURCE_BIT_GET(AudioCore.FrameReady, Index) && SrcAudioLen);
					break;
				}
				case SRA_ONLY:
				{
					SRA_ADAPTER * AdjAdapter = (SRA_ADAPTER *)Source->AdjAdapter;
					uint32_t     PcmDataLen = 	2 * Source->Channels;
#ifdef CFG_AUDIO_WIDTH_24BIT
					if(Source->BitWidth == PCM_DATA_24BIT_WIDTH)
						PcmDataLen *= 2;
#endif
					if(AdjAdapter->Enable)
					{
						do{
							SraAudioLen = Source->DataLenFunc();
							SraAudioLen = SRAValidLenGet(SraAudioLen, MCUCircular_GetSpaceLen(&AdjAdapter->SraBufHandler) / PcmDataLen);
							if(SraAudioLen)
							{
								SraAudioLen = Source->DataGetFunc(AudioCore.AdaptIn, SraAudioLen);
								//sra_apply(&AdjAdapter->SraCt, AudioCore.AdaptIn, AudioCore.AdaptOut, AdjAdapter->AdjustVal);
							#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Source->BitWidth == PCM_DATA_24BIT_WIDTH)
									resampler_farrow_apply24(&AdjAdapter->SraResFarCt, (int32_t *)AudioCore.AdaptIn, (int32_t *)AudioCore.AdaptOut, 128, 128 + AdjAdapter->AdjustVal);
								else
							#endif
									resampler_farrow_apply(&AdjAdapter->SraResFarCt, (int16_t *)AudioCore.AdaptIn, (int16_t *)AudioCore.AdaptOut, 128, 128 + AdjAdapter->AdjustVal);
								SraAudioLen += AdjAdapter->AdjustVal;
								AdjAdapter->AdjustVal = 0;
								MCUCircular_PutData(&AdjAdapter->SraBufHandler, AudioCore.AdaptOut, SraAudioLen * PcmDataLen);
							}

						}while(!SOURCE_BIT_GET(AudioCore.FrameReady, Index) && SraAudioLen);
					}
					else
					{
						SraAudioLen = Min3(Source->DataLenFunc(),
											MCUCircular_GetSpaceLen(&AdjAdapter->SraBufHandler) / PcmDataLen - 1,
											sizeof(AudioCore.AdaptOut)/PcmDataLen);
						if(SraAudioLen)
						{
							SraAudioLen = Source->DataGetFunc(AudioCore.AdaptOut, SraAudioLen);
							MCUCircular_PutData(&AdjAdapter->SraBufHandler, AudioCore.AdaptOut, SraAudioLen * PcmDataLen);
						}
					}
					if(MCUCircular_GetDataLen(&AdjAdapter->SraBufHandler) / PcmDataLen >= SOURCEFRAME(Index))
					{
						SOURCE_BIT_EN(AudioCore.FrameReady, Index);
					}
					else
					{
						SOURCE_BIT_DIS(AudioCore.FrameReady, Index);
					}
					break;
				}
				case SRC_SRA:
				{
					SRC_ADAPTER * SrcAdapter = Source->SrcAdapter;
					SRA_ADAPTER * AdjAdapter = (SRA_ADAPTER *)Source->AdjAdapter;
					uint32_t     PcmDataLen = 	2 * Source->Channels;
#ifdef CFG_AUDIO_WIDTH_24BIT
					if(Source->BitWidth == PCM_DATA_24BIT_WIDTH)
						PcmDataLen *= 2;
#endif
					do{
#ifdef CFG_FUNC_MIXER_SRC_EN
						if(SrcAdapter->SampleRate != AudioCore.SampleRate[AudioCore.AudioSource[Index].Net])
						{
							SrcAudioLen = SRCValidLenGet(SrcAdapter->SampleRate,
												Source->DataLenFunc(),
												AudioCore.SampleRate[AudioCore.AudioSource[Index].Net],
												MCUCircular_GetSpaceLen(&SrcAdapter->SrcBufHandler) / PcmDataLen);
							if(SrcAudioLen)
							{
								SrcAudioLen = Source->DataGetFunc(AudioCore.AdaptIn, SrcAudioLen);
							#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Source->BitWidth == PCM_DATA_24BIT_WIDTH)
									SrcAudioLen = resampler_polyphase_apply24(&SrcAdapter->SrcCt, (int32_t *)AudioCore.AdaptIn, (int32_t *)AudioCore.AdaptOut, SrcAudioLen);
								else
							#endif
									SrcAudioLen = resampler_polyphase_apply(&SrcAdapter->SrcCt, (int16_t *)AudioCore.AdaptIn, (int16_t *)AudioCore.AdaptOut, SrcAudioLen);
								MCUCircular_PutData(&SrcAdapter->SrcBufHandler, AudioCore.AdaptOut, SrcAudioLen * PcmDataLen);
							}
						}
						else
#endif
						{
							SrcAudioLen = Min3(Source->DataLenFunc(),
												MCUCircular_GetSpaceLen(&SrcAdapter->SrcBufHandler) / PcmDataLen - 1,
												sizeof(AudioCore.AdaptOut)/PcmDataLen);
							if(SrcAudioLen)
							{
								SrcAudioLen = Source->DataGetFunc(AudioCore.AdaptOut, SrcAudioLen);
								MCUCircular_PutData(&SrcAdapter->SrcBufHandler, AudioCore.AdaptOut, SrcAudioLen * PcmDataLen);
							}
						}
						if(AdjAdapter->Enable)
						{

							SraAudioLen = SRAValidLenGet(MCUCircular_GetDataLen(&SrcAdapter->SrcBufHandler) / PcmDataLen, MCUCircular_GetSpaceLen(&AdjAdapter->SraBufHandler) / PcmDataLen);
							if(SraAudioLen)
							{

								SraAudioLen = MCUCircular_GetData(&SrcAdapter->SrcBufHandler, AudioCore.AdaptIn, SraAudioLen * PcmDataLen) / PcmDataLen;
								//sra_apply(&AdjAdapter->SraCt, AudioCore.AdaptIn, AudioCore.AdaptOut, AdjAdapter->AdjustVal);
							#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Source->BitWidth == PCM_DATA_24BIT_WIDTH)
									resampler_farrow_apply24(&AdjAdapter->SraResFarCt, (int32_t *)AudioCore.AdaptIn, (int32_t *)AudioCore.AdaptOut, 128, 128 + AdjAdapter->AdjustVal);
								else
							#endif
									resampler_farrow_apply(&AdjAdapter->SraResFarCt, (int16_t *)AudioCore.AdaptIn, (int16_t *)AudioCore.AdaptOut, 128, 128 + AdjAdapter->AdjustVal);
								SraAudioLen += AdjAdapter->AdjustVal;
								AdjAdapter->AdjustVal = 0;
								MCUCircular_PutData(&AdjAdapter->SraBufHandler, AudioCore.AdaptOut, SraAudioLen * PcmDataLen);
							}
						}
						else
						{
							SraAudioLen = Min3(MCUCircular_GetDataLen(&SrcAdapter->SrcBufHandler) / PcmDataLen,
												MCUCircular_GetSpaceLen(&AdjAdapter->SraBufHandler) / PcmDataLen - 1,
												sizeof(AudioCore.AdaptOut)/PcmDataLen);
							if(SraAudioLen)
							{
								SraAudioLen = MCUCircular_GetData(&SrcAdapter->SrcBufHandler, AudioCore.AdaptOut, SraAudioLen * PcmDataLen) / PcmDataLen;
								MCUCircular_PutData(&AdjAdapter->SraBufHandler, AudioCore.AdaptOut, SraAudioLen * PcmDataLen);
							}
						}
						if(MCUCircular_GetDataLen(&AdjAdapter->SraBufHandler) / PcmDataLen >= SOURCEFRAME(Index))
						{
							SOURCE_BIT_EN(AudioCore.FrameReady, Index);
						}
						else
						{
							SOURCE_BIT_DIS(AudioCore.FrameReady, Index);
						}
					}while(!SOURCE_BIT_GET(AudioCore.FrameReady, Index) && (SrcAudioLen || SraAudioLen));
					break;
				}
			}
		}
	}

	for(Index = 0; Index < AUDIO_CORE_SINK_MAX_NUM; Index++)
	{
		Sink = &AudioCore.AudioSink[Index];
		if(Sink->Enable
#ifdef AUDIO_CORE_DEBUG
			&& Sink->DataSetFunc
			&& Sink->SpaceLenFunc)
#endif
		{

			switch(Sink->Adapt)
			{
				case STD:
				case CLK_ADJUST_ONLY:
					if(Sink->SpaceLenFunc() >= SINKFRAME(Index))
					{
						SINK_BIT_EN(AudioCore.FrameReady, Index);
					}
					else
					{
						SINK_BIT_DIS(AudioCore.FrameReady, Index);
					}
					break;
				case SRC_ONLY:
				case SRC_ADJUST:
				{
					SRC_ADAPTER * SrcAdapter = Sink->SrcAdapter;
					uint32_t      PcmDataLen = 2 * Sink->Channels;
#ifdef CFG_AUDIO_WIDTH_24BIT
					if(Sink->BitWidth == PCM_DATA_24BIT_WIDTH)
						PcmDataLen *= 2;
#endif
					do{
#ifdef CFG_FUNC_MIXER_SRC_EN
						if(SrcAdapter->SampleRate != AudioCore.SampleRate[AudioCore.AudioSink[Index].Net])
						{
							SrcAudioLen = SRCValidLenGet(AudioCore.SampleRate[AudioCore.AudioSink[Index].Net],
												MCUCircular_GetDataLen(&SrcAdapter->SrcBufHandler) / PcmDataLen,
												SrcAdapter->SampleRate,
												Sink->SpaceLenFunc() - 1);
							if(SrcAudioLen)
							{
								SrcAudioLen = MCUCircular_GetData(&SrcAdapter->SrcBufHandler, AudioCore.AdaptIn, SrcAudioLen * PcmDataLen) / PcmDataLen;
					#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Sink->BitWidth == PCM_DATA_24BIT_WIDTH)
									SrcAudioLen = resampler_polyphase_apply24(&SrcAdapter->SrcCt, (int32_t *)AudioCore.AdaptIn, (int32_t *)AudioCore.AdaptOut, SrcAudioLen);
								else
					#endif
									SrcAudioLen = resampler_polyphase_apply(&SrcAdapter->SrcCt, (int16_t *)AudioCore.AdaptIn, (int16_t *)AudioCore.AdaptOut, SrcAudioLen);
					#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Sink->BitWidthConvFlag)
								{
									AudioCorePcmDataBitWidthConv(AudioCore.AdaptOut,SrcAudioLen * PcmDataLen,Sink->BitWidth);
								}
					#endif
								Sink->DataSetFunc(AudioCore.AdaptOut, SrcAudioLen);
							}
						}
						else
#endif
						{
							SrcAudioLen = Min3(MCUCircular_GetDataLen(&SrcAdapter->SrcBufHandler) / PcmDataLen,
												Sink->SpaceLenFunc() - 1,
												sizeof(AudioCore.AdaptOut)/ PcmDataLen);
							if(SrcAudioLen)
							{
								SrcAudioLen = MCUCircular_GetData(&SrcAdapter->SrcBufHandler, AudioCore.AdaptOut, SrcAudioLen * PcmDataLen) / PcmDataLen;
					#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Sink->BitWidthConvFlag)
								{
									AudioCorePcmDataBitWidthConv(AudioCore.AdaptOut,SrcAudioLen * PcmDataLen,Sink->BitWidth);
								}
					#endif
								Sink->DataSetFunc(AudioCore.AdaptOut, SrcAudioLen);
							}
						}
						if(MCUCircular_GetSpaceLen(&SrcAdapter->SrcBufHandler) / PcmDataLen >= SINKFRAME(Index))
						{
							SINK_BIT_EN(AudioCore.FrameReady, Index);
						}
						else
						{
							SINK_BIT_DIS(AudioCore.FrameReady, Index);
						}
					}while(!SINK_BIT_GET(AudioCore.FrameReady, Index) && SrcAudioLen);
					break;
				}
				case SRA_ONLY:
				{
					SRA_ADAPTER * AdjAdapter = (SRA_ADAPTER *)Sink->AdjAdapter;
					uint32_t      PcmDataLen = 2 * Sink->Channels;
#ifdef CFG_AUDIO_WIDTH_24BIT
					if(Sink->BitWidth == PCM_DATA_24BIT_WIDTH)
						PcmDataLen *= 2;
#endif
					do{
						if(AdjAdapter->Enable)
						{
							SraAudioLen = Sink->SpaceLenFunc();
							SraAudioLen = SRAValidLenGet(MCUCircular_GetDataLen(&AdjAdapter->SraBufHandler) / PcmDataLen, SraAudioLen);
							if(SraAudioLen)
							{
								SraAudioLen = MCUCircular_GetData(&AdjAdapter->SraBufHandler, AudioCore.AdaptIn, SraAudioLen * PcmDataLen) / PcmDataLen;
								//sra_apply(&AdjAdapter->SraCt, AudioCore.AdaptIn, AudioCore.AdaptOut, AdjAdapter->AdjustVal);
			#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Sink->BitWidth == PCM_DATA_24BIT_WIDTH)
									resampler_farrow_apply24(&AdjAdapter->SraResFarCt, (int32_t *)AudioCore.AdaptIn, (int32_t *)AudioCore.AdaptOut, 128, 128 + AdjAdapter->AdjustVal);
								else
			#endif
									resampler_farrow_apply(&AdjAdapter->SraResFarCt, (int16_t *)AudioCore.AdaptIn, (int16_t *)AudioCore.AdaptOut, 128, 128 + AdjAdapter->AdjustVal);
								SraAudioLen += AdjAdapter->AdjustVal;
								AdjAdapter->AdjustVal = 0;
			#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Sink->BitWidthConvFlag)
								{
									AudioCorePcmDataBitWidthConv(AudioCore.AdaptOut,SraAudioLen * PcmDataLen,Sink->BitWidth);
								}
			#endif
								Sink->DataSetFunc(AudioCore.AdaptOut, SraAudioLen);
							}
						}
						else
						{
							SraAudioLen = Min3(MCUCircular_GetDataLen(&AdjAdapter->SraBufHandler) / PcmDataLen,
												Sink->SpaceLenFunc() - 1,
											sizeof(AudioCore.AdaptOut)/PcmDataLen);
							if(SraAudioLen)
							{
								SraAudioLen = MCUCircular_GetData(&AdjAdapter->SraBufHandler, AudioCore.AdaptOut, SraAudioLen * PcmDataLen) / PcmDataLen;
			#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Sink->BitWidthConvFlag)
								{
									AudioCorePcmDataBitWidthConv(AudioCore.AdaptOut,SraAudioLen * PcmDataLen,Sink->BitWidth);
								}
			#endif
								Sink->DataSetFunc(AudioCore.AdaptOut, SraAudioLen);
							}
						}

						if(MCUCircular_GetSpaceLen(&AdjAdapter->SraBufHandler) / PcmDataLen >= SINKFRAME(Index))
						{
							SINK_BIT_EN(AudioCore.FrameReady, Index);
						}
						else
						{
							SINK_BIT_DIS(AudioCore.FrameReady, Index);
						}
					}while(!SINK_BIT_GET(AudioCore.FrameReady, Index) && SraAudioLen);
					break;
				}
				case SRC_SRA:
				{
					SRA_ADAPTER * AdjAdapter = (SRA_ADAPTER *)Sink->AdjAdapter;
					SRC_ADAPTER * SrcAdapter = Sink->SrcAdapter;
					uint32_t      PcmDataLen = 2 * Sink->Channels;
#ifdef CFG_AUDIO_WIDTH_24BIT
					if(Sink->BitWidth == PCM_DATA_24BIT_WIDTH)
						PcmDataLen *= 2;
#endif
					do{
						if(AdjAdapter->Enable)
						{
							SraAudioLen = SRAValidLenGet(MCUCircular_GetDataLen(&AdjAdapter->SraBufHandler) / PcmDataLen,
														MCUCircular_GetSpaceLen(&SrcAdapter->SrcBufHandler) / PcmDataLen);
							if(SraAudioLen)
							{
								SraAudioLen = MCUCircular_GetData(&AdjAdapter->SraBufHandler, AudioCore.AdaptIn, SraAudioLen * PcmDataLen) / PcmDataLen;
								//sra_apply(&AdjAdapter->SraCt, AudioCore.AdaptIn, AudioCore.AdaptOut, AdjAdapter->AdjustVal);
				#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Sink->BitWidth == PCM_DATA_24BIT_WIDTH)
									resampler_farrow_apply24(&AdjAdapter->SraResFarCt, (int32_t *)AudioCore.AdaptIn, (int32_t *)AudioCore.AdaptOut, 128, 128 + AdjAdapter->AdjustVal);
								else
				#endif
									resampler_farrow_apply(&AdjAdapter->SraResFarCt, (int16_t *)AudioCore.AdaptIn, (int16_t *)AudioCore.AdaptOut, 128, 128 + AdjAdapter->AdjustVal);
								SraAudioLen += AdjAdapter->AdjustVal;
								AdjAdapter->AdjustVal = 0;
								MCUCircular_PutData(&SrcAdapter->SrcBufHandler, AudioCore.AdaptOut, SraAudioLen * PcmDataLen);
							}
						}
						else
						{
							SraAudioLen = Min3(MCUCircular_GetDataLen(&AdjAdapter->SraBufHandler) / PcmDataLen,
												MCUCircular_GetSpaceLen(&SrcAdapter->SrcBufHandler) / PcmDataLen - 1,
												sizeof(AudioCore.AdaptOut)/PcmDataLen);
							if(SraAudioLen)
							{
								SraAudioLen = MCUCircular_GetData(&AdjAdapter->SraBufHandler, AudioCore.AdaptOut, SraAudioLen * PcmDataLen) / PcmDataLen;
								MCUCircular_GetData(&SrcAdapter->SrcBufHandler, AudioCore.AdaptOut, SraAudioLen * PcmDataLen);
							}
						}
						if(MCUCircular_GetSpaceLen(&AdjAdapter->SraBufHandler) / PcmDataLen >= SINKFRAME(Index))
						{
							SINK_BIT_EN(AudioCore.FrameReady, Index);
						}
						else
						{
							SINK_BIT_DIS(AudioCore.FrameReady, Index);
						}
#ifdef CFG_FUNC_MIXER_SRC_EN
						if(SrcAdapter->SampleRate != AudioCore.SampleRate[AudioCore.AudioSink[Index].Net])
						{
							SrcAudioLen = SRCValidLenGet(AudioCore.SampleRate[AudioCore.AudioSink[Index].Net],
												MCUCircular_GetDataLen(&SrcAdapter->SrcBufHandler) / PcmDataLen,
												SrcAdapter->SampleRate,
												Sink->SpaceLenFunc());
							if(SrcAudioLen)
							{
								SrcAudioLen= MCUCircular_GetData(&SrcAdapter->SrcBufHandler, AudioCore.AdaptIn, SrcAudioLen * PcmDataLen) / PcmDataLen;
						#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Sink->BitWidth == PCM_DATA_24BIT_WIDTH)
									SrcAudioLen = resampler_polyphase_apply24(&SrcAdapter->SrcCt, (int32_t *)AudioCore.AdaptIn, (int32_t *)AudioCore.AdaptOut, SrcAudioLen);
								else
						#endif
									SrcAudioLen = resampler_polyphase_apply(&SrcAdapter->SrcCt, (int16_t *)AudioCore.AdaptIn, (int16_t *)AudioCore.AdaptOut, SrcAudioLen);
						#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Sink->BitWidthConvFlag)
								{
									AudioCorePcmDataBitWidthConv(AudioCore.AdaptOut,SrcAudioLen * PcmDataLen,Sink->BitWidth);
								}
						#endif
								Sink->DataSetFunc(AudioCore.AdaptOut, SrcAudioLen);
							}
						}
						else
#endif
						{
							SrcAudioLen = Min3(MCUCircular_GetDataLen(&SrcAdapter->SrcBufHandler) / PcmDataLen,
												Sink->SpaceLenFunc() - 1,
												sizeof(AudioCore.AdaptOut)/PcmDataLen);
							if(SrcAudioLen)
							{
								SrcAudioLen = MCUCircular_GetData(&SrcAdapter->SrcBufHandler, AudioCore.AdaptOut, SrcAudioLen * PcmDataLen) / PcmDataLen;
						#ifdef CFG_AUDIO_WIDTH_24BIT
								if(Sink->BitWidthConvFlag)
								{
									AudioCorePcmDataBitWidthConv(AudioCore.AdaptOut,SrcAudioLen * PcmDataLen,Sink->BitWidth);
								}
						#endif
								Sink->DataSetFunc(AudioCore.AdaptOut, SrcAudioLen);
							}
						}
					}while(!SINK_BIT_GET(AudioCore.FrameReady, Index) && (SrcAudioLen || SraAudioLen));
					break;
				}
			}
		}
	}
}

//获取adapter 数据等效存量
uint16_t AudioCoreSinkAdapterDataLenGet(uint8_t Index)
{
	AudioCoreSink *Sink = &AudioCore.AudioSink[Index];
	SRC_ADAPTER *SrcAdapter = Sink->SrcAdapter;
	uint16_t DataLen = 0;
	uint32_t PcmDataLen;

	if(!Sink->Enable)
		return 0;

	PcmDataLen = 2 * Sink->Channels;
#ifdef CFG_AUDIO_WIDTH_24BIT
	if(Sink->BitWidth == PCM_DATA_24BIT_WIDTH)
		PcmDataLen *= 2;
#endif

	switch(Sink->Adapt)
	{
		case STD:
		case CLK_ADJUST_ONLY:
			DataLen = 0;
			break;

		case SRC_ONLY:
		case SRC_ADJUST:
		{
			DataLen = MCUCircular_GetDataLen(&SrcAdapter->SrcBufHandler) / PcmDataLen;
			break;
		}

		case SRA_ONLY:
		{
			SRA_ADAPTER * AdjAdapter = (SRA_ADAPTER *)Sink->AdjAdapter;
			DataLen = MCUCircular_GetDataLen(&AdjAdapter->SraBufHandler) / PcmDataLen;
			break;
		}

		case SRC_SRA:
		{
			SRA_ADAPTER * AdjAdapter = (SRA_ADAPTER *)Sink->AdjAdapter;
			DataLen = (MCUCircular_GetDataLen(&AdjAdapter->SraBufHandler) + MCUCircular_GetDataLen(&SrcAdapter->SrcBufHandler)) / PcmDataLen;
			break;
		}
	}
	return DataLen;
}

//依据Spacelenfunc和depth，获取数据等效存量
uint32_t AudioCoreSinkDataLenGet(uint8_t Index)
{
	AudioCoreSink *Sink = &AudioCore.AudioSink[Index];
	SRC_ADAPTER *SrcAdapter = Sink->SrcAdapter;
	uint32_t DataLen = 0;

	if(!Sink->Enable)
		return 0;

	DataLen = Sink->SpaceLenFunc();
	if(Sink->Depth <= DataLen) //防(错配)溢出
	{
		DataLen = 0;
	}
	else
	{
		DataLen = Sink->Depth - DataLen;
	}

	switch(Sink->Adapt)
	{
		case SRC_ONLY:
		case SRC_SRA:
		case SRC_ADJUST:
			DataLen = ((uint64_t)AudioCore.SampleRate[AudioCore.AudioSink[Index].Net] * DataLen) / SrcAdapter->SampleRate;
			break;
		default:
			break;
	}
	return DataLen;
}

void AudioCoreSourceGet(uint8_t Index)
{
	AudioCoreSource *Source = &AudioCore.AudioSource[Index];
	SRC_ADAPTER * SrcAdapter = Source->SrcAdapter;
	uint32_t PcmDataLen;

	PcmDataLen = 2 * Source->Channels;
#ifdef CFG_AUDIO_WIDTH_24BIT
	if(Source->BitWidth == PCM_DATA_24BIT_WIDTH)
		PcmDataLen *= 2;
#endif

	switch(Source->Adapt)
	{
		case STD:
			Source->DataGetFunc((void*)Source->PcmInBuf, SOURCEFRAME(Index));
			break;
		case SRC_ONLY:
		{
			MCUCircular_GetData(&SrcAdapter->SrcBufHandler, Source->PcmInBuf, SOURCEFRAME(Index) * PcmDataLen);
			break;
		}
		case SRA_ONLY:
		{
			SRA_ADAPTER * AdjAdapter = (SRA_ADAPTER *)Source->AdjAdapter;
			MCUCircular_GetData(&AdjAdapter->SraBufHandler, Source->PcmInBuf, SOURCEFRAME(Index) * PcmDataLen);
			if(AdjAdapter->Enable)
			{
				AdjAdapter->TotalNum += Source->DataLenFunc() + MCUCircular_GetDataLen(&AdjAdapter->SraBufHandler) / PcmDataLen;
				AdjAdapter->Count++;
				if(AdjAdapter->Count >= ADJUST_PERIOD / SOURCEFRAME(Index))
				{
					uint32_t ValidDepth = AdjAdapter->Depth + AdjAdapter->SraBufHandler.BufDepth / PcmDataLen - SOURCEFRAME(Index) - SRA_BLOCK;
					AdjAdapter->AdjustVal = ADJLEVEL(AdjAdapter->TotalNum / AdjAdapter->Count,
										(ValidDepth * AdjAdapter->LowLevelCent) / 100,
										(ValidDepth * AdjAdapter->HighLevelCent) / 100);
					AdjAdapter->TotalNum = 0;
					AdjAdapter->Count = 0;
				}
			}
			break;
		}
		case CLK_ADJUST_ONLY:
		{
			CLK_ADJUST_ADAPTER * AdjAdapter = (CLK_ADJUST_ADAPTER *)Source->AdjAdapter;
			Source->DataGetFunc((void*)Source->PcmInBuf, SOURCEFRAME(Index));
			if(AdjAdapter->Enable)
			{
				AdjAdapter->TotalNum += Source->DataLenFunc();
				AdjAdapter->Count++;
				if(AdjAdapter->Count >= ADJUST_APLL_PERIOD / SOURCEFRAME(Index))
				{
					uint32_t ValidDepth = AdjAdapter->Depth - SOURCEFRAME(Index);
					ClkAdjust(AdjAdapter, ValidDepth, 1);
				}
			}
			break;
		}
		case SRC_SRA:
		{
			SRA_ADAPTER * AdjAdapter = (SRA_ADAPTER *)Source->AdjAdapter;
			MCUCircular_GetData(&AdjAdapter->SraBufHandler, Source->PcmInBuf, SOURCEFRAME(Index) * PcmDataLen);
			if(AdjAdapter->Enable)
			{
				#ifdef BT_AUDIO_AAC_ENABLE
				static int8_t  AdjustFlag = 0;//方向
				uint8_t index = btManager.btLinked_env[btManager.cur_index].a2dp_index;
				if((GetSystemMode() == ModeBtAudioPlay)
					&& (index < BT_LINK_DEV_NUM)
					&& (btManager.a2dpStreamType[index] == BT_A2DP_STREAM_TYPE_AAC))
				{
					AdjAdapter->TotalNum += GetValidFrameDataSize();
					AdjAdapter->Count++;

					if(AdjAdapter->Count >= ADJUST_PERIOD / SOURCEFRAME(Index))
					{
						AdjAdapter->TotalNum = AdjAdapter->TotalNum/AdjAdapter->Count;

						if(AdjustFlag == 0)
						{
							if(AdjAdapter->TotalNum < BT_AAC_LEVEL_LOW)
							{
								AdjustFlag = 1;//正
							}
							else if(AdjAdapter->TotalNum  > BT_AAC_LEVEL_HIGH)
							{
								AdjustFlag = -1;//负
							}
							else
							{
								AdjustFlag = 0;
							}
						}
						else if(AdjustFlag == 1)
						{
							if(AdjAdapter->TotalNum  < BT_AAC_LEVEL_LOW)
							{
								//插点
								AdjAdapter->AdjustVal = 1;
								//APP_DBG("!>!");
							}
							else
							{
								AdjustFlag = 0;
							}
						}
						else if(AdjustFlag == -1)
						{
							if(AdjAdapter->TotalNum > BT_AAC_LEVEL_HIGH)
							{
								//丢点
								AdjAdapter->AdjustVal = -1;
								//APP_DBG("!<!");
							}
							else
							{
								AdjustFlag = 0;
							}
						}

						AdjAdapter->TotalNum = 0;
						AdjAdapter->Count = 0;
					}
				}
				else
				#endif
				{
					AdjAdapter->TotalNum += (Source->DataLenFunc()
										 	 + MCUCircular_GetDataLen(&SrcAdapter->SrcBufHandler) / PcmDataLen
											 + MCUCircular_GetDataLen(&AdjAdapter->SraBufHandler) / PcmDataLen);
					AdjAdapter->Count++;
					if(AdjAdapter->Count >= ADJUST_PERIOD / SOURCEFRAME(Index))
					{
						uint32_t ValidDepth = AdjAdapter->Depth + (SrcAdapter->SrcBufHandler.BufDepth + AdjAdapter->SraBufHandler.BufDepth) / PcmDataLen;
						AdjAdapter->AdjustVal = ADJLEVEL(AdjAdapter->TotalNum / AdjAdapter->Count,
											(ValidDepth * AdjAdapter->LowLevelCent) / 100,
											(ValidDepth * AdjAdapter->HighLevelCent) / 100);
//						printf("\n %d %d val %d\n",ValidDepth,AdjAdapter->TotalNum / AdjAdapter->Count,AdjAdapter->AdjustVal);
						AdjAdapter->TotalNum = 0;
						AdjAdapter->Count = 0;
					}
				}
			}
			break;
		}
		case SRC_ADJUST:
		{
			CLK_ADJUST_ADAPTER * AdjAdapter = (CLK_ADJUST_ADAPTER *)Source->AdjAdapter;
			MCUCircular_GetData(&SrcAdapter->SrcBufHandler, Source->PcmInBuf, SOURCEFRAME(Index) * PcmDataLen);
			if(AdjAdapter->Enable)
			{
				AdjAdapter->TotalNum += ((uint64_t)AudioCore.SampleRate[AudioCore.AudioSource[Index].Net] * Source->DataLenFunc()) / SrcAdapter->SampleRate + MCUCircular_GetDataLen(&SrcAdapter->SrcBufHandler) / PcmDataLen;
				AdjAdapter->Count++;
				if(AdjAdapter->Count >= ADJUST_APLL_PERIOD / SOURCEFRAME(Index))
				{
					uint32_t ValidDepth = ((uint64_t)AudioCore.SampleRate[AudioCore.AudioSource[Index].Net] * AdjAdapter->Depth) / SrcAdapter->SampleRate
											+ SrcAdapter->SrcBufHandler.BufDepth / PcmDataLen
											- SOURCEFRAME(Index);
					ClkAdjust(AdjAdapter, ValidDepth, 1);
				}
			}
			break;
		}
	}
#ifdef CFG_AUDIO_WIDTH_24BIT
	////source 数据位宽扩展
	if(Source->BitWidthConvFlag)
	{
		AudioCorePcmDataBitWidthConv(Source->PcmInBuf,SOURCEFRAME(Index) * PcmDataLen,Source->BitWidth);
	}
#endif
}

void AudioCoreSinkSet(uint8_t Index)
{
	AudioCoreSink *Sink = &AudioCore.AudioSink[Index];
	SRC_ADAPTER *SrcAdapter = Sink->SrcAdapter;
	uint32_t PcmDataLen;

	PcmDataLen = 2 * Sink->Channels;
#ifdef CFG_AUDIO_WIDTH_24BIT
	if(Sink->BitWidth == PCM_DATA_24BIT_WIDTH)
		PcmDataLen *= 2;
#endif
	switch(Sink->Adapt)
	{
		case STD:
#ifdef CFG_AUDIO_WIDTH_24BIT
			if(Sink->BitWidthConvFlag)
			{
				AudioCorePcmDataBitWidthConv(Sink->PcmOutBuf,SINKFRAME(Index) * PcmDataLen,Sink->BitWidth);
			}
#endif
			Sink->DataSetFunc(Sink->PcmOutBuf, SINKFRAME(Index));
			break;
		case SRC_ONLY:
		{
			MCUCircular_PutData(&SrcAdapter->SrcBufHandler, Sink->PcmOutBuf, SINKFRAME(Index) * PcmDataLen);
			break;
		}
		case SRA_ONLY:
		{
			SRA_ADAPTER * AdjAdapter = (SRA_ADAPTER *)Sink->AdjAdapter;
			MCUCircular_PutData(&AdjAdapter->SraBufHandler, Sink->PcmOutBuf, SINKFRAME(Index) * PcmDataLen);
			if(AdjAdapter->Enable)
			{
				AdjAdapter->TotalNum += Sink->SpaceLenFunc() + MCUCircular_GetSpaceLen(&AdjAdapter->SraBufHandler) / PcmDataLen;
				AdjAdapter->Count++;
				if(AdjAdapter->Count >= ADJUST_PERIOD / SINKFRAME(Index))
				{
					uint32_t ValidDepth = AdjAdapter->Depth + AdjAdapter->SraBufHandler.BufDepth / PcmDataLen - SINKFRAME(Index) - SRA_BLOCK;
					AdjAdapter->AdjustVal = -ADJLEVEL(AdjAdapter->TotalNum / AdjAdapter->Count,
													(ValidDepth * AdjAdapter->LowLevelCent) / 100,
													(ValidDepth * AdjAdapter->HighLevelCent) / 100);
					AdjAdapter->TotalNum = 0;
					AdjAdapter->Count = 0;
				}
			}
			break;
		}
		case CLK_ADJUST_ONLY:
		{
			CLK_ADJUST_ADAPTER * AdjAdapter = (CLK_ADJUST_ADAPTER *)Sink->AdjAdapter;
#ifdef CFG_AUDIO_WIDTH_24BIT
			if(Sink->BitWidthConvFlag)
			{
				AudioCorePcmDataBitWidthConv(Sink->PcmOutBuf,SINKFRAME(Index) * PcmDataLen,Sink->BitWidth);
			}
#endif
			Sink->DataSetFunc(Sink->PcmOutBuf, SINKFRAME(Index));
			if(AdjAdapter->Enable)
			{
				AdjAdapter->TotalNum += Sink->SpaceLenFunc();
				AdjAdapter->Count++;
				if(AdjAdapter->Count >= ADJUST_PERIOD / SINKFRAME(Index))
				{
					uint32_t ValidDepth = AdjAdapter->Depth - SINKFRAME(Index);
					ClkAdjust(AdjAdapter, ValidDepth, 1);
				}
			}
			break;
		}
		case SRC_SRA://数据存放于fifo
		{
			SRA_ADAPTER * AdjAdapter = (SRA_ADAPTER *)Sink->AdjAdapter;
			MCUCircular_PutData(&AdjAdapter->SraBufHandler, Sink->PcmOutBuf, SINKFRAME(Index) * PcmDataLen);
			if(AdjAdapter->Enable)
			{
				AdjAdapter->TotalNum += (Sink->SpaceLenFunc() + MCUCircular_GetSpaceLen(&AdjAdapter->SraBufHandler) / PcmDataLen
															 + MCUCircular_GetSpaceLen(&SrcAdapter->SrcBufHandler) / PcmDataLen);
				AdjAdapter->Count++;
				if(AdjAdapter->Count >= ADJUST_PERIOD / SINKFRAME(Index))
				{
					uint32_t ValidDepth = AdjAdapter->Depth + (AdjAdapter->SraBufHandler.BufDepth + SrcAdapter->SrcBufHandler.BufDepth)/PcmDataLen;
					AdjAdapter->AdjustVal = -ADJLEVEL(AdjAdapter->TotalNum / AdjAdapter->Count,
													(ValidDepth * AdjAdapter->LowLevelCent) / 100,
													(ValidDepth * AdjAdapter->HighLevelCent) / 100);
//					printf("\n %d %d val %d\n",ValidDepth,AdjAdapter->TotalNum / AdjAdapter->Count,AdjAdapter->AdjustVal);
					AdjAdapter->TotalNum = 0;
					AdjAdapter->Count = 0;
				}
			}
			break;
		}
		case SRC_ADJUST:
		{
			CLK_ADJUST_ADAPTER * AdjAdapter = (CLK_ADJUST_ADAPTER *)Sink->AdjAdapter;
			MCUCircular_PutData(&SrcAdapter->SrcBufHandler, Sink->PcmOutBuf, SINKFRAME(Index) * PcmDataLen);
			if(AdjAdapter->Enable)
			{
				AdjAdapter->TotalNum += ((uint64_t)AudioCore.SampleRate[AudioCore.AudioSink[Index].Net] * Sink->SpaceLenFunc()) / SrcAdapter->SampleRate
										+ MCUCircular_GetSpaceLen(&SrcAdapter->SrcBufHandler) / PcmDataLen ;
				AdjAdapter->Count++;
				if(AdjAdapter->Count >= ADJUST_PERIOD / SINKFRAME(Index))
				{
					uint32_t ValidDepth = ((uint64_t)AudioCore.SampleRate[AudioCore.AudioSink[Index].Net] * AdjAdapter->Depth) / SrcAdapter->SampleRate
																+ SrcAdapter->SrcBufHandler.BufDepth / PcmDataLen
																- SINKFRAME(Index);
					ClkAdjust(AdjAdapter, ValidDepth, 1);
				}
			}
			break;
		}
	}
}


bool AudioCoreSourceSync(void)
{
	MIX_NET CurrentNet;
	uint8_t i;
	uint16_t ValidIndex, BlockIndex;

	for(CurrentNet = DefaultNet; CurrentNet < MaxNet; CurrentNet++)
	{
		ValidIndex = 0;
		BlockIndex = 0;
		for(i = 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
		{
			if(AudioCore.AudioSource[i].Enable
					&& (AudioCore.AudioSource[i].Net == CurrentNet)
#ifdef AUDIO_CORE_DEBUG
					&& AudioCore.AudioSource[i].DataGetFunc
					&& AudioCore.AudioSource[i].DataLenFunc
#endif
					)
			{
				SOURCE_BIT_EN(ValidIndex, i);
				if(AudioCore.AudioSource[i].Sync)
				{
					SOURCE_BIT_EN(BlockIndex, i);
				}
			}
		}
		for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
		{
			if(AudioCore.AudioSink[i].Enable
					&& (AudioCore.AudioSink[i].Net == CurrentNet)
#ifdef AUDIO_CORE_DEBUG
					&& AudioCore.AudioSink[i].DataSetFunc
					&& AudioCore.AudioSink[i].SpaceLenFunc
#endif
					)
			{
				SINK_BIT_EN(ValidIndex, i);
				if(AudioCore.AudioSink[i].Sync)
				{
					SINK_BIT_EN(BlockIndex, i);
				}
			}
		}

		if((SOURCE_BIT_MASK(ValidIndex)	//Souce >= 1
			|| SINK_BIT_MASK(ValidIndex)) //或 Sink >= 1
			&& (BlockIndex)				//Block >= 1
			&& (BlockIndex & AudioCore.FrameReady) == BlockIndex) //FrameSync
		{
			AudioCore.CurrentMix = CurrentNet;
			for(i = 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
			{
				if(SOURCE_BIT_GET(ValidIndex, i))
				{
					memset(AudioCore.AudioSource[i].PcmInBuf, 0, SOURCEFRAME(i) * sizeof(PCM_DATA_TYPE) * 2);
					SOURCE_BIT_DIS(AudioCore.FrameReady, i);
					AudioCore.AudioSource[i].Active = TRUE;
					AudioCoreSourceGet(i);
				}
			}

			for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
			{
				if(SINK_BIT_GET(ValidIndex, i))
				{
//					memset(AudioCore.AudioSink[i].PcmOutBuf, 0, SINKFRAME(i) * AudioCore.AudioSink[i].Channels * 2);
					AudioCore.AudioSink[i].Active = TRUE;
				}
			}
			NET_BIT_EN(AudioCore.NetReady, CurrentNet);
		}
	}
	if(NET_BIT_MASK(AudioCore.NetReady))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

bool AudioCoreSinkSync(void)
{
	MIX_NET CurrentNet;
	uint8_t i;

	for(i= 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
	{
		if(AudioCore.AudioSource[i].Active)
		{
			AudioCore.AudioSource[i].Active = FALSE;
		}
	}

	for(CurrentNet = DefaultNet; CurrentNet < MaxNet; CurrentNet++)
	{
		if(NET_BIT_GET(AudioCore.NetReady, CurrentNet))
		{
			for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
			{
				if((AudioCore.AudioSink[i].Net == CurrentNet) && (AudioCore.AudioSink[i].Active))
				{
					AudioCoreSinkSet(i);
					AudioCore.AudioSink[i].Active = FALSE;
					SINK_BIT_DIS(AudioCore.FrameReady, i);
				}
			}
			NET_BIT_DIS(AudioCore.NetReady, CurrentNet);
		}
	}
	return TRUE;
}

//除了Index 其他参数 非零：修改配置; 0：保留
void AudioCoreSourceChange(uint8_t Index, uint8_t Channels, uint32_t SampleRate)
{
	SRC_ADAPTER		*SrcAdapter = AudioCore.AudioSource[Index].SrcAdapter;

	if(!AudioCoreSourceIsInit(Index))
	{
		return;
	}
	if(Channels)
	{
		AudioCore.AudioSource[Index].Channels = Channels;
	}
	if(SrcAdapter)
	{
		if(SampleRate)
		{
			SrcAdapter->SampleRate = SampleRate;
		}
		if(SampleRate != AudioCore.SampleRate[AudioCore.AudioSource[Index].Net])
		{
			resampler_polyphase_init(&SrcAdapter->SrcCt, AudioCore.AudioSource[Index].Channels, GetRatioEnum(1000 * AudioCore.SampleRate[AudioCore.AudioSource[Index].Net] / SrcAdapter->SampleRate));
		}
		SrcAdapter->SrcBufHandler.R = 0;
		SrcAdapter->SrcBufHandler.W = 0;
	}
}

//除了Index 其他参数 非零：修改配置; 0：保留
void AudioCoreSinkChange(uint8_t Index, uint8_t Channels, uint32_t SampleRate)
{
	SRC_ADAPTER		*SrcAdapter = AudioCore.AudioSink[Index].SrcAdapter;
	if(!AudioCoreSinkIsInit(Index))
	{
		return;
	}
	if(Channels)
	{
		AudioCore.AudioSink[Index].Channels = Channels;
	}
	if(SrcAdapter)
	{
		if(SampleRate)
		{
			SrcAdapter->SampleRate = SampleRate;
		}
		if(SampleRate != AudioCore.SampleRate[AudioCore.AudioSink[Index].Net])
		{
			resampler_polyphase_init(&SrcAdapter->SrcCt, AudioCore.AudioSink[Index].Channels, GetRatioEnum(1000 * SrcAdapter->SampleRate / AudioCore.SampleRate[AudioCore.AudioSink[Index].Net]));
		}
		SrcAdapter->SrcBufHandler.R = 0;
		SrcAdapter->SrcBufHandler.W = 0;
	}
}

void AudioCoreSourceDepthChange(uint8_t Index, uint32_t NewDepth)
{
	AudioCoreSource * Source = &AudioCore.AudioSource[Index];
	if(!AudioCoreSourceIsInit(Index))
	{
		return;
	}
	switch(Source->Adapt)
	{
		case SRA_ONLY:
			((SRA_ADAPTER *)(Source->AdjAdapter))->Depth = NewDepth;
			break;

		case SRC_SRA:
			((SRA_ADAPTER *)(Source->AdjAdapter))->Depth = NewDepth;
			break;

		case CLK_ADJUST_ONLY:
			((CLK_ADJUST_ADAPTER *)(Source->AdjAdapter))->Depth = NewDepth;
			break;

		case SRC_ADJUST:
			((CLK_ADJUST_ADAPTER *)(Source->AdjAdapter))->Depth = NewDepth;
			break;

		default:
			break;
	}
}

void AudioCoreSinkDepthChange(uint8_t Index, uint32_t NewDepth)
{
	AudioCoreSink * Sink = &AudioCore.AudioSink[Index];
	if(!AudioCoreSinkIsInit(Index))
	{
		return;
	}
	Sink->Depth = NewDepth;
	switch(Sink->Adapt)
	{
		case SRA_ONLY:
			((SRA_ADAPTER *)(Sink->AdjAdapter))->Depth = NewDepth;
			break;

		case SRC_SRA:
			((SRA_ADAPTER *)(Sink->AdjAdapter))->Depth = NewDepth;
			break;

		case CLK_ADJUST_ONLY:
			((CLK_ADJUST_ADAPTER *)(Sink->AdjAdapter))->Depth = NewDepth;
			break;

		case SRC_ADJUST:
			((CLK_ADJUST_ADAPTER *)(Sink->AdjAdapter))->Depth = NewDepth;
			break;

		default:
			break;
	}
}

bool AudioCoreSourceIsInit(uint8_t Index)
{
	if(Index < AUDIO_CORE_SOURCE_MAX_NUM)
	{
		return AudioCore.AudioSource[Index].InitFlag;
	}
	return FALSE;
}

bool AudioCoreSinkIsInit(uint8_t Index)
{
	if(Index < AUDIO_CORE_SINK_MAX_NUM)
	{
		return AudioCore.AudioSink[Index].InitFlag;
	}
	return FALSE;
}

void AudioCoreSourceAdjust(uint8_t Index, bool OnOff)
{
	AudioCoreSource * Source = &AudioCore.AudioSource[Index];
	if(!AudioCoreSourceIsInit(Index))
	{
		return;
	}
	switch(Source->Adapt)
	{
		case SRA_ONLY:
			((SRA_ADAPTER *)(Source->AdjAdapter))->Enable = OnOff;
			break;

		case SRC_SRA:
			((SRA_ADAPTER *)(Source->AdjAdapter))->Enable = OnOff;
			break;

		case CLK_ADJUST_ONLY:
			((CLK_ADJUST_ADAPTER *)(Source->AdjAdapter))->Enable = OnOff;
			if(!OnOff)
			{
				Clock_AudioPllClockAdjust(PLL_CLK_1, 0, 0);
				Clock_AudioPllClockAdjust(PLL_CLK_2, 0, 0);
			}
			break;

		case SRC_ADJUST:
			((CLK_ADJUST_ADAPTER *)(Source->AdjAdapter))->Enable = OnOff;
			if(!OnOff)
			{
				Clock_AudioPllClockAdjust(PLL_CLK_1, 0, 0);
				Clock_AudioPllClockAdjust(PLL_CLK_2, 0, 0);
			}
			break;

		default:
			break;
	}
}

void AudioCoreSinkAdjust(uint8_t Index, bool OnOff)
{
	AudioCoreSink * Sink = &AudioCore.AudioSink[Index];
	if(!AudioCoreSinkIsInit(Index))
	{
		return;
	}
	switch(Sink->Adapt)
	{
		case SRA_ONLY:
			((SRA_ADAPTER *)(Sink->AdjAdapter))->Enable = OnOff;
			break;

		case SRC_SRA:
			((SRA_ADAPTER *)(Sink->AdjAdapter))->Enable = OnOff;
			break;

		case CLK_ADJUST_ONLY:
			((CLK_ADJUST_ADAPTER *)(Sink->AdjAdapter))->Enable = OnOff;
			break;

		case SRC_ADJUST:
			((CLK_ADJUST_ADAPTER *)(Sink->AdjAdapter))->Enable = OnOff;
			break;

		default:
			break;
	}
}

//AudioCore.FrameSize[DefaultNet]必须先配置，且为多通路最大，原因是音效buf共用 @Samples
bool AudioCoreFrameSizeSet(MIX_NET Nets, uint16_t Size)
{
	uint8_t i;
	if(Nets == DefaultNet)
	{
		for(i = 1; i < MaxNet; i++)
		{
			if(AudioCore.FrameSize[i] > Size)
			{
				DBG("FrameSize Error!\n");
				return FALSE;
			}
		}
	}
	else if(AudioCore.FrameSize[DefaultNet] < Size)
	{
		DBG("FrameSize Error!\n");
		return FALSE;
	}
	AudioCore.FrameSize[Nets] = Size;
	
	return TRUE;
}

uint16_t AudioCoreFrameSizeGet(MIX_NET MixNet)
{
	return AudioCore.FrameSize[MixNet];
}

void AudioCoreMixSampleRateSet(MIX_NET MixNet, uint32_t SampleRate)
{
	uint8_t i;

	if(AudioCore.SampleRate[MixNet] != SampleRate)
	{
		AudioCore.SampleRate[MixNet] = SampleRate;
		for(i= 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
		{
			if(AudioCore.AudioSource[i].SrcAdapter && AudioCore.AudioSource[i].Net == MixNet)
			{
				resampler_polyphase_init(&(AudioCore.AudioSource[i].SrcAdapter->SrcCt), AudioCore.AudioSource[i].Channels, GetRatioEnum(1000 * SampleRate / AudioCore.AudioSource[i].SrcAdapter->SampleRate));
			}
		}
		for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
		{
			if(AudioCore.AudioSink[i].SrcAdapter && AudioCore.AudioSink[i].Net == MixNet)
			{
				resampler_polyphase_init(&(AudioCore.AudioSink[i].SrcAdapter->SrcCt), AudioCore.AudioSink[i].Channels, GetRatioEnum(1000 * AudioCore.AudioSink[i].SrcAdapter->SampleRate / SampleRate));
			}
		}
	}
}

uint32_t AudioCoreMixSampleRateGet(MIX_NET MixNet)
{
	return AudioCore.SampleRate[MixNet];
}

MIX_NET AudioCoreSourceMixNetGet(uint8_t Index)
{
	if(Index < AUDIO_CORE_SOURCE_MAX_NUM)
	{
		return AudioCore.AudioSource[Index].Net;
	}
	return DefaultNet;
}

MIX_NET AudioCoreSinkMixNetGet(uint8_t Index)
{
	if(Index < AUDIO_CORE_SINK_MAX_NUM)
	{
		return AudioCore.AudioSink[Index].Net;
	}
	return DefaultNet;
}

#ifdef	CFG_AUDIO_WIDTH_24BIT
PCM_DATA_WIDTH AudioCoreSourceBitWidthGet(uint8_t Index)
{
	return AudioCore.AudioSource[Index].BitWidth;
}
#endif
