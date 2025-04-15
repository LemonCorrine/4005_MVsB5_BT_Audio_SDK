#ifndef __FLASH_BOOT_H__
#define __FLASH_BOOT_H__

#include "app_config.h"
#include "flash_table.h"
#include "nvm.h"
#define FLASH_BOOT_EN      				1//1
//��Ҫ��debug.h�ж����GPIOһһ��Ӧ
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

//��Ҫ��debug.h�ж���Ĳ�����һһ��Ӧ
typedef enum __UART_BAUDRATE
{
	BOOT_DEBUG_BAUDRATE_9600 = 0,
	BOOT_DEBUG_BAUDRATE_115200,
	BOOT_DEBUG_BAUDRATE_256000,
	BOOT_DEBUG_BAUDRATE_1000000,
	BOOT_DEBUG_BAUDRATE_2000000,
	BOOT_DEBUG_BAUDRATE_3000000,
}UART_BAUDRATE;


//����������
#if CFG_FLASHBOOT_DEBUG_EN

	#define  DEBUG_CONNECT(x, y)			x ## y
	#define  DEBUG_STRING_CONNECT(x, y)		DEBUG_CONNECT(x, y)

	#define CFG_BOOT_UART_BANDRATE   		DEBUG_STRING_CONNECT(BOOT_,CFG_UART_BANDRATE)
	#define CFG_BOOT_UART_TX_PORT    		DEBUG_STRING_CONNECT(BOOT_,CFG_UART_TX_PORT)
	#define BOOT_UART_CONFIG				((CFG_FLASHBOOT_DEBUG_EN<<7) + (CFG_BOOT_UART_BANDRATE<<4) + CFG_BOOT_UART_TX_PORT)

#else
	#define BOOT_UART_CONFIG				0
#endif


/*  JUDGEMENT_STANDARD˵��
 * �ָ�4bit���4bit��
 *   ��4bit��
 *      ΪF��code���汾������
 *      Ϊ5��code��CRC��������
 *   ��4bit:
 *     ΪF��������code��Ҫ�õ����ռ伴�������ռ�
 *     Ϊ5ʱ���ʶ����codeǰȫ������оƬ���ݣ���������ȫƬ����������flash��0��ַ��ʼflashbootռ�ÿռ䲻�����Լ����8K��������
 * ���磺0x5F ��Ϊ�Ƚ�CODE CRC�ж��Ƿ���Ҫ����������ʱ�����ֲ�����������ȫ����
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
#define UP_PORT				(CHN_MASK_USBCDC + CHN_MASK_UDISK + SD_PORT)//����Ӧ�������������Щ�����ӿ�
#endif

#if FLASH_BOOT_EN
extern const unsigned char flash_data[];
#endif

#endif

