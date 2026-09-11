







#ifndef _PI_SCRIPT_
#define _PI_SCRIPT_
#pragma once

#include "systems/rendering/effects/legacy_new/core/HoNewEffectHeader.h"
#ifndef _HO_NEW_WIN32_
extern "C"
{
   #include "HoBaram/NewEffect/Script/Lua/src/lua.h"
   #include "HoBaram/NewEffect/Script/Lua/src/lauxlib.h"
   #include "HoBaram/NewEffect/Script/Lua/src/lualib.h"
}
#endif

#ifdef  _HO_NEW_WIN32_
extern "C"
{
   #include "third_party/lua/legacy_win32/lua.h"
   #include "third_party/lua/legacy_win32/lauxlib.h"
   #include "third_party/lua/legacy_win32/lualib.h"
}
#endif

#pragma comment( lib, "..//Library//Lua.lib" )
#pragma comment( lib, "..//Library//Lualib.lib" )

class PiScript
{
public:
	PiScript(void);
	~PiScript(void);

private:
	lua_State *m_pLuaState;
private:
	void RegisterCommand(lua_State *state);
    void RegisterFunction(lua_CFunction fn, const char* name);

public:
	bool RunScriptFile(const char *fileName);
    bool RunScriptBuffer(const char *buffer);
};

#endif




