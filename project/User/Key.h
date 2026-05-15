#ifndef __KEY_H
#define __KEY_H
#include "stdint.h"

void Heat_Key_Init(void);
uint8_t Heat_Key(void);
void Key_Tick(void);
uint8_t Key_GetNum(void);
void Key_Init(void);

typedef struct
{
	uint8_t key_num_heat;
} key_context_t;

extern key_context_t g_key_context;

#define KeyNum_Heat (g_key_context.key_num_heat)


extern uint16_t g_current_pwm_duty;

#endif
