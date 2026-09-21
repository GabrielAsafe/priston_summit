#pragma once

#include <functional>
#include <string>
#include "systems/ui/v2/core/Widget.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"

namespace ui::v2
{
class Button : public Widget
{
public:
    using ClickHandler = std::function<void(const UIEvent&)>;

    explicit Button(UIId id = InvalidUIId, std::string text = {});

    void SetText(std::string text) { text_ = std::move(text); }
    [[nodiscard]] const std::string& Text() const noexcept { return text_; }

    void SetOnClick(ClickHandler handler) { onClick_ = std::move(handler); }
    void SetSelected(bool value) noexcept { selected_ = value; }
    [[nodiscard]] bool IsSelected() const noexcept { return selected_; }

    UIEventResult OnEvent(const UIEvent& event) override;

protected:
    void OnRender(UIRenderContext& context) override;

private:
    std::string text_;
    ClickHandler onClick_;
    bool hovered_ = false;
    bool pressed_ = false;
    bool selected_ = false;
};
}
