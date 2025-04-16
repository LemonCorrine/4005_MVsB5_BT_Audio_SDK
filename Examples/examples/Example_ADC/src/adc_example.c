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

extern void __c_init_rom(void);
extern void PMU_PowerkeyPullup22K(void);
extern void PMU_PowerkeySarADCEn(void);

#define SARADC_REF_VDD_SEL          ADC_VREF_VDDA
#define SARADC_REF_VDD_VOLTAGE      3340 //SARADC_REF_VDD_SEL real voltage
#define SARADC_SAMPLE_COUNT         3
void SarADC_ExampleLoop();

static uint8_t DmaChannelMap[6] = {
    PERIPHERAL_ID_ADC,
    255,
    255,
    255,
    255,
    255
};

void SarADC_SingleModeInit()
{
    Clock_Module1Enable(ADC_CLK_EN);

    ADC_Enable();
    ADC_ClockDivSet(10);        // 20��Ƶ
    ADC_VrefSet(ADC_VREF_VDDA); // 1:VDDA; 0:VDD
    ADC_ModeSet(ADC_CON_SINGLE);

    ADC_Calibration(); // �ϵ�У׼һ�μ���
}

int SarADC_GetChannelVoltageByADCData(uint32_t channel, int value)
{
    uint8_t multipler = 1;
    /*VBAT & DVDD33 channel sample data was 50% of real value�� need multiple 2*/
    if (channel == ADC_CHANNEL_VBAT || channel == ADC_CHANNEL_DVDD33)
    {
        multipler = 2;
    }

    value = (value * SARADC_REF_VDD_VOLTAGE) * multipler / 4095;
    return value;
}

/*ͨ�������̶��Ĵ���SARADC_SAMPLE_COUNT����߲���׼ȷ��*/
int16_t SarADC_GetChannelVoltage(uint32_t channel)
{
    int DC_Data1 = 0;
    int i = 0;

    ADC_SingleModeDataGet(channel); // abandon the first sample value
    while (i++ < SARADC_SAMPLE_COUNT)
    {
        DC_Data1 +=  ADC_SingleModeDataGet(channel);
    }
    DC_Data1 = DC_Data1 / SARADC_SAMPLE_COUNT;
    printf("[%s]: Sample = %d\n", __FUNCTION__, DC_Data1);


    DC_Data1 = SarADC_GetChannelVoltageByADCData(channel, DC_Data1);
    printf("[%s]: Voltage = %d mV\n", __FUNCTION__, DC_Data1);

    return (int16_t)DC_Data1;
}

//��ȡ��Ե�ѹ����ֵ��Ϊ������ֵ
void SarADC_DCSingleMode(void)
{
	uint16_t DC_Data = 0;

    DBG("SarADC DC example\n");
	//GPIOA23��ΪADC������������,����SarADC�İ���
	//BP15�������ϰ��²�ͬ�İ������ڻ��ӡ�����ͬ�ĵ�ѹ����ֵ
    SarADC_SingleModeInit();
    GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX23);   // channel

    while (1)
    {
        DC_Data = SarADC_GetChannelVoltage(ADC_CHANNEL_AD1_A23);
        DBG("DC_Data = %dmV\n", DC_Data);
        DelayMs(1000);
    }
}

//LDOIN����Ϊ���Ե�ѹ,������ֵ�Ѿ�ת��Ϊ��ѹֵ����λmv
void SarADC_DCSingleMode_VDD33D(void)
{
	uint32_t i = 0;
	uint16_t DC_Data = 0;

	DBG("SarADC DC example for LDO\n");

    SarADC_SingleModeInit();

    for (; i < 10; i++)
    {
        DC_Data = SarADC_GetChannelVoltage(ADC_CHANNEL_DVDD33);
        DBG("LDOVDD33D: = %d mV\n", DC_Data);
        DelayMs(500);
    }
}

//��ȡPowerkey��ѹ����ֵ��Ϊ������ֵ
void SarADC_DCSingleMode_PowerKey(void)
{
	uint16_t DC_Data = 0;

    // ʹ��PowerKey ����ADC��������
    PMU_PowerkeyPullup22K(); // PowerKey����������������Ϊ22K
    PMU_PowerkeySarADCEn();
    SarADC_SingleModeInit(); // PowerKey ��ʼ���Ѿ�����

    while (1)
    {
        DC_Data = SarADC_GetChannelVoltage(ADC_CHANNEL_BK_ADKEY);
        DBG("DC_Data = %d mV\n", DC_Data);
        DelayMs(1000);
    }
}

static uint16_t DcData[100] = {0};
uint32_t DataDoneFlag;
static void InterruptCallBack()//0
{
	DMA_InterruptFlagClear(PERIPHERAL_ID_ADC, DMA_DONE_INT);
	ADC_ContinuModeStop();
	DataDoneFlag = TRUE;
}

void SarADC_DcContinuousModeWithDMA(void)
{
	uint16_t i;

    DBG("Start continuous mode with DMA!\n");

    DataDoneFlag = FALSE;
	DMA_ChannelAllocTableSet(DmaChannelMap);

	ADC_ClockDivSet(12);
	ADC_VrefSet(1);//1:VDDA; 0:VDD
	ADC_Enable();
	ADC_ModeSet(ADC_CON_CONTINUA);

	DMA_ChannelDisable(PERIPHERAL_ID_ADC);
	DMA_BlockConfig(PERIPHERAL_ID_ADC);
	GIE_ENABLE();
	DMA_InterruptFunSet(PERIPHERAL_ID_ADC, DMA_DONE_INT, InterruptCallBack);
	DMA_InterruptEnable(PERIPHERAL_ID_ADC, DMA_DONE_INT, 1);

	ADC_DMAEnable();
	DMA_BlockBufSet(PERIPHERAL_ID_ADC, DcData, sizeof(DcData));
	DMA_ChannelEnable(PERIPHERAL_ID_ADC);

	GPIO_RegOneBitSet(GPIO_A_ANA_EN, GPIO_INDEX28);//channel
    ADC_ContinuModeStart(ADC_CHANNEL_AD3_A28);
    while(1)
	{
		if(DataDoneFlag == TRUE)
		{
			for(i=0; i<sizeof(DcData)/2; i++)
			{
                DBG("%d, sample = %d, voltage = %dmV\n", i, DcData[i], SarADC_GetChannelVoltageByADCData(ADC_CHANNEL_AD3_A28, DcData[i]));
            }
			break;
		}
	}
    ADC_ContinuModeStop();
}

int main(void)
{

	Chip_Init(1);
	WDG_Disable();
	__c_init_rom();

    Clock_Config(1, 24000000);
    Clock_HOSCCurrentSet(15);  // �Ӵ��˾����ƫ�õ���
    Clock_PllLock(240 * 1000); // 240MƵ��
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

	SpiFlashInit(80000000, MODE_4BIT, 0, 1);
    DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                     ADC Example                     |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");

    SarADC_ExampleLoop();
    DBG("Err!\n");
	while(1);
}

void SarADC_ExampleLoop()
{
    uint8_t Key = 0;
    DBG("/------------------SarADC_ExampleLoop---------------------\\\n");
    DBG("|      ����'a',����GPIOA23�˿���Ե�ѹ                 |\n");
    DBG("|      ����'b',����VDD33D�ľ��Ե�ѹ,��λmV             |\n");
    DBG("|      ����'c',����PowerKey�˿ڵ���Ե�ѹ              |\n");
    DBG("|      ����'d',����ģʽ����GPIOA28��Ե�ѹ             |\n");
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
                SarADC_DCSingleMode(); // ��ͨ����ѹ����
                break;
            case 'B':
            case 'b':
                SarADC_DCSingleMode_VDD33D(); // ��Դ��ѹ����
                break;
            case 'C':
            case 'c':
                SarADC_DCSingleMode_PowerKey(); // PowerKey���ù���
                break;
            case 'D':
            case 'd':
                SarADC_DcContinuousModeWithDMA();
                break;
            default:
                break;
            }
        }
    }
}
