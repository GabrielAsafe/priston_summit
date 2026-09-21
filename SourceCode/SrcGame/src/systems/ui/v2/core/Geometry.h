#pragma once

#include <algorithm>
#include <cstdint>

namespace ui::v2
{
struct Point
{
    int x = 0;
    int y = 0;
};

struct Size
{
    int width = 0;
    int height = 0;
};

struct Rect
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    [[nodiscard]] bool Contains(Point point) const noexcept
    {
        return point.x >= x && point.y >= y && point.x < (x + width) && point.y < (y + height);
    }

    [[nodiscard]] bool IsEmpty() const noexcept
    {
        return width <= 0 || height <= 0;
    }

    [[nodiscard]] Rect Intersect(const Rect& other) const noexcept
    {
        const int left = std::max(x, other.x);
        const int top = std::max(y, other.y);
        const int right = std::min(x + width, other.x + other.width);
        const int bottom = std::min(y + height, other.y + other.height);
        return {left, top, std::max(0, right - left), std::max(0, bottom - top)};
    }
};

struct Color
{
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;

    [[nodiscard]] constexpr std::uint32_t ToARGB() const noexcept
    {
        return (static_cast<std::uint32_t>(a) << 24U) |
               (static_cast<std::uint32_t>(r) << 16U) |
               (static_cast<std::uint32_t>(g) << 8U) |
               static_cast<std::uint32_t>(b);
    }
};
}
