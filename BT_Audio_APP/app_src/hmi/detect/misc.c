/*
 **************************************************************************************
 * @file    misc.c
 * @brief    
 * 
 * @author  
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdint.h>
#include <string.h>
#include "main_task.h"
#include "gpio.h"
#include "irqn.h"
#include "ctrlvars.h"
#include "audio_vol.h"
#include "misc.h"
#include "i2s.h"
#include "user_effect_parameter.h"

#define HW_DEVICE_NUMBER		3
#define  HW_DECTED_DLY	 2

#if CFG_REMINDSOUND_EN
//#include "sound_remind.h"
#endif
void MicVolSmoothProcess(void);
#if (defined(CFG_FUNC_DETECT_PHONE_EN))
static TIMER MuteOffTimer;
#endif

static TIMER HWDeviceTimer;
static uint8_t HWDeviceInit=0;
static uint8_t HWDeviceConut;

/*
****************************************************************
*常用应用处理函数列表，用户可在此扩展应用
*
*
****************************************************************
*/
void (*HWdeviceList[HW_DEVICE_NUMBER])(void)={
     //LedDisplay,
     DetectEarPhone,
     DetectMic3Or4Line,
     //DetectLineIn,
     //PowerMonitor,
     //PowerProcess,
     //DACVolumeSmoothProcess,
     MicVolSmoothProcess, 
};
/*
****************************************************************
*
*
*
****************************************************************
*/
void HWDeviceDected_Init(void)
{
	DetectEarPhone();
		
	DetectMic3Or4Line();
	
	#if CFG_DMA_GPIO_SIMULATE_PWM_EN
	DmaTimerCircleMode_GPIO_Init();
	DmaTimerCircleMode_GPIO_Config(50, 70, 90);
	#endif

	#if CFG_HW_PWM_EN
	HwPwmInit();
	HwPwmConfig(50);
	#endif
}

#ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
void SilenceDectorProcess(void)
{
	extern int16_t * AudioEffectGetAllParameter(AUDIOEFFECT_EFFECT_CONTROL effect);
	//根据实际情况进行修改和配置（MIC + MUSIC）
	int32_t mic_level = -1;
	int32_t music_level = -1;
	param_silence_detector *Param = NULL;

	Param = AudioEffectGetAllParameter(MUSIC_SILENCE_DETECTOR_PARAM);
	if(Param)
		music_level = Param->pcm_amplitude;

	Param = AudioEffectGetAllParameter(MIC_SILENCE_DETECTOR_PARAM);
	if(Param)
		mic_level = Param->pcm_amplitude;

	if(mic_level < 0)	//没有检测到MIC通道信号，不做关机处理
		mic_level = SILENCE_THRESHOLD + 1;
	if(music_level < 0) //没有检测到MUSIC通道信号，不做关机处理
		music_level = SILENCE_THRESHOLD + 1;

	if(	mic_level > SILENCE_THRESHOLD   &&		//MIC通道信号
		music_level > SILENCE_THRESHOLD			//MUSIC通道信号
	  )
		mainAppCt.Silence_Power_Off_Time = 0;
}
#endif
/*
****************************************************************
*常用应用处理主循环
*
*
****************************************************************
*/
void HWDeviceDected(void)
{
   ///////此函数仅调用对时间不作要求的功能////要求偶数次定时器//
    
	if(HWDeviceInit==0)
	{
		HWDeviceInit = 1;

		HWDeviceConut = 0;

		TimeOutSet(&HWDeviceTimer, 0); 

		return ;
	}
#ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
	SilenceDectorProcess();
#endif
#ifdef CFG_FUNC_SHUNNING_EN
	ShunningModeProcess();//闪避功能处理
#endif

	if(!IsTimeOut(&HWDeviceTimer)) return;

    TimeOutSet(&HWDeviceTimer, HW_DECTED_DLY); 
	
    HWDeviceConut++;

    //HWDeviceConut &= 0x8;/////8 *2ms = 16ms
    if(HWDeviceConut > (HW_DEVICE_NUMBER-1))
	{
		HWDeviceConut = 0;
	}
    (*HWdeviceList[HWDeviceConut])();
}

#ifdef CFG_FUNC_SHUNNING_EN
/*************************************************
*    闪避处理函数
*
*
***************************************************/
void ShunningModeProcess(void)
{
	////20ms调用一次此函数//增益值(dyn_gain)闪避功能专用///0~4096 分16级，总为0~16 256为一级
	static uint16_t shnning_recover_dly = 0;
	static uint16_t shnning_up_dly      = 0;
	static uint16_t shnning_down_dly    = 0;
	uint16_t music_pregain = mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM];

	extern int16_t * AudioEffectGetAllParameter(AUDIOEFFECT_EFFECT_CONTROL effect);
	int32_t mic_level = -1;
	param_silence_detector *Param = NULL;

	if(mainAppCt.ShunningMode == 0)
	{
	    if(shnning_up_dly)
		{
			shnning_up_dly--;
			return;
		}
		shnning_up_dly = SHNNIN_UP_DLY;
		if(mainAppCt.aux_out_dyn_gain != music_pregain)//////阀值
		{
			if(mainAppCt.aux_out_dyn_gain < music_pregain)
			{
				mainAppCt.aux_out_dyn_gain += SHNNIN_STEP;
			}

			if(mainAppCt.aux_out_dyn_gain > music_pregain)
			{
				mainAppCt.aux_out_dyn_gain = music_pregain;
			}
		}

		AudioEffect_SourceGain_Update(APP_SOURCE_NUM, mainAppCt.aux_out_dyn_gain);
		return;
	}

	Param = AudioEffectGetAllParameter(MIC_SILENCE_DETECTOR_PARAM);
	if(Param)
		mic_level = Param->pcm_amplitude;

	if(mic_level > SHNNIN_VALID_DATA)///vol----
	{
		shnning_recover_dly = SHNNIN_VOL_RECOVER_TIME;
		if(shnning_down_dly)
		{
			shnning_down_dly--;
			return;
		}
		shnning_down_dly = SHNNIN_DOWN_DLY;

		if((music_pregain - mainAppCt.aux_out_dyn_gain) <= SHNNIN_THRESHOLD && (mainAppCt.aux_out_dyn_gain > 0))
		{
			mainAppCt.aux_out_dyn_gain -= SHNNIN_STEP;
			APP_DBG("Aux Shunning start\n");
		}

		AudioEffect_SourceGain_Update(APP_SOURCE_NUM, mainAppCt.aux_out_dyn_gain);
	}
	else
	{
		if(shnning_up_dly)
		{
			shnning_up_dly--;
			return;
		}
		if(shnning_recover_dly)
		{
			shnning_recover_dly--;
			return;
		}
		shnning_up_dly = SHNNIN_UP_DLY;

		if(mainAppCt.aux_out_dyn_gain != music_pregain)//////阀值
		{
			if(mainAppCt.aux_out_dyn_gain < music_pregain)
			{
				mainAppCt.aux_out_dyn_gain += SHNNIN_STEP;
			}

			if(mainAppCt.aux_out_dyn_gain > music_pregain)
			{
				mainAppCt.aux_out_dyn_gain = music_pregain;
			}
		}

		AudioEffect_SourceGain_Update(APP_SOURCE_NUM, mainAppCt.aux_out_dyn_gain);
	}
}
#endif
/*
****************************************************************
* 耳机插拔检测处理demo
*
*
****************************************************************
*/
void DetectEarPhone(void)
{
#ifdef CFG_FUNC_DETECT_PHONE_EN
	static uint8_t PhoneTimeInit=0;
	static uint8_t PhoneCnt = 0;
//	uint32_t msg;

	if(!PhoneTimeInit)
	{
		PhoneTimeInit = 1;
		PhoneCnt = 0;
		gCtrlVars.EarPhoneOnlin = 0;
		GPIO_RegOneBitClear(PHONE_DETECT_OE, PHONE_DETECT_PIN);
		GPIO_RegOneBitSet(PHONE_DETECT_IE, PHONE_DETECT_PIN);
		///PULL enable
		GPIO_RegOneBitSet(PHONE_DETECT_PU, PHONE_DETECT_PIN);
		GPIO_RegOneBitClear(PHONE_DETECT_PD, PHONE_DETECT_PIN);
		DelayUs(50);
		if(!GPIO_RegOneBitGet(PHONE_DETECT_IN,PHONE_DETECT_PIN))
		//if(GPIO_RegOneBitGet(PHONE_DETECT_IN,PHONE_DETECT_PIN))
		{
			gCtrlVars.EarPhoneOnlin = 1;			
			TimeOutSet(&MuteOffTimer, 1000); 
			APP_DBG("Ear Phone In\n");
		}
	}
    else
	{
		if(!GPIO_RegOneBitGet(PHONE_DETECT_IN,PHONE_DETECT_PIN))
		//if(GPIO_RegOneBitGet(PHONE_DETECT_IN,PHONE_DETECT_PIN))
		{
			if(++PhoneCnt > 50)//消抖处理
			{
				PhoneCnt = 0;
				if(gCtrlVars.EarPhoneOnlin == 0)
				{
					gCtrlVars.EarPhoneOnlin = 1;	
					TimeOutSet(&MuteOffTimer, 1000); 
					//msg = FUNC_ID_EARPHONE_IN;
					//SendMessage(&msg,NULL);
					APP_DBG("Ear Phone In\n");
				}		
			}
		}
		else
		{
			PhoneCnt = 0;
			if(gCtrlVars.EarPhoneOnlin)
			{
				//msg =	FUNC_ID_EARPHONE_OUT;
				gCtrlVars.EarPhoneOnlin = 0;
				MUTE_ON();
				APP_DBG("Ear Phone Out\n");
				TimeOutSet(&MuteOffTimer, 1000); 
			}
		}
		if(IsTimeOut(&MuteOffTimer)) 
		{
			if(gCtrlVars.EarPhoneOnlin)
			{
				MUTE_OFF();
			}
		}
	}
#endif	
}

/*
****************************************************************
* 3线，4线耳机类型检测处理demo
*
*
****************************************************************
*/
void DetectMic3Or4Line(void)
{
#ifdef CFG_FUNC_DETECT_MIC_SEG_EN
    //---------mic1 var-------------------//
	static uint8_t MicSegmentInit=0;
	static uint8_t MicSegCnt = 0;
//	uint8_t val;
//	uint32_t msg;

	if(!MicSegmentInit)
	{
		MicSegmentInit = 1;
		MicSegCnt = 0;
		gCtrlVars.MicSegment = 0;////default 0 line
		GPIO_RegOneBitClear(MIC_SEGMENT_PU, MIC_SEGMENT_PIN);
		GPIO_RegOneBitSet(MIC_SEGMENT_PD, MIC_SEGMENT_PIN);			   
		GPIO_RegOneBitClear(MIC_SEGMENT_OE, MIC_SEGMENT_PIN);
		GPIO_RegOneBitSet(MIC_SEGMENT_IE, MIC_SEGMENT_PIN);
		DelayUs(50);
		if(GPIO_RegOneBitGet(MIC_SEGMENT_IN,MIC_SEGMENT_PIN))
		{
			gCtrlVars.MicSegment = 4;
			APP_DBG("Mic segmen is 4 line\n");
		}	   
	}
	else
	{
		/*if(!gCtrlVars.MicOnlin)
	    {
			MIC_MUTE_ON();
		}
		else*/
		{
			if(gCtrlVars.MicSegment == 4 )
			{
				MIC_MUTE_OFF();
			}
			else
			{
				MIC_MUTE_ON();
			}
		}
		if(GPIO_RegOneBitGet(MIC_SEGMENT_IN,MIC_SEGMENT_PIN))
		{
			MicSegCnt = 0;
			if(gCtrlVars.MicSegment != 4  )
			{
				gCtrlVars.MicSegment = 4;
				//msg =	FUNC_ID_MIC_3Or4_LINE;
				APP_DBG("Mic segmen is 4 line\n");
			}
		}
		else
		{
			if(++MicSegCnt > 50)
			{
				MicSegCnt = 0;
				if(gCtrlVars.MicSegment != 3 )//////H=4line l=3line
				{
					gCtrlVars.MicSegment = 3;	 
					//msg = FUNC_ID_MIC_3Or4_LINE;
					//SendMessage(&msg,NULL);
					APP_DBG("Mic segmen is 3 line\n");
				}	 
			}
		}
	}
#endif		
}

 /*
 ****************************************************************
 *电位器调节中的参数值的平滑处理
 *提供的电位器demo的调节步长是:0~32
 *
 ****************************************************************
 */
void MicVolSmoothProcess(void)
{
#ifdef CFG_ADC_LEVEL_KEY_EN//def CFG_ADC_LEVEL_KEY_EN
	//---------mic vol 电位器渐变调节-------------------//
	if(MicVolume!= mainAppCt.MicVolumeBak)
	{
		if(MicVolume > mainAppCt.MicVolumeBak)
		{
			MicVolume--;
		}
		else if(MicVolume < mainAppCt.MicVolumeBak)
		{
			MicVolume++;
		}
		mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] = MicVolume;
		AudioEffect_SourceGain_Update(MIC_SOURCE_NUM, mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]);
		APP_DBG("MicVolume = %d\n",MicVolume);
	}
//    //-------------bass 电位器渐变调节-----------------------------------------//
//	if(mainAppCt.MicBassStep !=  mainAppCt.MicBassStepBak)
//	{
//		if(mainAppCt.MicBassStep >  mainAppCt.MicBassStepBak)
//		{
//			mainAppCt.MicBassStep -= BASS_TREB_GAIN_STEP;
//
//		}
//		else if(mainAppCt.MicBassStep <  mainAppCt.MicBassStepBak)
//		{
//			mainAppCt.MicBassStep += BASS_TREB_GAIN_STEP;
//
//		}
//		MicBassTrebAjust(mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
//		APP_DBG("bass = %d\n",mainAppCt.MicBassStep);
//	}
//    //-------------treb 电位器渐变调节-----------------------------------------//
//	if(mainAppCt.MicTrebStep !=  mainAppCt.MicTrebStepBak)
//	{
//		if(mainAppCt.MicTrebStep >  mainAppCt.MicTrebStepBak)
//		{
//			mainAppCt.MicTrebStep -= BASS_TREB_GAIN_STEP;
//
//		}
//		else if(mainAppCt.MicTrebStep <  mainAppCt.MicTrebStepBak)
//		{
//			mainAppCt.MicTrebStep += BASS_TREB_GAIN_STEP;
//
//		}
//		MicBassTrebAjust(mainAppCt.MicBassStep, mainAppCt.MicTrebStep);
//		APP_DBG("treb = %d\n",mainAppCt.MicTrebStep);
//	}
//	//-------------reverb-gain 电位器渐变调节---------------------------------------//
//	if(mainAppCt.MicEffectDelayStep !=  mainAppCt.ReverbStepBak)
//	{
//		if(mainAppCt.MicEffectDelayStep >  mainAppCt.ReverbStepBak)
//		{
//			mainAppCt.MicEffectDelayStep -= 1;
//
//		}
//		else if(mainAppCt.MicEffectDelayStep <  mainAppCt.ReverbStepBak)
//		{
//			mainAppCt.MicEffectDelayStep += 1;
//
//		}
//		APP_DBG("MicEffectDelayStep  = %d\n",mainAppCt.MicEffectDelayStep);
//	}
#endif	  
}

