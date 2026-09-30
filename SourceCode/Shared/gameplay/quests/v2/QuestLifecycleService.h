#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace quests
{
class IQuestLifecycleService
{
public:
    virtual ~IQuestLifecycleService() {}

    // Revalidates active quests after a player-level change/login and applies
    // the legacy cancellation policy for definitions outside the level range.
    virtual void VerifyPlayerLevel(PlayerId player) = 0;
};
}
}
