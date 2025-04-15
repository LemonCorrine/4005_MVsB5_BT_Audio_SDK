/**
 **************************************************************************************
 * @file    main_task.c
 * @brief   Program Entry 
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <string.h>
#include "main_task.h"
#include "dma.h"
#include "timer.h"
#include "irqn.h"
#include "watchdog.h"
//#include "deepsleep.h"
#include "breakpoint.h"
#include "remind_sound.h"
#include "audio_vol.h"
#include "device_detect.h"
#include "otg_detect.h"
#include "sadc_interface.h"
//services
#include "shell.h"
#include "audio_core_service.h"
#include "bt_stack_service.h"
#include "bb_api.h"
#include "bt_config.h"
#include "bt_app_ddb_info.h"
#include "bt_hf_mode.h"
#include "idle_mode.h"
#include "bt_app_connect.h"
#include <components/soft_watchdog/soft_watch_dog.h>

MainAppContext	mainAppCt;
#ifdef CFG_RES_IR_NUMBERKEY
bool Number_select_flag = 0;
uint16_t Number_value = 0;
TIMER Number_selectTimer;
#endif
extern void CtrlVarsInit(void);
extern void report_up_grate(void);
extern volatile uint32_t gInsertEventDelayActTimer;
extern void BtTwsDisconnectApi(void);
#if FLASH_BOOT_EN
void start_up_grate(uint32_t UpdateResource);
#endif
extern void PowerOnModeGenerate(void *BpSysInfo);
#ifdef CFG_APP_HDMIIN_MODE_EN
extern void HDMI_CEC_ActivePowerOff(uint32_t timeout_value);
extern void	HDMI_CEC_DDC_Init(void);
#endif

#define MAIN_APP_TASK_STACK_SIZE		640//512//1024
#ifdef	CFG_FUNC_OPEN_SLOW_DEVICE_TASK
#define MAIN_APP_TASK_PRIO				4
#define MAIN_APP_MSG_TIMEOUT			10	
#else
#define MAIN_APP_TASK_PRIO				3
#define MAIN_APP_MSG_TIMEOUT			1	
#endif
#define MAIN_APP_TASK_SLEEP_PRIO		6 //进入deepsleep 需要相对其他task最高优先级。
#define MAIN_NUM_MESSAGE_QUEUE			20
#define SHELL_TASK_STACK_SIZE			512//1024
#define SHELL_TASK_PRIO					2


/**根据appconfig缺省配置:DMA 8个通道配置**/
/*1、cec需PERIPHERAL_ID_TIMER3*/
/*2、SD卡录音需PERIPHERAL_ID_SDIO RX/TX*/
/*3、在线串口调音需PERIPHERAL_ID_UART1 RX/TX,建议使用USB HID，节省DMA资源*/
/*4、线路输入需PERIPHERAL_ID_AUDIO_ADC0_RX*/
/*5、Mic开启需PERIPHERAL_ID_AUDIO_ADC1_RX，mode之间通道必须一致*/
/*6、Dac0开启需PERIPHERAL_ID_AUDIO_DAC0_TX mode之间通道必须一致*/
/*7、DacX需开启PERIPHERAL_ID_AUDIO_DAC1_TX mode之间通道必须一致*/
/*注意DMA 8个通道配置冲突:*/
/*a、默认在线调音使用USB HID*/

static const uint8_t DmaChannelMap[6] = {
	PERIPHERAL_ID_AUDIO_ADC0_RX,
	PERIPHERAL_ID_AUDIO_ADC1_RX,
	PERIPHERAL_ID_AUDIO_DAC0_TX,
//#ifdef CFG_DUMP_DEBUG_EN
//	CFG_DUMP_UART_TX_DMA_CHANNEL,
//#else
//	255,
//#endif
#ifdef CFG_RES_AUDIO_I2S_MIX_IN_EN
	PERIPHERAL_ID_I2S0_RX + 2 * CFG_RES_MIX_I2S_MODULE,
#else
	PERIPHERAL_ID_SDIO_RX,
#endif
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
};

static void MainAppInit(void)
{
	memset(&mainAppCt, 0, sizeof(MainAppContext));
#if FLASH_BOOT_EN
    report_up_grate();
#endif
	mainAppCt.msgHandle = MessageRegister(MAIN_NUM_MESSAGE_QUEUE);
	mainAppCt.state = TaskStateCreating;
	mainAppCt.SysCurrentMode = ModeIdle;
	mainAppCt.SysPrevMode = ModeIdle;
}

static void SysVarInit(void)
{
	int16_t i;

#ifdef CFG_FUNC_BREAKPOINT_EN
	BP_SYS_INFO *pBpSysInfo = NULL;

	pBpSysInfo = (BP_SYS_INFO *)BP_GetInfo(BP_SYS_INFO_TYPE);

#ifdef  VD51_REDMINE_13199
	SetRgbLedMode(pBpSysInfo->rgb_mode);
#endif

	mainAppCt.EffectMode = pBpSysInfo->EffectMode;
	APP_DBG("EffectMode:%d,%d\n", mainAppCt.EffectMode, pBpSysInfo->EffectMode);

	mainAppCt.MusicVolume = pBpSysInfo->MusicVolume;
	if((mainAppCt.MusicVolume > CFG_PARA_MAX_VOLUME_NUM) || (mainAppCt.MusicVolume <= 0))
	{
		mainAppCt.MusicVolume = CFG_PARA_MAX_VOLUME_NUM;
	}
	APP_DBG("MusicVolume:%d,%d\n", mainAppCt.MusicVolume, pBpSysInfo->MusicVolume);	
	
	mainAppCt.MicVolume = pBpSysInfo->MicVolume;
	if((mainAppCt.MicVolume > CFG_PARA_MAX_VOLUME_NUM) || (mainAppCt.MicVolume <= 0))
	{
		mainAppCt.MicVolume = CFG_PARA_MAX_VOLUME_NUM;
	}
	mainAppCt.MicVolumeBak = mainAppCt.MicVolume;
	APP_DBG("MicVolume:%d,%d\n", mainAppCt.MicVolume, pBpSysInfo->MicVolume);

	#ifdef CFG_APP_BT_MODE_EN
	mainAppCt.HfVolume = pBpSysInfo->HfVolume;
	if((mainAppCt.HfVolume > CFG_PARA_MAX_VOLUME_NUM) || (mainAppCt.HfVolume <= 0))
	{
		mainAppCt.HfVolume = CFG_PARA_MAX_VOLUME_NUM;
	}
	APP_DBG("HfVolume:%d,%d\n", mainAppCt.HfVolume, pBpSysInfo->HfVolume);
	#endif
	
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	mainAppCt.EqMode = pBpSysInfo->EqMode;
	if(mainAppCt.EqMode > EQ_MODE_VOCAL_BOOST)
	{
		mainAppCt.EqMode = EQ_MODE_FLAT;
	}
	AudioEffect_EQMode_Set(mainAppCt.EqMode);
//	EqModeSet(mainAppCt.EqMode);
	#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN    
	mainAppCt.EqModeBak = mainAppCt.EqMode;
	mainAppCt.EqModeFadeIn = 0;
	mainAppCt.eqSwitchFlag = 0;
	#endif
	APP_DBG("EqMode:%d,%d\n", mainAppCt.EqMode, pBpSysInfo->EqMode);
#endif

	mainAppCt.MicEffectDelayStep = pBpSysInfo->MicEffectDelayStep;
    if((mainAppCt.MicEffectDelayStep > MAX_MIC_EFFECT_DELAY_STEP) || (mainAppCt.MicEffectDelayStep <= 0))
	{
		mainAppCt.MicEffectDelayStep = MAX_MIC_EFFECT_DELAY_STEP;
	}
	mainAppCt.MicEffectDelayStepBak = mainAppCt.MicEffectDelayStep;
	APP_DBG("MicEffectDelayStep:%d,%d\n", mainAppCt.MicEffectDelayStep, pBpSysInfo->MicEffectDelayStep);

#ifdef CFG_FUNC_MIC_TREB_BASS_EN
    mainAppCt.MicBassStep = pBpSysInfo->MicBassStep;
    if((mainAppCt.MicBassStep > MAX_MUSIC_DIG_STEP) || (mainAppCt.MicBassStep <= 0))
	{
		mainAppCt.MicBassStep = 7;
	}
    mainAppCt.MicTrebStep = pBpSysInfo->MicTrebStep;
    if((mainAppCt.MicTrebStep > MAX_MUSIC_DIG_STEP) || (mainAppCt.MicTrebStep <= 0))
	{
		mainAppCt.MicTrebStep = 7;
	}
	APP_DBG("MicTrebStep:%d,%d\n", mainAppCt.MicTrebStep, pBpSysInfo->MicTrebStep);
	APP_DBG("MicBassStep:%d,%d\n", mainAppCt.MicBassStep, pBpSysInfo->MicBassStep);
#endif
#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN	
    mainAppCt.MusicBassStep = pBpSysInfo->MusicBassStep;
    if((mainAppCt.MusicBassStep > MAX_MUSIC_DIG_STEP) || (mainAppCt.MusicBassStep <= 0))
	{
		mainAppCt.MusicBassStep = 7;
	}
    mainAppCt.MusicTrebStep = pBpSysInfo->MusicTrebStep;
    if((mainAppCt.MusicTrebStep > MAX_MUSIC_DIG_STEP) || (mainAppCt.MusicTrebStep <= 0))
	{
		mainAppCt.MusicTrebStep = 7;
	}
	APP_DBG("MusicTrebStep:%d,%d\n", mainAppCt.MusicTrebStep, pBpSysInfo->MusicTrebStep);
	APP_DBG("MusicBassStep:%d,%d\n", mainAppCt.MusicBassStep, pBpSysInfo->MusicBassStep);
#endif

#else
	//mainAppCt.appBackupMode = ModeBtAudioPlay;		  
	mainAppCt.MusicVolume = CFG_PARA_MAX_VOLUME_NUM;
	#ifdef CFG_APP_BT_MODE_EN
	mainAppCt.HfVolume = CFG_PARA_MAX_VOLUME_NUM;
	#endif
	
	#ifdef CFG_EFFECT_PARAM_IN_FLASH_EN
	mainAppCt.EffectMode = EFFECT_MODE_FLASH_Music;
	#else
#ifdef CFG_FUNC_EFFECT_BYPASS_EN
	mainAppCt.EffectMode = EFFECT_MODE_BYPASS;
#else
#ifdef CFG_FUNC_MIC_KARAOKE_EN
	mainAppCt.EffectMode = EFFECT_MODE_HunXiang;
#else
	mainAppCt.EffectMode = EFFECT_MODE_MIC;
#endif
#endif
	#endif	
	mainAppCt.MicVolume = CFG_PARA_MAX_VOLUME_NUM;
	
#ifdef CFG_FUNC_MUSIC_EQ_MODE_EN
	mainAppCt.EqMode = EQ_MODE_FLAT;
 	#ifdef CFG_FUNC_EQMODE_FADIN_FADOUT_EN    
    mainAppCt.EqModeBak = mainAppCt.EqMode;
	mainAppCt.eqSwitchFlag = 0;
	#endif
#endif
	//mainAppCt.ReverbStep = MAX_MIC_DIG_STEP;
#ifdef CFG_FUNC_MIC_TREB_BASS_EN
	mainAppCt.MicBassStep = 7;
	mainAppCt.MicTrebStep = 7;
#endif
#ifdef CFG_FUNC_MUSIC_TREB_BASS_EN	
	mainAppCt.MusicBassStep = 7;
	mainAppCt.MusicTrebStep = 7;
#endif	

#endif

#ifdef CFG_APP_HDMIIN_MODE_EN
	mainAppCt.hdmiArcOnFlg = 1;
	mainAppCt.hdmiResetFlg = 0;
#endif

#ifdef CFG_FUNC_BREAKPOINT_EN
#ifdef CFG_APP_BT_MODE_EN
	if(SoftFlagGet(SoftFlagUpgradeOK))
	{
		pBpSysInfo->CurModuleId = ModeBtAudioPlay;
		DBG("u or sd upgrade ok ,set mode to ModeBtAudioPlay \n");
	}
#endif
	PowerOnModeGenerate((void *)pBpSysInfo);
#else
	PowerOnModeGenerate(NULL);
#endif


    for(i = 0; i < AUDIO_CORE_SINK_MAX_NUM; i++)
	{
		mainAppCt.gSysVol.AudioSinkVol[i] = CFG_PARA_MAX_VOLUME_NUM;
	}

	for(i = 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
	{
		if(i == MIC_SOURCE_NUM)
		{
			mainAppCt.gSysVol.AudioSourceVol[MIC_SOURCE_NUM] = mainAppCt.MicVolume;
		}
		else if(i == APP_SOURCE_NUM)
		{
			mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] = mainAppCt.MusicVolume;
		}
#ifdef CFG_FUNC_REMIND_SOUND_EN
		else if(i == REMIND_SOURCE_NUM)
		{
			#if CFG_PARAM_FIXED_REMIND_VOL
			mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = CFG_PARAM_FIXED_REMIND_VOL;
			#else
			mainAppCt.gSysVol.AudioSourceVol[REMIND_SOURCE_NUM] = mainAppCt.MusicVolume;
			#endif
		}
#endif
		else
		{
			mainAppCt.gSysVol.AudioSourceVol[i] = CFG_PARA_MAX_VOLUME_NUM;		
		}
	}
	mainAppCt.gSysVol.MuteFlag = TRUE;	
	mainAppCt.muteFlagPre = FALSE;
	
	#ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
	mainAppCt.Silence_Power_Off_Time = 0;
	#endif
}

void SystemTimerInit(void)
{
 	Timer_Config(TIMER2,1000,0);
 	Timer_Start(TIMER2);
 	Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
	NVIC_EnableIRQ(Timer2_IRQn);
}

static void SystemInit(void)
{
	int16_t i;

	DelayMsFunc = (DelayMsFunction)vTaskDelay; //提高Os条件下驱动层延时函数精度，非OS默认使用DelayMs
	DMA_ChannelAllocTableSet((uint8_t*)DmaChannelMap);
#ifdef CFG_DUMP_DEBUG_EN
	DumpUartConfig(TRUE);
#endif
	SarADC_Init();

	//For OTG check
#if defined(CFG_FUNC_UDISK_DETECT)
#if defined(CFG_FUNC_USB_DEVICE_DETECT)
 	OTG_PortSetDetectMode(1,1);
#else
 	OTG_PortSetDetectMode(1,0);
#endif
#else
#if defined(CFG_FUNC_USB_DEVICE_DETECT)
 	OTG_PortSetDetectMode(0,1);
#else
 	OTG_PortSetDetectMode(0,0);
#endif
#endif

 	SystemTimerInit();

#ifdef CFG_FUNC_BREAKPOINT_EN
 	BP_LoadInfo();
#endif

	SysVarInit();
	
#ifdef CFG_FUNC_BT_OTA_EN
	SoftFlagDeregister((~(SoftFlagUpgradeOK|SoftFlagBtOtaUpgradeOK))&SoftFlagMask);
#else
	SoftFlagDeregister((~SoftFlagUpgradeOK)&SoftFlagMask);
#endif

#ifdef CFG_APP_HDMIIN_MODE_EN
	HDMI_CEC_DDC_Init();
#endif
	///////////////////////////////AudioCore/////////////////////////////////////////
	mainAppCt.AudioCore =  (AudioCoreContext*)&AudioCore;
	memset(mainAppCt.AudioCore, 0, sizeof(AudioCoreContext));

	CtrlVarsInit();//音频系统硬件变量初始化，系统变量初始化

	for(i = 0; i < MaxNet; i++)
	{
		AudioCoreMixSampleRateSet(i, CFG_PARA_SAMPLE_RATE);//默认系统采样率
	}
	mainAppCt.SampleRate = CFG_PARA_SAMPLE_RATE;
	
	////Audio Core音量配置
	SystemVolSet();
	
	for( i = 0; i < AUDIO_CORE_SOURCE_MAX_NUM; i++)
	{
	   mainAppCt.AudioCore->AudioSource[i].PreGain = 4095;//默认使用4095， 0dB
	}

	AudioCoreServiceCreate(mainAppCt.msgHandle);
	mainAppCt.AudioCoreSync = FALSE;
#ifdef CFG_FUNC_REMIND_SOUND_EN	
	RemindSoundInit();
#endif

#ifdef CFG_FUNC_ALARM_EN
	mainAppCt.AlarmFlag = FALSE;
#endif

#ifdef CFG_APP_BT_MODE_EN
	//将蓝牙任务创建移至此处,以便优先申请协议栈使用的内存空间,不影响其他的任务; 开机睡眠时，蓝牙stack再次开，避免上电马上退出。
	if(sys_parameter.bt_BackgroundType != BT_BACKGROUND_DISABLE)
		BtStackServiceStart();//蓝牙设备驱动serivce 启动失败时，目前是挂起，无同步消息。
//IRKeyInit();//clk源被改？
#endif

	DeviceServiceInit();
	APP_DBG("MainApp:run\n");

}


//接收下层service created消息，完毕后start这些servcie
static void MainAppServiceCreating(uint16_t msgId)
{
	if(msgId == MSG_AUDIO_CORE_SERVICE_CREATED)
	{
		APP_DBG("MainApp:AudioCore service created\n");
		mainAppCt.AudioCoreSync = TRUE;
	}
	
	if(mainAppCt.AudioCoreSync)
	{
		AudioCoreServiceStart();
		mainAppCt.AudioCoreSync = FALSE;
		mainAppCt.state = TaskStateReady;
	}
}

//接收下层service started，完毕后准备模式切换。
static void MainAppServiceStarting(uint16_t msgId)
{
	if(msgId == MSG_AUDIO_CORE_SERVICE_STARTED)
	{
		APP_DBG("MainApp:AudioCore service started\n");
		mainAppCt.AudioCoreSync = TRUE;
	}

	if(mainAppCt.AudioCoreSync)
	{
		mainAppCt.AudioCoreSync = FALSE;
		mainAppCt.state = TaskStateRunning;
		SysModeTaskCreat();
#ifdef	CFG_FUNC_OPEN_SLOW_DEVICE_TASK
		{
		extern	void CreatSlowDeviceTask(void);
		CreatSlowDeviceTask();
		}
#endif
	}
}

static void PublicDetect(void)
{

#if defined(BT_SNIFF_ENABLE) && defined(CFG_APP_BT_MODE_EN)
		tws_sniff_check_adda_process();//检测sniff后adda是否恢复的轮询
#endif

#ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
		switch(GetSystemMode())
		{
			// Idle,Slave,HFP,USB Audio不省电关机
			case ModeIdle:
			case ModeTwsSlavePlay:
			case ModeBtHfPlay:
			case ModeUsbDevicePlay:
			mainAppCt.Silence_Power_Off_Time = 0;
			break;

			// BT 连上蓝牙不关机
			case ModeBtAudioPlay:
			if(btManager.btLinkState)
				mainAppCt.Silence_Power_Off_Time = 0;
			break;

			default:
			break;
		}

		mainAppCt.Silence_Power_Off_Time++;
        if(mainAppCt.Silence_Power_Off_Time >= SILENCE_POWER_OFF_DELAY_TIME)
		{
			mainAppCt.Silence_Power_Off_Time = 0;
			APP_DBG("Silence Power Off!!\n");

			MessageContext		msgSend;
#if	defined(CFG_IDLE_MODE_POWER_KEY) && (POWERKEY_MODE == POWERKEY_MODE_PUSH_BUTTON)
			msgSend.msgId = MSG_POWERDOWN;
			APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_POWERDOWN\n");
#elif defined(CFG_SOFT_POWER_KEY_EN)
			msgSend.msgId = MSG_SOFT_POWER;
			APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_SOFT_POWEROFF\n");
#elif defined(CFG_IDLE_MODE_DEEP_SLEEP)
			msgSend.msgId = MSG_DEEPSLEEP;
			APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_DEEPSLEEP\n");
#else
			msgSend.msgId = MSG_POWER;
			APP_DBG("msgSend.msgId = MSG_POWER\n");
#endif
			MessageSend(GetMainMessageHandle(), &msgSend);
        }
#endif

#if FLASH_BOOT_EN
		//设备经过播放，预搜索mva包登记，可升级。拔出时取消登记。
		#ifndef FUNC_UPDATE_CONTROL
		if(SoftFlagGet(SoftFlagMvaInCard) && GetSystemMode() == ModeCardAudioPlay && (!SoftFlagGet(SoftFlagUpgradeOK)))
		{
			APP_DBG("MainApp:updata file exist in Card\n");
			#ifdef FUNC_OS_EN
			if(SDIOMutex != NULL)
			{
				osMutexLock(SDIOMutex);
			}
			#endif
			start_up_grate(SysResourceCard);
			#ifdef FUNC_OS_EN
			if(SDIOMutex != NULL)
			{
				osMutexUnlock(SDIOMutex);
			}
			#endif
		}
		else if(SoftFlagGet(SoftFlagMvaInUDisk) && GetSystemMode() == ModeUDiskAudioPlay&& (!SoftFlagGet(SoftFlagUpgradeOK)))
		{
			APP_DBG("MainApp:updata file exist in Udisk\n");
			#ifdef FUNC_OS_EN
			if(UDiskMutex != NULL)
			{
				//osMutexLock(UDiskMutex);
				while(osMutexLock_1000ms(UDiskMutex) != 1)
				{
					WDG_Feed();
				}
			}
			#endif
			start_up_grate(SysResourceUDisk);
			#ifdef FUNC_OS_EN
			if(UDiskMutex != NULL)
			{
				osMutexUnlock(UDiskMutex);
			}
			#endif
		}
		#endif


		/*uint8_t cmd = 0;
		if(UART0_RecvByte(&cmd))
		{
			if(cmd == 'y')
			{
				start_up_grate(0xFFFFFFFF);
			}
		}
		*/
#endif

}
static void PublicMsgPross(MessageContext msg)
{
	switch(msg.msgId)
	{
		case MSG_AUDIO_CORE_SERVICE_CREATED:	
			MainAppServiceCreating(msg.msgId);
			break;
		
		case MSG_AUDIO_CORE_SERVICE_STARTED:
			MainAppServiceStarting(msg.msgId);
			break;
#if FLASH_BOOT_EN
		case MSG_DEVICE_SERVICE_CARD_OUT:
//			SoftFlagDeregister(SoftFlagUpgradeOK);
			SoftFlagDeregister(SoftFlagMvaInCard);//清理mva包标记
			break;
		
		case MSG_DEVICE_SERVICE_U_DISK_OUT:
//			SoftFlagDeregister(SoftFlagUpgradeOK);
			SoftFlagDeregister(SoftFlagMvaInUDisk);
			break;
		
		case MSG_UPDATE:
			//if(SoftFlagGet(SoftFlagUpgradeOK))break;
			#ifdef FUNC_UPDATE_CONTROL
			APP_DBG("MainApp:UPDATE MSG\n");
			//设备经过播放，预搜索mva包登记，可升级。拔出时取消登记。
			if(SoftFlagGet(SoftFlagMvaInCard) && GetSystemMode() == ModeCardAudioPlay)
			{
				APP_DBG("MainApp:updata file exist in Card\n");
				start_up_grate(SysResourceCard);
			}
			else if(SoftFlagGet(SoftFlagMvaInUDisk) && GetSystemMode() == ModeUDiskAudioPlay)
			{
				APP_DBG("MainApp:updata file exist in Udisk\n");
				start_up_grate(SysResourceUDisk);
			}
			#endif
			break;
#endif		

#ifdef CFG_APP_IDLE_MODE_EN		
		case MSG_POWER:
		case MSG_POWERDOWN:
		case MSG_DEEPSLEEP:
#ifdef CFG_SOFT_POWER_KEY_EN
		case MSG_SOFT_POWER:
#endif
#ifdef CFG_APP_HDMIIN_MODE_EN
			if(GetSystemMode() == ModeHdmiAudioPlay)
			{
				if(SoftFlagGet(SoftFlagDeepSleepMsgIsFromTV) == 0)//非电视端发来的关机
				{
					HDMI_CEC_ActivePowerOff(200);
				}
				SoftFlagDeregister(SoftFlagDeepSleepMsgIsFromTV);
			}
#endif
			if(GetSystemMode() != ModeIdle)
			{			
				if (msg.msgId == MSG_POWER){
					DBG("Main task MSG_POWER\n");
				}else if (msg.msgId == MSG_POWERDOWN){
					DBG("Main task MSG_POWERDOWN\n");
				}else if (msg.msgId == MSG_DEEPSLEEP){
					DBG("Main task MSG_DEEPSLEEP\n");
				}
				SendEnterIdleModeMsg();
			#if (defined(CFG_APP_BT_MODE_EN) && (BT_HFP_SUPPORT == ENABLE))
				if(GetSystemMode() == ModeBtHfPlay)
				{
					BtHfModeExit();				
				}
				extern void SetsBtHfModeEnterFlag(uint32_t flag);
				SetsBtHfModeEnterFlag(0);
			#endif		
			
			#ifdef	CFG_IDLE_MODE_POWER_KEY
				if(msg.msgId == MSG_POWERDOWN)
				{
					SoftFlagRegister(SoftFlagIdleModeEnterPowerDown);
				}
			#endif				
			
			#ifdef CFG_IDLE_MODE_DEEP_SLEEP
				if(msg.msgId == MSG_DEEPSLEEP)
				{					
					SoftFlagRegister(SoftFlagIdleModeEnterSleep);
				}
			#endif	
			#ifdef CFG_SOFT_POWER_KEY_EN
				if(msg.msgId == MSG_SOFT_POWER)
				{
					SoftFlagRegister(SoftFlagIdleModeEnterSoftPower);
				}
			#endif
			}
			break;
#else
#ifdef CFG_SOFT_POWER_KEY_EN
		case MSG_SOFT_POWER:
			SoftKeyPowerOff();
			break;
#endif
#endif

#ifdef CFG_APP_BT_MODE_EN
		case MSG_BT_ENTER_DUT_MODE:
			BtEnterDutModeFunc();
			break;

		case MSG_BTSTACK_DEEPSLEEP:
			APP_DBG("MSG_BTSTACK_DEEPSLEEP\n");
			break;

		case MSG_BTSTACK_BB_ERROR:
			APP_DBG("bb and bt stack reset\n");
			RF_PowerDownBySw();
			WDG_Feed();
			//reset bb and bt stack
			rwip_reset();
			BtStackServiceKill();
			WDG_Feed();
			vTaskDelay(50);
			RF_PowerDownByHw();
			WDG_Feed();
			//reset bb and bt stack
			BtStackServiceStart();

			//发起回连
			BtStackServiceMsgSend(MSG_BTSTACK_BB_ERROR_RESTART);

			//如蓝牙模式处于slave模式,则退出tws slave模式
			if(GetSystemMode() == ModeTwsSlavePlay)
			{
				MessageContext		msgSend;
				APP_DBG("Exit Tws Slave Mode\n");
				msgSend.msgId = MSG_DEVICE_SERVICE_TWS_SLAVE_DISCONNECT;
				MessageSend(GetMainMessageHandle(), &msgSend);
			}
			break;
#ifdef CFG_FUNC_BT_OTA_EN
		case MSG_BT_START_OTA:
			APP_DBG("\nMSG_BT_START_OTA\n");
			RF_PowerDownBySw();
			WDG_Feed();
			rwip_reset();
			BtStackServiceKill();
			WDG_Feed();
			vTaskDelay(50);
			RF_PowerDownByHw();
			WDG_Feed();
			start_up_grate(SysResourceBtOTA);
			break;
#endif
#if (BT_HFP_SUPPORT == ENABLE)
			case MSG_DEVICE_SERVICE_ENTER_BTHF_MODE:
			if(GetHfpState(BtCurIndex_Get()) >= BT_HFP_STATE_CONNECTED)
			{
				BtHfModeEnter();
			}
			break;
#endif
#ifdef BT_AUTO_ENTER_PLAY_MODE
		case MSG_BT_A2DP_STREAMING:
			//播放歌曲时,有模式切换需求,则在此消息中开始进行模式切换操作
			if((GetSystemMode() != ModeBtAudioPlay)&&(GetSystemMode() != ModeBtHfPlay))
			{
				MessageContext		msgSend;
				
				APP_DBG("Enter Bt Audio Play Mode...\n");
				//ResourceRegister(AppResourceBtPlay);
				
				// Send message to main app
				msgSend.msgId		= MSG_DEVICE_SERVICE_BTPLAY_IN;
				MessageSend(GetMainMessageHandle(), &msgSend);
			}
			break;
#endif
#endif
	}		
}

static void MainAppTaskEntrance(void * param)
{
	MessageContext		msg;

	SystemInit();
	while(1)
	{
		MessageRecv(mainAppCt.msgHandle, &msg, MAIN_APP_MSG_TIMEOUT);
		PublicDetect();
		PublicMsgPross(msg);
#ifdef SOFT_WACTH_DOG_ENABLE
		big_dog_feed();
#else
		WDG_Feed();
#endif
		#ifdef AUTO_TEST_ENABLE
		extern void AutoTestMain(uint16_t test_msg);
		AutoTestMain(msg.msgId);
		#endif

		if(mainAppCt.state == TaskStateRunning)
		{
			DeviceServicePocess(msg.msgId);
			if(msg.msgId != MSG_NONE)
			{
				SysModeGenerate(msg.msgId);
				MessageSend(GetSysModeMsgHandle(), &msg);	
			}
			SysModeChangeTimeoutProcess();
		}
	}
}

/***************************************************************************************
 *
 * APIs
 *
 */
int32_t MainAppTaskStart(void)
{
	MainAppInit();
#ifdef CFG_FUNC_SHELL_EN
	shell_init();
	xTaskCreate(mv_shell_task, "SHELL", SHELL_TASK_STACK_SIZE, NULL, SHELL_TASK_PRIO, NULL);
#endif
	xTaskCreate(MainAppTaskEntrance, "MainApp", MAIN_APP_TASK_STACK_SIZE, NULL, MAIN_APP_TASK_PRIO, &mainAppCt.taskHandle);
	return 0;
}

MessageHandle GetMainMessageHandle(void)
{
	return mainAppCt.msgHandle;
}


uint32_t GetSystemMode(void)
{
	return mainAppCt.SysCurrentMode;
}

void SamplesFrameUpdataMsg(void)//发现帧变化，发送消息
{
	MessageContext		msgSend;
	APP_DBG("SamplesFrameUpdataMsg\n");

	msgSend.msgId		= MSG_AUDIO_CORE_FRAME_SIZE_CHANGE;
    MessageSend(mainAppCt.msgHandle, &msgSend);
}

void EffectUpdataMsg(void)
{
	MessageContext		msgSend;
	APP_DBG("EffectUpdataMsg\n");

	msgSend.msgId		= MSG_AUDIO_CORE_EFFECT_CHANGE;
	MessageSend(mainAppCt.msgHandle, &msgSend);
}

uint32_t IsBtAudioMode(void)
{
	return (GetSysModeState(ModeBtAudioPlay) == ModeStateRunning);
}

uint32_t IsBtHfMode(void)
{
	return (GetSysModeState(ModeBtHfPlay) == ModeStateRunning);
}

uint32_t IsBtTwsSlaveMode(void)
{
	return (GetSysModeState(ModeTwsSlavePlay) == ModeStateRunning);
}

uint32_t IsIdleModeReady(void)
{
	if(GetModeDefineState(ModeIdle))
	{
		if(GetSysModeState(ModeIdle) == ModeStateInit || GetSysModeState(ModeIdle) == ModeStateRunning )
			return 1;
	}
	return 0;
}


void PowerOffMessage(void)
{
	MessageContext		msgSend;

#if	defined(CFG_IDLE_MODE_POWER_KEY) && (POWERKEY_MODE == POWERKEY_MODE_PUSH_BUTTON)
	msgSend.msgId = MSG_POWERDOWN;
	APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_POWERDOWN\n");
#elif defined(CFG_SOFT_POWER_KEY_EN)
	msgSend.msgId = MSG_SOFT_POWER;
	APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_SOFT_POWEROFF\n");
#elif	defined(CFG_IDLE_MODE_DEEP_SLEEP) 
	msgSend.msgId = MSG_DEEPSLEEP;
	APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_DEEPSLEEP\n");
#else
	msgSend.msgId = MSG_POWER;
	APP_DBG("msgSend.msgId = MSG_POWER\n");
#endif

	MessageSend(GetMainMessageHandle(), &msgSend);
}



void BatteryLowMessage(void)
{
	MessageContext		msgSend;

	APP_DBG("msgSend.msgId = MSG_DEVICE_SERVICE_BATTERY_LOW\n");
	msgSend.msgId = MSG_DEVICE_SERVICE_BATTERY_LOW;
	MessageSend(GetMainMessageHandle(), &msgSend);
}

void TwsSlaveModeSwitchDeal(SysModeNumber pre, SysModeNumber Cur)
{
#ifdef TWS_SLAVE_MODE_SWITCH_EN
	//null
	if(sys_parameter.bt_BackgroundType == BT_BACKGROUND_FAST_POWER_ON_OFF)
	{
		if (pre == ModeTwsSlavePlay)
		{
			if((Cur != ModeBtAudioPlay)&&(Cur != ModeTwsSlavePlay))
			{
				BtFastPowerOff();
				BtStackServiceWaitClear();
			}
			else
			{
				BtStackServiceWaitResume();
			}
		}
	}
	else if(sys_parameter.bt_BackgroundType == BT_BACKGROUND_DISABLE)
	{
		if(Cur != ModeBtAudioPlay)
		{
			BtPowerOff();
		}
	}
#endif	
}

