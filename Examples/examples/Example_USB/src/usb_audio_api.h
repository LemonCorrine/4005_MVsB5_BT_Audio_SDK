/**
 **************************************************************************************
 * @file    usb_audio_api.h
 * @brief   usb audio api 
 *
 * @author  pi
 * @version V1.0.0
 *
 * $Created: 2018-05-08 11:40:00$
 *
 * @Copyright (C) 2018, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __USB_AUDIO_MODE_H__
#define __USB_AUDIO_MODE_H__

#include "otg_device_standard_request.h"

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#define CFG_APP_USB_AUDIO_MODE_EN

#define CFG_PARA_USB_MODE READER

#define CFG_AUDIO_WIDTH_24BIT

#define CFG_PARA_SAMPLE_RATE	48000

#define ONE_MS_SAMPLE	256


#define CFG_RES_AUDIO_USB_IN_EN
#define CFG_RES_AUDIO_USB_OUT_EN
#define CFG_RES_AUDIO_USB_SRC_EN
#define CFG_RES_AUDIO_USB_VOL_SET_EN

#ifdef CFG_AUDIO_WIDTH_24BIT
	typedef int32_t PCM_DATA_TYPE;
#else
	typedef int16_t PCM_DATA_TYPE;
#endif

#define	USB_FIFO_LEN						(ONE_MS_SAMPLE*2 * sizeof(PCM_DATA_TYPE) * 2 * 8)
#define USB_AUDIO_SRC_BUF_LEN				(ONE_MS_SAMPLE*2 * sizeof(PCM_DATA_TYPE) * 2)

bool UsbDevicePlayInit(void);
bool UsbDevicePlayResMalloc(void);
uint16_t UsbAudioSpeakerDataGet(void *Buffer,uint16_t Len);
uint16_t UsbAudioSpeakerDataLenGet(void);
uint16_t UsbAudioMicDataSet(void *Buffer,uint16_t Len);
uint16_t UsbAudioMicSpaceLenGet(void);
void UsbAudioSpeakerStreamProcess(void);
void UsbAudioMicStreamProcess(void);

void UsbDeviceEnable(void);
void UsbDeviceDisable(void);
void UsbAudioTimer1msProcess(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __USB_AUDIO_MODE_H__

