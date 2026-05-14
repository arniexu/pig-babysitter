#ifndef IWDG_H_
#define IWDG_H_

#include "stm32f0xx.h"

#ifdef IWDG_C_
#define IWDG_C_
#else
#define IWDG_C_ extern
#endif

void IWDG_Init(uint8_t prer, uint16_t rlr);
void IWDG_Feed(void);

#endif
