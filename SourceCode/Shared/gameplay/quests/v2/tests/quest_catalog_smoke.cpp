#include <cassert>

#include "gameplay/quests/v2/QuestCatalog.h"

namespace
{
class FakeCatalog final : public gameplay::quests::IQuestCatalog
{
public:
    bool TryGetDefinition(gameplay::QuestId quest, gameplay::quests::QuestDefinition& out) const override
    {
        if(!quest.Valid()) return false;
        out.id = quest;
        out.kind = 9;
        out.type = 4;
        out.partyFlag = 1;
        return true;
    }
};
}

int main()
{
    FakeCatalog catalog;
    gameplay::quests::QuestDefinition definition;
    assert(catalog.TryGetDefinition(gameplay::QuestId(77u), definition));
    assert(definition.id.value == 77u);
    assert(definition.kind == 9);
    assert(definition.type == 4);
    assert(definition.partyFlag == 1);
    assert(!catalog.TryGetDefinition(gameplay::QuestId(), definition));
    return 0;
}
