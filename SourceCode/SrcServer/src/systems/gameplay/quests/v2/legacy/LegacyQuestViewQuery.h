#pragma once

#include "gameplay/quests/v2/QuestViewQuery.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
class LegacyQuestViewQuery final : public IQuestViewQuery
{
public:
    bool TryGetDetail(const std::string& playerKey,
                      QuestId quest,
                      int questKind,
                      QuestDetailView& out) const override;
    bool TryGetSpoilerBatches(const std::string& playerKey,
                              int playerLevel,
                              int spoilerNumber,
                              std::vector<QuestSpoilerBatchView>& out) const override;
};

const LegacyQuestViewQuery& ServerLegacyQuestViewQuery();
}
}
}
