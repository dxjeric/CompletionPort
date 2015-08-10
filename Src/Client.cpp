//-------------------------------------------------------------------------------------------------
//	Created:	2015-6-23   17:46
//	File Name:	c.cpp
//	Author:		Eric(沙鹰)
//	PS:			如果发现说明错误，代码风格错误，逻辑错问题，设计问题，请告诉我。谢谢！
//  Email:		frederick.dang@gmail.com
//	Purpose:	完成端口练习测试
//-------------------------------------------------------------------------------------------------
#include "System.h"
#include "Common.h"


#define IOCP_ASSERT(e, info) {if(!(e)) {printf(info); fflush(stdout); assert(false);}}
#define OverLappedBufferLen 10240
#define WaitingAcceptCon 5
#define AcceptExSockAddrInLen (sizeof(SOCKADDR_IN) + 16)
#define MustPrint(s) {printf("Must >> %s\n", s); fflush(stdout);}
#define TestIPAddr "192.168.150.128"


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

int main()
{
	WSADATA wsData;
	IOCP_ASSERT(WSAStartup(MAKEWORD(2, 2), &wsData) == 0, "WSAStartup Failed.\n");

	SOCKET Conn = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, 0, 0, WSA_FLAG_OVERLAPPED);
	IOCP_ASSERT(Conn != INVALID_SOCKET, "WSASocket Failed.\n");
	MustPrint("Create Socket OK.\n");

	// 长连接
	BOOL bKeepAlive = true;
	setsockopt(Conn, SOL_SOCKET, SO_KEEPALIVE, (char*)&bKeepAlive, sizeof(bKeepAlive));

	// 长连接的检测
	tcp_keepalive tIn, tOut;
	tIn.onoff = 1;
	tIn.keepalivetime = 1000;
	tIn.keepaliveinterval = 1000;
	DWORD dwBytes;
	WSAIoctl(Conn, SIO_KEEPALIVE_VALS, (LPVOID)&tIn, sizeof(tcp_keepalive), (LPVOID)&tOut, sizeof(tcp_keepalive), &dwBytes, 0, 0);

	//// 
	int nReuseAddr = 1;
	setsockopt(Conn, SOL_SOCKET, SO_REUSEADDR, (const char*)&nReuseAddr, sizeof(int));
	
	// 设置当有数据在待发送区时，closesocket的行为
	linger tLinger;
	tLinger.l_onoff = 1;
	tLinger.l_linger = 0;
	setsockopt(Conn, SOL_SOCKET, SO_LINGER, (const char*)&tLinger, sizeof(tLinger));
	
	// 关闭Nagle算法
	BOOL bNoDelay = true;
	setsockopt(Conn, IPPROTO_IP, TCP_NODELAY, (const char*)&bNoDelay, sizeof(BOOL));

	// 域名转IP
	//struct hostent* h;
	//if ((h = gethostbyname("163.com")) != NULL)
	//	char* ip = inet_ntoa(*((struct in_addr*)h->h_addr));

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6666);
	addr.sin_addr.s_addr = inet_addr(TestIPAddr); // htonl(INADDR_ANY);
	//WSAConnect(Conn, (sockaddr*)&addr, sizeof(addr), );
	if (connect(Conn, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		IOCP_ASSERT(false, "connect Failed.\n");
	}
	MustPrint("connect Server OK.\n");


	char pBuf[OverLappedBufferLen];
	int iErrorNo;
	while (true)
	{
		Sleep(100);
		ZeroMemory(pBuf, OverLappedBufferLen);

		int nRet = recv(Conn, pBuf, OverLappedBufferLen, 0);
		if (nRet <= 0 && ((iErrorNo = WSAGetLastError()) != WSAEWOULDBLOCK))
		{
			printf("WSAGetLastError = %d.\n", iErrorNo);
			// IOCP_ASSERT(false, "recv Error.\n");
			break;
		}
		else
		{
			printf("Recv From Server [%s].\n", pBuf);

			sprintf_s(pBuf, "Client new send time %d", GetTickCount());
			nRet = send(Conn, pBuf, strlen(pBuf), 0);
			if (nRet <= 0 && ((iErrorNo = WSAGetLastError()) != WSAEWOULDBLOCK))
			{
				printf("WSAGetLastError = %d.\n", iErrorNo);
				IOCP_ASSERT(false, "send Error.\n");
				return 0;
			}
		}
	}
	
	return 1;
}