#ifndef SUMMIT_SOUND_TABLE_258_H
#define SUMMIT_SOUND_TABLE_258_H
#include <cstddef>
namespace summit { namespace audio {
// No string scans and no out-of-bounds reads: N includes the NULL terminator.
// Legacy paths are mutable `char*` for the DirectSound loader interface.
template<std::size_t N>
inline char *SoundTablePath258(char * const (&files)[N], int index)
{
    if (N < 2u || index < 0 || static_cast<std::size_t>(index) >= N - 1u ||
        files[N - 1u] != NULL)
        return NULL;
    return files[index];
}
} }
#endif
