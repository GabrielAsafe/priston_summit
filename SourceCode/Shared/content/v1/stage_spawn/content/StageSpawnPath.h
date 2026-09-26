#pragma once

#include <cctype>
#include <string>

namespace summit
{
namespace content
{
namespace stage_spawn
{

inline std::string NormalizeStageSpawnPath(const char *path)
{
    std::string out;
    if(!path) return out;
    bool slash = false;
    for(const unsigned char *p = reinterpret_cast<const unsigned char *>(path); *p; ++p)
    {
        unsigned char ch = *p;
        if(ch == '\\' || ch == '/')
        {
            if(!out.empty() && !slash) out.push_back('/');
            slash = true;
            continue;
        }
        slash = false;
        if(ch < 0x80)
            out.push_back(static_cast<char>(std::tolower(ch)));
        else
            out.push_back(static_cast<char>(ch));
    }
    while(out.size() >= 2 && out[0] == '.' && out[1] == '/') out.erase(0,2);
    while(!out.empty() && out[out.size()-1] == '/') out.resize(out.size()-1);
    return out;
}

} // namespace stage_spawn
} // namespace content
} // namespace summit
