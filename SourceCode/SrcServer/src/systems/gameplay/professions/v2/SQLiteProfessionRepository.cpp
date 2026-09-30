#include "systems/gameplay/professions/v2/SQLiteProfessionRepository.h"

#include <cstring>
#include <memory>

#include "systems/persistence/database/core/Database.h"

namespace gameplay
{
namespace professions
{
namespace
{
bool OpenDb(std::shared_ptr<SQLConnection>& db)
{
    db = SQLConnection::GetConnection(DATABASEID_ServerDB);
    return db && db->Open();
}

void CopyText(char* destination, std::size_t size, const std::string& value)
{
    if (!destination || size == 0)
        return;
    std::memset(destination, 0, size);
    std::strncpy(destination, value.c_str(), size - 1);
}

bool ValidPlayerName(const std::string& playerName)
{
    return !playerName.empty() && playerName.size() <= 255u;
}

bool ValidProfessionType(ProfessionType type)
{
    const int value = static_cast<int>(type);
    return value > static_cast<int>(ProfessionType::None) &&
           value <= static_cast<int>(ProfessionType::Custom);
}
}

bool SQLiteProfessionRepository::EnsureSchema()
{
    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    const bool ok = db->Prepare(
        "CREATE TABLE IF NOT EXISTS ServerDB__ProfessionProgress ("
        "PlayerKey TEXT NOT NULL CHECK(length(PlayerKey) BETWEEN 1 AND 255), "
        "ProfessionType INTEGER NOT NULL CHECK(ProfessionType BETWEEN 1 AND 8), "
        "Level INTEGER NOT NULL CHECK(Level >= 0), "
        "Experience INTEGER NOT NULL CHECK(Experience >= 0), "
        "UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')), "
        "PRIMARY KEY(PlayerKey, ProfessionType))") && db->Execute(false);
    db->Close();
    return ok;
}

bool SQLiteProfessionRepository::Ready()
{
    return EnsureSchema();
}

bool SQLiteProfessionRepository::LoadPlayer(const std::string& playerName,
                                            std::vector<ProfessionRecord>& out)
{
    out.clear();
    if (!ValidPlayerName(playerName) || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    CopyText(player, sizeof(player), playerName);
    bool ok = db->Prepare(
        "SELECT ProfessionType,Level,Experience FROM ProfessionProgress "
        "WHERE PlayerKey=? ORDER BY ProfessionType") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String) && db->Execute(false);

    while (ok && db->NextRow())
    {
        int typeValue = 0;
        int level = 0;
        long long experience = 0;
        ok = db->GetData(&typeValue, sizeof(typeValue), 1, PARAMTYPE_Integer) &&
             db->GetData(&level, sizeof(level), 2, PARAMTYPE_Integer) &&
             db->GetData(&experience, sizeof(experience), 3, PARAMTYPE_Int64);
        if (!ok)
            break;

        const ProfessionType type = static_cast<ProfessionType>(typeValue);
        if (!ValidProfessionType(type) || level < 0 || experience < 0)
        {
            ok = false;
            break;
        }
        out.push_back(ProfessionRecord(type, ProfessionProgress(level, static_cast<std::uint64_t>(experience))));
    }
    db->Close();
    return ok;
}

bool SQLiteProfessionRepository::SaveProgress(const std::string& playerName,
                                              ProfessionType type,
                                              const ProfessionProgress& progress)
{
    if (!ValidPlayerName(playerName) || !ValidProfessionType(type) ||
        progress.level < 0 || progress.experience > 0x7fffffffffffffffULL || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    CopyText(player, sizeof(player), playerName);
    int typeValue = static_cast<int>(type);
    int level = progress.level;
    long long experience = static_cast<long long>(progress.experience);

    const bool ok = db->Prepare(
        "INSERT INTO ProfessionProgress(PlayerKey,ProfessionType,Level,Experience,UpdatedAt) "
        "VALUES(?,?,?,?,datetime('now','localtime')) "
        "ON CONFLICT(PlayerKey,ProfessionType) DO UPDATE SET "
        "Level=excluded.Level,Experience=excluded.Experience,UpdatedAt=datetime('now','localtime')") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&typeValue, 2, PARAMTYPE_Integer) &&
        db->BindInputParameter(&level, 3, PARAMTYPE_Integer) &&
        db->BindInputParameter(&experience, 4, PARAMTYPE_Int64) &&
        db->Execute(false);
    db->Close();
    return ok;
}
}
}
