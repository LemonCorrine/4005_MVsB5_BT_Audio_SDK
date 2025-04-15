///////////////////////////////////////////////////////////////////////////////
//               Mountain View Silicon Tech. Inc.
//  Copyright 2012, Mountain View Silicon Tech. Inc., Shanghai, China
//                       All rights reserved.
//  Filename: bt_config.h
//  maintainer: keke
///////////////////////////////////////////////////////////////////////////////
#ifndef __BT_DEVICE_CFG_H__
#define __BT_DEVICE_CFG_H__
#include "app_config.h"

/* "sys_param.h"蓝牙基础参数*/
/*****************************************************************
 * 蓝牙频偏参数
 *****************************************************************/
#define BT_DEFAULT_TRIM				0x7 //trim范围:0x00~0x0f
#define BT_MIN_TRIM					0x0
#define BT_MAX_TRIM					0xf

/*****************************************************************
 * 蓝牙功能开关
 *****************************************************************/
//BLE和BT(classic)模块宏定义开关
#define BLE_SUPPORT					DISABLE
#ifdef CFG_APP_BT_MODE_EN
#define BT_SUPPORT			        ENABLE
#else
#define BT_SUPPORT			        DISABLE
#endif
/*****************************************************************
 * 
 *****************************************************************/
//蓝牙双手机连接开关
//#define BT_MULTI_LINK_SUPPORT

#define BT_NAME						"BP15_BT"
#define BLE_NAME					"BP15_BLE"

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
#define SYS_DEFAULT_RING_TYPE		USE_LOCAL_AND_PHONE_RING			//2 -> 使用手机铃声，若没有则播本地铃声

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
#define SYS_BT_BACKGROUND_TYPE		BT_BACKGROUND_FAST_POWER_ON_OFF		//0 -> BT后台不能连接手机

#define BT_SIMPLEPAIRING_FLAG		TRUE //0:use pin code; 1:simple pairing
#define BT_PINCODE					"0000"

#define BT_ADDR_SIZE				6
#define BT_LSTO_DFT					8000    //连接超时时间 (换算公式: 8000*0.625=5s)
#define BT_PAGE_TIMEOUT				8000	//page timeout(ms)  //8000*0.625=5s  

#define	CFG_PARA_BT_SYNC					//BtPlay 异步时钟 采样点同步
#define CFG_PARA_HFP_SYNC					//通话 异步时钟 采样点同步
	
/*****************************************************************
 * 蓝牙协议宏定义开关
 *****************************************************************/
/*
 * 以下宏请勿随意修改，否则会引起编译错误
 */
#define BT_A2DP_SUPPORT				ENABLE //A2DP和AVRCP关联
#if CFG_RES_MIC_SELECT
#define BT_HFP_SUPPORT				ENABLE
#endif
#define BT_SPP_SUPPORT				DISABLE
	
//在非蓝牙模式下,播放音乐自动切换到播放模式
//#define BT_AUTO_ENTER_PLAY_MODE

//开机蓝牙不可见只回连状态
//#define POWER_ON_BT_ACCESS_MODE_SET

/*****************************************************************
 * 宏定义开关警告
 *****************************************************************/
#if (BT_SUPPORT != ENABLE)
#if (defined(CFG_APP_BT_MODE_EN))
#error Conflict: CFG_APP_BT_MODE_EN and BT_SUPPORT setting error 
#endif
#endif


/*****************************************************************
 * 双手机链路
 *****************************************************************/
#ifdef BT_MULTI_LINK_SUPPORT
#define BT_LINK_DEV_NUM				2 	//蓝牙连接手机个数 (1 or 2)
#define BT_DEVICE_NUMBER			2	//蓝牙ACL连接个数 (1 or 2)
#define BT_SCO_NUMBER				2	//蓝牙通话链路个数 (1 or 2),BT_SCO_NUMBER必须小于BT_DEVICE_NUMBER

#if (BT_LINK_DEV_NUM == 2)
#define LAST_PLAY_PRIORITY				//后播放优先
//#define BT_LINK_2DEV_ACCESS_DIS_CON		//开启后,第一个手机连上后,第二个手机需要能搜索到; 关闭后,第二个手机搜索不到,能回连上
#endif

#else
#define BT_LINK_DEV_NUM				1 	//蓝牙连接手机个数 (1 or 2)
#define BT_DEVICE_NUMBER			1	//蓝牙ACL连接个数 (1 or 2)
#define BT_SCO_NUMBER				1	//蓝牙通话链路个数 (1 or 2) ,BT_SCO_NUMBER必须小于BT_DEVICE_NUMBER
#endif

/*****************************************************************
 * 宏定义开关警告
 *****************************************************************/
#ifdef BT_DEVICE_NUMBER
#if ((BT_DEVICE_NUMBER != 2)&&(BT_DEVICE_NUMBER != 1))
#error Conflict: BT_DEVICE_NUMBER setting error 
#endif
#endif

#ifdef BT_SCO_NUMBER
#if ((BT_SCO_NUMBER != 2)&&(BT_SCO_NUMBER != 1)&&(BT_SCO_NUMBER > BT_DEVICE_NUMBER))
#error Conflict: BT_SCO_NUMBER setting error 
#endif
#endif

/*****************************************************************
 * A2DP config
 *****************************************************************/
#if BT_A2DP_SUPPORT == ENABLE

#include "bt_a2dp_api.h"

//Note:开启AAC,需要同步开启解码器类型USE_AAC_DECODER(app_config.h)
//Note:目前1V2跟AAC解码不能同时打开
//#define BT_AUDIO_AAC_ENABLE
#ifdef BT_AUDIO_AAC_ENABLE
//Note:目前1V2跟AAC解码不能同时打开
#ifdef BT_MULTI_LINK_SUPPORT
#error Conflict: BT_AUDIO_AAC_ENABLE and BT_MULTI_LINK_SUPPORT setting error
#endif
#endif

/*****************************************************************
 * AVRCP config
 *****************************************************************/
#include "bt_avrcp_api.h"
/*
 * If it doesn't support Advanced AVRCP, TG side will be ignored
 * 音量同步功能需要开启该宏开关
 */
#define BT_AVRCP_VOLUME_SYNC			DISABLE

/*
 * If it doesn't support Advanced AVRCP, TG side will be ignored
 * 和音量同步功能都用到AVRCP TG
 * player application setting and value和音量同步宏定义开关一致(eg:EQ/repeat mode/shuffle/scan configuration)
 */
#define BT_AVRCP_PLAYER_SETTING			DISABLE

/*
 * If it doesn't support Advanced AVRCP, song play state will be ignored
 * 歌曲播放时间
 */
#define BT_AVRCP_SONG_PLAY_STATE		DISABLE

/*
 * If it doesn't support Advanced AVRCP, song track infor will be ignored
 * 歌曲ID3信息反馈
 * 歌曲信息有依赖播放时间来获取,请和BT_AVRCP_SONG_PLAY_STATE同步开启
 */
#define BT_AVRCP_SONG_TRACK_INFOR		DISABLE

/*
 * If it doesn't support Advanced AVRCP, song track infor will be ignored
 * 歌曲ID3信息 获取间隔时间配置
 * time unit: ms
 */
#define BT_MEDIA_ID3_INTERVAL			500

/*
 * AVRCP连接成功后，自动播放歌曲
 */
#define BT_AUTO_PLAY_MUSIC				DISABLE

#endif /* BT_A2DP_SUPPORT == ENABLE */

/*****************************************************************
 * HFP config
 *****************************************************************/
#if BT_HFP_SUPPORT == ENABLE

#include "bt_hfp_api.h"

//DISABLE: only cvsd
//ENABLE: cvsd + msbc
#define BT_HFP_SUPPORT_WBS				ENABLE

/*
 * If it doesn't support WBS, only PCM format data can be
 * transfered to application.
 */
#define BT_HFP_AUDIO_DATA				HFP_AUDIO_DATA_mSBC

//电池电量同步(开启需要和 CFG_FUNC_POWER_MONITOR_EN 关联)
//#define BT_HFP_BATTERY_SYNC

/*
 * 通话相关配置
 */
//AEC相关参数配置 (MIC gain, AGC, DAC gain, 降噪参数)
#define BT_HFP_AEC_ENABLE
#define BT_REMOTE_AEC_DISABLE			//关闭手机端AEC

//MIC无运放,使用如下参数(参考)
#define BT_HFP_MIC_PGA_GAIN				15  //ADC PGA GAIN +18db(0~31, 0:max, 31:min)
#define BT_HFP_MIC_PGA_GAIN_BOOST_SEL	2
#define BT_HFP_MIC_DIGIT_GAIN			4095
#define BT_HFP_INPUT_DIGIT_GAIN			1100

//MIC有运放,使用如下参数(参考开发板)
//#define BT_HFP_MIC_PGA_GAIN				14  //ADC PGA GAIN +2db(0~31, 0:max, 31:min)
//#define BT_HFP_MIC_DIGIT_GAIN				4095
//#define BT_HFP_INPUT_DIGIT_GAIN			4095

#define BT_HFP_AEC_ECHO_LEVEL			4 //Echo suppression level: 0(min)~5(max)
#define BT_HFP_AEC_NOISE_LEVEL			2 //Noise suppression level: 0(min)~5(max)

#define BT_HFP_AEC_MAX_DELAY_BLK		32
#define BT_HFP_AEC_DELAY_BLK			4 //MIC无运放参考值
//#define BT_HFP_AEC_DELAY_BLK			14 //MIC有运放参考值(参考开发板)

//来电通话时长配置选项
#define BT_HFP_CALL_DURATION_DISP


#endif /* BT_HFP_SUPPORT == ENABLE */

#endif /*__BT_DEVICE_CFG_H__*/

