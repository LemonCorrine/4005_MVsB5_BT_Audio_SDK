/**
 **************************************************************************************
 * @file    flash_example.c
 * @brief   flash example
 *
 * @author  Peter
 * @version V1.0.0
 *
 * $Created: 2019-05-30 19:17:00$
 *
 * @Copyright (C) 2019, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdlib.h>
#include <nds32_intrinsic.h>
#include <string.h>
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "spi_flash.h"
#include "remap.h"
#include "irqn.h"
#include "watchdog.h"
#include "chip_info.h"

SPI_FLASH_INFO  FlashInfo;

uint32_t FlashCapacity = 0;
void GetFlashGD(int32_t protect)
{
	uint8_t  str[20];
	FlashCapacity = 0;

	switch(FlashInfo.Did)
	{
		case 0x1340:
			strcpy((char *)str,"GD25Q40(GD25Q40B)");
			FlashCapacity = 0x00080000;
			break;

		case 0x1240:
        	strcpy((char *)str,"GD25Q20(GD25Q20B)");
			FlashCapacity = 0x00040000;
			break;

		case 0x1540:
			strcpy((char *)str,"GD25Q16(GD25Q16B)");
			FlashCapacity = 0x00200000;
			break;

		case 0x1640:
        	strcpy((char *)str,"GD25Q32(GD25Q32B)");
			FlashCapacity = 0x00400000;
			break;

		case 0x1740:
        	strcpy((char *)str,"GD25Q64B");
			FlashCapacity = 0x00800000;
			break;

		case 0x1440:
        	strcpy((char *)str,"GD25Q80(GD25Q80B)");
			FlashCapacity = 0x00100000;
			break;

		case 0x1840:
            strcpy((char *)str,"GD25Q128B");
            FlashCapacity = 0x01000000;
            break;

		default:
			break;
	}

    if(FlashCapacity > 0)
    {
        DBG("�ͺ�:                         ");
        DBG("%s\r\n",str);
        DBG("����:                         ");
        DBG("0x%08X\r\n", (unsigned int)FlashCapacity);
    }
    else
    {
        DBG("����ʧ�� 1\r\n");
    }

}

void GetFlashWinBound(int32_t protect)
{
    uint8_t  str[20];
	FlashCapacity = 0;

	switch(FlashInfo.Did)
    {
        case 0x1440:
            strcpy((char *)str,"W25Q80BV");
            FlashCapacity = 0x00100000;
            break;

        case 0x1760:
            strcpy((char *)str,"W25Q64DW");
            FlashCapacity = 0x00800000;
            break;

        case 0x1740:
            strcpy((char *)str,"W25Q64CV");
            FlashCapacity = 0x00800000;
            break;
        case 0x1540:
            strcpy((char *)str,"W25Q16JV");
            FlashCapacity = 0x200000;
            break;

        case 0x1340:
            strcpy((char *)str,"W25Q40BV");
            FlashCapacity = 0x00080000;
        break;

        default:
            break;
    }

    if(FlashCapacity > 0)
    {
        DBG("�ͺ�:                         ");
        DBG("%s\r\n",str);
        DBG("����:                         ");
        DBG("0x%08X\r\n",(unsigned int)FlashCapacity);
    }
    else
    {
        DBG("����ʧ�� 2\r\n");
    }
}

void GetFlashPct(void)
{
    uint8_t  str[20];
	FlashCapacity = 0;

	switch(FlashInfo.Did)
    {
        case 0x0126:
            strcpy((char *)str,"PCT26VF016");
            FlashCapacity = 0x00200000;
			break;

        case 0x0226:
            strcpy((char *)str,"PCT26VF032");
            FlashCapacity = 0x00400000;
            break;

        default:
			break;
    }

    if(FlashCapacity > 0)
    {
        DBG("�ͺ�:                         ");
        DBG("%s\r\n",str);
        DBG("����:                         ");
        DBG("0x%08X\r\n",(unsigned int)FlashCapacity);
    }
    else
    {
        DBG("����ʧ�� 3\r\n");
    }

}

void GetFlashEon(int32_t protect)
{
    uint8_t  str[20];
	FlashCapacity = 0;

	switch(FlashInfo.Did)
    {
        case 0x1430:
            strcpy((char *)str,"EN25Q80A");
            FlashCapacity = 0x00100000;
            break;

        case 0x1530:
            strcpy((char *)str,"EN25Q16A");
            FlashCapacity = 0x00200000;
            break;

        case 0x1830:
            strcpy((char *)str,"EN25Q128");
            FlashCapacity = 0x01000000;
            break;

        case 0x1630:
            strcpy((char *)str,"EN25Q32A");
            FlashCapacity = 0x00400000;
            break;

        case 0x1330:
            strcpy((char *)str,"EN25Q40");
            FlashCapacity = 0x00080000;
            break;

        case 0x1730:
            strcpy((char *)str,"EN25Q64");
            FlashCapacity = 0x00800000;
            break;

        case 0x1570:
            strcpy((char *)str,"EN25QH16");
            FlashCapacity = 0x00200000;
            break;

        case 0x1670:
            strcpy((char *)str,"EN25QH32");
            FlashCapacity = 0x00400000;
            break;

        default:
            break;
    }

    if(FlashCapacity > 0)
    {
        DBG("�ͺ�:                         ");
        DBG("%s\r\n",str);
        DBG("����:                         ");
        DBG("0x%08X\r\n",(unsigned int)FlashCapacity);
    }
    else
    {
        DBG("����ʧ�� 4\r\n");
    }

}

void GetFlashBg(int32_t protect)//Paragon FAE say their product is the same with bg
{
    uint8_t  str[20];
	FlashCapacity = 0;

    switch(FlashInfo.Did)
    {
        case 0x1540:
            strcpy((char *)str,"PN25F16");
            FlashCapacity = 0x00200000;
            break;

        case 0x1340:
            strcpy((char *)str,"PN25F04");
            FlashCapacity = 0x00080000;
            break;

        case 0x1440:
            strcpy((char *)str,"PN25F08");
            FlashCapacity = 0x00100000;
            break;

        default:
			break;
    }

    if(FlashCapacity > 0)
    {
        DBG("�ͺ�:                         ");
        DBG("%s\r\n",str);
        DBG("����:                         ");
        DBG("0x%08X\r\n",(unsigned int)FlashCapacity);
    }
    else
    {
        DBG("����ʧ�� 5\r\n");
    }

}

void GetFlashEsmt(int32_t protect)
{
    uint8_t  str[20];
	FlashCapacity = 0;

	switch(FlashInfo.Did)
    {
        case 0x1440:
            strcpy((char *)str,"F25L08QA");
            FlashCapacity = 0x00100000;
            break;

        case 0x1540:
            strcpy((char *)str,"F25L16QA");
            FlashCapacity = 0x00200000;
            break;

        case 0x1641:
            strcpy((char *)str,"F25L32QA");
            FlashCapacity = 0x00400000;
            break;

        case 0x1741:
            strcpy((char *)str,"F25L64QA");
            FlashCapacity = 0x00800000;
            break;

        default:
            break;
    }

    if(FlashCapacity > 0)
    {
        DBG("�ͺ�:                         ");
        DBG("%s\r\n",str);
        DBG("����:                         ");
        DBG("0x%08X\r\n",(unsigned int)FlashCapacity);
    }
    else
    {
        DBG("����ʧ�� 6\r\n");
    }

}

void GetFlashPuya(int32_t protect)
{
    uint8_t  str[20];
	FlashCapacity = 0;

	switch(FlashInfo.Did)
    {
        case 0x1460:
            strcpy((char *)str,"P25Q80HB");
            FlashCapacity = 0x00100000;
            break;
        case 0x1560:
            strcpy((char *)str,"P25Q16SH");
            FlashCapacity = 0x00200000;
            break;
        case 0x1542:
            strcpy((char *)str,"P25Q16H");
            FlashCapacity = 0x00200000;
            break;
        default:
            break;
    }

    if(FlashCapacity > 0)
    {
        DBG("�ͺ�: ");
        DBG("%s\r\n",str);
        DBG("����:");
        DBG("0x%08X\r\n",(unsigned int)FlashCapacity);
    }
    else
    {
        DBG("����ʧ�� 6\r\n");
    }

}

void GetFlashMD(int32_t protect)
{
    uint8_t  str[20];
	FlashCapacity = 0;

	switch(FlashInfo.Did)
    {
        case 0x1340:
            strcpy((char *)str,"MD24D40D");
            FlashCapacity = 0x00080000;
            break;

        default:
            break;
    }

    if(FlashCapacity > 0)
    {
        DBG("�ͺ�: ");
        DBG("%s\r\n",str);
        DBG("����:");
        DBG("0x%08X\r\n",(unsigned int)FlashCapacity);
    }
    else
    {
        DBG("����ʧ�� 6\r\n");
    }
}

void GetFlashTH(int32_t protect)
{
    uint8_t  str[20];
	FlashCapacity = 0;

	switch(FlashInfo.Did)
    {
        case 0x1460:
            strcpy((char *)str,"TH25Q80");
            FlashCapacity = 0x00100000;
            break;
        case 0x1560:
            strcpy((char *)str,"TH25Q16");
            FlashCapacity = 0x00200000;
            break;
        default:
            break;
    }

    if(FlashCapacity > 0)
    {
        DBG("�ͺ�: ");
        DBG("%s\r\n",str);
        DBG("����:");
        DBG("0x%08X\r\n",(unsigned int)FlashCapacity);
    }
    else
    {
        DBG("����ʧ�� 6\r\n");
    }
}

void GetDidInfo(int32_t protect)
{
	DBG("Did:");
	DBG("%08X\r\n",FlashInfo.Did);

	DBG("������Χ(BP4~BP0):");
	DBG("%02X\r\n",(unsigned int)protect);
}

void GetFlashInfo(void)
{
	int32_t protect = 0;

	DBG("\r\n\r\n****************************************************************\n");
    DBG("%-30s\r\n","Flash��Ϣ");

	if(FlashInfo.Mid != FLASH_PCT)
	{
		SpiFlashIOCtrl(IOCTL_FLASH_RDSTATUS, &protect);
		protect = (protect >> 2) & 0x1F;
	}

    switch(FlashInfo.Mid)
    {
        case FLASH_GD:
            DBG("����:GD\r\n");
			GetDidInfo(protect);
            GetFlashGD(protect);
            break;

        case FLASH_WINBOUND:
            DBG("����:WINBOUND\r\n");
			GetDidInfo(protect);
            GetFlashWinBound(protect);
            break;

        case FLASH_PCT:
            DBG("����:PCT\r\n");
            GetFlashPct();
            break;

        case FLASH_EON:
            DBG("����:EN\r\n");
			GetDidInfo(protect);
            GetFlashEon(protect);
            break;

        case FLASH_BG://BG is Paragon
            //DBG("����:                         BG\r\n");
            DBG("����:Paragon\r\n");
			GetDidInfo(protect);
            GetFlashBg(protect);
            break;

        case FLASH_ESMT:
            DBG("����:ESMT\r\n");
			GetDidInfo(protect);
            GetFlashEsmt(protect);
            break;

        case FLASH_PY:
            DBG("����:Puya\r\n");
			GetDidInfo(protect);
			GetFlashPuya(protect);
			break;

        case FLASH_MD:
            DBG("����:MD\r\n");
			GetDidInfo(protect);
			GetFlashMD(protect);
			break;
        case FLASH_TH:
            DBG("����:TH\r\n");
			GetDidInfo(protect);
			GetFlashMD(protect);
			break;
        default:
            DBG("����:not found\r\n");
            break;
    }
	DBG("\r\n");
	DBG("****************************************************************\n");
}

/**
 * @brief	Flash����
 * @param	None
 * @return	�����ɹ�����TRUE�����򷵻�FALSE
 * @note
 */
bool FlashUnlock(void)
{
	char cmd[3] = "\x35\xBA\x69";

	if(SpiFlashIOCtrl(IOCTL_FLASH_UNPROTECT, cmd, sizeof(cmd)) != FLASH_NONE_ERR)
	{
		return FALSE;
	}

	return TRUE;
}

/**
 * @brief	Flash����
 * @param	lock_range,Flash������Χ:
 * 			FLASH_LOCK_RANGE_HALF : ����1/2 Flash �ռ�(��0��ʼ����ͬ)
 * 			FLASH_LOCK_RANGE_THREE_QUARTERS : ����3/4 Flash �ռ�
 * 			FLASH_LOCK_RANGE_SEVENTH_EIGHT : ����7/8 Flash �ռ�
 * 			FLASH_LOCK_RANGE_ALL : ����ȫ��Flash �ռ�
 * @return	�����ɹ�����TRUE�����򷵻�FALSE
 * @note
 */
bool FlashLock(SPI_FLASH_LOCK_RANGE lock_range)
{
	if(SpiFlashIOCtrl(IOCTL_FLASH_PROTECT, lock_range) != FLASH_NONE_ERR)
	{
		return FALSE;
	}

	return TRUE;
}

#define	SEC		40
#define	ADDR	40*4096
#define	LEN  	1896

uint8_t fshc_buf[LEN];
uint8_t memcpy_buf[LEN];
uint8_t erase_buf[LEN];
extern void __c_init_rom();
int main(void)
{
    uint8_t UartPort = 1;
    uint8_t Key;
	void *p;
    Chip_Init(1);
    WDG_Disable();
    __c_init_rom();
    Clock_Config(1, 24000000);
    Clock_HOSCCurrentSet(15);  // �Ӵ��˾����ƫ�õ���
    Clock_PllLock(240 * 1000); // 240MƵ��
    Clock_APllLock(240 * 1000);
    Clock_Module1Enable(ALL_MODULE1_CLK_SWITCH);
    Clock_Module2Enable(ALL_MODULE2_CLK_SWITCH);
    Clock_Module3Enable(ALL_MODULE3_CLK_SWITCH);
    Clock_SysClkSelect(PLL_CLK_MODE);
    Clock_UARTClkSelect(PLL_CLK_MODE);
    Clock_HOSCCurrentSet(5);

    SpiFlashInit(80000000, MODE_4BIT, 0, 1);

    // BP15ϵ�п��������ô��ڣ�Ĭ��ʹ��
    GPIO_PortAModeSet(GPIOA10, 5);// UART1 TX
    GPIO_PortAModeSet(GPIOA9, 1);// UART1 RX
    DbgUartInit(1, 2000000, 8, 0, 1);


	DBG("\n");
    DBG("/-----------------------------------------------------\\\n");
    DBG("|                     Flash Example                     |\n");
    DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
    DBG("\\-----------------------------------------------------/\n");
    DBG("\n");

    memcpy(&FlashInfo, SpiFlashInfoGet(), sizeof(SPI_FLASH_INFO));
    GetFlashInfo();

    p = (void *)0;
    memcpy(memcpy_buf, p, LEN);

    SpiFlashIOCtrl(IOCTL_FLASH_UNPROTECT, "\x35\xBA\x69", 3);

	DBG("0 ---> erase\n");
	DBG("1 ---> read\n");
	DBG("2 ---> write\n");
	DBG("3 ---> protect\n");
	DBG("4 ---> unprotect\n");

	Key = 0;
	while(1)
	{
		if(UARTS_Recv(UartPort, &Key, 1, 100) > 0)
		{
			switch(Key-'0')
			{
				case 0://erase
					memset(erase_buf, 0xff, LEN);
					memset(fshc_buf, 0, LEN);
					FlashErase(ADDR, 4096);
					SpiFlashRead(ADDR, fshc_buf, LEN, 100);
					if(memcmp(fshc_buf, erase_buf, LEN) == 0)
					{
						DBG("erase OK\n");
					}
					else
					{
						DBG("erase error\n");
						while(1);
					}
					break;

				case 1://read
					memset(fshc_buf, 0, LEN);
					SpiFlashRead(0, fshc_buf, LEN, 100);
					 p = (void *)0;
					if(memcmp(fshc_buf, p, LEN) == 0)
					{
						DBG("read OK\n");
					}
					else
					{
						DBG("read error\n");
						while(1);
					}
					break;

				case 2://write
					memset(fshc_buf, 0, LEN);
					SpiFlashErase(SECTOR_ERASE, SEC, 0);
					SpiFlashWrite(ADDR, memcpy_buf, LEN, 100);
					SpiFlashRead(ADDR, fshc_buf, LEN, 100);
					if(memcmp(fshc_buf, memcpy_buf, LEN) == 0)
					{
						DBG("write OK\n");
					}
					else
					{
						DBG("write error\n");
					}
					break;

				case 3://FLASH_PROTECT
					memset(fshc_buf, 0, LEN);
					SpiFlashIOCtrl(IOCTL_FLASH_UNPROTECT, "\x35\xBA\x69", 3);
					SpiFlashErase(SECTOR_ERASE, SEC, 0);
					SpiFlashWrite(ADDR, memcpy_buf, LEN, 100);
					SpiFlashIOCtrl(IOCTL_FLASH_PROTECT, FLASH_LOCK_RANGE_ALL);
					SpiFlashErase(SECTOR_ERASE, SEC, 0);
					SpiFlashRead(ADDR, fshc_buf, LEN, 100);
					if(memcmp(fshc_buf, memcpy_buf, LEN) == 0)
					{
						DBG("FLASH_PROTECT OK\n");
					}
					else
					{
						DBG("FLASH_PROTECT error\n");
						while(1);
					}
					SpiFlashIOCtrl(IOCTL_FLASH_UNPROTECT, "\x35\xBA\x69", 3);
					break;

				case 4://FLASH_UNPROTECT
					memset(fshc_buf, 0, LEN);
					memset(erase_buf, 0xff, LEN);
					SpiFlashErase(SECTOR_ERASE, SEC, 0);
					SpiFlashRead(ADDR, fshc_buf, LEN, 100);
					SpiFlashIOCtrl(IOCTL_FLASH_PROTECT, FLASH_LOCK_RANGE_ALL);
					SpiFlashIOCtrl(IOCTL_FLASH_UNPROTECT, "\x35\xBA\x69", 3);
					SpiFlashErase(SECTOR_ERASE, SEC, 0);
					SpiFlashRead(ADDR, fshc_buf, LEN, 100);
					if(memcmp(fshc_buf, erase_buf, LEN) == 0)
					{
						DBG("FLASH_UNPROTECT OK\n");
					}
					else
					{
						DBG("FLASH_UNPROTECT error\n");
						while(1);
					}
					break;
			}
		}
	}

	while(1);
}



