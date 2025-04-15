#ifndef _SYS_PARAM_H__
#define _SYS_PARAM_H__

#define BT_NAME						"BP15_BT"
#define BLE_NAME					"BP15_BLE"

//trim范围:0x00~0x1f
#define BT_TRIM						0x14 

/* Rf Tx Power Range 
{   level  dbm
	[23] = 8,
	[22] = 6,
	[21] = 4,
	[20] = 2,
	[19] = 0,
	[18] = -2,
	[17] = -4,
	[16] = -6,
	[15] = -8,
	[14] = -10,
	[13] = -13,
	[12] = -15,
	[11] = -17,
	[10] = -19,
	[9]  = -21,
	[8]  = -23,
	[7]  = -25,
	[6]  = -27,
	[5]  = -29,
	[4]  = -31,
	[3]  = -33,
	[2]  = -35,
	[1]  = -37,
	[0]  = -39,
}
*/
//蓝牙正常工作时发射功率
#define BT_TX_POWER_LEVEL			23//19 //Tony 20221109 for delay
//蓝牙回连发射功率
#define BT_PAGE_TX_POWER_LEVEL		16 

//bt 铃声设置
//0 -> 不支持来电铃声
//1 -> 来电报号和铃声
//2 -> 使用手机铃声，若没有则播本地铃声
//3 -> 强制使用本地铃声
enum
{
	USE_NULL_RING = 0,
	USE_NUMBER_REMIND_RING = 1,
	USE_LOCAL_AND_PHONE_RING = 2,
	USE_ONLY_LOCAL_RING = 3,
};

//BT 后台设置
//0 -> BT后台不能连接手机
//1 -> BT后台可以连接手机
//2 -> 无后台
enum
{
	BT_BACKGROUND_FAST_POWER_ON_OFF = 0,
	BT_BACKGROUND_POWER_ON = 1,
	BT_BACKGROUND_DISABLE = 2,
};

#define BT_NAME_SIZE				40
#define BLE_NAME_SIZE				40

#define BT_PIN_CODE_LEN				8

typedef struct _SYS_PARAMETER_
{
	char	bt_LocalDeviceName[BT_NAME_SIZE];
	char	ble_LocalDeviceName[BLE_NAME_SIZE];
	uint8_t	bt_TxPowerLevel;
	uint8_t	bt_PagePowerLevel;
	uint8_t	BtTrim;
	uint8_t	TwsVolSyncEnable;
	uint8_t	bt_CallinRingType;
	uint8_t	bt_BackgroundType;
	uint8_t	bt_SimplePairingEnable;
	char	bt_PinCode[BT_PIN_CODE_LEN];
	uint8_t	bt_ReconnectionEnable;
	uint8_t	bt_ReconnectionTryCounts;
	uint8_t	bt_ReconnectionInternalTime;
	uint8_t	bt_BBLostReconnectionEnable;
	uint8_t	bt_BBLostTryCounts;
	uint8_t	bt_BBLostInternalTime;
	uint8_t	bt_TwsReconnectionEnable;
	uint8_t	bt_TwsReconnectionTryCounts;
	uint8_t	bt_TwsReconnectionInternalTime;
	uint8_t	bt_TwsBBLostReconnectionEnable;
	uint8_t	bt_TwsBBLostTryCounts;
	uint8_t	bt_TwsBBLostInternalTime;
	uint8_t	bt_TwsPairingWhenPhoneConnectedSupport;
	uint8_t	bt_TwsConnectedWhenActiveDisconSupport;
}SYS_PARAMETER;

extern SYS_PARAMETER sys_parameter;
extern void sys_parameter_init(void);

#endif


