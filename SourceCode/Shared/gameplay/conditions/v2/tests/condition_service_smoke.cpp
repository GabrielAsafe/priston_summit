#include <cassert>
#include <map>
#include <string>

#include "gameplay/conditions/v2/Condition.h"
#include "gameplay/modifiers/v2/GameplayModifier.h"
#include "gameplay/rewards/v2/Reward.h"

namespace
{
class FakeContext : public gameplay::conditions::ConditionContext
{
public:
    FakeContext() : level(10), job(3), covenantRank(2), covenantRep(120) {}

    int PlayerLevel() const { return level; }
    int LegacyJobCode() const { return job; }
    bool HasStoryFlag(const std::string& key) const { return key == "intro.done"; }
    bool HasWorldStoryFlag(const std::string& key) const { return key == "world.open"; }
    int StoryNodeState(int storyId, int nodeId) const { return storyId == 1 && nodeId == 2 ? 3 : 0; }
    int QuestState(int questId) const { return questId == 77 ? 2 : 0; }
    bool HasCovenantMembership(int covenantId) const { return covenantId == 5; }
    int CovenantRank(int covenantId) const { return covenantId == 5 ? covenantRank : 0; }
    int CovenantReputation(int covenantId) const { return covenantId == 5 ? covenantRep : 0; }
    int ProfessionLevel(const std::string& key) const { return key == "miner" ? 4 : 0; }
    bool EvaluateCustomCondition(const gameplay::conditions::ConditionDefinition& definition) const
    {
        return definition.key == "custom.ok";
    }

    int level;
    int job;
    int covenantRank;
    int covenantRep;
};

class FakeRewardGateway : public gameplay::rewards::IRewardGateway
{
public:
    FakeRewardGateway() : calls(0) {}

    gameplay::rewards::RewardResult Grant(const std::string& playerKey,
                                          const gameplay::rewards::RewardDefinition& reward,
                                          const std::string& sourceKey)
    {
        ++calls;
        if (playerKey != "Alice" || sourceKey.empty())
            return gameplay::rewards::RewardResult::Rejected;
        if (reward.type == gameplay::rewards::RewardType::Item)
            return gameplay::rewards::RewardResult::Unsupported;
        return gameplay::rewards::RewardResult::Granted;
    }

    int calls;
};
}

int main()
{
    using namespace gameplay::conditions;
    FakeContext context;

    ConditionDefinition level;
    level.type = ConditionType::PlayerLevelAtLeast;
    level.value = 10;
    assert(ConditionService::Validate(level));
    assert(ConditionService::Evaluate(level, context));

    ConditionDefinition invalidQuest;
    invalidQuest.type = ConditionType::QuestState;
    invalidQuest.id = 77;
    invalidQuest.value = 99;
    assert(!ConditionService::Validate(invalidQuest));

    ConditionDefinition invalidCustom;
    invalidCustom.type = ConditionType::Custom;
    assert(!ConditionService::Validate(invalidCustom));

    ConditionDefinition covenant;
    covenant.type = ConditionType::CovenantReputationAtLeast;
    covenant.id = 5;
    covenant.value = 100;
    assert(ConditionService::Evaluate(covenant, context));

    ConditionDefinition flag;
    flag.type = ConditionType::HasStoryFlag;
    flag.key = "intro.done";
    assert(ConditionService::Evaluate(flag, context));

    ConditionDefinition worldFlag;
    worldFlag.type = ConditionType::HasWorldStoryFlag;
    worldFlag.key = "world.open";
    assert(ConditionService::Evaluate(worldFlag, context));

    std::vector<ConditionDefinition> all;
    all.push_back(level);
    all.push_back(covenant);
    all.push_back(flag);
    assert(ConditionService::EvaluateAll(all, context));

    gameplay::modifiers::ActiveModifierSet modifiers;
    gameplay::modifiers::GameplayModifier bonus;
    bonus.statKey = "attack";
    bonus.value = 5.0;
    std::vector<gameplay::modifiers::GameplayModifier> rankBonus(1, bonus);
    modifiers.ReplaceSource("covenant.test.rank.2", rankBonus);
    assert(modifiers.ForStat("attack").size() == 1u);
    modifiers.RemoveSource("covenant.test.rank.2");
    assert(modifiers.All().empty());

    FakeRewardGateway gateway;
    gameplay::rewards::RewardService rewardService(gateway);
    gameplay::rewards::RewardDefinition reward;
    reward.type = gameplay::rewards::RewardType::Gold;
    reward.amount = 10;
    assert(rewardService.Grant("Alice", reward, "story.test") == gameplay::rewards::RewardResult::Granted);
    reward.type = gameplay::rewards::RewardType::Item;
    assert(rewardService.Grant("Alice", reward, "story.test") == gameplay::rewards::RewardResult::Unsupported);
    assert(gateway.calls == 2);
    return 0;
}
