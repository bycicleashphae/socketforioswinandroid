
#ifndef _flsdjlfalskjfa_sdWrapper_h
#define _flsdjlfalskjfa_sdWrapper_h

#include "lua.hpp"
#define NULL (0)
#define LUNPLUS_METHOD_BEGIN(ClassName) lua::LuaPlus<ClassName>::RegFunction ClassName::functions[] = {
#define LUNPLUS_METHOD_DECLARE(ClassName, name) {#name, &ClassName::name},
#define LUNPLUS_METHOD_END    {NULL,NULL} \
};

#define LUNPLUS_DECLARE_INTERFACE(ClassName)	static const char className[]; \
static lua::LuaPlus<ClassName>::RegFunction functions[]; \
char m_instClassName[256]

#define LUNPLUS_DEFINE_INTERFACE(ClassName) const char ClassName::className[] = #ClassName

#define _L lua::state::Instance()->get_handle()

#define LUA_CALL(L, narg, nret)     lua_pcall(L, narg, nret, 0)




namespace lua
{
    /*!	\class state
     state wrapper, make a global lua_Stat*
     */
	class state
	{
		lua_State* L;
	public:
		state(bool bOpenLibs = true)
		{
			L = NULL;
	
		}
        
		~state()
		{
            //lua_gc(L, LUA_GCCOLLECT, 0);
			//lua_close(L);
		}
        
        void recreate(bool bOpenLibs = true)
        {
   //         if ( L )
   //         {
   //             lua_close(L);
   //         }

   //         L = lua_open();
			//if( bOpenLibs )
			//{
   //             luaL_openlibs(L);
			//}
        }
        
		lua_State* get_handle()
		{
			return L;
		}
        
		void setHook()
		{
			int startStackIndex = lua_gettop(L);

			const char* chunk = ""
				"function __ERROR_TRACKBACK__(errMsg)\n"
				"	local err = \"LUA ERROR: \" .. tostring(errMsg)\n"
				"	err = err .. debug.traceback()\n"
				"	return err\n"
				"end";

			if (luaL_loadstring(L, chunk) == 0)
			{
				lua_pcall(L, 0, 0, 0);
			}

			while (lua_gettop(L) > startStackIndex)
				lua_remove(L, startStackIndex + 1);
		}

		void set_handle(lua_State* l)
		{
			L = l;
		}

		static state* Instance()
		{
			static state S;
			return &S;
		}
	};

template <typename T> class LuaPlus {
    typedef struct { T *pT; } userdataType;
public:
    typedef int (T::*mfp)(lua_State *L);
    typedef struct { const char *name; mfp mfunc; } RegFunction;
    
    static void Register(lua_State *L) {
        lua_newtable(L);
        int methods = lua_gettop(L);
        
        luaL_newmetatable(L, T::className);
        int metatable = lua_gettop(L);
        
        // store method table in globals so that
        // scripts can add functions written in Lua.
        lua_pushvalue(L, methods);
        set(L, LUA_GLOBALSINDEX, T::className);
        
        // hide metatable from Lua getmetatable()
        lua_pushvalue(L, methods);
        set(L, metatable, "__metatable");
        
        lua_pushvalue(L, methods);
        set(L, metatable, "__index");
        
        lua_pushstring(L, T::className);
        lua_pushcclosure(L, tostring_T, 1);
        set(L, metatable, "__tostring");
        
        lua_pushcfunction(L, gc_T);
        set(L, metatable, "__gc");
        
        lua_newtable(L);                // mt for method table
        lua_pushstring(L, T::className);
        lua_pushcclosure(L, new_T, 1);
        lua_pushvalue(L, -1);           // dup new_T function
        set(L, methods, "new");         // add new_T to method table
        set(L, -3, "__call");           // mt.__call = new_T
        lua_setmetatable(L, methods);
        
        // fill method table with methods from class T
		for (RegFunction *l = T::functions; l->name; l++) {
            lua_pushstring(L, l->name);
            lua_pushlightuserdata(L, (void*)l);
            lua_pushstring(L, T::className);
            lua_pushcclosure(L, thunk, 2);
            lua_settable(L, methods);
        }
        
        lua_pop(L, 2);  // drop metatable and method table
    }
	static int RegisteSocketClassL(lua_State *L) {
		bool hasParentTable = lua_gettop(L) > 1;
		size_t len;
		const char* className = luaL_checklstring(L, 1, &len);
		const char* parentTableName = NULL;
		if (hasParentTable)
		{
			parentTableName = luaL_checklstring(L, 2, &len);
		}

		lua_newtable(L);
		int methods = lua_gettop(L);

		luaL_newmetatable(L, className);
		int metatable = lua_gettop(L);

		if (hasParentTable)
		{
			lua_getglobal(L, parentTableName);
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				lua_newtable(L);
				int flassClasses = lua_gettop(L);
				lua_pushvalue(L, flassClasses);
				set(L, LUA_GLOBALSINDEX, parentTableName);
			}

			lua_pushvalue(L, methods);
			set(L, -3, className);
			lua_pop(L, 1);
		}
		else
		{
			lua_pushvalue(L, methods);
			set(L, LUA_GLOBALSINDEX, className);
		}


		// hide metatable from Lua getmetatable()
		lua_pushvalue(L, methods);
		set(L, metatable, "__metatable");

		lua_pushvalue(L, methods);
		set(L, metatable, "__index");

		lua_pushstring(L, className);
		lua_pushcclosure(L, tostring_T, 1);
		set(L, metatable, "__tostring");

		lua_pushcfunction(L, gc_T);
		set(L, metatable, "__gc");

		lua_newtable(L);                // mt for method table
		lua_pushstring(L, className);
		lua_pushcclosure(L, new_T, 1);
		lua_pushvalue(L, -1);           // dup new_T function
		set(L, methods, "new");         // add new_T to method table
		set(L, -3, "__call");           // mt.__call = new_T
		lua_setmetatable(L, methods);

		// fill method table with methods from class T
		for (RegFunction *l = T::functions; l->name; l++) {
			lua_pushstring(L, l->name);
			lua_pushlightuserdata(L, (void*)l);
			lua_pushstring(L, className);
			lua_pushcclosure(L, thunk, 2);
			lua_settable(L, methods);
		}

		lua_pop(L, 2);  // drop metatable and method table
		return 0;
	}
    //check named lua method exist or not.
    static bool hasfunction(lua_State *L, const char *method, const char *className)
    {
        int base = lua_gettop(L);
        if (!luaL_checkudata(L, base, className))
        {
            return false;
        }
        
        lua_pushstring(L, method);
        lua_gettable(L, base);
        if (lua_isnil(L, -1))
        {
            lua_pop(L, 1);
            return false;
        }
        else
        {
            lua_pop(L, 1);
            return true;
        }
    }
    
    // call named lua method from userdata method table
    static int call(lua_State *L, const char *method, const char *className,
                    int nargs=0, int nresults=LUA_MULTRET, int errfunc=0)
    {
        int base = lua_gettop(L) - nargs;  // userdata index
        if (!luaL_checkudata(L, base, className)) {
            lua_settop(L, base-1);           // drop userdata and args
            lua_pushfstring(L, "not a valid %s userdata", className);
            return -1;
        }
        
        lua_pushstring(L, method);         // method name
        lua_gettable(L, base);             // get method from userdata
        if (lua_isnil(L, -1)) {            // no method?
            lua_settop(L, base-1);           // drop userdata and args
            lua_pushfstring(L, "%s missing method '%s'", className, method);
            return -1;
        }
        lua_insert(L, base);               // put method under userdata, args
        
        int status = lua_pcall(L, 1+nargs, nresults, errfunc);  // call method
        if (status) {
            const char *msg = lua_tostring(L, -1);
            if (msg == NULL) msg = "(error with no message)";
            lua_pushfstring(L, "%s:%s status = %d\n%s",
                            className, method, status, msg);
            lua_remove(L, base);             // remove old message
            return -1;
        }
        return lua_gettop(L) - base + 1;   // number of results
    }
    
    // push onto the Lua stack a userdata containing a pointer to T object
    static int push(lua_State *L, T *obj, bool gc, const char* className) {
        if (!obj) { lua_pushnil(L); return 0; }
        luaL_getmetatable(L, className);  // lookup metatable in Lua registry
        if (lua_isnil(L, -1)) luaL_error(L, "%s missing metatable", className);
        int mt = lua_gettop(L);
        subtable(L, mt, "userdata", "v");
        userdataType *ud = static_cast<userdataType*>(pushuserdata(L, obj, sizeof(userdataType)));
        if (ud) {
            ud->pT = obj;  // store pointer to object in userdata
            lua_pushvalue(L, mt);
            lua_setmetatable(L, -2);
            if (gc == false) {
                lua_checkstack(L, 3);
                subtable(L, mt, "do not trash", "k");
                lua_pushvalue(L, -2);
                lua_pushboolean(L, 1);
                lua_settable(L, -3);
                lua_pop(L, 1);
            }
        }
        lua_replace(L, mt);
        lua_settop(L, mt);
        return mt;  // index of userdata containing pointer to T object
    }
    
    // get userdata from Lua stack and return pointer to T object
    static T *check(lua_State *L, int narg, const char* className) {
        userdataType *ud =
        static_cast<userdataType*>(luaL_checkudata(L, narg, className));
        if(!ud) {
            luaL_typerror(L, narg, className);
            return NULL;
        }
        return ud->pT;  // pointer to T object
    }
    
private:
	LuaPlus();  // hide default constructor
    
    // member function dispatcher
    static int thunk(lua_State *L) {
        // get member function from upvalue
		RegFunction *l = static_cast<RegFunction*>(lua_touserdata(L, lua_upvalueindex(1)));
        const char* className = lua_tostring(L, lua_upvalueindex(2));
        // stack has userdata, followed by method args
        T *obj = check(L, 1, className);  // get 'self', or if you prefer, 'this'
        lua_remove(L, 1);  // remove self so member function args start at index 1
        return (obj->*(l->mfunc))(L);  // call member function
    }
    
    // create a new T object and
    // push onto the Lua stack a userdata containing a pointer to T object
    static int new_T(lua_State *L) {
        lua_remove(L, 1);   // use classname:new(), instead of classname.new()
        const char* className = lua_tostring(L, lua_upvalueindex(1));
        T *obj = new T(className);  // call constructor for T objects
        push(L, obj, true, className); // gc_T will delete this object
        return 1;           // userdata containing pointer to T object
    }
    
    // garbage collection metamethod
    static int gc_T(lua_State *L) {
        if (luaL_getmetafield(L, 1, "do not trash")) {
            lua_pushvalue(L, 1);  // dup userdata
            lua_gettable(L, -2);
            if (!lua_isnil(L, -1)) return 0;  // do not delete object
        }
        userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
		if (ud) {
			T *obj = ud->pT;
			if (obj){
				delete obj;  // call destructor for T objects
				ud->pT = NULL;
			}
		}
        return 0;
    }
    
    static int tostring_T (lua_State *L) {
        const char* className = lua_tostring(L, lua_upvalueindex(1));
        char buff[32];
        userdataType *ud = static_cast<userdataType*>(lua_touserdata(L, 1));
        T *obj = ud->pT;
        sprintf(buff, "%p", (void*)obj);
        lua_pushfstring(L, "%s (%s)", className, buff);
        
        return 1;
    }
    
    static void set(lua_State *L, int table_index, const char *key) {
        lua_pushstring(L, key);
        lua_insert(L, -2);  // swap value and key
        lua_settable(L, table_index);
    }
    
    static void weaktable(lua_State *L, const char *mode) {
        lua_newtable(L);
        lua_pushvalue(L, -1);  // table is its own metatable
        lua_setmetatable(L, -2);
        lua_pushliteral(L, "__mode");
        lua_pushstring(L, mode);
        lua_settable(L, -3);   // metatable.__mode = mode
    }
    
    static void subtable(lua_State *L, int tindex, const char *name, const char *mode) {
        lua_pushstring(L, name);
        lua_gettable(L, tindex);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_checkstack(L, 3);
            weaktable(L, mode);
            lua_pushstring(L, name);
            lua_pushvalue(L, -2);
            lua_settable(L, tindex);
        }
    }
    
    static void *pushuserdata(lua_State *L, void *key, size_t sz) {
        void *ud = 0;
        lua_pushlightuserdata(L, key);
        lua_gettable(L, -2);     // lookup[key]
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);         // drop nil
            lua_checkstack(L, 3);
            ud = lua_newuserdata(L, sz);  // create new userdata
            lua_pushlightuserdata(L, key);
            lua_pushvalue(L, -2);  // dup userdata
            lua_settable(L, -4);   // lookup[key] = userdata
        }
        return ud;
    }
};
	void	RegisteClassToScript();
	void	RegisteGlobalFunctions();
};

#endif
