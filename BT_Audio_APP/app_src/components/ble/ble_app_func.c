
#include "type.h"
#include <string.h>
#include "ble_api.h"
#include "ble_app_func.h"
//#include "bt_app_func.h"
#include "bt_manager.h"
#include "bt_config.h"

extern BT_CONFIGURATION_PARAMS		*btStackConfigParams;

#include "debug.h"
#if (BLE_SUPPORT == ENABLE)

extern void user_set_ble_bd_addr(uint8_t* local_addr);

#define BLE_DFLT_DEVICE_NAME			(sys_parameter.ble_LocalDeviceName)	//BLE����
#define BLE_DFLT_DEVICE_NAME_LEN		(strlen(BLE_DFLT_DEVICE_NAME))

/// default read perm
#define RD_P    (PROP(RD)  | SEC_LVL(RP, NOT_ENC))
/// default write without response perm
#define WC_P    (PROP(WC)  | SEC_LVL(WP, NOT_ENC))
/// default write perm
#define WR_P    (PROP(WR)  | SEC_LVL(WP, NOT_ENC))
/// default notify perm
#define NTF_P   (PROP(N)   | SEC_LVL(NIP, NOT_ENC))
/// ind perm
#define IND_P   (PROP(I)   | SEC_LVL(NIP, NOT_ENC))

//�û��Զ������
#define UUID16_SERVICE (0xae30)  // uuid
#define UDSS_IDX_NB 11           // att index
#define LE_VAL_MAX_LEN (247)
#define DEFAULT_MTU_SIZE (250)  // 250

static ble_gatt_att16_desc_t att16_db[UDSS_IDX_NB] = {
    //  ATT UUID
    //  | Permission			| EXT PERM | MAX ATT SIZE
    // User Data Service Service Declaration
    [0] = {GATT_DECL_PRIMARY_SERVICE, RD_P, 0},  // 0x2800

    // Characteristic1
    //  DataBase Index Increment Characteristic Declaration
    [1] = {GATT_DECL_CHARACTERISTIC, WR_P | NTF_P,
           0},  // 0x2803
                //  DataBase Index Increment Characteristic Value
    [2] = {0xae01, WC_P, LE_VAL_MAX_LEN},
    // Client Characteristic Configuration Descriptor
    //		[3] = {GATT_DESC_CLIENT_CHAR_CFG,				RD_P | WR_P,
    //OPT(NO_OFFSET) },//0x2902

    // Characteristic2
    //  DataBase Index Increment Characteristic Declaration
    [3] = {GATT_DECL_CHARACTERISTIC, NTF_P,
           0},  // 0x2803
                //  DataBase Index Increment Characteristic Value
    [4] = {0xae02, NTF_P, LE_VAL_MAX_LEN},
    // Client Characteristic Configuration Descriptor
    [5] = {GATT_DESC_CLIENT_CHAR_CFG, RD_P | WR_P, OPT(NO_OFFSET)},  // RW

    // Characteristic3
    //  DataBase Index Increment Characteristic Declaration
    [6] = {GATT_DECL_CHARACTERISTIC, NTF_P | WR_P,
           0},  // 0x2803
                //  DataBase Index Increment Characteristic Value
    [7] = {0xae05, IND_P, LE_VAL_MAX_LEN},
    //		// Client Characteristic Configuration Descriptor
    [8] = {GATT_DESC_CLIENT_CHAR_CFG, RD_P | WR_P, OPT(NO_OFFSET)},  // RW

    // Characteristic3
    //  DataBase Index Increment Characteristic Declaration
    [9] = {GATT_DECL_CHARACTERISTIC, WR_P,
           0},  // 0x2803
                //  DataBase Index Increment Characteristic Value
    [10] = {0xae10, WR_P, LE_VAL_MAX_LEN},

};

#define UDSS_IDX_NB_1 6
#define MV_CHAR_VAL_LEN_MAX 500
/* service_uuid and Characteristic_uuid*/
#define MV_service_uuid128                                            \
    {                                                                     \
        0x2F, 0x2A, 0x93, 0xA6, 0xBD, 0xD8, 0x41, 0x52, 0xAC, 0x0B, 0x10, \
            0x99, 0x2E, 0xC6, 0xFE, 0xED                                  \
    }
#define MV_device_info_uuid128                                        \
    {                                                                     \
        0xBA, 0x5C, 0x49, 0xD2, 0x04, 0xA3, 0x40, 0x71, 0xA0, 0xB5, 0x35, \
            0x85, 0x3E, 0xB0, 0x83, 0x07 \
	                              \
    }
#define MV_data_uuid128                                               \
    {                                                                     \
        0xe2, 0xa4, 0x1b, 0x54, 0x93, 0xe4, 0x6a, 0xb5, 0x20, 0x4e, 0xd0, \
            0x65, 0xe2, 0xff, 0x00, 0x00,                                 \
    }
#define MV_event_uuid                                                 \
    {                                                                     \
        0xB8, 0x5C, 0x49, 0xD2, 0x04, 0xA3, 0x40, 0x71, 0xA0, 0xB5, 0x35, \
            0x85, 0x3E, 0xB0, 0x83, 0x07,                                 \
    }
#define MV_ota_uuid                                                   \
    {                                                                     \
        0xe2, 0xa4, 0x1b, 0x54, 0x93, 0xe4, 0x6a, 0xb5, 0x20, 0x4e, 0xd0, \
            0x65, 0xe1, 0xff, 0x00, 0x00                                  \
    }

static const uint8_t MV_service[GATT_UUID_128_LEN] = MV_service_uuid128;
#define GATT_DECL_PRIMARY_SERVICE1 \
    { 0x00, 0x28 }
#define GATT_DECL_CHARACTERISTIC_128UUID \
    { 0x03, 0x28 }
#define GATT_DESC_CLIENT_CHAR_CFG_128UUID \
    { 0x02, 0x29 }
#define GATT_DESC_CHAR_USER_DESCRIPTION_128UUID \
    { 0x01, 0x29 }
// gatt_att_desc
const ble_gatt_att128_desc_t MV_att_db[] = {

    [0] = {GATT_DECL_PRIMARY_SERVICE1, RD_P, 0},  // 0x2800
    [1] = {GATT_DECL_CHARACTERISTIC_128UUID, PROP(RD), 0},
    [2] = {MV_device_info_uuid128, WR_P | ATT_UUID(128),
           MV_CHAR_VAL_LEN_MAX},
    //[3] = {GATT_DECL_CHARACTERISTIC_128UUID, PROP(RD), 0},
    //[4] = {MV_data_uuid128, WR_P | ATT_UUID(128), MV_CHAR_VAL_LEN_MAX},
    [3] = {GATT_DECL_CHARACTERISTIC_128UUID, PROP(RD), 0},
    [4] = {MV_event_uuid, NTF_P | ATT_UUID(128), MV_CHAR_VAL_LEN_MAX},
    [5] = {GATT_DESC_CLIENT_CHAR_CFG_128UUID, 0, 0},
   // [8] = {GATT_DECL_CHARACTERISTIC_128UUID, PROP(RD), 0},
   // [9] = {MV_ota_uuid, WC_P | ATT_UUID(128), MV_CHAR_VAL_LEN_MAX},
};

uint8_t ble_app_adv_data[30] = {
	//length + type + data
    // Flags general discoverable, BR/EDR not supported
    2, 0x01, 0x06,
    // Name
    9, 0x09, 'B','P','1','0','-','B','L','E',
};

const uint8_t ble_app_rsp_adv_data[30] = {
    0x03, 0xff, 0xff,0xff,
};

static uint8_t adv_len = sizeof(ble_app_adv_data);
static uint8_t rsp_adv_len = sizeof(ble_app_rsp_adv_data);

/***********************************************************************************
 * BLE��ʼ����������
 **********************************************************************************/
uint8_t LeInitConfigParams(void)
{

	printf("RD_P: 0x%04x,WR_P: 0x%04x,NTF_P: 0x%04x,IND_P: 0x%04x",RD_P,WR_P,NTF_P,IND_P);
    LeAppRegCB(AppEventCallBack); // ע��Ӧ�ò��¼��ص�����
    user_set_ble_bd_addr(btStackConfigParams->ble_LocalDeviceAddr);
    // BLE�㲥�����������
    le_user_config.ble_device_name_len = BLE_DFLT_DEVICE_NAME_LEN;
    memcpy(le_user_config.ble_device_name,BLE_DFLT_DEVICE_NAME,le_user_config.ble_device_name_len);
    ble_app_adv_data[3] = BLE_DFLT_DEVICE_NAME_LEN+1;
    memcpy(&ble_app_adv_data[5],le_user_config.ble_device_name,le_user_config.ble_device_name_len);

    le_user_config.adv_data.adv_data = (uint8_t *)ble_app_adv_data;
    le_user_config.adv_data.adv_len = adv_len;

    // �㲥�ظ�������Ҫʱ��д,����Ҫʱ��NULL
    le_user_config.rsp_data.adv_rsp_data = (uint8_t *)ble_app_rsp_adv_data;
    le_user_config.rsp_data.adv_rsp_len = rsp_adv_len;

    //���ù㲥������㲥ͨ��
    le_user_config.adv_interval_param.adv_intv_max = 0x0200;
    le_user_config.adv_interval_param.adv_intv_min = 0x0100;
    le_user_config.adv_interval_param.ch_map = 0x07; //37,38,39 ch map

    //��ʼ������
    le_user_config.ble_service_idxnb = UDSS_IDX_NB_1;
    le_user_config.profile_uuid128 = MV_att_db;
    le_user_config.ble_uuid128_service = MV_service;
    le_user_config.att_default_mtu = DEFAULT_MTU_SIZE;
    return 0;
}

/***********************************************************************************
 * BLE application initialize
 **********************************************************************************/
void BleAppInit(void)
{
    LeInitConfigParams(); //le parameters config
    rwble_enable_init();
}
#endif

