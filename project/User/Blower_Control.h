#ifndef __BLOWERCONTROL_H
#define __BLOWERCONTROL_H
#include "stdint.h"

//2025.9.16 改动传感器引脚  LED三色灯引脚  风机 热机引脚

#define GPIO_Sensor GPIOA // 传感器输入的GPIO（输入）
#define GPIO_Sensor1 GPIOC// 传感器输入的GPIO（输入）

#define GPIO_Blower GPIOB // 吹风机输出GPIO（输出）
#define GPIO_Heat GPIOA   // 加热装置的GPIO（输出）

#define Pin_Sensor_Up GPIO_Pin_8 // 三个传感器输入引脚（输入）
#define Pin_Sensor_Down1 GPIO_Pin_11
#define Pin_Sensor_Down2 GPIO_Pin_9

#define Pin_Blower GPIO_Pin_13 // 风机输出引脚（输出）
#define Pin_Heat GPIO_Pin_12   // 加热装置引脚（输出）

// extern volatile uint8_t Flag_Sensor_Up, Flag_Sensor_Down1, Flag_Sensor_Down2; // 传感器有效标志位
extern uint8_t ML307_Flag_Update; // 状态更新标志位（任意状态更新）
extern uint16_t Getup_Num;        // 母猪起身次数
extern uint8_t Flag_Warn;         // 报警标志位
extern uint8_t Getup_Flag;

extern uint8_t TIM6_Time; // 电机吹风时间
extern uint8_t TIM6_Frist;
extern uint32_t Frist_Run_Time;
extern uint8_t Frist_Run;

extern uint8_t Key_Heat;

typedef enum
{
    POSTPARTUM_MODE = 0, // 产后模式
    PRODUCTION_MODE      // 生产模式
} WorkMode;

typedef enum
{
    IDLE = 0,       // 空闲
    SENSOR_FILTER,  // 上传感器下降沿触发，滤波状态，当前滤波1s
    FIRST_RUN,      // 第一次运行态，中间触发下传感器时间更新
    CONTINUOUS_RUN, // 连续运行状态，第一次运行完成后，上下传感器如果均能继续检测到物体，则隔一段时间运行一次
    DEVICE_ERROR
} Blowerstate;

extern Blowerstate state;
extern WorkMode work_mode;


extern volatile uint8_t key_pressed;
/*
函数功能：红外引脚初始化
*/
void Infrared_Init(void);

// 输出引脚初始化（风机工作、热机工作）
void Heat_Init(void);

void Key_Scan_And_Adjust_Speed(void);


// 吹风机控制
void Blower_Control(void);

// 加热控制
void Heat_Control(void);
// 报警设置
void Warn_Control(void);
// 上传数据
void Upload_Data(void);
void TIM_KeyScan_Init(void);
void TIM6_IRQHandler(void);

void EXTI_Init_Config(void);

void postpartumMode_Function(void);
void productionMode_Function(void);
#endif
