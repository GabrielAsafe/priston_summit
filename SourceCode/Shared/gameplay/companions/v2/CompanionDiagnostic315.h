#pragma once
// SUMMIT 315 Companion diagnostics. No character names, account IDs, credentials,
// item names, chat or DAT bytes. No packet layout or persistence changes.
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>
#include <array>

namespace summit { namespace companion { namespace diagnostic {
inline void Trace(const char* side, const char* stage, int action, int value, int result, int info = 0)
{
    static std::mutex logMutex;
    std::lock_guard<std::mutex> lock(logMutex);
    const char* filename = (side && side[0] == 'S') ?
        "summit_companion_server_315.log" : "summit_companion_client_315.log";
    // Bounded rolling log: maximum 2 x 2 MiB on each side.
    std::FILE* f = std::fopen(filename, "a+");
    if(!f) return;
    if(std::fseek(f,0,SEEK_END) == 0 && std::ftell(f) > 2*1024*1024)
    {
        std::fclose(f);
        const std::string archive = std::string(filename) + ".1";
        std::remove(archive.c_str());
        std::rename(filename,archive.c_str());
        f = std::fopen(filename,"a");
        if(!f) return;
    }
    std::fprintf(f,"%lld side=%s stage=%s action=%d value=%d result=%d info=%d\n",
        static_cast<long long>(std::time(nullptr)), side ? side : "?",
        stage ? stage : "?",action,value,result,info);
    std::fflush(f);
    std::fclose(f);
}

// The UI receives many routine state updates after an error. Keep a separate,
// acknowledgeable history so a successful refresh cannot erase the failure.
struct ErrorEntry { int result=0; unsigned repeats=1; };
struct ClientErrorHistory
{
    std::array<ErrorEntry, 8> entries{};
    unsigned count=0;
    void Record(int result)
    {
        if(result >= 0) return;
        if(count && entries[0].result == result) { ++entries[0].repeats; return; }
        for(unsigned i=(count < entries.size() ? count : static_cast<unsigned>(entries.size()-1)); i>0; --i)
            entries[i]=entries[i-1];
        entries[0] = ErrorEntry{result,1};
        if(count < entries.size()) ++count;
    }
    void Clear() { count=0; }
    void DismissLatest() {
        if(!count) return;
        for(unsigned i=1;i<count;++i) entries[i-1]=entries[i];
        --count;
    }
};
inline ClientErrorHistory& Errors()
{
    static ClientErrorHistory state;
    return state;
}
}}}
