#include "stm32f0xx.h"

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
void Delay_us(uint32_t xus)
{
    // 1. 避免xus=0导致LOAD=0（直接返回，不等待）
    if (xus == 0)
        return;

    // 2. 保存当前SysTick配置（避免干扰其他地方的SysTick使用）
    uint32_t temp_ctrl = SysTick->CTRL;

    // 3. 配置SysTick：时钟源HCLK，关闭中断，清空计数
    SysTick->VAL = 0x00;                  // 清空当前计数值（必须先清零，否则可能残留计数）
    SysTick->LOAD = SystemCoreClock / 1000000 * xus;  // 动态计算LOAD值（适配实际HCLK）
    SysTick->CTRL = 0x00000005;           // 启动定时器（ENABLE=1，CLKSOURCE=1）

    // 4. 等待计数完成，增加超时机制（防止永久卡死）
    uint32_t timeout = 0;
    while (!(SysTick->CTRL & 0x00010000))
    {
        timeout++;
        // 超时阈值：根据最大可能延迟设置（如100万次循环，约几ms，可根据实际调整）
        if (timeout > 1000000)
        {
            // 超时处理（可选：打印日志、触发复位等）
            break;
        }
    }

    // 5. 恢复SysTick配置（避免影响其他使用场景）
    SysTick->CTRL = temp_ctrl;
}
/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
	while(xms--)
	{
		Delay_us(1000);
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
	while(xs--)
	{
		Delay_ms(1000);
	}
} 


/*

void delay_us(uint32_t us)
{
	SysTick->CTRL=0;		//先失能
	SysTick->LOAD=0xFF;		//写新的重转载值到LOAD寄存器
	SysTick->VAL=0;
	SysTick->CTRL=5;
	while((SysTick->CTRL&0x0001000)==0);
	SysTick->CTRL=0;
}
*/
