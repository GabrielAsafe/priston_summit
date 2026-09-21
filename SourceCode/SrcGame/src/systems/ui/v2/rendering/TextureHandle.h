#pragma once

#include <cstdint>

namespace ui::v2
{
class TextureHandle
{
public:
    constexpr TextureHandle() = default;
    explicit constexpr TextureHandle(std::uintptr_t value) : value_(value) {}

    [[nodiscard]] constexpr bool IsValid() const noexcept { return value_ != 0; }
    [[nodiscard]] constexpr std::uintptr_t Value() const noexcept { return value_; }

private:
    std::uintptr_t value_ = 0;
};
}
