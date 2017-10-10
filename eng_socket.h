#pragma once
#include "lua.hpp"
#define SOCKET_NAMESPACE "socketext"


#include <set>
#include "LuaWrapper.h"

#include "ConnectionManager.h"

class SocketObject 
{
public:
	SocketObject(const char* instClassName);
	~SocketObject();
private:
	CConnectionManager * m_pConnectionMgr;
	std::string m_socketObjectName;
public:
	static std::set<SocketObject*> s_objList;
public:
	CConnectionManager * GetConnection();
	void Restart();
	std::string getSocketName(){ return m_socketObjectName; };
	
	int GetName(lua_State *L);
	int UpdateNetMsg(lua_State *L);
	int HasPendingMsg(lua_State *L);	
	int CloseConnection(lua_State *L);
	int ConnectToServer(lua_State *L);
	int KeepAlived(lua_State *L);
	int InitPike(lua_State *L);
	int ReConnectToServer(lua_State *L);
	
	LUNPLUS_DECLARE_INTERFACE(SocketObject);
};
extern SocketObject*GetDefaultSocket();
extern SocketObject*GetSocketByName(const char * socketname);
static int connectToServer(lua_State *L);
static int reconnectToServer(lua_State *L);
static int hasPendingMsg(lua_State *L);
static int updateNetMsg(lua_State *L);
static int initPike(lua_State *L);
static int keepAlived(lua_State *L);
int ext_lua_socket(lua_State *L);
