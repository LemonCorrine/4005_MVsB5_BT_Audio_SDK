#include <string.h>
#include "type.h"
#include "sys_param.h"
#include "spi_flash.h"
#include "flash_table.h"
#include "flash_param.h"
#include "rtos_api.h"
#include "debug.h"
#include "bt_config.h"

SYS_PARAMETER sys_parameter;

static const SYS_PARAMETER default_parameter = 
{
	.bt_LocalDeviceName 	= BT_NAME,
	.ble_LocalDeviceName	= BLE_NAME,
	.bt_TxPowerLevel		= BT_TX_POWER_LEVEL,
	.bt_PagePowerLevel		= BT_PAGE_TX_POWER_LEVEL,
	.BtTrim					= BT_DEFAULT_TRIM,
	.TwsVolSyncEnable		= TRUE,	//����֮����������ͬ��
	.bt_CallinRingType		= SYS_DEFAULT_RING_TYPE,
	.bt_BackgroundType		= SYS_BT_BACKGROUND_TYPE,
	.bt_SimplePairingEnable	= BT_SIMPLEPAIRING_FLAG,
	.bt_PinCode				= BT_PINCODE,
	.bt_ReconnectionEnable			= TRUE,
	.bt_ReconnectionTryCounts		= 5,
	.bt_ReconnectionInternalTime	= 3,
	.bt_BBLostReconnectionEnable	= TRUE,
	.bt_BBLostTryCounts				= 90,
	.bt_BBLostInternalTime			= 5,
	.bt_TwsReconnectionEnable		= TRUE,
	.bt_TwsReconnectionTryCounts	= 3,
	.bt_TwsReconnectionInternalTime	= 3,
	.bt_TwsBBLostReconnectionEnable	= TRUE,
	.bt_TwsBBLostTryCounts			= 3,
	.bt_TwsBBLostInternalTime		= 5,
	.bt_TwsConnectedWhenActiveDisconSupport = FALSE,
	.bt_TwsPairingWhenPhoneConnectedSupport	= TRUE,
};

//flash�в�����ȡ
//��flash�ж�ȡ�����ɹ��������� ������dest_para
//��ȡ���ɹ�������Ĭ�ϲ���default_para��dest_para
static const struct
{
	SYS_PARAMETER_ID 	id;				//����ID
	void * 				dest_para;		//������ַ
	void * 				default_para;	//Ĭ�ϲ�����ַ
	uint8_t				len;			//��������
}FlashParamReadMap[] =
{
	{BT_PARA_BT_NAME_ID,				(void *)sys_parameter.bt_LocalDeviceName,		(void *)default_parameter.bt_LocalDeviceName,		BT_NAME_SIZE},
	{BT_PARA_BLE_NAME_ID,				(void *)sys_parameter.ble_LocalDeviceName,		(void *)default_parameter.ble_LocalDeviceName,		BLE_NAME_SIZE},
	{BT_PARA_RF_TX_LEVEL_ID,			(void *)&sys_parameter.bt_TxPowerLevel,			(void *)&default_parameter.bt_TxPowerLevel,			1},
	{BT_PARA_RF_PAGE_LEVEL_ID,			(void *)&sys_parameter.bt_PagePowerLevel,		(void *)&default_parameter.bt_PagePowerLevel,		1},
	{BT_PARA_TRIM_VAL_ID,				(void *)&sys_parameter.BtTrim,					(void *)&default_parameter.BtTrim,					1},
	{BT_PARA_CallinRingType_ID,			(void *)&sys_parameter.bt_CallinRingType,		(void *)&default_parameter.bt_CallinRingType,		1},
	{BT_PARA_BackgroundType_ID,			(void *)&sys_parameter.bt_BackgroundType,		(void *)&default_parameter.bt_BackgroundType,		1},
	{BT_PARA_SimplePairingEnable_ID,	(void *)&sys_parameter.bt_SimplePairingEnable,	(void *)&default_parameter.bt_SimplePairingEnable,	1},
	{BT_PARA_PinCode_ID,				(void *)sys_parameter.bt_PinCode,				(void *)default_parameter.bt_PinCode,				BT_PIN_CODE_LEN},
	{BT_PARA_ReconnectionEnable_ID,		(void *)&sys_parameter.bt_ReconnectionEnable,	(void *)&default_parameter.bt_ReconnectionEnable,	6},

	{TWS_PARA_TWS_VOL_SYNC_ID,			(void *)&sys_parameter.TwsVolSyncEnable,		(void *)&default_parameter.TwsVolSyncEnable,		1},
	{TWS_PARA_ReconnectionEnable_ID,	(void *)&sys_parameter.bt_TwsReconnectionEnable,(void *)&default_parameter.bt_TwsReconnectionEnable,6},
	{TWS_PARA_TwsPairingWhenPhoneConnectedSupport_ID,(void *)&sys_parameter.bt_TwsPairingWhenPhoneConnectedSupport,(void *)&default_parameter.bt_TwsPairingWhenPhoneConnectedSupport,1},
	{TWS_PARA_TwsConnectedWhenActiveDisconSupport_ID,(void *)&sys_parameter.bt_TwsConnectedWhenActiveDisconSupport,(void *)&default_parameter.bt_TwsConnectedWhenActiveDisconSupport,1},
};

uint8_t flash_parameter_read(SYS_PARAMETER_ID id,uint8_t * buf)
{
	uint32_t addr = get_sys_parameter_addr();
	uint8_t * offset;
	uint8_t  len;
	SYS_PARAMETER_ID read_id;

	//���������4K
	for(offset = (uint8_t *)addr; offset < (uint8_t *)(addr + 4096); offset += (2 + 1 + len))
	{
		read_id = offset[1];
		read_id <<= 8;
		read_id |= offset[0];
		len = offset[2];

		if(read_id == 0xffff && len == 0xff)
			break;
//		{
//			uint8_t i;
//			printf("ID: %x,len = %d\n",read_id,len);
//			for(i=0;i<len;i++)
//				printf("%02x ",offset[3+i]);
//			printf("\n");
//		}
		if(read_id == id)
		{
			memcpy(buf,offset+3,len);
			return len;
		}
	}

	return 0;
}

void sys_parameter_init(void)
{
	uint8_t * buf,len,i;

	memset(&sys_parameter,0,sizeof(sys_parameter));
	memcpy(&sys_parameter,&default_parameter,sizeof(SYS_PARAMETER));

	buf = osPortMalloc(256);
	if(flash_table_is_valid() && buf)
	{
		for(i=0;i<sizeof(FlashParamReadMap)/sizeof(FlashParamReadMap[0]);i++)
		{
			if((len = flash_parameter_read(FlashParamReadMap[i].id,buf)) > 0)
				memcpy(FlashParamReadMap[i].dest_para,buf,FlashParamReadMap[i].len);
			else
				memcpy(FlashParamReadMap[i].dest_para,FlashParamReadMap[i].default_para,FlashParamReadMap[i].len);
		}

		//��ȡ�����汾
		if((len = flash_parameter_read(SYS_PARA_VER_ID,buf)) > 0)
		{
			DBG("flash_parameter_ver: %s\n",buf);
		}

		//���ֵ�Ƿ���Ч
		if(sys_parameter.BtTrim > 0x1f)
			sys_parameter.BtTrim = default_parameter.BtTrim;
		if(sys_parameter.bt_TxPowerLevel > 23)
			sys_parameter.bt_TxPowerLevel = default_parameter.bt_TxPowerLevel;
		if(sys_parameter.bt_PagePowerLevel > 23)
			sys_parameter.bt_PagePowerLevel = default_parameter.bt_PagePowerLevel;
	}

	if(buf)
		osPortFree(buf);
}

#ifdef CFG_FUNC_FLASH_PARAM_ONLINE_TUNING_EN
#include "otg_device_hcd.h"
//���õ������ߵķ���buf
extern uint8_t  hid_tx_buf[];
static bool  	hid_tx_flag;
void Union_Effect_Send(uint8_t *buf, uint32_t len)
{
	if(len > 256)
		len = 256;
	memcpy(hid_tx_buf, buf, len);
	hid_tx_flag = TRUE;
}

typedef struct
{
	uint8_t	flash_param_buf[4*1024];	//4K ����buf
	uint8_t	hid_tx[256];				//hid ����buf
	uint8_t write_flag;
}FLASH_PARAMETER_TUNING_CTRL;

static FLASH_PARAMETER_TUNING_CTRL *FlashParam = NULL;

#define SET_HID_SEND_CMD(tx_buf,cmd)	tx_buf[0] = 0xA5,\
										tx_buf[1] = 0x5A,\
										tx_buf[2] = 0x30,\
										tx_buf[3] = cmd

enum
{
	FLASH_PARAMETER_TUNING_HANDSHAKE 		= 0x10,
	FLASH_PARAMETER_TUNING_HANDSHAKE_ACK 	= 0x11,

	FLASH_PARAMETER_TUNING_READ 			= 0x20,
	FLASH_PARAMETER_TUNING_READ_DATA 		= 0x21,
	FLASH_PARAMETER_TUNING_WRITE 			= 0x30,
	FLASH_PARAMETER_TUNING_WRITE_ACK 		= 0x31,

	FLASH_PARAMETER_TUNING_END 				= 0x40,
};

void FlashParamUsb_HandshakeACK(bool ack)
{
	if(!ack)
	{
		uint8_t buf[8];
		SET_HID_SEND_CMD(buf,FLASH_PARAMETER_TUNING_HANDSHAKE_ACK);
		buf[4] = 'N';
		buf[5] = 'G';
		buf[6] = 0;
		Union_Effect_Send(buf,8);
	}
	else
	{
		SET_HID_SEND_CMD(FlashParam->hid_tx,FLASH_PARAMETER_TUNING_HANDSHAKE_ACK);
		FlashParam->hid_tx[4] = 'O';
		FlashParam->hid_tx[5] = 'K';
		FlashParam->hid_tx[6] = 0;
	}
	hid_tx_flag = TRUE;
}

bool  FlashParamUsb_Tx(void)
{
	if(hid_tx_flag)
	{
		if(!FlashParam)
			OTG_DeviceControlSend(hid_tx_buf,256,6);
		else
			OTG_DeviceControlSend(FlashParam->hid_tx,256,6);
		hid_tx_flag = FALSE;
		return TRUE;
	}
	return FALSE;
}

void FlashParamUsb_Rx(uint8_t *buf,uint16_t buf_len)
{
	switch(buf[0])
	{
	case FLASH_PARAMETER_TUNING_READ:
		if(FlashParam)
		{
			uint32_t offset;
			uint16_t len;
			uint8_t  check,i;

			offset = buf[1];
			offset <<= 8;
			offset |= buf[2];
			offset <<= 8;
			offset |= buf[3];
			offset <<= 8;
			offset |= buf[4];

			len = buf[5];
			len <<= 8;
			len |= buf[6];

			if(offset >= sizeof(FlashParam->flash_param_buf)) //�ж�offset�����߽�
				len = 0;
			else if((offset+len) >= sizeof(FlashParam->flash_param_buf)) //�ж�offset+len�����߽磬�޸�len�ĳ���
				len = sizeof(FlashParam->flash_param_buf) - offset;

			if(len > (sizeof(FlashParam->hid_tx) - (2+1+1+4+2+1))) //�ж�len��������buf����
				len = sizeof(FlashParam->hid_tx) - (2+1+1+4+2+1);  //2B ֡ͷ  + 1B ������  + 1B ���� + 4B offset + 2B len + 1B checksum

			SET_HID_SEND_CMD(FlashParam->hid_tx,FLASH_PARAMETER_TUNING_READ_DATA);
			FlashParam->hid_tx[4] = buf[1];//ԭ�ⲻ�� ����offset
			FlashParam->hid_tx[5] = buf[2];
			FlashParam->hid_tx[6] = buf[3];
			FlashParam->hid_tx[7] = buf[4];

			FlashParam->hid_tx[8] = len >> 8;
			FlashParam->hid_tx[9] = len;

			memset(&FlashParam->hid_tx[10],0,sizeof(FlashParam->hid_tx) - (2+1+1+4+2+1));
			if(len > 0)
				memcpy(&FlashParam->hid_tx[10],&FlashParam->flash_param_buf[offset],len);

			check = FlashParam->hid_tx[3]; //����֡ͷ+������
			for(i=4;i<sizeof(FlashParam->hid_tx)-1;i++)
				check += FlashParam->hid_tx[i];

			FlashParam->hid_tx[sizeof(FlashParam->hid_tx)-1] = check;
			hid_tx_flag = TRUE;
		}
		break;
	case FLASH_PARAMETER_TUNING_WRITE:
		if(FlashParam)
		{
			uint32_t offset;
			uint16_t len;
			uint8_t  check,i;

			check = buf[0];
			for(i=1;i<buf_len-1;i++)
				check += buf[i];

			if(buf[buf_len-1] == check)
			{

			}

			offset = buf[1];
			offset <<= 8;
			offset |= buf[2];
			offset <<= 8;
			offset |= buf[3];
			offset <<= 8;
			offset |= buf[4];

			len = buf[5];
			len <<= 8;
			len |= buf[6];

			if(offset >= sizeof(FlashParam->flash_param_buf)) //�ж�offset�����߽�
				len = 0;
			else if((offset+len) >= sizeof(FlashParam->flash_param_buf)) //�ж�offset+len�����߽磬�޸�len�ĳ���
				len = sizeof(FlashParam->flash_param_buf) - offset;

			if(len > (buf_len - (1+4+2+1))) //�ж�len��������buf_len����
				len = buf_len - (1+4+2+1);	//1B ���� + 4B offset + 2B len + 1B checksum

			memcpy(&FlashParam->flash_param_buf[offset],buf+7,len);

			SET_HID_SEND_CMD(FlashParam->hid_tx,FLASH_PARAMETER_TUNING_WRITE_ACK);
			FlashParam->hid_tx[4] = buf[1]; //ԭ�ⲻ�� ����offset
			FlashParam->hid_tx[5] = buf[2];
			FlashParam->hid_tx[6] = buf[3];
			FlashParam->hid_tx[7] = buf[4];

			FlashParam->hid_tx[8] = len >> 8;
			FlashParam->hid_tx[9] = len;

			memset(&FlashParam->hid_tx[10],0,sizeof(FlashParam->hid_tx) - (2+1+1+4+2+1));

			check = FlashParam->hid_tx[3]; //����֡ͷ+������
			for(i=4;i<sizeof(FlashParam->hid_tx)-1;i++)
				check += FlashParam->hid_tx[i];

			FlashParam->hid_tx[sizeof(FlashParam->hid_tx)-1] = check;
			hid_tx_flag = TRUE;

			FlashParam->write_flag = 1;
		}
		break;
	case FLASH_PARAMETER_TUNING_HANDSHAKE:
		if(memcmp(buf+1,"OK?",3) != 0)
			break;
		if(!FlashParam)
		{
			FlashParam = osPortMalloc(sizeof(FLASH_PARAMETER_TUNING_CTRL));
			if(FlashParam)
			{
				uint32_t addr = get_sys_parameter_addr();
				memset(FlashParam,0,sizeof(FLASH_PARAMETER_TUNING_CTRL));
				if(addr && flash_table_is_valid())
				{
					SpiFlashRead(addr, FlashParam->flash_param_buf,sizeof(FlashParam->flash_param_buf), 10);
				}
				else
				{
					//û���ҵ�flash�������ͷ��ڴ�Ȼ�����λ������NG
					if(FlashParam)
					{
						osPortFree(FlashParam);
						FlashParam = NULL;
					}
					FlashParamUsb_HandshakeACK(0);
					break;
				}
			}
			else
			{
				//�ڴ�����ʧ��Ȼ�����λ������NG
				FlashParamUsb_HandshakeACK(0);
				break;
			}
		}
		FlashParamUsb_HandshakeACK(1);
		break;
	case FLASH_PARAMETER_TUNING_END:
		{
			uint8_t buf[8];

			SET_HID_SEND_CMD(buf,FLASH_PARAMETER_TUNING_END);
			buf[4] = 'N';
			buf[5] = 'G';
			buf[6] = 0;
			if(FlashParam)
			{
				if(FlashParam->write_flag)	//д���� ����
				{
					uint32_t addr = get_sys_parameter_addr();
					if(addr && flash_table_is_valid())
					{
						SpiFlashErase(SECTOR_ERASE, addr /4096 , 1);

						SpiFlashWrite(addr, FlashParam->flash_param_buf,sizeof(FlashParam->flash_param_buf), 1);

						buf[4] = 'O';
						buf[5] = 'K';
					}
				}
				else  //������ ����
				{
					buf[4] = 'O';
					buf[5] = 'K';
				}
				osPortFree(FlashParam);
				FlashParam = NULL;
			}
			Union_Effect_Send(buf,8);
		}
		break;
	}
}


#endif


