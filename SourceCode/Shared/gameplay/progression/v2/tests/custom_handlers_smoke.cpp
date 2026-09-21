#include <cassert>
#include <string>

#include "gameplay/progression/v2/ProgressionCustomHandlers.h"

namespace
{
class RewardHandler : public gameplay::progression::ICustomRewardHandler
{
public:
    RewardHandler() : grants(0) {}
    bool Validate(const gameplay::rewards::RewardDefinition& reward) const
    { return reward.key == "custom.safe" && reward.amount == 7; }
    bool IsRetrySafe(const gameplay::rewards::RewardDefinition&) const { return true; }
    gameplay::rewards::RewardResult Grant(const std::string& playerKey,
                                           const gameplay::rewards::RewardDefinition&,
                                           const std::string& sourceKey)
    {
        if (playerKey.empty() || sourceKey.empty()) return gameplay::rewards::RewardResult::Failed;
        ++grants; return gameplay::rewards::RewardResult::Granted;
    }
    int grants;
};

class ConditionHandler : public gameplay::progression::ICustomConditionHandler
{
public:
    bool Validate(const gameplay::conditions::ConditionDefinition& condition) const
    { return condition.key == "custom.condition" && condition.value == 42; }
    bool Evaluate(const std::string& playerKey,
                  const gameplay::conditions::ConditionDefinition& condition) const
    { return playerKey == "Alice" && Validate(condition); }
};

class ActionHandler : public gameplay::progression::ICustomStoryActionHandler
{
public:
    ActionHandler() : executions(0) {}
    bool Validate(const gameplay::story::StoryActionDefinition& action) const
    { return action.key == "custom.action" && action.value == 3; }
    bool IsRetrySafe(const gameplay::story::StoryActionDefinition&) const { return false; }
    bool Execute(const std::string& playerKey,
                 const gameplay::story::StoryActionDefinition&,
                 const std::string& sourceKey)
    {
        if (playerKey.empty() || sourceKey.empty()) return false;
        ++executions; return true;
    }
    int executions;
};
}

int main()
{
    using namespace gameplay;
    progression::ProgressionCustomHandlerRegistry registry;
    RewardHandler rewardHandler;
    ActionHandler actionHandler;
    ConditionHandler conditionHandler;
    assert(registry.RegisterRewardHandler("custom.safe", &rewardHandler));
    assert(!registry.RegisterRewardHandler("custom.safe", &rewardHandler));
    assert(registry.RegisterActionHandler("custom.action", &actionHandler));
    assert(!registry.RegisterActionHandler("custom.action", &actionHandler));
    assert(registry.RegisterConditionHandler("custom.condition", &conditionHandler));
    assert(!registry.RegisterConditionHandler("custom.condition", &conditionHandler));

    conditions::ConditionDefinition condition;
    condition.type = conditions::ConditionType::Custom;
    condition.key = "custom.condition";
    condition.value = 42;
    assert(registry.ValidateCondition(condition));
    assert(registry.EvaluateCondition("Alice", condition));
    assert(!registry.EvaluateCondition("Bob", condition));
    condition.key = "unregistered";
    assert(!registry.ValidateCondition(condition));

    rewards::RewardDefinition reward;
    reward.type = rewards::RewardType::Custom;
    reward.key = "custom.safe";
    reward.amount = 7;
    assert(registry.ValidateReward(reward));
    assert(registry.IsRewardRetrySafe(reward));
    assert(registry.GrantReward("Alice", reward, "story.custom.0") == rewards::RewardResult::Granted);
    assert(rewardHandler.grants == 1);
    reward.key = "unregistered";
    assert(!registry.ValidateReward(reward));
    assert(registry.GrantReward("Alice", reward, "story.custom.1") == rewards::RewardResult::Unsupported);

    story::StoryActionDefinition action;
    action.type = story::StoryActionType::Custom;
    action.key = "custom.action";
    action.value = 3;
    assert(registry.ValidateAction(action));
    assert(!registry.IsActionRetrySafe(action));
    assert(registry.ExecuteAction("Alice", action, "story.action.0"));
    assert(actionHandler.executions == 1);
    action.key = "unregistered";
    assert(!registry.ValidateAction(action));
    assert(!registry.ExecuteAction("Alice", action, "story.action.1"));
    return 0;
}
