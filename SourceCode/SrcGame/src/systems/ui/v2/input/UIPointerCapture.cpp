#include "systems/ui/v2/input/UIPointerCapture.h"
#include "systems/ui/v2/core/Widget.h"

namespace ui::v2
{
bool UIPointerCapture::Capture(Widget* widget) noexcept
{
    if (!widget || !widget->IsVisible() || !widget->IsEnabled())
        return false;
    captured_ = widget;
    return true;
}
}
