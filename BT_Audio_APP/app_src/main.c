#include <stdlib.h>
#include "main_task.h"
#include "clk.h"
#include "timer.h"
#include "i2s.h"
#include "watchdog.h"
#include "reset.h"
#include "rtc.h"
#include "spi_flash.h"
#include "gpio.h"
#include "chip_info.h"
#include "irqn.h"
#include "remap.h"
#include "otg_detect.h"
#include "remind_sound.h"
#ifdef CFG_APP_BT_MODE_EN
#include "bt_common_api.h"
#endif

#include "sadc_interface.h"
#include "powercontroller.h"
#include "audio_decoder_api.h"
#include "sys.h"
#ifdef CFG_FUNC_DISPLAY_EN
#include "display.h"
#endif

#ifdef CFG_APP_BT_MODE_EN
#if (BT_HFP_SUPPORT)
#include "bt_hf_mode.h"
#endif
#endif
#include "rtc_timer.h"
#include "rtc_ctrl.h"
#include "efuse.h"
#include "mode_task.h"
#include "device_detect.h"
#include "idle_mode.h"
#include "flash_table.h"
#include "sys_param.h"
#include "pmu.h"

#include "bt_em_config.h"

//-----------------globle timer----------------------//
volatile uint32_t gInsertEventDelayActTimer = 2000; // ms
volatile uint32_t gChangeModeTimeOutTimer = CHANGE_MODE_TIMEOUT_COUNT;
volatile uint32_t gDeviceCheckTimer = DEVICE_DETECT_TIMER; //ms
//volatile uint32_t gDeviceUSBDeviceTimer = DEVICE_USB_DEVICE_DETECT_TIMER ;//ms
#ifdef CFG_FUNC_CARD_DETECT	
volatile uint32_t gDeviceCardTimer = DEVICE_CARD_DETECT_TIMER ;//ms
#endif
#ifdef CFG_LINEIN_DET_EN
volatile uint32_t gDeviceLineTimer = DEVICE_LINEIN_DETECT_TIMER ;//ms
#endif

#ifdef HDMI_HPD_CHECK_DETECT_EN
volatile uint32_t gDevicehdmiTimer = DEVICE_HDMI_DETECT_TIMER ;//ms
#endif

#ifdef CFG_FUNC_BREAKPOINT_EN
volatile uint32_t gBreakPointTimer = 0 ;//ms
#endif
#if defined(CFG_APP_IDLE_MODE_EN)&&defined(CFG_FUNC_REMIND_SOUND_EN)
volatile uint32_t gIdleRemindSoundTimeOutTimer = 0 ;//ms
#endif
//-----------------globle timer----------------------//
extern void DBUS_Access_Area_Init(uint32_t start_addr);
extern const unsigned char *GetLibVersionFatfsACC(void);
extern void UsbAudioTimer1msProcess(void);
extern char *effect_lib_version_return(void);

void _printf_float()
{

}

void OneMSTimer(void)
{
	if(gInsertEventDelayActTimer)gInsertEventDelayActTimer--;
	if(gChangeModeTimeOutTimer)gChangeModeTimeOutTimer--;
	if(gDeviceCheckTimer)gDeviceCheckTimer--;
#ifdef CFG_FUNC_CARD_DETECT	
	if(gDeviceCardTimer > 1)gDeviceCardTimer--;
#endif
#ifdef CFG_LINEIN_DET_EN
	if(gDeviceLineTimer > 1)gDeviceLineTimer--;
#endif
#ifdef HDMI_HPD_CHECK_DETECT_EN
	if(gDevicehdmiTimer > 1)gDevicehdmiTimer--;
#endif
//	if(gDeviceUSBDeviceTimer > 1)gDeviceUSBDeviceTimer--;
#ifdef CFG_FUNC_BREAKPOINT_EN	
	if(gBreakPointTimer > 1)gBreakPointTimer--;
#endif
#if defined(CFG_APP_IDLE_MODE_EN)&&defined(CFG_FUNC_REMIND_SOUND_EN)
	gIdleRemindSoundTimeOutTimer++;
#endif
#ifdef CFG_FUNC_SILENCE_AUTO_POWER_OFF_EN
	mainAppCt.Silence_Power_Off_Time++;
#endif
}

void Timer2Interrupt(void)
{
	Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
#if defined(CFG_FUNC_USB_DEVICE_DETECT) || defined(CFG_FUNC_UDISK_DETECT)
	OTG_PortLinkCheck();
#endif

#ifdef CFG_APP_USB_AUDIO_MODE_EN
	UsbAudioTimer1msProcess(); //1ms中断监控
#endif

#ifdef CFG_APP_BT_MODE_EN
#if (BT_HFP_SUPPORT)
	BtHf_Timer1msProcess();
#endif
#endif
	OneMSTimer();
#ifdef VD51_REDMINE_13199
	RgbLed1MsInterrupt();
#endif
}

#ifdef CFG_FUNC_LED_REFRESH
__attribute__((section(".tcm_section")))
void Timer6Interrupt(void)
{
	Timer_InterruptFlagClear(TIMER6, UPDATE_INTERRUPT_SRC);

	//示例代码，需要添加段“.tcm_section”
	//关键字    __attribute__((section(".tcm_section")))
	//客户需要将自己的实现的API代码添加关键字
	//GPIO_RegOneBitSet(GPIO_A_TGL, GPIO_INDEX2);
	extern void LedFlushDisp(void);
	LedFlushDisp();
}
#endif

void SystemClockInit(bool FristPoweron)
{
#ifndef USB_CRYSTA_FREE_EN
	if(FristPoweron)
		Clock_HOSCCurrentSet(15);
#endif

#ifdef USB_CRYSTA_FREE_EN
	Clock_USBCrystaFreeSet(SYS_CORE_DPLL_FREQ);
#else
	Clock_Config(1, SYS_CRYSTAL_FREQ);
	Clock_PllLock(SYS_CORE_DPLL_FREQ);
	Clock_APllLock(SYS_CORE_APLL_FREQ);//蓝牙使用APLL 240M
#endif

	Clock_SysClkSelect(SYS_CORE_CLK_SELECT);

	//Note: USB和UART时钟配置DPLL和APLL必须是同一个时钟,但是UART可以单独选择RC
	Clock_USBClkSelect(SYS_USB_CLK_SELECT);
	Clock_UARTClkSelect(SYS_UART_CLK_SELECT);

	Clock_SpdifClkSelect(SYS_SPDIF_CLK_SELECT);
	//模块时钟使能配置
	Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
	Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
	Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);

#ifndef USB_CRYSTA_FREE_EN
	if(FristPoweron)
		Clock_HOSCCurrentSet(5);
#endif

	SpiFlashInit(SYS_FLASH_FREQ_SELECT, MODE_4BIT, 0, SYS_FLASH_CLK_SELECT);

#if (SYS_CORE_SET_MODE == CORE_ONLY_APLL_MODE)
	Clock_PllClose();
#endif

#if (SYS_CORE_APLL_FREQ >= (288*1000)) || (SYS_CORE_DPLL_FREQ >= (288*1000))
	//超频需要配置LDO11
	Power_LDO11DConfig(PWD_LDO11_LVL_1V15);
#endif

#ifdef CHIP_USE_DCDC
	ldo_switch_to_dcdc(3); // 3-1.6V Default:1.6V
#else
	Power_LDO16Config(1);
#endif
}

void LogUartConfig(bool InitBandRate)
{
#ifdef CFG_FUNC_DEBUG_EN

	if(GET_DEBUG_GPIO_PORT(CFG_UART_TX_PORT) == DEBUG_GPIO_PORT_A)
		GPIO_PortAModeSet(GET_DEBUG_GPIO_PIN(CFG_UART_TX_PORT), GET_DEBUG_GPIO_MODE(CFG_UART_TX_PORT));
	else
		GPIO_PortBModeSet(GET_DEBUG_GPIO_PIN(CFG_UART_TX_PORT), GET_DEBUG_GPIO_MODE(CFG_UART_TX_PORT));
#ifdef CFG_FUNC_SHELL_EN
	if(GET_DEBUG_GPIO_PORT(CFG_FUNC_SHELL_RX_PORT) == DEBUG_GPIO_PORT_A)
		GPIO_PortAModeSet(GET_DEBUG_GPIO_PIN(CFG_FUNC_SHELL_RX_PORT), GET_DEBUG_GPIO_MODE(CFG_FUNC_SHELL_RX_PORT));
	else
		GPIO_PortBModeSet(GET_DEBUG_GPIO_PIN(CFG_FUNC_SHELL_RX_PORT), GET_DEBUG_GPIO_MODE(CFG_FUNC_SHELL_RX_PORT));
#endif
	if(InitBandRate)
	{
		DbgUartInit(GET_DEBUG_GPIO_UARTPORT(CFG_UART_TX_PORT), CFG_UART_BANDRATE, 8, 0, 1);
	}
#endif
}

#ifdef CFG_IDLE_MODE_DEEP_SLEEP
extern void FshcClkSwitch(FSHC_CLK_MODE ClkSrc, uint32_t flash_clk);
void SleepMain(void)
{
	WDG_Feed();

#ifdef CHIP_USE_DCDC
	ldo_switch_to_dcdc(3); // 3-1.6V Default:1.6V
#else
	Power_LDO16Config(0);
#endif

	Clock_UARTClkSelect(RC_CLK_MODE);//先切换log clk。避免后续慢速处理
	LogUartConfig(TRUE); //scan不打印时 可屏蔽

	SpiFlashInit(80000000, MODE_1BIT, 0, FSHC_PLL_CLK_MODE);//rc时钟 不支持flash 4bit，系统恢复时重配。
	FshcClkSwitch(FSHC_RC_CLK_MODE, 80000000);//再切RC
	Clock_DeepSleepSysClkSelect(RC_CLK_MODE, FSHC_RC_CLK_MODE, 1);
	Clock_PllClose();
	Clock_APllClose();//APLL会增加约980uA的功耗
#if !defined(CFG_RES_RTC_EN)
	Clock_HOSCDisable();//若有RTC应用并且RTC所用时钟是HOSC，则不关闭HOSC，即24M晶振
#endif
//	Clock_LOSCDisable(); //若有RTC应用并且RTC所用时钟是LOSC，则不关闭LOSC，即32K晶振
}

void WakeupMain(void)
{
	WDG_Feed();

	Chip_Init(1);
	SystemClockInit(TRUE);
	LogUartConfig(TRUE);//调整时钟后，重配串口前不要打印。

	SysTickInit();
}
#endif


/*****************************************************************
 * BT EM Size
 *****************************************************************/
#ifdef CFG_APP_BT_MODE_EN
extern uint32_t bt_em_size(void);
void bt_em_size_init(void)
{
	uint32_t bt_em_mem;

	bt_em_mem = bt_em_size();
	APP_DBG("BB_EM_SIZE=%d,EM_BT_END=%d\n", (int)BB_EM_SIZE, (int)bt_em_mem);
	if(bt_em_mem%4096)
	{
		bt_em_mem = ((bt_em_mem/4096)+1)*4096;
	}
	if(bt_em_mem > BB_EM_SIZE)
	{
		APP_DBG("bt em config error!\nyou must check BB_EM_SIZE\n%s%u \n",__FILE__,__LINE__);
		while(1);
	}
	else
	{
		APP_DBG("bt em size:%uKB\n", (unsigned int)bt_em_mem/1024);
	}

}
#endif

/*****************************************************************
 * main function
 *****************************************************************/
int main(void)
{
	uint16_t RstFlag = 0;
//	extern char __sdk_code_start;

	Chip_Init(1);
	Chip_MemInit();
	WDG_Enable(WDG_STEP_4S);
//	WDG_Disable();

	RstFlag = Reset_FlagGet();
	Reset_FlagClear();

	//如果需要使用NVM内存时，需要调用该API，第一次系统上电需要清除对NVM内存清零操作，在breakpoint 内实施。
	PMU_NVMInit();

	SystemClockInit(TRUE);

	LogUartConfig(TRUE);

#ifdef CFG_FUNC_USBDEBUG_EN
	extern void usb_hid_printf_init(void);
	usb_hid_printf_init();
#endif
	
	DBUS_Access_Area_Init(0);//设置Databus访问区间为codesize
	
	//考虑到大容量的8M flash，写之前需要Unlock，SDK默认不做加锁保护
	//用户为了增加flash 安全性，请根据自己flash大小和实际情况酌情做flash加锁保护机制
//	SpiFlashIOCtrl(IOCTL_FLASH_PROTECT, FLASH_LOCK_RANGE_HALF);//加锁保护code区域
	prvInitialiseHeap();

	osSemaphoreMutexCreate();//硬件串口OS启用了软件锁，必须在创建锁之后输出log，否则死机，锁要初始化堆栈后创建。软件模拟串口不影响。

#ifdef CFG_SOFT_POWER_KEY_EN
	SoftPowerInit();
	WaitSoftKey();
#endif

	NVIC_EnableIRQ(SWI_IRQn);
	GIE_ENABLE();	//开启总中断
#ifdef CFG_FUNC_LED_REFRESH
	//默认优先级为0，旨在提高刷新速率，特别是断点记忆等写flash操作有影响刷屏，必须严格遵守所有timer6中断调用都是TCM代码，含调用的driver库代码
	//已确认GPIO_RegOneBitSet、GPIO_RegOneBitClear在TCM区，其他api请先确认。
	NVIC_SetPriority(Timer6_IRQn, 0);
 	Timer_Config(TIMER6,1000,0);
 	Timer_Start(TIMER6);
 	NVIC_EnableIRQ(Timer6_IRQn);

 	//此行代码仅仅用于延时，配合Timer中断处理函数，客户一定要做修改调整
 	//GPIO_RegOneBitSet(GPIO_A_OE, GPIO_INDEX2);//only test，user must modify
#endif

#ifdef CFG_FUNC_DISPLAY_EN
 	DispInit(0);
#endif

	APP_DBG("\n");
	APP_DBG("****************************************************************\n");
	APP_DBG("|                    MVsB5_BT_Audio_SDK                        |\n");
	APP_DBG("|            Mountain View Silicon Technology Co.,Ltd.         |\n");
	APP_DBG("|            SDK Version: %d.%d.%d                                |\n", CFG_SDK_MAJOR_VERSION, CFG_SDK_MINOR_VERSION, CFG_SDK_PATCH_VERSION);
	APP_DBG("****************************************************************\n");
	APP_DBG("sys clk =%ld\n",Clock_SysClockFreqGet());

#if defined(CFG_DOUBLE_KEY_EN) && !defined(CFG_AI_DENOISE_EN)
	{
		extern void AlgUserDemo(void);
		AlgUserDemo();
	}
#endif

#ifdef CFG_APP_IDLE_MODE_EN
 	IdleModeConfig();
#endif
	flash_table_init();
	sys_parameter_init();
#ifdef CFG_APP_BT_MODE_EN
	bt_em_size_init();
#endif

	APP_DBG("RstFlag = %x\n", RstFlag);
	APP_DBG("Audio Decoder Version: %s\n", (unsigned char *)audio_decoder_get_lib_version());
	APP_DBG("Audio Effect  Lib Version: %s\n", (char *)effect_lib_version_return());
	APP_DBG("Roboeffect  Lib Version: %s\n", ROBOEFFECT_LIB_VER);
	APP_DBG("Driver Version: %s\n", GetLibVersionDriver());
#ifdef CFG_FUNC_LRC_EN
    APP_DBG("Lrc Version: %s\n", GetLibVersionLrc()); 
#endif
#ifdef CFG_APP_BT_MODE_EN
    APP_DBG("BtLib Version: %s\n", (unsigned char *)GetLibVersionBt());
#endif
#ifdef CFG_RES_FLASHFS_EN
	APP_DBG("FlashFSLib Version: %s\n", (unsigned char *)GetLibVersionFlashFS());
#endif
	APP_DBG("Fatfs presearch acc Lib Version: %s\n", (unsigned char *)GetLibVersionFatfsACC());
#ifdef CFG_FUNC_ALARM_EN
	APP_DBG("RTC Version: %s\n", GetLibVersionRTC());//bkd 
#endif
	APP_DBG("ECO Flag: %x\n",Read_ChipECO_Version());
	APP_DBG("\n");

#ifdef USE_EXTERN_FLASH_SPACE
	SPIM_Init(0, 0);
	SPI_Flash_Init();
	APP_DBG("SPI_Flash_ReadMID=%x\n",SPI_Flash_ReadMID());
#endif

#ifdef LED_IO_TOGGLE
	LedPortInit();  //在此之后可以使用LedOn LedOff 观测 调试时序 特别是逻分
#endif
#ifdef CFG_FUNC_RTC_EN
	RTC_ServiceInit(RstFlag&0x01);
	RTC_IntEnable();
	NVIC_EnableIRQ(RTC_IRQn);
#endif

	__nds32__mtsr(0,NDS32_SR_PFMC0);
	__nds32__mtsr(1,NDS32_SR_PFM_CTL);
#ifdef VD51_REDMINE_13199
	RgbLedInit();
#endif

	MainAppTaskStart();
	vTaskStartScheduler();

	while(1);

}

