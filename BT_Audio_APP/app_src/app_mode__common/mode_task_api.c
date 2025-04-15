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
#include "user_defined_effect_api.h"
//app
#include "bt_stack_service.h"
#if (BT_AVRCP_VOLUME_SYNC == ENABLE)
#include "bt_app_avrcp_deal.h"
#endif

#define roboeffect_malloc osPortMallocFromEnd
#define roboeffect_free osPortFree
extern volatile SysModeStruct SysMode[];
extern uint32_t GetModeIndexInModeLoop(SysModeNumber *sys_mode);

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
		
		if(mainAppCt.EffectMode == EFFECT_MODE_MIC)
		{
			memcpy(&local_effect_list, &user_effect_list_mic, sizeof(roboeffect_effect_list_info));
			AudioCore.Roboeffect.user_effect_list = (roboeffect_effect_list_info *)&local_effect_list;
			AudioCore.Roboeffect.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_mic;
			AudioCore.Roboeffect.user_effects_script = (uint8_t *)user_effects_script_mic;
			AudioCore.Roboeffect.user_effects_script_len = (uint16_t)get_user_effects_script_len_mic();
			AudioCore.Roboeffect.user_effect_parameters = osPortMalloc(
					get_user_effect_parameters_len((uint8_t *)user_effect_parameters_mic_mic) * sizeof(uint8_t));
			memcpy(AudioCore.Roboeffect.user_effect_parameters, (uint8_t *)user_effect_parameters_mic_mic,
					get_user_effect_parameters_len((uint8_t *)user_effect_parameters_mic_mic) * sizeof(uint8_t));
			AudioCore.Roboeffect.user_module_parameters = (uint8_t *)user_module_parameters_mic_mic;
			AudioCore.Roboeffect.flow_chart_mode = 0;
			AudioCore.Roboeffect.effect_count = MIC_COUNT_ADDR - 1;
			DBG("EFFECT_MODE Mic\n");
		}
		else if(mainAppCt.EffectMode == EFFECT_MODE_MUSIC)
		{
			memcpy(&local_effect_list, &user_effect_list_music, sizeof(roboeffect_effect_list_info));
			AudioCore.Roboeffect.user_effect_list = (roboeffect_effect_list_info *)&local_effect_list;
			AudioCore.Roboeffect.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_music;
			AudioCore.Roboeffect.user_effects_script = (uint8_t *)user_effects_script_music;
			AudioCore.Roboeffect.user_effects_script_len = (uint16_t)get_user_effects_script_len_music();
			AudioCore.Roboeffect.user_effect_parameters = osPortMalloc(
					get_user_effect_parameters_len((uint8_t *)user_effect_parameters_music_music) * sizeof(uint8_t));
			memcpy(AudioCore.Roboeffect.user_effect_parameters, (uint8_t *)user_effect_parameters_music_music,
					get_user_effect_parameters_len((uint8_t *)user_effect_parameters_music_music) * sizeof(uint8_t));
			AudioCore.Roboeffect.user_module_parameters = (uint8_t *)user_module_parameters_music_music;
			AudioCore.Roboeffect.flow_chart_mode = 1;
			AudioCore.Roboeffect.effect_count = MUSIC_COUNT_ADDR - 1;
			DBG("EFFECT_MODE Music\n");
		}
		else if(mainAppCt.EffectMode == EFFECT_MODE_HFP_AEC)
		{
			memcpy(&local_effect_list, &user_effect_list_hfp, sizeof(roboeffect_effect_list_info));
			AudioCore.Roboeffect.user_effect_list = (roboeffect_effect_list_info *)&local_effect_list;
			AudioCore.Roboeffect.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_hfp;
			AudioCore.Roboeffect.user_effects_script = (uint8_t *)user_effects_script_hfp;
			AudioCore.Roboeffect.user_effects_script_len = (uint16_t)get_user_effects_script_len_hfp();
			AudioCore.Roboeffect.user_effect_parameters = osPortMalloc(
					get_user_effect_parameters_len((uint8_t *)user_effect_parameters_hfp_hfp) * sizeof(uint8_t));
			memcpy(AudioCore.Roboeffect.user_effect_parameters, (uint8_t *)user_effect_parameters_hfp_hfp,
					get_user_effect_parameters_len((uint8_t *)user_effect_parameters_hfp_hfp) * sizeof(uint8_t));
			AudioCore.Roboeffect.user_module_parameters = (uint8_t *)user_module_parameters_hfp_hfp;
			AudioCore.Roboeffect.flow_chart_mode = 2;
			AudioCore.Roboeffect.effect_count = HFP_COUNT_ADDR - 1;
			DBG("EFFECT_MODE HFP\n");
		}
#ifdef CFG_FUNC_MIC_KARAOKE_EN
		else if(mainAppCt.EffectMode >= EFFECT_MODE_HunXiang && mainAppCt.EffectMode <= EFFECT_MODE_WaWaYin)
		{
			memcpy(&local_effect_list, &user_effect_list_Karaoke, sizeof(roboeffect_effect_list_info));
			AudioCore.Roboeffect.user_effect_list = (roboeffect_effect_list_info *)&local_effect_list;
			AudioCore.Roboeffect.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_Karaoke;
			AudioCore.Roboeffect.user_effects_script = (uint8_t *)user_effects_script_Karaoke;
			AudioCore.Roboeffect.user_effects_script_len = (uint16_t)get_user_effects_script_len_Karaoke();
			AudioCore.Roboeffect.user_effect_parameters = osPortMalloc(
					get_user_effect_parameters_len((uint8_t *)user_effect_parameters_Karaoke_HunXiang) * sizeof(uint8_t));
			AudioCore.Roboeffect.effect_count = KARAOKE_COUNT_ADDR - 1;

			switch(mainAppCt.EffectMode){
				case EFFECT_MODE_HunXiang:
					memcpy(AudioCore.Roboeffect.user_effect_parameters, (uint8_t *)user_effect_parameters_Karaoke_HunXiang,
							get_user_effect_parameters_len((uint8_t *)user_effect_parameters_Karaoke_HunXiang) * sizeof(uint8_t));
					AudioCore.Roboeffect.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_HunXiang;
					AudioCore.Roboeffect.flow_chart_mode = 3;
					DBG("EFFECT_MODE HunXiang\n");
					break;
				case EFFECT_MODE_DianYin:
					memcpy(AudioCore.Roboeffect.user_effect_parameters, (uint8_t *)user_effect_parameters_Karaoke_DianYin,
							get_user_effect_parameters_len((uint8_t *)user_effect_parameters_Karaoke_DianYin) * sizeof(uint8_t));
					AudioCore.Roboeffect.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_DianYin;
					AudioCore.Roboeffect.flow_chart_mode = 4;
					DBG("EFFECT_MODE DianYin\n");
					break;
				case EFFECT_MODE_MoYin:
					memcpy(AudioCore.Roboeffect.user_effect_parameters, (uint8_t *)user_effect_parameters_Karaoke_MoYin,
							get_user_effect_parameters_len((uint8_t *)user_effect_parameters_Karaoke_MoYin) * sizeof(uint8_t));
					AudioCore.Roboeffect.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_MoYin;
					AudioCore.Roboeffect.flow_chart_mode = 5;
					DBG("EFFECT_MODE MoYin\n");
					break;
				case EFFECT_MODE_HanMai:
					memcpy(AudioCore.Roboeffect.user_effect_parameters, (uint8_t *)user_effect_parameters_Karaoke_HanMai,
							get_user_effect_parameters_len((uint8_t *)user_effect_parameters_Karaoke_HanMai) * sizeof(uint8_t));
					AudioCore.Roboeffect.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_HanMai;
					AudioCore.Roboeffect.flow_chart_mode = 6;
					DBG("EFFECT_MODE HanMai\n");
					break;
				case EFFECT_MODE_NanBianNv:
					memcpy(AudioCore.Roboeffect.user_effect_parameters, (uint8_t *)user_effect_parameters_Karaoke_NanBianNv,
							get_user_effect_parameters_len((uint8_t *)user_effect_parameters_Karaoke_NanBianNv) * sizeof(uint8_t));
					AudioCore.Roboeffect.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_NanBianNv;
					AudioCore.Roboeffect.flow_chart_mode = 7;
					DBG("EFFECT_MODE NanBianNv\n");
					break;
				case EFFECT_MODE_NvBianNan:
					memcpy(AudioCore.Roboeffect.user_effect_parameters, (uint8_t *)user_effect_parameters_Karaoke_NvBianNan,
							get_user_effect_parameters_len((uint8_t *)user_effect_parameters_Karaoke_NvBianNan) * sizeof(uint8_t));
					AudioCore.Roboeffect.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_NvBianNan;
					AudioCore.Roboeffect.flow_chart_mode = 8;
					DBG("EFFECT_MODE NvBianNan\n");
					break;
				case EFFECT_MODE_WaWaYin:
					memcpy(AudioCore.Roboeffect.user_effect_parameters, (uint8_t *)user_effect_parameters_Karaoke_WaWaYin,
							get_user_effect_parameters_len((uint8_t *)user_effect_parameters_Karaoke_WaWaYin) * sizeof(uint8_t));
					AudioCore.Roboeffect.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_WaWaYin;
					AudioCore.Roboeffect.flow_chart_mode = 9;
					DBG("EFFECT_MODE WaWaYin\n");
					break;
				default:
					memcpy(AudioCore.Roboeffect.user_effect_parameters, (uint8_t *)user_effect_parameters_Karaoke_HunXiang,
							get_user_effect_parameters_len((uint8_t *)user_effect_parameters_Karaoke_HunXiang) * sizeof(uint8_t));
					AudioCore.Roboeffect.user_module_parameters = (uint8_t *)user_module_parameters_Karaoke_HunXiang;
					AudioCore.Roboeffect.flow_chart_mode = 3;
					DBG("EFFECT_MODE HunXiang\n");
			}
		}
#else
		else
		{
			memcpy(&local_effect_list, &user_effect_list_mic, sizeof(roboeffect_effect_list_info));
			AudioCore.Roboeffect.user_effect_list = (roboeffect_effect_list_info *)&local_effect_list;
			AudioCore.Roboeffect.user_effect_steps = (roboeffect_effect_steps_table *)&user_effect_steps_mic;
			AudioCore.Roboeffect.user_effects_script = (uint8_t *)user_effects_script_mic;
			AudioCore.Roboeffect.user_effects_script_len = (uint16_t)get_user_effects_script_len_mic();
			AudioCore.Roboeffect.user_effect_parameters = osPortMalloc(
					get_user_effect_parameters_len((uint8_t *)user_effect_parameters_mic_mic) * sizeof(uint8_t));
			memcpy(AudioCore.Roboeffect.user_effect_parameters, (uint8_t *)user_effect_parameters_mic_mic,
					get_user_effect_parameters_len((uint8_t *)user_effect_parameters_mic_mic) * sizeof(uint8_t));
			AudioCore.Roboeffect.user_module_parameters = (uint8_t *)user_module_parameters_mic_mic;
			AudioCore.Roboeffect.flow_chart_mode = 0;
			AudioCore.Roboeffect.effect_count = MIC_COUNT_ADDR - 1;
			DBG("EFFECT_MODE Mic\n");
		}
#endif

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

//配置系统标准通路
bool ModeCommonInit(void)
{
	AudioCoreIO AudioIOSet;
	uint16_t SampleLen = AudioCoreFrameSizeGet(DefaultNet);
	uint16_t FifoLenStereo = SampleLen * sizeof(PCM_DATA_TYPE) * 2 * 2;//立体声8倍大小于帧长，单位byte
//	uint16_t FifoLenMono = SampleLen * sizeof(PCM_DATA_TYPE) * 2;//单声到4倍大小于帧长，单位byte

	if(!RoboeffectInit())
	{
		DBG("!!!roboeffect init must be earlier than sink init!!!.\n");
		if(AudioCore.Roboeffect.effect_addr)
		{
			AudioCore.Roboeffect.effect_enable = 0;
			DBG("roboeffect init again because cannot enable effect:%d\n", RoboeffectInit());
		}
	}
	DefaultParamgsInit();	//refresh local hardware config params(just storage not set)

	//////////申请DMA fifo
#ifdef CFG_RES_AUDIO_DAC0_EN
	memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
	AudioIOSet.Depth = AudioCore.FrameSize[0] * 2 ;
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
		AudioIOSet.DataIOFunc = AudioDAC0DataSet;
		AudioIOSet.LenGetFunc = AudioDAC0DataSpaceLenGet;
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;//不需要做位宽转换处理
	#endif
		if(!AudioCoreSinkInit(&AudioIOSet, AUDIO_DAC0_SINK_NUM))
		{
			DBG("Dac init error");
			return FALSE;
		}
		AudioDAC_Init(mainAppCt.SampleRate,24, (void*)mainAppCt.DACFIFO, mainAppCt.DACFIFO_LEN, NULL, 0);
	}
	else//sam add,20230221
	{
		AudioDAC0_SampleRateChange(CFG_PARA_SAMPLE_RATE);
	#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;//DAC 24bit ,sink最后一级输出时需要转变为24bi
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
		AudioADC_DigitalInit(ADC1_MODULE, mainAppCt.SampleRate, (void*)mainAppCt.ADCFIFO, AudioCoreFrameSizeGet(DefaultNet) * 2 * 2 * 2 * 2,ADC_WIDTH_24BITS);
	#else
		AudioADC_DigitalInit(ADC1_MODULE, mainAppCt.SampleRate, (void*)mainAppCt.ADCFIFO, AudioCoreFrameSizeGet(DefaultNet) * 2 * 2 * 2,ADC_WIDTH_16BITS);
	#endif

		//Soure0.
		memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
		AudioIOSet.Adapt = STD;
		AudioIOSet.Sync = TRUE;
		AudioIOSet.Channels = 1;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.DataIOFunc = AudioADC1DataGet;
		AudioIOSet.LenGetFunc = AudioADC1DataLenGet;
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
	AudioADC_AnaInit(ADC1_MODULE,MIC_LEFT,Single);
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
	#if defined(TWS_IIS0_OUT) || defined(TWS_IIS1_OUT)
		AudioIOSet.Resident = TRUE;
		AudioIOSet.Depth = TWS_SINK_DEV_FIFO_SAMPLES;
	#else
		AudioIOSet.Depth = AudioCore.FrameSize[0] * 2 ;
	#endif
	if(!AudioCoreSinkIsInit(AUDIO_I2SOUT_SINK_NUM))
	{
#if !defined(TWS_IIS0_OUT) && !defined(TWS_IIS1_OUT)
		mainAppCt.I2SFIFO_LEN = AudioIOSet.Depth * sizeof(PCM_DATA_TYPE) * 2;
		mainAppCt.I2SFIFO = (uint32_t*)osPortMalloc(mainAppCt.I2SFIFO_LEN);//I2S fifo
#endif
		if(mainAppCt.I2SFIFO != NULL)
		{
			memset(mainAppCt.I2SFIFO, 0, mainAppCt.I2SFIFO_LEN);
		}
		else
		{
			APP_DBG("malloc I2SFIFO error\n");
			return FALSE;
		}

		if((CFG_RES_I2S_MODE == I2S_MASTER_MODE) || !defined(CFG_FUNC_I2S_IN_SYNC_EN))
		{// Master 或不开微调
	#if CFG_PARA_I2S_SAMPLERATE == CFG_PARA_SAMPLE_RATE
			AudioIOSet.Adapt = STD;//SRC_ONLY
	#else
			AudioIOSet.Adapt = SRC_ONLY;
	#endif
		}
		else//slave
		{
	#if CFG_PARA_I2S_SAMPLERATE == CFG_PARA_SAMPLE_RATE
			AudioIOSet.Adapt = STD;//SRA_ONLY;//CLK_ADJUST_ONLY;
	#else
			AudioIOSet.Adapt = SRC_ONLY;//SRC_SRA;//SRC_ADJUST;
	#endif
		}
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

#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;//不需要做位宽转换处理
#endif
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
		AudioIOSet.IOBitWidth = PCM_DATA_24BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 0;//DAC 24bit ,sink最后一级输出时需要转变为24bi
		AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].BitWidth = AudioIOSet.IOBitWidth;
		AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].BitWidthConvFlag = AudioIOSet.IOBitWidthConvFlag;
		AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].Sync = TRUE;		
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

#if	defined(CFG_CHIP_BP1532A2)
	AudioDAC_AllPowerOn(DAC_Diff,DAC_NOLoad);
#else
	AudioDAC_AllPowerOn(DAC_Single,DAC_NOLoad);
#endif
	return TRUE;
}

//释放公共通路，
void ModeCommonDeinit(void)
{
	SoftFlagRegister(SoftFlagAudioCoreSourceIsDeInit);

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

#if defined(CFG_RES_AUDIO_DACX_EN) && !defined(TWS_DACX_OUT)
	AudioCoreSinkDisable(AUDIO_DACX_SINK_NUM);
	AudioDAC_Disable(DAC1);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC1_TX, DMA_DONE_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC1_TX, DMA_THRESHOLD_INT);
	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_DAC1_TX, DMA_ERROR_INT);
	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_DAC1_TX);
//	AudioDAC_Reset(DAC1);
	if(mainAppCt.DACXFIFO != NULL)
	{
		osPortFree(mainAppCt.DACXFIFO);
		mainAppCt.DACXFIFO = NULL;
	}
	AudioCoreSinkDeinit(AUDIO_DACX_SINK_NUM);
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
#if defined(CFG_RES_AUDIO_I2SOUT_EN) && !defined(TWS_IIS0_OUT)&& !defined(TWS_IIS1_OUT)
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

//#endif

#ifdef CFG_FUNC_REMIND_SOUND_EN
	AudioCoreSourceDisable(REMIND_SOURCE_NUM);
	AudioCoreSourceDeinit(REMIND_SOURCE_NUM);
	RemindSoundAudioPlayEnd();
#ifdef CFG_REMIND_SOUND_DECODING_USE_LIBRARY
	RemindMp3DecoderDeinit();
#endif
#endif

	roboeffect_free(AudioCore.Roboeffect.context_memory);
	AudioCore.Roboeffect.context_memory = NULL;
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

	GPIO_PortAModeSet(1 << GET_I2S_GPIO_INDEX(I2S_MCLK_GPIO), GET_I2S_GPIO_MODE(I2S_MCLK_GPIO));//mclk
	GPIO_PortAModeSet(1 << GET_I2S_GPIO_INDEX(I2S_LRCLK_GPIO),GET_I2S_GPIO_MODE(I2S_LRCLK_GPIO));//lrclk
	GPIO_PortAModeSet(1 << GET_I2S_GPIO_INDEX(I2S_BCLK_GPIO), GET_I2S_GPIO_MODE(I2S_BCLK_GPIO));//bclk
	GPIO_PortAModeSet(1 << GET_I2S_GPIO_INDEX(I2S_DOUT_GPIO), GET_I2S_GPIO_MODE(I2S_DOUT_GPIO));//do

	I2S_AlignModeSet(CFG_RES_I2S_MODULE, I2S_LOW_BITS_ACTIVE);
	AudioI2S_Init(CFG_RES_I2S_MODULE, &i2s_set);//
}
#endif



bool AudioIoCommonForHfp(uint32_t sampleRate, uint16_t gain, uint8_t gainBoostSel)
{
	AudioCoreIO AudioIOSet;
	uint16_t SampleLen = AudioCoreFrameSizeGet(DefaultNet);
	uint16_t FifoLenStereo = SampleLen * 2 * 2 * 2;//立体声8倍大小于帧长，单位byte
//	uint16_t FifoLenMono = SampleLen * 2 * 2;//单声到4倍大小于帧长，单位byte
	uint8_t EffectMode = mainAppCt.EffectMode;

	mainAppCt.EffectMode = EFFECT_MODE_HFP_AEC;
	if(!RoboeffectInit())
	{
		DBG("!!!roboeffect init must be earlier than sink init!!!.\n");
	}
	mainAppCt.EffectMode = EffectMode;

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
		AudioADC_DigitalInit(ADC1_MODULE, sampleRate, (void*)mainAppCt.ADCFIFO, AudioCoreFrameSizeGet(DefaultNet) * 2 * 2 * 2,ADC_WIDTH_16BITS);

		//Soure0.
		memset(&AudioIOSet, 0, sizeof(AudioCoreIO));
		AudioIOSet.Adapt = STD;
		AudioIOSet.Sync = TRUE;
		AudioIOSet.Channels = 1;
		AudioIOSet.Net = DefaultNet;
		AudioIOSet.DataIOFunc = AudioADC1DataGet;
		AudioIOSet.LenGetFunc = AudioADC1DataLenGet;
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
		AudioADC_AnaInit(ADC1_MODULE,MIC_LEFT,Single);
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
		memset(mainAppCt.ADCFIFO, 0, AudioCoreFrameSizeGet(DefaultNet) * 2 * 2 * 2);
		AudioADC_DigitalInit(ADC1_MODULE, sampleRate, (void*)mainAppCt.ADCFIFO, AudioCoreFrameSizeGet(DefaultNet) * 2 * 2 * 2,ADC_WIDTH_16BITS);
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
		AudioIOSet.DataIOFunc = AudioDAC0DataSet;
		AudioIOSet.LenGetFunc = AudioDAC0DataSpaceLenGet;

#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_16BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 1;//DAC 24bit ,sink最后一级输出时需要转变为24bi
#endif
		if(!AudioCoreSinkInit(&AudioIOSet, AUDIO_DAC0_SINK_NUM))
		{
			DBG("Dac init error");
			return FALSE;
		}
		AudioDAC_Init(sampleRate,24, (void*)mainAppCt.DACFIFO, mainAppCt.DACFIFO_LEN, NULL, 0);
	}
	else
	{
		AudioDAC0_SampleRateChange(sampleRate);
		printf("mode task io set\n");
#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_16BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 1;//DAC 24bit ,sink最后一级输出时需要转变为24bi
		AudioCore.AudioSink[AUDIO_DAC0_SINK_NUM].BitWidth = AudioIOSet.IOBitWidth;
		AudioCore.AudioSink[AUDIO_DAC0_SINK_NUM].BitWidthConvFlag = AudioIOSet.IOBitWidthConvFlag;
#endif
	}
	AudioCoreSinkEnable(AUDIO_DAC0_SINK_NUM);
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	#if defined(TWS_IIS0_OUT) || defined(TWS_IIS1_OUT)
	AudioIOSet.Resident = TRUE;
	AudioIOSet.Depth = TWS_SINK_DEV_FIFO_SAMPLES;
	#else
	AudioIOSet.Depth = AudioCore.FrameSize[DefaultNet] * 2 ;
	#endif
	if(!AudioCoreSinkIsInit(AUDIO_I2SOUT_SINK_NUM))
	{
	#if !defined(TWS_IIS0_OUT) && !defined(TWS_IIS1_OUT)
		mainAppCt.I2SFIFO_LEN = AudioIOSet.Depth * sizeof(PCM_DATA_TYPE) * 2;
		mainAppCt.I2SFIFO = (uint32_t*)osPortMalloc(mainAppCt.I2SFIFO_LEN);//I2S fifo
	#endif

		if(mainAppCt.I2SFIFO != NULL)
		{
			memset(mainAppCt.I2SFIFO, 0, mainAppCt.I2SFIFO_LEN);
		}
		else
		{
			APP_DBG("malloc I2SFIFO error\n");
			return FALSE;
		}

		if((CFG_RES_I2S_MODE == I2S_MASTER_MODE) || !defined(CFG_FUNC_I2S_IN_SYNC_EN))
		{// Master 或不开微调
	#if CFG_PARA_I2S_SAMPLERATE == CFG_BTHF_PARA_SAMPLE_RATE
			AudioIOSet.Adapt = STD;//SRC_ONLY
	#else
			AudioIOSet.Adapt = SRC_ONLY;
	#endif
		}
		else//slave
		{
	#if CFG_PARA_I2S_SAMPLERATE == CFG_BTHF_PARA_SAMPLE_RATE
			AudioIOSet.Adapt = SRA_ONLY;//CLK_ADJUST_ONLY;
	#else
			AudioIOSet.Adapt = SRC_SRA;//SRC_ADJUST;
	#endif
		}
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
#ifdef	CFG_AUDIO_WIDTH_24BIT
		AudioIOSet.IOBitWidth = PCM_DATA_16BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 1;//不需要做位宽转换处理
#endif
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
		AudioIOSet.IOBitWidth = PCM_DATA_16BIT_WIDTH;//0,16bit,1:24bit
		AudioIOSet.IOBitWidthConvFlag = 1;//DAC 24bit ,sink最后一级输出时需要转变为24bi
		AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].BitWidth = AudioIOSet.IOBitWidth;
		AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].BitWidthConvFlag = AudioIOSet.IOBitWidthConvFlag;
		AudioCore.AudioSink[AUDIO_I2SOUT_SINK_NUM].Sync = FALSE;
	#endif
	}
#endif

#if	defined(CFG_CHIP_BP1532A2)
	AudioDAC_AllPowerOn(DAC_Diff,DAC_NOLoad);
#else
	AudioDAC_AllPowerOn(DAC_Single,DAC_NOLoad);
#endif
	return TRUE;
}



void tws_device_open_isr(void)
{

}
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
			AudioCore.AudioSource[i].PcmInBuf = roboeffect_get_source_buffer(
							AudioCore.Roboeffect.context_memory, AudioCoreSourceToRoboeffect(i));
		}
		for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
		{
			AudioCore.AudioSink[i].PcmOutBuf = roboeffect_get_sink_buffer(
							AudioCore.Roboeffect.context_memory, AudioCoreSinkToRoboeffect(i));
		}

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
			Roboeffect_ReverbStep_Ajust(effect_addr[AudioCore.Roboeffect.flow_chart_mode].REVERB_ADDR,
										effect_addr[AudioCore.Roboeffect.flow_chart_mode].ECHO_ADDR, mainAppCt.ReverbStep);
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
			Roboeffect_ReverbStep_Ajust(effect_addr[AudioCore.Roboeffect.flow_chart_mode].REVERB_ADDR,
										effect_addr[AudioCore.Roboeffect.flow_chart_mode].ECHO_ADDR, mainAppCt.ReverbStep);
			APP_DBG("MSG_MIC_EFFECT_UP\n");
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
			Roboeffect_EQ_Ajust(effect_addr[AudioCore.Roboeffect.flow_chart_mode].MIC_EQ_ADDR, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
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
			Roboeffect_EQ_Ajust(effect_addr[AudioCore.Roboeffect.flow_chart_mode].MIC_EQ_ADDR, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
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
			Roboeffect_EQ_Ajust(effect_addr[AudioCore.Roboeffect.flow_chart_mode].MIC_EQ_ADDR, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
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
			Roboeffect_EQ_Ajust(effect_addr[AudioCore.Roboeffect.flow_chart_mode].MIC_EQ_ADDR, mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
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
			Roboeffect_EQ_Ajust(effect_addr[AudioCore.Roboeffect.flow_chart_mode].MUSIC_EQ_ADDR, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);

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
			Roboeffect_EQ_Ajust(effect_addr[AudioCore.Roboeffect.flow_chart_mode].MUSIC_EQ_ADDR, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
			
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
			Roboeffect_EQ_Ajust(effect_addr[AudioCore.Roboeffect.flow_chart_mode].MUSIC_EQ_ADDR, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);

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
			Roboeffect_EQ_Ajust(effect_addr[AudioCore.Roboeffect.flow_chart_mode].MUSIC_EQ_ADDR, mainAppCt.MusicBassStep, mainAppCt.MusicTrebStep);
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
//			APP_DBG("MSG_VOCAL_CUT\n");
//			gCtrlVars.vocal_cut_unit.vocal_cut_en = !gCtrlVars.vocal_cut_unit.vocal_cut_en;
//			#ifdef CFG_FUNC_DISPLAY_EN
//            msgSend.msgId = MSG_DISPLAY_SERVICE_VOCAL_CUT;
//            MessageSend(GetSysModeMsgHandle(), &msgSend);
//			#endif
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
#ifdef CFG_FUNC_MIC_KARAOKE_EN
		case MSG_KARAOKEMODE:
			APP_DBG("MSG_EFFECTMODE\n");
			if(mainAppCt.SysCurrentMode == ModeBtHfPlay)//蓝牙通话模式下，不支持音效模式切换
			{
				break;
			}

			if(IsAudioPlayerMute() == FALSE)
			{
				HardWareMuteOrUnMute();
			}

			if((mainAppCt.EffectMode >= EFFECT_MODE_WaWaYin)|| (mainAppCt.EffectMode < EFFECT_MODE_HunXiang))
			{
				mainAppCt.EffectMode = EFFECT_MODE_HunXiang;
			}
			else
			{
				mainAppCt.EffectMode++;
			}
			APP_DBG("EffectMode = %d\n", mainAppCt.EffectMode);

			AudioEffectModeSel(mainAppCt.EffectMode, 2);//sel: 0=init hw, 1=effect, 2=hw + effect

			if(IsAudioPlayerMute() == TRUE)
			{
				HardWareMuteOrUnMute();
			}
			gCtrlVars.AutoRefresh = 1;
//			switch(mainAppCt.EffectMode)
//			{
//				case EFFECT_MODE_HunXiang:
//					#ifdef CFG_FUNC_REMIND_SOUND_EN
//					RemindSoundServiceItemRequest(SOUND_REMIND_LIUXINGH, REMIND_PRIO_NORMAL);
//					#endif
//					break;
//				case EFFECT_MODE_DianYin:
//					#ifdef CFG_FUNC_REMIND_SOUND_EN
//					RemindSoundServiceItemRequest(SOUND_REMIND_DIANYIN, REMIND_PRIO_NORMAL);
//					#endif
//					break;
//				case EFFECT_MODE_MoYin:
//					#ifdef CFG_FUNC_REMIND_SOUND_EN
//					RemindSoundServiceItemRequest(SOUND_REMIND_MOYIN, REMIND_PRIO_NORMAL);
//					#endif
//					break;
//				case EFFECT_MODE_HanMai:
//					#ifdef CFG_FUNC_REMIND_SOUND_EN
//					RemindSoundServiceItemRequest(SOUND_REMIND_HANMAI, REMIND_PRIO_NORMAL);
//					#endif
//					break;
//				case EFFECT_MODE_NanBianNv:
//					#ifdef CFG_FUNC_REMIND_SOUND_EN
//					RemindSoundServiceItemRequest(SOUND_REMIND_NVSHEN, REMIND_PRIO_NORMAL);
//					#endif
//					break;
//				case EFFECT_MODE_NvBianNan:
//					#ifdef CFG_FUNC_REMIND_SOUND_EN
//					RemindSoundServiceItemRequest(SOUND_REMIND_NANSHEN, REMIND_PRIO_NORMAL);
//					#endif
//					break;
//				case EFFECT_MODE_WaWaYin:
//					#ifdef CFG_FUNC_REMIND_SOUND_EN
//					RemindSoundServiceItemRequest(SOUND_REMIND_WAWAYIN, REMIND_PRIO_NORMAL);
//					#endif
//					break;
//			}
			break;
#endif
		case MSG_EFFECTMODE:
			if(GetSystemMode() == ModeBtHfPlay)//蓝牙通话模式下，不支持音效模式切换
			{
				break;
			}

			if(IsAudioPlayerMute() == FALSE)
			{
				HardWareMuteOrUnMute();
			}

			if((mainAppCt.EffectMode >= EFFECT_MODE_MUSIC) || (mainAppCt.EffectMode < EFFECT_MODE_MIC))
			{
				mainAppCt.EffectMode = EFFECT_MODE_MIC;
			}
			else
			{
				mainAppCt.EffectMode++;
			}
			APP_DBG("EffectMode = %d\n", mainAppCt.EffectMode);

			AudioEffectModeSel(mainAppCt.EffectMode, 2);//sel: 0=init hw, 1=effect, 2=hw + effect

			if(IsAudioPlayerMute() == TRUE)
			{
				HardWareMuteOrUnMute();
			}
			gCtrlVars.AutoRefresh = 1;
#endif
			break;
		case MSG_EFFECTREINIT:
			APP_DBG("MSG_EFFECTREINIT\n");
			if(IsAudioPlayerMute() == FALSE)
			{
				HardWareMuteOrUnMute();
			}
			int8_t opera = AudioCore.Roboeffect.effect_enable ? 1 : -1;
//			APP_DBG("Roboeffect FrameSize %d, %d\n", opera, roboeffect_get_suit_frame_size(
//					AudioCore.Roboeffect.context_memory, CFG_PARA_SAMPLES_PER_FRAME, AudioCore.Roboeffect.effect_addr, opera));
			if(roboeffect_get_suit_frame_size(
					AudioCore.Roboeffect.context_memory, CFG_PARA_SAMPLES_PER_FRAME, AudioCore.Roboeffect.effect_addr, opera)
				&& AudioCoreFrameSizeGet(DefaultNet) != roboeffect_get_suit_frame_size(
					AudioCore.Roboeffect.context_memory, CFG_PARA_SAMPLES_PER_FRAME, AudioCore.Roboeffect.effect_addr, opera))
			{
				MessageContext	msg;
				MessageRecv(MessageRegister(20), &msg, 1);

				AudioCore.Roboeffect.user_effect_list->frame_size = roboeffect_get_suit_frame_size(
						AudioCore.Roboeffect.context_memory, CFG_PARA_SAMPLES_PER_FRAME, AudioCore.Roboeffect.effect_addr, opera);
				APP_DBG("Need Change FrameSize to %ld\n", AudioCore.Roboeffect.user_effect_list->frame_size);

				SysMode[GetModeIndexInModeLoop(&mainAppCt.SysCurrentMode)].SysModeDeInit();
				AudioCoreFrameSizeSet(DefaultNet, AudioCore.Roboeffect.user_effect_list->frame_size);
				SysMode[GetModeIndexInModeLoop(&mainAppCt.SysCurrentMode)].SysModeInit();
				SysMode[GetModeIndexInModeLoop(&mainAppCt.SysCurrentMode)].SysModeRun(msg.msgId);

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
			gCtrlVars.AutoRefresh = AudioCore.Roboeffect.effect_addr;
			break;
		case MSG_REC_MUSIC:
			SetRecMusic(0);
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
