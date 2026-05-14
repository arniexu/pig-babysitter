/********************************************************************************
  * @file    mqtt.c
  * @author  fengwu yang
  * @version V1.0.0
	* @version V1.0.1
	//2022/03/12  mqtt_function_connect_ack 判断是否连接到服务器函数里面处理下粘包
  * @date    2020/3/19
  * @brief
  ******************************************************************************
	一,使用说明
		1,把以下程序放在1ms定时器中断中
			mqtt_time_data(&mqtt_value);

		2,把以下程序放到主函数
		mqtt_t mqtt_value; //mqtt 变量
		int len;
		char mqtt_connect_flag=0;
		unsigned char* mqtt_msg;
		void mqtt_function_connect_callback(void){mqtt_connect_flag=1;}//连接回调
		void mqtt_function_disconnect(void){mqtt_connect_flag=0;}//断开回调
		void mqtt_function_receive(const char* topic, uint32_t topic_len,const char *data, uint32_t lengh){}//接收回调
		//初始化MQTT参数,和注册回调函数
		mqtt_init(&mymqtt);
		mqtt_connect_reg(&mqtt_value,mqtt_function_connect_callback);//注册连接回调函数
		mqtt_disconnect_reg(&mqtt_value,mqtt_function_disconnect);//注册断开连接回调函数
		mqtt_received_reg(&mqtt_value,mqtt_function_receive);//注册接收数据回调函数

		while(1)
		{
			if(连接上了TCP服务器)
			{
				if(!mqtt_connect_flag)//未连接上MQTT服务器
				{
					//调用api组合mqtt连接协议
					len = mqtt_function_connect_msg(&mqtt_value, "clientid", "用户名", "密码", 心跳包时间, 是否清除session, 遗嘱主题, 遗嘱消息, 遗嘱消息等级, 是否保留消息);
					tcp_send(mqtt_msg,len);//调用实际的TCP发送数据函数发送数据给服务器

					if(服务器返回了数据)//把服务器返回的数据交给mqtt_function_connect_ack函数处理
					{
						mqtt_function_connect_ack(&mqtt_value,数据地址,数据长度);//判断连接成功内部会调用注册的连接成功回调函数
					}
				}
				else
				{
					mqtt_send_function(&mqtt_value);//提取发送缓存的MQTT协议

					if(服务器返回了数据)
					{
						mqtt_read_function(&mqtt_value,数据地址,数据长度);
					}
				}
			}
			else
			{
				mqtt_connect_flag=0;
			}
		}

		3,订阅主题
		void mqtt_function_subscribed(int pdata){}//订阅成功回调
		void mqtt_function_failsubscribed(int pdata){}////订阅失败回调
		//假设订阅的主题为 "1111" 消息等级为0
		mqtt_subscribe(&mqtt_value, "1111", 0,mqtt_function_subscribed,mqtt_function_failsubscribed);//订阅主题

		4,发布消息
		void mqtt_function_published(void){}//发布成功回调
		//假设发布的主题为 "1111";  发布的消息为:"qqqq"; 消息等级为:0;  需要服务器记录消息
		mqtt_publish(&mqtt_value,"1111","qqqq",4, 0, 1,mqtt_function_published);//发布消息
  ******************************************************************************
  */

#define MQTT_C_
#include <stdio.h>
#include <string.h>
#include "mqtt.h"
#include "mqtt_msg.h"
#include "BufferManage.h"

#include "usart.h"
#include "cString.h"
#include "tcp_mqtt.h" //用一下里面的数组 tcp_mqtt_buff

// 替换自己的发送函数:ctrl+F   搜索:   Send function

/**
 * @brief   发送MQTT数据:把发送数据给网络模块的函数放在此处
 * @param   mqtt:  结构体指针
 * @param   mqtt   发送的数据
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void mqtt_send_function(mqtt_t *mqtt)
{
	int len;
	/*可以发送数据*/
	if (mqtt->timer_out_cnt <= 0 && mqtt->timer_out_send <= 0)
	{
		if (mqtt_cipsend_enable)
			mqtt->cipsend = 1;

		if (mqtt->cipsend == 0)
		{
#if mqtt_cipsend_enable == 0
			BufferManageRead(&mqtt->buff_manage_struct_t, mqtt->send_buff, &mqtt->buff_manage_struct_t.SendLen); /*提取缓存区的数据*/
			if (mqtt->buff_manage_struct_t.SendLen > 0)															 // 有数据需要发送
			{
				// Send function
				// mqtt->buff_manage_struct_t.SendLen:发送的数据长度
				// 假设先给模组发送AT+CIPSEND={数据长度}\r\n  等模组返回 >  再发送实际的数据
				// 下面是发送AT+CIPSEND={数据长度}\r\n   (请用户根据自己模组的实际情况修改)
				len = sprintf((char *)tcp_mqtt_buff, "AT+CIPSEND=%d,%d\r\n", tcp_mqtt_index, mqtt->buff_manage_struct_t.SendLen);
				usart4_send((char *)tcp_mqtt_buff, len); // 发送数据命令
				// 调用完上面的发送函数以后,底层默认延时3S.请用户在接收到 > 的地方写上以下程序
				/*
				if(接收到>)
				{
					mqtt_clear_cipsend(mqtt_t *mqtt)
				}
				*/

				mqtt->timer_out_cnt = mqtt_timerout_default; // 设置超时时间
				mqtt->cipsend = 1;
			}
#endif
		}
		else
		{
			mqtt->cipsend = 0;
			if (mqtt_cipsend_enable)
				BufferManageRead(&mqtt->buff_manage_struct_t, mqtt->send_buff, &mqtt->buff_manage_struct_t.SendLen); /*提取缓存区的数据*/
			if (mqtt->buff_manage_struct_t.SendLen > 0)																 // 有数据需要发送
			{
				if (mqtt_get_type(mqtt->send_buff) == MQTT_MSG_TYPE_SUBSCRIBE)
				{																							  // 发送的消息是订阅
					mqtt->mqtt_message_id = mqtt_get_id(mqtt->send_buff, mqtt->buff_manage_struct_t.SendLen); // 获取消息ID
					mqtt->mqtt_message_type = MQTT_MSG_TYPE_SUBSCRIBE;
					mqtt->timer_out_cnt = mqtt_timerout_default; // 设置超时时间
				}
				else if (mqtt_get_type(mqtt->send_buff) == MQTT_MSG_TYPE_PUBLISH)
				{																				  // 发送的消息是发布
					if (mqtt_get_qos(mqtt->send_buff) == 1 || mqtt_get_qos(mqtt->send_buff) == 2) // 消息等级是1或者2
					{
						mqtt->mqtt_message_id = mqtt_get_id(mqtt->send_buff, mqtt->buff_manage_struct_t.SendLen); // 获取消息ID,以便应答
						mqtt->mqtt_message_type = MQTT_MSG_TYPE_PUBLISH;
						mqtt->timer_out_cnt = mqtt_timerout_default; // 设置超时时间
					}
				}
				else if (mqtt_get_type(mqtt->send_buff) == MQTT_MSG_TYPE_PUBREL)
				{												 // 客户端上次发送了消息等级是2的消息,服务器返回PUBREC,客户端需要返回PUBREL
					mqtt->timer_out_cnt = mqtt_timerout_default; // 设置超时时间
				}
				// 把要发送的数据发送给MQTT服务器:请替换自己的发送函数
				// mqtt->send_buff:发送的数据
				// mqtt->buff_manage_struct_t.SendLen:发送的数据长度
				// Send function

				//				memset(tcp_mqtt_buff, 0, sizeof(tcp_mqtt_buff));
				//				len = sprintf((char*)tcp_mqtt_buff, "AT+CIPSEND=%d,%d\r\n", tcp_mqtt_index, mqtt->buff_manage_struct_t.SendLen);
				//				usart4_send((char*)tcp_mqtt_buff, len);//发送数据命令
				//				delay_ms(100);//等待返回 >
				// 发送MQTT协议数据
				usart4_send((char *)mqtt->send_buff, mqtt->buff_manage_struct_t.SendLen); // 发送数据给服务器

				// 调用上面的发送函数以后,程序默认等待3S,再发送下一条数据!
				// 解决方案1:调用自己的发送数据函数以后,在发送完成的地方写上: mymqtt.timer_out_send = 0; //记得包含 #include "mqtt.h"
				// 解决方案2:如果没有发送完成的地方,请在mqtt.h里面修改  mqtt_timerout_send_default  变量的默认延时时间 ms

				mqtt->timer_out_send = mqtt_timerout_send_default;
			}
		}
	}
	mqtt_keep_alive(mqtt); // 处理发送心跳包
}

/*非透传模式检测到返回调用改函数*/
void mqtt_clear_cipsend(mqtt_t *mqtt)
{
	if (mqtt->cipsend == 1 && mqtt->timer_out_cnt > 0)
		mqtt->timer_out_cnt = 0;
}

/**
 * @brief   接收处理MQTT数据:用户需要把网络模块接收到的MQTT数据传给此函数处理
 * @param   mqtt:    结构体指针
 * @param   buffer   接收的MQTT数据
 * @param   length   数据长度
 * @retval  None
 * @warning None
 * @example
 **/
void mqtt_read_function(mqtt_t *mqtt, unsigned char *buffer, uint16_t length)
{
	uint8_t msg_type;
	uint8_t msg_qos;
	uint16_t msg_id;
	uint16_t msg_len;
	int DataLen = 0;

	msg_type = mqtt_get_type(buffer);
	msg_qos = mqtt_get_qos(buffer);
	msg_id = mqtt_get_id(buffer, length);

	msg_len = mqtt_get_total_length(buffer, length);
	if (msg_len != length)
		msg_type = 0; // 错误消息

	if (mqtt->mqtt_message_type == MQTT_MSG_TYPE_SUBSCRIBE)
	{								// 上次发送的是订阅主题
		mqtt_keep_alive_init(mqtt); // 初始化心跳包变量
		// 上次发送的消息是订阅
		if (msg_type == MQTT_MSG_TYPE_SUBACK)
		{							 // 获取应答
			mqtt->timer_out_cnt = 0; // 停止超时定时器
			if (msg_id == mqtt->mqtt_message_id && (buffer[length - 1] & 0xff) != 0x80)
			{
				mqtt->mqtt_message_type = MQTT_MSG_TYPE_SUBACK;
				if (mqtt->subscribedCb != NULL)
				{
					mqtt->subscribedCb(mqtt->mqtt_message_id);
				}
			}
			else
			{
				mqtt->mqtt_message_type = 0;
				if (mqtt->failsubscribedCb != NULL)
					mqtt->failsubscribedCb(mqtt->mqtt_message_id);
			}
		}
	}

	switch (msg_type)
	{
	case MQTT_MSG_TYPE_PUBLISH:		// 接收到消息
		mqtt_keep_alive_init(mqtt); // 初始化心跳包变量
		if (msg_qos == 1)
		{ // 消息等级是1,打包需要返回的PUBACK数据
			mqtt->mqtt_send_data_len = mqtt_msg_puback(msg_id, &mqtt->ptr, mqtt->mqtt_data_buff, mqtt_send_buff_len);
		}
		else if (msg_qos == 2)
		{ // 消息等级是2,打包需要返回的PUBREC
			mqtt->mqtt_send_data_len = mqtt_msg_pubrec(msg_id, &mqtt->ptr, mqtt->mqtt_data_buff, mqtt_send_buff_len);
		}

		if (msg_qos == 1 || msg_qos == 2)
		{
			if (mqtt->mqtt_send_data_len > 0)
			{
				BufferManageWrite(&mqtt->buff_manage_struct_t, mqtt->ptr, mqtt->mqtt_send_data_len, &DataLen); /*把协议存入存入缓存*/
			}
		}

		// 调用接收回调函数
		if (mqtt->recCb)
		{
			mqtt->topic_length = length;
			mqtt->topic = mqtt_get_publish_topic(buffer, &mqtt->topic_length);
			mqtt->data_length = length;
			mqtt->data = mqtt_get_publish_data(buffer, &mqtt->data_length);

			mqtt->recCb(mqtt->topic, mqtt->topic_length, mqtt->data, mqtt->data_length);
		}
		break;

	case MQTT_MSG_TYPE_PUBACK: // 客户端上次发送了消息等级是1的消息,服务器返回PUBACK,说明消息已经送达
		if (mqtt->mqtt_message_type == MQTT_MSG_TYPE_PUBLISH && mqtt->mqtt_message_id == msg_id)
		{
			mqtt->mqtt_message_type = 0;
			mqtt->timer_out_cnt = 0; // 停止超时定时器
			if (mqtt->PublishedCb)
			{
				mqtt->PublishedCb();
			}
		}
		break;

	case MQTT_MSG_TYPE_PUBREC: // 客户端上次发送了消息等级是2的消息,服务器返回PUBREC,客户端需要返回PUBREL
		mqtt->mqtt_send_data_len = mqtt_msg_pubrel(msg_id, &mqtt->ptr, mqtt->mqtt_data_buff, mqtt_send_buff_len);
		mqtt->timer_out_cnt = 0;
		BufferManageWrite(&mqtt->buff_manage_struct_t, mqtt->ptr, mqtt->mqtt_send_data_len, &DataLen); /*把协议存入存入缓存*/
		break;
	case MQTT_MSG_TYPE_PUBCOMP: // 客户端上次发送了消息等级是2的消息,服务器返回PUBREC,客户端返回了PUBREL,服务器最后返回PUBCOMP,说明消息已经送达
		if (mqtt->mqtt_message_type == MQTT_MSG_TYPE_PUBLISH && mqtt->mqtt_message_id == msg_id)
		{
			mqtt->mqtt_message_type = 0;
			mqtt->timer_out_cnt = 0; // 停止超时定时器
			if (mqtt->PublishedCb)
			{
				mqtt->PublishedCb();
			}
		}
		break;
	case MQTT_MSG_TYPE_PUBREL: // 客户端收到了消息等级为2的消息,同时回复了PUBREC,服务器返回PUBREL,客户端最后需要返回PUBCOMP
		mqtt->mqtt_send_data_len = mqtt_msg_pubcomp(msg_id, &mqtt->ptr, mqtt->mqtt_data_buff, mqtt_send_buff_len);

		BufferManageWrite(&mqtt->buff_manage_struct_t, mqtt->ptr, mqtt->mqtt_send_data_len, &DataLen); /*把协议存入存入缓存*/
		break;
	case MQTT_MSG_TYPE_PINGRESP:	// 接收到心跳包数据
		mqtt_keep_alive_init(mqtt); // 初始化心跳包变量
		break;

	default:
		break;
	}
}

/**
 * @brief   定时轮训函数(该函数需要放在1ms定时器)
 * @param   mqtt:结构体指针
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void mqtt_time_data(mqtt_t *mqtt)
{
	/*MQTT通信超时检测*/
	if (mqtt->timer_out_cnt > 0)
	{
		mqtt->timer_out_cnt--;
	}

	/*发送数据超时检测,为确保发送完一条再发送下一条*/
	if (mqtt->timer_out_send > 0)
	{
		mqtt->timer_out_send--;
	}

	/*MQTT心跳包*/
	if (mqtt->KeepAliveSendCount > 0)
	{
		mqtt->KeepAliveTimeOut++;
	}
	else
	{
		mqtt->KeepAliveTimeOut = 0;
	}

	mqtt->KeepAliveTimeCnt++;
	if (mqtt->KeepAliveTimeCnt / 1000 >= mqtt->mqtt_connect_info.keepalive) // 到时间发送心跳包
	{
		mqtt->KeepAliveSendFlage = 1;
	}
}

/**
 * @brief  处理发送心跳包
 * @param
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void mqtt_keep_alive(mqtt_t *mqtt)
{
	if (mqtt->KeepAliveSendFlage) // 需要发送心跳包
	{
		mqtt_ping(mqtt);
	}
	if (mqtt->KeepAliveTimeOut > 5000) // 心跳包超过时间没有返回应答
	{
		mqtt->KeepAliveTimeOut = 0;
		mqtt_ping(mqtt);
	}
}

/**
 * @brief  初始化心跳包变量
 * @param
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void mqtt_keep_alive_init(mqtt_t *mqtt)
{
	//	mqtt->KeepAliveTimeCnt=0;
	mqtt->KeepAliveSendCount = 0;
	mqtt->KeepAliveTimeOut = 0;
}

/**
 * @brief   初始化MQTT结构体指针
 * @param   mqtt:结构体指针
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void mqtt_init(mqtt_t *mqtt)
{
	BufferManageCreate(&mqtt->buff_manage_struct_t, mqtt->send_buff_loop, send_buff_loop_len, mqtt->send_buff_loop_m, send_buff_loop_m_len * 4);

	mqtt->mqtt_message_type = 0;
	mqtt->mqtt_send_data_len = 0;
	mqtt->state = 0;
	mqtt->timer_out_cnt = 0;
	mqtt_keep_alive_init(mqtt);
}

/**
 * @brief   打包连接MQTT协议
 * @param   mqtt
 * @param   data_ptr:返回打包的数据指针
 * @retval  数据个数
 * @warning None
 * @example
 **/
int mqtt_connect(mqtt_t *mqtt, unsigned char **data_ptr)
{
	return mqtt_msg_connect(&mqtt->mqtt_connect_info, data_ptr, mqtt->mqtt_data_buff, mqtt_send_buff_len);
}

/**
 * @brief  判断是否连接上MQTT
 * @param  服务器返回的数据
 * @param
 * @retval 0 连接成功
 * @example
 **/
int mqtt_connect_ack(unsigned char *buff)
{
	if (mqtt_get_type(buff) == MQTT_MSG_TYPE_CONNACK)
	{
		return mqtt_get_connect_ret_code(buff);
	}
	return -1;
}

/**
 * @brief   注册连接函数
 * @param   mqtt
 * @param   connectedCb
 * @retval  None
 * @warning None
 * @example
 **/
void mqtt_connect_reg(mqtt_t *mqtt, MqttCallback connectCb)
{
	mqtt->connectCb = connectCb;
}

/**
 * @brief   注册断开连接函数
 * @param   mqtt
 * @param   connectedCb
 * @retval  None
 * @warning None
 * @example
 **/
void mqtt_disconnect_reg(mqtt_t *mqtt, MqttCallback disconnectCb)
{
	mqtt->disconnectCb = disconnectCb;
}

/**
 * @brief   注册MQTT接收到数据回调函数
 * @param   mqtt
 * @param   disconnectedCb
 * @retval  None
 * @warning None
 * @example
 **/
void mqtt_received_reg(mqtt_t *mqtt, MqttRecCallback recCb)
{
	mqtt->recCb = recCb;
}

/**
 * @brief   订阅主题
 * @param   mqtt:mqtt_t结构体变量
 * @param   topic:订阅的主题
 * @param   qos:消息等级
 * @param   subscribedCb:订阅成功回调
 * @param   failsubscribedCb:订阅失败回调
 * @param   None
 * @retval  0:Success  1:打包数据错误  2:缓存满
 * @warning None
 * @example
 **/
int mqtt_subscribe(mqtt_t *mqtt, unsigned char *topic, uint8_t qos, MqttCallback1 subscribedCb, MqttCallback1 failsubscribedCb)
{
	mqtt->mqtt_send_data_len = mqtt_msg_subscribe_topic(topic, qos, &mqtt->ptr, mqtt->mqtt_data_buff, mqtt_send_buff_len); /*打包订阅协议*/
	mqtt->subscribedCb = subscribedCb;
	mqtt->failsubscribedCb = failsubscribedCb;
	return mqtt_into_buff(mqtt);
}

/**
 * @brief   发布消息
 * @param   mqtt:mqtt_t结构体变量
 * @param   topic:发布的主题
 * @param   date: 发送的数据
 * @param   data_length:数据长度
 * @param   qos:消息等级
 * @param   retain:是否需要服务器保留消息
 * @param   PublishedCb:发布成功回调(只有qos=1/2时有效)
 * @retval  0:Success  1:打包数据错误  2:缓存满
 * @warning None
 * @example
 **/
int mqtt_publish(mqtt_t *mqtt, unsigned char *topic, unsigned char *date, int data_length, int qos, int retain, MqttCallback PublishedCb)
{
	mqtt->mqtt_send_data_len = mqtt_msg_publish(topic, date, data_length, qos, retain, &mqtt->ptr, mqtt->mqtt_data_buff, mqtt_send_buff_len); /*打包发布协议*/
	mqtt->PublishedCb = PublishedCb;
	return mqtt_into_buff(mqtt);
}

/**
 * @brief   发送心跳包数据
 * @param   mqtt:mqtt_t结构体变量
 * @param   None
 * @param   None
 * @param   None
 * @param   None
 * @param   None
 * @retval  0:Success  1:打包数据错误  2:缓存满
 * @warning None
 * @example
 **/
int mqtt_ping(mqtt_t *mqtt)
{
	int len;
	mqtt->mqtt_send_data_len = mqtt_msg_pingreq(&mqtt->ptr, mqtt->mqtt_data_buff, mqtt_send_buff_len); /*打包协议*/
	len = mqtt_into_buff(mqtt);

	if (len == 0)
	{
		mqtt->KeepAliveTimeCnt = 0;
		mqtt->KeepAliveSendFlage = 0;
		mqtt->KeepAliveSendCount++;
		if (mqtt->KeepAliveSendCount >= 4)
		{
			mqtt->KeepAliveSendCount = 0;
//			if (mqtt->disconnectCb)
//			{
//				mqtt->disconnectCb();
//			}
		}
	}
	return len;
}

/**
 * @brief   把数据插入缓存
 * @param   mqtt
 * @retval  0:Success  1:打包数据错误  2:缓存满
 * @warning None
 * @example
 **/
int mqtt_into_buff(mqtt_t *mqtt)
{
	int len;
	if (mqtt->mqtt_send_data_len > 0)
	{
		BufferManageWrite(&mqtt->buff_manage_struct_t, mqtt->ptr, mqtt->mqtt_send_data_len, &len); /*把协议存入存入缓存*/

		if (len == 0)
		{
			return 0;
		}
		else
		{
			return -2;
		}
	}
	else
	{
		return -1;
	}
}

/**
 * @brief  打包mqtt连接协议
 * @param  data 协议打包完成以后的地址
 * @param  client_id 设备的clientid
 * @param  keepalive 心跳包时间
 * @param  user_name 用户名
 * @param  password  密码
 * @param  clean_session 是否清除 session
 * @param  will_topic    遗嘱发布的主题
 * @param  will_message  遗嘱消息
 * @param  will_qos      遗嘱消息等级
 * @param  will_retained 遗嘱是否需要服务器记录
 * @retval 数据长度
 * @example
 **/
int mqtt_function_connect_msg(mqtt_t *mqtt, unsigned char **data, char *client_id, char *user_name, char *password, int keepalive, char clean_session, char *will_topic, char *will_message, char will_qos, char will_retained)
{
	int len;

	mqtt->mqtt_connect_info.client_id = (unsigned char *)client_id; // client_id
	mqtt->mqtt_connect_info.keepalive = keepalive;					// 心跳包时间
	mqtt->mqtt_connect_info.username = (unsigned char *)user_name;	// 用户名
	mqtt->mqtt_connect_info.password = (unsigned char *)password;	/// 密码
	mqtt->mqtt_connect_info.clean_session = clean_session;			// 是否清除session
	// 如果设置了遗嘱发布的主题 和 遗嘱消息
	if (will_topic != NULL && will_message != NULL)
	{
		mqtt->mqtt_connect_info.will_topic = (unsigned char *)will_topic;	  // 遗嘱发布的主题
		mqtt->mqtt_connect_info.will_message = (unsigned char *)will_message; // 遗嘱的消息
		mqtt->mqtt_connect_info.will_qos = will_qos;						  // 遗嘱的消息等级
		mqtt->mqtt_connect_info.will_retain = will_retained;				  // 是否需要服务器保留消息
	}

	len = mqtt_connect(mqtt, data); // 打包连接信息
	return len;
}

/**
 * @brief  判断是否连接上MQTT(连接成功会调用连接回调函数)
 * @param  data:MQTT服务器返回的数据
 * @param
 * @retval
 * @example
 **/
void mqtt_function_connect_ack(mqtt_t *mqtt, unsigned char *data, uint16_t length)
{
	if (mqtt_connect_ack(data) == 0)
	{
		if (mqtt->connectCb)
		{ // 调用连接回调函数
			mqtt->connectCb();
		}
		// 2022/03/12  如果还有数据可能是服务器发来的数据,数据粘包了
		if (length > 4)
		{
			mqtt_read_function(mqtt, &data[4], length - 4);
		}
	}
}
