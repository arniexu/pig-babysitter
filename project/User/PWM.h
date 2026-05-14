#ifndef __PWM_H
#define __PWM_H

#include "stdint.h"
#include "stm32f0xx.h"


void PWM_Init(void);
void PWM_SetDutyCycle(uint16_t duty_cycle);
void Blower_start(void);
void Blower_OFF(void);
void Blower_ON(void);
void Blower_Init(void);

#endif

