#pragma once

#include <windows.h>
#include <string>
#include "diagnostics/RuntimeAssetProbe.h"

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

inline bool RuntimeRegularFileExists(const std::string &path)
{
    if(path.empty()) return false;
    const DWORD attributes = GetFileAttributesA(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES &&
           (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

inline std::string RuntimeCurrentDirectory()
{
    char current[4096] = {0};
    const DWORD length = GetCurrentDirectoryA(static_cast<DWORD>(sizeof(current)), current);
    if(length == 0 || length >= sizeof(current))
        return std::string();
    return std::string(current, current + length);
}

inline std::string ResolveRuntimePackPath(const char *packFileName)
{
    const std::string fileName = packFileName ? packFileName : "";
    const std::string executableDirectory = RuntimeExecutableDirectory();
    const std::string currentDirectory = RuntimeCurrentDirectory();

    // Shipped runtime content always wins. During source-tree development the
    // executable commonly lives in summit/client(e) while canonical packs are
    // rebuilt under summit/SourceCode/Content/packs. Resolve the first existing
    // pack instead of returning a non-existent exe-local path and silently
    // disabling localization/text resources.
    std::string candidates[7];
    int count = 0;
    if(!executableDirectory.empty())
    {
        candidates[count++] = RuntimeJoinPath(executableDirectory, "Content\\packs\\" + fileName);
        candidates[count++] = RuntimeJoinPath(executableDirectory, "..\\Content\\packs\\" + fileName);
        candidates[count++] = RuntimeJoinPath(executableDirectory, "..\\SourceCode\\Content\\packs\\" + fileName);
        candidates[count++] = RuntimeJoinPath(executableDirectory, "..\\..\\SourceCode\\Content\\packs\\" + fileName);
    }
    if(!currentDirectory.empty())
    {
        candidates[count++] = RuntimeJoinPath(currentDirectory, "Content\\packs\\" + fileName);
        candidates[count++] = RuntimeJoinPath(currentDirectory, "SourceCode\\Content\\packs\\" + fileName);
    }
    candidates[count++] = RuntimeJoinPath("Content\\packs", fileName);

    for(int i = 0; i < count; ++i)
    {
        summit::diagnostics::RuntimeAssetProbeAttempt("RUNTIME_PACK", "RESOLVE", candidates[i].c_str());
        const bool exists = RuntimeRegularFileExists(candidates[i]);
        summit::diagnostics::RuntimeAssetProbeResult("RUNTIME_PACK", "RESOLVE", candidates[i].c_str(), exists);
        if(exists)
            return candidates[i];
    }

    // Deterministic diagnostic path when the pack is genuinely missing.
    if(!executableDirectory.empty())
        return RuntimeJoinPath(executableDirectory, "Content\\packs\\" + fileName);
    return RuntimeJoinPath("Content\\packs", fileName);
}

}}}
