#include "stm32f0xx.h"
#include "Blower_Control.h"
#include "led3.h"
#include "tcp_mqtt.h"
#include <stdio.h>

#define GPIO_Red GPIOC
#define GPIO_Yel GPIOC
#define GPIO_Gre GPIOC

#define Pin_Red GPIO_Pin_8 // 红色灯
#define Pin_Yel GPIO_Pin_7 // 黄灯
#define Pin_Gre GPIO_Pin_6 // 绿灯

led3_context_t g_led3_context = {
	.led_mode = LED_MODE_OFF,
	.warn_light = 0,
};

// 初始化3个LED引脚
void LED3_Init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure; // 定义GPIO结构体，便于设置引脚参数
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	// 可选
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1; // 根据需求选择速度
	GPIO_InitStructure.GPIO_Pin = Pin_Yel | Pin_Red | Pin_Gre;	// 同时设置PB3和PB4
	GPIO_Init(GPIOC, &GPIO_InitStructure);				// 先用结构体初始化GPIOB的两个LED
}

// 打开对应的灯
void LED_ON(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_SetBits(GPIOx, GPIO_Pin);
}
// 关闭对应的灯
void LED_OFF(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_ResetBits(GPIOx, GPIO_Pin);
}

// 灯控制
void LED3_Control(void)
{
	if (work_mode == PRODUCTION_MODE)
	{
		/* code */
		if (birth_led_flag == 1)
		{
			led_mode = LED_MODE_RED_FLICKER;
			// printf("birth_event_flag = %d\r\n", birth_event_flag);
		}
		else if (birth_led_flag == 0)
		{
			//			printf("birth_event_flag = %d\r\n",birth_event_flag);
			led_mode = LED_MODE_GREEN;
		}
	}

	if (work_mode == POSTPARTUM_MODE)
	{
		/* code */
		if (light_state == 1)
		{
			led_mode = LED_MODE_RED;
		}
		else
		{
			if (today_num >= warn_num && warn_num != 0)
			{
				led_mode = LED_MODE_RED_FLICKER;
			}
			else if (warn_light == 1)
			{
				/* code */
				led_mode = LED_MODE_RED_FLICKER;
			}

			else
			{
				if (state == DEVICE_ERROR)
				{
					/* code */
					led_mode = LED_MODE_YELLOW_FLICKER;
				}
				else
				{
					if (TIM6_Time > 0)
					{
						led_mode = LED_MODE_YELLOW;
					}
					if (TIM6_Time == 0)
					{
						led_mode = LED_MODE_GREEN;
					}
				}
			}
		}
	}
}

//// 点亮LED
//void LED3_ON(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
//{
//	if (GPIO_Pin == Pin_Red)
//	{
//		GPIO_SetBits(GPIO_Red, Pin_Red);
//		GPIO_ResetBits(GPIO_Yel, Pin_Yel);
//		GPIO_ResetBits(GPIO_Gre, Pin_Gre);
//	}
//	if (GPIO_Pin == Pin_Yel)
//	{
//		GPIO_SetBits(GPIO_Yel, Pin_Yel);
//		GPIO_ResetBits(GPIO_Gre, Pin_Gre);
//	}
//	if (GPIO_Pin == Pin_Gre)
//	{
//		GPIO_SetBits(GPIO_Gre, Pin_Gre);
//		GPIO_ResetBits(GPIO_Yel, Pin_Yel);
//	}
//}

//// 关闭LED
//void LED3_OFF(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
//{
//	GPIO_ResetBits(GPIOx, GPIO_Pin);
//}

void LED_MODE_Tick(void)
{
	static uint32_t LED_Count = 0;
	if (led_mode == LED_MODE_OFF)
	{
				LED_OFF(GPIO_Red, Pin_Red);
				LED_OFF(GPIO_Yel, Pin_Yel);
				LED_OFF(GPIO_Gre, Pin_Gre);
				LED_Count = 0;
	}
	else if (led_mode == LED_MODE_GREEN)
	{
				LED_OFF(GPIO_Red, Pin_Red);
				LED_OFF(GPIO_Yel, Pin_Yel);
				LED_ON(GPIO_Gre, Pin_Gre);
				LED_Count = 0;
	}
	else if (led_mode == LED_MODE_YELLOW)
	{
				LED_OFF(GPIO_Red, Pin_Red);
				LED_ON(GPIO_Yel, Pin_Yel);
				LED_OFF(GPIO_Gre, Pin_Gre);
				LED_Count = 0;
	}
	else if (led_mode == LED_MODE_RED)
			{

				
				LED_OFF(GPIO_Yel, Pin_Yel);
				LED_OFF(GPIO_Gre, Pin_Gre);
				LED_ON(GPIO_Red, Pin_Red);
				
				LED_Count = 0;
	}
	else if (led_mode == LED_MODE_RED_FLICKER)
	{
				/* code */
				LED_Count++;
				LED_Count %= 1000;
				if (LED_Count < 500)
				{
					LED_ON(GPIO_Red, Pin_Red);
					LED_OFF(GPIO_Yel, Pin_Yel);
					LED_OFF(GPIO_Gre, Pin_Gre);
				}
				else
				{
					LED_OFF(GPIO_Red, Pin_Red);
					LED_OFF(GPIO_Yel, Pin_Yel);
					LED_OFF(GPIO_Gre, Pin_Gre);
				}
	}
	else if (led_mode == LED_MODE_YELLOW_FLICKER)
	{
				/* code */
				LED_Count++;
				LED_Count %= 1000;
				if (LED_Count < 500)
				{
					LED_OFF(GPIO_Red, Pin_Red);
					LED_ON(GPIO_Yel, Pin_Yel);
					LED_OFF(GPIO_Gre, Pin_Gre);
				}
				else
				{
					LED_OFF(GPIO_Red, Pin_Red);
					LED_OFF(GPIO_Yel, Pin_Yel);
					LED_OFF(GPIO_Gre, Pin_Gre);
				}
	}
	else if (led_mode == LED_MODE_ORANGE_FLICKER)
	{
				/* code */
				LED_Count++;
				LED_Count %= 1000;
				if (LED_Count < 500)
				{
					LED_OFF(GPIO_Red, Pin_Red);
					LED_ON(GPIO_Yel, Pin_Yel);
					LED_ON(GPIO_Gre, Pin_Gre);
				}
				else
				{
					LED_OFF(GPIO_Red, Pin_Red);
					LED_OFF(GPIO_Yel, Pin_Yel);
					LED_OFF(GPIO_Gre, Pin_Gre);
				}
	}
}
