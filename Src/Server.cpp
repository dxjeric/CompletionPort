//-------------------------------------------------------------------------------------------------
//	Created:	2015-6-23   17:46
//	File Name:	Server.cpp
//	Author:		Eric(沙鹰)
//	PS:			如果发现说明错误，代码风格错误，逻辑错问题，设计问题，请告诉我。谢谢！
//  Email:		frederick.dang@gmail.com
//	Purpose:	完成端口练习测试
//-------------------------------------------------------------------------------------------------
#include "System.h"


#define IOCP_ASSERT(e, info) {if(!(e)) {printf(info); printf("GetLastError [%d].\n", WSAGetLastError()); fflush(stdout); assert(false);}}
#define OverLappedBufferLen 10240
#define WaitingAcceptCon 2
#define AcceptExSockAddrInLen (sizeof(SOCKADDR_IN) + 16)
#define MustPrint(s) {printf("Must >> %s\n", s); fflush(stdout);}
#define PrintLogInfoLen 256


void Log(const char* strFormat, ...)
{
	char strLog[PrintLogInfoLen];
	va_list vlArgs;
	va_start(vlArgs, strFormat);
	int offset = vsnprintf_s(strLog, PrintLogInfoLen, strFormat, vlArgs);
	va_end(vlArgs);
	printf("%s\n", strLog);
}

typedef struct OverLapped
{
public:
	typedef enum OverLappedOperatorType
	{
		EOLOT_Accept = 0,
		EOLOT_Send,
		EOLOT_Recv,
	} OLOpType;

public:
	WSAOVERLAPPED	sysOverLapped;
	WSABUF			sysBuffer;
	char			dataBuffer[OverLappedBufferLen];
	OLOpType		opType;

public:
	OverLapped();
} OverLapped, *OverLappedPtr;

inline OverLapped::OverLapped()
{
	ZeroMemory(&sysOverLapped, sizeof(sysOverLapped));
	sysBuffer.buf = dataBuffer;
}

struct ThreadInfo
{
	HANDLE hIOCP;
	//LPFN_ACCEPTEX lpfAccepEx;
	SOCKET Conn;
};


DWORD ThreadProcess(LPVOID pParam)
{
	ThreadInfo* pThreadInfo = (ThreadInfo*)pParam;

	HANDLE hIOCP = pThreadInfo->hIOCP;
	SOCKET ListenConn = pThreadInfo->Conn;
	OverLapped* pOver = NULL;
	SOCKET* pConn	  = NULL;
	DWORD	dwBytes;
	DWORD	dwFlag;

	for (;;)
	{
		GetQueuedCompletionStatus(hIOCP, &dwBytes, (PULONG_PTR)&pConn, (LPOVERLAPPED*)&pOver, INFINITE);

		if (!pConn && !pOver)
			return 0;

		
		if ((dwBytes == 0 && (pOver->opType == OverLapped::OLOpType::EOLOT_Send || pOver->opType == OverLapped::OLOpType::EOLOT_Recv)) || (pOver->opType == OverLapped::OLOpType::EOLOT_Accept && WSAGetLastError() == WSA_OPERATION_ABORTED))
		{
			closesocket(*pConn);
			delete pOver;
		}
		else
		{
			switch (pOver->opType)
			{
				case OverLapped::OLOpType::EOLOT_Accept:
				{
					SOCKET AcceptConn = *pConn;
					int iLocalAddr, iRemoteAddr;
					LPSOCKADDR pLocalAddr;
					sockaddr_in* pRemoteAddr = NULL;
					GetAcceptExSockaddrs(pOver->sysBuffer.buf, 0, AcceptExSockAddrInLen, AcceptExSockAddrInLen, 
										(PSOCKADDR*)&pLocalAddr, &iLocalAddr, (PSOCKADDR*)&pRemoteAddr, &iRemoteAddr);

					printf("new connect: %d.%d.%d.%d\n", pRemoteAddr->sin_addr.s_net, pRemoteAddr->sin_addr.s_host, pRemoteAddr->sin_addr.s_lh, pRemoteAddr->sin_addr.s_impno);
					
					// 更新连接进来的Socket，希望ClientSocket具有和ListenSocket相同的属性，对ClientSocket调用SO_UPDATE_ACCEPT_CONTEXT
					if (setsockopt(*pConn, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)ListenConn, sizeof(ListenConn)) == SOCKET_ERROR)
					{
						closesocket(AcceptConn);
						delete pOver;
						break;
					}

					// IOCP管理连接
					if (!CreateIoCompletionPort((HANDLE)AcceptConn, hIOCP, AcceptConn, 0))
					{
						closesocket(AcceptConn);
						delete pOver;
						break;
					}

					pOver->opType = OverLapped::EOLOT_Recv;
					pOver->sysBuffer.len = OverLappedBufferLen;

					// 等待接受数据
					int nResult = WSARecv(AcceptConn, &pOver->sysBuffer, 1, &dwBytes, &dwFlag, &pOver->sysOverLapped, 0);
					if (nResult == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
					{
						closesocket(AcceptConn);
						delete pOver;
						break;
					}

					// 发送第一个数据					
					OverLapped* pSendOver = new OverLapped;
					pSendOver->opType = OverLapped::OLOpType::EOLOT_Send;
					ZeroMemory(pSendOver->dataBuffer, OverLappedBufferLen);
					sprintf_s(pSendOver->dataBuffer, "server new send [%d].\n", GetTickCount());
					int nResult2 = WSASend(*pConn, &pSendOver->sysBuffer, 1, &dwBytes, 0, &pSendOver->sysOverLapped, 0);
					if (nResult2 == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
					{
						closesocket(*pConn);
						delete pOver;
						break;
					}
				}break; // OverLapped::OLOpType::EOLOT_Accept

				case OverLapped::OLOpType::EOLOT_Send:
				{
					delete pOver;
				}break; // OverLapped::OLOpType::EOLOT_Send
				
				case OverLapped::OLOpType::EOLOT_Recv:
				{
					char* pData = pOver->dataBuffer;
					printf("%s\n", pData);

					// 等待接受下一组数据
					int nResult = WSARecv(*pConn, &pOver->sysBuffer, 1, &dwBytes, &dwFlag, &pOver->sysOverLapped, 0);
					if (nResult == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
					{
						closesocket(*pConn);
						delete pOver;
						break;
					}

					// 模拟发送测试数据
					OverLapped* pSendOver = new OverLapped;
					pSendOver->opType = OverLapped::OLOpType::EOLOT_Send;
					ZeroMemory(pSendOver->dataBuffer, OverLappedBufferLen);
					sprintf_s(pSendOver->dataBuffer, "server new send [%d].\n", GetTickCount());
					int nResult2 = WSASend(*pConn, &pSendOver->sysBuffer, 1, &dwBytes, 0, &pSendOver->sysOverLapped, 0);
					if (nResult2 == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
					{
						closesocket(*pConn);
						delete pOver;
						break;
					}
				}break; // OverLapped::OLOpType::EOLOT_Recv
			}
		}
	}
}


void AddWaitingAcceptConn(SOCKET sListenConn, LPFN_ACCEPTEX lpfnAcceptEx)
{
	for (int a = 0; a < WaitingAcceptCon; a++)
	{
		SOCKET AcceptConn = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, 0, 0, WSA_FLAG_OVERLAPPED);
		if (AcceptConn == INVALID_SOCKET)
			return;
		
		Log("WSASocket new AccepteConn [%d].", AcceptConn);
		OverLapped* pAcceptExOverLapped = new OverLapped;
		pAcceptExOverLapped->opType = OverLapped::OLOpType::EOLOT_Accept;
		pAcceptExOverLapped->sysBuffer.len = (DWORD)AcceptConn;

		DWORD dwBytes;
		BOOL bRet = AcceptEx(sListenConn, AcceptConn, pAcceptExOverLapped->sysBuffer.buf, 0, AcceptExSockAddrInLen, AcceptExSockAddrInLen, &dwBytes, &pAcceptExOverLapped->sysOverLapped);		
		if (!bRet && WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("WSAGetLastError = [%d].\n", WSAGetLastError());
			delete pAcceptExOverLapped;
			return;
		}
	}

	MustPrint("AddWaitingAcceptConn OK.\n");
}



void Flush(SOCKET sListenConn, HANDLE hAcceptExEvent, LPFN_ACCEPTEX lpfnAcceptEx)
{
	DWORD dwResult = WaitForSingleObject(hAcceptExEvent, 0);

	if (dwResult == WAIT_FAILED)
	{
		IOCP_ASSERT(false, "WaitForSingleObject return WAIT_FAILED.\n");
	}
	else if (dwResult != WAIT_TIMEOUT)
	{
		AddWaitingAcceptConn(sListenConn, lpfnAcceptEx);
	}
}

int main()
{
	WSADATA wsData;
	IOCP_ASSERT(WSAStartup(MAKEWORD(2, 2), &wsData) == 0, "WSAStartup Failed.\n");

	HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 2);
	IOCP_ASSERT(hIOCP != NULL, "CreateIoCompletionPort Failed.\n");

	SOCKET sLinstenConn = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, 0, 0, WSA_FLAG_OVERLAPPED);
	IOCP_ASSERT(sLinstenConn != INVALID_SOCKET, "WSASocket Failed.\n");
	MustPrint("Socket Create Ok.\n");

	//int nReuseAddr = 1;
	//setsockopt(Conn, SOL_SOCKET, SO_REUSEADDR, (const char*)&nReuseAddr, sizeof(int));

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6666);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // inet_addr("127.0.0.1"); // htonl(INADDR_ANY);

	if (bind(sLinstenConn, (PSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR)
		IOCP_ASSERT(false, "bind Failed.\n");
	
	// 参数2：	The maximum length of the queue of pending connections.  
	//			If set to SOMAXCONN, the underlying service provider responsible for socket s will set the backlog to a maximum reasonable value. 
	//			There is no standard provision to obtain the actual backlog value
	//			等待队列的最大长度。
	if (listen(sLinstenConn, SOMAXCONN) == SOCKET_ERROR)
		IOCP_ASSERT(false, "listen Failed.\n");

	MustPrint("Listen OK.\n");

	// 关联监听连接和完成端口
	if (!CreateIoCompletionPort((HANDLE)sLinstenConn, hIOCP, (DWORD_PTR)&sLinstenConn, 0))
		IOCP_ASSERT(false, "CreateIoCompletionPort Associate IOCP with Conn Failed.\n");

	MustPrint("Create IOCP OK.\n");

	// Load the AcceptEx function into memory using WSAIoctl. The WSAIoctl function is an extension of the ioctlsocket()
	// function that can use overlapped I/O. The function's 3rd through 6th parameters are input and output buffers where
	// we pass the pointer to our AcceptEx function. This is used so that we can call the AcceptEx function directly, rather
	// than refer to the Mswsock.lib library.
	GUID GuidAcceptEx = WSAID_ACCEPTEX;	 // WSAID_GETACCEPTEXSOCKADDRS	
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	DWORD dwBytes;
	int iResult = WSAIoctl(sLinstenConn, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof (GuidAcceptEx),
		&lpfnAcceptEx, sizeof (lpfnAcceptEx),
		&dwBytes, NULL, NULL);

	// 创建工作线程, 线程关联数据
	ThreadInfo tThreadInfo;
	tThreadInfo.hIOCP = hIOCP;
	tThreadInfo.Conn = sLinstenConn;
	//tThreadInfo.lpfAccepEx = lpfnAcceptEx;
	HANDLE hWorkThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadProcess, &tThreadInfo, 0, 0);
	IOCP_ASSERT(hWorkThread, "CreateThread Failed.\n");
	MustPrint("CreateThread OK.\n");

	// 创建事件，在AcceptEx中的预分配的连接使用完时，再次创建
	HANDLE hAcceptExEvent = CreateEvent(0, false, false, 0);
	IOCP_ASSERT(hAcceptExEvent, "CreateEvent Failed.\n");
	iResult = WSAEventSelect(sLinstenConn, hAcceptExEvent, FD_ACCEPT);
	IOCP_ASSERT(iResult != SOCKET_ERROR, "WSAEventSelect Failed.\n");
	MustPrint("Event Select OK.\n");

	// 添加第一批的预创建连接
	AddWaitingAcceptConn(sLinstenConn, lpfnAcceptEx);

	while (true)
	{
		Flush(sLinstenConn, hAcceptExEvent, lpfnAcceptEx);
	}

	return 1;
}