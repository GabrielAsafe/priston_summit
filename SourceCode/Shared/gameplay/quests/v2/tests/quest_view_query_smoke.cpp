#include <cassert>

#include "gameplay/quests/v2/QuestViewQuery.h"

namespace
{
class FakeQuestViewQuery final : public gameplay::quests::IQuestViewQuery
{
public:
    bool TryGetDetail(const std::string& playerKey,
                      gameplay::QuestId quest,
                      int questKind,
                      gameplay::quests::QuestDetailView& out) const override
    {
        out = gameplay::quests::QuestDetailView();
        if(playerKey.empty() || !quest.Valid() || questKind != 9)
            return false;
        out.quest = quest;
        out.kind = questKind;
        out.type = 2;
        out.name = "Test Quest";
        out.rewardExperience = 123;
        gameplay::quests::QuestObjectiveView objective;
        objective.monsterName = "Prince";
        objective.required = 3;
        objective.killed = 1;
        objective.map = 4;
        out.objectives.push_back(objective);
        gameplay::quests::QuestItemRewardView item;
        item.contentKey = "GG101";
        item.count = 2;
        out.itemRewards.push_back(item);
        return true;
    }

    bool TryGetSpoilerBatches(const std::string& playerKey,
                              int playerLevel,
                              int spoilerNumber,
                              std::vector<gameplay::quests::QuestSpoilerBatchView>& out) const override
    {
        out.clear();
        if(playerKey.empty() || playerLevel <= 0 || spoilerNumber != 9)
            return false;
        gameplay::quests::QuestSpoilerBatchView batch;
        gameplay::quests::QuestSpoilerEntryView entry;
        entry.quest = gameplay::QuestId(10u);
        entry.kind = 9;
        entry.name = "Test Quest";
        entry.active = true;
        batch.entries.push_back(entry);
        out.push_back(batch);
        return true;
    }
};
}

int main()
{
    FakeQuestViewQuery query;
    gameplay::quests::QuestDetailView view;
    assert(query.TryGetDetail("player", gameplay::QuestId(10u), 9, view));
    assert(view.quest.value == 10u && view.type == 2 && view.name == "Test Quest");
    assert(view.objectives.size() == 1u && view.objectives[0].killed == 1);
    assert(view.itemRewards.size() == 1u && view.itemRewards[0].count == 2);
    std::vector<gameplay::quests::QuestSpoilerBatchView> batches;
    assert(query.TryGetSpoilerBatches("player", 100, 9, batches));
    assert(batches.size() == 1u && batches[0].entries.size() == 1u);
    assert(batches[0].entries[0].active && batches[0].entries[0].quest.value == 10u);
    return 0;
}
