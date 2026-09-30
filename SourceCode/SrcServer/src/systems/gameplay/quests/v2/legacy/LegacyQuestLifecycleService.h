#pragma once

#include "gameplay/quests/v2/QuestLifecycleService.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
class LegacyQuestLifecycleService final : public IQuestLifecycleService
{
public:
    void VerifyPlayerLevel(PlayerId player) override;
};

LegacyQuestLifecycleService& ServerLegacyQuestLifecycleService();
}
}
}
