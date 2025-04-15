/**
 **************************************************************************************
 * @file    i2s.h
 * @brief   I2S Module API
 *
 * @author  Cecilia Wang
 * @version V1.0.0
 *
 * $Created: 2017-10-22 15:32:38$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
/**
 * @addtogroup I2S
 * @{
 * @defgroup i2s i2s.h
 * @{
 */
 
#ifndef __I2S_H__
#define __I2S_H__
 
#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
/**
 * @brief I2Sģ��
 */
typedef enum _I2S_MODULE
{
	I2S0_MODULE = 0,
	I2S1_MODULE = 1
} I2S_MODULE;

    
/**
 * @brief I2S֧�ֵ����ݶ����ʽ
 */    
typedef enum _I2S_DATA_FORMAT
{
	I2S_FORMAT_TDM = 0,
    I2S_FORMAT_LEFT,
    I2S_FORMAT_I2S,
    I2S_FORMAT_DSPA,
    I2S_FORMAT_DSPB
} I2S_DATA_FORMAT;


/**
 * @brief I2S֧�ֵ�����λ��
 */ 
typedef enum _I2S_DATA_WORDLTH
{
    I2S_LENGTH_16BITS = 0,
    I2S_LENGTH_20BITS,
    I2S_LENGTH_24BITS,
    I2S_LENGTH_32BITS
} I2S_DATA_LENGTH;

/**
 * @brief 20bit��24bitʱ�����ݴ�Ÿ�ʽ
 */
typedef enum _I2S_20BIT_24BIT_ALIGN_MODE
{
	I2S_HIGH_BITS_ACTIVE = 0,

	I2S_LOW_BITS_ACTIVE,
} I2S_20BIT_24BIT_ALIGN_MODE;


/**
 * @brief I2S�����
 */
typedef enum _I2S_ERROR_CODE
{
	I2S_ERROR_MODULE_INDEX = -256,

	I2S_ERROR_OK		   = 0,
} I2S_ERROR_CODE;

/**
 * @brief  ��DSP��A�����ݶ����ʽ�£�����I2S���뵥������˫����ģʽ
 * @param  I2SModuleIndex I2S_MODULE
 * @param  IsMono  : 1: ���������ݴ���
 *                    0: ˫�������ݴ���
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_MonoModeSet(I2S_MODULE I2SModuleIndex, bool IsMono);

/**
 * @brief  ��20bit��24bitʱ�����ݸ�ʽѡ��
 * @param  I2SModuleIndex I2S_MODULE
 * @param  AlignMode I2S_20BIT_24BIT_ALIGN_MODE
 *
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_AlignModeSet(I2S_MODULE I2SModuleIndex, I2S_20BIT_24BIT_ALIGN_MODE AlignMode);

/****************************************************************************/
/*                Lrclk��Bclkʱ�ӵķ�Ƶ����λ��غ���                    */
/****************************************************************************/
/**
 * @brief  ����I2Sģ���Bclkʱ���Ƿ���
 * @param  I2SModuleIndex I2S_MODULE
 * @param  IsBclkInvert: 1:���� 0:������
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_BclkInvertSet(I2S_MODULE I2SModuleIndex, bool IsBclkInvert);

/**
 * @brief  ����I2Sģ���Bclk���ʱ��Ƶ��
 * @param  I2SModuleIndex  I2S_MODULE
 * @param  BclkDiv  : Bclk��Ƶֵ, ��Χ(0~20),ֻ��I2Sģ�鴦��Masterģʽ����Ч
 * @param          |  0: i2s_clk     |  1: i2s_clk/1.5  |   2: i2s_clk/2  |
 * @param          |  3: i2s_clk/3   |  4: i2s_clk/4    |   5: i2s_clk/5  |
 * @param          |  6: i2s_clk/5.5 |  7: i2s_clk/6    |   8: i2s_clk/8  |
 * @param          |  9: i2s_clk/10  | 10: i2s_clk/11   |  11: i2s_clk/12 |
 * @param          | 12: i2s_clk/16  | 13: i2s_clk/20   |  14: i2s_clk/22 |
 * @param          | 15: i2s_clk/24  | 16: i2s_clk/25   |  17: i2s_clk/30 |
 * @param          | 18: i2s_clk/32  | 19: i2s_clk/44   |  20: i2s_clk/48 |
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_BclkFreqSet(I2S_MODULE I2SModuleIndex, uint32_t BclkDiv);

/**
 * @brief  ����I2Sģ���Lrclkʱ���Ƿ���
 * @param  I2SModuleIndex I2S_MODULE
 * @param  IsLrclkInvert: 1:���� 0: ������
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_LrclkInvertSet(I2S_MODULE I2SModuleIndex, bool IsLrclkInvert);

/**
 * @brief  ����I2Sģ��Lrclkʱ�ӵ����Ƶ��
 * @param  I2SModuleIndex I2S_MODULE
 * @param  FreqDiv  : lrclk freq=bclk freq/FreqDiv,��Χ(8~2047),��ҪI2Sģ�鴦��
 *                    Masterģʽ��ʱ��Ч
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_LrclkFreqSet(I2S_MODULE I2SModuleIndex, uint32_t FreqDiv);

/****************************************************************************/
/*                Masterģʽ��Slaveģ����غ���                              */
/****************************************************************************/
/**
 * @brief  ��ȡI2Sģ�鵱ǰ������
 * @param  I2SModuleIndex I2S_MODULE
 * @return ��ǰ������: 8K/11.025K/12K/16K/22.05K/24K/32K/44.1K/48K/64K/88.2K/96K/176.4K/192K
 *         �������0�������
 */
uint32_t I2S_SampleRateGet(I2S_MODULE I2SModuleIndex);

/**
 * @brief  ����I2Sģ�������
 * @param  I2SModuleIndex I2S_MODULE
 * @param  SampleRate: ��Ҫ���õĲ����ʣ�8K/11.025K/12K/16K/22.05K/24K/32K/44.1K/48K/64K/88.2K/96K/176.4K/192K
 * @return I2S_ERROR_CODE
 */ 
I2S_ERROR_CODE I2S_SampleRateSet(I2S_MODULE I2SModuleIndex, uint32_t SampleRate);

/**
 * @brief  ��ȡI2Sģ�鵱ǰ����λ��
 * @param  I2SModuleIndex I2S_MODULE
 * @return I2S_DATA_LENGTH�� I2S_LENGTH_16BITS�� I2S_LENGTH_20BITS�� I2S_LENGTH_24BITS�� I2S_LENGTH_32BITS
 */
I2S_DATA_LENGTH I2S_WordlengthGet(I2S_MODULE I2SModuleIndex);

/**
 * @brief  ����I2Sģ�����Masterģʽ
 * @param  I2SModuleIndex I2S_MODULE
 * @param  I2SFormat : ���ݶ����ʽ��I2S0:left/i2s I2S1:left/i2s/dsp
 * @param  I2SWordlth: ����λ�������16bits/20bits/24bits/32bits
 * @return I2S_ERROR_CODE
 */ 
I2S_ERROR_CODE I2S_MasterModeSet(I2S_MODULE I2SModuleIndex, I2S_DATA_FORMAT I2SFormat, I2S_DATA_LENGTH I2SWordlth);

/**
 * @brief  ����I2Sģ�����Slaveģ��
 * @param  I2SModuleIndex I2S_MODULE
 * @param  I2SFormat : ���ݶ����ʽ: I2S0:left/i2s I2S1:left/i2s/dsp
 * @param  I2SWordlth: ����λ��16bits/20bits/24bits/32bits
 * @return I2S_ERROR_CODE
 */ 
I2S_ERROR_CODE I2S_SlaveModeSet(I2S_MODULE I2SModuleIndex, I2S_DATA_FORMAT I2SFormat, I2S_DATA_LENGTH I2SWordlth);


/****************************************************************************/
/*                ʹ�ܡ���ͣ�͸�λ��صĺ���                                 */
/****************************************************************************/

/**
 * @brief  ʹ��I2Sģ����ͣ��ȡ����ͣ����
 * @param  I2SModuleIndex I2S_MODULE
 * @param  IsPause  : �Ƿ���ͣ������0:��������, 1:��ͣ����
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleTxPause(I2S_MODULE I2SModuleIndex, bool IsPause);

/**
 * @brief  ʹ��I2S�ķ���ģ��
 * @param  I2SModuleIndex  I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleTxEnable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  ����I2S�ķ���ģ��
 * @param  I2SModuleIndex  I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleTxDisable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  ʹ��I2S Dout����ģʽ����I2S����ģ�����ʱ��DoutΪ����״̬
 *         ��I2S����ģ��ʹ��ʱ�����ʹ�ܸ���ģʽ������Чbitʱ���ڸ���״̬
 *                            ������ܸ���ģʽ������Чbitʱ���ڵ͵�ƽ
 * @param  I2SModuleIndex I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleTxHizEnable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  ����I2S Dout����ģʽ����I2S����ģ�����ʱ��DoutΪ����״̬
 *         ��I2S����ģ��ʹ��ʱ�����ʹ�ܸ���ģʽ������Чbitʱ���ڸ���״̬
 *                            ������ܸ���ģʽ������Чbitʱ���ڵ͵�ƽ
 * @param  I2SModuleIndex  I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleTxHizDisable(I2S_MODULE I2SModuleIndex);


/**
 * @brief  ʹ��I2S�Ľ���ģ��
 * @param  I2SModuleIndex  I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleRxEnable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  ����I2S�Ľ���ģ��
 * @param  I2SModuleIndex  I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleRxDisable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  ��������I2Sģ��
 * @param  I2SModuleIndex :I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleEnable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  �ر�����I2Sģ��
 * @param  I2SModuleIndex :I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleDisable(I2S_MODULE I2SModuleIndex);


/****************************************************************************/
/*                ��Slaveģ���²�����ʵʱ��⹦����غ���                    */
/****************************************************************************/
/**
 * @brief  ʹ��I2Sģ����Slaveģʽ�²�����ʵʱ����ж�
 * @param  I2SModuleIndex : I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_SampleRateCheckInterruptEnable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  ����I2Sģ����Slaveģʽ�²�����ʵʱ����ж�
 * @param  I2SModuleIndex : I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_SampleRateCheckInterruptDisable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  ��ȡI2S������ʵʱ����жϱ�־
 * @param  I2SModuleIndex I2S_MODULE
 * @return �жϱ�־��1:������ 0:��Ч
 */
bool I2S_SampleRateCheckInterruptGet(I2S_MODULE I2SModuleIndex);

/**
 * @brief  ���I2S������ʵʱ����жϱ�־
 * @param  I2SModuleIndex I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_SampleRateCheckInterruptClr(I2S_MODULE I2SModuleIndex);

/****************************************************************************/
/*                       ����������/����Ч��������غ���                     */
/****************************************************************************/
/**
 * @brief  ����I2S���뾲����Ǿ���״̬
 * @param  I2SModuleIndex I2S_MODULE
 * @param  IsMute   : �Ƿ�����־��0:�Ǿ��� 1:����
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_MuteSet(I2S_MODULE I2SModuleIndex, bool IsMute);

/**
 * @brief  ��ȡI2Sģ�鵱ǰ����״̬
 * @param  I2SModuleIndex I2S_MODULE
 * @return 1:����״̬ 0:�Ǿ���״̬
 */
bool I2S_MuteStatusGet(I2S_MODULE I2SModuleIndex);

/**
 * @brief  ����I2Sģ�鵭�뵭����ʱ�䡣
 * @param  FadeTime : ���뵭��ʱ������,��λ:Ms,��ʽΪ��FadeTime = (2^12)/(Fs*(1 ~ 255)),
 *                    ����FsΪ�����ʣ���λ��KHz.
 *                    ����:1)������Ϊ8KHzʱ��FadeTime��Χ��2ms ~ 512ms��
 *                         2)������Ϊ192KHzʱ��FadeTime��Χ��1ms ~ 21ms��
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_FadeTimeSet(I2S_MODULE I2SModuleIndex, uint32_t FadeTime);

/**
 * @brief  ����I2Sģ�鵭�뵭������
 * @param  I2SModuleIndex : I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_FadeEnable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  �ر�I2Sģ�鵭�뵭������
 * @param  I2SModuleIndex : I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_FadeDisable(I2S_MODULE I2SModuleIndex);


//0:master mode ;1:slave mode ����δ�Ӳ�Ҫ��slave
#define	I2S_MASTER_MODE			0
#define	I2S_SLAVE_MODE			1
//i2s_port: 0 ---> i2s0  1 ---> i2s1
//gpio_mode: GPIO���ù�������
//gpio_index��GPIO���ź�
#define SET_I2S_GPIO_VAL(i2s_port,gpio_mode,gpio_index)						((i2s_port<<16)|(gpio_mode<<8)|(gpio_index))
#define GET_I2S_I2S_PORT(val)												((val>>16)&0x01)
#define GET_I2S_GPIO_MODE(val)												((val>>8)&0x0f)
#define GET_I2S_GPIO_INDEX(val)												((val)&0x1f)
#define GET_I2S_GPIO_PORT(val)												(1<<((val)&0x1f))
//i2s_mode: I2S_MASTER_MODE I2S_SLAVE_MODE
#define SET_I2S_GPIO_ALL_VAL(i2s_mode,i2s_port,gpio_mode,gpio_index)		((i2s_mode<<24)|(i2s_port<<16)|(gpio_mode<<8)|(gpio_index))
#define GET_I2S_MODE(val)													((val>>24)&0x01)

//mclk out gpio I2S_MASTER_MODE
#define I2S0_MCLK_OUT_A24									SET_I2S_GPIO_ALL_VAL(I2S_MASTER_MODE,0,0x08,24)
#define I2S1_MCLK_OUT_A0									SET_I2S_GPIO_ALL_VAL(I2S_MASTER_MODE,1,0x06,0)
#define I2S1_MCLK_OUT_A6									SET_I2S_GPIO_ALL_VAL(I2S_MASTER_MODE,1,0x08,6)
#define I2S1_MCLK_OUT_A7									SET_I2S_GPIO_ALL_VAL(I2S_MASTER_MODE,1,0x08,7)
//mclk in gpio  I2S_SLAVE_MODE
#define I2S0_MCLK_IN_A24									SET_I2S_GPIO_ALL_VAL(I2S_SLAVE_MODE,0,0x02,24)
#define I2S1_MCLK_IN_A0										SET_I2S_GPIO_ALL_VAL(I2S_SLAVE_MODE,1,0x02,0)
#define I2S1_MCLK_IN_A6										SET_I2S_GPIO_ALL_VAL(I2S_SLAVE_MODE,1,0x02,6)
#define I2S1_MCLK_IN_A7										SET_I2S_GPIO_ALL_VAL(I2S_SLAVE_MODE,1,0x04,7)
//lrclk gpio
#define I2S0_LRCLK_A20										SET_I2S_GPIO_VAL(0,0x07,20)
#define I2S1_LRCLK_A7										SET_I2S_GPIO_VAL(1,0x03,7)
#define I2S1_LRCLK_A20										SET_I2S_GPIO_VAL(1,0x08,20)
#define I2S1_LRCLK_A28										SET_I2S_GPIO_VAL(1,0x02,28)
//bclk gpio
#define I2S0_BCLK_A21										SET_I2S_GPIO_VAL(0,0x05,21)
#define I2S1_BCLK_A9										SET_I2S_GPIO_VAL(1,0x02,9)
#define I2S1_BCLK_A21										SET_I2S_GPIO_VAL(1,0x06,21)
#define I2S1_BCLK_A29										SET_I2S_GPIO_VAL(1,0x02,29)
//dout gpio
#define I2S0_DOUT_A22										SET_I2S_GPIO_VAL(0,0x09,22)
#define I2S0_DOUT_A23										SET_I2S_GPIO_VAL(0,0x08,23)
#define I2S1_DOUT_A10										SET_I2S_GPIO_VAL(1,0x06,10)
#define I2S1_DOUT_A30										SET_I2S_GPIO_VAL(1,0x09,30)
#define I2S1_DOUT_A31										SET_I2S_GPIO_VAL(1,0x09,31)
//din gpio
#define I2S0_DIN_A22										SET_I2S_GPIO_VAL(0,0x03,22)
#define I2S0_DIN_A23										SET_I2S_GPIO_VAL(0,0x03,23)
#define I2S1_DIN_A10										SET_I2S_GPIO_VAL(1,0x02,10)
#define I2S1_DIN_A30										SET_I2S_GPIO_VAL(1,0x02,30)
#define I2S1_DIN_A31										SET_I2S_GPIO_VAL(1,0x02,31)

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__I2S_H__

/**
 * @}
 * @}
 */

