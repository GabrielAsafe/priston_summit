#include "systems/ui/v2/widgets/ConfirmationOverlay.h"

#include <algorithm>
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include "systems/ui/v2/theme/UITheme.h"
#include "systems/ui/v2/widgets/Button.h"
#include "systems/ui/v2/widgets/TextLayout.h"

namespace ui::v2
{
namespace
{
void Border(IUIRenderer& r, const Rect& rect, Color color)
{
    r.DrawRect({rect.x, rect.y, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y + rect.height - 1, rect.width, 1}, color);
    r.DrawRect({rect.x, rect.y, 1, rect.height}, color);
    r.DrawRect({rect.x + rect.width - 1, rect.y, 1, rect.height}, color);
}
}

ConfirmationOverlay::ConfirmationOverlay(UIId id) : Container(id)
{
    auto& confirm = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 1, "Confirmar");
    confirm.SetOnClick([this](const UIEvent&) { Confirm(); });
    confirmButton_ = &confirm;

    auto& cancel = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 2, "Cancelar");
    cancel.SetOnClick([this](const UIEvent&) { Cancel(); });
    cancelButton_ = &cancel;

    SetVisible(false);
}

void ConfirmationOverlay::Show(std::string title, std::string message, Handler onConfirm, Handler onCancel)
{
    title_ = std::move(title);
    message_ = std::move(message);
    onConfirm_ = std::move(onConfirm);
    onCancel_ = std::move(onCancel);
    SetVisible(true);
    LayoutButtons();
}

void ConfirmationOverlay::Hide()
{
    SetVisible(false);
    onConfirm_ = {};
    onCancel_ = {};
}

void ConfirmationOverlay::Confirm()
{
    Handler handler = std::move(onConfirm_);
    Hide();
    if (handler) handler();
}

void ConfirmationOverlay::Cancel()
{
    Handler handler = std::move(onCancel_);
    Hide();
    if (handler) handler();
}

bool ConfirmationOverlay::HandleKey(std::uint32_t key)
{
    if (!IsOpen()) return false;
    if (key == 13) { Confirm(); return true; }
    if (key == 27) { Cancel(); return true; }
    return true;
}

void ConfirmationOverlay::LayoutButtons()
{
    const Rect b = Bounds();
    const int boxWidth = std::min(360, std::max(220, b.width - 36));
    const int left = (b.width - boxWidth) / 2;
    const int top = std::max(18, (b.height - 150) / 2);
    const int buttonWidth = (boxWidth - 30) / 2;
    confirmButton_->SetBounds({left + 10, top + 104, buttonWidth, 30});
    cancelButton_->SetBounds({left + 20 + buttonWidth, top + 104, buttonWidth, 30});
}

UIEventResult ConfirmationOverlay::OnEvent(const UIEvent& event)
{
    if (!IsOpen()) return UIEventResult::Ignored;
    LayoutButtons();
    const UIEventResult child = Container::OnEvent(event);
    if (child != UIEventResult::Ignored) return child;
    if (event.IsPointerEvent()) return UIEventResult::Handled;
    if (event.type == UIEventType::KeyDown) { HandleKey(event.key); return UIEventResult::Handled; }
    return UIEventResult::Handled;
}

void ConfirmationOverlay::OnRender(UIRenderContext& context)
{
    if (!IsOpen()) return;
    LayoutButtons();
    auto& r = context.Renderer();
    const Rect root = GlobalBounds();
    if (dimBackground_)
        r.DrawRect(root, Color{7, 9, 13, 190});

    const int boxWidth = std::min(360, std::max(220, root.width - 36));
    const Rect box{root.x + (root.width - boxWidth) / 2,
                   root.y + std::max(18, (root.height - 150) / 2), boxWidth, 150};
    r.DrawRect(box, UITheme::SurfaceRaised);
    Border(r, box, UITheme::AccentMuted);
    r.DrawTextUtf8(FontHandle{}, title_, {box.x + 14, box.y + 12, box.width - 28, 24},
                   TextStyle{UITheme::Text, true, TextAlign::Left});
    const auto lines = WrapTextApprox(message_, box.width - 28, 3, 7);
    for (std::size_t i = 0; i < lines.size(); ++i)
        r.DrawTextUtf8(FontHandle{}, lines[i], {box.x + 14, box.y + 43 + static_cast<int>(i) * 18, box.width - 28, 18},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});
}
}
