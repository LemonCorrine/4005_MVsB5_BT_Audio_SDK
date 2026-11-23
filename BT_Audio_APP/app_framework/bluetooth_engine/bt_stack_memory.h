/**
 **************************************************************************************
 * @file    bt_stack_memory.h
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

#ifndef __BT_STACK_MEMORY_H__
#define __BT_STACK_MEMORY_H__

#include "type.h"
#include "bt_config.h"


/*****************************************************************
 * ram config
 *****************************************************************/
typedef struct _BT_STACK_MEM_ALLOC_
{
	uint8_t	BtBaseMem[4596+100]; //common
#ifdef BT_MULTI_LINK_SUPPORT
	uint8_t BtBaseMemMulti[2776];
#elif (BT_SRC_LINK_DEV_NUM>=2)
	uint8_t BtBaseMemMulti[2776];
#else
	uint8_t	BtBaseMemMulti[3600];
#endif

#if (BT_A2DP_SUPPORT)		//a2dp+avrcp
	uint8_t	BtA2dpMem[3824];
	uint8_t	BtAvrcpMem[5660];
#ifdef BT_MULTI_LINK_SUPPORT
	uint8_t BtA2dpMemMulti[1174];
	uint8_t BtAvrcpMemMulti[5120];
#elif (BT_SRC_LINK_DEV_NUM>=2)
	uint8_t BtA2dpMemMulti[1174];
	uint8_t BtAvrcpMemMulti[5120];
#endif

#endif

#ifdef BT_AUDIO_AAC_ENABLE
	uint8_t	BtAacMem[550];
#endif

#if ((BT_AVRCP_VOLUME_SYNC) || (BT_AVRCP_PLAYER_SETTING) ||(BT_SOURCE_SUPPORT))
	uint8_t	BtAvrcpTGMem[3072];
#ifdef BT_MULTI_LINK_SUPPORT
	uint8_t BtAvrcpTGMemMulti[3072];
#elif (BT_SRC_LINK_DEV_NUM>=2)
	uint8_t BtAvrcpTGMemMulti[3072];
#endif

#endif

#if (BT_HFP_SUPPORT)
	uint8_t	BtHfpMem[3128];
#ifdef BT_MULTI_LINK_SUPPORT
	uint8_t BtHfpMemMulti[2712];
#elif (BT_SRC_LINK_DEV_NUM>=2)
	uint8_t BtHfpMemMulti[2712];
#endif

#endif

#if (BT_SPP_SUPPORT)
	uint8_t	BtSppMem[668];
#endif

#if BT_SOURCE_SUPPORT
	uint8_t	BtSourceMem[120];
#endif

#if (BT_HFG_SUPPORT)
	uint8_t	BtHfgMem[3532];
#endif

#if (BT_OBEX_SUPPORT)
	uint8_t	BtObexMem[416];
#endif

#if (BT_PBAP_SUPPORT)
	uint8_t	BtPbapMem[848];
#endif

#if (BT_MFI_SUPPORT)
	uint8_t	BtMfiMem[656];
#endif

#if (BT_HID_SUPPORT)
	uint8_t	BtHidMem[980];
#endif

}BT_STACK_MEM_ALLOC;


#define BT_STACK_MEM_SIZE		sizeof(BT_STACK_MEM_ALLOC)

#endif //__BT_STACK_MEMORY_H__

