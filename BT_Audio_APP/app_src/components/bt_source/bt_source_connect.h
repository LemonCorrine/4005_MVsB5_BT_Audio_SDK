#ifndef BT_SOURCE_CONNECT_H
#define BT_SOURCE_CONNECT_H

#include <stdbool.h>
#include <stdint.h>
#include "type.h"
#ifdef __cplusplus
extern "C" {
#endif

// ==================== API声明 ====================

/**
 * @brief 初始化回连模块
 */
void bt_reconnect_init(void);

/**
 * @brief 设置设备地址
 * @param index 设备索引 (1或2)
 * @param addr 设备MAC地址，NULL表示清除
 */
void bt_reconnect_set_dev(uint8_t index, uint8_t* addr);

/**
 * @brief 清除设备地址
 * @param index 设备索引 (1或2)
 */
void bt_reconnect_clear_dev(uint8_t index);

/**
 * @brief 设置回连模式
 * @param mode 0:关闭, 1:自动, 2:双设备, 3:仅Index1, 4:仅Index2
 */
void bt_reconnect_set_mode(uint8_t mode);

/**
 * @brief 设置最大重试次数
 * @param max_retry 最大重试次数 (1-10)
 */
void bt_reconnect_set_max_retry(uint8_t max_retry);

/**
 * @brief 手动启动回连
 */
void bt_reconnect_start(void);

/**
 * @brief 停止回连
 * @return 是否立刻停止 1：立刻停止，0：等待time out/连接成功停止
 */
uint8_t bt_reconnect_stop(void);

/**
 * @brief 强制回连指定索引的设备
 * @param index 设备索引 (1或2)
 */
void bt_reconnect_force_index(uint8_t index);

/**
 * @brief 更新设备连接状态
 * @param index 设备索引 (1或2)
 * @param connected true:已连接, false:已断开
 */
void bt_reconnect_update(uint8_t index, uint8_t *addr, bool connected, uint8_t ReconnectFlag);

/**
 * @brief 回连任务函数（需要在RTOS任务中周期调用）
 */
void bt_reconnect_task(void);

// ==================== 状态查询API ====================

/**
 * @brief 检查是否需要回连
 * @return true:需要回连, false:不需要
 */
bool bt_reconnect_is_needed(void);

/**
 * @brief 检查是否正在回连
 * @return true:正在回连, false:空闲
 */
bool bt_reconnect_is_active(void);

/**
 * @brief 获取当前状态
 * @return 状态码 (0:idle, 1:connecting, 2:waiting)
 */
uint8_t bt_reconnect_get_state(void);

/**
 * @brief 获取当前目标索引
 * @return 目标索引 (1或2，0表示无目标)
 */
uint8_t bt_reconnect_get_target(void);

/**
 * @brief 获取当前重试次数
 * @return 重试次数
 */
uint8_t bt_reconnect_get_retry_count(void);

#ifdef __cplusplus
}
#endif

#endif // BT_RECONNECT_MINI_H