//2025.11.24  wifi模式下不上传起卧次数，运行一段时间后卡住，调整wifi模式下上传网络模式次数之前为一直上传导致占用上传buffer，程序奔溃和上传其他信息失败。

//2025.11.25  网络模式没有上传给服务器更新，调整网络模式上传次数为5次。

//2025.11.29  发现wifi模式在配网的时候经常失败或者太慢，由原来的模式3改为纯AP模式，模式2。收到账号密码后连接路由器的时候配置为模式1

//2025.12.11  测试wifi长时间连接是否会掉线

//2025/12/13 找到wifi掉线问题串口接收处没有解析发送ping包后服务器返回的数据，导致程序判断ping包没有发送成功，主动复位模块
//mqtt.c文件511行
//int mqtt_ping(mqtt_t *mqtt)
//{
//	int len;
//	mqtt->mqtt_send_data_len = mqtt_msg_pingreq(&mqtt->ptr, mqtt->mqtt_data_buff, mqtt_send_buff_len); /*打包协议*/
//	len = mqtt_into_buff(mqtt);

//	if (len == 0)
//	{
//		mqtt->KeepAliveTimeCnt = 0;
//		mqtt->KeepAliveSendFlage = 0;
//		mqtt->KeepAliveSendCount++;
//		if (mqtt->KeepAliveSendCount >= 4)
//		{
//			mqtt->KeepAliveSendCount = 0;  
////			if (mqtt->disconnectCb)                这个地方判断导致模块主动断开TCP
////			{
////				mqtt->disconnectCb();
////			}
//		}
//	}
//	return len;
//}
//2026.1.21 返场发现如果出厂有些wifi模组有问题，上电直接启用4G，4G小板启动需要时间，错过程序发送的mqttopen命令导致不上线，在4G模式发送mqttopen前添加15s延时
//2026.1.27 升级硬件为V3.1版本，替换风机控制逻辑，加热逻辑，删去PWM，尝试加入OTA

#include "stm32f0xx.h"
#include "LED3.h"
#include "Blower_Control.h"
#include "PWM.h"
#include "Timer.h"
#include "Key.h"

#include "usart.h"
#include "Delay.h"

#include "ConfigModuleNoBlock.h"

#include "mqtt.h"

#include "tcp_mqtt.h"

#include "timer.h"
#include "iwdg.h"
#include "ML307.h"
#include "MyFlash.h"

// extern uint8_t Flag_Sensor_Up, Flag_Sensor_Down1, Flag_Sensor_Down2; // 三个传感器有效标志位
extern uint8_t Flag_Heat;	// 加热开启关闭标志位：0/1
extern uint8_t Flag_Update; // 状态更新标志位（任意状态更新：传感器、热机、风机结束）

extern uint16_t Getup_Num; // 母猪起身次数（用于上传）
extern uint8_t Flag_Warn;  // 报警标志位（红灯亮）

int main(void)
{
		int len = 0;
		uint8_t Querynetwork_mode = 1;
		uint8_t network_apuconfig_flag = 1;

		TIM_KeyScan_Init(); // 定时器初始化
		Infrared_Init();	// 红外传感器初始化
		LED3_Init();		// LED初始化
		TIM3_Init();
		TIM_Cmd(TIM3, ENABLE);
	
		Heat_Init();
		Heat_Key_Init();
		Blower_Init();//风机初始化
	
//		Key_Init();//风速控制
//		PWM_Init();
		uart_init(115200, 115200,115200);
		EXTI_Init_Config(); // 外部中断初始化
		tcp_mqtt_init(); // 初始化mqtt

		Blower_ON();// 打开吹风机
		Frist_Run = 1;
		
		if (flash_read() == 1 && FlashData[0] != -1 && FlashData[1] != -1 && FlashData[2] != -1) // initialized
		{
			WorkMode temp1 = 0;
			uint8_t temp2 = 0;
			uint8_t temp3 = 0;
			memcpy(&temp1, &FlashData[0], sizeof(WorkMode));
			memcpy(&temp2, &FlashData[1], sizeof(uint8_t));
			memcpy(&temp3, &FlashData[2], sizeof(uint8_t));
			work_mode = temp1;
			ESP_Flag_Blower = temp2;
			ESP_Flag_Heat = temp3;
		}
		else //unitialized
		{
			uint32_t temp1 = 0;
			uint32_t temp2 = 0;
			uint32_t temp3 = 0;
			work_mode = POSTPARTUM_MODE; // 默认为产后模式
			ESP_Flag_Blower = 1;		 // 默认风遁打开
			ESP_Flag_Heat = 0;			 // 默认加热关闭
			memcpy(&temp1, &work_mode, sizeof(WorkMode));
			memcpy(&temp2, &ESP_Flag_Blower, sizeof(uint8_t));
			memcpy(&temp3, &ESP_Flag_Heat, sizeof(uint8_t));

			FlashData[0] = temp1;
			FlashData[1] = temp2;
			FlashData[2] = temp3;
		// 初始化IMEI存储为0（表示未存储）
			for (uint8_t i = FLASH_IMEI_INDEX; i < 7; i++)
			{
					FlashData[i] = 0;
			}
			
			Write_Flash(FlashData, 7); // 写入7个数据   
		}
		
		
		Frist_Run_Time = 2000;

	while (1)
	{
			imei_main_loop_process();
//			Key_Scan_And_Adjust_Speed();
		
		if (Frist_Run == 0) // 自检结束
		{
			if (Querynetwork_mode == 1) // 查询网络模块,每次上电检查一次，防止一错永错
			{
				/* code */
				if (ConfigModuleBlock("AT\r\n", 5, "OK", NULL, NULL, 3, 1000)) // 如果发AT有返回
				{
					network_mode = 1; // 则网络模块为WIFI
				}
				Querynetwork_mode = 0; // 自检标志清零
			}
			else // 查询结束
			{
				if (network_mode == 0)// 网络模块为4G
				{  
					
					if (mqttopen_sent_flag == 0)
					{
						if(delay_15s == 15000)
						{
							usart1_send_mqttopen(); // 发送"mqttopen"
							mqttopen_sent_flag = 1; // 置位发送标志，确保只发一次				
						}							
					}
					
					/* code */
					if (work_mode == POSTPARTUM_MODE)
					{
						/* code */
						Heat_Control();	  // 加热标志位确定
						Blower_Control(); // 风机、热机开关控制
						LED3_Control();	  // 灯控制
						SendControl1();	  // 向服务器发送控制
						Translate_JSON(); // 解析307发来的数据
						
					if (mqttopen_sent_flag == 1 && network_sent_flag == 0)
					{
							if (delay_3s_cnt >= 4000) // delay_3s_cnt=3000 → 3秒（1ms*3000）
							{
									SendNetworkmode1();       // 执行发送
									network_sent_flag = 1;    // 标记已发送，避免重复
									delay_3s_cnt = 0;         // 清零计数器，防止后续溢出
							}
					}
	
					}
					if (work_mode == PRODUCTION_MODE)
					{
						/* code */
						LED3_Control(); // 灯控制
						SendControl1();
						Translate_JSON(); // 解析307发来的数据
					}
				}

				else if (network_mode == 1) // 网络模块为WIFI
				{
					/* code */
					if (work_mode == POSTPARTUM_MODE)
					{
						/* code */
						Heat_Control();	  // 加热标志位确定
						Blower_Control(); // 风机、热机开关控制
						LED3_Control();	  // 灯控制
						SendControl();	  // 向服务器发送控制
			
					}
					if (work_mode == PRODUCTION_MODE)
					{
						/* code */
						LED3_Control(); // 灯控制
						SendControl();
					}

					if (QueryForNetworkFlags == 1)
					{
						/* code */
						QueryForNetworkFlags = 0;
						if (ConfigModuleBlock("AT+CIPSTATUS\r\n", 15, "STATUS:5", "STATUS:4", NULL, 1, 1000)) // 未联网
						{
							/* code */

							network_flag = 0;
							if (network_apuconfig_flag == 1)
							{
								/* code */
								printf("network_module_apuconfig\r\n");
								network_apuconfig_flag = 0;
								char apu_config_result = network_module_apuconfig();
								if (apu_config_result == 0)
								{
                network_mode = 0; // 切4G
                mqttopen_sent_flag = 0; // 重置4G的发送标志，确保能发"mqttopen"
								}
							}
						}
						else // 已联网
						{
							network_flag = 1;
							network_apuconfig_flag = 0;
						}
					}

					if (network_flag == 1)
					{	
						ConfigModuleNoBlock();

						tcp_mqtt_while(); // 和TCP服务器通信

						if (usart4_idle_flag == 1) 
							{
						usart4_idle_flag = 0;
						len = rbCanRead(&rb_t_usart4_read);
						if (len > 0) {
								memset(usart4_read_buff_copy, 0, rb_t_usart4_read_buff_len);
								rbRead(&rb_t_usart4_read, usart4_read_buff_copy, len);

								// 后续解析逻辑（与原串口1一致，如检测断开、调用ConfigConnectDispose等）
								if (strstr((char *)usart4_read_buff_copy, "0,CLOSED\r\n")) 
									{
										mqtt_connect_flag = 0;
										ConfigModuleNoBlockCaseValue = 0;
										ConfigModuleNoBlockFlage = 0;
								}
								if (ConfigConnectDispose != NULL)
									{
										ConfigConnectDispose((char *)usart4_read_buff_copy, len);
								}

								/*获取TCP数据*/
								len = rbCanRead(&rb_t_net_read); // 获取模组返回的TCP数据
								if (len > 0)
								{
									memset(usart4_read_buff_copy, 0, rb_t_usart4_read_buff_len);
									rbRead(&rb_t_net_read, usart4_read_buff_copy, len);

									tcp_mqtt_into_data((char *)usart4_read_buff_copy, len);
								}
							}
						}
					}
				}
			}
		}
		
	} 
}
