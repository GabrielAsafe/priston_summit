#include <cassert>

#include "gameplay/quests/v2/QuestStateQuery.h"

namespace
{
class FakeQuestStateQuery final : public gameplay::quests::IQuestStateQuery
{
public:
    bool TryGetState(const std::string& playerKey,
                     gameplay::QuestId quest,
                     gameplay::quests::QuestStateSnapshot& out) const override
    {
        if(playerKey.empty() || !quest.Valid())
            return false;
        out.quest = quest;
        out.completed = playerKey == "completed";
        out.inProgress = playerKey == "active";
        return true;
    }
};
}

int main()
{
    FakeQuestStateQuery query;
    gameplay::quests::QuestStateSnapshot state;
    assert(!query.TryGetState("", gameplay::QuestId(1u), state));
    assert(!query.TryGetState("active", gameplay::QuestId(), state));

    assert(query.TryGetState("active", gameplay::QuestId(77u), state));
    assert(state.quest.value == 77u);
    assert(!state.completed);
    assert(state.inProgress);
    assert(state.Active());

    assert(query.TryGetState("completed", gameplay::QuestId(77u), state));
    assert(state.completed);
    assert(!state.inProgress);
    assert(!state.Active());
    return 0;
}
