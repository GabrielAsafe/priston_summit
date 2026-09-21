#pragma once

#include "gameplay/covenants/v2/CovenantService.h"
#include "gameplay/rewards/v2/Reward.h"
#include "gameplay/reputation/v2/ReputationService.h"
#include "gameplay/story/v2/IStoryRepository.h"
#include "gameplay/story/v2/StoryService.h"

namespace gameplay
{
namespace progression
{
class ProgressionRewardGateway : public rewards::IRewardGateway
{
public:
    ProgressionRewardGateway(story::IStoryRepository& storyRepository,
                             covenants::CovenantService& covenantService,
                             reputation::ReputationService& reputationService)
        : storyRepository_(storyRepository), covenantService_(covenantService),
          reputationService_(reputationService)
    {
    }

    bool Supports(rewards::RewardType type) const;
    bool Validate(const rewards::RewardDefinition& reward) const;
    bool IsRetrySafe(rewards::RewardType type) const;
    rewards::RewardResult Grant(const std::string& playerKey,
                                const rewards::RewardDefinition& reward,
                                const std::string& sourceKey);

private:
    story::IStoryRepository& storyRepository_;
    covenants::CovenantService& covenantService_;
    reputation::ReputationService& reputationService_;
};

class ProgressionStoryCommandGateway : public story::IStoryCommandGateway
{
public:
    explicit ProgressionStoryCommandGateway(covenants::CovenantService& covenantService)
        : covenantService_(covenantService)
    {
    }

    bool Supports(story::StoryActionType type) const;
    bool ValidateAction(const story::StoryActionDefinition& action) const;
    bool IsRetrySafe(story::StoryActionType type) const;
    bool StartQuest(const std::string& playerKey, int questId);
    bool AddCovenantReputation(const std::string& playerKey,
                               int covenantId,
                               int amount,
                               const std::string& sourceKey);
    bool ExecuteCustomAction(const std::string& playerKey,
                             const story::StoryActionDefinition& action,
                             const std::string& sourceKey);

private:
    covenants::CovenantService& covenantService_;
};
}
}
