#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace quests
{
struct QuestObjectiveView
{
    std::string monsterName;
    int required;
    int killed;
    int map;

    QuestObjectiveView() : monsterName(), required(0), killed(0), map(0) {}
};

struct QuestItemRewardView
{
    std::string contentKey;
    int count;

    QuestItemRewardView() : contentKey(), count(0) {}
};

struct QuestSpoilerEntryView
{
    QuestId quest;
    int kind;
    std::string name;
    bool active;

    QuestSpoilerEntryView() : quest(), kind(0), name(), active(false) {}
};

struct QuestSpoilerBatchView
{
    std::vector<QuestSpoilerEntryView> entries;
};

struct QuestDetailView
{
    QuestId quest;
    int kind;
    int type;
    std::string name;
    std::string title;
    std::string description;
    int levelMin;
    int levelMax;
    int partyFlag;
    int rewardGold;
    std::int64_t rewardExperience;
    int rewardCoin;
    int rewardTCoin;
    std::vector<QuestObjectiveView> objectives;
    std::vector<QuestItemRewardView> itemRewards;

    QuestDetailView()
        : quest(), kind(0), type(0), name(), title(), description(),
          levelMin(0), levelMax(0), partyFlag(0), rewardGold(0),
          rewardExperience(0), rewardCoin(0), rewardTCoin(0),
          objectives(), itemRewards() {}
};

class IQuestViewQuery
{
public:
    virtual ~IQuestViewQuery() {}
    virtual bool TryGetDetail(const std::string& playerKey,
                              QuestId quest,
                              int questKind,
                              QuestDetailView& out) const = 0;
    virtual bool TryGetSpoilerBatches(const std::string& playerKey,
                                      int playerLevel,
                                      int spoilerNumber,
                                      std::vector<QuestSpoilerBatchView>& out) const = 0;
};
}
}
