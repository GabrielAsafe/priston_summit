#pragma once

#include "gameplay/story/v2/IStoryRepository.h"

namespace gameplay
{
namespace story
{
class SQLiteStoryRepository : public IStoryRepository
{
public:
    bool Ready() { return EnsureSchema(); }
    bool LoadProgress(const std::string& playerKey, StoryId storyId, StoryProgressState& state);
    StoryProgressLoadResult LoadProgressResult(const std::string& playerKey, StoryId storyId, StoryProgressState& state);
    bool SaveProgress(const std::string& playerKey, const StoryProgressState& state);
    StoryMutationResult CreateInitialProgressIfAbsent(const std::string& playerKey,
                                                       const StoryProgressState& state);
    StoryMutationResult SaveProgressIfUnchanged(const std::string& playerKey,
                                                const StoryProgressState& expected,
                                                const StoryProgressState& state);
    StoryMutationResult TransitionIfUnchanged(const std::string& playerKey,
                                              const StoryProgressState& expected,
                                              const StoryProgressState& nextState);
    StoryMutationResult CompleteIfUnchanged(const std::string& playerKey,
                                            const StoryProgressState& expected,
                                            const StoryProgressState& completedState);
    bool GetNodeState(const std::string& playerKey,
                      StoryId storyId,
                      StoryNodeId nodeId,
                      StoryNodeStatus& state);
    bool SetNodeState(const std::string& playerKey,
                      StoryId storyId,
                      StoryNodeId nodeId,
                      StoryNodeStatus state);
    bool GetPlayerFlag(const std::string& playerKey, const std::string& flagKey, int& value);
    bool SetPlayerFlag(const std::string& playerKey, const std::string& flagKey, int value);
    bool GetWorldFlag(const std::string& flagKey, int& value);
    bool SetWorldFlag(const std::string& flagKey, int value);
    StoryEffectClaimResult ClaimEffect(const std::string& playerKey,
                                       StoryId storyId,
                                       StoryNodeId nodeId,
                                       const std::string& effectKey);
    bool MarkEffectApplied(const std::string& playerKey,
                           StoryId storyId,
                           StoryNodeId nodeId,
                           const std::string& effectKey);
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
    bool EnsureSchema();
};
}
}
