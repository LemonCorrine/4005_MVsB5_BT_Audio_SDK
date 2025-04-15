#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "alg_api.h"

#define MultiApplyCode   MultiApplyCode1
//#define AUTHORIZATION_ENC_FLASH  //�򿪺����ʾ��Ȩ֤������flash�У��رպ����ʾ��Ȩ֤������sram��
//
unsigned char AccessCredential(uint8_t *param)
{
	int i,j;
	uint8_t TempUUID[8];
	uint8_t temp[8];

	Chip_IDGet((uint64_t*)TempUUID);

	MY_FUNLIST * fun = MY_FUNLIST_ADDR;


#ifdef AUTHORIZATION_ENC_FLASH
	unsigned char buf[8];
	printf("Pointer to main1: %p\n", fun->CertificateionAtFlash);
//	Remap_AddrRemapDisable(ADDR_REMAP0);
//	Remap_InitTcm((uint32_t)0x4000, 8);//Ҫ����flash,��Ҫ��flash���������ŵ�TCM
	SpiFlashRead(0xE4,buf,8,20);//ֻ��0xE4 8���ֽ�
	if((0xFFFFFFFF == *((uint32_t*)&buf[0]))&&(0xFFFFFFFF == *((uint32_t*)&buf[4])))//�ж�û����֤��
	{
		fun->CertificateionAtFlash(MultiApplyCode,buf);//��������֤�麯�����洢��flash�У�
	}
//step 1 ������Ȩ������
	//����N������㷨
	if(MultiApplyCode1 == MultiApplyCode)
	{
		for(j=0;j<8;j++)
		{
			temp[j] = buf[j]-1;//�����㷨1 �û��Զ���
		}
	}
	if(MultiApplyCode2 == MultiApplyCode)
	{
		for(j=0;j<8;j++)
		{
			temp[j] = buf[j]^0x5c;//�����㷨2 �û��Զ���
		}
	}

//step 2 ��֤���ܺ��������
	if(0 == memcmp(TempUUID,temp,8))
	{
		return 3;//��֤�ɹ�
	}
	else{
    //��ʽһ
	//�ӳ�
	for(i=0;i<100000;i++)
	{
		__asm__("nop");
	}
	//��ת�ص�ROMBOOT
	__asm("sethi $r2,#0x01000\n");
	__asm("jral $r2\n");
	return 0xFF;//error site

	// ��ʽ��
//	Reset_McuSystem();

	//��ʽ��
//	return 0;//��֤ʧ��
	}
#else
	unsigned char AUTHORIZ_RAM[12];//�����Ȩ֤��
	printf("Pointer to main2: %p\n", fun->CertificateionAtSram);
	if((0xB0 != AUTHORIZ_RAM[0])&&(0xBC!= AUTHORIZ_RAM[1]))//�ж�û����֤��
	{
		fun->CertificateionAtSram(MultiApplyCode,AUTHORIZ_RAM);//��������֤�麯�����洢��sram�У�
	}
//	uint8_t temp[8];
//step 1 ������Ȩ������
	//����N������㷨
	if(MultiApplyCode1 == MultiApplyCode)
	{
		for(j=0;j<8;j++)
		{
			temp[j] = AUTHORIZ_RAM[j+4]-1;//�����㷨1 �û��Զ���
		}
	}
	if(MultiApplyCode2 == MultiApplyCode)
	{
		for(j=0;j<8;j++)
		{
			temp[j] = AUTHORIZ_RAM[j+4]^0x5c;//�����㷨2 �û��Զ���
		}
	}

//step 2 ��֤���ܺ��������
	if(0 == memcmp(TempUUID,temp,8))
	{
		return 3;//��֤�ɹ�
	}
	else{

//step3 ��֤ʧ��
	//��ʽһ
	//�ӳ�
//	for(i=0;i<100000;i++)
//	{
//		__asm__("nop");
//	}
//	//��ת�ص�ROMBOOT
//	__asm("sethi $r2,#0x01000\n");
//	__asm("jral $r2\n");
//	return 0xFF;//error site

	// ��ʽ��
//	Reset_McuSystem();

	//��ʽ��
	return 0;
	}
#endif
}

