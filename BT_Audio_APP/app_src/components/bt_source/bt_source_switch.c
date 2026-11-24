#include "bt_source_switch.h"
#include <string.h>
#include <stdio.h>
#include "type.h"
#include "bt_manager.h"
#if BT_SOURCE_SUPPORT
#define USER_CTX btManager.g_ctx

#define	BTSOURCE_SWITCH_DBG(format, ...)		printf(format, ##__VA_ARGS__)

// ==================== 内部辅助函数 ====================

static bool is_valid_index(uint8_t index) {
    return (index == BT_CONNECTION_INDEX_1 || 
            index == BT_CONNECTION_INDEX_2 || 
            index == BT_CONNECTION_INDEX_ALL);
}

static int index_to_array(uint8_t index) {
    return index - 1;  // 1->0, 2->1
}

static uint8_t array_to_index(int array_idx) {
    return array_idx + 1;  // 0->1, 1->2
}

static void notify_state_change(uint8_t index, BT_ConnectionState state) {
    if (USER_CTX.state_callback) {
        USER_CTX.state_callback(index, state);
    }
}

static void notify_route_change(BT_RouteMode mode, uint8_t active_index) {
    // if (USER_CTX.route_callback) {
    //     USER_CTX.route_callback(mode, active_index);
    // }
    if(mode == BT_ROUTE_MODE_MULTICAST)
    {
        if(BT_Source_StartStream(BT_CONNECTION_INDEX_ALL) == BT_SUCCESS)
        {
            BTSOURCE_SWITCH_DBG("# BT_ROUTE_MODE_MULTICAST OK\n");
        }
        else
        {
            BTSOURCE_SWITCH_DBG("# BT_ROUTE_MODE_MULTICAST Fail\n");
        }
    }
    else if(mode == BT_ROUTE_MODE_UNICAST)
    {
        if(active_index == 1)
            BT_Source_StopStream(2);
        else
            BT_Source_StopStream(1);

        if(BT_Source_StartStream(active_index) == BT_SUCCESS)
        {
            BTSOURCE_SWITCH_DBG("# BT_ROUTE_MODE_UNICAST %d OK\n",active_index-1);
        }
        else
        {
            BTSOURCE_SWITCH_DBG("# BT_ROUTE_MODE_UNICAST %d Fail\n",active_index-1);
        }
    }
}

static bool is_connection_available(int array_idx) {
    return (array_idx >= 0 && array_idx < 2 && 
            USER_CTX.connections[array_idx].is_occupied && 
            USER_CTX.connections[array_idx].state >= BT_STATE_CONNECTED);
}

// ==================== 初始化/反初始化 ====================
BT_ErrorCode BT_Source_Init(void) {
    if (USER_CTX.initialized) {
        return BT_SUCCESS;
    }

    memset(&USER_CTX.connections, 0, sizeof(USER_CTX.connections));
    USER_CTX.route_mode = BT_ROUTE_MODE_UNICAST;
    USER_CTX.active_index = 0;
    USER_CTX.initialized = true;

    BTSOURCE_SWITCH_DBG("BT Source Switch initialized\n");
    return BT_SUCCESS;
}

// BT_ErrorCode BT_Source_Deinit(void) {
//     if (!USER_CTX.initialized) {
//         return BT_ERROR_NOT_INITIALIZED;
//     }
    
//     // 断开所有连接
//     BT_Source_DisconnectAll();
    
//     USER_CTX.initialized = false;
//     USER_CTX.state_callback = NULL;
//     USER_CTX.route_callback = NULL;
    
//     printf("BT Source Switch deinitialized\n");
//     return BT_SUCCESS;
// }

// ==================== 核心切换API实现 ====================
BT_ErrorCode BT_Source_SwitchTo(uint8_t index) {

    //index = array_to_index(index);//外部index 0-1需要转换

    if (!USER_CTX.initialized) {
        return BT_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_index(index) || index == BT_CONNECTION_INDEX_ALL) {
        return BT_ERROR_INVALID_PARAM;
    }
    
    int array_idx = index_to_array(index);
    
    // 检查目标连接是否可用
    if (!is_connection_available(array_idx)) {
        return BT_ERROR_CONNECTION_NOT_FOUND;
    }
    
    // 如果已经是活动连接且是单播模式，直接返回
    if (USER_CTX.route_mode == BT_ROUTE_MODE_UNICAST && USER_CTX.active_index == index) {
        return BT_SUCCESS;
    }
    
    uint8_t old_active = USER_CTX.active_index;
    
    // 切换到单播模式
    USER_CTX.route_mode = BT_ROUTE_MODE_UNICAST;
    USER_CTX.active_index = index;
    
    // 更新组播组状态
    USER_CTX.connections[0].in_multicast_group = false;
    USER_CTX.connections[1].in_multicast_group = false;
    
    // 通知路由变化
    notify_route_change(USER_CTX.route_mode, USER_CTX.active_index);
    
    // 停止旧连接的流媒体（如果正在流媒体）
    if (old_active != 0 && old_active != index) {
        int old_array_idx = index_to_array(old_active);
        if (USER_CTX.connections[old_array_idx].is_streaming) {
            USER_CTX.connections[old_array_idx].is_streaming = false;
            notify_state_change(old_active, BT_STATE_CONNECTED);
        }
    }
    
    BTSOURCE_SWITCH_DBG("Switched to connection %d\n", index);
    return BT_SUCCESS;
}

BT_ErrorCode BT_Source_Toggle(void) {
    if (!USER_CTX.initialized) {
        return BT_ERROR_NOT_INITIALIZED;
    }
    
    // 确保当前是单播模式
    if (USER_CTX.route_mode != BT_ROUTE_MODE_UNICAST) {
        USER_CTX.route_mode = BT_ROUTE_MODE_UNICAST;
        notify_route_change(USER_CTX.route_mode, USER_CTX.active_index);
    }
    
    uint8_t new_index = BT_CONNECTION_INDEX_NONE;
    
    // 查找另一个可用的连接
    for (int i = 0; i < 2; i++) {
        BTSOURCE_SWITCH_DBG("index[%d] connect state = %d\n",i,is_connection_available(i));
        uint8_t candidate = array_to_index(i);
        if (candidate != USER_CTX.active_index && is_connection_available(i)) {
            new_index = candidate;
            break;
        }
    }
    
    // 如果没有其他可用连接，保持当前连接
    if (new_index == BT_CONNECTION_INDEX_NONE) {
        return BT_ERROR_NO_ACTIVE_CONNECTION;
    }
    
    // 切换到新连接
    return BT_Source_SwitchTo(new_index);
}

BT_ErrorCode BT_Source_EnableMulticast(void) {
    if (!USER_CTX.initialized) {
        return BT_ERROR_NOT_INITIALIZED;
    }
    
    // 检查是否有至少两个连接的设备
    uint8_t connected_count = 0;
    for (int i = 0; i < 2; i++) {
        if (is_connection_available(i)) {
            connected_count++;
        }
    }
    
    if (connected_count < 2) {
        return BT_ERROR_NO_ACTIVE_CONNECTION;
    }
    
    // 切换到组播模式
    USER_CTX.route_mode = BT_ROUTE_MODE_MULTICAST;
    
    // 将两个连接都加入组播组
    USER_CTX.connections[0].in_multicast_group = true;
    USER_CTX.connections[1].in_multicast_group = true;
    
    // 组播模式下没有单一的active_index
    USER_CTX.active_index = 0;
    
    // 通知路由变化
    notify_route_change(USER_CTX.route_mode, 0);
    
    BTSOURCE_SWITCH_DBG("Multicast enabled\n");
    return BT_SUCCESS;
}

BT_ErrorCode BT_Source_DisableMulticast(void) {
    if (!USER_CTX.initialized) {
        return BT_ERROR_NOT_INITIALIZED;
    }
    
    if (USER_CTX.route_mode != BT_ROUTE_MODE_MULTICAST) {
        return BT_SUCCESS;  // 如果不是组播模式，直接返回成功
    }
    
    // 切换回单播模式，使用第一个可用的连接
    USER_CTX.route_mode = BT_ROUTE_MODE_UNICAST;
    
    // 退出组播组
    USER_CTX.connections[0].in_multicast_group = false;
    USER_CTX.connections[1].in_multicast_group = false;
    
    // 选择第一个可用的连接作为活动连接
    for (int i = 0; i < 2; i++) {
        if (is_connection_available(i)) {
            USER_CTX.active_index = array_to_index(i);
            break;
        }
    }
    
    // 如果没有可用连接，清空active_index
    if (USER_CTX.active_index == 0) {
        USER_CTX.route_mode = BT_ROUTE_MODE_UNICAST;
    }
    
    notify_route_change(USER_CTX.route_mode, USER_CTX.active_index);
    
    BTSOURCE_SWITCH_DBG("Multicast disabled, switched to unicast mode\n");
    return BT_SUCCESS;
}

BT_RouteMode BT_Source_GetRouteMode(void) {
    return USER_CTX.route_mode;
}

uint8_t BT_Source_GetActiveIndex(void) {
    if (USER_CTX.route_mode != BT_ROUTE_MODE_UNICAST) {
        return 0;  // 非单播模式下没有单一的活动连接
    }
    return USER_CTX.active_index;
}

BT_ErrorCode BT_Source_SetRouteMode(BT_RouteMode mode, uint8_t primary_index) {
    if (!USER_CTX.initialized) {
        return BT_ERROR_NOT_INITIALIZED;
    }
    
    switch (mode) {
        case BT_ROUTE_MODE_UNICAST:
            if (primary_index == 0) {
                // 如果没有指定primary_index，使用当前活动连接
                primary_index = USER_CTX.active_index;
            }
            return BT_Source_SwitchTo(primary_index);
            
        case BT_ROUTE_MODE_MULTICAST:
            return BT_Source_EnableMulticast();

        default:
            return BT_ERROR_INVALID_PARAM;
    }
}

// ==================== 连接管理 ====================
BT_ErrorCode BT_Source_Connect(uint8_t index, const uint8_t* device_addr) {

    index = array_to_index(index);//外部index 0-1需要转换
    if (!USER_CTX.initialized || !device_addr) {
       //printf("#ERROR1 %d,%d\n",USER_CTX.initialized,device_addr);
        return BT_ERROR_INVALID_PARAM;
    }
    
    if (!is_valid_index(index) || index == BT_CONNECTION_INDEX_ALL) {
        //printf("#ERROR2 %d,%d\n",!is_valid_index(index),index);
        return BT_ERROR_INVALID_PARAM;
    }
    
    int array_idx = index_to_array(index);
    
    // 检查连接槽是否被占用
    if (USER_CTX.connections[array_idx].is_occupied) {
        // 先断开现有连接
        // BT_Source_Disconnect(index);
        //printf("#ERROR3 %d\n",USER_CTX.connections[array_idx].is_occupied);
        return BT_ERROR_INVALID_PARAM;
    }
    
    // 设置连接信息
    memcpy(USER_CTX.connections[array_idx].device_addr, device_addr, 6);
    USER_CTX.connections[array_idx].state = BT_STATE_CONNECTED;
    USER_CTX.connections[array_idx].is_occupied = true;
    USER_CTX.connections[array_idx].is_streaming = false;
    USER_CTX.connections[array_idx].in_multicast_group = false;
    
    notify_state_change(index, BT_STATE_CONNECTING);


    bt_reconnect_update(index, USER_CTX.connections[array_idx].device_addr, 1, 0);
    
    // 模拟连接过程
    // TODO: 这里调用实际的蓝牙连接API
    
    // 连接成功
    // USER_CTX.connections[array_idx].state = BT_STATE_CONNECTED;
    // notify_state_change(index, BT_STATE_CONNECTED);
    
    // 如果是第一个连接且没有活动连接，设为活动连接
    if (USER_CTX.route_mode == BT_ROUTE_MODE_UNICAST && USER_CTX.active_index == 0) {
        USER_CTX.active_index = index;
        notify_route_change(USER_CTX.route_mode, USER_CTX.active_index);
    }
    
    BTSOURCE_SWITCH_DBG("addr[0x%x][0x%x][0x%x][0x%x][0x%x][0x%x] connected to index %d\n", device_addr[0],device_addr[1],device_addr[2],
        device_addr[3],device_addr[4],device_addr[5], index);
    return BT_SUCCESS;
}

BT_ErrorCode BT_Source_Disconnect(uint8_t index) {

     index = array_to_index(index);//外部index 0-1需要转换

    if (!USER_CTX.initialized) {
        return BT_ERROR_NOT_INITIALIZED;
    }
    
    if (index == BT_CONNECTION_INDEX_ALL) {
        // 断开所有连接
        BT_Source_Disconnect(BT_CONNECTION_INDEX_1);
        BT_Source_Disconnect(BT_CONNECTION_INDEX_2);
        return BT_SUCCESS;
    }
    
    if (!is_valid_index(index) || index == BT_CONNECTION_INDEX_ALL) {
        return BT_ERROR_INVALID_PARAM;
    }
    
    int array_idx = index_to_array(index);
    
    if (!USER_CTX.connections[array_idx].is_occupied) {
        return BT_SUCCESS;  // 已经断开，返回成功
    }
    
    // 通知断开
    notify_state_change(index, BT_STATE_DISCONNECTED);
    
    bt_reconnect_update(index, USER_CTX.connections[array_idx].device_addr, 0, 0);
    
    // 停止流媒体
    if (USER_CTX.connections[array_idx].is_streaming) {
        USER_CTX.connections[array_idx].is_streaming = false;
    }
    
    // 退出组播组
    USER_CTX.connections[array_idx].in_multicast_group = false;
    
    // 清空连接信息
    USER_CTX.connections[array_idx].is_occupied = false;
    USER_CTX.connections[array_idx].state = BT_STATE_DISCONNECTED;
    memset(USER_CTX.connections[array_idx].device_addr, 0, 6);
    
    // 如果是活动连接，需要切换到另一个连接
    if (USER_CTX.route_mode == BT_ROUTE_MODE_UNICAST && USER_CTX.active_index == index) {
        USER_CTX.active_index = 0;
        
        // 查找另一个可用的连接
        for (int i = 0; i < 2; i++) {
            if (USER_CTX.connections[i].is_occupied && 
                USER_CTX.connections[i].state >= BT_STATE_CONNECTED) {
                USER_CTX.active_index = array_to_index(i);
                break;
            }
        }
        
        notify_route_change(USER_CTX.route_mode, USER_CTX.active_index);
    }
    
    BTSOURCE_SWITCH_DBG("Connection %d disconnected\n", index);
    return BT_SUCCESS;
}

BT_ErrorCode BT_Source_DisconnectAll(void) {
    BT_Source_Disconnect(BT_CONNECTION_INDEX_1);
    BT_Source_Disconnect(BT_CONNECTION_INDEX_2);
    return BT_SUCCESS;
}

// ==================== 状态查询 ====================
bool BT_Source_IsConnected(uint8_t index) {
     index = array_to_index(index);//外部index 0-1需要转换
    if (!USER_CTX.initialized || !is_valid_index(index) || index == BT_CONNECTION_INDEX_ALL) {
        return false;
    }
    
    int array_idx = index_to_array(index);
    return (USER_CTX.connections[array_idx].is_occupied && 
            USER_CTX.connections[array_idx].state >= BT_STATE_CONNECTED);
}

bool BT_Source_IsStreaming(uint8_t index) {

     index = array_to_index(index);//外部index 0-1需要转换
    if (!USER_CTX.initialized || !is_valid_index(index) || index == BT_CONNECTION_INDEX_ALL) {
        return false;
    }
    
    int array_idx = index_to_array(index);
    return USER_CTX.connections[array_idx].is_streaming;
}

BT_ConnectionState BT_Source_GetState(uint8_t index) {
     index = array_to_index(index);//外部index 0-1需要转换
    if (!USER_CTX.initialized || !is_valid_index(index) || index == BT_CONNECTION_INDEX_ALL) {
        return BT_STATE_DISCONNECTED;
    }
    
    int array_idx = index_to_array(index);
    return USER_CTX.connections[array_idx].state;
}

uint8_t BT_Source_GetConnectedCount(void) {
    if (!USER_CTX.initialized) {
        return 0;
    }
    
    uint8_t count = 0;
    for (int i = 0; i < 2; i++) {
        if (USER_CTX.connections[i].is_occupied && 
            USER_CTX.connections[i].state >= BT_STATE_CONNECTED) {
            count++;
        }
    }
    return count;
}

uint8_t Bt_Source_GetRouteMode(void)
{
     return USER_CTX.route_mode;
}

uint8_t Bt_Source_GetActiveIndex(void)
{
    if(Bt_Source_GetRouteMode() == BT_ROUTE_MODE_MULTICAST)
        return index_to_array(BT_CONNECTION_INDEX_1);

     return index_to_array(USER_CTX.active_index);
}

// ==================== 音频控制 ====================
BT_ErrorCode BT_Source_StartStream(uint8_t index) {
    if (!USER_CTX.initialized) {
        return BT_ERROR_NOT_INITIALIZED;
    }
    
    if (index == BT_CONNECTION_INDEX_ALL) {
        // 根据路由模式决定如何处理
        if (USER_CTX.route_mode == BT_ROUTE_MODE_MULTICAST) {
            // 组播模式：启动两个连接的流媒体
            BT_Source_StartStream(BT_CONNECTION_INDEX_1);
            BT_Source_StartStream(BT_CONNECTION_INDEX_2);
            return BT_SUCCESS;
        } else {
            return BT_ERROR_INVALID_PARAM;
        }
    }
    
    if (!is_valid_index(index) || index == BT_CONNECTION_INDEX_ALL) {
        return BT_ERROR_INVALID_PARAM;
    }
    
    int array_idx = index_to_array(index);
    
    if (!is_connection_available(array_idx)) {
        return BT_ERROR_CONNECTION_NOT_FOUND;
    }
    
    // 根据路由模式检查是否可以启动流媒体
    if (USER_CTX.route_mode == BT_ROUTE_MODE_UNICAST && USER_CTX.active_index != index) {
        // 单播模式下，只有活动连接可以流媒体
        return BT_ERROR_OPERATION_FAILED;
    }
    
    // 启动流媒体
    BtSourcePlay(array_idx);
    USER_CTX.connections[array_idx].state = BT_STATE_STREAMING;
    USER_CTX.connections[array_idx].is_streaming = true;
    
    notify_state_change(index, BT_STATE_STREAMING);
    
    BTSOURCE_SWITCH_DBG("Stream started on connection %d\n", index);
    return BT_SUCCESS;
}

BT_ErrorCode BT_Source_StopStream(uint8_t index) {
    if (!USER_CTX.initialized) {
        return BT_ERROR_NOT_INITIALIZED;
    }
    
    if (index == BT_CONNECTION_INDEX_ALL) {
        // 停止所有流媒体
        BT_Source_StopStream(BT_CONNECTION_INDEX_1);
        BT_Source_StopStream(BT_CONNECTION_INDEX_2);
        return BT_SUCCESS;
    }
    
    if (!is_valid_index(index) || index == BT_CONNECTION_INDEX_ALL) {
        return BT_ERROR_INVALID_PARAM;
    }
    
    int array_idx = index_to_array(index);
    
    if (!USER_CTX.connections[array_idx].is_streaming) {
        return BT_SUCCESS;  // 已经停止，返回成功
    }
    
    
    // 停止流媒体
    BtSourceStop(array_idx);
    USER_CTX.connections[array_idx].state = BT_STATE_CONNECTED;
    USER_CTX.connections[array_idx].is_streaming = false;
    
    notify_state_change(index, BT_STATE_CONNECTED);
    
    BTSOURCE_SWITCH_DBG("Stream stopped on connection %d\n", index);
    return BT_SUCCESS;
}

#if 0
BT_ErrorCode BT_Source_Pause(uint8_t index) {
    if (!USER_CTX.initialized) {
        return BT_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_index(index) || index == BT_CONNECTION_INDEX_ALL) {
        return BT_ERROR_INVALID_PARAM;
    }
    
    int array_idx = index_to_array(index);
    
    if (!USER_CTX.connections[array_idx].is_streaming) {
        return BT_SUCCESS;  // 没有在流媒体，直接返回成功
    }
    
    // 暂停
    USER_CTX.connections[array_idx].state = BT_STATE_PAUSED;
    
    notify_state_change(index, BT_STATE_PAUSED);
    
    printf("Stream paused on connection %d\n", index);
    return BT_SUCCESS;
}

BT_ErrorCode BT_Source_Resume(uint8_t index) {
    if (!USER_CTX.initialized) {
        return BT_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_index(index) || index == BT_CONNECTION_INDEX_ALL) {
        return BT_ERROR_INVALID_PARAM;
    }
    
    int array_idx = index_to_array(index);
    
    if (USER_CTX.connections[array_idx].state != BT_STATE_PAUSED) {
        return BT_SUCCESS;  // 没有暂停，直接返回成功
    }
    
    // 恢复
    USER_CTX.connections[array_idx].state = BT_STATE_STREAMING;
    
    notify_state_change(index, BT_STATE_STREAMING);
    
    printf("Stream resumed on connection %d\n", index);
    return BT_SUCCESS;
}

// ==================== 回调注册 ====================
void BT_Source_RegisterStateCallback(BT_StateCallback callback) {
    USER_CTX.state_callback = callback;
}

void BT_Source_RegisterRouteCallback(BT_RouteCallback callback) {
    USER_CTX.route_callback = callback;
}
#endif

#endif