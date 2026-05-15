#include "stm32f0xx.h"
#include "usart.h"
#include "delay.h"
#include <string.h>
#include <stdio.h>
#include "Blower_Control.h"
#include "LED3.h"
#include "tcp_mqtt.h"
#include "MyFlash.h"

char ml307_recv_buf[60];
char *Usart1_rx_buf;
/**
 * @brief  接收数据(把数据接收的数据)
 * @param  none
 * @param  none
 * @param  none
 * @retval none
 * @example
 **/
int ML307Read(char **data)
{
    int len = 0;
    /*替换自己的接收数据函数----*/
    memset(usart1_read_buff_copy, 0, rb_t_usart1_read_buff_len);
    if (usart1_idle_flag)
    {
        usart1_idle_flag = 0;
        // 读取缓存数据个数
        len = rbCanRead(&rb_t_usart1_read);
        if (len > 0)
        {
            rbRead(&rb_t_usart1_read, usart1_read_buff_copy, len);
            usart1_read_buff_copy[len] = 0;
        }
    }
    // 把接收的数据地址赋值给 data(用户根据自己的修改)
    *data = (char *)usart1_read_buff_copy;

    return len;
}

/**
 * @brief  发送指令配置模块(内部替换自己的发送函数)
 * @param  none
 * @param  none
 * @param  none
 * @retval none
 * @example
 **/
void ML307Send(char *dat, int len)
{
    usart1_send(dat, len);
}

// 解析JSON函数
void Translate_JSON(void)
{

    if (ML307Read(&Usart1_rx_buf) != 0) // 串口1读到数据
    {
        snprintf(ml307_recv_buf, sizeof(ml307_recv_buf), "%s", Usart1_rx_buf);
//        printf("ml307_recv_buf:%s\r\n", ml307_recv_buf);
        if (strstr((const char *)ml307_recv_buf, "{\"mode\":0") != NULL)
        {
            work_mode = POSTPARTUM_MODE;
            work_mode_flag = 1;
            uint32_t temp = 0;
            memcpy(&temp, &work_mode, sizeof(WorkMode));
            FlashData[0] = temp;
            Write_Flash(FlashData, 3);
        }
        if (strstr((const char *)ml307_recv_buf, "{\"mode\":1") != NULL)
        {
            work_mode = PRODUCTION_MODE;
            TIM6_Time = 0;
            work_mode_flag = 1;
            uint32_t temp = 0;
            memcpy(&temp, &work_mode, sizeof(WorkMode));
            FlashData[0] = temp;
            Write_Flash(FlashData, 3);
        }
        if (strstr((const char *)ml307_recv_buf, "{\"light_state\":0") != NULL)
        {
            light_state = 0;
            light_state_Flag = 1;
        }
        if (strstr((const char *)ml307_recv_buf, "{\"light_state\":1") != NULL)
        {
            light_state = 1;
            light_state_Flag = 1;
        }
        if (strstr((const char *)ml307_recv_buf, "{\"wind_temp\":0") != NULL)
        {
            if (Key_Heat == 0)
            {
                /* code */
                ESP_Flag_Heat = 0;
                wind_temp_Flag = 1;
                uint32_t temp = 0;
                memcpy(&temp, &ESP_Flag_Heat, sizeof(uint8_t));
                FlashData[2] = temp;
                Write_Flash(FlashData, 3);
            }
        }

        if (strstr((const char *)ml307_recv_buf, "{\"wind_temp\":1") != NULL)
        {
            ESP_Flag_Heat = 1;
            wind_temp_Flag = 1;
            uint32_t temp = 0;
            memcpy(&temp, &ESP_Flag_Heat, sizeof(uint8_t));
            FlashData[2] = temp;
            Write_Flash(FlashData, 3);
        }

        if (strstr((const char *)ml307_recv_buf, "{\"fan_state\":0") != NULL)
        {
            ESP_Flag_Blower = 0;
            fan_state_Flag = 1;

            uint32_t temp = 0;
            memcpy(&temp, &ESP_Flag_Blower, sizeof(uint8_t));
            FlashData[1] = temp;
            Write_Flash(FlashData, 3);
        }
        if (strstr((const char *)ml307_recv_buf, "{\"fan_state\":1") != NULL)
        {
            ESP_Flag_Blower = 1;
            fan_state_Flag = 1;
            uint32_t temp = 0;
            memcpy(&temp, &ESP_Flag_Blower, sizeof(uint8_t));
            FlashData[1] = temp;
            Write_Flash(FlashData, 3);
        }

        if (strstr((const char *)ml307_recv_buf, "{\"warn_light\":0") != NULL)
        {
            if (work_mode == PRODUCTION_MODE)
            {
                /* code */
                birth_led_flag = 0;
            }

            warn_light = 0;
            warn_light_Flag = 1;
        }
        if (strstr((const char *)ml307_recv_buf, "{\"warn_light\":1") != NULL)
        {
            warn_light = 1;
            warn_light_Flag = 1;
        }

        if (strstr((const char *)ml307_recv_buf, "\"today_num\":") != NULL)
        {

            if (strchr(ml307_recv_buf, ',') != NULL)
            {
                char *str = StrBetwString(ml307_recv_buf, "today_num\":", ",");
                today_num = atoi(str);
                cStringRestore();
            }
            else
            {
                char *str = StrBetwString(ml307_recv_buf, "today_num\":", "}");
                today_num = atoi(str);
                cStringRestore();
            }

            // today_num = atoi(today_num_str + 1); // 转换字符串为整数
            printf("today_num:%d", today_num);
        }

        if (strstr((const char *)ml307_recv_buf, "\"warn_num\":") != NULL) // 5.26 将0去掉
        {
            char *str = StrBetwString(ml307_recv_buf, "warn_num\":", "}");

            warn_num = atoi(str); // 转换字符串为整数
            cStringRestore();
            warn_num_Flag = 1;
            printf("warn_num:%d", warn_num);
        }
    }
}
