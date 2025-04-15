#include "type.h"
#include "sys_param.h"
#include "flash_param.h"

const FLASH_PARAMETER SysDefaultParm =
{
	.SysVer = {SYS_PARA_VER_ID,sizeof(SysDefaultParm.SysVer.name),
			"Ver 1.0.0"},  //�汾

	.BtName = {BT_PARA_BT_NAME_ID,sizeof(SysDefaultParm.BtName.name),
			BT_NAME},	//��������
	.BleName = {BT_PARA_BLE_NAME_ID,sizeof(SysDefaultParm.BleName.name),
			BLE_NAME},	//BLE��������
	.bt_TxPowerLevel = {BT_PARA_RF_TX_LEVEL_ID,sizeof(SysDefaultParm.bt_TxPowerLevel.para_val),
			BT_TX_POWER_LEVEL},//������������ʱ���书��
	.bt_PagePowerLevel = {BT_PARA_RF_PAGE_LEVEL_ID,sizeof(SysDefaultParm.bt_PagePowerLevel.para_val),
			BT_PAGE_TX_POWER_LEVEL},//�����������书��

	.BtTrim = {BT_PARA_TRIM_VAL_ID,sizeof(SysDefaultParm.BtTrim.para_val),
			BT_TRIM},//trimֵ
	.bt_CallinRingType = {BT_PARA_CallinRingType_ID,sizeof(SysDefaultParm.bt_CallinRingType.para_val),
			USE_LOCAL_AND_PHONE_RING},//2 -> ʹ���ֻ���������û���򲥱�������
	.bt_BackgroundType = {BT_PARA_BackgroundType_ID,sizeof(SysDefaultParm.bt_BackgroundType.para_val),
			BT_BACKGROUND_FAST_POWER_ON_OFF},//0 -> BT��̨���������ֻ�

	.bt_SimplePairingEnable = {BT_PARA_SimplePairingEnable_ID,sizeof(SysDefaultParm.bt_SimplePairingEnable.para_val),
			1},// SIMPLEPAIRING ����
	.bt_PinCode = {BT_PARA_PinCode_ID,sizeof(SysDefaultParm.bt_PinCode.code),
			"0000"},// Pin code����

	.bt_Reconnection =  {BT_PARA_ReconnectionEnable_ID,sizeof(SysDefaultParm.bt_Reconnection.para),
			{1,5,3,1,90,5}},//����1 BT�Զ�����(���������л�ģʽ)  --- 1 ����
							//����2  �Զ��������Դ���  --- 5��
							//����3  �Զ�����ÿ���μ��ʱ��(in seconds) --- ���3S
							//����4 BB Lost֮���Զ����� 1-> ��/0->�ر�  --- 1 ��
							//����5 BB Lost ������������ --- 90��
							//����6 BB Lost ����ÿ���μ��ʱ��(in seconds) --- ���5S

	.TwsVolSyncEnable =  {TWS_PARA_TWS_VOL_SYNC_ID,sizeof(SysDefaultParm.TwsVolSyncEnable.para_val),
			1},//����tws ����֮����������ͬ��

	.bt_TwsReconnection =  {TWS_PARA_ReconnectionEnable_ID,sizeof(SysDefaultParm.bt_TwsReconnection.para),
			{1,3,3,1,3,5}},	//����1 tws�Զ�����(���������л�ģʽ) --- 1 ����
							//����2 tws�Զ��������Դ��� --- 3��
							//����3 tws�Զ�����ÿ���μ��ʱ��(in seconds) --- ���3S
							//����4 twsBB Lost֮���Զ����� 1-> ��/0->�ر� --- 1 ��
							//����5 twsBB Lost ������������ --- 3��
							//����6 twsBB Lost ����ÿ���μ��ʱ��(in seconds) --- ���5S

	.bt_TwsPairingWhenPhoneConnectedSupport =  {TWS_PARA_TwsPairingWhenPhoneConnectedSupport_ID,sizeof(SysDefaultParm.bt_TwsPairingWhenPhoneConnectedSupport.para_val),
			0}, //0 -> �ֻ�����ʱ���������޷��������
	.bt_TwsConnectedWhenActiveDisconSupport =  {TWS_PARA_TwsConnectedWhenActiveDisconSupport_ID,sizeof(SysDefaultParm.bt_TwsConnectedWhenActiveDisconSupport.para_val),
			0}, //0 -> �û������Ͽ�TWS��Ժ��´ο������ٻ���
};


