#include "stm32f0xx.h"
#include "Delay.h"
#include "Blower_Control.h"

#define GPIO_Heat_Key GPIOB		// 加热按键的GPIO
#define Pin_Heat_Key GPIO_Pin_15 // 加热按键控制引脚
//#define GPIO_PWM_Key GPIOB

uint8_t KeyNum_Heat = 0;


//// --- 全局变量定义 ---
//// 当前风速档位对应的PWM值 (0-1000)
//// 1000: 停止, 250: 4档, 500: 3档, 750: 2档, 900: 1档
//// 初始化为1档
//uint16_t g_current_pwm_duty = 150; 
//// 当前风速档位的索引 (1-4)
//uint8_t g_fan_speed_level = 1;


// 加热按键初始化
void Heat_Key_Init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); // 设置RCC_APB2外设，开启时钟
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		// 输入
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		// 上拉
	GPIO_InitStructure.GPIO_Pin = Pin_Heat_Key;			// PB15号按键输入信号检测
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1; // 输入模式下可忽略
	GPIO_Init(GPIO_Heat_Key, &GPIO_InitStructure);
}

uint8_t Key_GetNum(void)
{
	return KeyNum_Heat;
}

// 获取按键状态（1表示按下，0表示松开，不变）
uint8_t Key_GetState(void)
{
	// 上拉输入：按下时引脚为低电平（0），返回1表示按下；松开时为高电平（1），返回0
	return (GPIO_ReadInputDataBit(GPIO_Heat_Key, Pin_Heat_Key) == 0) ? 1 : 0;
}

// 按键状态检测（核心修改：点动逻辑）
void Key_Tick(void)
{
	static uint8_t debounce_cnt = 0; // 消抖计数器（0~20，20ms消抖）
	static uint8_t triggered = 0;    // 触发标志（避免一次按下多次触发）
	uint8_t current_state = Key_GetState(); // 当前按键状态

	if (current_state == 1) // 按键按下
	{
		if (debounce_cnt < 20)
		{
			debounce_cnt++; // 消抖计数递增（累计20ms确认按下）
		}
		else // 消抖完成，确认按下
		{
			if (!triggered) // 若未触发过，则输出一次触发信号
			{
				KeyNum_Heat = 1;
				triggered = 1; // 标记已触发，避免持续按下时重复输出
			}
			else
			{
				KeyNum_Heat = 0; // 已触发过，保持0
			}
		}
	}
	else // 按键松开
	{
		// 复位所有状态，确保下次按下可重新触发
		KeyNum_Heat = 0;
		debounce_cnt = 0;
		triggered = 0;
	}
}



//void Key_Init(void)
//{
//    GPIO_InitTypeDef GPIO_InitStructure;

//    // 使能GPIOB时钟
//    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

//    // 配置PB14为输入模式
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;      // 输入模式
//    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      // 上拉电阻
//    GPIO_Init(GPIOB, &GPIO_InitStructure);
//}



//void Key_Scan_And_Adjust_Speed(void)
//{
//    // 检查按键是否被按下 (低电平)
//    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == 0)
//    {
//        // 延时消抖
//        Delay_ms(20); 

//        // 再次确认按键是否仍被按下
//        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == 0)
//        {
//            // 循环切换风速档位
//            g_fan_speed_level++;
//            if (g_fan_speed_level > 4)
//            {
//                g_fan_speed_level = 1; // 回到1档
//            }

//            // 根据档位设置PWM占空比
//            switch(g_fan_speed_level)
//            {
//                case 1:
//                    g_current_pwm_duty = 150; // 15% 占空比
//                    break;
//                case 2:
//                    g_current_pwm_duty = 500; // 50% 占空比
//                    break;
//                case 3:
//                    g_current_pwm_duty = 750; // 75% 占空比
//                    break;
//                case 4:
//                    g_current_pwm_duty = 900; // 100% 占空比
//                    break;
//            }
//            
//            // 【关键】调用风机启动函数，应用新的风速
//            // 这样即使风机之前是停止的，按一下按键也会以新档位启动
//           

//            // 等待按键释放，防止一次按键被识别为多次
//            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == 0);
//        }
//    }
//}




