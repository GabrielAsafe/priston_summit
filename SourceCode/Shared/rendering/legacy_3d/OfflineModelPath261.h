#pragma once
// Shared path validation for the Game.exe offline cache reader.
#include <cctype>
#include <cstring>
namespace summit_offline_model_261 {
inline bool SafeCachePath(const char* path, bool mesh)
{
    if (!path) return false;
    const size_t n = std::strlen(path);
    if (n < 5 || n > 120) return false;
    size_t component = 0;
    for (size_t i=0;i<n;++i) {
        const unsigned char c = static_cast<unsigned char>(path[i]);
        if (c == '/') {
            if (component == 0) return false;
            component = 0;
        } else {
            if (!(std::isalnum(c) || c == '_' || c == '-' || c == '.')) return false;
            ++component;
        }
    }
    if (!component) return false;
    const char* ptr=path;
    while (*ptr) {
        const char* slash=std::strchr(ptr,'/');
        const size_t len=slash ? static_cast<size_t>(slash-ptr) : std::strlen(ptr);
        if ((len==1 && ptr[0]=='.') || (len==2 && ptr[0]=='.' && ptr[1]=='.')) return false;
        ptr=slash ? slash+1 : ptr+len;
    }
    const char* ext=path+n-4;
    const char* expected=mesh ? ".smd" : ".smb";
    for (int i=0;i<4;++i)
        if (std::tolower(static_cast<unsigned char>(ext[i])) != expected[i]) return false;
    return true;
}
}
