#include "gameplay/quests/v2/QuestObjectiveService.h"

#include <cassert>

namespace
{
class FakeObjectiveService final : public gameplay::quests::IQuestObjectiveService
{
public:
    bool IsInProgress(gameplay::PlayerId player, gameplay::QuestId quest) override
    { lastPlayer=player; lastQuest=quest; return inProgress; }
    bool HasPendingObjectives(gameplay::PlayerId player, gameplay::QuestId quest) override
    { lastPlayer=player; lastQuest=quest; return pending; }
    bool CompleteObjectives(gameplay::PlayerId player, gameplay::QuestId quest) override
    { lastPlayer=player; lastQuest=quest; ++completeCalls; return complete; }
    bool CancelProgress(gameplay::PlayerId player, gameplay::QuestId quest) override
    { lastPlayer=player; lastQuest=quest; ++cancelCalls; return cancel; }

    gameplay::PlayerId lastPlayer;
    gameplay::QuestId lastQuest;
    bool inProgress=true;
    bool pending=true;
    bool complete=true;
    bool cancel=true;
    int completeCalls=0;
    int cancelCalls=0;
};
}

int main()
{
    FakeObjectiveService service;
    const gameplay::PlayerId player(7);
    const gameplay::QuestId quest(10013);
    assert(service.IsInProgress(player, quest));
    assert(service.HasPendingObjectives(player, quest));
    assert(service.CompleteObjectives(player, quest));
    assert(service.CancelProgress(player, quest));
    assert(service.lastPlayer.value == 7u);
    assert(service.lastQuest.value == 10013u);
    assert(service.completeCalls == 1);
    assert(service.cancelCalls == 1);
    return 0;
}
