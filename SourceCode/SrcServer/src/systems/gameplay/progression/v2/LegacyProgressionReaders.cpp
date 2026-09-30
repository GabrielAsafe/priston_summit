#include "systems/gameplay/progression/v2/LegacyProgressionReaders.h"

#include <cctype>
#include <cstring>
#include <string>

#include "core/server/legacy_runtime/onserver.h"
#include "persistence/database/Database_sqlite_server.h"

namespace gameplay
{
namespace progression
{
namespace
{
bool EqualsIgnoreCase(const std::string& left, const char* right)
{
    if (!right || left.size() != std::strlen(right))
        return false;
    for (std::size_t i = 0; i < left.size(); ++i)
    {
        const unsigned char a = static_cast<unsigned char>(left[i]);
        const unsigned char b = static_cast<unsigned char>(right[i]);
        if (std::tolower(a) != std::tolower(b))
            return false;
    }
    return true;
}

rsPLAYINFO* FindOnlinePlayer(const std::string& playerKey)
{
    if (playerKey.empty())
        return 0;
    for (int i = 0; i < CONNECTMAX; ++i)
    {
        rsPLAYINFO* player = &rsPlayInfo[i];
        if (player->lpsmSock && EqualsIgnoreCase(playerKey, player->getName()))
            return player;
    }
    return 0;
}

int ProfessionCode(const std::string& key)
{
    if (EqualsIgnoreCase(key, "arcanist")) return 1;
    if (EqualsIgnoreCase(key, "blacksmith")) return 2;
    if (EqualsIgnoreCase(key, "lumberjack")) return 3;
    if (EqualsIgnoreCase(key, "miner")) return 4;
    if (EqualsIgnoreCase(key, "artisan")) return 5;
    return 0;
}
}

int LegacyQuestStateReader::QuestState(const std::string& playerKey, int questId) const
{
    if (playerKey.empty() || questId <= 0)
        return kProgressionUnavailableValue;

    quests::QuestStateSnapshot state;
    if (!quests_.TryGetState(playerKey, QuestId(static_cast<std::uint32_t>(questId)), state))
        return kProgressionUnavailableValue;
    if (state.completed)
        return static_cast<int>(conditions::QuestProgressState::Completed);
    return state.inProgress ? static_cast<int>(conditions::QuestProgressState::InProgress)
                            : static_cast<int>(conditions::QuestProgressState::NotActive);
}

int LegacyProfessionLevelReader::ProfessionLevel(const std::string& playerKey,
                                                 const std::string& professionKey) const
{
    const int expectedJob = ProfessionCode(professionKey);
    if (playerKey.empty() || expectedJob == 0)
        return kProgressionUnavailableValue;

    rsPLAYINFO* player = FindOnlinePlayer(playerKey);
    if (!player || !player->getAccount() || !player->getAccount()[0])
        return kProgressionUnavailableValue;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_NULL);
    if (!db || !db->Open())
        return kProgressionUnavailableValue;

    char account[256] = {};
    const std::size_t accountLength = std::strlen(player->getAccount());
    if (accountLength == 0 || accountLength >= sizeof(account))
    {
        db->Close();
        return kProgressionUnavailableValue;
    }
    std::memcpy(account, player->getAccount(), accountLength + 1);

    int job = 0;
    int level = 0;
    const bool prepared =
        db->Prepare("SELECT JOB,Job_Level FROM [JOBS].[dbo].[Jobs_Info] WHERE ID=?") != FALSE;
    const bool bound = prepared &&
        db->BindInputParameter(account, 1, PARAMTYPE_String) != FALSE;
    const bool executed = bound && db->Execute(false) != FALSE;
    const bool found = executed && db->NextRow() != FALSE;
    if (found)
    {
        db->GetData(&job, sizeof(job), 1, PARAMTYPE_Integer);
        db->GetData(&level, sizeof(level), 2, PARAMTYPE_Integer);
    }
    db->Close();

    if (!executed)
        return kProgressionUnavailableValue;
    if (!found || job != expectedJob)
        return 0;
    return level >= 0 ? level : kProgressionUnavailableValue;
}

bool RegisteredCustomConditionReader::Evaluate(
    const std::string& playerKey,
    const conditions::ConditionDefinition& definition) const
{
    return !playerKey.empty() && handlers_.EvaluateCondition(playerKey, definition);
}
}
}
