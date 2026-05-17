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

typedef struct
{
	uint8_t Blower_Flag;
	uint8_t Heat_Flag;
	volatile uint8_t Flag_Sensor_Up;
	volatile uint8_t Flag_Sensor_Down1;
	volatile uint8_t Flag_Sensor_Down2;
	uint8_t ML307_Flag_Update;
	uint16_t Getup_Num;
	uint8_t Flag_Warn;
	uint8_t Getup_Flag;
	uint8_t TIM6_Time;
	uint8_t TIM6_Frist;
	uint32_t Frist_Run_Time;
	uint8_t Frist_Run;
	uint8_t Key_Heat;
	volatile uint8_t Flag_exti_3;
	volatile uint8_t Flag_exti_4;
	volatile uint8_t Flag_exti_5;
	uint32_t Blower_sate_second;
	Blowerstate state;
	WorkMode work_mode;
	volatile uint8_t key_pressed;
} blower_control_context_t;

extern blower_control_context_t g_blower_control_context;

#define Blower_Flag (g_blower_control_context.Blower_Flag)
#define Heat_Flag (g_blower_control_context.Heat_Flag)
#define Flag_Sensor_Up (g_blower_control_context.Flag_Sensor_Up)
#define Flag_Sensor_Down1 (g_blower_control_context.Flag_Sensor_Down1)
#define Flag_Sensor_Down2 (g_blower_control_context.Flag_Sensor_Down2)
#define ML307_Flag_Update (g_blower_control_context.ML307_Flag_Update)
#define Getup_Num (g_blower_control_context.Getup_Num)
#define Flag_Warn (g_blower_control_context.Flag_Warn)
#define Getup_Flag (g_blower_control_context.Getup_Flag)
#define TIM6_Time (g_blower_control_context.TIM6_Time)
#define TIM6_Frist (g_blower_control_context.TIM6_Frist)
#define Frist_Run_Time (g_blower_control_context.Frist_Run_Time)
#define Frist_Run (g_blower_control_context.Frist_Run)
#define Key_Heat (g_blower_control_context.Key_Heat)
#define Flag_exti_3 (g_blower_control_context.Flag_exti_3)
#define Flag_exti_4 (g_blower_control_context.Flag_exti_4)
#define Flag_exti_5 (g_blower_control_context.Flag_exti_5)
#define Blower_sate_second (g_blower_control_context.Blower_sate_second)
#define state (g_blower_control_context.state)
#define work_mode (g_blower_control_context.work_mode)
#define key_pressed (g_blower_control_context.key_pressed)
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
