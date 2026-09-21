#include <cassert>
#include <map>
#include <string>

#include "gameplay/progression/v2/ProgressionPlayerRuntime.h"

namespace
{
class Repo : public gameplay::story::IStoryRepository
{
public:
    bool LoadProgress(const std::string& player, int storyId, gameplay::story::StoryProgressState& state)
    {
        const std::string key = player + ":" + std::to_string(storyId);
        std::map<std::string, gameplay::story::StoryProgressState>::const_iterator it = progress.find(key);
        if (it == progress.end()) return false;
        state = it->second; return true;
    }
    bool SaveProgress(const std::string& player, const gameplay::story::StoryProgressState& state)
    { progress[player + ":" + std::to_string(state.storyId)] = state; return true; }
    bool GetNodeState(const std::string&, int, int, gameplay::story::StoryNodeStatus&) { return false; }
    bool SetNodeState(const std::string&, int, int, gameplay::story::StoryNodeStatus) { return true; }
    bool GetPlayerFlag(const std::string& player, const std::string& flag, int& value)
    {
        const std::string key = player + ":" + flag;
        std::map<std::string, int>::const_iterator it = playerFlags.find(key);
        if (it == playerFlags.end()) return false;
        value = it->second; return true;
    }
    bool SetPlayerFlag(const std::string& player, const std::string& flag, int value)
    { playerFlags[player + ":" + flag] = value; return true; }
    bool GetWorldFlag(const std::string&, int&) { return false; }
    bool SetWorldFlag(const std::string&, int) { return true; }
    gameplay::story::StoryEffectClaimResult ClaimEffect(const std::string&, int, int, const std::string&)
    { return gameplay::story::StoryEffectClaimResult::Claimed; }
    bool MarkEffectApplied(const std::string&, int, int, const std::string&) { return true; }
    std::map<std::string, gameplay::story::StoryProgressState> progress;
    std::map<std::string, int> playerFlags;
};

class Commands : public gameplay::story::IStoryCommandGateway
{
public:
    bool StartQuest(const std::string&, int) { return false; }
    bool AddCovenantReputation(const std::string&, int, int, const std::string&) { return false; }
    bool ExecuteCustomAction(const std::string&, const gameplay::story::StoryActionDefinition&, const std::string&) { return false; }
};

class RewardGateway : public gameplay::rewards::IRewardGateway
{
public:
    gameplay::rewards::RewardResult Grant(const std::string&, const gameplay::rewards::RewardDefinition&, const std::string&)
    { return gameplay::rewards::RewardResult::Unsupported; }
};

class Context : public gameplay::conditions::ConditionContext
{
public:
    explicit Context(int levelValue, const Repo* repoValue = 0, const std::string& playerValue = std::string())
        : level(levelValue), repo(repoValue), player(playerValue) {}
    int PlayerLevel() const { return level; }
    int LegacyJobCode() const { return 0; }
    bool HasStoryFlag(const std::string& flag) const
    {
        if (!repo) return false;
        int value = 0;
        return const_cast<Repo*>(repo)->GetPlayerFlag(player, flag, value) && value != 0;
    }
    int StoryNodeState(int, int) const { return 0; }
    int QuestState(int) const { return 0; }
    bool HasCovenantMembership(int) const { return false; }
    int CovenantRank(int) const { return 0; }
    int CovenantReputation(int) const { return 0; }
    int ProfessionLevel(const std::string&) const { return 0; }
    bool EvaluateCustomCondition(const gameplay::conditions::ConditionDefinition&) const { return false; }
    int level;
    const Repo* repo;
    std::string player;
};


void RegisterFlagProducer(gameplay::story::StoryService& service, int storyId, const std::string& flag)
{
    gameplay::story::StoryDefinition story;
    story.id = storyId; story.key = "story.producer"; story.initialChapterId = 1; story.entryNodeId = 10;
    assert(service.RegisterStory(story));
    gameplay::story::StoryChapterDefinition chapter;
    chapter.id = 1; chapter.storyId = storyId; chapter.key = "chapter.producer";
    assert(service.RegisterChapter(chapter));
    gameplay::story::StoryNodeDefinition node;
    node.id = 10; node.chapterId = 1; node.key = "producer";
    gameplay::story::StoryActionDefinition action;
    action.type = gameplay::story::StoryActionType::SetPlayerFlag; action.key = flag; action.value = 1;
    node.onCompleteActions.push_back(action);
    assert(service.RegisterNode(storyId, node));
    assert(service.ValidateStory(storyId));
}

void RegisterFlagConsumer(gameplay::story::StoryService& service, int storyId, const std::string& flag)
{
    gameplay::story::StoryDefinition story;
    story.id = storyId; story.key = "story.consumer"; story.initialChapterId = 1; story.entryNodeId = 10;
    gameplay::conditions::ConditionDefinition condition;
    condition.type = gameplay::conditions::ConditionType::HasStoryFlag; condition.key = flag;
    story.startConditions.push_back(condition);
    assert(service.RegisterStory(story));
    gameplay::story::StoryChapterDefinition chapter;
    chapter.id = 1; chapter.storyId = storyId; chapter.key = "chapter.consumer";
    assert(service.RegisterChapter(chapter));
    gameplay::story::StoryNodeDefinition node;
    node.id = 10; node.chapterId = 1; node.key = "consumer";
    assert(service.RegisterNode(storyId, node));
    assert(service.ValidateStory(storyId));
}
void RegisterSimpleStory(gameplay::story::StoryService& service,
                         int storyId,
                         int requiredLevel,
                         gameplay::story::StoryStartMode startMode = gameplay::story::StoryStartMode::Automatic)
{
    gameplay::story::StoryDefinition story;
    story.id = storyId;
    story.key = "story." + std::to_string(storyId);
    story.startMode = startMode;
    story.initialChapterId = 1;
    story.entryNodeId = 10;
    gameplay::conditions::ConditionDefinition level;
    level.type = gameplay::conditions::ConditionType::PlayerLevelAtLeast;
    level.value = requiredLevel;
    story.startConditions.push_back(level);
    assert(service.RegisterStory(story));

    gameplay::story::StoryChapterDefinition chapter;
    chapter.id = 1; chapter.storyId = storyId; chapter.key = "chapter";
    assert(service.RegisterChapter(chapter));

    gameplay::story::StoryNodeDefinition node;
    node.id = 10; node.chapterId = 1; node.key = "node";
    assert(service.RegisterNode(storyId, node));
    assert(service.ValidateStory(storyId));
}
}

int main()
{
    Repo repo;
    Commands commands;
    RewardGateway rewardGateway;
    gameplay::rewards::RewardService rewards(rewardGateway);
    gameplay::story::StoryService service(repo, commands, rewards);
    RegisterSimpleStory(service, 1, 5);
    RegisterSimpleStory(service, 2, 20);
    RegisterSimpleStory(service, 4, 5, gameplay::story::StoryStartMode::Explicit);

    Context context(10);
    gameplay::progression::ProgressionPlayerRunReport report =
        gameplay::progression::ProgressionPlayerStoryRunner::Run(service, "Alice", context);
    assert(report.Ok());
    assert(report.storiesStarted == 1);
    assert(report.storiesAdvanced == 1); // the eligible terminal story stabilizes in one run
    assert(report.storiesUnchanged == 2);
    assert(report.advanceSteps == 1);
    assert(report.stepLimitHits == 0);

    report = gameplay::progression::ProgressionPlayerStoryRunner::Run(service, "Alice", context);
    assert(report.Ok());
    assert(report.storiesStarted == 0);
    assert(report.storiesAdvanced == 0);
    assert(report.storiesUnchanged == 3);
    assert(report.advanceSteps == 0);

    // Explicit stories never auto-start, but once started by a server-authoritative
    // caller they participate in the same deterministic runner.
    gameplay::story::StoryProgressState explicitState;
    assert(service.GetProgressResult("Alice", 4, explicitState) ==
           gameplay::story::StoryProgressLoadResult::NotFound);
    assert(service.StartDetailed("Alice", 4, context) == gameplay::story::StoryStartResult::Started);
    report = gameplay::progression::ProgressionPlayerStoryRunner::Run(service, "Alice", context);
    assert(report.Ok());
    assert(report.storiesStarted == 0);
    assert(report.storiesAdvanced == 1);
    assert(service.GetProgress("Alice", 4, explicitState));
    assert(explicitState.status == gameplay::story::StoryNodeStatus::Completed);

    // Cross-story effects must converge regardless of story ordering. Story 10 is
    // processed before Story 20, but Story 20 unlocks it during the first pass.
    Repo crossRepo;
    gameplay::story::StoryService crossService(crossRepo, commands, rewards);
    RegisterFlagConsumer(crossService, 10, "cross.unlock");
    RegisterFlagProducer(crossService, 20, "cross.unlock");
    Context crossContext(10, &crossRepo, "Cross");
    report = gameplay::progression::ProgressionPlayerStoryRunner::Run(
        crossService, "Cross", crossContext);
    assert(report.Ok());
    assert(report.storiesStarted == 2);
    assert(report.storiesAdvanced == 2);
    assert(report.storiesUnchanged == 0);
    gameplay::story::StoryProgressState crossState;
    assert(crossService.GetProgress("Cross", 10, crossState));
    assert(crossState.status == gameplay::story::StoryNodeStatus::Completed);

    // A two-node cycle with unconditional completion must be capped, never spin forever.
    Repo cycleRepo;
    gameplay::story::StoryService cycleService(cycleRepo, commands, rewards);
    gameplay::story::StoryDefinition cycle;
    cycle.id = 3; cycle.key = "story.cycle"; cycle.initialChapterId = 1; cycle.entryNodeId = 10;
    assert(cycleService.RegisterStory(cycle));
    gameplay::story::StoryChapterDefinition cycleChapter;
    cycleChapter.id = 1; cycleChapter.storyId = 3; cycleChapter.key = "chapter.cycle";
    assert(cycleService.RegisterChapter(cycleChapter));
    gameplay::story::StoryNodeDefinition cycleA;
    cycleA.id = 10; cycleA.chapterId = 1; cycleA.key = "cycle.a"; cycleA.transitions.push_back(20);
    assert(cycleService.RegisterNode(3, cycleA));
    gameplay::story::StoryNodeDefinition cycleB;
    cycleB.id = 20; cycleB.chapterId = 1; cycleB.key = "cycle.b"; cycleB.transitions.push_back(10);
    assert(cycleService.RegisterNode(3, cycleB));
    assert(cycleService.ValidateStory(3));
    report = gameplay::progression::ProgressionPlayerStoryRunner::Run(cycleService, "Loop", context);
    assert(!report.Ok());
    assert(report.stepLimitHits == 1);
    assert(report.failures == 1);
    assert(report.advanceSteps == 64);
    return 0;
}
