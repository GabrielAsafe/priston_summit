#include "systems/gameplay/quests/v2/legacy/LegacyQuestRepository.h"

#include <cstdio>
#include <cstring>
#include <memory>

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

bool ValidKey(const std::string& playerKey, QuestId quest)
{
    return !playerKey.empty() && playerKey.size() < 32u && quest.Valid();
}

void R25SkipTierDiag(const char* stage, int questId, const char* player, const char* detail)
{
    FILE* f = std::fopen("LOGS\\r25_skiptier_diag.log", "a");
    if(!f)
        return;

    std::fprintf(
        f,
        "[R25_SKIPTIER] stage=%s quest=%d player=%s detail=%s\n",
        stage ? stage : "<null>",
        questId,
        player ? player : "<null>",
        detail ? detail : "<null>");
    std::fclose(f);
}

bool QueryCount(const char* sql,
                const std::string& playerKey,
                QuestId quest,
                bool& present)
{
    present = false;
    if(!sql || !ValidKey(playerKey, quest))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    int questId = static_cast<int>(quest.value);
    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());

    const bool ok = db->Prepare(sql) &&
                    db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
                    db->BindInputParameter(playerName, 2, PARAMTYPE_String) &&
                    db->Execute();
    if(!ok)
    {
        db->Close();
        return false;
    }

    int count = 0;
    const bool read = db->GetData(&count, sizeof(count), 1, PARAMTYPE_Integer) != FALSE;
    db->Close();
    if(!read)
        return false;

    present = count > 0;
    return true;
}

bool ValidMonsterName(const std::string& monsterName)
{
    return !monsterName.empty() && monsterName.size() < 32u;
}

bool QueryQuestPlayerCount(const char* sql,
                           const std::string& playerKey,
                           QuestId quest,
                           bool& present)
{
    present = false;
    if(!sql || !ValidKey(playerKey, quest))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    int questId = static_cast<int>(quest.value);
    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());

    const bool ok = db->Prepare(sql) &&
                    db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
                    db->BindInputParameter(playerName, 2, PARAMTYPE_String) &&
                    db->Execute();
    if(!ok)
    {
        db->Close();
        return false;
    }

    int count = 0;
    const bool read = db->GetData(&count, sizeof(count), 1, PARAMTYPE_Integer) != FALSE;
    db->Close();
    if(!read)
        return false;

    present = count > 0;
    return true;
}

bool ExecuteQuestPlayer(const char* sql,
                        const std::string& playerKey,
                        QuestId quest)
{
    if(!sql || !ValidKey(playerKey, quest))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    int questId = static_cast<int>(quest.value);
    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());

    const bool ok = db->Prepare(sql) &&
                    db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
                    db->BindInputParameter(playerName, 2, PARAMTYPE_String) &&
                    db->Execute(false);
    db->Close();
    return ok;
}

bool ExecuteQuestPlayerMonster(const char* sql,
                               const std::string& playerKey,
                               QuestId quest,
                               const std::string& monsterName)
{
    if(!sql || !ValidKey(playerKey, quest) || !ValidMonsterName(monsterName))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    int questId = static_cast<int>(quest.value);
    char playerName[32] = {};
    char monster[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());
    std::memcpy(monster, monsterName.c_str(), monsterName.size());

    const bool ok = db->Prepare(sql) &&
                    db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
                    db->BindInputParameter(playerName, 2, PARAMTYPE_String) &&
                    db->BindInputParameter(monster, 3, PARAMTYPE_String) &&
                    db->Execute(false);
    db->Close();
    return ok;
}

struct QuestMonsterRow
{
    char name[32];
    int count;
    int map;

    QuestMonsterRow() : name(), count(0), map(0) {}
};

int LoadQuestMonsters(const std::shared_ptr<SQLConnection>& db,
                      int questId,
                      int questKind,
                      int questType,
                      QuestMonsterRow (&rows)[10])
{
    if(!db || !db->Prepare("SELECT * FROM QuestMonsters WHERE QuestID=? AND QuestKind=? AND QuestType=?"))
        return 0;

    if(!db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) ||
       !db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer) ||
       !db->BindInputParameter(&questType, 3, PARAMTYPE_Integer) ||
       !db->Execute(false))
        return 0;

    int count = 0;
    while(db->NextRow() && static_cast<std::size_t>(count) < kLegacyQuestFullEntryMax)
    {
        db->GetData(&rows[count].name, 32, 3, PARAMTYPE_String);
        db->GetData(&rows[count].count, 4, 4, PARAMTYPE_Integer);
        db->GetData(&rows[count].map, 4, 6, PARAMTYPE_Integer);
        ++count;
    }
    return count;
}

void InsertQuestMonsters(const std::shared_ptr<SQLConnection>& db,
                         const std::string& playerKey,
                         int questId,
                         int questKind,
                         int questType,
                         int partyFlag)
{
    QuestMonsterRow rows[10];
    const int rowCount = LoadQuestMonsters(db, questId, questKind, questType, rows);
    if(rowCount <= 0)
        return;

    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());
    for(int i = 0; i < rowCount; ++i)
    {
        if(!db->Prepare("INSERT INTO MonsterKilled VALUES (?,0,?,?,?,?,?,?,?)"))
            continue;

        db->BindInputParameter(&rows[i].name, 1, PARAMTYPE_String);
        db->BindInputParameter(&rows[i].count, 2, PARAMTYPE_Integer);
        db->BindInputParameter(&questId, 3, PARAMTYPE_Integer);
        db->BindInputParameter(&questKind, 4, PARAMTYPE_Integer);
        db->BindInputParameter(&questType, 5, PARAMTYPE_Integer);
        db->BindInputParameter(playerName, 6, PARAMTYPE_String);
        db->BindInputParameter(&partyFlag, 7, PARAMTYPE_Integer);
        db->BindInputParameter(&rows[i].map, 8, PARAMTYPE_Integer);
        db->Execute(false);
    }
}

}

bool LegacyQuestRepository::TryIsCompleted(const std::string& playerKey,
                                           QuestId quest,
                                           bool& completed) const
{
    return QueryCount("SELECT COUNT(*) FROM Complete WHERE QuestID=? AND PlayerName=?",
                      playerKey, quest, completed);
}

bool LegacyQuestRepository::TryIsCompletedVariant(const std::string& playerKey,
                                                   QuestId quest,
                                                   int questKind,
                                                   int questType,
                                                   bool& completed) const
{
    completed = false;
    if(!ValidKey(playerKey, quest))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    int questId = static_cast<int>(quest.value);
    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());
    const bool ok = db->Prepare(
                        "SELECT COUNT(*) FROM Complete WHERE QuestID=? AND QuestKind=? AND QuestType=? AND PlayerName=?") &&
                    db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
                    db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer) &&
                    db->BindInputParameter(&questType, 3, PARAMTYPE_Integer) &&
                    db->BindInputParameter(playerName, 4, PARAMTYPE_String) &&
                    db->Execute();
    if(!ok)
    {
        db->Close();
        return false;
    }

    int count = 0;
    const bool read = db->GetData(&count, sizeof(count), 1, PARAMTYPE_Integer) != FALSE;
    db->Close();
    if(!read)
        return false;

    completed = count > 0;
    return true;
}

bool LegacyQuestRepository::TryIsInProgress(const std::string& playerKey,
                                            QuestId quest,
                                            bool& inProgress) const
{
    return QueryCount("SELECT COUNT(*) FROM Progress WHERE QuestID=? AND PlayerName=?",
                      playerKey, quest, inProgress);
}

bool LegacyQuestRepository::TryHasPendingObjectives(const std::string& playerKey,
                                                        QuestId quest,
                                                        bool& pending) const
{
    return QueryQuestPlayerCount(
        "SELECT COUNT(*) FROM MonsterKilled WHERE QuestID=? AND PlayerName=? AND Killed < Count",
        playerKey, quest, pending);
}

bool LegacyQuestRepository::TryIsObjectivePending(const std::string& playerKey,
                                                    QuestId quest,
                                                    const std::string& monsterName,
                                                    bool& pending) const
{
    pending = false;
    if(!ValidKey(playerKey, quest) || !ValidMonsterName(monsterName))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    int questId = static_cast<int>(quest.value);
    char playerName[32] = {};
    char monster[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());
    std::memcpy(monster, monsterName.c_str(), monsterName.size());

    const bool ok = db->Prepare(
                        "SELECT COUNT(*) FROM MonsterKilled WHERE QuestID=? AND PlayerName=? AND MonsterName=? AND Killed < Count") &&
                    db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
                    db->BindInputParameter(playerName, 2, PARAMTYPE_String) &&
                    db->BindInputParameter(monster, 3, PARAMTYPE_String) &&
                    db->Execute();
    if(!ok)
    {
        db->Close();
        return false;
    }

    int count = 0;
    const bool read = db->GetData(&count, sizeof(count), 1, PARAMTYPE_Integer) != FALSE;
    db->Close();
    if(!read)
        return false;

    pending = count > 0;
    return true;
}

bool LegacyQuestRepository::CompleteObjective(const std::string& playerKey,
                                               QuestId quest,
                                               const std::string& monsterName) const
{
    return ExecuteQuestPlayerMonster(
        "UPDATE MonsterKilled SET Killed=Count WHERE QuestID=? AND PlayerName=? AND MonsterName=? AND Killed < Count",
        playerKey, quest, monsterName);
}

bool LegacyQuestRepository::CompleteObjectives(const std::string& playerKey,
                                                QuestId quest) const
{
    return ExecuteQuestPlayer(
        "UPDATE MonsterKilled SET Killed=Count WHERE QuestID=? AND PlayerName=? AND Killed < Count",
        playerKey, quest);
}

bool LegacyQuestRepository::CancelProgress(const std::string& playerKey,
                                            QuestId quest) const
{
    if(!ValidKey(playerKey, quest))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    int questId = static_cast<int>(quest.value);
    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());
    bool ok = true;

    if(!(db->Prepare("DELETE FROM MonsterKilled WHERE QuestID=? AND PlayerName=?") &&
         db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
         db->BindInputParameter(playerName, 2, PARAMTYPE_String) &&
         db->Execute(false)))
        ok = false;

    if(!(db->Prepare("DELETE FROM Progress WHERE QuestID=? AND PlayerName=?") &&
         db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
         db->BindInputParameter(playerName, 2, PARAMTYPE_String) &&
         db->Execute(false)))
        ok = false;

    db->Close();
    return ok;
}

bool LegacyQuestRepository::StartProgress(const std::string& playerKey,
                                         QuestId quest,
                                         int questKind,
                                         int questType,
                                         int partyFlag) const
{
    if(!ValidKey(playerKey, quest) ||
       (questKind != 21 && questKind != 7 && questKind != 9))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    int questId = static_cast<int>(quest.value);
    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());
    bool inserted = false;

    if(questKind == 21)
    {
        if(db->Prepare("INSERT INTO Progress SELECT ?2, ?3, ?4, ?5, datetime('now','localtime') WHERE EXISTS(SELECT 1 FROM RepeatedQuest WHERE QuestID=?1)"))
        {
            db->BindInputParameter(&questId, 1, PARAMTYPE_Integer);
            db->BindInputParameter(&questId, 2, PARAMTYPE_Integer);
            db->BindInputParameter(&questKind, 3, PARAMTYPE_Integer);
            db->BindInputParameter(&questType, 4, PARAMTYPE_Integer);
            db->BindInputParameter(playerName, 5, PARAMTYPE_String);
            inserted = db->Execute(false) ? true : false;
        }
    }
    else if(questKind == 7)
    {
        if(db->Prepare("INSERT INTO Progress SELECT ?2, ?3, ?4, ?5, datetime('now','localtime') WHERE EXISTS(SELECT 1 FROM DailyQuest WHERE QuestID=?1)"))
        {
            db->BindInputParameter(&questId, 1, PARAMTYPE_Integer);
            db->BindInputParameter(&questId, 2, PARAMTYPE_Integer);
            db->BindInputParameter(&questKind, 3, PARAMTYPE_Integer);
            db->BindInputParameter(&questType, 4, PARAMTYPE_Integer);
            db->BindInputParameter(playerName, 5, PARAMTYPE_String);
            inserted = db->Execute(false) ? true : false;
        }
    }
    else
    {
        if(db->Prepare("INSERT INTO Progress SELECT ?5, ?6, ?7, ?8, datetime('now','localtime') WHERE EXISTS(SELECT 1 FROM SingleQuest WHERE QuestID=?1) AND NOT EXISTS(SELECT 1 FROM Progress WHERE QuestID=?2 AND QuestKind=?3 AND PlayerName=?4)"))
        {
            db->BindInputParameter(&questId, 1, PARAMTYPE_Integer);
            db->BindInputParameter(&questId, 2, PARAMTYPE_Integer);
            db->BindInputParameter(&questKind, 3, PARAMTYPE_Integer);
            db->BindInputParameter(playerName, 4, PARAMTYPE_String);
            db->BindInputParameter(&questId, 5, PARAMTYPE_Integer);
            db->BindInputParameter(&questKind, 6, PARAMTYPE_Integer);
            db->BindInputParameter(&questType, 7, PARAMTYPE_Integer);
            db->BindInputParameter(playerName, 8, PARAMTYPE_String);
            inserted = db->Execute(false) ? true : false;
        }
    }

    if(inserted)
        InsertQuestMonsters(db, playerKey, questId, questKind, questType, partyFlag);

    db->Close();
    return inserted;
}

bool LegacyQuestRepository::CancelProgressVariant(const std::string& playerKey,
                                                   QuestId quest,
                                                   int questKind,
                                                   int questType) const
{
    if(!ValidKey(playerKey, quest))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    int questId = static_cast<int>(quest.value);
    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());
    bool ok = true;

    if(!(db->Prepare("DELETE FROM Progress WHERE QuestID=? AND QuestKind=? AND QuestType=? AND PlayerName=?") &&
         db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
         db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer) &&
         db->BindInputParameter(&questType, 3, PARAMTYPE_Integer) &&
         db->BindInputParameter(playerName, 4, PARAMTYPE_String) &&
         db->Execute()))
        ok = false;

    if(!(db->Prepare("DELETE FROM MonsterKilled WHERE QuestID=? AND QuestKind=? AND QuestType=? AND PlayerName=?") &&
         db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
         db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer) &&
         db->BindInputParameter(&questType, 3, PARAMTYPE_Integer) &&
         db->BindInputParameter(playerName, 4, PARAMTYPE_String) &&
         db->Execute()))
        ok = false;

    db->Close();
    return ok;
}

bool LegacyQuestRepository::ListInvalidForLevel(
    const std::string& playerKey,
    int playerLevel,
    std::vector<QuestProgressRecord>& out) const
{
    out.clear();
    if(playerKey.empty() || playerKey.size() >= 32u)
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());
    std::vector<QuestProgressRecord> active;

    if(!db->Prepare("SELECT * FROM Progress WHERE PlayerName=?") ||
       !db->BindInputParameter(playerName, 1, PARAMTYPE_String) ||
       !db->Execute(false))
    {
        db->Close();
        return false;
    }

    while(db->NextRow())
    {
        int questId = 0;
        int questKind = 0;
        int questType = 0;
        if(!db->GetData(&questId, 4, 1, PARAMTYPE_Integer) ||
           !db->GetData(&questKind, 4, 2, PARAMTYPE_Integer) ||
           !db->GetData(&questType, 4, 3, PARAMTYPE_Integer))
        {
            db->Close();
            out.clear();
            return false;
        }
        active.push_back(QuestProgressRecord(
            QuestId(static_cast<std::uint32_t>(questId)), questKind, questType));
    }

    for(std::vector<QuestProgressRecord>::const_iterator it = active.begin();
        it != active.end(); ++it)
    {
        const char* sql = 0;
        switch(it->kind)
        {
        case 21:
            sql = "SELECT * FROM RepeatedQuest WHERE QuestID=? AND QuestType=? AND (LevelMin > ? OR LevelMax < ?)";
            break;
        case 7:
            sql = "SELECT * FROM DailyQuest WHERE QuestID=? AND QuestType=? AND (LevelMin > ? OR LevelMax < ?)";
            break;
        case 9:
            sql = "SELECT * FROM SingleQuest WHERE QuestID=? AND QuestType=? AND (LevelMin > ? OR LevelMax < ?)";
            break;
        default:
            out.push_back(*it);
            continue;
        }

        int questId = static_cast<int>(it->quest.value);
        int questType = it->type;
        const bool queryOk = db->Prepare(sql) &&
            db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
            db->BindInputParameter(&questType, 2, PARAMTYPE_Integer) &&
            db->BindInputParameter(&playerLevel, 3, PARAMTYPE_Integer) &&
            db->BindInputParameter(&playerLevel, 4, PARAMTYPE_Integer) &&
            db->Execute(false);

        // Preserve legacy fail-closed behavior for definition lookup failures:
        // only a successful query proving the quest is still in-range removes it.
        if(!queryOk || db->NextRow())
            out.push_back(*it);
    }

    db->Close();
    return true;
}

bool LegacyQuestRepository::RecordMonsterKill(
    const std::string& playerKey,
    const std::string& monsterName,
    int area,
    bool partyCredit,
    std::vector<QuestKillProgressRecord>& updated) const
{
    updated.clear();
    if(playerKey.empty() || playerKey.size() >= 32u || !ValidMonsterName(monsterName))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    char playerName[32] = {};
    char monster[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());
    std::memcpy(monster, monsterName.c_str(), monsterName.size());

    const char* updateSql =
        "UPDATE MonsterKilled SET Killed=Killed+1 "
        "WHERE PlayerName=? COLLATE NOCASE "
        "AND (((MonsterName='0') AND (QuestMap=-1 OR QuestMap=?)) "
        "OR (MonsterName=? COLLATE NOCASE AND (QuestMap=-1 OR QuestMap=?))) "
        "AND QuestID NOT IN (10010,10011) "
        "AND Killed < \"Count\"";

    if(partyCredit)
    {
        updateSql =
            "UPDATE MonsterKilled SET Killed=Killed+1 "
            "WHERE PlayerName=? COLLATE NOCASE "
            "AND (((MonsterName='0') AND (QuestMap=-1 OR QuestMap=?)) "
            "OR (MonsterName=? COLLATE NOCASE AND (QuestMap=-1 OR QuestMap=?))) "
            "AND PartyFlag=1 "
            "AND QuestKind != 8 "
            "AND QuestID NOT IN (10010,10011) "
            "AND Killed < \"Count\"";
    }

    bool success = db->Prepare(updateSql) &&
        db->BindInputParameter(playerName, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&area, 2, PARAMTYPE_Integer) &&
        db->BindInputParameter(monster, 3, PARAMTYPE_String) &&
        db->BindInputParameter(&area, 4, PARAMTYPE_Integer) &&
        db->Execute(false);

    if(success)
    {
        int changed = 0;
        success = db->Prepare("SELECT changes()") &&
            db->Execute() &&
            db->GetData(&changed, 4, 1, PARAMTYPE_Integer);

        if(success && changed > 0)
        {
            const char* selectSql =
                "SELECT QuestID,QuestKind,QuestType,Killed,\"Count\" "
                "FROM MonsterKilled "
                "WHERE PlayerName=? COLLATE NOCASE "
                "AND (((MonsterName='0') AND (QuestMap=-1 OR QuestMap=?)) "
                "OR (MonsterName=? COLLATE NOCASE AND (QuestMap=-1 OR QuestMap=?))) "
                "AND QuestID NOT IN (10010,10011)";

            if(partyCredit)
            {
                selectSql =
                    "SELECT QuestID,QuestKind,QuestType,Killed,\"Count\" "
                    "FROM MonsterKilled "
                    "WHERE PlayerName=? COLLATE NOCASE "
                    "AND (((MonsterName='0') AND (QuestMap=-1 OR QuestMap=?)) "
                    "OR (MonsterName=? COLLATE NOCASE AND (QuestMap=-1 OR QuestMap=?))) "
                    "AND PartyFlag=1 "
                    "AND QuestKind != 8 "
                    "AND QuestID NOT IN (10010,10011)";
            }

            success = db->Prepare(selectSql) &&
                db->BindInputParameter(playerName, 1, PARAMTYPE_String) &&
                db->BindInputParameter(&area, 2, PARAMTYPE_Integer) &&
                db->BindInputParameter(monster, 3, PARAMTYPE_String) &&
                db->BindInputParameter(&area, 4, PARAMTYPE_Integer) &&
                db->Execute(false);

            while(success && db->NextRow())
            {
                QuestKillProgressRecord row;
                int questId = 0;
                if(!db->GetData(&questId, 4, 1, PARAMTYPE_Integer) ||
                   !db->GetData(&row.kind, 4, 2, PARAMTYPE_Integer) ||
                   !db->GetData(&row.type, 4, 3, PARAMTYPE_Integer) ||
                   !db->GetData(&row.killed, 4, 4, PARAMTYPE_Integer) ||
                   !db->GetData(&row.count, 4, 5, PARAMTYPE_Integer))
                {
                    success = false;
                    break;
                }
                row.quest = QuestId(static_cast<std::uint32_t>(questId));

                bool duplicate = false;
                for(std::vector<QuestKillProgressRecord>::const_iterator it = updated.begin();
                    it != updated.end(); ++it)
                {
                    if(it->quest.value == row.quest.value && it->kind == row.kind && it->type == row.type)
                    {
                        duplicate = true;
                        break;
                    }
                }
                if(!duplicate)
                    updated.push_back(row);
            }
        }
    }

    db->Close();
    return success;
}

bool LegacyQuestRepository::ClaimReward(const std::string& playerKey,
                                        QuestId quest,
                                        int questKind,
                                        int questType,
                                        QuestRewardPayload& reward) const
{
    reward = QuestRewardPayload();
    if(!ValidKey(playerKey, quest))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    int questId = static_cast<int>(quest.value);
    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());

    const bool hasProgress = db->Prepare(
        "SELECT * FROM Progress WHERE QuestID=? AND QuestKind=? AND QuestType=? AND PlayerName=?") &&
        db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
        db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer) &&
        db->BindInputParameter(&questType, 3, PARAMTYPE_Integer) &&
        db->BindInputParameter(playerName, 4, PARAMTYPE_String) &&
        db->Execute();
    if(!hasProgress)
    {
        db->Close();
        return false;
    }

    int killed[10] = {};
    int required[10] = {};
    int objectiveCount = 0;
    if(db->Prepare(
        "SELECT * FROM MonsterKilled WHERE QuestID=? AND QuestKind=? AND QuestType=? AND PlayerName=?") &&
       db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) &&
       db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer) &&
       db->BindInputParameter(&questType, 3, PARAMTYPE_Integer) &&
       db->BindInputParameter(playerName, 4, PARAMTYPE_String) &&
       db->Execute(false))
    {
        while(db->NextRow() && static_cast<std::size_t>(objectiveCount) < kLegacyQuestFullEntryMax)
        {
            db->GetData(&killed[objectiveCount], 4, 2, PARAMTYPE_Integer);
            db->GetData(&required[objectiveCount], 4, 3, PARAMTYPE_Integer);
            ++objectiveCount;
        }
    }

    if(objectiveCount <= 0)
    {
        db->Close();
        return false;
    }

    for(int i = 0; i < objectiveCount; ++i)
    {
        if(killed[i] != required[i])
        {
            db->Close();
            return false;
        }
    }

    // Preserve legacy semantics: completion persistence is best-effort once the
    // authoritative progress/objective checks have accepted the claim.
    if(db->Prepare("DELETE FROM Progress WHERE QuestID=? AND QuestKind=? AND QuestType=? AND PlayerName=?"))
    {
        db->BindInputParameter(&questId, 1, PARAMTYPE_Integer);
        db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer);
        db->BindInputParameter(&questType, 3, PARAMTYPE_Integer);
        db->BindInputParameter(playerName, 4, PARAMTYPE_String);
        db->Execute();
    }

    if(db->Prepare("DELETE FROM MonsterKilled WHERE QuestID=? AND QuestKind=? AND QuestType=? AND PlayerName=?"))
    {
        db->BindInputParameter(&questId, 1, PARAMTYPE_Integer);
        db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer);
        db->BindInputParameter(&questType, 3, PARAMTYPE_Integer);
        db->BindInputParameter(playerName, 4, PARAMTYPE_String);
        db->Execute();
    }

    if(db->Prepare(
        "INSERT OR REPLACE INTO Complete (QuestID, QuestKind, QuestType, PlayerName, EndDate) VALUES (?1, ?2, ?3, ?4, datetime('now','localtime'))"))
    {
        db->BindInputParameter(&questId, 1, PARAMTYPE_Integer);
        db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer);
        db->BindInputParameter(&questType, 3, PARAMTYPE_Integer);
        db->BindInputParameter(playerName, 4, PARAMTYPE_String);
        db->Execute();
    }

    if(db->Prepare("SELECT * FROM Rewards WHERE QuestID=? AND QuestKind=? AND QuestType=?"))
    {
        db->BindInputParameter(&questId, 1, PARAMTYPE_Integer);
        db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer);
        db->BindInputParameter(&questType, 3, PARAMTYPE_Integer);
        if(db->Execute())
        {
            db->GetData(&reward.gold, 4, 3, PARAMTYPE_Integer);
            db->GetData(&reward.experience, 8, 4, PARAMTYPE_Int64);
            db->GetData(&reward.coin, 4, 6, PARAMTYPE_Integer);
            db->GetData(&reward.tCoin, 4, 7, PARAMTYPE_Integer);
        }
    }

    if(db->Prepare("SELECT * FROM ItemRewards WHERE QuestID=? AND QuestKind=? AND QuestType=?"))
    {
        db->BindInputParameter(&questId, 1, PARAMTYPE_Integer);
        db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer);
        db->BindInputParameter(&questType, 3, PARAMTYPE_Integer);
        if(db->Execute(false))
        {
            while(db->NextRow() && reward.items.size() < kLegacyQuestFullEntryMax)
            {
                char itemCode[32] = {};
                int itemCount = 0;
                db->GetData(itemCode, 32, 3, PARAMTYPE_String);
                db->GetData(&itemCount, 4, 4, PARAMTYPE_Integer);
                reward.items.push_back(QuestItemRewardRecord(itemCode, itemCount));
            }
        }
    }

    db->Close();
    return true;
}

bool LegacyQuestRepository::HasPartyObjectiveTarget(const std::string& playerKey,
                                                    const std::string& monsterName,
                                                    int area,
                                                    bool& pending) const
{
    pending = false;
    if(playerKey.empty() || playerKey.size() >= 32u || !ValidMonsterName(monsterName))
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    char playerName[32] = {};
    char monster[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());
    std::memcpy(monster, monsterName.c_str(), monsterName.size());

    const bool ok = db->Prepare(
        "SELECT 1 FROM MonsterKilled "
        "WHERE PlayerName=? COLLATE NOCASE "
        "AND (((MonsterName='0') AND (QuestMap=-1 OR QuestMap=?)) "
        "OR (MonsterName=? COLLATE NOCASE AND (QuestMap=-1 OR QuestMap=?))) "
        "AND PartyFlag=1 AND Killed < \"Count\" LIMIT 1") &&
        db->BindInputParameter(playerName, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&area, 2, PARAMTYPE_Integer) &&
        db->BindInputParameter(monster, 3, PARAMTYPE_String) &&
        db->BindInputParameter(&area, 4, PARAMTYPE_Integer);

    if(ok)
        pending = db->Execute() ? true : false;
    db->Close();
    return ok;
}

bool LegacyQuestRepository::EnsureObjective(const std::string& playerKey,
                                            QuestId quest,
                                            int questKind,
                                            int questType,
                                            const std::string& monsterName,
                                            int count,
                                            int partyFlag,
                                            int map) const
{
    if(!ValidKey(playerKey, quest) || !ValidMonsterName(monsterName) || count <= 0)
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    int questId = static_cast<int>(quest.value);
    char playerName[32] = {};
    char monster[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());
    std::memcpy(monster, monsterName.c_str(), monsterName.size());

    const bool ok = db->Prepare(
        "INSERT INTO MonsterKilled "
        "SELECT ?1,0,?2,?3,?4,?5,?6,?7,?8 "
        "WHERE NOT EXISTS(SELECT 1 FROM MonsterKilled "
        "WHERE QuestID=?3 AND QuestKind=?4 AND QuestType=?5 AND PlayerName=?6 AND MonsterName=?1)") &&
        db->BindInputParameter(monster, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&count, 2, PARAMTYPE_Integer) &&
        db->BindInputParameter(&questId, 3, PARAMTYPE_Integer) &&
        db->BindInputParameter(&questKind, 4, PARAMTYPE_Integer) &&
        db->BindInputParameter(&questType, 5, PARAMTYPE_Integer) &&
        db->BindInputParameter(playerName, 6, PARAMTYPE_String) &&
        db->BindInputParameter(&partyFlag, 7, PARAMTYPE_Integer) &&
        db->BindInputParameter(&map, 8, PARAMTYPE_Integer) &&
        db->Execute(false);
    db->Close();
    return ok;
}

bool LegacyQuestRepository::MarkSingleCompletedForTesting(const std::string& playerKey,
                                                          QuestId quest) const
{
    int questId = static_cast<int>(quest.value);
    if(!ValidKey(playerKey, quest))
    {
        R25SkipTierDiag("invalid_args", questId, playerKey.empty() ? "<empty>" : playerKey.c_str(), "");
        return false;
    }

    char playerName[32] = {};
    std::memcpy(playerName, playerKey.c_str(), playerKey.size());
    R25SkipTierDiag("enter", questId, playerName, "");

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db)
    {
        R25SkipTierDiag("get_connection_fail", questId, playerName, "DATABASEID_QuestDB returned null");
        return false;
    }
    if(!db->Open())
    {
        R25SkipTierDiag("open_fail", questId, playerName, db->GetLastError());
        return false;
    }

    bool ok = false;
    int questType = 0;
    int questKind = 9;
    int questDefinitionCount = 0;
    char diag[256] = {};

    if(!db->Prepare("SELECT COALESCE(MAX(QuestType),0), COUNT(*) FROM SingleQuest WHERE QuestID=?"))
    {
        R25SkipTierDiag("select_prepare_fail", questId, playerName, db->GetLastError());
        goto done;
    }
    if(!db->BindInputParameter(&questId, 1, PARAMTYPE_Integer))
    {
        R25SkipTierDiag("select_bind_fail", questId, playerName, db->GetLastError());
        goto done;
    }
    if(!db->Execute())
    {
        R25SkipTierDiag("select_execute_fail", questId, playerName, db->GetLastError());
        goto done;
    }
    if(!db->GetData(&questType, 4, 1, PARAMTYPE_Integer))
    {
        R25SkipTierDiag("select_get_questtype_fail", questId, playerName, db->GetLastError());
        goto done;
    }
    if(!db->GetData(&questDefinitionCount, 4, 2, PARAMTYPE_Integer))
    {
        R25SkipTierDiag("select_get_count_fail", questId, playerName, db->GetLastError());
        goto done;
    }

    if(questDefinitionCount <= 0)
        questType = 0;
    std::snprintf(diag, sizeof(diag), "definitionCount=%d questType=%d", questDefinitionCount, questType);
    R25SkipTierDiag("select_ok", questId, playerName, diag);

    if(!db->Prepare("DELETE FROM MonsterKilled WHERE QuestID=? AND PlayerName=?"))
    {
        R25SkipTierDiag("monster_delete_prepare_fail", questId, playerName, db->GetLastError());
        goto done;
    }
    if(!db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) ||
       !db->BindInputParameter(playerName, 2, PARAMTYPE_String))
    {
        R25SkipTierDiag("monster_delete_bind_fail", questId, playerName, db->GetLastError());
        goto done;
    }
    if(!db->Execute(false))
    {
        R25SkipTierDiag("monster_delete_execute_fail", questId, playerName, db->GetLastError());
        goto done;
    }
    R25SkipTierDiag("monster_delete_ok", questId, playerName, "");

    if(!db->Prepare("DELETE FROM Progress WHERE QuestID=? AND PlayerName=?"))
    {
        R25SkipTierDiag("progress_delete_prepare_fail", questId, playerName, db->GetLastError());
        goto done;
    }
    if(!db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) ||
       !db->BindInputParameter(playerName, 2, PARAMTYPE_String))
    {
        R25SkipTierDiag("progress_delete_bind_fail", questId, playerName, db->GetLastError());
        goto done;
    }
    if(!db->Execute(false))
    {
        R25SkipTierDiag("progress_delete_execute_fail", questId, playerName, db->GetLastError());
        goto done;
    }
    R25SkipTierDiag("progress_delete_ok", questId, playerName, "");

    if(!db->Prepare("INSERT OR REPLACE INTO Complete (QuestID, QuestKind, QuestType, PlayerName, EndDate) VALUES (?1, ?2, ?3, ?4, datetime('now','localtime'))"))
    {
        R25SkipTierDiag("complete_prepare_fail", questId, playerName, db->GetLastError());
        goto done;
    }
    if(!db->BindInputParameter(&questId, 1, PARAMTYPE_Integer) ||
       !db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer) ||
       !db->BindInputParameter(&questType, 3, PARAMTYPE_Integer) ||
       !db->BindInputParameter(playerName, 4, PARAMTYPE_String))
    {
        R25SkipTierDiag("complete_bind_fail", questId, playerName, db->GetLastError());
        goto done;
    }

    ok = db->Execute(false) ? true : false;
    R25SkipTierDiag(ok ? "complete_execute_ok" : "complete_execute_fail",
                    questId, playerName, ok ? "" : db->GetLastError());

done:
    R25SkipTierDiag(ok ? "return_true" : "return_false", questId, playerName, db->GetLastError());
    db->Close();
    return ok;
}

const LegacyQuestRepository& ServerLegacyQuestRepository()
{
    static LegacyQuestRepository repository;
    return repository;
}
}
}
}
