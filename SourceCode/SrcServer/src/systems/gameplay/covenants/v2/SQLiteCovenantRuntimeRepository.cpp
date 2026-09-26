#include "systems/gameplay/covenants/v2/SQLiteCovenantRuntimeRepository.h"

#include <cstring>
#include <memory>
#include <limits>
#include <utility>
#include <vector>

#include "systems/persistence/database/core/Database.h"

namespace gameplay
{
namespace covenants
{
namespace
{
bool OpenDb(std::shared_ptr<SQLConnection>& db)
{
    db = SQLConnection::GetConnection(DATABASEID_ServerDB);
    return db && db->Open();
}

void CopyKey(char* destination, std::size_t size, const std::string& value)
{
    std::memset(destination, 0, size);
    if (size)
        std::strncpy(destination, value.c_str(), size - 1);
}

bool NoDatabaseError(const std::shared_ptr<SQLConnection>& db)
{
    const char* error = db->GetLastError();
    return !error || !error[0];
}
}

bool SQLiteCovenantRuntimeRepository::EnsureSchema()
{
    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    const char* statements[] = {
        "CREATE TABLE IF NOT EXISTS CovenantPlayerState("
        "PlayerKey TEXT NOT NULL PRIMARY KEY,"
        "SwitchCooldownUntil INTEGER NOT NULL DEFAULT 0,"
        "UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')))",

        "CREATE TABLE IF NOT EXISTS CovenantMastery("
        "PlayerKey TEXT NOT NULL,"
        "CovenantId INTEGER NOT NULL,"
        "MasteredAt INTEGER NOT NULL,"
        "PRIMARY KEY(PlayerKey,CovenantId))",

        "CREATE TABLE IF NOT EXISTS CovenantCollection("
        "PlayerKey TEXT NOT NULL,"
        "CovenantId INTEGER NOT NULL,"
        "CollectibleKey TEXT NOT NULL,"
        "AcquiredAt INTEGER NOT NULL,"
        "PRIMARY KEY(PlayerKey,CovenantId,CollectibleKey))",

        "CREATE TABLE IF NOT EXISTS CovenantChallengeProgress("
        "PlayerKey TEXT NOT NULL,"
        "CovenantId INTEGER NOT NULL,"
        "ChallengeKey TEXT NOT NULL,"
        "Progress INTEGER NOT NULL DEFAULT 0,"
        "Completed INTEGER NOT NULL DEFAULT 0,"
        "RewardClaimed INTEGER NOT NULL DEFAULT 0,"
        "UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),"
        "PRIMARY KEY(PlayerKey,CovenantId,ChallengeKey))",

        "CREATE TABLE IF NOT EXISTS CovenantPowerState("
        "PlayerKey TEXT NOT NULL,"
        "CovenantId INTEGER NOT NULL,"
        "PowerKey TEXT NOT NULL,"
        "ActiveUntil INTEGER NOT NULL DEFAULT 0,"
        "CooldownUntil INTEGER NOT NULL DEFAULT 0,"
        "TargetSerial INTEGER NOT NULL DEFAULT 0,"
        "UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),"
        "PRIMARY KEY(PlayerKey,CovenantId,PowerKey))"
,

        "CREATE TABLE IF NOT EXISTS CovenantInventory("
        "PlayerKey TEXT NOT NULL,"
        "CovenantId INTEGER NOT NULL,"
        "ItemKey TEXT NOT NULL,"
        "Quantity INTEGER NOT NULL DEFAULT 0,"
        "UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')),"
        "PRIMARY KEY(PlayerKey,CovenantId,ItemKey))",

        "CREATE TABLE IF NOT EXISTS CovenantArtisanState("
        "PlayerKey TEXT NOT NULL PRIMARY KEY,"
        "Mode INTEGER NOT NULL DEFAULT 0 CHECK(Mode BETWEEN 0 AND 3),"
        "UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')))"
    };

    bool ok = true;
    const int statementCount = static_cast<int>(sizeof(statements) / sizeof(statements[0]));
    for (int i = 0; i < statementCount && ok; ++i)
        ok = db->Prepare(statements[i]) && db->Execute(false);

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::Ready()
{
    if (!EnsureSchema())
        return false;

    // Runtime content is migration-owned. Verify the required definition tables
    // are present and seeded instead of silently running with no activities.
    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    int activityCount = 0;
    int challengeCount = 0;
    int powerCount = 0;
    bool ok = db->Prepare(
                  "SELECT "
                  "(SELECT COUNT(*) FROM CovenantActivityDefinition WHERE Enabled=1),"
                  "(SELECT COUNT(*) FROM CovenantChallengeDefinition WHERE Enabled=1),"
                  "(SELECT COUNT(*) FROM CovenantPowerDefinition WHERE Enabled=1)") &&
              db->Execute() &&
              db->GetData(&activityCount, sizeof(activityCount), 1, PARAMTYPE_Integer) &&
              db->GetData(&challengeCount, sizeof(challengeCount), 2, PARAMTYPE_Integer) &&
              db->GetData(&powerCount, sizeof(powerCount), 3, PARAMTYPE_Integer);
    db->Close();
    return ok && activityCount >= 5 && challengeCount >= 15 && powerCount >= 3;
}

bool SQLiteCovenantRuntimeRepository::GetSwitchCooldownUntil(
    const std::string& playerKey, std::int64_t& value)
{
    value = 0;
    if (playerKey.empty() || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    bool ok = false;
    if (db->Prepare("SELECT SwitchCooldownUntil FROM CovenantPlayerState WHERE PlayerKey=?") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String))
    {
        if (db->Execute())
        {
            long long stored = 0;
            ok = db->GetData(&stored, sizeof(stored), 1, PARAMTYPE_Int64);
            if (ok)
                value = stored;
        }
        else
        {
            // No row is the initial state, not a persistence failure.
            ok = NoDatabaseError(db);
        }
    }

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::SetSwitchCooldownUntil(
    const std::string& playerKey, std::int64_t value)
{
    if (playerKey.empty() || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    long long stored = value;
    const bool ok =
        db->Prepare(
            "INSERT OR REPLACE INTO CovenantPlayerState"
            "(PlayerKey,SwitchCooldownUntil,UpdatedAt) "
            "VALUES(?,?,datetime('now','localtime'))") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&stored, 2, PARAMTYPE_Int64) &&
        db->Execute(false);

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::GrantMastery(
    const std::string& playerKey, CovenantId covenantId, std::int64_t now)
{
    if (playerKey.empty() || covenantId <= 0 || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    int id = covenantId;
    long long masteredAt = now;
    const bool ok =
        db->Prepare(
            "INSERT OR IGNORE INTO CovenantMastery(PlayerKey,CovenantId,MasteredAt) "
            "VALUES(?,?,?)") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
        db->BindInputParameter(&masteredAt, 3, PARAMTYPE_Int64) &&
        db->Execute(false);

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::HasMastery(
    const std::string& playerKey, CovenantId covenantId, bool& value)
{
    value = false;
    if (playerKey.empty() || covenantId <= 0 || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    int id = covenantId;
    bool ok = false;
    if (db->Prepare("SELECT 1 FROM CovenantMastery WHERE PlayerKey=? AND CovenantId=?") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&id, 2, PARAMTYPE_Integer))
    {
        if (db->Execute())
        {
            value = true;
            ok = true;
        }
        else
        {
            ok = NoDatabaseError(db);
        }
    }

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::ListMasteries(
    const std::string& playerKey, std::vector<CovenantId>& covenantIds)
{
    covenantIds.clear();
    if (playerKey.empty() || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    bool ok = db->Prepare(
                  "SELECT CovenantId FROM CovenantMastery WHERE PlayerKey=? ORDER BY CovenantId") &&
              db->BindInputParameter(player, 1, PARAMTYPE_String) &&
              db->Execute(false);

    while (ok && db->NextRow())
    {
        int id = 0;
        if (!db->GetData(&id, sizeof(id), 1, PARAMTYPE_Integer))
        {
            ok = false;
            break;
        }
        covenantIds.push_back(id);
    }

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::GrantCollectible(
    const std::string& playerKey,
    CovenantId covenantId,
    const std::string& key,
    std::int64_t now)
{
    if (playerKey.empty() || covenantId <= 0 || key.empty() || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    char collectible[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(collectible, sizeof(collectible), key);
    int id = covenantId;
    long long acquiredAt = now;
    const bool ok =
        db->Prepare(
            "INSERT OR IGNORE INTO CovenantCollection"
            "(PlayerKey,CovenantId,CollectibleKey,AcquiredAt) VALUES(?,?,?,?)") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
        db->BindInputParameter(collectible, 3, PARAMTYPE_String) &&
        db->BindInputParameter(&acquiredAt, 4, PARAMTYPE_Int64) &&
        db->Execute(false);

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::ListCollectibles(
    const std::string& playerKey, std::vector<std::string>& keys)
{
    keys.clear();
    if (playerKey.empty() || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    bool ok = db->Prepare(
                  "SELECT CollectibleKey FROM CovenantCollection "
                  "WHERE PlayerKey=? ORDER BY CovenantId,CollectibleKey") &&
              db->BindInputParameter(player, 1, PARAMTYPE_String) &&
              db->Execute(false);

    while (ok && db->NextRow())
    {
        char key[256] = { 0 };
        if (!db->GetData(key, sizeof(key), 1, PARAMTYPE_String))
        {
            ok = false;
            break;
        }
        keys.push_back(key);
    }

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::AddInventoryItem(
    const std::string& playerKey,
    CovenantId covenantId,
    const std::string& key,
    int amount,
    std::int64_t now)
{
    if (playerKey.empty() || covenantId <= 0 || key.empty() || amount <= 0 || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db) || !db->BeginTransaction())
        return false;

    char player[256];
    char itemKey[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(itemKey, sizeof(itemKey), key);
    int id = covenantId;
    int quantity = 0;

    // Avoid modern SQLite UPSERT syntax. Read the current stack and then
    // replace the complete row, which works with old sqlite3.dll versions.
    bool ok = db->Prepare(
                  "SELECT Quantity FROM CovenantInventory "
                  "WHERE PlayerKey=? AND CovenantId=? AND ItemKey=?") &&
              db->BindInputParameter(player, 1, PARAMTYPE_String) &&
              db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
              db->BindInputParameter(itemKey, 3, PARAMTYPE_String);
    if (ok)
    {
        if (db->Execute())
            ok = db->GetData(&quantity, sizeof(quantity), 1, PARAMTYPE_Integer);
        else
            ok = NoDatabaseError(db); // no existing stack is a valid initial state
    }

    if (ok)
    {
        const long long sum = static_cast<long long>(quantity) + static_cast<long long>(amount);
        quantity = sum > std::numeric_limits<int>::max()
            ? std::numeric_limits<int>::max() : static_cast<int>(sum);
        long long updatedAt = now;
        ok = db->Prepare(
                 "INSERT OR REPLACE INTO CovenantInventory"
                 "(PlayerKey,CovenantId,ItemKey,Quantity,UpdatedAt) "
                 "VALUES(?,?,?,?,datetime(?,'unixepoch','localtime'))") &&
             db->BindInputParameter(player, 1, PARAMTYPE_String) &&
             db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
             db->BindInputParameter(itemKey, 3, PARAMTYPE_String) &&
             db->BindInputParameter(&quantity, 4, PARAMTYPE_Integer) &&
             db->BindInputParameter(&updatedAt, 5, PARAMTYPE_Int64) &&
             db->Execute(false);
    }

    if (ok)
        ok = db->CommitTransaction();
    if (!ok)
        db->RollbackTransaction();
    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::ConsumeInventoryItem(
    const std::string& playerKey,
    CovenantId covenantId,
    const std::string& key,
    int amount)
{
    if (playerKey.empty() || covenantId <= 0 || key.empty() || amount <= 0 || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db) || !db->BeginTransaction())
        return false;

    char player[256];
    char itemKey[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(itemKey, sizeof(itemKey), key);
    int id = covenantId;
    int quantity = 0;
    bool ok = db->Prepare(
                  "SELECT Quantity FROM CovenantInventory "
                  "WHERE PlayerKey=? AND CovenantId=? AND ItemKey=?") &&
              db->BindInputParameter(player, 1, PARAMTYPE_String) &&
              db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
              db->BindInputParameter(itemKey, 3, PARAMTYPE_String) &&
              db->Execute() &&
              db->GetData(&quantity, sizeof(quantity), 1, PARAMTYPE_Integer);
    if (!ok || quantity < amount)
    {
        db->RollbackTransaction();
        db->Close();
        return false;
    }

    int remaining = quantity - amount;
    if (remaining == 0)
    {
        ok = db->Prepare(
                 "DELETE FROM CovenantInventory WHERE PlayerKey=? AND CovenantId=? AND ItemKey=?") &&
             db->BindInputParameter(player, 1, PARAMTYPE_String) &&
             db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
             db->BindInputParameter(itemKey, 3, PARAMTYPE_String) &&
             db->Execute(false);
    }
    else
    {
        ok = db->Prepare(
                 "UPDATE CovenantInventory SET Quantity=?,UpdatedAt=datetime('now','localtime') "
                 "WHERE PlayerKey=? AND CovenantId=? AND ItemKey=?") &&
             db->BindInputParameter(&remaining, 1, PARAMTYPE_Integer) &&
             db->BindInputParameter(player, 2, PARAMTYPE_String) &&
             db->BindInputParameter(&id, 3, PARAMTYPE_Integer) &&
             db->BindInputParameter(itemKey, 4, PARAMTYPE_String) &&
             db->Execute(false);
    }

    if (!ok || !db->CommitTransaction())
    {
        db->RollbackTransaction();
        db->Close();
        return false;
    }
    db->Close();
    return true;
}

bool SQLiteCovenantRuntimeRepository::ListInventoryItems(
    const std::string& playerKey,
    std::vector<CovenantInventoryItem>& items)
{
    items.clear();
    if (playerKey.empty() || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    bool ok = db->Prepare(
                  "SELECT CovenantId,ItemKey,Quantity FROM CovenantInventory "
                  "WHERE PlayerKey=? AND Quantity>0 ORDER BY CovenantId,ItemKey") &&
              db->BindInputParameter(player, 1, PARAMTYPE_String) &&
              db->Execute(false);

    while (ok && db->NextRow())
    {
        CovenantInventoryItem item;
        char key[256] = { 0 };
        if (!db->GetData(&item.covenantId, sizeof(item.covenantId), 1, PARAMTYPE_Integer) ||
            !db->GetData(key, sizeof(key), 2, PARAMTYPE_String) ||
            !db->GetData(&item.quantity, sizeof(item.quantity), 3, PARAMTYPE_Integer))
        {
            ok = false;
            break;
        }
        item.key = key;
        items.push_back(item);
    }

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::GetArtisanMode(
    const std::string& playerKey, int& mode)
{
    mode = 0;
    if (playerKey.empty() || !EnsureSchema())
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;
    char player[128]; CopyKey(player, sizeof(player), playerKey);
    bool ok = db->Prepare("SELECT Mode FROM CovenantArtisanState WHERE PlayerKey=?") &&
              db->BindInputParameter(player, 1, PARAMTYPE_String) && db->Execute();
    if (ok)
    {
        if (!db->GetData(&mode, sizeof(mode), 1, PARAMTYPE_Integer))
        {
            if (NoDatabaseError(db)) mode = 0;
            else ok = false;
        }
    }
    db->Close();
    if (mode < 0 || mode > 3) mode = 0;
    return ok;
}

bool SQLiteCovenantRuntimeRepository::SetArtisanMode(
    const std::string& playerKey, int mode)
{
    if (playerKey.empty() || mode < 0 || mode > 3 || !EnsureSchema())
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;
    char player[128]; CopyKey(player, sizeof(player), playerKey);
    bool ok = db->Prepare(
                  "INSERT OR REPLACE INTO CovenantArtisanState"
                  "(PlayerKey,Mode,UpdatedAt) "
                  "VALUES(?,?,datetime('now','localtime'))") &&
              db->BindInputParameter(player, 1, PARAMTYPE_String) &&
              db->BindInputParameter(&mode, 2, PARAMTYPE_Integer) &&
              db->Execute(false);
    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::GetActivityReputation(
    CovenantId covenantId, const std::string& activityKey, int& reputationPerUnit)
{
    reputationPerUnit = 0;
    if (covenantId <= 0 || activityKey.empty())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    int id = covenantId;
    char activity[256];
    CopyKey(activity, sizeof(activity), activityKey);
    bool ok = false;
    if (db->Prepare(
            "SELECT ReputationPerUnit FROM CovenantActivityDefinition "
            "WHERE CovenantId=? AND ActivityKey=? AND Enabled=1") &&
        db->BindInputParameter(&id, 1, PARAMTYPE_Integer) &&
        db->BindInputParameter(activity, 2, PARAMTYPE_String))
    {
        if (db->Execute())
            ok = db->GetData(&reputationPerUnit, sizeof(reputationPerUnit), 1, PARAMTYPE_Integer);
        else
            ok = false; // no definition means this activity is not relevant
    }

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::ListChallenges(
    const std::string& playerKey,
    CovenantId covenantId,
    std::vector<CovenantChallengeState>& states)
{
    states.clear();
    if (playerKey.empty() || covenantId <= 0)
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    int id = covenantId;
    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    const char* query =
        "SELECT d.ChallengeKey,d.ActivityKey,d.TargetCount,d.ReputationReward,"
        "COALESCE(p.Progress,0),COALESCE(p.Completed,0),COALESCE(p.RewardClaimed,0) "
        "FROM CovenantChallengeDefinition d "
        "LEFT JOIN CovenantChallengeProgress p ON "
        "p.PlayerKey=? AND p.CovenantId=d.CovenantId AND p.ChallengeKey=d.ChallengeKey "
        "WHERE d.CovenantId=? AND d.Enabled=1 ORDER BY d.SortOrder,d.ChallengeKey";

    bool ok = db->Prepare(query) &&
              db->BindInputParameter(player, 1, PARAMTYPE_String) &&
              db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
              db->Execute(false);

    while (ok && db->NextRow())
    {
        CovenantChallengeState state;
        state.covenantId = covenantId;
        char challengeKey[256] = { 0 };
        char activityKey[256] = { 0 };
        int completed = 0;
        int claimed = 0;
        if (!db->GetData(challengeKey, sizeof(challengeKey), 1, PARAMTYPE_String) ||
            !db->GetData(activityKey, sizeof(activityKey), 2, PARAMTYPE_String) ||
            !db->GetData(&state.targetCount, sizeof(state.targetCount), 3, PARAMTYPE_Integer) ||
            !db->GetData(&state.reputationReward, sizeof(state.reputationReward), 4, PARAMTYPE_Integer) ||
            !db->GetData(&state.progress, sizeof(state.progress), 5, PARAMTYPE_Integer) ||
            !db->GetData(&completed, sizeof(completed), 6, PARAMTYPE_Integer) ||
            !db->GetData(&claimed, sizeof(claimed), 7, PARAMTYPE_Integer))
        {
            ok = false;
            break;
        }
        state.key = challengeKey;
        state.activityKey = activityKey;
        state.completed = completed != 0;
        state.rewardClaimed = claimed != 0;
        states.push_back(state);
    }

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::IncrementChallenges(
    const std::string& playerKey,
    CovenantId covenantId,
    const std::string& activityKey,
    int amount,
    std::vector<CovenantChallengeState>& states)
{
    states.clear();
    if (playerKey.empty() || covenantId <= 0 || activityKey.empty() || amount <= 0 ||
        !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db) || !db->BeginTransaction())
        return false;

    char player[256];
    char activity[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(activity, sizeof(activity), activityKey);
    int id = covenantId;

    if (!db->Prepare(
            "SELECT ChallengeKey,TargetCount,ReputationReward "
            "FROM CovenantChallengeDefinition "
            "WHERE CovenantId=? AND ActivityKey=? AND Enabled=1 ORDER BY SortOrder") ||
        !db->BindInputParameter(&id, 1, PARAMTYPE_Integer) ||
        !db->BindInputParameter(activity, 2, PARAMTYPE_String) ||
        !db->Execute(false))
    {
        db->RollbackTransaction();
        db->Close();
        return false;
    }

    struct Definition
    {
        std::string key;
        int target;
        int reward;
    };
    std::vector<Definition> definitions;
    while (db->NextRow())
    {
        char key[256] = { 0 };
        Definition definition;
        if (!db->GetData(key, sizeof(key), 1, PARAMTYPE_String) ||
            !db->GetData(&definition.target, sizeof(definition.target), 2, PARAMTYPE_Integer) ||
            !db->GetData(&definition.reward, sizeof(definition.reward), 3, PARAMTYPE_Integer))
        {
            db->RollbackTransaction();
            db->Close();
            return false;
        }
        definition.key = key;
        definitions.push_back(definition);
    }

    for (std::vector<Definition>::const_iterator it = definitions.begin();
         it != definitions.end(); ++it)
    {
        char key[256];
        CopyKey(key, sizeof(key), it->key);
        int currentProgress = 0;
        int currentCompleted = 0;
        int rewardClaimed = 0;

        bool rowOk = db->Prepare(
                         "SELECT Progress,Completed,RewardClaimed FROM CovenantChallengeProgress "
                         "WHERE PlayerKey=? AND CovenantId=? AND ChallengeKey=?") &&
                     db->BindInputParameter(player, 1, PARAMTYPE_String) &&
                     db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
                     db->BindInputParameter(key, 3, PARAMTYPE_String);
        if (rowOk)
        {
            if (db->Execute())
            {
                rowOk = db->GetData(&currentProgress, sizeof(currentProgress), 1, PARAMTYPE_Integer) &&
                        db->GetData(&currentCompleted, sizeof(currentCompleted), 2, PARAMTYPE_Integer) &&
                        db->GetData(&rewardClaimed, sizeof(rewardClaimed), 3, PARAMTYPE_Integer);
            }
            else
            {
                rowOk = NoDatabaseError(db); // first progress for this challenge
            }
        }
        if (!rowOk)
        {
            db->RollbackTransaction();
            db->Close();
            return false;
        }

        const long long advanced = static_cast<long long>(currentProgress) + amount;
        int progress = advanced >= it->target ? it->target : static_cast<int>(advanced);
        int completed = (currentCompleted != 0 || progress >= it->target) ? 1 : 0;
        if (!db->Prepare(
                "INSERT OR REPLACE INTO CovenantChallengeProgress"
                "(PlayerKey,CovenantId,ChallengeKey,Progress,Completed,RewardClaimed,UpdatedAt) "
                "VALUES(?,?,?,?,?,?,datetime('now','localtime'))") ||
            !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
            !db->BindInputParameter(&id, 2, PARAMTYPE_Integer) ||
            !db->BindInputParameter(key, 3, PARAMTYPE_String) ||
            !db->BindInputParameter(&progress, 4, PARAMTYPE_Integer) ||
            !db->BindInputParameter(&completed, 5, PARAMTYPE_Integer) ||
            !db->BindInputParameter(&rewardClaimed, 6, PARAMTYPE_Integer) ||
            !db->Execute(false))
        {
            db->RollbackTransaction();
            db->Close();
            return false;
        }
    }

    if (!db->CommitTransaction())
    {
        db->RollbackTransaction();
        db->Close();
        return false;
    }
    db->Close();
    return ListChallenges(playerKey, covenantId, states);
}

bool SQLiteCovenantRuntimeRepository::MarkChallengeRewardClaimed(
    const std::string& playerKey, CovenantId covenantId, const std::string& challengeKey)
{
    if (playerKey.empty() || covenantId <= 0 || challengeKey.empty())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    char key[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(key, sizeof(key), challengeKey);
    int id = covenantId;
    const bool ok =
        db->Prepare(
            "UPDATE CovenantChallengeProgress SET RewardClaimed=1,"
            "UpdatedAt=datetime('now','localtime') "
            "WHERE PlayerKey=? AND CovenantId=? AND ChallengeKey=? AND Completed=1") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
        db->BindInputParameter(key, 3, PARAMTYPE_String) &&
        db->Execute(false);

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::AllChallengesCompleted(
    const std::string& playerKey, CovenantId covenantId, bool& completed)
{
    completed = false;
    std::vector<CovenantChallengeState> states;
    if (!ListChallenges(playerKey, covenantId, states) || states.empty())
        return false;

    completed = true;
    for (std::vector<CovenantChallengeState>::const_iterator it = states.begin();
         it != states.end(); ++it)
    {
        if (!it->completed)
        {
            completed = false;
            break;
        }
    }
    return true;
}

bool SQLiteCovenantRuntimeRepository::LoadPowerDefinition(
    CovenantId covenantId, const std::string& powerKey, CovenantPowerDefinition& definition)
{
    definition = CovenantPowerDefinition();
    if (covenantId <= 0 || powerKey.empty())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    int id = covenantId;
    char key[256];
    CopyKey(key, sizeof(key), powerKey);
    int mode = 0;
    bool ok =
        db->Prepare(
            "SELECT RequiredRankId,DurationSeconds,CooldownSeconds,TargetMode "
            "FROM CovenantPowerDefinition WHERE CovenantId=? AND PowerKey=? AND Enabled=1") &&
        db->BindInputParameter(&id, 1, PARAMTYPE_Integer) &&
        db->BindInputParameter(key, 2, PARAMTYPE_String) &&
        db->Execute() &&
        db->GetData(&definition.requiredRankId, sizeof(definition.requiredRankId), 1,
                    PARAMTYPE_Integer) &&
        db->GetData(&definition.durationSeconds, sizeof(definition.durationSeconds), 2,
                    PARAMTYPE_Integer) &&
        db->GetData(&definition.cooldownSeconds, sizeof(definition.cooldownSeconds), 3,
                    PARAMTYPE_Integer) &&
        db->GetData(&mode, sizeof(mode), 4, PARAMTYPE_Integer);

    if (ok)
    {
        definition.covenantId = covenantId;
        definition.key = powerKey;
        definition.targetMode = mode == 1 ? CovenantPowerTargetMode::HuntTarget
                                          : CovenantPowerTargetMode::Self;
    }

    if (ok)
    {
        ok = db->Prepare(
                 "SELECT StatKey,Operation,Value,ScopeKey FROM CovenantPowerModifier "
                 "WHERE CovenantId=? AND PowerKey=? ORDER BY SortOrder") &&
             db->BindInputParameter(&id, 1, PARAMTYPE_Integer) &&
             db->BindInputParameter(key, 2, PARAMTYPE_String) &&
             db->Execute(false);
    }

    while (ok && db->NextRow())
    {
        modifiers::GameplayModifier modifier;
        char statKey[256] = { 0 };
        char scopeKey[256] = { 0 };
        int operation = 0;
        float value = 0.0f;
        if (!db->GetData(statKey, sizeof(statKey), 1, PARAMTYPE_String) ||
            !db->GetData(&operation, sizeof(operation), 2, PARAMTYPE_Integer) ||
            !db->GetData(&value, sizeof(value), 3, PARAMTYPE_Float) ||
            !db->GetData(scopeKey, sizeof(scopeKey), 4, PARAMTYPE_String))
        {
            ok = false;
            break;
        }
        modifier.value = static_cast<double>(value);
        modifier.statKey = statKey;
        modifier.scopeKey = scopeKey;
        modifier.operation = static_cast<modifiers::ModifierOperation>(operation);
        modifier.sourceKey = std::string("covenant:power:") + powerKey;
        definition.modifiers.push_back(modifier);
    }

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::LoadPowerState(
    const std::string& playerKey,
    CovenantId covenantId,
    const std::string& powerKey,
    CovenantPowerState& state)
{
    state = CovenantPowerState();
    state.covenantId = covenantId;
    state.key = powerKey;
    if (playerKey.empty() || covenantId <= 0 || powerKey.empty() || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    char key[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(key, sizeof(key), powerKey);
    int id = covenantId;
    bool ok = true;
    if (db->Prepare(
            "SELECT ActiveUntil,CooldownUntil,TargetSerial FROM CovenantPowerState "
            "WHERE PlayerKey=? AND CovenantId=? AND PowerKey=?") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
        db->BindInputParameter(key, 3, PARAMTYPE_String))
    {
        if (db->Execute())
        {
            long long activeUntil = 0;
            long long cooldownUntil = 0;
            int targetSerial = 0;
            ok = db->GetData(&activeUntil, sizeof(activeUntil), 1, PARAMTYPE_Int64) &&
                 db->GetData(&cooldownUntil, sizeof(cooldownUntil), 2, PARAMTYPE_Int64) &&
                 db->GetData(&targetSerial, sizeof(targetSerial), 3, PARAMTYPE_Integer);
            if (ok)
            {
                state.activeUntil = activeUntil;
                state.cooldownUntil = cooldownUntil;
                state.targetSerial = static_cast<std::uint32_t>(targetSerial);
            }
        }
        else
        {
            ok = NoDatabaseError(db); // first use: all timestamps remain zero
        }
    }
    else
    {
        ok = false;
    }

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::SavePowerState(
    const std::string& playerKey, const CovenantPowerState& state)
{
    if (playerKey.empty() || state.covenantId <= 0 || state.key.empty() || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    char key[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(key, sizeof(key), state.key);
    int id = state.covenantId;
    int targetSerial = static_cast<int>(state.targetSerial);
    long long activeUntil = state.activeUntil;
    long long cooldownUntil = state.cooldownUntil;
    const bool ok =
        db->Prepare(
            "INSERT OR REPLACE INTO CovenantPowerState"
            "(PlayerKey,CovenantId,PowerKey,ActiveUntil,CooldownUntil,TargetSerial,UpdatedAt) "
            "VALUES(?,?,?,?,?,?,datetime('now','localtime'))") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
        db->BindInputParameter(key, 3, PARAMTYPE_String) &&
        db->BindInputParameter(&activeUntil, 4, PARAMTYPE_Int64) &&
        db->BindInputParameter(&cooldownUntil, 5, PARAMTYPE_Int64) &&
        db->BindInputParameter(&targetSerial, 6, PARAMTYPE_Integer) &&
        db->Execute(false);

    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::DeactivatePowers(
    const std::string& playerKey, CovenantId covenantId)
{
    if (playerKey.empty() || covenantId <= 0 || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    int id = covenantId;
    // Preserve CooldownUntil deliberately: switching specialization must never
    // reset or shorten the Tier III cooldown. Only the active effect/target is
    // cancelled.
    const bool ok =
        db->Prepare(
            "UPDATE CovenantPowerState SET ActiveUntil=0,TargetSerial=0,"
            "UpdatedAt=datetime('now','localtime') "
            "WHERE PlayerKey=? AND CovenantId=? AND ActiveUntil>0") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
        db->Execute(false);
    db->Close();
    return ok;
}

bool SQLiteCovenantRuntimeRepository::ListActivePowerModifiers(
    const std::string& playerKey,
    CovenantId covenantId,
    std::int64_t now,
    std::uint32_t contextTargetSerial,
    std::vector<modifiers::GameplayModifier>& modifiersOut)
{
    modifiersOut.clear();
    if (playerKey.empty() || covenantId <= 0)
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    int id = covenantId;
    long long currentTime = now;
    bool ok =
        db->Prepare(
            "SELECT PowerKey,TargetSerial FROM CovenantPowerState "
            "WHERE PlayerKey=? AND CovenantId=? AND ActiveUntil>?") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&id, 2, PARAMTYPE_Integer) &&
        db->BindInputParameter(&currentTime, 3, PARAMTYPE_Int64) &&
        db->Execute(false);

    std::vector<std::pair<std::string, std::uint32_t> > powers;
    while (ok && db->NextRow())
    {
        char key[256] = { 0 };
        int targetSerial = 0;
        if (!db->GetData(key, sizeof(key), 1, PARAMTYPE_String) ||
            !db->GetData(&targetSerial, sizeof(targetSerial), 2, PARAMTYPE_Integer))
        {
            ok = false;
            break;
        }
        powers.push_back(std::make_pair(
            std::string(key), static_cast<std::uint32_t>(targetSerial)));
    }
    db->Close();
    if (!ok)
        return false;

    for (std::vector<std::pair<std::string, std::uint32_t> >::const_iterator it =
             powers.begin();
         it != powers.end(); ++it)
    {
        CovenantPowerDefinition definition;
        if (!LoadPowerDefinition(covenantId, it->first, definition))
            return false;
        if (definition.targetMode == CovenantPowerTargetMode::HuntTarget &&
            it->second != 0 && it->second != contextTargetSerial)
            continue;
        modifiersOut.insert(
            modifiersOut.end(), definition.modifiers.begin(), definition.modifiers.end());
    }
    return true;
}

}
}
