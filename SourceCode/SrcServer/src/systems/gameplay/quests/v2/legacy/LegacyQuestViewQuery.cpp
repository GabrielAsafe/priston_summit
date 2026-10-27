#include "systems/gameplay/quests/v2/legacy/LegacyQuestViewQuery.h"
#include "gameplay/quests/v2/QuestCanonicalRegistry351B.h"
#include "systems/gameplay/quests/v2/legacy/CanonicalQuestExecution351C.h"
#include <ctime>

#include <cstring>
#include <memory>
#include <set>
#include <vector>

#include "systems/persistence/database/core/Database.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
namespace
{
const std::size_t kLegacyQuestFullEntryMax = 10u; // Mirrors Shared/skillsPacket.h::QUEST_FULL_ENTRY_MAX.

const char* DefinitionSql(int questKind)
{
    switch(questKind)
    {
    case 21: return "SELECT * FROM RepeatedQuest WHERE QuestID=?";
    case 7: return "SELECT * FROM DailyQuest WHERE QuestID=?";
    case 9: return "SELECT * FROM SingleQuest WHERE QuestID=?";
    default: return 0;
    }
}

bool ValidKey(const std::string& playerKey, QuestId quest)
{
    return !playerKey.empty() && playerKey.size() < 32u && quest.Valid();
}

struct ObjectiveDefinition
{
    char name[32];
    int required;
    int map;

    ObjectiveDefinition() : name(), required(0), map(0) {}
};
}

bool LegacyQuestViewQuery::TryGetDetail(const std::string& playerKey,
                                        QuestId quest,
                                        int questKind,
                                        QuestDetailView& out) const
{
    out = QuestDetailView();
    // Reference-only metadata. Never imitate progress, payout or monster kills.
    // Canonical IDs are not QuestDB IDs; look them up before any SQL.
    if(canonical::IsCanonicalQuest351B(quest))
    {
        if(playerKey.empty() || playerKey.size() >= 32u) return false;
        const canonical::Entry351B* entry=canonical::FindCanonicalQuest351B(quest);
        if(!entry || questKind!=canonical::Kind351B(*entry)) return false;
        out.quest=quest;
        out.kind=questKind;
        out.name=entry->name;
        out.title=entry->name;
        out.levelMin=entry->minLevel;
        out.levelMax=entry->maxLevel;
        out.description="CATALOGO DE REFERENCIA (nao ativa): etapas e recompensas "
                        "disponiveis apenas no registo canonico V2.";
        // Reward values remain deliberately unset: no grant from reference text.
        if(canonical::Enabled351C()) {
            canonical::KillStatus351C state;
            if(canonical::ReadKills351C(playerKey,*entry,std::time(0),state) && state.started) {
                const canonical::KillObjective351C* base=0;
                const unsigned int count=canonical::ObjectivesFor351C(entry->id,base);
                for(unsigned int i=0;i<count;++i) {
                    QuestObjectiveView obj;
                    obj.monsterName=base[i].target;
                    obj.required=base[i].needed;
                    obj.killed=state.counts[i];
                    out.objectives.push_back(obj);
                }
                out.description=state.ready
                    ? "Objetivos completos; recompensa ainda requer validacao no NPC legado."
                    : "Progresso V2 persistido; recompensa nao concedida automaticamente.";
            }
        }
        return true;
    }
    const char* definitionSql = DefinitionSql(questKind);
    if(!definitionSql || !ValidKey(playerKey, quest))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    int questId = static_cast<int>(quest.value);
    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());

    char questName[128] = {};
    char questTitle[128] = {};
    char questDescription[500] = {};
    int levelMin = 0;
    int levelMax = 0;
    int partyFlag = 0;
    int questType = 0;

    const bool definitionRead = db->Prepare(definitionSql) &&
        db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
        db->Execute() &&
        db->GetData(&questId, 4, 1, PARAMTYPE_Integer) &&
        db->GetData(questName, 128, 2, PARAMTYPE_String) &&
        db->GetData(questTitle, 128, 3, PARAMTYPE_String) &&
        db->GetData(&levelMin, 4, 4, PARAMTYPE_Integer) &&
        db->GetData(&levelMax, 4, 5, PARAMTYPE_Integer) &&
        db->GetData(&partyFlag, 4, 6, PARAMTYPE_Integer) &&
        db->GetData(questDescription, 500, 7, PARAMTYPE_String) &&
        db->GetData(&questType, 4, 8, PARAMTYPE_Integer);

    if(!definitionRead)
    {
        db->Close();
        return false;
    }

    out.quest = QuestId(static_cast<std::uint32_t>(questId));
    out.kind = questKind;
    out.type = questType;
    out.name = questName;
    out.title = questTitle;
    out.description = questDescription;
    out.levelMin = levelMin;
    out.levelMax = levelMax;
    out.partyFlag = partyFlag;

    ObjectiveDefinition defs[kLegacyQuestFullEntryMax];
    int defCount = 0;
    if(db->Prepare("SELECT * FROM QuestMonsters WHERE QuestID=? AND QuestKind=? AND QuestType=?") &&
       db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
       db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer) &&
       db->BindInputParameter(&questType, 3, PARAMTYPE_Integer) &&
       db->Execute(false))
    {
        while(db->NextRow() && static_cast<std::size_t>(defCount) < kLegacyQuestFullEntryMax)
        {
            db->GetData(defs[defCount].name, 32, 3, PARAMTYPE_String);
            db->GetData(&defs[defCount].required, 4, 4, PARAMTYPE_Integer);
            db->GetData(&defs[defCount].map, 4, 6, PARAMTYPE_Integer);
            ++defCount;
        }
    }

    for(int i = 0; i < defCount; ++i)
    {
        QuestObjectiveView objective;
        objective.monsterName = defs[i].name;
        objective.required = defs[i].required;
        objective.map = defs[i].map;

        if(db->Prepare("SELECT * FROM MonsterKilled WHERE PlayerName=? AND MonsterName=? AND QuestType=? AND QuestKind=? AND QuestID=? AND QuestMap=?") &&
           db->BindInputParameter(playerName, 1, PARAMTYPE_String) &&
           db->BindInputParameter(defs[i].name, 2, PARAMTYPE_String) &&
           db->BindInputParameter(&questType, 3, PARAMTYPE_Integer) &&
           db->BindInputParameter(&questKind, 4, PARAMTYPE_Integer) &&
           db->BindInputParameter(&questId, 5, PARAMTYPE_Integer) &&
           db->BindInputParameter(&defs[i].map, 6, PARAMTYPE_Integer) &&
           db->Execute())
        {
            db->GetData(&objective.killed, 4, 2, PARAMTYPE_Integer);
        }
        out.objectives.push_back(objective);
    }

    if(db->Prepare("SELECT * FROM Rewards WHERE QuestID=? AND QuestKind=? AND QuestType=?") &&
       db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
       db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer) &&
       db->BindInputParameter(&questType, 3, PARAMTYPE_Integer) &&
       db->Execute())
    {
        db->GetData(&out.rewardGold, 4, 3, PARAMTYPE_Integer);
        db->GetData(&out.rewardExperience, 8, 4, PARAMTYPE_Int64);
        db->GetData(&out.rewardCoin, 4, 6, PARAMTYPE_Integer);
        db->GetData(&out.rewardTCoin, 4, 7, PARAMTYPE_Integer);
    }

    if(db->Prepare("SELECT * FROM ItemRewards WHERE QuestID=? AND QuestKind=? AND QuestType=?") &&
       db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
       db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer) &&
       db->BindInputParameter(&questType, 3, PARAMTYPE_Integer) &&
       db->Execute(false))
    {
        while(db->NextRow() && out.itemRewards.size() < kLegacyQuestFullEntryMax)
        {
            char contentKey[32] = {};
            QuestItemRewardView item;
            db->GetData(contentKey, 32, 3, PARAMTYPE_String);
            db->GetData(&item.count, 4, 4, PARAMTYPE_Integer);
            item.contentKey = contentKey;
            out.itemRewards.push_back(item);
        }
    }

    db->Close();
    return true;
}


namespace
{
const char* LevelQuerySql(int questKind, bool inRange)
{
    switch(questKind)
    {
    case 21:
        return inRange
            ? "SELECT * FROM RepeatedQuest WHERE LevelMin <= ? AND LevelMax >= ? ORDER BY QuestName ASC"
            : "SELECT * FROM RepeatedQuest WHERE LevelMin > ? OR LevelMax < ? ORDER BY QuestName ASC";
    case 7:
        return inRange
            ? "SELECT * FROM DailyQuest WHERE LevelMin <= ? AND LevelMax >= ? ORDER BY QuestName ASC"
            : "SELECT * FROM DailyQuest WHERE LevelMin > ? OR LevelMax < ? ORDER BY QuestName ASC";
    case 9:
        return inRange
            ? "SELECT * FROM SingleQuest WHERE LevelMin <= ? AND LevelMax >= ? ORDER BY QuestName ASC"
            : "SELECT * FROM SingleQuest WHERE LevelMin > ? OR LevelMax < ? ORDER BY QuestName ASC";
    default:
        return 0;
    }
}

const char* DefinitionByIdTypeSql(int questKind)
{
    switch(questKind)
    {
    case 21: return "SELECT * FROM RepeatedQuest WHERE QuestID=? AND QuestType=? ORDER BY QuestName ASC";
    case 7: return "SELECT * FROM DailyQuest WHERE QuestID=? AND QuestType=? ORDER BY QuestName ASC";
    case 9: return "SELECT * FROM SingleQuest WHERE QuestID=? AND QuestType=? ORDER BY QuestName ASC";
    default: return 0;
    }
}

void LoadSeenValues(const std::shared_ptr<SQLConnection>& db,
                    const char* sql,
                    const char* playerName,
                    int questKind,
                    int column,
                    std::set<int>& seen)
{
    if(!db || !sql || !playerName)
        return;
    if(!db->Prepare(sql) ||
       !db->BindInputParameter(&questKind, 1, PARAMTYPE_Integer) ||
       !db->BindInputParameter(const_cast<char*>(playerName), 2, PARAMTYPE_String) ||
       !db->Execute(false))
        return;

    while(db->NextRow())
    {
        int value = -1;
        if(db->GetData(&value, 4, column, PARAMTYPE_Integer))
            seen.insert(value);
    }
}

bool LoadLevelBatch(const std::shared_ptr<SQLConnection>& db,
                    int questKind,
                    int playerLevel,
                    bool inRange,
                    bool matchByQuestId,
                    const std::set<int>& seen,
                    QuestSpoilerBatchView& batch)
{
    const char* sql = LevelQuerySql(questKind, inRange);
    if(!db || !sql || !db->Prepare(sql) ||
       !db->BindInputParameter(&playerLevel, 1, PARAMTYPE_Integer) ||
       !db->BindInputParameter(&playerLevel, 2, PARAMTYPE_Integer) ||
       !db->Execute(false))
        return false;

    while(db->NextRow())
    {
        int questId = 0;
        int questType = 0;
        char questName[128] = {};
        db->GetData(&questId, 4, 1, PARAMTYPE_Integer);
        db->GetData(questName, 128, 2, PARAMTYPE_String);
        db->GetData(&questType, 4, 8, PARAMTYPE_Integer);

        QuestSpoilerEntryView entry;
        entry.quest = QuestId(static_cast<std::uint32_t>(questId));
        entry.kind = questKind;
        entry.name = questName;
        entry.active = inRange;
        if(entry.active)
        {
            const int key = matchByQuestId ? questId : questType;
            if(seen.find(key) != seen.end())
                entry.active = false;
        }
        batch.entries.push_back(entry);
    }
    return true;
}

struct QuestRef
{
    int questId;
    int questKind;
    int questType;

    QuestRef() : questId(0), questKind(0), questType(0) {}
};

void LoadQuestRefs(const std::shared_ptr<SQLConnection>& db,
                   const char* sql,
                   const char* playerName,
                   std::vector<QuestRef>& refs)
{
    if(!db || !sql || !playerName || !db->Prepare(sql) ||
       !db->BindInputParameter(const_cast<char*>(playerName), 1, PARAMTYPE_String) ||
       !db->Execute(false))
        return;

    while(db->NextRow())
    {
        QuestRef ref;
        db->GetData(&ref.questId, 4, 1, PARAMTYPE_Integer);
        db->GetData(&ref.questKind, 4, 2, PARAMTYPE_Integer);
        db->GetData(&ref.questType, 4, 3, PARAMTYPE_Integer);
        refs.push_back(ref);
    }
}

void AppendReferencedDefinitions(const std::shared_ptr<SQLConnection>& db,
                                 const std::vector<QuestRef>& refs,
                                 bool active,
                                 QuestSpoilerBatchView& batch)
{
    for(std::vector<QuestRef>::const_iterator it = refs.begin(); it != refs.end(); ++it)
    {
        const char* sql = DefinitionByIdTypeSql(it->questKind);
        if(!sql || !db->Prepare(sql))
            continue;

        int questId = it->questId;
        int questType = it->questType;
        if(!db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) ||
           !db->BindInputParameter(&questType, 2, PARAMTYPE_Integer) ||
           !db->Execute(false))
            continue;

        while(db->NextRow())
        {
            int id = 0;
            char questName[128] = {};
            db->GetData(&id, 4, 1, PARAMTYPE_Integer);
            db->GetData(questName, 128, 2, PARAMTYPE_String);

            QuestSpoilerEntryView entry;
            entry.quest = QuestId(static_cast<std::uint32_t>(id));
            entry.kind = it->questKind;
            entry.name = questName;
            entry.active = active;
            batch.entries.push_back(entry);
        }
    }
}
}

bool LegacyQuestViewQuery::TryGetSpoilerBatches(
    const std::string& playerKey,
    int playerLevel,
    int spoilerNumber,
    std::vector<QuestSpoilerBatchView>& out) const
{
    out.clear();
    if(playerKey.empty() || playerKey.size() >= 32u)
        return false;
    if(spoilerNumber != 21 && spoilerNumber != 7 && spoilerNumber != 9 &&
       spoilerNumber != 10 && spoilerNumber != 11)
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());

    if(spoilerNumber == 21 || spoilerNumber == 7 || spoilerNumber == 9)
    {
        std::set<int> seen;
        const bool byQuestId = spoilerNumber == 9;
        const int seenColumn = byQuestId ? 1 : 3;
        LoadSeenValues(db,
            "SELECT * FROM Progress WHERE QuestKind=? AND PlayerName=?",
            playerName, spoilerNumber, seenColumn, seen);

        if(spoilerNumber == 7)
        {
            LoadSeenValues(db,
                "SELECT * FROM Complete WHERE QuestKind=? AND PlayerName=? AND DAY(GETDATE()) <= DAY(EndDate) AND MONTH(GETDATE())=MONTH(EndDate)",
                playerName, spoilerNumber, 3, seen);
        }
        else if(spoilerNumber == 9)
        {
            LoadSeenValues(db,
                "SELECT * FROM Complete WHERE QuestKind=? AND PlayerName=?",
                playerName, spoilerNumber, 1, seen);
        }

        QuestSpoilerBatchView eligible;
        if(LoadLevelBatch(db, spoilerNumber, playerLevel, true, byQuestId, seen, eligible))
            out.push_back(eligible);

        QuestSpoilerBatchView ineligible;
        if(LoadLevelBatch(db, spoilerNumber, playerLevel, false, byQuestId, seen, ineligible))
            out.push_back(ineligible);
    }
    else
    {
        std::vector<QuestRef> refs;
        LoadQuestRefs(db,
            spoilerNumber == 10
                ? "SELECT * FROM Progress WHERE PlayerName=?"
                : "SELECT * FROM Complete WHERE PlayerName=?",
            playerName, refs);

        QuestSpoilerBatchView batch;
        AppendReferencedDefinitions(db, refs, spoilerNumber == 10, batch);
        out.push_back(batch);
    }

    db->Close();
    return true;
}

const LegacyQuestViewQuery& ServerLegacyQuestViewQuery()
{
    static LegacyQuestViewQuery query;
    return query;
}
}
}
}
