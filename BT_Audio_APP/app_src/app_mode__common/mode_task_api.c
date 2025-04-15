/*
 * mode_task_api.c
 *
 *  Created on: Mar 30, 2021
 *      Author: piwang
 */
#include "main_task.h"
#include "audio_core_service.h"
#include "roboeffect_prot.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "ctrlvars.h"
#include "breakpoint.h"
#include "remind_sound.h"
#include "i2s_interface.h"
#include "dma.h"
#include "watchdog.h"
//service
#include "bt_manager.h"
#include "audio_vol.h"
#include "audio_effect.h"
#include "remind_sound.h"
#include "hdmi_in_api.h"
#include "roboeffect_api.h"
#include "user_effect_parameter.h"
//app
#include "bt_stack_service.h"
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
#include "bt_app_avrcp_deal.h"
#endif

#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
#include "spdif_out.h"
#endif

#define roboeffect_malloc osPortMallocFromEnd
#define roboeffect_free osPortFree
extern volatile SysModeStruct SysMode[];
extern uint32_t GetModeIndexInModeLoop(SysModeNumber *sys_mode);

#ifdef CFG_FUNC_I2S_MIX_MODE
extern void I2S_MixInit(void);
extern void I2S_MixDeinit(void);
#endif
#ifdef CFG_FUNC_LINEIN_MIX_MODE
extern bool LineInMixPlayInit(void);
extern bool LineInMixPlayDeinit(void);
#endif

#ifdef CFG_REMIND_SOUND_DECODING_USE_LIBRARY
extern int32_t RemindMp3DecoderInit(void);
extern int32_t RemindMp3DecoderDeinit(void);
#endif

#ifdef CFG_APP_USB_PLAY_MODE_EN
void WaitUdiskUnlock(void)
{
	if(GetSysModeState(ModeUDiskAudioPlay) == ModeStateDeinit)
	{
		while(osMutexLock_1000ms(UDiskMutex) != TRUE)WDG_Feed();
	}
}
#endif

#ifdef CFG_APP_CARD_PLAY_MODE_EN
void SDCardForceExitFuc(void)
{
	if(GetSysModeState(ModeCardAudioPlay) == ModeStateDeinit)
	{
		SDCard_Force_Exit();
	}
}
#endif

void PauseAuidoCore(void)
{
	while(GetAudioCoreServiceState() != TaskStatePaused)
	{
		AudioCoreServicePause();
		osTaskDelay(2);
	}
}

bool RoboeffectInit()
{
	if(AudioCore.Roboeffect.effect_addr)
	{
		uint8_t *params = AudioCore.Roboeffect.user_effect_parameters + 5;
		uint16_t data_len = *(uint16_t *)AudioCore.Roboeffect.user_effect_parameters - 5;
		uint8_t len = 0;
		while(data_len)
		{
			if(*params == AudioCore.Roboeffect.effect_addr)
			{
				params += 2;
				*params = AudioCore.Roboeffect.effect_enable;
				break;
			}
			else
			{
				params++;
				len = *params;
				params += (len + 1);
				data_len -= (len + 1);
			}
		};

		DBG("Roboeffect ReInit:0x%x\n", AudioCore.Roboeffect.effect_addr);
	}
	else
	{
		if(AudioCore.Roboeffect.user_effect_parameters)
		{
			//先释放资源
			osPortFree(AudioCore.Roboeffect.user_effect_parameters);
		}
		
		ROBOEFFECT_EFFECT_PARA *para = get_user_effect_parameters(mainAppCt.EffectMode);

		AudioCore.Roboeffect.effect_count = para->user_effect_list->count + 0x80;
		AudioCore.Roboeffect.user_effect_steps = para->user_effect_steps;
		AudioCore.Roboeffect.user_effects_script = para->user_effects_script;
		AudioCore.Roboeffect.user_effects_script_len = para->get_user_effects_script_len();
		AudioCore.Roboeffect.user_effect_list = para->user_effect_list;
		AudioCore.Roboeffect.user_effect_parameters = osPortMalloc(get_user_effect_parameters_len(para->user_effect_parameters) * sizeof(uint8_t));
		memcpy(AudioCore.Roboeffect.user_effect_parameters, para->user_effect_parameters,get_user_effect_parameters_len(para->user_effect_parameters) * sizeof(uint8_t));
		AudioCore.Roboeffect.user_module_parameters = para->user_module_parameters;
	}

	//When effect change framesize to 512 and then switch mode, AudioCore need reset to default.
	//AudioCoreFrameSizeSet(DefaultNet, CFG_PARA_SAMPLES_PER_FRAME);
	AudioCoreFrameSizeSet(DefaultNet, AudioCore.Roboeffect.user_effect_list->frame_size);

	AudioCore.Roboeffect.roboeffect_size_max = roboeffect_get_memory_max_size(
			AudioCore.Roboeffect.user_effect_steps, AudioCore.Roboeffect.user_effect_list, AudioCore.Roboeffect.user_effect_parameters);
	AudioCore.Roboeffect.roboeffect_size = roboeffect_get_memory_current_size(
			AudioCore.Roboeffect.user_effect_steps, AudioCore.Roboeffect.user_effect_list, AudioCore.Roboeffect.user_effect_parameters);
	DBG("max memory: %ld, current memory: %ld\n", AudioCore.Roboeffect.roboeffect_size_max, AudioCore.Roboeffect.roboeffect_size);

	if(AudioCore.Roboeffect.roboeffect_size < 0 || AudioCore.Roboeffect.roboeffect_size_max < 0)
	{
		DBG("get context size failed. %ld,%ld\n", AudioCore.Roboeffect.roboeffect_size, AudioCore.Roboeffect.roboeffect_size_max);
		return FALSE;
	}
	/**
	 * malloc context memory
	*/
	if(AudioCore.Roboeffect.roboeffect_size < xPortGetFreeHeapSize())
	{
		AudioCore.Roboeffect.context_memory = roboeffect_malloc(AudioCore.Roboeffect.roboeffect_size);
		if(AudioCore.Roboeffect.context_memory == NULL)
		{
			return FALSE;
		}
		/**
		 * initial roboeffect context memory
		*/
		if(ROBOEFFECT_ERROR_OK != roboeffect_init(AudioCore.Roboeffect.context_memory,
												  AudioCore.Roboeffect.roboeffect_size,
												  AudioCore.Roboeffect.user_effect_steps,
												  AudioCore.Roboeffect.user_effect_list,
												  AudioCore.Roboeffect.user_effect_parameters) )
		{
			DBG("roboeffect_init failed.\n");
			return FALSE;
		}
		else
		{
			DBG("roboeffect_init ok.\n");
			AudioCore.Roboeffect.effect_addr = 0;
			Roboeffect_GetAudioEffectMaxValue();

			////Audio Core & roboeffect音量配置
			SystemVolSet();
		}
	}
	else
	{
		DBG("**************************************\n");
		DBG("Error:memory is not enough!!!\n");
		DBG("malloc:%ld, leave:%ld\n", AudioCore.Roboeffect.roboeffect_size_max, xPortGetFreeHeapSize());
		DBG("**************************************\n");
		return FALSE;
	}

	roboeffect_prot_init();
	return TRUE;
}

#ifdef CFG_RES_AUDIO_I2SOUT_EN
void AudioI2sOutParamsSet(void)
{
	I2SParamCt i2s_set;
	i2s_set.IsMasterMode = CFG_RES_I2S_MODE;// 0:master 1:slave
	i2s_set.SampleRate = CFG_PARA_I2S_SAMPLERATE; //外设采样率
	i2s_set.I2sFormat = I2S_FORMAT_I2S;
#ifdef	CFG_AUDIO_WIDTH_24BIT
	i2s_set.I2sBits = I2S_LENGTH_24BITS;
#else
	i2s_set.I2sBits = I2S_LENGTH_16BITS;
#endif
	i2s_set.I2sTxRxEnable = 1;

	i2s_set.TxPeripheralID = PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_I2S_MODULE;

	i2s_set.TxBuf = (void*)mainAppCt.I2SFIFO;

	i2s_set.TxLen = mainAppCt.I2SFIFO_LEN;

	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_MCLK_GPIO), GET_I2S_GPIO_MODE(I2S_MCLK_GPIO));//mclk
	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_LRCLK_GPIO),GET_I2S_GPIO_MODE(I2S_LRCLK_GPIO));//lrclk
	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_BCLK_GPIO), GET_I2S_GPIO_MODE(I2S_BCLK_GPIO));//bclk
	GPIO_PortAModeSet(GET_I2S_GPIO_PORT(I2S_DOUT_GPIO), GET_I2S_GPIO_MODE(I2S_DOUT_GPIO));//do

	I2S_AlignModeSet(CFG_RES_I2S_MODULE, I2S_LOW_BITS_ACTIVE);
	AudioI2S_Init(CFG_RES_I2S_MODULE, &i2s_set);//
}
#endif

//配置系统标准通路
bool ModeCommonInit(void)
{
	AudioCoreIO AudioIOSet;
	uint16_t FifoLenStereo;

	if(!RoboeffectInit())
	{
		DBG("!!!roboeffect init must be earlier than sink init!!!.\n");
		if(AudioCore.Roboeffect.effect_addr)
		{
			AudioCore.Roboeffect.effect_enable = 0;
			DBG("roboeffect init again because cannot enable effect:%d\n", RoboeffectInit());
		}
	}

	FifoLenStereo = AudioCoreFrameSizeGet(DefaultNet) * sizeof(PCM_DATA_TYPE) * 2 * 2;//立体声8倍大小于帧长，单位byte

	DefaultParamgsInit();	//refresh local hardware config params(just storage not set)

	//////////申请DMA fifo
#ifdef CFG_RES_AUDIO_DAC0_EN
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
	AudioIOSet.Depth = AudioCore.FrameSize[0] * 2 ;
#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 0;//不需要做位宽转换处理
#endif
	if(!AudioCoreSinkIsInit(AUDIO_DAC0_SINK_NUM))
	{
		mainAppCt.DACFIFO_LEN = AudioIOSet.Depth * sizeof(PCM_DATA_TYPE) * 2;
		mainAppCt.DACFIFO = (uint32_t*)osPortMalloc(mainAppCt.DACFIFO_LEN);//DAC fifo
		if(mainAppCt.DACFIFO != NULL)
		{
			memset(mainAppCt.DACFIFO, 0, mainAppCt.DACFIFO_LEN);
		}
		else
		{
			APP_DBG("malloc DACFIFO error\n");
			return FALSE;
		}

		//sink0
		AudioIOSet.Adapt = STD;
		AudioIOSet.Sync = TRUE;
		AudioIOSet.Channels = 2;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.DataIOFunc = AudioDAC0_DataSet;
		AudioIOSet.LenGetFunc = AudioDAC0_DataSpaceLenGet;
		if(!AudioCoreSinkInit(&AudioIOSet, AUDIO_DAC0_SINK_NUM))
		{
			DBG("Dac init error");
			return FALSE;
		}

		DACParamCt ct;
		uint16_t BitWidth;
	#ifdef CHIP_DAC_USE_DIFF
		ct.DACModel = DAC_Diff;
	#else
		ct.DACModel = DAC_Single;
	#endif

	#ifdef CHIP_DAC_USE_PVDD16
		ct.PVDDModel = PVDD16;
	#else
		ct.PVDDModel = PVDD33;
	#endif
		ct.DACLoadStatus = DAC_NOLoad;
		ct.DACEnergyModel = DACCommonEnergy;
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		BitWidth = 24;
	#else
		BitWidth = 16;
	#endif
		AudioDAC_Init(&ct,mainAppCt.SampleRate,BitWidth, (void*)mainAppCt.DACFIFO, mainAppCt.DACFIFO_LEN, NULL, 0);
	}
	else//sam add,20230221
	{
		AudioDAC0_SampleRateChange(CFG_PARA_SAMPLE_RATE);
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioCore.AudioSink[AUDIO_DAC0_SINK_NUM].BitWidth = AudioIOSet.IOBitWidth;
		AudioCore.AudioSink[AUDIO_DAC0_SINK_NUM].BitWidthConvFlag = AudioIOSet.IOBitWidthConvFlag;
	#endif
	}
	AudioCoreSinkEnable(AUDIO_DAC0_SINK_NUM);
#endif

	//Mic1 analog  = Soure0.
	//AudioADC_AnaInit();
	//AudioADC_VcomConfig(1);//MicBias en
	// AudioADC_MicBias1Enable(1);
#if CFG_RES_MIC_SELECT
	if(!AudioCoreSourceIsInit(MIC_SOURCE_NUM))
	{
		mainAppCt.ADCFIFO = (uint32_t*)osPortMalloc(FifoLenStereo);//ADC fifo
		if(mainAppCt.ADCFIFO != NULL)
		{
			memset(mainAppCt.ADCFIFO, 0, FifoLenStereo);
		}
		else
		{
			APP_DBG("malloc ADCFIFO error\n");
			return FALSE;
		}

		AudioADC_DynamicElementMatch(ADC1_MODULE, TRUE, TRUE);
//		AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);
//		AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);
//		AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 15, 4);//0db bypass
//		AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 15, 4);

		//Mic1   digital
	#ifdef CFG_AUDIO_WIDTH_24BIT
		AudioADC_DigitalInit(ADC1_MODULE, mainAppCt.SampleRate,ADC_WIDTH_24BITS,(void*)mainAppCt.ADCFIFO, FifoLenStereo);
	#else
		AudioADC_DigitalInit(ADC1_MODULE, mainAppCt.SampleRate,ADC_WIDTH_16BITS,(void*)mainAppCt.ADCFIFO, FifoLenStereo);
	#endif

		//Soure0.
		memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
		AudioIOSet.Adapt = STD;
		AudioIOSet.Sync = TRUE;
		AudioIOSet.Channels = 1;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.DataIOFunc = AudioADC1_DataGet;
		AudioIOSet.LenGetFunc = AudioADC1_DataLenGet;
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;//需要数据进行位宽扩展
	#endif
		if(!AudioCoreSourceInit(&AudioIOSet, MIC_SOURCE_NUM))
		{
			DBG("mic Source error");
			return FALSE;
		}
	}
#ifdef CFG_ADCDAC_SEL_LOWPOWERMODE
	AudioADC_AnaInit(ADC1_MODULE,CHANNEL_LEFT,MIC_LEFT,Single,ADCLowEnergy,31 - gCtrlVars.HwCt.ADC1PGACt.pga_mic_gain);
#else
	AudioADC_AnaInit(ADC1_MODULE,CHANNEL_LEFT,MIC_LEFT,Single,ADCCommonEnergy,31 - gCtrlVars.HwCt.ADC1PGACt.pga_mic_gain);
#endif // CFG_ADCDAC_SEL_LOWPOWERMODE
	//MADC_MIC_PowerUP(SingleEnded);
	AudioCoreSourceEnable(MIC_SOURCE_NUM);
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(!AudioCoreSourceIsInit(REMIND_SOURCE_NUM))
	{
		memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
		AudioIOSet.Adapt = STD;//SRC_ONLY;
		AudioIOSet.SampleRate = CFG_PARA_SAMPLE_RATE;//初始值
		AudioIOSet.Sync = FALSE;
		AudioIOSet.Channels = 1;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.DataIOFunc = RemindDataGet;
		AudioIOSet.LenGetFunc = RemindDataLenGet;

	#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_16BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 1;//需要数据进行位宽扩展
	#endif
		if(!AudioCoreSourceInit(&AudioIOSet, REMIND_SOURCE_NUM))
		{
			DBG("remind source error!\n");
			SoftFlagRegister(SoftFlagNoRemind);
			return FALSE;
		}
	#ifdef CFG_REMIND_SOUND_DECODING_USE_LIBRARY
		RemindMp3DecoderInit();
	#endif
	}
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
	AudioIOSet.Depth = AudioCore.FrameSize[0] * 2 ;
#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 0;//不需要做位宽转换处理
#endif
	if(!AudioCoreSinkIsInit(AUDIO_I2SOUT_SINK_NUM))
	{
		mainAppCt.I2SFIFO_LEN = AudioIOSet.Depth * sizeof(PCM_DATA_TYPE) * 2;
		mainAppCt.I2SFIFO = (uint32_t*)osPortMalloc(mainAppCt.I2SFIFO_LEN);//I2S fifo
		if(mainAppCt.I2SFIFO != NULL)
		{
			memset(mainAppCt.I2SFIFO, 0, mainAppCt.I2SFIFO_LEN);
		}
		else
		{
			APP_DBG("malloc I2SFIFO error\n");
			return FALSE;
		}

#if((CFG_RES_I2S_MODE == I2S_MASTER_MODE) || !defined(CFG_FUNC_I2S_IN_SYNC_EN))
		// Master 或不开微调
	#if CFG_PARA_I2S_SAMPLERATE == CFG_PARA_SAMPLE_RATE
			AudioIOSet.Adapt = STD;//SRC_ONLY
	#else
			AudioIOSet.Adapt = SRC_ONLY;
	#endif
#else//slave
	#if CFG_PARA_I2S_SAMPLERATE == CFG_PARA_SAMPLE_RATE
			AudioIOSet.Adapt = STD;//SRA_ONLY;//CLK_ADJUST_ONLY;
	#else
			AudioIOSet.Adapt = SRC_ONLY;//SRC_SRA;//SRC_ADJUST;
	#endif
#endif
		AudioIOSet.Sync = TRUE;//I2S slave 时候如果master没有接，有可能会导致DAC也不出声音。
		AudioIOSet.Channels = 2;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.HighLevelCent = 60;
		AudioIOSet.LowLevelCent = 40;
		AudioIOSet.SampleRate = CFG_PARA_I2S_SAMPLERATE;//根据实际外设选择
		if(CFG_RES_I2S_MODULE == 0)
		{
			AudioIOSet.DataIOFunc = AudioI2S0_DataSet;
			AudioIOSet.LenGetFunc = AudioI2S0_DataSpaceLenGet;
		}
		else
		{
			AudioIOSet.DataIOFunc = AudioI2S1_DataSet;
			AudioIOSet.LenGetFunc = AudioI2S1_DataSpaceLenGet;
		}


		if(!AudioCoreSinkInit(&AudioIOSet, AUDIO_I2SOUT_SINK_NUM))
		{
			DBG("I2S out init error");
			return FALSE;
		}

		AudioI2sOutParamsSet();
		AudioCoreSinkEnable(AUDIO_I2SOUT_SINK_NUM);
		AudioCoreSinkAdjust(AUDIO_I2SOUT_SINK_NUM, TRUE);
	}
	else//sam add,20230221
	{
		I2S_SampleRateSet(CFG_RES_I2S_MODULE, CFG_PARA_SAMPLE_RATE);
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].BitWidth = AudioIOSet.IOBitWidth;
		AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].BitWidthConvFlag = AudioIOSet.IOBitWidthConvFlag;
		AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].Sync = TRUE;		
	#endif
	}
#endif

#ifdef CFG_RES_AUDIO_SPDIFOUT_EN

	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
	AudioIOSet.Depth = AudioCore.FrameSize[0] * 2 ;
#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 0;//不需要做位宽转换处理
#endif
	if(!AudioCoreSinkIsInit(AUDIO_SPDIF_SINK_NUM))
	{
		mainAppCt.SPDIF_OUT_FIFO_LEN = AudioIOSet.Depth * sizeof(PCM_DATA_TYPE) * 2;
		mainAppCt.SPDIF_OUT_FIFO = (uint32_t*)osPortMalloc(mainAppCt.SPDIF_OUT_FIFO_LEN);//I2S fifo
		if(mainAppCt.SPDIF_OUT_FIFO != NULL)
		{
			memset(mainAppCt.SPDIF_OUT_FIFO, 0, mainAppCt.SPDIF_OUT_FIFO_LEN);
		}
		else
		{
			APP_DBG("malloc SPDIF_OUT_FIFO error\n");
			return FALSE;
		}

		AudioIOSet.Adapt = SRC_ONLY;
		AudioIOSet.Sync = TRUE;
		AudioIOSet.Channels = 2;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.HighLevelCent = 60;
		AudioIOSet.LowLevelCent = 40;
		AudioIOSet.SampleRate = CFG_PARA_SAMPLE_RATE;//根据实际外设选择

		AudioIOSet.DataIOFunc = AudioSpdifTXDataSet;
		AudioIOSet.LenGetFunc = AudioSpdifTXDataSpaceLenGet;



		if(!AudioCoreSinkInit(&AudioIOSet, AUDIO_SPDIF_SINK_NUM))
		{
			DBG("spdif out init error");
			return FALSE;
		}
		AudioSpdifOutParamsSet();
		AudioCoreSinkEnable(AUDIO_SPDIF_SINK_NUM);
		AudioCoreSinkAdjust(AUDIO_SPDIF_SINK_NUM, TRUE);
	}
	else//sam add,20230221
	{
		SPDIF_SampleRateSet(SPDIF_OUT_NUM,CFG_PARA_SAMPLE_RATE);
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioCore.AudioSink[AUDIO_SPDIF_SINK_NUM].BitWidth = AudioIOSet.IOBitWidth;
		AudioCore.AudioSink[AUDIO_SPDIF_SINK_NUM].BitWidthConvFlag = AudioIOSet.IOBitWidthConvFlag;
		AudioCore.AudioSink[AUDIO_SPDIF_SINK_NUM].Sync = TRUE;
	#endif
	}
#endif

#ifdef CFG_FUNC_BREAKPOINT_EN
	//注意 是否需要模式过滤，部分模式无需记忆和恢复
	if(GetSystemMode() != ModeIdle)
	{
		BackupInfoUpdata(BACKUP_SYS_INFO);
	}
#endif

#ifdef CFG_FUNC_I2S_MIX_MODE
	I2S_MixInit();
#endif
#ifdef CFG_FUNC_LINEIN_MIX_MODE
	LineInMixPlayInit();
#endif
#ifdef CFG_FUNC_USB_AUDIO_MIX_MODE
	UsbDevicePlayMixInit();
#endif
	AudioDAC_Enable(DAC0);
	return TRUE;
}

//释放公共通路，
void ModeCommonDeinit(void)
{
	SoftFlagRegister(SoftFlagAudioCoreSourceIsDeInit);
#ifdef CFG_RES_AUDIO_DAC0_EN
//	AudioCoreSinkDisable(AUDIO_DAC0_SINK_NUM);
	AudioDAC_Disable(DAC0);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC0_TX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC0_TX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC0_TX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC0_TX);
//	AudioDAC_Reset(DAC0);
	if(mainAppCt.DACFIFO != NULL)
	{
		osPortFree(mainAppCt.DACFIFO);
		mainAppCt.DACFIFO = NULL;
	}
	AudioCoreSinkDeinit(AUDIO_DAC0_SINK_NUM);
#endif
#if CFG_RES_MIC_SELECT
	AudioCoreSourceDisable(MIC_SOURCE_NUM);
	vTaskDelay(5);
	AudioADC_Disable(ADC1_MODULE);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
	if(mainAppCt.ADCFIFO != NULL)
	{
		APP_DBG("ADC1FIFO\n");
		osPortFree(mainAppCt.ADCFIFO);
		mainAppCt.ADCFIFO = NULL;
	}
	AudioCoreSourceDeinit(MIC_SOURCE_NUM);
#endif
#if defined(CFG_RES_AUDIO_I2SOUT_EN)
	I2S_ModuleDisable(CFG_RES_I2S_MODULE);
	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_I2S_MODULE, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_I2S_MODULE, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_I2S_MODULE, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_I2S_MODULE);

	if(mainAppCt.I2SFIFO != NULL)
	{
		APP_DBG("I2SFIFO\n");
		osPortFree(mainAppCt.I2SFIFO);
		mainAppCt.I2SFIFO = NULL;
	}
	AudioCoreSinkDeinit(AUDIO_I2SOUT_SINK_NUM);
#endif

#if defined(CFG_RES_AUDIO_SPDIFOUT_EN)
	SPDIF_ModuleDisable(SPDIF_OUT_NUM);
	DMA_InterruptFlagClear(SPDIF_OUT_DMA_ID, DMA_DONE_INT);
	DMA_InterruptFlagClear(SPDIF_OUT_DMA_ID, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(SPDIF_OUT_DMA_ID, DMA_ERROR_INT);
	DMA_ChannelDisable(SPDIF_OUT_DMA_ID);

	if(mainAppCt.SPDIF_OUT_FIFO != NULL)
	{
		APP_DBG("SPDIF OUT FIFO\n");
		osPortFree(mainAppCt.SPDIF_OUT_FIFO);
		mainAppCt.SPDIF_OUT_FIFO = NULL;
	}
	AudioCoreSinkDeinit(AUDIO_SPDIF_SINK_NUM);
#endif

//#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
	AudioCoreSourceDeinit(REMIND_SOURCE_NUM);
	RemindSoundAudioPlayEnd();
#ifdef CFG_REMIND_SOUND_DECODING_USE_LIBRARY
	RemindMp3DecoderDeinit();
#endif
#endif

#ifdef CFG_FUNC_I2S_MIX_MODE
	I2S_MixDeinit();
#endif
#ifdef CFG_FUNC_LINEIN_MIX_MODE
	LineInMixPlayDeinit();
#endif
#ifdef CFG_FUNC_USB_AUDIO_MIX_MODE
	UsbDevicePlayMixDeinit();
#endif
	roboeffect_free(AudioCore.Roboeffect.context_memory);
	AudioCore.Roboeffect.context_memory = NULL;
}

bool AudioIoCommonForHfp(uint32_t sampleRate, uint16_t gain)
{
	AudioCoreIO AudioIOSet;
	uint16_t FifoLenStereo;
//	uint16_t FifoLenMono = SampleLen * 2 * 2;//单声到4倍大小于帧长，单位byte

	if(!RoboeffectInit())
	{
		DBG("!!!roboeffect init must be earlier than sink init!!!.\n");
	}
	FifoLenStereo = AudioCoreFrameSizeGet(DefaultNet) * 2 * 2 * 2;//立体声8倍大小于帧长，单位byte

#if CFG_RES_MIC_SELECT
	AudioCoreSourceDisable(MIC_SOURCE_NUM);

	if(!AudioCoreSourceIsInit(MIC_SOURCE_NUM))
	{
		//Mic1 analog  = Soure0.
//		AudioADC_AnaInit();
		//AudioADC_VcomConfig(1);//MicBias en
		// AudioADC_MicBias1Enable(1);
		mainAppCt.ADCFIFO = (uint32_t*)osPortMalloc(FifoLenStereo);//ADC fifo
		if(mainAppCt.ADCFIFO != NULL)
		{
			memset(mainAppCt.ADCFIFO, 0, FifoLenStereo);
		}
		else
		{
			APP_DBG("malloc ADCFIFO error\n");
			return FALSE;
		}

		AudioADC_DynamicElementMatch(ADC1_MODULE, TRUE, TRUE);
//		AudioADC_PGASel(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1);
//		AudioADC_PGASel(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2);
//		AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, LINEIN3_LEFT_OR_MIC1, 15, 4);//0db bypass
//		AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, 15, 4);

		//Mic1   digital
		AudioADC_DigitalInit(ADC1_MODULE, sampleRate,ADC_WIDTH_16BITS,(void*)mainAppCt.ADCFIFO,FifoLenStereo);

		//Soure0.
		memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
		AudioIOSet.Adapt = STD;
		AudioIOSet.Sync = TRUE;
		AudioIOSet.Channels = 1;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.DataIOFunc = AudioADC1_DataGet;
		AudioIOSet.LenGetFunc = AudioADC1_DataLenGet;
#ifdef CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_16BIT_WIDTH;			//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;	//需要数据位宽不扩展
#endif
		if(!AudioCoreSourceInit(&AudioIOSet, MIC_SOURCE_NUM))
		{
			DBG("mic Source error");
			return FALSE;
		}
		//MADC_MIC_PowerUP(SingleEnded);
#ifdef CFG_ADCDAC_SEL_LOWPOWERMODE
		AudioADC_AnaInit(ADC1_MODULE,CHANNEL_LEFT,MIC_LEFT,Single,ADCLowEnergy,gain);
#else
		AudioADC_AnaInit(ADC1_MODULE,CHANNEL_LEFT,MIC_LEFT,Single,ADCCommonEnergy,gain);
#endif // CFG_ADCDAC_SEL_LOWPOWERMODE
		AudioCoreSourceEnable(MIC_SOURCE_NUM);
	}
	else //采样率等 重配
	{
		AudioCoreSourceDisable(MIC_SOURCE_NUM);
//		AudioADC_AnaInit();
		//AudioADC_VcomConfig(1);//MicBias en
//		AudioADC_MicBias1Enable(1);

		AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, MIC_LEFT, gain);
//		AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, LINEIN3_RIGHT_OR_MIC2, gain);

		//Mic1	 digital
		memset(mainAppCt.ADCFIFO, 0, FifoLenStereo);
		AudioADC_DigitalInit(ADC1_MODULE, sampleRate,ADC_WIDTH_16BITS, (void*)mainAppCt.ADCFIFO, FifoLenStereo);
	}
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(!AudioCoreSourceIsInit(REMIND_SOURCE_NUM))
	{
		memset(&AudioIOSet, 0, sizeof(AudioCoreIO));

		AudioIOSet.Adapt = SRC_ONLY;
		AudioIOSet.SampleRate = CFG_PARA_SAMPLE_RATE;//初始值
		AudioIOSet.Sync = FALSE;
		AudioIOSet.Channels = 1;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.DataIOFunc = RemindDataGet;
		AudioIOSet.LenGetFunc = RemindDataLenGet;

#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_16BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;//需要数据进行位宽扩展
#endif
		if(!AudioCoreSourceInit(&AudioIOSet, REMIND_SOURCE_NUM))
		{
			DBG("remind source error!\n");
			SoftFlagRegister(SoftFlagNoRemind);
			return FALSE;
		}
#ifdef CFG_REMIND_SOUND_DECODING_USE_LIBRARY		
		RemindMp3DecoderInit();
#endif
	}
#endif


#ifdef CFG_RES_AUDIO_DAC0_EN
	AudioCoreSinkDisable(AUDIO_DAC0_SINK_NUM);
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
	AudioIOSet.Depth = AudioCore.FrameSize[DefaultNet] * 2 ;
#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = PCM_DATA_16BIT_WIDTH;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 1;//DAC 24bit ,sink最后一级输出时需要转变为24bi
#endif
	if(!AudioCoreSinkIsInit(AUDIO_DAC0_SINK_NUM))
	{
		mainAppCt.DACFIFO_LEN = AudioIOSet.Depth * sizeof(PCM_DATA_TYPE) * 2;
		mainAppCt.DACFIFO = (uint32_t*)osPortMalloc(mainAppCt.DACFIFO_LEN);//DAC fifo
		if(mainAppCt.DACFIFO != NULL)
		{
			memset(mainAppCt.DACFIFO, 0, mainAppCt.DACFIFO_LEN);
		}
		else
		{
			APP_DBG("malloc DACFIFO error\n");
			return FALSE;
		}
		//sink0

		AudioIOSet.Adapt = STD;
		AudioIOSet.Sync = TRUE;
		AudioIOSet.Channels = 2;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.DataIOFunc = AudioDAC0_DataSet;
		AudioIOSet.LenGetFunc = AudioDAC0_DataSpaceLenGet;

		if(!AudioCoreSinkInit(&AudioIOSet, AUDIO_DAC0_SINK_NUM))
		{
			DBG("Dac init error");
			return FALSE;
		}


		DACParamCt ct;
		uint16_t BitWidth;
	#ifdef CHIP_DAC_USE_DIFF
		ct.DACModel = DAC_Diff;
	#else
		ct.DACModel = DAC_Single;
	#endif

	#ifdef CHIP_DAC_USE_PVDD16
		ct.PVDDModel = PVDD16;
	#else
		ct.PVDDModel = PVDD33;
	#endif
		ct.DACLoadStatus = DAC_NOLoad;
		ct.DACEnergyModel = DACCommonEnergy;
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		BitWidth = 24;
	#else
		BitWidth = 16;
	#endif
		AudioDAC_Init(&ct,sampleRate,BitWidth, (void*)mainAppCt.DACFIFO, mainAppCt.DACFIFO_LEN, NULL, 0);
	}
	else
	{
		AudioDAC0_SampleRateChange(sampleRate);
		printf("mode task io set\n");
#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioCore.AudioSink[AUDIO_DAC0_SINK_NUM].BitWidth = AudioIOSet.IOBitWidth;
		AudioCore.AudioSink[AUDIO_DAC0_SINK_NUM].BitWidthConvFlag = AudioIOSet.IOBitWidthConvFlag;
#endif
	}
	AudioCoreSinkEnable(AUDIO_DAC0_SINK_NUM);
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN

	AudioIOSet.Depth = AudioCore.FrameSize[DefaultNet] * 2 ;
#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = PCM_DATA_16BIT_WIDTH;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 1;//不需要做位宽转换处理
#endif
	if(!AudioCoreSinkIsInit(AUDIO_I2SOUT_SINK_NUM))
	{
		mainAppCt.I2SFIFO_LEN = AudioIOSet.Depth * sizeof(PCM_DATA_TYPE) * 2;
		mainAppCt.I2SFIFO = (uint32_t*)osPortMalloc(mainAppCt.I2SFIFO_LEN);//I2S fifo

		if(mainAppCt.I2SFIFO != NULL)
		{
			memset(mainAppCt.I2SFIFO, 0, mainAppCt.I2SFIFO_LEN);
		}
		else
		{
			APP_DBG("malloc I2SFIFO error\n");
			return FALSE;
		}

#if((CFG_RES_I2S_MODE == I2S_MASTER_MODE) || !defined(CFG_FUNC_I2S_IN_SYNC_EN))
		// Master 或不开微调
	#if CFG_PARA_I2S_SAMPLERATE == CFG_BTHF_PARA_SAMPLE_RATE
			AudioIOSet.Adapt = STD;//SRC_ONLY
	#else
			AudioIOSet.Adapt = SRC_ONLY;
	#endif
#else//slave
	#if CFG_PARA_I2S_SAMPLERATE == CFG_BTHF_PARA_SAMPLE_RATE
			AudioIOSet.Adapt = SRA_ONLY;//CLK_ADJUST_ONLY;
	#else
			AudioIOSet.Adapt = SRC_SRA;//SRC_ADJUST;
	#endif
#endif
		AudioIOSet.Sync = TRUE;//I2S slave 时候如果master没有接，有可能会导致DAC也不出声音。
		AudioIOSet.Channels = 2;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.HighLevelCent = 60;
		AudioIOSet.LowLevelCent = 40;
		AudioIOSet.SampleRate = CFG_PARA_I2S_SAMPLERATE;//根据实际外设选择
//		AudioIOSet.CoreSampleRate = CFG_BTHF_PARA_SAMPLE_RATE;
		if(CFG_RES_I2S_MODULE == 0)
		{
			AudioIOSet.DataIOFunc = AudioI2S0_DataSet;
			AudioIOSet.LenGetFunc = AudioI2S0_DataSpaceLenGet;
		}
		else
		{
			AudioIOSet.DataIOFunc = AudioI2S1_DataSet;
			AudioIOSet.LenGetFunc = AudioI2S1_DataSpaceLenGet;
		}

		if(!AudioCoreSinkInit(&AudioIOSet, AUDIO_I2SOUT_SINK_NUM))
		{
			DBG("I2S out init error");
			return FALSE;
		}

		AudioI2sOutParamsSet();
		AudioCoreSinkEnable(AUDIO_I2SOUT_SINK_NUM);
		AudioCoreSinkAdjust(AUDIO_I2SOUT_SINK_NUM, TRUE);
	}
	else
	{
		I2S_SampleRateSet(CFG_RES_I2S_MODULE, sampleRate);
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].BitWidth = AudioIOSet.IOBitWidth;
		AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].BitWidthConvFlag = AudioIOSet.IOBitWidthConvFlag;
		AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].Sync = FALSE;
	#endif
	}
#endif

	AudioDAC_Enable(DAC0);
	return TRUE;
}



//void tws_device_open_isr(void)
//{
//
//}
//sel: 0 = init hw, 1 = effect, 2 = hw + effect
void AudioEffectModeSel(EFFECT_MODE effectMode, uint8_t sel)
{
	uint8_t i;

	if(sel == 1 || sel == 2)
	{
		PauseAuidoCore();
		roboeffect_free(AudioCore.Roboeffect.context_memory);
		AudioCore.Roboeffect.context_memory = NULL;

		if(!RoboeffectInit())
		{
			if(AudioCore.Roboeffect.effect_addr)
			{
				AudioCore.Roboeffect.effect_enable = 0;
				DBG("roboeffect init again because cannot enable effect:%d\n", RoboeffectInit());
			}
		}

		for(i = 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
		{
			if(AudioCoreSourceToRoboeffect(i) != AUDIOCORE_SOURCE_SINK_ERROR)
			{
				AudioCore.AudioSource[i].PcmInBuf = roboeffect_get_source_buffer(
								AudioCore.Roboeffect.context_memory, AudioCoreSourceToRoboeffect(i));
			}
		}
		for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
		{
			if(AudioCoreSinkToRoboeffect(i) != AUDIOCORE_SOURCE_SINK_ERROR)
			{
				AudioCore.AudioSink[i].PcmOutBuf = roboeffect_get_sink_buffer(
								AudioCore.Roboeffect.context_memory, AudioCoreSinkToRoboeffect(i));
			}
		}

#ifdef CFG_APP_LINEIN_MODE_EN
		if(GetSystemMode() == ModeLineAudioPlay)
		{
			//linein模式需要重新初始化ADC0，否则delay会变大
			extern void LineinADCDigitalInit(void);
			LineinADCDigitalInit();
		}
#endif

		SoftFlagDeregister(SoftFlagAudioCoreSourceIsDeInit);
		AudioCoreServiceResume();
	}
	if(sel == 0 || sel == 2)
	{
		DefaultParamgsInit();
		AudioCodecGainUpdata();//update hardware config
	}
}

//各模式下的通用消息处理, 共有的提示音在此处理，因此要求调用次API前，确保APP running状态。避免解码器没准备好。
void CommonMsgProccess(uint16_t Msg)
{
#if defined(CFG_FUNC_DISPLAY_EN)
	MessageContext	msgSend;
#endif
	if(SoftFlagGet(SoftFlagDiscDelayMask) && Msg == MSG_NONE)
	{
		Msg = MSG_BT_STATE_DISCONNECT;
	}

	switch(Msg)
	{
		case MSG_MENU://菜单键
			APP_DBG("menu key\n");
			AudioPlayerMenu();
			break;
#ifdef VD51_REDMINE_13199
		case MSG_RGB_MODE:
			if(IsRgbLedModeMenuExit())
				RgbLedModeMenuEnter();
			else
				RgbLedModeMenuExit();
			break;
		case MSG_MUTE:
			if(!IsRgbLedModeMenuExit())
			{
				RgbLedModeSet();
			#ifdef CFG_FUNC_BREAKPOINT_EN
				BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
				break;
			}
#else
		case MSG_MUTE:
#endif			
			APP_DBG("MSG_MUTE\n");
			#ifdef  CFG_APP_HDMIIN_MODE_EN
			extern HDMIInfo         *gHdmiCt;
			if(GetSystemMode() == ModeHdmiAudioPlay)
			{
				if(IsHDMISourceMute() == TRUE)
					HDMISourceUnmute();
				else
					HDMISourceMute();
				gHdmiCt->hdmiActiveReportMuteStatus = IsHDMISourceMute();
				gHdmiCt->hdmiActiveReportMuteflag = 2;
			}
			else
			#endif
			{
				HardWareMuteOrUnMute();
			}
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_MUTE;
            MessageSend(GetSysModeMsgHandle(), &msgSend);
			#endif
			break;

		case MSG_MAIN_VOL_UP:
			SystemVolUp();
			APP_DBG("MSG_MAIN_VOL_UP\n");
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_VOL;
            MessageSend(GetSysModeMsgHandle(), &msgSend);
			#endif
			break;

		case MSG_MAIN_VOL_DW:
			SystemVolDown();
			APP_DBG("MSG_MAIN_VOL_DW\n");
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_VOL;
            MessageSend(GetSysModeMsgHandle(), &msgSend);
			#endif
			break;

		case MSG_MUSIC_VOLUP:
			AudioMusicVolUp();
			APP_DBG("MSG_MUSIC_VOLUP\n");
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_MUSIC_VOL;
            MessageSend(GetSysModeMsgHandle(), &msgSend);
			#endif
			break;

		case MSG_MUSIC_VOLDOWN:
			AudioMusicVolDown();
			APP_DBG("MSG_MUSIC_VOLDOWN\n");
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_MUSIC_VOL;
            MessageSend(GetSysModeMsgHandle(), &msgSend);
			#endif
			break;

		#if CFG_RES_MIC_SELECT
		case MSG_MIC_VOLUP:
			AudioMicVolUp();
			APP_DBG("MSG_MIC_VOLUP\n");
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_MIC_VOL;
            MessageSend(GetSysModeMsgHandle(), &msgSend);
			#endif
			break;

		case MSG_MIC_VOLDOWN:
			AudioMicVolDown();
			APP_DBG("MSG_MIC_VOLDOWN\n");
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_MIC_VOL;
            MessageSend(GetSysModeMsgHandle(), &msgSend);
			#endif
			break;
		#endif

#ifdef CFG_APP_BT_MODE_EN
		case MSG_BT_PLAY_SYNC_VOLUME_CHANGED:
			APP_DBG("MSG_BT_PLAY_SYNC_VOLUME_CHANGED\n");
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
			AudioMusicVolSet(GetBtSyncVolume());
#endif
			break;
#endif

#ifdef CFG_FUNC_AUDIO_EFFECT_EN
		case MSG_MIC_EFFECT_UP:
			if(mainAppCt.ReverbStep < MAX_MIC_REVB_STEP)
			{
				mainAppCt.ReverbStep++;
			}
			else
			{
				mainAppCt.ReverbStep = 0;
			}
			Roboeffect_ReverbStep_Ajust(mainAppCt.ReverbStep);
			APP_DBG("MSG_MIC_EFFECT_UP\n");
			APP_DBG("ReverbStep = %d\n", mainAppCt.ReverbStep);
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MIC_EFFECT_DW:
			if(mainAppCt.ReverbStep > 0)
			{
				mainAppCt.ReverbStep--;
			}
			else
			{
				mainAppCt.ReverbStep = 0;
			}
			Roboeffect_ReverbStep_Ajust(mainAppCt.ReverbStep);
			APP_DBG("MSG_MIC_EFFECT_DW\n");
			APP_DBG("ReverbStep = %d\n", mainAppCt.ReverbStep);
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif

			break;


		#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
		case MSG_EQ:
			APP_DBG("MSG_EQ\n");
			if(mainAppCt.EqMode < EQ_MODE_VOCAL_BOOST)
			{
				mainAppCt.EqMode++;
			}
			else
			{
				mainAppCt.EqMode = EQ_MODE_FLAT;
			}
			APP_DBG("EqMode = %d\n", mainAppCt.EqMode);
			Roboeffect_EQMode_Set(mainAppCt.EqMode);

			#ifdef CFG_FUNC_DISPLAY_EN
			msgSend.msgId = MSG_DISPLAY_SERVICE_EQ;
			MessageSend(GetSysModeMsgHandle(), &msgSend);
			#endif
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;
		#endif

		#ifdef CFG_FUNC_MIC_TREB_BASS_EN
		case MSG_MIC_TREB_UP:
			APP_DBG("MSG_MIC_TREB_UP\n");
			if(mainAppCt.MicTrebStep < MAX_BASS_TREB_GAIN)
			{
				mainAppCt.MicTrebStep++;
			}
			APP_DBG("MicTrebStep = %d\n", mainAppCt.MicTrebStep);
			Roboeffect_EQ_Ajust(MIC_EQ, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
			#ifdef CFG_FUNC_DISPLAY_EN
			msgSend.msgId = MSG_DISPLAY_SERVICE_TRE;
			MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MIC_TREB_DW:
			APP_DBG("MSG_MIC_TREB_DW\n");
			if(mainAppCt.MicTrebStep)
			{
				mainAppCt.MicTrebStep--;
			}
			APP_DBG("MicTrebStep = %d\n", mainAppCt.MicTrebStep);
			Roboeffect_EQ_Ajust(MIC_EQ, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
			#ifdef CFG_FUNC_DISPLAY_EN
			msgSend.msgId = MSG_DISPLAY_SERVICE_TRE;
			MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MIC_BASS_UP:
			APP_DBG("MSG_MIC_BASS_UP\n");
			if(mainAppCt.MicBassStep < MAX_BASS_TREB_GAIN)
			{
				mainAppCt.MicBassStep++;
			}
			APP_DBG("MicBassStep = %d\n", mainAppCt.MicBassStep);
			Roboeffect_EQ_Ajust(MIC_EQ, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
			#ifdef CFG_FUNC_DISPLAY_EN
			msgSend.msgId = MSG_DISPLAY_SERVICE_BAS;
			MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MIC_BASS_DW:
			APP_DBG("MSG_MIC_BASS_DW\n");
			if(mainAppCt.MicBassStep)
			{
				mainAppCt.MicBassStep--;
			}
			APP_DBG("MicBassStep = %d\n", mainAppCt.MicBassStep);
			Roboeffect_EQ_Ajust(MIC_EQ, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
			#ifdef CFG_FUNC_DISPLAY_EN
			msgSend.msgId = MSG_DISPLAY_SERVICE_BAS;
			MessageSend(GetDisplayMessageHandle(), &msgSend);
			#endif
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;
		#endif

		#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN
		case MSG_MUSIC_TREB_UP:
			APP_DBG("MSG_MUSIC_TREB_UP\n");
			if(mainAppCt.MusicTrebStep < MAX_BASS_TREB_GAIN)
			{
				mainAppCt.MusicTrebStep++;
			}
			APP_DBG("MusicTrebStep = %d\n", mainAppCt.MusicTrebStep);
			Roboeffect_EQ_Ajust(MUSIC_EQ, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);

			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MUSIC_TREB_DW:
			APP_DBG("MSG_MUSIC_TREB_DW\n");
			if(mainAppCt.MusicTrebStep)
			{
				mainAppCt.MusicTrebStep--;
			}
			APP_DBG("MusicTrebStep = %d\n", mainAppCt.MusicTrebStep);
			Roboeffect_EQ_Ajust(MUSIC_EQ, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
			
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MUSIC_BASS_UP:
			APP_DBG("MSG_MUSIC_BASS_UP\n");
			if(mainAppCt.MusicBassStep < MAX_BASS_TREB_GAIN)
			{
				mainAppCt.MusicBassStep++;
			}
			APP_DBG("MusicBassStep = %d\n", mainAppCt.MusicBassStep);
			Roboeffect_EQ_Ajust(MUSIC_EQ, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);

			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;

		case MSG_MUSIC_BASS_DW:
			APP_DBG("MSG_MUSIC_BASS_DW\n");
			if(mainAppCt.MusicBassStep)
			{
				mainAppCt.MusicBassStep--;
			}
			APP_DBG("MusicBassStep = %d\n", mainAppCt.MusicBassStep);
			Roboeffect_EQ_Ajust(MUSIC_EQ, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
			#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
			#endif
			break;
		#endif

		case MSG_3D:
			APP_DBG("MSG_3D\n");
			#if CFG_AUDIO_EFFECT_MUSIC_3D_EN
			gCtrlVars.music_threed_unit.three_d_en = !gCtrlVars.music_threed_unit.three_d_en;
			#endif

			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_3D;
            MessageSend(GetSysModeMsgHandle(), &msgSend);
			#endif
			break;

		case MSG_VOCAL_CUT:
			APP_DBG("MSG_VOCAL_CUT\n");
			Roboeffect_effect_enable(VOCAL_CUT, !Roboeffect_effect_status_Get(VOCAL_CUT));
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_VOCAL_CUT;
            MessageSend(GetSysModeMsgHandle(), &msgSend);
			#endif
			break;

		case MSG_VB:
			APP_DBG("MSG_VB\n");
			#if CFG_AUDIO_EFFECT_MUSIC_VIRTUAL_BASS_EN
			gCtrlVars.music_vb_unit.vb_en = !gCtrlVars.music_vb_unit.vb_en;
			#endif
			#ifdef CFG_FUNC_DISPLAY_EN
            msgSend.msgId = MSG_DISPLAY_SERVICE_VB;
            MessageSend(GetSysModeMsgHandle(), &msgSend);
			#endif
			break;
		case MSG_EFFECTMODE:
#ifndef CFG_FUNC_EFFECT_BYPASS_EN
			if(GetSystemMode() == ModeBtHfPlay)//蓝牙通话模式下，不支持音效模式切换
			{
				break;
			}

			if(IsAudioPlayerMute() == FALSE)
			{
				HardWareMuteOrUnMute();
			}
#ifdef CFG_FUNC_MIC_KARAOKE_EN
			if((mainAppCt.EffectMode >= EFFECT_MODE_WaWaYin)|| (mainAppCt.EffectMode < EFFECT_MODE_HunXiang))
			{
				mainAppCt.EffectMode = EFFECT_MODE_HunXiang;
			}
#else
			if((mainAppCt.EffectMode >= EFFECT_MODE_MUSIC) || (mainAppCt.EffectMode < EFFECT_MODE_MIC))
			{
				mainAppCt.EffectMode = EFFECT_MODE_MIC;
			}
#endif
			else
			{
				mainAppCt.EffectMode++;
			}
#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
#endif
			APP_DBG("EffectMode = %d\n", mainAppCt.EffectMode);

			ROBOEFFECT_EFFECT_PARA *mpara = get_user_effect_parameters(mainAppCt.EffectMode);

			if (mpara->user_effect_list->frame_size == AudioCoreFrameSizeGet(DefaultNet))
			{
				AudioEffectModeSel(mainAppCt.EffectMode, 2);//sel: 0=init hw, 1=effect, 2=hw + effect
			}
			else
			{
				PauseAuidoCore();
				ModeCommonDeinit();
				if(!ModeCommonInit())
				{
					APP_DBG("MSG_EFFECTMODE ModeInit Error!!!\n");
					break;
				}
				for(int8_t i = 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
				{
					if(AudioCoreSourceToRoboeffect(i) != AUDIOCORE_SOURCE_SINK_ERROR)
					{
						AudioCore.AudioSource[i].PcmInBuf = roboeffect_get_source_buffer(
										AudioCore.Roboeffect.context_memory, AudioCoreSourceToRoboeffect(i));
					}
				}
				for(int8_t i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
				{
					if(AudioCoreSinkToRoboeffect(i) != AUDIOCORE_SOURCE_SINK_ERROR)
					{
						AudioCore.AudioSink[i].PcmOutBuf = roboeffect_get_sink_buffer(
										AudioCore.Roboeffect.context_memory, AudioCoreSinkToRoboeffect(i));
					}
				}
				SoftFlagDeregister(SoftFlagAudioCoreSourceIsDeInit);
				AudioCoreServiceResume();
			}

			AudioEffectParamSync();		//switch effect mode also need to sync some params

			if(IsAudioPlayerMute() == TRUE)
			{
				HardWareMuteOrUnMute();
			}
			gCtrlVars.AutoRefresh = 1;
#endif
#endif
			break;
		case MSG_EFFECTREINIT:
			APP_DBG("MSG_EFFECTREINIT\n");
			if(IsAudioPlayerMute() == FALSE)
			{
				HardWareMuteOrUnMute();
			}
			int8_t opera = AudioCore.Roboeffect.effect_enable ? 1 : -1;
			uint8_t addr = AudioCore.Roboeffect.effect_addr;
			ROBOEFFECT_EFFECT_PARA *para = get_user_effect_parameters(mainAppCt.EffectMode);

//			APP_DBG("Roboeffect FrameSize %d, %d\n", opera, roboeffect_get_suit_frame_size(
//					AudioCore.Roboeffect.context_memory, CFG_PARA_SAMPLES_PER_FRAME, AudioCore.Roboeffect.effect_addr, opera));
			if(roboeffect_get_suit_frame_size(
					AudioCore.Roboeffect.context_memory, para->user_effect_list->frame_size, AudioCore.Roboeffect.effect_addr, opera)
				&& AudioCoreFrameSizeGet(DefaultNet) != roboeffect_get_suit_frame_size(
					AudioCore.Roboeffect.context_memory, para->user_effect_list->frame_size, AudioCore.Roboeffect.effect_addr, opera))
			{
				AudioCore.Roboeffect.user_effect_list->frame_size = roboeffect_get_suit_frame_size(
						AudioCore.Roboeffect.context_memory, para->user_effect_list->frame_size, AudioCore.Roboeffect.effect_addr, opera);
				APP_DBG("Need Change FrameSize to %ld\n", AudioCore.Roboeffect.user_effect_list->frame_size);

				SysMode[GetModeIndexInModeLoop(&mainAppCt.SysCurrentMode)].SysModeDeInit();
				AudioCoreFrameSizeSet(DefaultNet, AudioCore.Roboeffect.user_effect_list->frame_size);
				SysMode[GetModeIndexInModeLoop(&mainAppCt.SysCurrentMode)].SysModeInit();
				SysMode[GetModeIndexInModeLoop(&mainAppCt.SysCurrentMode)].SysModeRun(0);

				SoftFlagDeregister(SoftFlagAudioCoreSourceIsDeInit);
				AudioCoreServiceResume();
			}
			else
			{
				AudioEffectModeSel(mainAppCt.EffectMode, 1);//sel: 0=init hw, 1=effect, 2=hw + effect
			}

			if(IsAudioPlayerMute() == TRUE)
			{
				HardWareMuteOrUnMute();
			}
            AudioCore.Roboeffect.reinit_done = 1;
			gCtrlVars.AutoRefresh = addr;
			break;

		#ifdef CFG_FUNC_RTC_EN
		case MSG_RTC_SET_TIME:
			RTC_ServiceModeSelect(0,0);
			break;

		case MSG_RTC_SET_ALARM:
			RTC_ServiceModeSelect(0,1);
			break;

		case MSG_RTC_DISP_TIME:
			RTC_ServiceModeSelect(0,2);
			break;

		case MSG_RTC_UP:
			RTC_RtcUp();
			break;

		case MSG_RTC_DOWN:
			RTC_RtcDown();
			break;

		#ifdef CFG_FUNC_SNOOZE_EN
		case MSG_RTC_SNOOZE:
			if(mainAppCt.AlarmRemindStart)
			{
				mainAppCt.AlarmRemindCnt = 0;
				mainAppCt.AlarmRemindStart = 0;
				mainAppCt.SnoozeOn = 1;
				mainAppCt.SnoozeCnt = 0;
			}
			break;
		#endif

		#endif //end of  CFG_FUNC_RTC_EN

		case MSG_REMIND1:
			#ifdef CFG_FUNC_REMIND_SOUND_EN
			RemindSoundServiceItemRequest(SOUND_REMIND_SHANGYIS,FALSE);
			#endif
			break;

		case MSG_DEVICE_SERVICE_BATTERY_LOW:
			//RemindSound request
			APP_DBG("MSG_DEVICE_SERVICE_BATTERY_LOW\n");
			#ifdef CFG_FUNC_REMIND_SOUND_EN
			RemindSoundServiceItemRequest(SOUND_REMIND_DLGUODI, FALSE);
			#endif
			break;

#ifdef CFG_APP_BT_MODE_EN
		#ifdef POWER_ON_BT_ACCESS_MODE_SET
		case MSG_BT_OPEN_ACCESS:
			if (GetBtManager()->btAccessModeEnable == 0)
			{
				GetBtManager()->btAccessModeEnable = 1;
				BtStackServiceMsgSend(MSG_BTSTACK_ACCESS_MODE_SET);
				DBG("open bt access\n");
			}
			break;
		#endif
			
		//蓝牙连接断开消息,用于提示音
		case MSG_BT_STATE_CONNECTED:
			APP_DBG("[BT_STATE]:BT Connected...\n");
			//异常回连过程中，不提示连接断开提示音

			BtStackServiceMsgSend(MSG_BTSTACK_ACCESS_MODE_SET);

			if(btManager.btDutModeEnable)
				break;

			//if(!(btCheckEventList&BT_EVENT_L2CAP_LINK_DISCONNECT))
			{
				#ifdef CFG_FUNC_REMIND_SOUND_EN
				if(RemindSoundServiceItemRequest(SOUND_REMIND_CONNECT, REMIND_PRIO_SYS|REMIND_ATTR_NEED_HOLD_PLAY))
				{
					if(!SoftFlagGet(SoftFlagWaitBtRemindEnd)&&SoftFlagGet(SoftFlagDelayEnterBtHf))
					{
						SoftFlagRegister(SoftFlagWaitBtRemindEnd);
					}
				}
				else
				#endif
				{
					if(SoftFlagGet(SoftFlagDelayEnterBtHf))
					{
						MessageContext		msgSend;
						SoftFlagDeregister(SoftFlagDelayEnterBtHf);

						msgSend.msgId = MSG_DEVICE_SERVICE_ENTER_BTHF_MODE;
						MessageSend(GetMainMessageHandle(), &msgSend);
					}
				}
			}
			break;

		case MSG_BT_STATE_DISCONNECT:
			APP_DBG("[BT_STATE]:BT Disconnected...\n");
			SoftFlagDeregister(SoftFlagDiscDelayMask);
			
			if(btManager.btDutModeEnable)
				break;

			BtStackServiceMsgSend(MSG_BTSTACK_ACCESS_MODE_SET);

			//异常回连过程中，不提示连接断开提示音
			//if(!(btCheckEventList&BT_EVENT_L2CAP_LINK_DISCONNECT))
			{
				#ifdef CFG_FUNC_REMIND_SOUND_EN
				if(GetSystemMode() != ModeIdle)
				{
					RemindSoundServiceItemRequest(SOUND_REMIND_DISCONNE, REMIND_PRIO_SYS|REMIND_ATTR_NEED_HOLD_PLAY);
				}
				#endif
			}
			break;
#endif

		default:
			#ifdef CFG_FUNC_DISPLAY_EN
			//display
			Display(Msg);
			#endif
			#ifdef CFG_ADC_LEVEL_KEY_EN
			AdcLevelMsgProcess(Msg);
			#endif
			break;
	}
}
