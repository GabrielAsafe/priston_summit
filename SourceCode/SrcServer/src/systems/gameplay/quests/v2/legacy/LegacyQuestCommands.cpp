#include "systems/gameplay/quests/v2/legacy/LegacyQuestCommands.h"

#include "systems/gameplay/quests/server/cQuestHandler.h"
// Complete rsPLAYINFO definition (350X only saw the forward declaration).
// Same includes as LegacyQuestLifecycleService.cpp / LegacyPlayerAdapter.cpp.
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/gameplay/quests/v2/legacy/LegacyQuestCatalog.h"
#include "systems/gameplay/quests/v2/legacy/LegacyQuestRepository.h"
#include "gameplay/quests/v2/QuestDefinitionGate350X.h"
#include "gameplay/quests/v2/QuestLegacySource350Z.h"
#include "gameplay/quests/v2/QuestCanonicalRegistry351B.h"
#include "systems/gameplay/quests/v2/legacy/CanonicalQuestExecution351C.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
bool LegacyQuestCommands::Start(PlayerId player, const QuestStartCommand& command)
{
    if(canonical::IsCanonicalQuest351B(command.quest)) {
        const canonical::Entry351B* e=canonical::FindCanonicalQuest351B(command.quest);
        rsPLAYINFO* p=players_.Resolve(player);
        if(!e||!p||!p->getName()||!p->getName()[0] ||
           command.kind!=canonical::Kind351B(*e) || command.type!=0 || command.partyFlag!=0)
            return false;
        return canonical::StartKills351C(p->getName(),*e,
                p->smCharInfo.Level,p->smCharInfo.JOB_CODE,std::time(0));
    }
    // A character-record quest MUST NOT be sent to the SQL quest handler.
    // Legacy entrypoints continue to own stages, rewards and saving until
    // the state-machine adapters are migrated with equivalent tests.
    if(IsLegacyNamespacedQuest350Z(command.quest) ||
       canonical::IsCanonicalQuest351B(command.quest)) return false;
    rsPLAYINFO* resolved = players_.Resolve(player);
    if(!resolved || !command.quest.Valid() || !resolved->getName() ||
       !resolved->getName()[0]) return false;

    // 350X: a request is NOT a definition. Use the authoritative QuestDB
    // catalog for every DB-backed quest, including future/legacy rows.
    QuestDefinition definition;
    if(!ServerLegacyQuestCatalog().TryGetDefinition(command.quest, definition) ||
       !QuestDefinitionGate350X::MatchesStart(command, definition)) return false;

    cQuestHandler.StartQuest(resolved,
                             static_cast<int>(command.quest.value),
                             definition.kind, definition.type,
                             definition.partyFlag);
    // Legacy StartQuest returns void (and can silently refuse a rank gate).
    // Only report success if it REALLY created/retained active progress.
    bool started=false;
    return ServerLegacyQuestRepository().TryIsInProgress(
        resolved->getName(),command.quest,started) && started;
}

bool LegacyQuestCommands::Cancel(PlayerId player, const QuestCommand& command)
{
    if(canonical::IsCanonicalQuest351B(command.quest)) {
        const canonical::Entry351B* e=canonical::FindCanonicalQuest351B(command.quest);
        rsPLAYINFO* p=players_.Resolve(player);
        if(!e||!p||!p->getName()||!p->getName()[0] ||
           command.kind!=canonical::Kind351B(*e) || command.type!=0)return false;
        return canonical::CancelKills351C(p->getName(),*e,std::time(0));
    }
    // A character-record quest MUST NOT be sent to the SQL quest handler.
    // Legacy entrypoints continue to own stages, rewards and saving until
    // the state-machine adapters are migrated with equivalent tests.
    if(IsLegacyNamespacedQuest350Z(command.quest) ||
       canonical::IsCanonicalQuest351B(command.quest)) return false;
    rsPLAYINFO* resolved = players_.Resolve(player);
    if(!resolved || !command.quest.Valid() || !resolved->getName() ||
       !resolved->getName()[0]) return false;

    QuestDefinition definition;
    if(!ServerLegacyQuestCatalog().TryGetDefinition(command.quest, definition) ||
       !QuestDefinitionGate350X::MatchesVariant(command, definition)) return false;

    const char* playerKey = resolved->getName();
    bool before = false;
    if(!ServerLegacyQuestRepository().TryIsInProgress(playerKey, command.quest, before) ||
       !before) return false;

    cQuestHandler.CancelQuest(resolved,
                              static_cast<int>(command.quest.value),
                              definition.kind, definition.type);
    // A legacy CancelQuest returns void; never claim success for a no-op.
    bool after = true;
    return ServerLegacyQuestRepository().TryIsInProgress(playerKey,command.quest,after) &&
           !after;
}

bool LegacyQuestCommands::ClaimReward(PlayerId player, const QuestCommand& command)
{
    // A character-record quest MUST NOT be sent to the SQL quest handler.
    // Legacy entrypoints continue to own stages, rewards and saving until
    // the state-machine adapters are migrated with equivalent tests.
    if(IsLegacyNamespacedQuest350Z(command.quest) ||
       canonical::IsCanonicalQuest351B(command.quest)) return false;
    rsPLAYINFO* resolved = players_.Resolve(player);
    if(!resolved || !command.quest.Valid() || !resolved->getName() ||
       !resolved->getName()[0]) return false;

    QuestDefinition definition;
    if(!ServerLegacyQuestCatalog().TryGetDefinition(command.quest, definition) ||
       !QuestDefinitionGate350X::MatchesVariant(command, definition)) return false;

    const char* playerKey = resolved->getName();
    bool before = false;
    if(!ServerLegacyQuestRepository().TryIsInProgress(playerKey,command.quest,before) ||
       !before) return false;

    cQuestHandler.GetReward(resolved,
                            static_cast<int>(command.quest.value),
                            definition.kind, definition.type);
    // No completion is reported unless the legacy repository persisted it.
    bool completed = false;
    bool after = true;
    return ServerLegacyQuestRepository().TryIsCompletedVariant(
               playerKey,command.quest,definition.kind,definition.type,completed) &&
           completed &&
           ServerLegacyQuestRepository().TryIsInProgress(playerKey,command.quest,after) &&
           !after;
}

LegacyQuestCommands& ServerLegacyQuestCommands()
{
    static LegacyQuestCommands commands(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return commands;
}
}
}
}
