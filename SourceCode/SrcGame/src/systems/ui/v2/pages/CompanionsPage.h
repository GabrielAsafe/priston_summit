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
    Button* equipmentModeButton_ = nullptr;
    Button* detailModeButton_ = nullptr;
    Button* skillModeButton_ = nullptr;
    Button* tacticsModeButton_ = nullptr;
    Button* customTacticsToggleButton_ = nullptr;
    std::array<Button*, 4> activeSkillPrevButtons_{};
    std::array<Button*, 4> activeSkillNextButtons_{};
    std::array<Button*, 4> activeMinResourceButtons_{};
    std::array<Button*, 4> activeMinTargetsButtons_{};
    std::array<Button*, 4> activeTargetModeButtons_{};
    std::array<Button*, 4> buffSkillPrevButtons_{};
    std::array<Button*, 4> buffSkillNextButtons_{};
    std::array<Button*, 4> buffIntervalPrevButtons_{};
    std::array<Button*, 4> buffIntervalNextButtons_{};
    Button* resetActiveSkillsButton_ = nullptr;
    Button* resetBuffsButton_ = nullptr;
    std::array<Button*, 4> tacticRuleEnabledButtons_{};
    std::array<Button*, 4> tacticRuleConditionButtons_{};
    std::array<Button*, 4> tacticRuleActionButtons_{};
    std::array<Button*, 4> tacticRuleThresholdButtons_{};
    std::array<Button*, 4> tacticRuleUpButtons_{};
    std::array<Button*, 4> tacticRuleDownButtons_{};
    bool tacticsView_ = false;
    bool skillsView_ = false;
    bool buffSkillsView_ = false;
    float refreshAccumulator_ = 1000.0f;
};
}
