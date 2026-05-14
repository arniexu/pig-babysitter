#include "stm32f0xx.h"
#include <stdio.h>
#include <string.h>
#include "PWM.h"

//风机启动初始化
void Blower_Init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);				
}

// 关闭风机
void Blower_OFF(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_13);
}
// 打开风机
void Blower_ON(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_13);
}




// 初始化TIM1为PWM输出
void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_BDTRInitTypeDef TIM_BDTRInitStructure; // 用于配置高级定时器的死区和刹车功能

    // 1. 使能时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); // TIM1在APB2总线上

    // 2. 配置PB13为复用功能（TIM1_CH1N）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;       // 复用功能模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     // 推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 无上下拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 将PB13引脚复用映射到TIM1_CH1N
    // 对于STM32F0系列，使用GPIO_PinAFConfig函数
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_2); // AF2对应TIM1

    // 3. 配置TIM1时基单元
    // STM32F030的系统时钟通常配置为48MHz
    // 预分频器 (Prescaler) = 48MHz / 期望的时钟频率 - 1
    // 这里我们设置定时器时钟为 1MHz (48MHz / 48)
    TIM_TimeBaseStructure.TIM_Prescaler = 48 - 1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    // 计数周期 (Period) = 定时器时钟频率 / 期望的PWM频率 - 1
    // 这里我们设置PWM频率为 1kHz (1MHz / 1000)
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0; // 高级定时器特有，此处设为0
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    // 4. 配置TIM1通道1为PWM模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; // PWM模式1
    // 即使我们不用主通道，也需要使能它，因为互补通道依赖主通道的比较结果
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    // 【关键】使能互补通道输出
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
    // 初始占空比为0 (TIM_Pulse / TIM_Period)
    TIM_OCInitStructure.TIM_Pulse = 0xFFFFFFFF ;
    // 主通道极性：有效电平为高电平
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    // 互补通道极性：有效电平为低电平 (通常与主通道相反)
    // 当TIM_Pulse=0时，输出无效电平，即高电平。
    // 如果你希望初始为低电平，可以将极性设置为TIM_OCNPolarity_High。
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High; 
    // 空闲状态配置，此处使用默认值
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;
    
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);

    // 5. 【关键】配置BDTR寄存器
    // BDTR寄存器控制着死区时间、刹车功能和PWM主输出使能
    TIM_BDTRInitStructure.TIM_DeadTime = 0; // 死区时间为0
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable; // 关闭刹车功能，防止意外触发
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
    // 【极其关键】使能主输出，否则TIM1的所有PWM输出都将被禁止
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable; 
    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);
    
    // 6. 使能TIM1计数器
    TIM_Cmd(TIM1, ENABLE);
}

// 示例：设置PWM占空比的函数
// duty_cycle: 占空比 (0 ~ 1000)，对应0% ~ 100%
void PWM_SetDutyCycle(uint16_t duty_cycle)
{
    if (duty_cycle > 1000) {
        duty_cycle = 1000;
    }
    TIM_SetCompare1(TIM1, duty_cycle);
}

