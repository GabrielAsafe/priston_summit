#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "gameplay/story/v2/StoryService.h"

namespace
{
class FakeRepo : public gameplay::story::IStoryRepository
{
public:
    FakeRepo() : failMarkOnce(false) {}

    bool LoadProgress(const std::string& playerKey, int storyId, gameplay::story::StoryProgressState& state)
    {
        const std::string key = playerKey + ":" + std::to_string(storyId);
        std::map<std::string, gameplay::story::StoryProgressState>::const_iterator it = progress.find(key);
        if (it == progress.end()) return false;
        state = it->second; return true;
    }
    bool SaveProgress(const std::string& playerKey, const gameplay::story::StoryProgressState& state)
    {
        progress[playerKey + ":" + std::to_string(state.storyId)] = state; return true;
    }
    bool GetNodeState(const std::string& playerKey, int storyId, int nodeId,
                      gameplay::story::StoryNodeStatus& state)
    {
        const std::string key = playerKey + ":" + std::to_string(storyId) + ":" + std::to_string(nodeId);
        if (nodeStates.count(key) == 0u) return false;
        state = nodeStates[key]; return true;
    }
    bool SetNodeState(const std::string& playerKey, int storyId, int nodeId,
                      gameplay::story::StoryNodeStatus state)
    {
        nodeStates[playerKey + ":" + std::to_string(storyId) + ":" + std::to_string(nodeId)] = state;
        return true;
    }
    bool GetPlayerFlag(const std::string& playerKey, const std::string& flagKey, int& value)
    {
        const std::string key = playerKey + ":" + flagKey;
        if (playerFlags.count(key) == 0u) return false;
        value = playerFlags[key]; return true;
    }
    bool SetPlayerFlag(const std::string& playerKey, const std::string& flagKey, int value)
    { playerFlags[playerKey + ":" + flagKey] = value; return true; }
    bool GetWorldFlag(const std::string& flagKey, int& value)
    { if (worldFlags.count(flagKey) == 0u) return false; value = worldFlags[flagKey]; return true; }
    bool SetWorldFlag(const std::string& flagKey, int value)
    { worldFlags[flagKey] = value; return true; }

    gameplay::story::StoryEffectClaimResult ClaimEffect(const std::string& playerKey,
                                                          int storyId,
                                                          int nodeId,
                                                          const std::string& effectKey)
    {
        const std::string key = EffectKey(playerKey, storyId, nodeId, effectKey);
        std::map<std::string, int>::const_iterator it = effects.find(key);
        if (it == effects.end())
        {
            effects[key] = 0;
            effectPlayers[key] = playerKey;
            gameplay::story::StoryEffectReceiptEntry entry;
            entry.storyId = storyId;
            entry.nodeId = nodeId;
            entry.effectKey = effectKey;
            entry.status = gameplay::story::StoryEffectReceiptStatus::Pending;
            effectEntries[key] = entry;
            return gameplay::story::StoryEffectClaimResult::Claimed;
        }
        return it->second != 0 ? gameplay::story::StoryEffectClaimResult::AlreadyApplied
                               : gameplay::story::StoryEffectClaimResult::Pending;
    }

    bool MarkEffectApplied(const std::string& playerKey,
                           int storyId,
                           int nodeId,
                           const std::string& effectKey)
    {
        const std::string key = EffectKey(playerKey, storyId, nodeId, effectKey);
        if (failMarkOnce && effectKey == failMarkEffectKey)
        {
            failMarkOnce = false;
            return false;
        }
        if (effects.count(key) == 0u)
            return false;
        effects[key] = 1;
        effectEntries[key].status = gameplay::story::StoryEffectReceiptStatus::Applied;
        return true;
    }

    gameplay::story::StoryEffectReceiptLoadResult GetEffectReceiptStatus(
        const std::string& playerKey,
        int storyId,
        int nodeId,
        const std::string& effectKey,
        gameplay::story::StoryEffectReceiptStatus& status)
    {
        const std::string key = EffectKey(playerKey, storyId, nodeId, effectKey);
        std::map<std::string, int>::const_iterator it = effects.find(key);
        if (it == effects.end())
            return gameplay::story::StoryEffectReceiptLoadResult::NotFound;
        status = it->second != 0 ? gameplay::story::StoryEffectReceiptStatus::Applied
                                 : gameplay::story::StoryEffectReceiptStatus::Pending;
        return gameplay::story::StoryEffectReceiptLoadResult::Found;
    }

    bool ListPendingEffects(const std::string& playerKey,
                            std::vector<gameplay::story::StoryEffectReceiptEntry>& receipts)
    {
        receipts.clear();
        for (std::map<std::string, gameplay::story::StoryEffectReceiptEntry>::const_iterator it =
                 effectEntries.begin();
             it != effectEntries.end(); ++it)
        {
            std::map<std::string, std::string>::const_iterator owner = effectPlayers.find(it->first);
            if (owner != effectPlayers.end() && owner->second == playerKey &&
                it->second.status == gameplay::story::StoryEffectReceiptStatus::Pending)
                receipts.push_back(it->second);
        }
        return true;
    }

    bool AcknowledgePendingEffect(const std::string& playerKey,
                                  int storyId,
                                  int nodeId,
                                  const std::string& effectKey)
    {
        gameplay::story::StoryEffectReceiptStatus status =
            gameplay::story::StoryEffectReceiptStatus::Pending;
        if (GetEffectReceiptStatus(playerKey, storyId, nodeId, effectKey, status) !=
            gameplay::story::StoryEffectReceiptLoadResult::Found)
            return false;
        return status == gameplay::story::StoryEffectReceiptStatus::Applied ||
               MarkEffectApplied(playerKey, storyId, nodeId, effectKey);
    }

    static std::string EffectKey(const std::string& playerKey,
                                 int storyId,
                                 int nodeId,
                                 const std::string& effectKey)
    {
        return playerKey + ":" + std::to_string(storyId) + ":" +
               std::to_string(nodeId) + ":" + effectKey;
    }

    std::map<std::string, gameplay::story::StoryProgressState> progress;
    std::map<std::string, gameplay::story::StoryNodeStatus> nodeStates;
    std::map<std::string, int> playerFlags;
    std::map<std::string, int> worldFlags;
    std::map<std::string, int> effects;
    std::map<std::string, std::string> effectPlayers;
    std::map<std::string, gameplay::story::StoryEffectReceiptEntry> effectEntries;
    bool failMarkOnce;
    std::string failMarkEffectKey;
};

class FakeCommands : public gameplay::story::IStoryCommandGateway
{
public:
    FakeCommands() : questStarts(0), covenantAdds(0), customs(0) {}
    bool StartQuest(const std::string&, int) { ++questStarts; return true; }
    bool AddCovenantReputation(const std::string&, int, int, const std::string&) { ++covenantAdds; return true; }
    bool ExecuteCustomAction(const std::string&, const gameplay::story::StoryActionDefinition&, const std::string&) { ++customs; return true; }
    int questStarts; int covenantAdds; int customs;
};

class FakeRewards : public gameplay::rewards::IRewardGateway
{
public:
    explicit FakeRewards(bool retrySafeValue = false) : grants(0), retrySafe(retrySafeValue) {}
    bool IsRetrySafe(gameplay::rewards::RewardType) const { return retrySafe; }
    gameplay::rewards::RewardResult Grant(const std::string&, const gameplay::rewards::RewardDefinition&, const std::string& source)
    { ++grants; sources.push_back(source); return gameplay::rewards::RewardResult::Granted; }
    int grants; bool retrySafe; std::vector<std::string> sources;
};

class FakeContext : public gameplay::conditions::ConditionContext
{
public:
    FakeContext() : level(10), questState(0), covenantRank(0), covenantRep(0) {}
    int PlayerLevel() const { return level; }
    int LegacyJobCode() const { return 1; }
    bool HasStoryFlag(const std::string&) const { return false; }
    int StoryNodeState(int, int) const { return 0; }
    int QuestState(int questId) const { return questId == 100 ? questState : 0; }
    bool HasCovenantMembership(int covenantId) const { return covenantId == 5; }
    int CovenantRank(int covenantId) const { return covenantId == 5 ? covenantRank : 0; }
    int CovenantReputation(int covenantId) const { return covenantId == 5 ? covenantRep : 0; }
    int ProfessionLevel(const std::string&) const { return 0; }
    bool EvaluateCustomCondition(const gameplay::conditions::ConditionDefinition&) const { return false; }
    int level; int questState; int covenantRank; int covenantRep;
};

void RegisterStory(gameplay::story::StoryService& service)
{
    using namespace gameplay;
    story::StoryDefinition definition;
    definition.id = 1; definition.key = "story.intro"; definition.initialChapterId = 1; definition.entryNodeId = 10;
    conditions::ConditionDefinition level;
    level.type = conditions::ConditionType::PlayerLevelAtLeast; level.value = 10;
    definition.startConditions.push_back(level);
    assert(service.RegisterStory(definition));

    story::StoryArcDefinition arc;
    arc.id = 1000; arc.storyId = 1; arc.key = "arc.intro"; arc.displayNameKey = "story.arc.intro";
    assert(service.RegisterArc(arc));

    story::StoryChapterDefinition chapter;
    chapter.id = 1; chapter.storyId = 1; chapter.arcId = 1000; chapter.key = "chapter.intro";
    chapter.displayNameKey = "story.chapter.intro";
    assert(service.RegisterChapter(chapter));

    story::StoryNodeDefinition first;
    first.id = 10; first.chapterId = 1; first.key = "meet"; first.questId = 100;
    story::StoryActionDefinition startQuest;
    startQuest.type = story::StoryActionType::StartQuest; startQuest.id = 100;
    first.onEnterActions.push_back(startQuest);
    conditions::ConditionDefinition questDone;
    questDone.type = conditions::ConditionType::QuestState; questDone.id = 100; questDone.value = 2;
    first.completionConditions.push_back(questDone);
    story::StoryActionDefinition playerFlag;
    playerFlag.type = story::StoryActionType::SetPlayerFlag; playerFlag.key = "intro.quest_done"; playerFlag.value = 1;
    first.onCompleteActions.push_back(playerFlag);
    rewards::RewardDefinition reward;
    reward.type = rewards::RewardType::Gold; reward.amount = 1;
    first.completionRewards.push_back(reward);
    first.transitions.push_back(20);
    assert(service.RegisterNode(1, first));

    story::StoryNodeDefinition second;
    second.id = 20; second.chapterId = 1; second.key = "covenant_gate";
    second.dialogueKey = "dialogue.intro.covenant";
    second.speakerKey = "speaker.guildmaster";
    second.dialogueOptionKeys.push_back("dialogue.option.continue");
    conditions::ConditionDefinition rank;
    rank.type = conditions::ConditionType::CovenantRankAtLeast; rank.id = 5; rank.value = 2;
    second.entryConditions.push_back(rank);
    conditions::ConditionDefinition rep;
    rep.type = conditions::ConditionType::CovenantReputationAtLeast; rep.id = 5; rep.value = 100;
    second.completionConditions.push_back(rep);
    story::StoryActionDefinition worldFlag;
    worldFlag.type = story::StoryActionType::SetWorldFlag; worldFlag.key = "world.intro_complete"; worldFlag.value = 1;
    second.onCompleteActions.push_back(worldFlag);
    assert(service.RegisterNode(1, second));
    assert(service.ValidateStory(1));
}

void RegisterChoiceStory(gameplay::story::StoryService& service)
{
    using namespace gameplay;
    story::StoryDefinition definition;
    definition.id = 2;
    definition.key = "story.choice";
    definition.initialChapterId = 2;
    definition.entryNodeId = 100;
    assert(service.RegisterStory(definition));

    story::StoryChapterDefinition chapter;
    chapter.id = 2;
    chapter.storyId = 2;
    chapter.key = "chapter.choice";
    assert(service.RegisterChapter(chapter));

    story::StoryNodeDefinition prompt;
    prompt.id = 100;
    prompt.chapterId = 2;
    prompt.key = "prompt";
    prompt.dialogueKey = "dialogue.choice.prompt";

    story::StoryDialogueChoiceDefinition left;
    left.key = "choice.left";
    left.displayKey = "dialogue.choice.left";
    left.targetNodeId = 200;
    conditions::ConditionDefinition leftLevel;
    leftLevel.type = conditions::ConditionType::PlayerLevelAtLeast;
    leftLevel.value = 10;
    left.conditions.push_back(leftLevel);
    prompt.dialogueChoices.push_back(left);

    story::StoryDialogueChoiceDefinition right;
    right.key = "choice.right";
    right.displayKey = "dialogue.choice.right";
    right.targetNodeId = 300;
    conditions::ConditionDefinition rightLevel;
    rightLevel.type = conditions::ConditionType::PlayerLevelAtLeast;
    rightLevel.value = 20;
    right.conditions.push_back(rightLevel);
    prompt.dialogueChoices.push_back(right);
    assert(service.RegisterNode(2, prompt));

    story::StoryNodeDefinition leftNode;
    leftNode.id = 200;
    leftNode.chapterId = 2;
    leftNode.key = "left";
    assert(service.RegisterNode(2, leftNode));

    story::StoryNodeDefinition rightNode;
    rightNode.id = 300;
    rightNode.chapterId = 2;
    rightNode.key = "right";
    assert(service.RegisterNode(2, rightNode));

    assert(service.ValidateStory(2));
}
}

int main()
{
    using namespace gameplay;
    FakeRepo repo; FakeCommands commands; FakeRewards rewardGateway;
    rewards::RewardService rewards(rewardGateway);
    events::GameplayEventHub hub;
    int storyStarts = 0;
    int storyCompletes = 0;
    int nodeEnters = 0;
    int nodeCompletes = 0;
    int worldFlagChanges = 0;
    hub.Subscribe<story::StoryStartedEvent>([&](const story::StoryStartedEvent&) { ++storyStarts; });
    hub.Subscribe<story::StoryNodeEnteredEvent>([&](const story::StoryNodeEnteredEvent&) { ++nodeEnters; });
    hub.Subscribe<story::StoryNodeCompletedEvent>([&](const story::StoryNodeCompletedEvent&) { ++nodeCompletes; });
    hub.Subscribe<story::StoryCompletedEvent>([&](const story::StoryCompletedEvent&) { ++storyCompletes; });
    hub.Subscribe<story::StoryWorldFlagChangedEvent>([&](const story::StoryWorldFlagChangedEvent& event)
    {
        if (event.flagKey == "world.intro_complete" && event.value == 1)
            ++worldFlagChanges;
    });
    story::StoryService service(repo, commands, rewards, &hub);
    RegisterStory(service);

    std::vector<story::StoryCatalogEntry> storyCatalog;
    service.GetCatalog(storyCatalog);
    assert(storyCatalog.size() == 1u);
    assert(storyCatalog[0].id == 1);
    assert(storyCatalog[0].key == "story.intro");
    assert(storyCatalog[0].displayNameKey.empty());
    assert(storyCatalog[0].startMode == story::StoryStartMode::Automatic);

    FakeContext context;
    assert(service.Start("Alice", 1, context));
    assert(storyStarts == 1 && nodeEnters == 1 && nodeCompletes == 0);
    assert(commands.questStarts == 1);
    assert(!service.Start("Alice", 1, context));
    assert(!service.Evaluate("Alice", 1, context));

    context.questState = 2;
    assert(service.Evaluate("Alice", 1, context));
    story::StoryProgressState state;
    assert(service.GetProgress("Alice", 1, state));
    assert(state.status == story::StoryNodeStatus::Completed); // transition is deferred, not lost
    assert(storyStarts == 1 && nodeEnters == 1 && nodeCompletes == 1);
    assert(rewardGateway.grants == 1);
    assert(rewardGateway.sources[0] == "story.1.node.10.reward.0");
    int flagValue = 0;
    assert(repo.GetPlayerFlag("Alice", "intro.quest_done", flagValue) && flagValue == 1);
    assert(!repo.GetPlayerFlag("Bob", "intro.quest_done", flagValue));
    std::vector<story::StorySnapshot> startedSnapshots;
    assert(service.GetStartedSnapshots("Alice", startedSnapshots));
    assert(startedSnapshots.size() == 1u);
    assert(startedSnapshots[0].storyKey == "story.intro");
    assert(service.GetStartedSnapshots("Bob", startedSnapshots));
    assert(startedSnapshots.empty());

    // A completed node can transition later when its target entry conditions become true.
    context.covenantRank = 2;
    assert(service.Evaluate("Alice", 1, context));
    assert(service.GetProgress("Alice", 1, state));
    assert(state.nodeId == 20 && state.status == story::StoryNodeStatus::Active);
    story::StoryNodeStatus historical = story::StoryNodeStatus::Locked;
    assert(repo.GetNodeState("Alice", 1, 10, historical));
    assert(historical == story::StoryNodeStatus::Completed);
    assert(repo.GetNodeState("Alice", 1, 20, historical));
    assert(historical == story::StoryNodeStatus::Active);
    story::StorySnapshot snapshot;
    assert(service.GetSnapshot("Alice", 1, snapshot));
    assert(snapshot.arcId == 1000);
    assert(snapshot.arcKey == "arc.intro");
    assert(snapshot.chapterKey == "chapter.intro");
    assert(snapshot.nodeKey == "covenant_gate");
    assert(snapshot.dialogueKey == "dialogue.intro.covenant");
    assert(snapshot.dialogueOptionKeys.size() == 1u);
    assert(!snapshot.terminal && !snapshot.awaitingChoice && !snapshot.awaitingTransition);
    assert(storyStarts == 1 && nodeEnters == 2 && nodeCompletes == 1);
    context.covenantRep = 100;
    assert(service.Evaluate("Alice", 1, context));
    assert(service.GetProgress("Alice", 1, state));
    assert(state.status == story::StoryNodeStatus::Completed);
    assert(service.GetSnapshot("Alice", 1, snapshot));
    assert(snapshot.terminal && !snapshot.awaitingChoice && !snapshot.awaitingTransition);
    assert(storyStarts == 1 && storyCompletes == 1 && nodeEnters == 2 && nodeCompletes == 2);
    assert(repo.GetWorldFlag("world.intro_complete", flagValue) && flagValue == 1);
    assert(worldFlagChanges == 1);
    assert(rewardGateway.grants == 1);

    // Durable effect receipts fail closed across the ambiguous window after a grant.
    FakeRepo crashRepo; FakeCommands crashCommands; FakeRewards crashRewardsGateway;
    rewards::RewardService crashRewards(crashRewardsGateway);
    story::StoryService crashService(crashRepo, crashCommands, crashRewards);
    RegisterStory(crashService);
    FakeContext crashContext; crashContext.questState = 2;
    assert(crashService.Start("Crash", 1, crashContext));
    crashRepo.failMarkOnce = true;
    crashRepo.failMarkEffectKey = "reward.0";
    assert(!crashService.Evaluate("Crash", 1, crashContext));
    assert(crashRewardsGateway.grants == 1);
    // Retry sees Pending and refuses to grant a reward whose gateway does not
    // explicitly guarantee source-key idempotency.
    assert(!crashService.Evaluate("Crash", 1, crashContext));
    assert(crashRewardsGateway.grants == 1);
    story::StoryEffectReceiptStatus receiptStatus = story::StoryEffectReceiptStatus::Applied;
    assert(crashService.GetEffectReceiptStatus("Crash", 1, 10, "reward.0", receiptStatus) ==
           story::StoryEffectReceiptLoadResult::Found);
    assert(receiptStatus == story::StoryEffectReceiptStatus::Pending);
    std::vector<story::StoryEffectReceiptEntry> pendingReceipts;
    assert(crashService.ListPendingEffects("Crash", pendingReceipts));
    assert(pendingReceipts.size() == 1u);
    assert(pendingReceipts[0].storyId == 1);
    assert(pendingReceipts[0].nodeId == 10);
    assert(pendingReceipts[0].effectKey == "reward.0");
    assert(pendingReceipts[0].status == story::StoryEffectReceiptStatus::Pending);
    assert(crashService.AcknowledgePendingEffect("Crash", 1, 10, "reward.0"));
    assert(crashService.GetEffectReceiptStatus("Crash", 1, 10, "reward.0", receiptStatus) ==
           story::StoryEffectReceiptLoadResult::Found);
    assert(receiptStatus == story::StoryEffectReceiptStatus::Applied);
    pendingReceipts.clear();
    assert(crashService.ListPendingEffects("Crash", pendingReceipts));
    assert(pendingReceipts.empty());
    assert(crashService.Evaluate("Crash", 1, crashContext));
    assert(crashRewardsGateway.grants == 1);

    // Retry-safe gateways may reconcile the same Pending window. The gateway is
    // responsible for making the repeated source key idempotent at its boundary.
    FakeRepo safeRepo; FakeCommands safeCommands; FakeRewards safeRewardsGateway(true);
    rewards::RewardService safeRewards(safeRewardsGateway);
    story::StoryService safeService(safeRepo, safeCommands, safeRewards);
    RegisterStory(safeService);
    FakeContext safeContext; safeContext.questState = 2;
    assert(safeService.Start("RetrySafe", 1, safeContext));
    safeRepo.failMarkOnce = true;
    safeRepo.failMarkEffectKey = "reward.0";
    assert(!safeService.Evaluate("RetrySafe", 1, safeContext));
    assert(safeRewardsGateway.grants == 1);
    assert(safeService.Evaluate("RetrySafe", 1, safeContext));
    assert(safeRewardsGateway.grants == 2);
    assert(safeService.GetProgress("RetrySafe", 1, state));
    assert(state.status == story::StoryNodeStatus::Completed);

    // Explicit dialogue choices are server-authoritative branches, not inferred from option order.
    FakeRepo choiceRepo; FakeCommands choiceCommands; FakeRewards choiceRewardsGateway;
    rewards::RewardService choiceRewards(choiceRewardsGateway);
    events::GameplayEventHub choiceHub;
    int choiceEvents = 0;
    choiceHub.Subscribe<story::StoryDialogueChoiceSelectedEvent>(
        [&](const story::StoryDialogueChoiceSelectedEvent& event)
        {
            if (event.storyId == 2 && event.nodeId == 100 &&
                event.choiceKey == "choice.left" && event.targetNodeId == 200)
                ++choiceEvents;
        });
    story::StoryService choiceService(choiceRepo, choiceCommands, choiceRewards, &choiceHub);
    RegisterChoiceStory(choiceService);
    std::vector<story::StoryCatalogEntry> choiceCatalog;
    choiceService.GetCatalog(choiceCatalog);
    assert(choiceCatalog.size() == 1u && choiceCatalog[0].id == 2);
    FakeContext choiceContext;
    assert(choiceService.Start("Choice", 2, choiceContext));
    assert(choiceService.Evaluate("Choice", 2, choiceContext));
    assert(choiceService.GetProgress("Choice", 2, state));
    assert(state.nodeId == 100 && state.status == story::StoryNodeStatus::Completed);
    story::StorySnapshot choiceSnapshot;
    assert(choiceService.GetSnapshot("Choice", 2, choiceSnapshot));
    assert(choiceSnapshot.dialogueChoiceKeys.size() == 2u);
    assert(choiceSnapshot.dialogueChoices.size() == 2u);
    assert(!choiceSnapshot.terminal && choiceSnapshot.awaitingChoice && !choiceSnapshot.awaitingTransition);
    assert(choiceSnapshot.dialogueChoices[0].displayKey == "dialogue.choice.left");
    std::vector<story::StoryDialogueChoiceSnapshot> availableChoiceSnapshots;
    assert(choiceService.GetAvailableDialogueChoiceSnapshots(
        "Choice", 2, choiceContext, availableChoiceSnapshots));
    assert(availableChoiceSnapshots.size() == 1u);
    assert(availableChoiceSnapshots[0].key == "choice.left");
    assert(availableChoiceSnapshots[0].displayKey == "dialogue.choice.left");
    assert(availableChoiceSnapshots[0].targetNodeId == 200);
    std::vector<std::string> availableChoices;
    assert(choiceService.GetAvailableDialogueChoices("Choice", 2, choiceContext, availableChoices));
    assert(availableChoices.size() == 1u && availableChoices[0] == "choice.left");
    assert(choiceService.ChooseDialogueOption("Choice", 2, "choice.right", choiceContext) ==
           story::StoryChoiceResult::NotAvailable);
    assert(choiceService.ChooseDialogueOption("Choice", 2, "choice.left", choiceContext) ==
           story::StoryChoiceResult::Selected);
    assert(choiceService.GetProgress("Choice", 2, state));
    assert(state.nodeId == 200 && state.status == story::StoryNodeStatus::Active);
    assert(choiceEvents == 1);
    assert(choiceService.ChooseDialogueOption("Choice", 2, "choice.left", choiceContext) ==
           story::StoryChoiceResult::NotAvailable);
    return 0;
}
