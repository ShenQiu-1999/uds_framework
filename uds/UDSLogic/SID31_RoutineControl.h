#ifndef _SID31_ROUTINE_CONTROL_H_
#define _SID31_ROUTINE_CONTROL_H_


#include <stdint.h>
#include "uds_type.h"

#define RTCTRL_NUM     3

#define RID_FF00    0xFF00  // eraseMemory
#define RID_FF01    0xFF01  // checkProgrammingDependencies
#define RID_0202    0x0202  // CRC32 编程完整性检查


typedef struct
{
    uint8_t rid;                                                // 例程 RID
    uint8_t (*start_routine) (uint8_t *data, uint8_t data_len); // 例程启动函数
	void (*stop_routine) (void);	                            // 例程停止函数
    uint8_t rtst;                                               // 例程状态 空闲/运行
} routine_control_t;

typedef enum
{
    UDS_RT_ST_IDLE,
    UDS_RT_ST_RUNNING,
} uds_routine_state_t;

/******************************************************************************
* 函数名称: void service_31_RoutineControl(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 31 服务 - 例程控制
* 输入参数: uint8_t*    msg_buf         --数据首地址
    　　　　uint8_t     msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
void service_31_RoutineControl(const uint8_t* msg_buf, uint16_t msg_dlc);


/******************************************************************************
* 函数名称: bool_t service_31_check_len(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 检查 31 服务数据长度是否合法
* 输入参数: uint16_t msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: TRUE: 合法; FALSE: 非法
* 其它说明: 无
******************************************************************************/
bool_t service_31_check_len(const uint8_t* msg_buf, uint16_t msg_dlc);

#endif

/****************EOF****************/
