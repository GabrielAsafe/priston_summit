#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace ui::v2
{
inline std::size_t ApproxCharsForWidth(int width, int averageGlyphWidth = 7)
{
    return static_cast<std::size_t>(std::max(1, width / std::max(1, averageGlyphWidth)));
}

inline std::string EllipsizeText(const std::string& text, int width, int averageGlyphWidth = 7)
{
    const std::size_t maxChars = ApproxCharsForWidth(width, averageGlyphWidth);
    if (text.size() <= maxChars)
        return text;
    if (maxChars <= 3)
        return text.substr(0, maxChars);
    return text.substr(0, maxChars - 3) + "...";
}

inline std::vector<std::string> WrapTextApprox(const std::string& text, int width, int maxLines,
                                                int averageGlyphWidth = 7)
{
    std::vector<std::string> lines;
    if (text.empty() || maxLines <= 0)
        return lines;

    const std::size_t limit = ApproxCharsForWidth(width, averageGlyphWidth);
    std::size_t offset = 0;
    while (offset < text.size() && static_cast<int>(lines.size()) < maxLines)
    {
        while (offset < text.size() && text[offset] == ' ')
            ++offset;
        if (offset >= text.size())
            break;

        std::size_t length = std::min(limit, text.size() - offset);
        if (offset + length < text.size())
        {
            const std::size_t split = text.rfind(' ', offset + length);
            if (split != std::string::npos && split > offset)
                length = split - offset;
        }
        lines.push_back(text.substr(offset, length));
        offset += length;
    }

    if (offset < text.size() && !lines.empty())
        lines.back() = EllipsizeText(lines.back() + "...", width, averageGlyphWidth);
    return lines;
}
}
