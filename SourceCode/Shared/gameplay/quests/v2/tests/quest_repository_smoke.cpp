#include <cassert>

#include "gameplay/quests/v2/IQuestRepository.h"

namespace
{
class FakeRepository final : public gameplay::quests::IQuestRepository
{
public:
    bool TryIsCompleted(const std::string& playerKey,
                        gameplay::QuestId quest,
                        bool& completed) const override
    {
        if(playerKey.empty() || !quest.Valid()) return false;
        completed = playerKey == "done";
        return true;
    }

    bool TryIsCompletedVariant(const std::string& playerKey,
                               gameplay::QuestId quest,
                               int questKind,
                               int questType,
                               bool& completed) const override
    {
        if(playerKey.empty() || !quest.Valid()) return false;
        completed = playerKey == "done" && questKind == 9 && questType == 1;
        return true;
    }

    bool TryIsInProgress(const std::string& playerKey,
                         gameplay::QuestId quest,
                         bool& inProgress) const override
    {
        if(playerKey.empty() || !quest.Valid()) return false;
        inProgress = playerKey == "active";
        return true;
    }

    bool TryHasPendingObjectives(const std::string& playerKey,
                                 gameplay::QuestId quest,
                                 bool& pending) const override
    {
        if(playerKey.empty() || !quest.Valid()) return false;
        pending = playerKey == "pending";
        return true;
    }

    bool TryIsObjectivePending(const std::string& playerKey,
                               gameplay::QuestId quest,
                               const std::string& monsterName,
                               bool& pending) const override
    {
        if(playerKey.empty() || !quest.Valid() || monsterName.empty()) return false;
        pending = monsterName == "Prince";
        return true;
    }

    bool CompleteObjective(const std::string& playerKey,
                           gameplay::QuestId quest,
                           const std::string& monsterName) const override
    {
        return !playerKey.empty() && quest.Valid() && !monsterName.empty();
    }

    bool CompleteObjectives(const std::string& playerKey,
                            gameplay::QuestId quest) const override
    {
        return !playerKey.empty() && quest.Valid();
    }

    bool CancelProgress(const std::string& playerKey,
                        gameplay::QuestId quest) const override
    {
        return !playerKey.empty() && quest.Valid();
    }

    bool StartProgress(const std::string& playerKey,
                       gameplay::QuestId quest,
                       int questKind,
                       int questType,
                       int partyFlag) const override
    {
        return !playerKey.empty() && quest.Valid() &&
            (questKind == 21 || questKind == 7 || questKind == 9) &&
            questType >= 0 && partyFlag >= 0;
    }

    bool CancelProgressVariant(const std::string& playerKey,
                               gameplay::QuestId quest,
                               int questKind,
                               int questType) const override
    {
        return !playerKey.empty() && quest.Valid() && questKind > 0 && questType >= 0;
    }

    bool ListInvalidForLevel(const std::string& playerKey,
                             int playerLevel,
                             std::vector<gameplay::quests::QuestProgressRecord>& out) const override
    {
        out.clear();
        if(playerKey.empty()) return false;
        if(playerLevel < 10)
            out.push_back(gameplay::quests::QuestProgressRecord(gameplay::QuestId(2u), 9, 0));
        return true;
    }

    bool RecordMonsterKill(const std::string& playerKey,
                           const std::string& monsterName,
                           int area,
                           bool partyCredit,
                           std::vector<gameplay::quests::QuestKillProgressRecord>& updated) const override
    {
        updated.clear();
        if(playerKey.empty() || monsterName.empty() || area < 0) return false;
        gameplay::quests::QuestKillProgressRecord row;
        row.quest = gameplay::QuestId(3u);
        row.kind = partyCredit ? 7 : 9;
        row.type = 1;
        row.killed = 2;
        row.count = 5;
        updated.push_back(row);
        return true;
    }

    bool ClaimReward(const std::string& playerKey,
                     gameplay::QuestId quest,
                     int questKind,
                     int questType,
                     gameplay::quests::QuestRewardPayload& reward) const override
    {
        reward = gameplay::quests::QuestRewardPayload();
        if(playerKey.empty() || !quest.Valid() || questKind <= 0 || questType < 0) return false;
        reward.experience = 1234;
        reward.gold = 50;
        reward.coin = 2;
        reward.tCoin = 1;
        reward.items.push_back(gameplay::quests::QuestItemRewardRecord("GG101", 3));
        return true;
    }

    bool HasPartyObjectiveTarget(const std::string& playerKey,
                                 const std::string& monsterName,
                                 int area,
                                 bool& pending) const override
    {
        if(playerKey.empty() || monsterName.empty() || area < 0) return false;
        pending = monsterName == "Prince" && area == 4;
        return true;
    }

    bool EnsureObjective(const std::string& playerKey,
                         gameplay::QuestId quest,
                         int questKind,
                         int questType,
                         const std::string& monsterName,
                         int count,
                         int partyFlag,
                         int map) const override
    {
        return !playerKey.empty() && quest.Valid() && questKind > 0 && questType >= 0 &&
            !monsterName.empty() && count > 0 && partyFlag >= 0 && map >= -1;
    }

    bool MarkSingleCompletedForTesting(const std::string& playerKey,
                                       gameplay::QuestId quest) const override
    {
        return !playerKey.empty() && quest.Valid();
    }
};
}

int main()
{
    FakeRepository repository;
    bool value = false;
    assert(repository.TryIsCompleted("done", gameplay::QuestId(1u), value) && value);
    value = false;
    assert(repository.TryIsCompletedVariant("done", gameplay::QuestId(1u), 9, 1, value) && value);
    value = false;
    assert(repository.TryIsInProgress("active", gameplay::QuestId(1u), value) && value);
    assert(!repository.TryIsCompleted("", gameplay::QuestId(1u), value));
    value = false;
    assert(repository.TryHasPendingObjectives("pending", gameplay::QuestId(1u), value) && value);
    value = false;
    assert(repository.TryIsObjectivePending("active", gameplay::QuestId(1u), "Prince", value) && value);
    assert(repository.CompleteObjective("active", gameplay::QuestId(1u), "Prince"));
    assert(repository.CompleteObjectives("active", gameplay::QuestId(1u)));
    assert(repository.CancelProgress("active", gameplay::QuestId(1u)));
    assert(repository.StartProgress("active", gameplay::QuestId(1u), 9, 0, 1));
    assert(repository.CancelProgressVariant("active", gameplay::QuestId(1u), 9, 0));
    std::vector<gameplay::quests::QuestProgressRecord> invalid;
    assert(repository.ListInvalidForLevel("active", 1, invalid));
    assert(invalid.size() == 1u && invalid[0].quest.value == 2u);
    std::vector<gameplay::quests::QuestKillProgressRecord> updated;
    assert(repository.RecordMonsterKill("active", "Prince", 4, true, updated));
    assert(updated.size() == 1u && updated[0].quest.value == 3u);
    assert(updated[0].kind == 7 && updated[0].killed == 2 && updated[0].count == 5);
    gameplay::quests::QuestRewardPayload reward;
    assert(repository.ClaimReward("active", gameplay::QuestId(3u), 9, 0, reward));
    assert(reward.experience == 1234 && reward.gold == 50 && reward.coin == 2 && reward.tCoin == 1);
    assert(reward.items.size() == 1u && reward.items[0].itemCode == "GG101" && reward.items[0].count == 3);
    bool partyPending = false;
    assert(repository.HasPartyObjectiveTarget("active", "Prince", 4, partyPending) && partyPending);
    assert(repository.EnsureObjective("active", gameplay::QuestId(10012u), 9, 0, "Crystal Tower", 1, 0, 32));
    assert(repository.MarkSingleCompletedForTesting("active", gameplay::QuestId(10012u)));
    return 0;
}
