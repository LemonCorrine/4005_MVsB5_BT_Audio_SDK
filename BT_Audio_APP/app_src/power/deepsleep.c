#include "app_config.h"
#include "powercontroller.h"
#include "deepsleep.h"
#include "gpio.h"
#include "timeout.h"
#include "audio_adc.h"
#include "dac.h"
#include "clk.h"
#include "chip_info.h"
#include "otg_device_hcd.h"
#include "rtc.h"
#include "irqn.h"
#include "debug.h"
#include "adc_key.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "adc_key.h"
#include "uarts.h"
//#include "OrioleReg.h"//for test
#include "sys.h"
#include "sadc_interface.h"
#include "watchdog.h"
#include "pmu.h"
#include "ir_key.h"
#include "app_message.h"
#include "reset.h"
#include "bt_stack_service.h"
#include "key.h"
#include "main_task.h"
#include "efuse.h"
#include "bt_common_api.h"
#include "bt_manager.h"
#include "bt_app_sniff.h"
#include "hdmi_in_api.h"
#include "adc_interface.h"
#include "dac_interface.h"
#ifdef CFG_IDLE_MODE_DEEP_SLEEP
HDMIInfo  			 *gHdmiCt;
void GIE_ENABLE(void);
void WakeupMain(void);
void SleepMain(void);
void LogUartConfig(bool InitBandRate);

#define CHECK_SCAN_TIME				5000		//醒来确认有效唤醒源 扫描限时ms。
TIMER   waitCECTime;
uint8_t	waitCECTimeFlag = 0;

#define DPLL_QUICK_START_SLOPE	(*(volatile unsigned long *) 0x40026028)
//DPLL 快速启动参数获取。
void Clock_GetDPll(uint32_t* NDAC, uint32_t* OS, uint32_t* K1, uint32_t* FC);

static uint32_t sources;
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
uint32_t alarm = 0;
#endif

__attribute__((section(".driver.isr")))void WakeupInterrupt(void)
{
	sources |= Power_WakeupSourceGet();

	Power_WakeupSourceClear();
}


void SystermGPIOWakeupConfig(PWR_SYSWAKEUP_SOURCE_SEL source,PWR_WAKEUP_GPIO_SEL gpio,PWR_SYSWAKEUP_SOURCE_EDGE_SEL edge)
{
	if(gpio <= WAKEUP_GPIOA31)
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   (1 << gpio));
		GPIO_RegOneBitClear(GPIO_A_OE, (1 << gpio));
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			GPIO_RegOneBitSet(GPIO_A_PU, (1 << gpio));//因为芯片的GPIO有内部上下拉电阻,选择下降沿触发时要将指定的GPIO唤醒管脚配置为上拉
			GPIO_RegOneBitClear(GPIO_A_PD, (1 << gpio));
		}
		else if(edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			GPIO_RegOneBitClear(GPIO_A_PU, (1 << gpio));//因为芯片的GPIO有内部上下拉电阻，所以选择上升沿触发时要将指定的GPIO唤醒管脚配置为下拉
			GPIO_RegOneBitSet(GPIO_A_PD, (1 << gpio));
		}
	}
	else if(gpio <= WAKEUP_GPIOB9)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   (1 << (gpio - WAKEUP_GPIOB0)));
		GPIO_RegOneBitClear(GPIO_B_OE, (1 << (gpio - WAKEUP_GPIOB0)));
		if( edge == SYSWAKEUP_SOURCE_NEGE_TRIG )
		{
			GPIO_RegOneBitSet(GPIO_B_PU, (1 << (gpio - WAKEUP_GPIOB0)));//因为芯片的GPIO有内部上下拉电阻,选择下降沿触发时要将指定的GPIO唤醒管脚配置为上拉
			GPIO_RegOneBitClear(GPIO_B_PD, (1 << (gpio - WAKEUP_GPIOB0)));
		}
		else if( edge == SYSWAKEUP_SOURCE_POSE_TRIG )
		{
			GPIO_RegOneBitClear(GPIO_B_PU, (1 << (gpio - WAKEUP_GPIOB0)));//因为芯片的GPIO有内部上下拉电阻，所以选择上升沿触发时要将指定的GPIO唤醒管脚配置为下拉
			GPIO_RegOneBitSet(GPIO_B_PD, (1 << (gpio - WAKEUP_GPIOB0)));
		}
	}


	Power_WakeupSourceClear();
	Power_WakeupSourceSet(source, gpio, edge);
	Power_WakeupEnable(source);

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();
}

void SystermIRWakeupConfig(IR_MODE_SEL ModeSel, IR_IO_SEL GpioSel, IR_CMD_LEN_SEL CMDLenSel)
{
	Clock_BTDMClkSelect(RC_CLK32_MODE);//sniff开启时使用影响蓝牙功能呢
	Reset_FunctionReset(IR_FUNC_SEPA);
	IRKeyInit();
	IR_WakeupEnable();

	if(GpioSel == IR_GPIOB6)
	{
		GPIO_RegOneBitSet(GPIO_B_IE,   GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_OE, GPIO_INDEX6);
		GPIO_RegOneBitSet(GPIO_B_IN,   GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_OUT, GPIO_INDEX6);
		GPIO_RegOneBitClear(GPIO_B_PD, GPIO_INDEX6);
	}
	else if(GpioSel == IR_GPIOA1)
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   GPIO_INDEX1);
		GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX1);
		GPIO_RegOneBitSet(GPIO_A_IN,   GPIO_INDEX1);
		GPIO_RegOneBitClear(GPIO_A_OUT, GPIO_INDEX1);
		GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX1);
	}
	else if(GpioSel == IR_GPIOA29)
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX29);
		GPIO_RegOneBitSet(GPIO_A_IN,   GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_OUT, GPIO_INDEX29);
		GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX29);
	}
	else if(GpioSel == IR_GPIOA31)
	{
		GPIO_RegOneBitSet(GPIO_A_IE,   GPIO_INDEX31);
		GPIO_RegOneBitClear(GPIO_A_OE, GPIO_INDEX31);
		GPIO_RegOneBitSet(GPIO_A_IN,   GPIO_INDEX31);
		GPIO_RegOneBitClear(GPIO_A_OUT, GPIO_INDEX31);
		GPIO_RegOneBitClear(GPIO_A_PD, GPIO_INDEX31);
	}
	else
	{
		return;
	}
	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(CFG_PARA_WAKEUP_SOURCE_IR, 0, 0);
	Power_WakeupEnable(CFG_PARA_WAKEUP_SOURCE_IR);
}


#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
//RTC唤醒 并不会进入RTC中断
void SystermRTCWakeupConfig(uint32_t SleepSecond)
{
	//RTC_REG_TIME_UNIT start;
	RTC_ClockSrcSel(OSC_24M);
	RTC_IntDisable();
	RTC_IntFlagClear();
	RTC_WakeupDisable();

	alarm = RTC_SecGet() + SleepSecond;
	RTC_SecAlarmSet(alarm);
	RTC_WakeupEnable();
	RTC_IntEnable();

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	NVIC_EnableIRQ(Rtc_IRQn);
	NVIC_SetPriority(Rtc_IRQn, 1);
	GIE_ENABLE();

	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE7_RTC, 0, 0);
	Power_WakeupEnable(SYSWAKEUP_SOURCE7_RTC);
}
#endif //CFG_PARA_WAKEUP_RTC

//启用多个唤醒源时，source通道配置灵活，但不可重复。
void WakeupSourceSet(void)
{
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
	SystermRTCWakeupConfig(CFG_PARA_WAKEUP_TIME_RTC);
#endif	
#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY) && defined(CFG_PARA_WAKEUP_GPIO_ADCKEY)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_ADCKEY, CFG_PARA_WAKEUP_GPIO_ADCKEY, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif
#ifdef CFG_PARA_WAKEUP_SOURCE_POWERKEY
	SystermGPIOWakeupConfig(SYSWAKEUP_SOURCE6_POWERKEY, 42, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_IOKEY1) && defined(CFG_PARA_WAKEUP_GPIO_IOKEY1)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_IOKEY1, CFG_PARA_WAKEUP_GPIO_IOKEY1, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_IOKEY2) && defined(CFG_PARA_WAKEUP_GPIO_IOKEY2)
	SystermGPIOWakeupConfig(CFG_PARA_WAKEUP_SOURCE_IOKEY2, CFG_PARA_WAKEUP_GPIO_IOKEY2, SYSWAKEUP_SOURCE_NEGE_TRIG);
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_IR)
	SystermIRWakeupConfig(CFG_PARA_IR_SEL, CFG_RES_IR_PIN, CFG_PARA_IR_BIT);
#endif
}


void DeepSleeping(void)
{
	uint32_t GpioAPU_Back,GpioAPD_Back,GpioBPU_Back,GpioBPD_Back;

//	WDG_Disable();
	WDG_Feed();

#if CFG_RES_MIC_SELECT
	AudioADC_DeInit(ADC1_MODULE);
	AudioCoreSourceDisable(MIC_SOURCE_NUM);
#endif
	AudioDAC_AllPowerDown();

	GpioAPU_Back = GPIO_RegGet(GPIO_A_PU);
	GpioAPD_Back = GPIO_RegGet(GPIO_A_PD);
	GpioBPU_Back = GPIO_RegGet(GPIO_B_PU);
	GpioBPD_Back = GPIO_RegGet(GPIO_B_PD);
	WakeupSourceSet();
	SleepMain();
	waitCECTimeFlag = 0;
	WDG_Disable();
	Power_GotoDeepSleep();
	WDG_Enable(WDG_STEP_4S);
	while(!SystermWackupSourceCheck())
	{
		Power_WakeupDisable(0xff);
		WakeupSourceSet();
		waitCECTimeFlag = 0;
		WDG_Disable();
		Power_GotoDeepSleep();
		WDG_Enable(WDG_STEP_4S);
	}
	Power_WakeupDisable(0xff);
	//GPIO恢复上下拉
	GPIO_RegSet(GPIO_A_PU, GpioAPU_Back);
	GPIO_RegSet(GPIO_A_PD, GpioAPD_Back);
	GPIO_RegSet(GPIO_B_PU, GpioBPU_Back);
	GPIO_RegSet(GPIO_B_PD, GpioBPD_Back);
#if 1
    GPIO_PortBModeSet(GPIOB0, 0x0001); //调试口 SW恢复 方便下载
    GPIO_PortBModeSet(GPIOB1, 0x0001);
#else
	GPIO_PortAModeSet(GPIOA30, 0x0005);//调试口 SW恢复 方便下载
	GPIO_PortAModeSet(GPIOA31, 0x0004);
#endif
	WDG_Feed();
	WakeupMain();
	WDG_Feed();
#if defined(CFG_RES_ADC_KEY_SCAN) || defined(CFG_RES_IR_KEY_SCAN) || defined(CFG_RES_CODE_KEY_USE)|| defined(CFG_ADC_LEVEL_KEY_EN) || defined(CFG_RES_IO_KEY_SCAN)
	KeyInit();//Init keys
#endif	

#ifdef CFG_FUNC_LED_REFRESH
	//默认优先级为0，旨在提高刷新速率，特别是断点记忆等写flash操作有影响刷屏，必须严格遵守所有timer6中断调用都是TCM代码，含调用的driver库代码
	//已确认GPIO_RegOneBitSet、GPIO_RegOneBitClear在TCM区，其他api请先确认。
	NVIC_SetPriority(Timer6_IRQn, 0);
 	Timer_Config(TIMER6,1000,0);
 	Timer_Start(TIMER6);
 	NVIC_EnableIRQ(Timer6_IRQn);
#endif

#ifdef CFG_FUNC_POWER_MONITOR_EN
	extern void PowerMonitorInit(void);
	PowerMonitorInit();
#endif

#if defined(CFG_FUNC_DISPLAY_EN)
    DispInit(0);
#endif

    extern const DACParamCt DACDefaultParamCt;
    AudioDAC_AllPowerOn(DACDefaultParamCt.DACModel, DACDefaultParamCt.DACLoadStatus, DACDefaultParamCt.PVDDModel, DACDefaultParamCt.DACEnergyModel, DACDefaultParamCt.DACVcomModel);
}


bool SystermWackupSourceCheck(void)
{
	MessageId KeyVal;
	TIMER WaitScan;

#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY)
	SarADC_Init();
	AdcKeyInit();
#endif

//********************
	//串口IO设置
	LogUartConfig(FALSE);//此处如果重配clk波特率，较为耗时，不重配。
	SysTickInit();
	//APP_DBG("Scan:%x\n", (int)sources);
#ifdef CFG_PARA_WAKEUP_SOURCE_RTC
	if(sources & CFG_PARA_WAKEUP_SOURCE_RTC)
	{
		sources = 0;//唤醒源清零
		//APP_DBG("Alarm!%d", RTC_SecGet());
		return TRUE;
	}
	else if(alarm)//避免RTC唤醒事件错失
	{
		uint32_t NowTime;
		NowTime = RTC_SecGet();
		if(NowTime + 2 + CHECK_SCAN_TIME / 1000 > alarm)//如果存在多个唤醒源，rtc可以提前唤醒()，以避免丢失
		{
			//APP_DBG("Timer");
			sources = 0;//唤醒源清零
			alarm = 0;
			return TRUE;
		}
	}
#endif

	TimeOutSet(&WaitScan, CHECK_SCAN_TIME);
	while(!IsTimeOut(&WaitScan)
		)
	{
		WDG_Feed();
#ifdef CFG_PARA_WAKEUP_SOURCE_IR
		if(sources & CFG_PARA_WAKEUP_SOURCE_IR)
		{
			KeyVal = GetGlobalKeyMessageId(IRKeyScan());
			if(KeyVal == MSG_DEEPSLEEP || KeyVal == MSG_BT_SNIFF)
			{
				sources = 0;
				return TRUE;
			}
		}
#endif
#ifdef CFG_PARA_WAKEUP_SOURCE_POWERKEY
//		if(sources & SYSWAKEUP_SOURCE6_POWERKEY)
//		{
//			sources = 0;
//			return TRUE;
//		}
#endif
#if defined(CFG_PARA_WAKEUP_SOURCE_ADCKEY) && defined(CFG_RES_ADC_KEY_SCAN)
		if(sources & (CFG_PARA_WAKEUP_SOURCE_ADCKEY | SYSWAKEUP_SOURCE6_POWERKEY))
		{
			KeyVal = GetGlobalKeyMessageId(AdcKeyScan());
			if(KeyVal == MSG_DEEPSLEEP ||KeyVal == MSG_BT_SNIFF)
			{
				sources = 0;
				return TRUE;
			}
		}
#endif
#ifdef CFG_RES_IO_KEY_SCAN
#ifdef CFG_PARA_WAKEUP_SOURCE_IOKEY1
		if(sources & CFG_PARA_WAKEUP_SOURCE_IOKEY1)
		{
			return TRUE;
		}
#endif
#ifdef CFG_PARA_WAKEUP_SOURCE_IOKEY2
		if(sources & CFG_PARA_WAKEUP_SOURCE_IOKEY2)
		{
			return TRUE;
		}
#endif
#endif

	}
	sources = 0;
//	SysTickDeInit();
	return FALSE;
}


#endif//CFG_FUNC_DEEPSLEEP_EN

void BtDeepSleepForUsr(void)	//蓝牙休眠配置入口
{
	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	Power_WakeupSourceClear();
	Power_WakeupSourceSet(SYSWAKEUP_SOURCE15_BT, 0, 0);//设置蓝牙为唤醒源，无需IO所以随便填了0
	Power_WakeupEnable(SYSWAKEUP_SOURCE15_BT);

	NVIC_DisableForDeepsleep();
	SleepMain();
	Power_GotoDeepSleep();
	WakeupMain();
	NVIC_EnableForDeepsleep();
	APP_DBG("LogUartConfig\n");
}
