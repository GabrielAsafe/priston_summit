#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace quests
{
class IQuestAdminService
{
public:
    virtual ~IQuestAdminService() {}

    // Moderator/test-only operation used by the legacy /skiptier flow.
    virtual bool MarkCompletedForTesting(PlayerId player, QuestId quest) = 0;
};
}
}
