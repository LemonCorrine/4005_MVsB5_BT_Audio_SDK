
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

/* "sys_param.h"������������*/

/*****************************************************************
 * �������ܿ���
 *****************************************************************/
//BLE��BT(classic)ģ��궨�忪��
#define BLE_SUPPORT					DISABLE
#ifdef CFG_APP_BT_MODE_EN
#define BT_SUPPORT			        ENABLE
#else
#define BT_SUPPORT			        DISABLE
#endif
/*****************************************************************
 * 
 *****************************************************************/
//����˫�ֻ����ӿ���
//#define BT_MULTI_LINK_SUPPORT

/*****************************************************************
 * Note: �������ֲ�������ֲ��system_config/parameter.ini�ļ��ڽ�������,��������:
 * 1. ��������  BT_NAME/BLE_NAME
 * 2. �������书��  TxPowerLevel/PagePowerLevel
 * 3. Ĭ�ϵ�Ƶƫ����  BtTrim
 * 4. TWS����֮����������ͬ����־  TwsVolSyncEnable
 *****************************************************************/
#define BT_ADDR_SIZE				6
#define BT_LSTO_DFT					8000    //���ӳ�ʱʱ�� (���㹫ʽ: 8000*0.625=5s)
#define BT_PAGE_TIMEOUT				8000	//page timeout(ms)  //8000*0.625=5s  

#define	CFG_PARA_BT_SYNC					//BtPlay �첽ʱ�� ������ͬ��
#define CFG_PARA_HFP_SYNC					//ͨ�� �첽ʱ�� ������ͬ��
	
/*****************************************************************
 * ����Э��궨�忪��
 *****************************************************************/
/*
 * ���º����������޸ģ����������������
 */
#define BT_A2DP_SUPPORT				ENABLE //A2DP��AVRCP����
#if CFG_RES_MIC_SELECT
#define BT_HFP_SUPPORT				ENABLE
#endif
#define BT_SPP_SUPPORT				DISABLE
	
//�ڷ�����ģʽ��,���������Զ��л�������ģʽ
//#define BT_AUTO_ENTER_PLAY_MODE

//�����������ɼ�ֻ����״̬
//#define POWER_ON_BT_ACCESS_MODE_SET

/*****************************************************************
 * �궨�忪�ؾ���
 *****************************************************************/
#if (BT_SUPPORT != ENABLE)
#if (defined(CFG_APP_BT_MODE_EN))
#error Conflict: CFG_APP_BT_MODE_EN and BT_SUPPORT setting error 
#endif
#endif


/*****************************************************************
 * ˫�ֻ���·
 *****************************************************************/
#ifdef BT_MULTI_LINK_SUPPORT
#define BT_LINK_DEV_NUM				2 	//���������ֻ����� (1 or 2)
#define BT_DEVICE_NUMBER			2	//����ACL���Ӹ��� (1 or 2)
#define BT_SCO_NUMBER				2	//����ͨ����·���� (1 or 2),BT_SCO_NUMBER����С��BT_DEVICE_NUMBER

#if (BT_LINK_DEV_NUM == 2)
#define LAST_PLAY_PRIORITY				//�󲥷�����
//#define BT_LINK_2DEV_ACCESS_DIS_CON		//������,��һ���ֻ����Ϻ�,�ڶ����ֻ���Ҫ��������; �رպ�,�ڶ����ֻ���������,�ܻ�����
#endif

#else
#define BT_LINK_DEV_NUM				1 	//���������ֻ����� (1 or 2)
#define BT_DEVICE_NUMBER			1	//����ACL���Ӹ��� (1 or 2)
#define BT_SCO_NUMBER				1	//����ͨ����·���� (1 or 2) ,BT_SCO_NUMBER����С��BT_DEVICE_NUMBER
#endif

/*****************************************************************
 * �궨�忪�ؾ���
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

//Note:����AAC,��Ҫͬ����������������USE_AAC_DECODER(app_config.h)
//Note:Ŀǰ1V2��AAC���벻��ͬʱ��
//#define BT_AUDIO_AAC_ENABLE
#ifdef BT_AUDIO_AAC_ENABLE
//Note:Ŀǰ1V2��AAC���벻��ͬʱ��
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
 * ����ͬ��������Ҫ�����ú꿪��
 */
#define BT_AVRCP_VOLUME_SYNC			DISABLE

/*
 * If it doesn't support Advanced AVRCP, TG side will be ignored
 * ������ͬ�����ܶ��õ�AVRCP TG
 * player application setting and value������ͬ���궨�忪��һ��(eg:EQ/repeat mode/shuffle/scan configuration)
 */
#define BT_AVRCP_PLAYER_SETTING			DISABLE

/*
 * If it doesn't support Advanced AVRCP, song play state will be ignored
 * ��������ʱ��
 */
#define BT_AVRCP_SONG_PLAY_STATE		DISABLE

/*
 * If it doesn't support Advanced AVRCP, song track infor will be ignored
 * ����ID3��Ϣ����
 * ������Ϣ����������ʱ������ȡ,���BT_AVRCP_SONG_PLAY_STATEͬ������
 */
#define BT_AVRCP_SONG_TRACK_INFOR		DISABLE

/*
 * If it doesn't support Advanced AVRCP, song track infor will be ignored
 * ����ID3��Ϣ ��ȡ���ʱ������
 * time unit: ms
 */
#define BT_MEDIA_ID3_INTERVAL			500

/*
 * AVRCP���ӳɹ����Զ����Ÿ���
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

//��ص���ͬ��(������Ҫ�� CFG_FUNC_POWER_MONITOR_EN ����)
//#define BT_HFP_BATTERY_SYNC

/*
 * ͨ���������
 */
//AEC��ز������� (MIC gain, AGC, DAC gain, �������)
#define BT_HFP_AEC_ENABLE
#define BT_REMOTE_AEC_DISABLE			//�ر��ֻ���AEC

//MIC���˷�,ʹ�����²���(�ο�)
#define BT_HFP_MIC_PGA_GAIN				15  //ADC PGA GAIN +18db(0~31, 0:max, 31:min)
#define BT_HFP_MIC_PGA_GAIN_BOOST_SEL	2
#define BT_HFP_MIC_DIGIT_GAIN			4095
#define BT_HFP_INPUT_DIGIT_GAIN			1100

//MIC���˷�,ʹ�����²���(�ο�������)
//#define BT_HFP_MIC_PGA_GAIN				14  //ADC PGA GAIN +2db(0~31, 0:max, 31:min)
//#define BT_HFP_MIC_DIGIT_GAIN				4095
//#define BT_HFP_INPUT_DIGIT_GAIN			4095

#define BT_HFP_AEC_ECHO_LEVEL			4 //Echo suppression level: 0(min)~5(max)
#define BT_HFP_AEC_NOISE_LEVEL			2 //Noise suppression level: 0(min)~5(max)

#define BT_HFP_AEC_MAX_DELAY_BLK		32
#define BT_HFP_AEC_DELAY_BLK			4 //MIC���˷Ųο�ֵ
//#define BT_HFP_AEC_DELAY_BLK			14 //MIC���˷Ųο�ֵ(�ο�������)

//����ͨ��ʱ������ѡ��
#define BT_HFP_CALL_DURATION_DISP


#endif /* BT_HFP_SUPPORT == ENABLE */

#endif /*__BT_DEVICE_CFG_H__*/

