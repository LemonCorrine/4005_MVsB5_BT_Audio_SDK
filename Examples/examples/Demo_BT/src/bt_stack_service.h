/**
 **************************************************************************************
 * @file    bt_stack_service.h
 * @brief   
 *
 * @author  kk
 * @version V1.0.0
 *
 * $Created: 2016-6-29 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __BT_STACK_SERVICE_H__
#define __BT_STACK_SERVICE_H__

#include "type.h"

typedef unsigned char (*BBSniffNotifyCallback)(void);

typedef struct _BtBbParams
{
	uint8_t		*localDevName;
	uint8_t		localDevAddr[6];
	uint8_t		freqTrim;
	uint32_t	em_start_addr;

	//agc config
	uint8_t		pAgcDisable;
	uint8_t		pAgcLevel;

	//sniff config
	uint8_t		pSniffNego;
	uint16_t	pSniffDelay;
	uint16_t	pSniffInterval;
	uint16_t	pSniffAttempt;
	uint16_t	pSniffTimeout;
	
	BBSniffNotifyCallback	bbSniffNotify;

}BtBbParams;


/**
 * @brief	Start bluetooth stack service.
 * @param	NONE
 * @return  
 */
void BtStackServiceStart(void);
/**
 * @brief	Kill bluetooth stack service.
 * @param	NONE
 * @return  
 */
bool BtStackServiceKill(void);
void BtBbStart(void);
void BT_IntDisable(void);
void BT_ModuleClose(void);
void BtStackServiceWaitResume(void);
void BtStackServiceWaitClear(void);

/***********************************************************************************
 * ��������DUTģʽ
 * �˳�DUT��,���ϵͳ����
 **********************************************************************************/
void BtEnterDutModeFunc(void);
/***********************************************************************************
 * ���ٿ�������
 * ����֮ǰ���ӹ��������豸
 * ����Э��ջ�ں�̨��������,δ�ر�
 **********************************************************************************/
void BtFastPowerOn(void);
/***********************************************************************************
 * ���ٹر�����
 * �Ͽ��������ӣ��������벻�ɱ����������ɱ�����״̬
 * δ�ر�����Э��ջ
 **********************************************************************************/
void BtFastPowerOff(void);



void BtPowerOff(void);
void BtPowerOn(void);



#endif //__BT_STACK_SERVICE_H__

