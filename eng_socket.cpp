#include "assert.h"
#include "eng_socket.h"
#include "ConnectionManager.h"
#include "lua.hpp"

//------------------------------------------------------ Lua --------------------------------------------------
static int hasPendingMsg(lua_State *L)
{
	return GetDefaultSocket()->HasPendingMsg(L);

}

static int connectToServer(lua_State *L)
{
	return GetDefaultSocket()->ConnectToServer(L);
}

static int reconnectToServer(lua_State *L)
{
	return GetDefaultSocket()->ReConnectToServer(L);
}

static int updateNetMsg(lua_State *L)
{
	return GetDefaultSocket()->UpdateNetMsg(L);
}

static int closeConnection(lua_State *L)
{
	return GetDefaultSocket()->CloseConnection(L);
}

static int initPike(lua_State *L)
{
	return GetDefaultSocket()->InitPike(L);
}

static int keepAlived(lua_State *L)
{
	return GetDefaultSocket()->KeepAlived(L);
}

static int generateSecret(lua_State *L)
{
	unsigned int key1 = (unsigned int)luaL_checkinteger(L, 1);
	unsigned int key2 = (unsigned int)luaL_checkinteger(L, 2);
	unsigned int u1 = (unsigned int)(key2 >> 3) & 0xFF;
	unsigned int u2 = (unsigned int)(key2 >> 23) & 0xFF;
	unsigned int u3 = (unsigned int)(key1 >> 5) & 0xFF;
	unsigned int u4 = (unsigned int)(key1 >> 15) & 0xFF;
	unsigned int key = (u1 << 24) | (u3 << 16) | (u2 << 8) | u4;
	lua_pushinteger(L,key);
	return 1;
}
static const struct luaL_Reg emptyfuns[] = {
    {NULL, NULL}
};

static const struct luaL_Reg funs[] = {
    {"hasPendingMsg", hasPendingMsg},
    {"updateNetMsg", updateNetMsg},
    {"closeConnection", closeConnection},
    {"connectToServer", connectToServer},
    {"keepAlived",keepAlived},
    {"initPike", initPike},
	{"generateSecret", generateSecret },
    {"reconnectToServer", reconnectToServer},
    {NULL, NULL}
};

int ext_lua_socket(lua_State *L) {
	int __recordStackIndex = lua_gettop((L));
    
    luaL_newmetatable(L, SOCKET_NAMESPACE);
	luaL_register(L, NULL, emptyfuns);
	luaL_register(L, SOCKET_NAMESPACE, funs);
    
    lua_pushvalue(L, -2);
    lua_setmetatable(L, -2);
    
	while (lua_gettop((L)) > (__recordStackIndex + (0))) lua_remove((L), __recordStackIndex + 1);
    return 0;
}


SocketObject*GetDefaultSocket()
{
	return GetSocketByName("defaultSocket");
}
SocketObject * GetSocketByName(const char * socketname)
{
	for (std::set<SocketObject*>::iterator it = SocketObject::s_objList.begin(); it != SocketObject::s_objList.end(); ++it)
	{
		if ((*it)->getSocketName().compare(socketname) == 0)
		{
			return (*it);
		}
	}
	return NULL;
}
SocketObject::SocketObject(const char* instClassName)
{
	m_socketObjectName = instClassName;
	s_objList.insert(this);
	m_pConnectionMgr = NULL;
}
SocketObject::~SocketObject()
{
	if(m_pConnectionMgr)
	{
		m_pConnectionMgr->Close();	
		m_pConnectionMgr->ClearCached();
	}
	s_objList.erase(this);
}
CConnectionManager*SocketObject::GetConnection()
{
	if (m_pConnectionMgr == NULL)
	{
		m_pConnectionMgr = new CConnectionManager;
		m_pConnectionMgr->SetName(getSocketName().c_str());
	}	
	return m_pConnectionMgr;
}
void SocketObject::Restart()
{
	if (m_pConnectionMgr != NULL)
	{
		delete m_pConnectionMgr;
	}
	m_pConnectionMgr = new CConnectionManager;
	m_pConnectionMgr->SetName(getSocketName().c_str());
}
std::set<SocketObject*> SocketObject::s_objList;
int SocketObject::HasPendingMsg(lua_State *L)
{	
	NetMessage* pCurrentReadMsg = GetConnection()->getMsg();
	if (pCurrentReadMsg && pCurrentReadMsg->isEnd())
	{
		lua_pushboolean(L, true);
	}
	else
	{
		lua_pushboolean(L, false);
	}
	return 1;
}

int SocketObject::CloseConnection(lua_State *L)
{
	GetConnection()->Close();	
	GetConnection()->ClearCached();
	return 1;
}

int SocketObject::ConnectToServer(lua_State *L)
{
	const char* pAddr = luaL_checkstring(L, 1);
	int port = luaL_checkinteger(L, 2);
	bool needEncrypt = luaL_checkinteger(L, 3) == 1;
	bool isSuccess = GetConnection()->Connect(pAddr, port);
	lua_pushboolean(L, isSuccess);
	return 1;
}
int SocketObject::KeepAlived(lua_State *L)
{
	int keepalive = (int)luaL_checkinteger(L, 1);
	int keepidle = (int)luaL_checkinteger(L, 2);
	int keepinterval = (int)luaL_checkinteger(L, 3);
	int keepcount = (int)luaL_checkinteger(L, 4);

	int rst = GetConnection()->KeepAlived(keepalive, keepidle, keepinterval, keepcount);
	lua_pushinteger(L, rst);
	return 1;
}
int SocketObject::InitPike(lua_State *L)
{
	int sendSeed = (int)luaL_checkinteger(L, 1);
	int recvSeed = (int)luaL_checkinteger(L, 2);
	GetConnection()->InitEncrypt(sendSeed, recvSeed);
	return 1;
}
int SocketObject::ReConnectToServer(lua_State *L)
{
	GetConnection()->ReConnect();
	return 1;
}
int SocketObject::GetName(lua_State *L)
{
	lua_pushstring(L,this->getSocketName().c_str());
	return 1;
}

int SocketObject::UpdateNetMsg(lua_State*L)
{
	int dt = luaL_checkinteger(L, 1);
	GetConnection()->Update(dt);
	return 1;
}
LUNPLUS_DEFINE_INTERFACE(SocketObject);
LUNPLUS_METHOD_BEGIN(SocketObject)
LUNPLUS_METHOD_DECLARE(SocketObject, GetName)
LUNPLUS_METHOD_DECLARE(SocketObject, HasPendingMsg)
LUNPLUS_METHOD_DECLARE(SocketObject, CloseConnection)
LUNPLUS_METHOD_DECLARE(SocketObject, ConnectToServer)
LUNPLUS_METHOD_DECLARE(SocketObject, KeepAlived)
LUNPLUS_METHOD_DECLARE(SocketObject, InitPike)
LUNPLUS_METHOD_DECLARE(SocketObject, ReConnectToServer)
LUNPLUS_METHOD_DECLARE(SocketObject, UpdateNetMsg)
LUNPLUS_METHOD_END
