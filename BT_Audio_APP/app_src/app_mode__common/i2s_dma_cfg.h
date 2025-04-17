#ifndef __I2S_DMA_CFG_H__
#define __I2S_DMA_CFG_H__

//���ļ�ֻ��һЩ�����ã�����I2Sģ���DMA���ã��ο����£�����Ҫ��ע���޸�
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

// I2Sģ��DMA�����漰������Щ�꣬������Щ�꿪�ض����������£�����ʶ���i2s0/i2s1 TX/RX��DMAͨ��ʹ�����
// CFG_APP_I2SIN_MODE_EN/CFG_RES_AUDIO_I2SOUT_EN/CFG_RES_AUDIO_I2S_MIX_OUT_EN
// CFG_RES_AUDIO_I2S_MIX_IN_EN/CFG_RES_AUDIO_I2S_MIX2_OUT_EN/CFG_RES_AUDIO_I2S_MIX2_IN_EN
#define I2S0_TX_NEED_ENABLE		0x01		//��ҪI2S0_TX DMA
#define I2S0_RX_NEED_ENABLE		0x02		//��ҪI2S0_RX DMA
#define I2S1_TX_NEED_ENABLE		0x04		//��ҪI2S1_TX DMA
#define I2S1_RX_NEED_ENABLE		0x08		//��ҪI2S1_RX DMA

#ifdef CFG_APP_I2SIN_MODE_EN
	//i2s inģʽ��I2S DMAͨ��
	#if CFG_RES_I2S_MODULE
		#define I2SIN_MODE_I2S_DMA_CH	I2S1_RX_NEED_ENABLE
	#else
		#define I2SIN_MODE_I2S_DMA_CH	I2S0_RX_NEED_ENABLE
	#endif
#endif

#ifdef CFG_RES_AUDIO_I2SOUT_EN
	//I2S_OUT��I2S DMAͨ��
	#if CFG_RES_I2S_MODULE
		#define I2S_OUT_DMA_CH		I2S1_TX_NEED_ENABLE
	#else
		#define I2S_OUT_DMA_CH		I2S0_TX_NEED_ENABLE
	#endif
#else
	#define I2S_OUT_DMA_CH			0
#endif

#ifdef CFG_RES_AUDIO_I2S_MIX_OUT_EN
	//I2S_MIX_OUT��I2S DMAͨ��
	#if CFG_RES_MIX_I2S_MODULE
		#define I2S_MIX_OUT_DMA_CH	I2S1_TX_NEED_ENABLE
	#else
		#define I2S_MIX_OUT_DMA_CH	I2S0_TX_NEED_ENABLE
	#endif
#else
	#define I2S_MIX_OUT_DMA_CH		0
#endif

#ifdef CFG_RES_AUDIO_I2S_MIX_IN_EN
	//I2S_MIX_IN��I2S DMAͨ��
	#if CFG_RES_MIX_I2S_MODULE
		#define I2S_MIX_IN_DMA_CH	I2S1_RX_NEED_ENABLE
	#else
		#define I2S_MIX_IN_DMA_CH	I2S0_RX_NEED_ENABLE
	#endif
#else
	#define I2S_MIX_IN_DMA_CH		0
#endif

#ifdef CFG_RES_AUDIO_I2S_MIX2_OUT_EN
	//I2S_MIX2_OUT��I2S DMAͨ��
	#if CFG_RES_MIX2_I2S_MODULE
		#define I2S_MIX2_OUT_DMA_CH	I2S1_TX_NEED_ENABLE
	#else
		#define I2S_MIX2_OUT_DMA_CH	I2S0_TX_NEED_ENABLE
	#endif
#else
	#define I2S_MIX2_OUT_DMA_CH		0
#endif

#ifdef CFG_RES_AUDIO_I2S_MIX2_IN_EN
	//I2S_MIX_IN��I2S DMAͨ��
	#if CFG_RES_MIX2_I2S_MODULE
		#define I2S_MIX2_IN_DMA_CH	I2S1_RX_NEED_ENABLE
	#else
		#define I2S_MIX2_IN_DMA_CH	I2S0_RX_NEED_ENABLE
	#endif
#else
	#define I2S_MIX2_IN_DMA_CH		0
#endif

//����app_config.h��i2s�����ã�����I2S��DMAʹ�����
#define I2S_ALL_DMA_CH_CFG 			(I2S_OUT_DMA_CH|I2S_MIX_OUT_DMA_CH|I2S_MIX_IN_DMA_CH|I2S_MIX2_OUT_DMA_CH|I2S_MIX2_IN_DMA_CH)


//Ĭ������Ϊ��ʹ��(ȫ��Ϊ255),DMA���ñ���ҪĬ�ϼ��ڿ�ʼ��λ��
#define DMA_CFG_TABLE_DEFAULT_INIT	0xff,0xff,0xff,0xff,0xff,0xff,[0]=

#endif



