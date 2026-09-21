#include "systems/ui/v2/widgets/Label.h"

namespace ui::v2
{
Label::Label(UIId id, std::string text) : Widget(id), text_(std::move(text))
{
    SetHitTestVisible(false);
}

void Label::OnRender(UIRenderContext& context)
{
    context.Renderer().DrawTextUtf8(FontHandle{}, text_, GlobalBounds(), style_);
}
}
