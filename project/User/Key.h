#ifndef __KEY_H
#define __KEY_H
#include "stdint.h"

void Heat_Key_Init(void);
uint8_t Heat_Key(void);
void Key_Tick(void);
uint8_t Key_GetNum(void);
void Key_Init(void);



extern uint16_t g_current_pwm_duty;

#endif
