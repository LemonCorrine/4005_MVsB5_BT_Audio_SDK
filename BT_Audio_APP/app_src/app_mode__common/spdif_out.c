#include <string.h>
#include "type.h"
#include "gpio.h"
#include "dma.h"
#include "app_message.h"
#include "app_config.h"
#include "debug.h"
#include "delay.h"
#include "spdif.h"
#include "clk.h"
#include "mcu_circular_buf.h"
#include "spdif_mode.h"
#include "spdif_out.h"

#ifdef CFG_RES_AUDIO_SPDIFOUT_EN

#define SPDIF_FIFO_LEN				(10 * 1024)
uint32_t SpdifAddr[SPDIF_FIFO_LEN/4];
int32_t SpdifBuf[1024];


bool Clock_APllLock(uint32_t PllFreq);
#include "clk.h"

void AudioSpdifOut_SampleRateChange(uint32_t SampleRate)
{
	if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100)
			|| (SampleRate == 88200) || (SampleRate == 176400))
	{
		Clock_PllLock(225792);
	}
	else
	{
		Clock_PllLock(245760);
	}
	SPDIF_SampleRateSet(SPDIF_OUT_NUM,SampleRate);
}

void AudioSpdifOutParamsSet(void)
{
	GPIO_PortAModeSet(GPIOA29, 0x0b);
	Clock_SpdifClkSelect(PLL_CLK_MODE);//DPLL
	SPDIF_TXInit(SPDIF_OUT_NUM,1, 1, 0, 10);
	DMA_CircularConfig(SPDIF_OUT_DMA_ID, sizeof(SpdifAddr)/4, SpdifAddr, sizeof(SpdifAddr));
	AudioSpdifOut_SampleRateChange(CFG_PARA_SAMPLE_RATE);
	SPDIF_ModuleEnable(SPDIF_OUT_NUM);
	DMA_ChannelEnable(SPDIF_OUT_DMA_ID);
}

uint16_t AudioSpdifTXDataSet(void* Buf, uint16_t Len)
{
	uint16_t Length;
	int m;
	if(Buf == NULL) return 0;
#ifdef CFG_AUDIO_WIDTH_24BIT
	Length = Len * 8;
	m = SPDIF_PCMDataToSPDIFData(SPDIF_OUT_NUM,(int32_t *)Buf, Length, (int32_t *)SpdifBuf, SPDIF_WORDLTH_24BIT);
#else
	Length = Len * 4;
	m = SPDIF_PCMDataToSPDIFData(SPDIF_OUT_NUM,(int32_t *)Buf, Length, (int32_t *)SpdifBuf, SPDIF_WORDLTH_16BIT);
#endif
	DMA_CircularDataPut(SPDIF_OUT_DMA_ID, (void *)SpdifBuf, m & 0xFFFFFFFC);


	return 0;
}


uint16_t AudioSpdifTXDataSpaceLenGet(void)
{
#ifdef CFG_AUDIO_WIDTH_24BIT
	return DMA_CircularSpaceLenGet(SPDIF_OUT_DMA_ID) / 16;
#else
	return DMA_CircularSpaceLenGet(SPDIF_OUT_DMA_ID) / 8;
#endif
}


#endif
