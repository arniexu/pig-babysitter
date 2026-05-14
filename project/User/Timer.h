#ifndef __TIMER_H
#define __TIMER_H
#include "stdint.h"

#include "ConfigModuleNoBlock.h"
#include "ConfigModuleBlock.h"
#include "usart.h"
#include "mqtt.h"
#include "tcp_mqtt.h"

void TIM3_Init(void);
extern uint8_t Flag_Blower;
extern uint8_t QueryForNetworkFlags; // 查询网路状态标志位，开机30秒后会置1
extern uint8_t mqttopen_sent_flag;
extern uint32_t delay_3s_cnt;
extern uint8_t network_sent_flag;
extern uint32_t delay_15s;

#endif
