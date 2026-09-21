#pragma once

#include "systems/ui/v2/core/Widget.h"
#include "systems/ui/v2/core/Geometry.h"

namespace ui::v2
{
class Panel : public Widget
{
public:
    explicit Panel(UIId id = InvalidUIId, Color color = Color{30, 34, 43, 220})
        : Widget(id), color_(color) {}

    void SetColor(Color color) noexcept { color_ = color; }

protected:
    void OnRender(UIRenderContext& context) override;

private:
    Color color_;
};
}
