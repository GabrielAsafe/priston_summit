#include "systems/ui/v2/core/Window.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"

namespace ui::v2
{
Window::Window(UIId id, std::string title) : Container(id), title_(std::move(title))
{
    SetFocusable(true);
}

UIEventResult Window::OnEvent(const UIEvent& event)
{
    const Rect bounds = GlobalBounds();

    // A window drag owns pointer motion until release. Handle it before child
    // dispatch so moving across the page body cannot steal the drag from the
    // title bar that captured it.
    if (dragging_)
    {
        if (event.type == UIEventType::PointerMove)
        {
            Rect local = Bounds();
            local.x = event.pointer.x - dragOffset_.x;
            local.y = event.pointer.y - dragOffset_.y;
            SetBounds(local);
            return UIEventResult::Captured;
        }

        if (event.type == UIEventType::PointerUp && event.button == MouseButton::Left)
        {
            dragging_ = false;
            return UIEventResult::Handled;
        }
    }

    const UIEventResult childResult = Container::OnEvent(event);
    if (childResult != UIEventResult::Ignored)
        return childResult;

    const Rect titleBar{bounds.x, bounds.y, bounds.width, titleBarHeight_};

    if (event.type == UIEventType::PointerDown && event.button == MouseButton::Left &&
        movable_ && titleBar.Contains(event.pointer))
    {
        dragging_ = true;
        dragOffset_ = {event.pointer.x - bounds.x, event.pointer.y - bounds.y};
        return UIEventResult::Captured;
    }

    if (event.IsPointerEvent() && bounds.Contains(event.pointer))
        return UIEventResult::Handled;

    return UIEventResult::Ignored;
}

void Window::OnRender(UIRenderContext& context)
{
    const Rect bounds = GlobalBounds();
    auto& renderer = context.Renderer();
    renderer.DrawRect(bounds, UITheme::WindowBg);
    renderer.DrawRect({bounds.x, bounds.y, bounds.width, titleBarHeight_}, UITheme::SurfaceRaised);
    renderer.DrawRect({bounds.x, bounds.y + titleBarHeight_ - 1, bounds.width, 1}, UITheme::AccentMuted);
    renderer.DrawRect({bounds.x, bounds.y, bounds.width, 1}, UITheme::Border);
    renderer.DrawRect({bounds.x, bounds.y + bounds.height - 1, bounds.width, 1}, UITheme::Border);
    renderer.DrawRect({bounds.x, bounds.y, 1, bounds.height}, UITheme::Border);
    renderer.DrawRect({bounds.x + bounds.width - 1, bounds.y, 1, bounds.height}, UITheme::Border);
    renderer.DrawTextUtf8(FontHandle{}, title_, {bounds.x + 12, bounds.y + 5, bounds.width - 24, titleBarHeight_ - 8},
                      TextStyle{UITheme::Text, true, TextAlign::Left});
}
}
