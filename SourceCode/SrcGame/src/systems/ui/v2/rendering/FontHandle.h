#pragma once

#include <cstdint>

namespace ui::v2
{
class FontHandle
{
public:
    constexpr FontHandle() = default;
    explicit constexpr FontHandle(std::uint32_t value) : value_(value) {}

    [[nodiscard]] constexpr std::uint32_t Value() const noexcept { return value_; }

private:
    std::uint32_t value_ = 0;
};
}
