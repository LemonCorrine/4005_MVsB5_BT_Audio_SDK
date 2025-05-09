#ifndef __I2S_DMA_CFG_H__
#define __I2S_DMA_CFG_H__

//此文件只是一些宏配置，用于I2S模块的DMA配置，参考如下；不需要关注和修改
/*------------------------------------------------------------------
#if (I2S_ALL_DMA_CH_CFG & I2S0_TX_NEED_ENABLE)
	PERIPHERAL_ID_I2S0_TX,
#endif
#if (I2S_ALL_DMA_CH_CFG & I2S1_TX_NEED_ENABLE)
	PERIPHERAL_ID_I2S1_TX,
#endif
#if (I2S_ALL_DMA_CH_CFG & I2S0_RX_NEED_ENABLE)
	PERIPHERAL_ID_I2S0_RX,
#endif
#if (I2S_ALL_DMA_CH_CFG & I2S1_RX_NEED_ENABLE)
	PERIPHERAL_ID_I2S1_RX,
#endif
------------------------------------------------------------------*/

// I2S模块DMA配置涉及如下这些宏，用于这些宏开关多种组合情况下，快速识别出i2s0/i2s1 TX/RX的DMA通道使用情况
// CFG_APP_I2SIN_MODE_EN/CFG_RES_AUDIO_I2SOUT_EN/CFG_RES_AUDIO_I2S_MIX_OUT_EN
// CFG_RES_AUDIO_I2S_MIX_IN_EN/CFG_RES_AUDIO_I2S_MIX2_OUT_EN/CFG_RES_AUDIO_I2S_MIX2_IN_EN
#define I2S0_TX_NEED_ENABLE		0x01		//需要I2S0_TX DMA
#define I2S0_RX_NEED_ENABLE		0x02		//需要I2S0_RX DMA
#define I2S1_TX_NEED_ENABLE		0x04		//需要I2S1_TX DMA
#define I2S1_RX_NEED_ENABLE		0x08		//需要I2S1_RX DMA

#ifdef CFG_APP_I2SIN_MODE_EN
	//i2s in模式的I2S DMA通道
	#if CFG_RES_I2S_MODULE
		#define I2SIN_MODE_I2S_DMA_CH	I2S1_RX_NEED_ENABLE
	#else
		#define I2SIN_MODE_I2S_DMA_CH	I2S0_RX_NEED_ENABLE
	#endif
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	//I2S_OUT的I2S DMA通道
	#if CFG_RES_I2S_MODULE
		#define I2S_OUT_DMA_CH		I2S1_TX_NEED_ENABLE
	#else
		#define I2S_OUT_DMA_CH		I2S0_TX_NEED_ENABLE
	#endif
#else
	#define I2S_OUT_DMA_CH			0
#endif

#ifdef CFG_RES_AUDIO_I2S_MIX_OUT_EN
	//I2S_MIX_OUT的I2S DMA通道
	#if CFG_RES_MIX_I2S_MODULE
		#define I2S_MIX_OUT_DMA_CH	I2S1_TX_NEED_ENABLE
	#else
		#define I2S_MIX_OUT_DMA_CH	I2S0_TX_NEED_ENABLE
	#endif
#else
	#define I2S_MIX_OUT_DMA_CH		0
#endif

#ifdef CFG_RES_AUDIO_I2S_MIX_IN_EN
	//I2S_MIX_IN的I2S DMA通道
	#if CFG_RES_MIX_I2S_MODULE
		#define I2S_MIX_IN_DMA_CH	I2S1_RX_NEED_ENABLE
	#else
		#define I2S_MIX_IN_DMA_CH	I2S0_RX_NEED_ENABLE
	#endif
#else
	#define I2S_MIX_IN_DMA_CH		0
#endif

#ifdef CFG_RES_AUDIO_I2S_MIX2_OUT_EN
	//I2S_MIX2_OUT的I2S DMA通道
	#if CFG_RES_MIX2_I2S_MODULE
		#define I2S_MIX2_OUT_DMA_CH	I2S1_TX_NEED_ENABLE
	#else
		#define I2S_MIX2_OUT_DMA_CH	I2S0_TX_NEED_ENABLE
	#endif
#else
	#define I2S_MIX2_OUT_DMA_CH		0
#endif

#ifdef CFG_RES_AUDIO_I2S_MIX2_IN_EN
	//I2S_MIX_IN的I2S DMA通道
	#if CFG_RES_MIX2_I2S_MODULE
		#define I2S_MIX2_IN_DMA_CH	I2S1_RX_NEED_ENABLE
	#else
		#define I2S_MIX2_IN_DMA_CH	I2S0_RX_NEED_ENABLE
	#endif
#else
	#define I2S_MIX2_IN_DMA_CH		0
#endif

//根据app_config.h中i2s的配置，生成I2S的DMA使用情况
#define I2S_ALL_DMA_CH_CFG 			(I2S_OUT_DMA_CH|I2S_MIX_OUT_DMA_CH|I2S_MIX_IN_DMA_CH|I2S_MIX2_OUT_DMA_CH|I2S_MIX2_IN_DMA_CH)


//默认配置为不使用(全部为255),DMA配置表需要默认加在开始的位置
#define DMA_CFG_TABLE_DEFAULT_INIT	0xff,0xff,0xff,0xff,0xff,0xff,[0]=

#endif



