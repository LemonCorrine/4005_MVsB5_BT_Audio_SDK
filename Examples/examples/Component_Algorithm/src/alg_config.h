/*algorithm_config.h*/
#include <stdint.h>

#ifndef  _ALGORITHM_CONFIG_H_
#define  _ALGORITHM_CONFIG_H_

#define CHIP_TYPE 				0xB5

#define ALG_AREA 				1  	//2bits  4����λ �㷨�ռ��ַ��ΧN��[0kB/64kB/128kB,/192kB]
#define ALG_ICODE_SIZE  		1 	//6bits 64����λ  �㷨�ռ���Icode ��ΧM��[0kB/4kB/8kB/,,/n*4kB/252kB]

#define ALG_FLASH_BOOT_ADDR  	(ALG_AREA*0x10000)
#define ALG_HIGH_ADDR 			(ALG_FLASH_BOOT_ADDR>>(16+1))&0xFF
#define ALG_MID_ADDR  			(ALG_FLASH_BOOT_ADDR>>(8+1))&0xFF
#define ALG_LOW_ADDR  			(ALG_FLASH_BOOT_ADDR>>(0+1))&0xFF

/******************��������������**************************/
#define ALG_AREA_SET 			((ALG_AREA<<6) + ALG_ICODE_SIZE)
#define ALG_ENCYPTION_FLAG 		0x00  //0x00:������    0x55:����

#define ALG_MAJOR_VERSION  		0
#define ALG_MINOR_VERSION  		0
#define ALG_PATCH_VERSION  		1

#endif
