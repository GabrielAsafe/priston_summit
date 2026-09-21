#pragma once

#include "systems/ui/v2/domain/QuestUIModels.h"

namespace ui::v2
{
class IQuestGateway
{
public:
    virtual ~IQuestGateway() = default;

    [[nodiscard]] virtual QuestJournalSnapshot GetSnapshot() const = 0;
    [[nodiscard]] virtual QuestEntrySnapshot GetTrackedQuest() const = 0;
    virtual bool SelectQuest(int sectionId, int questId) = 0;
    virtual bool RequestAcceptSelected() = 0;
    virtual bool RequestCancelSelected() = 0;
    virtual bool ClaimSelectedReward() = 0;
    virtual bool TrackSelected() = 0;
};
}
