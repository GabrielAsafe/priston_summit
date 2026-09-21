#pragma once

#include "systems/ui/v2/core/Widget.h"

namespace ui::v2
{
class Separator : public Widget
{
public:
    explicit Separator(UIId id = InvalidUIId, Color color = Color{55, 63, 78, 255}) : Widget(id), color_(color)
    {
        SetHitTestVisible(false);
    }

protected:
    void OnRender(UIRenderContext& context) override;

private:
    Color color_;
};
}
