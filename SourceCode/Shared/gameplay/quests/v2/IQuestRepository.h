#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace quests
{
struct QuestProgressRecord
{
    QuestId quest;
    int kind;
    int type;

    QuestProgressRecord() : quest(), kind(0), type(0) {}
    QuestProgressRecord(QuestId questValue, int kindValue, int typeValue)
        : quest(questValue), kind(kindValue), type(typeValue) {}
};

struct QuestKillProgressRecord
{
    QuestId quest;
    int kind;
    int type;
    int killed;
    int count;

    QuestKillProgressRecord()
        : quest(), kind(0), type(0), killed(0), count(0) {}
};

struct QuestItemRewardRecord
{
    std::string itemCode;
    int count;

    QuestItemRewardRecord() : itemCode(), count(0) {}
    QuestItemRewardRecord(const std::string& itemCodeValue, int countValue)
        : itemCode(itemCodeValue), count(countValue) {}
};

struct QuestRewardPayload
{
    std::int64_t experience;
    int gold;
    int coin;
    int tCoin;
    std::vector<QuestItemRewardRecord> items;

    QuestRewardPayload()
        : experience(0), gold(0), coin(0), tCoin(0), items() {}
};

class IQuestRepository
{
public:
    virtual ~IQuestRepository() {}

    // false means the authoritative persistence read failed. The boolean output
    // is only meaningful on success.
    virtual bool TryIsCompleted(const std::string& playerKey,
                                QuestId quest,
                                bool& completed) const = 0;
    virtual bool TryIsCompletedVariant(const std::string& playerKey,
                                       QuestId quest,
                                       int questKind,
                                       int questType,
                                       bool& completed) const = 0;
    virtual bool TryIsInProgress(const std::string& playerKey,
                                 QuestId quest,
                                 bool& inProgress) const = 0;
    virtual bool TryHasPendingObjectives(const std::string& playerKey,
                                         QuestId quest,
                                         bool& pending) const = 0;
    virtual bool TryIsObjectivePending(const std::string& playerKey,
                                       QuestId quest,
                                       const std::string& monsterName,
                                       bool& pending) const = 0;
    virtual bool CompleteObjective(const std::string& playerKey,
                                   QuestId quest,
                                   const std::string& monsterName) const = 0;
    virtual bool CompleteObjectives(const std::string& playerKey,
                                    QuestId quest) const = 0;
    virtual bool CancelProgress(const std::string& playerKey,
                                QuestId quest) const = 0;
    virtual bool StartProgress(const std::string& playerKey,
                               QuestId quest,
                               int questKind,
                               int questType,
                               int partyFlag) const = 0;
    virtual bool CancelProgressVariant(const std::string& playerKey,
                                       QuestId quest,
                                       int questKind,
                                       int questType) const = 0;
    virtual bool ListInvalidForLevel(const std::string& playerKey,
                                     int playerLevel,
                                     std::vector<QuestProgressRecord>& out) const = 0;
    virtual bool RecordMonsterKill(const std::string& playerKey,
                                   const std::string& monsterName,
                                   int area,
                                   bool partyCredit,
                                   std::vector<QuestKillProgressRecord>& updated) const = 0;
    virtual bool ClaimReward(const std::string& playerKey,
                             QuestId quest,
                             int questKind,
                             int questType,
                             QuestRewardPayload& reward) const = 0;
    virtual bool HasPartyObjectiveTarget(const std::string& playerKey,
                                         const std::string& monsterName,
                                         int area,
                                         bool& pending) const = 0;
    virtual bool EnsureObjective(const std::string& playerKey,
                                 QuestId quest,
                                 int questKind,
                                 int questType,
                                 const std::string& monsterName,
                                 int count,
                                 int partyFlag,
                                 int map) const = 0;
    virtual bool MarkSingleCompletedForTesting(const std::string& playerKey,
                                               QuestId quest) const = 0;
};
}
}
