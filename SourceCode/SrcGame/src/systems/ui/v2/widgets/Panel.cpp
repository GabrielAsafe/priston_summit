#include "systems/ui/v2/widgets/Panel.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"

namespace ui::v2
{
void Panel::OnRender(UIRenderContext& context)
{
    context.Renderer().DrawRect(GlobalBounds(), color_);
}
}
