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
	UsbAudioTimer1msProcess(); //1ms�жϼ��
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

	//ʾ�����룬��Ҫ��ӶΡ�.tcm_section��
	//�ؼ���    __attribute__((section(".tcm_section")))
	//�ͻ���Ҫ���Լ���ʵ�ֵ�API������ӹؼ���
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
	Clock_APllLock(SYS_CORE_APLL_FREQ);//����ʹ��APLL 240M
#endif

	Clock_SysClkSelect(SYS_CORE_CLK_SELECT);

	//Note: USB��UARTʱ������DPLL��APLL������ͬһ��ʱ��,����UART���Ե���ѡ��RC
	Clock_USBClkSelect(SYS_USB_CLK_SELECT);
	Clock_UARTClkSelect(SYS_UART_CLK_SELECT);

	Clock_SpdifClkSelect(SYS_SPDIF_CLK_SELECT);
	//ģ��ʱ��ʹ������
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
//	Power_DeepSleepLDO12ConfigTest(0,5,0);//����deepsleep��1V2 ����Ϊ1.0V���Ը���ʡ�硣
	Clock_UARTClkSelect(RC_CLK_MODE);//���л�log clk������������ٴ���
	LogUartConfig(TRUE); //scan����ӡʱ ������
	SysTickDeInit();
//	Efuse_ReadDataDisable(); //////////�ر�EFUSE////////
	SpiFlashInit(80000000, MODE_1BIT, 0, FSHC_PLL_CLK_MODE);//rcʱ�� ��֧��flash 4bit��ϵͳ�ָ�ʱ���䡣
	FshcClkSwitch(FSHC_RC_CLK_MODE, 80000000);//����RC
	Clock_DeepSleepSysClkSelect(RC_CLK_MODE, FSHC_RC_CLK_MODE, 1);
	Clock_PllClose();
	Clock_APllClose();//APLL������Լ980uA�Ĺ���
#if !defined(CFG_RES_RTC_EN)
	Clock_HOSCDisable();//����RTCӦ�ò���RTC����ʱ����HOSC���򲻹ر�HOSC����24M����
#endif
//	Clock_LOSCDisable(); //����RTCӦ�ò���RTC����ʱ����LOSC���򲻹ر�LOSC����32K����

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
	LogUartConfig(TRUE);//����ʱ�Ӻ����䴮��ǰ��Ҫ��ӡ��
	//Clock_Pll5ClkDivSet(8);// 
	//B0B1ΪSW��ʽ�˿ڣ��ڵ��Խ׶���ϵͳ�����˵͹���ģʽʱ�ر���GPIO����ģʽ�����ڴ˴�����
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

	//�����Ҫʹ��NVM�ڴ�ʱ����Ҫ���ø�API����һ��ϵͳ�ϵ���Ҫ�����NVM�ڴ������������breakpoint ��ʵʩ��
	//PMU_NVMInit();//BACKUP_NVMInit();

	//Power_DeepSleepLDO12Config(1);//Power_LDO12Config(1250);	//ʹ��320M��Ƶʱ��Ҫ������1.25v��ʹ��288M��������ε����� //ZSQ

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
	
	DBUS_Access_Area_Init(0);//����Databus��������Ϊcodesize

	//Clock_RC32KClkDivSet(Clock_RcFreqGet(TRUE) / 32000);//��������//ZSQ
	
	//���ǵ���������8M flash��д֮ǰ��ҪUnlock��SDKĬ�ϲ�����������
	//�û�Ϊ������flash ��ȫ�ԣ�������Լ�flash��С��ʵ�����������flash������������
//	SpiFlashIOCtrl(IOCTL_FLASH_PROTECT, FLASH_LOCK_RANGE_HALF);//��������code����
	prvInitialiseHeap();

	osSemaphoreMutexCreate();//Ӳ������OS������������������ڴ�����֮�����log��������������Ҫ��ʼ����ջ�󴴽������ģ�⴮�ڲ�Ӱ�졣
	
#ifdef CFG_RES_RTC_EN
	#ifdef CFG_CHIP_BP1064L2
    RTC_ClockSrcSel(OSC_32K);//�˺����Ĳ���ѡ����������ϵͳ��ʼ��ѡ��ľ��񣨡�Clock_Config()��������һ��
	#else
	RTC_ClockSrcSel(OSC_24M);
	#endif
	
	RTC_IntDisable();//Ĭ�Ϲر�RTC�ж�
	RTC_IntFlagClear();
	RTC_WakeupDisable();	
		
#endif

#ifdef CFG_SOFT_POWER_KEY_EN
	SoftPowerInit();
	WaitSoftKey();
#endif

	NVIC_EnableIRQ(SWI_IRQn);
	GIE_ENABLE();	//�������ж�
#ifdef CFG_FUNC_LED_REFRESH
	//Ĭ�����ȼ�Ϊ0��ּ�����ˢ�����ʣ��ر��Ƕϵ�����дflash������Ӱ��ˢ���������ϸ���������timer6�жϵ��ö���TCM���룬�����õ�driver�����
	//��ȷ��GPIO_RegOneBitSet��GPIO_RegOneBitClear��TCM��������api����ȷ�ϡ�
	NVIC_SetPriority(Timer6_IRQn, 0);
 	Timer_Config(TIMER6,1000,0);
 	Timer_Start(TIMER6);
 	NVIC_EnableIRQ(Timer6_IRQn);

 	//���д������������ʱ�����Timer�жϴ��������ͻ�һ��Ҫ���޸ĵ���
 	//GPIO_RegOneBitSet(GPIO_A_OE, GPIO_INDEX2);//only test��user must modify
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
	LedPortInit();  //�ڴ�֮�����ʹ��LedOn LedOff �۲� ����ʱ�� �ر����߷�
#endif
#ifdef CFG_FUNC_RTC_EN
#ifdef CFG_RES_RTC_EN
	RTC_IntEnable();
	NVIC_EnableIRQ(Rtc_IRQn);
#endif
	RTC_ServiceInit(RstFlag&0x01);
#endif
	RTC_SecGet();//OWEN: ����ȥ��

	__nds32__mtsr(0,NDS32_SR_PFMC0);
	__nds32__mtsr(1,NDS32_SR_PFM_CTL);
#ifdef VD51_REDMINE_13199
	RgbLedInit();
#endif

	MainAppTaskStart();
	vTaskStartScheduler();

	while(1);

}

