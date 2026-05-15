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

typedef struct
{
	int tcp_mqtt_index;
	char mqtt_client_id[66];
	char tcp_mqtt_addr[55];
	char tcp_mqtt_ip[55];
	int tcp_mqtt_port;
	int connect_mqtt_delay_value;
	int dht11_delay_value;
	unsigned char tcp_mqtt_buff[128];
	char mqtt_connect_flag;
	char mqtt_connect_cnt;
	uint8_t Updown_flag;
	uint8_t network_mode;
	uint16_t warn_num;
	uint16_t today_num;
	uint8_t ESP_Flag_Blower;
	uint8_t ESP_Flag_Heat;
	uint8_t light_state;
	uint8_t warn_num_Flag;
	uint8_t light_state_Flag;
	uint8_t fan_state_Flag;
	uint8_t wind_temp_Flag;
	uint8_t warn_light_Flag;
	uint8_t birth_event_flag;
	uint8_t work_mode_flag;
	uint8_t birth_led_flag;
	char mqtt_user_name[20];
	char mqtt_password[50];
	char mqtt_keepalive;
	char mqtt_publish_topic[60];
	char mqtt_publish_topic_event[60];
	char mqtt_subscribe_topic[60];
	unsigned char *p_str;
	int tcp_mqtt_len;
} tcp_mqtt_context_t;

extern tcp_mqtt_context_t g_tcp_mqtt_context;

#define tcp_mqtt_index (g_tcp_mqtt_context.tcp_mqtt_index)
#define mqtt_client_id (g_tcp_mqtt_context.mqtt_client_id)
#define tcp_mqtt_addr (g_tcp_mqtt_context.tcp_mqtt_addr)
#define tcp_mqtt_ip (g_tcp_mqtt_context.tcp_mqtt_ip)
#define tcp_mqtt_port (g_tcp_mqtt_context.tcp_mqtt_port)
#define connect_mqtt_delay_value (g_tcp_mqtt_context.connect_mqtt_delay_value)
#define dht11_delay_value (g_tcp_mqtt_context.dht11_delay_value)
#define tcp_mqtt_buff (g_tcp_mqtt_context.tcp_mqtt_buff)
#define mqtt_connect_flag (g_tcp_mqtt_context.mqtt_connect_flag)
#define mqtt_connect_cnt (g_tcp_mqtt_context.mqtt_connect_cnt)
#define Updown_flag (g_tcp_mqtt_context.Updown_flag)
#define network_mode (g_tcp_mqtt_context.network_mode)
#define warn_num (g_tcp_mqtt_context.warn_num)
#define today_num (g_tcp_mqtt_context.today_num)
#define ESP_Flag_Blower (g_tcp_mqtt_context.ESP_Flag_Blower)
#define ESP_Flag_Heat (g_tcp_mqtt_context.ESP_Flag_Heat)
#define light_state (g_tcp_mqtt_context.light_state)
#define warn_num_Flag (g_tcp_mqtt_context.warn_num_Flag)
#define light_state_Flag (g_tcp_mqtt_context.light_state_Flag)
#define fan_state_Flag (g_tcp_mqtt_context.fan_state_Flag)
#define wind_temp_Flag (g_tcp_mqtt_context.wind_temp_Flag)
#define warn_light_Flag (g_tcp_mqtt_context.warn_light_Flag)
#define birth_event_flag (g_tcp_mqtt_context.birth_event_flag)
#define work_mode_flag (g_tcp_mqtt_context.work_mode_flag)
#define birth_led_flag (g_tcp_mqtt_context.birth_led_flag)
#define mqtt_user_name (g_tcp_mqtt_context.mqtt_user_name)
#define mqtt_password (g_tcp_mqtt_context.mqtt_password)
#define mqtt_keepalive (g_tcp_mqtt_context.mqtt_keepalive)
#define mqtt_publish_topic (g_tcp_mqtt_context.mqtt_publish_topic)
#define mqtt_publish_topic_event (g_tcp_mqtt_context.mqtt_publish_topic_event)
#define mqtt_subscribe_topic (g_tcp_mqtt_context.mqtt_subscribe_topic)
#define p_str (g_tcp_mqtt_context.p_str)
#define tcp_mqtt_len (g_tcp_mqtt_context.tcp_mqtt_len)
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

extern uint8_t Usart2_rx_flag;

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
