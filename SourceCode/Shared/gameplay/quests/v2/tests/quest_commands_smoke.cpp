#include <cassert>

#include "gameplay/quests/v2/QuestCommands.h"

namespace
{
class FakeQuestCommands final : public gameplay::quests::IQuestCommands
{
public:
    FakeQuestCommands() : starts(0), cancels(0), rewards(0) {}

    bool Start(gameplay::PlayerId player, const gameplay::quests::QuestStartCommand& command) override
    {
        if(!player.Valid() || !command.quest.Valid()) return false;
        ++starts;
        return true;
    }
    bool Cancel(gameplay::PlayerId player, const gameplay::quests::QuestCommand& command) override
    {
        if(!player.Valid() || !command.quest.Valid()) return false;
        ++cancels;
        return true;
    }
    bool ClaimReward(gameplay::PlayerId player, const gameplay::quests::QuestCommand& command) override
    {
        if(!player.Valid() || !command.quest.Valid()) return false;
        ++rewards;
        return true;
    }

    int starts;
    int cancels;
    int rewards;
};
}

int main()
{
    FakeQuestCommands commands;
    gameplay::quests::QuestStartCommand start;
    start.quest = gameplay::QuestId(101u);
    start.kind = 9;
    start.type = 2;
    start.partyFlag = 1;
    assert(commands.Start(gameplay::PlayerId(7u), start));

    gameplay::quests::QuestCommand command;
    command.quest = gameplay::QuestId(101u);
    command.kind = 9;
    command.type = 2;
    assert(commands.Cancel(gameplay::PlayerId(7u), command));
    assert(commands.ClaimReward(gameplay::PlayerId(7u), command));
    assert(commands.starts == 1 && commands.cancels == 1 && commands.rewards == 1);
    assert(!commands.Start(gameplay::PlayerId(), start));
    return 0;
}
