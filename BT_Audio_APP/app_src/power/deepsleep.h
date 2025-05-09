#ifndef __DEEPSLEEP_H__
#define __DEEPSLEEP_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "ir.h"
#include "powercontroller.h"

void SystermGPIOWakeupConfig(PWR_SYSWAKEUP_SOURCE_SEL source,PWR_WAKEUP_GPIO_SEL gpio,PWR_SYSWAKEUP_SOURCE_EDGE_SEL edge);
void SystermIRWakeupConfig(IR_MODE_SEL ModeSel, IR_IO_SEL GpioSel, IR_CMD_LEN_SEL CMDLenSel);

//void SystermRTCWakeupConfig(uint8_t alarm_clock);

void SystermDeepSleepConfig(void);

void DeepSleeping(void);

bool SystermWackupSourceCheck(void);

void SystermWakeupConfig();

void NVIC_DisableForDeepsleep();//deepsleep期间关闭无用中断
void NVIC_EnableForDeepsleep();

#ifdef __cplusplus
}
#endif//__cplusplus

#endif/*__BREAKPOINT_H_*/
