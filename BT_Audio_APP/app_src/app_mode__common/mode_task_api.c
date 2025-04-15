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
#include "clk.h"
//app
#include "bt_stack_service.h"
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
#include "bt_app_avrcp_deal.h"
#endif

#ifdef CFG_RES_AUDIO_SPDIFOUT_EN
#include "spdif_out.h"
#endif

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

const DACParamCt DACDefaultParamCt =
{
#ifdef CHIP_DAC_USE_DIFF
	.DACModel = DAC_Diff,
#else
	.DACModel = DAC_Single,
#endif

#ifdef CHIP_DAC_USE_PVDD16
	.PVDDModel = PVDD16,
#else
	.PVDDModel = PVDD33,
#endif
	.DACLoadStatus = DAC_NOLoad,
	.DACEnergyModel = DACCommonEnergy,

#ifdef CFG_VCOM_DRIVE_EN
	.DACVcomModel = Direct,
#else
	.DACVcomModel = Disable,
#endif
};

uint32_t get_audioeffect_default_frame_size(roboeffect_effect_list_info *user_effect_list)
{
	uint32_t offset;
	roboeffect_effect_list_info *p;

	extern char __data_lmastart;
	extern char __data_start;
	extern char _edata;

	if((uint32_t)user_effect_list <  (uint32_t)&__data_start ||
	   (uint32_t)user_effect_list >  (uint32_t)&_edata)
	{
		//������Ҫ��ʼ��ֵ��ȫ�ֱ�����
		return 0;
	}
	offset = (uint32_t)user_effect_list - (uint32_t)(&__data_start);
	p = (roboeffect_effect_list_info *)((uint32_t)&__data_lmastart + offset);

	return p->frame_size;
}

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

#ifdef CFG_AUDIO_OUT_AUTO_SAMPLE_RATE_44100_48000
void AudioOutSampleRateSet(uint32_t SampleRate)
{
	extern uint32_t IsBtHfMode(void);

	if(IsBtHfMode() || mainAppCt.EffectMode == EFFECT_MODE_HFP_AEC)
		return;

	if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100)
			|| (SampleRate == 88200) || (SampleRate == 176400))
	{
		SampleRate = 44100;
	}
	else
	{
		SampleRate = 48000;
	}

	if(AudioCoreMixSampleRateGet(DefaultNet) == SampleRate)
		return;
	APP_DBG("SampleRate: %d --> %d\n", (int)AudioCoreMixSampleRateGet(DefaultNet), (int)SampleRate);
	AudioCoreMixSampleRateSet(DefaultNet, SampleRate);

#ifdef CFG_RES_AUDIO_DAC0_EN
	AudioDAC0_SampleRateChange(SampleRate);
	gCtrlVars.HwCt.DAC0Ct.dac_mclk_source = Clock_AudioMclkGet(AUDIO_DAC0);
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	AudioI2S_SampleRateChange(CFG_RES_I2S_MODULE,SampleRate);
	if(CFG_RES_I2S_MODULE == I2S0_MODULE)
		gCtrlVars.HwCt.I2S0Ct.i2s_mclk_source = Clock_AudioMclkGet(AUDIO_I2S0);
	else
		gCtrlVars.HwCt.I2S1Ct.i2s_mclk_source = Clock_AudioMclkGet(AUDIO_I2S1);
#endif

#if CFG_RES_MIC_SELECT
	AudioADC_SampleRateChange(ADC1_MODULE,SampleRate);
	gCtrlVars.HwCt.ADC1DigitalCt.adc_mclk_source = Clock_AudioMclkGet(AUDIO_ADC1);
#endif

    MessageContext msgSend;
    msgSend.msgId = MSG_EFFECTREINIT;
    MessageSend(GetMainMessageHandle(), &msgSend);
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

bool AudioEffectInit()
{
	if(AudioCore.Audioeffect.effect_addr)
	{
		AudioEffect_update_local_effect_status(AudioCore.Audioeffect.effect_addr, AudioCore.Audioeffect.effect_enable);
		DBG("Audioeffect ReInit:0x%x\n", AudioCore.Audioeffect.effect_addr);
	}
	else
	{
		if(AudioCore.Audioeffect.user_effect_parameters)
		{
			//���ͷ���Դ
			osPortFree(AudioCore.Audioeffect.user_effect_parameters);
		}
		
		AUDIOEFFECT_EFFECT_PARA *para = get_user_effect_parameters(mainAppCt.EffectMode);

		AudioCore.Audioeffect.effect_count = para->user_effect_list->count + 0x80;
		AudioCore.Audioeffect.cur_effect_para = para;
		AudioCore.Audioeffect.user_effects_script_len = para->get_user_effects_script_len();
		AudioCore.Audioeffect.user_effect_parameters = osPortMalloc(get_user_effect_parameters_len(para->user_effect_parameters) * sizeof(uint8_t));
		memcpy(AudioCore.Audioeffect.user_effect_parameters, para->user_effect_parameters,get_user_effect_parameters_len(para->user_effect_parameters) * sizeof(uint8_t));
		//�����е�ֵ���ܱ��л�֡���ı�,��ȡ��ʼ��ԭʼ֡��(data��)
		AudioCore.Audioeffect.audioeffect_frame_size = get_audioeffect_default_frame_size(para->user_effect_list);
		if(AudioCore.Audioeffect.audioeffect_frame_size == 0)
		{
			AudioCore.Audioeffect.audioeffect_frame_size = para->user_effect_list->frame_size;
		}
		else
		{
			para->user_effect_list->frame_size = AudioCore.Audioeffect.audioeffect_frame_size;
			DBG("audioeffect_default_frame_size: %lu\n",AudioCore.Audioeffect.audioeffect_frame_size);
		}
		para->user_effect_list->sample_rate = AudioCoreMixSampleRateGet(DefaultNet);

#ifdef CFG_EFFECT_PARAM_IN_FLASH_EN
		AudioCore.Audioeffect.EffectFlashUseFlag = FALSE;
		if (AudioEffect_GetFlashEffectParam(mainAppCt.EffectMode, AudioCore.Audioeffect.user_effect_parameters))
		{
			AudioCore.Audioeffect.EffectFlashUseFlag = TRUE;
			DBG("read effect parameters from flash ok\n");
			DBG("EFFECT_MODE: %s%s\n", EFFECT_FLASH_MODE_NAME,AudioCore.Audioeffect.cur_effect_para->user_effect_name);
		}
		else
#endif
		{
			DBG("EFFECT_MODE: %s\n", AudioCore.Audioeffect.cur_effect_para->user_effect_name);
		}

	}

	AudioCore.Audioeffect.audioeffect_memory_size = roboeffect_estimate_memory_size(
			AudioCore.Audioeffect.cur_effect_para->user_effect_steps, AudioCore.Audioeffect.cur_effect_para->user_effect_list, AudioCore.Audioeffect.user_effect_parameters);
	DBG("Audio effect need malloc memory: %ld\n", AudioCore.Audioeffect.audioeffect_memory_size);

	if(AudioCore.Audioeffect.audioeffect_memory_size == ROBOEFFECT_FRAME_SIZE_ERROR
		&& AudioCore.Audioeffect.cur_effect_para->user_effect_list->frame_size != roboeffect_estimate_frame_size(AudioCore.Audioeffect.cur_effect_para->user_effect_list, AudioCore.Audioeffect.user_effect_parameters))
	{
		//֡������  ��������֡��
		AudioCore.Audioeffect.cur_effect_para->user_effect_list->frame_size = roboeffect_estimate_frame_size(AudioCore.Audioeffect.cur_effect_para->user_effect_list, AudioCore.Audioeffect.user_effect_parameters);
		AudioCore.Audioeffect.audioeffect_memory_size = roboeffect_estimate_memory_size(
				AudioCore.Audioeffect.cur_effect_para->user_effect_steps, AudioCore.Audioeffect.cur_effect_para->user_effect_list, AudioCore.Audioeffect.user_effect_parameters);
		DBG("Audio effect need malloc memory: %ld %lu\n", AudioCore.Audioeffect.audioeffect_memory_size,AudioCore.Audioeffect.cur_effect_para->user_effect_list->frame_size);
	}
	if(AudioCore.Audioeffect.audioeffect_memory_size < 0)
	{
		DBG("**************************************\n!!!ERROR!!!\n");
		switch(AudioCore.Audioeffect.audioeffect_memory_size)
		{
			case ROBOEFFECT_EFFECT_NOT_EXISTED:
			DBG("ĳ����Ч������ \n");
			break;
			case ROBOEFFECT_EFFECT_PARAMS_NOT_FOUND:
			DBG("û���ҵ���ص�ַ����Ч���� \n");
			break;
			case ROBOEFFECT_INSUFFICIENT_MEMORY:
			DBG("�ڴ治�� \n");
			break;
			case ROBOEFFECT_EFFECT_INIT_FAILED:
			DBG("��Ч�������Ϸ� \n");
			break;
			case ROBOEFFECT_ILLEGAL_OPERATION:
			DBG("��Ч�����д��ڴ������ \n");
			break;
			case ROBOEFFECT_EFFECT_LIB_NOT_MATCH_1:
			DBG("��Ч�����汾��Ҫ���� \n");
			break;
			case ROBOEFFECT_EFFECT_LIB_NOT_MATCH_2:
			DBG("Roboeffect�����Ч��汾��ƥ�� \n");
			break;
			case ROBOEFFECT_ADDRESS_NOT_EXISTED:
			DBG("��Ч������ַ������  \n");
			break;
			case ROBOEFFECT_PARAMS_ERROR:
			DBG("�Զ�����Ч����  \n");
			break;
			case ROBOEFFECT_FRAME_SIZE_ERROR:
			DBG("֡������  \n");
			break;
			case ROBOEFFECT_MEMORY_SIZE_QUERY_ERROR:
			DBG("�ڴ��ѯ����  \n");
			break;
		}
		DBG("**************************************\n");
		return FALSE;
	}

	if(AudioCore.Audioeffect.audioeffect_memory_size >= (xPortGetFreeHeapSize() - 5120))
	{
		DBG("**************************************\n");
		if(AudioCore.Audioeffect.effect_addr)
		{
			DBG("Error:memory is not enough because effect 0x%x need too much!!!\nDon't open it.\n", AudioCore.Audioeffect.effect_addr);
		}
		else
		{
			DBG("Error:memory is not enough!!! Please disable some effects.\n");
			DBG("**************************************\n");
			return FALSE;
		}
		DBG("**************************************\n");
		AudioCore.Audioeffect.effect_enable = 0;
		AudioEffect_update_local_effect_status(AudioCore.Audioeffect.effect_addr, AudioCore.Audioeffect.effect_enable);
		AudioCore.Audioeffect.cur_effect_para->user_effect_list->frame_size = AudioCoreFrameSizeGet(DefaultNet);
		AudioCore.Audioeffect.audioeffect_memory_size = roboeffect_estimate_memory_size(
				AudioCore.Audioeffect.cur_effect_para->user_effect_steps, AudioCore.Audioeffect.cur_effect_para->user_effect_list, AudioCore.Audioeffect.user_effect_parameters);
		DBG("Finally malloc:%ld, leave:%ld\n", AudioCore.Audioeffect.audioeffect_memory_size, xPortGetFreeHeapSize());
	}
	/**
	 * malloc context memory
	*/
	if(AudioCore.Audioeffect.audioeffect_memory_size < xPortGetFreeHeapSize())
	{
		AudioCore.Audioeffect.context_memory = osPortMallocFromEnd(AudioCore.Audioeffect.audioeffect_memory_size);
		if(AudioCore.Audioeffect.context_memory == NULL)
		{
			return FALSE;
		}
		/**
		 * initial roboeffect context memory
		*/
		if(ROBOEFFECT_ERROR_OK != roboeffect_init(AudioCore.Audioeffect.context_memory,
												  AudioCore.Audioeffect.audioeffect_memory_size,
												  AudioCore.Audioeffect.cur_effect_para->user_effect_steps,
												  AudioCore.Audioeffect.cur_effect_para->user_effect_list,
												  AudioCore.Audioeffect.user_effect_parameters) )
		{
			DBG("roboeffect_init failed.\n");
			return FALSE;
		}
		else
		{
			DBG("roboeffect_init ok.\n");
			AudioCore.Audioeffect.effect_addr = 0;

			////Audio Core & Audioeffect��������
			SystemVolSet();
		}
	}

	//After effect init done, AudioCore know what frame size should be set.
	AudioCoreFrameSizeSet(DefaultNet, roboeffect_estimate_frame_size(AudioCore.Audioeffect.cur_effect_para->user_effect_list, AudioCore.Audioeffect.user_effect_parameters));

	roboeffect_prot_init();
	return TRUE;
}

#ifdef CFG_RES_AUDIO_I2SOUT_EN
void AudioI2sOutParamsSet(void)
{
	I2SParamCt i2s_set;
	i2s_set.IsMasterMode = CFG_RES_I2S_MODE;// 0:master 1:slave
	i2s_set.SampleRate = CFG_PARA_I2S_SAMPLERATE; //���������
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
}
#endif

//����ϵͳ��׼ͨ·
bool ModeCommonInit(void)
{
	AudioCoreIO AudioIOSet;
	uint16_t FifoLenStereo,i;

	for(i = 0; i < MaxNet; i++)
	{
		AudioCoreMixSampleRateSet(i, CFG_PARA_SAMPLE_RATE);//Ĭ��ϵͳ������
	}
	APP_DBG("Systerm SampleRate: %d\n", (uint16_t)AudioCoreMixSampleRateGet(DefaultNet));

	if(!AudioEffectInit())
	{
		DBG("!!!audioeffect init must be earlier than sink init!!!.\n");
	}
	AudioEffect_GetAudioEffectMaxValue();

	FifoLenStereo = AudioCoreFrameSizeGet(DefaultNet) * sizeof(PCM_DATA_TYPE) * 2 * 2;//������8����С��֡������λbyte

	DefaultParamgsInit();	//refresh local hardware config params(just storage not set)

	//////////����DMA fifo
#ifdef CFG_RES_AUDIO_DAC0_EN
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
	AudioIOSet.Depth = AudioCore.FrameSize[0] * 2 ;
#ifdef	CFG_AUDIO_WIDTH_24BIT
	AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
	AudioIOSet.IOBitWidthConvFlag = 0;//����Ҫ��λ��ת������
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
		uint16_t BitWidth;
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		BitWidth = 24;
	#else
		BitWidth = 16;
	#endif
		AudioDAC_Init(&DACDefaultParamCt,mainAppCt.SampleRate,BitWidth, (void*)mainAppCt.DACFIFO, mainAppCt.DACFIFO_LEN, NULL, 0);

	#ifdef CFG_FUNC_MCLK_USE_CUSTOMIZED_EN
		Clock_AudioMclkSel(AUDIO_DAC0, gCtrlVars.HwCt.DAC0Ct.dac_mclk_source);
	#else
		gCtrlVars.HwCt.DAC0Ct.dac_mclk_source = Clock_AudioMclkGet(AUDIO_DAC0);
	#endif
	}
	else//sam add,20230221
	{
		AudioDAC0_SampleRateChange(CFG_PARA_SAMPLE_RATE);
		gCtrlVars.HwCt.DAC0Ct.dac_mclk_source = Clock_AudioMclkGet(AUDIO_DAC0);
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

	#ifdef CFG_FUNC_MCLK_USE_CUSTOMIZED_EN
		Clock_AudioMclkSel(AUDIO_ADC1, gCtrlVars.HwCt.ADC1DigitalCt.adc_mclk_source);
	#else
		gCtrlVars.HwCt.ADC1DigitalCt.adc_mclk_source = Clock_AudioMclkGet(AUDIO_ADC1);
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
		AudioIOSet.IOBitWidthConvFlag = 0;//��Ҫ���ݽ���λ����չ
	#endif
		if(!AudioCoreSourceInit(&AudioIOSet, MIC_SOURCE_NUM))
		{
			DBG("mic Source error");
			return FALSE;
		}
	}
#ifdef CFG_ADCDAC_SEL_LOWPOWERMODE
	AudioADC_AnaInit(ADC1_MODULE,CHANNEL_LEFT,MIC_LEFT,gCtrlVars.HwCt.ADC1PGACt.pga_mic_mode,ADCLowEnergy,31 - gCtrlVars.HwCt.ADC1PGACt.pga_mic_gain);
#else
	AudioADC_AnaInit(ADC1_MODULE,CHANNEL_LEFT,MIC_LEFT,gCtrlVars.HwCt.ADC1PGACt.pga_mic_mode,ADCCommonEnergy,31 - gCtrlVars.HwCt.ADC1PGACt.pga_mic_gain);
#endif // CFG_ADCDAC_SEL_LOWPOWERMODE
	//MADC_MIC_PowerUP(SingleEnded);
	AudioCoreSourceEnable(MIC_SOURCE_NUM);
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(!AudioCoreSourceIsInit(REMIND_SOURCE_NUM))
	{
		memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
		AudioIOSet.Adapt = STD;//SRC_ONLY;
		AudioIOSet.SampleRate = CFG_PARA_SAMPLE_RATE;//��ʼֵ
		AudioIOSet.Sync = FALSE;
		AudioIOSet.Channels = 1;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.DataIOFunc = RemindDataGet;
		AudioIOSet.LenGetFunc = RemindDataLenGet;

	#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_16BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 1;//��Ҫ���ݽ���λ����չ
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
	AudioIOSet.IOBitWidthConvFlag = 0;//����Ҫ��λ��ת������
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
		// Master �򲻿�΢��
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
		AudioIOSet.Sync = TRUE;//I2S slave ʱ�����masterû�нӣ��п��ܻᵼ��DACҲ����������
		AudioIOSet.Channels = 2;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.HighLevelCent = 60;
		AudioIOSet.LowLevelCent = 40;
		AudioIOSet.SampleRate = CFG_PARA_I2S_SAMPLERATE;//����ʵ������ѡ��
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
	AudioIOSet.IOBitWidthConvFlag = 0;//����Ҫ��λ��ת������
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

#ifdef CFG_I2S_SLAVE_TO_SPDIFOUT_EN
		AudioIOSet.Adapt = STD;
#else
		AudioIOSet.Adapt = SRC_ONLY;
#endif
		AudioIOSet.Sync = TRUE;
		AudioIOSet.Channels = 2;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.HighLevelCent = 60;
		AudioIOSet.LowLevelCent = 40;
		AudioIOSet.SampleRate = CFG_PARA_SAMPLE_RATE;//����ʵ������ѡ��

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
	//ע�� �Ƿ���Ҫģʽ���ˣ�����ģʽ�������ͻָ�
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

//�ͷŹ���ͨ·��
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
	osPortFree(AudioCore.Audioeffect.context_memory);
	AudioCore.Audioeffect.context_memory = NULL;
}

bool AudioIoCommonForHfp(uint32_t sampleRate, uint16_t gain)
{
	AudioCoreIO AudioIOSet;
	uint16_t FifoLenStereo;
//	uint16_t FifoLenMono = SampleLen * 2 * 2;//������4����С��֡������λbyte

	if(!AudioEffectInit())
	{
		DBG("!!!audioeffect init must be earlier than sink init!!!.\n");
	}
	FifoLenStereo = AudioCoreFrameSizeGet(DefaultNet) * 2 * 2 * 2;//������8����С��֡������λbyte

	DefaultParamgsInit();	//refresh local hardware config params(just storage not set)

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

	#ifdef CFG_FUNC_MCLK_USE_CUSTOMIZED_EN
		Clock_AudioMclkSel(AUDIO_ADC1, gCtrlVars.HwCt.ADC1DigitalCt.adc_mclk_source);
	#else
		gCtrlVars.HwCt.ADC1DigitalCt.adc_mclk_source = Clock_AudioMclkGet(AUDIO_ADC1);
	#endif
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
		AudioIOSet.IOBitWidthConvFlag = 0;	//��Ҫ����λ����չ
#endif
		if(!AudioCoreSourceInit(&AudioIOSet, MIC_SOURCE_NUM))
		{
			DBG("mic Source error");
			return FALSE;
		}
		//MADC_MIC_PowerUP(SingleEnded);
#ifdef CFG_ADCDAC_SEL_LOWPOWERMODE
		AudioADC_AnaInit(ADC1_MODULE,CHANNEL_LEFT,MIC_LEFT,gCtrlVars.HwCt.ADC1PGACt.pga_mic_mode,ADCLowEnergy,gain);
#else
		AudioADC_AnaInit(ADC1_MODULE,CHANNEL_LEFT,MIC_LEFT,gCtrlVars.HwCt.ADC1PGACt.pga_mic_mode,ADCCommonEnergy,gain);
#endif // CFG_ADCDAC_SEL_LOWPOWERMODE
		AudioCoreSourceEnable(MIC_SOURCE_NUM);
	}
	else //�����ʵ� ����
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

	#ifdef CFG_FUNC_MCLK_USE_CUSTOMIZED_EN
		Clock_AudioMclkSel(AUDIO_ADC1, gCtrlVars.HwCt.ADC1DigitalCt.adc_mclk_source);
	#else
		gCtrlVars.HwCt.ADC1DigitalCt.adc_mclk_source = Clock_AudioMclkGet(AUDIO_ADC1);
	#endif
	}
#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
	if(!AudioCoreSourceIsInit(REMIND_SOURCE_NUM))
	{
		memset(&AudioIOSet, 0, sizeof(AudioCoreIO));

		AudioIOSet.Adapt = SRC_ONLY;
		AudioIOSet.SampleRate = CFG_PARA_SAMPLE_RATE;//��ʼֵ
		AudioIOSet.Sync = FALSE;
		AudioIOSet.Channels = 1;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.DataIOFunc = RemindDataGet;
		AudioIOSet.LenGetFunc = RemindDataLenGet;

#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_16BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;//��Ҫ���ݽ���λ����չ
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
	AudioIOSet.IOBitWidthConvFlag = 1;//DAC 24bit ,sink���һ�����ʱ��Ҫת��Ϊ24bi
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

		uint16_t BitWidth;
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		BitWidth = 24;
	#else
		BitWidth = 16;
	#endif
		AudioDAC_Init(&DACDefaultParamCt,sampleRate,BitWidth, (void*)mainAppCt.DACFIFO, mainAppCt.DACFIFO_LEN, NULL, 0);

	#ifdef CFG_FUNC_MCLK_USE_CUSTOMIZED_EN
		Clock_AudioMclkSel(AUDIO_DAC0, gCtrlVars.HwCt.DAC0Ct.dac_mclk_source);
	#else
		gCtrlVars.HwCt.DAC0Ct.dac_mclk_source = Clock_AudioMclkGet(AUDIO_DAC0);
	#endif
	}
	else
	{
		AudioDAC0_SampleRateChange(sampleRate);
		gCtrlVars.HwCt.DAC0Ct.dac_mclk_source = Clock_AudioMclkGet(AUDIO_DAC0);
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
	AudioIOSet.IOBitWidthConvFlag = 1;//����Ҫ��λ��ת������
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
		// Master �򲻿�΢��
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
		AudioIOSet.Sync = TRUE;//I2S slave ʱ�����masterû�нӣ��п��ܻᵼ��DACҲ����������
		AudioIOSet.Channels = 2;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.HighLevelCent = 60;
		AudioIOSet.LowLevelCent = 40;
		AudioIOSet.SampleRate = CFG_PARA_I2S_SAMPLERATE;//����ʵ������ѡ��
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
bool AudioEffectModeSel(EFFECT_MODE effectMode, uint8_t sel)
{
	uint8_t i;

	if(sel == 1 || sel == 2)
	{
		PauseAuidoCore();
		osPortFree(AudioCore.Audioeffect.context_memory);
		AudioCore.Audioeffect.context_memory = NULL;

		if(!AudioEffectInit())
		{
			DBG("audioeffect init fail, please check!!!\n");
			return FALSE;
		}

		for(i = 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
		{
			if(AudioCoreSourceToRoboeffect(i) != AUDIOCORE_SOURCE_SINK_ERROR)
			{
				AudioCore.AudioSource[i].PcmInBuf = roboeffect_get_source_buffer(
								AudioCore.Audioeffect.context_memory, AudioCoreSourceToRoboeffect(i));
			}
		}
		for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
		{
			if(AudioCoreSinkToRoboeffect(i) != AUDIOCORE_SOURCE_SINK_ERROR)
			{
				AudioCore.AudioSink[i].PcmOutBuf = roboeffect_get_sink_buffer(
								AudioCore.Audioeffect.context_memory, AudioCoreSinkToRoboeffect(i));
			}
		}

#ifdef CFG_APP_LINEIN_MODE_EN
		if(GetSystemMode() == ModeLineAudioPlay)
		{
			//lineinģʽ��Ҫ���³�ʼ��ADC0������delay����
			extern void LineinADCDigitalInit(void);
			LineinADCDigitalInit();
		}
#endif

#ifdef CFG_RES_I2S_MODULE
		extern void RST_I2SModule(I2S_MODULE I2SModuleIndex);
		RST_I2SModule(CFG_RES_I2S_MODULE);
#ifdef CFG_APP_I2SIN_MODE_EN
		DMA_ChannelDisable(PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_I2S_MODULE);
		DMA_CircularFIFOClear(PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_I2S_MODULE);
		DMA_ChannelEnable(PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_I2S_MODULE);
#endif
#ifdef CFG_RES_AUDIO_I2SOUT_EN
		DMA_ChannelDisable(PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_I2S_MODULE);
		DMA_CircularFIFOClear(PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_I2S_MODULE);
		DMA_ChannelEnable(PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_I2S_MODULE);
#endif
#endif
#ifdef CFG_RES_MIX_I2S_MODULE
		extern void RST_I2SModule(I2S_MODULE I2SModuleIndex);
		RST_I2SModule(CFG_RES_MIX_I2S_MODULE);
#ifdef CFG_RES_AUDIO_I2S_MIX_IN_EN
		DMA_ChannelDisable(PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_MIX_I2S_MODULE);
		DMA_CircularFIFOClear(PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_MIX_I2S_MODULE);
		DMA_ChannelEnable(PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_MIX_I2S_MODULE);
#endif
#ifdef CFG_RES_AUDIO_I2S_MIX_OUT_EN
		DMA_ChannelDisable(PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_MIX_I2S_MODULE);
		DMA_CircularFIFOClear(PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_MIX_I2S_MODULE);
		DMA_ChannelEnable(PERIPHERAL_ID_I2S0_TX + 2 * CFG_RES_MIX_I2S_MODULE);
#endif
#endif

		SoftFlagDeregister(SoftFlagAudioCoreSourceIsDeInit);
		AudioCoreServiceResume();
	}
	if(sel == 0 || sel == 2)
	{
		DefaultParamgsInit();
		AudioCodecGainUpdata();//update hardware config
	}
	return TRUE;
}

//��ģʽ�µ�ͨ����Ϣ����, ���е���ʾ���ڴ˴������Ҫ����ô�APIǰ��ȷ��APP running״̬�����������û׼���á�
void CommonMsgProccess(uint16_t Msg)
{
	uint8_t     EffectMode;
#if defined(CFG_FUNC_DISPLAY_EN)
	MessageContext	msgSend;
#endif
	if(SoftFlagGet(SoftFlagDiscDelayMask) && Msg == MSG_NONE)
	{
		Msg = MSG_BT_STATE_DISCONNECT;
	}

	switch(Msg)
	{
		case MSG_MENU://�˵���
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
			AudioEffect_ReverbStep_Ajust(mainAppCt.ReverbStep);
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
			AudioEffect_ReverbStep_Ajust(mainAppCt.ReverbStep);
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
			AudioEffect_EQMode_Set(mainAppCt.EqMode);

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
			AudioEffect_EQ_Ajust(MIC_EQ, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
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
			AudioEffect_EQ_Ajust(MIC_EQ, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
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
			AudioEffect_EQ_Ajust(MIC_EQ, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
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
			AudioEffect_EQ_Ajust(MIC_EQ, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
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
			AudioEffect_EQ_Ajust(MUSIC_EQ, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);

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
			AudioEffect_EQ_Ajust(MUSIC_EQ, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
			
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
			AudioEffect_EQ_Ajust(MUSIC_EQ, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);

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
			AudioEffect_EQ_Ajust(MUSIC_EQ, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
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
			AudioEffect_effect_enable(VOCAL_CUT, !AudioEffect_effect_status_Get(VOCAL_CUT));
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
			EffectMode = mainAppCt.EffectMode;
#ifndef CFG_FUNC_EFFECT_BYPASS_EN
			if(GetSystemMode() == ModeBtHfPlay	//����ͨ��ģʽ�£���֧����Чģʽ�л�
#ifdef CFG_FUNC_RECORDER_EN
			|| SoftFlagGet(SoftFlagRecording)	//¼���У������л���Чģʽ
#endif
			)
			{
				break;
			}

			if(IsAudioPlayerMute() == FALSE)
			{
				HardWareMuteOrUnMute();
			}
			if(AudioCore.Audioeffect.effect_mode_expected)
			{
				mainAppCt.EffectMode = AudioCore.Audioeffect.effect_mode_expected;
			}
			else
			{
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
			}
#ifdef CFG_FUNC_BREAKPOINT_EN
			BackupInfoUpdata(BACKUP_SYS_INFO);
#endif

			AUDIOEFFECT_EFFECT_PARA *mpara = get_user_effect_parameters(mainAppCt.EffectMode);

			DBG("EFFECT_MODE: %s\n", mpara->user_effect_name);
			if (roboeffect_estimate_frame_size(mpara->user_effect_list, mpara->user_effect_parameters) == AudioCoreFrameSizeGet(DefaultNet))
			{
				if(!AudioEffectModeSel(mainAppCt.EffectMode, 2))//sel: 0=init hw, 1=effect, 2=hw + effect
				{
					 mainAppCt.EffectMode = EffectMode; //�л����ɹ����ָ�֮ǰEffectMode
					 mpara = get_user_effect_parameters(mainAppCt.EffectMode);
					 AudioEffectModeSel(mainAppCt.EffectMode, 2);
				}
			}
			else
			{
				uint8_t defaultFrameSize= mpara->user_effect_list->frame_size;
				mpara->user_effect_list->frame_size = roboeffect_estimate_frame_size(mpara->user_effect_list, mpara->user_effect_parameters);
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
										AudioCore.Audioeffect.context_memory, AudioCoreSourceToRoboeffect(i));
					}
				}
				for(int8_t i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
				{
					if(AudioCoreSinkToRoboeffect(i) != AUDIOCORE_SOURCE_SINK_ERROR)
					{
						AudioCore.AudioSink[i].PcmOutBuf = roboeffect_get_sink_buffer(
										AudioCore.Audioeffect.context_memory, AudioCoreSinkToRoboeffect(i));
					}
				}
				SoftFlagDeregister(SoftFlagAudioCoreSourceIsDeInit);
				AudioCoreServiceResume();
				mpara->user_effect_list->frame_size = defaultFrameSize;
			}

			AudioEffectParamSync();		//switch effect mode also need to sync some params

			if(IsAudioPlayerMute() == TRUE)
			{
				HardWareMuteOrUnMute();
			}
			if(AudioCore.Audioeffect.effect_mode_expected)
			{
				AudioCore.Audioeffect.effect_mode_expected = 0;
				gCtrlVars.AutoRefresh = AutoRefresh_ALL_EFFECTS_PARA;
			}
			else
			{
				extern AUDIOEFFECT_EFFECT_PARA_TABLE * GetCurEffectParaNode(void);
				AUDIOEFFECT_EFFECT_PARA_TABLE *param_t = GetCurEffectParaNode();
				if((EffectMode < param_t->effect_id ) || (EffectMode >= (param_t->effect_id + param_t->effect_id_count)))
				{
					gCtrlVars.AutoRefresh = AutoRefresh_ALL_PARA;
				}
				else
				{
					gCtrlVars.AutoRefresh = AutoRefresh_ALL_EFFECTS_PARA;
				}
			}
#endif
#endif
			break;
		case MSG_EFFECTREINIT:
			APP_DBG("MSG_EFFECTREINIT\n");
			if(IsAudioPlayerMute() == FALSE)
			{
				HardWareMuteOrUnMute();
			}

			AUDIOEFFECT_EFFECT_PARA *para = get_user_effect_parameters(mainAppCt.EffectMode);
			if (para->user_effect_list->sample_rate != AudioCoreMixSampleRateGet(DefaultNet))
			{
				AudioEffectModeSel(mainAppCt.EffectMode, 1);//sel: 0=init hw, 1=effect, 2=hw + effect
				gCtrlVars.AutoRefresh = AutoRefresh_ALL_EFFECTS_PARA;
			}
			else
			{
				int8_t opera = AudioCore.Audioeffect.effect_enable ? 1 : -1;
				uint8_t addr = AudioCore.Audioeffect.effect_addr;
				uint32_t FrameSize = AudioCoreFrameSizeGet(DefaultNet);

				if(roboeffect_get_frame_size_after_effect_change(
						AudioCore.Audioeffect.context_memory, AudioCore.Audioeffect.audioeffect_frame_size, AudioCore.Audioeffect.effect_addr, opera)
					&& AudioCoreFrameSizeGet(DefaultNet) != roboeffect_get_frame_size_after_effect_change(
						AudioCore.Audioeffect.context_memory, AudioCore.Audioeffect.audioeffect_frame_size, AudioCore.Audioeffect.effect_addr, opera))
				{
					AudioCore.Audioeffect.cur_effect_para->user_effect_list->frame_size = roboeffect_get_frame_size_after_effect_change(
							AudioCore.Audioeffect.context_memory, AudioCore.Audioeffect.audioeffect_frame_size, AudioCore.Audioeffect.effect_addr, opera);
					APP_DBG("Need Change FrameSize to %ld\n", AudioCore.Audioeffect.cur_effect_para->user_effect_list->frame_size);

					if(!SysCurModeReboot())
					{
						//��ǰģʽ����ʧ�ܣ��ָ��ϴβ������ٴ�����ģʽ
						if(AudioCore.Audioeffect.effect_enable)
							AudioCore.Audioeffect.effect_enable = 0;
						AudioCore.Audioeffect.cur_effect_para->user_effect_list->frame_size = FrameSize;
						SysCurModeReboot();
					}
					gCtrlVars.AutoRefresh = AutoRefresh_ALL_EFFECTS_PARA;
				}
				else
				{
					AudioEffectModeSel(mainAppCt.EffectMode, 1);//sel: 0=init hw, 1=effect, 2=hw + effect
					gCtrlVars.AutoRefresh = addr;
				}
			}

			if(IsAudioPlayerMute() == TRUE)
			{
				HardWareMuteOrUnMute();
			}
            AudioCore.Audioeffect.reinit_done = 1;
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
		case MSG_BT_OPEN_ACCESS:
			if (GetBtManager()->btAccessModeEnable != BT_ACCESSBLE_GENERAL)
			{
				GetBtManager()->btAccessModeEnable = BT_ACCESSBLE_GENERAL;
				BtStackServiceMsgSend(MSG_BTSTACK_ACCESS_MODE_SET);
				DBG("open bt access\n");
			}
			break;
			
		//�������ӶϿ���Ϣ,������ʾ��
		case MSG_BT_STATE_CONNECTED:
			APP_DBG("[BT_STATE]:BT Connected...\n");
			//�쳣���������У�����ʾ���ӶϿ���ʾ��

			#ifdef BT_REAL_STATE
			SetBtUserState(BT_USER_STATE_CONNECTED);
			#endif

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

			#ifdef BT_REAL_STATE
			SetBtUserState(BT_USER_STATE_DISCONNECTED);
			#endif
			
			if(btManager.btDutModeEnable)
				break;

			BtStackServiceMsgSend(MSG_BTSTACK_ACCESS_MODE_SET);

			//�쳣���������У�����ʾ���ӶϿ���ʾ��
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
