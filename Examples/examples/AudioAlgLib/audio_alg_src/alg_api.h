#ifndef  _ALGORITHM_API_H_
#define  _ALGORITHM_API_H_

typedef enum
{
	MultiApplyCode1 = 0x12345678,//user defined
	MultiApplyCode2 = 0x78654321,//user defined
}MODE_MAGIC;

typedef void (*Alg_FlashRead)(uint32_t StartAddr, uint8_t* Buffer, uint32_t Length, uint32_t TimeOut);
typedef void (*Alg_FlashWrite)(uint32_t Addr, uint8_t	*Buffer, uint32_t 	Length, uint32_t IsSuspend);

typedef unsigned char (*Ida_Apply)(unsigned int ApplyMode, unsigned char* Param , Alg_FlashRead read_callback,Alg_FlashWrite write_callback);
typedef void (*Alg_Init)(void);
typedef unsigned char (*Get_String)(unsigned char * string);
typedef unsigned char (*Alg_Test)(unsigned char* buf ,int len);

typedef struct
{
	Ida_Apply 	CertificateionAtFlash;
	Ida_Apply 	CertificateionAtSram;
	Alg_Init  	Alg_Ram_Init;
	Get_String	Alg_GetVer;
	Alg_Test	Alg_TestFunc;
}MY_FUNLIST;

#define MY_FUNLIST_FLASH_ADDR			0x20

#endif
