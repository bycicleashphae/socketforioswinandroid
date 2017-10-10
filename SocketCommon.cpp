#include "stdio.h"
#ifdef WIN32
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>

#endif

#include "SocketCommon.h"

CSocketCommon::CSocketCommon()
{
	m_readFlag    = false;
    m_writeFlag   = false;
    m_socket = -1;
#ifdef WIN32
	struct WSAData wsa;
	WORD wVersionRequested;
	wVersionRequested = MAKEWORD( 1, 1 );
	WSAStartup(wVersionRequested, &wsa);
#endif
}

CSocketCommon::CSocketCommon(int socket)
{
	m_readFlag    = false;
    m_writeFlag   = false;
    m_socket = socket;
}

CSocketCommon::~CSocketCommon()
{
#ifdef WIN32
    WSACleanup();
#endif
}

int CSocketCommon::GetError()
{
#ifdef WIN32
	return GetLastError();
#else
    return errno;
#endif
}

int CSocketCommon::CreateSocket(int family, int type, int protocal)
{
	m_socket = socket(family, type, protocal);
	if(m_socket == -1)
	{
		printf("error : create socket fialed, errno is %d \n", GetError());
	}
#ifndef WIN32
	else{
        int flag = 1;
        setsockopt(m_socket,IPPROTO_TCP,TCP_NODELAY,(char *) &flag,sizeof(int));
    }
#endif
	return m_socket;
}

int CSocketCommon::CloseSocket()
{
#ifdef WIN32
	closesocket(m_socket);
#else
	close(m_socket);
    m_socket = -1;
#endif
	return 1;
}

int CSocketCommon::Bind(const struct sockaddr* addr)
{
#ifdef WIN32
	return 0;
#else
    socklen_t len = sizeof(struct sockaddr);
    return bind(m_socket, addr, len);
#endif
}


int CSocketCommon::Listen(int count)
{
#ifdef WIN32
	return 0;
#else
    return listen(m_socket, count);
#endif
}

int CSocketCommon::Accept(struct sockaddr* addr)
{
#ifdef WIN32
	return 0;
#else
	socklen_t len = sizeof(struct sockaddr);
	return accept(m_socket, addr, &len);
#endif
}



int CSocketCommon::ConnectTo(const struct addrinfo* addr)
{
	return connect(m_socket, addr->ai_addr, addr->ai_addrlen);
}

int CSocketCommon::Send(char* buf, int sz)
{
	return send(m_socket, buf, sz, 0);
}

int CSocketCommon::Recv(char* buf, int size)
{
	return recv(m_socket, buf, size, 0);
}

int CSocketCommon::KeepAlived(int keepalive, int keepidle, int keepinterval, int keepcount)
#ifdef WIN32
{
return 0;
}
#else
{
    int rst = setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive ));
    if(keepalive != 0)
    {
#if defined(OS_ANDROID) || defined(OS_LINUX)
        setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&keepidle , sizeof(keepidle ));
#else
        setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPALIVE, (void*)&keepidle , sizeof(keepidle ));
#endif
        setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keepinterval , sizeof(keepinterval ));
        setsockopt(m_socket, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepcount , sizeof(keepcount ));
    }
    return rst;
}
#endif
