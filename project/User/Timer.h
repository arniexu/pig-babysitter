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
uint8_t Timer_ShouldQueryNetworkStatus(void); // 查询网路状态标志位，开机30秒后会置1
uint8_t Timer_CanSendMqttOpen(void);
void Timer_MarkMqttOpenSent(void);
void Timer_ResetMqttOpenSent(void);
uint8_t Timer_CanSendNetworkMode(void);
void Timer_MarkNetworkModeSent(void);

#endif
