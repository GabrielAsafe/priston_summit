#ifndef SUMMIT_WAV_PATH_250_H
#define SUMMIT_WAV_PATH_250_H
#include <cstddef>
#include <cstring>
namespace summit { namespace audio {
// Explicit source capacities make a missing terminator a failed request,
// rather than letting strlen walk off a fixed-size legacy config array.
inline bool JoinWavPath250(const char *directory, std::size_t directoryCapacity,
                           const char *leaf, std::size_t leafCapacity,
                           char *output, std::size_t outputCapacity)
{
    if (!directory || !leaf || !output || !directoryCapacity ||
        !leafCapacity || !outputCapacity) return false;
    const char *dirEnd = static_cast<const char *>(std::memchr(directory, 0, directoryCapacity));
    const char *leafEnd = static_cast<const char *>(std::memchr(leaf, 0, leafCapacity));
    if (!dirEnd || !leafEnd) return false;
    const std::size_t a = static_cast<std::size_t>(dirEnd - directory);
    const std::size_t b = static_cast<std::size_t>(leafEnd - leaf);
    if (!b || a >= outputCapacity || b >= outputCapacity - a) return false;
    std::memcpy(output, directory, a);
    std::memcpy(output + a, leaf, b);
    output[a+b] = 0;
    return true;
}
} }
#endif
