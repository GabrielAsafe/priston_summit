#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "systems/gameplay/progression/v2/ServerProgressionConditionContext.h"

namespace
{
class CovenantRepo : public gameplay::covenants::ICovenantRepository
{
public:
    bool LoadAll(const std::string& playerKey,
                 std::vector<gameplay::covenants::CovenantMembershipState>& memberships)
    {
        memberships = data[playerKey];
        return true;
    }
    bool Save(const std::string&, const gameplay::covenants::CovenantMembershipState&) { return false; }
    gameplay::covenants::CovenantReputationSaveResult SaveReputationChange(
        const std::string&, const gameplay::covenants::CovenantMembershipState&, int,
        const std::string&)
    {
        return gameplay::covenants::CovenantReputationSaveResult::Failed;
    }
    std::map<std::string, std::vector<gameplay::covenants::CovenantMembershipState> > data;
};


class ReputationRepo : public gameplay::reputation::IReputationRepository
{
public:
    gameplay::reputation::ReputationLoadResult LoadValue(
        const std::string& playerKey, const std::string& reputationKey, int& value)
    {
        const std::string key = playerKey + ":" + reputationKey;
        if (values.count(key) == 0u)
            return gameplay::reputation::ReputationLoadResult::NotFound;
        value = values[key];
        return gameplay::reputation::ReputationLoadResult::Found;
    }
    gameplay::reputation::ReputationChangeResult ApplyChange(
        const std::string&, const std::string&, int, int, int, const std::string&)
    {
        return gameplay::reputation::ReputationChangeResult::Failed;
    }
    std::map<std::string, int> values;
};

class StoryRepo : public gameplay::story::IStoryRepository
{
public:
    bool LoadProgress(const std::string&, int, gameplay::story::StoryProgressState&) { return false; }
    bool SaveProgress(const std::string&, const gameplay::story::StoryProgressState&) { return false; }
    bool GetNodeState(const std::string& playerKey, int storyId, int nodeId,
                      gameplay::story::StoryNodeStatus& state)
    {
        const std::string key = playerKey + ":" + std::to_string(storyId) + ":" + std::to_string(nodeId);
        if (nodes.count(key) == 0u) return false;
        state = nodes[key]; return true;
    }
    bool SetNodeState(const std::string&, int, int, gameplay::story::StoryNodeStatus) { return false; }
    bool GetPlayerFlag(const std::string& playerKey, const std::string& flagKey, int& value)
    {
        const std::string key = playerKey + ":" + flagKey;
        if (flags.count(key) == 0u) return false;
        value = flags[key]; return true;
    }
    bool SetPlayerFlag(const std::string&, const std::string&, int) { return false; }
    bool GetWorldFlag(const std::string& flagKey, int& value)
    {
        if (worldFlags.count(flagKey) == 0u) return false;
        value = worldFlags[flagKey]; return true;
    }
    bool SetWorldFlag(const std::string& flagKey, int value)
    { worldFlags[flagKey] = value; return true; }
    gameplay::story::StoryEffectClaimResult ClaimEffect(const std::string&, int, int,
                                                          const std::string&)
    {
        return gameplay::story::StoryEffectClaimResult::Failed;
    }
    bool MarkEffectApplied(const std::string&, int, int, const std::string&) { return false; }

    std::map<std::string, int> flags;
    std::map<std::string, int> worldFlags;
    std::map<std::string, gameplay::story::StoryNodeStatus> nodes;
};

class QuestReader : public gameplay::progression::IQuestStateReader
{
public:
    int QuestState(const std::string& playerKey, int questId) const
    {
        return playerKey == "Alice" && questId == 77 ? 2 : 0;
    }
};

class ProfessionReader : public gameplay::progression::IProfessionLevelReader
{
public:
    int ProfessionLevel(const std::string& playerKey, const std::string& key) const
    {
        return playerKey == "Alice" && key == "miner" ? 4 : 0;
    }
};
}

int main()
{
    using namespace gameplay;
    CovenantRepo covenantRepo;
    covenants::CovenantMembershipState membership;
    membership.covenantId = 5;
    membership.reputation = 140;
    membership.rankId = 20;
    membership.active = true;
    covenantRepo.data["Alice"].push_back(membership);
    covenants::CovenantService covenantService(covenantRepo);

    ReputationRepo reputationRepo;
    reputationRepo.values["Alice:faction.alpha"] = 55;
    reputation::ReputationService reputationService(reputationRepo);
    reputation::ReputationDefinition reputationDefinition;
    reputationDefinition.key = "faction.alpha";
    assert(reputationService.RegisterDefinition(reputationDefinition));

    StoryRepo storyRepo;
    storyRepo.flags["Alice:intro.done"] = 1;
    storyRepo.worldFlags["world.ready"] = 1;
    storyRepo.nodes["Alice:3:9"] = story::StoryNodeStatus::Completed;

    QuestReader quests;
    ProfessionReader professions;
    progression::ServerProgressionConditionContext context(
        "Alice", 42, 3, storyRepo, covenantService, reputationService, &quests, &professions);

    conditions::ConditionDefinition condition;
    condition.type = conditions::ConditionType::PlayerLevelAtLeast;
    condition.value = 40;
    assert(conditions::ConditionService::Evaluate(condition, context));

    condition.type = conditions::ConditionType::ClassIs;
    condition.value = 3;
    assert(conditions::ConditionService::Evaluate(condition, context));

    condition.type = conditions::ConditionType::HasStoryFlag;
    condition.key = "intro.done";
    assert(conditions::ConditionService::Evaluate(condition, context));

    condition.type = conditions::ConditionType::HasWorldStoryFlag;
    condition.key = "world.ready";
    assert(conditions::ConditionService::Evaluate(condition, context));

    condition.type = conditions::ConditionType::StoryNodeState;
    condition.id = 3;
    condition.auxiliaryId = 9;
    condition.value = static_cast<int>(story::StoryNodeStatus::Completed);
    assert(conditions::ConditionService::Evaluate(condition, context));

    condition.type = conditions::ConditionType::QuestState;
    condition.id = 77;
    condition.value = 2;
    assert(conditions::ConditionService::Evaluate(condition, context));

    condition.type = conditions::ConditionType::CovenantMembership;
    condition.id = 5;
    assert(conditions::ConditionService::Evaluate(condition, context));

    condition.type = conditions::ConditionType::CovenantRankAtLeast;
    condition.id = 5;
    condition.value = 20;
    assert(conditions::ConditionService::Evaluate(condition, context));

    condition.type = conditions::ConditionType::CovenantReputationAtLeast;
    condition.id = 5;
    condition.value = 100;
    assert(conditions::ConditionService::Evaluate(condition, context));

    condition.type = conditions::ConditionType::ProfessionLevelAtLeast;
    condition.key = "miner";
    condition.value = 4;
    assert(conditions::ConditionService::Evaluate(condition, context));

    condition.type = conditions::ConditionType::ReputationAtLeast;
    condition.key = "faction.alpha";
    condition.value = 50;
    assert(conditions::ConditionService::Evaluate(condition, context));
    condition.value = 56;
    assert(!conditions::ConditionService::Evaluate(condition, context));

    progression::ServerProgressionConditionContext unavailableContext(
        "Alice", 42, 3, storyRepo, covenantService, reputationService);
    assert(unavailableContext.QuestState(77) == progression::kProgressionUnavailableValue);
    assert(unavailableContext.ProfessionLevel("miner") == progression::kProgressionUnavailableValue);
    assert(unavailableContext.StoryNodeState(99, 99) == progression::kProgressionUnavailableValue);
    assert(unavailableContext.CovenantRank(999) == progression::kProgressionUnavailableValue);
    assert(unavailableContext.CovenantReputation(999) == progression::kProgressionUnavailableValue);
    assert(unavailableContext.Reputation("unknown") == progression::kProgressionUnavailableValue);
    return 0;
}
