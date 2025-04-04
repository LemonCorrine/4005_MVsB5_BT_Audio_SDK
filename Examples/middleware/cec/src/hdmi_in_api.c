/**
 **************************************************************************************
 * @file    hdmi_in_API.c
 * @brief
 *
 * @author  Cecilia Wang
 * @version V1.0.0
 *
 * $Created: 2018-03-23 13:06:47$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdlib.h>
#include "string.h"
#include <nds32_intrinsic.h>
#include "app_config.h"
#include "uarts.h"
#include "uarts_interface.h"
#include "type.h"
#include "debug.h"
#include "timeout.h"
#include "clk.h"
#include "dma.h"
#include "timer.h"
#include "watchdog.h"
#include "spi_flash.h"
#include "remap.h"
#include "gpio.h"
#include "chip_info.h"
#include "irqn.h"
#include "dac.h"
#include "dac_interface.h"
#include "hdmi_in_api.h"
#include "i2c.h"
#include "i2c_interface.h"
#include "app_config.h"
#include "mcu_circular_buf.h"
#include "audio_vol.h"
#include "main_task.h"
#include "delay.h"
#include "cec.h"
#include "reset.h"
#include "bt_config.h"
#include "hdmi_in_mode.h"

#ifdef  CFG_APP_HDMIIN_MODE_EN

#define HDMI_API_VERSION	"1.0.1"

#ifdef DDC_USE_SW_I2C
static void* DDCI2cHandler = NULL;
#endif

#define HDMI_OSD_NAME_LEN					8
uint8_t hdmi_osd_name[HDMI_OSD_NAME_LEN] = "Soundbar";

#define HDMI_CEC_TX_FIFO_LEN				(480 * (HDMI_OSD_NAME_LEN + 3))//如果该长度小于4096，建议设置为4096
#define HDMI_CEC_RX_FIFO_LEN				(1024)

extern bool    gIsVolSetEnable;
extern int32_t SetChannel ;

HDMIInfo  			 *gHdmiCt   = NULL;
uint32_t        	 *gCECRxFIFO;
uint32_t       	     *gCECTxFIFO;

MCU_CIRCULAR_CONTEXT *hdmiMCUCt = NULL;
CECInitTypeDef       *gCecInitDef = NULL;

const HDMI_TV_Info HDMITvInfo[HDMI_COMPATIBLE_TV_NUM] =
{		//TV Type				TV Product					  		thinking time    scan function               arbitration time
		{TV_SAMSUNG_1670,       {'s', 'a', 'm', 0x16, 0x70},  		 1,              HDMI_CEC_Default_Scan,      20, 16           },
		{TV_SONY_03EF,          {'s', 'n', 'y', 0x03, 0xef},  		 1,              HDMI_CEC_Default_Scan,      20, 16           },
		{TV_SONY_0537,          {'s', 'n', 'y', 0x05, 0x37},  		 1,              HDMI_CEC_Default_Scan,      20, 16           },
		{TV_POLARIOD_010B,      {'m', 't', 'c', 0x01, 0x0b},  		 1,              HDMI_CEC_Default_Scan,      20, 16           },
		{TV_LETV_0300,          {'l', 't', 'v', 0x30, 0x00},  		2000,            HDMI_CEC_UserDefined_Scan,  20, 16           },
		{TV_SONY_047C,          {'s', 'n', 'y', 0x04, 0x7c},  		 1,              HDMI_CEC_Default_Scan,      20, 16           },
		{TV_HISENSE_3000,       {'h', 'e', 'c', 0x30, 0x00},  		 1,              HDMI_CEC_Default_Scan,      20, 16           },
		{TV_XIAOMI_XMD,			{'x', 'm', 'd', 0x02, 0x00},         1,              HDMI_CEC_Default_Scan,      20, 16           },
		{TV_SONY_04A2,          {'s', 'n', 'y', 0x04, 0xa2},  		 1,              HDMI_CEC_Default_Scan,      20, 16           },
		{TV_SAMSUNG_170F,       {'s', 'a', 'm', 0x17, 0x0f},  		 1,              HDMI_CEC_Default_Scan,      20, 16           },
		{TV_TCL_2009,		    {'t', 'c', 'l', 0x20, 0x09},		 1, 			 HDMI_CEC_Default_Scan, 	 20, 16 		  },
		{TV_APX,       			{'a', 'p', 'x', 0x00, 0x70},  		 1,              HDMI_CEC_Default_Scan,      20, 16           },
		{TV_SONY_0571,			{'s', 'n', 'y', 0x05, 0x71},         1,              HDMI_CEC_Default_Scan,      20, 16           },
		{TV_SAMSUNG_0371,       {'s', 'a', 'm', 0x03, 0x71},  		 1,              HDMI_CEC_Default_Scan,      20, 16           },
		{TV_SAMSUNG_5770,       {'s', 'a', 'm', 0x57, 0x70},  		 1,              HDMI_CEC_Default_Scan,      20, 16           },
};

uint8_t HDMI_DDC_GetInfo(void);

void HDMI_HPD_CHECK_IO_INIT(void)
{
	GPIO_RegOneBitClear(HDMI_HPD_CHECK_STATUS_IO + 0x8, HDMI_HPD_CHECK_STATUS_IO_PIN);
	GPIO_RegOneBitSet  (HDMI_HPD_CHECK_STATUS_IO + 0x9, HDMI_HPD_CHECK_STATUS_IO_PIN);
	GPIO_RegOneBitClear(HDMI_HPD_CHECK_STATUS_IO + 0x6, HDMI_HPD_CHECK_STATUS_IO_PIN);
	GPIO_RegOneBitSet  (HDMI_HPD_CHECK_STATUS_IO + 0x5, HDMI_HPD_CHECK_STATUS_IO_PIN);
	GPIO_INTEnable     (HDMI_HPD_CHECK_INI_IO, HDMI_HPD_CHECK_STATUS_IO_PIN, GPIO_EDGE_TRIGGER);
	GPIO_INTFlagClear  (HDMI_HPD_CHECK_INI_IO, HDMI_HPD_CHECK_STATUS_IO_PIN);
}

void HDMI_HPD_INTFlagClear(void)
{
	GPIO_INTFlagClear(HDMI_HPD_CHECK_INI_IO, HDMI_HPD_CHECK_STATUS_IO_PIN);
}

bool HDMI_HPD_INTFlagGet(void)
{
	if(GPIO_INTFlagGet(HDMI_HPD_CHECK_INI_IO) & HDMI_HPD_CHECK_STATUS_IO_PIN)
		return 1;
	else
		return 0;
}

HDMI_HPD_STATUS HDMI_HPD_StatusGet(void)
{
	if(GPIO_RegOneBitGet(HDMI_HPD_CHECK_STATUS_IO, HDMI_HPD_CHECK_STATUS_IO_PIN) == HDMI_HPD_ACTIVE_LEVEL)
		return HDMI_HPD_CONNECTED_STATUS;
	else
		return HDMI_HPD_NOT_CONNECTED_STATUS;
}

void HDMI_CEC_DDC_Init(void)
{

	if(gHdmiCt)   		osPortFree(gHdmiCt);
	gHdmiCt = (HDMIInfo *)osPortMalloc(sizeof(HDMIInfo));
	if(gHdmiCt == NULL)	return;
	memset(gHdmiCt, 0, sizeof(HDMIInfo));

	if(gCECRxFIFO )		osPortFree(gCECRxFIFO);
	gCECRxFIFO = (uint32_t *)osPortMalloc(HDMI_CEC_RX_FIFO_LEN);
	if(gCECRxFIFO== NULL) return;
	memset(gCECRxFIFO, 0, HDMI_CEC_RX_FIFO_LEN);

	if(gCECTxFIFO)		osPortFree(gCECTxFIFO);
	gCECTxFIFO = (uint32_t *)osPortMalloc(HDMI_CEC_TX_FIFO_LEN);
	if(gCECTxFIFO== NULL) return;
	memset(gCECTxFIFO, 0, HDMI_CEC_TX_FIFO_LEN);

	if(gCecInitDef)		osPortFree(gCecInitDef);
	gCecInitDef = (CECInitTypeDef *)osPortMalloc(sizeof(CECInitTypeDef));
	if(gCecInitDef== NULL) return;
	memset(gCecInitDef, 0, sizeof(CECInitTypeDef));


	//交换与RX初始化的顺序，使其第一次发送没有毛刺
	gCecInitDef->pwm_io							=  HDMI_CEC_SEND_IO;
	gCecInitDef->pwm_io_pin						=  HDMI_CEC_SEND_IO_PIN;
	gCecInitDef->pwm_timer_id					=  HDMI_CEC_SEND_TIMER_ID;
	gCecInitDef->dma_tx_id						=  HDMI_CEC_SEND_DMA_ID;
	gCecInitDef->pwm_tx_fifo					=  (uint16_t *)gCECTxFIFO;
	gCecInitDef->dma_tx_param.Dir  				=  DMA_CHANNEL_DIR_MEM2PERI;
	gCecInitDef->dma_tx_param.Mode 				=  DMA_BLOCK_MODE;
	gCecInitDef->dma_tx_param.ThresholdLen 		=  0;
#if (CFG_SDK_VER_CHIPID == 0xB1)
	gCecInitDef->dma_tx_param.SrcDataWidth 		=  DMA_SRC_DWIDTH_HALF_WORD;
	gCecInitDef->dma_tx_param.DstDataWidth 		=  DMA_DST_DWIDTH_HALF_WORD;
#elif (CFG_SDK_VER_CHIPID == 0xB5)
	gCecInitDef->dma_tx_param.DataWidth			=  DMA_DWIDTH_HALF_WORD;
#endif
	gCecInitDef->dma_tx_param.SrcAddrIncremental=  DMA_SRC_AINCR_SRC_WIDTH;
	gCecInitDef->dma_tx_param.DstAddrIncremental=  DMA_DST_AINCR_NO;
	gCecInitDef->dma_tx_param.DstAddress        =  HDMI_CEC_SEND_DATA_ADDR;
	gCecInitDef->dma_tx_param.BufferLen         =  0;
	gCecInitDef->pwm_param.CounterMode   		=  PWM_COUNTER_MODE_UP;
	gCecInitDef->pwm_param.OutputType    		=  PWM_OUTPUT_SINGLE_1;

	if(HDMI_CEC_CLK_MODE == RC_CLK_MODE)
	{
		uint32_t tmp_clk = Clock_RcFreqGet(0);
		gCecInitDef->pwm_param.FreqDiv 			= (uint32_t)((120.0 / 10) * tmp_clk / (120 * 1000 * 1000 / 10) * 100 +0.5);
	}
	else
	{
#if (CFG_SDK_VER_CHIPID == 0xB1)
		gCecInitDef->pwm_param.FreqDiv 			= 14400;
#elif (CFG_SDK_VER_CHIPID == 0xB5)
		gCecInitDef->pwm_param.FreqDiv 			= 12000;
#endif
	}
	gCecInitDef->pwm_param.Duty          		=  100;
	gCecInitDef->pwm_param.DMAReqEnable			=  1;

	//gCecInitDef->pwc_io							=   HDMI_CEC_RECV_IO;
	//gCecInitDef->pwc_io_pin						=   HDMI_CEC_RECV_IO_PIN;
	gCecInitDef->pwc_timer_id					=   HDMI_CEC_RECV_TIMER_ID;
	gCecInitDef->dma_rx_id						=   HDMI_CEC_RECV_DMA_ID;
	gCecInitDef->dma_rx_param.Dir				=	DMA_CHANNEL_DIR_PERI2MEM;
	gCecInitDef->dma_rx_param.Mode  			=   DMA_CIRCULAR_MODE;
	gCecInitDef->dma_rx_param.ThresholdLen		=   2;//TaoWen; for rc clk
#if (CFG_SDK_VER_CHIPID == 0xB1)
	gCecInitDef->dma_rx_param.SrcDataWidth   	=   DMA_SRC_DWIDTH_HALF_WORD;
	gCecInitDef->dma_rx_param.DstDataWidth   	=   DMA_SRC_DWIDTH_HALF_WORD;
#elif (CFG_SDK_VER_CHIPID == 0xB5)
	gCecInitDef->dma_rx_param.DataWidth			= 	DMA_DWIDTH_HALF_WORD;
#endif
	gCecInitDef->dma_rx_param.SrcAddrIncremental=   DMA_SRC_AINCR_NO;
	gCecInitDef->dma_rx_param.DstAddrIncremental=   DMA_DST_AINCR_DST_WIDTH;
	gCecInitDef->dma_rx_param.SrcAddress		=	HDMI_CEC_RECV_DATA_ADDR;
	gCecInitDef->dma_rx_param.DstAddress		=	(uint32_t)gCECRxFIFO;
	gCecInitDef->dma_rx_param.BufferLen			=	HDMI_CEC_RX_FIFO_LEN;
	gCecInitDef->pwc_param.Polarity     		=   PWC_POLARITY_BOTH;
	gCecInitDef->pwc_param.SingleGet   			=   0;
	gCecInitDef->pwc_param.DMAReqEnable 		=   1;
	gCecInitDef->pwc_param.PwcSourceSelect		=   0;
	gCecInitDef->pwc_param.PwcOpt				=	1;

	if(HDMI_CEC_CLK_MODE == RC_CLK_MODE)//RC Clk 精度/使用10us
	{
		gCecInitDef->pwc_param.TimeScale 		=  (uint32_t)((120.0 / 10) * Clock_RcFreqGet(0) / (120 * 1000 * 1000 / 10) * 10 + 0.5) ;//RC时钟唤醒使用：10us步长
	}
	else
	{
#if (CFG_SDK_VER_CHIPID == 0xB1)
		gCecInitDef->pwc_param.TimeScale 		=  1440;//10us
#elif (CFG_SDK_VER_CHIPID == 0xB5)
		gCecInitDef->pwc_param.TimeScale 		=  1200;//10us
#endif
	}

	gCecInitDef->pwc_param.FilterTime   		=   12;

	gCecInitDef->cec_io_type					=  HDMI_CEC_IO_TYPE;
	gCecInitDef->cec_io_index					=  HDMI_CEC_IO_INDEX;
	gCecInitDef->hpd_status_io   	     		=  HDMI_HPD_CHECK_STATUS_IO;
	gCecInitDef->hpd_status_io_pin				=  HDMI_HPD_CHECK_STATUS_IO_PIN;
	gCecInitDef->arbitration_timer_id			=  HDMI_CEC_ARBITRATION_TIMER_ID;
	gCecInitDef->arbitration_timer_irq			=  HDMI_CEC_ARBITRATION_TIMER_IRQ;
	memset(&gCecInitDef->cec_info, 0, sizeof(CECInfo));
	HDMI_CEC_Init(gCecInitDef);

//#ifndef HDMI_HPD_CHECK_DETECT_EN
	HDMI_HPD_CHECK_IO_INIT();
//#endif

	#ifdef DDC_USE_SW_I2C
	if(DDCSclPortSel == PORT_A)
	{
		GPIO_PortAModeSet((1 << HdmiSclIndex), 0x0);
	}
	else if(DDCSclPortSel == PORT_B)
	{
		GPIO_PortBModeSet((1 << HdmiSclIndex), 0x0);

	}
	if(DDCSdaPortSel == PORT_A)
	{
		GPIO_PortAModeSet((1 << HdmiSdaIndex), 0x0);
	}
	else if(DDCSdaPortSel == PORT_B)
	{
		GPIO_PortBModeSet((1 << HdmiSdaIndex), 0x0);
	}

	if(!DDCI2cHandler)
	{
		DDCI2cHandler = I2cMasterCreate(DDCSclPortSel, HdmiSclIndex, DDCSdaPortSel, HdmiSdaIndex);
		if(!DDCI2cHandler)
		{
			APP_DBG("DDC I2C CREATE FAIL,PLS CHECK\n");
		}
	}
	#else
	HDMI_DDC_DATA_IO_INIT();
	HDMI_DDC_CLK_IO_INIT();
	I2C_Init(0x3F, HDMI_DDC_IO_PIN, 0xA0);
	#endif

	mainAppCt.hdmiResetFlg = 0;
	gHdmiCt->hdmi_poweron_flag = 0;
	gHdmiCt->hdmiReportStatus = 0;
	gHdmiCt->hdmi_arc_flag = 0;
	gHdmiCt->hdmiReportARCStatus = 0;
	gHdmiCt->hdmi_audiomute_flag = 0;
	gHdmiCt->hdmiActiveReportVolUpDownflag = 0;
	gHdmiCt->hdmiActiveReportMuteStatus = 0;
	gHdmiCt->hdmiActiveReportMuteflag = 0;
	gHdmiCt->hdmiActiveReportMuteAfterInit = 1;

	gHdmiCt->hdmi_tv_inf.tv_type = 0;
	gHdmiCt->hdmi_tv_inf.HDMI_CEC_SCAN_Func = HDMI_CEC_Default_Scan;
	gHdmiCt->hdmi_tv_inf.tv_reaction_time = 0;
	gHdmiCt->hdmi_tv_inf.GetArbitrTime = 16;
	gHdmiCt->hdmi_tv_inf.SendArbitrTime = 20;

	//HPD检测到高电平之后再读取EDID信息
	gHdmiCt->hdmiHPDCheck = HDMI_HPD_StatusGet();
	if(gHdmiCt->hdmiHPDCheck == HDMI_HPD_CONNECTED_STATUS)
	{
		osTaskDelay(5);//确保HPD的高电平状态
		gHdmiCt->hdmiHPDCheck = HDMI_HPD_StatusGet();
		if(gHdmiCt->hdmiHPDCheck == HDMI_HPD_CONNECTED_STATUS)
		{
			if(HDMI_DDC_GetInfo())//更新为读取所有的DDC信息
			{
				gHdmiCt->hdmiReportStatus = 1;
				gHdmiCt->hdmiReTransCnt = 0;
				TimeOutSet(&gHdmiCt->time_reaction, gHdmiCt->hdmi_tv_inf.tv_reaction_time);
			}
		}
	}
	TimeOutSet(&gHdmiCt->time_reaction, gHdmiCt->hdmi_tv_inf.tv_reaction_time);
}

void HDMI_CEC_DDC_DeInit(void)
{
	if(gCecInitDef)
	{//保障通信时序完整性，留意潜在风险。
		HDMI_CEC_DeInit(20);
		osPortFree(gCecInitDef);
	}
	gCecInitDef	= NULL;

	if(gHdmiCt)
		osPortFree(gHdmiCt);
	gHdmiCt = NULL;

	if(gCECRxFIFO )
		osPortFree(gCECRxFIFO);
	gCECRxFIFO = NULL;

	if(gCECTxFIFO)
		osPortFree(gCECTxFIFO);
	gCECTxFIFO = NULL;
}

//获取EDID所有信息,返回值 ：1 -- 正确 ， 0 -- 错误
uint8_t HDMI_DDC_GetInfo(void)
{
	const uint8_t IEEERegist[3] = {0x03, 0x0c, 0x00};
	const uint8_t MVDDCHeader[8] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
	char Tv_Manufacture[5];
	uint8_t i;
	uint8_t flag;
	uint16_t check_sum;
	#ifdef DDC_USE_SW_I2C
	#else
	I2C_ErrorState ret;
	#endif

	#ifdef DDC_USE_SW_I2C
	uint8_t MVDDCReadAddr_sf = 0xA0;
	#else
	uint8_t MVDDCReadAddr    = 0x50;
	#endif

	WDG_Feed();

	gHdmiCt->hdmi_tv_inf.tv_type = 0;
	gHdmiCt->hdmi_tv_inf.HDMI_CEC_SCAN_Func = HDMI_CEC_Default_Scan;
	gHdmiCt->hdmi_tv_inf.tv_reaction_time = 0;
	gHdmiCt->hdmi_tv_inf.GetArbitrTime = 16;
	gHdmiCt->hdmi_tv_inf.SendArbitrTime = 20;

	memset(&gHdmiCt->edid_buf[0], 0, 256);
	#ifdef DDC_USE_SW_I2C
	I2cReadNByte(DDCI2cHandler, MVDDCReadAddr_sf, 0, &gHdmiCt->edid_buf[0], 128);

	HDMI_DDC_TVManufacturerGet(gHdmiCt->edid_buf, (uint8_t*)Tv_Manufacture);

    //读取电视的厂商名字
    DBG("TV Manufacturer: %c%c%c\n" , Tv_Manufacture[0], Tv_Manufacture[1],Tv_Manufacture[2]);
    //读取电视的产品ID
    DBG("TV Product ID: 0x%x  0x%x\n" , Tv_Manufacture[3],Tv_Manufacture[4]);

    gHdmiCt->hdmi_tv_inf.tv_type = 0;
    for(i=0; i<HDMI_COMPATIBLE_TV_NUM; i++)
    {
    	if(!memcmp(HDMITvInfo[i].tv_vendor, Tv_Manufacture, 5))
    	{
    		memcpy(&gHdmiCt->hdmi_tv_inf.tv_type, &HDMITvInfo[i].tv_type, sizeof(HDMI_TV_Info));
    		if(gHdmiCt->hdmi_tv_inf.tv_type == TV_SAMSUNG_1670)
    	    {
    	        DBG("Samsung TV !!\n");
    	    }
    	    else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_HISENSE_3000)
    	    {
    	        DBG("Hisense_3000 TV !!\n");
    	    }
    	    else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_SONY_03EF)
    	    {
    	        DBG("Sony TV !!\n");
    	    }
    	    else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_SONY_0537)
    	    {
    	        DBG("TV_SONY_0537\n");
    	    }
    	    else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_POLARIOD_010B)
    	    {
    	        DBG("TV_POLARIOD\n");
    	    }
    	    else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_LETV_0300)
    	    {
    	        DBG("TV_LETV\n");
    	    }
    	    else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_SONY_047C)
    	    {
    	        DBG("TV_SONY_047C\n");
    	    }
    	    else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_SONY_04A2)
    	    {
    	        DBG("TV_SONY_04A2\n");
    	    }
    	    else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_SONY_0571)
    	    {
    	        DBG("TV_SONY_0571\n");
    	    }
    	    else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_SAMSUNG_170F)
    	    {
    	        DBG("TV Samsung_170f\n");
    	    }
			else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_TCL_2009)
    	    {
    	        DBG("TV TCL_2009\n");
    	    }
			else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_APX)
    	    {
    	        DBG("TV APX\n");
    	    }
    	    else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_SAMSUNG_0371)
    	    {
    	        DBG("TV_SAMSUNG_0371\n");
    	    }
    	    else if(gHdmiCt->hdmi_tv_inf.tv_type == TV_SAMSUNG_5770)
    	    {
    	        DBG("TV_SAMSUNG_5770\n");
    	    }
    		break;
    	}
    }

	#else
	I2C_MasterReceiveData(MVDDCReadAddr, 0x00, &gHdmiCt->edid_buf[0], 128, 1000);
	#endif

	flag = memcmp(gHdmiCt->edid_buf, MVDDCHeader, 8);
	check_sum = 0;
	for(i=0; i<127; i++)
	{
		check_sum += gHdmiCt->edid_buf[i];
	}
	check_sum = (0x100 - (check_sum & 0xff)) & 0xff;

	if((flag == 0) && (check_sum == gHdmiCt->edid_buf[127]))
	{
		if(gHdmiCt->edid_buf[126] >= 1)
		{
			#ifdef DDC_USE_SW_I2C
			I2cReadNByte(DDCI2cHandler, MVDDCReadAddr_sf, 128, &gHdmiCt->edid_buf[128], 128);
			#else
			I2C_MasterReceiveData(MVDDCReadAddr, 0x80, &gHdmiCt->edid_buf[128], 128, 1000);
			#endif

			check_sum = 0;
			for(i=128; i<255; i++)
			{
				check_sum += gHdmiCt->edid_buf[i];
			}
			check_sum = (0x100 - (check_sum & 0xff)) & 0xff;
			if(check_sum == gHdmiCt->edid_buf[255])
			{
				for(i=128; i<255; i++)
				{
					flag = memcmp(&gHdmiCt->edid_buf[i], IEEERegist, 3);
					if(flag == 0)
					{
						gCecInitDef->cec_info.physical_addr[0] = gHdmiCt->edid_buf[i+3];
						gCecInitDef->cec_info.physical_addr[1] = gHdmiCt->edid_buf[i+4];
						DBG("gCecInitDef->cec_info.physical_addr[0] = 0x%x\n",gCecInitDef->cec_info.physical_addr[0]);
						DBG("gCecInitDef->cec_info.physical_addr[1] = 0x%x\n",gCecInitDef->cec_info.physical_addr[1]);
						return 1;
					}
				}
			}
		}
	}

	return 0;
}


__attribute__((section(".driver.isr"))) void HDMI_CEC_ARBITRATION_TIMER_FUNC(void)
{
	if(gHdmiCt->hdmi_tv_inf.GetArbitrTime  == 0 || gHdmiCt->hdmi_tv_inf.GetArbitrTime  >= 200
	|| gHdmiCt->hdmi_tv_inf.SendArbitrTime == 0 || gHdmiCt->hdmi_tv_inf.SendArbitrTime >= 200)
	{
		gHdmiCt->hdmi_tv_inf.GetArbitrTime = 16;
		gHdmiCt->hdmi_tv_inf.SendArbitrTime = 20;
	}
	HDMI_Arbitration_Time_Process(gHdmiCt->hdmi_tv_inf.GetArbitrTime, gHdmiCt->hdmi_tv_inf.SendArbitrTime);
	if(gHdmiCt->hdmiBusyTime > 0) gHdmiCt->hdmiBusyTime--;
    Timer_InterruptFlagClear(HDMI_CEC_ARBITRATION_TIMER_ID, UPDATE_INTERRUPT_SRC);
}

//进入standby装填
void HDMI_CEC_StandbyStatus(void)
{
//	Reset_McuSystem();
#ifdef CFG_PARA_WAKEUP_SOURCE_CEC
	SoftFlagRegister(SoftFlagDeepSleepMsgIsFromTV);
	MessageContext		msgSend;
#ifdef BT_SNIFF_ENABLE
	msgSend.msgId		= MSG_BT_SNIFF;
#else
	msgSend.msgId		= MSG_DEEPSLEEP;
#endif
	MessageSend(mainAppCt.msgHandle, &msgSend);
#endif

}

//主动发起关机
void HDMI_CEC_ActivePowerOff(uint32_t timeout_value)
{
	TIMER busytime;
	bool  exit_flag = 0;
	if(gHdmiCt)
	{
		gHdmiCt->hdmi_poweron_flag = -1;
		TimeOutSet(&busytime, timeout_value);
		while(HDMI_CEC_IsWorking() == CEC_IS_WORKING)
		{
			WDG_Feed();
			if(IsTimeOut(&busytime))
			{
				exit_flag = 1;
				break;
			}
		}
		if(exit_flag == 0)
		{
			HDMI_CEC_UserDefined(0x5f, 0x36, NULL, 0);
			while(HDMI_CEC_IsWorking() == CEC_IS_WORKING)
			{
				WDG_Feed();
			}
			HDMI_CEC_UserDefined(0x5f, 0x36, NULL, 0);
			while(HDMI_CEC_IsWorking() == CEC_IS_WORKING)
			{
				WDG_Feed();
			}
		}
	}
}

extern bool HdmiSpdifLockFlag;
extern void AudioMusicVolUp(void);
extern void AudioMusicVolDown(void);
#if !defined(HDMI_HPD_CHECK_DETECT_EN)
extern bool Hdmi_Int_flag;
#endif
void HDMI_CEC_Scan(uint8_t isWorkStatus)
{
#ifndef HDMI_HPD_CHECK_DETECT_EN
	if(HDMI_HPD_INTFlagGet() || Hdmi_Int_flag)
#else
	if(HDMI_HPD_INTFlagGet() && (isWorkStatus == 0))
#endif
	{
#ifndef HDMI_HPD_CHECK_DETECT_EN
        Hdmi_Int_flag = 0;
#endif
        HDMI_HPD_INTFlagClear();
		gHdmiCt->hdmiHPDCheck = HDMI_HPD_StatusGet();
		if(gHdmiCt->hdmiHPDCheck == HDMI_HPD_CONNECTED_STATUS)
		{
			osTaskDelay(5);//确保HPD的高电平状态
			gHdmiCt->hdmiHPDCheck = HDMI_HPD_StatusGet();
			if(gHdmiCt->hdmiHPDCheck == HDMI_HPD_CONNECTED_STATUS)
			{
				if(HDMI_DDC_GetInfo())//更新为读取所有的DDC信息
				{
					gHdmiCt->hdmiReportStatus = 1;
					gHdmiCt->hdmi_poweron_flag = 0;
					gHdmiCt->hdmi_arc_flag = 0;
					gHdmiCt->hdmiReportARCStatus = 0;
					gHdmiCt->hdmi_audiomute_flag = 0;
					gHdmiCt->hdmiActiveReportVolUpDownflag = 0;
					gHdmiCt->hdmiActiveReportMuteStatus = 0;
					gHdmiCt->hdmiActiveReportMuteflag = 0;
					gHdmiCt->hdmiActiveReportMuteAfterInit = 1;
					TimeOutSet(&gHdmiCt->time_reaction, gHdmiCt->hdmi_tv_inf.tv_reaction_time);
					DBG("hdmiHPDCheck in\n");
					if(gHdmiCt->hdmi_tv_inf.tv_type == TV_SAMSUNG_170F)
			        {
			            if(HDMI_HPD_StatusGet())
						HDMI_CEC_SetSystemAudioModeOn();
			        }
				}
			}
	        else
	        {
	          DBG("hdmiHPDCheck out\n");
	        }
		}
	}

	gHdmiCt->hdmi_tv_inf.HDMI_CEC_SCAN_Func(isWorkStatus);

}


//------------------------------------------------------------------------

void HDMI_ARC_Init(uint16_t *DMAFifoAddr, uint32_t DMAFifoSize, MCU_CIRCULAR_CONTEXT *ct)
{
#if (CFG_SDK_VER_CHIPID == 0xB1)
	SPDIF_ModuleDisable();

	GPIO_RegOneBitClear(HDMI_ARC_RECV_IO_OE, HDMI_ARC_RECV_IO_PIN);
	GPIO_RegOneBitClear(HDMI_ARC_RECV_IO_IE, HDMI_ARC_RECV_IO_PIN);
	GPIO_RegOneBitSet(HDMI_ARC_RECV_IO_ANA,  HDMI_ARC_RECV_IO_PIN);
	SPDIF_AnalogModuleEnable(HDMI_ARC_RECV_ANA_PIN, SPDIF_ANA_LEVEL_200mVpp);

	SPDIF_RXInit(1, 1, 0);

	DMA_ChannelDisable(PERIPHERAL_ID_SPDIF_RX);
	//Reset_FunctionReset(DMAC_FUNC_SEPA);
	DMA_CircularConfig(PERIPHERAL_ID_SPDIF_RX, DMAFifoSize/2, DMAFifoAddr, DMAFifoSize);
	DMA_ChannelEnable(PERIPHERAL_ID_SPDIF_RX);

	SPDIF_ModuleEnable();
#elif (CFG_SDK_VER_CHIPID == 0xB5)
	SPDIF_ModuleDisable(CFG_HDMI_SPDIF_NUM);

	GPIO_RegOneBitClear(HDMI_ARC_RECV_IO_OE, HDMI_ARC_RECV_IO_PIN);
	GPIO_RegOneBitClear(HDMI_ARC_RECV_IO_IE, HDMI_ARC_RECV_IO_PIN);
	GPIO_RegOneBitSet(HDMI_ARC_RECV_IO_ANA,  HDMI_ARC_RECV_IO_PIN);

#if (CFG_HDMI_SPDIF_NUM == SPDIF0)
	SPDIF0_AnalogModuleEnable(HDMI_ARC_RECV_ANA_PIN, SPDIF_ANA_LEVEL_200mVpp);
#else
	SPDIF1_AnalogModuleEnable(HDMI_ARC_RECV_ANA_PIN, SPDIF_ANA_LEVEL_200mVpp);
#endif

	SPDIF_RXInit(CFG_HDMI_SPDIF_NUM,1, 0, 0);

	DMA_ChannelDisable(CFG_HDMI_DMA_CHANNEL);
	//Reset_FunctionReset(DMAC_FUNC_SEPA);
	DMA_CircularConfig(CFG_HDMI_DMA_CHANNEL, DMAFifoSize/2, DMAFifoAddr, DMAFifoSize);
	DMA_ChannelEnable(CFG_HDMI_DMA_CHANNEL);

	SPDIF_ModuleEnable(CFG_HDMI_SPDIF_NUM);
#endif
	hdmiMCUCt = ct;
}

uint8_t HDMI_ARC_LockStatusGet(void)
{
#if (CFG_SDK_VER_CHIPID == 0xB1)
	return SPDIF_FlagStatusGet(LOCK_FLAG_STATUS);
#elif (CFG_SDK_VER_CHIPID == 0xB5)
	return SPDIF_FlagStatusGet(CFG_HDMI_SPDIF_NUM,LOCK_FLAG_STATUS);
#endif
}

//0: not ready, 1: ready
bool HDMI_ARC_IsReady(void)
{
	//待cec通信完成，开启ARC，防止CEC对ARC的干扰
	//1. 有通讯； 2. cec没有收到arc开启的标志； 3. 非cec交互时间低于100ms
	if((gHdmiCt->hdmiReportStatus != 0) || (gHdmiCt->hdmi_arc_flag != 1) || (gHdmiCt->hdmiBusyTime != 0))
	{
		return 0;
	}
	else
	{
		return 1;
	}

}

uint16_t HDMI_ARC_DataLenGet(void)
{
#ifdef	CFG_AUDIO_WIDTH_24BIT
	uint8_t bitwidth = AudioCore.AudioSource[HDMI_IN_SOURCE_NUM].BitWidth ? sizeof(int32_t):sizeof(int16_t);
#else
	uint8_t bitwidth = sizeof(int16_t);
#endif
	if(hdmiMCUCt != NULL)
	{
		return (MCUCircular_GetDataLen(hdmiMCUCt) / bitwidth * 2);
	}
	else
	{
		return 0;
	}
}

uint16_t HDMI_ARC_DataGet(void *pcm_out, uint16_t MaxSize)
{
	uint16_t len;
#ifdef	CFG_AUDIO_WIDTH_24BIT
	uint8_t bitwidth = AudioCore.AudioSource[HDMI_IN_SOURCE_NUM].BitWidth ? sizeof(int32_t):sizeof(int16_t);
#else
	uint8_t bitwidth = sizeof(int16_t);
#endif

	if(hdmiMCUCt != NULL)
	{
		len = MCUCircular_GetData(hdmiMCUCt, pcm_out,MaxSize * bitwidth * 2);
		len = (len / (bitwidth * 2));
		return len;
	}
	else
	{
		return 0;
	}
}

void HDMI_ARC_DeInit(void)
{
#if (CFG_SDK_VER_CHIPID == 0xB1)
	SPDIF_ModuleDisable();
	DMA_ChannelDisable(PERIPHERAL_ID_SPDIF_RX);
	SPDIF_AnalogModuleDisable();
#elif (CFG_SDK_VER_CHIPID == 0xB5)
	SPDIF_ModuleDisable(CFG_HDMI_SPDIF_NUM);
	DMA_ChannelDisable(CFG_HDMI_DMA_CHANNEL);
#if (CFG_HDMI_SPDIF_NUM == SPDIF0)
	SPDIF0_AnalogModuleDisable();
#else
	SPDIF1_AnalogModuleDisable();
#endif
#endif
	GPIO_PortAModeSet(HDMI_ARC_RECV_IO_PIN, 0);
}

void HDMI_ARC_OnOff(uint8_t ArcOnFlag)
{
	if(ArcOnFlag == 1)
	{
		if(IsHDMISourceMute() == TRUE)
		{
			HDMISourceUnmute();
		}
		HDMI_CEC_SetSystemAudioModeOn();
		gHdmiCt->hdmi_audiomute_flag = 0;
		APP_DBG("StartARC\n");

	}
	else
	{
		if(IsHDMISourceMute() == FALSE)
		{
			HDMISourceMute();
		}
		AudioCoreSourceMute(APP_SOURCE_NUM, TRUE, TRUE);
		HDMI_CEC_SetSystemAudioModeoff();
		APP_DBG("StopARC\n");

	}
}



//--------------------------------cec scan function ----------------------
void HDMI_CEC_Default_Scan(uint8_t isWorkStatus)
{
	uint32_t flag;
	uint8_t  operValue[16];
	uint8_t  operLen;


	if(HDMI_CEC_IsWorking() == CEC_IS_IDLE)
    {
		flag = 0;
		memset(operValue, 0, sizeof(operValue));
		operLen = 0;
		operLen = HDMI_CEC_MessageDataGet(operValue);
		if(operLen > 0)
		{
			APP_DBG("#0x%x\n", operValue[0]);
			switch(operValue[0])
			{
				case 0xa0:
					if(gHdmiCt->hdmi_poweron_flag == 0)
						gHdmiCt->hdmiReportStatus = 4;
					break;

				case 0x36:
					if(isWorkStatus)//工作状态时发送MSG消息
					{
						HDMI_CEC_StandbyStatus();
					}
					gHdmiCt->hdmiReportStatus = 0;
					gHdmiCt->hdmi_poweron_flag = -1;
					gHdmiCt->hdmi_arc_flag = 0;
					gHdmiCt->hdmiReportARCStatus = 0;
					gHdmiCt->hdmi_audiomute_flag = 0;
					gHdmiCt->hdmiActiveReportVolUpDownflag = 0;
					gHdmiCt->hdmiActiveReportMuteStatus = 0;
					gHdmiCt->hdmiActiveReportMuteflag = 0;
					gHdmiCt->hdmiActiveReportMuteAfterInit = 0;
					flag = 1;
					break;
				case 0x46:
					HDMI_CEC_ReportOSDNameWithParam(hdmi_osd_name, HDMI_OSD_NAME_LEN);
					if(gHdmiCt->hdmi_poweron_flag == 0)
					 {
						if((gHdmiCt->hdmi_tv_inf.tv_type == TV_SONY_0571))
						   gHdmiCt->hdmiReportStatus = 0;
						else
						 {
							gHdmiCt->hdmiReportStatus = 4;
						 }
					 }
					flag = 1;
					break;

				case 0x80:
					HDMI_CEC_RoutingInfo();//切换网络后，重新注册和链接
					gHdmiCt->hdmiReportStatus = 1;
					gHdmiCt->hdmi_poweron_flag = 0;
					gHdmiCt->hdmi_arc_flag = 0;
					flag = 1;
					break;

				case 0x82:
					if(gHdmiCt->hdmi_poweron_flag == 0)//有些电视关机过程中，还会回复在线状态，此处禁止在关机过程中setAudioModeOn
						gHdmiCt->hdmi_poweron_flag = 1;
					if(gHdmiCt->hdmi_poweron_flag == 1)
						HDMI_CEC_SetSystemAudioModeOn();

					if((gHdmiCt->hdmi_poweron_flag == 1) && (gHdmiCt->hdmi_arc_flag == 0))
					{
						gHdmiCt->hdmiReportARCStatus = 1;
					}
					flag = 1;
					break;

				case 0x83://give physical address
					//HDMI_DDC_GetInfo();
					HDMI_CEC_ReportPhysicalAddress();
					if(gHdmiCt->hdmi_poweron_flag == 0)
						gHdmiCt->hdmiReportStatus = 4;
					flag = 1;
					break;

				case 0x70://system audio mode request
					if((gHdmiCt->hdmi_tv_inf.tv_type == TV_SAMSUNG_170F) || (gHdmiCt->hdmi_tv_inf.tv_type == TV_SONY_04A2))
					{
						if(operLen > 1)
						{
							HDMI_CEC_SetSystemAudioModeOn();
							gHdmiCt->hdmi_poweron_flag = 1;
							gHdmiCt->hdmiReportStatus  = 0;
							if(gHdmiCt->hdmi_arc_flag == 0)
								gHdmiCt->hdmiReportARCStatus = 1;
						}
						else
						{
							HDMI_CEC_SetSystemAudioModeoff();
							gHdmiCt->hdmi_poweron_flag = 1;
							gHdmiCt->hdmiReportStatus  = 0;
						}
					}
					else
					{
						{
							HDMI_CEC_SetSystemAudioModeOn();
							gHdmiCt->hdmi_poweron_flag = 1;
							gHdmiCt->hdmiReportStatus  = 0;
							if(gHdmiCt->hdmi_arc_flag == 0)
								gHdmiCt->hdmiReportARCStatus = 1;
						}
					}
					flag = 1;
					break;

				case 0x71://system audio status request
					gHdmiCt->hdmiGiveReportStatus = 1;
					if(IsHDMISourceMute() == TRUE)
					{
						HDMI_CEC_SystemAudioModeStatus((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) | 0x80);
					}
					else
					{
						HDMI_CEC_SystemAudioModeStatus((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) & 0x7f);
					}

					flag = 1;
					break;

				case 0x8c://Give Device Vendor ID
					HDMI_CEC_DeviceVendorID();
					if(gHdmiCt->hdmi_poweron_flag == 0)
						gHdmiCt->hdmiReportStatus = 4;
					flag = 1;
					break;

				case 0x7d://Give System Audio Mode Status
					if(gHdmiCt->hdmi_poweron_flag == 1)
					{
						HDMI_CEC_SystemAudioModeStatusOn();
						if(gHdmiCt->hdmi_arc_flag == 0)
							gHdmiCt->hdmiReportARCStatus = 1;
					}
					else
						HDMI_CEC_SystemAudioModeStatusOff();
					flag = 1;
					break;

				case 0xC1:
					gHdmiCt->hdmi_arc_flag = 1;
					gHdmiCt->hdmiReportARCStatus = 0;
					break;

				case 0xC2:
					gHdmiCt->hdmi_arc_flag = 0;
					gHdmiCt->hdmiReportARCStatus = 0;
					break;

				case 0xC3://Request ARC Initiation
					if(gHdmiCt->hdmi_poweron_flag == 0)
					{
						gHdmiCt->hdmi_poweron_flag = 1;
						HDMI_CEC_SetSystemAudioModeOn();
						gHdmiCt->hdmiReportARCStatus = 1;
					}
					else
					{
						HDMI_CEC_InitiateARC();
					}
					flag = 1;
					break;

				case 0xC4://Request ARC Termination
					HDMI_CEC_TerminationARC();
					flag = 1;
					break;

				case 0x44:
					gIsVolSetEnable = TRUE;
					SetChannel = AUDIO_DAC0_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM;

					if(operValue[1] == 0x41)//vol up
					{
						gHdmiCt->hdmi_audiomute_flag = 0;
						AudioMusicVolUp();
						HDMI_CEC_SystemAudioModeStatus(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM);
						//printf("cec vol up key\n");
					}
					else if(operValue[1] == 0x42)//vol DOWN
					{
						gHdmiCt->hdmi_audiomute_flag = 0;
						AudioMusicVolDown();
						HDMI_CEC_SystemAudioModeStatus(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM);
						//printf("cec vol down key\n");
					}
					else if(operValue[1] == 0x43)//vol MUTE
					{
						if(IsHDMISourceMute() == TRUE)//true-->false
						{
							HDMISourceMute();//消除unmute时的pop音
							HDMI_CEC_SystemAudioModeStatus((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) & 0x7f);
							gHdmiCt->hdmi_audiomute_flag = 0;
							APP_DBG("cec vol unmute key\n");
							HdmiSpdifLockFlag = 0;//消除unmute时的pop音
						}
						else//false-->true
						{
							HDMISourceMute();//消除unmute时的pop音
							HDMI_CEC_SystemAudioModeStatus((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) | 0x80);
							gHdmiCt->hdmi_audiomute_flag = 1;
							if((gHdmiCt->hdmi_tv_inf.tv_type == TV_SAMSUNG_1670) || (gHdmiCt->hdmi_tv_inf.tv_type == TV_SONY_0537))
							{
								gHdmiCt->hdmiActiveReportMuteflag = 2;
								gHdmiCt->hdmiActiveReportMuteStatus = 1;
							}
							APP_DBG("cec vol mute key\n");
						}

					}
					flag = 1;
					break;
				case 0x8f:
					HDMI_CEC_SystemPowerOn();
					printf("cec HDMI_CEC_SystemPowerOn\n");
					flag = 1;
					break;
				case 0x9f:
					HDMI_CEC_Version();
					flag = 1;
					break;

				case 0xA4:
#if 1
//					if((operValue[1] & 0xf) == 1)
//					{
//						HDMI_CEC_AudioFormatCode();
//					}
//					else
//					{
//						//uint8_t buf[3] = {0x16, 0x7F, 0x00};//第三个参数待定
//						//buf[0] = ((operValue[1] & 0xf)<<3) + 6;
//						//HDMI_CEC_UserDefined(0x50, 0xA3, buf, 3);
//						HDMI_CEC_AudioFormatCode();
//					}
					HDMI_CEC_AudioFormatCode();
#else
					if((operValue[1] & 0xf) != 1)
						HDMI_CEC_FeatureAbort(0xA4, 0x3);
					else
						HDMI_CEC_AudioFormatCode();
#endif
					flag = 1;
					break;

				default:
					break;
			}
		}

		if(flag == 0)
		{
			if(gHdmiCt->hdmiReportStatus == 1)//两次allocate逻辑地址(1,2)
			{
				HDMI_CEC_AllocateLogicalAddress();
				gHdmiCt->hdmiReportStatus = 2;
			}
			else if(gHdmiCt->hdmiReportStatus == 2)
			{
				HDMI_CEC_AllocateLogicalAddress();
				gHdmiCt->hdmiReportStatus = 3;
			}
			else if(gHdmiCt->hdmiReportStatus == 3)
			{
				HDMI_CEC_ReportPhysicalAddress();
				if(gHdmiCt->hdmi_tv_inf.tv_type == TV_SONY_047C)
				 {
					while(HDMI_CEC_IsWorking() == CEC_IS_WORKING)
					{
						WDG_Feed();
					}
					HDMI_CEC_SetSystemAudioModeOn();
					while(HDMI_CEC_IsWorking() == CEC_IS_WORKING)
					{
						WDG_Feed();
					}
					HDMI_CEC_SetSystemAudioModeOn();
					while(HDMI_CEC_IsWorking() == CEC_IS_WORKING)
					{
						WDG_Feed();
					}
				 }
				gHdmiCt->hdmiReportStatus = 4;
			}
			else if(gHdmiCt->hdmiReportStatus == 4)//增加重要信息的重发机制
			{
				HDMI_CEC_RequestActiveSource();
				gHdmiCt->hdmiReportStatus = 5;
			}
			else if(gHdmiCt->hdmiReportStatus == 5)
			{
				HDMI_CEC_RequestActiveSource();
				gHdmiCt->hdmiReportStatus = 0;
			}
			else if(gHdmiCt->hdmiReportARCStatus == 1)
			{
				HDMI_CEC_InitiateARC();
				gHdmiCt->hdmiReportARCStatus = 0;
			}
			else if(gHdmiCt->hdmiActiveReportVolUpDownflag > 0)//由于该消息没有回应信息，增加重发机制保证成功率
			{
				gHdmiCt->hdmi_audiomute_flag = 0;
				HDMI_CEC_SystemAudioModeStatus((uint8_t)(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) & 0x7f);
				gHdmiCt->hdmiActiveReportVolUpDownflag--;
			}
			else if(gHdmiCt->hdmiActiveReportMuteflag > 0)//由于该消息没有回应信息，增加重发机制保证成功率
			{
				if(gHdmiCt->hdmiActiveReportMuteStatus == 1)
				{
					HDMISourceMute();//消除unmute时的pop音
					gHdmiCt->hdmi_audiomute_flag = 1;
					HDMI_CEC_SystemAudioModeStatus((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) | 0x80);
				}
				else
				{
					HDMI_CEC_SystemAudioModeStatus((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) & 0x7f);
					HDMISourceUnmute();
					gHdmiCt->hdmi_audiomute_flag = 0;
				}
				gHdmiCt->hdmiActiveReportMuteflag--;
			}
		}

		if(mainAppCt.hdmiResetFlg == 2)//电视兼容性问题：解决小米电视快速插入偶发性无声问题
		{
			mainAppCt.hdmiResetFlg = 0;
			gHdmiCt->hdmi_poweron_flag = 0;
			gHdmiCt->hdmi_arc_flag = 0;
			gHdmiCt->hdmiReportARCStatus = 0;
			gHdmiCt->hdmi_audiomute_flag = 0;
			gHdmiCt->hdmiActiveReportVolUpDownflag = 0;
			gHdmiCt->hdmiActiveReportMuteStatus = 0;
			gHdmiCt->hdmiActiveReportMuteflag = 0;
			gHdmiCt->hdmiActiveReportMuteAfterInit = 1;
			HDMI_DDC_GetInfo();
			TimeOutSet(&gHdmiCt->time_reaction, gHdmiCt->hdmi_tv_inf.tv_reaction_time);
			gHdmiCt->hdmiReportStatus = 1;
		}
	}
	else if(HDMI_CEC_IsWorking() == CEC_IS_WORKING)
	{
		gHdmiCt->hdmiBusyTime = 1000;
	}
}


void HDMI_CEC_UserDefined_Scan(uint8_t isWorkStatus)
{
	uint32_t flag;
	uint8_t  operValue[16];
	uint8_t  operLen;

	if(HDMI_CEC_IsWorking() == CEC_IS_IDLE)
    {
		flag = 0;
		memset(operValue, 0, sizeof(operValue));
		operLen = 0;
		operLen = HDMI_CEC_MessageDataGet(operValue);
		if(operLen > 0)
		{
			APP_DBG("#0x%x\n", operValue[0]);
			if(!IsTimeOut(&gHdmiCt->time_reaction))
				TimeOutSet(&gHdmiCt->time_reaction, 1);
			switch(operValue[0])
			{
				case 0xa0:
					break;

				case 0x36:
					if(isWorkStatus)//工作状态时发送MSG消息
					{
						HDMI_CEC_StandbyStatus();
					}
					gHdmiCt->hdmiReportStatus = HDMI_INIT_STATUS;
					gHdmiCt->hdmiReTransCnt = 0;
					gHdmiCt->hdmi_poweron_flag = -1;
					gHdmiCt->hdmi_arc_flag = 0;
					gHdmiCt->hdmiReportARCStatus = 0;
					gHdmiCt->hdmi_audiomute_flag = 0;
					gHdmiCt->hdmiActiveReportVolUpDownflag = 0;
					gHdmiCt->hdmiActiveReportMuteStatus = 0;
					gHdmiCt->hdmiActiveReportMuteflag = 0;
					gHdmiCt->hdmiActiveReportMuteAfterInit = 0;
					flag = 1;
					break;
				case 0x46:
					HDMI_CEC_ReportOSDNameWithParam(hdmi_osd_name, HDMI_OSD_NAME_LEN);
					flag = 1;
					break;

				case 0x80:
					HDMI_CEC_RoutingInfo();//切换网络后，重新注册和链接
					gHdmiCt->hdmiReportStatus = HDMI_ALLOCATE_ADDR;
					gHdmiCt->hdmiReTransCnt = 0;
					gHdmiCt->hdmi_poweron_flag = 0;
					gHdmiCt->hdmi_arc_flag = 0;
					flag = 1;
					break;

				case 0x82:
					if(gHdmiCt->hdmi_poweron_flag == 0)//有些电视关机过程中，还会回复在线状态，此处禁止在关机过程中setAudioModeOn
						gHdmiCt->hdmi_poweron_flag = 1;
					if(gHdmiCt->hdmiReportStatus == HDMI_REQUEST_ACTIVE_SRC)
					{
						HDMI_CEC_SetSystemAudioModeOn();
						gHdmiCt->hdmiReportStatus = HDMI_SET_AUDIO_MODE;
						gHdmiCt->hdmiReTransCnt = 1;

						if((gHdmiCt->hdmi_poweron_flag == 1) && (gHdmiCt->hdmi_arc_flag == 0))
						{
							gHdmiCt->hdmiReportARCStatus = 1;
						}
					}
					flag = 1;
					break;

				case 0x83://give physical address
					if(gCecInitDef->cec_info.physical_addr[0] == 0x00)
						HDMI_DDC_GetInfo();
					HDMI_CEC_ReportPhysicalAddress();
					if(gHdmiCt->hdmi_poweron_flag == 0)
					{
						gHdmiCt->hdmiReportStatus = HDMI_REQUEST_ACTIVE_SRC;
						gHdmiCt->hdmiReTransCnt = 0;
						TimeOutSet(&gHdmiCt->time_waitTime, 1);
					}
					flag = 1;
					break;

				case 0x70://system audio mode request
					if(operLen > 1)
					{
						HDMI_CEC_SetSystemAudioModeOn();
						gHdmiCt->hdmi_poweron_flag = 1;
						gHdmiCt->hdmiReportStatus = HDMI_SET_AUDIO_MODE;
						gHdmiCt->hdmiReTransCnt = 1;//重发机制
						if(gHdmiCt->hdmi_arc_flag == 0)
							gHdmiCt->hdmiReportARCStatus = 1;
					}
					/*else
					{
						HDMI_CEC_SetSystemAudioModeoff();
						gHdmiCt->hdmi_poweron_flag = 1;
						gHdmiCt->hdmiReportStatus  = 1;
					}*/
					flag = 1;
					break;

				case 0x71://system audio status request
					gHdmiCt->hdmiGiveReportStatus = 1;
					if(IsHDMISourceMute() == TRUE)
					{
						HDMI_CEC_SystemAudioModeStatus((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) | 0x80);
					}
					else
					{
						HDMI_CEC_SystemAudioModeStatus((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) & 0x7f);
					}

					flag = 1;
					break;

				case 0x8c://Give Device Vendor ID
					HDMI_CEC_DeviceVendorID();
					if(gHdmiCt->hdmiReportStatus != HDMI_ALLOCATE_ADDR)
					{
						if(gHdmiCt->hdmi_poweron_flag == 0)
						{
							gHdmiCt->hdmiReportStatus = HDMI_REQUEST_ACTIVE_SRC;
							gHdmiCt->hdmiReTransCnt = 0;
							TimeOutSet(&gHdmiCt->time_waitTime, 1);
						}
					}
					flag = 1;
					break;

				case 0x7d://Give System Audio Mode Status
					if(gHdmiCt->hdmi_poweron_flag == 1)
					{
						HDMI_CEC_SystemAudioModeStatusOn();
						if(gHdmiCt->hdmi_arc_flag == 0)
						{
							gHdmiCt->hdmiReportARCStatus = 1;
							gHdmiCt->hdmiReTransCnt = 0;
						}
					}
					else
						HDMI_CEC_SystemAudioModeStatusOff();
					flag = 1;
					break;

				case 0xC1:
					gHdmiCt->hdmi_arc_flag = 1;
					gHdmiCt->hdmiReportARCStatus = 0;
					break;

				case 0xC2:
					gHdmiCt->hdmi_arc_flag = 0;
					gHdmiCt->hdmiReportARCStatus = 0;
					break;

				case 0xC3://Request ARC Initiation
					if(gHdmiCt->hdmi_poweron_flag == 0)
					{
						gHdmiCt->hdmi_poweron_flag = 1;
						gHdmiCt->hdmiReportStatus = HDMI_SET_AUDIO_MODE;
						gHdmiCt->hdmiReTransCnt = 0;
						gHdmiCt->hdmiReportARCStatus = 1;
					}
					else
					{
						HDMI_CEC_InitiateARC();
						gHdmiCt->hdmiReTransCnt = 1;
						gHdmiCt->hdmiReportARCStatus = 1;
					}
					flag = 1;
					break;

				case 0xC4://Request ARC Termination
					HDMI_CEC_TerminationARC();
					flag = 1;
					break;

				case 0x44:
					gIsVolSetEnable = TRUE;
					SetChannel = AUDIO_DAC0_SINK_NUM + AUDIO_CORE_SOURCE_MAX_NUM;

					if(operValue[1] == 0x41)//vol up
					{
						gHdmiCt->hdmi_audiomute_flag = 0;
						AudioMusicVolUp();
						HDMI_CEC_SystemAudioModeStatus(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM);
						APP_DBG("cec vol up key\n");
					}
					else if(operValue[1] == 0x42)//vol DOWN
					{
						gHdmiCt->hdmi_audiomute_flag = 0;
						AudioMusicVolDown();
						HDMI_CEC_SystemAudioModeStatus(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM);
						APP_DBG("cec vol down key\n");
					}
					else if(operValue[1] == 0x43)//vol MUTE
					{
						if(IsHDMISourceMute() == TRUE)//true-->false
						{
							HDMISourceMute();//消除unmute时的pop音
							HDMI_CEC_SystemAudioModeStatus((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) & 0x7f);
							gHdmiCt->hdmi_audiomute_flag = 0;
							APP_DBG("cec vol unmute key\n");
							HdmiSpdifLockFlag = 0;//消除unmute时的pop音
						}
						else//false-->true
						{
							HDMISourceMute();//消除unmute时的pop音
							HDMI_CEC_SystemAudioModeStatus((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) | 0x80);
							gHdmiCt->hdmi_audiomute_flag = 1;
							gHdmiCt->hdmiActiveReportMuteflag = 2;
							gHdmiCt->hdmiActiveReportMuteStatus = 1;
							APP_DBG("cec vol mute key\n");
						}

					}
					flag = 1;
					break;
				case 0x8f:
					HDMI_CEC_SystemPowerOn();
					APP_DBG("cec HDMI_CEC_SystemPowerOn\n");
					flag = 1;
					break;
				case 0x9f:
					HDMI_CEC_Version();
					flag = 1;
					break;

				case 0xA4:
#if 1
					if((operValue[1] & 0xf) == 1)
					{
						HDMI_CEC_AudioFormatCode();
					}
					else
					{
						uint8_t buf[3] = {0x16, 0x7F, 0x00};//第三个参数待定
						buf[0] = ((operValue[1] & 0xf)<<3) + 6;
						HDMI_CEC_UserDefined(0x50, 0xA3, buf, 3);
					}
#else
					if((operValue[1] & 0xf) != 1)
						HDMI_CEC_FeatureAbort(0xA4, 0x3);
					else
						HDMI_CEC_AudioFormatCode();
#endif
					flag = 1;
					break;

				case 0x00:
					if(operValue[1] == 0xC0 || operValue[1] == 0x72)
					{
						gHdmiCt->hdmiReportStatus = HDMI_ALLOCATE_ADDR;
						gHdmiCt->hdmiReTransCnt = 0;
						TimeOutSet(&gHdmiCt->time_reaction, 2000);
					}
					break;

				default:
					break;
			}
		}

		if(flag == 0)
		{
			if(IsTimeOut(&gHdmiCt->time_reaction))
			{
			if(gHdmiCt->hdmiReportStatus == HDMI_ALLOCATE_ADDR)//allocate逻辑地址
			{
				HDMI_CEC_AllocateLogicalAddress();
				if(++gHdmiCt->hdmiReTransCnt >= 2)//重传次数
				{
					gHdmiCt->hdmiReTransCnt = 0;
					gHdmiCt->hdmiReportStatus = HDMI_REPORT_PHYSICAL_ADDR;
				}
			}
			else if(gHdmiCt->hdmiReportStatus == HDMI_REPORT_PHYSICAL_ADDR)//物理地址
			{
				HDMI_CEC_ReportPhysicalAddress();
				if(++gHdmiCt->hdmiReTransCnt >= 2)
				{
					gHdmiCt->hdmiReTransCnt = 0;
					gHdmiCt->hdmiReportStatus = HDMI_REQUEST_ACTIVE_SRC;
					TimeOutSet(&gHdmiCt->time_waitTime, 1);
				}
			}
			else if(gHdmiCt->hdmiReportStatus == HDMI_REQUEST_ACTIVE_SRC)//request source
			{
				if(IsTimeOut(&gHdmiCt->time_waitTime))
				{
					TimeOutSet(&gHdmiCt->time_waitTime, 400);
					if(++gHdmiCt->hdmiReTransCnt >= 3)
					{
						if(gHdmiCt->hdmi_poweron_flag == 1)
						{
							gHdmiCt->hdmiReTransCnt = 0;
							gHdmiCt->hdmiReportStatus = HDMI_SET_AUDIO_MODE;
						}
						else
						{
							gHdmiCt->hdmiReTransCnt = 0;
							gHdmiCt->hdmiReportStatus = HDMI_ALLOCATE_ADDR;
							TimeOutSet(&gHdmiCt->time_reaction, gHdmiCt->hdmi_tv_inf.tv_reaction_time);
						}

					}
					else
						HDMI_CEC_RequestActiveSource();

				}
			}
			else if(gHdmiCt->hdmiReportStatus == HDMI_SET_AUDIO_MODE)//设置audio on
			{
				HDMI_CEC_SystemAudioModeStatusOn();
				if(++gHdmiCt->hdmiReTransCnt >= 2)
				{
					gHdmiCt->hdmiReTransCnt = 0;
					gHdmiCt->hdmiReportStatus = HDMI_INIT_STATUS;
					if(gHdmiCt->hdmi_arc_flag == 0)
					{
						gHdmiCt->hdmiReportARCStatus = 1;
					}
				}
			}
			else if(gHdmiCt->hdmiReportARCStatus == 1)
			{
				HDMI_CEC_InitiateARC();
				if(++gHdmiCt->hdmiReTransCnt >= 2)
				{
					gHdmiCt->hdmiReTransCnt = 0;
					gHdmiCt->hdmiReportARCStatus = 0;
				}
			}
			else if(gHdmiCt->hdmiActiveReportVolUpDownflag > 0)//由于该消息没有回应信息，增加重发机制保证成功率
			{
				gHdmiCt->hdmi_audiomute_flag = 0;
				HDMI_CEC_SystemAudioModeStatus((uint8_t)(mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) & 0x7f);
				gHdmiCt->hdmiActiveReportVolUpDownflag--;
			}
			else if(gHdmiCt->hdmiActiveReportMuteflag > 0)//由于该消息没有回应信息，增加重发机制保证成功率
			{
				if(gHdmiCt->hdmiActiveReportMuteStatus == 1)
				{
					HDMISourceMute();//消除unmute时的pop音
					gHdmiCt->hdmi_audiomute_flag = 1;
					HDMI_CEC_SystemAudioModeStatus((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) | 0x80);
				}
				else
				{
					HDMI_CEC_SystemAudioModeStatus((mainAppCt.gSysVol.AudioSourceVol[APP_SOURCE_NUM] * 100 / CFG_PARA_MAX_VOLUME_NUM) & 0x7f);
					HDMISourceUnmute();
					gHdmiCt->hdmi_audiomute_flag = 0;
				}
				gHdmiCt->hdmiActiveReportMuteflag--;
			}
			}
		}

		if(mainAppCt.hdmiResetFlg == 2)//电视兼容性问题：解决小米电视快速插入偶发性无声问题
		{
			mainAppCt.hdmiResetFlg = 0;
			gHdmiCt->hdmi_poweron_flag = 0;
			gHdmiCt->hdmi_arc_flag = 0;
			gHdmiCt->hdmiReportARCStatus = 0;
			gHdmiCt->hdmi_audiomute_flag = 0;
			gHdmiCt->hdmiActiveReportVolUpDownflag = 0;
			gHdmiCt->hdmiActiveReportMuteStatus = 0;
			gHdmiCt->hdmiActiveReportMuteflag = 0;
			gHdmiCt->hdmiActiveReportMuteAfterInit = 1;
			HDMI_DDC_GetInfo();
			TimeOutSet(&gHdmiCt->time_reaction, gHdmiCt->hdmi_tv_inf.tv_reaction_time);
			gHdmiCt->hdmiReportStatus = 1;
			gHdmiCt->hdmiReTransCnt = 0;
		}
	}
	else if(HDMI_CEC_IsWorking() == CEC_IS_WORKING)
	{
		gHdmiCt->hdmiBusyTime = 1000;
	}
}

#endif  //CFG_APP_HDMIIN_MODE_EN



