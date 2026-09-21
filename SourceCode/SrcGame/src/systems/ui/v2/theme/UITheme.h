#pragma once

#include "systems/ui/v2/core/Geometry.h"

namespace ui::v2
{
struct UITheme final
{
    static constexpr Color WindowBg{18, 21, 28, 245};
    static constexpr Color Surface{25, 30, 39, 245};
    static constexpr Color SurfaceRaised{32, 38, 49, 250};
    static constexpr Color SurfaceHover{42, 49, 62, 250};
    static constexpr Color SurfacePressed{51, 59, 73, 255};
    static constexpr Color Border{67, 76, 94, 255};
    static constexpr Color BorderSoft{48, 56, 70, 255};
    static constexpr Color Accent{214, 165, 75, 255};
    static constexpr Color AccentMuted{132, 103, 55, 255};
    static constexpr Color Text{236, 239, 244, 255};
    static constexpr Color TextMuted{164, 175, 192, 255};
    static constexpr Color Positive{111, 196, 145, 255};
    static constexpr Color Warning{224, 177, 88, 255};
    static constexpr Color Danger{210, 103, 103, 255};
    static constexpr Color Slot{20, 24, 31, 255};
    static constexpr Color SlotHover{43, 50, 62, 255};
    static constexpr Color Disabled{90, 96, 108, 255};
};
}
