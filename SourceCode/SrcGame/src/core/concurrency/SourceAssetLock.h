#pragma once

#include <mutex>

namespace summit {
namespace core {
namespace concurrency {

// Striped case-insensitive path lock used only around legacy source/cache
// conversion (.ini/.inx, .ase/.smd/.smb). Different assets remain parallel.
std::mutex &SourceAssetMutex(const char *path);

class SourceAssetLock
{
public:
    explicit SourceAssetLock(const char *path)
        : lock_(SourceAssetMutex(path))
    {
    }

private:
    std::unique_lock<std::mutex> lock_;
};

} // namespace concurrency
} // namespace core
} // namespace summit
