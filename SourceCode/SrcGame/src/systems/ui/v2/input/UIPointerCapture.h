#pragma once

namespace ui::v2
{
class Widget;

class UIPointerCapture
{
public:
    bool Capture(Widget* widget) noexcept;
    void Release() noexcept { captured_ = nullptr; }
    [[nodiscard]] Widget* Captured() const noexcept { return captured_; }

private:
    Widget* captured_ = nullptr;
};
}
