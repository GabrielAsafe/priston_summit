#pragma once

#include <string>
#include <cctype>

// Offline cooker and Windows/Wine client share this exact relative identity:
// cooked/<profile>/<original logical path including extension>.dds
// Deliberately *no* legacy foo.dds fallback: that path may contain a BMP/TGA
// last-writer collision. Profiles are selected by the invoking consumer.
namespace summit { namespace rendering { namespace cooked {

enum Profile { Material, Dib };

inline std::string BuildRelativePath(const std::string &logical, Profile profile)
{
    if (logical.empty() || logical[0] == '/' || logical[0] == '\\' ||
        logical.find(':') != std::string::npos)
        return std::string();

    std::string normalized;
    normalized.reserve(logical.size());
    std::string segment;
    for (std::size_t i = 0; i <= logical.size(); ++i)
    {
        const char ch = i == logical.size() ? '/' : logical[i];
        if (ch == '/' || ch == '\\')
        {
            if (segment == "..")
                return std::string();
            if (!segment.empty() && segment != ".")
            {
                if (!normalized.empty()) normalized += '\\';
                normalized += segment;
            }
            segment.clear();
        }
        else
            segment += ch;
    }
    if (normalized.empty()) return std::string();

    const std::size_t dot = normalized.find_last_of('.');
    if (dot == std::string::npos) return std::string();
    std::string extension = normalized.substr(dot);
    for (std::size_t i = 0; i < extension.size(); ++i)
        extension[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(extension[i])));
    if (extension != ".bmp" && extension != ".tga" && extension != ".png" &&
        extension != ".jpg" && extension != ".jpeg" && extension != ".stg" &&
        extension != ".sta")
        return std::string();
    return std::string(profile == Dib ? "dib\\" : "material\\") + normalized + ".dds";
}

}}} // namespace summit::rendering::cooked
