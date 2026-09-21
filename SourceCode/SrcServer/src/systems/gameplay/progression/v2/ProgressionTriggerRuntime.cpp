#include "systems/gameplay/progression/v2/ProgressionTriggerRuntime.h"

#include <cctype>
#include <cstring>
#include <mutex>

#include "core/server/legacy_runtime/onserver.h"
#include "gameplay/progression/v2/ProgressionTriggerCoordinator.h"
#include "gameplay/covenants/v2/CovenantEvents.h"
#include "gameplay/reputation/v2/ReputationEvents.h"
#include "gameplay/story/v2/StoryEvents.h"
#include "systems/gameplay/events/v2/GameplayEventRuntime.h"
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

bool SamePlayerName(const char* left, const std::string& right)
{
    if (!left || std::strlen(left) != right.size())
        return false;
    for (std::size_t i = 0; i < right.size(); ++i)
    {
        const unsigned char a = static_cast<unsigned char>(left[i]);
        const unsigned char b = static_cast<unsigned char>(right[i]);
        if (std::tolower(a) != std::tolower(b))
            return false;
    }
    return true;
}

rsPLAYINFO* FindOnlinePlayerByName(const std::string& playerKey)
{
    if (playerKey.empty())
        return 0;
    for (int i = 0; i < CONNECTMAX; ++i)
    {
        rsPLAYINFO* player = &rsPlayInfo[i];
        if (player->lpsmSock && SamePlayerName(player->getName(), playerKey))
            return player;
    }
    return 0;
}

void NotifyProgressionPlayer(const std::string& playerKey)
{
    rsPLAYINFO* player = FindOnlinePlayerByName(playerKey);
    if (player)
        NotifyServerProgressionPlayerStateChanged(player);
}
}
bool ProcessServerProgressionOnlinePlayer(rsPLAYINFO* player)
{
    if (!player || !player->lpsmSock || !IsServerProgressionRuntimeReady())
        return false;

    const char* name = player->getName();
    if (!name || !name[0])
        return false;

    const std::string playerKey(name);
    const int playerLevel = player->smCharInfo.Level;
    const int legacyJobCode = player->smCharInfo.JOB_CODE;
    return ProcessServerProgressionPlayer(playerKey, playerLevel, legacyJobCode).Ok();
}

void NotifyServerProgressionPlayerStateChanged(rsPLAYINFO* player)
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
        for (int i = 0; i < CONNECTMAX; ++i)
        {
            rsPLAYINFO* player = &rsPlayInfo[i];
            if (player->lpsmSock && !ProcessServerProgressionOnlinePlayer(player))
                ok = false;
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
