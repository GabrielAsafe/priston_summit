#pragma once

#include <string>

#include "gameplay/progression/v2/ProgressionGateways.h"
#include "gameplay/progression/v2/ProgressionCustomHandlers.h"

namespace gameplay
{
namespace progression
{
class LegacyProgressionRewardGateway : public rewards::IRewardGateway
{
public:
    LegacyProgressionRewardGateway(story::IStoryRepository& storyRepository,
                                   covenants::CovenantService& covenantService,
                                   reputation::ReputationService& reputationService,
                                   ProgressionCustomHandlerRegistry& customHandlers);

    bool Supports(rewards::RewardType type) const;
    bool Validate(const rewards::RewardDefinition& reward) const;
    bool IsRetrySafe(rewards::RewardType type) const;
    bool IsRetrySafe(const rewards::RewardDefinition& reward) const;
    rewards::RewardResult Grant(const std::string& playerKey,
                                const rewards::RewardDefinition& reward,
                                const std::string& sourceKey);

    void SetItemCatalogReady(bool ready);

private:
    unsigned long ResolveItemCode(const std::string& shortCode) const;

    ProgressionRewardGateway core_;
    ProgressionCustomHandlerRegistry& customHandlers_;
    bool itemCatalogReady_;
};

class LegacyProgressionStoryCommandGateway : public story::IStoryCommandGateway
{
public:
    LegacyProgressionStoryCommandGateway(covenants::CovenantService& covenantService,
                                         ProgressionCustomHandlerRegistry& customHandlers);

    bool Supports(story::StoryActionType type) const;
    bool ValidateAction(const story::StoryActionDefinition& action) const;
    bool IsRetrySafe(story::StoryActionType type) const;
    bool IsRetrySafe(const story::StoryActionDefinition& action) const;
    bool StartQuest(const std::string& playerKey, int questId);
    bool AddCovenantReputation(const std::string& playerKey,
                               int covenantId,
                               int amount,
                               const std::string& sourceKey);
    bool ExecuteCustomAction(const std::string& playerKey,
                             const story::StoryActionDefinition& action,
                             const std::string& sourceKey);

public:
    struct QuestDefinition
    {
        int kind;
        int type;
        int partyFlag;

        QuestDefinition() : kind(0), type(0), partyFlag(0) {}
    };

private:
    bool ResolveQuestDefinition(int questId, QuestDefinition& definition) const;

    ProgressionStoryCommandGateway core_;
    ProgressionCustomHandlerRegistry& customHandlers_;
};
}
}
