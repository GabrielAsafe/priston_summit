#pragma once

#include "systems/ui/v2/domain/IQuestGateway.h"

namespace ui::v2::legacy
{
class LegacyQuestGateway final : public IQuestGateway
{
public:
    [[nodiscard]] QuestJournalSnapshot GetSnapshot() const override;
    [[nodiscard]] QuestEntrySnapshot GetTrackedQuest() const override;
    bool SelectQuest(int sectionId, int questId) override;
    bool RequestAcceptSelected() override;
    bool RequestCancelSelected() override;
    bool ClaimSelectedReward() override;
    bool TrackSelected() override;

    bool EnsureBackendOpen();
    void HideBackend();
    void ShutdownBackend();

private:
    int trackedSectionId_ = 0;
    int trackedQuestId_ = 0;
    mutable int selectedClassicQuestId_ = 0;
    mutable int selectedAmandaQuestId_ = 0;
    mutable int selectedCodexQuestId_ = 0;
};
}
