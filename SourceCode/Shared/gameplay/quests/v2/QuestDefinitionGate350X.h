#pragma once
#include "gameplay/quests/v2/QuestCatalog.h"
#include "gameplay/quests/v2/QuestCommands.h"
namespace gameplay { namespace quests {
// Single authoritative definition check for QuestDB-backed V2 starts.
// A client cannot invent QuestKind/QuestType/PartyFlag to select another row.
struct QuestDefinitionGate350X {
    // All QuestDB families (daily, repeated and single) route by the persisted
    // ID+kind+type, not by untrusted client flags or hand-maintained ID tables.
    static bool MatchesVariant(const QuestCommand& request,
                               const QuestDefinition& definition) {
        return request.quest.Valid() && definition.id.Valid() &&
            request.quest.value==definition.id.value &&
            (definition.kind==21 || definition.kind==7 || definition.kind==9) &&
            request.kind==definition.kind && request.type==definition.type;
    }
    static bool MatchesStart(const QuestStartCommand& request,
                             const QuestDefinition& definition) {
        return request.quest.Valid() && definition.id.Valid() &&
            request.quest.value==definition.id.value &&
            (definition.kind==21 || definition.kind==7 || definition.kind==9) &&
            request.kind==definition.kind && request.type==definition.type &&
            request.partyFlag==definition.partyFlag;
    }
};
}}
