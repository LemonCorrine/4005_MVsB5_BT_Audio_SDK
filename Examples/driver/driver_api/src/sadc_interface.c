#include <string.h>
#include "type.h"
#include "adc.h"
#include "clk.h"
#include "pmu.h"
#include "powercontroller.h"
#include "debug.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#else
#define CFG_RES_POWERKEY_ADC_EN
#endif
void SarADC_Init(void)
{
	Clock_Module1Enable(ADC_CLK_EN);
	
#ifdef  CFG_RES_POWERKEY_ADC_EN
	//使能PowerKey 复用ADC采样功能
//	BACKUP_VBK22KMode();//PowerKey内置上拉电阻配置为22K
	PMU_PowerkeyPullup22K();
	PMU_PowerkeySarADCEn();
#endif

	ADC_Enable();
	ADC_ClockDivSet(4);  //120M / (2*4) = 15M
	ADC_VrefSet(ADC_VREF_VDD);//1:VDDA; 0:VDD
	ADC_ModeSet(ADC_CON_SINGLE);

	ADC_Calibration();//上电校准一次即可
}

//此处未做多次采样取平均操作
//原因：ADC采样获取结果是阻塞式，消抖滤波对系统会有影响
//请用软件状态机方式在应用层代码解决
int16_t SarADC_LDOINVolGet(void)
{
	uint32_t DC_Data1;
	uint32_t DC_Data2;

	DC_Data1 = ADC_SingleModeDataGet(ADC_CHANNEL_VBAT);//zsq changes from ADC_CHANNEL_DAC_AVDD to ADC_CHANNEL_A16_MICBIAS
	DC_Data2 = ADC_SingleModeDataGet(ADC_CHANNEL_DVDD12);//zsq from  ADC_CHANNEL_VDD1V2 to ADC_CHANNEL_DVDD12

	//DBG("\n%d, %d\n", DC_Data1, DC_Data2);
	//DBG("11D = %d\n",Power_GetLDO11DConfigVoltage());
	DC_Data1 = (DC_Data1 * 2 * Power_GetLDO11DConfigVoltage()) / DC_Data2;

	//DC_Data1 = (DC_Data1 * 2 * 1000)/(4095*1000/3300);
	//DC_Data1 = (DC_Data1 * 2 * 1000)/(4095*1000/3000);
	if(Power_GetLDO33DConfigVoltage() < 3100)
	{
		DC_Data1 += 50;//VDD参考电压偏低需要做一些修正
	}
	if(DC_Data1 < 3200)
	{
		DC_Data1 += 30;//电压低于3.3V之后，采样计算值偏低
	}
	//DBG("LDOIN ： %d\n", (int)DC_Data1);

	return (int16_t)DC_Data1;
}

