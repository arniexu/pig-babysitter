#include "stm32f0xx.h"
#include "Blower_Control.h"
#include "LED3.h"
#include "Ml307.h"
#include "Key.h"
#include "Timer.h"
#include "PWM.h"

extern uint8_t Frist_Run; // 上电运行标志位

static uint32_t TIM3_S;
static uint32_t TIM3_count;
static uint8_t QueryForNetworkFlags; // 查询网路状态标志位，开机30秒后会置1

// 定时器初始化
void TIM3_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // 使能TIM3时钟
	// TIM_InternalClockConfig(TIM3);		//配置TIM2使用内部时源，不是默认？？

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; // 定时器初始化结构体

	// 定时器基本参数配置
	TIM_TimeBaseStructure.TIM_Prescaler = 4800 - 1; // 预分频值,10KHz
	TIM_TimeBaseStructure.TIM_Period = 10 - 1;		// 自动重装载值，定时1ms

	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	// 使能TIM3中断
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	// 配置NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// 使能TIM3
	TIM_Cmd(TIM3, ENABLE); // 初始时关闭定时器
}
// 延时计数器（单位：ms，累计到3000表示3秒）
static uint32_t delay_3s_cnt = 0;
static uint8_t mqttopen_sent_flag = 0;
static uint32_t delay_15s = 0;

static uint8_t network_sent_flag = 0;

uint8_t Timer_ShouldQueryNetworkStatus(void)
{
	if (QueryForNetworkFlags == 0)
	{
		return 0;
	}
	QueryForNetworkFlags = 0;
	return 1;
}

uint8_t Timer_CanSendMqttOpen(void)
{
	return (mqttopen_sent_flag == 0 && delay_15s == 15000);
}

void Timer_MarkMqttOpenSent(void)
{
	mqttopen_sent_flag = 1;
}

void Timer_ResetMqttOpenSent(void)
{
	mqttopen_sent_flag = 0;
}

uint8_t Timer_CanSendNetworkMode(void)
{
	return (mqttopen_sent_flag == 1 && network_sent_flag == 0 && delay_3s_cnt >= 4000);
}

void Timer_MarkNetworkModeSent(void)
{
	network_sent_flag = 1;
	delay_3s_cnt = 0;
}
// 定时器3中断函数
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		connect_mqtt_delay_value++;
		ConfigModuleNoBlockCnt++;
		ConfigModuleBlockDelay++;
	
		
		usart1_idle_loop(20);
		usart4_idle_loop(20);
		mqtt_time_data(&mqtt_value);
// 修正后代码（发送mqttopen后，且未发SendNetworkmode1时，才计时）
	if (mqttopen_sent_flag == 1 && network_sent_flag == 0)
	{
			delay_3s_cnt++; // 1ms递增1，累计到3000即3秒
	}
	
	if(delay_15s < 15000)
	{
			delay_15s++;
	}
	
		if (Frist_Run != 0)
		{
			led_mode = LED_MODE_YELLOW;
			Frist_Run_Time--;
			if (Frist_Run_Time == 0)
			{
				Blower_OFF(); // 关闭风机
				Frist_Run = 0;
				// TIM_Cmd(TIM3, DISABLE);
			}
		}
		TIM3_count++;
		if (TIM3_count % 1000 == 0)
		{
			/* code */
			TIM3_count = 0;
			TIM3_S++;
			// printf("TIM3_S = %d", TIM3_S);
//			 printf("LED_mode = %d", led_mode);
		}
		if (TIM3_S == 20)
		{
			/* code */
			QueryForNetworkFlags = 1;
			TIM3_S = 0;
		}

		if (network_config_flage == 1)
		{
			/* code */
			led_mode = LED_MODE_ORANGE_FLICKER;
			Blower_OFF(); // 直接关闭风机
		}
		LED_MODE_Tick();
		Key_Tick();

		TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // 5.27 将清除标志位放进括号内
	}
}
