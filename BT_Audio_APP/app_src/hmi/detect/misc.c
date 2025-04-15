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
/*************************************************
*    闪避处理函数
*
*
***************************************************/
void ShunningModeProcess(void)
{
	////20ms调用一次此函数//增益值(dyn_gain)闪避功能专用///0~4096 分16级，总为0~16 256为一级
#ifdef CFG_FUNC_SHUNNING_EN
	static uint16_t shnning_recover_dly = 0;
	static uint16_t shnning_up_dly      = 0;
	static uint16_t shnning_down_dly    = 0;
	static uint16_t cnt = 0;
	uint16_t music_pregain = Roboeffect_GainControl_Get(MUSIC_PREGAIN_ADDR);

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
			else if(mainAppCt.aux_out_dyn_gain > music_pregain)
			{
			    if(mainAppCt.aux_out_dyn_gain >= SHNNIN_STEP)
		    	{
			    	mainAppCt.aux_out_dyn_gain -= SHNNIN_STEP;
		    	}
			}
			if(mainAppCt.aux_out_dyn_gain > music_pregain)
			{
				mainAppCt.aux_out_dyn_gain = music_pregain;
			}
		}
//		#ifdef CFG_FUNC_REMIND_SOUND_EN
//		if(mainAppCt.remind_out_dyn_gain !=  music_pregain)//////阀值
//		{
//			if(mainAppCt.remind_out_dyn_gain < music_pregain)
//			{
//				mainAppCt.remind_out_dyn_gain += SHNNIN_STEP;
//			}
//			else if(mainAppCt.remind_out_dyn_gain > music_pregain)
//			{
//			    if(mainAppCt.remind_out_dyn_gain >= SHNNIN_STEP)
//		    	{
//			    	mainAppCt.remind_out_dyn_gain -= SHNNIN_STEP;
//		    	}
//			}
//			if(mainAppCt.remind_out_dyn_gain > music_pregain)
//			{
//				mainAppCt.remind_out_dyn_gain = music_pregain;
//			}
//		}
//		#endif
		return;
	}
    cnt++;
	cnt &= 0x07;
	//if(cnt==0) APP_DBG("Sdct level:%ld\n",gCtrlVars.AudioSdct_unit.level);

	if(Roboeffect_SilenceDetector_Get(SILENCE_DETECTOR_ADDR) > SHNNIN_VALID_DATA)///vol----
	{
		shnning_recover_dly = SHNNIN_VOL_RECOVER_TIME;
        if(shnning_down_dly)
		{
			shnning_down_dly--;
			return;
		}
        shnning_down_dly = SHNNIN_DOWN_DLY;

		if(music_pregain > SHNNIN_THRESHOLD)
		{
			if(mainAppCt.aux_out_dyn_gain > SHNNIN_THRESHOLD)//////阀值
			{
				if(mainAppCt.aux_out_dyn_gain >= SHNNIN_STEP)
				{
					mainAppCt.aux_out_dyn_gain -= SHNNIN_STEP;
				}
				APP_DBG("Aux Shunning start\n");
			}
		}
		else
		{
			mainAppCt.aux_out_dyn_gain = music_pregain;
		}
//		#ifdef CFG_FUNC_REMIND_SOUND_EN
//		if(music_pregain > SHNNIN_THRESHOLD)
//		{
//			if(mainAppCt.remind_out_dyn_gain > SHNNIN_THRESHOLD)//////阀值
//			{
//				if(mainAppCt.remind_out_dyn_gain >= SHNNIN_STEP)
//		    	{
//					mainAppCt.remind_out_dyn_gain -= SHNNIN_STEP;
//		    	}
//
//				APP_DBG("Remind Shunning start\n");
//			}
//		}
//		else
//		{
//			mainAppCt.remind_out_dyn_gain = music_pregain;
//		}
//		#endif
	}
	else/////vol+++
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

		if(mainAppCt.aux_out_dyn_gain !=  music_pregain)//////阀值
		{
			if(mainAppCt.aux_out_dyn_gain < music_pregain)
			{
				mainAppCt.aux_out_dyn_gain += SHNNIN_STEP;
			}
			else if(mainAppCt.aux_out_dyn_gain > music_pregain)
			{
				if(mainAppCt.aux_out_dyn_gain >= SHNNIN_STEP)
				{
					mainAppCt.aux_out_dyn_gain -= SHNNIN_STEP;
				}
			}
			if(mainAppCt.aux_out_dyn_gain > music_pregain)
			{
				mainAppCt.aux_out_dyn_gain  = music_pregain;
			}
		}
//		#ifdef CFG_FUNC_REMIND_SOUND_EN
//		if(mainAppCt.remind_out_dyn_gain !=  music_pregain)//////阀值
//		{
//			if(mainAppCt.remind_out_dyn_gain < music_pregain)
//			{
//				mainAppCt.remind_out_dyn_gain += SHNNIN_STEP;
//			}
//			else if(mainAppCt.remind_out_dyn_gain > music_pregain)
//			{
//				if(mainAppCt.remind_out_dyn_gain >= SHNNIN_STEP)
//		    	{
//					mainAppCt.remind_out_dyn_gain -= SHNNIN_STEP;
//		    	}
//			}
//			if(mainAppCt.remind_out_dyn_gain > music_pregain)
//			{
//				mainAppCt.remind_out_dyn_gain  = music_pregain;
//			}
//		}
//		#endif
	}
#endif
}

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
	if(mainAppCt.MicVolume!= mainAppCt.MicVolumeBak)
	{
		if(mainAppCt.MicVolume > mainAppCt.MicVolumeBak)
		{
			mainAppCt.MicVolume--;
		}
		else if(mainAppCt.MicVolume < mainAppCt.MicVolumeBak)
		{
			mainAppCt.MicVolume++;
		}
		mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] = mainAppCt.MicVolume;
		AudioCoreSourceVolSet(MIC_SOURCE_NUM, gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]], gSysVolArr[mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM]]);
		APP_DBG("MicVolume = %d\n",mainAppCt.MicVolume);
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

