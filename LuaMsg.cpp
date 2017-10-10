#include "LuaMsg.h"

#include "ConnectionManager.h"
#include "eng_socket.h"
CLuaMsg::CLuaMsg(const char* instClassName)
{
    strncpy(m_instClassName, instClassName, 64);
    m_pMsg = NULL;
}

CLuaMsg::~CLuaMsg()
{
    if(m_pMsg)
    {
        delete m_pMsg;
        m_pMsg = NULL;
    }
}

int CLuaMsg::CreateNewMessage(lua_State *L)
{
	m_pMsg = new NetMessage(512);
    return 1;
}

int CLuaMsg::LoadMsg(lua_State* L)
{
	int n = lua_gettop(L);
	if (n == 0)
	{
		m_pMsg = GetDefaultSocket()->GetConnection()->getMsg();
		GetDefaultSocket()->GetConnection()->RemoveMsg(m_pMsg);
		m_pMsg->ResetForRead();
		m_pMsg->dump();
		return 1;
	}
	else
	{
		size_t len;
		const char *socketname = luaL_checklstring(L, 1, &len);
		SocketObject*tmp = GetSocketByName(socketname);
		m_pMsg = tmp->GetConnection()->getMsg();
		tmp->GetConnection()->RemoveMsg(m_pMsg);
		m_pMsg->ResetForRead();
		m_pMsg->dump();
	}
	
	
    return 1;
}

int CLuaMsg::GetSize(lua_State *L)
{
	lua_pushinteger(L, m_pMsg->getSize());
	return 1;
}

int CLuaMsg::ReadRaw(lua_State *L)
{
	int size = luaL_checkinteger(L, 1);
	const char* buf = m_pMsg->rRaw(size);
	lua_pushlstring(L, buf, size);
	return 1;
}

int CLuaMsg::WriteRaw(lua_State *L)
{
	size_t size = 0;
	const char* buf = luaL_checklstring(L, 1, &size);
	m_pMsg->wRaw(buf, (int)size);
	return 1;
}

int CLuaMsg::SendMsg(lua_State *L)
{
	int n = lua_gettop(L);
	if (n == 0)
	{
		GetDefaultSocket()->GetConnection()->SendMsg(m_pMsg);
	}
	else
	{
		size_t len;
		const char *socketname = luaL_checklstring(L, 1, &len);
		SocketObject*tmp = GetSocketByName(socketname);
		tmp->GetConnection()->SendMsg(m_pMsg);
	}    
    m_pMsg = NULL;
    return 1;
}

int CLuaMsg::BeginForWrite(lua_State *L)
{
    m_pMsg->beginForWrite();
    return 1;
}

int CLuaMsg::EndForWrite(lua_State *L)
{
    m_pMsg->endForWrite();
    return 1;
}

LUNPLUS_DEFINE_INTERFACE(CLuaMsg);
LUNPLUS_METHOD_BEGIN(CLuaMsg)
LUNPLUS_METHOD_DECLARE(CLuaMsg, CreateNewMessage)
LUNPLUS_METHOD_DECLARE(CLuaMsg, LoadMsg)
LUNPLUS_METHOD_DECLARE(CLuaMsg, BeginForWrite)
LUNPLUS_METHOD_DECLARE(CLuaMsg, EndForWrite)
LUNPLUS_METHOD_DECLARE(CLuaMsg, SendMsg)
LUNPLUS_METHOD_DECLARE(CLuaMsg, GetSize)
LUNPLUS_METHOD_DECLARE(CLuaMsg, ReadRaw)
LUNPLUS_METHOD_DECLARE(CLuaMsg, WriteRaw)
LUNPLUS_METHOD_END