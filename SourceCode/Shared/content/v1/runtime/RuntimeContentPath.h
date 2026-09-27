#pragma once

#include <windows.h>
#include <string>

namespace summit { namespace content { namespace runtime {

inline std::string RuntimeExecutableDirectory()
{
    char modulePath[4096] = {0};
    const DWORD length = GetModuleFileNameA(NULL, modulePath, static_cast<DWORD>(sizeof(modulePath)));
    if(length == 0 || length >= sizeof(modulePath))
        return std::string();

    std::string path(modulePath, modulePath + length);
    const std::string::size_type slash = path.find_last_of("\\/");
    if(slash == std::string::npos)
        return std::string();
    return path.substr(0, slash);
}

inline std::string RuntimeJoinPath(const std::string &left, const std::string &right)
{
    if(left.empty()) return right;
    const char tail = left[left.size() - 1];
    if(tail == '\\' || tail == '/') return left + right;
    return left + "\\" + right;
}

inline std::string ResolveRuntimePackPath(const char *packFileName)
{
    const std::string fileName = packFileName ? packFileName : "";
    const std::string executableDirectory = RuntimeExecutableDirectory();

    // Production contract: every standalone client/server deployment owns its
    // runtime content. Development/source trees are never consulted at runtime.
    if(!executableDirectory.empty())
        return RuntimeJoinPath(executableDirectory, "Content\\packs\\" + fileName);

    // Keep diagnostics deterministic even if GetModuleFileNameA itself fails.
    return RuntimeJoinPath("Content\\packs", fileName);
}

}}}
