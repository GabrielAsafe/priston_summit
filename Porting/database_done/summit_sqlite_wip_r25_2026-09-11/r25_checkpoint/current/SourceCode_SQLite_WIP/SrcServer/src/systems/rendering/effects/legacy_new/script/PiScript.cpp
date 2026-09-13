
#include <windows.h>
#include "systems/rendering/effects/legacy_new/script/PiScript.h"
#include "systems/rendering/effects/legacy_new/script/PiScriptLuaCommand.h"

PiScript::PiScript(void) : m_pLuaState(NULL)
{
	
}

PiScript::~PiScript(void)
{

}







void PiScript::RegisterCommand(lua_State *state)
{
	
}







void PiScript::RegisterFunction(lua_CFunction fn, const char* name)
{
	
}







bool PiScript::RunScriptFile(const char *fileName)
{

	return true;
}







bool PiScript::RunScriptBuffer(const char *buffer)
{

	return true;
}


