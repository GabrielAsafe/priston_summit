#include "gameplay/quests/v2/QuestAdminService.h"

#include <cassert>

namespace
{
class FakeQuestAdminService final : public gameplay::quests::IQuestAdminService
{
public:
    bool MarkCompletedForTesting(gameplay::PlayerId player, gameplay::QuestId quest) override
    {
        lastPlayer = player;
        lastQuest = quest;
        ++calls;
        return result;
    }

    gameplay::PlayerId lastPlayer;
    gameplay::QuestId lastQuest;
    int calls = 0;
    bool result = true;
};
}

int main()
{
    FakeQuestAdminService service;
    assert(service.MarkCompletedForTesting(gameplay::PlayerId(42), gameplay::QuestId(1337)));
    assert(service.calls == 1);
    assert(service.lastPlayer.value == 42u);
    assert(service.lastQuest.value == 1337u);
    return 0;
}
