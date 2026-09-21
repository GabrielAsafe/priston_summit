#pragma once

#include "systems/ui/v2/core/Widget.h"

namespace ui::v2
{
class ProgressBar : public Widget
{
public:
    using Widget::Widget;

    void SetValue(float value) noexcept;
    [[nodiscard]] float Value() const noexcept { return value_; }
    void SetFill(Color color) noexcept { fill_ = color; }
    void SetBackground(Color color) noexcept { background_ = color; }

protected:
    void OnRender(UIRenderContext& context) override;

private:
    float value_ = 0.0f;
    Color fill_{100, 170, 120, 255};
    Color background_{20, 24, 31, 255};
};
}
