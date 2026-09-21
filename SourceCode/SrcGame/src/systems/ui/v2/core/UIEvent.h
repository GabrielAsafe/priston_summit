#pragma once

#include <cstdint>
#include <string>
#include "systems/ui/v2/core/Geometry.h"

namespace ui::v2
{
enum class UIEventType
{
    PointerMove,
    PointerDown,
    PointerUp,
    Wheel,
    KeyDown,
    KeyUp,
    TextInput
};

enum class MouseButton
{
    None,
    Left,
    Right,
    Middle
};

enum class UIEventResult
{
    Ignored,
    Handled,
    Captured
};

struct KeyModifiers
{
    bool shift = false;
    bool control = false;
    bool alt = false;
};

struct UIEvent
{
    UIEventType type = UIEventType::PointerMove;
    Point pointer{};
    MouseButton button = MouseButton::None;
    int wheelDelta = 0;
    std::uint32_t key = 0;
    KeyModifiers modifiers{};
    std::string textUtf8{};

    [[nodiscard]] bool IsPointerEvent() const noexcept
    {
        return type == UIEventType::PointerMove || type == UIEventType::PointerDown ||
               type == UIEventType::PointerUp || type == UIEventType::Wheel;
    }
};
}
