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
    UIEventResult OnEvent(const UIEvent& event) override;
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
    void RenderInventoryManager(UIRenderContext& context);
    void RenderTradePreview(UIRenderContext& context);
    void RequestTradePreview(int side, int page);
    void OpenTradePreview();
    void RequestTradeAttempt();
    void RebuildInventoryRects();

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
    // Dedicated owner-side equipment manager; all actions reuse UI v6.
    Button* manageEquipmentButton_ = nullptr;
    Button* closeEquipmentManagerButton_ = nullptr;
    Button* managerRightPrevButton_ = nullptr;
    Button* managerRightNextButton_ = nullptr;
    Button* managerLeftPrevButton_ = nullptr;
    Button* managerLeftNextButton_ = nullptr;
    Button* managerResetButton_ = nullptr;
    Button* managerResyncButton_ = nullptr;
    bool equipmentManagerOpen_ = false;
    bool tradePreviewOpen_ = false;
    std::string tradePreviewName_;
    Button* tradeOpenButton_ = nullptr;
    Button* tradeCloseButton_ = nullptr;
    Button* tradeRefreshButton_ = nullptr;
    Button* tradeAttemptButton_ = nullptr;
    std::array<Button*, 2> tradePrevButtons_{};
    std::array<Button*, 2> tradeNextButtons_{};
    std::array<std::array<Button*,10>,2> tradeItemButtons_{};
    int tradePageRequested_[2]{0,0};
    int tradeSelected_[2]{-1,-1};
    std::uint64_t tradeRefreshedRequest351I_ = 0;
    int selectedInventoryRecordIndex_ = -1;
    std::array<Button*, 10> inventoryItemButtons_{};
    std::array<Button*, 16> inventorySlotButtons_{};
    int selectedEquipSlot_ = 0;
    // Pointer drag is presentation-only: the existing server remains authoritative.
    int dragCandidateIndex_ = -1;
    bool dragActive_ = false;
    int dragEquipmentSlot_ = 0;
    Point dragStart_{};
    Point dragPointer_{};
    Button* inventoryBagAButton_ = nullptr;
    Button* inventoryBagBButton_ = nullptr;
    Button* inventoryMoveBagButton_ = nullptr;
    Button* inventoryOrganizeButton_ = nullptr;
    int activeCompanionBag_ = 1;
    int inventoryDisplayPage_ = 0;
    std::array<CompanionInventoryItemSnapshot, 256> inventoryCache_{};
    std::array<Rect,256> inventoryItemRects_{};
    std::array<int,256> inventoryItemPages_{};
    int inventoryCacheCount_ = 0;
    int inventoryExpectedPage_ = 0;
    int inventoryLastProcessedPage_ = -1;
    bool inventoryCacheComplete_ = false;
    std::string inventoryCacheName_;
    Button* inventoryPrevPageButton_ = nullptr;
    Button* inventoryNextPageButton_ = nullptr;
    Button* inventoryEquipRightButton_ = nullptr;
    Button* inventoryEquipLeftButton_ = nullptr;
    Button* inventoryRefreshButton_ = nullptr;
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
    float iconWarmAccumulator_ = 0.0f;
};
}
