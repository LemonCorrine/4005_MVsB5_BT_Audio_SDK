/**
 **************************************************************************************
 * @file    bt_hf_api.h
 * @brief	蓝牙通话模式
 *
 * @author  kk
 * @version V1.0.0
 *
 * $Created: 2018-7-17 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __BT_HF_API_H__
#define __BT_HF_API_H__

#include "type.h"

//通话采样率微调水位监控参数
#define BT_HF_LEVEL_LOW			256*2*4
#define BT_HF_LEVEL_HIGH		256*2*12

//编码处理
void BtHf_EncodeProcess(void);

//处理发送的SCO数据(CVSD or MSBC)
void BtHf_SendData(void);

//处理接发的SCO数据(CVSD or MSBC)
int16_t BtHf_SaveScoData(uint8_t* data, uint16_t len);

//MSBC编码
void BtHf_MsbcEncoderInit(void);
void BtHf_MsbcEncoderDeinit(void);

//MSBC解码
void BtHf_MsbcMemoryReset(void);
int32_t BtHf_MsbcDecoderInit(void);
int32_t BtHf_MsbcDecoderDeinit(void);
void BtHf_MsbcDecoderStartedSet(bool flag);
bool BtHf_MsbcDecoderStartedGet(void);
uint32_t BtHf_MsbcDataLenGet(void);

//设置蓝牙通话音量
void SetBtHfSyncVolume(uint8_t gain);

//缓存数据处理 (AudioCore sink输出)
uint16_t BtHf_SinkScoDataSet(void* InBuf, uint16_t InLen);
void BtHf_SinkScoDataGet(void* OutBuf, uint16_t OutLen);
uint16_t BtHf_SinkScoDataSpaceLenGet(void);
uint16_t BtHf_SinkScoDataLenGet(void);

//接收到的CVSD缓存数据
uint16_t BtHf_ScoDataGet(void *OutBuf,uint16_t OutLen);
void BtHf_ScoDataSet(void *InBuf,uint16_t InLen);
uint16_t BtHf_ScoDataLenGet(void);
uint16_t BtHf_ScoDataSpaceLenGet(void);


#endif /*__BT_HF_API_H__*/



