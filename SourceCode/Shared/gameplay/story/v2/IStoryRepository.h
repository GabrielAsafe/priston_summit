#pragma once

#include <string>
#include <vector>

#include "gameplay/story/v2/StoryTypes.h"

namespace gameplay
{
namespace story
{
enum class StoryProgressLoadResult
{
    Found,
    NotFound,
    Failed
};

enum class StoryEffectClaimResult
{
    Claimed,
    AlreadyApplied,
    Pending,
    Failed
};

enum class StoryMutationResult
{
    Applied,
    AlreadyExists,
    Conflict,
    Failed
};

enum class StoryEffectReceiptStatus
{
    Pending,
    Applied
};

enum class StoryEffectReceiptLoadResult
{
    Found,
    NotFound,
    Failed
};

struct StoryEffectReceiptEntry
{
    StoryId storyId;
    StoryNodeId nodeId;
    std::string effectKey;
    StoryEffectReceiptStatus status;

    StoryEffectReceiptEntry()
        : storyId(0), nodeId(0), status(StoryEffectReceiptStatus::Pending)
    {
    }
};

class IStoryRepository
{
public:
    virtual ~IStoryRepository() {}
    virtual bool LoadProgress(const std::string& playerKey,
                              StoryId storyId,
                              StoryProgressState& state) = 0;
    virtual StoryProgressLoadResult LoadProgressResult(const std::string& playerKey,
                                                       StoryId storyId,
                                                       StoryProgressState& state)
    {
        return LoadProgress(playerKey, storyId, state) ? StoryProgressLoadResult::Found
                                                        : StoryProgressLoadResult::NotFound;
    }
    virtual bool SaveProgress(const std::string& playerKey,
                              const StoryProgressState& state) = 0;
    virtual StoryMutationResult CreateInitialProgressIfAbsent(
        const std::string& playerKey,
        const StoryProgressState& state)
    {
        StoryProgressState existing;
        const StoryProgressLoadResult loaded = LoadProgressResult(playerKey, state.storyId, existing);
        if (loaded == StoryProgressLoadResult::Found)
            return StoryMutationResult::AlreadyExists;
        if (loaded == StoryProgressLoadResult::Failed)
            return StoryMutationResult::Failed;
        if (!SetNodeState(playerKey, state.storyId, state.nodeId, StoryNodeStatus::Active) ||
            !SaveProgress(playerKey, state))
            return StoryMutationResult::Failed;
        return StoryMutationResult::Applied;
    }
    virtual StoryMutationResult SaveProgressIfUnchanged(
        const std::string& playerKey,
        const StoryProgressState& expected,
        const StoryProgressState& state)
    {
        StoryProgressState current;
        if (LoadProgressResult(playerKey, expected.storyId, current) != StoryProgressLoadResult::Found)
            return StoryMutationResult::Failed;
        if (current.chapterId != expected.chapterId || current.nodeId != expected.nodeId ||
            current.status != expected.status ||
            current.enterActionsApplied != expected.enterActionsApplied ||
            current.completionActionsApplied != expected.completionActionsApplied)
            return StoryMutationResult::Conflict;
        return SaveProgress(playerKey, state) ? StoryMutationResult::Applied
                                               : StoryMutationResult::Failed;
    }
    virtual StoryMutationResult TransitionIfUnchanged(
        const std::string& playerKey,
        const StoryProgressState& expected,
        const StoryProgressState& nextState)
    {
        StoryProgressState current;
        if (LoadProgressResult(playerKey, expected.storyId, current) != StoryProgressLoadResult::Found)
            return StoryMutationResult::Failed;
        if (current.chapterId != expected.chapterId || current.nodeId != expected.nodeId ||
            current.status != expected.status ||
            current.enterActionsApplied != expected.enterActionsApplied ||
            current.completionActionsApplied != expected.completionActionsApplied)
            return StoryMutationResult::Conflict;
        if (!SetNodeState(playerKey, expected.storyId, expected.nodeId, StoryNodeStatus::Completed) ||
            !SetNodeState(playerKey, nextState.storyId, nextState.nodeId, StoryNodeStatus::Active) ||
            !SaveProgress(playerKey, nextState))
            return StoryMutationResult::Failed;
        return StoryMutationResult::Applied;
    }
    virtual StoryMutationResult CompleteIfUnchanged(
        const std::string& playerKey,
        const StoryProgressState& expected,
        const StoryProgressState& completedState)
    {
        StoryProgressState current;
        if (LoadProgressResult(playerKey, expected.storyId, current) != StoryProgressLoadResult::Found)
            return StoryMutationResult::Failed;
        if (current.chapterId != expected.chapterId || current.nodeId != expected.nodeId ||
            current.status != expected.status ||
            current.enterActionsApplied != expected.enterActionsApplied ||
            current.completionActionsApplied != expected.completionActionsApplied)
            return StoryMutationResult::Conflict;
        if (!SetNodeState(playerKey, expected.storyId, expected.nodeId, StoryNodeStatus::Completed) ||
            !SaveProgress(playerKey, completedState))
            return StoryMutationResult::Failed;
        return StoryMutationResult::Applied;
    }
    virtual bool GetNodeState(const std::string& playerKey,
                              StoryId storyId,
                              StoryNodeId nodeId,
                              StoryNodeStatus& state) = 0;
    virtual bool SetNodeState(const std::string& playerKey,
                              StoryId storyId,
                              StoryNodeId nodeId,
                              StoryNodeStatus state) = 0;
    virtual bool GetPlayerFlag(const std::string& playerKey,
                               const std::string& flagKey,
                               int& value) = 0;
    virtual bool SetPlayerFlag(const std::string& playerKey,
                               const std::string& flagKey,
                               int value) = 0;
    virtual bool GetWorldFlag(const std::string& flagKey, int& value) = 0;
    virtual bool SetWorldFlag(const std::string& flagKey, int value) = 0;

    virtual StoryEffectClaimResult ClaimEffect(const std::string& playerKey,
                                               StoryId storyId,
                                               StoryNodeId nodeId,
                                               const std::string& effectKey) = 0;
    virtual bool MarkEffectApplied(const std::string& playerKey,
                                   StoryId storyId,
                                   StoryNodeId nodeId,
                                   const std::string& effectKey) = 0;
    virtual StoryEffectReceiptLoadResult GetEffectReceiptStatus(
        const std::string& playerKey,
        StoryId storyId,
        StoryNodeId nodeId,
        const std::string& effectKey,
        StoryEffectReceiptStatus& status)
    {
        (void)playerKey;
        (void)storyId;
        (void)nodeId;
        (void)effectKey;
        (void)status;
        return StoryEffectReceiptLoadResult::Failed;
    }
    virtual bool ListPendingEffects(const std::string& playerKey,
                                    std::vector<StoryEffectReceiptEntry>& receipts)
    {
        (void)playerKey;
        receipts.clear();
        return false;
    }
    virtual bool AcknowledgePendingEffect(const std::string& playerKey,
                                          StoryId storyId,
                                          StoryNodeId nodeId,
                                          const std::string& effectKey)
    {
        StoryEffectReceiptStatus status = StoryEffectReceiptStatus::Pending;
        const StoryEffectReceiptLoadResult loaded =
            GetEffectReceiptStatus(playerKey, storyId, nodeId, effectKey, status);
        if (loaded != StoryEffectReceiptLoadResult::Found)
            return false;
        if (status == StoryEffectReceiptStatus::Applied)
            return true;
        return MarkEffectApplied(playerKey, storyId, nodeId, effectKey);
    }
};
}
}
