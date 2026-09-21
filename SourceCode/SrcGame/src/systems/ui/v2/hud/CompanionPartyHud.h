#pragma once

#include "systems/ui/v2/core/Window.h"
#include "systems/ui/v2/domain/ICompanionGateway.h"

namespace ui::v2
{
class CompanionPartyHud final : public Window
{
public:
    explicit CompanionPartyHud(ICompanionGateway& gateway, UIId id = InvalidUIId);

    void SetViewport(int width, int height);
    UIEventResult OnEvent(const UIEvent& event) override;

protected:
    void OnRender(UIRenderContext& context) override;

private:
    [[nodiscard]] Rect RowRect(int index) const noexcept;
    ICompanionGateway& gateway_;
};
}
