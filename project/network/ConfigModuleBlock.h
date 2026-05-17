#ifndef CONFIGMODULEBLOCK_H_
#define CONFIGMODULEBLOCK_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "usart.h"
#include "iwdg.h"

typedef struct
{
	int cnt;
	int flage;
	int delay;
	char *data;
} config_module_block_context_t;

extern config_module_block_context_t g_config_module_block_context;

#define ConfigModuleBlockDelay (g_config_module_block_context.delay)

/**
 * @brief  发送指令配置模块,阻塞版
 * @param  dat:      发送的数据
 * @param  returnc:  预期返回的数据1
 * @param  returncc: 预期返回的数据2
 * @param  执行一个函数
 * @param  send_cnt: 尝试次数
 * @param  send_cnt: 尝试间隔时间
 * @retval 1:配置当前指令OK
 * @example
 **/
char ConfigModuleBlock(char *send_data, int send_len, char *returnc, char *returncc, char (*fun)(char *fun_data, int fun_data_len), int send_cnt, int timet_out);

/**
 * @brief  接收数据
 * @param  none
 * @param  none
 * @param  none
 * @retval none
 * @example
 **/
int ConfigModuleBlockRead(char **data);
#endif
