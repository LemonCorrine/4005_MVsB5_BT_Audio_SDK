#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "type.h"
#include "spi_flash.h"
#include "chip_info.h"
#include "alg_config.h"
#include "alg_api.h"

//step 0 ���û����������Ȩ����
__attribute__((section(".alg_function"), optimize("O0")))
uint8_t Authoriz_CertificateionAtFlash(uint32_t  ApplyMode, uint8_t* Param)//��Ȩ֤������Flash��
{
	int i;
	uint8_t TempUUID[8];
	uint8_t buf[8];

	//step 1 ��ȡоƬUUID
	Chip_IDGet((uint64_t*)TempUUID);
	//step 3  ������Ȩ֤��,�������Flash
	if(MultiApplyCode1 == ApplyMode)
	{
		for(i=0;i<8;i++)
		{
			Param[i] = TempUUID[i]+1;//�����㷨1 �û��Զ���
		}
	}
	else if(MultiApplyCode2 == ApplyMode)
	{
		for(i=0;i<8;i++)
		{
			Param[i] = TempUUID[i]^0x5c;//�����㷨2 �û��Զ���
		}
	}
	else
	{
		return 0;
	}

	SpiFlashInit(80000000, MODE_1BIT, 0, FSHC_RC_CLK_MODE);//ע���û�������Ҫ���ú�flash_clk
	SpiFlashWrite(0xE4,Param,8,0);
	//ע��: �����������û�����������
	//ע�⣺�û��������Ҫ������֤֮��
	Reset_McuSystem();//��λ �����ٽ���SDK�󣬻���������flash������Ҫ���Ļ�ԭ
	SpiFlashRead(0xE4,buf,8,20);//���¶�ȡ����ֹǰ���д����ʧ��
	if(0 == memcmp(Param,buf,8))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

__attribute__((section(".alg_function"), optimize("O0")))
uint8_t Authoriz_CertificateionAtSram(uint32_t  ApplyMode, uint8_t* Param)//��Ȩ֤������SRAM��
{
	int i;
	uint8_t TempUUID[8];
///step 1 ��ȡоƬUUID
	Chip_IDGet((uint64_t*)TempUUID);
	//step2 ������Ȩ֤��
	if(MultiApplyCode1 == ApplyMode)
	{
		for(i=0;i<8;i++)
		{
			Param[i+4] = TempUUID[i]+1;//�����㷨1 �û��Զ���
		}
	}
	else if(MultiApplyCode2 == ApplyMode)
	{
		for(i=0;i<8;i++)
		{
			Param[i+4] = TempUUID[i]^0x5c;//�����㷨2 �û��Զ���
		}
	}
	//3  ������Ȩ֤��,�������
	Param[1] = 0xBC;
	Param[0] = 0xB0;
}

//ȫ�ֱ�����ZI���������ʼ������
//SDK������ʹ��֮ǰ���ã��������δ��ʼ��
__attribute__((section(".alg_main"), optimize("O0")))
void __c_init_key1()
{
/* Use compiler builtin memcpy and memset */
#define MEMCPY(des, src, n) __builtin_memcpy ((des), (src), (n))
#define MEMSET(s, c, n) __builtin_memset ((s), (c), (n))

	extern char _end;
	extern char __bss_start;
	int size;

	/* data section will be copied before we remap.
	 * We don't need to copy data section here. */
	extern char __data_lmastart_rom;
	extern char __data_start_rom;
	extern char _edata;

	/* Copy data section to RAM */
	size = &_edata - &__data_start_rom;
	MEMCPY(&__data_start_rom, &__data_lmastart_rom, size);

	/* Clear bss section */
	size = &_end - &__bss_start;
	MEMSET(&__bss_start, 0, size);
	return;
}

unsigned char Alg_GetVersion(unsigned char * string)
{
	if(string)
	{
		string[0] = ALG_MAJOR_VERSION + '0';
		string[1] = '.';
		string[2] = ALG_MINOR_VERSION + '0';
		string[3] = '.';
		string[4] = ALG_PATCH_VERSION + '0';
		string[5] = '\0';
		return 1;
	}
	return 0;
}

unsigned char Alg_TestAdd(unsigned char* buf ,int len)
{
	if(buf && len > 0)
	{
		int i;
		for(i = 0;i<len; i++)
			buf[i] += i;
		return 1;
	}
	return 0;
}

const MY_FUNLIST Alg_Funlist =
{
	.CertificateionAtFlash = Authoriz_CertificateionAtFlash,
	.CertificateionAtSram = Authoriz_CertificateionAtSram,
	.Alg_Ram_Init = __c_init_key1,
	.Alg_GetVer = Alg_GetVersion,
	.Alg_TestFunc = Alg_TestAdd
};

