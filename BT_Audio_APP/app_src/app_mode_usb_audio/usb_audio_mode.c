/**
 **************************************************************************************
 * @file    usb_audio_mode.c
 * @brief
 *
 * @author  Owen
 * @version V1.0.0
 *
 * $Created: 2018-04-27 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include "main_task.h"
#include "device_detect.h"
#include "otg_detect.h"
#include "otg_device_stor.h"
#include "otg_device_hcd.h"
#include "otg_device_audio.h"
#include "otg_device_standard_request.h"
#include "ctrlvars.h"
#include "remind_sound.h"
#include "audio_vol.h"
#include "audio_effect.h"
#include "irqn.h"
#include "dma.h"
#include "clk.h"
#include "usb_audio_mode.h"
#include "breakpoint.h"
#include "user_effect_parameter.h"
#ifdef CFG_APP_USB_AUDIO_MODE_EN


MemHandle UsbphBuffer;
//AEC
#define AEC_REF_BUF_LEN   (256 * 1 * sizeof(PCM_DATA_TYPE)) * 4		//(framesize * channel * wordlen) * count
static uint8_t usbph_raw_buf[AEC_REF_BUF_LEN] = {0};

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
		255
	#endif
#endif
};

void UsbDevicePlayResRelease(void)
{
#ifdef CFG_OTG_MODE_AUDIO_EN
	if(UsbAudioSpeaker.PCMBuffer != NULL)
	{
		APP_DBG("UsbAudioSpeaker.PCMBuffer free\n");
		osPortFree(UsbAudioSpeaker.PCMBuffer);
		UsbAudioSpeaker.PCMBuffer = NULL;
	}
#endif
#ifdef CFG_OTG_MODE_MIC_EN
	if(UsbAudioMic.PCMBuffer != NULL)
	{
		APP_DBG("UsbAudioMic.PCMBuffer free\n");
		osPortFree(UsbAudioMic.PCMBuffer);
		UsbAudioMic.PCMBuffer = NULL;
	}
#endif

}

bool UsbDevicePlayResMalloc(uint16_t SampleLen)
{
	uint32_t sampleRate  = AudioCoreMixSampleRateGet(DefaultNet);
	uint32_t buf_len;

	APP_DBG("UsbDevicePlayResMalloc %u\n", SampleLen);
//pc->chip
#ifdef CFG_OTG_MODE_AUDIO_EN
	//Speaker FIFO
	if(USBD_AUDIO_FREQ > sampleRate)
		buf_len = SampleLen * 16 * (USBD_AUDIO_FREQ / sampleRate);
	else
		buf_len = SampleLen * 16 * (sampleRate / USBD_AUDIO_FREQ);

	UsbAudioSpeaker.PCMBuffer = osPortMalloc(buf_len);
	if(UsbAudioSpeaker.PCMBuffer == NULL)
	{
		APP_DBG("UsbAudioSpeaker.PCMBuffer memory error\n");
		return FALSE;
	}
	memset(UsbAudioSpeaker.PCMBuffer, 0, buf_len);
	MCUCircular_Config(&UsbAudioSpeaker.CircularBuf, UsbAudioSpeaker.PCMBuffer, buf_len);
#endif//end of CFG_RES_USB_IN_EN

#ifdef CFG_OTG_MODE_MIC_EN
	//MIC FIFO
	if(USBD_AUDIO_MIC_FREQ > sampleRate)
		buf_len = SampleLen * 16 * (USBD_AUDIO_MIC_FREQ / sampleRate);
	else
		buf_len = SampleLen * 16 * (sampleRate / USBD_AUDIO_MIC_FREQ);

	UsbAudioMic.PCMBuffer = osPortMalloc(buf_len);
	if(UsbAudioMic.PCMBuffer == NULL)
	{
		APP_DBG("UsbAudioMic.PCMBuffer memory error\n");
		return FALSE;
	}
	memset(UsbAudioMic.PCMBuffer, 0, buf_len);
	MCUCircular_Config(&UsbAudioMic.CircularBuf, UsbAudioMic.PCMBuffer,buf_len);
#endif///end of CFG_REGS_AUDIO_USB_OUT_EN

	return TRUE;
}

bool UsbDevicePlayResInit(void)
{
	//Core Source1 para
	AudioCoreIO	AudioIOSet;
#ifdef CFG_OTG_MODE_AUDIO_EN
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
#if !defined(CFG_PARA_AUDIO_USB_IN_SYNC)
#if !defined(CFG_PARA_AUDIO_USB_IN_SRC)
	AudioIOSet.Adapt = STD;
#else
	AudioIOSet.Adapt = SRC_ONLY;
#endif
#else //需微调   启用硬件时需要和Out协同
#if !defined(CFG_PARA_AUDIO_USB_IN_SRC)
	AudioIOSet.Adapt = SRA_ONLY;//CLK_ADJUST_ONLY;
#else
	AudioIOSet.Adapt = SRC_SRA;//STD;//SRC_ADJUST;
#endif
#endif

	AudioIOSet.Sync = TRUE;
	AudioIOSet.Channels = PACKET_CHANNELS_NUM;
	AudioIOSet.Net = AudioEffect_getSourceNet(USB_AUDIO_SOURCE_NUM);
	AudioIOSet.DataIOFunc = UsbAudioSpeakerDataGet;
	AudioIOSet.LenGetFunc = UsbAudioSpeakerDataLenGet;
	AudioIOSet.Depth = UsbAudioSpeakerDepthGet();
	AudioIOSet.LowLevelCent = 40;
	AudioIOSet.HighLevelCent = 60;
	if(UsbAudioSpeaker.AudioSampleRate) //已枚举
	{
		AudioIOSet.SampleRate = UsbAudioSpeaker.AudioSampleRate;
	}
	else
	{
		AudioIOSet.SampleRate = AudioCoreMixSampleRateGet(AudioIOSet.Net);//初始值
	}
#ifdef CFG_AUDIO_OUT_AUTO_SAMPLE_RATE_44100_48000
	AudioOutSampleRateSet(AudioIOSet.SampleRate);
#endif
#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 0;//不需要数据进行位宽扩展
#endif
	if(!AudioCoreSourceInit(&AudioIOSet, USB_AUDIO_SOURCE_NUM))
	{
		DBG("Usbin source error!\n");
		return FALSE;
	}
	AudioCoreSourceAdjust(USB_AUDIO_SOURCE_NUM, TRUE);//仅在init通路配置微调后，通路使能时 有效

#endif
#ifdef CFG_OTG_MODE_MIC_EN
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
#if !defined(CFG_PARA_AUDIO_USB_OUT_SYNC)
#if !defined(CFG_PARA_AUDIO_USB_OUT_SRC)
	AudioIOSet.Adapt = STD;
#else
	AudioIOSet.Adapt = SRC_ONLY;
#endif
#else //需微调   启用硬件时需要和Out协同
#if !defined(CFG_PARA_AUDIO_USB_OUT_SRC)
	AudioIOSet.Adapt = SRA_ONLY;//CLK_ADJUST_ONLY;
#else
	AudioIOSet.Adapt = SRC_SRA;//SRC_ADJUST;
#endif
#endif
	AudioIOSet.Sync = TRUE;//FALSE;//
	AudioIOSet.Channels = MIC_CHANNELS_NUM;
	AudioIOSet.Net = AudioEffect_getSinkNet(USB_AUDIO_SINK_NUM);
	AudioIOSet.DataIOFunc = UsbAudioMicDataSet;
	AudioIOSet.LenGetFunc = UsbAudioMicSpaceLenGet;
	AudioIOSet.Depth = UsbAudioMicDepthGet();
	AudioIOSet.LowLevelCent = 40;
	AudioIOSet.HighLevelCent = 60;

	if(UsbAudioMic.AudioSampleRate) //已枚举
	{
		AudioIOSet.SampleRate = UsbAudioMic.AudioSampleRate;
	}
	else
	{
		AudioIOSet.SampleRate = USBD_AUDIO_MIC_FREQ5;//初始值
	}
#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 0;//不需要数据进行位宽扩展
#endif
	if(!AudioCoreSinkInit(&AudioIOSet, USB_AUDIO_SINK_NUM))
	{
		DBG("Usbout sink error!\n");
	}
	AudioCoreSinkAdjust(USB_AUDIO_SINK_NUM,TRUE);
#endif


	//CC_TODO: AEC Reference source and sink
	AudioUsbPhoneBufInit();

	//New Reference sink
	AudioIOSet.Adapt = SRC_ONLY;
	AudioIOSet.Sync = FALSE;//
	AudioIOSet.Channels = 1;
	AudioIOSet.Net = AudioEffect_getSinkNet(AEC_REF_SINK_NUM);
	AudioIOSet.DataIOFunc = AudioUsbPhoneBuf_DataPut;
	AudioIOSet.LenGetFunc = AudioUsbPhoneBuf_SpaceGet;
	// AudioIOSet.Depth = AudioCoreFrameSizeGet(DefaultNet) * 4;//fifo 采样点深度
	AudioIOSet.Depth = AudioCoreFrameSizeGet(AudioIOSet.Net) * 2;//fifo 采样点深度
	// AudioIOSet.LowLevelCent = 40;
	// AudioIOSet.HighLevelCent = 60;

	AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 0;//需要数据进行位宽扩展

	AudioIOSet.SampleRate = AudioCoreMixSampleRateGet(AudioEffect_getSourceNet(AEC_REF_SOURCE_NUM));
	if(!AudioCoreSinkInit(&AudioIOSet, AEC_REF_SINK_NUM))
	{
		DBG("AEC_REF_SINK_NUM sink error!\n");
	}

	//New Reference source
	AudioIOSet.Adapt = STD;
	AudioIOSet.Sync = FALSE;//
	AudioIOSet.Channels = 1;
	AudioIOSet.Net = AudioEffect_getSourceNet(AEC_REF_SOURCE_NUM);
	AudioIOSet.DataIOFunc = AudioUsbPhoneBuf_DataGet;
	AudioIOSet.LenGetFunc = AudioUsbPhoneBuf_DataLenGet;
	// AudioIOSet.Depth = AudioCoreFrameSizeGet(AECNet) * 4;//fifo 采样点深度
	AudioIOSet.Depth = AudioCoreFrameSizeGet(AudioIOSet.Net) * 2;//fifo 采样点深度
	// AudioIOSet.LowLevelCent = 40;
	// AudioIOSet.HighLevelCent = 60;
	AudioIOSet.SampleRate = AudioCoreMixSampleRateGet(AudioIOSet.Net);
	if(!AudioCoreSourceInit(&AudioIOSet, AEC_REF_SOURCE_NUM))
	{
		DBG("AEC_REF_SOURCE_NUM source error!\n");
	}
	AudioCoreSinkEnable(AEC_REF_SINK_NUM);
	AudioCoreSourceEnable(AEC_REF_SOURCE_NUM);


	//Core Process
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
//	UsbDevicePlayCt->AudioCoreUsb->AudioEffectProcess = (AudioCoreProcessFunc)AudioMusicProcess;
	AudioCoreProcessConfig((void*)AudioUsbAECProcess);
#else
//	UsbDevicePlayCt->AudioCoreUsb->AudioEffectProcess = (AudioCoreProcessFunc)AudioBypassProcess;
	AudioCoreProcessConfig((void*)AudioBypassProcess);
#endif
	return TRUE;
}
//usb声卡模式硬件相关初始化
void UsbDevicePlayHardwareInit(void)
{
#ifdef CFG_OTG_MODE_AUDIO_EN
	if(UsbAudioSpeaker.InitOk != 1)
	{
		//不清除FIFO,只清除usb声卡相关配置
		memset(&UsbAudioSpeaker,0,sizeof(UsbAudio)-sizeof(MCU_CIRCULAR_CONTEXT)-sizeof(int16_t*));
		UsbAudioSpeaker.Channels   = PACKET_CHANNELS_NUM;
		UsbAudioSpeaker.LeftGain   = UsbValToMcuGain(AUDIO_MAX_VOLUME);
		UsbAudioSpeaker.RightGain  = UsbValToMcuGain(AUDIO_MAX_VOLUME);
	}
#endif

#ifdef CFG_OTG_MODE_MIC_EN
	if(UsbAudioMic.InitOk != 1)
	{
		//不清除FIFO,只清除usb声卡相关配置
		memset(&UsbAudioMic,0,sizeof(UsbAudio)-sizeof(MCU_CIRCULAR_CONTEXT)-sizeof(int16_t*));
		UsbAudioMic.Channels       = MIC_CHANNELS_NUM;
		UsbAudioMic.LeftGain       = UsbValToMcuGain(AUDIO_MAX_VOLUME);
		UsbAudioMic.RightGain      = UsbValToMcuGain(AUDIO_MAX_VOLUME);
	}
#endif

	OTG_DeviceModeSel(CFG_PARA_USB_MODE, USB_VID, USBPID(CFG_PARA_USB_MODE));
#if( (CFG_PARA_USB_MODE >= READER))
	if(GetSysModeState(ModeCardAudioPlay)!=ModeStateSusend)
	{
		CardPortInit(CFG_RES_CARD_GPIO);
		if(SDCard_Init() == NONE_ERR)
		{
			APP_DBG("SD INIT OK\n");
			//sd_link = 1;
		}
	}
	OTG_DeviceStorInit();
#endif
	OTG_DeviceFifoInit();
	OTG_DeviceInit();
	NVIC_EnableIRQ(Usb_IRQn);
}

static uint8_t sysEffectMode;
//USB声卡模式参数配置，资源初始化
bool UsbDevicePlayInit(void)
{
	APP_DBG("UsbDevice Play Init\n");

	sysEffectMode = mainAppCt.EffectMode;	//保存EffectMode，退出AEC以后需要恢复
	mainAppCt.EffectMode = EFFECT_MODE_USBAEC;

	//1st：DMA通道初始化
	DMA_ChannelAllocTableSet((uint8_t *)DmaChannelMap);
	//2st：usb声卡相关硬件配置
	UsbDevicePlayHardwareInit();
	//3st：配置系统标准通路
	if(!ModeCommonInit())
	{
		ModeCommonDeinit();
		return FALSE;
	}
	//4st：usb声卡通路资源申请及配置
	if(!UsbDevicePlayResMalloc(AudioCoreFrameSizeGet(DefaultNet)))
	{
		APP_DBG("UsbDevicePlayResMalloc Res Error!\n");
		return FALSE;
	}
	if(!UsbDevicePlayResInit())
	{
		APP_DBG("UsbDevicePlayResInit Res Error!\n");
		return FALSE;
	}
	//Core Process
#ifdef CFG_FUNC_AUDIO_EFFECT_EN
	AudioCoreProcessConfig((AudioCoreProcessFunc)AudioUsbAECProcess);
#else
	AudioCoreProcessConfig((AudioCoreProcessFunc)AudioBypassProcess);
#endif

	AudioCodecGainUpdata();//update hardware config

#ifdef CFG_COMMUNICATION_BY_USB
	SetUSBDeviceInitState(TRUE);
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(RemindSoundServiceItemRequest(SOUND_REMIND_SHENGKAM, REMIND_PRIO_NORMAL) == FALSE)
	{
		if(IsAudioPlayerMute() == TRUE)
		{
			HardWareMuteOrUnMute();
		}
	}
#endif
#ifdef CFG_OTG_MODE_AUDIO_EN
	AudioCoreSourceUnmute(USB_AUDIO_SOURCE_NUM,TRUE,TRUE);
#endif

#ifndef CFG_FUNC_REMIND_SOUND_EN
	if(IsAudioPlayerMute() == TRUE)
	{
		HardWareMuteOrUnMute();
	}
#endif
	return TRUE;
}
void UsbDevicePlayRun(uint16_t msgId)
{
	switch(msgId)
	{
	case MSG_HID_KEY1_DOWN:
		APP_DBG("KEY1_DOWN\n");
		OTG_DeviceAudioSendHidKeyDown(AUDIO_VOL_UP);
		break;

	case MSG_HID_KEY2_DOWN:
		APP_DBG("KEY2_DOWN\n");
		OTG_DeviceAudioSendHidKeyDown(AUDIO_VOL_DN);
		break;

	case MSG_HID_KEY3_DOWN:
		APP_DBG("KEY3_DOWN\n");
		OTG_DeviceAudioSendHidKeyDown(AUDIO_PP);
		break;

	case MSG_HID_KEY_UP:
		APP_DBG("KEY_UP\n");
		OTG_DeviceAudioSendHidKeyUp();
		break;

	case MSG_PLAY_PAUSE:
		APP_DBG("Play Pause\n");
		OTG_DeviceAudioSendPcCmd(AUDIO_PP);
		break;

	case MSG_PRE:
		APP_DBG("PRE Song\n");
		OTG_DeviceAudioSendPcCmd(AUDIO_PREV);
		break;

	case MSG_NEXT:
		APP_DBG("next Song\n");
		OTG_DeviceAudioSendPcCmd(AUDIO_NEXT);
		break;

	case MSG_MUSIC_VOLUP:
		APP_DBG("VOLUP\n");
		OTG_DeviceAudioSendPcCmd(AUDIO_VOL_UP);
		break;

	case MSG_MUSIC_VOLDOWN:
		APP_DBG("VOLDOWN\n");
		OTG_DeviceAudioSendPcCmd(AUDIO_VOL_DN);
		break;

	default:
		CommonMsgProccess(msgId);
		break;
	}
#ifdef USB_CRYSTA_FREE_EN
	Clock_USBCrystaFreeAdjustProcess();
#endif
	OTG_DeviceRequestProcess();
#if( (CFG_PARA_USB_MODE >= READER))
	OTG_DeviceStorProcess();
#endif
}

bool UsbDevicePlayDeinit(void)
{
	APP_DBG("UsbDevice Play Deinit\n");
#ifdef CFG_OTG_MODE_AUDIO_EN
	AudioCoreSourceMute(USB_AUDIO_SOURCE_NUM,TRUE,TRUE);
#endif
	if(IsAudioPlayerMute() == FALSE)
	{
		HardWareMuteOrUnMute();
	}	
	
	PauseAuidoCore();	

	//注意：AudioCore父任务调整到mainApp下，此处只关闭AudioCore通道，不关闭任务
	AudioCoreProcessConfig((void*)AudioNoAppProcess);
#ifdef CFG_OTG_MODE_AUDIO_EN
	AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);
#endif
#ifdef CFG_OTG_MODE_MIC_EN
	AudioCoreSinkDisable(USB_AUDIO_SINK_NUM);
#endif

	AudioCoreSourceDeinit(AEC_REF_SOURCE_NUM);
	AudioCoreSinkDeinit(AEC_REF_SINK_NUM);
	AudioUsbPhoneBufDeInit();

	if(!OTG_PortDeviceIsLink())
	{
		APP_DBG("OTG_DeviceDisConnect\n");
		OTG_DeviceDisConnect();
#ifdef CFG_COMMUNICATION_BY_USB
		SetUSBDeviceInitState(FALSE);
#endif
	}
	else
	{
		APP_DBG("OTG_DeviceModeSel HID\n");
		OTG_DeviceModeSel(HID, USB_VID, USBPID(HID));
		OTG_DeviceFifoInit();
		OTG_DeviceInit();
	}
	
	//NVIC_DisableIRQ(Usb_IRQn);
	UsbDevicePlayResRelease();
#ifdef CFG_OTG_MODE_AUDIO_EN
	AudioCoreSourceDeinit(USB_AUDIO_SOURCE_NUM);
#endif
#ifdef CFG_OTG_MODE_MIC_EN
	AudioCoreSinkDeinit(USB_AUDIO_SINK_NUM);
#endif
	ModeCommonDeinit();//通路全部释放

	mainAppCt.EffectMode = sysEffectMode;	//恢复EffectMode

	return TRUE;
}

bool UsbDevicePlayMixInit(void)
{
	APP_DBG("UsbDevice Play Mix Init\n");

	if(!UsbDevicePlayResMalloc(AudioCoreFrameSizeGet(DefaultNet)))
	{
		APP_DBG("UsbDevicePlayResMalloc Res Error!\n");
		return FALSE;
	}
	UsbDevicePlayResInit();
#ifdef CFG_OTG_MODE_AUDIO_EN
	AudioCoreSourceUnmute(USB_AUDIO_SOURCE_NUM,TRUE,TRUE);
#endif
	return TRUE;
}
bool UsbDevicePlayMixDeinit(void)
{
	APP_DBG("UsbDevice Play Mix Deinit\n");
#ifdef CFG_OTG_MODE_AUDIO_EN
	AudioCoreSourceMute(USB_AUDIO_SOURCE_NUM,TRUE,TRUE);
	AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);
#endif
#ifdef CFG_OTG_MODE_MIC_EN
	AudioCoreSinkDisable(USB_AUDIO_SINK_NUM);
#endif
	UsbDevicePlayResRelease();
#ifdef CFG_OTG_MODE_AUDIO_EN
	AudioCoreSourceDeinit(USB_AUDIO_SOURCE_NUM);
#endif
#ifdef CFG_OTG_MODE_MIC_EN
	AudioCoreSinkDeinit(USB_AUDIO_SINK_NUM);
#endif
	return TRUE;
}



void AudioUsbPhoneBufInit(void)
{
	//mv_mopen(&UsbphBuffer, usbph_raw_buf, sizeof(usbph_raw_buf) - 4, NULL);

	UsbphBuffer.addr = usbph_raw_buf;
	UsbphBuffer.mem_capacity = AEC_REF_BUF_LEN;
	UsbphBuffer.mem_len = 0;	//store data can be used as a delay, not use so set to 0
	UsbphBuffer.p = 0;
}

void AudioUsbPhoneBufDeInit(void)
{
	UsbphBuffer.addr = NULL;
	UsbphBuffer.mem_capacity = 0;
	UsbphBuffer.mem_len = 0;
	UsbphBuffer.p = 0;
}

uint16_t AudioUsbPhoneBuf_DataLenGet(void)
{
	return mv_msize(&UsbphBuffer)/sizeof(PCM_DATA_TYPE);//always return samples
}

uint16_t AudioUsbPhoneBuf_SpaceGet(void)
{
	return mv_mremain(&UsbphBuffer)/sizeof(PCM_DATA_TYPE);//always return samples
}

uint16_t AudioUsbPhoneBuf_DataGet(void *Buffer, uint16_t Len)
{
	uint16_t tlen = mv_mread(Buffer, sizeof(PCM_DATA_TYPE), Len, &UsbphBuffer);
//	DBG("#  tlen = %d, datalen:%d\n", tlen/sizeof(PCM_DATA_TYPE), AudioUsbPhoneBuf_DataLenGet());
	return tlen/sizeof(PCM_DATA_TYPE);
}
uint16_t AudioUsbPhoneBuf_DataPut(void *Buffer, uint16_t Len)
{
	uint16_t tlen = mv_mwrite(Buffer, sizeof(PCM_DATA_TYPE), Len, &UsbphBuffer);
//	DBG("put = %d, space:%d\n", tlen/sizeof(PCM_DATA_TYPE), AudioUsbPhoneBuf_SpaceGet());
	return tlen/sizeof(PCM_DATA_TYPE);
}


#endif //CFG_APP_USB_AUDIO_MODE_EN
