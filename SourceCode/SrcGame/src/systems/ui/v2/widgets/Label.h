#pragma once

#include <string>
#include "systems/ui/v2/core/Widget.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"

namespace ui::v2
{
class Label : public Widget
{
public:
    explicit Label(UIId id = InvalidUIId, std::string text = {});

    void SetText(std::string text) { text_ = std::move(text); }
    [[nodiscard]] const std::string& Text() const noexcept { return text_; }

    void SetStyle(TextStyle style) noexcept { style_ = style; }
    [[nodiscard]] const TextStyle& Style() const noexcept { return style_; }

protected:
    void OnRender(UIRenderContext& context) override;

private:
    std::string text_;
    TextStyle style_{};
};
}
