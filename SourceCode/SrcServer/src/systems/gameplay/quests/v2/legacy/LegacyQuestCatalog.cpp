#include "systems/gameplay/quests/v2/legacy/LegacyQuestCatalog.h"

#include <memory>
#include <string>

#include "systems/persistence/database/core/Database.h"

namespace gameplay
{
namespace quests
{
namespace legacy
{
namespace
{
const int kRepeatedQuestKind = 21;
const int kDailyQuestKind = 7;
const int kSingleQuestKind = 9;

bool ReadRows(const char* tableName,
              int questKind,
              QuestId quest,
              int& matchCount,
              QuestDefinition& match)
{
    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
    if(!db || !db->Open())
        return false;

    std::string sql = "SELECT PartyFlag,QuestType FROM ";
    sql += tableName;
    sql += " WHERE QuestID=?";
    if(!db->Prepare(sql.c_str()))
    {
        db->Close();
        return false;
    }

    int id = static_cast<int>(quest.value);
    if(!db->BindInputParameter(&id, 1, PARAMTYPE_Integer) || !db->Execute(false))
    {
        db->Close();
        return false;
    }

    while(db->NextRow())
    {
        int partyFlag = 0;
        int questType = 0;
        if(!db->GetData(&partyFlag, sizeof(partyFlag), 1, PARAMTYPE_Integer) ||
           !db->GetData(&questType, sizeof(questType), 2, PARAMTYPE_Integer))
        {
            db->Close();
            return false;
        }
        ++matchCount;
        if(matchCount == 1)
        {
            match.id = quest;
            match.kind = questKind;
            match.type = questType;
            match.partyFlag = partyFlag;
        }
    }

    db->Close();
    return true;
}
}

bool LegacyQuestCatalog::TryGetDefinition(QuestId quest, QuestDefinition& out) const
{
    if(!quest.Valid())
        return false;

    int matches = 0;
    QuestDefinition match;
    if(!ReadRows("RepeatedQuest", kRepeatedQuestKind, quest, matches, match) ||
       !ReadRows("DailyQuest", kDailyQuestKind, quest, matches, match) ||
       !ReadRows("SingleQuest", kSingleQuestKind, quest, matches, match))
        return false;

    if(matches != 1)
        return false;
    out = match;
    return true;
}

const LegacyQuestCatalog& ServerLegacyQuestCatalog()
{
    static LegacyQuestCatalog catalog;
    return catalog;
}
}
}
}
