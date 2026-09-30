#pragma once

#include "gameplay/quests/v2/QuestAdminService.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
class LegacyQuestAdminService final : public IQuestAdminService
{
public:
    bool MarkCompletedForTesting(PlayerId player, QuestId quest) override;
};

LegacyQuestAdminService& ServerLegacyQuestAdminService();
}
}
}
