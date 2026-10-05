#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "systems/ui/v2/core/Window.h"

namespace ui::v2
{
class Button;

class GuideWindow final : public Window
{
public:
    using CloseHandler = std::function<void()>;

    explicit GuideWindow(UIId id = InvalidUIId);
    void SetViewport(int width, int height);
    void SetOnClose(CloseHandler handler) { onClose_ = std::move(handler); }
    void Reload();
    UIEventResult OnEvent(const UIEvent& event) override;

protected:
    void OnRender(UIRenderContext& context) override;

private:
    struct Line
    {
        std::string text;
        bool heading = false;
    };

    void Layout();
    int MaxScroll() const noexcept;

    std::vector<Line> lines_;
    Button* closeButton_ = nullptr;
    int scroll_ = 0;
    CloseHandler onClose_;
};
}
