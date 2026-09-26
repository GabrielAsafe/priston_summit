#include "systems/gameplay/progression/v2/ServerProgressionConditionContext.h"

namespace gameplay
{
namespace progression
{
ServerProgressionConditionContext::ServerProgressionConditionContext(
    const std::string& playerKey,
    int playerLevel,
    int legacyJobCode,
    story::IStoryRepository& storyRepository,
    covenants::CovenantService& covenantService,
    reputation::ReputationService& reputationService,
    const IQuestStateReader* questReader,
    const IProfessionLevelReader* professionReader,
    const ICustomConditionReader* customReader)
    : playerKey_(playerKey),
      playerLevel_(playerLevel),
      legacyJobCode_(legacyJobCode),
      storyRepository_(storyRepository),
      covenantService_(covenantService),
      reputationService_(reputationService),
      questReader_(questReader),
      professionReader_(professionReader),
      customReader_(customReader)
{
}

int ServerProgressionConditionContext::PlayerLevel() const
{
    return playerLevel_;
}

int ServerProgressionConditionContext::LegacyJobCode() const
{
    return legacyJobCode_;
}

bool ServerProgressionConditionContext::HasStoryFlag(const std::string& key) const
{
    int value = 0;
    return !playerKey_.empty() && !key.empty() &&
           storyRepository_.GetPlayerFlag(playerKey_, key, value) && value != 0;
}

bool ServerProgressionConditionContext::HasWorldStoryFlag(const std::string& key) const
{
    int value = 0;
    return !key.empty() && storyRepository_.GetWorldFlag(key, value) && value != 0;
}

int ServerProgressionConditionContext::StoryNodeState(int storyId, int nodeId) const
{
    story::StoryNodeStatus state = story::StoryNodeStatus::Locked;
    if (playerKey_.empty() || storyId <= 0 || nodeId <= 0 ||
        !storyRepository_.GetNodeState(playerKey_, storyId, nodeId, state))
        return kProgressionUnavailableValue;
    return static_cast<int>(state);
}

int ServerProgressionConditionContext::QuestState(int questId) const
{
    return questReader_ && questId > 0 ? questReader_->QuestState(playerKey_, questId)
                                        : kProgressionUnavailableValue;
}

bool ServerProgressionConditionContext::HasCovenantMembership(int covenantId) const
{
    covenants::CovenantMembershipState membership;
    return covenantId > 0 && covenantService_.GetMembership(playerKey_, covenantId, membership);
}

int ServerProgressionConditionContext::CovenantRank(int covenantId) const
{
    covenants::CovenantMembershipState membership;
    return covenantId > 0 && covenantService_.GetMembership(playerKey_, covenantId, membership)
               ? membership.rankId
               : kProgressionUnavailableValue;
}

int ServerProgressionConditionContext::CovenantReputation(int covenantId) const
{
    covenants::CovenantMembershipState membership;
    return covenantId > 0 && covenantService_.GetMembership(playerKey_, covenantId, membership)
               ? membership.reputation
               : kProgressionUnavailableValue;
}

int ServerProgressionConditionContext::ProfessionLevel(const std::string& professionKey) const
{
    return professionReader_ && !professionKey.empty()
               ? professionReader_->ProfessionLevel(playerKey_, professionKey)
               : kProgressionUnavailableValue;
}

int ServerProgressionConditionContext::Reputation(const std::string& reputationKey) const
{
    int value = 0;
    return !playerKey_.empty() && !reputationKey.empty() &&
                   reputationService_.GetValue(playerKey_, reputationKey, value)
               ? value
               : kProgressionUnavailableValue;
}

bool ServerProgressionConditionContext::EvaluateCustomCondition(
    const conditions::ConditionDefinition& definition) const
{
    return customReader_ && customReader_->Evaluate(playerKey_, definition);
}
}
}
