/**
  ******************************************************************************
  * @file    usart.c
  * @author  fengwu yang
  * @version V1.0.0
  * @date    2019/10/12
  * @brief   串口配置函数

  ******************************************************************************
  */
#define USART_C_
#include "usart.h"
#include "MyFlash.h"
/****************************串口1****************************/
/***串口接收环形队列****/
// 环形队列变量
rb_t rb_t_usart1_read;
// 环形队列缓存
unsigned char rb_t_usart1_read_buff[rb_t_usart1_read_buff_len];
// 从缓存拷贝数据使用
unsigned char usart1_read_buff_copy[rb_t_usart1_read_buff_len];
// 自定义空闲中断
usart_runtime_context_t g_usart_runtime_context = {
	.usart1_read_count = 0,
	.usart1_read_count_copy = 0,
	.usart1_read_idle_count = 0,
	.usart1_idle_flag = 0,
	.usart2_read_count = 0,
	.usart2_idle_flag = 0,
	.usart4_read_count = 0,
	.usart4_read_count_copy = 0,
	.usart4_read_idle_count = 0,
	.usart4_idle_flag = 0,
	.imei_buff = {0},
	.imei_receive_cnt = 0,
	.imei_receive_done = 0,
	.imei_receiving_en = 0,
};

/*******************************************usart2********************************************/
// 接收环形队列
rb_t rb_t_usart2_read;
// 环形队列缓存数组
unsigned char rb_t_usart2_read_buff[rb_t_usart2_read_buff_len];
// 从缓存拷贝数据使用
unsigned char usart2_read_buff_copy[rb_t_usart2_read_buff_len];
// 发送环形队列
rb_t rb_t_usart2_send;
// 环形队列缓存数组
unsigned char rb_t_usart2_send_buff[rb_t_usart2_send_buff_len];
// 串口提取环形队列1个字节
unsigned char rb_t_usart2_send_byte;
/*******************************************缓存网络通信数据********************************************/
// 环形队列变量
rb_t rb_t_net_read;
// 环形队列缓存
unsigned char rb_t_net_read_buff[rb_t_net_read_buff_len];


//10.16修改添加串口四
rb_t rb_t_usart4_read;
unsigned char rb_t_usart4_read_buff[rb_t_usart4_read_buff_len];
unsigned char usart4_read_buff_copy[rb_t_usart4_read_buff_len];

/**
 * @brief   串口初始化
 * @param   bound  波特率
 * @param   None
 * @param   None
 * @retval  None
 * @warning None
 * @example
 **/
void uart_init(uint32_t bound1, uint32_t bound2,uint32_t bound3)
{
	// GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	rbCreate(&rb_t_usart4_read, rb_t_usart4_read_buff, rb_t_usart4_read_buff_len);
	
	// 创建环形队列(用于串口接收数据)
	rbCreate(&rb_t_usart2_read, rb_t_usart2_read_buff, rb_t_usart2_read_buff_len);
	// 创建环形队列(用于串口发送数据)
	rbCreate(&rb_t_usart2_send, rb_t_usart2_send_buff, rb_t_usart2_send_buff_len);
	// 创建环形队列(用于串口接收数据)
	rbCreate(&rb_t_usart1_read, rb_t_usart1_read_buff, rb_t_usart1_read_buff_len);

	// 创建环形队列(缓存网络通信数据)
	rbCreate(&rb_t_net_read, rb_t_net_read_buff, rb_t_net_read_buff_len);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART4, ENABLE); 
	
	// 2. 配置GPIO引脚
	// PA9 - USART1_TX (复用推挽输出)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_2;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);

	// PA10 - USART1_RX (浮空输入)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

	// PA2 - USART2_TX (复用推挽输出)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_2;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);

	// PA3 - USART2_RX (浮空输入)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

	USART_InitStructure.USART_BaudRate = bound1;									// 设置波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 选None不使用流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 串口模式：TX发送，RX接收
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 校验位：odd奇校验，even偶校验
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 停止位：四种位长 0.5，1，1.5，2
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 选择字长
	USART_Init(USART1, &USART_InitStructure);

	USART_InitStructure.USART_BaudRate = bound2;									// 设置波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 选None不使用流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 串口模式：TX发送，RX接收
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 校验位：odd奇校验，even偶校验
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 停止位：四种位长 0.5，1，1.5，2
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 选择字长
	USART_Init(USART2, &USART_InitStructure);
	
	USART_InitStructure.USART_BaudRate = bound3; // 与WiFi模块波特率一致（如115200）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; // TXD4 -> PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // 复用功能
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // 推挽输出（驱动外部模块）
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // 上拉（避免空闲时电平不稳定）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 速度根据需要设置
	GPIO_Init(GPIOA, &GPIO_InitStructure); // 端口是GPIOA
	
		// 配置PA1为USART4_RX（复用输入）
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; // RXD4 -> PA1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // 复用功能
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // 上拉（建议，部分WiFi模块需要）
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_4); // PA0复用为USART4_TX（AF编号需确认）
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_4); // PA1复用为USART4_RX（AF编号需确认）
	USART_Init(USART4, &USART_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
		// 串口4中断配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1; // 优先级合理设置
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // 开启串口接受中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // 开启串口接受中断

	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE); // 空闲中断
		// 使能接收中断和串口
	USART_ITConfig(USART4, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART4, ENABLE);
	USART_Cmd(USART1, ENABLE);
	USART_Cmd(USART2, ENABLE);

}




/**
 * @brief  串口中断发送数据
 * @param  c:数据的首地址  cnt:发送的数据个数
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void usart2_send_it(char *c, uint32_t cnt)
{
	PutData(&rb_t_usart2_send, c, cnt);
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

/**
 * @brief  串口发送字符串数据
 * @param  *c:发送的数据指针  cnt:数据个数
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void usart2_send(char *c, uint32_t cnt)
{
	while (cnt--)
	{
		USART_SendData(USART2, *c++);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
			;
	}
}

/**
 * @brief  串口发送字符串数据
 * @param  *c:发送的数据指针  cnt:数据个数
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void usart1_send(char *c, uint32_t cnt)
{
	usart2_send_it(c, cnt); // 打印日志

	while (cnt--)
	{
		USART_SendData(USART1, *c++);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
			;
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
/**
 * @brief  串口2自定义空闲中断检测(放到1ms定时器)
 * @param  value: 空闲时间
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void usart1_idle_loop(int value)
{
	if (usart1_read_count != 0)
	{ // 串口接收到数据
		if (usart1_read_count_copy != usart1_read_count)
		{
			usart1_read_count_copy = usart1_read_count;
			usart1_read_idle_count = 0;
		}
		else
		{
			usart1_read_idle_count++;
			if (usart1_read_idle_count > value)
			{
				usart1_read_idle_count = 0;

				usart1_read_count_copy = usart1_read_count;
				usart1_read_count = 0;
				usart1_idle_flag = 1; // 空闲标志
			}
		}
	}
}

// 串口中断服务程序
void USART2_IRQHandler(void)
{
	uint8_t Res;
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		Res = USART_ReceiveData(USART2); // 读取接收到的数据

		PutData(&rb_t_usart2_read, &Res, 1);
		usart2_read_count++;
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
	else if (USART_GetITStatus(USART2, USART_IT_IDLE) == SET) // 空闲中断
	{
		// USART2->DR; // 清除USART_IT_IDLE标志
		USART_ClearITPendingBit(USART2, USART_IT_IDLE); // 清除USART_IT_IDLE标志
		usart2_idle_flag = 1;
	}

	if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
	{
		if (rbCanRead(&rb_t_usart2_send) > 0) // 如果里面的数据个数大于0
		{
			rbRead(&rb_t_usart2_send, &rb_t_usart2_send_byte, 1);
			USART_SendData(USART2, rb_t_usart2_send_byte);
		}
		else
		{
			// 发送字节结束
			USART_ClearITPendingBit(USART2, USART_IT_TXE);
			USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
			USART_ITConfig(USART2, USART_IT_TC, ENABLE);
		}
	}
	// 发送完成
	if (USART_GetITStatus(USART2, USART_IT_TC) != RESET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_TC);
		USART_ITConfig(USART2, USART_IT_TC, DISABLE);
	}
}


void USART3_6_IRQHandler(void) {
    char data;
    char socket_id;
    char Res;
    if (USART_GetITStatus(USART4, USART_IT_RXNE) != RESET) {
        Res = USART_ReceiveData(USART4); // 读取串口4接收的数据

        // 存入串口4的环形缓冲区（替代原串口1的rb_t_usart1_read）
        PutData(&rb_t_usart4_read, &Res, 1);

        // 解析网络数据（与原串口1逻辑一致）
        if (network_module_extract_data(Res, &data, &socket_id)) {
            PutData(&rb_t_net_read, &data, 1);
        }

        // 更新串口4的接收计数（替代usart1_read_count）
        usart4_read_count++;

        // 调试：通过串口2打印接收数据
        USART_SendData(USART2, Res);

        // 处理特殊字符（与原串口1逻辑一致）
        if (Res == '>') 
					{
            mqtt_clear_cipsend(&mqtt_value);
					}

        USART_ClearITPendingBit(USART4, USART_IT_RXNE);
    }
}

void usart4_idle_loop(int value) {
    if (usart4_read_count != 0) {
        if (usart4_read_count_copy != usart4_read_count) {
            usart4_read_count_copy = usart4_read_count;
            usart4_read_idle_count = 0;
        } else {
            usart4_read_idle_count++;
            if (usart4_read_idle_count > value) {
                usart4_read_idle_count = 0;
                usart4_read_count_copy = usart4_read_count;
                usart4_read_count = 0;
                usart4_idle_flag = 1; // 串口4空闲标志
            }
        }
    }
}
void imei_main_loop_process(void)
{
    if (imei_receive_done == 1) // 仅当收到新IMEI时处理
    {
        unsigned char valid_flag = 1;
        // 1. 验证新接收的IMEI（15位数字）
        for (unsigned char i = 0; i < 15; i++)
        {
            if (imei_buff[i] < '0' || imei_buff[i] > '9')
            {
                valid_flag = 0;
                break;
            }
        }

        if (valid_flag == 1)
        {
            // 2. 提取Flash中存储的IMEI
            unsigned char flash_imei[16];
            memcpy(flash_imei, &FlashData[FLASH_IMEI_INDEX], 16);

            // 3. 判断Flash中IMEI是否未初始化
            int is_uninitialized = 0;
            if (flash_imei[0] == 0 || flash_imei[0] == 0xFF)
            {
                is_uninitialized = 1;
            }
            else
            {
                // 补充验证Flash中原有IMEI的有效性（防止脏数据）
                for (uint8_t i = 0; i < 15; i++)
                {
                    if (flash_imei[i] < '0' || flash_imei[i] > '9')
                    {
                        is_uninitialized = 1; // 原有数据无效，视为未初始化
                        break;
                    }
                }
            }

            // 4. 对比新IMEI与Flash中的IMEI
            int is_different = memcmp(imei_buff, flash_imei, 15) != 0;

            // 5. 未初始化或不同则更新
            if (is_uninitialized || is_different)
            {
                memcpy(&FlashData[FLASH_IMEI_INDEX], imei_buff, 16);
                if (Write_Flash(FlashData, 7) == 1)
                {
                    usart2_send("IMEI updated to Flash: ", 22);
                    usart2_send(imei_buff, 16);
                    usart2_send("\r\n", 2);
                    memcpy(mqtt_client_id, imei_buff, 16); // 更新MQTT ID
                }
               else
                {
                    usart2_send("Flash write failed\r\n", 19);
                }
            }
            else
            {
                // 相同则保持Flash值（已在初始化时加载，此处仅提示）
                usart2_send("Received IMEI same as Flash, no update\r\n", 40);
            }
        }
        else
        {
            usart2_send("Invalid IMEI (not 15 digits)\r\n", 28);
        }

        // 重置接收状态
        imei_receive_done = 0;
        imei_receive_cnt = 0;
        memset(imei_buff, 0, sizeof(imei_buff));
    }
}
void USART1_IRQHandler(void)
{
    static char data;
    static char socket_id;
    char Res;
    uint32_t isr_flag = USART1->ISR; // 读取所有状态标志

    // 处理接收非空（RXNE）中断
    if (isr_flag & USART_ISR_RXNE)
    {
        Res = USART1->RDR; // 读取数据（自动清除RXNE）

        /************************* IMEI接收逻辑 *************************/
        if (Res >= '0' && Res <= '9')
        {
            if (imei_receiving_en == 0)
            {
                imei_receiving_en = 1;
                imei_receive_cnt = 0;
                memset(imei_buff, 0, sizeof(imei_buff));
            }

            if (imei_receiving_en == 1 && imei_receive_cnt < 15)
            {
                imei_buff[imei_receive_cnt] = Res;
                imei_receive_cnt++;
                if (imei_receive_cnt == 15)
                {
                    imei_buff[15] = '\0';
                    imei_receive_done = 1;
                    imei_receiving_en = 0;
                }
            }
        }
        /************************* IMEI接收逻辑结束 *************************/

        // 原有数据处理
        PutData(&rb_t_usart1_read, &Res, 1);
        if (network_module_extract_data(Res, &data, &socket_id))
        {
            PutData(&rb_t_net_read, &data, 1);
        }
        usart1_read_count++;

        // USART2非阻塞转发
        if (USART2->ISR & USART_ISR_TXE)
        {
            USART2->TDR = Res;
        }
    }

    // 处理接收溢出错误（ORE）
    if (isr_flag & USART_ISR_ORE)
    {
        (void)USART1->RDR; // 先读RDR，再清标志
        USART1->ICR = USART_ICR_ORECF; // 清除ORE标志
    }

    // 处理帧错误（FE）和奇偶校验错误（PE）（移除NECF相关）
    if (isr_flag & USART_ISR_FE)
    {
        USART1->ICR = USART_ICR_FECF; // 清除帧错误标志
    }
    if (isr_flag & USART_ISR_PE)
    {
        USART1->ICR = USART_ICR_PECF; // 清除奇偶校验错误标志
    }
}

void usart1_send_mqttopen(void)
{
    char mqtt_open_str[] = "mqttopen\r\n";
    uint32_t str_len = sizeof(mqtt_open_str) - 1;
    usart1_send(mqtt_open_str, str_len);
}
/**
 * @brief  使用microLib的方法,使用printf
 * @warningg 勾选 microLib
 * @param  None
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/

int fputc(int ch, FILE *f)
{
	static char flage = 0;
	uint8_t Data = ch;
	if (!flage)
	{
		flage = 1;
		USART_ClearITPendingBit(USART2, USART_IT_TC); // 解决printf丢失第一个字节的问题
	}
	usart2_send_it((char *)&Data, 1);
	return ch;
}
