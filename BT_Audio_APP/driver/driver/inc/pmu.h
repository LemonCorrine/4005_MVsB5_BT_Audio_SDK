/**
 **************************************************************************************
 * @file    pmu.h
 * @brief	pmu module driver interface
 *
 * @author  tony
 * @version V1.0.7
 *
 * @Created: 201-4-16
 *
 * @Copyright (C) 2014, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
 /**
 * @addtogroup PMU
 * @{
 * @defgroup PMU pmu.h
 * @{
 */

#ifndef __PMU_H__
#define __PMU_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "type.h"

/* Typedef -------------------------------------------------------------------*/

typedef enum _OSC_CLK_SEL
{
	LOSC_ANA_MODE = 0,
	LOSC_DIG_MODE = 1
}OSC_CLK_SEL;

typedef enum _POWERUP_EVENT_SEL
{
	CHARGE_IN_OUT_EVENT = (0),
	GPIO_C0_EVENT 		= (1),
	POWERKEY_EVENT      = (2)
}POWERUP_EVENT_SEL;

typedef enum _POWERKEY_PULLUP_SEL
{
	SEL_330K = (0),
	SEL_22K  = (1),
}POWERKEY_PULLUP_SEL;

typedef enum _OSC_DIRECT_INPUT_SEL
{
	 OSC_USE = 0,
	 EXTERNAL_DIRECT_INPUT_USE = 1
}OSC_DIRECT_INPUT_SEL;

typedef enum _POWERKEY_SWITCH_MODE
{
	HARD_MODE = 0,
	SOFT_MODE = 1
}POWERKEY_SWITCH_MODE;

typedef enum _HARD_TRIGGER_MODE
{
	LEVEL_TRIGGER = 0,
	EDGE_TRIGGER = 1
}HARD_TRIGGER_MODE;

typedef enum _POWERKEY_ACTIVE_LEVEL
{
	LOW_INDICATE_POWERON = 0,
	HIGH_INDICATE_POWERON = 1
}POWERKEY_ACTIVE_LEVEL;

typedef enum _POWERKEY_LONGORSHORT_PRESS_SEL
{
	POWERKEY_LONG_PRESS_MODE = 0,
	POWERKEY_SHORT_PRESS_MODE = 1
}POWERKEY_LONGORSHORT_PRESS_SEL;

typedef enum _POWERKEY_INT_SEL
{
	POWERKEY_TRIGLE_INT_NONE  = 0,
	POWERKEY_TRIGLE_INT 	  = 1,
	POWERKEY_SHORT_TRIGLE_INT = 2,
	POWERKEY_LONG_TRIGLE_INT  = 3
}POWERKEY_INT_SEL;

typedef enum _POWERKEY_RES_TUNNING
{
	PULLUP_22K,
	PULLUP_330K
}POWERKEY_RES_TUNNING;

typedef enum _POWERKEY_VTH_TUNNING
{
	VTH_0V8,
	VTH_2V4
}POWERKEY_VTH_TUNNING;

/* Define --------------------------------------------------------------------*/
#define		PMU_GPIO_C0_REG_START						(0x40024168)

#define 	PMU_GPIO_C0_REG_O_OFF							(0x00)
#define 	PMU_GPIO_C0_REG_O_SET_OFF						(0x04)
#define 	PMU_GPIO_C0_REG_O_CLR_OFF						(0x08)
#define 	PMU_GPIO_C0_REG_O_TGL_OFF						(0x0C)

#define 	PMU_GPIO_C0_REG_IE_OFF							(0x10)
#define 	PMU_GPIO_C0_REG_OE_OFF							(0x11)
#define 	PMU_GPIO_C0_REG_DS_OFF							(0x12)
#define 	PMU_GPIO_C0_REG_PU_OFF							(0x13)
#define 	PMU_GPIO_C0_REG_PD_OFF							(0x14)
#define 	PMU_GPIO_C0_REG_PILLDOWN_OFF					(0x15)

/* Variables -----------------------------------------------------------------*/


/* API -----------------------------------------------------------------------*/


//REG_PMU_ACCESS_PROTECT
void PMU_WriteEnable(void);
void PMU_WriteDisable(void);

void PMU_SystemPowerDown(void);

void PMU_AutoPowerOnEnable(void);
void PMU_AutoPowerOnDisable(void);
bool PMU_IsAutoPowerOnEnable(void);

void PMU_DisablePolarityCtrlSet(uint8_t set);
uint8_t PMU_DisablePolarityCtrlGet(void);

bool PMU_PowerDownFailHistory(void);
void PMU_PowerDownFailHistoryClear(void);

void PMU_ResetCoreEnable(void);//for test;
void PMU_ResetCoreDisable(void);
bool PMU_ResetCoreGet(void);

void PMU_PuPwDisable(void);
bool PMU_PuPwGet(void);

//TODO ldo or dcdc
void PMU_LDO12OffEnable(void);
void PMU_LDO12OffDisable(void);
bool PMU_IsLDO12OffEnable(void);

//TODO	??
//SREG_PMU_CHARGE_CTRL0.SW_PD_CHARGE_PD_ST  /**< 1:powerdown charge function in POWERDOWN state  */
uint8_t PMU_IsLDO12OffEnable1(void);

//TODO	??
//SREG_PMU_CHARGE_CTRL0.SW_PD_CHARGE_CW_ST	/**< 1:powerdown charge function in COREWORK state  */
uint8_t PMU_IsLDO12OffEnable2(void);

uint8_t BACKUP_IsChargeFull(void);

uint8_t BACKUP_IsChargeDc5VOver3V8(void);

uint8_t BACKUP_IsChargeDc5VOver2V1(void);

uint8_t BACKUP_IsChargeDc5VOver1V2(void);

uint8_t BACKUP_IsTrickleMode(void);

void BACKUP_SetSoftwareCtrlCharge(uint8_t set);
uint8_t BACKUP_IsSoftwareCtrlCharge(void);

void BACKUP_GotoChargeMode(void);

void BACKUP_SetHardwareCtrlCharge(uint8_t set);
uint8_t BACKUP_IsHardwareCtrlCharge(void);

void BACKUP_ChargeFullFilterCntSet(uint8_t set);
uint8_t BACKUP_ChargeFullFilterCntGet(void);

void BACKUP_TrickleModeFilterCntSet(uint8_t set);
uint8_t BACKUP_TrickleModeFilterCntGet(void);

void BACKUP_Pwr3V8FilterCntSet(uint8_t set);
uint8_t BACKUP_Pwr3V8FilterCntGet(void);
//TODO ??
//SREG_PMU_CHARGE_CTRL5.CHARGE_FULL_TIMEOUT_MAX_CNT  /**< define the max time  of charge full,the time is 34.95min*charge_full_timeout_max  */
void BACKUP_ChargeFullTimeoutCntSet(uint8_t set);
uint8_t BACKUP_ChargeFullTimeoutCntGet(void);

bool BACKUP_IsChargeFullTimeout(void);

void BACKUP_ChargeFullTimeoutClear(void);

void BACKUP_TrickleModeTimeoutCntSet(uint8_t set);
uint8_t BACKUP_TrickleModeTimeoutCntGet(void);

bool BACKUP_IsTrickleModeTimeout(void);

void BACKUP_TrickleModeTimeoutClear(void);

void 	PMU_Dc5vPosedge3v8StartupEnable(void);
void 	PMU_Dc5vPosedge3v8StartupDisable(void);
uint8_t PMU_Dc5vPosedge3v8StartupGet(void);

void 	PMU_Dc5vPosedge2v1StartupEnable(void);
void 	PMU_Dc5vPosedge2v1StartupDisable(void);
uint8_t PMU_Dc5vPosedge2v1StartupGet(void);

void PMU_Dc5vPosedge1v2StartupEnable(void);
void PMU_Dc5vPosedge1v2StartupDisable(void);
uint8_t PMU_Dc5vPosedge1v2StartupGet(void);

void PMU_Dc5vNegedge3v8StartupEnable(void);
void PMU_Dc5vNegedge3v8StartupDisable(void);
uint8_t PMU_Dc5vNegedge3v8StartupGet(void);

void PMU_Dc5vNegedge2v1StartupEnable(void);
void PMU_Dc5vNegedge2v1StartupDisable(void);
uint8_t PMU_Dc5vNegedge2v1StartupGet(void);

void PMU_Dc5vNegedge1v2StartupEnable(void);
void PMU_Dc5vNegedge1v2StartupDisable(void);
uint8_t PMU_Dc5vNegedge1v2StartupGet(void);

void PMU_Chargeon2v1FilterCntSet(uint8_t set);

uint8_t PMU_Chargeon2v1FilterCntGet(void);

void PMU_Chargeon1v2FilterCntSet(uint8_t set);
uint8_t PMU_Chargeon1v2FilterCntGet(void);

void PMU_ChargeResetByHardwareTimeSet(uint8_t set);
uint8_t PMU_ChargeResetByHardwareTimeGet(void);

void PMU_NVMInit(void);
bool PMU_NvmRead(uint8_t Nvmoffset, uint8_t* Buf, uint8_t Length);
bool PMU_NvmWrite(uint8_t Nvmoffset, uint8_t* Buf, uint8_t Length);

POWERUP_EVENT_SEL PMU_PowerupEventGet(void);
void PMU_PowerupEventClr(void);

void PMU_PowerkeySarADCEn(void);
void PMU_PowerkeySarADCDis(void);
uint8_t PMU_PowerkeySarADCEnGet(void);

void PMU_PowerkeyPullup330K(void);
void PMU_PowerkeyPullup22K(void);
POWERKEY_PULLUP_SEL PMU_PowerkeyPullupOhmGet(void);

void PMU_PowerKeyStateClear(void);

void PMU_PowerKeyShortPressStateClear(void);

void PMU_PowerKeyLongPressStateClear(void);

void PMU_PowerKeyModeSet(POWERKEY_SWITCH_MODE PowerKeyMode);
POWERKEY_SWITCH_MODE PMU_PowerKeyModeGet(void);

void PMU_PowerKeyHardModeSet(HARD_TRIGGER_MODE HardTriggerMode);
HARD_TRIGGER_MODE PMU_PowerKeyHardModeGet(void);

bool PMU_PowerkeyCoreWorkStateGet(void);

void PMU_PowerKeyActiveLevelSet(POWERKEY_ACTIVE_LEVEL PowerkeyActiveLevel);
POWERKEY_ACTIVE_LEVEL PMU_PowerKeyActiveLevelGet(void);

void PMU_PowerKeyLongOrShortPressSet(POWERKEY_LONGORSHORT_PRESS_SEL mode_set);
POWERKEY_LONGORSHORT_PRESS_SEL PMU_PowerKeyLongOrShortPressGet(void);

bool PMU_PowerKeyTrigStateGet(void);

bool PMU_PowerKeyShortPressTrigStateGet(void);

bool PMU_PowerKeyLongPressTrigStateGet(void);

bool PMU_PowerKeyPinStateGet(void);

void PMU_PowerKeyEnable(void);
void PMU_PowerKeyDisable(void);
bool PMU_PowerKeyEnGet(void);

void PMU_PowerKeyShortPressTrigMaxCntSet(uint8_t cnt);
uint8_t PMU_PowerKeyShortPressTrigMaxCntGet(void);

void PMU_PowerKeyLongPressTrigMaxCntSet(uint8_t cnt);
uint8_t PMU_PowerKeyLongPressTrigMaxCntGet(void);

void PMU_PowerKeyResetTrigMaxCntSet(uint8_t cnt);
uint8_t PMU_PowerKeyResetTrigMaxCntGet(void);

void PMU_PowerKeyNoiseFilterMaxCntSet(uint8_t cnt);
uint8_t PMU_PowerKeyNoiseFilterMaxCntGet(void);

void PMU_PowerKeyLongOrShortPressInterruptSet(POWERKEY_INT_SEL mode_set);
POWERKEY_INT_SEL PMU_PowerKeyLongOrShortPressInterruptGet(void);

/**
 * @brief  PMU restore config for wakeup
 * @param  void
 * @return void
 * @note   none
 */
void PMU_WakeupRestoreConfig();

/**
 * @brief  PMU config for deepsleep
 * @param  void
 * @return void
 * @note   none
 */
void PMU_DeepSleepConfig();


#ifdef __cplusplus
}
#endif

#endif

/**
 * @}
 * @}
 */
