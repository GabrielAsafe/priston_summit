#include "systems/ui/v2/core/Widget.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"

#include <algorithm>

namespace ui::v2
{
Widget::Widget(UIId id) : id_(id) {}

void Widget::Update(float) {}

void Widget::Render(UIRenderContext& context)
{
    if (!visible_)
        return;
    OnRender(context);
}

UIEventResult Widget::OnEvent(const UIEvent&)
{
    return UIEventResult::Ignored;
}

Rect Widget::GlobalBounds() const noexcept
{
    Rect result = bounds_;
    const Widget* current = parent_;
    while (current)
    {
        const Rect parentBounds = current->Bounds();
        result.x += parentBounds.x;
        result.y += parentBounds.y;
        current = current->Parent();
    }
    return result;
}

void Widget::SetOpacity(float value) noexcept
{
    opacity_ = std::clamp(value, 0.0f, 1.0f);
}

bool Widget::HitTest(Point point) const noexcept
{
    return visible_ && enabled_ && hitTestVisible_ && GlobalBounds().Contains(point);
}

void Widget::OnRender(UIRenderContext&) {}
}
