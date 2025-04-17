/**
 *****************************************************************************
 * @file     otg_device_audio.c
 * @author   Owen
 * @version  V1.0.0
 * @date     7-September-2015
 * @brief    device audio module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#include <string.h>
#include <math.h>
#include "main_task.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "otg_device_audio.h"

#ifdef CFG_APP_USB_AUDIO_MODE_EN

#ifdef CFG_AUDIO_WIDTH_24BIT
#define USB_AUDIO_WIDTH	24
typedef int32_t pcm_int;
typedef int64_t gain_int;
#else
#define USB_AUDIO_WIDTH	16
typedef int16_t pcm_int;
typedef int32_t gain_int;
#endif

#ifdef CFG_OTG_MODE_AUDIO1_EN
UsbAudio UsbAudioSpeaker1;
#endif
#ifdef CFG_OTG_MODE_AUDIO_EN
UsbAudio UsbAudioSpeaker;
#endif
#ifdef CFG_OTG_MODE_MIC_EN
UsbAudio UsbAudioMic;
#endif

uint8_t iso_buf[PCM_LEN_MAX*4/3];
uint8_t AudioCmd[3] = {1,0,0};
/////////////////////////////////////////
/**
 * @brief  USB����ģʽ�£����ͷ����������
 * @param  Cmd �����������
 * @return 1-�ɹ���0-ʧ��
 */
/////////////////////////
bool OTG_DeviceAudioSendPcCmd(uint16_t Cmd)
{
	AudioCmd[1] = Cmd;
	AudioCmd[2] = Cmd >> 8;
	OTG_DeviceInterruptSend(0x01,AudioCmd, 3,1000);
	AudioCmd[1] = 0;
	AudioCmd[2] = 0;
	OTG_DeviceInterruptSend(0x01,AudioCmd, 3,1000);
	return TRUE;
}
bool OTG_DeviceAudioSendHidKeyDown(uint16_t Cmd)
{
	AudioCmd[1] = Cmd;
	AudioCmd[2] = Cmd >> 8;
	OTG_DeviceInterruptSend(0x01,AudioCmd, 3,1000);
	return TRUE;
}
bool OTG_DeviceAudioSendHidKeyUp(void)
{
	AudioCmd[1] = 0;
	AudioCmd[2] = 0;
	OTG_DeviceInterruptSend(0x01,AudioCmd, 3,1000);
	return TRUE;
}
//////////////////////////////////////////////////audio core api/////////////////////////////////////////////////////

//�������뵭��
#define MixerFadeVolume(a, b, c)  	\
    if(a > b + c)		    \
	{						\
		a -= c;				\
	}						\
	else if(a + c < b)	   	\
	{						\
		a += c;				\
	}						\
	else					\
	{						\
		a = b;				\
	}

int16_t UsbValToMcuGain(int16_t UsbVal)
{
	float Db;
	Db = (float)UsbVal/256;
	return (int16_t)roundf(powf(10.0f,(Db/20.0f)) * (1<<12));
}
void UsbAudioGainApply(uint8_t *pcm_buf,uint32_t sample,int16_t left_pregain,int16_t rigth_pregain,uint8_t channel)
{
	int32_t s;
	for(s = 0; s<sample; s++)
	{
		pcm_int *pcm = (pcm_int *)pcm_buf;
		if(channel == 2)//������
		{
			pcm[2 * s + 0] = __nds32__clips(((((gain_int)pcm[2 * s + 0]) * left_pregain + 2048) >> 12), (USB_AUDIO_WIDTH)-1);
			pcm[2 * s + 1] = __nds32__clips(((((gain_int)pcm[2 * s + 1]) * rigth_pregain + 2048) >> 12), (USB_AUDIO_WIDTH)-1);
		}
		else
		{
			pcm[s] = __nds32__clips(((((gain_int)pcm[s]) * left_pregain + 2048) >> 12), (USB_AUDIO_WIDTH)-1);
		}
	}
}
#ifdef CFG_OTG_MODE_AUDIO1_EN
//pc->chip �ӻ�������ȡ����
uint16_t UsbAudioSpeaker1DataGet(void *Buffer, uint16_t Len)
{
	uint16_t Length = 0;
	int16_t left_pregain = UsbAudioSpeaker1.LeftGain;
	int16_t rigth_pregain = UsbAudioSpeaker1.RightGain;
	uint8_t channel = UsbAudioSpeaker1.Channels;
	
	if(!UsbAudioSpeaker1.PCMBuffer)
	{
		return 0;
	}
	Length = Len * sizeof(PCM_DATA_TYPE) * channel;
	Length = MCUCircular_GetData(&UsbAudioSpeaker1.CircularBuf, Buffer, Length);

	Length = Length/(sizeof(PCM_DATA_TYPE)*channel);

#ifdef CFG_RES_AUDIO_USB_VOL_SET_EN
	if(UsbAudioSpeaker1.Mute)
	{
		left_pregain = 0;
		rigth_pregain = 0;
	}
	MixerFadeVolume(UsbAudioSpeaker1.LeftCurGain,left_pregain,100);
	MixerFadeVolume(UsbAudioSpeaker1.RightCurGain,rigth_pregain,100);

	UsbAudioGainApply(Buffer,Length,UsbAudioSpeaker1.LeftCurGain,UsbAudioSpeaker1.RightCurGain,channel);
#endif
	return Length;
}

//pc->chip ��ȡ���������ݳ���
uint16_t UsbAudioSpeaker1DataLenGet(void)
{
	uint16_t Len;

	if(!UsbAudioSpeaker1.PCMBuffer)
	{
		return 0;
	}
	Len = MCUCircular_GetDataLen(&UsbAudioSpeaker1.CircularBuf);
	Len = Len / (sizeof(PCM_DATA_TYPE) * PACKET1_CHANNELS_NUM);
	return Len;
}
uint16_t UsbAudioSpeaker1DepthGet(void)
{
	uint16_t Len;
	Len = UsbAudioSpeaker1.CircularBuf.BufDepth;
	Len = Len / (sizeof(PCM_DATA_TYPE) * PACKET1_CHANNELS_NUM);
	return Len;
}
#endif

#ifdef CFG_OTG_MODE_AUDIO_EN
//pc->chip �ӻ�������ȡ����
uint16_t UsbAudioSpeakerDataGet(void *Buffer, uint16_t Len)
{
	uint16_t Length = 0;
	int16_t left_pregain = UsbAudioSpeaker.LeftGain;
	int16_t rigth_pregain = UsbAudioSpeaker.RightGain;
	uint8_t channel = UsbAudioSpeaker.Channels;
	
	if(!UsbAudioSpeaker.PCMBuffer)
	{
		return 0;
	}
	Length = Len * sizeof(PCM_DATA_TYPE) * channel;
	Length = MCUCircular_GetData(&UsbAudioSpeaker.CircularBuf, Buffer, Length);

	Length = Length/(sizeof(PCM_DATA_TYPE)*channel);

#ifdef CFG_RES_AUDIO_USB_VOL_SET_EN
	if(UsbAudioSpeaker.Mute)
	{
		left_pregain = 0;
		rigth_pregain = 0;
	}
	MixerFadeVolume(UsbAudioSpeaker.LeftCurGain,left_pregain,100);
	MixerFadeVolume(UsbAudioSpeaker.RightCurGain,rigth_pregain,100);

	UsbAudioGainApply(Buffer,Length,UsbAudioSpeaker.LeftCurGain,UsbAudioSpeaker.RightCurGain,channel);
#endif
	return Length;
}

//pc->chip ��ȡ���������ݳ���
uint16_t UsbAudioSpeakerDataLenGet(void)
{
	uint16_t Len;

	if(!UsbAudioSpeaker.PCMBuffer)
	{
		return 0;
	}
	Len = MCUCircular_GetDataLen(&UsbAudioSpeaker.CircularBuf);
	Len = Len / (sizeof(PCM_DATA_TYPE) * PACKET_CHANNELS_NUM);
//	APP_DBG("Len: %d\r\n",Len);
	return Len;
}
uint16_t UsbAudioSpeakerDepthGet(void)
{
	uint16_t Len;
	Len = UsbAudioSpeaker.CircularBuf.BufDepth;
	Len = Len / (sizeof(PCM_DATA_TYPE) * PACKET_CHANNELS_NUM);
//	APP_DBG(" UsbAudioSpeaker BufDepth: %d\r\n",Len);
	return Len;
}
#endif

#ifdef CFG_OTG_MODE_MIC_EN
//chip->pc �������ݵ�������
uint16_t UsbAudioMicDataSet(void *Buffer, uint16_t Len)
{
	int16_t left_pregain = UsbAudioMic.LeftGain;
	int16_t rigth_pregain = UsbAudioMic.RightGain;
	uint8_t channel = UsbAudioMic.Channels;

	if(!UsbAudioMic.PCMBuffer)
	{
		return 0;
	}

#ifdef CFG_RES_AUDIO_USB_VOL_SET_EN
	if(UsbAudioMic.Mute)
	{
		left_pregain = 0;
		rigth_pregain = 0;
	}
	MixerFadeVolume(UsbAudioMic.LeftCurGain,left_pregain,100);
	MixerFadeVolume(UsbAudioMic.RightCurGain,rigth_pregain,100);

	UsbAudioGainApply(Buffer,Len,UsbAudioMic.LeftCurGain,UsbAudioMic.RightCurGain,channel);
#endif

	MCUCircular_PutData(&UsbAudioMic.CircularBuf, Buffer, Len * sizeof(PCM_DATA_TYPE) * channel);
	return Len;
}

//chip->pc ���ݻ�����ʣ��ռ�
uint16_t UsbAudioMicSpaceLenGet(void)
{
	uint16_t Len;

	if(!UsbAudioMic.PCMBuffer)
	{
		return 0;
	}
	Len = MCUCircular_GetSpaceLen(&UsbAudioMic.CircularBuf);
	Len = Len / (sizeof(PCM_DATA_TYPE) * UsbAudioMic.Channels);
	return Len;
}

uint16_t UsbAudioMicDepthGet(void)
{
	uint16_t Len;
	Len = UsbAudioMic.CircularBuf.BufDepth;
	Len = Len / (sizeof(PCM_DATA_TYPE) * MIC_CHANNELS_NUM);
//	APP_DBG(" UsbAudioMic BufDepth: %d\r\n",Len);
	return Len;
}
#endif

#ifdef CFG_OTG_MODE_AUDIO1_EN
//ע��һ����Ҫ4�ֽڶ���
void OnDeviceAudioRcvIsoPacket1(void)
{

	uint32_t Len;
	int32_t s;
	uint32_t sample = 0;
	OTG_DeviceISOReceive(DEVICE_ISO_OUT_EP1, (uint8_t*)iso_buf, OUT1_PCM_LEN, &Len);

	UsbAudioSpeaker1.FramCount++;

	if(UsbAudioSpeaker1.ByteSet == PCM16BIT)
	{
		sample = Len/PCM16BIT;
#ifdef CFG_AUDIO_WIDTH_24BIT
		for(s = sample-1; s >= 0; s--)
		{
//			memcpy(&iso_buf[s*4+1],&iso_buf[s*2],4);
			iso_buf[s*4+2] = iso_buf[s*2+1];
			iso_buf[s*4+1] = iso_buf[s*2];

			if(iso_buf[s*4+2] & 0x80)
			{
				iso_buf[s*4+3] = 0xff;
			}else{
				iso_buf[s*4+3] = 0x00;
			}
			iso_buf[s*4] = 0x00;
		}
#endif
	}
	else if(UsbAudioSpeaker1.ByteSet == PCM24BIT)
	{
		sample = Len/PCM24BIT;
#ifdef CFG_AUDIO_WIDTH_24BIT
		for(s = sample-1; s >= 0; s--)
		{
//			memcpy(&iso_buf[s*4],&iso_buf[s*3],4);
			iso_buf[s*4+2] = iso_buf[s*3+2];
			iso_buf[s*4+1] = iso_buf[s*3+1];
			iso_buf[s*4] = iso_buf[s*3];

			if(iso_buf[s*4+2] & 0x80)
			{
				iso_buf[s*4+3] = 0xff;
			}else{
				iso_buf[s*4+3] = 0x00;
			}
		}
#else
		for(s = 0; s < sample; s++)
		{
			iso_buf[s*2] = iso_buf[s*3+1];
			iso_buf[s*2+1] = iso_buf[s*3+2];
		}
#endif
	}

	if(UsbAudioSpeaker1.PCMBuffer == NULL)
	{
		return;
	}

	MCUCircular_PutData(&UsbAudioSpeaker1.CircularBuf, (uint8_t*)iso_buf, sample*sizeof(PCM_DATA_TYPE));
}
#endif

//ע��һ����Ҫ4�ֽڶ���
#ifdef CFG_OTG_MODE_AUDIO_EN
void OnDeviceAudioRcvIsoPacket(void)
{

	uint32_t Len;
	int32_t s;
	uint32_t sample = 0;
	OTG_DeviceISOReceive(DEVICE_ISO_OUT_EP, (uint8_t*)iso_buf, OUT_PCM_LEN, &Len);

	UsbAudioSpeaker.FramCount++;

	if(UsbAudioSpeaker.ByteSet == PCM16BIT)
	{
		sample = Len/PCM16BIT;
#ifdef CFG_AUDIO_WIDTH_24BIT
		for(s = sample-1; s >= 0; s--)
		{
//			memcpy(&iso_buf[s*4+1],&iso_buf[s*2],4);
			iso_buf[s*4+2] = iso_buf[s*2+1];
			iso_buf[s*4+1] = iso_buf[s*2];

			if(iso_buf[s*4+2] & 0x80)
			{
				iso_buf[s*4+3] = 0xff;
			}else{
				iso_buf[s*4+3] = 0x00;
			}
			iso_buf[s*4] = 0x00;
		}
#endif
	}
	else if(UsbAudioSpeaker.ByteSet == PCM24BIT)
	{
		sample = Len/PCM24BIT;
#ifdef CFG_AUDIO_WIDTH_24BIT
		for(s = sample-1; s >= 0; s--)
		{
//			memcpy(&iso_buf[s*4],&iso_buf[s*3],4);
			iso_buf[s*4+2] = iso_buf[s*3+2];
			iso_buf[s*4+1] = iso_buf[s*3+1];
			iso_buf[s*4] = iso_buf[s*3];

			if(iso_buf[s*4+2] & 0x80)
			{
				iso_buf[s*4+3] = 0xff;
			}else{
				iso_buf[s*4+3] = 0x00;
			}
		}
#else
		for(s = 0; s < sample; s++)
		{
			iso_buf[s*2] = iso_buf[s*3+1];
			iso_buf[s*2+1] = iso_buf[s*3+2];
		}
#endif
	}

	if(UsbAudioSpeaker.PCMBuffer == NULL)
	{
		return;
	}

	MCUCircular_PutData(&UsbAudioSpeaker.CircularBuf, (uint8_t*)iso_buf, sample*sizeof(PCM_DATA_TYPE));
}
#endif

#ifdef CFG_OTG_MODE_MIC_EN
void OnDeviceAudioSendIsoPacket(void)
{
	int32_t s;
	uint32_t RealLen = 0;
	uint8_t channel = UsbAudioMic.Channels;
	UsbAudioMic.FramCount++;

	RealLen = UsbAudioMic.AudioSampleRate/1000;
	UsbAudioMic.Accumulator += UsbAudioMic.AudioSampleRate%1000;
	if(UsbAudioMic.Accumulator > 1000)
	{
		UsbAudioMic.Accumulator -= 1000;
		RealLen += 1;
	}

/*	if(UsbAudioMic.FramCount < (UsbAudioMic.AudioSampleRate/AudioCoreMixSampleRateGet(DefaultNet))*10)
	{
		memset(iso_buf,0,RealLen*sizeof(PCM_DATA_TYPE)*channel);
	}
	else */if(UsbAudioMic.PCMBuffer != NULL)
	{
		if(MCUCircular_GetDataLen(&UsbAudioMic.CircularBuf) < RealLen*sizeof(PCM_DATA_TYPE)*channel)
		{
			memset(iso_buf,0,RealLen*sizeof(PCM_DATA_TYPE)*channel);
		}
		else
		{
			MCUCircular_GetData(&UsbAudioMic.CircularBuf, iso_buf,RealLen*sizeof(PCM_DATA_TYPE)*channel);
		}
	}

	if(UsbAudioMic.ByteSet == PCM16BIT)
	{
#ifdef CFG_AUDIO_WIDTH_24BIT
		int32_t *PcmBuf32 = (int32_t *)iso_buf;
		int16_t *PcmBuf16 = (int16_t *)iso_buf;
		for(s=0; s < RealLen*channel; s++)
		{
			PcmBuf16[s] = PcmBuf32[s] >> 8;
		}
#endif
	}
	else if(UsbAudioMic.ByteSet == PCM24BIT)
	{
#ifdef CFG_AUDIO_WIDTH_24BIT
		for(s = 0; s<RealLen*channel; s++)
		{
			memcpy(&iso_buf[s*3],&iso_buf[s*4],4);
		}
#else
		for(s = RealLen*channel-1; s >= 0; s--)
		{
			iso_buf[s*3+2] = iso_buf[s*2+1];
			iso_buf[s*3+1] = iso_buf[s*2];
		}
#endif
	}
	OTG_DeviceISOSend(DEVICE_ISO_IN_EP,(uint8_t*)iso_buf,RealLen*UsbAudioMic.ByteSet*channel);
}
#endif

//USB�豸�����λ-����
void OTG_DeviceAudioSoftwareReset(void)
{
#ifdef CFG_OTG_MODE_AUDIO1_EN
	//�����FIFO,ֻ���usb�����������
	memset(&UsbAudioSpeaker1,0,sizeof(UsbAudio)-sizeof(MCU_CIRCULAR_CONTEXT)-sizeof(int16_t*));
	UsbAudioSpeaker1.Channels   = PACKET1_CHANNELS_NUM;
	UsbAudioSpeaker1.LeftGain    = UsbValToMcuGain(AUDIO_MAX_VOLUME);
	UsbAudioSpeaker1.RightGain   = UsbValToMcuGain(AUDIO_MAX_VOLUME);
#endif

#ifdef CFG_OTG_MODE_AUDIO_EN
	//�����FIFO,ֻ���usb�����������
	memset(&UsbAudioSpeaker,0,sizeof(UsbAudio)-sizeof(MCU_CIRCULAR_CONTEXT)-sizeof(int16_t*));
	UsbAudioSpeaker.Channels   = PACKET_CHANNELS_NUM;
	UsbAudioSpeaker.LeftGain    = UsbValToMcuGain(AUDIO_MAX_VOLUME);
	UsbAudioSpeaker.RightGain   = UsbValToMcuGain(AUDIO_MAX_VOLUME);
#endif

#ifdef CFG_OTG_MODE_MIC_EN
	//�����FIFO,ֻ���usb�����������
	memset(&UsbAudioMic,0,sizeof(UsbAudio)-sizeof(MCU_CIRCULAR_CONTEXT)-sizeof(int16_t*));
	UsbAudioMic.Channels       = MIC_CHANNELS_NUM;
	UsbAudioMic.LeftGain        = UsbValToMcuGain(AUDIO_MAX_VOLUME);
	UsbAudioMic.RightGain       = UsbValToMcuGain(AUDIO_MAX_VOLUME);
#endif
}


#define RequestCmd	((Setup[0] << 8) | Setup[1])
#define Channel		Setup[2]
#define Control		Setup[3]
#define Entity		Setup[5]
#define wLength		((Setup[7] << 8) | Setup[6])

#if (USB_AUDIO_PROTOCOL == AUDIO_UAC_10)
//�����������������
static void AudioVolumeRequest(UsbAudio *Stream)
{
	if(Control == AUDIO_CONTROL_MUTE)
	{
		if(RequestCmd == GET_CUR)
		{
			Setup[0] = Stream->Mute;
			OTG_DeviceControlSend(Setup,1,3);
		}
		else if(RequestCmd == SET_CUR)
		{
			Stream->Mute=Request[0];
		}
		else
		{
			//APP_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
	else if(Control == AUDIO_CONTROL_VOLUME)
	{
		if(RequestCmd == GET_MIN)
		{
			OTG_DeviceSendResp(AUDIO_MIN_VOLUME, 2);
		}
		else if(RequestCmd == GET_MAX)
		{
			OTG_DeviceSendResp(AUDIO_MAX_VOLUME, 2);
		}
		else if(RequestCmd == GET_RES)
		{
			OTG_DeviceSendResp(AUDIO_RES_VOLUME, 2);
		}
		else if(RequestCmd == GET_CUR)
		{
			int16_t Vol = 0;
			if(Channel == 0x01)
			{
				Vol = Stream->LeftVol;
			}
			else
			{
				Vol = Stream->RightVol;
			}
			OTG_DeviceSendResp(Vol, 2);
		}
		else if(RequestCmd == SET_CUR)
		{
			int16_t Temp = 0;
			Temp = Request[1]* 256 + Request[0];
			if(Setup[2] == 0x01)
			{
				Stream->LeftVol = Temp;
				Stream->LeftGain = UsbValToMcuGain(Temp);
			}
			else
			{
				Stream->RightVol = Temp;
				Stream->RightGain = UsbValToMcuGain(Temp);
			}
		}
		else
		{
			//APP_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
}
//�������������������
static void AudioSampleRateRequest(UsbAudio *Stream,uint8_t Num,uint8_t AudioCoreNum)
{
	uint32_t Temp = 0;
	if(RequestCmd == SET_CUR_EP)
	{
		if(Stream->AudioSampleRate != SWAP_BUF_TO_U32(Request))
		{
			Stream->AudioSampleRate = SWAP_BUF_TO_U32(Request);
			APP_DBG("Stream->AudioSampleRate:%u\n",(unsigned int)Stream->AudioSampleRate);
			if(Num)
			{
				AudioCoreSinkChange(AudioCoreNum, Stream->Channels, Stream->AudioSampleRate);
			}
			else
			{
				AudioCoreSourceChange(AudioCoreNum, Stream->Channels, Stream->AudioSampleRate);
			}
		}
	}
	if(RequestCmd == GET_CUR_EP)
	{
		Temp = Stream->AudioSampleRate;
		Setup[0] = (Temp>>0 ) & 0x000000FF;
		Setup[1] = (Temp>>8 ) & 0x000000FF;
		Setup[2] = (Temp>>16) & 0x000000FF;
		OTG_DeviceControlSend(Setup,3,3);
	}
}
//usb�����ӿ����������
void OTG_DeviceAudioInterfaceRequest(void)
{
#ifdef CFG_OTG_MODE_MIC_EN
	if(Entity == AUDIO_MIC_FU_ID)
	{
//		APP_DBG("Mic\n");
		return AudioVolumeRequest(&UsbAudioMic);
	}
#endif
#ifdef CFG_OTG_MODE_AUDIO_EN
	if(Entity == AUDIO_SPEAKER_FU_ID)
	{
//		APP_DBG("Speaker\n");
		return AudioVolumeRequest(&UsbAudioSpeaker);
	}
#endif
#ifdef CFG_OTG_MODE_AUDIO1_EN
	if(Entity == AUDIO_SPEAKER1_FU_ID)
	{
//		APP_DBG("Speaker1\n");
		return AudioVolumeRequest(&UsbAudioSpeaker1);
	}
#endif
}
//usb�����˵����������
void OTG_DeviceAudioEpRequest(void)
{
#ifdef CFG_OTG_MODE_MIC_EN
	if(Setup[4] == DEVICE_ISO_IN_EP)
	{
//		APP_DBG("Mic\n");
		return AudioSampleRateRequest(&UsbAudioMic,1,USB_AUDIO_SINK_NUM);
	}
#endif
#ifdef CFG_OTG_MODE_AUDIO_EN
	if(Setup[4] == DEVICE_ISO_OUT_EP)
	{
//		APP_DBG("Speaker\n");
		return AudioSampleRateRequest(&UsbAudioSpeaker,0,USB_AUDIO_SOURCE_NUM);
	}
#endif
#ifdef CFG_OTG_MODE_AUDIO1_EN
	if(Setup[4] == DEVICE_ISO_OUT_EP1)
	{
//		APP_DBG("Speaker1\n");
		return AudioSampleRateRequest(&UsbAudioSpeaker1,0,USB_AUDIO_SOURCE1_NUM);
	}
#endif
}

#elif(USB_AUDIO_PROTOCOL == AUDIO_UAC_20)
//�����������������
static void AudioVolumeRequest(UsbAudio *Stream)
{
	if(Control == AUDIO_CONTROL_MUTE)
	{
		//Speaker mute�Ĳ���
		if(RequestCmd == GET_CUR_2)
		{
			Setup[0] = Stream->Mute;
			OTG_DeviceControlSend(Setup,1,3);
		}
		else if(RequestCmd == SET_CUR)
		{
			APP_DBG("mute: %d\n", Request[0]);
			Stream->Mute=Request[0];
		}
		else
		{
			//APP_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
	else if(Control == AUDIO_CONTROL_VOLUME)
	{
		if(RequestCmd == GET_RANGE)
		{
			uint8_t AudioCtl[8];
			AudioCtl[0] = 1;    //wNumSubRanges
			AudioCtl[1] = 0;
			AudioCtl[2] = (uint8_t)(AUDIO_MIN_VOLUME);    //wMIN(1)
			AudioCtl[3] = (uint8_t)(AUDIO_MIN_VOLUME >> 8);
			AudioCtl[4] = (uint8_t)(AUDIO_MAX_VOLUME);  //wMAX(1)
			AudioCtl[5] = (uint8_t)(AUDIO_MAX_VOLUME >> 8);
			AudioCtl[6] = (uint8_t)(AUDIO_RES_VOLUME);    //wRES(1)
			AudioCtl[7] = (uint8_t)(AUDIO_RES_VOLUME >> 8);
			OTG_DeviceControlSend(AudioCtl, sizeof(AudioCtl),3);
		}
		else if(RequestCmd == GET_CUR_2)
		{
			int16_t Vol = 0;
			if(Channel == 0x01)
			{
				Vol = Stream->LeftVol;
			}
			else
			{
				Vol = Stream->RightVol;
			}
			OTG_DeviceSendResp(Vol, 2);
		}
		else if(RequestCmd == SET_CUR)
		{
			uint16_t Temp = 0;
			Temp = Request[1]* 256 + Request[0];
			if(Setup[2] == 0x01)
			{
				Stream->LeftVol = Temp;
				Stream->LeftGain = UsbValToMcuGain(Temp);
			}
			else
			{
				Stream->RightVol = Temp;
				Stream->RightGain = UsbValToMcuGain(Temp);
			}
		}
	}
}

//Get Layout 3 parameter block
const uint8_t para_block_Mic[] = {
	SAMPLE_FREQ_NUM(MIC_FREQ_NUM),           /* wNumSubRanges */
#if (MIC_FREQ_NUM >= 6)
    SAMPLE_FREQ_4B(USBD_AUDIO_MIC_FREQ5),        /* dMIN(1) */
    SAMPLE_FREQ_4B(USBD_AUDIO_MIC_FREQ5),        /* dMAX(1) */
    SAMPLE_FREQ_4B(0x00),         /* dRES(1) */
#endif
#if (MIC_FREQ_NUM >= 5)
    SAMPLE_FREQ_4B(USBD_AUDIO_MIC_FREQ4),        /* dMIN(1) */
    SAMPLE_FREQ_4B(USBD_AUDIO_MIC_FREQ4),        /* dMAX(1) */
    SAMPLE_FREQ_4B(0x00),         /* dRES(1) */
#endif
#if (MIC_FREQ_NUM >= 4)
    SAMPLE_FREQ_4B(USBD_AUDIO_MIC_FREQ3),        /* dMIN(1) */
    SAMPLE_FREQ_4B(USBD_AUDIO_MIC_FREQ3),        /* dMAX(1) */
    SAMPLE_FREQ_4B(0x00),         /* dRES(1) */
#endif
#if (MIC_FREQ_NUM >= 3)
    SAMPLE_FREQ_4B(USBD_AUDIO_MIC_FREQ2),        /* dMIN(1) */
    SAMPLE_FREQ_4B(USBD_AUDIO_MIC_FREQ2),        /* dMAX(1) */
    SAMPLE_FREQ_4B(0x00),         /* dRES(1) */
#endif
#if (MIC_FREQ_NUM >= 2)
    SAMPLE_FREQ_4B(USBD_AUDIO_MIC_FREQ1),        /* dMIN(1) */
    SAMPLE_FREQ_4B(USBD_AUDIO_MIC_FREQ1),        /* dMAX(1) */
    SAMPLE_FREQ_4B(0x00),         /* dRES(1) */
#endif
#if (MIC_FREQ_NUM >= 1)
    SAMPLE_FREQ_4B(USBD_AUDIO_MIC_FREQ),        /* dMIN(2) */
    SAMPLE_FREQ_4B(USBD_AUDIO_MIC_FREQ),        /* dMAX(2) */
    SAMPLE_FREQ_4B(0x00),         /* dRES(2) */
#endif
};

//Get Layout 3 parameter block
const uint8_t para_block_Speaker[] = {
	SAMPLE_FREQ_NUM(SPEAKER_FREQ_NUM),           /* wNumSubRanges */
#if (SPEAKER_FREQ_NUM >= 6)
    SAMPLE_FREQ_4B(USBD_AUDIO_FREQ5),        /* dMIN(1) */
    SAMPLE_FREQ_4B(USBD_AUDIO_FREQ5),        /* dMAX(1) */
    SAMPLE_FREQ_4B(0x00),         /* dRES(1) */
#endif
#if (SPEAKER_FREQ_NUM >= 5)
    SAMPLE_FREQ_4B(USBD_AUDIO_FREQ4),        /* dMIN(1) */
    SAMPLE_FREQ_4B(USBD_AUDIO_FREQ4),        /* dMAX(1) */
    SAMPLE_FREQ_4B(0x00),         /* dRES(1) */
#endif
#if (SPEAKER_FREQ_NUM >= 4)
    SAMPLE_FREQ_4B(USBD_AUDIO_FREQ3),        /* dMIN(1) */
    SAMPLE_FREQ_4B(USBD_AUDIO_FREQ3),        /* dMAX(1) */
    SAMPLE_FREQ_4B(0x00),         /* dRES(1) */
#endif
#if (SPEAKER_FREQ_NUM >= 3)
    SAMPLE_FREQ_4B(USBD_AUDIO_FREQ2),        /* dMIN(1) */
    SAMPLE_FREQ_4B(USBD_AUDIO_FREQ2),        /* dMAX(1) */
    SAMPLE_FREQ_4B(0x00),         /* dRES(1) */
#endif
#if (SPEAKER_FREQ_NUM >= 2)
    SAMPLE_FREQ_4B(USBD_AUDIO_FREQ1),        /* dMIN(1) */
    SAMPLE_FREQ_4B(USBD_AUDIO_FREQ1),        /* dMAX(1) */
    SAMPLE_FREQ_4B(0x00),         /* dRES(1) */
#endif
#if (SPEAKER_FREQ_NUM >= 1)
    SAMPLE_FREQ_4B(USBD_AUDIO_FREQ),        /* dMIN(2) */
    SAMPLE_FREQ_4B(USBD_AUDIO_FREQ),        /* dMAX(2) */
    SAMPLE_FREQ_4B(0x00),         /* dRES(2) */
#endif
};

//�������������������
static void AudioSampleRateRequest(UsbAudio *Stream,uint8_t Num,uint8_t AudioCoreNum)
{
	uint8_t *para_block;
	uint16_t block_size;
	if(RequestCmd == SET_CUR)
	{
		if(Stream->AudioSampleRate != SWAP_BUF_TO_U32(Request))
		{
			Stream->AudioSampleRate = SWAP_BUF_TO_U32(Request);
			APP_DBG("Stream->AudioSampleRate:%u\n",(unsigned int)Stream->AudioSampleRate);
			if(Num)
			{
				AudioCoreSinkChange(AudioCoreNum, Stream->Channels, Stream->AudioSampleRate);
			}
			else
			{
				AudioCoreSourceChange(AudioCoreNum, Stream->Channels, Stream->AudioSampleRate);
			}
		}
	}
	else if(RequestCmd == GET_CUR_2)
	{
		OTG_DeviceControlSend((uint8_t*)&Stream->AudioSampleRate, wLength,3);
	}
	else if(RequestCmd == GET_RANGE)
	{
		if(Num)
		{
			para_block = (uint8_t*)para_block_Mic;
			block_size = sizeof(para_block_Mic);
		}
		else
		{
			para_block = (uint8_t*)para_block_Speaker;
			block_size = sizeof(para_block_Speaker);
		}

        if(wLength > block_size)
        {
            OTG_DeviceControlSend(para_block, block_size,3);
        }
        else
        {
        	OTG_DeviceControlSend(para_block, wLength,3);
        }
	}
}
//usb�����ӿ����������
void OTG_DeviceAudioInterfaceRequest(void)
{
	if((Entity == AUDIO_FU_ID))
	{
//		APP_DBG("Speaker\n");
		return AudioVolumeRequest(&UsbAudioSpeaker);
	}
	else if((Entity == AUDIO_MIC_FU_ID))
	{
//		APP_DBG("Mic\n");
		return AudioVolumeRequest(&UsbAudioMic);
	}
	else if(Entity == AUDIO_MIC_CLK_ID)
	{
//		APP_DBG("Mic\n");
		return AudioSampleRateRequest(&UsbAudioMic,1,USB_AUDIO_SINK_NUM);
	}
	else if(Entity == AUDIO_CLK_ID)
	{
//		APP_DBG("Speaker\n");
		return AudioSampleRateRequest(&UsbAudioSpeaker,0,USB_AUDIO_SOURCE_NUM);
	}
}
//usb�����˵����������
void OTG_DeviceAudioEpRequest(void)
{
	//usb����2.0Э�����޶˵�ָ��
}
#endif


static uint32_t FramCount = 0;
void UsbAudioTimer1msProcess(void)
{
#ifndef CFG_FUNC_USB_AUDIO_MIX_MODE
	if(GetSystemMode() != ModeUsbDevicePlay)
	{
		return;
	}
#else
	if(!GetUSBDeviceInitState())
	{
		return;
	}
#endif
	FramCount++;
	if(FramCount % 2)//2ms
	{
		return;
	}
#ifdef CFG_OTG_MODE_AUDIO1_EN
	if(UsbAudioSpeaker1.AltSet)//open stream
	{
		if(UsbAudioSpeaker1.FramCount)//���ڴ����� 1-2֡����
		{
			if(UsbAudioSpeaker1.FramCount != UsbAudioSpeaker1.TempFramCount)
			{
				UsbAudioSpeaker1.TempFramCount = UsbAudioSpeaker1.FramCount;
				if(!AudioCoreSourceIsEnable(USB_AUDIO_SOURCE1_NUM))
				{
					AudioCoreSourceEnable(USB_AUDIO_SOURCE1_NUM);
				}
			}
			else
			{
				AudioCoreSourceDisable(USB_AUDIO_SOURCE1_NUM);
			}
		}
	}
	else
	{
		UsbAudioSpeaker1.FramCount = 0;
		UsbAudioSpeaker1.TempFramCount = 0;
		AudioCoreSourceDisable(USB_AUDIO_SOURCE1_NUM);
	}
	if(UsbAudioSpeaker1.AltSlow)
	{
		if(UsbAudioSpeaker1.FramCount > 100)
		{
			AudioCoreSourceUnmute(USB_AUDIO_SOURCE1_NUM,1,1);
			UsbAudioSpeaker1.AltSlow = 0;
		}
		else
		{
			AudioCoreSourceMute(USB_AUDIO_SOURCE1_NUM,1,1);
		}
	}
#endif

#ifdef CFG_OTG_MODE_AUDIO_EN
	if(UsbAudioSpeaker.AltSet)//open stream
	{
		if(UsbAudioSpeaker.FramCount)//���ڴ����� 1-2֡����
		{
			if(UsbAudioSpeaker.FramCount != UsbAudioSpeaker.TempFramCount)
			{
				UsbAudioSpeaker.TempFramCount = UsbAudioSpeaker.FramCount;
				if(!AudioCoreSourceIsEnable(USB_AUDIO_SOURCE_NUM))
				{
					AudioCoreSourceEnable(USB_AUDIO_SOURCE_NUM);
				}
			}
			else
			{
				AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);
			}
		}
	}
	else
	{
		UsbAudioSpeaker.FramCount = 0;
		UsbAudioSpeaker.TempFramCount = 0;
		AudioCoreSourceDisable(USB_AUDIO_SOURCE_NUM);
	}
	if(UsbAudioSpeaker.AltSlow)
	{
		if(UsbAudioSpeaker.FramCount > 100)
		{
			AudioCoreSourceUnmute(USB_AUDIO_SOURCE_NUM,1,1);
			UsbAudioSpeaker.AltSlow = 0;
		}
		else
		{
			AudioCoreSourceMute(USB_AUDIO_SOURCE_NUM,1,1);
		}
	}

#endif

#ifdef CFG_OTG_MODE_MIC_EN
	if(UsbAudioMic.AltSet)//open stream
	{
		if(UsbAudioMic.FramCount)//���ڴ����� �д�����1-2֡����
		{
			if(UsbAudioMic.FramCount != UsbAudioMic.TempFramCount)
			{
				UsbAudioMic.TempFramCount = UsbAudioMic.FramCount;
				if(!AudioCoreSinkIsEnable(USB_AUDIO_SINK_NUM))
				{
					AudioCoreSinkEnable(USB_AUDIO_SINK_NUM);
				}
			}
		}
	}
	else
	{
		UsbAudioMic.Accumulator = 0;
		UsbAudioMic.FramCount = 0;
		UsbAudioMic.TempFramCount = 0;
		AudioCoreSinkDisable(USB_AUDIO_SINK_NUM);
	}
#endif
}
#endif
