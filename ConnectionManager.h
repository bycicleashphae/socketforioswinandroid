#ifndef __CONNNNECTRIONNKJJITMGRL_KLKLLKJWIOEJF_H__
#define __CONNNNECTRIONNKJJITMGRL_KLKLLKJWIOEJF_H__
#include <list>
#include "SocketCommon.h"
#include "NetMessage.h"
#include <string>
#define NetConState_Connecting         (0)
#define NetConState_ConnectTimeOut	   (1)
#define NetConState_ConnectError	   (2)
#define NetConState_Connected		   (3)
#define NetConState_Disconnected	   (4)

#define NetErrorCode_CreateSocket	   (0)
#define NetErrorCode_ConnectToServer   (1)
#define NetErrorCode_Select		       (2)
#define NetErrorCode_RecvZeroByte      (3)
#define NetErrorCode_SendZeroByte      (4)
#define NetErrorCode_DNSError   	   (10)
#define NetErrorCode_ConnectTimeOut	   (11)



class CConnectionManager
{
public:
	static CConnectionManager* GetInstance();
	CConnectionManager();
	~CConnectionManager();
	int     GetUrlAddrinfo(const char*url, int port, struct addrinfo**out);
	int     KeepAlived(int keepalive, int keepidle, int keepinterval, int keepcount);
	bool    Connect(const char* pAddr, int port);
	bool    DoConnect(const char* pAddr, int port);    
    void    ReConnect();    
    void    Close();
	void    SendMsg(NetMessage* msg);
	void    InitEncrypt(int send, int recv);    
    void    DecodeBuf(void *buf, int len);
	void    EncodeMsg(NetMessage *msg);
	void    ClearCached();
	void    Restart();
	
	int     GetState() const { return m_netState; }
	NetMessage*   getMsg();
	void    RemoveMsg(NetMessage* Msg);
	void    SetName(const char * name){ m_name = name; }
	void    Update(int dt);
private:
	void    OnTimeOut();
	void    OnError(int errorID);
	void    OnSuccess();
	bool    CheckState();
	void    CreateNewMsg();
	char                    m_server[128];
	int                     m_port;
	std::list<NetMessage*>  m_sendList;
	std::list<NetMessage*>  m_recvList;    
	CSocketCommon*          m_pSocket;
    int                     m_netState;
    bool                    m_encryptInited;    
	static CConnectionManager*  m_instance;
	std::string             m_name;
	int                     m_tOutTimer;
};

#endif