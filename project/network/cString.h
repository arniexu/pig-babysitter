#ifndef CSTRING_H_
#define CSTRING_H_

#ifndef CSTRING_C_ // 如果没有定义
#define CSTRING_Ex_ extern
#else
#define CSTRING_Ex_
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

void cStringFree(void);
void cStringRestore(void);
char *StrBetwString(char *Str, char *StrBegin, char *StrEnd);
void remove_colons(char *str);
int split(char *src, const char *separator, char **dest, int DestLen);
void HexToStr(char *pbSrc, char *pbDest, int nlen);
void StrToHex(char *pbSrc, char *pbDest, int nLen);

/**
 *@brief		用于解析字符串IP,返回1说明转换成功,换回0说明IP不可解析,可能是域名
 *@param		str 要转换的数据
 *@param		ip  转换后存储的位置
 *@return 	0:转换失败,可能是域名   1:转换成功
 *@example  net_dns_str_ip("192.168.0.1", &ip) ip[0]=192;ip[1]=168;ip[2]=0;ip[3]=1;
 */
uint8_t net_dns_str_ip(const char *str, void *ip);
#endif
