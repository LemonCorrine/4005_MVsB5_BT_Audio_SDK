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
#if (BT_HFP_SUPPORT == ENABLE)
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

#include "bt_em_config.h"

//-----------------globle timer----------------------//
volatile uint32_t gInsertEventDelayActTimer = 2500; // ms
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
extern void Power_LDO16Config(uint8_t value);
extern void ldo_switch_to_dcdc(uint8_t trim_cfg);

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

}

void Timer2Interrupt(void)
{
	Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
#if defined(CFG_FUNC_USB_DEVICE_EN) || defined(CFG_FUNC_UDISK_DETECT)
	OTG_PortLinkCheck();
#endif

#ifdef CFG_APP_USB_AUDIO_MODE_EN
	UsbAudioTimer1msProcess(); //1ms中断监控
#endif

#ifdef CFG_APP_BT_MODE_EN
#if (BT_HFP_SUPPORT == ENABLE)
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
		Clock_HOSCCurrentSet(31);//initial set 31, after clock start set to 9
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
		Clock_HOSCCurrentSet(9);
#endif

	SpiFlashInit(SYS_FLASH_FREQ_SELECT, MODE_4BIT, 0, SYS_FLASH_CLK_SELECT);

#if (SYS_CORE_SET_MODE == CORE_ONLY_APLL_MODE)
	Clock_PllClose();
#endif
}

void LogUartConfig(bool InitBandRate)
{
#ifdef CFG_FUNC_DEBUG_EN
	if(GET_DEBUG_GPIO_PORT(CFG_UART_TX_PORT) == DEBUG_GPIO_PORT_A)
		GPIO_PortAModeSet(1 << GET_DEBUG_GPIO_INDEX(CFG_UART_TX_PORT), GET_DEBUG_GPIO_MODE(CFG_UART_TX_PORT));
	else
		GPIO_PortBModeSet(1 << GET_DEBUG_GPIO_INDEX(CFG_UART_TX_PORT), GET_DEBUG_GPIO_MODE(CFG_UART_TX_PORT));
	if(InitBandRate)
	{
		DbgUartInit(GET_DEBUG_GPIO_UARTPORT(CFG_UART_TX_PORT), CFG_UART_BANDRATE, 8, 0, 1);
	}
#else
	DbgUartInit(1, 0, 8, 0, 1);
#endif
}

#ifdef CFG_IDLE_MODE_DEEP_SLEEP
extern void FshcClkSwitch(FSHC_CLK_MODE ClkSrc, uint32_t flash_clk);
void SleepMain(void)
{
//	Power_DeepSleepLDO12ConfigTest(0,5,0);//进入deepsleep后将1V2 调整为1.0V可以更加省电。
	Clock_UARTClkSelect(RC_CLK_MODE);//先切换log clk。避免后续慢速处理
	LogUartConfig(TRUE); //scan不打印时 可屏蔽
	SysTickDeInit();
//	Efuse_ReadDataDisable(); //////////关闭EFUSE////////
	SpiFlashInit(80000000, MODE_1BIT, 0, FSHC_PLL_CLK_MODE);//rc时钟 不支持flash 4bit，系统恢复时重配。
	FshcClkSwitch(FSHC_RC_CLK_MODE, 80000000);//再切RC
	Clock_DeepSleepSysClkSelect(RC_CLK_MODE, FSHC_RC_CLK_MODE, 1);
	Clock_PllClose();
	Clock_APllClose();//APLL会增加约980uA的功耗
#if !defined(CFG_RES_RTC_EN)
	Clock_HOSCDisable();//若有RTC应用并且RTC所用时钟是HOSC，则不关闭HOSC，即24M晶振
#endif
//	Clock_LOSCDisable(); //若有RTC应用并且RTC所用时钟是LOSC，则不关闭LOSC，即32K晶振

	SysTickInit();
}

void WakeupMain(void)
{
	Chip_Init(1);
	SysTickDeInit();
	WDG_Enable(WDG_STEP_4S);
	SystemClockInit(FALSE);
	SysTickInit();
//	Efuse_ReadDataEnable();
	LogUartConfig(TRUE);//调整时钟后，重配串口前不要打印。
	//Clock_Pll5ClkDivSet(8);// 
	//B0B1为SW调式端口，在调试阶段若系统进入了低功耗模式时关闭了GPIO复用模式，请在此处开启
	//GPIO_PortBModeSet(GPIOB0, 0x3); //B0 sw_clk(i) 
	//GPIO_PortBModeSet(GPIOB1, 0x4); //B1 sw_d(io)
	//APP_DBG("Main:wakeup\n");
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
	WDG_Enable(WDG_STEP_4S);
//	WDG_Disable();

//#if FLASH_BOOT_EN
//	RstFlag = Reset_FlagGet_Flash_Boot();
//#else
	RstFlag = Reset_FlagGet();
	Reset_FlagClear();
//#endif

	//如果需要使用NVM内存时，需要调用该API，第一次系统上电需要清除对NVM内存清零操作，在breakpoint 内实施。
	//PMU_NVMInit();//BACKUP_NVMInit();

	//Power_DeepSleepLDO12Config(1);//Power_LDO12Config(1250);	//使用320M主频时需要提升到1.25v，使用288M则可以屏蔽掉这行 //ZSQ

	SystemClockInit(TRUE);

#ifdef CHIP_USE_DCDC
//	ldo_switch_to_dcdc(7);  //3-1.9V;7-1.8;12-1.7V;18-1.6V;27-1.5V;39-1.4V;57-1.3V 18:Default:1.6V //max power
	ldo_switch_to_dcdc(27); // 3-1.9V;7-1.8;12-1.7V;18-1.6V;27-1.5V;39-1.4V;57-1.3V 18:Default:1.6V //max power
#else
	Power_LDO16Config(7);// 7: 1.69V; 6:1.75; 0:1.65 default max power
#endif

//	Power_DeepSleepLDO11DConfig(0x0F); //trim LDO11 //0x2F=0.9V, 0x0F=1V

	LogUartConfig(TRUE);

#ifdef CFG_FUNC_USBDEBUG_EN
	extern void usb_hid_printf_init(void);
	usb_hid_printf_init();
#endif
	
	DBUS_Access_Area_Init(0);//设置Databus访问区间为codesize

	//Clock_RC32KClkDivSet(Clock_RcFreqGet(TRUE) / 32000);//不可屏蔽//ZSQ
	
	//考虑到大容量的8M flash，写之前需要Unlock，SDK默认不做加锁保护
	//用户为了增加flash 安全性，请根据自己flash大小和实际情况酌情做flash加锁保护机制
//	SpiFlashIOCtrl(IOCTL_FLASH_PROTECT, FLASH_LOCK_RANGE_HALF);//加锁保护code区域
	prvInitialiseHeap();

	osSemaphoreMutexCreate();//硬件串口OS启用了软件锁，必须在创建锁之后输出log，否则死机，锁要初始化堆栈后创建。软件模拟串口不影响。
	
#ifdef CFG_RES_RTC_EN
	#ifdef CFG_CHIP_BP1064L2
    RTC_ClockSrcSel(OSC_32K);//此函数的参数选择必须和上面系统初始化选择的晶振（“Clock_Config()”）保持一致
	#else
	RTC_ClockSrcSel(OSC_24M);
	#endif
	
	RTC_IntDisable();//默认关闭RTC中断
	RTC_IntFlagClear();
	RTC_WakeupDisable();	
		
#endif

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
	APP_DBG("Driver Version: %s %x\n", GetLibVersionDriver(),Read_ChipECO_Version());
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
	APP_DBG("\n");
#ifdef LED_IO_TOGGLE
	LedPortInit();  //在此之后可以使用LedOn LedOff 观测 调试时序 特别是逻分
#endif
#ifdef CFG_FUNC_RTC_EN
#ifdef CFG_RES_RTC_EN
	RTC_IntEnable();
	NVIC_EnableIRQ(Rtc_IRQn);
#endif
	RTC_ServiceInit(RstFlag&0x01);
#endif
	RTC_SecGet();//OWEN: 不能去掉

	__nds32__mtsr(0,NDS32_SR_PFMC0);
	__nds32__mtsr(1,NDS32_SR_PFM_CTL);
#ifdef VD51_REDMINE_13199
	RgbLedInit();
#endif

	MainAppTaskStart();
	vTaskStartScheduler();

	while(1);

}

