#pragma once

// Per-installation durable request identifiers for both S-Coin and T-Coin.
// A request id identifies a single purchase attempt, NOT a product. The
// server's durable replay receipts therefore must never see it reused after a
// client restart. Persist the next id BEFORE sending a packet; fail closed
// when the counter cannot be stored. No account names or premium balances are
// written to this file.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cerrno>
#include <cstring>
#include <mutex>
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <random>
#endif

namespace gameplay
{
namespace commerce
{
class PremiumPurchaseRequestSequence final
{
public:
    PremiumPurchaseRequestSequence() {}

    std::uint32_t Next()
    {
        // Both shop implementations (and multiple instances of this class)
        // re-read the same on-disk state. The lock is shared across TUs.
        std::lock_guard<std::mutex> guard(ProcessLock());
#ifdef _WIN32
        CrossProcessLock cross;
        if(!cross.ok())
            return 0u;
#endif
        const std::string path = StatePath();
        if(path.empty())
            return 0u;
        std::uint32_t previous = 0u;
        bool missing = false;
        if(!ReadState(path, previous, missing))
            return 0u; // Never reset a malformed/unreadable counter to one.
        if(missing)
            previous = InitialSeed();
        if(previous == 0u || previous >= 0x7FFFFFFFu)
            return 0u;
        const std::uint32_t next = previous + 1u;
        if(!WriteState(path, next))
            return 0u;
        return next;
    }

private:
    static std::mutex& ProcessLock()
    {
        static std::mutex mutex;
        return mutex;
    }
#ifdef _WIN32
    struct CrossProcessLock
    {
        HANDLE handle;
        bool acquired;
        CrossProcessLock() : handle(CreateMutexA(NULL, FALSE,
            "Local\\SummitPremiumPurchaseRequestSequenceV1")), acquired(false)
        {
            if(handle)
            {
                const DWORD result = WaitForSingleObject(handle, INFINITE);
                acquired = result == WAIT_OBJECT_0 || result == WAIT_ABANDONED;
            }
        }
        ~CrossProcessLock()
        {
            if(acquired) ReleaseMutex(handle);
            if(handle) CloseHandle(handle);
        }
        bool ok() const { return acquired; }
    };
#endif
    static std::string StatePath()
    {
#ifdef _WIN32
        // Use the Game.exe directory, not the current working directory:
        // launching the same game from a different shell must not reset IDs.
        char exe[MAX_PATH] = {0};
        const DWORD n = GetModuleFileNameA(NULL, exe, MAX_PATH);
        if(n == 0 || n >= MAX_PATH)
            return std::string();
        std::string path(exe, n);
        const std::string::size_type slash = path.find_last_of("\\/");
        if(slash == std::string::npos)
            return std::string();
        return path.substr(0, slash + 1) + "premium_purchase_sequence_v1.dat";
#else
        return "premium_purchase_sequence_v1.dat";
#endif
    }

    static std::uint32_t InitialSeed()
    {
        std::uint32_t randomValue = 0u;
        bool haveRandom = false;
#ifdef _WIN32
        // Dynamically resolve the system RNG: no extra link-library needed.
        HMODULE advapi = LoadLibraryA("advapi32.dll");
        if(advapi)
        {
            typedef BOOLEAN (WINAPI *RtlGenRandomFn)(PVOID, ULONG);
            const RtlGenRandomFn fn = reinterpret_cast<RtlGenRandomFn>(
                GetProcAddress(advapi, "SystemFunction036"));
            if(fn)
                haveRandom = fn(&randomValue, sizeof(randomValue)) != FALSE;
            FreeLibrary(advapi);
        }
#else
        try
        {
            std::random_device random;
            randomValue = static_cast<std::uint32_t>(random());
            haveRandom = true;
        }
        catch(...) {}
#endif
        if(!haveRandom)
        {
            // Fallback is deliberately *not* a small 1,2,3 sequence. The
            // durable file remains authoritative after the first purchase.
            randomValue = static_cast<std::uint32_t>(std::time(NULL));
#ifdef _WIN32
            randomValue ^= GetTickCount();
            randomValue ^= GetCurrentProcessId() << 12;
#endif
        }
        // Signed-positive on the existing wire; leave ample space for buys.
        return 0x40000000u + (randomValue % 0x30000000u);
    }

    static bool ReadState(const std::string& path, std::uint32_t& value, bool& missing)
    {
        missing = false;
        errno = 0;
        FILE* file = std::fopen(path.c_str(), "rb");
        if(!file)
        {
            missing = errno == ENOENT;
            return missing;
        }
        char line[128] = {0};
        const bool hasLine = std::fgets(line, sizeof(line), file) != NULL;
        const bool extraLine = std::fgetc(file) != EOF;
        const bool ioError = std::ferror(file) != 0;
        const bool closed = std::fclose(file) == 0;
        if(!hasLine || extraLine || ioError || !closed)
            return false;
        unsigned int number = 0u, check = 0u;
        char trailing = 0;
        if(std::sscanf(line, "SUMMIT-PREMIUM-SEQ-V1 %u %u %c", &number, &check, &trailing) != 2 ||
           number == 0u || number > 0x7FFFFFFFu ||
           check != (number ^ 0xA59673D1u))
            return false;
        value = static_cast<std::uint32_t>(number);
        return true;
    }

    static bool WriteState(const std::string& path, std::uint32_t number)
    {
        const std::string tmp = path + ".tmp";
        FILE* file = std::fopen(tmp.c_str(), "wb");
        if(!file)
            return false;
        const int len = std::fprintf(file, "SUMMIT-PREMIUM-SEQ-V1 %u %u\n",
            static_cast<unsigned int>(number),
            static_cast<unsigned int>(number ^ 0xA59673D1u));
        bool ok = len > 0 && std::fflush(file) == 0;
#ifdef _WIN32
        if(ok) ok = _commit(_fileno(file)) == 0;
#else
        if(ok) ok = fsync(fileno(file)) == 0;
#endif
        if(std::fclose(file) != 0)
            ok = false;
        if(ok)
        {
#ifdef _WIN32
            ok = MoveFileExA(tmp.c_str(), path.c_str(),
                MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
            ok = std::rename(tmp.c_str(), path.c_str()) == 0;
#endif
        }
        if(!ok)
            std::remove(tmp.c_str());
        return ok;
    }
};
}
}
