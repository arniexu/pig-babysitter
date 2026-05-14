/******************************** Copyright (c) ***************************************
**
**
**
**---------------------------------File Info-------------------------------------------
** @file:               MyFlash.c
** @author:             ${author.body}
** @date:               2025-06-17
** @version:            V0.0
** @brief:
**
**-------------------------------------------------------------------------------------
** @modified:
** @date:               2025-06-17
** @version:            V0.0
** @description:
** @note:
**-------------------------------------------------------------------------------------
** @copyright:
**************************************************************************************/

#include "stm32f0xx.h"
#include "MyFlash.h"
#include "string.h"
#include "tcp_mqtt.h"
#include "usart.h"

uint32_t FlashData[FLASH_DATA_WORDS]; // 工作模式、风盾标志、加热标志，IMEI卡号

static uint32_t flash_crc32_calc(const uint32_t *data, uint8_t words)
{
    uint32_t crc = 0xFFFFFFFFU;
    uint8_t i;
    uint8_t j;
    uint8_t k;

    for (i = 0; i < words; i++)
    {
        uint32_t value = data[i];
        for (j = 0; j < 4U; j++)
        {
            uint8_t byte = (uint8_t)((value >> (8U * j)) & 0xFFU);
            crc ^= byte;
            for (k = 0; k < 8U; k++)
            {
                if ((crc & 1U) != 0U)
                {
                    crc = (crc >> 1) ^ 0xEDB88320U;
                }
                else
                {
                    crc >>= 1;
                }
            }
        }
    }

    return crc ^ 0xFFFFFFFFU;
}

static uint8_t flash_read_words_with_crc(uint32_t *data, uint8_t words)
{
    uint32_t stored_crc;
    uint32_t calc_crc;
    uint8_t i;

    if ((data == 0) || (words == 0U) || (words > FLASH_DATA_WORDS))
    {
        return 0;
    }

    for (i = 0; i < words; i++)
    {
        data[i] = (*(__IO uint32_t *)(WriteFlashAddress + 4U * i));
    }

    stored_crc = (*(__IO uint32_t *)(WriteFlashAddress + 4U * words));
    calc_crc = flash_crc32_calc(data, words);

    if (stored_crc != calc_crc)
    {
        return 0;
    }

    return 1;
}

/*******************************************************************************
写Flash函数
buff为数组 len为长度
return  出错 0 成功 1
如  写Flash_Write_Data 数组 2个字符
Write_Flash(Flash_Write_Data,2);
((u32)0x08010000)
flash的起始地址
*******************************************************************************/
uint8_t Write_Flash(uint32_t *buff, uint8_t len)
{
    volatile FLASH_Status FLASHStatus;
    uint8_t k = 0;
    uint32_t Address;
    uint32_t crc_value;
    uint8_t write_len;
    uint32_t write_buf[FLASH_DATA_WORDS];
    uint32_t flash_backup[FLASH_DATA_WORDS];

    if ((buff == 0) || (len == 0U) || (len > FLASH_DATA_WORDS))
    {
        return 0;
    }

    write_len = len;

    if (len == FLASH_IMEI_INDEX) // 只针对3字补全IMEI
    {
        // 先从Flash实际读取（带CRC校验）
        if (flash_read_words_with_crc(flash_backup, FLASH_DATA_WORDS) == 1U)
        {
            memcpy(write_buf, buff, (uint32_t)len * sizeof(uint32_t));
            memcpy(&write_buf[FLASH_IMEI_INDEX],
                   &flash_backup[FLASH_IMEI_INDEX],
                   (FLASH_DATA_WORDS - FLASH_IMEI_INDEX) * sizeof(uint32_t));
        }
        else
        {
            memcpy(write_buf, buff, (uint32_t)len * sizeof(uint32_t));
            // IMEI 区补0
            memset(&write_buf[FLASH_IMEI_INDEX], 0, (FLASH_DATA_WORDS - FLASH_IMEI_INDEX) * sizeof(uint32_t));
        }
        buff = write_buf;
        write_len = FLASH_DATA_WORDS;
    }

    crc_value = flash_crc32_calc(buff, write_len);

    Address = WriteFlashAddress;
    FLASHStatus = FLASH_COMPLETE;
	 __disable_irq(); // 关闭全局中断（关键：避免Flash操作时被中断打断）
    FLASH_Unlock();                                                                          // 解锁
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR); // 清除标志位
    FLASHStatus = FLASH_ErasePage(WriteFlashAddress);                                        // 擦除一页
    if (FLASHStatus == FLASH_COMPLETE)
    {
        for (k = 0; (k < write_len) && (FLASHStatus == FLASH_COMPLETE); k++)
        {
            FLASHStatus = FLASH_ProgramWord(Address, buff[k]); // 写字，一个字4字节
            if (FLASHStatus != FLASH_COMPLETE)
            {
                break; // 写入失败，退出循环
            }
            Address = Address + 4;                             // 地址+4字节
        }

        if (FLASHStatus == FLASH_COMPLETE)
        {
            FLASHStatus = FLASH_ProgramWord(Address, crc_value); // 在数据末尾追加CRC
        }

        FLASH_Lock(); // 上锁
				__enable_irq(); // 恢复全局中断
    }
    else
    {
        FLASH_Lock(); // 上锁
        __enable_irq(); // 恢复全局中断
        return 0;
    }
    if (FLASHStatus == FLASH_COMPLETE)
    {
        return 1;
    }
    return 0;
}

uint8_t flash_read(void)
{
    if (flash_read_words_with_crc(FlashData, FLASH_DATA_WORDS) != 1U)
    {
        for (uint8_t i = 0; i < FLASH_DATA_WORDS; i++)
        {
            FlashData[i] = 0xFFFFFFFFU;
        }
        memset(mqtt_client_id, 0, 16);
        usart2_send("Flash CRC check failed, use default config.\r\n", 44);
        return 0;
    }

    // 【新增】初始化时加载Flash中的IMEI到mqtt_client_id
    unsigned char flash_imei[16];
    memcpy(flash_imei, &FlashData[FLASH_IMEI_INDEX], 16);
    // 检查Flash中是否有有效IMEI（非0且非0xFF，且前15位为数字）
    int flash_imei_valid = 1;
    if (flash_imei[0] == 0 || flash_imei[0] == 0xFF)
    {
        flash_imei_valid = 0; // 未初始化
    }
    else
    {
        // 验证前15位是否为数字
        for (uint8_t i = 0; i < 15; i++)
        {
            if (flash_imei[i] < '0' || flash_imei[i] > '9')
            {
                flash_imei_valid = 0;
                break;
            }
        }
    }

    // 若Flash中IMEI有效，直接赋值给mqtt_client_id
    if (flash_imei_valid)
    {
        memcpy(mqtt_client_id, flash_imei, 16);
        usart2_send("Loaded IMEI from Flash: ", 23);
        usart2_send(mqtt_client_id, 16);
        usart2_send("\r\n", 2);
    }
    else
    {
        // Flash中无有效IMEI，等待串口接收（可设置默认值或提示）
        memset(mqtt_client_id, 0, 16);
        usart2_send("No valid IMEI in Flash, waiting for new...\r\n", 41);
    }

    return 1;
}
	




