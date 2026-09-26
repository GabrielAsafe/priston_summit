#pragma once

#include <array>

#include "systems/ui/v2/core/Container.h"
#include "systems/ui/v2/domain/ICovenantGateway.h"

namespace ui::v2
{
class Button;

class CovenantsPage final : public Container
{
public:
    explicit CovenantsPage(ICovenantGateway& gateway, UIId id = InvalidUIId);

    void Update(float deltaSeconds) override;
    void ForceRefresh();

protected:
    void OnRender(UIRenderContext& context) override;

private:
    void BuildControls();
    void Layout();
    void RefreshControls();
    [[nodiscard]] const CovenantUiEntry* SelectedEntry() const noexcept;
    [[nodiscard]] std::string FirstRefinableTrophy() const;

    ICovenantGateway& gateway_;
    std::array<Button*, CovenantUiMaxCovenants> covenantButtons_{};
    Button* primaryButton_ = nullptr;
    Button* powerButton_ = nullptr;
    Button* artisanModeButton_ = nullptr;
    Button* refineButton_ = nullptr;
    Button* mineButton_ = nullptr;
    Button* gatherButton_ = nullptr;
    Button* refreshButton_ = nullptr;
    float refreshAccumulator_ = 0.0f;
};
}
