#define network_module_c_

#include "network_module.h"

//#define GPIO_PORT (GPIOA)
//#define GPIO_PIN (GPIO_Pin_8)

//#define GPIO_SET (GPIO_SetBits(GPIO_PORT, GPIO_PIN))
//#define GPIO_RESET (GPIO_ResetBits(GPIO_PORT, GPIO_PIN))
//#define GPIO_INPUT (GPIO_ReadOutputDataBit(GPIO_PORT, GPIO_PIN))
//#define GPIO_TOGGLE (GPIO_WriteBit(GPIO_PORT, GPIO_PIN, 1 - GPIO_INPUT))

//#define GPIO_RCC_ENADLE (RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE))

/**
 * @brief  init
 * @param
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
//void network_module_init(void)
//{
//	// GPIO_InitTypeDef GPIO_InitStructure;

//	// GPIO_RCC_ENADLE;						// Clock
//	// GPIO_InitStructure.GPIO_Pin = GPIO_PIN; // pin
//	// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	// GPIO_Init(GPIO_PORT, &GPIO_InitStructure);
//}

/**
 * @brief  network_module_rst_set
 * @param  value 1:ON; 0:OFF
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
//void network_module_rst_set(int value)
//{
//	static char f = 0;
//	if (f == 0)
//	{
//		f = 1;
//		network_module_init();
//	}
//	switch (value)
//	{
//	case 1:
//		GPIO_SET;
//		break;
//	case 0:
//		GPIO_RESET;
//		break;
//	}
//}

char network_config_flage = 0; // 是不是在配网

char network_config_ssid[32] = "";	   // 记录路由器名称
char network_config_password[64] = ""; // 记录密码

char network_config_mac[18] = "";		// 记录设备MAC
char network_config_ip[21] = "0.0.0.0"; // 记录设备连接路由器分得的IP

uint8_t network_flag = 0; // 驻网标志

// 获取模组MAC地址
char network_module_get_mac(char *data, int length)
{
	char flag = 0;
	char *str;

	str = StrBetwString(data, "MAC_CUR:\"", "\""); // 得到MAC

	if (str != NULL)
	{
		if (strlen(str) == 17)
		{
			memset(network_config_mac, 0, sizeof(network_config_mac));
			memcpy(network_config_mac, str, 17);
			remove_colons(network_config_mac);
		}
		flag = 1;
	}
	cStringRestore();
	return flag;
}

// 解析UDP数据
char network_module_udp_data(char *data, int length)
{
	char flag = 0;
	char *str;

	str = StrBetwString(data, "\"ssid\":\"", "\""); // 获取ssid
	if (str != NULL)
	{
		memset(network_config_ssid, 0, sizeof(network_config_ssid));
		sprintf(network_config_ssid, "%s", str);
		cStringRestore();

		str = StrBetwString(data, "\"pwd\":\"", "\""); // 获取pwd
		if (str != NULL)
		{
			memset(network_config_password, 0, sizeof(network_config_password));
			sprintf(network_config_password, "%s", str);
			flag = 1;
		}
	}
	cStringRestore();
	return flag;
}

/**
 * @brief  复位8266
 * @param  None
 * @param  None
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
// void config8266_reset(void)
// {
// 	/*复位模组*/
// 	network_module_rst_set(0);
// 	delay_ms(500);
// 	network_module_rst_set(1);
// 	delay_ms(1000);
// 	delay_ms(1000);
// 	delay_ms(1000);
// }
/*
char smart_config_flage = 0; // 是不是在配网

char config8266_smart_airkiss(void)
{
	int len;
	char *read_data;
	char smart_config_success = 0; // 是否配网成功
	u32 smart_config_cnt = 0;	   // 配网连接路由器延时使用
	network_config_flage = 1;	   // 设置配网标志

	smart_config_flage = 1;

	config8266_reset();
	if (ConfigModuleBlock("+++", 3, "+++", NULL, NULL, 3, 3000))
	{ // 退出透传
		if (ConfigModuleBlock("AT+RESTORE\r\n", 12, "ready", NULL, NULL, 3, 3000))
		{ // 恢复出厂设置
			if (ConfigModuleBlock("AT+CWMODE_DEF=1\r\n", 17, "OK", NULL, NULL, 3, 3000))
			{ // STA 模式
				if (ConfigModuleBlock("AT+CWAUTOCONN=1\r\n", 17, "OK", NULL, NULL, 3, 3000))
				{																				   // 自动连接路由器
					if (ConfigModuleBlock("AT+CWSTARTSMART=3\r\n", 19, "OK", NULL, NULL, 3, 3000)) //(??SmartConfig+AirKiss)
					{
						smart_config_cnt = 0;
						while (1)
						{
							IWDG_Feed();
							// 不断的获取模块返回的数据
							len = ConfigModuleBlockRead(&read_data);

							if (len > 0)
							{
								if (strstr(read_data, "WIFI CONNECTED") || strstr(read_data, "WIFI GOT IP"))
								{
									smart_config_success = 1;
									printf("aaaaaaaaaaaaaaaa");
									break;
								}
							}
							// 30S超时
							delay_ms(1);
							smart_config_cnt++;
							if (smart_config_cnt > 30000)
							{
								smart_config_success = 0;
								break;
							}
						}

						smart_config_cnt = 0;
						// 如果配网成功,延时一段时间,让模组把MAC地址信息传给APP
						if (smart_config_success)
						{
							while (smart_config_cnt < 5000)
							{
								delay_ms(1);
								smart_config_cnt++;
								IWDG_Feed();
							}
							ConfigModuleBlock("AT+CWSTOPSMART\r\n", 16, "OK", NULL, NULL, 3, 3000);
						}
					}
				}
			}
		}
	}

	config8266_reset();
	smart_config_flage = 0;
	network_config_flage = 0; // 清除配网标志
	return smart_config_success;
}
 */
extern char location_flag;
/**
 * @brief  启用APUConfig 给WIFI配网
 * @ warn  None
 * @param  None
 * @param  None
 * @param  None
 * @param  None
 * @retval 1:成功
 * @example
 **/
char network_module_apuconfig(void)
{
	int i = 0;
	char data[100];
	int len = 0;
	int config_success = 0;
	//	u32 temp[100];

	network_config_flage = 1; // 设置配网标志
	/*复位模组*/
	// network_module_rst_set(0);
	// delay_ms(500);
	// network_module_rst_set(1);
	// delay_ms(1000);
	// delay_ms(1000);
	// delay_ms(1000);

	if (ConfigModuleBlock("AT\r\n", 4, "OK", NULL, NULL, 3, 3000))
	{
		// if(ConfigModuleBlock("AT+RESTORE\r\n",12,"ready",NULL, NULL, 3, 3000))//恢复出厂设置
		//{
		if (ConfigModuleBlock("ATE0\r\n", 6, "OK", NULL, NULL, 3, 3000)) // 关闭回显
		{
			if (ConfigModuleBlock("AT+CWMODE_DEF=2\r\n", 17, "OK", NULL, NULL, 3, 3000)) // 模式2
			{
				if (ConfigModuleBlock("AT+CIPSTAMAC_CUR?\r\n", 19, NULL, NULL, network_module_get_mac, 3, 3000)) // MAC
				{
					len = sprintf(data, "AT+CWSAP_DEF=\"wifi_8266_bind\",\"11223344\",11,4,4\r\n");
					if (ConfigModuleBlock(data, len, "OK", NULL, NULL, 3, 3000)) // 配置发出的无线
					{
						len = sprintf(data, "%s", "AT+CIPSTART=\"UDP\",\"192.168.4.2\",5555,5556,2\r\n");
						if (ConfigModuleBlock(data, len, "OK", NULL, network_module_udp_data, 3, 3000)) // 配置UDP
						{
							if (ConfigModuleBlock(NULL, 0, NULL, NULL, network_module_udp_data, 20, 3000)) // 解析接收的UDP数据
							{
								for (i = 0; i < 3; i++) // 连续发送3次数据
								{
									//{"mac":"XX:XX:XX:XX:XX:XX","ip":"192.168.10.2"}
									len = sprintf(data, "{\"mac\":\"%s\",\"ip\":\"%s\"}", mqtt_client_id, network_config_ip);
									// 发送数据命令
									len = sprintf(data, "AT+CIPSEND=%d\r\n", len);
									ConfigModuleBlock(data, len, ">", NULL, NULL, 1, 50); // 发送一次,超时等待50ms(等待返回>)

									// 发送数据
									len = sprintf(data, "{\"mac\":\"%s\",\"ip\":\"%s\"}", mqtt_client_id, network_config_ip);
									ConfigModuleBlock(data, len, NULL, NULL, NULL, 1, 500); // 发送数据,超时等待500ms
								}
								ConfigModuleBlock("AT+CWMODE_DEF=1\r\n", 17, "OK", NULL, NULL, 3, 3000);		
								/*连接路由器*/
								len = sprintf(data, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", network_config_ssid, network_config_password);
								if (ConfigModuleBlock(data, len, "WIFI CONNECTED", "WIFI GOT IP", NULL, 3, 3000)) // 设置连接的路由器
								{
									config_success = 1; // 配置成功
									network_flag = 1;	// 驻网标志
														//  location_flag = 1;
								}
							}
						}
					}
				}
			}
		}
		//}//
	}
	/*设置为模式1, 主要用于关闭热点*/
	ConfigModuleBlock("AT+CWMODE_DEF=1\r\n", 17, "OK", NULL, NULL, 3, 3000); // 模式1
	network_config_flage = 0;												 // 清除配网标志
	return config_success;
}

char NETRECV_FLAG = 0;
char NETRECV_CNT = 0;
char NETRECV_COUNT = 0;
char NETRECV_START = 0;
char NETRECV_ID = 0;
char NETRECV_BUFF[20];
char NETRECV_BUFF_LEN = 0;
int NETRECV_DATA_LEN = 0; //

/**
 * @brief  解析NB模组返回的网络数据
 * @param  Res 模组返回的数据
 * @param  return_data 返回解析之后的数据(只保留网络数据)
 * @param  return_id 消息id(socket 编号)
 * @retval 1: 解析成功
 * @example
 **/
char network_module_extract_data(char Res, char *return_data, char *return_id)
{
	char value = 0;
	/*返回网络数据*/
	if (NETRECV_FLAG == 1)
	{
		if (NETRECV_DATA_LEN > 0)
		{ // 还没结束
			NETRECV_DATA_LEN--;
			value = 1;
			*return_data = Res;
		}
		else
		{ // 数据结束
			NETRECV_FLAG = 0;
			NETRECV_DATA_LEN = 0;
			NETRECV_START = 0;
		}
	}

	/****接到的数据个数****/
	if (NETRECV_START == 1)
	{
		if (NETRECV_COUNT >= 0 && NETRECV_COUNT <= 5)
		{
			NETRECV_COUNT++;
			if (Res == ':')
			{
				NETRECV_START = 0;
				NETRECV_COUNT = 0;
				NETRECV_FLAG = 1;
			}
			else
			{
				if (Res >= '0' && Res <= '9')
				{
					NETRECV_DATA_LEN = NETRECV_DATA_LEN * 10;
					NETRECV_DATA_LEN = NETRECV_DATA_LEN + (Res - 0x30);
				}
			}
		}
		else
		{
			NETRECV_COUNT = 0;
		}
	}

	/*检测数据开头*/
	if (Res == '+' && NETRECV_CNT == 0)
		NETRECV_CNT++;
	else if (Res == 'I' && NETRECV_CNT == 1)
		NETRECV_CNT++;
	else if (Res == 'P' && NETRECV_CNT == 2)
		NETRECV_CNT++;
	else if (Res == 'D' && NETRECV_CNT == 3)
		NETRECV_CNT++;
	else if (Res == ',' && NETRECV_CNT == 4)
		NETRECV_CNT++;
	else if (Res >= '0' && Res <= '9' && NETRECV_CNT == 5)
	{
		NETRECV_CNT++;
		NETRECV_ID = Res;
		if (return_id != NULL)
			*return_id = NETRECV_ID;
	}
	else if (Res == ',' && NETRECV_CNT == 6)
	{
		NETRECV_CNT = 0;
		NETRECV_START = 1;
		NETRECV_COUNT = 0;
		NETRECV_DATA_LEN = 0;
	}
	else
	{
		if (Res == '+' && NETRECV_CNT == 1)
			NETRECV_CNT = 1;
		else
			NETRECV_CNT = 0;
	}

	return value;
}