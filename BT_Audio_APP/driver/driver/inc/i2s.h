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
 * @brief I2S模块
 */
typedef enum _I2S_MODULE
{
	I2S0_MODULE = 0,
	I2S1_MODULE = 1
} I2S_MODULE;

    
/**
 * @brief I2S支持的数据对齐格式
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
 * @brief I2S支持的数据位宽
 */ 
typedef enum _I2S_DATA_WORDLTH
{
    I2S_LENGTH_16BITS = 0,
    I2S_LENGTH_20BITS,
    I2S_LENGTH_24BITS,
    I2S_LENGTH_32BITS
} I2S_DATA_LENGTH;

/**
 * @brief 20bit和24bit时，数据存放格式
 */
typedef enum _I2S_20BIT_24BIT_ALIGN_MODE
{
	I2S_HIGH_BITS_ACTIVE = 0,

	I2S_LOW_BITS_ACTIVE,
} I2S_20BIT_24BIT_ALIGN_MODE;


/**
 * @brief I2S错误号
 */
typedef enum _I2S_ERROR_CODE
{
	I2S_ERROR_MODULE_INDEX = -256,

	I2S_ERROR_OK		   = 0,
} I2S_ERROR_CODE;

/**
 * @brief  在DSP（A）数据对齐格式下，设置I2S进入单声道或双声道模式
 * @param  I2SModuleIndex I2S_MODULE
 * @param  IsMono  : 1: 单声道数据传输
 *                    0: 双声道数据传输
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_MonoModeSet(I2S_MODULE I2SModuleIndex, bool IsMono);

/**
 * @brief  在20bit和24bit时，数据格式选择
 * @param  I2SModuleIndex I2S_MODULE
 * @param  AlignMode I2S_20BIT_24BIT_ALIGN_MODE
 *
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_AlignModeSet(I2S_MODULE I2SModuleIndex, I2S_20BIT_24BIT_ALIGN_MODE AlignMode);

/****************************************************************************/
/*                Lrclk和Bclk时钟的分频和相位相关函数                    */
/****************************************************************************/
/**
 * @brief  设置I2S模块的Bclk时钟是否反相
 * @param  I2SModuleIndex I2S_MODULE
 * @param  IsBclkInvert: 1:反相 0:不反相
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_BclkInvertSet(I2S_MODULE I2SModuleIndex, bool IsBclkInvert);

/**
 * @brief  设置I2S模块的Bclk输出时钟频率
 * @param  I2SModuleIndex  I2S_MODULE
 * @param  BclkDiv  : Bclk分频值, 范围(0~20),只有I2S模块处于Master模式下有效
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
 * @brief  设置I2S模块的Lrclk时钟是否反相
 * @param  I2SModuleIndex I2S_MODULE
 * @param  IsLrclkInvert: 1:反相 0: 不反相
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_LrclkInvertSet(I2S_MODULE I2SModuleIndex, bool IsLrclkInvert);

/**
 * @brief  设置I2S模块Lrclk时钟的输出频率
 * @param  I2SModuleIndex I2S_MODULE
 * @param  FreqDiv  : lrclk freq=bclk freq/FreqDiv,范围(8~2047),主要I2S模块处于
 *                    Master模式下时有效
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_LrclkFreqSet(I2S_MODULE I2SModuleIndex, uint32_t FreqDiv);

/****************************************************************************/
/*                Master模式和Slave模块相关函数                              */
/****************************************************************************/
/**
 * @brief  获取I2S模块当前采样率
 * @param  I2SModuleIndex I2S_MODULE
 * @return 当前采样率: 8K/11.025K/12K/16K/22.05K/24K/32K/44.1K/48K/64K/88.2K/96K/176.4K/192K
 *         如果返回0，则错误
 */
uint32_t I2S_SampleRateGet(I2S_MODULE I2SModuleIndex);

/**
 * @brief  设置I2S模块采样率
 * @param  I2SModuleIndex I2S_MODULE
 * @param  SampleRate: 需要设置的采样率，8K/11.025K/12K/16K/22.05K/24K/32K/44.1K/48K/64K/88.2K/96K/176.4K/192K
 * @return I2S_ERROR_CODE
 */ 
I2S_ERROR_CODE I2S_SampleRateSet(I2S_MODULE I2SModuleIndex, uint32_t SampleRate);

/**
 * @brief  获取I2S模块当前数据位宽
 * @param  I2SModuleIndex I2S_MODULE
 * @return I2S_DATA_LENGTH： I2S_LENGTH_16BITS， I2S_LENGTH_20BITS， I2S_LENGTH_24BITS， I2S_LENGTH_32BITS
 */
I2S_DATA_LENGTH I2S_WordlengthGet(I2S_MODULE I2SModuleIndex);

/**
 * @brief  设置I2S模块进入Master模式
 * @param  I2SModuleIndex I2S_MODULE
 * @param  I2SFormat : 数据对齐格式：I2S0:left/i2s I2S1:left/i2s/dsp
 * @param  I2SWordlth: 数据位宽参数：16bits/20bits/24bits/32bits
 * @return I2S_ERROR_CODE
 */ 
I2S_ERROR_CODE I2S_MasterModeSet(I2S_MODULE I2SModuleIndex, I2S_DATA_FORMAT I2SFormat, I2S_DATA_LENGTH I2SWordlth);

/**
 * @brief  设置I2S模块进入Slave模块
 * @param  I2SModuleIndex I2S_MODULE
 * @param  I2SFormat : 数据对齐格式: I2S0:left/i2s I2S1:left/i2s/dsp
 * @param  I2SWordlth: 数据位宽，16bits/20bits/24bits/32bits
 * @return I2S_ERROR_CODE
 */ 
I2S_ERROR_CODE I2S_SlaveModeSet(I2S_MODULE I2SModuleIndex, I2S_DATA_FORMAT I2SFormat, I2S_DATA_LENGTH I2SWordlth);


/****************************************************************************/
/*                使能、暂停和复位相关的函数                                 */
/****************************************************************************/

/**
 * @brief  使能I2S模块暂停或取消暂停功能
 * @param  I2SModuleIndex I2S_MODULE
 * @param  IsPause  : 是否暂停参数，0:继续发送, 1:暂停发送
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleTxPause(I2S_MODULE I2SModuleIndex, bool IsPause);

/**
 * @brief  使能I2S的发送模块
 * @param  I2SModuleIndex  I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleTxEnable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  禁能I2S的发送模块
 * @param  I2SModuleIndex  I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleTxDisable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  使能I2S Dout高组模式，当I2S发送模块禁能时，Dout为高组状态
 *         当I2S发送模块使能时，如果使能高组模式，则当无效bit时处于高组状态
 *                            如果禁能高组模式，则当无效bit时处于低电平
 * @param  I2SModuleIndex I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleTxHizEnable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  禁能I2S Dout高组模式，当I2S发送模块禁能时，Dout为高组状态
 *         当I2S发送模块使能时，如果使能高组模式，则当无效bit时处于高组状态
 *                            如果禁能高组模式，则当无效bit时处于低电平
 * @param  I2SModuleIndex  I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleTxHizDisable(I2S_MODULE I2SModuleIndex);


/**
 * @brief  使能I2S的接收模块
 * @param  I2SModuleIndex  I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleRxEnable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  禁能I2S的接收模块
 * @param  I2SModuleIndex  I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleRxDisable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  开启整个I2S模块
 * @param  I2SModuleIndex :I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleEnable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  关闭整个I2S模块
 * @param  I2SModuleIndex :I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_ModuleDisable(I2S_MODULE I2SModuleIndex);


/****************************************************************************/
/*                在Slave模块下采样率实时监测功能相关函数                    */
/****************************************************************************/
/**
 * @brief  使能I2S模块在Slave模式下采样率实时检测中断
 * @param  I2SModuleIndex : I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_SampleRateCheckInterruptEnable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  禁能I2S模块在Slave模式下采样率实时检测中断
 * @param  I2SModuleIndex : I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_SampleRateCheckInterruptDisable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  获取I2S采样率实时检测中断标志
 * @param  I2SModuleIndex I2S_MODULE
 * @return 中断标志，1:检测完成 0:无效
 */
bool I2S_SampleRateCheckInterruptGet(I2S_MODULE I2SModuleIndex);

/**
 * @brief  清除I2S采样率实时检测中断标志
 * @param  I2SModuleIndex I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_SampleRateCheckInterruptClr(I2S_MODULE I2SModuleIndex);

/****************************************************************************/
/*                       静音，淡入/淡出效果设置相关函数                     */
/****************************************************************************/
/**
 * @brief  设置I2S进入静音或非静音状态
 * @param  I2SModuleIndex I2S_MODULE
 * @param  IsMute   : 是否静音标志，0:非静音 1:静音
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_MuteSet(I2S_MODULE I2SModuleIndex, bool IsMute);

/**
 * @brief  获取I2S模块当前静音状态
 * @param  I2SModuleIndex I2S_MODULE
 * @return 1:静音状态 0:非静音状态
 */
bool I2S_MuteStatusGet(I2S_MODULE I2SModuleIndex);

/**
 * @brief  设置I2S模块淡入淡出的时间。
 * @param  FadeTime : 淡入淡出时间设置,单位:Ms,公式为：FadeTime = (2^12)/(Fs*(1 ~ 255)),
 *                    其中Fs为采样率，单位：KHz.
 *                    例如:1)采样率为8KHz时，FadeTime范围（2ms ~ 512ms）
 *                         2)采样率为192KHz时，FadeTime范围（1ms ~ 21ms）
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_FadeTimeSet(I2S_MODULE I2SModuleIndex, uint32_t FadeTime);

/**
 * @brief  开启I2S模块淡入淡出功能
 * @param  I2SModuleIndex : I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_FadeEnable(I2S_MODULE I2SModuleIndex);

/**
 * @brief  关闭I2S模块淡入淡出功能
 * @param  I2SModuleIndex : I2S_MODULE
 * @return I2S_ERROR_CODE
 */
I2S_ERROR_CODE I2S_FadeDisable(I2S_MODULE I2SModuleIndex);


//0:master mode ;1:slave mode 外设未接不要配slave
#define	I2S_MASTER_MODE			0
#define	I2S_SLAVE_MODE			1
//i2s_port: 0 ---> i2s0  1 ---> i2s1
//gpio_mode: GPIO复用功能配置
//gpio_index：GPIO引脚号
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

