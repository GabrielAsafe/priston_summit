#include "systems/gameplay/reputation/v2/SQLiteReputationRepository.h"

#include <cstring>
#include <memory>

#include "systems/persistence/database/core/Database.h"

namespace gameplay
{
namespace reputation
{
namespace
{
bool OpenServerDatabase(std::shared_ptr<SQLConnection>& db)
{
    db = SQLConnection::GetConnection(DATABASEID_ServerDB);
    return db && db->Open();
}

void CopyKey(char* destination, std::size_t size, const std::string& value)
{
    if (!destination || size == 0)
        return;
    std::memset(destination, 0, size);
    std::strncpy(destination, value.c_str(), size - 1);
}

bool HasDatabaseError(const std::shared_ptr<SQLConnection>& db)
{
    const char* error = db ? db->GetLastError() : 0;
    return error && error[0] != 0;
}
}

bool SQLiteReputationRepository::EnsureSchema()
{
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;

    const char* statements[] = {
        "CREATE TABLE IF NOT EXISTS ServerDB__GenericReputation ("
        "PlayerKey TEXT NOT NULL CHECK(length(PlayerKey) BETWEEN 1 AND 255), "
        "ReputationKey TEXT NOT NULL CHECK(length(ReputationKey) BETWEEN 1 AND 255), "
        "Value INTEGER NOT NULL, "
        "UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')), "
        "PRIMARY KEY(PlayerKey, ReputationKey))",
        "CREATE TABLE IF NOT EXISTS ServerDB__GenericReputationLedger ("
        "PlayerKey TEXT NOT NULL CHECK(length(PlayerKey) BETWEEN 1 AND 255), "
        "ReputationKey TEXT NOT NULL CHECK(length(ReputationKey) BETWEEN 1 AND 255), "
        "SourceKey TEXT NOT NULL CHECK(length(SourceKey) BETWEEN 1 AND 255), "
        "Delta INTEGER NOT NULL CHECK(Delta <> 0), "
        "ValueAfter INTEGER NOT NULL, "
        "AppliedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')), "
        "PRIMARY KEY(PlayerKey, ReputationKey, SourceKey))"
    };

    bool ok = true;
    for (int i = 0; i < 2 && ok; ++i)
        ok = db->Prepare(statements[i]) && db->Execute(false);
    db->Close();
    return ok;
}

ReputationLoadResult SQLiteReputationRepository::LoadValue(
    const std::string& playerKey,
    const std::string& reputationKey,
    int& value)
{
    if (playerKey.empty() || playerKey.size() > 255u ||
        reputationKey.empty() || reputationKey.size() > 255u || !EnsureSchema())
        return ReputationLoadResult::Failed;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return ReputationLoadResult::Failed;

    char player[256];
    char reputation[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(reputation, sizeof(reputation), reputationKey);
    if (!db->Prepare("SELECT Value FROM GenericReputation WHERE PlayerKey=? AND ReputationKey=?") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(reputation, 2, PARAMTYPE_String))
    {
        db->Close();
        return ReputationLoadResult::Failed;
    }

    if (!db->Execute())
    {
        const bool failed = HasDatabaseError(db);
        db->Close();
        return failed ? ReputationLoadResult::Failed : ReputationLoadResult::NotFound;
    }

    int stored = 0;
    const bool ok = db->GetData(&stored, sizeof(stored), 1, PARAMTYPE_Integer);
    db->Close();
    if (!ok)
        return ReputationLoadResult::Failed;

    value = stored;
    return ReputationLoadResult::Found;
}

ReputationChangeResult SQLiteReputationRepository::ApplyChange(
    const std::string& playerKey,
    const std::string& reputationKey,
    int expectedValue,
    int newValue,
    int delta,
    const std::string& sourceKey)
{
    if (playerKey.empty() || playerKey.size() > 255u ||
        reputationKey.empty() || reputationKey.size() > 255u ||
        sourceKey.empty() || sourceKey.size() > 255u || delta == 0 || !EnsureSchema())
        return ReputationChangeResult::Failed;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db) || !db->BeginTransaction())
        return ReputationChangeResult::Failed;

    char player[256];
    char reputation[256];
    char source[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(reputation, sizeof(reputation), reputationKey);
    CopyKey(source, sizeof(source), sourceKey);

    if (!db->Prepare(
            "SELECT Delta FROM GenericReputationLedger "
            "WHERE PlayerKey=? AND ReputationKey=? AND SourceKey=?") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(reputation, 2, PARAMTYPE_String) ||
        !db->BindInputParameter(source, 3, PARAMTYPE_String))
    {
        db->RollbackTransaction();
        db->Close();
        return ReputationChangeResult::Failed;
    }

    if (db->Execute())
    {
        int existingDelta = 0;
        if (!db->GetData(&existingDelta, sizeof(existingDelta), 1, PARAMTYPE_Integer))
        {
            db->RollbackTransaction();
            db->Close();
            return ReputationChangeResult::Failed;
        }
        const bool committed = db->CommitTransaction();
        db->Close();
        if (!committed)
            return ReputationChangeResult::Failed;
        return existingDelta == delta ? ReputationChangeResult::AlreadyApplied
                                      : ReputationChangeResult::Failed;
    }
    if (HasDatabaseError(db))
    {
        db->RollbackTransaction();
        db->Close();
        return ReputationChangeResult::Failed;
    }

    if (!db->Prepare("SELECT Value FROM GenericReputation WHERE PlayerKey=? AND ReputationKey=?") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(reputation, 2, PARAMTYPE_String))
    {
        db->RollbackTransaction();
        db->Close();
        return ReputationChangeResult::Failed;
    }

    bool found = false;
    int currentValue = 0;
    if (db->Execute())
    {
        found = true;
        if (!db->GetData(&currentValue, sizeof(currentValue), 1, PARAMTYPE_Integer))
        {
            db->RollbackTransaction();
            db->Close();
            return ReputationChangeResult::Failed;
        }
    }
    else if (HasDatabaseError(db))
    {
        db->RollbackTransaction();
        db->Close();
        return ReputationChangeResult::Failed;
    }

    if ((found ? currentValue : 0) != expectedValue)
    {
        db->RollbackTransaction();
        db->Close();
        return ReputationChangeResult::Conflict;
    }

    if (!db->Prepare(
            "INSERT INTO GenericReputation(PlayerKey,ReputationKey,Value,UpdatedAt) "
            "VALUES(?,?,?,datetime('now','localtime')) "
            "ON CONFLICT(PlayerKey,ReputationKey) DO UPDATE SET "
            "Value=excluded.Value,UpdatedAt=datetime('now','localtime')") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(reputation, 2, PARAMTYPE_String) ||
        !db->BindInputParameter(&newValue, 3, PARAMTYPE_Integer) ||
        !db->Execute(false))
    {
        db->RollbackTransaction();
        db->Close();
        return ReputationChangeResult::Failed;
    }

    int valueAfter = newValue;
    if (!db->Prepare(
            "INSERT INTO GenericReputationLedger"
            "(PlayerKey,ReputationKey,SourceKey,Delta,ValueAfter,AppliedAt) "
            "VALUES(?,?,?,?,?,datetime('now','localtime'))") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(reputation, 2, PARAMTYPE_String) ||
        !db->BindInputParameter(source, 3, PARAMTYPE_String) ||
        !db->BindInputParameter(&delta, 4, PARAMTYPE_Integer) ||
        !db->BindInputParameter(&valueAfter, 5, PARAMTYPE_Integer) ||
        !db->Execute(false))
    {
        db->RollbackTransaction();
        db->Close();
        return ReputationChangeResult::Failed;
    }

    if (!db->CommitTransaction())
    {
        db->RollbackTransaction();
        db->Close();
        return ReputationChangeResult::Failed;
    }
    db->Close();
    return ReputationChangeResult::Applied;
}
}
}
