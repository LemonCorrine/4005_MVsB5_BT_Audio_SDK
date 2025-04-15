/**
 *******************************************************************************
 * @file    adc.h
 * @brief	SarAdc module driver interface
 
 * @author  Sam
 * @version V1.0.0
 
 * $Created: 2014-12-26 14:01:05$
 * @Copyright (C) 2014, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 ******************************************************************************* 
 */

/**
 * @addtogroup ADC
 * @{
 * @defgroup adc adc.h
 * @{
 */
 
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

typedef enum __ADC_DC_CLK_DIV
{
	CLK_DIV_NONE = 0,
	CLK_DIV_2,
	CLK_DIV_4,
	CLK_DIV_8,
	CLK_DIV_16,
	CLK_DIV_32,
	CLK_DIV_64,
	CLK_DIV_128,
	CLK_DIV_256,
	CLK_DIV_512,
	CLK_DIV_1024,
	CLK_DIV_2048,
	CLK_DIV_4096
	
}ADC_DC_CLK_DIV;

//0000 :PAD_VBAT
//0001 :PAD_DVDD33
//0010 :PAD_VDD15
//0011 :PAD_DAC_AVDD
//0100 :PAD_DVDD12
//0101 :PAD_TESTA
//0110 :AD0
//0111 :AD1
//1000 :AD2
//1001 :AD3
//1010 :AD4
//1011 :AD5
//1100 :AD6
//1101 :AD7
//1110 :AD8
//1111 :BK_ADKEY
typedef enum __ADC_DC_CHANNEL_SEL
{
	ADC_CHANNEL_VBAT = 0,						/**channel 0*/
	ADC_CHANNEL_DVDD33,							/**channel 1*/
	ADC_CHANNEL_VDD15,							/**channel 2*/
	ADC_CHANNEL_DAC_AVDD,						/**channel 3*/
	ADC_CHANNEL_DVDD12,							/**channel 4*/
	ADC_CHANNEL_PAD_TESTA,						/**channel 5*/
	ADC_CHANNEL_AD0_A3A4A5A6A7A9A10A15A16A17A20A21A22,	/**channel 6*/
	ADC_CHANNEL_AD1_A23,						/**channel 7*/
	ADC_CHANNEL_AD2_A24,						/**channel 8*/
	ADC_CHANNEL_AD3_A28,						/**channel 9*/
	ADC_CHANNEL_AD4_A29,						/**channel 10*/
	ADC_CHANNEL_AD5_A30,						/**channel 11*/
	ADC_CHANNEL_AD6_A31,						/**channel 12*/
	ADC_CHANNEL_AD7_B0B1,						/**channel 13*/
	ADC_CHANNEL_AD8_A0A1A2B4B5B9,				/**channel 14*/
	ADC_CHANNEL_BK_ADKEY						/**channel 15*/
}ADC_DC_CHANNEL_SEL;


typedef enum __ADC_VREF
{
	ADC_VREF_VDD,
	ADC_VREF_VDDA,
	ADC_VREF_Extern //�ⲿ����
	
}ADC_VREF;

typedef enum __ADC_CONV
{
	ADC_CON_SINGLE,
	ADC_CON_CONTINUA
}ADC_CONV;

/**
 * @brief  ADCʹ��
 * @param  ��
 * @return ��
 */
void ADC_Enable(void);

/**
 * @brief  ADC����
 * @param  ��
 * @return ��
 */
void ADC_Disable(void);

/**
 * @brief  ADC����У׼��У׼֮������ADC��������
 * @param  ��
 * @return ��
 */
void ADC_Calibration(void);

/**
 * @brief  ����ADC����ʱ�ӷ�Ƶ
 * @param  Div ��Ƶϵ��
 * @return ��
 */
void ADC_ClockDivSet(ADC_DC_CLK_DIV Div);

/**
 * @brief  ��ȡADC����ʱ�ӷ�Ƶ
 * @param  ��
 * @return ��Ƶϵ��
 */
ADC_DC_CLK_DIV ADC_ClockDivGet(void);

/**
 * @brief  ADC�ο���ѹѡ��
 * @param  Mode �ο���ѹԴѡ��2:Extern Vref; 1:VDDA; 0: 2*VMID
 * @return ��
 */
void ADC_VrefSet(ADC_VREF Mode);

/**
 * @brief  ����ADCת������ģʽ
 * @param  Mode ����ģʽ��0:����ģʽ; 1:����ģʽ�����DMAʹ�ã�ֻ�ܲ���һ��ͨ����
 * @return ��
 */
void ADC_ModeSet(ADC_CONV Mode);

/**
 * @brief  ��ȡADC��������
 * @param  ChannalNum ADC������ͨ����
 * @return ADC��������������[0-4095]
 */
int16_t ADC_SingleModeDataGet(uint32_t ChannalNum);

/**
 * @brief  ADC������������������ģʽ
 * @param  ChannalNum ADC������ͨ����
 * @return ��
 * @note ADC_SingleModeDataStart,ADC_SingleModeDataConvertionState,ADC_SingleModeDataOut3���������ʹ��
 */
void ADC_SingleModeDataStart(uint32_t ChannalNum);

/**
 * @brief  ADC���������Ƿ����
 * @param  ��
 * @return �Ƿ�������
 *  @arg	TRUE �������
 *  @arg	FALSE ����δ���
 * @note ADC_SingleModeDataStart,ADC_SingleModeDataConvertionState,ADC_SingleModeDataOut3���������ʹ��
 */
bool ADC_SingleModeDataConvertionState(void);

/**
 * @brief  ��ȡADC��������
 * @param  ��
 * @return ADC��������������[0-4095]
 * @note ADC_SingleModeDataStart,ADC_SingleModeDataConvertionState,ADC_SingleModeDataOut3���������ʹ��
 */
int16_t ADC_SingleModeDataOut(void);

//����ģʽ�¿���ʹ��DMA
/**
 * @brief  ʹ��ADCת������DMA��ʽ
 * @param  ��
 * @return ��
 * @note  ʹ��ǰ������DMA���õ�ͨ��������DMA��ʽ��ADCֻ�ܵ�ͨ������
 */
void ADC_DMAEnable(void);

/**
 * @brief  ADCת��DMAģʽ����
 * @param  ��
 * @return ��
 * @note  ʹ��ǰ������DMA���õ�ͨ��������DMA��ʽ��ADCֻ�ܵ�ͨ������
 */
void ADC_DMADisable(void);

/**
 * @brief  ADC����������ʽ����
 * @param  ChannalNum ADC������ͨ����
 * @return ��
 */
void ADC_ContinuModeStart(uint32_t ChannalNum);

/**
 * @brief  MCU��ȡADC����������ʽ�������
 * @param  ��
 * @return ADC��������������[0-4095]
 */
uint16_t ADC_ContinuModeDataGet(void);

/**
 * @brief  ADC����������ʽ����
 * @param  ��
 * @return ��
 */
void ADC_ContinuModeStop(void);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // __ADC_H__
/**
 * @}
 * @}
 */
 
