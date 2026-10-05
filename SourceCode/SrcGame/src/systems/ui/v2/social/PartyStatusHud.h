#pragma once

#include "systems/ui/v2/core/Window.h"

namespace ui::v2
{
class PartyStatusHud final : public Window
{
public:
    explicit PartyStatusHud(UIId id = InvalidUIId);
    void SetViewport(int width, int height);

protected:
    void OnRender(UIRenderContext& context) override;
};
}
