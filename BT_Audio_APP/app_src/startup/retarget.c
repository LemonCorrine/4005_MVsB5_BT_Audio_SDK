/*
 * retarget.c
 *
 *  Created on: Mar 8, 2017
 *      Author: peter
 */

#include <stdio.h>
#include "uarts_interface.h"
#include "type.h"
#include "remap.h"
#ifdef CFG_APP_CONFIG
#include "app_config.h"
#endif
uint8_t DebugPrintPort = UART_PORT0;

#include "uarts.h"
#include "sw_uart.h"
#include "rtos_api.h"
#include "mcu_circular_buf.h"

int DbgUartInit(int Which, unsigned int BaudRate, unsigned char DatumBits, unsigned char Parity, unsigned char StopBits)
{
#ifdef CFG_FUNC_DEBUG_EN
	DebugPrintPort = Which;
	if(DebugPrintPort == UART_PORT0 || DebugPrintPort == UART_PORT1)
	{
		UARTS_Init(DebugPrintPort,BaudRate, DatumBits,  Parity,  StopBits);
	}
	else
	{
		//uartfun = (uartfun)SwUartSend;
	}
#endif
	return 0;
}
#ifdef CFG_FUNC_USBDEBUG_EN
uint8_t 	usb_buffer[4096];
MCU_CIRCULAR_CONTEXT usb_fifo;

void usb_hid_printf_init(void)
{
	MCUCircular_Config(&usb_fifo,usb_buffer,sizeof(usb_buffer));
}
#endif

__attribute__((used))
int putchar(int c)
{

#ifdef CFG_FUNC_DEBUG_EN
	{
		if (c == '\n')
		{
			UARTS_SendByte_In_Interrupt(DebugPrintPort, '\r');
			UARTS_SendByte_In_Interrupt(DebugPrintPort, '\n');

			//UART0_SendByte('\r');
			//UART0_SendByte('\n');
		}
		else
		{
			UARTS_SendByte_In_Interrupt(DebugPrintPort, (uint8_t)c);
			//UART0_SendByte((uint8_t)c);
		}
	}
#endif
	return c;
}


__attribute__((used))
void nds_write(const unsigned char *buf, int size)
{
#ifdef CFG_FUNC_DEBUG_EN	
	int i;
	//usb_hid.usb_len = size;

		for (i = 0; i < size; i++)
		{
			putchar(buf[i]);
		}
#endif

#ifdef CFG_FUNC_USBDEBUG_EN
	MCUCircular_PutData(&usb_fifo,(void*)buf,size);
#endif		
}
