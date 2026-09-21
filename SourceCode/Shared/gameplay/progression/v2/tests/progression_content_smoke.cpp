#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "gameplay/progression/v2/ProgressionContent.h"
#include "gameplay/covenants/v2/CovenantService.h"
#include "gameplay/story/v2/StoryService.h"

namespace
{
class CovenantRepo : public gameplay::covenants::ICovenantRepository
{
public:
    bool LoadAll(const std::string&, std::vector<gameplay::covenants::CovenantMembershipState>& memberships)
    { memberships.clear(); return true; }
    bool Save(const std::string&, const gameplay::covenants::CovenantMembershipState&) { return true; }
    gameplay::covenants::CovenantReputationSaveResult SaveReputationChange(
        const std::string&, const gameplay::covenants::CovenantMembershipState&, int, const std::string&)
    { return gameplay::covenants::CovenantReputationSaveResult::Applied; }
};

class StoryRepo : public gameplay::story::IStoryRepository
{
public:
    bool LoadProgress(const std::string&, int, gameplay::story::StoryProgressState&) { return false; }
    bool SaveProgress(const std::string&, const gameplay::story::StoryProgressState&) { return true; }
    bool GetNodeState(const std::string&, int, int, gameplay::story::StoryNodeStatus&) { return false; }
    bool SetNodeState(const std::string&, int, int, gameplay::story::StoryNodeStatus) { return true; }
    bool GetPlayerFlag(const std::string&, const std::string&, int&) { return false; }
    bool SetPlayerFlag(const std::string&, const std::string&, int) { return true; }
    bool GetWorldFlag(const std::string&, int&) { return false; }
    bool SetWorldFlag(const std::string&, int) { return true; }
    gameplay::story::StoryEffectClaimResult ClaimEffect(const std::string&, int, int, const std::string&)
    { return gameplay::story::StoryEffectClaimResult::Claimed; }
    bool MarkEffectApplied(const std::string&, int, int, const std::string&) { return true; }
};

class Commands : public gameplay::story::IStoryCommandGateway
{
public:
    bool Supports(gameplay::story::StoryActionType) const { return true; }
    bool StartQuest(const std::string&, int) { return true; }
    bool AddCovenantReputation(const std::string&, int, int, const std::string&) { return true; }
    bool ExecuteCustomAction(const std::string&, const gameplay::story::StoryActionDefinition&, const std::string&) { return true; }
};

class Rewards : public gameplay::rewards::IRewardGateway
{
public:
    bool Supports(gameplay::rewards::RewardType) const { return true; }
    gameplay::rewards::RewardResult Grant(const std::string&, const gameplay::rewards::RewardDefinition&, const std::string&)
    { return gameplay::rewards::RewardResult::Granted; }
};

gameplay::progression::ProgressionContentCatalog ValidCatalog()
{
    using namespace gameplay;
    progression::ProgressionContentCatalog catalog;

    reputation::ReputationDefinition genericReputation;
    genericReputation.key = "faction.validator";
    genericReputation.displayNameKey = "faction.validator.name";
    genericReputation.hasBounds = true;
    genericReputation.minValue = -100;
    genericReputation.maxValue = 1000;
    catalog.reputations.push_back(genericReputation);

    progression::CovenantContentDefinition covenant;
    covenant.definition.id = 7;
    covenant.definition.key = "covenant.validator";
    covenant.definition.displayNameKey = "covenant.validator.name";
    covenant.definition.exclusiveMembership = true;
    covenant.definition.hasReputationBounds = true;
    covenant.definition.minReputation = 0;
    covenant.definition.maxReputation = 1000;
    conditions::ConditionDefinition joinLevel;
    joinLevel.type = conditions::ConditionType::PlayerLevelAtLeast;
    joinLevel.value = 10;
    covenant.definition.joinConditions.push_back(joinLevel);
    covenant.definition.relatedQuestIds.push_back(9001);
    covenant.definition.relatedStoryIds.push_back(11);
    covenants::CovenantRewardDefinition joinReward;
    joinReward.key = "join.flag";
    joinReward.reward.type = rewards::RewardType::StoryFlag;
    joinReward.reward.key = "covenant.joined";
    joinReward.reward.amount = 1;
    covenant.definition.joinRewards.push_back(joinReward);
    covenants::CovenantRankDefinition rank;
    rank.id = 1;
    rank.covenantId = 7;
    rank.key = "initiate";
    rank.minReputation = 0;
    modifiers::GameplayModifier benefit;
    benefit.statKey = "validator.stat";
    benefit.operation = modifiers::ModifierOperation::Flat;
    benefit.value = 5.0;
    rank.benefits.push_back(benefit);
    covenants::CovenantRewardDefinition rankReward;
    rankReward.key = "rank.rep";
    rankReward.reward.type = rewards::RewardType::CovenantReputation;
    rankReward.reward.id = 7;
    rankReward.reward.amount = 1;
    rank.entryRewards.push_back(rankReward);
    covenant.ranks.push_back(rank);
    catalog.covenants.push_back(covenant);

    progression::StoryContentDefinition storyContent;
    storyContent.definition.id = 11;
    storyContent.definition.key = "story.validator";
    storyContent.definition.initialChapterId = 1;
    storyContent.definition.entryNodeId = 10;
    conditions::ConditionDefinition startLevel;
    startLevel.type = conditions::ConditionType::PlayerLevelAtLeast;
    startLevel.value = 1;
    storyContent.definition.startConditions.push_back(startLevel);
    conditions::ConditionDefinition worldReady;
    worldReady.type = conditions::ConditionType::HasWorldStoryFlag;
    worldReady.key = "world.ready";
    storyContent.definition.startConditions.push_back(worldReady);
    conditions::ConditionDefinition genericReputationReady;
    genericReputationReady.type = conditions::ConditionType::ReputationAtLeast;
    genericReputationReady.key = "faction.validator";
    genericReputationReady.value = 10;
    storyContent.definition.startConditions.push_back(genericReputationReady);

    story::StoryArcDefinition arc;
    arc.id = 100;
    arc.storyId = 11;
    arc.key = "arc.validator";
    storyContent.arcs.push_back(arc);

    story::StoryChapterDefinition chapter;
    chapter.id = 1;
    chapter.storyId = 11;
    chapter.arcId = 100;
    chapter.key = "chapter.validator";
    storyContent.chapters.push_back(chapter);

    story::StoryNodeDefinition node;
    node.id = 10;
    node.chapterId = 1;
    node.key = "node.validator";
    node.dialogueKey = "dialogue.validator";
    node.speakerKey = "speaker.validator";
    node.dialogueOptionKeys.push_back("dialogue.validator.continue");
    node.covenantId = 7;
    conditions::ConditionDefinition membership;
    membership.type = conditions::ConditionType::CovenantMembership;
    membership.id = 7;
    node.entryConditions.push_back(membership);
    story::StoryActionDefinition flag;
    flag.type = story::StoryActionType::SetPlayerFlag;
    flag.key = "validator.entered";
    flag.value = 1;
    node.onEnterActions.push_back(flag);
    rewards::RewardDefinition reputation;
    reputation.type = rewards::RewardType::CovenantReputation;
    reputation.id = 7;
    reputation.amount = 5;
    node.completionRewards.push_back(reputation);
    rewards::RewardDefinition genericReward;
    genericReward.type = rewards::RewardType::Reputation;
    genericReward.key = "faction.validator";
    genericReward.amount = 3;
    node.completionRewards.push_back(genericReward);
    story::StoryDialogueChoiceDefinition choice;
    choice.key = "choice.continue";
    choice.displayKey = "dialogue.choice.continue";
    choice.targetNodeId = 20;
    conditions::ConditionDefinition choiceLevel;
    choiceLevel.type = conditions::ConditionType::PlayerLevelAtLeast;
    choiceLevel.value = 1;
    choice.conditions.push_back(choiceLevel);
    node.dialogueChoices.push_back(choice);
    storyContent.nodes.push_back(node);

    story::StoryNodeDefinition target;
    target.id = 20;
    target.chapterId = 1;
    target.key = "node.choice.target";
    storyContent.nodes.push_back(target);
    catalog.stories.push_back(storyContent);
    return catalog;
}


bool RegisterCatalogFails(const gameplay::progression::ProgressionContentCatalog& catalog)
{
    CovenantRepo covenantRepo;
    StoryRepo storyRepo;
    Commands commands;
    Rewards rewardGateway;
    gameplay::rewards::RewardService rewardService(rewardGateway);
    gameplay::covenants::CovenantService covenantService(covenantRepo);
    gameplay::story::StoryService storyService(storyRepo, commands, rewardService);
    std::string error;
    return !gameplay::progression::ProgressionContentRegistrar::Register(
               catalog, covenantService, storyService, &error) && !error.empty();
}
}

int main()
{
    using namespace gameplay;
    CovenantRepo covenantRepo;
    StoryRepo storyRepo;
    Commands commands;
    Rewards rewardGateway;
    rewards::RewardService rewardService(rewardGateway);
    covenants::CovenantService covenantService(covenantRepo);
    story::StoryService storyService(storyRepo, commands, rewardService);

    progression::ProgressionContentCatalog catalog = ValidCatalog();
    std::string error;
    assert(progression::ProgressionContentRegistrar::Register(
        catalog, covenantService, storyService, &error));
    assert(error.empty());

    progression::ProgressionContentCatalog explicitStart = ValidCatalog();
    explicitStart.stories[0].definition.startMode = story::StoryStartMode::Explicit;
    CovenantRepo explicitCovenantRepo;
    StoryRepo explicitStoryRepo;
    covenants::CovenantService explicitCovenantService(explicitCovenantRepo);
    story::StoryService explicitStoryService(explicitStoryRepo, commands, rewardService);
    error.clear();
    assert(progression::ProgressionContentRegistrar::Register(
        explicitStart, explicitCovenantService, explicitStoryService, &error));
    assert(!explicitStoryService.IsAutomaticStart(11));

    progression::ProgressionContentCatalog invalidStartMode = ValidCatalog();
    invalidStartMode.stories[0].definition.startMode = static_cast<story::StoryStartMode>(99);
    assert(RegisterCatalogFails(invalidStartMode));

    // Character/class-dependent content is intentionally frozen and must fail closed.
    CovenantRepo frozenCovenantRepo;
    StoryRepo frozenStoryRepo;
    covenants::CovenantService frozenCovenantService(frozenCovenantRepo);
    story::StoryService frozenStoryService(frozenStoryRepo, commands, rewardService);
    progression::ProgressionContentCatalog frozen = ValidCatalog();
    conditions::ConditionDefinition classCondition;
    classCondition.type = conditions::ConditionType::ClassIs;
    classCondition.value = 9;
    frozen.stories[0].definition.startConditions.push_back(classCondition);
    error.clear();
    assert(!progression::ProgressionContentRegistrar::Register(
        frozen, frozenCovenantService, frozenStoryService, &error));
    assert(!error.empty());

    progression::ProgressionContentCatalog danglingStory = ValidCatalog();
    danglingStory.covenants[0].definition.relatedStoryIds[0] = 999;
    assert(RegisterCatalogFails(danglingStory));

    progression::ProgressionContentCatalog danglingCovenant = ValidCatalog();
    danglingCovenant.stories[0].nodes[0].covenantId = 999;
    assert(RegisterCatalogFails(danglingCovenant));

    progression::ProgressionContentCatalog danglingNodeCondition = ValidCatalog();
    conditions::ConditionDefinition nodeState;
    nodeState.type = conditions::ConditionType::StoryNodeState;
    nodeState.id = 11;
    nodeState.auxiliaryId = 999;
    nodeState.value = static_cast<int>(story::StoryNodeStatus::Completed);
    danglingNodeCondition.stories[0].nodes[0].completionConditions.push_back(nodeState);
    assert(RegisterCatalogFails(danglingNodeCondition));

    progression::ProgressionContentCatalog danglingRankCondition = ValidCatalog();
    conditions::ConditionDefinition rankCondition;
    rankCondition.type = conditions::ConditionType::CovenantRankAtLeast;
    rankCondition.id = 7;
    rankCondition.value = 999;
    danglingRankCondition.covenants[0].definition.joinConditions.push_back(rankCondition);
    assert(RegisterCatalogFails(danglingRankCondition));

    progression::ProgressionContentCatalog danglingChoice = ValidCatalog();
    danglingChoice.stories[0].nodes[0].dialogueChoices[0].targetNodeId = 999;
    assert(RegisterCatalogFails(danglingChoice));

    progression::ProgressionContentCatalog mixedChoiceTransition = ValidCatalog();
    mixedChoiceTransition.stories[0].nodes[0].transitions.push_back(20);
    assert(RegisterCatalogFails(mixedChoiceTransition));

    progression::ProgressionContentCatalog invalidChoiceCondition = ValidCatalog();
    conditions::ConditionDefinition choiceClass;
    choiceClass.type = conditions::ConditionType::ClassIs;
    choiceClass.value = 1;
    invalidChoiceCondition.stories[0].nodes[0].dialogueChoices[0].conditions.push_back(choiceClass);
    assert(RegisterCatalogFails(invalidChoiceCondition));

    progression::ProgressionContentCatalog unknownCovenantReward = ValidCatalog();
    unknownCovenantReward.covenants[0].ranks[0].entryRewards[0].reward.id = 999;
    assert(RegisterCatalogFails(unknownCovenantReward));

    progression::ProgressionContentCatalog unknownGenericCondition = ValidCatalog();
    unknownGenericCondition.stories[0].definition.startConditions.back().key = "faction.unknown";
    assert(RegisterCatalogFails(unknownGenericCondition));

    progression::ProgressionContentCatalog unknownGenericReward = ValidCatalog();
    unknownGenericReward.stories[0].nodes[0].completionRewards[1].key = "faction.unknown";
    assert(RegisterCatalogFails(unknownGenericReward));

    progression::ProgressionContentCatalog genericCovenantReward = ValidCatalog();
    genericCovenantReward.covenants[0].definition.joinRewards[0].reward.type = rewards::RewardType::Reputation;
    genericCovenantReward.covenants[0].definition.joinRewards[0].reward.key = "faction.validator";
    genericCovenantReward.covenants[0].definition.joinRewards[0].reward.id = 7;
    genericCovenantReward.covenants[0].definition.joinRewards[0].reward.amount = 1;
    assert(RegisterCatalogFails(genericCovenantReward));

    progression::ProgressionContentCatalog invalidQuestState = ValidCatalog();
    conditions::ConditionDefinition questState;
    questState.type = conditions::ConditionType::QuestState;
    questState.id = 9001;
    questState.value = 3;
    invalidQuestState.stories[0].nodes[0].completionConditions.push_back(questState);
    assert(RegisterCatalogFails(invalidQuestState));

    return 0;
}
