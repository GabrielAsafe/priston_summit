#pragma once

#include <cstdint>

namespace ui::v2
{
using UIId = std::uint64_t;
using WindowId = std::uint64_t;

constexpr UIId InvalidUIId = 0;
constexpr WindowId InvalidWindowId = 0;
}
