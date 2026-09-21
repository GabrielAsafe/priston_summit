#include "systems/ui/v2/input/UIFocusManager.h"
#include "systems/ui/v2/core/Widget.h"

namespace ui::v2
{
void UIFocusManager::SetFocus(Widget* widget) noexcept
{
    focused_ = (widget && widget->IsFocusable()) ? widget : nullptr;
}
}
