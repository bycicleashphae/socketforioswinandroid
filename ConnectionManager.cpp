#include "assert.h"
#include "stdio.h"
#ifdef WIN32
#include <ws2tcpip.h>
#else
#include <netdb.h>
#endif
#include "ConnectionManager.h"
#include "LuaWrapper.h"
#include "lua.hpp"
#define CONNECT_TIMEOUT         5000
#ifdef WIN32
#include "SocketCommon.h"
#define socklen_t		long
#define DWOULDBLOCK		WSAEWOULDBLOCK
#else
#include <sys/select.h>
#include "SocketCommon.h"
#include "strings.h"
#define DWOULDBLOCK		EINPROGRESS
#endif

void CConnectionManager::OnTimeOut()
{
	Close();
	m_tOutTimer = -1;
	m_netState = NetConState_ConnectTimeOut;
	printf("%s%d",m_name.c_str(), NetErrorCode_ConnectTimeOut);
}
CConnectionManager::CConnectionManager()
{
    m_port		    = -1;
    m_pSocket	    = NULL;
	m_netState      = NetConState_Disconnected;
	m_tOutTimer     = -1;
	m_encryptInited = false;
    memset(m_server, 0, sizeof(m_server));
	m_name			= "";
}

CConnectionManager::~CConnectionManager()
{
    Close();
    ClearCached();
}

void CConnectionManager::Update(int dt)
{
	if (!m_pSocket
		|| GetState() == NetConState_ConnectTimeOut
		|| GetState() == NetConState_ConnectError
		|| GetState() == NetConState_Disconnected)
		return;
	CheckState();
	if (GetState() == NetConState_ConnectError)	return;
	if (GetState() == NetConState_Connecting)
	{
		if (m_pSocket->getWriteFlag())
		{
			if (CONNECT_TIMEOUT != m_tOutTimer)
			{
				OnSuccess();
			}
			else
			{
				m_tOutTimer -= dt;
				return;
			}
		}
		else
		{
			m_tOutTimer -= dt;
			if (m_tOutTimer <= 0)
			{
				m_tOutTimer = -1;
				OnTimeOut();
			}
		}
		return;
	}
	while (m_pSocket->getWriteFlag())
	{
		if (!m_sendList.empty())
		{
			NetMessage* pMessage = m_sendList.front();
			while (!pMessage->isEnd())
			{
				int sizeSended = m_pSocket->Send(pMessage->getCurrentBuf(), pMessage->getSendCap());
				if (sizeSended < 0)
				{
					OnError(NetErrorCode_SendZeroByte);
					return;
				}
				pMessage->onSend(sizeSended);
			}
			if (pMessage->isEnd())
			{
				delete pMessage;
			}
			m_sendList.pop_front();
		}
		else
		{
			break;
		}
		CheckState();
	}
	while (m_pSocket->getReadFlag()) {
		NetMessage* pMessage = NULL;
		if (!m_recvList.empty())
		{
			pMessage = m_recvList.back();
			if (pMessage->isEnd())
			{
				pMessage = NULL;
			}
		}

		if (!pMessage)
		{
			pMessage = new NetMessage();
			m_recvList.push_back(pMessage);
		}
		int sizeReceived = m_pSocket->Recv(pMessage->getReadBuf(), pMessage->getReadCap());
		if (sizeReceived <= 0)
		{
			OnError(NetErrorCode_RecvZeroByte);
			return;
		}

		if (pMessage->getSize() >= 0)
		{
			DecodeBuf(pMessage->getCurrentBuf(), sizeReceived);
		}
		pMessage->onReceive(sizeReceived);
		CheckState();
	}

}

bool CConnectionManager::DoConnect(const char* oldpAddr, int port)
{
	char pAddr[129];
	struct addrinfo*aip = NULL;
	struct addrinfo*res = NULL;
	memcpy(pAddr, oldpAddr, strlen(oldpAddr) + 1);
	memcpy(m_server, oldpAddr, strlen(oldpAddr) + 1);	
	m_port = port;
	
	if (GetUrlAddrinfo(oldpAddr, port, &res) != 0)
	{
		OnError(NetErrorCode_DNSError);
		return false;
	}
	m_pSocket = new CSocketCommon(); 
	aip = res;
	for (; aip; aip = aip->ai_next)
	{
		struct sockaddr_in * tmpaddrin = NULL;
		tmpaddrin = (struct sockaddr_in*)(aip->ai_addr);
		tmpaddrin->sin_port = htons(port);
		int ret = m_pSocket->CreateSocket(aip->ai_family, aip->ai_socktype, aip->ai_protocol);
		if (ret == -1)
		{
			if (aip->ai_next != NULL)
				continue;
			printf("ConnectToServer create socket failed, error id is %d \n", m_pSocket->GetError());
			OnError(NetErrorCode_CreateSocket);
			freeaddrinfo(res);
			return false;
		}		
#ifdef WIN32
		{
			u_long mode = 1;
			int iResult = ioctlsocket(m_pSocket->GetSocket(), FIONBIO, &mode);
			if (iResult != NO_ERROR)
			{
				printf("ioctlsocket failed with error: %ld\n", iResult);
			}
		}
#else
		{
			int flag = fcntl(m_pSocket->GetSocket(), F_GETFL, 0);
			fcntl(m_pSocket->GetSocket(), F_SETFL, flag | O_NONBLOCK);
		}
#endif
		
		if (ret < 0)
		{
#ifdef WIN32
			if (errno == WSAEAFNOSUPPORT || errno == WSAEPROTONOSUPPORT)
#else
			if (errno == EAFNOSUPPORT || errno == EPROTONOSUPPORT)
#endif
			{
				if (aip->ai_next)
					continue;
				else			
					break;
			}
		}
		else
		{
			ret = m_pSocket->ConnectTo(aip);
			if (ret == -1 && m_pSocket->GetError() != DWOULDBLOCK)
			{
				if (aip->ai_next)
					continue;
				printf("ConnectToServer connect failed, error id is %d \n", m_pSocket->GetError());
				OnError(NetErrorCode_ConnectToServer);
				freeaddrinfo(res);
				return false;
			}else
            {
                break;
            }
		}
	}
	freeaddrinfo(res);	
	m_netState = NetConState_Connecting;
	m_tOutTimer = CONNECT_TIMEOUT;
	return CheckState();
}
void CConnectionManager::OnError(int errorCode)
{
	Close();
	m_netState = NetConState_ConnectError;
	printf("%s%d", m_name.c_str(), errorCode);
}

CConnectionManager* CConnectionManager::GetInstance()
{
    if(m_instance == NULL) m_instance = new CConnectionManager();
    return m_instance;
}

CConnectionManager* CConnectionManager::m_instance = NULL;


void CConnectionManager::ReConnect()
{
    if(m_port == -1) return;
    Close();
	ClearCached();
    Connect(m_server, m_port);
	printf("%s", m_name.c_str());
}

void CConnectionManager::OnSuccess()
{
	m_netState = NetConState_Connected;
	m_tOutTimer = -1;
	printf("%s", m_name.c_str());
}

bool CConnectionManager::Connect(const char* oldpAddr, int port)
{
	if (m_port == -1)
	{
		return DoConnect(oldpAddr, port);		
	}
	else
	{
		Close();
		ClearCached();
		bool returnvalue = DoConnect(oldpAddr, port);
		printf("%s", m_name.c_str());
		return returnvalue;
	}
}

int CConnectionManager::GetUrlAddrinfo(const char*url, int port, struct addrinfo**out)
{
	//
	struct addrinfo hints;
	struct addrinfo *res = NULL;
	struct addrinfo *aip = NULL;
	int sockfd = -1;
	int errorCode = 0;
	char portStr[256];
	sprintf(portStr, "%d", port);
	memset(&hints,0,sizeof(hints));	
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;	
	errorCode = getaddrinfo(url, portStr, &hints, &res);
	if (errorCode != 0)
    {
		printf("getaddrinfo error content: %s", gai_strerror(errorCode));
        return -1;
    }
    *out = res;
	return 0;
 
}
 


void CConnectionManager::Close()
{
	m_encryptInited = false;
	if (m_pSocket == NULL)
	{
		return;
	}
	m_pSocket->CloseSocket();
	delete m_pSocket;
	m_pSocket = NULL;
	m_netState = NetConState_Disconnected;
}


bool CConnectionManager::CheckState()
{
	struct timeval timevalue;
    int socketFD = m_pSocket->GetSocket();    
	timevalue.tv_usec = 0;
	timevalue.tv_sec = 0;
    fd_set  t_read, t_write, t_error;
	FD_ZERO(&t_error);
	FD_SET(socketFD, &t_error);
	FD_ZERO(&t_write);
	FD_SET(socketFD, &t_write);
	FD_ZERO(&t_read);
	FD_SET(socketFD, &t_read);
	int ret = select(socketFD + 1, &t_read, &t_write, &t_error, &timevalue);
	if (ret < 0 || FD_ISSET(socketFD, &t_error))
    {
		m_pSocket->SetWriteFlag(false);
		m_pSocket->SetReadFlag(false);        
		if (GetState() == NetConState_Connecting)
			OnError(NetErrorCode_ConnectToServer);
		else
			OnError(NetErrorCode_Select);
        return false;
    }    
	m_pSocket->SetWriteFlag(FD_ISSET(socketFD, &t_write));
	m_pSocket->SetReadFlag(FD_ISSET(socketFD, &t_read));
	return true;
}


NetMessage* CConnectionManager::getMsg()
{
	return m_recvList.empty() ? NULL : m_recvList.front();
}


void CConnectionManager::RemoveMsg(NetMessage* pMsg)
{	
	for (std::list<NetMessage*>::iterator iter = m_recvList.begin(); iter != m_recvList.end(); ++iter)
    {
        if((*iter) == pMsg)
        {
            m_recvList.erase(iter);
            return;
        }
    }
}

void CConnectionManager::SendMsg(NetMessage* msg)
{
	msg->ResetForSend();
	EncodeMsg(msg);
	m_sendList.push_back(msg);
}
void CConnectionManager::InitEncrypt(int sendSeed, int recvSeed)
{
	m_encryptInited = true;
	for (std::list<NetMessage*>::iterator iter = m_sendList.begin(); iter != m_sendList.end(); ++iter)
	{
		EncodeMsg(*iter);
	}
}
/*
//unsigned long	seed = 243703;
int getencryptvalue(unsigned long seed)
{
	seed = (1664525L * seed + 1013904223L) & 0xffffffff);
	return seed;
}
*/
void CConnectionManager::DecodeBuf(void *buf, int len)
{
	if (m_encryptInited)
	{
	}
}
void CConnectionManager::EncodeMsg(NetMessage *msg)
{
	if (m_encryptInited)
	{
	}
}
int CConnectionManager::KeepAlived(int keepalive, int keepidle, int keepinterval, int keepcount)
{
	if (m_pSocket == NULL)
	{
		return -1;
	}

	return m_pSocket->KeepAlived(keepalive, keepidle, keepinterval, keepcount);
}

void CConnectionManager::Restart()
{
	if (m_instance != NULL)
		delete m_instance;
	m_instance = new CConnectionManager();
}
void CConnectionManager::ClearCached()
{
	for (std::list<NetMessage*>::iterator iterSend = m_sendList.begin(); iterSend != m_sendList.end(); ++iterSend)
	{
		delete *iterSend;
	}
	m_sendList.clear();
	for (std::list<NetMessage*>::iterator iter = m_recvList.begin(); iter != m_recvList.end(); ++iter)
	{
		delete *iter;
	}
	m_recvList.clear();
}
