#include "systems/ui/v2/widgets/NumericInputOverlay.h"

#include <algorithm>
#include <cstdlib>
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

NumericInputOverlay::NumericInputOverlay(UIId id) : Container(id)
{
    auto& primary = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 1, "Gold");
    primary.SetOnClick([this](const UIEvent&) { alternateSelected_ = false; RefreshOptionButtons(); });
    primaryOptionButton_ = &primary;

    auto& alternate = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 2, "Coins");
    alternate.SetOnClick([this](const UIEvent&) { alternateSelected_ = true; RefreshOptionButtons(); });
    alternateOptionButton_ = &alternate;

    auto& confirm = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 3, "OK");
    confirm.SetOnClick([this](const UIEvent&) { Confirm(); });
    confirmButton_ = &confirm;

    auto& cancel = AddChild<Button>(id == InvalidUIId ? InvalidUIId : id + 4, "Cancelar");
    cancel.SetOnClick([this](const UIEvent&) { Cancel(); });
    cancelButton_ = &cancel;
    SetVisible(false);
}

void NumericInputOverlay::Show(std::string title, std::string message, int minimum, int maximum, int initial,
                               ConfirmHandler onConfirm, CancelHandler onCancel)
{
    ShowWithOption(std::move(title), std::move(message), minimum, maximum, initial, {}, {},
                   [handler = std::move(onConfirm)](int value, bool) { if (handler) handler(value); },
                   std::move(onCancel));
}

void NumericInputOverlay::ShowWithOption(std::string title, std::string message, int minimum, int maximum, int initial,
                                         std::string primaryOption, std::string alternateOption,
                                         OptionConfirmHandler onConfirm, CancelHandler onCancel)
{
    title_ = std::move(title);
    message_ = std::move(message);
    minimum_ = std::min(minimum, maximum);
    maximum_ = std::max(minimum, maximum);
    valueText_ = std::to_string(std::clamp(initial, minimum_, maximum_));
    primaryOption_ = std::move(primaryOption);
    alternateOption_ = std::move(alternateOption);
    alternateSelected_ = false;
    onConfirm_ = std::move(onConfirm);
    onCancel_ = std::move(onCancel);
    SetVisible(true);
    RefreshOptionButtons();
    LayoutButtons();
}

void NumericInputOverlay::Hide()
{
    SetVisible(false);
    onConfirm_ = {};
    onCancel_ = {};
}

int NumericInputOverlay::Value() const noexcept
{
    if (valueText_.empty()) return minimum_;
    const long long parsed = std::strtoll(valueText_.c_str(), nullptr, 10);
    return static_cast<int>(std::clamp<long long>(parsed, minimum_, maximum_));
}

void NumericInputOverlay::Normalize() { valueText_ = std::to_string(Value()); }

void NumericInputOverlay::Confirm()
{
    Normalize();
    const int value = Value();
    const bool alternate = alternateSelected_;
    OptionConfirmHandler handler = std::move(onConfirm_);
    Hide();
    if (handler) handler(value, alternate);
}

void NumericInputOverlay::Cancel()
{
    CancelHandler handler = std::move(onCancel_);
    Hide();
    if (handler) handler();
}

void NumericInputOverlay::RefreshOptionButtons()
{
    const bool options = !primaryOption_.empty() && !alternateOption_.empty();
    primaryOptionButton_->SetVisible(options);
    alternateOptionButton_->SetVisible(options);
    if (options)
    {
        primaryOptionButton_->SetText(primaryOption_);
        alternateOptionButton_->SetText(alternateOption_);
        primaryOptionButton_->SetSelected(!alternateSelected_);
        alternateOptionButton_->SetSelected(alternateSelected_);
    }
}

bool NumericInputOverlay::HandleKey(std::uint32_t key)
{
    if (!IsOpen()) return false;
    if (key == 13) { Confirm(); return true; }
    if (key == 27) { Cancel(); return true; }
    if (key == 8)
    {
        if (!valueText_.empty()) valueText_.pop_back();
        return true;
    }
    unsigned digit = 10;
    if (key >= '0' && key <= '9') digit = key - '0';
    else if (key >= 0x60 && key <= 0x69) digit = key - 0x60;
    if (digit <= 9)
    {
        if (valueText_ == "0") valueText_.clear();
        if (valueText_.size() < 10) valueText_.push_back(static_cast<char>('0' + digit));
        return true;
    }
    return true;
}

void NumericInputOverlay::LayoutButtons()
{
    const Rect b = Bounds();
    const bool options = primaryOptionButton_->IsVisible();
    const int boxHeight = options ? 204 : 174;
    const int boxWidth = std::min(360, std::max(220, b.width - 36));
    const int left = (b.width - boxWidth) / 2;
    const int top = std::max(18, (b.height - boxHeight) / 2);
    const int buttonWidth = (boxWidth - 30) / 2;
    if (options)
    {
        primaryOptionButton_->SetBounds({left + 10, top + 120, buttonWidth, 26});
        alternateOptionButton_->SetBounds({left + 20 + buttonWidth, top + 120, buttonWidth, 26});
    }
    const int actionY = top + (options ? 158 : 128);
    confirmButton_->SetBounds({left + 10, actionY, buttonWidth, 30});
    cancelButton_->SetBounds({left + 20 + buttonWidth, actionY, buttonWidth, 30});
}

UIEventResult NumericInputOverlay::OnEvent(const UIEvent& event)
{
    if (!IsOpen()) return UIEventResult::Ignored;
    LayoutButtons();
    const UIEventResult child = Container::OnEvent(event);
    if (child != UIEventResult::Ignored) return child;
    if (event.IsPointerEvent()) return UIEventResult::Handled;
    if (event.type == UIEventType::KeyDown) { HandleKey(event.key); return UIEventResult::Handled; }
    return UIEventResult::Handled;
}

void NumericInputOverlay::OnRender(UIRenderContext& context)
{
    if (!IsOpen()) return;
    RefreshOptionButtons();
    LayoutButtons();
    auto& r = context.Renderer();
    const Rect root = GlobalBounds();
    r.DrawRect(root, Color{7, 9, 13, 190});

    const bool options = primaryOptionButton_->IsVisible();
    const int boxHeight = options ? 204 : 174;
    const int boxWidth = std::min(360, std::max(220, root.width - 36));
    const Rect box{root.x + (root.width - boxWidth) / 2,
                   root.y + std::max(18, (root.height - boxHeight) / 2), boxWidth, boxHeight};
    r.DrawRect(box, UITheme::SurfaceRaised);
    Border(r, box, UITheme::AccentMuted);
    r.DrawTextUtf8(FontHandle{}, title_, {box.x + 14, box.y + 12, box.width - 28, 24},
                   TextStyle{UITheme::Text, true, TextAlign::Left});
    const auto lines = WrapTextApprox(message_, box.width - 28, 2, 7);
    for (std::size_t i = 0; i < lines.size(); ++i)
        r.DrawTextUtf8(FontHandle{}, lines[i], {box.x + 14, box.y + 42 + static_cast<int>(i) * 18, box.width - 28, 18},
                       TextStyle{UITheme::TextMuted, false, TextAlign::Left});

    const Rect edit{box.x + 14, box.y + 82, box.width - 28, 34};
    r.DrawRect(edit, UITheme::Slot);
    Border(r, edit, UITheme::AccentMuted);
    r.DrawTextUtf8(FontHandle{}, valueText_.empty() ? "0" : valueText_,
                   {edit.x + 8, edit.y + 6, edit.width - 16, 22},
                   TextStyle{UITheme::Text, true, TextAlign::Right});
    r.DrawTextUtf8(FontHandle{}, std::to_string(minimum_) + " - " + std::to_string(maximum_),
                   {edit.x, edit.y + edit.height + 2, edit.width, 14},
                   TextStyle{UITheme::TextMuted, false, TextAlign::Right});
}
}
