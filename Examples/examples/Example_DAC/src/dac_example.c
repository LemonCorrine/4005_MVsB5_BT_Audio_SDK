/**
 **************************************************************************************
 * @file    dac_example.c
 * @brief   dac example
 *
 * @author  Mike
 * @version V1.0.0
 *
 * $Created: 2024-02-19 11:30:00$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "dac.h"
#include "timer.h"
#include "i2s.h"
#include "watchdog.h"
#include "remap.h"
#include "gpio.h"
#include "chip_info.h"
#include "dac_interface.h"
#include "powercontroller.h"
#include "sine_gen.h"

extern void __c_init_rom(void);

uint32_t AudioDACBuf[1024] = {0};  // 1024 * 4 = 4K

static int32_t PcmBuf[1024] = {0};

static uint8_t DmaChannelMap[] =
{
    PERIPHERAL_ID_AUDIO_DAC0_TX,
    255,
    255,
    255,
    255,
    255,
};

// DAC演示工程，主要演示DAC配置流程
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

    DMA_ChannelAllocTableSet(DmaChannelMap);

    DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                     DAC Example                      |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.       |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

    uint16_t n;

    uint32_t SampleRate = 48000;
    uint32_t DACBitWidth = 24;

    DACParamCt ct;
    ct.DACModel = DAC_Single;
    ct.DACLoadStatus = DAC_Load;
    ct.PVDDModel = PVDD33;
    ct.DACEnergyModel = DACLowEnergy;
    ct.DACVcomModel = Disable;

    static Sine32GenContext sineGen32;
    sine24_generator_init(&sineGen32, SampleRate, 3, 1000, 1000, -200, -200);

    // DAC init
    AudioDAC_Init(&ct, SampleRate, DACBitWidth, (void *)AudioDACBuf, sizeof(AudioDACBuf), NULL, 0);

    while(1)
    {
        memset(PcmBuf, 0x00, sizeof(PcmBuf));
        n = AudioDAC0_DataSpaceLenGet();
        if(n >= 256)
        {
            sine24_generator_apply(&sineGen32, PcmBuf, PcmBuf, 256);   //生成256samples
            AudioDAC0_DataSet(PcmBuf, 256);
        }
    }
    while(1);
}
