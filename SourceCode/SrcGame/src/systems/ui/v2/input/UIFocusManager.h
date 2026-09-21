#pragma once

namespace ui::v2
{
class Widget;

class UIFocusManager
{
public:
    void SetFocus(Widget* widget) noexcept;
    void ClearFocus() noexcept { focused_ = nullptr; }
    [[nodiscard]] Widget* Focused() const noexcept { return focused_; }

private:
    Widget* focused_ = nullptr;
};
}
