#ifndef CONFIGMODULENOBLOCK_H_
#define CONFIGMODULENOBLOCK_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "network_module.h"
#include "cString.h"

typedef struct
{
	uint32_t cnt;
	char flage;
	int case_value;
	uint32_t compare_value;
	uint32_t send_next_delay;
	int retry_cnt;
	char data_return_flage;
	char hope_return_data1[20];
	char hope_return_data2[20];
	void (*connect_dispose)(char *data, int len);
} config_module_noblock_context_t;

extern config_module_noblock_context_t g_config_module_noblock_context;

#define ConfigModuleNoBlockCnt (g_config_module_noblock_context.cnt)
#define ConfigModuleNoBlockFlage (g_config_module_noblock_context.flage)
#define ConfigModuleNoBlockCaseValue (g_config_module_noblock_context.case_value)
#define CompareValue (g_config_module_noblock_context.compare_value)
#define SendNextDelay (g_config_module_noblock_context.send_next_delay)
#define Cnt (g_config_module_noblock_context.retry_cnt)
#define DataReturnFlage (g_config_module_noblock_context.data_return_flage)
#define HopeReturnData1 (g_config_module_noblock_context.hope_return_data1)
#define HopeReturnData2 (g_config_module_noblock_context.hope_return_data2)
#define ConfigConnectDispose (g_config_module_noblock_context.connect_dispose)

void ConfigModuleNoBlock(void);

void FunctionParseCompare(char *data, int len); // 对比判断是否返回想要的数据
void SendConfigFunction(char *order, void (*FunctionSend)(), char *HopeReturn1, char *HopeReturn2, void (*FunctionParse)(char *data, int len), uint32_t ConfigFunctionValue);
// void espstart();

#endif
