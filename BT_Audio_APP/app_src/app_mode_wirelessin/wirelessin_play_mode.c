/**
 **************************************************************************************
 * @file    wirelessin_play_mode.c
 * @brief   
 *
 * @author  Pi
 * @version V1.0.0
 *
 * $Created: 2024-12-10 18:00:00$
 *
 * @Copyright (C) 2024, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */


#include "app_config.h"
#include "app_message.h"
#include "dma.h"
#include "main_task.h"
#include "audio_vol.h"
#include "audio_core_api.h"
#include "remind_sound.h"
#include "wireless_service.h"
#include "audio_effect.h"
#include "mode_task.h"
#include "wirelessin_play_mode.h"


#ifdef CFG_APP_WIRELESSIN_MODE_EN

static const uint8_t DmaChannelMap[6] = {
		PERIPHERAL_ID_AUDIO_ADC0_RX,
		PERIPHERAL_ID_AUDIO_ADC1_RX,
		PERIPHERAL_ID_AUDIO_DAC0_TX,
	#ifdef CFG_RES_AUDIO_I2S_MIX_IN_EN
		PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_MIX_I2S_MODULE,
	#else
		PERIPHERAL_ID_SDIO_RX,
	#endif
#ifdef CFG_COMMUNICATION_BY_UART
	CFG_FUNC_COMMUNICATION_TX_DMA_PORT,
	CFG_FUNC_COMMUNICATION_RX_DMA_PORT,
#else
	#ifdef CFG_RES_AUDIO_I2S_MIX_OUT_EN
		PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_MIX_I2S_MODULE,
	#else
		255,
	#endif
	#ifdef CFG_RES_AUDIO_I2SOUT_EN
		PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_I2S_MODULE,
	#else
		#ifdef CFG_RES_AUDIO_I2S_MIX2_IN_EN
			PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_MIX2_I2S_MODULE,
		#else
			255,
		#endif
	#endif
#endif
};

/*******************Sync porting******************************/

/****************************************************/

typedef struct _WirelessinPlayContext
{
	uint8_t *Fifo;
	MCU_CIRCULAR_CONTEXT FifoCt;
} WirelessinPlayContext;
WirelessinPlayContext *WirelessinPlayCt = NULL;

uint16_t WirelessInDataLenGet(void)
{
	if(WirelessinPlayCt != NULL && WirelessinPlayCt->Fifo != NULL)
	{
		return MCUCircular_GetDataLen(&WirelessinPlayCt->FifoCt) / 4;
	}
	return 0;
}

uint16_t WirelessInDataGet(void* Buf, uint16_t Samples)
{
	if(WirelessinPlayCt != NULL && WirelessinPlayCt->Fifo != NULL)
	{
		return MCUCircular_GetData(&WirelessinPlayCt->FifoCt, Buf, Samples * 4) / 4;
	}
	return 0;
}

uint16_t WirelessInSpaceLenGet(void)
{
	if(WirelessinPlayCt != NULL && WirelessinPlayCt->Fifo != NULL)
	{
		return MCUCircular_GetSpaceLen(&WirelessinPlayCt->FifoCt) / 4;
	}
	return 0;
}

uint16_t WirelessInDataSet(int16_t Buf, uint32_t Samples)
{
	if(WirelessinPlayCt != NULL && WirelessinPlayCt->Fifo != NULL)
	{
		MCUCircular_PutData(&WirelessinPlayCt->FifoCt, Buf, Samples * 4);
	}
	return 0;
}

/************************************************************************************************************
 * @func        WirelessinPlayInit
 * @brief       WirelessinPlay模式参数配置，资源初始化
 * @param       MessageHandle   
 * @Output      None
 * @return      bool
 * @Others      任务块、Dac、AudioCore配置
 * @Others      数据流从Decoder到audiocore配有函数指针，audioCore到Dac同理，由audiocoreService任务按需驱动
 * Record
 ************************************************************************************************************/
static bool WirelessinPlayInitRes(void)
{
	//DMA channel

	WirelessinPlayCt = (WirelessinPlayContext*)osPortMalloc(sizeof(WirelessinPlayContext));
	if(WirelessinPlayCt == NULL)
	{
		return FALSE;
	}
	memset(WirelessinPlayCt, 0, sizeof(WirelessinPlayContext));
	
	WirelessinPlayCt->Fifo = osPortMalloc(AudioCoreFrameSizeGet(DefaultNet) * 2 * 4);
	if(WirelessinPlayCt->Fifo == NULL)
	{
		osPortFree(WirelessinPlayCt);
		WirelessinPlayCt = NULL;
		return FALSE;
	}
	MCUCircular_Config(&WirelessinPlayCt->FifoCt, WirelessinPlayCt->Fifo, AudioCoreFrameSizeGet(DefaultNet) * 2 * 4);

	AudioCoreIO	AudioIOSet;
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
	AudioIOSet.Adapt = STD;
	AudioIOSet.Sync = FALSE;
	AudioIOSet.Channels = 2;
	AudioIOSet.Net = DefaultNet;
	AudioIOSet.DataIOFunc = WirelessInDataGet;
	AudioIOSet.LenGetFunc = WirelessInDataLenGet;
	AudioIOSet.SampleRate =  SAMPLE_RATE;
	AudioIOSet.Depth = AudioCoreFrameSizeGet(DefaultNet) * 2;
	AudioIOSet.LowLevelCent = 40;
	AudioIOSet.HighLevelCent = 90;
#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = 0;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 1;//需调位宽后给Core
#endif
	if(!AudioCoreSourceInit(&AudioIOSet, APP_SOURCE_NUM))
	{
		DBG("wirelessinplay source error!\n");
		return FALSE;
	}
	AudioCoreSourceEnable(APP_SOURCE_NUM);

#if defined(CFG_WIRELESS_EN) && defined(PACKET_AUDIO_CH_BACKWARD)
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
	AudioIOSet.Adapt = STD;
	AudioIOSet.Sync = TRUE;
	AudioIOSet.Channels = 2;
	AudioIOSet.Net = DefaultNet;
	AudioIOSet.DataIOFunc = WirelessOutDataSet;
	AudioIOSet.LenGetFunc = WirelessOutSpaceLenGet;
	AudioIOSet.SampleRate =  SAMPLE_RATE;
	AudioIOSet.Depth = AudioCoreFrameSizeGet(DefaultNet) * 2;
	AudioIOSet.LowLevelCent = 40;
	AudioIOSet.HighLevelCent = 90;
	#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 1;//需调位宽后给sink Dev
	#endif
	if(!AudioCoreSinkInit(&AudioIOSet, WIRELESS_AUDIO_SINK_NUM))
	{
		DBG("wirelessinplay sink error!\n");
		return FALSE;
	}
	AudioCoreSinkEnable(WIRELESS_AUDIO_SINK_NUM);
#endif

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioCoreProcessConfig((void*)AudioMusicProcess);
#else
	AudioCoreProcessConfig((void*)AudioBypassProcess);
#endif

	return TRUE;
}

/************************************************************************************************************
 * @func        WirelessinPlayRun
 * @brief       WirelessinPlay模式运行函数
 * @param       msgId
 * @return      none
 ************************************************************************************************************/
void WirelessinPlayRun(uint16_t msgId)
{
	switch(msgId)
	{
#ifdef CFG_FUNC_REMIND_SOUND_EN
		case MSG_REMIND_PLAY_END:
			break;
#endif
		default:
			CommonMsgProccess(msgId);
			break;
	}
	wireless_audio_in_process();
}

/***********************************************************************************
 * Wirelessin Play 初始化
 **********************************************************************************/
bool WirelessinPlayInit(void)
{
	bool		ret = TRUE;

	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);
	if(!ModeCommonInit())
	{
		DBG("Test Wirelessin comm init error!\n");
		ModeCommonDeinit();
		return FALSE;
	}
	if(GetWirelessServiceTaskHandle() == NULL)
	{
		MessageContext		newmsg;
		newmsg.msgId = MSG_WIRELESS;
		MessageSend(GetMainMessageHandle(), &newmsg);
	}
	ret = WirelessinPlayInitRes();
	APP_DBG("wirelessin Play mode\n");


	
#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(RemindSoundServiceItemRequest(SOUND_REMIND_BTMODE, REMIND_PRIO_NORMAL) == FALSE)
	{
		if(IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
	}
#else
	if(IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}
#endif
	AudioCoreSourceUnmute(APP_SOURCE_NUM,TRUE,TRUE);
	return ret;
}


/***********************************************************************************
 * Wirelessin Play 反初始化
 **********************************************************************************/
bool WirelessinPlayDeinit(void)
{
	if(WirelessinPlayCt == NULL)
	{
		return TRUE;
	}
	if(GetWirelessServiceTaskHandle() != NULL)
	{
		MessageContext		newmsg;
		newmsg.msgId = MSG_WIRELESS_EXIT;
		MessageSend(mainAppCt.msgHandle, &newmsg);
	}
	APP_DBG("Wirelessin Play mode Deinit\n");

	if(IsAudioPlayerMute() == FALSE)
	{
		HardWareMuteOrUnMute();
	}	
	PauseAuidoCore();
	
	AudioCoreProcessConfig((void*)AudioNoAppProcess);

	AudioCoreSourceUnmute(APP_SOURCE_NUM,TRUE,TRUE);
	AudioCoreSourceDisable(APP_SOURCE_NUM);
	AudioCoreSourceDeinit(APP_SOURCE_NUM);

#if defined(CFG_WIRELESS_EN) && defined(PACKET_AUDIO_CH_BACKWARD)
	AudioCoreSinkDisable(WIRELESS_AUDIO_SINK_NUM);
	AudioCoreSinkDeinit(WIRELESS_AUDIO_SINK_NUM);
#endif
	ModeCommonDeinit();//通路全部释放
	osPortFree(WirelessinPlayCt->Fifo);

	APP_DBG("!!WirelessinPlayCt\n");
	osPortFree(WirelessinPlayCt);
	WirelessinPlayCt = NULL;

	osTaskDelay(10);// for printf 
	return TRUE;
}

#endif//#ifdef CFG_APP_WIRELESSIN_MODE_EN
