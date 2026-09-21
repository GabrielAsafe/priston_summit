#pragma once

#include <map>
#include <string>
#include <vector>

#include "gameplay/conditions/v2/Condition.h"
#include "gameplay/rewards/v2/Reward.h"
#include "gameplay/events/v2/GameplayEventHub.h"
#include "gameplay/story/v2/IStoryRepository.h"
#include "gameplay/story/v2/StoryEvents.h"

namespace gameplay
{
namespace story
{
class IStoryCommandGateway
{
public:
    virtual ~IStoryCommandGateway() {}
    virtual bool Supports(StoryActionType) const { return true; }
    virtual bool ValidateAction(const StoryActionDefinition& action) const { return Supports(action.type); }
    virtual bool IsRetrySafe(StoryActionType) const { return false; }
    virtual bool IsRetrySafe(const StoryActionDefinition& action) const { return IsRetrySafe(action.type); }
    virtual bool StartQuest(const std::string& playerKey, int questId) = 0;
    virtual bool AddCovenantReputation(const std::string& playerKey,
                                       int covenantId,
                                       int amount,
                                       const std::string& sourceKey) = 0;
    virtual bool ExecuteCustomAction(const std::string& playerKey,
                                     const StoryActionDefinition& action,
                                     const std::string& sourceKey) = 0;
};

enum class StoryStartResult
{
    Started,
    AlreadyStarted,
    NotEligible,
    Failed
};

enum class StoryEvaluateResult
{
    Advanced,
    NoChange,
    Failed
};

enum class StoryChoiceResult
{
    Selected,
    NotAvailable,
    Conflict,
    Failed
};

class StoryService
{
public:
    StoryService(IStoryRepository& repository,
                 IStoryCommandGateway& commands,
                 rewards::RewardService& rewards,
                 events::GameplayEventHub* eventHub = 0);

    bool RegisterStory(const StoryDefinition& definition);
    bool RegisterArc(const StoryArcDefinition& definition);
    bool RegisterChapter(const StoryChapterDefinition& definition);
    bool RegisterNode(StoryId storyId, const StoryNodeDefinition& node);
    bool ValidateStory(StoryId storyId, std::string* error = 0) const;
    bool ValidateAllStories(std::string* error = 0) const;
    std::vector<StoryId> StoryIds() const;
    bool IsAutomaticStart(StoryId storyId) const;
    void GetCatalog(std::vector<StoryCatalogEntry>& entries) const;

    StoryStartResult StartDetailed(const std::string& playerKey,
                                   StoryId storyId,
                                   const conditions::ConditionContext& context);
    bool Start(const std::string& playerKey,
               StoryId storyId,
               const conditions::ConditionContext& context);
    StoryEvaluateResult EvaluateDetailed(const std::string& playerKey,
                                         StoryId storyId,
                                         const conditions::ConditionContext& context);
    bool Evaluate(const std::string& playerKey,
                  StoryId storyId,
                  const conditions::ConditionContext& context);
    StoryChoiceResult ChooseDialogueOption(const std::string& playerKey,
                                                StoryId storyId,
                                                const std::string& choiceKey,
                                                const conditions::ConditionContext& context);
    bool GetAvailableDialogueChoices(const std::string& playerKey,
                                     StoryId storyId,
                                     const conditions::ConditionContext& context,
                                     std::vector<std::string>& choiceKeys);
    bool GetAvailableDialogueChoiceSnapshots(
        const std::string& playerKey,
        StoryId storyId,
        const conditions::ConditionContext& context,
        std::vector<StoryDialogueChoiceSnapshot>& choices);
    StoryProgressLoadResult GetProgressResult(const std::string& playerKey,
                                              StoryId storyId,
                                              StoryProgressState& state);
    bool GetProgress(const std::string& playerKey,
                     StoryId storyId,
                     StoryProgressState& state);
    bool GetSnapshot(const std::string& playerKey,
                     StoryId storyId,
                     StorySnapshot& snapshot);
    bool GetStartedSnapshots(const std::string& playerKey,
                             std::vector<StorySnapshot>& snapshots);
    StoryEffectReceiptLoadResult GetEffectReceiptStatus(const std::string& playerKey,
                                                        StoryId storyId,
                                                        StoryNodeId nodeId,
                                                        const std::string& effectKey,
                                                        StoryEffectReceiptStatus& status);
    bool ListPendingEffects(const std::string& playerKey,
                            std::vector<StoryEffectReceiptEntry>& receipts);
    bool AcknowledgePendingEffect(const std::string& playerKey,
                                  StoryId storyId,
                                  StoryNodeId nodeId,
                                  const std::string& effectKey);

private:
    typedef std::map<StoryId, StoryDefinition> StoryMap;
    typedef std::map<StoryArcId, StoryArcDefinition> ArcMap;
    typedef std::map<StoryChapterId, StoryChapterDefinition> ChapterMap;
    typedef std::map<StoryNodeId, StoryNodeDefinition> NodeMap;
    typedef std::map<StoryId, ArcMap> StoryArcMap;
    typedef std::map<StoryId, ChapterMap> StoryChapterMap;
    typedef std::map<StoryId, NodeMap> StoryNodeMap;

    const StoryDefinition* FindStory(StoryId storyId) const;
    const StoryArcDefinition* FindArc(StoryId storyId, StoryArcId arcId) const;
    const StoryChapterDefinition* FindChapter(StoryId storyId, StoryChapterId chapterId) const;
    const StoryNodeDefinition* FindNode(StoryId storyId, StoryNodeId nodeId) const;
    bool BuildSnapshot(const StoryProgressState& progress, StorySnapshot& snapshot) const;
    bool CanEnterNode(StoryId storyId,
                      const StoryNodeDefinition& node,
                      const conditions::ConditionContext& context) const;
    bool ApplyActions(const std::string& playerKey,
                      StoryId storyId,
                      StoryNodeId nodeId,
                      const std::vector<StoryActionDefinition>& actions,
                      const char* phase);
    bool IsActionRetrySafe(const StoryActionDefinition& action) const;
    bool ApplyRewards(const std::string& playerKey,
                      StoryId storyId,
                      StoryNodeId nodeId,
                      const std::vector<rewards::RewardDefinition>& rewards);
    bool EnsureEnterActions(const std::string& playerKey,
                            StoryId storyId,
                            const StoryNodeDefinition& node,
                            StoryProgressState& state);
    bool EnsureCompletionActions(const std::string& playerKey,
                                 StoryId storyId,
                                 const StoryNodeDefinition& node,
                                 StoryProgressState& state);
    enum class TransitionAttemptResult
    {
        Transitioned,
        NoEligibleTarget,
        Conflict,
        Failed
    };
    TransitionAttemptResult TryTransition(const std::string& playerKey,
                                          StoryId storyId,
                                          const StoryNodeDefinition& node,
                                          StoryProgressState& state,
                                          const conditions::ConditionContext& context);
    std::string SourceKey(StoryId storyId, StoryNodeId nodeId, const char* phase) const;
    std::string IndexedEffectKey(const char* phase, std::size_t index) const;

    IStoryRepository& repository_;
    IStoryCommandGateway& commands_;
    rewards::RewardService& rewards_;
    events::GameplayEventHub* eventHub_;
    StoryMap stories_;
    StoryArcMap arcs_;
    StoryChapterMap chapters_;
    StoryNodeMap nodes_;
};
}
}
