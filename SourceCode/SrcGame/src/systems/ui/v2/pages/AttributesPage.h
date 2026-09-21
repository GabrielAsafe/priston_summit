#pragma once

#include <array>
#include "systems/ui/v2/core/Container.h"
#include "systems/ui/v2/viewmodels/AttributesViewModel.h"

namespace ui::v2
{
class Button;

class AttributesPage final : public Container
{
public:
    explicit AttributesPage(ICharacterStatsGateway& gateway, UIId id = InvalidUIId);

    void Update(float deltaSeconds) override;

protected:
    void OnRender(UIRenderContext& context) override;

private:
    struct AttributeButtons
    {
        Button* plusOne = nullptr;
        Button* plusTen = nullptr;
    };

    void BuildControls();
    void LayoutControls();

    AttributesViewModel viewModel_;
    std::array<AttributeButtons, 5> buttons_{};
    Button* undoButton_ = nullptr;
    float refreshAccumulator_ = 0.0f;
};
}
