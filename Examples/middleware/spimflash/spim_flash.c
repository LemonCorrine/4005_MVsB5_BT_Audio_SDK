#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif

#include "spim_flash.h"
#include "timeout.h"
#include "spim.h"
#include "debug.h"
#include "spi_flash.h"
#include "delay.h"
#include "rtos_api.h"

#ifdef CFG_FUNC_RECORD_EXTERN_FLASH_EN

extern void WDG_Feed(void);

#define SPI_FLASH_CS_HIGH   	GPIO_RegOneBitSet(GPIO_A_OUT, GPIOA24);
#define SPI_FLASH_CS_LOW    	GPIO_RegOneBitClear(GPIO_A_OUT, GPIOA24);
#define SPI_FLASH_CS_PIN_INIT   GPIO_RegOneBitSet(GPIO_A_OE, GPIOA24),GPIO_RegOneBitClear(GPIO_A_IE, GPIOA24)

static osMutexId ExFlashMutex = NULL;
//uint8_t SPI_FLASH_BUFFER[4096];

inline void SpiMasterSendByte(uint8_t data)
{
	SPIM_Send(&data, 1);
}

uint8_t SpiMasterRecvByte(void)
{
	uint8_t ret;
	SPIM_Recv(&ret, 1);
	return ret;
}

void SPI_Flash_Init(void)
{
	SPI_FLASH_CS_PIN_INIT;
	SPI_FLASH_CS_HIGH;
	SPIM_IoConfig(SPIM_PORT1_A20_A21_A22_A28);

	if(ExFlashMutex == NULL)
		ExFlashMutex = osMutexCreate();
}



void SPI_CS_HIGH(void)
{
	SPI_FLASH_CS_HIGH;
}
void SPI_CS_low(void)
{
	SPI_FLASH_CS_LOW;
}


uint16_t  SPI_Flash_ReadMID(void) //��ȡFLASH ID
{
    uint16_t FlashId = 0;
    SPI_FLASH_CS_LOW;
    SpiMasterSendByte(Flash_ManufactDeviceID);//0x90:��ȡDeviceId������
    SpiMasterSendByte(0x00);//����24λ�ĵ�ַ
    SpiMasterSendByte(0x00);
    SpiMasterSendByte(0x00);
    
    FlashId = SpiMasterRecvByte()<<8;//��ȡ��8λ
    FlashId += SpiMasterRecvByte();//��ȡ��8λ
    
    SPI_FLASH_CS_HIGH;
    return FlashId;
}

uint32_t SPI_Flash_ReadDeviceID()//��flash device id
{
    //��ȡFlash ID  
    uint32_t FlashDeviceID = 0;
    SPI_FLASH_CS_LOW;
	SpiMasterSendByte(0x9f);//���Ͷ�ȡID����	    
	FlashDeviceID |= SpiMasterRecvByte()<<16;  
	FlashDeviceID |= SpiMasterRecvByte()<<0;
	FlashDeviceID |= SpiMasterRecvByte()<<8;

	SPI_FLASH_CS_HIGH;	       
    return FlashDeviceID;
}

void SPI_FLASH_Write_Enable(void)//дʹ��
{
    SPI_FLASH_CS_LOW;
    SpiMasterSendByte(Flash_WriteEnable);//0x06:дʹ��
    SPI_FLASH_CS_HIGH;
}

void SPI_FLASH_Write_Disable(void)//д����
{
    SPI_FLASH_CS_LOW;
    SpiMasterSendByte(Flash_WriteDisable);//0x04:д����
    SPI_FLASH_CS_HIGH;
}

void SPI_FLASH_Write_SR(uint8_t sr)//д״̬�Ĵ���
{
    SPI_FLASH_CS_LOW;
    SpiMasterSendByte(Flash_EnableWriteStatusReg);//0x50:ʹ��д״̬�Ĵ���
    SpiMasterSendByte(Flash_WriteStatusReg);//0x01:д״̬�Ĵ���
    SpiMasterSendByte(sr);//д��һ���ֽ�    
    SPI_FLASH_CS_HIGH;
}

uint8_t SPI_Flash_ReadSR(void) //��ȡ״̬�Ĵ���
{   
    uint8_t byte=0;
    SPI_FLASH_CS_LOW;
    SpiMasterSendByte(Flash_ReadStatusReg);//0x05:��״̬�Ĵ�������
    SpiMasterSendByte(0xFF); 
    byte = SpiMasterRecvByte();
    SPI_FLASH_CS_HIGH;
    return byte;
}

uint16_t SPI_Flash_ReadStatus()//��ȡ������16bit״ֵ̬
{
    unsigned short	Status;	
    SPI_FLASH_CS_LOW;
	SpiMasterSendByte(0x05);	//low byte
	((unsigned char *)&Status)[0] = SpiMasterRecvByte();
    SPI_FLASH_CS_HIGH;

    SPI_FLASH_CS_LOW;
	SpiMasterSendByte(0x35);	//high byte
	((unsigned char *)&Status)[1] = SpiMasterRecvByte();
    SPI_FLASH_CS_HIGH;
    return 0;
}


void SPI_Flash_Wait_Busy(void)//æ��ȴ�
{
    while((SPI_Flash_ReadSR()&0x01) == 0x01);//�ȴ�BUSY���
}

void SPI_Flash_Erase_Chip(void)//��Ƭ����
{
    //DBG("Start erase flash...\n");
    //DBG("Erasing, please waiting...\n");

    SPI_FLASH_Write_Enable();//дʹ��
    SPI_Flash_Wait_Busy();
	SPI_FLASH_CS_LOW;
	SpiMasterSendByte(Flash_ChipErase);
	SPI_FLASH_CS_HIGH;
    SPI_Flash_Wait_Busy();
    
    //DBG(("Erase over!\n"));    
}

void SPI_Flash_Erase_Sector(uint32_t Dst_Addr)//��������Dst_Addr = 0,1,2...511, ����һ����������ʱ��Ϊ150ms   4
{
    Dst_Addr*=4096;   //(4*1024)
    SPI_FLASH_Write_Enable();//дʹ��
    SPI_Flash_Wait_Busy();//�ȴ����߿���
    
    SPI_FLASH_CS_LOW;
    SpiMasterSendByte(Flash_4KByte_BlockERASE);
    SpiMasterSendByte((uint8_t)(Dst_Addr>>16));
    SpiMasterSendByte((uint8_t)(Dst_Addr>>8));
    SpiMasterSendByte((uint8_t)Dst_Addr);
    
    SPI_FLASH_CS_HIGH;
    SPI_Flash_Wait_Busy();//�ȴ�оƬ��������
}


void SPI_Flash_Erase_32Block(uint32_t Dst_Addr)//32kBlock����0,1,2...
{
    Dst_Addr*=(32*1024);
    SPI_FLASH_Write_Enable();//дʹ��
    SPI_Flash_Wait_Busy();//�ȴ����߿���
    
    SPI_FLASH_CS_LOW;
    SpiMasterSendByte(Flash_32KByte_BlockErase);
    SpiMasterSendByte((uint8_t)(Dst_Addr>>16));
    SpiMasterSendByte((uint8_t)(Dst_Addr>>8));
    SpiMasterSendByte((uint8_t)Dst_Addr);
    SPI_FLASH_CS_HIGH;
    SPI_Flash_Wait_Busy();//�ȴ�оƬ��������
}



void SPI_Flash_Erase_64Block(uint32_t Dst_Addr)
{

uint32_t i_count=0;
#if 0
//DBG("KJKJHJHJKHJJHJKKKJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ\n")
    Dst_Addr*=(64*1024);
    SPI_FLASH_Write_Enable();//дʹ��
    SPI_Flash_Wait_Busy();//�ȴ����߿���
    
    SPI_FLASH_CS_LOW;
    SpiMasterSendByte(Flash_64KByte_BlockErase);
    SpiMasterSendByte((uint8_t)(Dst_Addr>>16));
    SpiMasterSendByte((uint8_t)(Dst_Addr>>8));
    SpiMasterSendByte((uint8_t)Dst_Addr);
    SPI_FLASH_CS_HIGH;
    SPI_Flash_Wait_Busy();//�ȴ�оƬ��������
#else
    osMutexLock(ExFlashMutex);
    Dst_Addr=Dst_Addr*64*1024/4096;
	
	for(i_count=0;i_count<16;i_count++)
	{
		SPI_Flash_Erase_Sector(i_count+Dst_Addr);
		WDG_Feed();		
	}

	osMutexUnlock(ExFlashMutex);
#endif

	
}

void SPI_Flash_Erase_4K(uint32_t Dst_Addr)
{
    osMutexLock(ExFlashMutex);

	SPI_Flash_Erase_Sector(Dst_Addr);

	osMutexUnlock(ExFlashMutex);
}

void SPI_Flash_Erase(ERASE_TYPE_ENUM type,uint32_t Dst_Addr)
{
	GIE_DISABLE();

	switch(type)
	{
		case CHIP_ERASE:
			SPI_Flash_Erase_Chip();
		break;
		case SECTOR_ERASE:
			SPI_Flash_Erase_Sector(Dst_Addr);
		break;
		case BLOCK_ERASE:
			SPI_Flash_Erase_64Block(Dst_Addr);
		break;
	}
	
	GIE_ENABLE();
	
	DelayUs(100);
}



//void SPI_Flash_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)//��ȡflash
//{

//}

//void SPI_Flash_Write(uint8_t pBuffer[],uint32_t WriteAddr,uint16_t NumByteToWrite)//д��Flash
//{


//}


//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!	 
void SPI_Flash_Write_Page(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	//uint16_t i;
    SPI_FLASH_Write_Enable();                      //SET WEL 
	SPI_FLASH_CS_LOW;                              //ʹ������   
    SpiMasterSendByte(Flash_PageProgramm);         //����дҳ����   
    SpiMasterSendByte((uint8_t)((WriteAddr)>>16)); //����24bit��ַ    
    SpiMasterSendByte((uint8_t)((WriteAddr)>>8));   
    SpiMasterSendByte((uint8_t)WriteAddr);   
//    for(i=0;i<NumByteToWrite;i++)
//        SpiMasterSendByte(pBuffer[i]);             //ѭ��д��
    SPIM_Send(pBuffer, NumByteToWrite);
	SPI_FLASH_CS_HIGH;                             //ȡ��Ƭѡ 
	SPI_Flash_Wait_Busy();					       //�ȴ�д�����
} 


//�޼���дSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void SPI_Flash_Write_NoCheck(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
{
	uint16_t pageremain;	   
	pageremain=256-WriteAddr%256; //��ҳʣ����ֽ���		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//������256���ֽ�
	while(1)
	{	   
		SPI_Flash_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//д�������
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
			if(NumByteToWrite>256)pageremain=256; //һ�ο���д��256���ֽ�
			else pageremain=NumByteToWrite; 	  //����256���ֽ���
		}
	};	
}

uint8_t SPI_Flash_Read(uint32_t ReadAddr,uint8_t* pBuffer,uint32_t NumByteToRead)//��ȡflash
{
    //uint16_t i;
	osMutexLock(ExFlashMutex);
	//GIE_DISABLE();
	SPI_FLASH_CS_LOW;                            //ʹ������   
    SpiMasterSendByte(Flash_ReadData);         //���Ͷ�ȡ����   
    SpiMasterSendByte((uint8_t)((ReadAddr)>>16));  //����24bit��ַ    
    SpiMasterSendByte((uint8_t)((ReadAddr)>>8));   
    SpiMasterSendByte((uint8_t)ReadAddr);

    SPIM_Recv(pBuffer, NumByteToRead);
//    for(i=0;i<NumByteToRead;i++)
//	{
//        pBuffer[i]=SpiMasterRecvByte();   //ѭ������
//    }
	SPI_FLASH_CS_HIGH;
	//GIE_ENABLE();
	osMutexUnlock(ExFlashMutex);
	DelayUs(20);
	return 0;
}

//дSPI FLASH  
//��ָ����ַ����
//�ú�������������!��ʼд��ָ�����ȵ�
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)						
//NumByteToWrite:Ҫд����ֽ���(���65535)   	 
void SPI_Flash_Write(uint32_t WriteAddr, uint8_t* pBuffer, uint32_t NumByteToWrite)
{ 
//	GIE_DISABLE();
	osMutexLock(ExFlashMutex);
	SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,NumByteToWrite);
	osMutexUnlock(ExFlashMutex);
//	GIE_ENABLE();
	
#if 0
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;	   
 	uint16_t i;    
	//uint8_t * SPI_FLASH_BUF;
   	//SPI_FLASH_BUF=SPI_FLASH_BUFFER;
 	secpos=WriteAddr/4096;//������ַ  
	secoff=WriteAddr%4096;//�������ڵ�ƫ��
	secremain=4096-secoff;//����ʣ��ռ��С   
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//������
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//������4096���ֽ�
	while(1) 
	{
#if 0
		SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			SPI_Flash_Erase_Sector(secpos);//�����������
			for(i=0;i<secremain;i++)	   //����
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//д����������  
		}
        else
#endif
        {  
			SPI_Flash_Erase_Sector(secpos);//�����������
            SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
        }

		if(NumByteToWrite==secremain)
        {
            break;//д�������

        }
        else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 	 

		   	pBuffer+=secremain;  //ָ��ƫ��
			WriteAddr+=secremain;//д��ַƫ��	   
		   	NumByteToWrite-=secremain;				//�ֽ����ݼ�
			if(NumByteToWrite>4096)secremain=4096;	//��һ����������д����
			else secremain=NumByteToWrite;			//��һ����������д����
		}	 
	}
#endif
}

//�������ģʽ
void SPI_Flash_PowerDown(void)   
{ 
  	SPI_FLASH_CS_LOW;                     //ʹ������   
    SpiMasterSendByte(Flash_PowerDown);   //���͵�������  
	SPI_FLASH_CS_HIGH;                    //ȡ��Ƭѡ     	      
    DelayMs(3);                           //�ȴ�TPD  
}   
//����
void SPI_Flash_WAKEUP(void)   
{  
  	SPI_FLASH_CS_LOW;                     //ʹ������   
    SpiMasterSendByte(Flash_ReleasePowerDown);//send Flash_PowerDown command 0xAB    
	SPI_FLASH_CS_HIGH;                    //ȡ��Ƭѡ     	      
    DelayMs(3);                           //�ȴ�TRES1
}   


void SPI_FLASH_LOCK()   //Flash��������
{  
    //SPI_FLASH_Write_SR(0x7E);
}


void SPI_FLASH_UNLOCK() //Flash����
{
    //SPI_FLASH_Write_SR(0x98);
}

#endif
