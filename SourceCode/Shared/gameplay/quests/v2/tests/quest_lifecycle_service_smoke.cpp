#include "gameplay/quests/v2/QuestLifecycleService.h"

#include <cassert>

namespace
{
class FakeLifecycleService final : public gameplay::quests::IQuestLifecycleService
{
public:
    void VerifyPlayerLevel(gameplay::PlayerId player) override
    {
        lastPlayer = player;
        ++calls;
    }

    gameplay::PlayerId lastPlayer;
    int calls = 0;
};
}

int main()
{
    FakeLifecycleService service;
    service.VerifyPlayerLevel(gameplay::PlayerId(55));
    assert(service.calls == 1);
    assert(service.lastPlayer.value == 55u);
    return 0;
}
