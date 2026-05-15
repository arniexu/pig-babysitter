/**
  ******************************************************************************
  * @file    usart.c
  * @author  fengwu yang
  * @version V1.0.0
  * @date    2019/10/12
  * @brief   https://www.cnblogs.com/yangfengwu/p/11674814.html
  ******************************************************************************
	一,使用说明
		1,把以下程序放在1ms定时器中断中
			ConfigModuleNoBlockCnt++;

		2,把以下程序放到主函数
		while(1)
		{
			ConfigModuleNoBlock();//配置模块

			if(串口接收到一条完整的数据)
			{
			  ConfigConnectDispose(接收的数据地址,接收的数据个数);
			}
		}

  ******************************************************************************
  */

#define CONFIGMODULENOBLOCK_C_
#include "ConfigModuleNoBlock.h"

#include "usart.h"
#include "tcp_mqtt.h"

/*********************************************************************************************************/
config_module_noblock_context_t g_config_module_noblock_context = {
	.cnt = 0,
	.flage = 0,
	.case_value = 0,
	.compare_value = 5000,
	.send_next_delay = 0,
	.retry_cnt = 0,
	.data_return_flage = 0,
	.hope_return_data1 = "",
	.hope_return_data2 = "",
	.connect_dispose = NULL,
};

void ConfigModuleRunNext(int delay);
/*********************************************************************************************************/

/**
 * @brief  //获取唯一ID,WIFI获取MAC,GPRS获取IMEI
 * @param  data
 * @param
 * @retval
 * @example
 **/
//void FunctionParseGetID(char *data, int length)
//{
//	char *str;
//	str = StrBetwString(data, "MAC_CUR:\"", "\"");

//	if (strlen((char *)str) == 17)
//	{
//		if (strlen(mqtt_client_id) == 0)
//		{
//			memset(mqtt_client_id, 0, sizeof(mqtt_client_id));
//			memcpy(mqtt_client_id, str, 17);
//			remove_colons(mqtt_client_id);
//		}

//		ConfigModuleRunNext(CompareValue); // 执行下一条
//	}
//	cStringRestore();
//}

/**
 * @brief  发送指令配置模块,非阻塞版
 * @waring
 * @param  None
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/

// void espstart()
// {
// 	network_module_rst_set(0);
// 	delay_ms(500);
// 	network_module_rst_set(1);
// 	delay_ms(1000);
// 	delay_ms(1000);
// 	delay_ms(1000);
// }
void ConfigModuleNoBlock(void)
{

	char buff[500];
	int len;

	if (ConfigModuleNoBlockCnt > CompareValue && ConfigModuleNoBlockFlage == 0)
	{
		ConfigModuleNoBlockCnt = 0;
		if (DataReturnFlage == 1) // 上一条指令是OK的
		{
			Cnt = 0;
			DataReturnFlage = 0;
			ConfigModuleNoBlockCaseValue++; // 执行下一条
		}
		else
		{
			Cnt++;
			if (Cnt >= 3) // 超过3次重新执行
			{
				Cnt = 0;
				ConfigModuleNoBlockCaseValue = 0;
				// RTC_Enter_StandbyMode(60*120);//进入待机
			}
		}
		switch (ConfigModuleNoBlockCaseValue)
		{
		case 0: //
			SendConfigFunction("AT+RST\r\n", NULL, "OK", NULL, FunctionParseCompare, CompareValue);
			Delay_s(5);
			break;
		case 1: //
			SendConfigFunction("AT\r\n", NULL, "OK", NULL, FunctionParseCompare, CompareValue);
			break;
		case 2: // 关闭回显
			SendConfigFunction("ATE0\r\n", NULL, "OK", NULL, FunctionParseCompare, CompareValue);
			break;
		case 3: // 设置非透传模式
			SendConfigFunction("AT+CIPMODE=0\r\n", NULL, "OK", NULL, FunctionParseCompare, CompareValue);
			break;
		case 4: // 多链接
			SendConfigFunction("AT+CIPMUX=1\r\n", NULL, "OK", NULL, FunctionParseCompare, CompareValue);
			break;
		case 5: // 连接TCP
			len = sprintf((char *)buff, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d\r\n", tcp_mqtt_index, tcp_mqtt_addr, tcp_mqtt_port);
			buff[len] = 0;
			SendConfigFunction(buff, NULL, "CONNECT", "ALREADY CONNECTED", FunctionParseCompare, CompareValue);
			break;
		default:
			SendConfigFunction(NULL, NULL, NULL, NULL, NULL, CompareValue); // 这句必须加,清除所有的执行函数
			DataReturnFlage = 0;
			ConfigModuleNoBlockFlage = 1;
			break;
		}
	}
}
/* void ConfigModuleNoBlock(void)
{
	char buff[500];
	int len;
	if (ConfigModuleNoBlockCnt > CompareValue && ConfigModuleNoBlockFlage == 0)
	{
		ConfigModuleNoBlockCnt = 0;
		if (DataReturnFlage == 1) // 上一条指令是OK的
		{
			Cnt = 0;
			DataReturnFlage = 0;
			ConfigModuleNoBlockCaseValue++; // 执行下一条
		}
		else
		{
			Cnt++;
			if (Cnt >= 3) // 超过3次重新执行
			{
				Cnt = 0;
				ConfigModuleNoBlockCaseValue = 0;
				// RTC_Enter_StandbyMode(60*120);//进入待机
			}
		}
		switch (ConfigModuleNoBlockCaseValue)
		{
		case 0: // 关闭模块
			network_module_rst_set(0);
			ConfigModuleRunNext(CompareValue - 1500);
			break;
		case 1: // 启动模块
			network_module_rst_set(1);
			ConfigModuleRunNext(0);
			break;
		case 2: //
			SendConfigFunction("AT\r\n", NULL, "OK", NULL, FunctionParseCompare, CompareValue);
			break;
		case 3: // 关闭回显
			SendConfigFunction("ATE0\r\n", NULL, "OK", NULL, FunctionParseCompare, CompareValue);
			break;
		case 4: // 获取唯一ID,WIFI获取MAC,GPRS获取IMEI
			SendConfigFunction("AT+CIPSTAMAC_CUR?\r\n", NULL, NULL, NULL, FunctionParseGetID, CompareValue);
			break;
		case 5: // 设置非透传模式
			SendConfigFunction("AT+CIPMODE=0\r\n", NULL, "OK", NULL, FunctionParseCompare, CompareValue);
			break;
		case 6: // 多链接
			SendConfigFunction("AT+CIPMUX=1\r\n", NULL, "OK", NULL, FunctionParseCompare, CompareValue);
			break;
		case 7: // 连接TCP
			len = sprintf((char *)buff, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d\r\n", tcp_mqtt_index, tcp_mqtt_addr, tcp_mqtt_port);
			buff[len] = 0;
			SendConfigFunction(buff, NULL, "CONNECT", "ALREADY CONNECTED", FunctionParseCompare, CompareValue);
			break;
		default:
			SendConfigFunction(NULL, NULL, NULL, NULL, NULL, CompareValue); // 这句必须加,清除所有的执行函数
			DataReturnFlage = 0;
			ConfigModuleNoBlockFlage = 1;
			break;
		}
	}
} */

/**
* @brief  执行下一条
* @param  delay 延时多少时间再执行下一条
* @param
* @retval
* @example
延时1000ms再执行下一条
ConfigModuleRunNext(CompareValue-1000);
立即执行下一条
ConfigModuleRunNext(CompareValue);
**/
void ConfigModuleRunNext(int delay)
{
	SendNextDelay = delay;
	DataReturnFlage = 1;
	ConfigModuleNoBlockCnt = (SendNextDelay == 0 ? ConfigModuleNoBlockCnt : SendNextDelay);
}

/**
 * @brief  发送配置指令函数
 * @param  order          发送数据(字符串)
 * @param  FunctionSend   发送数据(运行发送数据函数)
 * @param  HopeReturn1    接收数据(希望返回的字符串1)
 * @param  HopeReturn2    接收数据(希望返回的字符串2)
 * @param  FunctionParse  数据处理函数
 * @param  ConfigFunctionValue  控制下一条数据的发送时间,取值为 ConfigFunctionCntCom:下一条立即发送
 * @retval
 * @example
 **/
void SendConfigFunction(char *order, void (*FunctionSend)(), char *HopeReturn1, char *HopeReturn2, void (*FunctionParse)(char *data, int len), uint32_t ConfigFunctionValue)
{
	memset(HopeReturnData1, NULL, strlen(HopeReturnData1));
	memset(HopeReturnData2, NULL, strlen(HopeReturnData2));
	if (HopeReturn1 != NULL)
		sprintf(HopeReturnData1, "%s", HopeReturn1); // 拷贝数据到数组 HopeReturn1,希望返回的数据1
	if (HopeReturn2 != NULL)
		sprintf(HopeReturnData2, "%s", HopeReturn2); // 拷贝数据到数组 HopeReturn1,希望返回的数据2
	if (FunctionSend != NULL)
		FunctionSend(); // 调用一个函数发送指令

	ConfigConnectDispose = FunctionParse; // 传递处理函数指针
	SendNextDelay = ConfigFunctionValue;  // 如果上一条数据处理成功,控制发送下一条数据的时间

	if (order != NULL)
	{
		usart4_send(order, strlen(order));
	}
}

/**
 * @brief  处理配置返回数据:判断是否返回想要的数据
 * @param  data
 * @param
 * @retval
 * @example
 **/
void FunctionParseCompare(char *data, int len)
{
	if (strlen(HopeReturnData1) != 0 && strstr(data, HopeReturnData1))
	{
		DataReturnFlage = 1;
		ConfigModuleNoBlockCnt = (SendNextDelay == 0 ? ConfigModuleNoBlockCnt : SendNextDelay);
	}
	if (strlen(HopeReturnData2) != 0 && strstr(data, HopeReturnData2))
	{
		DataReturnFlage = 1;
		ConfigModuleNoBlockCnt = (SendNextDelay == 0 ? ConfigModuleNoBlockCnt : SendNextDelay);
	}
}
