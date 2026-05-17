#ifndef network_module_h_
#define network_module_h_

#include <string.h>
#include <stdio.h>

#include "stm32f0xx.h"

#include "delay.h"
#include "cString.h"

#include "ConfigModuleBlock.h"

typedef struct
{
	char network_config_flage; // 是不是在配网
	uint8_t network_flag;	   // 驻网标志
	char ssid[32];			   // 记录路由器名称
	char password[64];		   // 记录密码
	char mac[18];			   // 记录设备MAC
	char ip[21];			   // 记录设备连接路由器分得的IP
} network_module_config_state_t;

typedef struct
{
	char flag;
	char cnt;
	char count;
	char start;
	char id;
	char buff[20];
	char buff_len;
	int data_len;
} network_module_recv_parse_state_t;

extern network_module_config_state_t g_network_module_config_state;
extern network_module_recv_parse_state_t g_network_module_recv_parse_state;

#define network_config_flage (g_network_module_config_state.network_config_flage)
#define network_flag (g_network_module_config_state.network_flag)

/**
 * @brief  APUConfig
 * @ warn  None
 * @param  None
 * @param  None
 * @param  None
 * @param  None
 * @retval 1:³É¹¦
 * @example
 **/
char network_module_apuconfig(void);
char config8266_smart_airkiss(void);

/**
 * @brief  network_module_rst_set
 * @param  value 1:ON; 0:OFF
 * @param  None
 * @param  None
 * @retval None
 * @example
 **/
void network_module_rst_set(int value);

/**
 * @brief  Á¬½ÓÂ·ÓÉÆ÷
 * @param  ap_name:Á¬½ÓµÄÂ·ÓÉÆ÷Ãû³Æ
 * @param  ap_password:Á¬½ÓµÄÂ·ÓÉÆ÷ÃÜÂë
 * @param  None
 * @param  None
 * @retval None
 * @warnig None
 * @example
 **/
void network_module_connect_ap(char *ap_name, char *ap_password);

/**
 * @brief  ½âÎöNBÄ£×é·µ»ØµÄÍøÂçÊý¾Ý
 * @param  Res Ä£×é·µ»ØµÄÊý¾Ý
 * @param  return_data ·µ»Ø½âÎöÖ®ºóµÄÊý¾Ý(Ö»±£ÁôÍøÂçÊý¾Ý)
 * @param  None
 * @retval 1: ½âÎö³É¹¦
 * @example
 **/
char network_module_extract_data(char Res, char *return_data, char *return_id);
#endif
