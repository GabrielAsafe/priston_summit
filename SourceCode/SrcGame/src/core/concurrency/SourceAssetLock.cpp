#include "core/concurrency/SourceAssetLock.h"

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>

namespace summit {
namespace core {
namespace concurrency {

std::mutex &SourceAssetMutex(const char *path)
{
    static std::array<std::mutex, 127> *locks = new std::array<std::mutex, 127>();

    // FNV-1a over a normalized path. Slash direction and ASCII case should not
    // allow the same Windows file to be converted concurrently.
    std::uint32_t hash = 2166136261u;
    if (path)
    {
        for (const unsigned char *p = reinterpret_cast<const unsigned char *>(path); *p; ++p)
        {
            unsigned char ch = *p;
            if (ch == '/') ch = '\\';
            if (ch >= 'A' && ch <= 'Z') ch = static_cast<unsigned char>(ch - 'A' + 'a');
            hash ^= ch;
            hash *= 16777619u;
        }
    }
    return (*locks)[hash % locks->size()];
}

} // namespace concurrency
} // namespace core
} // namespace summit
