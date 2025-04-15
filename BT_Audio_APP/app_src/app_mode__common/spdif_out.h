#ifndef _SPDIF_OUT_H_
#define _SPDIF_OUT_H_

void AudioSpdifOutParamsSet(void);
uint16_t AudioSpdifTXDataSet(void* Buf, uint16_t Len);
uint16_t AudioSpdifTXDataSpaceLenGet(void);
#endif
