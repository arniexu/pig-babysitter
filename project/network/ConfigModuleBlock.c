/**
  ******************************************************************************
  * @file    usart.c
  * @author  fengwu yang
  * @version V1.0.0
  * @date    2019/10/12
  * @brief
  ******************************************************************************
	一,使用说明
		1,把以下程序放在1ms定时器中断中
			ConfigModuleBlockDelay++;
		2,使用
		//发送AT\r\n,希望串口返回的数据是 OK,
		ConfigModuleBlock("AT\r\n","OK",NULL) //判断返回了OK,或者该指令发送了3次,执行下一条

		//发送AT+RST\r\n,希望串口返回的数据是 OK,
		ConfigModuleBlock("AT+RST\r\n","ready","OK") //判断返回了OK,或者返回了"ready",或者该指令发送了3次,执行下一条

		//接收返回的配置状态
		flage = ConfigModuleBlock("AT\r\n","OK",NULL);//flage:1  配置OK
  ******************************************************************************
  */

#define CONFIGMODULEBLOCK_C_

#include "ConfigModuleBlock.h"

int ConfigModuleBlockCnt = 0;
int ConfigModuleBlockFlage = 1;
int ConfigModuleBlockDelay = 0;
char *ConfigModuleBlockData;

/**
 * @brief  接收数据(把数据接收的数据)
 * @param  none
 * @param  none
 * @param  none
 * @retval none
 * @example
 **/
int ConfigModuleBlockRead(char **data)
{
	int len = 0;
	/*替换自己的接收数据函数----*/
	memset(usart4_read_buff_copy, 0, rb_t_usart4_read_buff_len);
	if (usart4_idle_flag)
	{
		usart4_idle_flag = 0;
		// 读取缓存数据个数
		len = rbCanRead(&rb_t_usart4_read);
		if (len > 0)
		{
			rbRead(&rb_t_usart4_read, usart4_read_buff_copy, len);
			usart4_read_buff_copy[len] = 0;
		}
	}
	// 把接收的数据地址赋值给 data(用户根据自己的修改)
	*data = (char *)usart4_read_buff_copy;

	return len;
}

/**
 * @brief  发送指令配置模块(内部替换自己的发送函数)
 * @param  none
 * @param  none
 * @param  none
 * @retval none
 * @example
 **/
void ConfigModuleBlockSend(char *dat, int len)
{
	usart4_send(dat, len);
}

/**
 * @brief  发送指令配置模块,阻塞版
 * @param  dat:      发送的数据
 * @param  returnc:  预期返回的数据1
 * @param  returncc: 预期返回的数据2
 * @param  执行一个函数
 * @param  send_cnt: 尝试次数
 * @param  send_cnt: 尝试间隔时间(ms)
 * @retval 1:配置当前指令OK
 * @example
 **/
char ConfigModuleBlock(char *send_data, int send_len, char *returnc, char *returncc, char (*fun)(char *fun_data, int fun_data_len), int send_cnt, int timet_out)
{
	int len;
	ConfigModuleBlockCnt = 0;
	ConfigModuleBlockFlage = 1;
	while (1)
	{
		// IWDG_Feed();
		if (ConfigModuleBlockFlage == 1) // 发送指令
		{
			ConfigModuleBlockDelay = 0;
			ConfigModuleBlockFlage = 0;
			if (send_data != NULL && send_len != 0)
				ConfigModuleBlockSend(send_data, send_len);
		}

		if ((len = ConfigModuleBlockRead(&ConfigModuleBlockData)) != 0)
		{
			if (returnc != NULL && strstr(ConfigModuleBlockData, returnc)) // 比较数据
			{
				return 1;
			}
			if (returncc != NULL && strstr(ConfigModuleBlockData, returncc)) // 比较数据
			{
				return 1;
			}
			if (fun != NULL)
			{
				if (fun(ConfigModuleBlockData, len) == 1)
				{
					return 1;
				}
			}
		}

		if (ConfigModuleBlockDelay >= timet_out) // 超过3S
		{
			ConfigModuleBlockDelay = 0;
			ConfigModuleBlockFlage = 1; // 允许发送数据
			ConfigModuleBlockCnt++;
		}
		if (ConfigModuleBlockCnt >= send_cnt) // 超过三次继续发送下一条
		{
			ConfigModuleBlockCnt = 0;
			return 0;
		}
	}
}
