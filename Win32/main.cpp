//-------------------------------------------------------------------------------------------------
//	Created:	2015-6-23   17:46
//	File Name:	main.cpp
//	Author:		Eric(沙鹰)
//	PS:			如果发现说明错误，代码风格错误，逻辑错问题，设计问题，请告诉我。谢谢！
//  Email:		frederick.dang@gmail.com
//	Purpose:	完成端口练习测试
//-------------------------------------------------------------------------------------------------
#include "System.h"


#define ASSERT(e, info) {if(!e) {printf(info); fflush(stdout); assert(false);}}
#define OverLappedBufferLen 10240




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

DWORD ThreadProcess(LPVOID pParam)
{

}



void AddNewAcceptConn(SOCKET ListenConn, int addNum)
{
	for (int a = 0; a < addNum; a++)
	{

	}
}


void Flush(LPVOID pParam)
{

}

int main()
{
	WSADATA wsData;
	ASSERT(WSAStartup(MAKEWORD(2, 2), &wsData) == 0, "WSAStartup Failed.\n");

	HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 2);
	ASSERT(hIOCP != NULL, "CreateIoCompletionPort Failed.\n");

	SOCKET Conn = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, 0, 0, WSA_FLAG_OVERLAPPED);
	ASSERT(Conn != INVALID_SOCKET, "WSASocket Failed.\n");

	// 关联监听连接和完成端口
	if (CreateIoCompletionPort((HANDLE)Conn, hIOCP, (DWORD_PTR)&Conn, 0))
		ASSERT(false, "CreateIoCompletionPort Associate IOCP with Conn Failed.\n");


	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(20055);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // htonl(INADDR_ANY);

	if (bind(Conn, (PSOCKADDR)&addr, sizeof(addr)) == SOCKET_ERROR)
		ASSERT(false, "bind Failed.\n");
	
	// 参数2：	The maximum length of the queue of pending connections.  
	//			If set to SOMAXCONN, the underlying service provider responsible for socket s will set the backlog to a maximum reasonable value. 
	//			There is no standard provision to obtain the actual backlog value
	//			等待队列的最大长度。
	if (listen(Conn, SOMAXCONN) == SOCKET_ERROR)
		ASSERT(false, "listen Failed.\n");


	// Load the AcceptEx function into memory using WSAIoctl. The WSAIoctl function is an extension of the ioctlsocket()
	// function that can use overlapped I/O. The function's 3rd through 6th parameters are input and output buffers where
	// we pass the pointer to our AcceptEx function. This is used so that we can call the AcceptEx function directly, rather
	// than refer to the Mswsock.lib library.
	GUID GuidAcceptEx = WSAID_ACCEPTEX;	 // WSAID_GETACCEPTEXSOCKADDRS	
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	DWORD dwBytes;
	int iResult = WSAIoctl(Conn, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof (GuidAcceptEx),
		&lpfnAcceptEx, sizeof (lpfnAcceptEx),
		&dwBytes, NULL, NULL);


	// WSAEventSelect();

	



	return 1;
}