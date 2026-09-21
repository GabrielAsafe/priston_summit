#include "gameplay/progression/v2/ProgressionGateways.h"

namespace gameplay
{
namespace progression
{
bool ProgressionRewardGateway::Supports(rewards::RewardType type) const
{
    return type == rewards::RewardType::StoryFlag ||
           type == rewards::RewardType::Reputation ||
           type == rewards::RewardType::CovenantReputation;
}

bool ProgressionRewardGateway::Validate(const rewards::RewardDefinition& reward) const
{
    switch (reward.type)
    {
    case rewards::RewardType::StoryFlag:
        return !reward.key.empty();
    case rewards::RewardType::Reputation:
        return !reward.key.empty() && reward.key.size() <= 255u &&
               reward.id == 0 && reward.amount != 0 &&
               reputationService_.FindDefinition(reward.key) != 0;
    case rewards::RewardType::CovenantReputation:
        return reward.id > 0 && reward.amount != 0;
    case rewards::RewardType::Item:
    case rewards::RewardType::Gold:
    case rewards::RewardType::Experience:
    case rewards::RewardType::Custom:
        return false;
    }
    return false;
}

bool ProgressionRewardGateway::IsRetrySafe(rewards::RewardType type) const
{
    // StoryFlag is an idempotent SET. Generic and Covenant reputation are
    // protected by their authoritative source-key ledgers.
    return type == rewards::RewardType::StoryFlag ||
           type == rewards::RewardType::Reputation ||
           type == rewards::RewardType::CovenantReputation;
}

rewards::RewardResult ProgressionRewardGateway::Grant(const std::string& playerKey,
                                                       const rewards::RewardDefinition& reward,
                                                       const std::string& sourceKey)
{
    if (playerKey.empty() || sourceKey.empty())
        return rewards::RewardResult::Rejected;

    switch (reward.type)
    {
    case rewards::RewardType::StoryFlag:
        if (reward.key.empty())
            return rewards::RewardResult::Rejected;
        return storyRepository_.SetPlayerFlag(playerKey, reward.key, reward.amount)
                   ? rewards::RewardResult::Granted
                   : rewards::RewardResult::Failed;
    case rewards::RewardType::Reputation:
        if (reward.key.empty() || reward.id != 0 || reward.amount == 0)
            return rewards::RewardResult::Rejected;
        return reputationService_.Add(playerKey, reward.key, reward.amount, sourceKey)
                   ? rewards::RewardResult::Granted
                   : rewards::RewardResult::Failed;
    case rewards::RewardType::CovenantReputation:
        if (reward.id <= 0)
            return rewards::RewardResult::Rejected;
        return covenantService_.AddReputation(playerKey, reward.id, reward.amount, sourceKey)
                   ? rewards::RewardResult::Granted
                   : rewards::RewardResult::Failed;
    case rewards::RewardType::Item:
    case rewards::RewardType::Gold:
    case rewards::RewardType::Experience:
    case rewards::RewardType::Custom:
        return rewards::RewardResult::Unsupported;
    }
    return rewards::RewardResult::Unsupported;
}

bool ProgressionStoryCommandGateway::Supports(story::StoryActionType type) const
{
    return type == story::StoryActionType::AddCovenantReputation;
}

bool ProgressionStoryCommandGateway::ValidateAction(
    const story::StoryActionDefinition& action) const
{
    return action.type == story::StoryActionType::AddCovenantReputation &&
           action.id > 0 && action.value != 0;
}

bool ProgressionStoryCommandGateway::IsRetrySafe(story::StoryActionType type) const
{
    return type == story::StoryActionType::AddCovenantReputation;
}

bool ProgressionStoryCommandGateway::StartQuest(const std::string&, int)
{
    return false;
}

bool ProgressionStoryCommandGateway::AddCovenantReputation(const std::string& playerKey,
                                                            int covenantId,
                                                            int amount,
                                                            const std::string& sourceKey)
{
    return covenantService_.AddReputation(playerKey, covenantId, amount, sourceKey);
}

bool ProgressionStoryCommandGateway::ExecuteCustomAction(const std::string&,
                                                          const story::StoryActionDefinition&,
                                                          const std::string&)
{
    return false;
}
}
}
