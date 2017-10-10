 
#ifndef testset_netmesage_h_lasjdlfals
#define testset_netmesage_h_lasjdlfals

#include "LuaWrapper.h"

#include "NetMessage.h"


class CLuaMsg
{
public:
	CLuaMsg(const char* instClassName);
	~CLuaMsg();
    
public:
    
    int LoadMsg(lua_State *L);
    int CreateNewMessage(lua_State *L);

	int GetSize(lua_State *L);
	int ReadRaw(lua_State *L);
	int WriteRaw(lua_State *L);
    
	int BeginForWrite(lua_State *L);
    int EndForWrite(lua_State *L);
    
    int SendMsg(lua_State *L);
    
	LUNPLUS_DECLARE_INTERFACE(CLuaMsg);
private:
	NetMessage* m_pMsg;
};


#endif
