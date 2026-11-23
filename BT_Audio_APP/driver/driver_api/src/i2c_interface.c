/**
 **************************************************************************************
 * @file    i2c_interface.c
 * @brief   i2c interface
 *
 * @author  KK
 * @version V1.0.0
 *
 * $Created: 2016-10-28 19:15:00$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
#include "type.h"
//#include "app_config.h"
#include "uarts.h"
#include "gpio.h"
#include "timeout.h"
#include "i2c_interface.h"
#include "delay.h"

#define I2C_ERR_RECOVER_TIME 	10//出现锁死超时恢复时间

typedef struct _I2C_CFG
{
	uint8_t I2cClkParam;
	I2C_PORT_MODE port;
	uint8_t OwnAddr;
}I2C_CFG;
I2C_CFG I2C_Cfg;

/**
 * @brief     I2C初始化配置
 *
 * @param[in] I2cClkParam 分频参数设置(范围0x00-0x3f)，详见 # i2c div table
 * @param[in] port      i2c使用端口选择，详见 # I2C_PORT_MODE
 * @param[in] OwnAddr   i2c做slave，需要配置自身地址
 *                      i2c做master，可随意配置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_Config(uint8_t I2cClkParam, I2C_PORT_MODE port, uint8_t OwnAddr)
{

	I2C_Cfg.I2cClkParam = I2cClkParam;
	I2C_Cfg.port = port;
	I2C_Cfg.OwnAddr = OwnAddr;
	I2C_Init(I2C_Cfg.I2cClkParam, I2C_Cfg.port, I2C_Cfg.OwnAddr);
}

void I2C_Clock_Recovery(I2C_PORT_MODE port) {

	uint32_t i2c_sda_gpio_index = 0;
	uint32_t i2c_scl_gpio_index = 0;
	uint32_t i2c_gpiox = 0;

	switch(port)
	{
		case I2C_PORT_B4_B5:
		i2c_sda_gpio_index = GPIOB4;
		i2c_scl_gpio_index = GPIOB5;
		i2c_gpiox = GPIO_B_START;
		break;
		case I2C_PORT_A5_A6:
		i2c_sda_gpio_index = GPIOA5;
		i2c_scl_gpio_index = GPIOA6;
		i2c_gpiox = GPIO_A_START;
		break;
		case I2C_PORT_A28_A29:
		i2c_sda_gpio_index = GPIOA28;
		i2c_scl_gpio_index = GPIOA29;
		i2c_gpiox = GPIO_A_START;
		break;
		case I2C_PORT_A30_A31:
		i2c_sda_gpio_index = GPIOA30;
		i2c_scl_gpio_index = GPIOA31;
		i2c_gpiox = GPIO_A_START;
		break;
	}

	if(i2c_gpiox == GPIO_B_START)
	{
		GPIO_PortBModeSet(i2c_sda_gpio_index,0);
		GPIO_PortBModeSet(i2c_scl_gpio_index,0);
	}
	else
	{
		GPIO_PortAModeSet(i2c_sda_gpio_index,0);
		GPIO_PortAModeSet(i2c_scl_gpio_index,0);
	}

	GPIO_RegOneBitSet((i2c_gpiox+0x05), i2c_sda_gpio_index);
	GPIO_RegOneBitClear((i2c_gpiox+0x06), i2c_sda_gpio_index);

	GPIO_RegOneBitSet((i2c_gpiox+0x06), i2c_scl_gpio_index);
	GPIO_RegOneBitClear((i2c_gpiox+0x05), i2c_scl_gpio_index);

	uint8_t i = 0;
	#if 0
	 for (i = 0; i < 16; i++) {	//16脉冲恢复
        GPIO_RegOneBitClear((i2c_gpiox+0x01), i2c_scl_gpio_index);
        DelayUs(5);

        // 在关键脉冲检查SDA
        if (i == 8 || i == 15) {
            if (GPIO_RegOneBitGet(GPIO_B_IN, GPIO_INDEX4)) {
                printf("SDA在脉冲%d后释放\n", i+1);
                break; // 提前结束
            }
        }
        GPIO_RegOneBitSet((i2c_gpiox+0x01), i2c_scl_gpio_index);
        DelayUs(5);
    }
	#else
	for (i = 0; i < 9; i++) {
        GPIO_RegOneBitClear((i2c_gpiox+0x01), i2c_scl_gpio_index);
        DelayUs(5);
        GPIO_RegOneBitSet((i2c_gpiox+0x01), i2c_scl_gpio_index);
        DelayUs(5);
    }
	#endif
}

void I2C_MasterReset(void)
{
	I2C_Disable();

	I2C_Clock_Recovery(I2C_Cfg.port);//9脉冲恢复

	I2C_Init(I2C_Cfg.I2cClkParam, I2C_Cfg.port, I2C_Cfg.OwnAddr);	//重新初始化
	I2C_SendStop();
}

I2C_ErrorState I2C_WaitIntFlag(void)
{
	TIMER I2CTimer;

	TimeOutSet(&I2CTimer, 200);//等待中断标志200ms超时
	while(!I2C_GetIntFlag())
	{
		if(IsTimeOut(&I2CTimer))
		{
			return ERROR_TIMEOUT;
		}
	}

	return ERROR_OK;
}


/**
 * @brief     I2C主模式下发送数据
 *
 * @param[in] SlaveAddr 从设备地址
 * @param[in] SendBuf   发送数据缓存
 * @param[in] BufLen    数据长度
 * @param[in] timeout   超时时间设置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_MasterSendBuffer(uint8_t SlaveAddr, void* SendBuf, uint32_t BufLen, uint32_t timeout)
{
	TIMER I2CTimer;
	uint32_t i;
	I2C_ErrorState ret = ERROR_OK;
	
	TimeOutSet(&I2CTimer, timeout);
	while(I2C_IsBusy())
	{
		if(IsTimeOut(&I2CTimer))
		{
			ret = ERROR_BUSY;
			goto i2c_send_buff_err;
			//return ERROR_BUSY;
		}
	}

	I2C_MasterConfig();

	//send slave address
	I2C_SendByte(SlaveAddr<<1);
	if(ERROR_TIMEOUT == I2C_WaitIntFlag())
	{
		ret = ERROR_TIMEOUT;
		goto i2c_send_buff_err;
		//return ERROR_TIMEOUT;
	}

	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		ret = ERROR_NOACK;
		goto i2c_send_buff_err;
		//return ERROR_NOACK;
	}
	
	//send data
	for(i=0; i<BufLen; i++)
	{
		I2C_SendByte(((uint8_t *)SendBuf)[i]);
		if(ERROR_TIMEOUT == I2C_WaitIntFlag())
		{
			ret = ERROR_TIMEOUT;
			goto i2c_send_buff_err;
			//return ERROR_TIMEOUT;
		}
		I2C_IntClr();
		if(I2C_ReceiveAcknowledge())
		{
			I2C_SendStop();
			ret = ERROR_NOACK;
			goto i2c_send_buff_err;
			//return ERROR_NOACK;
		}
	}
	I2C_SendStop();

	return ret;

i2c_send_buff_err:
	TimeOutSet(&I2CTimer, I2C_ERR_RECOVER_TIME);
	while(I2C_IsBusy())
	{
		if(IsTimeOut(&I2CTimer) || ret == ERROR_BUSY)
		{
			I2C_MasterReset();
			return ERROR_BUSY;
		}
	}
	return ret;
}

/**
 * @brief     I2C主模式下发送数据
 *
 * @param[in] SlaveAddr 从设备地址
 * @param[in] RegAddr   从设备寄存器地址
 * @param[in] SendBuf   发送数据缓存
 * @param[in] BufLen    数据长度
 * @param[in] timeout   超时时间设置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_MasterSendData(uint8_t SlaveAddr, uint8_t RegAddr, void* SendBuf, uint32_t BufLen, uint32_t timeout)
{
	TIMER I2CTimer;
	uint32_t i;
	I2C_ErrorState ret = ERROR_OK;

	TimeOutSet(&I2CTimer, timeout);
	while(I2C_IsBusy())
	{
		if(IsTimeOut(&I2CTimer))
		{
			ret = ERROR_BUSY;
			goto i2c_send_err;
			//return ERROR_BUSY;
		}
	}

	I2C_MasterConfig();

	//send slave address
	I2C_SendByte(SlaveAddr<<1);
	if(ERROR_TIMEOUT == I2C_WaitIntFlag())
	{
		ret = ERROR_TIMEOUT;
		goto i2c_send_err;
		//return ERROR_TIMEOUT;
	}
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();

		ret = ERROR_NOACK;
		goto i2c_send_err;
		//return ERROR_NOACK;
	}
	
	//send register address
	I2C_SendByte(RegAddr);
	if(ERROR_TIMEOUT == I2C_WaitIntFlag())
	{
		ret = ERROR_TIMEOUT;
		goto i2c_send_err;
		//return ERROR_TIMEOUT;
	}
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		ret = ERROR_NOACK;
		goto i2c_send_err;
		//return ERROR_NOACK;
	}

	//send data
	for(i=0; i<BufLen; i++)
	{
		I2C_SendByte(((uint8_t *)SendBuf)[i]);
		if(ERROR_TIMEOUT == I2C_WaitIntFlag())
		{
			ret = ERROR_TIMEOUT;
			goto i2c_send_err;
			//return ERROR_TIMEOUT;
		}
		I2C_IntClr();
		if(I2C_ReceiveAcknowledge())
		{
			I2C_SendStop();
			ret = ERROR_NOACK;
			goto i2c_send_err;
			//return ERROR_NOACK;
		}
	}
	I2C_SendStop();

	return ret;

i2c_send_err:
	TimeOutSet(&I2CTimer, I2C_ERR_RECOVER_TIME);
	while(I2C_IsBusy())
	{
		if(IsTimeOut(&I2CTimer) || ret == ERROR_BUSY)
		{
			I2C_MasterReset();
			return ERROR_BUSY;
		}
	}
	return ret;
}

/**
 * @brief     I2C主模式下接收数据
 *
 * @param[in] SlaveAddr 从设备地址
 * @param[in] SendBuf   发送数据缓存
 * @param[in] BufLen    数据长度
 * @param[in] timeout   超时时间设置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_MasterReceiveBuffer(uint8_t SlaveAddr, void* RecvBuf, uint32_t BufLen, uint32_t timeout)
{
	TIMER I2CTimer;
	uint32_t i;
	I2C_ErrorState ret = ERROR_OK;

	TimeOutSet(&I2CTimer, timeout);
	while(I2C_IsBusy())
	{
		if(IsTimeOut(&I2CTimer))
		{
			ret = ERROR_BUSY;
			goto i2c_recv_buff_err;
			//return ERROR_BUSY;
		}
	}

	I2C_MasterConfig();

	//send slave address(read)
	I2C_SendByte((SlaveAddr<<1) | I2C_READ);
	if(ERROR_TIMEOUT == I2C_WaitIntFlag())
	{
		ret = ERROR_TIMEOUT;
		goto i2c_recv_buff_err;
		//return ERROR_TIMEOUT;
	}
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		ret = ERROR_NOACK;
		goto i2c_recv_buff_err;
		//return ERROR_NOACK;
	}
	
	I2C_SendAck();
	I2C_ReceiveStart();

	((uint8_t *)RecvBuf)[0] = I2C_ReceiveByte();//dummy read for trigger recv

	for(i=0; i<BufLen-1; i++)
	{
		I2C_SendAck();
		if(ERROR_TIMEOUT == I2C_WaitIntFlag())
		{
			ret = ERROR_TIMEOUT;
			goto i2c_recv_buff_err;
			//return ERROR_TIMEOUT;
		}
		I2C_IntClr();
		((uint8_t *)RecvBuf)[i] = I2C_ReceiveByte();
	}
	
	I2C_SendNoAck();
	if(ERROR_TIMEOUT == I2C_WaitIntFlag())
	{
		ret = ERROR_TIMEOUT;
		goto i2c_recv_buff_err;
		//return ERROR_TIMEOUT;
	}
	I2C_IntClr();
	I2C_SendStop();
	DelayUs(2);
	((uint8_t *)RecvBuf)[i] = I2C_ReceiveByte();

	return ret;

i2c_recv_buff_err:
	TimeOutSet(&I2CTimer, I2C_ERR_RECOVER_TIME);
	while(I2C_IsBusy())
	{
		if(IsTimeOut(&I2CTimer) || ret == ERROR_BUSY)
		{
			I2C_MasterReset();
			return ERROR_BUSY;
		}
	}
	return ret;
}

/**
 * @brief     I2C主模式下接收数据
 *
 * @param[in] SlaveAddr 从设备地址
 * @param[in] RegAddr   从设备寄存器地址
 * @param[in] SendBuf   发送数据缓存
 * @param[in] BufLen    数据长度
 * @param[in] timeout   超时时间设置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_MasterReceiveData(uint8_t SlaveAddr, uint8_t RegAddr, void* RecvBuf, uint32_t BufLen, uint32_t timeout)
{
	TIMER I2CTimer;
	uint32_t i;
	I2C_ErrorState ret = ERROR_OK;
	
	TimeOutSet(&I2CTimer, timeout);
	while(I2C_IsBusy())
	{
		if(IsTimeOut(&I2CTimer))
		{
			ret = ERROR_BUSY;
			goto i2c_recv_err;
			//return ERROR_BUSY;
		}
	}

	I2C_MasterConfig();

	//send slave address(write)
	I2C_SendByte(SlaveAddr<<1);
	if(ERROR_TIMEOUT == I2C_WaitIntFlag())
	{
		ret = ERROR_TIMEOUT;
		goto i2c_recv_err;
		//return ERROR_TIMEOUT;
	}
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		ret = ERROR_NOACK;
		goto i2c_recv_err;
		//return ERROR_NOACK;
	}
	
	//send register address
	I2C_SendByte(RegAddr);
	if(ERROR_TIMEOUT == I2C_WaitIntFlag())
	{
		ret = ERROR_TIMEOUT;
		goto i2c_recv_err;
		//return ERROR_TIMEOUT;
	}
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		ret = ERROR_NOACK;
		goto i2c_recv_err;
		//return ERROR_NOACK;
	}

	//send restart
	I2C_SendReStart();
	
	//send slave address(read)
	I2C_SendByte((SlaveAddr<<1) | I2C_READ);
	if(ERROR_TIMEOUT == I2C_WaitIntFlag())
	{
		ret = ERROR_TIMEOUT;
		goto i2c_recv_err;
		//return ERROR_TIMEOUT;
	}
	I2C_IntClr();
	if(I2C_ReceiveAcknowledge())
	{
		I2C_SendStop();
		ret = ERROR_NOACK;
		goto i2c_recv_err;
		//return ERROR_NOACK;
	}

	I2C_SendAck();
	I2C_ReceiveStart();
	((uint8_t *)RecvBuf)[0] = I2C_ReceiveByte();//dummy read for trigger recv

	for(i=0; i<BufLen-1; i++)
	{
		I2C_SendAck();
		if(ERROR_TIMEOUT == I2C_WaitIntFlag())
		{
			ret = ERROR_TIMEOUT;
			goto i2c_recv_err;
			//return ERROR_TIMEOUT;
		}
		I2C_IntClr();
		((uint8_t *)RecvBuf)[i] = I2C_ReceiveByte();
	}
	
	I2C_SendNoAck();
	if(ERROR_TIMEOUT == I2C_WaitIntFlag())
	{
		ret = ERROR_TIMEOUT;
		goto i2c_recv_err;
		//return ERROR_TIMEOUT;
	}
	I2C_IntClr();
	I2C_SendStop();
	DelayUs(2);
	((uint8_t *)RecvBuf)[i] = I2C_ReceiveByte();

	return ret;

i2c_recv_err:
	TimeOutSet(&I2CTimer, I2C_ERR_RECOVER_TIME);
	while(I2C_IsBusy())
	{
		if(IsTimeOut(&I2CTimer) || ret == ERROR_BUSY)
		{
			I2C_MasterReset();
			return ERROR_BUSY;
		}
	}
	return ret;
}

/**
 * @brief     I2C从模式下发送数据
 *
 * @param[in] SlaveAddr 从设备地址
 * @param[in] SendBuf   发送数据缓存
 * @param[in] BufLen    数据长度
 * @param[in] timeout   超时时间设置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_SlaveSendBuffer(uint8_t *SendBuf, uint32_t BufLen, uint32_t timeout)
{
	uint32_t i;
	TIMER I2CTimer;
	I2C_ErrorState ret;
	TimeOutSet(&I2CTimer, timeout);

	I2C_SlaveConfig();

	ret = I2C_SlaveRequestWrite();
	if(ret)
	{
		return ret;
	}
	
	I2C_SendStart();
	for(i = 0;i < BufLen; i++)
	{
		I2C_SendByte(SendBuf[i]);
		while(!I2C_GetIntFlag());
		I2C_IntClr();
		if(I2C_ReceiveAcknowledge())
		{
			break;
		}
	}

	I2C_SlaveReleaseI2C();
	while(I2C_IsBusy());

	return ERROR_OK;
}

/**
 * @brief     I2C从模式下接收数据
 *
 * @param[in] SlaveAddr 从设备地址
 * @param[in] SendBuf   发送数据缓存
 * @param[in] BufLen    数据长度
 * @param[in] timeout   超时时间设置
 *
 * @return    i2c state  详见 I2C_ErrorState
 */
I2C_ErrorState I2C_SlaveReceiveBuffer(uint8_t *RecvBuf, uint32_t BufLen, uint32_t timeout)
{
	uint32_t i;
	TIMER I2CTimer;
	I2C_ErrorState ret;
	TimeOutSet(&I2CTimer, timeout);	

	I2C_SlaveConfig();

	ret = I2C_SlaveRequestRead();
	if(ret)
	{
		return ret;
	}
	
	RecvBuf[0] = I2C_ReceiveByte();//dummy read for trigger recv
	for(i = 0; i < BufLen; i++)
	{
		while(!I2C_GetIntFlag());
		I2C_IntClr();
		RecvBuf[i] = I2C_ReceiveByte();
		if(!I2C_IsBusy())
		{	//end receive data
			break;
		}
	}
	return ERROR_OK;
}




