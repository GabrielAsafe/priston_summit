#include "systems/ui/v2/widgets/Button.h"
#include <algorithm>
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2
{
Button::Button(UIId id, std::string text) : Widget(id), text_(std::move(text))
{
    SetFocusable(true);
}

UIEventResult Button::OnEvent(const UIEvent& event)
{
    if (!IsVisible() || !IsEnabled())
        return UIEventResult::Ignored;

    if (event.type == UIEventType::PointerMove)
    {
        hovered_ = HitTest(event.pointer);
        return hovered_ ? UIEventResult::Handled : UIEventResult::Ignored;
    }

    if (event.type == UIEventType::PointerDown && event.button == MouseButton::Left && HitTest(event.pointer))
    {
        pressed_ = true;
        hovered_ = true;
        return UIEventResult::Handled;
    }

    if (event.type == UIEventType::PointerUp && event.button == MouseButton::Left)
    {
        const bool activate = pressed_ && HitTest(event.pointer);
        pressed_ = false;
        hovered_ = HitTest(event.pointer);
        if (activate && onClick_)
            onClick_(event);
        return activate ? UIEventResult::Handled : UIEventResult::Ignored;
    }

    return UIEventResult::Ignored;
}

void Button::OnRender(UIRenderContext& context)
{
    auto& renderer = context.Renderer();
    Color background = UITheme::SurfaceRaised;
    if (!IsEnabled())
        background = UITheme::Surface;
    else if (pressed_)
        background = UITheme::SurfacePressed;
    else if (hovered_ || selected_)
        background = UITheme::SurfaceHover;

    const Rect bounds = GlobalBounds();
    renderer.DrawRect(bounds, background);

    const Color border = selected_ ? UITheme::Accent : UITheme::BorderSoft;
    renderer.DrawRect({bounds.x, bounds.y, bounds.width, 1}, border);
    renderer.DrawRect({bounds.x, bounds.y + bounds.height - 1, bounds.width, 1}, border);
    renderer.DrawRect({bounds.x, bounds.y, 1, bounds.height}, border);
    renderer.DrawRect({bounds.x + bounds.width - 1, bounds.y, 1, bounds.height}, border);

    // The button controls its own image: the parent never draws behind a
    // later opaque button, and no BMP/D3D creation happens in this draw path.
    if (icon_.IsValid())
    {
        const int side = std::max(1, std::min(bounds.width - 8, bounds.height - 8));
        renderer.DrawImage(icon_, {bounds.x+(bounds.width-side)/2,
            bounds.y+(bounds.height-side)/2,side,side}, nullptr);
        return;
    }
    const std::string displayText = EllipsizeText(text_, std::max(1, bounds.width - 16), 7);
    renderer.DrawTextUtf8(FontHandle{}, displayText, {bounds.x + 8, bounds.y + 2, bounds.width - 16, bounds.height - 4},
                      TextStyle{IsEnabled() ? UITheme::Text : UITheme::Disabled, selected_, TextAlign::Center});
}
}
