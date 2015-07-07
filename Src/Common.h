//-------------------------------------------------------------------------------------------------
//	Created:	2015-7-7   14:57
//	File Name:	Common.h
//	Author:		Eric(沙鹰)
//	PS:			如果发现说明错误，代码风格错误，逻辑错问题，设计问题，请告诉我。谢谢！
//  Email:		frederick.dang@gmail.com
//	Purpose:	Common
//-------------------------------------------------------------------------------------------------
#include "System.h"

// log 相关
#define PrintLogInfoLen 256
#define LogPrintFlush(hFile) fflush(hFile)

void LogPrint(const char* strFormat, ...);
#define Log(strFormat, ...) {LogPrint("%s-%s-%d: "##strFormat, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);}

inline void LogPrint(const char* strFormat, ...)
{
	char strLog[PrintLogInfoLen];
	va_list vlArgs;
	va_start(vlArgs, strFormat);
	int offset = vsnprintf_s(strLog, PrintLogInfoLen, strFormat, vlArgs);
	va_end(vlArgs);
	printf("%s\n", strLog);
	LogPrintFlush(stdout);
}

