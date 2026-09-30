#include "systems/gameplay/progression/v2/ProgressionTriggerRuntime.h"

#include <mutex>
#include <vector>

#include "gameplay/progression/v2/ProgressionTriggerCoordinator.h"
#include "gameplay/covenants/v2/CovenantEvents.h"
#include "gameplay/reputation/v2/ReputationEvents.h"
#include "gameplay/story/v2/StoryEvents.h"
#include "systems/gameplay/events/v2/GameplayEventRuntime.h"
#include "systems/gameplay/player/v2/legacy/LegacyPlayerAdapter.h"
#include "systems/gameplay/progression/v2/ProgressionRuntime.h"

namespace gameplay
{
namespace progression
{
namespace
{
ProgressionTriggerCoordinator& WorldProcessingCoordinator()
{
    static ProgressionTriggerCoordinator coordinator;
    return coordinator;
}

std::recursive_mutex& WorldProcessingMutex()
{
    static std::recursive_mutex mutex;
    return mutex;
}

bool ProcessSnapshot(const player::PlayerSnapshot& snapshot)
{
    if (!snapshot.online || snapshot.playerKey.empty() || !IsServerProgressionRuntimeReady())
        return false;

    return ProcessServerProgressionPlayer(
        snapshot.playerKey, snapshot.level, snapshot.legacyJobCode).Ok();
}

void NotifyProgressionPlayer(const std::string& playerKey)
{
    player::PlayerSnapshot snapshot;
    if (player::legacy::ServerLegacyPlayerAdapter().TryGetByKey(playerKey, snapshot))
        (void)ProcessSnapshot(snapshot);
}
}

bool ProcessServerProgressionOnlinePlayer(PlayerId playerId)
{
    player::PlayerSnapshot snapshot;
    if (!player::legacy::ServerLegacyPlayerAdapter().TryGet(playerId, snapshot))
        return false;
    return ProcessSnapshot(snapshot);
}

void NotifyServerProgressionPlayerStateChanged(PlayerId player)
{
    (void)ProcessServerProgressionOnlinePlayer(player);
}

void NotifyServerProgressionWorldStateChanged()
{
    if (!IsServerProgressionRuntimeReady())
        return;

    std::lock_guard<std::recursive_mutex> lock(WorldProcessingMutex());
    (void)WorldProcessingCoordinator().Request("__world__", []()
    {
        bool ok = true;
        const std::vector<PlayerHandle> handles =
            player::legacy::ServerLegacyOnlinePlayerRegistry().OnlineHandles();
        for (std::vector<PlayerHandle>::const_iterator it = handles.begin(); it != handles.end(); ++it)
        {
            player::PlayerSnapshot snapshot;
            if (!player::legacy::ServerLegacyPlayerAdapter().TryGet(*it, snapshot) ||
                !ProcessSnapshot(snapshot))
            {
                ok = false;
            }
        }
        return ok;
    });
}

void RegisterServerProgressionEventTriggers()
{
    static const events::GameplayEventHub::SubscriptionId worldFlagSubscription =
        events::ServerGameplayEventHub().Subscribe<story::StoryWorldFlagChangedEvent>(
            [](const story::StoryWorldFlagChangedEvent&)
            {
                NotifyServerProgressionWorldStateChanged();
            });
    static const events::GameplayEventHub::SubscriptionId covenantJoinedSubscription =
        events::ServerGameplayEventHub().Subscribe<covenants::CovenantJoinedEvent>(
            [](const covenants::CovenantJoinedEvent& event)
            {
                NotifyProgressionPlayer(event.playerKey);
            });
    static const events::GameplayEventHub::SubscriptionId covenantLeftSubscription =
        events::ServerGameplayEventHub().Subscribe<covenants::CovenantLeftEvent>(
            [](const covenants::CovenantLeftEvent& event)
            {
                NotifyProgressionPlayer(event.playerKey);
            });
    static const events::GameplayEventHub::SubscriptionId covenantReputationSubscription =
        events::ServerGameplayEventHub().Subscribe<covenants::CovenantReputationChangedEvent>(
            [](const covenants::CovenantReputationChangedEvent& event)
            {
                NotifyProgressionPlayer(event.playerKey);
            });
    static const events::GameplayEventHub::SubscriptionId covenantRankSubscription =
        events::ServerGameplayEventHub().Subscribe<covenants::CovenantRankChangedEvent>(
            [](const covenants::CovenantRankChangedEvent& event)
            {
                NotifyProgressionPlayer(event.playerKey);
            });

    static const events::GameplayEventHub::SubscriptionId reputationSubscription =
        events::ServerGameplayEventHub().Subscribe<reputation::ReputationChangedEvent>(
            [](const reputation::ReputationChangedEvent& event)
            {
                NotifyProgressionPlayer(event.playerKey);
            });

    (void)worldFlagSubscription;
    (void)covenantJoinedSubscription;
    (void)covenantLeftSubscription;
    (void)covenantReputationSubscription;
    (void)covenantRankSubscription;
    (void)reputationSubscription;
}
}
}
