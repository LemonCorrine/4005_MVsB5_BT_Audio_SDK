/**
 **************************************************************************************
 * @file    Spdif_mode.c
 * @brief
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2017-12-26 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
#include <string.h>
#include "type.h"
#include "irqn.h"
#include "gpio.h"
#include "dma.h"
#include "rtos_api.h"
#include "app_message.h"
#include "app_config.h"
#include "debug.h"
#include "delay.h"
#include "audio_adc.h"
#include "dac.h"
#include "adc_interface.h"
#include "spdif.h"
#include "dac_interface.h"
#include "clk.h"
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "audio_effect.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "resampler_polyphase.h"
#include "mcu_circular_buf.h"
#include "breakpoint.h"
#include "main_task.h"
#include "remind_sound_item.h"
#include "mode_task_api.h"
#include "remind_sound.h"
#include "spdif_mode.h"
#include "audio_vol.h"
#include "ctrlvars.h"
#include "reset.h"

#if defined(CFG_APP_OPTICAL_MODE_EN) || defined(CFG_APP_COAXIAL_MODE_EN)

//#define SPDIF_DPLL_LOCK_MODE

#ifdef SPDIF_DPLL_LOCK_MODE
void DpllLockReInit(void);
#endif

extern const char* GetModeNameStr(SysModeNumber Mode);

#define SPDIF_SOURCE_NUM				APP_SOURCE_NUM
#define PCM_REMAIN_SMAPLES				3//spdif数据丢声道时,返回数据会超过一帧128，用于缓存数据，试听感更加好
//spdif单个采样点8字节
//recv, dma buf len,MAX_FRAME_SAMPLES * 2 * 2 * 2 * 2是基础，OS切换间隔实测需要加倍。
#define	SPDIF_FIFO_LEN					(MAX_FRAME_SAMPLES * sizeof(PCM_DATA_TYPE) * 2 * 2 * 2 * 2)
//由于采样值从32bit转换为16bit，可以使用同一个buf，否则要独立申请
#define SPDIF_CARRY_LEN					(MAX_FRAME_SAMPLES * sizeof(PCM_DATA_TYPE) * 2 * 2 + PCM_REMAIN_SMAPLES * 2 * 4)//支持192000输入 buf len for get data form dma fifo, deal + 6samples 用于缓存偶尔多余的数据，#9817

static const uint8_t DmaChannelMap[6] = {
	PERIPHERAL_ID_SDIO_RX,
	PERIPHERAL_ID_AUDIO_ADC0_RX,
	PERIPHERAL_ID_AUDIO_ADC1_RX,
	PERIPHERAL_ID_AUDIO_DAC0_TX,
	CFG_SPDIF_RX_DMA_CHANNEL,
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_I2S_MODULE,
#else
	255
#endif
};

typedef struct _SpdifPlayContext
{
	uint32_t			*SpdifPwcFIFO;		//
	uint32_t            *SpdifCarry;
	uint32_t			*SpdifPcmFifo;
	MCU_CIRCULAR_CONTEXT SpdifPcmCircularBuf;
#ifdef CFG_AUDIO_WIDTH_24BIT
	uint32_t            *SpdifCarry24;
#endif
	//play
	uint32_t 			SampleRate;

	bool 				SpdifLockFlag;
#ifdef SPDIF_DPLL_LOCK_MODE
	bool 				SpdifDpllLockFlag;
	uint32_t 			dpll_lock_cnt;
#endif
}SpdifPlayContext;

static  SpdifPlayContext*		SpdifPlayCt = NULL;

//sample为单位
uint16_t Spdif_Rx_DataLenGet(void)
{
	return MCUCircular_GetDataLen(&SpdifPlayCt->SpdifPcmCircularBuf) / (sizeof(PCM_DATA_TYPE) * 2);
}

//sample为单位，buf大小：8 * MaxSize
uint16_t Spdif_Rx_DataGet(void *pcm_out, uint16_t MaxPoint)
{
	return MCUCircular_GetData(&SpdifPlayCt->SpdifPcmCircularBuf, pcm_out, MaxPoint * (sizeof(PCM_DATA_TYPE) * 2)) / (sizeof(PCM_DATA_TYPE) * 2);
}

void SpdifDataCarry(void)
{
	int16_t pcm_space;
	uint16_t spdif_len;
	int16_t pcm_len;
	int16_t *pcmBuf  = (int16_t *)SpdifPlayCt->SpdifCarry;
	uint16_t cnt;

	spdif_len = DMA_CircularDataLenGet(CFG_SPDIF_RX_DMA_CHANNEL);
	pcm_space = MCUCircular_GetSpaceLen(&SpdifPlayCt->SpdifPcmCircularBuf) - (sizeof(PCM_DATA_TYPE) * 8);

	if(pcm_space < (sizeof(PCM_DATA_TYPE) * 8))
	{
		DBG("err\n");
		return;
	}

	if((spdif_len >> 1) > pcm_space)
	{
		spdif_len = pcm_space * 2;
	}

	spdif_len = spdif_len & 0xFFF8;
	if(!spdif_len)
	{
		return ;
	}

	cnt = (spdif_len / (sizeof(PCM_DATA_TYPE) * 4)) / (MAX_FRAME_SAMPLES);

	while(cnt--)
	{
#ifdef CFG_AUDIO_WIDTH_24BIT
		DMA_CircularDataGet(CFG_SPDIF_RX_DMA_CHANNEL, pcmBuf, MAX_FRAME_SAMPLES * 16);

		pcm_len = SPDIF_SPDIFDataToPCMData(CFG_SPDIF_MODULE,(int32_t *)pcmBuf, MAX_FRAME_SAMPLES * 16, (int32_t *)SpdifPlayCt->SpdifCarry24, SPDIF_WORDLTH_24BIT);
		if(pcm_len > 0)
		{
			int32_t *pcmBuf32  =  (int32_t *)SpdifPlayCt->SpdifCarry24;
			uint16_t i;
			//高8位无符号位，需要移位产生
			for(i=0;i<pcm_len/4;i++)
			{
				pcmBuf32[i] <<= 8;
				pcmBuf32[i] >>= 8;
			}
			MCUCircular_PutData(&SpdifPlayCt->SpdifPcmCircularBuf, SpdifPlayCt->SpdifCarry24, pcm_len);
		}
		if(pcm_len < 0)
		{
			return;
		}
#else
		DMA_CircularDataGet(CFG_SPDIF_RX_DMA_CHANNEL, pcmBuf, MAX_FRAME_SAMPLES * 8);
		//由于从32bit转换为16bit，buf可以使用同一个，否则要独立申请。
		pcm_len = SPDIF_SPDIFDataToPCMData(CFG_SPDIF_MODULE,(int32_t *)pcmBuf, MAX_FRAME_SAMPLES * 8, (int32_t *)pcmBuf, SPDIF_WORDLTH_16BIT);
		if(pcm_len < 0)
		{
			return;
		}
		MCUCircular_PutData(&SpdifPlayCt->SpdifPcmCircularBuf, pcmBuf, pcm_len);
#endif
	}
}


bool SpdifPlayInit(void)
{
	AudioCoreIO AudioIOSet;
	bool ret = FALSE;

	if(SpdifPlayCt != NULL)
	{
		return FALSE;
	}	
	
	DBG("%s Init!\n",GetModeNameStr(GetSystemMode()));

	SPDIF_ModuleDisable(CFG_SPDIF_MODULE);
	DMA_ChannelAllocTableSet((uint8_t*)DmaChannelMap);//optical
	
	SpdifPlayCt = (SpdifPlayContext*)osPortMalloc(sizeof(SpdifPlayContext));
	if(SpdifPlayCt == NULL)
	{
		return FALSE;
	}
	memset(SpdifPlayCt, 0, sizeof(SpdifPlayContext));
	
	SpdifPlayCt->SpdifPcmFifo = (uint32_t *)osPortMalloc(AudioCoreFrameSizeGet(DefaultNet) * sizeof(PCM_DATA_TYPE) * 2 * 2 * 2);//end bkd
	if(SpdifPlayCt->SpdifPcmFifo == NULL)
	{
		return FALSE;
	}
	memset(SpdifPlayCt->SpdifPcmFifo, 0, AudioCoreFrameSizeGet(DefaultNet) * sizeof(PCM_DATA_TYPE) * 2 * 2 * 2);
	MCUCircular_Config(&SpdifPlayCt->SpdifPcmCircularBuf, SpdifPlayCt->SpdifPcmFifo, AudioCoreFrameSizeGet(DefaultNet) * sizeof(PCM_DATA_TYPE) * 2 * 2 * 2);
	DBG("spdif frame size %d sample / frame\n",AudioCoreFrameSizeGet(DefaultNet));


	//  (DMA buffer)
	SpdifPlayCt->SpdifPwcFIFO = (uint32_t*)osPortMalloc(SPDIF_FIFO_LEN);
	if(SpdifPlayCt->SpdifPwcFIFO == NULL)
	{
		return FALSE;
	}
	memset(SpdifPlayCt->SpdifPwcFIFO, 0, SPDIF_FIFO_LEN);
	

	SpdifPlayCt->SpdifCarry = (uint32_t *)osPortMalloc(SPDIF_CARRY_LEN);
	if(SpdifPlayCt->SpdifCarry == NULL)
	{
		return FALSE;
	}
	memset(SpdifPlayCt->SpdifCarry, 0, SPDIF_CARRY_LEN);

#ifdef CFG_AUDIO_WIDTH_24BIT
	SpdifPlayCt->SpdifCarry24 = (uint32_t *)osPortMalloc(SPDIF_CARRY_LEN);
	if(SpdifPlayCt->SpdifCarry24 == NULL)
	{
		return FALSE;
	}
	memset(SpdifPlayCt->SpdifCarry24, 0, SPDIF_CARRY_LEN);
#endif
	if(!ModeCommonInit())
	{
		ModeCommonDeinit();
		return FALSE;
	}

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioCoreProcessConfig((AudioCoreProcessFunc)AudioMusicProcess);
#else
	AudioCoreProcessConfig((AudioCoreProcessFunc)AudioBypassProcess);
#endif
	//Audio init
	
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));

	AudioIOSet.Adapt = SRC_ADJUST;//SRC_SRA;
	AudioIOSet.Sync = FALSE;
	AudioIOSet.Channels = 2;
	AudioIOSet.Net = DefaultNet;

	AudioIOSet.DataIOFunc = Spdif_Rx_DataGet;
	AudioIOSet.LenGetFunc = Spdif_Rx_DataLenGet;
	
	AudioIOSet.Depth = AudioCoreFrameSizeGet(DefaultNet) * 2;
	AudioIOSet.LowLevelCent = 40;
	AudioIOSet.HighLevelCent = 60;
	
	SpdifPlayCt->SampleRate = AudioCoreMixSampleRateGet(DefaultNet);
	AudioIOSet.SampleRate = SpdifPlayCt->SampleRate;

#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;
	AudioIOSet.IOBitWidthConvFlag = 0;
#endif
	if(!AudioCoreSourceInit(&AudioIOSet, SPDIF_SOURCE_NUM))
	{
		DBG("spdif source init error!\n");
		return FALSE;
	}
	
	AudioCoreSourceAdjust(SPDIF_SOURCE_NUM, TRUE);
	SPDIF0_AnalogModuleDisable();

#ifdef CFG_APP_OPTICAL_MODE_EN
	if(GetSystemMode() == ModeOpticalAudioPlay)
	{
		//spdif config
		GPIO_RegBitsSet(GPIO_A_ANA_EN,SPDIF0_OPTICAL_INDEX);
		GPIO_PortAModeSet(SPDIF0_OPTICAL_INDEX, SPDIF0_OPTICAL_PORT_MODE);
#ifdef CFG_APP_COAXIAL_MODE_EN
		GPIO_PortAModeSet(SPDIF0_COAXIAL_INDEX, 0);
#endif
		SPDIF0_AnalogModuleEnable(SPDIF0_OPTICAL_PORT_ANA_INPUT, SPDIF_ANA_LEVEL_300mVpp);
	}
#endif

#ifdef CFG_APP_COAXIAL_MODE_EN
	if(GetSystemMode() == ModeCoaxialAudioPlay)
	{	
		GPIO_RegBitsSet(GPIO_A_ANA_EN,SPDIF0_COAXIAL_INDEX);
		GPIO_PortAModeSet(SPDIF0_COAXIAL_INDEX, SPDIF0_COAXIAL_PORT_MODE);
#ifdef CFG_APP_OPTICAL_MODE_EN
		GPIO_PortAModeSet(SPDIF0_OPTICAL_INDEX, 0);
#endif
		SPDIF0_AnalogModuleEnable(SPDIF0_COAXIAL_PORT_ANA_INPUT, SPDIF_ANA_LEVEL_300mVpp);
	}
#endif

	SPDIF_ModuleRst(CFG_SPDIF_MODULE);
	SPDIF_ModuleDisable(CFG_SPDIF_MODULE);
	SPDIF_RXInit(CFG_SPDIF_MODULE,1, 0, 0);
	//使用
	DMA_ChannelDisable(CFG_SPDIF_RX_DMA_CHANNEL);
	DMA_CircularConfig(CFG_SPDIF_RX_DMA_CHANNEL, SPDIF_FIFO_LEN / 2, (void*)SpdifPlayCt->SpdifPwcFIFO, SPDIF_FIFO_LEN);
	DMA_ChannelEnable(CFG_SPDIF_RX_DMA_CHANNEL);

#ifdef SPDIF_DPLL_LOCK_MODE
	SPDIF_DpllLockInit();//dpll lock mode
#endif

	SPDIF_ModuleEnable(CFG_SPDIF_MODULE);

	
#ifdef CFG_FUNC_BREAKPOINT_EN
	BackupInfoUpdata(BACKUP_SYS_INFO);
#endif

	AudioCodecGainUpdata();//update hardware config

#ifdef CFG_FUNC_REMIND_SOUND_EN
	{
		if(GetSystemMode() == ModeOpticalAudioPlay)
		{
			DBG("Optical Play run\n");
			ret = RemindSoundServiceItemRequest(SOUND_REMIND_GXIANMOD, REMIND_PRIO_NORMAL);
		}
		else
		{
			DBG("Coaxial Play run\n");
			ret = RemindSoundServiceItemRequest(SOUND_REMIND_TZHOUMOD, REMIND_PRIO_NORMAL);
		}
		if(ret == FALSE)
		{
			if(IsAudioPlayerMute() == TRUE)
				{
					HardWareMuteOrUnMute();
				}
		}
	}		
#endif

	SpdifPlayCt->SpdifLockFlag = FALSE;
	
#ifndef CFG_FUNC_REMIND_SOUND_EN
	 if(IsAudioPlayerMute() == TRUE)
	 {
		 HardWareMuteOrUnMute();
	 }
#endif

	return TRUE;
}

void SpdifPlayRun(uint16_t msgId)
{
	switch(msgId)
	{
		default:
			CommonMsgProccess(msgId);
			break;
	}

#ifdef CFG_APP_COAXIAL_MODE_EN
	if(GetSystemMode() == ModeCoaxialAudioPlay)
	{
		if(SPDIF_FlagStatusGet(CFG_SPDIF_MODULE,SYNC_FLAG_STATUS) || (!SPDIF_FlagStatusGet(CFG_SPDIF_MODULE,LOCK_FLAG_STATUS)))
		{
			SPDIF_RXInit(CFG_SPDIF_MODULE,1, 0, 0);
			SPDIF_ModuleEnable(CFG_SPDIF_MODULE);
		}
	}
#endif

	if(SpdifPlayCt->SpdifLockFlag && !SPDIF_FlagStatusGet(CFG_SPDIF_MODULE,LOCK_FLAG_STATUS))
	{
		APP_DBG("SPDIF RX UNLOCK!\n");
		SpdifPlayCt->SpdifLockFlag = FALSE;

#ifdef SPDIF_DPLL_LOCK_MODE
		SpdifPlayCt->dpll_lock_cnt = 0;
		SPDIF_DpllLockDeInit();
		SPDIF_DpllLockInit();//dpll lock mode
		SpdifPlayCt->SpdifDpllLockFlag = FALSE;
#endif

		AudioCoreSourceDisable(SPDIF_SOURCE_NUM);
		/*#ifdef CFG_FUNC_FREQ_ADJUST
		AudioCoreSourceFreqAdjustDisable();
		#endif
		*/
	}
	if(!SpdifPlayCt->SpdifLockFlag && SPDIF_FlagStatusGet(CFG_SPDIF_MODULE,LOCK_FLAG_STATUS))
	{
		APP_DBG("SPDIF RX LOCK!\n");
		SpdifPlayCt->SpdifLockFlag = TRUE;
		if(IsAudioPlayerMute() == FALSE)
		{
			HardWareMuteOrUnMute();
		}
		vTaskDelay(20);
		AudioCoreSourceEnable(SPDIF_SOURCE_NUM);

#ifdef SPDIF_DPLL_LOCK_MODE
		SPDIF_DpllLockStart();

		TIMER timeout;
		TimeOutSet(&timeout,1000);
		while(!SPDIF_GetDpllLockFlag())
		{
			if(IsTimeOut(&timeout))
			{
				TimeOutSet(&timeout,1000);
				SPDIF_DpllLockDeInit();
				if(!SPDIF_FlagStatusGet(CFG_SPDIF_MODULE,LOCK_FLAG_STATUS))
				{
					printf("unlock return\n");
					return ;
				}
				else
				{
					printf("reinit dpll lock\n");
					SPDIF_DpllLockInit();//dpll lock mode
					SPDIF_DpllLockStart();
				}
			}
		}

		if(SPDIF_GetDpllLockFlag())
		{
			APP_DBG("Dpll LOCK!\n");
			SPDIF_DpllLockHold();
		}
		else
		{
			APP_DBG("Dpll LOCK FAIL!\n");
			SpdifPlayCt->SpdifDpllLockFlag = FALSE;
		}
#endif

		AudioCoreSourceUnmute(SPDIF_SOURCE_NUM, TRUE, TRUE);
		if(IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
	}

#ifdef SPDIF_DPLL_LOCK_MODE
	if(SpdifPlayCt->SpdifLockFlag == TRUE)
	{
		SpdifPlayCt->dpll_lock_cnt++;
		if(SpdifPlayCt->dpll_lock_cnt>1000 && SDPIF_DpllLockHoldGet())
		{
			SpdifPlayCt->dpll_lock_cnt = 0;
			APP_DBG("REG_DPLL_NDAC_HOLD_EN\n");
			SPDIF_DpllLockHoldSet(0);//DPLL彻底跟随SPDIF0
			SpdifPlayCt->SpdifDpllLockFlag = TRUE;
		}
	}
#endif

	//监控SPDIF RX采样率是否改变
#ifdef SPDIF_DPLL_LOCK_MODE
	if(SpdifPlayCt->SpdifDpllLockFlag == TRUE)
#else
	if(SpdifPlayCt->SpdifLockFlag == TRUE)
#endif
	{
		if(SpdifPlayCt->SampleRate != SPDIF_SampleRateGet(CFG_SPDIF_MODULE))
		{
			SpdifPlayCt->SampleRate = SPDIF_SampleRateGet(CFG_SPDIF_MODULE);
			APP_DBG("Get SampleRate: %d\n", (int)SpdifPlayCt->SampleRate);
#ifdef CFG_AUDIO_OUT_AUTO_SAMPLE_RATE_44100_48000
			AudioOutSampleRateSet(SpdifPlayCt->SampleRate);
#endif
			AudioCoreSourceChange(SPDIF_SOURCE_NUM, 0, SpdifPlayCt->SampleRate);

#ifdef SPDIF_DPLL_LOCK_MODE
			DpllLockReInit();
#endif
		}
		
		SpdifDataCarry();
	}
	//任务优先级设置为4,通过发送该命令，可以提高AudioCore service有效利用率
	{
		MessageContext		msgSend;
		msgSend.msgId		= MSG_NONE;
		MessageSend(GetAudioCoreServiceMsgHandle(), &msgSend);
	}
}


bool SpdifPlayDeinit(void)
{

	APP_DBG("%s Deinit\n",GetModeNameStr(GetSystemMode()));
	
	if(SpdifPlayCt == NULL)
	{
		return FALSE;
	}

	if(IsAudioPlayerMute() == FALSE)
	{
		HardWareMuteOrUnMute();
	}
	AudioCoreSourceAdjust(SPDIF_SOURCE_NUM, FALSE);
	PauseAuidoCore();
	
//#if	defined(CFG_FUNC_REMIND_SOUND_EN)
//	AudioCoreSourceMute(REMIND_SOURCE_NUM, TRUE, TRUE);
//#endif
	//vTaskDelay(30);

	AudioCoreProcessConfig((void*)AudioNoAppProcess);
	AudioCoreSourceDisable(SPDIF_SOURCE_NUM);//SPDIF_SOURCE_NUM
	AudioCoreSourceDeinit(SPDIF_SOURCE_NUM);
	
	SPDIF_ModuleDisable(CFG_SPDIF_MODULE);
	DMA_InterruptFlagClear(CFG_SPDIF_RX_DMA_CHANNEL, DMA_DONE_INT);
	DMA_InterruptFlagClear(CFG_SPDIF_RX_DMA_CHANNEL, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(CFG_SPDIF_RX_DMA_CHANNEL, DMA_ERROR_INT);
	DMA_ChannelDisable(CFG_SPDIF_RX_DMA_CHANNEL);

	SPDIF0_AnalogModuleDisable();

#ifdef CFG_APP_COAXIAL_MODE_EN
	GPIO_PortAModeSet(SPDIF0_COAXIAL_INDEX, 0);
#endif
#ifdef CFG_APP_OPTICAL_MODE_EN
	GPIO_PortAModeSet(SPDIF0_OPTICAL_INDEX, 0);
#endif


	if(SpdifPlayCt->SpdifCarry != NULL)
	{
		osPortFree(SpdifPlayCt->SpdifCarry);
		SpdifPlayCt->SpdifCarry = NULL;
	}

#ifdef CFG_AUDIO_WIDTH_24BIT
	if(SpdifPlayCt->SpdifCarry24 != NULL)
	{
		osPortFree(SpdifPlayCt->SpdifCarry24);
		SpdifPlayCt->SpdifCarry24 = NULL;
	}
#endif

	if(SpdifPlayCt->SpdifPcmFifo != NULL)
	{
		osPortFree(SpdifPlayCt->SpdifPcmFifo);
		SpdifPlayCt->SpdifPcmFifo = NULL;
	}

	if(SpdifPlayCt->SpdifPwcFIFO != NULL)
	{
		osPortFree(SpdifPlayCt->SpdifPwcFIFO);
		SpdifPlayCt->SpdifPwcFIFO = NULL;
	}

	ModeCommonDeinit();//通路全部释放
	
#ifdef SPDIF_DPLL_LOCK_MODE
	SPDIF_DpllLockDeInit();
#endif

	osPortFree(SpdifPlayCt);
	SpdifPlayCt = NULL;
	
	return TRUE;
}

#ifdef SPDIF_DPLL_LOCK_MODE
void DpllLockReInit(void)
{
	SPDIF_DpllLockDeInit();
	SPDIF_DpllLockInit();//dpll lock mode
	SpdifPlayCt->SpdifDpllLockFlag = FALSE;
	SpdifPlayCt->SpdifLockFlag = FALSE;
}
#endif

#endif 

