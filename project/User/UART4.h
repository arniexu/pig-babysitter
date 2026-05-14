#ifndef __UART4_H
#define __UART4_H

#ifndef USART_B_ // 如果没有定义
#define USART_Ex_ extern
#else
#define USART_Ex_
#endif

#include <stdio.h>

#include <stm32f0xx.h>

#include "LoopList.h"

#include "network_module.h"

#include "tcp_mqtt.h"

/****************************串口4****************************/

/***串口接收环形队列****/
// 环形队列变量
USART_Ex_ rb_t rb_t_usart4_read;
#define rb_t_usart4_read_buff_len 200
// 环形队列缓存
USART_Ex_ unsigned char rb_t_usart4_read_buff[rb_t_usart4_read_buff_len];
// 从缓存拷贝数据使用
USART_Ex_ unsigned char usart4_read_buff_copy[rb_t_usart4_read_buff_len];
// 自定义空闲中断
USART_Ex_ int usart4_read_count_copy;
USART_Ex_ char usart4_idle_flag;

/*******************************************缓存网络通信数据********************************************/
// 环形队列变量
USART_Ex_ rb_t rb_t_wifi_read;
#define rb_t_wifi_read_buff_len 200
// 环形队列缓存
USART_Ex_ unsigned char rb_t_wifi_read_buff[rb_t_wifi_read_buff_len];

/**
 * @brief   串口初始化
 * @param   bound  波特率
 * @param   None
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void uart4_init(void);

/**
 * @brief  串口中断发送数据
 * @param  c:数据的首地址  cnt:发送的数据个数
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
//void usart2_send_it(char *c, uint32_t cnt);

/**
 * @brief  串口发送字符串数据
 * @param  *c:发送的数据指针  cnt:数据个数
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void usart4_send(char *c, uint32_t cnt);

/**
 * @brief  串口1自定义空闲中断检测(放到1ms定时器)
 * @param  value: 空闲时间
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void usart4_idle_loop(int value);

/**
 * @brief  串口发送字符串数据
 * @param  *c:发送的数据指针  cnt:数据个数
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
//void usart2_send(char *c, uint32_t cnt);

#endif
