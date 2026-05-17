#ifndef __USART_H
#define __USART_H

#include <stdio.h>

#include <stm32f0xx.h>

#include "LoopList.h"

#include "network_module.h"

#include "tcp_mqtt.h"

typedef struct
{
	int usart1_read_count;
	int usart1_read_count_copy;
	int usart1_read_idle_count;
	char usart1_idle_flag;
	int usart2_read_count;
	unsigned char usart2_idle_flag;
	int usart4_read_count;
	int usart4_read_count_copy;
	int usart4_read_idle_count;
	char usart4_idle_flag;
	unsigned char imei_buff[16];
	unsigned char imei_receive_cnt;
	int imei_receive_done;
	int imei_receiving_en;
} usart_runtime_context_t;

extern usart_runtime_context_t g_usart_runtime_context;

#define usart1_read_count (g_usart_runtime_context.usart1_read_count)
#define usart1_read_count_copy (g_usart_runtime_context.usart1_read_count_copy)
#define usart1_read_idle_count (g_usart_runtime_context.usart1_read_idle_count)
#define usart1_idle_flag (g_usart_runtime_context.usart1_idle_flag)
#define usart2_read_count (g_usart_runtime_context.usart2_read_count)
#define usart2_idle_flag (g_usart_runtime_context.usart2_idle_flag)
#define usart4_read_count (g_usart_runtime_context.usart4_read_count)
#define usart4_read_count_copy (g_usart_runtime_context.usart4_read_count_copy)
#define usart4_read_idle_count (g_usart_runtime_context.usart4_read_idle_count)
#define usart4_idle_flag (g_usart_runtime_context.usart4_idle_flag)
#define imei_buff (g_usart_runtime_context.imei_buff)
#define imei_receive_cnt (g_usart_runtime_context.imei_receive_cnt)
#define imei_receive_done (g_usart_runtime_context.imei_receive_done)
#define imei_receiving_en (g_usart_runtime_context.imei_receiving_en)

/****************************串口1****************************/

/***串口接收环形队列****/
// 环形队列变量
extern rb_t rb_t_usart1_read;
#define rb_t_usart1_read_buff_len 200
// 环形队列缓存
extern unsigned char rb_t_usart1_read_buff[rb_t_usart1_read_buff_len];
// 从缓存拷贝数据使用
extern unsigned char usart1_read_buff_copy[rb_t_usart1_read_buff_len];

/*******************************************usart2********************************************/

// 接收环形队列
extern rb_t rb_t_usart2_read;
#define rb_t_usart2_read_buff_len 100
extern unsigned char rb_t_usart2_read_buff[rb_t_usart2_read_buff_len];
// 从缓存拷贝数据使用
extern unsigned char usart2_read_buff_copy[rb_t_usart2_read_buff_len];

// 发送环形队列
extern rb_t rb_t_usart2_send;
#define rb_t_usart2_send_buff_len 400
extern unsigned char rb_t_usart2_send_buff[rb_t_usart2_send_buff_len];
extern unsigned char rb_t_usart2_send_byte; // 串口提取环形队列1个字节

/*******************************************缓存网络通信数据********************************************/
// 环形队列变量
extern rb_t rb_t_net_read;
#define rb_t_net_read_buff_len 512
// 环形队列缓存
extern unsigned char rb_t_net_read_buff[rb_t_net_read_buff_len];


//10.16修改添加串口四
extern rb_t rb_t_usart4_read;
#define rb_t_usart4_read_buff_len 200
extern unsigned char rb_t_usart4_read_buff[rb_t_usart4_read_buff_len];
extern unsigned char usart4_read_buff_copy[rb_t_usart4_read_buff_len];

void usart1_send_mqttopen(void);
void imei_main_loop_process(void);
/**
 * @brief   串口初始化
 * @param   bound  波特率
 * @param   None
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void uart_init(uint32_t bound1, uint32_t bound2,uint32_t bound3);

/**
 * @brief  串口中断发送数据
 * @param  c:数据的首地址  cnt:发送的数据个数
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void usart2_send_it(char *c, uint32_t cnt);

/**
 * @brief  串口发送字符串数据
 * @param  *c:发送的数据指针  cnt:数据个数
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void usart1_send(char *c, uint32_t cnt);
void usart4_send(char *c, uint32_t cnt);
/**
 * @brief  串口1自定义空闲中断检测(放到1ms定时器)
 * @param  value: 空闲时间
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void usart1_idle_loop(int value);
void usart4_idle_loop(int value);
/**
 * @brief  串口发送字符串数据
 * @param  *c:发送的数据指针  cnt:数据个数
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void usart2_send(char *c, uint32_t cnt);

#endif
