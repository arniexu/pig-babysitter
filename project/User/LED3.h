#ifndef __LED3_H
#define __LED3_H

#include "stm32f0xx.h"
#include "stdint.h"
#include "Delay.h"

#define GPIO_Red GPIOC
#define GPIO_Yel GPIOC
#define GPIO_Gre GPIOC

#define Pin_Red GPIO_Pin_8 // 红色灯
#define Pin_Yel GPIO_Pin_7 // 黄灯
#define Pin_Gre GPIO_Pin_6 // 绿灯

typedef enum
{
    LED_MODE_OFF = 0,
    LED_MODE_GREEN,
    LED_MODE_YELLOW,
    LED_MODE_RED,
    LED_MODE_RED_FLICKER,
    LED_MODE_YELLOW_FLICKER,
    LED_MODE_ORANGE_FLICKER,
	
	
} LED_Mode;

extern LED_Mode led_mode;

extern uint8_t warn_light;

void LED3_Init(void);
void LED_ON(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void LED_OFF(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void LED3_Control(void);
void LED_MODE_Tick(void);

#endif
