#include <string.h>
#include "type.h"
#include "audio_adc.h"
#include "dma.h"
#include "clk.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif
#include "adc_interface.h"

static AUDIO_BitWidth ADC0_BitWidth = ADC_WIDTH_24BITS;
static AUDIO_BitWidth ADC1_BitWidth = ADC_WIDTH_24BITS;

void AudioADC_DigitalInit(ADC_MODULE Module, uint32_t SampleRate, void* Buf, uint16_t Len,AUDIO_BitWidth BitWidth)
{
	//音频时钟使能，其他模块可能也会开启
#ifdef USB_CRYSTA_FREE_EN
	Clock_AudioPllClockSet(PLL_CLK_MODE, PLL_CLK_1, AUDIO_PLL_CLK1_FREQ);
	Clock_AudioPllClockSet(PLL_CLK_MODE, PLL_CLK_2, AUDIO_PLL_CLK2_FREQ);
#else
	Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_1, AUDIO_PLL_CLK1_FREQ);
	Clock_AudioPllClockSet(APLL_CLK_MODE, PLL_CLK_2, AUDIO_PLL_CLK2_FREQ);
#endif


	if(Module == ADC0_MODULE)
    {
        ADC0_BitWidth = BitWidth;
    	AudioADC_Disable(ADC0_MODULE);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_DONE_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_THRESHOLD_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC0_RX, DMA_ERROR_INT);
		DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
        DMA_CircularConfig(PERIPHERAL_ID_AUDIO_ADC0_RX, Len/2, Buf, Len);
        if(AudioADC_IsOverflow(ADC0_MODULE))
		{
        	AudioADC_OverflowClear(ADC0_MODULE);
		}

    	if(IsSelectMclkClk1(SampleRate))
    	{
    		Clock_AudioMclkSel(AUDIO_ADC0, PLL_CLOCK1);
    	}
    	else
    	{
    		Clock_AudioMclkSel(AUDIO_ADC0, PLL_CLOCK2);
    	}
    	AudioADC_SampleRateSet(ADC0_MODULE, SampleRate);

    	AudioADC_HighPassFilterConfig(ADC0_MODULE, 0xFFE);
    	AudioADC_HighPassFilterSet(ADC0_MODULE, TRUE);
        AudioADC_LREnable(ADC0_MODULE, 1, 1);
        AudioADC_FadeTimeSet(ADC0_MODULE, 10);
        AudioADC_FadeEnable(ADC0_MODULE);
		//AudioADC_FadeDisable(ADC0_MODULE);
        AudioADC_VolSet(ADC0_MODULE, 0x1000, 0x1000);
        AudioADC_SoftMute(ADC0_MODULE, FALSE, FALSE);
        AudioADC_Clear(ADC0_MODULE);

        AudioADC_WidthSet(ADC0_MODULE,BitWidth);
        AudioADC_Enable(ADC0_MODULE);

        DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC0_RX);
    }
    else if(Module == ADC1_MODULE)
    {
    	ADC1_BitWidth = BitWidth;
    	AudioADC_Disable(ADC1_MODULE);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_DONE_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_THRESHOLD_INT);
    	DMA_InterruptFlagClear(PERIPHERAL_ID_AUDIO_ADC1_RX, DMA_ERROR_INT);
		DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
        DMA_CircularConfig(PERIPHERAL_ID_AUDIO_ADC1_RX, Len/2, Buf, Len);
    	if(AudioADC_IsOverflow(ADC1_MODULE))
    	{
    		AudioADC_OverflowClear(ADC1_MODULE);
    	}
        
    	if(IsSelectMclkClk1(SampleRate))
    	{
    		Clock_AudioMclkSel(AUDIO_ADC1, PLL_CLOCK1);
    	}
    	else
    	{
    		Clock_AudioMclkSel(AUDIO_ADC1, PLL_CLOCK2);
    	}
    	AudioADC_SampleRateSet(ADC1_MODULE, SampleRate);

    	AudioADC_HighPassFilterConfig(ADC1_MODULE, 0xFFE);
        AudioADC_HighPassFilterSet(ADC1_MODULE, TRUE);
        AudioADC_LREnable(ADC1_MODULE, 1, 1);
        AudioADC_FadeTimeSet(ADC1_MODULE, 10);
        AudioADC_FadeEnable(ADC1_MODULE);
        //AudioADC_FadeDisable(ADC1_MODULE);
        AudioADC_VolSet(ADC1_MODULE, 0x1000, 0x1000);
        AudioADC_SoftMute(ADC1_MODULE, FALSE, FALSE);
        AudioADC_Clear(ADC1_MODULE);
        AudioADC_WidthSet(ADC1_MODULE,BitWidth);
        AudioADC_Enable(ADC1_MODULE);

        DMA_ChannelEnable(PERIPHERAL_ID_AUDIO_ADC1_RX);
    }
}

void AudioADC_AnaInit(ADC_MODULE ADCMODULE, AUDIO_ADC_INPUT InputSel, AUDIO_Mode AUDIOMode)
{
    if(ADC0_MODULE ==ADCMODULE)
    {
        // 设置初值
    	AudioADC_BIASPowerOn();
        AudioADC_ComparatorIBiasSet(ADC0_MODULE, 4, 4);
        AudioADC_OTA1IBiasSet(ADC0_MODULE, 4, 4);
        AudioADC_OTA2IBiasSet(ADC0_MODULE, 4, 4);
        AudioADC_LatchDelayIBiasSet(ADC0_MODULE, 4, 4);
        AudioADC_PGAIBiasSet(ADC0_MODULE, 4, 4);

        // 选择linein1通路
        if(LINEIN1_LEFT == InputSel)
        {
            AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT, LINEIN1_LEFT);
        }
        if(LINEIN1_RIGHT == InputSel)
        {
            AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT);
        }

        if(LINEIN2_LEFT == InputSel)
        {
            AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT, LINEIN2_LEFT);
        }

        if(LINEIN2_RIGHT == InputSel)
        {
            AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT, LINEIN2_RIGHT);
        }

        AudioADC_PGAMute(ADC0_MODULE, 0, 0);
        AudioADC_LREnable(ADC0_MODULE, 1, 1);

        // 0db校准
        AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN1_LEFT, 17);
        AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN1_RIGHT, 17);

        AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, LINEIN2_LEFT, 17);
        AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, LINEIN2_RIGHT, 17);
    }
    else
    {
        if(MIC_LEFT == InputSel)
        {
            // 设置初值
        	AudioADC_BIASPowerOn();
            AudioADC_ComparatorIBiasSet(ADC1_MODULE, 4, 4);
            AudioADC_OTA1IBiasSet(ADC1_MODULE, 4, 4);
            AudioADC_OTA2IBiasSet(ADC1_MODULE, 4, 4);
            AudioADC_LatchDelayIBiasSet(ADC1_MODULE, 4, 4);
            AudioADC_PGAIBiasSet(ADC1_MODULE, 4, 4);

            AudioADC_PGAPowerUp(ADC1_MODULE, 1, 1);
            AudioADC_PGAAbsMute(ADC1_MODULE, 0, 0);
            AudioADC_PGAMute(ADC1_MODULE, 0, 0);
            AudioADC_PowerUp(ADC1_MODULE, 1, 1);

            AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, MIC_LEFT, 23); // PGA校准 0db
            //AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, MIC_LEFT, 8);  //+20db 增益

            AudioADC_PGAZeroCrossEnable(ADC1_MODULE, 1, 1);
            if(Diff == AUDIOMode) // 差分模式
            {
                AudioADC_PGAMode(ADC1_MODULE, Diff);
            }
            else if(Single == AUDIOMode)
            {
                AudioADC_PGAMode(ADC1_MODULE, Single);
            }
        }
    }
}

void AudioADC_DeInit(ADC_MODULE Module)
{
	if(Module == ADC0_MODULE)
    {
	    AudioADC_Disable(ADC0_MODULE);
        DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC0_RX);
        //DMA_ChannelClose(PERIPHERAL_ID_AUDIO_ADC0_RX);
//        AudioADC_AnaDeInit(ADC0_MODULE);
    }
    else if(Module == ADC1_MODULE)
    {
	    AudioADC_Disable(ADC1_MODULE);
    	DMA_ChannelDisable(PERIPHERAL_ID_AUDIO_ADC1_RX);
    	//DMA_ChannelClose(PERIPHERAL_ID_AUDIO_ADC1_RX);
//    	AudioADC_AnaDeInit(ADC1_MODULE);
    }
}

uint16_t AudioADC0DataLenGet(void)
{
	uint16_t NumSamples = 0;

	NumSamples = DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC0_RX);
	if(ADC0_BitWidth == ADC_WIDTH_16BITS)
		return NumSamples / 4;
	else
		return NumSamples / 8;
}

//ADC1 单声道
uint16_t AudioADC1DataLenGet(void)
{
	uint16_t NumSamples = 0;

    NumSamples = DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC1_RX);

    return NumSamples / 4;
}

//返回Length uint：sample
uint16_t AudioADC0DataGet(void* Buf, uint16_t Len)
{
	uint16_t Length = 0;//Samples

    Length = DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC0_RX);

    if(ADC0_BitWidth == ADC_WIDTH_16BITS)
    {
		if(Length > Len * 4)
		{
			Length = Len * 4;
		}
		DMA_CircularDataGet(PERIPHERAL_ID_AUDIO_ADC0_RX, Buf, Length & 0xFFFFFFFC);
		return Length / 4;
    }
    else
    {
		if(Length > Len * 8)
		{
			Length = Len * 8;
		}
		DMA_CircularDataGet(PERIPHERAL_ID_AUDIO_ADC0_RX, Buf, Length & 0xFFFFFFF8);

		//高8位无符号位，需要移位产生
		int32_t *pcm = Buf;
		uint32_t i;
		for(i=0;i<Length / 4;i++)
		{
			pcm[i]<<=8;
			pcm[i]>>=8;
		}
		return Length / 8;
    }
}

//此处后续可能要做处理，区分单声道和立体声,数据格式转换在外边处理
//和DMA的配置也会有关系
//返回Length uint：sample
uint16_t AudioADC1DataGet(void* Buf, uint16_t Len)
{
	uint16_t Length = 0;//Samples

    Length = DMA_CircularDataLenGet(PERIPHERAL_ID_AUDIO_ADC1_RX);
	if(Length > Len * 4)
	{
		Length = Len * 4;
	}
	DMA_CircularDataGet(PERIPHERAL_ID_AUDIO_ADC1_RX, Buf, Length & 0xFFFFFFFC);

    if(ADC1_BitWidth == ADC_WIDTH_16BITS)
    {
    	//高16位为0，转成16位数据
		int16_t *pcm = Buf;
		int32_t *pcm32 = Buf;
		uint32_t i;
		for(i=0;i<Length / 4;i++)
		{
			pcm[i] = pcm32[i];
		}
    }
    else
    {
		//高8位为0，符号位需要移位产生
		int32_t *pcm = Buf;
		uint32_t i;
		for(i=0;i<Length / 4;i++)
		{
			pcm[i]<<=8;
			pcm[i]>>=8;
		}
    }
    return Length / 4;
}

