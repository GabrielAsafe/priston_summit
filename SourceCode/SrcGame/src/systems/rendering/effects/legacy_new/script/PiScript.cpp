







#include <windows.h>
#include <string>
#include <cstring>
#include "systems/rendering/effects/legacy_new/script/PiScript.h"
#include "systems/rendering/effects/legacy_new/script/PiScriptLuaCommand.h"

PiScript::PiScript(void) : m_cachedScriptBytes(0), m_pLuaState(NULL)
{
	m_pLuaState = lua_open();
	lua_baselibopen( m_pLuaState);
	
	RegisterCommand(m_pLuaState);
}

PiScript::~PiScript(void)
{
	if(m_pLuaState != NULL)
	{
		lua_close(m_pLuaState);
		m_pLuaState = NULL;
	}
}







void PiScript::RegisterCommand(lua_State *state)
{
    RegisterFunction(luaCommand_Begin,                  "Begin");
    RegisterFunction(luaCommand_End,                    "End");
    RegisterFunction(luaCommand_InitPos,                "InitPos");
    RegisterFunction(luaCommand_InitColor,              "InitColor");
    RegisterFunction(luaCommand_InitSize,               "InitSize");

    RegisterFunction(luaCommand_InitMeshName,           "InitMeshName");
    RegisterFunction(luaCommand_InitMaxFrame,           "InitMaxFrame");
    RegisterFunction(luaCommand_InitLoop,               "InitLoop");
    RegisterFunction(luaCommand_InitTextureName,        "InitTextureName");
    RegisterFunction(luaCommand_InitAniTextureName,     "InitAniTextureName");
    
    RegisterFunction(luaCommand_InitStartDelayTime,     "InitStartDelayTime");
    RegisterFunction(luaCommand_InitEndTime,            "InitEndTime");

    RegisterFunction(luaCommand_EventColor,             "EventColor");
    RegisterFunction(luaCommand_EventFadeColor,         "EventFadeColor");

    RegisterFunction(luaCommand_EventSize,              "EventSize");
    RegisterFunction(luaCommand_EventFadeSize,          "EventFadeSize");

    RegisterFunction(luaCommand_InitSpawnBoundingBox,      "InitSpawnBoundingBox");
    RegisterFunction(luaCommand_InitSpawnBoundingSphere,   "InitSpawnBoundingSphere");
    RegisterFunction(luaCommand_InitSpawnBoundingDoughnut, "InitSpawnBoundingDoughnut");

    RegisterFunction(luaCommand_InitParticleType,       "InitParticleType");
    RegisterFunction(luaCommand_InitVelocityType,       "InitVelocityType");

    RegisterFunction(luaCommand_InitParticleNum,        "InitParticleNum");
    RegisterFunction(luaCommand_InitEmitRate,           "InitEmitRate");
    RegisterFunction(luaCommand_InitVelocity,           "InitVelocity");

    RegisterFunction(luaCommand_InitAxialPos,           "InitAxialPos");
    RegisterFunction(luaCommand_InitBlendType,          "InitBlendType");
}







void PiScript::RegisterFunction(lua_CFunction fn, const char* name)
{
	lua_pushstring(m_pLuaState, name);
	lua_pushlightuserdata(m_pLuaState, this);
	lua_pushcclosure(m_pLuaState, fn, 1);
	lua_settable(m_pLuaState, LUA_GLOBALSINDEX);
	lua_settop(m_pLuaState, 0);
}







namespace {
// Bounded cache: enough for all known skill scripts but cannot grow forever.
static const size_t SUMMIT_LUA_MAX_ENTRY = 1024u * 1024u;
static const size_t SUMMIT_LUA_MAX_CACHE = 8u * 1024u * 1024u;
static const size_t SUMMIT_LUA_MAX_FILES = 128u;

static int SummitCaptureLuaBytecode(lua_State *, const void *bytes, size_t len, void *userdata)
{
    std::string *output = static_cast<std::string *>(userdata);
    if (!bytes || len > SUMMIT_LUA_MAX_ENTRY || output->size() > SUMMIT_LUA_MAX_ENTRY - len)
        return 1; // Abort capturing; the loaded Lua function is still executable.
    output->append(static_cast<const char *>(bytes), len);
    return 0;
}
}

bool PiScript::RunScriptFile(const char *fileName)
{
    if (!m_pLuaState || !fileName || !*fileName)
        return false;

    WIN32_FILE_ATTRIBUTE_DATA attrs;
    const bool hasIdentity = GetFileAttributesExA(fileName, GetFileExInfoStandard, &attrs) != 0 &&
        (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    const std::string key(fileName);
    std::map<std::string, SummitCachedScript>::iterator it = m_cachedScripts.find(key);
    if (it != m_cachedScripts.end())
    {
        const bool current = hasIdentity &&
            it->second.sizeLow == attrs.nFileSizeLow &&
            it->second.sizeHigh == attrs.nFileSizeHigh &&
            it->second.modifiedLow == attrs.ftLastWriteTime.dwLowDateTime &&
            it->second.modifiedHigh == attrs.ftLastWriteTime.dwHighDateTime;
        if (!current)
        {
            m_cachedScriptBytes -= it->second.bytecode.size();
            m_cachedScripts.erase(it);
            it = m_cachedScripts.end();
        }
    }

    bool loadedFromCache = false;
    if (it != m_cachedScripts.end())
    {
        const std::string &bytes = it->second.bytecode;
        if (luaL_loadbuffer(m_pLuaState, bytes.data(), bytes.size(), fileName) == 0)
            loadedFromCache = true;
        else
        {
            // Corrupt/stale Lua bytecode must never replace the original script.
            lua_pop(m_pLuaState, 1);
            m_cachedScriptBytes -= bytes.size();
            m_cachedScripts.erase(it);
        }
    }

    if (!loadedFromCache)
    {
        if (luaL_loadfile(m_pLuaState, fileName))
        {
            MessageBox(NULL, lua_tostring(m_pLuaState, -1), "File: Lua Syntax Error", MB_OK);
            lua_pop(m_pLuaState, 1);
            return false;
        }
        // lua_dump compiles once per source revision, but does not execute the
        // Lua chunk. Each skill use calls lua_pcall separately, preserving the
        // original creation of fresh effect instances and callback side effects.
        WIN32_FILE_ATTRIBUTE_DATA afterRead;
        const bool unchangedDuringRead = hasIdentity &&
            GetFileAttributesExA(fileName, GetFileExInfoStandard, &afterRead) != 0 &&
            afterRead.nFileSizeLow == attrs.nFileSizeLow &&
            afterRead.nFileSizeHigh == attrs.nFileSizeHigh &&
            afterRead.ftLastWriteTime.dwLowDateTime == attrs.ftLastWriteTime.dwLowDateTime &&
            afterRead.ftLastWriteTime.dwHighDateTime == attrs.ftLastWriteTime.dwHighDateTime;
        if (unchangedDuringRead && attrs.nFileSizeHigh == 0 &&
            attrs.nFileSizeLow <= SUMMIT_LUA_MAX_ENTRY &&
            m_cachedScripts.size() < SUMMIT_LUA_MAX_FILES)
        {
            std::string bytecode;
            if (lua_dump(m_pLuaState, &SummitCaptureLuaBytecode, &bytecode) == 0 &&
                !bytecode.empty() && bytecode.size() <= SUMMIT_LUA_MAX_CACHE - m_cachedScriptBytes)
            {
                SummitCachedScript &entry = m_cachedScripts[key];
                entry.bytecode.swap(bytecode);
                entry.sizeLow = attrs.nFileSizeLow;
                entry.sizeHigh = attrs.nFileSizeHigh;
                entry.modifiedLow = attrs.ftLastWriteTime.dwLowDateTime;
                entry.modifiedHigh = attrs.ftLastWriteTime.dwHighDateTime;
                m_cachedScriptBytes += entry.bytecode.size();
            }
        }
    }
    if (lua_pcall(m_pLuaState, 0, 0, 0))
    {
        MessageBox(NULL, lua_tostring(m_pLuaState, -1), "File: Lua Syntax Error", MB_OK);
        lua_pop(m_pLuaState, 1);
        return false;
    }
    return true;
}





bool PiScript::RunScriptBuffer(const char *buffer)
{
	if(m_pLuaState == 0)
		return false;
	
	if (luaL_loadbuffer(m_pLuaState, buffer, strlen(buffer), "line") || lua_pcall(m_pLuaState, 0, 0, 0))
    {
		MessageBox(NULL, lua_tostring(m_pLuaState, -1), "Buffer: Lua Syntax Error", MB_OK);
		lua_pop(m_pLuaState, 1);
		return false;
	}

	return true;
}




