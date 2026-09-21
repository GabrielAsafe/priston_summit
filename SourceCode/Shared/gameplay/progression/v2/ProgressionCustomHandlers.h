#pragma once

#include <map>
#include <string>

#include "gameplay/rewards/v2/Reward.h"
#include "gameplay/conditions/v2/Condition.h"
#include "gameplay/story/v2/StoryTypes.h"

namespace gameplay
{
namespace progression
{
class ICustomRewardHandler
{
public:
    virtual ~ICustomRewardHandler() {}
    virtual bool Validate(const rewards::RewardDefinition& reward) const = 0;
    virtual bool IsRetrySafe(const rewards::RewardDefinition& reward) const = 0;
    virtual rewards::RewardResult Grant(const std::string& playerKey,
                                        const rewards::RewardDefinition& reward,
                                        const std::string& sourceKey) = 0;
};

class ICustomConditionHandler
{
public:
    virtual ~ICustomConditionHandler() {}
    virtual bool Validate(const conditions::ConditionDefinition& condition) const = 0;
    virtual bool Evaluate(const std::string& playerKey,
                          const conditions::ConditionDefinition& condition) const = 0;
};

class ICustomStoryActionHandler
{
public:
    virtual ~ICustomStoryActionHandler() {}
    virtual bool Validate(const story::StoryActionDefinition& action) const = 0;
    virtual bool IsRetrySafe(const story::StoryActionDefinition& action) const = 0;
    virtual bool Execute(const std::string& playerKey,
                         const story::StoryActionDefinition& action,
                         const std::string& sourceKey) = 0;
};

class ProgressionCustomHandlerRegistry
{
public:
    bool RegisterRewardHandler(const std::string& key, ICustomRewardHandler* handler)
    {
        if (key.empty() || !handler || rewardHandlers_.count(key) != 0u)
            return false;
        rewardHandlers_[key] = handler;
        return true;
    }

    bool RegisterActionHandler(const std::string& key, ICustomStoryActionHandler* handler)
    {
        if (key.empty() || !handler || actionHandlers_.count(key) != 0u)
            return false;
        actionHandlers_[key] = handler;
        return true;
    }

    bool RegisterConditionHandler(const std::string& key, ICustomConditionHandler* handler)
    {
        if (key.empty() || !handler || conditionHandlers_.count(key) != 0u)
            return false;
        conditionHandlers_[key] = handler;
        return true;
    }

    bool ValidateCondition(const conditions::ConditionDefinition& condition) const
    {
        const ConditionHandlerMap::const_iterator it = conditionHandlers_.find(condition.key);
        return condition.type == conditions::ConditionType::Custom &&
               it != conditionHandlers_.end() && it->second->Validate(condition);
    }

    bool EvaluateCondition(const std::string& playerKey,
                           const conditions::ConditionDefinition& condition) const
    {
        const ConditionHandlerMap::const_iterator it = conditionHandlers_.find(condition.key);
        return condition.type == conditions::ConditionType::Custom &&
               it != conditionHandlers_.end() && it->second->Validate(condition) &&
               it->second->Evaluate(playerKey, condition);
    }

    bool ValidateReward(const rewards::RewardDefinition& reward) const
    {
        const RewardHandlerMap::const_iterator it = rewardHandlers_.find(reward.key);
        return reward.type == rewards::RewardType::Custom &&
               it != rewardHandlers_.end() && it->second->Validate(reward);
    }

    bool IsRewardRetrySafe(const rewards::RewardDefinition& reward) const
    {
        const RewardHandlerMap::const_iterator it = rewardHandlers_.find(reward.key);
        return reward.type == rewards::RewardType::Custom &&
               it != rewardHandlers_.end() && it->second->IsRetrySafe(reward);
    }

    rewards::RewardResult GrantReward(const std::string& playerKey,
                                      const rewards::RewardDefinition& reward,
                                      const std::string& sourceKey)
    {
        const RewardHandlerMap::iterator it = rewardHandlers_.find(reward.key);
        if (reward.type != rewards::RewardType::Custom || it == rewardHandlers_.end())
            return rewards::RewardResult::Unsupported;
        if (!it->second->Validate(reward))
            return rewards::RewardResult::Rejected;
        return it->second->Grant(playerKey, reward, sourceKey);
    }

    bool ValidateAction(const story::StoryActionDefinition& action) const
    {
        const ActionHandlerMap::const_iterator it = actionHandlers_.find(action.key);
        return action.type == story::StoryActionType::Custom &&
               it != actionHandlers_.end() && it->second->Validate(action);
    }

    bool IsActionRetrySafe(const story::StoryActionDefinition& action) const
    {
        const ActionHandlerMap::const_iterator it = actionHandlers_.find(action.key);
        return action.type == story::StoryActionType::Custom &&
               it != actionHandlers_.end() && it->second->IsRetrySafe(action);
    }

    bool ExecuteAction(const std::string& playerKey,
                       const story::StoryActionDefinition& action,
                       const std::string& sourceKey)
    {
        const ActionHandlerMap::iterator it = actionHandlers_.find(action.key);
        if (action.type != story::StoryActionType::Custom || it == actionHandlers_.end() ||
            !it->second->Validate(action))
            return false;
        return it->second->Execute(playerKey, action, sourceKey);
    }

private:
    typedef std::map<std::string, ICustomRewardHandler*> RewardHandlerMap;
    typedef std::map<std::string, ICustomStoryActionHandler*> ActionHandlerMap;
    typedef std::map<std::string, ICustomConditionHandler*> ConditionHandlerMap;

    RewardHandlerMap rewardHandlers_;
    ActionHandlerMap actionHandlers_;
    ConditionHandlerMap conditionHandlers_;
};
}
}
