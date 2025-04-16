/**
**************************************************************************************
* @file    deepsleep_test.c
* @brief   deepsleep_test.c
*
* @author  Long
* @version V1.0.0
*
* @Created: 2024/03/05 
*
* @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
**************************************************************************************
*/

#include <string.h>
#include <nds32_intrinsic.h>
#include "gpio.h"
#include "type.h"
#include "irqn.h"
#include "chip_info.h"
#include "uarts.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "timer.h"
#include "delay.h"
#include "rtc.h"
#include "watchdog.h"
#include "gpio.h"
#include "uarts_interface.h"
#include "dac.h"
#include "powercontroller.h"
#include "fft.h"
#include "ir.h"
#include "pwc.h"
#include "can.h"
#include "reset.h"
#include "efuse.h"
#include "spdif.h"
#include "otg_detect.h"
#include "spi_flash.h"
#include "pmu.h"
#include "adc.h"
#include "sys.h"

typedef enum
{
    WAKEUP_TEST_GPIO,
    WAKEUP_TEST_ONKEY,
    WAKEUP_TEST_CHARGEON,
    WAKEUP_TEST_LVD,
    WAKEUP_TEST_UART0RX,
    WAKEUP_TEST_RTC,
    WAKEUP_TEST_IR_CMD,
    WAKEUP_TEST_TIMER5,
    WAKEUP_TEST_TIMER5_PWC,
    WAKEUP_TEST_UART1RX,
    WAKEUP_TEST_CAN,
} WAKEUP_TEST_SOURCE;


#define GPIOA_CNT 32
#define GPIOB_CNT 10
#define GPIOC_CNT 1
#define ENTER_DEEPSLEEP_TEST_SECS (1)

static uint32_t sources;
extern volatile uint32_t gSysTick;
extern uint8_t UartPort;

extern void UARTLogIOConfig(void);

void SetRegBit(uint32_t addr, uint32_t bit)
{
    (*(volatile unsigned long *)addr) |= (1 << bit);
}

void ClrRegBit(uint32_t addr, uint32_t bit)
{
    (*(volatile unsigned long *)addr) &= ~(1 << bit);
}

__attribute__((section(".driver.isr"))) void Timer5Interrupt(void)
{
    uint32_t PWCValue;
    uint32_t ret;

    ret = Timer_InterruptFlagGet(TIMER5, PWC1_CAP_DATA_INTERRUPT_SRC |
                                             PWC_OVER_RUN_INTERRUPT_SRC | UPDATE_INTERRUPT_SRC);

    if (ret & PWC1_CAP_DATA_INTERRUPT_SRC)
    {
        PWCValue = PWC_IOCTRL(TIMER5, PWC_DATA_GET, NULL); // ���Ƚ�ֵ���Զ���CAP�ж�
        DBG("\n> Get value1 : %d\n", (int)PWCValue);
    }

    if (ret & PWC_OVER_RUN_INTERRUPT_SRC) // ����ʱ�����
    {
        Timer_InterruptFlagClear(TIMER5, PWC_OVER_RUN_INTERRUPT_SRC); // ���жϱ�־
        DBG("\nTimer5 PWC_OVER_RUN\n");
    }
}

void StartTimer5(uint32_t msec)
{
    uint32_t usec = 1000 * msec;
    NVIC_SetPriority(Timer5_IRQn, 2);
    NVIC_EnableIRQ(Timer5_IRQn);

    SetRegBit(0x4002103C, 6); //Timer5 ʹ��RC Clk
    Timer_InterruptFlagClear(TIMER5, UPDATE_INTERRUPT_SRC);
    Timer_Config(TIMER5, usec, 0);
    Timer_Start(TIMER5);
}

void StartTimer5PWCTest()
{
    TIMER_INDEX TimerIdx = TIMER5;
    uint8_t PolarIdx = PWC_POLARITY_RAISING;
    uint32_t PwcTimeScale = 12000;
    PWC_StructInit PWCParam;

    SetRegBit(0x4002103C, 6); // Timer5 ʹ��RC Clk
    NVIC_SetPriority(Timer5_IRQn, 2);
    NVIC_EnableIRQ(Timer5_IRQn);
    Timer_InterruptFlagClear(TIMER5, UPDATE_INTERRUPT_SRC);
    
    GPIO_RegOneBitSet(GPIO_A_IE, GPIO_INDEX4);
    PWC_GpioConfig(TIMER5, 4);
    DBG("PWC INPUT: A4\n");

    // PWC��������
    PWCParam.Polarity = PolarIdx;
    PWCParam.SingleGet = 0; // IsSingle;
    PWCParam.DMAReqEnable = 0;
    PWCParam.TimeScale = PwcTimeScale; // PWC�Ĳ�������ֵ = PWMFreqDiv / PwcTimeScale
    PWCParam.FilterTime = 3;

    PWC_Config(TimerIdx, &PWCParam);
    Timer_InterrputSrcEnable(TimerIdx, PWC1_CAP_DATA_INTERRUPT_SRC);
    PWC_Enable(TimerIdx);
}



static void WakeupSourceGet()
{
    uint32_t i=0;
    uint32_t getGpio;
    uint8_t getEdge;
    PWR_SYSWAKEUP_SOURCE_SEL source = SYSWAKEUP_SOURCE_NONE ;
    uint8_t UART_RecvByteVal;

	for(i = 0; i<= 15;i++)
	{
		source = sources & (1<<i);
		if(source == SYSWAKEUP_SOURCE_NONE)
		{
			continue;
		}

		switch(source)
		{
        case SYSWAKEUP_SOURCE6_POWERKEY:
            getEdge = Power_WakeupEdgeGet(source);
            DBG("ONKEY Wakeup--------------");
            if (getEdge)
            {
                DBG("posedge trigger \n");
            }
            else
            {
                DBG("negedge trigger \n");
            }
            break;
        case SYSWAKEUP_SOURCE7_CHARGEON:
            DBG("Chargeon Wakeup\n");
            break;

        case SYSWAKEUP_SOURCE8_LVD:
            DBG("LVD Wakeup\n");
            break;

        case SYSWAKEUP_SOURCE9_UARTRX:
            DBG("UART0_RX Wakeup\n");
            if (UARTS_RecvByte(0, &UART_RecvByteVal))
            {
                DBG("UART_RecvByteVal : %x\n", UART_RecvByteVal);
            }
            break;

        case SYSWAKEUP_SOURCE10_RTC:
            DBG("RTC Wakeup\n");
            break;

        case SYSWAKEUP_SOURCE11_IR:
            DBG("IR cmd Wakeup\n");
            break;

        case SYSWAKEUP_SOURCE12_TIMER5:
            DBG("Timer5 Wakeup\n");
            break;

        case SYSWAKEUP_SOURCE13_UART1:
            DBG("UART1 Wakeup\n");
            if (UARTS_RecvByte(1, &UART_RecvByteVal))
            {
                DBG("UART_RecvByteVal : %x\n", UART_RecvByteVal);
            }
            break;

        case SYSWAKEUP_SOURCE14_CAN:
            DBG("CAN Wakeup\n");
            break;

        case SYSWAKEUP_SOURCE15_BT:
            DBG(" baseband Wakeup\n");
            break;

        default:
            getGpio = Power_WakeupGpioGet(source);
            getEdge = Power_WakeupEdgeGet(source);

            DBG("GPIO Wakeup :    ");
            DBG("GPIO_SOURCE%ld ;", (i));
            if (getGpio <= WAKEUP_GPIOA31)
            {
                DBG("GPIO_A%ld ;", getGpio);
            }
            else if (getGpio <= WAKEUP_GPIOB9)
            {
                getGpio = getGpio - WAKEUP_GPIOB0;
                DBG("GPIO_B%ld ;", getGpio);
            }
            else if (getGpio == WAKEUP_GPIOC0)
            {
                getGpio = getGpio - WAKEUP_GPIOC0;
                DBG("GPIO_C%ld ;", getGpio);
            }

            if (getEdge)
            {
                DBG("posedge trigger \n");
            }
            else
            {
                DBG("negedge trigger \n");
            }
            break;
        }
	}
}


//���õ�GPIO����Դ
static void SystermGPIOWakeupConfig(PWR_SYSWAKEUP_SOURCE_SEL source,PWR_WAKEUP_GPIO_SEL gpio,PWR_SYSWAKEUP_SOURCE_EDGE_SEL edge)
{
    if (gpio < WAKEUP_GPIOB0)
    {
        GPIO_RegOneBitSet(GPIO_A_IE, (1 << gpio));
        GPIO_RegOneBitClear(GPIO_A_OE, (1 << gpio));
        if (edge == SYSWAKEUP_SOURCE_NEGE_TRIG)
        {
            GPIO_RegOneBitSet(GPIO_A_PU, (1 << gpio)); // ��ΪоƬ��GPIO���ڲ�����������,ѡ���½��ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
            GPIO_RegOneBitClear(GPIO_A_PD, (1 << gpio));
        }
        else if (edge == SYSWAKEUP_SOURCE_POSE_TRIG)
        {
            GPIO_RegOneBitClear(GPIO_A_PU, (1 << gpio)); // ��ΪоƬ��GPIO���ڲ����������裬����ѡ�������ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
            GPIO_RegOneBitSet(GPIO_A_PD, (1 << gpio));
        }
    }
    else if (gpio < WAKEUP_GPIOC0)
    {
        GPIO_RegOneBitSet(GPIO_B_IE, (1 << (gpio - WAKEUP_GPIOB0)));
        GPIO_RegOneBitClear(GPIO_B_OE, (1 << (gpio - WAKEUP_GPIOB0)));
        if (edge == SYSWAKEUP_SOURCE_NEGE_TRIG)
        {
            GPIO_RegOneBitSet(GPIO_B_PU, (1 << (gpio - WAKEUP_GPIOB0))); // ��ΪоƬ��GPIO���ڲ�����������,ѡ���½��ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
            GPIO_RegOneBitClear(GPIO_B_PD, (1 << (gpio - WAKEUP_GPIOB0)));
        }
        else if (edge == SYSWAKEUP_SOURCE_POSE_TRIG)
        {
            GPIO_RegOneBitClear(GPIO_B_PU, (1 << (gpio - WAKEUP_GPIOB0))); // ��ΪоƬ��GPIO���ڲ����������裬����ѡ�������ش���ʱҪ��ָ����GPIO���ѹܽ�����Ϊ����
            GPIO_RegOneBitSet(GPIO_B_PD, (1 << (gpio - WAKEUP_GPIOB0)));
        }
    }
    else if (gpio == WAKEUP_GPIOC0)
    {
        PMU_C0GPIOCtrlSet(1, 0, 0, (edge == SYSWAKEUP_SOURCE_POSE_TRIG));
    }

    Power_WakeupSourceClear();
	Power_WakeupSourceSet(source, gpio, edge);
	Power_WakeupEnable(source);

	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();
}

static void SystemWakeupConfig(PWR_SYSWAKEUP_SOURCE_SEL source, PWR_SYSWAKEUP_SOURCE_EDGE_SEL edge)
{
    NVIC_EnableIRQ(Wakeup_IRQn);
    NVIC_SetPriority(Wakeup_IRQn, 0);
    GIE_ENABLE();

    Power_WakeupSourceClear();
    Power_WakeupSourceSet(source, 0, edge);
    Power_WakeupEnable(source);
}

static void SystermUART_RXWakeupConfig(UART_PORT_T PortSel)
{
    if(PortSel == UART_PORT0)
    {
        /*Init uart0*/
        GPIO_PortAModeSet(GPIOA5, 2); // UART0 Rx
        GPIO_PortAModeSet(GPIOA6, 7); // UART0 Tx
        UARTS_Init(0, 2000000, 8, 0, 1);

        Power_WakeupSourceClear();
    	Power_WakeupSourceSet(SYSWAKEUP_SOURCE9_UARTRX, 0, 0);
    	Power_WakeupEnable(SYSWAKEUP_SOURCE9_UARTRX);
    }
    else if (PortSel == UART_PORT1)
    {
        /*Init uart1*/
        GPIO_PortAModeSet(GPIOA18, 4); // UART1 Tx
        GPIO_PortAModeSet(GPIOA19, 1); // UART1 Rx
        UARTS_Init(1, 2000000, 8, 0, 1);

        Power_WakeupSourceClear();
        Power_WakeupSourceSet(SYSWAKEUP_SOURCE13_UART1, 0, 0);
        Power_WakeupEnable(SYSWAKEUP_SOURCE13_UART1);
    }
    
	NVIC_EnableIRQ(Wakeup_IRQn);
	NVIC_SetPriority(Wakeup_IRQn, 0);
	GIE_ENABLE();
}

// ������Ҫ��RTC�жϺ�������������жϱ�־���������
__attribute__((section(".driver.isr"))) void RtcInterrupt(void)
{
    DBG("in %s\n", __FUNCTION__);
    if (RTC_IntFlagGet() == TRUE)
    {
        RTC_IntFlagClear();
    }
}


static void RTC_WakeupTestConfig()
{
    // Clock_EnableRTCLOSC32K();  //Temp enable RTC LOSC 32K clock
    RTC_ClockSrcSel(OSC_24M); // �˺����Ĳ���ѡ����������ϵͳ��ʼ��ѡ��ľ��񣨡�Clock_Config()��������һ��
    RTC_IntDisable();         // Ĭ�Ϲر�RTC�ж�
    RTC_IntFlagClear();
    RTC_WakeupEnable();
    RTC_IntEnable();
    NVIC_EnableIRQ(RTC_IRQn);

    RTC_SecSet(10);
    RTC_SecAlarmSet(20);
}

static void IR_WakeupTestConfig()
{
    GPIO_RegOneBitSet(GPIO_B_IE, GPIO_INDEX6);
    GPIO_RegOneBitClear(GPIO_B_OE, GPIO_INDEX6);
    GPIO_RegOneBitClear(GPIO_B_PU, GPIO_INDEX6);
    GPIO_RegOneBitClear(GPIO_B_PD, GPIO_INDEX6);

    IR_Config(IR_MODE_NEC, IR_GPIOB6, IR_NEC_16BITS);
    IR_Enable();

    IR_WakeupEnable();
    while(1)
    {
        DelayMs(150);
        if (IR_CommandFlagGet() == TRUE)
        {
            uint32_t Cmd = IR_CommandDataGet();
            DBG("\n---IR_Cmd:%08lx\n", Cmd);
            IR_CommandFlagClear();
            break;
        }
    }
    
    DBG("in %s\n", __FUNCTION__);
}

static void LVD_WakeupTestConfig()
{
    DBG("\r\n  %s  \r\n", __FUNCTION__);
    Power_LVDEnable();
    Power_LVDWakeupConfig(PWR_LVD_Threshold_3V0);
}

#define MAX_RX_LENGTH 32
typedef struct _BYTES_FIFO
{
    uint32_t Head;
    uint32_t Count;
    CAN_DATA_MSG Msg[MAX_RX_LENGTH];
} BYTES_RX_FIFO;

static volatile BYTES_RX_FIFO RxMsg;
CAN_INIT_STRUCT can_init;
#define SPDIF0_CAN_IRQn SPDIF_IRQn // 22
enum
{
    RATE_50KBPS = 0,
    RATE_100KBPS,
    RATE_125KBPS,
    RATE_250KBPS,
    RATE_500KBPS,
    RATE_1000KBPS,
    RATE_MAX_TABLE,
};

const uint8_t CanTest_Rate[RATE_MAX_TABLE][5] =
    {
        // PHSEG1	PHSEG2	SAM   TQ_BRP	SJW
        {12, 5, 1, 11, 0}, // 50K
        // {12, 5, 1, 5, 0}, // 50K   RC12K
        {12, 5, 1, 5, 0},  // 100K
        {10, 3, 1, 5, 0},  // 125K
        {10, 3, 1, 2, 1},  // 250K
        {15, 6, 1, 0, 1},  // 500K
        {7, 2, 1, 0, 1},   // 1M
};

// RATE_500KBPS
#define CAN_RC_TEST  (0)
#if CAN_RC_TEST
#define CAN_TESTRATE_SET RATE_50KBPS 
#else
// #define CAN_TESTRATE_SET RATE_50KBPS
#define CAN_TESTRATE_SET RATE_500KBPS
#endif

void can_msg_printf(CAN_DATA_MSG Msg)
{
    uint8_t len, i;

    DBG("ID:%X ,%s%s\n", Msg.Id, Msg.EFF ? "Extend Frame" : "Standard Frame", Msg.RTR ? ",Remote Frame" : "\0");

    len = Msg.DATALENGTH;
    if (Msg.RTR == 0)
    {
        DBG("DATA(%d): ", len);
        i = 0;
        while (len-- > 0)
        {
            DBG("%02X ", Msg.Data[i++]);
        }
        DBG("\n");
    }
}

static void CAN_ReadData()
{
    CAN_DATA_MSG Msg;
    
    while(1)
    {
		if (CAN_GetStatus() & CAN_RX_RDY)
		{
			DBG("RxMsgCnt: %d\n", CAN_GetRxMsgCnt());
			CAN_Recv(&Msg, 100);
            can_msg_printf(Msg);
            break;
        }
		DelayMs(2*1000);
		DBG("DATA_OR_FLAG:%d  %x\n", (CAN_GetStatus() & CAN_DATA_OR_FLAG) ? 1 : 0, CAN_GetStatus());
		if (CAN_GetStatus() & CAN_DATA_OR_FLAG)
			CAN_SetModeCmd(CAN_CMD_CLR_OR_FLAG);
    }
}

static void CAN_setRC12MClk()
{
    if (CAN_RC_TEST != 1)
    {
        return;
    }
    DBG("in %s\n", __FUNCTION__);
    CAN_ClkSelect(CAN_CLK_RC_12M);
}

static void CAN_restore24MClk()
{
    if (CAN_RC_TEST != 1)
    {
        return;
    }
    DBG("in %s\n", __FUNCTION__);
    CAN_ClkSelect(CAN_CLK_OSC_24M);
}

static void CAN_WakeupTestConfig()
{
    memset(&RxMsg, 0, sizeof(RxMsg));

    CAN_PortSelect(CAN_PORT_A3_A4);
    CAN_setRC12MClk();

    Reset_FunctionReset(CAN_FUNC_SEPA);

    CAN_SetModeCmd(CAN_MODE_RST_SELECT);

    can_init.PHSEG1 = CanTest_Rate[CAN_TESTRATE_SET][0];
    can_init.PHSEG2 = CanTest_Rate[CAN_TESTRATE_SET][1];
    can_init.SAM = CanTest_Rate[CAN_TESTRATE_SET][2];
    can_init.TQ_BRP = CanTest_Rate[CAN_TESTRATE_SET][3];
    can_init.SJW = CanTest_Rate[CAN_TESTRATE_SET][4];
    // Acceptance code and mask
    can_init.CAN_ACPC = 0x00;
    can_init.CAN_ACPM = 0xffffffff;
    CAN_Init(&can_init);


    CAN_RXTX_ERR_CNT cnt;
    cnt.ERR_WRN_LMT = 0x80;
    cnt.RX_ERR_CNT = 0;
    cnt.TX_ERR_CNT = 0;
    CAN_SetRxTxErrCnt(&cnt);

    // SREG_CAN_INT_EN.WAKEUP_EN = 1;
    CAN_SetModeCmd(CAN_MODE_AUWK_MODE);
    CAN_SetModeCmd(CAN_MODE_RST_DISABLE);
    CAN_IntTypeEnable(CAN_INT_OR_EN | CAN_INT_WAKEUP_EN);
    // CAN�жϺ�SPDIF����һ���жϺ� 22
    NVIC_EnableIRQ(SPDIF0_CAN_IRQn); // 22
    GIE_ENABLE();

    memset(&RxMsg, 0, sizeof(RxMsg));

    CAN_ReadData();
}


__attribute__((section(".driver.isr"))) void SystickInterrupt(void)
{
    SysTimerIntFlagClear();
    gSysTick++;
}

__attribute__((section(".driver.isr")))void WakeupInterrupt(void)
{
	if(Power_WakeupSourceGet() != SYSWAKEUP_SOURCE_NONE)
	{
		sources = Power_WakeupSourceGet();
		Power_WakeupSourceClear();
		// DBG("get_wk_src:%d,%d\r\n",Power_WakeupSourceGet(),sources);
	}
}

void closeADC()
{
    #include "audio_adc.h"
    // disable HPF
    AudioADC_HighPassFilterSet(ADC0_MODULE, FALSE); // SREG_ASDM0_CTRL.ASDM0_HPF_EN = 0;
    AudioADC_HighPassFilterSet(ADC1_MODULE, FALSE); // SREG_ASDM1_CTRL.ASDM1_HPF_EN = 0; //dac������������Ӱ��

    // disable aux channel
    AudioADC_LREnable(ADC0_MODULE, FALSE, FALSE);
    AudioADC_PGASel(ADC0_MODULE, CHANNEL_LEFT, LINEIN_NONE);
    AudioADC_PGASel(ADC0_MODULE, CHANNEL_RIGHT, LINEIN_NONE);

    // disbale mic channel
    AudioADC_LREnable(ADC1_MODULE, FALSE, FALSE);

    // disable sarADC
    ADC_Disable();
}

extern void DisableIDCache();

void SleepMain(uint8_t wakeup_src)
{
    Clock_UARTClkSelect(RC_CLK_MODE);         // ���л�log clk������������ٴ���
    closeADC();

    // disable DAC
    AudioDAC_AllPowerDown();

    // Flash 1bit mode
    SpiFlashInit(80000000, MODE_1BIT, 0, 1);

    // �л���RCʱ��
    Clock_DeepSleepSysClkSelect(RC_CLK_MODE, FSHC_RC_CLK_MODE, 1);
    // �ر�PLL
    Clock_PllClose();

    // �ر�APLL
    Clock_APllClose(); // APLL������Լ980uA�Ĺ���
    // �ر�HOSC
    if (wakeup_src != WAKEUP_TEST_RTC && wakeup_src != WAKEUP_TEST_CAN)
    {
        Clock_HOSCDisable(); // ����RTCӦ�ò���RTC����ʱ����HOSC���򲻹ر�HOSC����24M����
    }

    Power_LDO11DConfig(PWD_LDO11_LVL_0V95); // ���͵�0.95V

    // /*LDO11D���ó�0x4F*/
    //  (*(volatile unsigned long *)0x4002502C) &= (~(0x1FF << 19));
    //  (*(volatile unsigned long *)0x4002502C) |= (0x4F << 19);
    //  SetRegBit(0x4002502C, 28);
}


void DeepSleepIOConfig()
{
    GPIO_PortAModeSet(GPIOA0, 0x0000);
    GPIO_PortAModeSet(GPIOA1, 0x0000);
    GPIO_PortAModeSet(GPIOA2, 0x0000);
    GPIO_PortAModeSet(GPIOA3, 0x0000);
    GPIO_PortAModeSet(GPIOA4, 0x0000);
    GPIO_PortAModeSet(GPIOA5, 0x0000);
    GPIO_PortAModeSet(GPIOA6, 0x0000);
    GPIO_PortAModeSet(GPIOA7, 0x0000);
    // GPIO_PortAModeSet(GPIOA8, 0x0000);
    // GPIO_PortAModeSet(GPIOA9, 0x0000);
    // GPIO_PortAModeSet(GPIOA10, 0x0000);
    // GPIO_PortAModeSet(GPIOA11, 0x0000);
    // GPIO_PortAModeSet(GPIOA12, 0x0000);
    // GPIO_PortAModeSet(GPIOA13, 0x0000);
    // GPIO_PortAModeSet(GPIOA14, 0x0000);
    GPIO_PortAModeSet(GPIOA15, 0x0000);
    GPIO_PortAModeSet(GPIOA16, 0x0000);
    GPIO_PortAModeSet(GPIOA17, 0x0000);
    GPIO_PortAModeSet(GPIOA18, 0x0000);
    GPIO_PortAModeSet(GPIOA19, 0x0000);
    GPIO_PortAModeSet(GPIOA20, 0x0000);
    GPIO_PortAModeSet(GPIOA21, 0x0000);
    GPIO_PortAModeSet(GPIOA22, 0x0000);
    GPIO_PortAModeSet(GPIOA23, 0x0000); 
    GPIO_PortAModeSet(GPIOA24, 0x0000);
    //    GPIO_PortAModeSet(GPIOA25, 0x0000); //default 1:fshc_wp(io) for flash ctrl
    //    GPIO_PortAModeSet(GPIOA26, 0x0000); //default 1:fshc_hold(io) for flash ctrl
    GPIO_PortAModeSet(GPIOA28, 0x0000);
    GPIO_PortAModeSet(GPIOA29, 0x0000);
    GPIO_PortAModeSet(GPIOA30, 0x0000);
    GPIO_PortAModeSet(GPIOA31, 0x0000);

//    GPIO_PortBModeSet(GPIOB0, 0x000); // B0��B1һ�㸴��ΪSW���ص��Կ�
//    GPIO_PortBModeSet(GPIOB1, 0x000);
    GPIO_PortBModeSet(GPIOB2, 0x000);
    GPIO_PortBModeSet(GPIOB3, 0x000);
    GPIO_PortBModeSet(GPIOB4, 0x000);
    GPIO_PortBModeSet(GPIOB5, 0x000);
    GPIO_PortBModeSet(GPIOB6, 0x000);
    GPIO_PortBModeSet(GPIOB7, 0x000);
    GPIO_PortBModeSet(GPIOB8, 0x000);
    GPIO_PortBModeSet(GPIOB9, 0x000);

    GPIO_RegSet(GPIO_A_IE, 0x00000000);
    GPIO_RegSet(GPIO_A_OE, 0x00000000);
    GPIO_RegSet(GPIO_A_OUTDS0, 0x00000000);
    GPIO_RegSet(GPIO_A_OUTDS1, 0x00000000);
    GPIO_RegSet(GPIO_A_PD, 0xffffffff); //
    GPIO_RegSet(GPIO_A_PU, 0x00000000); //
    GPIO_RegSet(GPIO_A_ANA_EN, 0x00000000);
    GPIO_RegSet(GPIO_A_PULLDOWN0, 0x00000000);
    GPIO_RegSet(GPIO_A_PULLDOWN1, 0x00000000);

    GPIO_RegSet(GPIO_B_IE, 0x000);
    GPIO_RegSet(GPIO_B_OE, 0x000);
    GPIO_RegSet(GPIO_B_OUTDS, 0x000);
    GPIO_RegSet(GPIO_B_PD, 0x3ff); // B2��B3������B4,B5����
    GPIO_RegSet(GPIO_B_PU, 0x00); // B0��B1����
    GPIO_RegSet(GPIO_B_ANA_EN, 0x000);
    GPIO_RegSet(GPIO_B_PULLDOWN0, 0x000);
}

extern void SystemClockInit(void);

void WakeupMain(void)
{
    Chip_Init(1);

    Power_LDO11DConfig(PWD_LDO11_LVL_1V10); // �ָ�Ĭ��

    SystemClockInit();
    SysTickInit();

    Clock_DeepSleepSysClkSelect(PLL_CLK_MODE, FSHC_PLL_CLK_MODE, 0);
	SpiFlashInit(80000000, MODE_4BIT, 0, 1);
    UARTLogIOConfig(); // ����ʱ�Ӻ����䴮��ǰ��Ҫ��ӡ��

    DBG("wakeup\n");
}

void Power_GotoDeepsleeping(uint8_t wakeup_src)
{
    DelayMs(ENTER_DEEPSLEEP_TEST_SECS*1000);
    DBG("\r\n  %s!!!  \r\n", __FUNCTION__);
    
    SleepMain(wakeup_src);

    Power_GotoDeepSleep();

    WakeupMain();
}

void DeepSleep_Wakeup_Test(uint8_t WakeupSourceSel)
{
//    DeepSleepIOConfig();
    switch(WakeupSourceSel)
	{
    case WAKEUP_TEST_GPIO:
		DBG("\n        Wait for %d sec,then Enter DeepSleep_GPIOWakeup_Test                   \n"
				"A3/B9 NEGETRIG, A4/C0 POSE TRIG\n", ENTER_DEEPSLEEP_TEST_SECS);
        SystermGPIOWakeupConfig(SYSWAKEUP_SOURCE2_GPIO, WAKEUP_GPIOA3, SYSWAKEUP_SOURCE_NEGE_TRIG); // �˴�����GPIO����ԴGPIOA3,�½��ػ��ѣ�
        SystermGPIOWakeupConfig(SYSWAKEUP_SOURCE3_GPIO, WAKEUP_GPIOA4, SYSWAKEUP_SOURCE_POSE_TRIG); // �˴�����GPIO����ԴGPIOA4,�����ػ��ѣ�
        SystermGPIOWakeupConfig(SYSWAKEUP_SOURCE4_GPIO, WAKEUP_GPIOB9, SYSWAKEUP_SOURCE_NEGE_TRIG); // �˴�����GPIO����ԴGPIOB9,�½��ػ��ѣ�
        SystermGPIOWakeupConfig(SYSWAKEUP_SOURCE5_GPIO, WAKEUP_GPIOC0, SYSWAKEUP_SOURCE_POSE_TRIG); // �˴�����GPIO����ԴGPIOA3,�����ػ��ѣ�
        break;

    case WAKEUP_TEST_ONKEY:
        DBG("\n        Wait for %d sec,then Enter DeepSleep_ONKEYWakeup_Test                   \n", ENTER_DEEPSLEEP_TEST_SECS);
        SystemWakeupConfig(SYSWAKEUP_SOURCE6_POWERKEY, SYSWAKEUP_SOURCE_POSE_TRIG);
        break;

    case WAKEUP_TEST_LVD:
        DBG("\n        Wait for %d sec,then Enter DeepSleep_LVDWakeup_Test                   \n", ENTER_DEEPSLEEP_TEST_SECS);
        LVD_WakeupTestConfig();
        SystemWakeupConfig(SYSWAKEUP_SOURCE8_LVD, SYSWAKEUP_SOURCE_NEGE_TRIG);
        break;
    
    case WAKEUP_TEST_UART0RX:
        DBG("\n        Wait for %d sec,then Enter DeepSleep_UART0_RX_Wakeup_Test                   \n", ENTER_DEEPSLEEP_TEST_SECS);
        SystermUART_RXWakeupConfig(UART_PORT0);
        break;

    case WAKEUP_TEST_RTC:
        DBG("\n        Wait for %d sec,then Enter DeepSleep_RTCWakeup_Test                   \n", ENTER_DEEPSLEEP_TEST_SECS);
        RTC_WakeupTestConfig();
        SystemWakeupConfig(SYSWAKEUP_SOURCE10_RTC, SYSWAKEUP_SOURCE_NEGE_TRIG);
        break;

    case WAKEUP_TEST_IR_CMD:
        DBG("\n        Wait for %d sec,then Enter DeepSleep_IRWakeup_Test                   \n", ENTER_DEEPSLEEP_TEST_SECS);
        IR_WakeupTestConfig();
        SystemWakeupConfig(SYSWAKEUP_SOURCE11_IR, SYSWAKEUP_SOURCE_NEGE_TRIG);
        break;

    case WAKEUP_TEST_TIMER5:
        DBG("\n        Wait for %d sec,then Enter DeepSleep_TIMER5Wakeup_Test                  \n", ENTER_DEEPSLEEP_TEST_SECS);
        StartTimer5(3000);
        SystemWakeupConfig(SYSWAKEUP_SOURCE12_TIMER5, SYSWAKEUP_SOURCE_NEGE_TRIG);
        break;

    case WAKEUP_TEST_TIMER5_PWC:
        DBG("\n        Wait for %d sec,then Enter DeepSleep_TIMER5 PWC Wakeup_Test                  \n", ENTER_DEEPSLEEP_TEST_SECS);
        StartTimer5PWCTest();
        SystemWakeupConfig(SYSWAKEUP_SOURCE12_TIMER5, SYSWAKEUP_SOURCE_NEGE_TRIG);
        break;

    case WAKEUP_TEST_UART1RX:
        DBG("\n        Wait for %d sec,then Enter DeepSleep_UART1_RX_Wakeup_Test                  \n", ENTER_DEEPSLEEP_TEST_SECS);
        SystermUART_RXWakeupConfig(UART_PORT1);
        break;

    case WAKEUP_TEST_CAN:
        DBG("\n        Wait for %d sec,then Enter DeepSleep_CANWakeup_Test                  \n", ENTER_DEEPSLEEP_TEST_SECS);
        CAN_WakeupTestConfig();
        SystemWakeupConfig(SYSWAKEUP_SOURCE14_CAN, SYSWAKEUP_SOURCE_NEGE_TRIG);
        break;

    default:
		break;
	}

    Power_GotoDeepsleeping(WakeupSourceSel);

    GPIO_PortBModeSet(GPIOB0, 1);//SW_CLK
    GPIO_PortBModeSet(GPIOB1, 1);//SW_DATA

    NVIC_DisableIRQ(Wakeup_IRQn);
    NVIC_DisableIRQ(Timer5_IRQn);
    DBG("\n  -------------- Wake up from DeepSleep -------------  \n");
    // ��ȡIR��ֵ
    if (WakeupSourceSel == WAKEUP_TEST_IR_CMD && (IR_CommandFlagGet() == TRUE))
    {
        uint32_t Cmd = IR_CommandDataGet();
        DBG("\n---IR_Cmd:%08lx\n", Cmd);
        IR_IntFlagClear();
        IR_CommandFlagClear();
        Clock_IRRestoreDefaultClk();
    }
    else if (WakeupSourceSel == WAKEUP_TEST_CAN)
    {
        CAN_restore24MClk();
        // CAN_ReadData();
    }
    else if (WAKEUP_TEST_RTC == WakeupSourceSel)
    {
        RTC_ClockSrcSel(OSC_24M);
        Clock_DisableRTCLOSC32K();
    }
}


void DeepSleepProc()
{
	uint8_t recvBuf = 0;
    uint8_t WakeupSourceSel = 0;

    DBG("\n----------------- DeepSleepTest ------------------\n");
    DBG("Input '1' to enter DeepSleep_GOIOWakeup_Test\n");
    DBG("Input '2' to enter DeepSleep_ONKEYWakeup_Test\n");
    DBG("Input '3' to enter DeepSleep_ChargeOnWakeup_Test\n");
    DBG("Input '4' to enter DeepSleep_LVDWakeup_Test\n");
    DBG("Input '5' to enter DeepSleep_UART0RXWakeup_Test\n");
    DBG("Input '6' to enter DeepSleep_RTCWakeup_Test\n");
    DBG("Input '7' to enter DeepSleep_IRCmdWakeup_Test\n");
    DBG("Input '8' to enter DeepSleep_Timer5Wakeup_Test\n");
    DBG("Input '9' to enter DeepSleep_Timer5 PWC Wakeup_Test\n");
    DBG("Input 'A' to enter DeepSleep_UART1RXWakeup_Test\n");
    DBG("Input 'B' to enter DeepSleep_CanWakeup_Test\n");
    DBG("----------------- DeepSleepTest ------------------\n");
    while(1)
    {
        UARTS_Recv(UartPort, &recvBuf, 1, 10);

        if(recvBuf >= '1' && recvBuf <= '9')
        {
            WakeupSourceSel = WAKEUP_TEST_GPIO + (recvBuf - '1');
            DeepSleep_Wakeup_Test(WakeupSourceSel);
            WakeupSourceGet();
        }
        else if(recvBuf >= 'A' && recvBuf <= 'B')
        {
            WakeupSourceSel = WAKEUP_TEST_UART1RX + (recvBuf - 'A');
            DeepSleep_Wakeup_Test(WakeupSourceSel);
            WakeupSourceGet();
        }
    }
}
