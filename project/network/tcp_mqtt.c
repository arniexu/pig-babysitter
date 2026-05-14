#define tcp_mqtt_c_

#include "tcp_mqtt.h"
#include "string.h"
#include "Delay.h"
#include "led3.h"
#include "Blower_Control.h"
#include "cString.h"
#include "MyFlash.h"
#include "UART4.h"
#if 1 // 是否打印日志
#define debug_printf printf
#else
#define debug_printf(...)
#endif

mqtt_t mqtt_value; // mqtt 变量

// TCP
int tcp_mqtt_index = 0;						// socket编号
char tcp_mqtt_addr[55] = "iot.smartgw.net"; // IP地址(可以填写域名也可以填写IP)
// char tcp_mqtt_addr[55]="broker.emqx.io";

char tcp_mqtt_ip[55];
int tcp_mqtt_port = 18883; // TCP服务器(MQTT服务器)端口号
// MQTT
char mqtt_client_id[66] = "";								 // client_id(默认使用模组的MAC,用户不需要填写)
char mqtt_user_name[20] = "protect_child";					 // 用户名
char mqtt_password[50] = "c1g7ugsq9g509h9rkdtk29i1va7tdqrk"; // 密码
char mqtt_keepalive = 60;									 // 心跳包时间

char mqtt_publish_topic[60] = "";		// 存储发布的主题
char mqtt_publish_topic_event[60] = ""; // 存储发布事件的主题
char mqtt_subscribe_topic[60] = "";		// 存储订阅的主题

char mqtt_connect_flag = 0; // MQTT连接标志 1:连接上
char mqtt_connect_cnt = 0;

unsigned char *p_str;
unsigned char tcp_mqtt_buff[128]; // mqtt发送缓冲区
int tcp_mqtt_len = 0;

int connect_mqtt_delay_value = 0;
int dht11_delay_value = 0;

uint8_t network_mode; // 网络模式，0:4g模组  1:WIFI模组

extern float temp_err;
extern float hum_err;
uint8_t Updown_flag;
uint16_t warn_num;
uint16_t today_num;
uint8_t ESP_Flag_Blower, ESP_Flag_Heat; // 定义小程序中设定的风机开关量和加热开关量

uint8_t light_state; // 灯的状态
uint8_t warn_num_Flag, light_state_Flag, fan_state_Flag, wind_temp_Flag, warn_light_Flag, birth_event_flag, work_mode_flag, birth_led_flag;


/**
 * @brief   MQTT订阅主题成功回调
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void mqtt_subscribed_back(int pdata)
{
	debug_printf("\r\nmqtt_function_subscribed\r\n");
}

/**
 * @brief   MQTT订阅主题失败回调
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void mqtt_failsubscribed_back(int pdata)
{
	debug_printf("\r\nmqtt_function_failsubscribed\r\n");
}

/**
 * @brief   MQTT发送消息成功回调
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void mqtt_published_back(void)
{
	debug_printf("\r\nmqtt_function_published\r\n");
}

/**
 * @brief   MQTT连接成功回调
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void mqtt_connected_back(void)
{
	mqtt_connect_flag = 1; // 设置连接标志
	debug_printf("\r\nmqtt connected\r\n");

	/*设置订阅的主题*/
	memset(mqtt_subscribe_topic, NULL, sizeof(mqtt_subscribe_topic));
	snprintf((char *)mqtt_subscribe_topic, sizeof(mqtt_subscribe_topic), "/iot/protect_child/%s/property/set", &mqtt_client_id[0]);
	debug_printf("subscribe_topic:%s\r\n", mqtt_subscribe_topic);
	/*订阅主题*/
	mqtt_subscribe(&mqtt_value, (unsigned char *)mqtt_subscribe_topic, 1, mqtt_subscribed_back, mqtt_failsubscribed_back);

	//	/*设置订阅的主题*/
	//	memset(mqtt_subscribe_topic,NULL,sizeof(mqtt_subscribe_topic));
	//	snprintf((char*)mqtt_subscribe_topic, sizeof(mqtt_subscribe_topic), "/sys/6kYp6jszrDns2yh4/%s/c/service/property/set",&mqtt_client_id[0]);
	//	debug_printf("subscribe_topic:%s\r\n",mqtt_subscribe_topic);
	//	/*订阅主题*/
	//	mqtt_subscribe(&mqtt_value, (unsigned char*)mqtt_subscribe_topic, 1, mqtt_subscribed_back, mqtt_failsubscribed_back);

	/*设置发布的主题*/
	memset(mqtt_publish_topic, NULL, sizeof(mqtt_publish_topic));
	snprintf((char *)mqtt_publish_topic, sizeof(mqtt_publish_topic), "/iot/protect_child/%s/property/post", &mqtt_client_id[0]);
	/*设置发布事件的主题*/
	memset(mqtt_publish_topic_event, NULL, sizeof(mqtt_publish_topic_event));
	snprintf((char *)mqtt_publish_topic_event, sizeof(mqtt_publish_topic_event), "/iot/protect_child/%s/event/post", &mqtt_client_id[0]);
}

/**
 * @brief   MQTT断开连接回调
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void mqtt_disconnect_back(void)
{
	mqtt_init(&mqtt_value);

	mqtt_connect_flag = 0; // MQTT断开连接
	debug_printf("\r\nmqtt disconnected\r\n");
}

char *my_strstr(char *str0, char *str1)
{
	char *pos = NULL;
	char *p1 = NULL;
	char *p0 = NULL;

	pos = str0;

	while (*pos != '\0')
	{
		p0 = pos;
		p1 = str1;
		// while((*p1 != '\0')&& (*p0 != '\0') && (*p1 == *p0))
		while (*p1 != '\0' && *p0 != '\0')
		{
			if (*p1 != *p0)
			{
				break;
			}
			p0++;
			p1++;
		}
		if (*p1 == '\0')
		{
			return pos + strlen(str1);
		}
		pos++;
	}
	return NULL;
}

/**
 * @brief   MQTT接收数据回调
 * @param   topic:主题
 * @param   topic_len:主题长度
 * @param   data:消息
 * @param   lengh:消息长度
 * @retval  None
 * @warning None
 * @example
 **/
int tempRecvflag = 0;
int humRecvflag = 0;
float temp_recv;
float hum_recv;
void mqtt_receive_back(const char *topic, uint32_t topic_len, const char *data, uint32_t lengh)
{
	char recv_topic[60] = {0};
	char recv_data[50] = {0};

	// recv_topic = (char *)malloc(topic_len + 1);
	// recv_data = (char *)malloc(lengh + 1);

	// memcpy(recv_topic, (char *)topic, topic_len);
	// recv_topic[topic_len] = 0;
	// memcpy(recv_data, (char *)data, lengh);
	// recv_data[lengh] = 0;
	snprintf(recv_topic, sizeof(recv_topic), "%s", topic);
	snprintf(recv_data, sizeof(recv_topic), "%s", data);

	debug_printf("mqtt_recv:%s %s\r\n", recv_topic, recv_data); // 打印接收的主题和消息

	if (strstr(recv_topic, "/property/set") != NULL)
	{
		/* code */
		if (strstr((const char *)recv_data, "{\"mode\":0") != NULL)
		{
			work_mode = POSTPARTUM_MODE;
			work_mode_flag = 1;
			uint32_t temp = 0;
			memcpy(&temp, &work_mode, sizeof(WorkMode));
			FlashData[0] = temp;
			Write_Flash(FlashData, 3);
			printf("work_mode = %d\r\n", work_mode);
		}
		if (strstr((const char *)recv_data, "{\"mode\":1") != NULL)
		{
			work_mode = PRODUCTION_MODE;
			TIM6_Time = 0;
			work_mode_flag = 1;
			uint32_t temp = 0;
			memcpy(&temp, &work_mode, sizeof(WorkMode));
			FlashData[0] = temp;
			Write_Flash(FlashData, 3);
			printf("work_mode = %d\r\n", work_mode);
		}
		if (strstr((const char *)recv_data, "{\"light_state\":0") != NULL)
		{
			light_state = 0;
			light_state_Flag = 1;
			printf("light_state = %s\r\n", recv_data);
		}
		if (strstr((const char *)recv_data, "{\"light_state\":1") != NULL)
		{
			light_state = 1;
			light_state_Flag = 1;
		}
		if (strstr((const char *)recv_data, "{\"wind_temp\":0") != NULL)
		{
			if (Key_Heat == 0)
			{
				/* code */
				ESP_Flag_Heat = 0;
				wind_temp_Flag = 1;
				uint32_t temp = 0;
				memcpy(&temp, &ESP_Flag_Heat, sizeof(uint8_t));
				FlashData[2] = temp;
				Write_Flash(FlashData, 3);
			}
		}

		if (strstr((const char *)recv_data, "{\"wind_temp\":1") != NULL)
		{
			ESP_Flag_Heat = 1;
			wind_temp_Flag = 1;
			uint32_t temp = 0;
			memcpy(&temp, &ESP_Flag_Heat, sizeof(uint8_t));
			FlashData[2] = temp;
			Write_Flash(FlashData, 3);
		}

		if (strstr((const char *)recv_data, "{\"fan_state\":0") != NULL)
		{
			ESP_Flag_Blower = 0;
			fan_state_Flag = 1;
			uint32_t temp = 0;
			memcpy(&temp, &ESP_Flag_Blower, sizeof(uint8_t));
			FlashData[1] = temp;
			Write_Flash(FlashData, 3);
		}
		if (strstr((const char *)recv_data, "{\"fan_state\":1") != NULL)
		{
			ESP_Flag_Blower = 1;
			fan_state_Flag = 1;
			uint32_t temp = 0;
			memcpy(&temp, &ESP_Flag_Blower, sizeof(uint8_t));
			FlashData[1] = temp;
			Write_Flash(FlashData, 3);
		}

		if (strstr((const char *)recv_data, "{\"warn_light\":0") != NULL)
		{
			if (work_mode == PRODUCTION_MODE)
			{
				/* code */
				birth_led_flag = 0;
			}

			warn_light = 0;
			warn_light_Flag = 1;
		}
		if (strstr((const char *)recv_data, "{\"warn_light\":1") != NULL)
		{
			warn_light = 1;
			warn_light_Flag = 1;
		}

		if (strstr((const char *)recv_data, "\"today_num\":") != NULL)
		{

			if (strchr(recv_data, ',') != NULL)
			{
				char *str = StrBetwString(recv_data, "today_num\":", ",");
				today_num = atoi(str);
				cStringRestore();
			}
			else
			{
				char *str = StrBetwString(recv_data, "today_num\":", "}");
				today_num = atoi(str);
				cStringRestore();
			}

			// today_num = atoi(today_num_str + 1); // 转换字符串为整数
			printf("today_num:%d", today_num);
		}

		if (strstr((const char *)recv_data, "\"warn_num\":") != NULL) // 5.26 将0去掉
		{
			char *str = StrBetwString(recv_data, "warn_num\":", "}");

			warn_num = atoi(str); // 转换字符串为整数
			cStringRestore();
			warn_num_Flag = 1;
			printf("warn_num:%d", warn_num);
		}
	}

	// free(recv_topic);
	// free(recv_data);
}

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
void tcp_mqtt_init(void)
{
	// 初始化MQTT参数,和注册回调函数
	mqtt_init(&mqtt_value);
	mqtt_connect_reg(&mqtt_value, mqtt_connected_back);		// 注册连接回调函数
	mqtt_disconnect_reg(&mqtt_value, mqtt_disconnect_back); // 注册断开连接回调函数
	mqtt_received_reg(&mqtt_value, mqtt_receive_back);		// 注册接收数据回调函数
}

/**
 * @brief   TCP客户端
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
uint8_t send_networkmode4_count = 0;
void tcp_mqtt_while(void)
{
	int len = 0;
	//	char buff[200];
	if (ConfigModuleNoBlockFlage == 1) // 连接上TCP
	{
		/*未建立MQTT协议连接,发送连接MQTT协议*/
		if (mqtt_connect_flag == 0 && connect_mqtt_delay_value > 1000)
		{
			connect_mqtt_delay_value = 0;
			// 打包MQTT连接协议
			// memset(temp, 0, sizeof(temp));
			// len = sprintf((char *)temp, "%s", mqtt_client_id);
			printf("mqtt_client_id = %s\r\n", mqtt_client_id);
			len = mqtt_function_connect_msg(&mqtt_value, &p_str, mqtt_client_id, mqtt_user_name, mqtt_password, mqtt_keepalive, 1, NULL, NULL, 0, 0);
			printf("len = %d\r\n", len);
			if (len > 0)
			{
				memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff));
				// 组合发送数据命令
				tcp_mqtt_len = sprintf((char *)tcp_mqtt_buff, "AT+CIPSEND=%d,%d\r\n", tcp_mqtt_index, len);
				usart4_send((char *)tcp_mqtt_buff, tcp_mqtt_len); // 发送数据命令
				Delay_ms(100);
				// 发送MQTT协议数据
				usart4_send((char *)p_str, len); // 发送数据给服务器

				debug_printf("\r\nconnect mqtt server ...\r\n");
			}
			else
			{
				debug_printf("\r\nconnect mqtt msg err =%d ...\r\n", tcp_mqtt_len);
			}

			// 超过3次没有连接上MQTT,重新连接TCP
			mqtt_connect_cnt++;
			if (mqtt_connect_cnt > 3)
			{
				mqtt_connect_cnt = 0;
				mqtt_connect_flag = 0;

				/*重新配置模块连接TCP*/
				ConfigModuleNoBlockCaseValue = 0;
				ConfigModuleNoBlockFlage = 0;
			}
		}
		else if (mqtt_connect_flag == 1) // 已经连接上了MQTT服务器
		{
				if (send_networkmode4_count < 3)
				{
					SendNetworkmode4();
					send_networkmode4_count++;
					Delay_ms(10);
				}
			// 处理要发送的消息给服务器
			mqtt_send_function(&mqtt_value);
		}
	}
}

/**
 * @brief   接收到模组数据以后,把数据传递给此函数
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void tcp_mqtt_into_data(char *data, int len)
{
	// 没有连接上MQTT服务器
	if (ConfigModuleNoBlockFlage && !mqtt_connect_flag)
	{
		// 解析返回的数据,如果是MQTT连接成功消息,则内部会调用MQTT连接成功回调函数
		mqtt_function_connect_ack(&mqtt_value, (unsigned char *)data, len);
	}

	// 连接上MQTT服务器
	if (ConfigModuleNoBlockFlage && mqtt_connect_flag)
	{
		// 处理MQTT正常通信的消息
		mqtt_read_function(&mqtt_value, (unsigned char *)data, len);
	}
}

extern uint8_t send_flag;


void SendNetworkmode4(void)
{
if (network_mode == 1)
	{
		/* code */
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"network\":%d}", 1);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}

void SendNetworkmode1(void)
{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		sprintf(tcp_mqtt_buff, "{\"network\":%d}", 2);
		usart1_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
}

void SendUpdown(void)
{
		/* code */
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"up_down\":%d}", 1);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}	
}
void SendUpdown1(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		sprintf(tcp_mqtt_buff, "{\"up_down\":%d}", 1);
		usart1_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		/* code */
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"up_down\":%d}", 1);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}


void SendWran_num(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"warn_num\":%d}", warn_num);
		usart4_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
//			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"warn_num\":%d}", warn_num);
//			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}

void SendWran_num1(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"warn_num\":%d}", warn_num);
		usart1_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
//			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"warn_num\":%d}", warn_num);
//			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}

void SendLight_state(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"light_state\":%d}", light_state);
		usart4_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"light_state\":%d}", light_state);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}
void SendLight_state1(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"light_state\":%d}", light_state);
		usart1_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"light_state\":%d}", light_state);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}

void SendFan_state(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"fan_state\":%d}", ESP_Flag_Blower);
		usart1_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"fan_state\":%d}", ESP_Flag_Blower);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}
void SendFan_state1(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"fan_state\":%d}", ESP_Flag_Blower);
		usart1_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"fan_state\":%d}", ESP_Flag_Blower);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}
void SendWind_temp1(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"wind_temp\":%d}", ESP_Flag_Heat);
		usart1_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"wind_temp\":%d}", ESP_Flag_Heat);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}
void SendWind_temp(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"wind_temp\":%d}", ESP_Flag_Heat);
		usart4_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"wind_temp\":%d}", ESP_Flag_Heat);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}
void SendWarn_light(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"warn_light\":%d}", warn_light);
		usart4_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"warn_light\":%d}", warn_light);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}
void SendWarn_light1(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"warn_light\":%d}", warn_light);
		usart1_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"warn_light\":%d}", warn_light);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}
void SendWorkMode1(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"mode\":%d}", work_mode);
		usart1_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"mode\":%d}", work_mode);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}
void SendWorkMode(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"mode\":%d}", work_mode);
		usart4_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"mode\":%d}", work_mode);
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}
void SendBirthEvent(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"type\":\"%s\"}", "birth");
		usart4_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"type\":\"%s\"}", "birth");
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic_event, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}
void SendBirthEvent1(void)
{
	if (network_mode == 0)
	{
		/* code */
		memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff)); // 清空发送缓冲区
		snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"type\":\"%s\"}", "birth");
		usart1_send(tcp_mqtt_buff, strlen(tcp_mqtt_buff));
		printf("SEND:%s", tcp_mqtt_buff);
	}
	else if (network_mode == 1)
	{
		if (mqtt_connect_flag) // 连接上MQTT
		{
			// 发送起卧数据给MQTT服务器
			tcp_mqtt_len = snprintf((char *)tcp_mqtt_buff, sizeof(tcp_mqtt_buff), "{\"type\":\"%s\"}", "birth");
			mqtt_publish(&mqtt_value, (unsigned char *)mqtt_publish_topic_event, tcp_mqtt_buff, tcp_mqtt_len, 0, 0, NULL);

			printf("tcp_mqtt_buff : %s", tcp_mqtt_buff);
		}
	}
}
void SendControl(void)
{
	if (Getup_Flag == 1) // 更新数据标志位有效
	{
		SendUpdown();
		printf("Getup_Flag\r\n");
		Getup_Flag = 0;
	}
	if (warn_num_Flag == 1)
	{
		/* code */
//		SendWran_num();
		printf("warn_num_Flag\r\n");
		warn_num_Flag = 0;
	}
	if (light_state_Flag == 1)
	{
		/* code */
		SendLight_state();
		printf("light_state_Flag\r\n");
		light_state_Flag = 0;
	}
	if (fan_state_Flag == 1)
	{
		/* code */
		SendFan_state();
		printf("fan_state_Flag\r\n");
		fan_state_Flag = 0;
	}
	if (wind_temp_Flag == 1)
	{
		/* code */
		SendWind_temp();
		printf("wind_temp_Flag\r\n");
		wind_temp_Flag = 0;
	}
	if (warn_light_Flag == 1)
	{
		/* code */
		SendWarn_light();
		printf("warn_light_Flag\r\n");
		warn_light_Flag = 0;
	}
	if (work_mode_flag == 1)
	{
		/* code */
		SendWorkMode();
		printf("warn_light_Flag\r\n");
		work_mode_flag = 0;
	}
	if (birth_event_flag == 1)
	{
		/* code */
		SendBirthEvent();
		printf("warn_light_Flag\r\n");
		birth_event_flag = 0;
	}
}
void SendControl1(void)
{
	
	if (Getup_Flag == 1) // 更新数据标志位有效
	{
		SendUpdown1();
		printf("Getup_Flag\r\n");
		Getup_Flag = 0;
	}
	if (warn_num_Flag == 1)
	{
		/* code */
//		SendWran_num1();
		printf("warn_num_Flag\r\n");
		warn_num_Flag = 0;
	}
	if (light_state_Flag == 1)
	{
		/* code */
		SendLight_state1();
		printf("light_state_Flag\r\n");
		light_state_Flag = 0;
	}
	if (fan_state_Flag == 1)
	{
		/* code */
		SendFan_state();
		printf("fan_state_Flag\r\n");
		fan_state_Flag = 0;
	}
	if (wind_temp_Flag == 1)
	{
		/* code */
		SendWind_temp1();
		printf("wind_temp_Flag\r\n");
		wind_temp_Flag = 0;
	}
	if (warn_light_Flag == 1)
	{
		/* code */
		SendWarn_light1();
		printf("warn_light_Flag\r\n");
		warn_light_Flag = 0;
	}
	if (work_mode_flag == 1)
	{
		/* code */
		SendWorkMode1();
		printf("warn_light_Flag\r\n");
		work_mode_flag = 0;
	}
	if (birth_event_flag == 1)
	{
		/* code */
		SendBirthEvent1();
		printf("warn_light_Flag\r\n");
		birth_event_flag = 0;
	}
}
