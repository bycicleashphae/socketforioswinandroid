#ifndef _SOCKETIioweiofjiowioefioiowioefo_H__
#define _SOCKETIioweiofjiowioefioiowioefo_H__
#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#endif
 
class CSocketCommon 
{
public:
    CSocketCommon();
    
    void    SetWriteFlag(bool val)  {   m_writeFlag = val;    }
	
    void    SetReadFlag(bool val)   {   m_readFlag = val;     }

    int     getWriteFlag() const     {   return m_writeFlag;   }
	
	int     getReadFlag() const       {   return m_readFlag;    }
    
	CSocketCommon(int socket);

    ~CSocketCommon();
    
    virtual int     Recv(char* buf, int sz) ;
    
    virtual int     Send(char* buf, int sz) ;
    
    virtual int     CreateSocket(int family, int type, int protocal) ;
    
    virtual int     CloseSocket() ;
    
    virtual int     Bind(const struct sockaddr* addr)  ;
    
    virtual int     Listen(int count)   ;
    
    virtual int     Accept(struct sockaddr* addr)  ;
    
    virtual int     GetError()     ;
    
	virtual int     ConnectTo(const struct addrinfo* addr);

    int             GetSocket() const   {   return m_socket;    }

	virtual int     KeepAlived(int keepalive, int keepidle, int keepinterval, int keepcount);
    
private:
    bool    m_readFlag;
    bool    m_writeFlag;
    int     m_socket;   
};

#endif