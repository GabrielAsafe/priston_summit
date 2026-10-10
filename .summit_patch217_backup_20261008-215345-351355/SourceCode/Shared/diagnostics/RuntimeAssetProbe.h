#pragma once

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <string>
#include <unordered_set>

namespace summit { namespace diagnostics {

// Runtime asset attempt probe. Enabled by default because missing/incorrect
// legacy assets often fail silently. Set SUMMIT_RUNTIME_ASSET_PROBE=0 to
// disable it for performance-sensitive release runs.
inline bool RuntimeAssetProbeEnabled()
{
    const char *value = std::getenv("SUMMIT_RUNTIME_ASSET_PROBE");
    return !value || !*value || *value != '0';
}

inline const char *RuntimeAssetProbeRole()
{
#ifdef _W_SERVER
    return "SERVER";
#else
    return "CLIENT";
#endif
}

inline std::string RuntimeAssetProbeNormalizePath(const char *path)
{
    std::string out;
    if(!path) return out;
    for(const unsigned char *p = reinterpret_cast<const unsigned char *>(path); *p; ++p)
    {
        unsigned char ch = *p;
        if(ch == '/') ch = '\\';
        if(ch >= 'A' && ch <= 'Z') ch = static_cast<unsigned char>(ch - 'A' + 'a');
        if(ch == '\t' || ch == '\r' || ch == '\n') ch = ' ';
        out.push_back(static_cast<char>(ch));
    }
    return out;
}

struct RuntimeAssetProbeState
{
    std::mutex mutex;
    FILE *attempts;
    FILE *unique;
    std::unordered_set<std::string> seen;
    bool sessionHeaderWritten;
    RuntimeAssetProbeState() : attempts(NULL), unique(NULL), sessionHeaderWritten(false) {}
};

inline RuntimeAssetProbeState &RuntimeAssetProbeGetState()
{
    // Deliberately process-lifetime storage. Asset loads may happen while other
    // global systems are being torn down and logging must remain safe there.
    static RuntimeAssetProbeState *state = new RuntimeAssetProbeState();
    return *state;
}

inline void RuntimeAssetProbeOpenLocked(RuntimeAssetProbeState &state)
{
    if(state.attempts && state.unique) return;
    CreateDirectoryA("LOGS", NULL);
    if(!state.attempts)
        fopen_s(&state.attempts, "LOGS\\runtime_asset_attempts.log", "a");
    if(!state.unique)
        fopen_s(&state.unique, "LOGS\\runtime_asset_unique.log", "a");
    if(!state.sessionHeaderWritten)
    {
        char exe[2048] = {0};
        GetModuleFileNameA(NULL, exe, static_cast<DWORD>(sizeof(exe)));
        const DWORD tick = GetTickCount();
        if(state.attempts)
        {
            fprintf(state.attempts, "SESSION_BEGIN\ttick=%lu\tpid=%lu\trole=%s\texe=%s\n",
                (unsigned long)tick, (unsigned long)GetCurrentProcessId(), RuntimeAssetProbeRole(), exe);
            fflush(state.attempts);
        }
        if(state.unique)
        {
            fprintf(state.unique, "SESSION_BEGIN\ttick=%lu\tpid=%lu\trole=%s\texe=%s\n",
                (unsigned long)tick, (unsigned long)GetCurrentProcessId(), RuntimeAssetProbeRole(), exe);
            fflush(state.unique);
        }
        state.sessionHeaderWritten = true;
    }
}

inline void RuntimeAssetProbeAttempt(const char *category, const char *operation, const char *path)
{
    if(!RuntimeAssetProbeEnabled() || !path || !*path) return;
    RuntimeAssetProbeState &state = RuntimeAssetProbeGetState();
    const std::string normalized = RuntimeAssetProbeNormalizePath(path);
    std::lock_guard<std::mutex> guard(state.mutex);
    RuntimeAssetProbeOpenLocked(state);
    if(state.attempts)
    {
        fprintf(state.attempts, "%lu\t%lu\t%s\t%s\t%s\t%s\n",
            (unsigned long)GetTickCount(), (unsigned long)GetCurrentThreadId(),
            RuntimeAssetProbeRole(), category ? category : "UNKNOWN",
            operation ? operation : "TRY", path);
        fflush(state.attempts);
    }
    const std::string key = std::string(category ? category : "UNKNOWN") + "\t" + normalized;
    if(state.seen.insert(key).second && state.unique)
    {
        fprintf(state.unique, "%s\t%s\t%s\n",
            RuntimeAssetProbeRole(), category ? category : "UNKNOWN", path);
        fflush(state.unique);
    }
}

inline void RuntimeAssetProbeResult(const char *category, const char *operation, const char *path, bool ok)
{
    if(!RuntimeAssetProbeEnabled() || !path || !*path) return;
    RuntimeAssetProbeState &state = RuntimeAssetProbeGetState();
    std::lock_guard<std::mutex> guard(state.mutex);
    RuntimeAssetProbeOpenLocked(state);
    if(state.attempts)
    {
        fprintf(state.attempts, "%lu\t%lu\t%s\t%s\t%s_%s\t%s\n",
            (unsigned long)GetTickCount(), (unsigned long)GetCurrentThreadId(),
            RuntimeAssetProbeRole(), category ? category : "UNKNOWN",
            operation ? operation : "LOAD", ok ? "OK" : "FAIL", path);
        fflush(state.attempts);
    }
}


inline void RuntimeAssetProbeAttemptW(const char *category, const char *operation, const wchar_t *path)
{
    if(!path || !*path) return;
    char utf8[4096] = {0};
    const int written = WideCharToMultiByte(CP_UTF8, 0, path, -1, utf8, static_cast<int>(sizeof(utf8)), NULL, NULL);
    if(written > 0)
        RuntimeAssetProbeAttempt(category, operation, utf8);
}

}} // namespace summit::diagnostics
