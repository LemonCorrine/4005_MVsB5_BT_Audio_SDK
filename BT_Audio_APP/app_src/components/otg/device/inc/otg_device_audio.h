/**
 *****************************************************************************
 * @file     otg_device_audio.h
 * @author   Owen
 * @version  V1.0.0
 * @date     24-June-2015
 * @brief    audio device interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#ifndef __OTG_DEVICE_AUDIO_H__
#define	__OTG_DEVICE_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
	
#include "type.h"
#include "resampler_polyphase.h"
#include "mcu_circular_buf.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

#define DB_TO_USBVAL(db)	((int16_t)(db*256))

#define AUDIO_MAX_VOLUME	DB_TO_USBVAL(0)		//0db
#define AUDIO_MIN_VOLUME	DB_TO_USBVAL(-60)	//-60db
#define AUDIO_RES_VOLUME	0x0080				//0.5db


typedef struct _UsbAudio
{
	uint8_t					InitOk;
	uint8_t 				AltSlow;
	uint8_t					AltSet;
	uint8_t					ByteSet;
	uint8_t 				Channels;
	uint8_t 				Mute;
	int16_t					LeftVol;
	int16_t					RightVol;
	int16_t					LeftGain;
	int16_t					RightGain;
	uint32_t				AudioSampleRate;
	uint32_t				FramCount;
	uint32_t				TempFramCount;
	uint32_t 				Accumulator;
#ifdef CFG_RES_AUDIO_USB_VOL_SET_EN
	int16_t					LeftCurGain;
	int16_t					RightCurGain;
#endif
	//缓存FIFO
	MCU_CIRCULAR_CONTEXT 	CircularBuf;
	int16_t*				PCMBuffer;
}UsbAudio;
extern UsbAudio UsbAudioSpeaker;
extern UsbAudio UsbAudioMic;

bool OTG_DeviceAudioSendPcCmd(uint8_t Cmd);

void PCAudioPP(void);
void PCAudioNext(void);
void PCAudioPrev(void);
void PCAudioStop(void);
void PCAudioVolUp(void);
void PCAudioVolDn(void);

//pc->chip 从缓存区获取数据
uint16_t UsbAudioSpeakerDataGet(void *Buffer,uint16_t Len);
//pc->chip 获取缓存区数据长度
uint16_t UsbAudioSpeakerDataLenGet(void);
uint16_t UsbAudioSpeakerDepthGet(void);


//chip->pc 保存数据到缓存区
uint16_t UsbAudioMicDataSet(void *Buffer,uint16_t Len);
//chip->pc 数据缓存区剩余空间
uint16_t UsbAudioMicSpaceLenGet(void);
uint16_t UsbAudioMicDepthGet(void);
int16_t UsbValToMcuGain(int16_t UsbVal);
#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
