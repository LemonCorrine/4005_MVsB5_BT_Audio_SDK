#ifndef __FLASH_BOOT_H__
#define __FLASH_BOOT_H__

#include "app_config.h"
#include "flash_table.h"
#include "nvm.h"
#define FLASH_BOOT_EN      				1//1
//需要和debug.h中定义的GPIO一一对应
typedef enum __UART_TX
{
	BOOT_DEBUG_TX_A0 = 0,
	BOOT_DEBUG_TX_A1,
	BOOT_DEBUG_TX_A6,
	BOOT_DEBUG_TX_A10,
	BOOT_DEBUG_TX_A18,
	BOOT_DEBUG_TX_A19,
	BOOT_DEBUG_TX_A20,
	BOOT_DEBUG_TX_A21,
	BOOT_DEBUG_TX_A22,
	BOOT_DEBUG_TX_A23,
	BOOT_DEBUG_TX_A24,
	BOOT_DEBUG_TX_A28,
	BOOT_DEBUG_TX_A29,
	BOOT_DEBUG_TX_A30,
	BOOT_DEBUG_TX_A31,
	BOOT_DEBUG_TX_B5
}UART_TX;

//需要和debug.h中定义的波特率一一对应
typedef enum __UART_BAUDRATE
{
	BOOT_DEBUG_BAUDRATE_9600 = 0,
	BOOT_DEBUG_BAUDRATE_115200,
	BOOT_DEBUG_BAUDRATE_256000,
	BOOT_DEBUG_BAUDRATE_1000000,
	BOOT_DEBUG_BAUDRATE_2000000,
	BOOT_DEBUG_BAUDRATE_3000000,
}UART_BAUDRATE;


//波特率配置
#if CFG_FLASHBOOT_DEBUG_EN

	#define  DEBUG_CONNECT(x, y)			x ## y
	#define  DEBUG_STRING_CONNECT(x, y)		DEBUG_CONNECT(x, y)

	#define CFG_BOOT_UART_BANDRATE   		DEBUG_STRING_CONNECT(BOOT_,CFG_UART_BANDRATE)
	#define CFG_BOOT_UART_TX_PORT    		DEBUG_STRING_CONNECT(BOOT_,CFG_UART_TX_PORT)
	#define BOOT_UART_CONFIG				((CFG_FLASHBOOT_DEBUG_EN<<7) + (CFG_BOOT_UART_BANDRATE<<4) + CFG_BOOT_UART_TX_PORT)

#else
	#define BOOT_UART_CONFIG				0
#endif


/*  JUDGEMENT_STANDARD说明
 * 分高4bit与低4bit：
 *   高4bit：
 *      为F则code按版本号升级
 *      为5则按code的CRC进行升级
 *   低4bit:
 *     为F则在升级code需要用到多大空间即擦除多大空间
 *     为5时则标识升级code前全部擦除芯片数据，即擦除“全片”（即除开flash的0地址开始flashboot占用空间不擦除以及最后8K不擦除）
 * 例如：0x5F 则为比较CODE CRC判断是否需要升级；升级时仅部分擦除，不进行全擦除
 */
#define JUDGEMENT_STANDARD		0x55

#if CFG_RES_CARD_GPIO == SDIO_A15_A16_A17
#define SD_PORT				CHN_MASK_SDCARD
#else
#define SD_PORT				CHN_MASK_SDCARD1
#endif

#ifdef CFG_FUNC_BT_OTA_EN
#define UP_PORT				(CHN_MASK_USBCDC + CHN_MASK_UDISK + SD_PORT + CHN_MASK_BLE)
#else
#define UP_PORT				(CHN_MASK_USBCDC + CHN_MASK_UDISK + SD_PORT)//根据应用情况决定打开那些升级接口
#endif

#if FLASH_BOOT_EN
extern const unsigned char flash_data[];
#endif

#endif

