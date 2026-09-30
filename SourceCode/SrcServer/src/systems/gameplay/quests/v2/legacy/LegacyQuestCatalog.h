#pragma once

#include "gameplay/quests/v2/QuestCatalog.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
class LegacyQuestCatalog final : public IQuestCatalog
{
public:
    bool TryGetDefinition(QuestId quest, QuestDefinition& out) const override;
};

const LegacyQuestCatalog& ServerLegacyQuestCatalog();
}
}
}
