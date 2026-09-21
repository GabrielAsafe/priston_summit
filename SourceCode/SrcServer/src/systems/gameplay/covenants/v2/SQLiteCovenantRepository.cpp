#include "systems/gameplay/covenants/v2/SQLiteCovenantRepository.h"

#include <cstring>
#include <memory>

#include "systems/persistence/database/core/Database.h"

namespace gameplay
{
namespace covenants
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

bool BindAndSaveMembership(SQLConnection& db,
                           const std::string& playerKey,
                           const CovenantMembershipState& membership)
{
    if (!db.Prepare(
            "INSERT INTO CovenantMembership(PlayerKey,CovenantId,Reputation,RankId,Active,UpdatedAt) "
            "VALUES(?,?,?,?,?,datetime('now','localtime')) "
            "ON CONFLICT(PlayerKey,CovenantId) DO UPDATE SET "
            "Reputation=excluded.Reputation, RankId=excluded.RankId, Active=excluded.Active, "
            "UpdatedAt=datetime('now','localtime')"))
        return false;

    char key[256];
    CopyKey(key, sizeof(key), playerKey);
    int covenantId = membership.covenantId;
    int reputation = membership.reputation;
    int rankId = membership.rankId;
    int active = membership.active ? 1 : 0;
    return db.BindInputParameter(key, 1, PARAMTYPE_String) &&
           db.BindInputParameter(&covenantId, 2, PARAMTYPE_Integer) &&
           db.BindInputParameter(&reputation, 3, PARAMTYPE_Integer) &&
           db.BindInputParameter(&rankId, 4, PARAMTYPE_Integer) &&
           db.BindInputParameter(&active, 5, PARAMTYPE_Integer) &&
           db.Execute(false);
}
}

bool SQLiteCovenantRepository::EnsureSchema()
{
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    const char* statements[] = {
        "CREATE TABLE IF NOT EXISTS ServerDB__CovenantMembership ("
        "PlayerKey TEXT NOT NULL, CovenantId INTEGER NOT NULL CHECK (CovenantId > 0), Reputation INTEGER NOT NULL, "
        "RankId INTEGER NOT NULL, Active INTEGER NOT NULL CHECK (Active IN (0,1)), "
        "UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')), "
        "PRIMARY KEY(PlayerKey, CovenantId))",
        "CREATE TABLE IF NOT EXISTS ServerDB__CovenantReputationLedger ("
        "PlayerKey TEXT NOT NULL, CovenantId INTEGER NOT NULL CHECK (CovenantId > 0), SourceKey TEXT NOT NULL, "
        "Delta INTEGER NOT NULL, ReputationAfter INTEGER NOT NULL, RankIdAfter INTEGER NOT NULL, "
        "AppliedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')), "
        "PRIMARY KEY(PlayerKey, CovenantId, SourceKey))",
        "CREATE TABLE IF NOT EXISTS ServerDB__CovenantEffectReceipt ("
        "PlayerKey TEXT NOT NULL CHECK(length(PlayerKey) BETWEEN 1 AND 255), "
        "CovenantId INTEGER NOT NULL CHECK(CovenantId > 0), "
        "EffectKey TEXT NOT NULL CHECK(length(EffectKey) BETWEEN 1 AND 255), "
        "Applied INTEGER NOT NULL DEFAULT 0 CHECK(Applied IN (0,1)), "
        "ClaimedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')), "
        "UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')), "
        "PRIMARY KEY(PlayerKey, CovenantId, EffectKey))"
    };
    bool ok = true;
    for (int i = 0; i < 3 && ok; ++i)
        ok = db->Prepare(statements[i]) && db->Execute(false);
    db->Close();
    return ok;
}

bool SQLiteCovenantRepository::LoadAll(const std::string& playerKey,
                                       std::vector<CovenantMembershipState>& memberships)
{
    memberships.clear();
    if (playerKey.empty() || playerKey.size() > 255u || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    bool ok = false;
    if (db->Prepare("SELECT CovenantId, Reputation, RankId, Active FROM CovenantMembership WHERE PlayerKey=?"))
    {
        char key[256];
        CopyKey(key, sizeof(key), playerKey);
        if (db->BindInputParameter(key, 1, PARAMTYPE_String) && db->Execute(false))
        {
            ok = true;
            while (db->NextRow())
            {
                CovenantMembershipState state;
                int active = 0;
                if (!db->GetData(&state.covenantId, sizeof(state.covenantId), 1, PARAMTYPE_Integer) ||
                    !db->GetData(&state.reputation, sizeof(state.reputation), 2, PARAMTYPE_Integer) ||
                    !db->GetData(&state.rankId, sizeof(state.rankId), 3, PARAMTYPE_Integer) ||
                    !db->GetData(&active, sizeof(active), 4, PARAMTYPE_Integer) ||
                    state.covenantId <= 0 || (active != 0 && active != 1))
                {
                    ok = false;
                    memberships.clear();
                    break;
                }
                state.active = active != 0;
                memberships.push_back(state);
            }
        }
    }
    db->Close();
    return ok;
}

bool SQLiteCovenantRepository::Save(const std::string& playerKey,
                                    const CovenantMembershipState& membership)
{
    if (playerKey.empty() || playerKey.size() > 255u || membership.covenantId <= 0 || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    const bool ok = BindAndSaveMembership(*db, playerKey, membership);
    db->Close();
    return ok;
}


bool SQLiteCovenantRepository::TryJoin(
    const std::string& playerKey,
    const CovenantMembershipState& membership,
    bool targetExclusive,
    const std::vector<CovenantId>& knownCovenantIds,
    const std::vector<CovenantId>& exclusiveCovenantIds)
{
    if (playerKey.empty() || playerKey.size() > 255u || membership.covenantId <= 0 ||
        !membership.active || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db) || !db->BeginTransaction())
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    if (!db->Prepare("SELECT CovenantId FROM CovenantMembership WHERE PlayerKey=? AND Active=1") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) || !db->Execute(false))
    {
        db->RollbackTransaction(); db->Close(); return false;
    }

    bool blocked = false;
    while (db->NextRow())
    {
        int activeCovenantId = 0;
        if (!db->GetData(&activeCovenantId, sizeof(activeCovenantId), 1, PARAMTYPE_Integer) ||
            activeCovenantId <= 0)
        {
            db->RollbackTransaction(); db->Close(); return false;
        }
        if (activeCovenantId == membership.covenantId || targetExclusive)
        {
            blocked = true;
            break;
        }
        bool known = false;
        for (std::vector<CovenantId>::const_iterator it = knownCovenantIds.begin();
             it != knownCovenantIds.end(); ++it)
        {
            if (*it == activeCovenantId)
            {
                known = true;
                break;
            }
        }
        if (!known)
        {
            blocked = true;
            break;
        }
        for (std::vector<CovenantId>::const_iterator it = exclusiveCovenantIds.begin();
             it != exclusiveCovenantIds.end(); ++it)
        {
            if (*it == activeCovenantId)
            {
                blocked = true;
                break;
            }
        }
        if (blocked) break;
    }

    if (blocked || !BindAndSaveMembership(*db, playerKey, membership) || !db->CommitTransaction())
    {
        db->RollbackTransaction(); db->Close(); return false;
    }
    db->Close();
    return true;
}

bool SQLiteCovenantRepository::SaveIfUnchanged(
    const std::string& playerKey,
    const CovenantMembershipState& expected,
    const CovenantMembershipState& membership)
{
    if (playerKey.empty() || playerKey.size() > 255u || membership.covenantId <= 0 ||
        expected.covenantId != membership.covenantId || !EnsureSchema())
        return false;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db) || !db->BeginTransaction())
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    int covenantId = membership.covenantId;
    if (!db->Prepare("SELECT Reputation,RankId,Active FROM CovenantMembership WHERE PlayerKey=? AND CovenantId=?") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(&covenantId, 2, PARAMTYPE_Integer) || !db->Execute())
    {
        db->RollbackTransaction(); db->Close(); return false;
    }

    int reputation = 0;
    int rankId = 0;
    int active = 0;
    if (!db->GetData(&reputation, sizeof(reputation), 1, PARAMTYPE_Integer) ||
        !db->GetData(&rankId, sizeof(rankId), 2, PARAMTYPE_Integer) ||
        !db->GetData(&active, sizeof(active), 3, PARAMTYPE_Integer) ||
        (active != 0 && active != 1) ||
        reputation != expected.reputation || rankId != expected.rankId ||
        ((active != 0) != expected.active))
    {
        db->RollbackTransaction(); db->Close(); return false;
    }

    if (!BindAndSaveMembership(*db, playerKey, membership) || !db->CommitTransaction())
    {
        db->RollbackTransaction(); db->Close(); return false;
    }
    db->Close();
    return true;
}

CovenantReputationSaveResult SQLiteCovenantRepository::SaveReputationChange(
    const std::string& playerKey,
    const CovenantMembershipState& membership,
    int delta,
    const std::string& sourceKey)
{
    CovenantMembershipState expected = membership;
    expected.reputation = membership.reputation - delta;
    return SaveReputationChangeChecked(playerKey, expected, membership, delta, sourceKey);
}

CovenantReputationSaveResult SQLiteCovenantRepository::SaveReputationChangeChecked(
    const std::string& playerKey,
    const CovenantMembershipState& expected,
    const CovenantMembershipState& membership,
    int delta,
    const std::string& sourceKey)
{
    if (playerKey.empty() || playerKey.size() > 255u ||
        membership.covenantId <= 0 || sourceKey.empty() || sourceKey.size() > 255u ||
        expected.covenantId != membership.covenantId || !EnsureSchema())
        return CovenantReputationSaveResult::Failed;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db) || !db->BeginTransaction())
        return CovenantReputationSaveResult::Failed;

    char player[256];
    char source[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(source, sizeof(source), sourceKey);
    int covenantId = membership.covenantId;

    if (!db->Prepare(
            "SELECT Delta FROM CovenantReputationLedger WHERE PlayerKey=? AND CovenantId=? AND SourceKey=?") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(&covenantId, 2, PARAMTYPE_Integer) ||
        !db->BindInputParameter(source, 3, PARAMTYPE_String))
    {
        db->RollbackTransaction(); db->Close(); return CovenantReputationSaveResult::Failed;
    }

    if (db->Execute())
    {
        int existingDelta = 0;
        if (!db->GetData(&existingDelta, sizeof(existingDelta), 1, PARAMTYPE_Integer))
        {
            db->RollbackTransaction(); db->Close(); return CovenantReputationSaveResult::Failed;
        }
        const bool committed = db->CommitTransaction();
        db->Close();
        if (!committed) return CovenantReputationSaveResult::Failed;
        return existingDelta == delta ? CovenantReputationSaveResult::AlreadyApplied
                                      : CovenantReputationSaveResult::Failed;
    }
    if (db->GetLastError() && db->GetLastError()[0] != 0)
    {
        db->RollbackTransaction(); db->Close(); return CovenantReputationSaveResult::Failed;
    }

    if (!db->Prepare(
            "SELECT Reputation,RankId,Active FROM CovenantMembership WHERE PlayerKey=? AND CovenantId=?") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(&covenantId, 2, PARAMTYPE_Integer))
    {
        db->RollbackTransaction(); db->Close(); return CovenantReputationSaveResult::Failed;
    }

    if (!db->Execute())
    {
        const char* error = db->GetLastError();
        const bool dbFailed = error && error[0] != 0;
        db->RollbackTransaction(); db->Close();
        return dbFailed ? CovenantReputationSaveResult::Failed
                        : CovenantReputationSaveResult::Conflict;
    }

    int currentReputation = 0;
    int currentRank = 0;
    int currentActive = 0;
    if (!db->GetData(&currentReputation, sizeof(currentReputation), 1, PARAMTYPE_Integer) ||
        !db->GetData(&currentRank, sizeof(currentRank), 2, PARAMTYPE_Integer) ||
        !db->GetData(&currentActive, sizeof(currentActive), 3, PARAMTYPE_Integer) ||
        (currentActive != 0 && currentActive != 1))
    {
        db->RollbackTransaction(); db->Close(); return CovenantReputationSaveResult::Failed;
    }

    if (currentReputation != expected.reputation || currentRank != expected.rankId ||
        (currentActive != 0) != expected.active)
    {
        db->RollbackTransaction(); db->Close(); return CovenantReputationSaveResult::Conflict;
    }

    if (!BindAndSaveMembership(*db, playerKey, membership))
    {
        db->RollbackTransaction(); db->Close(); return CovenantReputationSaveResult::Failed;
    }

    int reputationAfter = membership.reputation;
    int rankIdAfter = membership.rankId;
    if (!db->Prepare(
            "INSERT INTO CovenantReputationLedger(PlayerKey,CovenantId,SourceKey,Delta,ReputationAfter,RankIdAfter,AppliedAt) "
            "VALUES(?,?,?,?,?,?,datetime('now','localtime'))") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(&covenantId, 2, PARAMTYPE_Integer) ||
        !db->BindInputParameter(source, 3, PARAMTYPE_String) ||
        !db->BindInputParameter(&delta, 4, PARAMTYPE_Integer) ||
        !db->BindInputParameter(&reputationAfter, 5, PARAMTYPE_Integer) ||
        !db->BindInputParameter(&rankIdAfter, 6, PARAMTYPE_Integer) ||
        !db->Execute(false))
    {
        db->RollbackTransaction(); db->Close(); return CovenantReputationSaveResult::Failed;
    }

    if (!db->CommitTransaction())
    {
        db->RollbackTransaction(); db->Close(); return CovenantReputationSaveResult::Failed;
    }
    db->Close();
    return CovenantReputationSaveResult::Applied;
}

CovenantEffectClaimResult SQLiteCovenantRepository::ClaimEffect(
    const std::string& playerKey,
    CovenantId covenantId,
    const std::string& effectKey)
{
    if (playerKey.empty() || playerKey.size() > 255u || covenantId <= 0 ||
        effectKey.empty() || effectKey.size() > 255u || !EnsureSchema())
        return CovenantEffectClaimResult::Failed;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db) || !db->BeginTransaction())
        return CovenantEffectClaimResult::Failed;

    char player[256];
    char effect[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(effect, sizeof(effect), effectKey);
    if (!db->Prepare("SELECT Applied FROM CovenantEffectReceipt WHERE PlayerKey=? AND CovenantId=? AND EffectKey=?") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(&covenantId, 2, PARAMTYPE_Integer) ||
        !db->BindInputParameter(effect, 3, PARAMTYPE_String))
    {
        db->RollbackTransaction(); db->Close(); return CovenantEffectClaimResult::Failed;
    }

    int applied = 0;
    if (db->Execute())
    {
        if (!db->GetData(&applied, sizeof(applied), 1, PARAMTYPE_Integer) ||
            (applied != 0 && applied != 1) || !db->CommitTransaction())
        {
            db->RollbackTransaction(); db->Close(); return CovenantEffectClaimResult::Failed;
        }
        db->Close();
        return applied != 0 ? CovenantEffectClaimResult::AlreadyApplied
                            : CovenantEffectClaimResult::Pending;
    }
    if (db->GetLastError() && db->GetLastError()[0] != 0)
    {
        db->RollbackTransaction(); db->Close(); return CovenantEffectClaimResult::Failed;
    }

    if (!db->Prepare(
            "INSERT INTO CovenantEffectReceipt(PlayerKey,CovenantId,EffectKey,Applied,UpdatedAt) "
            "VALUES(?,?,?,0,datetime('now','localtime'))") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(&covenantId, 2, PARAMTYPE_Integer) ||
        !db->BindInputParameter(effect, 3, PARAMTYPE_String) ||
        !db->Execute(false) || !db->CommitTransaction())
    {
        db->RollbackTransaction(); db->Close(); return CovenantEffectClaimResult::Failed;
    }
    db->Close();
    return CovenantEffectClaimResult::Claimed;
}

bool SQLiteCovenantRepository::MarkEffectApplied(
    const std::string& playerKey,
    CovenantId covenantId,
    const std::string& effectKey)
{
    if (playerKey.empty() || playerKey.size() > 255u || covenantId <= 0 ||
        effectKey.empty() || effectKey.size() > 255u || !EnsureSchema())
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    char player[256];
    char effect[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(effect, sizeof(effect), effectKey);
    const bool ok = db->Prepare(
            "UPDATE CovenantEffectReceipt SET Applied=1,UpdatedAt=datetime('now','localtime') "
            "WHERE PlayerKey=? AND CovenantId=? AND EffectKey=?") &&
        db->BindInputParameter(player, 1, PARAMTYPE_String) &&
        db->BindInputParameter(&covenantId, 2, PARAMTYPE_Integer) &&
        db->BindInputParameter(effect, 3, PARAMTYPE_String) && db->Execute(false);
    db->Close();
    return ok;
}

CovenantEffectReceiptLoadResult SQLiteCovenantRepository::GetEffectReceiptStatus(
    const std::string& playerKey,
    CovenantId covenantId,
    const std::string& effectKey,
    CovenantEffectReceiptStatus& status)
{
    if (playerKey.empty() || playerKey.size() > 255u || covenantId <= 0 ||
        effectKey.empty() || effectKey.size() > 255u || !EnsureSchema())
        return CovenantEffectReceiptLoadResult::Failed;
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return CovenantEffectReceiptLoadResult::Failed;
    char player[256];
    char effect[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(effect, sizeof(effect), effectKey);
    if (!db->Prepare("SELECT Applied FROM CovenantEffectReceipt WHERE PlayerKey=? AND CovenantId=? AND EffectKey=?") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(&covenantId, 2, PARAMTYPE_Integer) ||
        !db->BindInputParameter(effect, 3, PARAMTYPE_String))
    {
        db->Close(); return CovenantEffectReceiptLoadResult::Failed;
    }
    if (!db->Execute())
    {
        const char* error = db->GetLastError();
        const bool failed = error && error[0] != 0;
        db->Close();
        return failed ? CovenantEffectReceiptLoadResult::Failed
                      : CovenantEffectReceiptLoadResult::NotFound;
    }
    int applied = 0;
    const bool ok = db->GetData(&applied, sizeof(applied), 1, PARAMTYPE_Integer);
    db->Close();
    if (!ok || (applied != 0 && applied != 1))
        return CovenantEffectReceiptLoadResult::Failed;
    status = applied != 0 ? CovenantEffectReceiptStatus::Applied
                          : CovenantEffectReceiptStatus::Pending;
    return CovenantEffectReceiptLoadResult::Found;
}

bool SQLiteCovenantRepository::ListPendingEffects(
    const std::string& playerKey,
    std::vector<CovenantEffectReceiptEntry>& receipts)
{
    receipts.clear();
    if (playerKey.empty() || playerKey.size() > 255u || !EnsureSchema())
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    if (!db->Prepare(
            "SELECT CovenantId,EffectKey,Applied FROM CovenantEffectReceipt "
            "WHERE PlayerKey=? AND Applied=0 ORDER BY CovenantId,EffectKey") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) || !db->Execute(false))
    {
        db->Close(); return false;
    }
    while (db->NextRow())
    {
        CovenantEffectReceiptEntry entry;
        char effect[256] = {0};
        int applied = -1;
        if (!db->GetData(&entry.covenantId, sizeof(entry.covenantId), 1, PARAMTYPE_Integer) ||
            !db->GetData(effect, sizeof(effect), 2, PARAMTYPE_String) ||
            !db->GetData(&applied, sizeof(applied), 3, PARAMTYPE_Integer) ||
            entry.covenantId <= 0 || effect[0] == 0 || applied != 0)
        {
            receipts.clear(); db->Close(); return false;
        }
        entry.effectKey = effect;
        entry.status = CovenantEffectReceiptStatus::Pending;
        receipts.push_back(entry);
    }
    const char* error = db->GetLastError();
    const bool ok = !error || error[0] == 0;
    db->Close();
    if (!ok) receipts.clear();
    return ok;
}

bool SQLiteCovenantRepository::AcknowledgePendingEffect(
    const std::string& playerKey,
    CovenantId covenantId,
    const std::string& effectKey)
{
    CovenantEffectReceiptStatus status = CovenantEffectReceiptStatus::Pending;
    const CovenantEffectReceiptLoadResult loaded =
        GetEffectReceiptStatus(playerKey, covenantId, effectKey, status);
    if (loaded != CovenantEffectReceiptLoadResult::Found)
        return false;
    return status == CovenantEffectReceiptStatus::Applied ||
           MarkEffectApplied(playerKey, covenantId, effectKey);
}

}
}
