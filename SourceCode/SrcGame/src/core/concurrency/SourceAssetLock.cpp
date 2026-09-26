#include "core/concurrency/SourceAssetLock.h"

#include <cctype>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace summit {
namespace core {
namespace concurrency {

namespace
{

std::string NormalizeSourceAssetPath(const char *path)
{
    std::string normalized;
    if (!path)
        return normalized;

    for (const unsigned char *p = reinterpret_cast<const unsigned char *>(path); *p; ++p)
    {
        unsigned char ch = *p;
        if (ch == '/') ch = '\\';
        if (ch >= 'A' && ch <= 'Z') ch = static_cast<unsigned char>(ch - 'A' + 'a');
        normalized.push_back(static_cast<char>(ch));
    }
    return normalized;
}

struct SourceAssetLockTable
{
    std::mutex tableMutex;
    std::unordered_map<std::string, std::unique_ptr<std::recursive_mutex> > locks;
};

SourceAssetLockTable &LockTable()
{
    // Deliberately process-lifetime storage: SourceAssetMutex returns references
    // and active loaders may still be unwinding during static destruction.
    static SourceAssetLockTable *table = new SourceAssetLockTable();
    return *table;
}

} // anonymous namespace

std::recursive_mutex &SourceAssetMutex(const char *path)
{
    SourceAssetLockTable &table = LockTable();
    const std::string normalized = NormalizeSourceAssetPath(path);

    std::lock_guard<std::mutex> guard(table.tableMutex);
    std::unique_ptr<std::recursive_mutex> &slot = table.locks[normalized];
    if (!slot)
        slot.reset(new std::recursive_mutex());
    return *slot;
}

} // namespace concurrency
} // namespace core
} // namespace summit
