/**
 **************************************************************************************
 * @file    app_message.h
 * @brief   
 *
 * @author  halley
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */


#ifndef __APP_MESSAGE_H__
#define __APP_MESSAGE_H__

#include "type.h"
// Task:容器state、MSG; Operation:功能性State、MSG
/*************************************************************************************
 *
 * Message defines
 *
 */

typedef enum 
{
/** generally */
	MSG_NONE,
/** Require down to task(mode app or service) */
	MSG_TASK_CREATE,
	MSG_TASK_START,
	MSG_TASK_RESUME,		//恢复服务，
	MSG_TASK_PAUSE,			//暂停服务，功能复位
	MSG_TASK_STOP,
	MSG_TASK_POWERDOWN,		//PowerKey软开关应用下发送消息在task应用中处理
	MSG_TASK_DEEPSLEEP,		//供maintask转译
	MSG_TASK_BTSNIFF,
	MSG_POWERCHAGE_POWERDOWN,		//当使用Powerdown提示音使用
	MSG_APP_CREATED,
	MSG_APP_STARTED,
	MSG_APP_STOPPED,//用于消息归一化
	MSG_APP_EXIT,
	MSG_APP_RES_RELEASE,
	MSG_APP_RES_RELEASE_SUC,//释放成功
	MSG_APP_RES_MALLOC,
	MSG_APP_RES_MALLOC_SUC,//申请成功
	MSG_APP_RES_INIT,
	MSG_APP_RES_INIT_SUC,//初始化成功
	
/** Main App*/
	MSG_MAINAPP_GROUP					= 0x0100,
/** Operation Msg */	
	MSG_MAINAPP_NEXT_MODE,

/*****************app msg***********/

/** Media Play Mode -Create by mainapp */
	MSG_WAITING_PLAY_MODE_GROUP			= 0x0200,
/** Task msg */
	MSG_WAITING_PLAY_MODE_CREATED,
	MSG_WAITING_PLAY_MODE_STARTED,
	MSG_WAITING_PLAY_MODE_PAUSED,
	MSG_WAITING_PLAY_MODE_STOPPED,
/** Operation Msg */
	MSG_WAITING_PERMISSION_MODE,//允许首个模式启动。


/** Media Play Mode -Create by mainapp */
	MSG_MEDIA_PLAY_MODE_GROUP			= 0x0300,
/** Task msg */
	MSG_MEDIA_PLAY_MODE_CREATED,
	MSG_MEDIA_PLAY_MODE_STARTED,
	MSG_MEDIA_PLAY_MODE_PAUSED,
	MSG_MEDIA_PLAY_MODE_STOPPED,
	MSG_MEDIA_PLAY_BROWER_UP,
	MSG_MEDIA_PLAY_BROWER_DN,
	MSG_MEDIA_PLAY_BROWER_ENTER,
	MSG_MEDIA_PLAY_BROWER_RETURN,
	MSG_MEDIA_PLAY_MODE_FAILURE,

/** BT PLAY Mode -Create by mainapp */
	MSG_BT_PLAY_MODE_GROUP				= 0x0400,
/** Task msg */
	MSG_BT_PLAY_MODE_CREATED,
	MSG_BT_PLAY_MODE_STARTED,
	MSG_BT_PLAY_MODE_PAUSED,
	MSG_BT_PLAY_MODE_STOPPED,
/** Operation Msg */
	MSG_BT_PLAY_VOLUME_SET,

/*** from bt callback ***/
	MSG_BT_PLAY_DECODER_START,
	MSG_BT_PLAY_STATE_CHANGED,			//A2DP播放状态改变
	MSG_BT_PLAY_SYNC_VOLUME_CHANGED,	//音量同步-音量改变
	MSG_BT_PLAY_STREAM_PASUE,			//A2DP数据流暂停

/*** from bt ai msg ***/
	MSG_BT_STATE_CONNECTED,  			//连接状态更新,main_task中用于提示音更新
	MSG_BT_STATE_DISCONNECT, 			//断开状态更新,main_task中用于提示音更新			
	MSG_BT_A2DP_STREAMING,   			//开始播放音乐,如模式切换需求,按照该事件来进行处理
	
/*** from bt source ***/	
	MSG_BT_SOURCE_INQUIRY,				//
	MSG_BT_SOURCE_CONNECT, 				//
	MSG_BT_SOURCE_DISCONNECT, 			//
	MSG_BT_SOURCE_SINK_SWITCH,			//
	
/** BT HF Mode -Create by mainapp */
	MSG_BT_HF_MODE_GROUP				= 0x0500,
/** Task msg */
	MSG_BT_HF_MODE_CREATED,
	MSG_BT_HF_MODE_STARTED,
	MSG_BT_HF_MODE_PAUSED,
	MSG_BT_HF_MODE_STOPPED,
	MSG_BT_HF_MODE_RUN_CONFIG,			//模式所有参数ready后，开始初始化播放相关的参数
	MSG_BT_HF_MODE_REMIND_PLAY,			//呼入电话，未接通前,播放提示音
	MSG_BT_HF_MODE_CODEC_UPDATE,		//incoming call，codec type更新比初始化晚到，需要更新对应的通路，开启相应的初始化数据结构
/** Operation Msg */
	MSG_BT_HF_TRANS_CHANGED,
	MSG_BT_HF_CALL_REJECT,				//拒接电话
	MSG_BT_HF_REDAIL_LAST_NUM,			//回拨最后来电
	MSG_BT_HF_VOICE_RECOGNITION,		//开启语音助手

/** Line Mode -Create by mainapp */
	MSG_LINE_AUDIO_MODE_GROUP			= 0x0700,
/** Task msg */
	MSG_LINE_AUDIO_MODE_CREATED,
	MSG_LINE_AUDIO_MODE_STARTED,
	MSG_LINE_AUDIO_MODE_PAUSED,
	MSG_LINE_AUDIO_MODE_STOPPED,

	/** Hdmi Mode -Create by mainapp */
		MSG_HDMI_AUDIO_MODE_GROUP			= 0x0800,
	/** Task msg */
		MSG_HDMI_AUDIO_MODE_CREATED,
		MSG_HDMI_AUDIO_MODE_STARTED,
		MSG_HDMI_AUDIO_MODE_PAUSED,
		MSG_HDMI_AUDIO_MODE_STOPPED,
	/** Operation Msg */
		MSG_HDMI_AUDIO_ARC_ONOFF,

/** Radio Mode -Create by mainapp */
	MSG_RADIO_AUDIO_MODE_GROUP			= 0x0A00,
/** Task msg */
	MSG_RADIO_AUDIO_MODE_CREATED,
	MSG_RADIO_AUDIO_MODE_STARTED,
	MSG_RADIO_AUDIO_MODE_PAUSED,
	MSG_RADIO_AUDIO_MODE_STOPPED,
/**  Operation Msg */
	MSG_RADIO_PLAY_SCAN,
	MSG_RADIO_PLAY_SCAN_UP,
	MSG_RADIO_PLAY_SCAN_DN,
	MSG_RADIO_PLAY_PRE,
	MSG_RADIO_PLAY_NEXT,

/** USB Device PLAY Mode -Create by mainapp */
	MSG_USB_DEVICE_MODE_GROUP			= 0x0B00,
/** Task msg */
	MSG_USB_DEVICE_MODE_CREATED,
	MSG_USB_DEVICE_MODE_STOPPED,
	MSG_USB_DEVICE_MODE_STARTED,
/**  Operation Msg */
	MSG_DEVICE_SERVICE_USB_DEVICE_IN,
	MSG_DEVICE_SERVICE_USB_DEVICE_OUT,

	//SlowDeviceEventProcess
	MSG_DEVICE_BREAK_POINT_WRITE,

	MSG_DEVICE_BT_LINKED_INFOR_WRITE,		//更新蓝牙配对记录
	MSG_DEVICE_BT_LINKED_PROFILE_UPDATE,	//更新蓝牙连接的协议
	MSG_DEVICE_BT_LINKED_INFOR_ERASE,		//擦除配对区域的记录
	MSG_DEVICE_BT_NAME_WRITE,				//更新蓝牙名称
	MSG_DEVICE_BT_FREQ_OFFSET_WRITE,

/** Rest Mode -Create by mainapp */
	MSG_REST_PLAY_MODE_GROUP			= 0x0C00,
/** Task msg */
	MSG_REST_PLAY_MODE_CREATED,
	MSG_REST_PLAY_MODE_STARTED,
	MSG_REST_PLAY_MODE_PAUSED,
	MSG_REST_PLAY_MODE_STOPPED,

/** I2SIN Mode -Create by mainapp */
	MSG_I2SIN_AUDIO_MODE_GROUP			= 0x0D00,
/** Task msg */
	MSG_I2SIN_AUDIO_MODE_CREATED,
	MSG_I2SIN_AUDIO_MODE_STARTED,
	MSG_I2SIN_AUDIO_MODE_PAUSED,
	MSG_I2SIN_AUDIO_MODE_STOPPED,
	
/** TWS SLAVE Mode -Create by mainapp */
	MSG_TWS_SLAVE_MODE_GROUP			= 0x0E00,
/** Task msg */
	MSG_TWS_SLAVE_MODE_CREATED,
	MSG_TWS_SLAVE_MODE_STARTED,
	MSG_TWS_SLAVE_MODE_PAUSED,
	MSG_TWS_SLAVE_MODE_STOPPED,
	

/******************service msg***************/
/** Encoder Service -Create by App */
	MSG_ENCODER_SERVICE_GROUP	        = 0x7700,
/** Task msg */
	MSG_ENCODER_SERVICE_CREATED,
	MSG_ENCODER_SERVICE_STARTED,
	MSG_ENCODER_SERVICE_PAUSED,
	MSG_ENCODER_SERVICE_STOPPED,
/** Recorder Service -Create by app */
	MSG_MEDIA_RECORDER_SERVICE_GROUP	= 0x7800,
/** Task msg */
	MSG_MEDIA_RECORDER_SERVICE_CREATED,
	MSG_MEDIA_RECORDER_SERVICE_STARTED,
	MSG_MEDIA_RECORDER_SERVICE_PAUSED,
	MSG_MEDIA_RECORDER_GO_PAUSED,
	MSG_MEDIA_RECORDER_SERVICE_STOPPED,
/** Operation Msg */
	MSG_MEDIA_RECORDER_RUN,
	MSG_MEDIA_RECORDER_STOP,
	MSG_MEDIA_RECORDER_STOPPED,
	MSG_MEDIA_RECORDER_ERROR,
/** Bluetooth Stack service - Create by maintask or app*/
	MSG_BTSTACK_GROUP					= 0x7900,
	MSG_BTSTACK_RX_INT,		//rx data arrival
	MSG_BTSTACK_BB_ERROR,
	MSG_BTSTACK_BB_ERROR_RESTART,
	MSG_BTSTACK_SNIFF_STANDBY,
	MSG_BTSTACK_SNIFF_ENTER,
	MSG_BTSTACK_SNIFF_EXIT,
	MSG_BTSTACK_DEEPSLEEP,
	MSG_BTSTACK_LOCAL_DEVICE_NAME_UPDATE,

	//TWS connect
	MSG_BTSTACK_TWS_CONNECT,		//
	MSG_BTSTACK_TWS_DISCONNECT,		//
	MSG_BT_STACK_TWS_PAIRING_START, //开始组网
	MSG_BT_STACK_TWS_PAIRING_START_RESUME,
	MSG_BT_STACK_TWS_PAIRING_STOP, // 退出组网
	MSG_BT_STACK_TWS_SYNC_POWERDOWN, // TWS同步关机

	//reconnect event
	MSG_BTSTACK_RECONNECT_REMOTE_SUCCESS,
	MSG_BTSTACK_RECONNECT_REMOTE_PROFILE,
	MSG_BTSTACK_RECONNECT_REMOTE_STOP,
	MSG_BTSTACK_RECONNECT_REMOTE_PAGE_TIMEOUT,
	
	MSG_BTSTACK_RECONNECT_TWS_STOP,
	MSG_BTSTACK_RECONNECT_TWS_PAGE_TIMEOUT,

	MSG_BTSTACK_RUN_START,
	MSG_BTSTACK_ACCESS_MODE_SET,

	//蓝牙控制命令(应用层调用协议栈的命令都需要调用)
	MSG_BTSTACK_MSG_BT_CONNECT_DEV_CTRL,
	MSG_BTSTACK_MSG_BT_DISCONNECT_DEV_CTRL,
	MSG_BTSTACK_MSG_BT_CONNECT_TWS_CTRL,
	MSG_BTSTACK_MSG_BT_DISCONNECT_TWS_CTRL,
	MSG_BTSTACK_MSG_BT_CONNECT_A2DP,
	MSG_BTSTACK_MSG_BT_DISCONNECT_A2DP,
	MSG_BTSTACK_MSG_BT_CONNECT_AVRCP,
	MSG_BTSTACK_MSG_BT_DISCONNECT_AVRCP,
	MSG_BTSTACK_MSG_BT_PLAY,
	MSG_BTSTACK_MSG_BT_PAUSE,
	MSG_BTSTACK_MSG_BT_NEXT,
	MSG_BTSTACK_MSG_BT_PREV,
	MSG_BTSTACK_MSG_BT_FF_START,
	MSG_BTSTACK_MSG_BT_FF_END,
	MSG_BTSTACK_MSG_BT_FB_START,
	MSG_BTSTACK_MSG_BT_FB_END,
	MSG_BTSTACK_MSG_BT_VOLUME_SET,
	
	MSG_BTSTACK_MSG_BT_CONNECT_HFP,
	MSG_BTSTACK_MSG_BT_DISCONNECT_HFP,
	MSG_BTSTACK_MSG_BT_REDIAL,
	MSG_BTSTACK_MSG_BT_OPEN_VOICE_RECONGNITION,
	MSG_BTSTACK_MSG_BT_CLOSE_VOICE_RECONGNITION,
	MSG_BTSTACK_MSG_BT_ANSWER_CALL,
	MSG_BTSTACK_MSG_BT_HANGUP,
	MSG_BTSTACK_MSG_BT_HF_AUDIO_TRANSFER,
	MSG_BTSTACK_MSG_BT_HF_CALL_HOLDCUR_ANSWER_CALL, //三方通话: hold当前通话,接听另外电话
	MSG_BTSTACK_MSG_BT_HF_CALL_HANGUP_ANSWER_CALL,  //三方通话: 挂断当前通话,接听另外电话
	MSG_BTSTACK_MSG_BT_HF_CALL_HANGUP_ANOTHER_CALL, //三方通话: 挂断 挂起的通话
	MSG_BTSTACK_MSG_BT_HF_SET_BATTERY,	//设置HF电池电量
	MSG_BTSTACK_MSG_BT_HF_VOLUME_SET,
	MSG_BTSTACK_MSG_BT_HF_GET_CUR_CALLNUMBER,
	MSG_BTSTACK_MSG_BT_HF_DISABLE_NREC,
	MSG_BTSTACK_MSG_BT_HF_SCO_DISCONNECT,
	MSG_BTSTACK_MSG_BT_HF_SCO_CONNECT,

	MSG_BTSTACK_MSG_BT_TWS_PAIRING_CANCEL, //取消蓝牙组网

/** Audio Core Service -Create by maintask */ 
	MSG_AUDIO_CORE_SERVICE_GROUP		= 0x7A00,
/** Task msg */
	MSG_AUDIO_CORE_SERVICE_CREATED,
	MSG_AUDIO_CORE_SERVICE_STARTED,	
	MSG_AUDIO_CORE_SERVICE_PAUSED,
	MSG_AUDIO_CORE_SERVICE_STOPPED,
	MSG_AUDIO_CORE_HOLD,
	MSG_AUDIO_CORE_FRAME_SIZE_CHANGE,
	MSG_AUDIO_CORE_EFFECT_CHANGE,
/** Operation Msg */

/** Remind sound Service -Create by maintask */
	MSG_REMIND_SOUND_SERVICE_GROUP		= 0x7B00,
/** Task msg */
	MSG_REMIND_SOUND_SERVICE_CREATED,
	MSG_REMIND_SOUND_SERVICE_STARTED, 
	MSG_REMIND_SOUND_SERVICE_PAUSED,
	MSG_REMIND_SOUND_SERVICE_STOPPED,
/** Operation Msg */
	MSG_REMIND_SOUND_NEED_DECODER,
	MSG_REMIND_SOUND_ITEM_END,
	MSG_REMIND_SOUND_PLAY_STOPPED,
	MSG_REMIND_SOUND_PLAY_RESET,
	MSG_REMIND_SOUND_PLAY_END,
	MSG_REMIND_SOUND_PLAY_REQUEST,
	MSG_REMIND_SOUND_PLAY_REQUEST_FAIL,
	MSG_REMIND_SOUND_PLAY,
	MSG_REMIND_SOUND_PLAY_RENEW,
	MSG_REMIND_SOUND_PLAY_START,//一条提示音播放开始
	MSG_REMIND_SOUND_PLAY_DONE,//一条提示音播放结束

/** Decoder Service -Create by mode app */
	MSG_DECODER_SERVICE_GROUP			= 0x7C00,
/** Task msg */
	MSG_DECODER_SERVICE_CREATED,
	MSG_DECODER_SERVICE_STARTED, 
	MSG_DECODER_SERVICE_PAUSED,
	MSG_DECODER_SERVICE_RESUMED,
	MSG_DECODER_SERVICE_STOPPED,
	MSG_DECODER_SERVICE_FREED,
	MSG_DECODER_SERVICE_ERR,
	MSG_DECODER_SERVICE_FIFO_EMPTY,
	MSG_DECODER_SERVICE_SONG_PLAY_END,
	MSG_DECODER_SERVICE_UPDATA_PLAY_TIME,
	MSG_DECODER_SERVICE_MUTLI_EMPTY, //设备异常
	MSG_DECODER_SERVICE_DISK_ERROR,
/** Operation Msg */
	MSG_DECODER_PLAY,
	MSG_DECODER_PAUSE,
	MSG_DECODER_RESUME,
	MSG_DECODER_STOP,
	MSG_DECODER_FF,
	MSG_DECODER_FB,
	MSG_DECODER_RESET,

	MSG_DECODER_STOPPED,
	MSG_DECODER_MODE_PLAYING,
	MSG_DECODER_REMIND_PLAYING,

/** Display Service -Create by mainapp */
	MSG_DISPLAY_SERVICE_GROUP			= 0x7E00,
	/** Task msg */
	MSG_DISPLAY_SERVICE_CREATED,
	MSG_DISPLAY_SERVICE_STARTED,
	MSG_DISPLAY_SERVICE_PAUSED,
	MSG_DISPLAY_SERVICE_STOPPED,
	/** Display Msg */
	MSG_DISPLAY_SERVICE_DEV,
	MSG_DISPLAY_SERVICE_BT_UNLINK,
	MSG_DISPLAY_SERVICE_BT_LINKED,
	MSG_DISPLAY_SERVICE_LINE,
	MSG_DISPLAY_SERVICE_RADIO,
	MSG_DISPLAY_SERVICE_MEDIA,
	MSG_DISPLAY_SERVICE_FILE_NUM,
	MSG_DISPLAY_SERVICE_NUMBER,
	MSG_DISPLAY_SERVICE_STATION,
	MSG_DISPLAY_SERVICE_SEARCH_STATION,
	MSG_DISPLAY_SERVICE_VOL,
	MSG_DISPLAY_SERVICE_MUSIC_VOL,
	MSG_DISPLAY_SERVICE_MIC_VOL,
	MSG_DISPLAY_SERVICE_TRE,
	MSG_DISPLAY_SERVICE_BAS,
	MSG_DISPLAY_SERVICE_3D,
	MSG_DISPLAY_SERVICE_VB,
	MSG_DISPLAY_SERVICE_SHUNNING,
	MSG_DISPLAY_SERVICE_VOCAL_CUT,
	MSG_DISPLAY_SERVICE_MUTE,
	MSG_DISPLAY_SERVICE_EQ,
	MSG_DISPLAY_SERVICE_REPEAT,
	MSG_DISPLAY_SERVICE_PLAY,
	MSG_DISPLAY_SERVICE_PAUSE,
	MSG_DISPLAY_SERVICE_RTC_TIME,
	
/** Device Service -Create by mainapp */
	MSG_DEVICE_SERVICE_GROUP			= 0x7F00,
/** Task msg */
	MSG_DEVICE_SERVICE_CREATED,
	MSG_DEVICE_SERVICE_STARTED,
	MSG_DEVICE_SERVICE_PAUSED,
	MSG_DEVICE_SERVICE_STOPPED,
/** Operation Msg */
	MSG_DEVICE_SERVICE_LINE_IN,
	MSG_DEVICE_SERVICE_LINE_OUT,
	MSG_DEVICE_SERVICE_CARD_IN,
	MSG_DEVICE_SERVICE_CARD_OUT,
	MSG_DEVICE_SERVICE_U_DISK_IN,
	MSG_DEVICE_SERVICE_U_DISK_OUT,
	MSG_DEVICE_SERVICE_U_DISK_BACK_IN,
	MSG_DEVICE_SERVICE_U_DISK_BACK_OUT,
	MSG_DEVICE_SERVICE_CARD_BACK_IN,
	MSG_DEVICE_SERVICE_CARD_BACK_OUT,

	MSG_DEVICE_SERVICE_HDMI_IN,
	MSG_DEVICE_SERVICE_HDMI_OUT,

	//BT CALL MODE
	MSG_DEVICE_SERVICE_BTHF_IN,
	MSG_DEVICE_SERVICE_BTHF_OUT,
	MSG_DEVICE_SERVICE_ENTER_BTHF_MODE,
	//BT PLAY MODE
	MSG_DEVICE_SERVICE_BTPLAY_IN,	
	//TWS SLAVE MODE
	MSG_DEVICE_SERVICE_TWS_SLAVE_CONNECTED,
	MSG_DEVICE_SERVICE_TWS_SLAVE_DISCONNECT,
	
	MSG_DEVICE_SERVICE_BATTERY_LOW,
	
	MSG_DEVICE_SERVICE_BP_SYS_INFO,			//掉电记忆系统信息更新
	MSG_DEVICE_SERVICE_BP_PLAYER_INFO,		//掉电记忆媒体播放信息更新
	MSG_DEVICE_SERVICE_BP_PLAYER_INFO_2NVM,	//掉电记忆媒体播放信息到NVM，需要记忆时间到S
	MSG_DEVICE_SERVICE_BP_RADIO_INFO,		//掉电记忆媒体播放信息更新
	MSG_DEVICE_SERVICE_BP_ALL_INFO,			//掉电记忆所有信息同时更新
	
	/** User Com Msg */
	MSG_POWERDOWN,			//PowerKey软开关应用下发送消息在task应用中处理
	MSG_DEEPSLEEP,			//待机模式，断点处接着运行
	MSG_BT_SNIFF,			//tws进入deepsleep
	MSG_SOFT_POWER,         //外部CMOS电路软开关消息
	MSG_RTC_SET,
	MSG_RTC_UP,
	MSG_RTC_DOWN,	
	MSG_RTC_DISP_TIME,
	MSG_RTC_SET_TIME,
	MSG_RTC_SET_ALARM,
	MSG_RTC_ALARMING,
	MSG_RTC_SNOOZE,
	MSG_REMIND,             //提示音开关
	MSG_LANG,               //中英文切换
	MSG_UPDATE,             //MVA升级确认键
	MSG_MODE,
	MSG_SOFT_MODE,
	MSG_POWER,
	MSG_PLAY_PAUSE,
	
	MSG_PLAY_PAUSE_BLUE_STACK,
	MSG_STOP,						
	MSG_FF_START,
    MSG_FB_START,
	MSG_FF_FB_END,
    MSG_PRE,
    MSG_SOFT_PRE,
    MSG_NEXT,
    MSG_SOFT_NEXT,
    MSG_REPEAT,	            //播放模式切换
    MSG_REPEAT_AB,          //AB循环模式
    MSG_FOLDER_MODE,		// 打开、关闭文件夹播放模式
    MSG_FOLDER_NEXT,		// 上一个文件夹
    MSG_FOLDER_PRE,			// 下一个文件夹
    MSG_BROWSE,			    // 文件浏览
    MSG_FOLDER_ERGE,	// 
    MSG_FOLDER_GUSHI,	// 
    MSG_FOLDER_GUOXUE,	//  
    MSG_FOLDER_YINGYU,	// 
    MSG_REC,
	MSG_REC1,
	MSG_REC2,
	MSG_REC1_PLAYBACK,
	MSG_REC2_PLAYBACK,
	MSG_DEL_ALL_REC,
	MSG_STOP_REC,
    MSG_REC_PLAYBACK,
    MSG_REC_FILE_DEL,	
    MSG_REC_MUSIC,          //是否对音乐录音选择
    MSG_MENU,			    //菜单，音量控制等
    MSG_NUM_0,	
	MSG_NUM_1,	
	MSG_NUM_2,	
	MSG_NUM_3,	
	MSG_NUM_4,	
	MSG_NUM_5,	
	MSG_NUM_6,	
	MSG_NUM_7,	
	MSG_NUM_8,	
	MSG_NUM_9,	
	MSG_EFFECT_SYNC,
	MSG_MUSIC_VOL_SYNC,
	MSG_MIC_VOL_SYNC,
	MSG_MUSIC_SILENCT_DETECTOR,
	MSG_MIC_SILENCT_DETECTOR,
	MSG_3D_STATUS,
	MSG_ECHO_PARAM,
    MSG_MAIN_VOL_UP,
    MSG_MAIN_VOL_DW,
	MSG_MUSIC_VOLUP,
	MSG_MUSIC_VOLDOWN,
	MSG_MIC_VOLUP,
	MSG_MIC_VOLDOWN,
	MSG_MIC_EFFECT_UP,
	MSG_MIC_EFFECT_DW,
	MSG_MIC_TREB_UP,
	MSG_MIC_TREB_DW,
	MSG_MIC_BASS_UP,
	MSG_MIC_BASS_DW,
	MSG_MUSIC_TREB_UP,
	MSG_MUSIC_TREB_DW,
	MSG_MUSIC_BASS_UP,
	MSG_MUSIC_BASS_DW,
	MSG_PITCH_UP,            //变调加
 	MSG_PITCH_DN,            //变调减	
	MSG_VOCAL_CUT,	
	MSG_MUTE,
	MSG_EQ,
	MSG_3D,
	MSG_VB,	
	MSG_REMIND1,
 	MSG_EFFECTMODE,
	MSG_EFFECTREINIT,
    MSG_MIC_FIRST,     
	MSG_BT_AI,
	MSG_BT_XM_AI_START,
	MSG_BT_XM_AI_STOP,
	MSG_BT_CONNECT_CTRL,     //手动连接/断开蓝牙
	MSG_BT_CLEAR_PAIRED_LIST, //清除BT所有的配对记录(包含TWS)
	MSG_BT_CONNECT_MODE,
	MSG_BT_OPEN_ACCESS,
	MSG_RGB_MODE,
	MSG_HID_KEY1_DOWN,
	MSG_HID_KEY2_DOWN,
	MSG_HID_KEY3_DOWN,
	MSG_HID_KEY4_DOWN,
	MSG_HID_KEY_UP,
//tws
	MSG_BT_TWS_PAIRING,            //开始进行TWS组网
	MSG_BT_TWS_CLEAR_PAIRED_LIST,  //清除配对记录
	MSG_BT_TWS_RECONNECT,          //开始TWS回连
	MSG_BT_TWS_DISCONNECT,         //断开TWS连接
	MSG_BT_TWS_LINKLOSS,           //TWS连接丢失
	MSG_BT_TWS_SLAVE_MODE,         //进入、退出TWS SLAVE模式
	MSG_BT_TWS_MASTER_CONNECTED,
	MSG_BT_TWS_SLAVE_CONNECTED,
	MSG_BT_TWS_AUDIO_START,		//TWS同步音频起始
	MSG_BT_RST,              //蓝牙reset，恢复出厂设置
	MSG_BT_ENTER_DUT_MODE,   //进入DUT测试模式
	MSG_BT_TWS_OUT_MODE,
	MSG_BT_SOUNDBAR_SLAVE_TEST_MODE,  //Soundbar Slave进入可以校频偏状态

	MSG_ENTER_IDLE_MODE,
	MSG_QUIT_IDLE_MODE,
	MSG_AUTO_TEST_START,
	MSG_TWS_UNMUTE,
	MSG_REMIND_PLAY_END,
	MSG_BT_START_OTA,

	//BT SOURCE
	MSG_BT_SOURCE_HFG_HANG_UP,
	MSG_BT_SOURCE_HFG_OUTGOING,//HFG进入呼出等待状态
	MSG_BT_SOURCE_HFG_INCOMING,//HFG进入呼入等待状态
	MSG_BT_SOURCE_HFG_ANSWER,//HFG接听呼入/呼出
	MSG_BT_SOURCE_HFG_CONNECT,
	MSG_BT_SOURCE_A2DP_CONNECT,
	MSG_BT_SOURCE_A2DP_DISCONNECT,


	//BQB
	MSG_A2DP_CONNECT,
	MSG_AVRCP_CONNECT,
	MSG_BT_BQB_AVDTP_SMG_BI38C,
	MSG_BT_BQB_AVDTP_SMG,
	MSG_HFP_CONNECT,
	MSG_SCO_CONNECT,
	MSG_BT_BQB_VRR_BV_01_C,
	MSG_AVRCP_STOP,

/******************adc level ,Sliding resistance msg, Reservations***************/
    MSG_ADC_LEVEL_MSG_START       = 0x9000,
    MSG_ADC_LEVEL_CH1             = 0x9100,
    MSG_ADC_LEVEL_CH2             = 0x9200,
    MSG_ADC_LEVEL_CH3             = 0x9300,
	MSG_ADC_LEVEL_CH4             = 0x9400,
	MSG_ADC_LEVEL_CH5             = 0x9500,
	MSG_ADC_LEVEL_CH6             = 0x9600,
	MSG_ADC_LEVEL_CH7             = 0x9700,
	MSG_ADC_LEVEL_CH8             = 0x9800,
	MSG_ADC_LEVEL_CH9             = 0x9900,
	MSG_ADC_LEVEL_CH10            = 0x9a00,
	MSG_ADC_LEVEL_CH11            = 0x9b00,
	MSG_ADC_LEVEL_CH12            = 0x9c00,
	MSG_ADC_LEVEL_CH13            = 0x9d00,
	MSG_ADC_LEVEL_CH14            = 0x9e00,
	MSG_ADC_LEVEL_MSG_END         = 0x9f00,   
} MessageId;

#define MAX_RECV_MSG_TIMEOUT		0xFFFFFFFF


/*************************************************************************************
 *
 * Module Message defines
 *
 *************************************************************************************/

typedef enum
{
	TaskStateNone	= 0,
	TaskStateCreating,
	TaskStateReady, // TaskStateCreated
	TaskStateStarting,
	TaskStateRunning, // TaskStateStarted
	TaskStatePausing,
	TaskStatePaused,
	TaskStateResuming,
	TaskStateStopping,
	TaskStateStopped,
	TaskStateError,
}TaskState;

#endif /*__APP_MESSAGE_H__*/

