#include "systems/ui/v2/widgets/ProgressBar.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"
#include <algorithm>

namespace ui::v2
{
void ProgressBar::SetValue(float value) noexcept
{
    value_ = std::clamp(value, 0.0f, 1.0f);
}

void ProgressBar::OnRender(UIRenderContext& context)
{
    const Rect bounds = GlobalBounds();
    auto& renderer = context.Renderer();
    renderer.DrawRect(bounds, background_);
    const int fillWidth = static_cast<int>(static_cast<float>(bounds.width) * value_);
    if (fillWidth > 0)
        renderer.DrawRect({bounds.x, bounds.y, fillWidth, bounds.height}, fill_);
}
}
