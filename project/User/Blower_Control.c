#include "stm32f0xx.h"
#include "Key.h"
#include "Delay.h"
#include "LED3.h"
#include "Blower_Control.h"
#include "tcp_mqtt.h"
#include "PWM.h"

//2025.9.16 改动传感器引脚  LED三色灯引脚  风机 热机引脚  PWM波控制风机启动关闭

#define GPIO_Sensor GPIOA // 传感器输入的GPIO（输入）
#define GPIO_Sensor1 GPIOC

#define GPIO_Blower GPIOB // 吹风机输出GPIO（输出）
#define GPIO_Heat GPIOA	  // 加热装置的GPIO（输出）

#define Pin_Sensor_Up GPIO_Pin_8 // 三个传感器输入引脚（输入）
#define Pin_Sensor_Down1 GPIO_Pin_11
#define Pin_Sensor_Down2 GPIO_Pin_9

#define Pin_Blower GPIO_Pin_13 // 风机输出引脚（输出）
#define Pin_Heat GPIO_Pin_12	  // 加热装置引脚（输出）

#define SATE_TIME 300 // 连续吹风间隔时间300S

// uint8_t state = IDLE;
uint8_t Blower_Flag, Heat_Flag;

volatile uint8_t Flag_Sensor_Up, Flag_Sensor_Down1, Flag_Sensor_Down2; // 传感器有效标志位
uint8_t ML307_Flag_Update;											   // 状态更新标志位：0/1（任意状态更新）

uint8_t Getup_Flag = 0;
uint16_t Getup_Num = 0; // 母猪起身次数

uint8_t TIM6_Time;	   // 电机吹风时间
uint8_t Flag_Warn = 0; // 报警标志位

uint32_t Blower_sate_second = 0; // 吹风间隔秒数

uint8_t Frist_Run;
uint32_t Frist_Run_Time;

uint8_t Key_Heat = 0; // 热机键值

volatile uint8_t Flag_exti_3 = 0;
volatile uint8_t Flag_exti_4 = 0;
volatile uint8_t Flag_exti_5 = 0;

void TIM_KeyScan_Init(void);
void TIM6_IRQHandler(void);

Blowerstate state;
WorkMode work_mode;

 
volatile uint8_t key_pressed = 0;

/*
函数功能：红外引脚初始化（输入）
*/
void Infrared_Init()
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // 输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // 上拉
	GPIO_InitStructure.GPIO_Pin = Pin_Sensor_Up | Pin_Sensor_Down1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1; // 输入模式下可忽略
	GPIO_Init(GPIO_Sensor, &GPIO_InitStructure);		// 初始化传感器输入引脚
	
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // 输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // 上拉
	GPIO_InitStructure.GPIO_Pin = Pin_Sensor_Down2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1; // 输入模式下可忽略
	GPIO_Init(GPIO_Sensor1, &GPIO_InitStructure);		// 初始化传感器输入引脚
}

// 热机引脚初始化（输出）
void Heat_Init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	// 可选
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_2; // 根据需求选择速度
	GPIO_InitStructure.GPIO_Pin =  Pin_Heat;
	GPIO_Init(GPIO_Heat, &GPIO_InitStructure); // 初始化传感器输入引脚

}

void EXTI_Init_Config(void)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // 使能SYSCFG时钟（EXTI配置必需）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // 初始化EXTI配置结构体
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;

    // 配置PA8 - EXTI8
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);
    EXTI_InitStruct.EXTI_Line = EXTI_Line8;
    EXTI_Init(&EXTI_InitStruct);

    // 配置PA11 - EXTI11
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource11);
    EXTI_InitStruct.EXTI_Line = EXTI_Line11;
    EXTI_Init(&EXTI_InitStruct);

    // 配置PC9 - EXTI9
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource9);
    EXTI_InitStruct.EXTI_Line = EXTI_Line9;
    EXTI_Init(&EXTI_InitStruct);

    // 配置NVIC（只需要配置一次，因为EXTI4-15共享同一个中断通道）
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_15_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

// 中断服务函数（处理EXTI4到EXTI15的中断）
void EXTI4_15_IRQHandler(void)
{
       // PA8 - EXTI8 处理（上传感器）
    if (EXTI_GetITStatus(EXTI_Line8) != RESET) 
    {
        Delay_ms(10); // 新增：10ms消抖，避免误触发
        if (GPIO_ReadInputDataBit(GPIO_Sensor, Pin_Sensor_Up) == 0) // 二次确认低电平
        {
            if (work_mode == POSTPARTUM_MODE)
            {
                TIM_Cmd(TIM6, ENABLE); 
                state = SENSOR_FILTER;
                Blower_Flag = 1; 
                TIM6_Time = 30;  
            }
        }
        EXTI_ClearITPendingBit(EXTI_Line8); 
    }
    
    // PC9 - EXTI9 处理
    if (EXTI_GetITStatus(EXTI_Line9) != RESET) // 检查EXTI9是否触发
    {
        Flag_exti_5 = 1;  // 注意：变量名可能需要根据实际功能调整
        if (work_mode == PRODUCTION_MODE)
        {
            TIM6_Time = 0;
            TIM_Cmd(TIM6, ENABLE); // 开启定时器6
        }
//					printf("Flag_exti_5 = 1\r\n");
        EXTI_ClearITPendingBit(EXTI_Line9); // 清除中断标志
    }
    
    // PA11 - EXTI11 处理
    if (EXTI_GetITStatus(EXTI_Line11) != RESET) // 检查EXTI11是否触发
    {
        Flag_exti_3 = 1;  // 注意：变量名可能需要根据实际功能调整
        if (work_mode == PRODUCTION_MODE)
        {
            TIM6_Time = 0;
            TIM_Cmd(TIM6, ENABLE); // 开启定时器6
        }
//        printf("Flag_exti_3 = 1\r\n");
        EXTI_ClearITPendingBit(EXTI_Line11); // 清除中断标志
    }
}

// 吹风机打开控制
void Blower_Control(void)
{
	//	if(Frist_Run==0)
	//	{
	if (ESP_Flag_Blower == 0) // 如果总开关关闭
	{
	//	TIM6_Time = 0;
		Blower_OFF(); // 直接关闭风机
		GPIO_SetBits(GPIO_Heat, Pin_Heat);	 // 直接关闭热机
	}

	if (ESP_Flag_Blower == 1) // 如果总开关打开
	{
		if (TIM6_Time == 0) // 总开关打开但是运行时间为0
		{
			Blower_OFF(); // 直接关闭风机
			GPIO_SetBits(GPIO_Heat, Pin_Heat);	 // 直接关闭热机
		}
		if (TIM6_Time > 0) // 总开关打开，如果运行时间不为0
		{
			Blower_ON(); // 打开风机
			if (ESP_Flag_Heat == 1)				   // 如果热机标志位有效
			{
				GPIO_ResetBits(GPIO_Heat, Pin_Heat); // 打开热机
			}
			else // 热机标志位无效
			{
				GPIO_SetBits(GPIO_Heat, Pin_Heat); // 关闭热机
			}
		}
	}
	//	}
}

// 加热信号控制（适配点动按键）
void Heat_Control(void)
{
    static uint8_t last_heat_key = 0; // 记录上一次按键状态（用于检测单次触发）
    uint8_t current_heat_key = Key_GetNum(); // 当前按键状态（1表示触发，0表示未触发）

    // 检测到点动按键的单次触发（当前为1，上一次为0，即上升沿）
    if (current_heat_key == 1 && last_heat_key == 0)
    {
        // 切换加热状态（0→1 或 1→0）
        ESP_Flag_Heat = !ESP_Flag_Heat;
        // 发送状态更新
        SendWind_temp1();
			
        // 打印调试信息
//        printf("Heat state toggled: ESP_Flag_Heat = %d\r\n", ESP_Flag_Heat);
    }

    // 更新上一次按键状态（用于下次判断）
    last_heat_key = current_heat_key;
}

// TIM6初始化
void TIM_KeyScan_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	// 使能TIM6时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	// 配置TIM6基础参数,1s定时器
	TIM_InitStruct.TIM_Prescaler = 4800 - 1; // 48分频（假设系统时钟为48MHz）
	TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStruct.TIM_Period = 10000 - 1; // 1000次计数触发中断（10000 * 100us = 1s，实际周期需调整）
	TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM6, &TIM_InitStruct);

	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	// 使能TIM6更新中断
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

	// 配置NVIC中断优先级
	NVIC_InitStruct.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	TIM_Cmd(TIM6, DISABLE); // 先关闭定时器
}

// 定时器6中断函数，获取传感器状态，确定风机机运行时间
void TIM6_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update); // 清除定时器6的标志位
		if (work_mode == POSTPARTUM_MODE)			// 产后模式
		{
			/* code */
			postpartumMode_Function();
		}
		if (work_mode == PRODUCTION_MODE)
		{
			/* code */
			productionMode_Function();
		}
	}
}

void postpartumMode_Function(void)
{
	Flag_Sensor_Up = !(GPIO_ReadInputDataBit(GPIO_Sensor, Pin_Sensor_Up)); // 读取三个传感器的状态，低电平有效所以取反
	Flag_Sensor_Down1 = !(GPIO_ReadInputDataBit(GPIO_Sensor, Pin_Sensor_Down1));
	Flag_Sensor_Down2 = !(GPIO_ReadInputDataBit(GPIO_Sensor, Pin_Sensor_Down2));
	if (state == SENSOR_FILTER) // 如果是第一次进入中断，消抖作用
	{
	
		state = FIRST_RUN;		 // 第一次进入中断标志位清0
		if (Flag_Sensor_Up == 0) // 上传感器无效
		{
			state = IDLE;
			TIM6_Time = 0; // 运行时间清0
			Flag_exti_3 = 0;
			Flag_exti_5 = 0;
			Blower_sate_second = 0;
			TIM_Cmd(TIM6, DISABLE); // 关闭定时器6
		}
	
		if (Flag_Sensor_Up == 1) // 上传感器有效
		{
			Getup_Flag = 1; // 起身标志位置1，用于上传
			// ML307_Flag_Update = 1; // 上传更新标志位，用于更新起身计数

			if ((Flag_exti_3 == 1) || (Flag_exti_5 == 1)) // 上面有效，如果下面的也有效
			{
				Blower_Flag = 1; // 风机启动标志位
				TIM6_Time = 30;	 // 风机开启时间30s
				if (Flag_exti_3 == 1)
				{
					/* code */
			
					Flag_exti_3 = 0;
				}
				if (Flag_exti_5 == 1)
				{
					/* code */
					Flag_exti_5 = 0;
					
				}
			}
		}
	}

	if (state == FIRST_RUN) // 如果不是第一次进入中断，30s还没吹完
	{
	
		if (Flag_Sensor_Up == 0) // 如果上面无效
		{
        state = IDLE;          // 切回空闲状态
        TIM6_Time = 0;         // 清吹风计时
        Flag_exti_3 = 0;       // 清下传感器中断标志
        Flag_exti_5 = 0;       
        Blower_sate_second = 0;// 清间隔计时（关键：避免累积值干扰）
        Blower_Flag = 0;       // 强制停风机
        TIM_Cmd(TIM6, DISABLE); // 关定时器
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update); // 清定时器中断残留（关键：避免定时器“幽灵启动”）
		}
		else
		{
			if ((Flag_exti_3 == 1) || (Flag_exti_5 == 1)) // 上面有效，如果下面的也有效
			{
				Blower_Flag = 1; // 风机启动标志位
				TIM6_Time = 30;	 // 风机开启时间30s
		
				if (Flag_exti_3 == 1)
				{
					/* code */
					Flag_exti_3 = 0;

				}
				if (Flag_exti_5 == 1)
				{
					/* code */
					Flag_exti_5 = 0;
				}
			}
		}
	}

	if (state == CONTINUOUS_RUN) // 如果不是第一次进入中断，同时30s已经吹完
	{
	
    if (Flag_Sensor_Up == 0) // 传感器断开（长时间触发后拿开）
    {
        state = IDLE;          // 切回空闲状态
        TIM6_Time = 0;         // 清吹风计时
        Flag_exti_3 = 0;       // 清下传感器中断标志
        Flag_exti_5 = 0;       
        Blower_sate_second = 0;// 清间隔计时（关键：避免累积值干扰）
        Blower_Flag = 0;       // 强制停风机
        TIM_Cmd(TIM6, DISABLE); // 关定时器
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update); // 清定时器中断残留（关键：避免定时器“幽灵启动”）
    }
		else // 30s过后上面传感器仍然有效
		{
			if ((Flag_exti_3 == 1) || (Flag_exti_5 == 1)) // 上面有效，如果下面的也有效
			{
				Blower_Flag = 1; // 风机启动标志位
				TIM6_Time = 30;	 // 风机开启时间30s
				if (Flag_exti_3 == 1)
				{
					/* code */
					Flag_exti_3 = 0;
					
				}
				if (Flag_exti_5 == 1)
				{
					/* code */
					Flag_exti_5 = 0;
				}
			}
			if (Flag_Sensor_Down1 == 1 || Flag_Sensor_Down2 == 1)
			{
				/* code */
				if (TIM6_Time == 0)
				{
					Blower_sate_second++;
				
					if ((Blower_sate_second != 0) && (Blower_sate_second % SATE_TIME) == 0)
					{
						/* code */
				
						Blower_Flag = 1; // 风机启动标志位
						TIM6_Time = 30;	 // 风机开启时间30s
					}
					if (Blower_sate_second == 3600)
					{
						/* code */
				
						state = DEVICE_ERROR;
					}
				}
			}
			else
			{
				/* code */
				Blower_sate_second = 0;
			}
		}
	}
	if (state == DEVICE_ERROR) // 如果为设备故障状态
	{
		printf("ERROR");
	}
	if (TIM6_Time > 0)
	{
		TIM6_Time--; // 最后再TIM6_Time--
		printf("TIM6_Time = %d\r\n", TIM6_Time);
		if (TIM6_Time == 1)
		{
			/* code */
			state = CONTINUOUS_RUN;
			printf("state = %d\r\n", state);
		}
	}
	printf("state = %d\r\n", state);
}

void productionMode_Function(void)
{
	if ((Flag_exti_3 == 1) || (Flag_exti_5 == 1)) // 上面有效，如果下面的也有效
	{
		if (Flag_exti_3 == 1)
		{
			/* code */
			printf("state  Flag_exti_3\r\n");
			Flag_exti_3 = 0;
			birth_event_flag = 1;
			birth_led_flag = 1;
		}
		if (Flag_exti_5 == 1)
		{
			/* code */
			Flag_exti_5 = 0;
			birth_event_flag = 1;
			birth_led_flag = 1;
			printf("state  Flag_exti_5\r\n");
		}
	}
	else
	{
		TIM_Cmd(TIM6, DISABLE); // 关闭定时器6
	}
}
