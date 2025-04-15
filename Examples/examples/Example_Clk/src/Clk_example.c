/*
 * Clk_example.c
 *
 *  Created on: Nov 20, 2023
 *      Author: zhaoshengqi
 */

#include <stdio.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "gpio.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "spi_flash.h"
#include "timeout.h"
#include "clk.h"
#include "pwm.h"
#include "irqn.h"
#include "delay.h"
#include "rtc.h"
#include "spim.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "sys.h"
#include "remap.h"
#include "chip_info.h"

#define UartPort   1

//ʱ����ʾ���̣���Ҫ��ʾϵͳʱ��ѡ��ο�Դ
//	1: 24M��������DPLL����240M���ں�ʱ��ΪPLLʱ�ӣ�ϵͳʱ��PLL 2��ƵΪ120M��
//	2: 24M��������APLL����240M���ں�ʱ��ΪPLLʱ�ӣ�ϵͳʱ��PLL 2��ƵΪ120M��
//	3��RC12M��ϵͳ������RCʱ����
//��������ʾ���ⲿ��������ʱ��Ҫ����ؾ���
//��PWM 10��Ƶ�� GPIOA0��ʾ�����۲�
//ϵͳĬ��ʹ��24M������DPLL240M,APLL240M

void main( void )
{
	   uint8_t key = 0;
	   PWM_StructInit PWMParam;

	   Chip_Init(1);
	   WDG_Disable();

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

	    // BP15ϵ�п��������ô��ڣ�Ĭ��ʹ��
	    GPIO_PortAModeSet(GPIOA9, 1);  // Rx, A9:uart1_rxd_1
	    GPIO_PortAModeSet(GPIOA10, 5); // Tx, A10:uart1_txd_1
	    DbgUartInit(1, 2000000, 8, 0, 1);




	   DBG("\n");
	   DBG("/-----------------------------------------------------\\\n");
	   DBG("|                    Clock Example                    |\n");
	   DBG("|      Mountain View Silicon Technology Co.,Ltd.      |\n");
	   DBG("\\-----------------------------------------------------/\n");
	   DBG("\n");

	   DBG("Driver Version: %s\n", GetLibVersionDriver());

		//PWM ������ڹ۲⣬
		PWM_GpioConfig(TIMER5_PWM_A0_A7_A10_A22_A24_B0_B1, 0, PWM_IO_MODE_OUT);
		DBG("PWM Init OUTPUT: A0\n");

		//PWM��������
		PWMParam.CounterMode   = PWM_COUNTER_MODE_DOWN;
		PWMParam.OutputType    = PWM_OUTPUT_SINGLE_1;
		PWMParam.FreqDiv       = 100;//PWMFreqTable[FreqIndex];
		PWMParam.Duty          = 50;//PWMDutyTable[DutyIndex];
		PWMParam.DMAReqEnable  = 0;
		PWM_Config(TIMER5, &PWMParam);
		PWM_Enable(TIMER5);

	    while(1)
	    {
	    	if(UARTS_Recv(UartPort, &key, 1,100) > 0)
			{
				switch( key ){
				case 'A':
				case 'a':
					Clock_Config(1, 24000000);
					Clock_PllLock(240000);
					Clock_SysClkSelect(PLL_CLK_MODE);
					Clock_UARTClkSelect(PLL_CLK_MODE);
					GPIO_PortAModeSet(GPIOA18, 4);
					GPIO_PortAModeSet(GPIOA19, 1);
					DbgUartInit(1, 256000, 8, 0, 1);
					DBG("system work on DPLL 240M, Soc 24M\n");
					break;
				case 'B':
				case 'b':
					Clock_Config(1, 24000000);
					Clock_APllLock(240000);
					Clock_SysClkSelect(APLL_CLK_MODE);
					Clock_UARTClkSelect(APLL_CLK_MODE);
					GPIO_PortAModeSet(GPIOA18, 4);
					GPIO_PortAModeSet(GPIOA19, 1);
					DbgUartInit(1, 256000, 8, 0, 1);
					DBG("system work on APLL 240M, Soc 24M\n");
					break;

				case 'C':
				case 'c':
					Clock_SysClkSelect(RC_CLK_MODE);
					Clock_UARTClkSelect(RC_CLK_MODE);
					GPIO_PortAModeSet(GPIOA18, 4);
					GPIO_PortAModeSet(GPIOA19, 1);
					DbgUartInit(1, 256000, 8, 0, 1);
					DBG("system work on RC 12M\n");
					break;
				default:
					break;
				}
			}
	    }
	    DBG("Err!\n");
		while(1);



}
