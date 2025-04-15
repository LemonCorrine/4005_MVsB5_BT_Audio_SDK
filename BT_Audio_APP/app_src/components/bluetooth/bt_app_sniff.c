/**
 **************************************************************************************
 * @file    bluetooth_sniff.c
 * @brief   bluetooth sniff相关函数功能接口
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2021-4-18 18:00:00$
 *
 * @Copyright (C) Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <string.h>
#include "type.h"
#include "app_config.h"
#include "bt_config.h"
//driver
#include "chip_info.h"
#include "debug.h"
//middleware
#include "main_task.h"
#include "bt_manager.h"
//application
#include "bt_app_sniff.h"
#include "bt_app_connect.h"


void SendDeepSleepMsg(void)
{
#ifdef BT_SNIFF_ENABLE
	extern void BtDeepSleepForUsr(void);

	BtDeepSleepForUsr();
#endif
}



