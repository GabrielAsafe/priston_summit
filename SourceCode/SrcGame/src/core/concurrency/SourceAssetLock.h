#pragma once

#include <mutex>

namespace summit {
namespace core {
namespace concurrency {

// Case-insensitive per-path lock used around legacy source/cache conversion
// (.ini/.inx, .ase/.smd/.smb). Different assets remain parallel without hash
// stripe collisions; recursive acquisition is allowed for wrapper + loader.
std::recursive_mutex &SourceAssetMutex(const char *path);

class SourceAssetLock
{
public:
    explicit SourceAssetLock(const char *path)
        : lock_(SourceAssetMutex(path))
    {
    }

private:
    std::unique_lock<std::recursive_mutex> lock_;
};

} // namespace concurrency
} // namespace core
} // namespace summit
