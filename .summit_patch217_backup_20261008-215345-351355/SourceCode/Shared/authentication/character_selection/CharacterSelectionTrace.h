#pragma once
// SUMMIT_CHARACTER_SELECTION_TRACE_V14
// Temporary diagnostic instrumentation for the character-selection/model-streaming path.
// It intentionally logs only state transitions and decision points, not every frame.

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <windows.h>

inline void SummitCharacterSelectionTrace(const char *component, const char *format, ...)
{
    CreateDirectoryA("LOGS", NULL);
    FILE *fp = std::fopen("LOGS\\character_selection_debug.log", "a");
    if(!fp)
        return;

    SYSTEMTIME st;
    GetLocalTime(&st);
    std::fprintf(fp,
        "%04u-%02u-%02u %02u:%02u:%02u.%03u pid=%lu tid=%lu [%s] ",
        static_cast<unsigned>(st.wYear),
        static_cast<unsigned>(st.wMonth),
        static_cast<unsigned>(st.wDay),
        static_cast<unsigned>(st.wHour),
        static_cast<unsigned>(st.wMinute),
        static_cast<unsigned>(st.wSecond),
        static_cast<unsigned>(st.wMilliseconds),
        static_cast<unsigned long>(GetCurrentProcessId()),
        static_cast<unsigned long>(GetCurrentThreadId()),
        component ? component : "?");

    va_list args;
    va_start(args, format);
    std::vfprintf(fp, format ? format : "", args);
    va_end(args);
    std::fputc('\n', fp);
    std::fclose(fp);
}
