//-------------------------------------------------------------------------------------------------
//	Created:	2015-6-23   17:47
//	File Name:	System.h
//	Author:		Eric(沙鹰)
//	PS:			如果发现说明错误，代码风格错误，逻辑错问题，设计问题，请告诉我。谢谢！
//  Email:		frederick.dang@gmail.com
//	Purpose:	IOCP需要包含的一些系统文件
//-------------------------------------------------------------------------------------------------

#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#include <WinSock2.h>
#include <stdio.h>
#include <assert.h>
#include <MSWSock.h>

#pragma comment(lib, "Ws2_32.lib")

#endif // _WIN32