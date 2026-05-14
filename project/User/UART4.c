/**
  ******************************************************************************
  * @file    usart.c
  * @author  fengwu yang
  * @version V1.0.0
  * @date    2019/10/12
  * @brief   串口配置函数

  ******************************************************************************
  */
#define USART_B_
#include "UART4.h"
#include "BufferManage.h"
#include "usart.h"
/****************************串口4****************************/
/***串口接收环形队列****/
// 环形队列变量
rb_t rb_t_usart4_read;
// 环形队列缓存
unsigned char rb_t_usart4_read_buff[rb_t_usart4_read_buff_len];
// 从缓存拷贝数据使用
unsigned char usart4_read_buff_copy[rb_t_usart4_read_buff_len];
// 自定义空闲中断
int usart4_read_count = 0;
int usart4_read_count_copy = 0;
int usart4_read_idle_count = 0;
char usart4_idle_flag = 0;

///*******************************************usart2********************************************/
//// 接收环形队列
//rb_t rb_t_usart2_read;
//// 环形队列缓存数组
//unsigned char rb_t_usart2_read_buff[rb_t_usart2_read_buff_len];
//// 从缓存拷贝数据使用
//unsigned char usart2_read_buff_copy[rb_t_usart2_read_buff_len];
//// 接收计数
//int usart2_read_count = 0;

//// 发送环形队列
//rb_t rb_t_usart2_send;
//// 环形队列缓存数组
//unsigned char rb_t_usart2_send_buff[rb_t_usart2_send_buff_len];
//// 串口提取环形队列1个字节
//unsigned char rb_t_usart2_send_byte;
//// 空闲中断标志
//unsigned char usart2_idle_flag = 0;

/*******************************************缓存网络通信数据********************************************/
// 环形队列变量
rb_t rb_t_wifi_read;
// 环形队列缓存
unsigned char rb_t_wifi_read_buff[rb_t_wifi_read_buff_len];

/**
 * @brief   串口初始化
 * @param   bound  波特率
 * @param   None
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void uart4_init(void)
{
	// GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// 创建环形队列(用于串口接收数据)
	rbCreate(&rb_t_usart4_read, rb_t_usart4_read_buff, rb_t_usart4_read_buff_len);

	// 创建环形队列(缓存网络通信数据)
	rbCreate(&rb_t_wifi_read, rb_t_wifi_read_buff, rb_t_wifi_read_buff_len);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART4, ENABLE);
	
	// 2. 配置PA0 (USART4_TX) 和 PA1 (USART4_RX)
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_4); // PA0 -> USART4_TX
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_4); // PA1 -> USART4_RX

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3; // 高速
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 推挽
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. 配置USART4参数
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 开收发
    USART_Init(USART4, &USART_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = USART3_6_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		USART_ITConfig(USART4, USART_IT_RXNE, ENABLE);
		
		USART_Cmd(USART4, ENABLE);
}

void usart4_idle_loop(int value)
{
	if (usart4_read_count != 0)
	{ // 串口接收到数据
		if (usart4_read_count_copy != usart4_read_count)
		{
			usart4_read_count_copy = usart4_read_count;
			usart4_read_idle_count = 0;
		}
		else
		{
			usart4_read_idle_count++;
			if (usart4_read_idle_count > value)
			{
				usart4_read_idle_count = 0;

				usart4_read_count_copy = usart4_read_count;
				usart4_read_count = 0;
				usart4_idle_flag = 1; // 空闲标志
			}
		}
	}
}
void usart4_send(char *c, uint32_t cnt)
{
	usart2_send_it(c, cnt); // 打印日志

	while (cnt--)
	{
		USART_SendData(USART4, *c++);
		while (USART_GetFlagStatus(USART4, USART_FLAG_TXE) == RESET)
			;
	}
}
// 串口中断服务程序
void USART3_6_IRQHandler(void)
{
	char Data;
	char socket1_id;
	char res;
	if (USART_GetITStatus(USART4, USART_IT_RXNE) != RESET)
	{
		res = USART_ReceiveData(USART4); // 读取接收到的数据

		PutData(&rb_t_usart4_read, &res, 1);

		// 解析网络数据
		if (network_module_extract_data(res, &Data, &socket1_id))
		{
			PutData(&rb_t_wifi_read, &Data, 1);
		}

		usart4_read_count++;

		/*使用串口2打印串口4接收的数据*/
		USART_SendData(USART2, res);

		if (res == '>')
		{
			mqtt_clear_cipsend(&mqtt_value);
		}
	}
}
