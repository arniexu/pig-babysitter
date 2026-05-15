#ifndef tcp_mqtt_h_
#define tcp_mqtt_h_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "usart.h"
#include "timer.h"

#include "mqtt.h"
#include "ConfigModuleNoBlock.h"

extern mqtt_t mqtt_value; // mqtt 变量

extern int tcp_mqtt_index;
extern char mqtt_client_id[66]; // client_id
extern char tcp_mqtt_addr[55];  // IP地址(可以填写域名也可以填写IP)
extern char tcp_mqtt_ip[55];
extern int tcp_mqtt_port; // TCP服务器(MQTT服务器)端口号

extern int connect_mqtt_delay_value;
extern int dht11_delay_value;

extern unsigned char tcp_mqtt_buff[128];

extern char mqtt_connect_flag; // MQTT连接标志 1:连接上

extern uint8_t Updown_flag;

extern uint8_t network_mode; // 网络模式，0:4g模组  1:WIFI模组
/**
 * @brief   初始化MQTT的一些参数
 * @param   None
 * @param   None
 * @param   None
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void tcp_mqtt_init(void);

/**
 * @brief   TCP客户端
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void tcp_mqtt_while(void);

/**
 * @brief   接收到模组数据以后,把数据传递给此函数
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void tcp_mqtt_into_data(char *data, int len);

extern uint8_t ESP_Flag_Blower, ESP_Flag_Heat; // 定义小程序中设定的风机开关量和加热开关量
extern uint8_t light_state;
extern uint16_t warn_num, today_num;
extern uint8_t Usart2_rx_flag, warn_num_Flag, light_state_Flag, fan_state_Flag, wind_temp_Flag, warn_light_Flag, birth_event_flag, work_mode_flag, birth_led_flag;

void SendUpdown(void);

void SendWran_num(void);
void SendLight_state(void);
void SendFan_state(void);
void SendWind_temp(void);
void SendWorkMode(void);
void SendBirthEvent(void);
void SendControl(void);
void SendNetworkmode1(void);
void SendNetworkmode4(void);
void SendUpdown1(void);

void SendWran_num1(void);
void SendLight_state1(void);
void SendFan_state1(void);
void SendWind_temp1(void);
void SendWorkMode1(void);
void SendBirthEvent1(void);
void SendControl1(void);
#endif
