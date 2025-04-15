#include "alg_api.h"


void Alg_RamInit(void)
{
	MY_FUNLIST * fun = MY_FUNLIST_ADDR;
	if(fun->Alg_Ram_Init)
		fun->Alg_Ram_Init();
}

unsigned char Get_AlgVersion(unsigned char * string)
{
	MY_FUNLIST * fun = MY_FUNLIST_ADDR;
	if(fun->Alg_GetVer)
		return fun->Alg_GetVer(string);
	return 0;
}

unsigned char Alg_UserTest(unsigned char* buf ,int len)
{
	MY_FUNLIST * fun = MY_FUNLIST_ADDR;
	if(fun->Alg_TestFunc)
		return fun->Alg_TestFunc(buf,len);
	return 0;
}

