#pragma once

#include "gameplay/quests/v2/IQuestRepository.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
class LegacyQuestRepository final : public IQuestRepository
{
public:
    bool TryIsCompleted(const std::string& playerKey,
                        QuestId quest,
                        bool& completed) const override;
    bool TryIsCompletedVariant(const std::string& playerKey,
                               QuestId quest,
                               int questKind,
                               int questType,
                               bool& completed) const override;
    bool TryIsInProgress(const std::string& playerKey,
                         QuestId quest,
                         bool& inProgress) const override;
    bool TryHasPendingObjectives(const std::string& playerKey,
                                 QuestId quest,
                                 bool& pending) const override;
    bool TryIsObjectivePending(const std::string& playerKey,
                               QuestId quest,
                               const std::string& monsterName,
                               bool& pending) const override;
    bool CompleteObjective(const std::string& playerKey,
                           QuestId quest,
                           const std::string& monsterName) const override;
    bool CompleteObjectives(const std::string& playerKey,
                            QuestId quest) const override;
    bool CancelProgress(const std::string& playerKey,
                        QuestId quest) const override;
    bool StartProgress(const std::string& playerKey,
                       QuestId quest,
                       int questKind,
                       int questType,
                       int partyFlag) const override;
    bool CancelProgressVariant(const std::string& playerKey,
                               QuestId quest,
                               int questKind,
                               int questType) const override;
    bool ListInvalidForLevel(const std::string& playerKey,
                             int playerLevel,
                             std::vector<QuestProgressRecord>& out) const override;
    bool RecordMonsterKill(const std::string& playerKey,
                           const std::string& monsterName,
                           int area,
                           bool partyCredit,
                           std::vector<QuestKillProgressRecord>& updated) const override;
    bool ClaimReward(const std::string& playerKey,
                     QuestId quest,
                     int questKind,
                     int questType,
                     QuestRewardPayload& reward) const override;
    bool HasPartyObjectiveTarget(const std::string& playerKey,
                                 const std::string& monsterName,
                                 int area,
                                 bool& pending) const override;
    bool EnsureObjective(const std::string& playerKey,
                         QuestId quest,
                         int questKind,
                         int questType,
                         const std::string& monsterName,
                         int count,
                         int partyFlag,
                         int map) const override;
    bool MarkSingleCompletedForTesting(const std::string& playerKey,
                                       QuestId quest) const override;
};

const LegacyQuestRepository& ServerLegacyQuestRepository();
}
}
}
