/**
 **************************************************************************************
 * @file    dac_interface.h
 * @brief   audio dac interface
 *
 * @author  Sam
 * @version V1.1.0
 *
 * $Created: 2015-09-23 14:01:05$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef		__DAC_INTERFACE_H__
#define		__DAC_INTERFACE_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 
#include "dac.h"

typedef struct __DACParamCt
{
	DAC_Model DACModel;
	DAC_LoadStatus DACLoadStatus;
	PVDD_Model PVDDModel;
	DAC_EnergyModel DACEnergyModel;
	DAC_VcomModel DACVcomModel;
} DACParamCt;

void AudioDAC_Init(DACParamCt *ct, uint32_t SampleRate, uint16_t BitWidth, void *Buf, uint16_t Len,  void *BufEXT, uint16_t LenEXT);
void AudioDAC0_SampleRateChange(uint32_t SampleRate);

uint16_t AudioDAC0_DataSpaceLenGet(void);
uint16_t AudioDAC0_DataSet(void* Buf, uint16_t Len);
uint16_t AudioDAC0_DataLenGet(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__DAC_INTERFACE_H__

