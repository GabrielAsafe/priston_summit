#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "gameplay/progression/v2/ProgressionGateways.h"

namespace
{
class FakeCovenantRepo : public gameplay::covenants::ICovenantRepository
{
public:
    bool LoadAll(const std::string& playerKey,
                 std::vector<gameplay::covenants::CovenantMembershipState>& memberships)
    {
        memberships = data[playerKey];
        return true;
    }

    bool Save(const std::string& playerKey,
              const gameplay::covenants::CovenantMembershipState& membership)
    {
        std::vector<gameplay::covenants::CovenantMembershipState>& list = data[playerKey];
        for (std::vector<gameplay::covenants::CovenantMembershipState>::iterator it = list.begin();
             it != list.end(); ++it)
        {
            if (it->covenantId == membership.covenantId)
            {
                *it = membership;
                return true;
            }
        }
        list.push_back(membership);
        return true;
    }

    gameplay::covenants::CovenantReputationSaveResult SaveReputationChange(
        const std::string& playerKey,
        const gameplay::covenants::CovenantMembershipState& membership,
        int,
        const std::string& sourceKey)
    {
        const std::string ledgerKey = playerKey + ":" +
                                      std::to_string(membership.covenantId) + ":" + sourceKey;
        if (ledger.count(ledgerKey) != 0u)
            return gameplay::covenants::CovenantReputationSaveResult::AlreadyApplied;
        if (!Save(playerKey, membership))
            return gameplay::covenants::CovenantReputationSaveResult::Failed;
        ledger[ledgerKey] = true;
        return gameplay::covenants::CovenantReputationSaveResult::Applied;
    }

    std::map<std::string, std::vector<gameplay::covenants::CovenantMembershipState> > data;
    std::map<std::string, bool> ledger;
};


class FakeReputationRepo : public gameplay::reputation::IReputationRepository
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
        const std::string& playerKey, const std::string& reputationKey,
        int expectedValue, int newValue, int delta, const std::string& sourceKey)
    {
        const std::string key = playerKey + ":" + reputationKey;
        const std::string source = key + ":" + sourceKey;
        if (ledger.count(source) != 0u)
            return ledger[source] == delta
                       ? gameplay::reputation::ReputationChangeResult::AlreadyApplied
                       : gameplay::reputation::ReputationChangeResult::Failed;
        const int current = values.count(key) ? values[key] : 0;
        if (current != expectedValue)
            return gameplay::reputation::ReputationChangeResult::Conflict;
        values[key] = newValue;
        ledger[source] = delta;
        return gameplay::reputation::ReputationChangeResult::Applied;
    }

    std::map<std::string, int> values;
    std::map<std::string, int> ledger;
};

class FakeStoryRepo : public gameplay::story::IStoryRepository
{
public:
    FakeStoryRepo() : claims(0) {}

    bool LoadProgress(const std::string& playerKey, int storyId, gameplay::story::StoryProgressState& state)
    {
        const std::string key = playerKey + ":" + std::to_string(storyId);
        std::map<std::string, gameplay::story::StoryProgressState>::const_iterator it = progress.find(key);
        if (it == progress.end()) return false;
        state = it->second;
        return true;
    }

    bool SaveProgress(const std::string& playerKey, const gameplay::story::StoryProgressState& state)
    {
        progress[playerKey + ":" + std::to_string(state.storyId)] = state;
        return true;
    }

    bool GetNodeState(const std::string&, int, int, gameplay::story::StoryNodeStatus&) { return false; }
    bool SetNodeState(const std::string&, int, int, gameplay::story::StoryNodeStatus) { return true; }

    bool GetPlayerFlag(const std::string& playerKey, const std::string& flagKey, int& value)
    {
        const std::string key = playerKey + ":" + flagKey;
        if (flags.count(key) == 0u) return false;
        value = flags[key];
        return true;
    }

    bool SetPlayerFlag(const std::string& playerKey, const std::string& flagKey, int value)
    {
        flags[playerKey + ":" + flagKey] = value;
        return true;
    }

    bool GetWorldFlag(const std::string&, int&) { return false; }
    bool SetWorldFlag(const std::string&, int) { return true; }

    gameplay::story::StoryEffectClaimResult ClaimEffect(const std::string& playerKey,
                                                          int storyId,
                                                          int nodeId,
                                                          const std::string& effectKey)
    {
        ++claims;
        const std::string key = playerKey + ":" + std::to_string(storyId) + ":" +
                                std::to_string(nodeId) + ":" + effectKey;
        std::map<std::string, int>::const_iterator it = effects.find(key);
        if (it == effects.end())
        {
            effects[key] = 0;
            return gameplay::story::StoryEffectClaimResult::Claimed;
        }
        return it->second ? gameplay::story::StoryEffectClaimResult::AlreadyApplied
                          : gameplay::story::StoryEffectClaimResult::Pending;
    }

    bool MarkEffectApplied(const std::string& playerKey,
                           int storyId,
                           int nodeId,
                           const std::string& effectKey)
    {
        const std::string key = playerKey + ":" + std::to_string(storyId) + ":" +
                                std::to_string(nodeId) + ":" + effectKey;
        if (effects.count(key) == 0u) return false;
        effects[key] = 1;
        return true;
    }

    std::map<std::string, gameplay::story::StoryProgressState> progress;
    std::map<std::string, int> flags;
    std::map<std::string, int> effects;
    int claims;
};

class AlwaysContext : public gameplay::conditions::ConditionContext
{
public:
    int PlayerLevel() const { return 99; }
    int LegacyJobCode() const { return 1; }
    bool HasStoryFlag(const std::string&) const { return false; }
    int StoryNodeState(int, int) const { return 0; }
    int QuestState(int) const { return 0; }
    bool HasCovenantMembership(int) const { return false; }
    int CovenantRank(int) const { return 0; }
    int CovenantReputation(int) const { return 0; }
    int ProfessionLevel(const std::string&) const { return 0; }
    bool EvaluateCustomCondition(const gameplay::conditions::ConditionDefinition&) const { return false; }
};
}

int main()
{
    using namespace gameplay;

    FakeCovenantRepo covenantRepo;
    covenants::CovenantService covenantService(covenantRepo);
    covenants::CovenantDefinition covenant;
    covenant.id = 7;
    covenant.key = "guild.test";
    assert(covenantService.RegisterCovenant(covenant));
    covenants::CovenantRankDefinition rank;
    rank.id = 1;
    rank.covenantId = 7;
    rank.key = "member";
    rank.minReputation = 0;
    assert(covenantService.RegisterRank(rank));
    AlwaysContext context;
    assert(covenantService.Join("Alice", 7, context));

    FakeReputationRepo reputationRepo;
    reputation::ReputationService reputationService(reputationRepo);
    reputation::ReputationDefinition genericDefinition;
    genericDefinition.key = "faction.alpha";
    assert(reputationService.RegisterDefinition(genericDefinition));

    FakeStoryRepo storyRepo;
    progression::ProgressionRewardGateway rewardGateway(
        storyRepo, covenantService, reputationService);
    rewards::RewardService rewardService(rewardGateway);

    rewards::RewardDefinition storyFlag;
    storyFlag.type = rewards::RewardType::StoryFlag;
    storyFlag.key = "arc.done";
    storyFlag.amount = 1;
    assert(rewardService.IsRetrySafe(storyFlag));
    assert(rewardService.Grant("Alice", storyFlag, "story.test.reward.0") == rewards::RewardResult::Granted);
    int flagValue = 0;
    assert(storyRepo.GetPlayerFlag("Alice", "arc.done", flagValue) && flagValue == 1);

    rewards::RewardDefinition genericRep;
    genericRep.type = rewards::RewardType::Reputation;
    genericRep.key = "faction.alpha";
    genericRep.amount = 15;
    assert(rewardService.Supports(genericRep));
    assert(rewardService.Validate(genericRep));
    assert(rewardService.IsRetrySafe(genericRep));
    assert(rewardService.Grant("Alice", genericRep, "story.test.reward.generic") ==
           rewards::RewardResult::Granted);
    int genericValue = 0;
    assert(reputationService.GetValue("Alice", "faction.alpha", genericValue));
    assert(genericValue == 15);
    assert(rewardService.Grant("Alice", genericRep, "story.test.reward.generic") ==
           rewards::RewardResult::Granted);
    assert(reputationService.GetValue("Alice", "faction.alpha", genericValue));
    assert(genericValue == 15);

    rewards::RewardDefinition unknownRep = genericRep;
    unknownRep.key = "faction.unknown";
    assert(!rewardService.Validate(unknownRep));

    rewards::RewardDefinition covenantRep;
    covenantRep.type = rewards::RewardType::CovenantReputation;
    covenantRep.id = 7;
    covenantRep.amount = 25;
    assert(rewardService.IsRetrySafe(covenantRep));
    assert(rewardService.Grant("Alice", covenantRep, "story.test.reward.1") == rewards::RewardResult::Granted);
    covenants::CovenantMembershipState membership;
    assert(covenantService.GetMembership("Alice", 7, membership));
    assert(membership.reputation == 25);
    // Replaying the same reward source is acknowledged without applying the delta twice.
    assert(rewardService.Grant("Alice", covenantRep, "story.test.reward.1") == rewards::RewardResult::Granted);
    assert(covenantService.GetMembership("Alice", 7, membership));
    assert(membership.reputation == 25);

    rewards::RewardDefinition zeroRep = covenantRep;
    zeroRep.amount = 0;
    assert(!rewardService.Validate(zeroRep));
    assert(rewardService.Grant("Alice", zeroRep, "story.test.reward.zero") ==
           rewards::RewardResult::Rejected);

    rewards::RewardDefinition gold;
    gold.type = rewards::RewardType::Gold;
    gold.amount = 100;
    assert(!rewardService.Supports(gold));
    assert(!rewardService.IsRetrySafe(gold));
    assert(rewardService.Grant("Alice", gold, "story.test.reward.2") == rewards::RewardResult::Unsupported);

    // GrantAll preflights all reward types, so no partial grant occurs before an unsupported reward.
    std::vector<rewards::RewardDefinition> mixedRewards;
    rewards::RewardDefinition secondFlag = storyFlag;
    secondFlag.key = "should.not.be.set";
    mixedRewards.push_back(secondFlag);
    mixedRewards.push_back(gold);
    assert(rewardService.GrantAll("Alice", mixedRewards, "story.test.mixed") == rewards::RewardResult::Unsupported);
    assert(!storyRepo.GetPlayerFlag("Alice", "should.not.be.set", flagValue));

    progression::ProgressionStoryCommandGateway commandGateway(covenantService);
    assert(commandGateway.Supports(story::StoryActionType::AddCovenantReputation));
    assert(commandGateway.IsRetrySafe(story::StoryActionType::AddCovenantReputation));
    assert(!commandGateway.Supports(story::StoryActionType::StartQuest));
    assert(!commandGateway.IsRetrySafe(story::StoryActionType::StartQuest));
    story::StoryActionDefinition zeroRepAction;
    zeroRepAction.type = story::StoryActionType::AddCovenantReputation;
    zeroRepAction.id = 7;
    zeroRepAction.value = 0;
    assert(!commandGateway.ValidateAction(zeroRepAction));
    assert(commandGateway.AddCovenantReputation("Alice", 7, 5, "story.test.action.0"));
    assert(covenantService.GetMembership("Alice", 7, membership));
    assert(membership.reputation == 30);

    // Unsupported command actions are rejected before a durable effect receipt is claimed.
    story::StoryService storyService(storyRepo, commandGateway, rewardService);
    story::StoryDefinition definition;
    definition.id = 9;
    definition.key = "story.unsupported.quest";
    definition.initialChapterId = 1;
    definition.entryNodeId = 90;
    assert(storyService.RegisterStory(definition));
    story::StoryChapterDefinition chapter;
    chapter.id = 1;
    chapter.storyId = 9;
    chapter.key = "chapter.unsupported.quest";
    assert(storyService.RegisterChapter(chapter));
    story::StoryNodeDefinition node;
    node.id = 90;
    node.chapterId = 1;
    node.key = "quest.start";
    story::StoryActionDefinition startQuest;
    startQuest.type = story::StoryActionType::StartQuest;
    startQuest.id = 1234;
    node.onEnterActions.push_back(startQuest);
    assert(storyService.RegisterNode(9, node));
    const int claimsBefore = storyRepo.claims;
    assert(!storyService.Start("Bob", 9, context));
    assert(storyRepo.claims == claimsBefore);

    // Unsupported completion rewards are also rejected before any reward receipt is claimed.
    story::StoryService rewardStory(storyRepo, commandGateway, rewardService);
    story::StoryDefinition rewardDefinition;
    rewardDefinition.id = 10;
    rewardDefinition.key = "story.unsupported.reward";
    rewardDefinition.initialChapterId = 1;
    rewardDefinition.entryNodeId = 100;
    assert(rewardStory.RegisterStory(rewardDefinition));
    story::StoryChapterDefinition rewardChapter;
    rewardChapter.id = 1;
    rewardChapter.storyId = 10;
    rewardChapter.key = "chapter.unsupported.reward";
    assert(rewardStory.RegisterChapter(rewardChapter));
    story::StoryNodeDefinition rewardNode;
    rewardNode.id = 100;
    rewardNode.chapterId = 1;
    rewardNode.key = "reward.gold";
    rewardNode.completionRewards.push_back(gold);
    assert(rewardStory.RegisterNode(10, rewardNode));
    const int rewardClaimsBefore = storyRepo.claims;
    assert(!rewardStory.ValidateStory(10));
    assert(!rewardStory.Start("Carol", 10, context));
    assert(storyRepo.claims == rewardClaimsBefore);
    return 0;
}
