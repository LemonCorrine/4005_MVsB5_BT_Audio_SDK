#include "type.h"
#include "dac.h"
#include "dma.h"
#include "clk.h"
#include "ppwm.h"
#include "debug.h"

void AudioPPWM_Init(uint32_t SampleRate, void* Buf, uint16_t Len)
{
#if zsq
	Clock_AudioPllClockSet(PLL_CLK_MODE, PLL_CLK_1, AUDIO_PLL_CLK1_FREQ);
	Clock_AudioPllClockSet(PLL_CLK_MODE, PLL_CLK_2, AUDIO_PLL_CLK2_FREQ);

	if(IsSelectMclkClk1(SampleRate))
	{
		Clock_AudioMclkSel(AUDIO_PPWM, PLL_CLOCK1);
	}
	else
	{
		Clock_AudioMclkSel(AUDIO_PPWM, PLL_CLOCK2);
	}

	PPWM_ClkEnable();
	PPWM_Disable();
	PPWM_Pause();
	PPWM_VolSet(0x1000);
	PPWM_FadeTimeSet(10);
	PPWM_Run();

	DMA_CircularConfig(PERIPHERAL_ID_PPWM, Len/2, Buf, Len);
    DMA_ChannelEnable(PERIPHERAL_ID_PPWM);
	PPWM_Enable();
#endif
}

void AudioPPWM_SampleRateChange(uint32_t SampleRate)
{
#if zsq
		if(IsSelectMclkClk1(SampleRate))
		{
			Clock_AudioMclkSel(AUDIO_PPWM, PLL_CLOCK1);
		}
		else
		{
			Clock_AudioMclkSel(AUDIO_PPWM, PLL_CLOCK2);
		}
#endif
}


uint16_t AudioPPWM_DataSpaceLenGet(void)
{
#if zsq
	return DMA_CircularSpaceLenGet(PERIPHERAL_ID_PPWM) / 2;
#else
	return 0;
#endif
}

uint16_t AudioPPWM_DataSet(void* Buf, uint16_t Len)
{
#if zsq
	uint16_t Length;
	Length = Len * 2;
	return (uint16_t)DMA_CircularDataPut(PERIPHERAL_ID_PPWM, Buf, Length & 0xFFFFFFFE);
#else
	return 0;
#endif
}

