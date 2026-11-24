#ifndef _BT_SOURCE_SWITCH_H_
#define _BT_SOURCE_SWITCH_H_

#include <stdbool.h>
#include <stdint.h>
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

// 错误码定义
typedef enum {
    BT_SUCCESS = 0,
    BT_ERROR_INVALID_PARAM = -1,
    BT_ERROR_NOT_INITIALIZED = -2,
    BT_ERROR_CONNECTION_NOT_FOUND = -3,
    BT_ERROR_OPERATION_FAILED = -4,
    BT_ERROR_NO_ACTIVE_CONNECTION = -5
} BT_ErrorCode;

// 连接索引定义
typedef enum {
    BT_CONNECTION_INDEX_NONE = 0,
    BT_CONNECTION_INDEX_1 = 1,
    BT_CONNECTION_INDEX_2 = 2,
    BT_CONNECTION_INDEX_ALL = 3    // 表示同时发送到两个连接
} BT_ConnectionIndex;

// 音频路由模式
typedef enum {
    BT_ROUTE_MODE_UNICAST = 0,     // 单播：发送到一个Sink
    BT_ROUTE_MODE_MULTICAST = 1,   // 组播：同时发送到两个Sink
} BT_RouteMode;

// 连接状态定义
typedef enum {
    BT_STATE_DISCONNECTED = 0,
    BT_STATE_CONNECTING,
    BT_STATE_CONNECTED,
    BT_STATE_STREAMING,
    BT_STATE_PAUSED
} BT_ConnectionState;

// 回调函数类型定义
typedef void (*BT_StateCallback)(uint8_t index, BT_ConnectionState state);
typedef void (*BT_RouteCallback)(BT_RouteMode mode, uint8_t active_index);


typedef struct {
    uint8_t device_addr[6];        // 设备地址
    BT_ConnectionState state;   // 连接状态
    bool is_occupied;          // 是否被占用
    bool is_streaming;         // 是否正在流媒体
    bool in_multicast_group;   // 是否在组播组中
} BT_Connection;

typedef struct {
    bool initialized;
    BT_Connection connections[2];  // 索引0->连接1, 索引1->连接2
    
    // 音频路由状态
    BT_RouteMode route_mode;
    uint8_t active_index;          // 单播模式下的活动连接
    
    // 回调函数
    BT_StateCallback state_callback;
    BT_RouteCallback route_callback;
} BT_SourceContext;

// ==================== 初始化/反初始化 ====================
BT_ErrorCode BT_Source_Init(void);
BT_ErrorCode BT_Source_Deinit(void);

// ==================== 核心切换API ====================
// 切换到指定Index的连接（单播模式）
BT_ErrorCode BT_Source_SwitchTo(uint8_t index);

// 在连接1和连接2之间切换（单播模式）
BT_ErrorCode BT_Source_Toggle(void);

// 启用组播模式（同时发送到两个Sink）
BT_ErrorCode BT_Source_EnableMulticast(void);

// 禁用组播模式
BT_ErrorCode BT_Source_DisableMulticast(void);

// 获取当前音频路由模式
BT_RouteMode BT_Source_GetRouteMode(void);

// 获取当前活动连接（单播模式下有效）
uint8_t BT_Source_GetActiveIndex(void);

// 设置音频路由模式
BT_ErrorCode BT_Source_SetRouteMode(BT_RouteMode mode, uint8_t primary_index);

// ==================== 连接管理 ====================
BT_ErrorCode BT_Source_Connect(uint8_t index, const uint8_t* device_addr);
BT_ErrorCode BT_Source_Disconnect(uint8_t index);
BT_ErrorCode BT_Source_DisconnectAll(void);

// ==================== 状态查询 ====================
bool BT_Source_IsConnected(uint8_t index);
bool BT_Source_IsStreaming(uint8_t index);
BT_ConnectionState BT_Source_GetState(uint8_t index);
uint8_t BT_Source_GetConnectedCount(void);

// ==================== 音频控制 ====================
BT_ErrorCode BT_Source_StartStream(uint8_t index);
BT_ErrorCode BT_Source_StopStream(uint8_t index);

#if 0
BT_ErrorCode BT_Source_Pause(uint8_t index);
BT_ErrorCode BT_Source_Resume(uint8_t index);

// ==================== 回调注册 ====================
void BT_Source_RegisterStateCallback(BT_StateCallback callback);
void BT_Source_RegisterRouteCallback(BT_RouteCallback callback);
#endif

#ifdef __cplusplus
}
#endif

#endif // BT_SOURCE_SWITCH_H