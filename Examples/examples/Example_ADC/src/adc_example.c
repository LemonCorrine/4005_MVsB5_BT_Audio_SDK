/**
 **************************************************************************************
 * @file    adc_example.c
 * @brief   adc example
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2019-5-30 13:25:00$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "adc.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "remap.h"
#include "chip_info.h"
#include "sadc_interface.h"
#include "irqn.h"
#include "powercontroller.h"

extern void __c_init_rom(void);
extern void PMU_PowerkeyPullup22K(void);
extern void PMU_PowerkeySarADCEn(void);

#define SARADC_REF_VDD_SEL          ADC_VREF_VDDA
#define SARADC_REF_VDD_VOLTAGE      3300 //SARADC_REF_VDD_SEL real voltage
#define SARADC_SAMPLE_COUNT         3

#define MAX_DMA_CHANNEL         6  //DMA通道最大数量,不能修改

void SarADC_ExampleLoop();

static uint8_t DmaChannelMap[MAX_DMA_CHANNEL] = {
    PERIPHERAL_ID_ADC,
    255,
    255,
    255,
    255,
    255
};


void SarADC_SingleModeInit()
{
    ADC_ModeSet(ADC_CON_SINGLE);
}

int SarADC_GetChannelVoltageByADCData(uint8_t channel, int value)
{
    uint8_t multipler = 1;
    /*VBAT & DVDD33 channel sample data was 50% of real value， need multiple 2*/
    if (channel == ADC_CHANNEL_VBAT || channel == ADC_CHANNEL_DVDD33)
    {
        multipler = 2;
    }

    value = (value * SARADC_REF_VDD_VOLTAGE) * multipler / 4095;
    return value;
}

/*通过采样固定的次数SARADC_SAMPLE_COUNT来提高采样准确性*/
int16_t SarADC_GetChannelVoltage(uint8_t channel)
{
    int DC_Data1 = 0;
    int i = 0;

    ADC_SingleModeDataGet(channel); // abandon the first sample value
    while (i++ < SARADC_SAMPLE_COUNT)
    {
        DC_Data1 +=  ADC_SingleModeDataGet(channel);
    }
    DC_Data1 = DC_Data1 / SARADC_SAMPLE_COUNT;
    DBG("[%s]: Sample = %d\n", __FUNCTION__, DC_Data1);


    DC_Data1 = SarADC_GetChannelVoltageByADCData(channel, DC_Data1);
    DBG("[%s]: Voltage = %d mV\n", __FUNCTION__, DC_Data1);

    return (int16_t)DC_Data1;
}

//获取相对电压采样值，为采样数值
void SarADC_DCSingleMode(void)
{
	uint16_t DC_Data = 0;

    DBG("SarADC DC example\n");
	//GPIOA30做为ADC采样输入引脚,接上SarADC的按键
	//BP15开发板上按下不同的按键串口会打印输出不同的电压采样值
    SarADC_SingleModeInit();
    GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX30); // channel

    while (1)
    {
        DC_Data = SarADC_GetChannelVoltage(ADC_CHANNEL_AD5_A30);
        DBG("DC_Data = %dmV\n", DC_Data);
        DelayMs(1000);
    }
}

//LDOIN采样为绝对电压,返回数值已经转换为电压值，单位mv
void SarADC_DCSingleMode_VDD33D(void)
{
	uint32_t i = 0;
	uint16_t DC_Data = 0;

	DBG("SarADC DC example for LDO\n");

    SarADC_SingleModeInit();

    for (i = 0; i < 10; i++)
    {
        DC_Data = SarADC_GetChannelVoltage(ADC_CHANNEL_DVDD33);
        DBG("LDOVDD33D: = %d mV\n", DC_Data);
        DelayMs(500);
    }
}

uint16_t SarADC_GetPowerKeyVoltage(uint16_t sampleVoltage)
{
#define MUL_COEFF 94867 // 19 * 4993 = 94867
#define SHIFT_BITS 20   // 2^20 = 1048576

    const uint16_t VDD33 = Power_GetLDO33DConfigVoltage();

    int32_t diff = (int32_t)VDD33 - sampleVoltage;

    int32_t temp = (diff * MUL_COEFF) >> SHIFT_BITS;

    // 防止 temp > sampleVoltage 导致 uint16_t 减法溢出
    if (temp >= (int32_t)sampleVoltage)
    {
        return 0;
    }
    return (uint16_t)(sampleVoltage - temp);
}

//获取Powerkey电压采样值，为采样数值
void SarADC_DCSingleMode_PowerKey(void)
{
	uint16_t DC_Data = 0;

    // 使能PowerKey 复用ADC采样功能
    PMU_PowerkeyPullup22K(); // PowerKey内置上拉电阻配置为22K
    PMU_PowerkeySarADCEn();
    SarADC_SingleModeInit(); // PowerKey 初始化已经配置

    while (1)
    {
        DC_Data = SarADC_GetChannelVoltage(ADC_CHANNEL_BK_ADKEY);
        DC_Data = SarADC_GetPowerKeyVoltage(DC_Data);
        DBG("PowerKey: = %d mV\n", DC_Data);
        DelayMs(1000);
    }
}

volatile bool DataDoneFlag;
static void ADCDMADoneIntCallBack(void)
{
    DMA_InterruptFlagClear(PERIPHERAL_ID_ADC, DMA_DONE_INT);
    DataDoneFlag = TRUE;
}

void SarADC_ConfigDMABuf(uint16_t *pADCDataBuf, uint16_t bufLen)
{
    DMA_BlockConfig(PERIPHERAL_ID_ADC);
    memset(pADCDataBuf, 0, bufLen);
    DMA_BlockBufSet(PERIPHERAL_ID_ADC, pADCDataBuf, bufLen);
    DMA_InterruptFunSet(PERIPHERAL_ID_ADC, DMA_DONE_INT, ADCDMADoneIntCallBack);
    DMA_InterruptEnable(PERIPHERAL_ID_ADC, DMA_DONE_INT, 1);
}

static uint16_t DcData[10] = {0};
void SarADC_DcContinuousModeWithDMA(void)
{
	uint16_t i;

    DBG("Start continuous mode with DMA!\n");

    DataDoneFlag = FALSE;
	DMA_ChannelAllocTableSet(DmaChannelMap);

    GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX28); // channel
    SarADC_ConfigDMABuf(DcData, sizeof(DcData));

    /*Start continuous mode with DMA*/
    ADC_ModeSet(ADC_CON_CONTINUA);
    ADC_DMADisable();
    DMA_ChannelDisable(PERIPHERAL_ID_ADC);
    ADC_ContinuModeStart(ADC_CHANNEL_AD3_A28);
	DMA_ChannelEnable(PERIPHERAL_ID_ADC);
    ADC_DMAEnable();

    while(1)
	{
		if(DataDoneFlag == TRUE)
		{
            ADC_ContinuModeStop();
            for (i = 0; i < sizeof(DcData) / sizeof(DcData[0]); i++)
			{
                DBG("%d, sample = %d, voltage = %dmV\n", i, DcData[i], SarADC_GetChannelVoltageByADCData(ADC_CHANNEL_AD3_A28, DcData[i]));
            }
			break;
		}
	}
}

#define AUTO_LOAD_SAMPLE_COUNT 1
static uint16_t autoLoadDcData[8*AUTO_LOAD_SAMPLE_COUNT] = {0};
#define GET_SAMPLE(ADCOutData) ((ADCOutData) & 0x0FFF)

/*注意配置通道总数不要超过ADC_AUTOLOAD_MAX_CHANNEL_NUM*/
static uint8_t g_autoLoadChannels[8] =
{
        ADC_CHANNEL_AD1_A23,
        ADC_CHANNEL_DVDD33,
        ADC_CHANNEL_AD2_A24,
        ADC_CHANNEL_VDD15,
        ADC_CHANNEL_AD5_A30,
        ADC_CHANNEL_VBAT,
        ADC_CHANNEL_AD6_A31,
        ADC_CHANNEL_DVDD12,
};

void SarADC_StartAutoLoadChannels(uint8_t *multiChannelTable, uint16_t channelNum)
{
    ADC_ModeSet(ADC_CON_CONTINUA);

    ADC_DMADisable();
    DMA_ChannelDisable(PERIPHERAL_ID_ADC);
    DMA_ChannelEnable(PERIPHERAL_ID_ADC);
    ADC_AutoLoadModeStart(multiChannelTable, channelNum);
    ADC_DMAEnable();
}

void SarADC_DcAutoLoadModeWithDMA_8ch(void)
{
    uint16_t i;
    uint16_t* pAutoLoadDcData = NULL;
    uint16_t channelNum = 0;

    DBG("Start continuous mode with DMA!\n");
    /*Init GPIO for ADC channels */
    GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX23);
    GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX24);
    GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX30);
    GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX31);


    DataDoneFlag = FALSE;
    DMA_ChannelAllocTableSet(DmaChannelMap);
    SarADC_ConfigDMABuf(autoLoadDcData, sizeof(autoLoadDcData));

    channelNum = sizeof(g_autoLoadChannels) / sizeof(g_autoLoadChannels[0]);
    channelNum = channelNum > ADC_AUTOLOAD_MAX_CHANNEL_NUM ? ADC_AUTOLOAD_MAX_CHANNEL_NUM : channelNum;
    SarADC_StartAutoLoadChannels(g_autoLoadChannels, channelNum);

    while (1)
    {
        if (DataDoneFlag == TRUE)
        {
            ADC_AutoLoadModeStop();
            ADC_ContinuModeStop();
            pAutoLoadDcData = autoLoadDcData;
            for (i = 0; i < AUTO_LOAD_SAMPLE_COUNT; i++)
            {
                uint8_t ch;
                DBG("=================%d=====================\n", i);
                for (ch = 0; ch < channelNum; ch++)
                {
                    uint16_t sample = GET_SAMPLE(pAutoLoadDcData[ch]);
                    DBG("ch%d, sample = %d, voltage = %dmV\n",
                        ch, sample,
                        SarADC_GetChannelVoltageByADCData(g_autoLoadChannels[ch], sample));
                }
                pAutoLoadDcData += channelNum;
            }
            DataDoneFlag = FALSE;
            DelayMs(1000);
            SarADC_StartAutoLoadChannels(g_autoLoadChannels, channelNum);
        }
    }
}

void Init_SarADC(void)
{
    /*ADC初始化*/
    Clock_Module1Enable(ADC_CLK_EN);
    ADC_Enable();
    ADC_ClockDivSet(Clock_SysClockFreqGet() / 1000000 / 12 / 2); // 12m
    ADC_VrefSet(ADC_VREF_VDDA);                                  // 1:VDDA; 0:VDD
    ADC_ModeSet(ADC_CON_SINGLE);
    ADC_Calibration(); // 上电校准一次即可
}

int main(void)
{

	Chip_Init(1);
	WDG_Disable();
	__c_init_rom();

    Clock_Config(1, 24000000);
    Clock_HOSCCurrentSet(31);  // 加大了晶体的偏置电流
    Clock_PllLock(240 * 1000); // 240M频率
    Clock_APllLock(240 * 1000);
    Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
    Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
    Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);
    Clock_SysClkSelect(PLL_CLK_MODE);
    Clock_UARTClkSelect(PLL_CLK_MODE);
    Clock_HOSCCurrentSet(5);

//	Clock_Timer3ClkSelect(SYSTEM_CLK_MODE);

    GPIO_PortAModeSet(GPIOA9, 1);  // Rx, A9:uart1_rxd_1
    GPIO_PortAModeSet(GPIOA10, 5); // Tx, A10:uart1_txd_1
    DbgUartInit(1, 2000000, 8, 0, 1);

    //flash驱动  remap 到内存高端地址, 与 init-default.c 中内存使用配置相一致
    Remap_InitTcm(0x0, 0x20040000-(20*1024)-DRV_REMAP_SIZE, DRV_REMAP_SIZE/1024);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);
    DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                     ADC Example                     |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    Power_LDO33DConfig(3);

    Init_SarADC();
    SarADC_ExampleLoop();
    DBG("Err!\n");
	while(1);
}

void SarADC_ExampleLoop()
{
    uint8_t Key = 0;
    DBG("/------------------SarADC_ExampleLoop---------------------\\\n");
    DBG("|      输入'a',采样GPIOA30端口相对电压                 |\n");
    DBG("|      输入'b',采样VDD33D的绝对电压,单位mV             |\n");
    DBG("|      输入'c',采样PowerKey端口的相对电压              |\n");
    DBG("|      输入'd',连续模式采样GPIOA28相对电压             |\n");
    DBG("|      输入't',自动轮询8ch的ADC采样数据                |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");
    while (1)
    {
        DBG("please press any key\n");
        if (UARTS_Recv(1, &Key, 1, 100) > 0)
        {
            switch (Key)
            {
            case 'A':
            case 'a':
                SarADC_DCSingleMode(); // 单通道电压采样
                break;
            case 'B':
            case 'b':
                SarADC_DCSingleMode_VDD33D(); // 电源电压采样
                break;
            case 'C':
            case 'c':
                SarADC_DCSingleMode_PowerKey(); // PowerKey复用功能
                break;
            case 'D':
            case 'd':
                SarADC_DcContinuousModeWithDMA();
                break;

            case 'T':
            case 't':
                SarADC_DcAutoLoadModeWithDMA_8ch();
                break;
            default:
                break;
            }
        }
    }
}
