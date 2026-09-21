#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "gameplay/conditions/v2/Condition.h"
#include "gameplay/covenants/v2/CovenantRewardProcessor.h"

namespace
{
class FakeRepository : public gameplay::covenants::ICovenantRepository
{
public:
    std::map<std::string, std::vector<gameplay::covenants::CovenantMembershipState> > memberships;
    std::map<std::string, gameplay::covenants::CovenantEffectReceiptStatus> receipts;
    bool failNextMark;

    FakeRepository() : failNextMark(false) {}

    bool LoadAll(const std::string& playerKey,
                 std::vector<gameplay::covenants::CovenantMembershipState>& values)
    {
        values = memberships[playerKey];
        return true;
    }

    bool Save(const std::string&, const gameplay::covenants::CovenantMembershipState&)
    {
        return true;
    }

    gameplay::covenants::CovenantReputationSaveResult SaveReputationChange(
        const std::string&,
        const gameplay::covenants::CovenantMembershipState&,
        int,
        const std::string&)
    {
        return gameplay::covenants::CovenantReputationSaveResult::Applied;
    }

    std::string Key(const std::string& playerKey, int covenantId, const std::string& effectKey) const
    {
        return playerKey + ":" + std::to_string(covenantId) + ":" + effectKey;
    }

    gameplay::covenants::CovenantEffectClaimResult ClaimEffect(
        const std::string& playerKey, int covenantId, const std::string& effectKey)
    {
        const std::string key = Key(playerKey, covenantId, effectKey);
        std::map<std::string, gameplay::covenants::CovenantEffectReceiptStatus>::iterator it =
            receipts.find(key);
        if (it == receipts.end())
        {
            receipts[key] = gameplay::covenants::CovenantEffectReceiptStatus::Pending;
            return gameplay::covenants::CovenantEffectClaimResult::Claimed;
        }
        return it->second == gameplay::covenants::CovenantEffectReceiptStatus::Applied
                   ? gameplay::covenants::CovenantEffectClaimResult::AlreadyApplied
                   : gameplay::covenants::CovenantEffectClaimResult::Pending;
    }

    bool MarkEffectApplied(const std::string& playerKey, int covenantId, const std::string& effectKey)
    {
        if (failNextMark)
        {
            failNextMark = false;
            return false;
        }
        const std::string key = Key(playerKey, covenantId, effectKey);
        if (receipts.find(key) == receipts.end())
            return false;
        receipts[key] = gameplay::covenants::CovenantEffectReceiptStatus::Applied;
        return true;
    }

    gameplay::covenants::CovenantEffectReceiptLoadResult GetEffectReceiptStatus(
        const std::string& playerKey,
        int covenantId,
        const std::string& effectKey,
        gameplay::covenants::CovenantEffectReceiptStatus& status)
    {
        const std::string key = Key(playerKey, covenantId, effectKey);
        std::map<std::string, gameplay::covenants::CovenantEffectReceiptStatus>::const_iterator it =
            receipts.find(key);
        if (it == receipts.end())
            return gameplay::covenants::CovenantEffectReceiptLoadResult::NotFound;
        status = it->second;
        return gameplay::covenants::CovenantEffectReceiptLoadResult::Found;
    }

    bool ListPendingEffects(const std::string& playerKey,
                            std::vector<gameplay::covenants::CovenantEffectReceiptEntry>& values)
    {
        values.clear();
        const std::string prefix = playerKey + ":";
        for (std::map<std::string, gameplay::covenants::CovenantEffectReceiptStatus>::const_iterator it =
                 receipts.begin(); it != receipts.end(); ++it)
        {
            if (it->second != gameplay::covenants::CovenantEffectReceiptStatus::Pending ||
                it->first.compare(0, prefix.size(), prefix) != 0)
                continue;
            const std::string rest = it->first.substr(prefix.size());
            const std::size_t separator = rest.find(':');
            assert(separator != std::string::npos);
            gameplay::covenants::CovenantEffectReceiptEntry entry;
            entry.covenantId = std::stoi(rest.substr(0, separator));
            entry.effectKey = rest.substr(separator + 1);
            values.push_back(entry);
        }
        return true;
    }
};

class FakeRewardGateway : public gameplay::rewards::IRewardGateway
{
public:
    int grants;
    explicit FakeRewardGateway() : grants(0) {}

    bool Supports(gameplay::rewards::RewardType type) const
    {
        return type == gameplay::rewards::RewardType::Gold ||
               type == gameplay::rewards::RewardType::StoryFlag;
    }

    bool Validate(const gameplay::rewards::RewardDefinition& reward) const
    {
        return Supports(reward.type) &&
               ((reward.type == gameplay::rewards::RewardType::Gold && reward.amount > 0) ||
                (reward.type == gameplay::rewards::RewardType::StoryFlag && !reward.key.empty()));
    }

    bool IsRetrySafe(const gameplay::rewards::RewardDefinition& reward) const
    {
        return reward.type == gameplay::rewards::RewardType::StoryFlag;
    }

    gameplay::rewards::RewardResult Grant(const std::string&,
                                           const gameplay::rewards::RewardDefinition&,
                                           const std::string&)
    {
        ++grants;
        return gameplay::rewards::RewardResult::Granted;
    }
};

class AlwaysContext : public gameplay::conditions::ConditionContext
{
public:
    int PlayerLevel() const { return 1; }
    int LegacyJobCode() const { return 0; }
    bool HasStoryFlag(const std::string&) const { return false; }
    int StoryNodeState(int, int) const { return 0; }
    int QuestState(int) const { return 0; }
    bool HasCovenant(int) const { return false; }
    int CovenantRank(int) const { return 0; }
    int CovenantReputation(int) const { return 0; }
    int ProfessionLevel(const std::string&) const { return 0; }
};

gameplay::covenants::CovenantRewardDefinition GoldReward(const std::string& key, int amount)
{
    gameplay::covenants::CovenantRewardDefinition result;
    result.key = key;
    result.reward.type = gameplay::rewards::RewardType::Gold;
    result.reward.amount = amount;
    return result;
}

gameplay::covenants::CovenantRewardDefinition SafeFlagReward(const std::string& key)
{
    gameplay::covenants::CovenantRewardDefinition result;
    result.key = key;
    result.reward.type = gameplay::rewards::RewardType::StoryFlag;
    result.reward.key = "flag." + key;
    result.reward.amount = 1;
    return result;
}
}

int main()
{
    FakeRepository repository;
    gameplay::covenants::CovenantService service(repository);
    FakeRewardGateway gateway;
    gameplay::rewards::RewardService rewardService(gateway);
    gameplay::covenants::CovenantRewardProcessor processor(repository, service, rewardService);

    gameplay::covenants::CovenantDefinition covenant;
    covenant.id = 5;
    covenant.key = "guild";
    covenant.joinRewards.push_back(GoldReward("join.gold", 10));
    assert(service.RegisterCovenant(covenant));

    gameplay::covenants::CovenantRankDefinition first;
    first.id = 1;
    first.covenantId = 5;
    first.key = "first";
    first.minReputation = 0;
    first.entryRewards.push_back(SafeFlagReward("rank.first"));
    assert(service.RegisterRank(first));

    gameplay::covenants::CovenantRankDefinition second;
    second.id = 2;
    second.covenantId = 5;
    second.key = "second";
    second.minReputation = 100;
    second.entryRewards.push_back(GoldReward("rank.second", 20));
    assert(service.RegisterRank(second));
    assert(service.ValidateDefinitions());
    assert(service.ValidateRewards(rewardService));

    gameplay::covenants::CovenantMembershipState membership;
    membership.covenantId = 5;
    membership.rankId = 1;
    membership.active = true;
    repository.memberships["Alice"].push_back(membership);

    assert(processor.ProcessPlayer("Alice"));
    assert(gateway.grants == 2);
    assert(processor.ProcessPlayer("Alice"));
    assert(gateway.grants == 2);

    repository.memberships["Alice"][0].rankId = 2;
    repository.memberships["Alice"][0].reputation = 100;
    assert(processor.ProcessPlayer("Alice"));
    assert(gateway.grants == 3);
    assert(processor.ProcessPlayer("Alice"));
    assert(gateway.grants == 3);

    repository.memberships["Bob"].push_back(membership);
    repository.failNextMark = true;
    assert(!processor.ProcessPlayer("Bob"));
    const int afterAmbiguousGrant = gateway.grants;
    assert(!processor.ProcessPlayer("Bob"));
    assert(gateway.grants == afterAmbiguousGrant);

    std::vector<gameplay::covenants::CovenantEffectReceiptEntry> pending;
    assert(processor.ListPendingEffects("Bob", pending));
    assert(pending.size() == 1u);
    assert(pending[0].effectKey == "join.gold");
    assert(processor.AcknowledgePendingEffect("Bob", 5, "join.gold"));
    assert(processor.ProcessPlayer("Bob"));
    assert(gateway.grants == afterAmbiguousGrant + 1);

    // Retry-safe reward can be replayed from Pending.
    repository.memberships["Carol"].push_back(membership);
    repository.receipts[repository.Key("Carol", 5, "join.gold")] =
        gameplay::covenants::CovenantEffectReceiptStatus::Applied;
    repository.failNextMark = true;
    assert(!processor.ProcessPlayer("Carol"));
    const int afterSafeAmbiguous = gateway.grants;
    assert(processor.ProcessPlayer("Carol"));
    assert(gateway.grants == afterSafeAmbiguous + 1);

    return 0;
}
