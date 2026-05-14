#ifndef __USART_H
#define __USART_H

#ifndef USART_C_ // 如果没有定义

#else
#define USART_Ex_
#endif

#include <stdio.h>

#include <stm32f0xx.h>

#include "LoopList.h"

#include "network_module.h"

#include "tcp_mqtt.h"

/****************************串口1****************************/

/***串口接收环形队列****/
// 环形队列变量
extern rb_t rb_t_usart1_read;
#define rb_t_usart1_read_buff_len 200
// 环形队列缓存
extern unsigned char rb_t_usart1_read_buff[rb_t_usart1_read_buff_len];
// 从缓存拷贝数据使用
extern unsigned char usart1_read_buff_copy[rb_t_usart1_read_buff_len];
// 自定义空闲中断
extern int usart1_read_count_copy;
extern char usart1_idle_flag;

/*******************************************usart2********************************************/

// 接收环形队列
extern rb_t rb_t_usart2_read;
#define rb_t_usart2_read_buff_len 100
extern unsigned char rb_t_usart2_read_buff[rb_t_usart2_read_buff_len];
// 从缓存拷贝数据使用
extern unsigned char usart2_read_buff_copy[rb_t_usart2_read_buff_len];
// 接收计数
extern int usart2_read_count;
// 空闲中断标志
extern unsigned char usart2_idle_flag; // 标志

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
extern int usart4_read_count;
extern char usart4_idle_flag;

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
