#pragma once

#include <algorithm>
#include <vector>
#include "systems/ui/v2/domain/CharacterUIModels.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2
{
// Expands the legacy two-column detail buffers into visual rows that fit the
// modern panel. Description-only rows are word-wrapped; key/value rows retain
// the right-aligned value column and only ellipsize the key when necessary.
inline std::vector<DetailLineSnapshot> LayoutDetailRows(const std::vector<DetailLineSnapshot>& source,
                                                        int width,
                                                        int rightColumnWidth = 92,
                                                        int averageGlyphWidth = 8)
{
    std::vector<DetailLineSnapshot> rows;
    const int safeWidth = std::max(24, width);

    for (const auto& line : source)
    {
        if (!line.right.empty())
        {
            // Preserve the complete legacy key/value line. Long labels wrap
            // vertically instead of being truncated with an ellipsis; the
            // value stays right-aligned on the first visual row. This pairs
            // with the detail panel scrollbar so information is never lost
            // just to fit a narrow character window.
            const int rightWidth = std::min(rightColumnWidth, std::max(54, safeWidth / 3));
            const int leftWidth = std::max(24, safeWidth - rightWidth - 6);
            const auto wrappedLeft = WrapTextApprox(line.left, leftWidth, 128, averageGlyphWidth);
            if (wrappedLeft.empty())
            {
                rows.push_back(line);
                continue;
            }

            for (std::size_t i = 0; i < wrappedLeft.size(); ++i)
            {
                DetailLineSnapshot row;
                row.left = wrappedLeft[i];
                row.right = (i == 0) ? line.right : std::string{};
                row.tone = line.tone;
                rows.push_back(std::move(row));
            }
            continue;
        }

        if (line.left.empty())
        {
            rows.push_back(line);
            continue;
        }

        const auto wrapped = WrapTextApprox(line.left, safeWidth, 128, averageGlyphWidth);
        if (wrapped.empty())
        {
            rows.push_back(line);
            continue;
        }

        for (const auto& text : wrapped)
        {
            DetailLineSnapshot row;
            row.left = text;
            row.tone = line.tone;
            rows.push_back(std::move(row));
        }
    }
    return rows;
}
}
