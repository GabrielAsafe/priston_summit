#include "systems/ui/v2/widgets/Separator.h"
#include "systems/ui/v2/rendering/IUIRenderer.h"

namespace ui::v2
{
void Separator::OnRender(UIRenderContext& context)
{
    context.Renderer().DrawRect(GlobalBounds(), color_);
}
}
