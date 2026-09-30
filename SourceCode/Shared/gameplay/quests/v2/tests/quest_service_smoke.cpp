#include <cassert>
#include <map>
#include <string>

#include "gameplay/quests/v2/QuestService.h"

namespace
{
struct State
{
    bool readable;
    bool completed;
    bool active;
    State(bool r=true, bool c=false, bool a=false) : readable(r), completed(c), active(a) {}
};

class FakeStates final : public gameplay::quests::IQuestStateQuery
{
public:
    mutable std::map<std::string, State> states;

    bool TryGetState(const std::string& playerKey,
                     gameplay::QuestId quest,
                     gameplay::quests::QuestStateSnapshot& out) const override
    {
        if(!quest.Valid()) return false;
        std::map<std::string, State>::const_iterator it=states.find(playerKey);
        if(it==states.end() || !it->second.readable) return false;
        out.quest=quest;
        out.completed=it->second.completed;
        out.inProgress=it->second.active;
        return true;
    }
};

class FakeCommands final : public gameplay::quests::IQuestCommands
{
public:
    FakeCommands(FakeStates& states) : states_(states), starts(0), accept(true) {}

    bool Start(gameplay::PlayerId player, const gameplay::quests::QuestStartCommand&) override
    {
        if(!player.Valid() || !accept) return false;
        ++starts;
        states_.states["new"].active=true;
        return true;
    }
    bool Cancel(gameplay::PlayerId, const gameplay::quests::QuestCommand&) override { return false; }
    bool ClaimReward(gameplay::PlayerId, const gameplay::quests::QuestCommand&) override { return false; }

    FakeStates& states_;
    int starts;
    bool accept;
};
}

int main()
{
    FakeStates states;
    states.states["done"] = State(true, true, false);
    states.states["active"] = State(true, false, true);
    states.states["new"] = State(true, false, false);
    states.states["unreadable"] = State(false, false, false);
    FakeCommands commands(states);
    gameplay::quests::QuestService service(states, commands);

    gameplay::quests::QuestStartCommand start;
    start.quest = gameplay::QuestId(500u);
    start.kind = 9;

    assert(!service.EnsureStarted(gameplay::PlayerId(7u), "done", start));
    assert(commands.starts == 0);
    assert(service.EnsureStarted(gameplay::PlayerId(7u), "active", start));
    assert(commands.starts == 0);
    assert(service.EnsureStarted(gameplay::PlayerId(7u), "new", start));
    assert(commands.starts == 1);
    assert(!service.EnsureStarted(gameplay::PlayerId(7u), "unreadable", start));
    assert(commands.starts == 1);

    states.states["new"].active=false;
    commands.accept=false;
    assert(!service.EnsureStarted(gameplay::PlayerId(7u), "new", start));
    assert(commands.starts == 1);
    return 0;
}
