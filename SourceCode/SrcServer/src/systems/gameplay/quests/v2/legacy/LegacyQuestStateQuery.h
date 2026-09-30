#pragma once

#include "gameplay/quests/v2/QuestStateQuery.h"
#include "gameplay/quests/v2/IQuestRepository.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
class LegacyQuestStateQuery final : public IQuestStateQuery
{
public:
    explicit LegacyQuestStateQuery(const IQuestRepository& repository)
        : repository_(repository) {}

    bool TryGetState(const std::string& playerKey,
                     QuestId quest,
                     QuestStateSnapshot& out) const override;

private:
    const IQuestRepository& repository_;
};

const LegacyQuestStateQuery& ServerLegacyQuestStateQuery();
}
}
}
