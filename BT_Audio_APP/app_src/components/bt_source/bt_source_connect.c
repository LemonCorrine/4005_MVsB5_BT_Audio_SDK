
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "type.h"
#include "bt_source_connect.h"
#include "bt_source.h"
#include "bt_config.h"
#if BT_SOURCE_SUPPORT
#define	BTSOURCE_RECONN_DBG(format, ...)		printf(format, ##__VA_ARGS__)

#define BT_RECONNECT_TIME_OUT   8000 //8s超时

static void handle_connecting_state(void);
static void handle_connect_delay_state(void);
static void handle_waiting_state(void);
static void select_next_target(void);
bool bt_reconnect_addr_valid(uint8_t *addr);

enum
{
    BT_RECONNECT_STATE_IDLE = 0,
    BT_RECONNECT_STATE_CONNECTING,
    BT_RECONNECT_STATE_CONNECT_DELAY,
    BT_RECONNECT_STATE_WAITING,
};

enum
{
    BT_RECONNECT_MODE_IDLE = 0,
    BT_RECONNECT_MODE_AUTO,
    BT_RECONNECT_MODE_BOTH,
    BT_RECONNECT_MODE_INDEX1,
    BT_RECONNECT_MODE_INDEX2,
};

typedef struct {
    // 设备信息
    uint8_t dev1_addr[6];
    uint8_t dev2_addr[6];
    
    // 状态
    bool dev1_connected;
    bool dev2_connected;
    
    // 回连控制
    uint8_t stop_flag;
    uint8_t mode;      // 0:off, 1:auto, 2:both, 3:index1, 4:index2
    uint8_t state;     // 0:idle, 1:connecting, 2:waiting
    uint8_t target;    // 1 or 2
    uint8_t retry;
    uint8_t max_retry;
    uint32_t wait_start; // 等待开始时间（tick）
    uint32_t conn_delay_start;
} bt_reconnect_t;

static bt_reconnect_t rc;

extern uint32_t gSysTick;
uint32_t get_system_tick(void)
{
    return gSysTick;
}

// ==================== API实现 ====================

void bt_reconnect_init(void) {
    memset(&rc, 0, sizeof(rc));
    #if BT_SRC_RECONNECT_NUM >= 2
    bt_reconnect_set_mode(BT_RECONNECT_MODE_AUTO);       // auto mode
    #else
    bt_reconnect_set_mode(BT_RECONNECT_MODE_INDEX1);    //回连单设备
    #endif

    rc.max_retry = BT_SRC_POR_TRY_COUNTS;   // 默认最大重试10次
    rc.state = BT_RECONNECT_STATE_IDLE;      // idle
}

void bt_reconnect_set_dev(uint8_t index, uint8_t* addr) {

    BTSOURCE_RECONN_DBG("bt_reconnect_set_dev index %d, addr: [0x%x][0x%x][0x%x][0x%x][0x%x][0x%x]\n",index, addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
    if (index == 1 && addr) {
        memcpy(rc.dev1_addr, addr, 6);
    } else if (index == 2 && addr) {
        memcpy(rc.dev2_addr, addr, 6);
    }
}

void bt_reconnect_clear_dev(uint8_t index) {
    if (index == 1) {
         memset(rc.dev1_addr, 0, 6);
        rc.dev1_connected = false;
    } else if (index == 2) {
        memset(rc.dev2_addr, 0, 6);
    }
}

void bt_reconnect_set_mode(uint8_t mode) {
    if (mode <= 4) {
        rc.mode = mode;
    }
}

void bt_reconnect_set_max_retry(uint8_t max_retry) {
    rc.max_retry = max_retry;
}

void bt_reconnect_start(void) {

    BTSOURCE_RECONN_DBG("#bt_reconnect_start\n");
    
    // 检查是否需要回连
    if (!bt_reconnect_is_needed()) {
        BTSOURCE_RECONN_DBG("#No Need Reconnect!!!\n");
        return;
    }

    if(rc.state == BT_RECONNECT_STATE_IDLE)//没有回连可以直接回连  有回连在需要time out后重置回连
        rc.state = BT_RECONNECT_STATE_CONNECTING;  // connecting
    
#if BT_SRC_RECONNECT_NUM >= 2
    bt_reconnect_set_mode(BT_RECONNECT_MODE_AUTO);       // auto mode
    #else
    bt_reconnect_set_mode(BT_RECONNECT_MODE_INDEX1);    //回连单设备
    #endif

    rc.stop_flag = 0;
    rc.target = 0;
    rc.retry = 0;
}

//停止回连API
uint8_t bt_reconnect_stop(void) {

    uint8_t ret = 0;
    if(rc.state != BT_RECONNECT_STATE_CONNECT_DELAY)//不能直接停 等time out 后停止
    {
        BTSOURCE_RECONN_DBG("ReConnect Stop!\n");
        rc.state = BT_RECONNECT_STATE_IDLE;

        ret = 1;
    }
    else
    {
         BTSOURCE_RECONN_DBG("ReConnect Delay Stop!\n");
        rc.stop_flag = 1;

        ret = 0;
    }

    rc.target = 0;
    rc.retry = 0;
    
    return ret;
}

//强制回连指定索引
void bt_reconnect_force_index(uint8_t index) {
    if (index != 1 && index != 2) return;
    
    if(rc.state == BT_RECONNECT_STATE_IDLE)//如没有回连存在直接回连，如有回连存在，等待time out后替换回连设备
    {
        rc.state = BT_RECONNECT_STATE_CONNECTING;      // connecting
    }
    rc.stop_flag = 0;
    rc.target = index;
    rc.retry = 0;

    if(rc.target == 2)
    bt_reconnect_set_mode(BT_RECONNECT_MODE_INDEX2);
    else
    bt_reconnect_set_mode(BT_RECONNECT_MODE_INDEX1);
   
}

void bt_reconnect_update(uint8_t index, uint8_t *addr, bool connected, uint8_t ReconnectFlag) {

    if(connected && addr)//校正index 地址与index匹配
    {
        if( (0 == memcmp(rc.dev1_addr,addr,6)) && (index == 2) )
        {
            memcpy(rc.dev1_addr,rc.dev2_addr,6);
        }
        else if( (0 == memcmp(rc.dev2_addr,addr,6)) && (index == 1) )
        {
            memcpy(rc.dev2_addr,rc.dev1_addr,6);
        }

            if(index == 1)
            {
                 rc.dev1_connected = connected;
                  memcpy(rc.dev1_addr,addr,6);
            }
            if(index == 2)
            {
                 rc.dev2_connected = connected;
                  memcpy(rc.dev2_addr,addr,6);
            }
    }
    else
    {
            if(index == 1)
            {
                 rc.dev1_connected = connected;
            }
            if(index == 2)
            {
                 rc.dev2_connected = connected;
            }
    }

    if(rc.stop_flag) //停止回连
    {
        rc.stop_flag = 0;
        rc.state = BT_RECONNECT_STATE_IDLE;
        BTSOURCE_RECONN_DBG("ReConnect Auto Stop!\n");
        return ;
    }

    if(!connected && !ReconnectFlag) return ;
    
    // 断开时自动触发（如果启用了auto模式）
    //if (!connected && rc.mode == BT_RECONNECT_MODE_AUTO && rc.state == BT_RECONNECT_STATE_IDLE) {
    if (!connected && rc.mode == BT_RECONNECT_MODE_AUTO) {
        // 检查这个设备是否配置了地址
        if ((index == 1 && bt_reconnect_addr_valid(rc.dev1_addr)) || 
            (index == 2 && bt_reconnect_addr_valid(rc.dev2_addr))) {
            //rc.state = BT_RECONNECT_STATE_CONNECTING;  // 开始回连
            rc.state = BT_RECONNECT_STATE_WAITING; // waiting  等待BT_SRC_POR_INTERNAL_TIME s再去回连
            rc.wait_start = get_system_tick();
            return ;
        }
    }
    
    // 如果正在连接这个设备并且连接成功，清除目标
    //if (connected && rc.target == index && (rc.state == BT_RECONNECT_STATE_CONNECTING || rc.state == BT_RECONNECT_STATE_CONNECT_DELAY)) {
    if (connected && rc.target == index) {
        rc.target = 0;
        rc.retry = 0;
        
        // 如果是BOTH模式且另一个设备也需要连接
        if (rc.mode == BT_RECONNECT_MODE_BOTH || rc.mode == BT_RECONNECT_MODE_AUTO) {
            if (index == 1 && !rc.dev2_connected && bt_reconnect_addr_valid(rc.dev2_addr)) {
                rc.target = 2;
                rc.state = BT_RECONNECT_STATE_WAITING; // waiting  等待BT_SRC_POR_INTERNAL_TIME s再去回连第二台设备
                rc.wait_start = get_system_tick();
            } else if (index == 2 && !rc.dev1_connected && bt_reconnect_addr_valid(rc.dev1_addr)) {
                rc.target = 1;
                rc.state = BT_RECONNECT_STATE_WAITING; // waiting  等待BT_SRC_POR_INTERNAL_TIME s再去回连第二台设备
                rc.wait_start = get_system_tick();
            } else {
                rc.state = BT_RECONNECT_STATE_IDLE; // 都连接完了
            }
        } else {
            rc.state = BT_RECONNECT_STATE_IDLE; // 单设备模式完成
        }
    }
}

uint8_t task_delay_cnt = 0;
void bt_reconnect_task(void) {
    if (rc.state == BT_RECONNECT_STATE_IDLE) return;  // idle

    task_delay_cnt++;
    if(task_delay_cnt >= 20)
    {
        task_delay_cnt = 0;
    }
    else
    {
        return;
    }
    // 状态机处理
    switch (rc.state) {
        case BT_RECONNECT_STATE_CONNECTING: // connecting
            handle_connecting_state();
            break;
            
        case BT_RECONNECT_STATE_WAITING: // waiting
            handle_waiting_state();
            break;

        case BT_RECONNECT_STATE_CONNECT_DELAY: // waiting
            handle_connect_delay_state();
            break;
    }
}

// ==================== 内部状态处理 ====================

static void handle_connecting_state(void) {
    // 选择目标
    if (rc.target == 0) {
        select_next_target();
        
        if (rc.target == 0) {
            rc.state = BT_RECONNECT_STATE_IDLE;  // 没有需要回连的
            return;
        }
    }
    
    // 尝试连接
    uint8_t* addr = rc.target == 1 ? rc.dev1_addr : rc.dev2_addr;
    if (0==bt_reconnect_addr_valid(addr)) {
        rc.target = 0;
        rc.state = BT_RECONNECT_STATE_IDLE;
        return;
    }
    BTSOURCE_RECONN_DBG("[Reconnect] Try connect index %d, addr: [0x%x][0x%x][0x%x][0x%x][0x%x][0x%x]\n",rc.target, addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
    
    if (BtSourceReconnectDev(addr, rc.target-1)) {
        // 连接请求已发送，等待结果（通过update函数通知）
        rc.state = BT_RECONNECT_STATE_CONNECT_DELAY; //等待连接成功 / time out
        rc.conn_delay_start = get_system_tick();
        BTSOURCE_RECONN_DBG("[Reconnect] Connect request sent\n");
    } else {
        // 连接失败，进入等待重试状态
        BTSOURCE_RECONN_DBG("[Reconnect] Connect failed\n");
        rc.state = BT_RECONNECT_STATE_WAITING; // waiting
        rc.wait_start = get_system_tick();
    }
}

static void handle_connect_delay_state(void) {
    //超时检测
    uint32_t current_tick = get_system_tick();
    if (current_tick - rc.conn_delay_start >= BT_RECONNECT_TIME_OUT) {
        rc.conn_delay_start = get_system_tick();

        rc.retry++;
        BTSOURCE_RECONN_DBG("[ReconnectTimeOut] Retry %d/%d\n", rc.retry, rc.max_retry);
        
        if (rc.retry >= rc.max_retry) {
            // 达到最大重试次数
            BTSOURCE_RECONN_DBG("[ReconnectTimeOut] Max retry reached, stop\n");
            rc.state = BT_RECONNECT_STATE_IDLE;
            rc.target = 0;
            rc.retry = 0;
        } else {
            // 继续尝试连接
            rc.state = BT_RECONNECT_STATE_CONNECTING; // connecting
        }
        }
}

static void handle_waiting_state(void) {
    // 检查等待时间（BT_SRC_POR_INTERNAL_TIME 秒重试间隔）
    uint32_t current_tick = get_system_tick();
    if (current_tick - rc.wait_start < BT_SRC_POR_INTERNAL_TIME * 1000) {
        return; // 还没到重试时间
    }
    
    rc.retry++;
    BTSOURCE_RECONN_DBG("[Reconnect] Retry %d/%d\n", rc.retry, rc.max_retry);
    
    if (rc.retry >= rc.max_retry) {
        // 达到最大重试次数
        BTSOURCE_RECONN_DBG("[Reconnect] Max retry reached, stop\n");
        rc.state = BT_RECONNECT_STATE_IDLE;
        rc.target = 0;
        rc.retry = 0;
    } else {
        // 继续尝试连接
        rc.state = BT_RECONNECT_STATE_CONNECTING; // connecting
    }
}

static void select_next_target(void) {
    switch (rc.mode) {
        case BT_RECONNECT_MODE_AUTO: // auto mode: 优先index1，然后index2
            if (!rc.dev1_connected && rc.dev1_addr[0]) {
                rc.target = 1;
            } else if (!rc.dev2_connected && rc.dev2_addr[0]) {
                rc.target = 2;
            }
            break;
            
        case BT_RECONNECT_MODE_BOTH: // both mode: 两个都连
            if (!rc.dev1_connected && rc.dev1_addr[0]) {
                rc.target = 1;
            } else if (!rc.dev2_connected && rc.dev2_addr[0]) {
                rc.target = 2;
            }
            break;
            
        case BT_RECONNECT_MODE_INDEX1: // index1 only
            if (!rc.dev1_connected && rc.dev1_addr[0]) {
                rc.target = 1;
            }
            break;
            
        case BT_RECONNECT_MODE_INDEX2: // index2 only
            if (!rc.dev2_connected && rc.dev2_addr[0]) {
                rc.target = 2;
            }
            break;
            
        default: // off
            break;
    }
}

// ==================== 查询API ====================

bool bt_reconnect_is_needed(void) {

    switch (rc.mode) {
        case BT_RECONNECT_MODE_AUTO: // auto
        //BTSOURCE_RECONN_DBG("bt_reconnect_is_needed 1 %d, addr: [0x%x][0x%x][0x%x][0x%x][0x%x][0x%x]\n",rc.dev1_connected, rc.dev1_addr[0],rc.dev1_addr[1],rc.dev1_addr[2],rc.dev1_addr[3],rc.dev1_addr[4],rc.dev1_addr[5]);
        //BTSOURCE_RECONN_DBG("bt_reconnect_is_needed 2 %d, addr: [0x%x][0x%x][0x%x][0x%x][0x%x][0x%x]\n",rc.dev2_connected, rc.dev2_addr[0],rc.dev2_addr[1],rc.dev2_addr[2],rc.dev2_addr[3],rc.dev2_addr[4],rc.dev2_addr[5]);
            return (!rc.dev1_connected && bt_reconnect_addr_valid(rc.dev1_addr)) || 
                   (!rc.dev2_connected && bt_reconnect_addr_valid(rc.dev2_addr));
            
        case BT_RECONNECT_MODE_BOTH: // both
            return (!rc.dev1_connected && bt_reconnect_addr_valid(rc.dev1_addr)) || 
                   (!rc.dev2_connected && bt_reconnect_addr_valid(rc.dev2_addr));
            
        case BT_RECONNECT_MODE_INDEX1: // index1
            return (!rc.dev1_connected && bt_reconnect_addr_valid(rc.dev1_addr));
            
        case BT_RECONNECT_MODE_INDEX2: // index2
            return (!rc.dev2_connected && bt_reconnect_addr_valid(rc.dev2_addr));
            
        default: // off
            return false;
    }
}

bool bt_reconnect_addr_valid(uint8_t *addr)
{
	if(((addr[0]==0)
		&&(addr[1]==0)
		&&(addr[2]==0)
		&&(addr[3]==0)
		&&(addr[4]==0)
		&&(addr[5]==0))
		||
		((addr[0]==0xff)
		&&(addr[1]==0xff)
		&&(addr[2]==0xff)
		&&(addr[3]==0xff)
		&&(addr[4]==0xff)
		&&(addr[5]==0xff)))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

bool bt_reconnect_is_active(void) {
    return rc.state != BT_RECONNECT_MODE_IDLE;
}

uint8_t bt_reconnect_get_state(void) {
    return rc.state;
}

uint8_t bt_reconnect_get_target(void) {
    return rc.target;
}

uint8_t bt_reconnect_get_retry_count(void) {
    return rc.retry;
}
#endif