/**
 **************************************************************************************
 * @file    i2sin_mode.c
 * @brief   
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2019-1-4 17:29:47$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
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
#include "dac_interface.h"
#include "audio_core_api.h"
#include "audio_core_service.h"
#include "decoder.h"
#include "remind_sound.h"
#include "main_task.h"
#include "audio_effect.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "breakpoint.h"
#include "audio_vol.h"
#include "i2s.h"
#include "i2s_interface.h"
#include "ctrlvars.h"
#include "mcu_circular_buf.h"
#include "mode_task_api.h"
#include "clk.h"

#ifdef CFG_APP_I2SIN_MODE_EN

#define I2SIN_SOURCE_NUM				APP_SOURCE_NUM

#ifdef CFG_I2S_SLAVE_TO_SPDIFOUT_EN
volatile uint32_t CurrentSampleRate = 0;
#endif

typedef struct _I2SInPlayContext
{
	xTaskHandle 		taskHandle;
	MessageHandle		msgHandle;

	uint32_t			*I2SFIFO1;			//I2S的DMA循环fifo
	AudioCoreContext 	*AudioCoreI2SIn;

	//play
	uint32_t 			SampleRate; //带提示音时，如果不重采样，要避免采样率配置冲突

}I2SInPlayContext;


static const uint8_t DmaChannelMap[6] = {
	PERIPHERAL_ID_AUDIO_ADC0_RX,
	PERIPHERAL_ID_AUDIO_ADC1_RX,
	PERIPHERAL_ID_AUDIO_DAC0_TX,
	PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_I2S_MODULE,
#ifdef CFG_RES_AUDIO_I2SOUT_EN
	PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_I2S_MODULE,
#else
#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
	SPDIF_OUT_DMA_ID,
#else
	255,
#endif
#endif
#ifdef CFG_RES_AUDIO_I2S_MIX_IN_EN
	PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_MIX_I2S_MODULE,
#else
	PERIPHERAL_ID_SDIO_RX,
#endif
};

static  I2SInPlayContext*		sI2SInPlayCt;
uint8_t I2SInDecoderSourceNum(void);

void I2SInPlayResFree(void)
{
	//注意：AudioCore父任务调整到mainApp下，此处只关闭AudioCore通道，不关闭任务
	AudioCoreProcessConfig((void*)AudioNoAppProcess);
	AudioCoreSourceDisable(I2SIN_SOURCE_NUM);
	AudioCoreSourceDeinit(I2SIN_SOURCE_NUM);

	I2S_ModuleRxDisable(CFG_RES_I2S_MODULE);
	I2S_ModuleDisable(CFG_RES_I2S_MODULE);
#ifndef CFG_RES_AUDIO_I2SOUT_EN
	DMA_ChannelDisable(PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_I2S_MODULE);
#endif

	//PortFree
	sI2SInPlayCt->AudioCoreI2SIn = NULL;

#if	0//defined(CFG_FUNC_REMIND_SOUND_EN)
	AudioCoreSourceDeinit(REMIND_SOURCE_NUM);
#endif

	if(sI2SInPlayCt->I2SFIFO1 != NULL)
	{
		osPortFree(sI2SInPlayCt->I2SFIFO1);
		sI2SInPlayCt->I2SFIFO1 = NULL;
	}

	APP_DBG("I2s:Kill Ct\n");
}

bool I2SInPlayResMalloc(uint16_t SampleLen)
{
	sI2SInPlayCt = (I2SInPlayContext*)osPortMalloc(sizeof(I2SInPlayContext));
	if(sI2SInPlayCt == NULL)
	{
		return FALSE;
	}
	memset(sI2SInPlayCt, 0, sizeof(I2SInPlayContext));

	//I2SIn  digital (DMA)
	sI2SInPlayCt->I2SFIFO1 = (uint32_t*)osPortMalloc(SampleLen * sizeof(PCM_DATA_TYPE) * 2 * 2);
	if(sI2SInPlayCt->I2SFIFO1 == NULL)
	{
		return FALSE;
	}
	memset(sI2SInPlayCt->I2SFIFO1, 0, SampleLen * sizeof(PCM_DATA_TYPE) * 2 * 2);

	return TRUE;
}

bool I2SInPlayResInit(void)
{
	I2SParamCt i2s_set;
	uint32_t sampleRate  = AudioCoreMixSampleRateGet(DefaultNet);

	sI2SInPlayCt->SampleRate = CFG_PARA_I2S_SAMPLERATE;//根据实际外设选择
	//Core Source1 para
	sI2SInPlayCt->AudioCoreI2SIn = (AudioCoreContext*)&AudioCore;

	i2s_set.IsMasterMode=CFG_RES_I2S_MODE;// 0:master 1:slave
	i2s_set.SampleRate=sI2SInPlayCt->SampleRate;
	i2s_set.I2sFormat=I2S_FORMAT_I2S;
#ifdef	CFG_AUDIO_WIDTH_24BIT
	i2s_set.I2sBits = I2S_LENGTH_24BITS;
#else
	i2s_set.I2sBits = I2S_LENGTH_16BITS;
#endif
	i2s_set.I2sTxRxEnable = 2;

	i2s_set.RxPeripheralID = PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_I2S_MODULE;

	i2s_set.RxBuf=sI2SInPlayCt->I2SFIFO1;
	i2s_set.RxLen=AudioCoreFrameSizeGet(DefaultNet) * sizeof(PCM_DATA_TYPE) * 2 * 2 ;//I2SIN_FIFO_LEN;

#ifdef CFG_RES_AUDIO_I2SOUT_EN

	i2s_set.TxPeripheralID = PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_I2S_MODULE;

	i2s_set.TxBuf = (void*)mainAppCt.I2SFIFO;

	i2s_set.TxLen = mainAppCt.I2SFIFO_LEN;

	i2s_set.I2sTxRxEnable = 3;
#endif

	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_MCLK_GPIO), GET_I2S_GPIO_MODE(I2S_MCLK_GPIO));//mclk
	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_LRCLK_GPIO),GET_I2S_GPIO_MODE(I2S_LRCLK_GPIO));//lrclk
	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_BCLK_GPIO), GET_I2S_GPIO_MODE(I2S_BCLK_GPIO));//bclk
#ifdef I2S_DOUT_GPIO
	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_DOUT_GPIO), GET_I2S_GPIO_MODE(I2S_DOUT_GPIO));//do
#endif
#ifdef I2S_DIN_GPIO
	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_DIN_GPIO), GET_I2S_GPIO_MODE(I2S_DIN_GPIO));//di
#endif

	I2S_ModuleDisable(CFG_RES_I2S_MODULE);
	I2S_AlignModeSet(CFG_RES_I2S_MODULE, I2S_LOW_BITS_ACTIVE);
	AudioI2S_Init(CFG_RES_I2S_MODULE,&i2s_set);

#ifdef CFG_FUNC_MCLK_USE_CUSTOMIZED_EN
	if(CFG_RES_I2S_MODULE == I2S0_MODULE)
		Clock_AudioMclkSel(AUDIO_I2S0, gCtrlVars.HwCt.I2S0Ct.i2s_mclk_source);
	else
		Clock_AudioMclkSel(AUDIO_I2S1, gCtrlVars.HwCt.I2S1Ct.i2s_mclk_source);
#else
	if(CFG_RES_I2S_MODULE == I2S0_MODULE)
		gCtrlVars.HwCt.I2S0Ct.i2s_mclk_source = Clock_AudioMclkGet(AUDIO_I2S0);
	else
		gCtrlVars.HwCt.I2S1Ct.i2s_mclk_source = Clock_AudioMclkGet(AUDIO_I2S1);
#endif

#ifdef CFG_I2S_SLAVE_TO_SPDIFOUT_EN
#if CFG_RES_I2S_MODE == 1
	I2S_SampleRateCheckInterruptClr(CFG_RES_I2S_MODULE);
	I2S_SampleRateCheckInterruptEnable(CFG_RES_I2S_MODULE);
#endif
#endif
//	//note Soure0.和sink0已经在main app中配置，不要随意配置
	//Core Soure1.Para
	AudioCoreIO	AudioIOSet;
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));

#if ((CFG_RES_I2S_MODE == I2S_MASTER_MODE) || !defined(CFG_FUNC_I2S_IN_SYNC_EN))
	{//master 或者关微调
		if(CFG_PARA_I2S_SAMPLERATE == sampleRate)
			AudioIOSet.Adapt = STD;
		else
			AudioIOSet.Adapt = SRC_ONLY;
	}
#else
	{//slave
		if(CFG_PARA_I2S_SAMPLERATE == sampleRate)
			AudioIOSet.Adapt = STD;//SRA_ONLY;//CLK_ADJUST_ONLY;//
		else
			AudioIOSet.Adapt = SRC_SRA;//SRC_ADJUST;//
	}
#endif

	AudioIOSet.Sync = TRUE;//FALSE;//
	AudioIOSet.Channels = 2;
	AudioIOSet.Net = DefaultNet;
	AudioIOSet.Depth = AudioCoreFrameSizeGet(DefaultNet) * 2;//sI2SInPlayCt->I2SFIFO1 采样点深度
//	DBG("Depth:%d", AudioIOSet.Depth);
	AudioIOSet.HighLevelCent = 60;
	AudioIOSet.LowLevelCent = 40;
	AudioIOSet.SampleRate = CFG_PARA_I2S_SAMPLERATE;//根据实际外设选择
//	AudioIOSet.CoreSampleRate = CFG_PARA_SAMPLE_RATE;
	if(CFG_RES_I2S_MODULE == 0)
	{
		AudioIOSet.DataIOFunc = AudioI2S0_DataGet ;
		AudioIOSet.LenGetFunc = AudioI2S0_DataLenGet;
	}
	else
	{
		AudioIOSet.DataIOFunc = AudioI2S1_DataGet ;
		AudioIOSet.LenGetFunc = AudioI2S1_DataLenGet;
	}

#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = 1;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 0;//需要数据进行位宽扩展
#endif
	if(!AudioCoreSourceInit(&AudioIOSet, I2SIN_SOURCE_NUM))
	{
		DBG("I2Splay source error!\n");
		return FALSE;
	}

	//Core Process	
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	sI2SInPlayCt->AudioCoreI2SIn->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
#else
	sI2SInPlayCt->AudioCoreI2SIn->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
#endif

	return TRUE;
}

/**
 * @func        I2SInPlay_Init
 * @brief       I2SIn模式参数配置，资源初始化
 * @param       MessageHandle 
 * @Output      None
 * @return      bool
 * @Others      任务块、I2S、Dac、AudioCore配置
 * @Others      数据流从I2S到audiocore配有函数指针，audioCore到Dac同理，由audiocoreService任务按需驱动
 * Record
 */
bool  I2SInPlayInit(void)
{
	bool ret;

	APP_DBG("I2SIn init\n");
	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);//

	if(!ModeCommonInit())
	{
		ModeCommonDeinit();
		return FALSE;
	}
	if(!I2SInPlayResMalloc(AudioCoreFrameSizeGet(DefaultNet)))
	{
		APP_DBG("I2SInPlay Res Error!\n");
		return FALSE;
	}

	ret = I2SInPlayResInit();
	AudioCoreSourceEnable(APP_SOURCE_NUM);
	AudioCoreSourceAdjust(APP_SOURCE_NUM, TRUE);

	AudioCodecGainUpdata();//update hardware config

#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(RemindSoundServiceItemRequest(SOUND_REMIND_I2SMODE, REMIND_PRIO_NORMAL) == FALSE)
	{
		if(IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
	}
#endif

#ifndef CFG_FUNC_REMIND_SOUND_EN
	if(IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}
#endif
	AudioCoreSourceUnmute(I2SIN_SOURCE_NUM, TRUE, TRUE);
	return ret;
}
/**
 * @func        I2sInPlayEntrance
 * @brief       模式执行主体
 * @param       void * param  
 * @Output      None
 * @return      None
 * @Others      模式建立和结束过程
 * Record
 */
void I2SInPlayRun(uint16_t msgId)
{
#if defined (CFG_RES_AUDIO_SPDIFOUT_EN) && (CFG_RES_I2S_MODE == 1)
#ifdef CFG_I2S_SLAVE_TO_SPDIFOUT_EN
	extern void AudioSpdifOut_SampleRateChange(uint32_t SampleRate);
	if (I2S_SampleRateCheckInterruptGet(CFG_RES_I2S_MODULE))
	{
		{
			Clock_PllLock(225792);
		}//Add the above actions to make I2S_SampleRateGet right
		CurrentSampleRate = I2S_SampleRateGet(CFG_RES_I2S_MODULE);
		I2S_SampleRateSet(CFG_RES_I2S_MODULE, CurrentSampleRate);
		APP_DBG("I2SIn samplerate change to:%ld\n", CurrentSampleRate);
		AudioSpdifOut_SampleRateChange(CurrentSampleRate);
		SyncModule_Reset();

		extern bool AudioEffectModeSel(EFFECT_MODE effectMode, uint8_t sel);
		AudioEffectModeSel(mainAppCt.EffectMode, 1);

		I2S_SampleRateCheckInterruptClr(CFG_RES_I2S_MODULE);
	}
#endif
#endif

	switch(msgId)//警告：在此段代码，禁止新增提示音插播位置。
	{
/*	case MSG_REMIND_SOUND_PLAY_START:
			break;

		case MSG_REMIND_SOUND_PLAY_DONE://提示音播放结束
		case MSG_REMIND_SOUND_PLAY_REQUEST_FAIL:
			//AudioCoreSourceUnmute(APP_SOURCE_NUM, TRUE, TRUE);
			break;
*/
		default:
			CommonMsgProccess(msgId);
			break;
	}
}

bool I2SInPlayDeinit(void)
{
	if(sI2SInPlayCt == NULL)
	{
		return TRUE;
	}
	APP_DBG("I2SIn Play deinit\n");
	if(IsAudioPlayerMute() == FALSE)
	{
		HardWareMuteOrUnMute();
	}	
	//Kill used services
#if	0//defined(CFG_FUNC_REMIND_SOUND_EN)
	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
	AudioCoreSourceDisable(PLAYBACK_SOURCE_NUM);
	DecoderServiceDeinit(DECODER_REMIND_CHANNEL);
	while(GetDecoderServiceState()!=TaskStateStopped)
	{
		APP_DBG("I2S IN:%d\n",GetDecoderServiceState());
		osTaskDelay(1);
	}
#endif

	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_MCLK_GPIO), 0);//mclk
	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_LRCLK_GPIO),0);//lrclk
	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_BCLK_GPIO), 0);//bclk
#ifdef I2S_DIN_GPIO
	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_DIN_GPIO), 0);//di
#endif

	AudioCoreSourceDisable(I2SIN_SOURCE_NUM);
	PauseAuidoCore();
	
	I2SInPlayResFree();
	ModeCommonDeinit();//通路全部释放

	osPortFree(sI2SInPlayCt);
	sI2SInPlayCt = NULL;

	return TRUE;
}

#endif//#ifdef CFG_APP_I2SIN_MODE_EN
