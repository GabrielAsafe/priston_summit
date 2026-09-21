#pragma once

#include <array>
#include <string>
#include "systems/ui/v2/core/Container.h"
#include "systems/ui/v2/domain/ICompanionGateway.h"

namespace ui::v2
{
class Button;

class CompanionsPage final : public Container
{
public:
    explicit CompanionsPage(ICompanionGateway& gateway, UIId id = InvalidUIId);

    void Update(float deltaSeconds) override;
    void ForceRefresh();

protected:
    void OnRender(UIRenderContext& context) override;

private:
    static constexpr std::size_t VisibleCharacterRows = 5;

    const CompanionUiEntry* SelectedEntry() const;
    void BuildControls();
    void LayoutControls();
    void SelectVisibleRow(std::size_t row);
    void ExecuteForSelected(CompanionUiAction action, int value = 0);
    void RefreshControls();

    ICompanionGateway& gateway_;
    std::array<Button*, VisibleCharacterRows> characterButtons_{};
    Button* summonDismissButton_ = nullptr;
    Button* dismissAllButton_ = nullptr;
    Button* refreshButton_ = nullptr;
    Button* aggressiveButton_ = nullptr;
    Button* followButton_ = nullptr;
    Button* passiveButton_ = nullptr;
    Button* tacticPrevButton_ = nullptr;
    Button* tacticNextButton_ = nullptr;
    Button* retreatPrevButton_ = nullptr;
    Button* retreatNextButton_ = nullptr;
    Button* rangePrevButton_ = nullptr;
    Button* rangeNextButton_ = nullptr;
    Button* rightPrevButton_ = nullptr;
    Button* rightNextButton_ = nullptr;
    Button* leftPrevButton_ = nullptr;
    Button* leftNextButton_ = nullptr;
    Button* resetEquipmentButton_ = nullptr;
    Button* resyncButton_ = nullptr;
    float refreshAccumulator_ = 1000.0f;
};
}
