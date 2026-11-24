/**
 *****************************************************************************
 * @file     otg_host_audio10.h
 * @author   Shanks
 * @version  V1.0.0
 * @date     2024.1.11
 * @brief    host audio V1.0 module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2024 MVSilicon </center></h2>
 */
#ifndef __OTG_HOST_AUDIO_H__
#define __OTG_HOST_AUDIO_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include <string.h>
#include "otg_host_audiox.h"
#include "otg_host_hid.h"

typedef struct _AUDIO_Process
{
	uint8_t PortSpeed;	 //1-LS设备连接，2-FS设备连接，3-HS设备连接
	uint8_t MicStartFlag;
	uint8_t SpeakerStartFlag;

	uint16_t SpeakerFramesPerSecond;
	uint32_t SpeakerAccumulator;
	uint8_t  SpeakerIntervalFrames;

	uint16_t TimeBase;					//时间基准
	uint32_t FeedbackRequest;			//反馈值
	uint32_t FeedbackSampleRate; 		//最终计算出的真实采样率 整数部分Hz
	uint32_t FeedbackSampleRatemHz; 	//最终计算出的真实采样率 小数部分mHz

	USBH_AudioStreamFormatInfo   		Speaker;
	USBH_AudioStreamFormatInfo   		Mic;
	HID_ClassSpecificDescTypedef		Hid;
}
AUDIO_HandleTypeDef;


#define USBH_ISO_PACKET_SIZE(wMaxPacketSize)     ((wMaxPacketSize) & 0x07FF)
#define USBH_ISO_MULT(wMaxPacketSize)            ((((wMaxPacketSize) >> 11) & 0x03) + 1)
#define USBH_ISO_FRAME_SIZE(wMaxPacketSize)      (USBH_ISO_PACKET_SIZE(wMaxPacketSize) * USBH_ISO_MULT(wMaxPacketSize))


bool UsbHostPlayHardwareInit(void);
bool UsbHostPlayMixInit(void);
bool UsbHostPlayMixDeinit(void);
void UsbHostPlayStart(void);
void UsbHostHidDateProcess(void);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__UDISK_H__

