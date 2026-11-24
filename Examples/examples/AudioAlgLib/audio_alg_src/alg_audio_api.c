#include "type.h"
#include "alg_api.h"
#include "alg_user_api.h"

MY_FUNLIST * GetAlgFunlist(void)
{
#if 0	//未加密的情况下 可以直接访问
	return *(unsigned int *)MY_FUNLIST_FLASH_ADDR;
#else	//加密的情况下需要用 SpiFlashRead
	uint32_t startAddress;
	if(AlgFlashApi.AlgFlashRead(MY_FUNLIST_FLASH_ADDR, &startAddress,4,20) == 0)
	{
		return startAddress;
	}
	else
	{
		return 0;
	}
#endif
}

void Alg_RamInit(void)
{
	MY_FUNLIST * fun = GetAlgFunlist();
	if(fun->Alg_Ram_Init)
		fun->Alg_Ram_Init();
}

unsigned char Get_AlgVersion(unsigned char * string)
{
	MY_FUNLIST * fun = GetAlgFunlist();
	if(fun->Alg_GetVer)
		return fun->Alg_GetVer(string);
	return 0;
}

unsigned char Alg_UserTest(unsigned char* buf ,int len)
{
	MY_FUNLIST * fun = GetAlgFunlist();
	if(fun->Alg_TestFunc)
		return fun->Alg_TestFunc(buf,len);
	return 0;
}

