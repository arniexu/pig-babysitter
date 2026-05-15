#ifndef __UART4_H
#define __UART4_H

#include <stdio.h>

#include <stm32f0xx.h>

#include "LoopList.h"

#include "network_module.h"

#include "tcp_mqtt.h"

/****************************串口4****************************/

/***串口接收环形队列****/
// 环形队列变量
extern rb_t rb_t_usart4_read;
#define rb_t_usart4_read_buff_len 200
// 环形队列缓存
extern unsigned char rb_t_usart4_read_buff[rb_t_usart4_read_buff_len];
// 从缓存拷贝数据使用
extern unsigned char usart4_read_buff_copy[rb_t_usart4_read_buff_len];
// 自定义空闲中断
extern int usart4_read_count_copy;
extern char usart4_idle_flag;

/*******************************************网络通信通道********************************************/
// 环形队列变量
extern rb_t rb_t_wifi_read;
#define rb_t_wifi_read_buff_len 200
// 环形队列缓存
extern unsigned char rb_t_wifi_read_buff[rb_t_wifi_read_buff_len];

/**
 * @brief   ���ڳ�ʼ��
 * @param   bound  ������
 * @param   None
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void uart4_init(void);

/**
 * @brief  �����жϷ�������
 * @param  c:���ݵ��׵�ַ  cnt:���͵����ݸ���
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
//void usart2_send_it(char *c, uint32_t cnt);

/**
 * @brief  ���ڷ����ַ�������
 * @param  *c:���͵�����ָ��  cnt:���ݸ���
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void usart4_send(char *c, uint32_t cnt);

/**
 * @brief  ����1�Զ�������жϼ��(�ŵ�1ms��ʱ��)
 * @param  value: ����ʱ��
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void usart4_idle_loop(int value);

/**
 * @brief  ���ڷ����ַ�������
 * @param  *c:���͵�����ָ��  cnt:���ݸ���
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
//void usart2_send(char *c, uint32_t cnt);

#endif
