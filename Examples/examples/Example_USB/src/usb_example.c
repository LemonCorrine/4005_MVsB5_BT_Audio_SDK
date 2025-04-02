/**
 **************************************************************************************
 * @file    usb_example.c
 * @brief   usb example
 *
 * @author  owen
 * @version V1.0.0
 *
 * $Created: 2018-6-7 10:16:10$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "spi_flash.h"
#include "timeout.h"
#include "clk.h"
#include "pwm.h"
#include "delay.h"
#include "rtc.h"
#include "sd_card.h"
#include "watchdog.h"
#include "irqn.h"
#include "spi_flash.h"
#include "remap.h"
#include "otg_host_udisk.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "otg_device_stor.h"
#include "chip_info.h"
#include "otg_detect.h"
#include "otg_device_audio.h"
#include "dac.h"
#include "usb_audio_api.h"

extern void SysTickInit(void);
extern void UsbAudioMicDacInit(void);
extern void OTG_DeviceAudioInit();
extern void UsbAudioTimer1msProcess(void);
static uint8_t DmaChannelMap[] =
{
    PERIPHERAL_ID_AUDIO_ADC0_RX,
    PERIPHERAL_ID_AUDIO_ADC1_RX,
    PERIPHERAL_ID_AUDIO_DAC0_TX,
	PERIPHERAL_ID_SDIO_TX,
	PERIPHERAL_ID_SDIO_RX,
    255,
};

void Timer2Interrupt(void)
{
	Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
#ifdef CFG_APP_USB_AUDIO_MODE_EN
	UsbAudioTimer1msProcess(); //1ms中断监控
#endif
}

uint8_t read_buf[4096];
uint8_t write_buf[4096];
extern uint8_t OtgPortLinkState;

void usb_host_example(void)
{
	DBG("usb host example\n");
	OtgPortLinkState = 0;
	OTG_HostFifoInit();
	OTG_HostControlInit();
	OTG_HostEnumDevice();
	if(UDiskInit() == 1)
	{
		DBG("枚举MASSSTORAGE接口OK\n");
		UDiskReadBlock(0,write_buf,1);
		UDiskWriteBlock(0,write_buf,1);
		UDiskReadBlock(0,read_buf,1);
		if(memcmp(write_buf,read_buf,512) == 0)
		{
			DBG("读写U盘数据OK\n");
		}
		else
		{
			DBG("读写U盘数据错误\n");
		}
	}
	else
	{
		DBG("UDisk Init err\n");
	}
}

extern void audio_init(uint32_t SampleRate);
void usb_device_example(void)
{
	printf("OTG_DeviceInit\n");
	DMA_ChannelAllocTableSet(DmaChannelMap);
#if (CFG_PARA_USB_MODE >= READER)
	CardPortInit(SDIO_A15_A16_A17);
	if(SDCard_Init() != NONE_ERR)
	{
		DBG("sd card error\n");
		while(1);
	}
#else
	audio_init(CFG_PARA_SAMPLE_RATE);
	UsbDevicePlayInit();
	UsbDevicePlayResMalloc();
#endif
	OTG_DeviceModeSel(CFG_PARA_USB_MODE,USB_VID,USB_PID_BASE);
	OTG_DeviceFifoInit();
	OTG_DeviceInit();
	NVIC_EnableIRQ(Usb_IRQn);

	while(1)
	{
		OTG_DeviceRequestProcess();
#if (CFG_PARA_USB_MODE >= READER)
		OTG_DeviceStorProcess();
#endif
		UsbAudioSpeakerStreamProcess();
		UsbAudioMicStreamProcess();
	}
}

extern void USB_DelayLine(void);
extern void __c_init_rom();
int main(void)
{
	Chip_Init(1);
	WDG_Disable();
	__c_init_rom();
    Clock_Config(1, 24000000);
    Clock_HOSCCurrentSet(15);  // 加大了晶体的偏置电流
    Clock_PllLock(240 * 1000); // 240M频率
    Clock_APllLock(240 * 1000);
    Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
    Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
    Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);
    Clock_SysClkSelect(PLL_CLK_MODE);
    Clock_UARTClkSelect(PLL_CLK_MODE);
    Clock_HOSCCurrentSet(5);

	SpiFlashInit(80000000, MODE_4BIT, 0, 1);

    // BP15系列开发板启用串口，默认使用
    GPIO_PortAModeSet(GPIOA10, 5);// UART1 TX
    GPIO_PortAModeSet(GPIOA9, 1);// UART1 RX
    DbgUartInit(1, 2000000, 8, 0, 1);

	GIE_ENABLE();
	SysTickInit();

	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                    USB Example                      |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

 	Timer_Config(TIMER2,1000,0);
 	Timer_Start(TIMER2);
 	NVIC_EnableIRQ(Timer2_IRQn);

 	usb_device_example();
// 	usb_host_example();
	while(1);
}
