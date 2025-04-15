

#ifndef __OTG_DEVICE_STANDARD_H__
#define	__OTG_DEVICE_STANDARD_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 
#include "app_config.h"
#include "type.h"

#define USB_DT_DEVICE					1
#define USB_DT_CONFIG					2
#define USB_DT_STRING					3
#define USB_DT_INTERFACE				4
#define USB_DT_ENDPOINT					5
#define USB_DT_DEVICE_QUALIFIER			6
#define USB_HID_REPORT					0x22

#define USB_REQ_GET_STATUS				0
#define USB_REQ_CLEAR_FEATURE			1
#define USB_REQ_SET_FEATURE				3
#define USB_REQ_SET_ADDRESS				5
#define USB_REQ_GET_DESCRIPTOR			6
#define USB_REQ_SET_DESCRIPTOR			7
#define USB_REQ_GET_CONFIGURATION		8
#define USB_REQ_SET_CONFIGURATION		9
#define USB_REQ_GET_INTERFACE			10
#define USB_REQ_SET_INTERFACE			11
#define USB_REQ_SYNCH_FRAME				12

#define MSC_INTERFACE_NUM			0
#define AUDIO_ATL_INTERFACE_NUM		1
#define AUDIO_SRM_OUT_INTERFACE_NUM	2
#define AUDIO_SRM_IN_INTERFACE_NUM	3
#define HID_CTL_INTERFACE_NUM		4
#define HID_DATA_INTERFACE_NUM		5

#define USB_VID				0x1236
#define USB_PID_BASE		0x17B1//具体PID叠加下列功能值作为Offset
#define AUDIO_ONLY			0
#define MIC_ONLY			1
#define AUDIO_MIC			2

#ifdef USB_READER_EN
#define READER				3
#define AUDIO_READER		4
#define MIC_READER			5
#define AUDIO_MIC_READER	6
#endif

#define AUDIO_UAC_10	10
#define AUDIO_UAC_20	20
#define USB_AUDIO_PROTOCOL	AUDIO_UAC_10


#define HID					7
#if (USB_AUDIO_PROTOCOL == AUDIO_UAC_10)
#define USBPID(x)			(USB_PID_BASE + x)
#elif (USB_AUDIO_PROTOCOL == AUDIO_UAC_20)
#define USBPID(x)			(USB_PID_BASE + 1 + x)
#endif
/* UAC 2.0 */

#define AUDIO_CONFIG_DESC_SIZE                        145+110+43-4
#define AUDIO_CONFIG_DESC_MIC_ONLY_SIZE               145+43
#define AUDIO_CONFIG_DESC_AUDIO_ONLY_SIZE             145+43

#define AUDIO_INTERFACE_DESC_SIZE                     9
#define USB_AUDIO_DESC_SIZ                            0x09
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE            0x07

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05
#define USB_SIZ_DEVICE_DESC                     18
#define USB_SIZ_STRING_LANGID                   4

#define AUDIO_DESCRIPTOR_TYPE                         0x21
#define USB_DEVICE_CLASS_AUDIO                        0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02
#define AUDIO_PROTOCOL_UNDEFINED                      0x00
#define AUDIO_IP_VERSION_02_00                        0x20
#define AUDIO_STREAMING_GENERAL                       0x01
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02
#define AUDIO_STREAMING_ENCODER                       0x03

/* Audio Descriptor Types */
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_HEADER                          0x01
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06
#define AUDIO_CONTROL_CLOCK_SOURCE                    0x0A

/* Audio Function Category Codes */
#define AUDIO_DESKTOP_SPEAKER                         0x01
#define AUDIO_HOME_THEATER                            0x02
#define AUDIO_MICROPHONE                              0x03
#define AUDIO_HEADSET                                 0x04

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07

#define AUDIO_CONTROL_MUTE                            0x01
#define AUDIO_CONTROL_VOLUME                          0x02

#define AUDIO_20_CLK_SOURCE_DESC_SIZE                 0x08
#define AUDIO_20_IT_DESC_SIZE                         0x11
#define AUDIO_20_OT_DESC_SIZE                         0x0C
#define AUDIO_20_STREAMING_INTERFACE_DESC_SIZE        0x10

#define CONTROL_BITMAP_NONE                           (0x00)
#define CONTROL_BITMAP_RO                             (0x01)
#define CONTROL_BITMAP_PROG                           (0x03)

#define AUDIO_20_CTL_MUTE(bmaControl)                 (bmaControl)
#define AUDIO_20_CTL_VOLUME(bmaControl)               (bmaControl<<2)

#define AUDIO_20_STANDARD_ENDPOINT_DESC_SIZE          0x07
#define AUDIO_20_STREAMING_ENDPOINT_DESC_SIZE         0x08

#define AUDIO_FORMAT_TYPE_I                           0x01
#define AUDIO_FORMAT_TYPE_II                          0x02
#define AUDIO_FORMAT_TYPE_III                         0x03
#define AUDIO_FORMAT_TYPE_IV                          0x04

#define AUDIO_IT_ID                                   0x01
#define AUDIO_FU_ID                                   0x02
#define AUDIO_OT_ID                                   0x03
#define AUDIO_CLK_ID                                  0x08

#define AUDIO_MIC_IT_ID								  0x04
#define AUDIO_MIC_FU_ID								  0x05
#define AUDIO_MIC_SL_ID								  0x06
#define AUDIO_MIC_OT_ID								  0x07
#define AUDIO_MIC_CLK_ID                              0x09

#define USB_ENDPOINT_TYPE_ISOCHRONOUS                 0x01
#define USB_ENDPOINT_TYPE_ASYNCHRONOUS				  0x05
#define USB_ENDPOINT_TYPE_ADAPTIVE                    0x09
#define USB_ENDPOINT_TYPE_SYNCHRONOUS                 0x0D
#define AUDIO_ENDPOINT_GENERAL                        0x01

//works: 48 KHz, 16bit - mode 0
#if (CFG_PARA_USB_MODE == MIC_ONLY)
#define	USBD_AUDIO_MIC_FREQ								192000		//48000 : bits per seconds
#define	USBD_AUDIO_MIC_FREQ1							96000		//44100 : bits per seconds
#define MIC_BYTES								  		3			//3 half word (one channel)
#define MIC_SAMPLE_BITS									24			//24 bit per channel
#define MIC_CHANNELS_NUM								1
#else
#define	USBD_AUDIO_MIC_FREQ								192000		//48000 : bits per seconds
#define	USBD_AUDIO_MIC_FREQ1							96000		//44100 : bits per seconds
#define	USBD_AUDIO_MIC_FREQ2							48000		//44100 : bits per seconds
#define MIC_BYTES								  		3			//3 half word (one channel)
#define MIC_SAMPLE_BITS									24			//24 bit per channel
#define MIC_CHANNELS_NUM								1
#endif

#define	USBD_AUDIO_FREQ								48000		//96000 : bits per seconds
#define	USBD_AUDIO_FREQ1							44100		//48000 : bits per seconds
#define	USBD_AUDIO_FREQ2							44100		//44100 : bits per seconds
#define PACKET_BYTES								3			//2 half word (one channel)
#define PACKET_SAMPLE_BITS							24			//16 bit per channel
#define PACKET_CHANNELS_NUM							2

#ifndef USE_USB_OTG_HS
#define AUDIO_PACKET_SZE(frq)          (uint8_t)(((frq * PACKET_CHANNELS_NUM * PACKET_BYTES)/1000) & 0xFF), \
                                       (uint8_t)((((frq * PACKET_CHANNELS_NUM * PACKET_BYTES)/1000) >> 8) & 0xFF)

#define AUDIO_MIC_SZE(frq)          (uint8_t)(((frq * MIC_CHANNELS_NUM * MIC_BYTES)/1000) & 0xFF), \
                                       (uint8_t)((((frq * MIC_CHANNELS_NUM * MIC_BYTES)/1000) >> 8) & 0xFF)

#define AUDIO_EP_MAX_SZE(frq,chn,bytes) (uint8_t)(((frq * chn * bytes)/1000) & 0xFF), \
                                       (uint8_t)((((frq * chn * bytes)/1000) >> 8) & 0xFF)

#else
#define AUDIO_PACKET_SZE(frq)          (uint8_t)(((frq * PACKET_CHANNELS_NUM * HALF_WORD_BYTES)/8000) & 0xFF), \
                                       (uint8_t)((((frq * PACKET_CHANNELS_NUM * HALF_WORD_BYTES)/8000) >> 8) & 0xFF)
#endif

#define W_TOTAL_LENGTH(num)            (uint8_t)(num), (uint8_t)((num) >> 8)
#define SAMPLE_FREQ_NUM(num)           (uint8_t)(num), (uint8_t)((num >> 8))
#define SAMPLE_FREQ(frq)               (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))
#define SAMPLE_FREQ_4B(frq)            (uint8_t)(frq), (uint8_t)((frq >> 8)), \
                                       (uint8_t)((frq >> 16)), (uint8_t)((frq >> 24))

#define SWAP_BUF_TO_U16(buf) 			((buf[0]) | (buf[1]<<8))
#define SWAP_BUF_TO_U32(buf) 			((buf[0]) | (buf[1]<<8) | (buf[2]<<16) | (buf[3]<<24))

#define	LOBYTE(w) ((uint8_t)(w))
#define	HIBYTE(w) ((uint8_t)(((uint16_t)(w) >> 8) & 0xFF))
/* UAC 2.0  end*/

//用于在线调音
#define HID_DATA_FUN_EN	1

void OTG_DeviceModeSel(uint8_t Mode,uint16_t UsbVid,uint16_t UsbPid);
void OTG_DeviceRequestProcess(void);


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__OTG_DEVICE_STANDARD_H__

