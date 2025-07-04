/**
 **************************************************************************************
 * @file    usb_audio_mode.h
 * @brief   usb audio mode 
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


#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

extern uint8_t GetFristEffectModeforUSBAEC(void);
extern bool isEffectModeforUSBAEC(uint8_t EffectMode);

bool UsbDevicePlayInit(void);
bool UsbDevicePlayDeinit(void);
void UsbDevicePlayRun(uint16_t msgId);

bool UsbDevicePlayMixInit(void);
bool UsbDevicePlayMixDeinit(void);

void AudioUsbPhoneBufInit(void);
void AudioUsbPhoneBufDeInit(void);
uint16_t AudioUsbPhoneBuf_DataLenGet(void);
uint16_t AudioUsbPhoneBuf_SpaceGet(void);
uint16_t AudioUsbPhoneBuf_DataGet(void *Buffer, uint16_t Len);
uint16_t AudioUsbPhoneBuf_DataPut(void *Buffer, uint16_t Len);
#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __USB_AUDIO_MODE_H__
