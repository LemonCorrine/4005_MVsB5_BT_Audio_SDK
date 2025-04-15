#include "type.h"
#include "sys_param.h"
#include "flash_param.h"

const FLASH_PARAMETER SysDefaultParm =
{
	.SysVer = {SYS_PARA_VER_ID,sizeof(SysDefaultParm.SysVer.name),
			"Ver 1.0.0"},  //版本

	.BtName = {BT_PARA_BT_NAME_ID,sizeof(SysDefaultParm.BtName.name),
			BT_NAME},	//蓝牙名称
	.BleName = {BT_PARA_BLE_NAME_ID,sizeof(SysDefaultParm.BleName.name),
			BLE_NAME},	//BLE蓝牙名称
	.bt_TxPowerLevel = {BT_PARA_RF_TX_LEVEL_ID,sizeof(SysDefaultParm.bt_TxPowerLevel.para_val),
			BT_TX_POWER_LEVEL},//蓝牙正常工作时发射功率
	.bt_PagePowerLevel = {BT_PARA_RF_PAGE_LEVEL_ID,sizeof(SysDefaultParm.bt_PagePowerLevel.para_val),
			BT_PAGE_TX_POWER_LEVEL},//蓝牙回连发射功率

	.BtTrim = {BT_PARA_TRIM_VAL_ID,sizeof(SysDefaultParm.BtTrim.para_val),
			BT_TRIM},//trim值
	.bt_CallinRingType = {BT_PARA_CallinRingType_ID,sizeof(SysDefaultParm.bt_CallinRingType.para_val),
			USE_LOCAL_AND_PHONE_RING},//2 -> 使用手机铃声，若没有则播本地铃声
	.bt_BackgroundType = {BT_PARA_BackgroundType_ID,sizeof(SysDefaultParm.bt_BackgroundType.para_val),
			BT_BACKGROUND_FAST_POWER_ON_OFF},//0 -> BT后台不能连接手机

	.bt_SimplePairingEnable = {BT_PARA_SimplePairingEnable_ID,sizeof(SysDefaultParm.bt_SimplePairingEnable.para_val),
			1},// SIMPLEPAIRING 开启
	.bt_PinCode = {BT_PARA_PinCode_ID,sizeof(SysDefaultParm.bt_PinCode.code),
			"0000"},// Pin code设置

	.bt_Reconnection =  {BT_PARA_ReconnectionEnable_ID,sizeof(SysDefaultParm.bt_Reconnection.para),
			{1,5,3,1,90,5}},//参数1 BT自动重连(开机或者切换模式)  --- 1 开启
							//参数2  自动重连尝试次数  --- 5次
							//参数3  自动重连每两次间隔时间(in seconds) --- 间隔3S
							//参数4 BB Lost之后自动重连 1-> 打开/0->关闭  --- 1 打开
							//参数5 BB Lost 尝试重连次数 --- 90次
							//参数6 BB Lost 重连每两次间隔时间(in seconds) --- 间隔5S

	.TwsVolSyncEnable =  {TWS_PARA_TWS_VOL_SYNC_ID,sizeof(SysDefaultParm.TwsVolSyncEnable.para_val),
			1},//开启tws 主从之间音量控制同步

	.bt_TwsReconnection =  {TWS_PARA_ReconnectionEnable_ID,sizeof(SysDefaultParm.bt_TwsReconnection.para),
			{1,3,3,1,3,5}},	//参数1 tws自动重连(开机或者切换模式) --- 1 开启
							//参数2 tws自动重连尝试次数 --- 3次
							//参数3 tws自动重连每两次间隔时间(in seconds) --- 间隔3S
							//参数4 twsBB Lost之后自动重连 1-> 打开/0->关闭 --- 1 打开
							//参数5 twsBB Lost 尝试重连次数 --- 3次
							//参数6 twsBB Lost 重连每两次间隔时间(in seconds) --- 间隔5S

	.bt_TwsPairingWhenPhoneConnectedSupport =  {TWS_PARA_TwsPairingWhenPhoneConnectedSupport_ID,sizeof(SysDefaultParm.bt_TwsPairingWhenPhoneConnectedSupport.para_val),
			0}, //0 -> 手机连接时，该音箱无法发起组队
	.bt_TwsConnectedWhenActiveDisconSupport =  {TWS_PARA_TwsConnectedWhenActiveDisconSupport_ID,sizeof(SysDefaultParm.bt_TwsConnectedWhenActiveDisconSupport.para_val),
			0}, //0 -> 用户主动断开TWS配对后，下次开机能再回连
};


