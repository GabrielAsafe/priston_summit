#ifndef SUMMIT_EFFECT_FRAMES_248_H
#define SUMMIT_EFFECT_FRAMES_248_H

#include <cstdio>
#include <string>
#include <vector>

namespace summit { namespace effects {

// HoEffectResTexture's legacy filename interface accepts at most 127 bytes.
// Build each animation path in std::string rather than fixed strcpy/wsprintf
// buffers; return failure without publishing a partial sequence.
inline bool BuildEffectFrameNames248(const std::string &source, int count,
                                    std::vector<std::string> &out)
{
    out.clear();
    if (source.empty() || count <= 0 || count > 512)
        return false;

    const std::string::size_type slash = source.find_last_of("/\\");
    const std::string::size_type dot = source.find_last_of('.');
    if (dot == std::string::npos || (slash != std::string::npos && dot <= slash))
        return false;

    const std::string prefix = source.substr(0, dot);
    const std::string ext = source.substr(dot + 1);
    const bool bmp = ext.size() == 3 &&
        (ext[0] == 'b' || ext[0] == 'B') &&
        (ext[1] == 'm' || ext[1] == 'M') &&
        (ext[2] == 'p' || ext[2] == 'P');
    // Match the original convention: BMP frames use BMP, other effect frames TGA.
    const char *suffix = bmp ? ".bmp" : ".tga";
    std::vector<std::string> prepared;
    prepared.reserve(static_cast<size_t>(count));
    for (int i = 1; i <= count; ++i)
    {
        char digits[16];
        std::snprintf(digits, sizeof(digits), "%d", i);
        std::string path = prefix + digits + suffix;
        if (path.size() >= 128u)
            return false;
        prepared.push_back(path);
    }
    out.swap(prepared);
    return true;
}

} } // namespace summit::effects
#endif
