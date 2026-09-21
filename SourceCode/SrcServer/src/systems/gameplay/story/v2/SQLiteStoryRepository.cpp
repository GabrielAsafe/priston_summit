#include "systems/gameplay/story/v2/SQLiteStoryRepository.h"

#include <cstring>
#include <memory>

#include "systems/persistence/database/core/Database.h"

namespace gameplay
{
namespace story
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

bool SameProgress(const StoryProgressState& left, const StoryProgressState& right)
{
    return left.storyId == right.storyId &&
           left.chapterId == right.chapterId &&
           left.nodeId == right.nodeId &&
           left.status == right.status &&
           left.enterActionsApplied == right.enterActionsApplied &&
           left.completionActionsApplied == right.completionActionsApplied;
}

StoryProgressLoadResult ReadProgress(SQLConnection& db,
                                     const std::string& playerKey,
                                     StoryId storyId,
                                     StoryProgressState& state)
{
    if (!db.Prepare("SELECT ChapterId,NodeId,State,EnterApplied,CompletionApplied FROM StoryProgress WHERE PlayerKey=? AND StoryId=?"))
        return StoryProgressLoadResult::Failed;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    if (!db.BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db.BindInputParameter(&storyId, 2, PARAMTYPE_Integer))
        return StoryProgressLoadResult::Failed;

    if (!db.Execute())
    {
        const char* lastError = db.GetLastError();
        return lastError && lastError[0] != 0 ? StoryProgressLoadResult::Failed
                                               : StoryProgressLoadResult::NotFound;
    }

    int status = 0;
    int enterApplied = 0;
    int completionApplied = 0;
    state.storyId = storyId;
    const bool read =
        db.GetData(&state.chapterId, sizeof(state.chapterId), 1, PARAMTYPE_Integer) &&
        db.GetData(&state.nodeId, sizeof(state.nodeId), 2, PARAMTYPE_Integer) &&
        db.GetData(&status, sizeof(status), 3, PARAMTYPE_Integer) &&
        db.GetData(&enterApplied, sizeof(enterApplied), 4, PARAMTYPE_Integer) &&
        db.GetData(&completionApplied, sizeof(completionApplied), 5, PARAMTYPE_Integer);
    if (!read || state.chapterId <= 0 || state.nodeId <= 0 ||
        status < static_cast<int>(StoryNodeStatus::Locked) ||
        status > static_cast<int>(StoryNodeStatus::Completed) ||
        (enterApplied != 0 && enterApplied != 1) ||
        (completionApplied != 0 && completionApplied != 1))
        return StoryProgressLoadResult::Failed;

    state.status = static_cast<StoryNodeStatus>(status);
    state.enterActionsApplied = enterApplied != 0;
    state.completionActionsApplied = completionApplied != 0;
    return StoryProgressLoadResult::Found;
}

bool BindAndSaveProgress(SQLConnection& db,
                         const std::string& playerKey,
                         const StoryProgressState& state)
{
    if (!db.Prepare(
            "INSERT INTO StoryProgress(PlayerKey,StoryId,ChapterId,NodeId,State,EnterApplied,CompletionApplied,UpdatedAt) "
            "VALUES(?,?,?,?,?,?,?,datetime('now','localtime')) ON CONFLICT(PlayerKey,StoryId) DO UPDATE SET "
            "ChapterId=excluded.ChapterId,NodeId=excluded.NodeId,State=excluded.State,"
            "EnterApplied=excluded.EnterApplied,CompletionApplied=excluded.CompletionApplied,UpdatedAt=datetime('now','localtime')"))
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    int status = static_cast<int>(state.status);
    int enterApplied = state.enterActionsApplied ? 1 : 0;
    int completionApplied = state.completionActionsApplied ? 1 : 0;
    int storyId = state.storyId;
    int chapterId = state.chapterId;
    int nodeId = state.nodeId;
    return db.BindInputParameter(player, 1, PARAMTYPE_String) &&
           db.BindInputParameter(&storyId, 2, PARAMTYPE_Integer) &&
           db.BindInputParameter(&chapterId, 3, PARAMTYPE_Integer) &&
           db.BindInputParameter(&nodeId, 4, PARAMTYPE_Integer) &&
           db.BindInputParameter(&status, 5, PARAMTYPE_Integer) &&
           db.BindInputParameter(&enterApplied, 6, PARAMTYPE_Integer) &&
           db.BindInputParameter(&completionApplied, 7, PARAMTYPE_Integer) &&
           db.Execute(false);
}

bool BindAndSaveNodeState(SQLConnection& db,
                          const std::string& playerKey,
                          StoryId storyId,
                          StoryNodeId nodeId,
                          StoryNodeStatus state)
{
    if (!db.Prepare(
            "INSERT INTO StoryNodeState(PlayerKey,StoryId,NodeId,State,UpdatedAt) "
            "VALUES(?,?,?,?,datetime('now','localtime')) ON CONFLICT(PlayerKey,StoryId,NodeId) DO UPDATE SET "
            "State=excluded.State,UpdatedAt=datetime('now','localtime')"))
        return false;

    char player[256];
    CopyKey(player, sizeof(player), playerKey);
    int rawState = static_cast<int>(state);
    return db.BindInputParameter(player, 1, PARAMTYPE_String) &&
           db.BindInputParameter(&storyId, 2, PARAMTYPE_Integer) &&
           db.BindInputParameter(&nodeId, 3, PARAMTYPE_Integer) &&
           db.BindInputParameter(&rawState, 4, PARAMTYPE_Integer) &&
           db.Execute(false);
}
}

bool SQLiteStoryRepository::EnsureSchema()
{
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    const char* statements[] = {
        "CREATE TABLE IF NOT EXISTS ServerDB__StoryProgress (PlayerKey TEXT NOT NULL, StoryId INTEGER NOT NULL, "
        "ChapterId INTEGER NOT NULL, NodeId INTEGER NOT NULL, State INTEGER NOT NULL CHECK (State IN (0,1,2)), "
        "EnterApplied INTEGER NOT NULL CHECK (EnterApplied IN (0,1)), "
        "CompletionApplied INTEGER NOT NULL CHECK (CompletionApplied IN (0,1)), "
        "UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')), PRIMARY KEY(PlayerKey, StoryId))",
        "CREATE TABLE IF NOT EXISTS ServerDB__StoryPlayerFlag (PlayerKey TEXT NOT NULL, FlagKey TEXT NOT NULL, "
        "IntValue INTEGER NOT NULL, PRIMARY KEY(PlayerKey, FlagKey))",
        "CREATE TABLE IF NOT EXISTS ServerDB__StoryWorldFlag (FlagKey TEXT NOT NULL PRIMARY KEY, IntValue INTEGER NOT NULL)",
        "CREATE TABLE IF NOT EXISTS ServerDB__StoryEffectReceipt (PlayerKey TEXT NOT NULL, StoryId INTEGER NOT NULL, "
        "NodeId INTEGER NOT NULL, EffectKey TEXT NOT NULL, Applied INTEGER NOT NULL DEFAULT 0 CHECK (Applied IN (0,1)), "
        "UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')), "
        "PRIMARY KEY(PlayerKey, StoryId, NodeId, EffectKey))",
        "CREATE TABLE IF NOT EXISTS ServerDB__StoryNodeState (PlayerKey TEXT NOT NULL, StoryId INTEGER NOT NULL, "
        "NodeId INTEGER NOT NULL, State INTEGER NOT NULL CHECK (State IN (0,1,2)), "
        "UpdatedAt TEXT NOT NULL DEFAULT (datetime('now','localtime')), "
        "PRIMARY KEY(PlayerKey, StoryId, NodeId))"
    };
    bool ok = true;
    for (int i = 0; i < 5 && ok; ++i)
        ok = db->Prepare(statements[i]) && db->Execute(false);
    db->Close();
    return ok;
}

bool SQLiteStoryRepository::LoadProgress(const std::string& playerKey,
                                         StoryId storyId,
                                         StoryProgressState& state)
{
    return LoadProgressResult(playerKey, storyId, state) == StoryProgressLoadResult::Found;
}

StoryProgressLoadResult SQLiteStoryRepository::LoadProgressResult(
    const std::string& playerKey,
    StoryId storyId,
    StoryProgressState& state)
{
    if (playerKey.empty() || playerKey.size() > 255u || storyId <= 0 || !EnsureSchema())
        return StoryProgressLoadResult::Failed;
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return StoryProgressLoadResult::Failed;
    const StoryProgressLoadResult result = ReadProgress(*db, playerKey, storyId, state);
    db->Close();
    return result;
}

bool SQLiteStoryRepository::SaveProgress(const std::string& playerKey,
                                         const StoryProgressState& state)
{
    if (playerKey.empty() || playerKey.size() > 255u || state.storyId <= 0 ||
        state.chapterId <= 0 || state.nodeId <= 0 || !EnsureSchema())
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    const bool ok = BindAndSaveProgress(*db, playerKey, state);
    db->Close();
    return ok;
}

StoryMutationResult SQLiteStoryRepository::CreateInitialProgressIfAbsent(
    const std::string& playerKey,
    const StoryProgressState& state)
{
    if (playerKey.empty() || playerKey.size() > 255u || state.storyId <= 0 ||
        state.chapterId <= 0 || state.nodeId <= 0 || state.status != StoryNodeStatus::Active ||
        !EnsureSchema())
        return StoryMutationResult::Failed;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db) || !db->BeginTransaction())
        return StoryMutationResult::Failed;

    StoryProgressState existing;
    const StoryProgressLoadResult loaded = ReadProgress(*db, playerKey, state.storyId, existing);
    if (loaded == StoryProgressLoadResult::Found)
    {
        const bool committed = db->CommitTransaction();
        db->Close();
        return committed ? StoryMutationResult::AlreadyExists : StoryMutationResult::Failed;
    }
    if (loaded == StoryProgressLoadResult::Failed ||
        !BindAndSaveNodeState(*db, playerKey, state.storyId, state.nodeId, StoryNodeStatus::Active) ||
        !BindAndSaveProgress(*db, playerKey, state) ||
        !db->CommitTransaction())
    {
        db->RollbackTransaction();
        db->Close();
        return StoryMutationResult::Failed;
    }
    db->Close();
    return StoryMutationResult::Applied;
}

StoryMutationResult SQLiteStoryRepository::SaveProgressIfUnchanged(
    const std::string& playerKey,
    const StoryProgressState& expected,
    const StoryProgressState& state)
{
    if (playerKey.empty() || playerKey.size() > 255u || expected.storyId <= 0 ||
        expected.storyId != state.storyId || !EnsureSchema())
        return StoryMutationResult::Failed;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db) || !db->BeginTransaction())
        return StoryMutationResult::Failed;

    StoryProgressState current;
    const StoryProgressLoadResult loaded = ReadProgress(*db, playerKey, expected.storyId, current);
    if (loaded != StoryProgressLoadResult::Found)
    {
        db->RollbackTransaction();
        db->Close();
        return loaded == StoryProgressLoadResult::NotFound ? StoryMutationResult::Conflict
                                                           : StoryMutationResult::Failed;
    }
    if (!SameProgress(current, expected))
    {
        db->RollbackTransaction();
        db->Close();
        return StoryMutationResult::Conflict;
    }
    if (!BindAndSaveProgress(*db, playerKey, state) || !db->CommitTransaction())
    {
        db->RollbackTransaction();
        db->Close();
        return StoryMutationResult::Failed;
    }
    db->Close();
    return StoryMutationResult::Applied;
}

StoryMutationResult SQLiteStoryRepository::TransitionIfUnchanged(
    const std::string& playerKey,
    const StoryProgressState& expected,
    const StoryProgressState& nextState)
{
    if (playerKey.empty() || playerKey.size() > 255u || expected.storyId <= 0 ||
        expected.storyId != nextState.storyId || expected.nodeId <= 0 || nextState.nodeId <= 0 ||
        nextState.status != StoryNodeStatus::Active || !EnsureSchema())
        return StoryMutationResult::Failed;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db) || !db->BeginTransaction())
        return StoryMutationResult::Failed;

    StoryProgressState current;
    const StoryProgressLoadResult loaded = ReadProgress(*db, playerKey, expected.storyId, current);
    if (loaded != StoryProgressLoadResult::Found)
    {
        db->RollbackTransaction();
        db->Close();
        return loaded == StoryProgressLoadResult::NotFound ? StoryMutationResult::Conflict
                                                           : StoryMutationResult::Failed;
    }
    if (!SameProgress(current, expected))
    {
        db->RollbackTransaction();
        db->Close();
        return StoryMutationResult::Conflict;
    }
    if (!BindAndSaveNodeState(*db, playerKey, expected.storyId, expected.nodeId, StoryNodeStatus::Completed) ||
        !BindAndSaveNodeState(*db, playerKey, nextState.storyId, nextState.nodeId, StoryNodeStatus::Active) ||
        !BindAndSaveProgress(*db, playerKey, nextState) ||
        !db->CommitTransaction())
    {
        db->RollbackTransaction();
        db->Close();
        return StoryMutationResult::Failed;
    }
    db->Close();
    return StoryMutationResult::Applied;
}

StoryMutationResult SQLiteStoryRepository::CompleteIfUnchanged(
    const std::string& playerKey,
    const StoryProgressState& expected,
    const StoryProgressState& completedState)
{
    if (playerKey.empty() || playerKey.size() > 255u || expected.storyId <= 0 ||
        expected.storyId != completedState.storyId || expected.nodeId <= 0 ||
        expected.nodeId != completedState.nodeId || completedState.status != StoryNodeStatus::Completed ||
        !EnsureSchema())
        return StoryMutationResult::Failed;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db) || !db->BeginTransaction())
        return StoryMutationResult::Failed;

    StoryProgressState current;
    const StoryProgressLoadResult loaded = ReadProgress(*db, playerKey, expected.storyId, current);
    if (loaded != StoryProgressLoadResult::Found)
    {
        db->RollbackTransaction();
        db->Close();
        return loaded == StoryProgressLoadResult::NotFound ? StoryMutationResult::Conflict
                                                           : StoryMutationResult::Failed;
    }
    if (!SameProgress(current, expected))
    {
        db->RollbackTransaction();
        db->Close();
        return StoryMutationResult::Conflict;
    }
    if (!BindAndSaveNodeState(*db, playerKey, expected.storyId, expected.nodeId, StoryNodeStatus::Completed) ||
        !BindAndSaveProgress(*db, playerKey, completedState) ||
        !db->CommitTransaction())
    {
        db->RollbackTransaction();
        db->Close();
        return StoryMutationResult::Failed;
    }
    db->Close();
    return StoryMutationResult::Applied;
}

bool SQLiteStoryRepository::GetNodeState(const std::string& playerKey,
                                         StoryId storyId,
                                         StoryNodeId nodeId,
                                         StoryNodeStatus& state)
{
    if (playerKey.empty() || playerKey.size() > 255u || storyId <= 0 || nodeId <= 0 || !EnsureSchema())
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    bool found = false;
    if (db->Prepare("SELECT State FROM StoryNodeState WHERE PlayerKey=? AND StoryId=? AND NodeId=?"))
    {
        char player[256];
        CopyKey(player, sizeof(player), playerKey);
        int rawState = 0;
        found = db->BindInputParameter(player, 1, PARAMTYPE_String) &&
                db->BindInputParameter(&storyId, 2, PARAMTYPE_Integer) &&
                db->BindInputParameter(&nodeId, 3, PARAMTYPE_Integer) &&
                db->Execute() &&
                db->GetData(&rawState, sizeof(rawState), 1, PARAMTYPE_Integer);
        if (found)
        {
            if (rawState < static_cast<int>(StoryNodeStatus::Locked) ||
                rawState > static_cast<int>(StoryNodeStatus::Completed))
                found = false;
            else
                state = static_cast<StoryNodeStatus>(rawState);
        }
    }
    db->Close();
    return found;
}

bool SQLiteStoryRepository::SetNodeState(const std::string& playerKey,
                                         StoryId storyId,
                                         StoryNodeId nodeId,
                                         StoryNodeStatus state)
{
    if (playerKey.empty() || playerKey.size() > 255u || storyId <= 0 || nodeId <= 0 || !EnsureSchema())
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    const bool ok = BindAndSaveNodeState(*db, playerKey, storyId, nodeId, state);
    db->Close();
    return ok;
}

bool SQLiteStoryRepository::GetPlayerFlag(const std::string& playerKey,
                                          const std::string& flagKey,
                                          int& value)
{
    if (playerKey.empty() || playerKey.size() > 255u || flagKey.empty() ||
        flagKey.size() > 255u || !EnsureSchema())
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    bool found = false;
    if (db->Prepare("SELECT IntValue FROM StoryPlayerFlag WHERE PlayerKey=? AND FlagKey=?"))
    {
        char player[256]; char flag[256];
        CopyKey(player, sizeof(player), playerKey); CopyKey(flag, sizeof(flag), flagKey);
        found = db->BindInputParameter(player, 1, PARAMTYPE_String) &&
                db->BindInputParameter(flag, 2, PARAMTYPE_String) && db->Execute() &&
                db->GetData(&value, sizeof(value), 1, PARAMTYPE_Integer);
    }
    db->Close();
    return found;
}

bool SQLiteStoryRepository::SetPlayerFlag(const std::string& playerKey,
                                          const std::string& flagKey,
                                          int value)
{
    if (playerKey.empty() || playerKey.size() > 255u || flagKey.empty() ||
        flagKey.size() > 255u || !EnsureSchema())
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    bool ok = false;
    if (db->Prepare("INSERT INTO StoryPlayerFlag(PlayerKey,FlagKey,IntValue) VALUES(?,?,?) "
                    "ON CONFLICT(PlayerKey,FlagKey) DO UPDATE SET IntValue=excluded.IntValue"))
    {
        char player[256]; char flag[256];
        CopyKey(player, sizeof(player), playerKey); CopyKey(flag, sizeof(flag), flagKey);
        ok = db->BindInputParameter(player, 1, PARAMTYPE_String) &&
             db->BindInputParameter(flag, 2, PARAMTYPE_String) &&
             db->BindInputParameter(&value, 3, PARAMTYPE_Integer) && db->Execute(false);
    }
    db->Close();
    return ok;
}

bool SQLiteStoryRepository::GetWorldFlag(const std::string& flagKey, int& value)
{
    if (flagKey.empty() || flagKey.size() > 255u || !EnsureSchema())
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    bool found = false;
    if (db->Prepare("SELECT IntValue FROM StoryWorldFlag WHERE FlagKey=?"))
    {
        char flag[256]; CopyKey(flag, sizeof(flag), flagKey);
        found = db->BindInputParameter(flag, 1, PARAMTYPE_String) && db->Execute() &&
                db->GetData(&value, sizeof(value), 1, PARAMTYPE_Integer);
    }
    db->Close();
    return found;
}

bool SQLiteStoryRepository::SetWorldFlag(const std::string& flagKey, int value)
{
    if (flagKey.empty() || flagKey.size() > 255u || !EnsureSchema())
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;
    bool ok = false;
    if (db->Prepare("INSERT INTO StoryWorldFlag(FlagKey,IntValue) VALUES(?,?) "
                    "ON CONFLICT(FlagKey) DO UPDATE SET IntValue=excluded.IntValue"))
    {
        char flag[256]; CopyKey(flag, sizeof(flag), flagKey);
        ok = db->BindInputParameter(flag, 1, PARAMTYPE_String) &&
             db->BindInputParameter(&value, 2, PARAMTYPE_Integer) && db->Execute(false);
    }
    db->Close();
    return ok;
}

StoryEffectClaimResult SQLiteStoryRepository::ClaimEffect(const std::string& playerKey,
                                                           StoryId storyId,
                                                           StoryNodeId nodeId,
                                                           const std::string& effectKey)
{
    if (playerKey.empty() || playerKey.size() > 255u || storyId <= 0 || nodeId <= 0 ||
        effectKey.empty() || effectKey.size() > 255u || !EnsureSchema())
        return StoryEffectClaimResult::Failed;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db) || !db->BeginTransaction())
        return StoryEffectClaimResult::Failed;

    char player[256];
    char effect[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(effect, sizeof(effect), effectKey);

    bool existing = false;
    int applied = 0;
    if (!db->Prepare("SELECT Applied FROM StoryEffectReceipt WHERE PlayerKey=? AND StoryId=? AND NodeId=? AND EffectKey=?") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(&storyId, 2, PARAMTYPE_Integer) ||
        !db->BindInputParameter(&nodeId, 3, PARAMTYPE_Integer) ||
        !db->BindInputParameter(effect, 4, PARAMTYPE_String))
    {
        db->RollbackTransaction();
        db->Close();
        return StoryEffectClaimResult::Failed;
    }
    if (db->Execute())
    {
        if (!db->GetData(&applied, sizeof(applied), 1, PARAMTYPE_Integer))
        {
            db->RollbackTransaction();
            db->Close();
            return StoryEffectClaimResult::Failed;
        }
        existing = true;
    }
    else if (db->GetLastError() && db->GetLastError()[0] != 0)
    {
        db->RollbackTransaction();
        db->Close();
        return StoryEffectClaimResult::Failed;
    }

    if (existing)
    {
        const bool committed = db->CommitTransaction();
        db->Close();
        if (!committed)
            return StoryEffectClaimResult::Failed;
        return applied != 0 ? StoryEffectClaimResult::AlreadyApplied : StoryEffectClaimResult::Pending;
    }

    bool inserted = false;
    if (db->Prepare(
            "INSERT INTO StoryEffectReceipt(PlayerKey,StoryId,NodeId,EffectKey,Applied,UpdatedAt) "
            "VALUES(?,?,?,?,0,datetime('now','localtime'))"))
    {
        inserted = db->BindInputParameter(player, 1, PARAMTYPE_String) &&
                   db->BindInputParameter(&storyId, 2, PARAMTYPE_Integer) &&
                   db->BindInputParameter(&nodeId, 3, PARAMTYPE_Integer) &&
                   db->BindInputParameter(effect, 4, PARAMTYPE_String) && db->Execute(false);
    }
    if (!inserted || !db->CommitTransaction())
    {
        db->RollbackTransaction();
        db->Close();
        return StoryEffectClaimResult::Failed;
    }
    db->Close();
    return StoryEffectClaimResult::Claimed;
}

bool SQLiteStoryRepository::MarkEffectApplied(const std::string& playerKey,
                                               StoryId storyId,
                                               StoryNodeId nodeId,
                                               const std::string& effectKey)
{
    if (playerKey.empty() || playerKey.size() > 255u || storyId <= 0 || nodeId <= 0 ||
        effectKey.empty() || effectKey.size() > 255u || !EnsureSchema())
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return false;

    bool ok = false;
    if (db->Prepare(
            "UPDATE StoryEffectReceipt SET Applied=1,UpdatedAt=datetime('now','localtime') "
            "WHERE PlayerKey=? AND StoryId=? AND NodeId=? AND EffectKey=?"))
    {
        char player[256];
        char effect[256];
        CopyKey(player, sizeof(player), playerKey);
        CopyKey(effect, sizeof(effect), effectKey);
        ok = db->BindInputParameter(player, 1, PARAMTYPE_String) &&
             db->BindInputParameter(&storyId, 2, PARAMTYPE_Integer) &&
             db->BindInputParameter(&nodeId, 3, PARAMTYPE_Integer) &&
             db->BindInputParameter(effect, 4, PARAMTYPE_String) && db->Execute(false);
    }
    db->Close();
    return ok;
}


StoryEffectReceiptLoadResult SQLiteStoryRepository::GetEffectReceiptStatus(
    const std::string& playerKey,
    StoryId storyId,
    StoryNodeId nodeId,
    const std::string& effectKey,
    StoryEffectReceiptStatus& status)
{
    if (playerKey.empty() || playerKey.size() > 255u || storyId <= 0 || nodeId <= 0 ||
        effectKey.empty() || effectKey.size() > 255u || !EnsureSchema())
        return StoryEffectReceiptLoadResult::Failed;

    std::shared_ptr<SQLConnection> db;
    if (!OpenServerDatabase(db))
        return StoryEffectReceiptLoadResult::Failed;

    char player[256];
    char effect[256];
    CopyKey(player, sizeof(player), playerKey);
    CopyKey(effect, sizeof(effect), effectKey);
    if (!db->Prepare("SELECT Applied FROM StoryEffectReceipt WHERE PlayerKey=? AND StoryId=? AND NodeId=? AND EffectKey=?") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->BindInputParameter(&storyId, 2, PARAMTYPE_Integer) ||
        !db->BindInputParameter(&nodeId, 3, PARAMTYPE_Integer) ||
        !db->BindInputParameter(effect, 4, PARAMTYPE_String))
    {
        db->Close();
        return StoryEffectReceiptLoadResult::Failed;
    }

    if (!db->Execute())
    {
        const char* error = db->GetLastError();
        const bool failed = error && error[0] != 0;
        db->Close();
        return failed ? StoryEffectReceiptLoadResult::Failed
                      : StoryEffectReceiptLoadResult::NotFound;
    }

    int applied = 0;
    const bool ok = db->GetData(&applied, sizeof(applied), 1, PARAMTYPE_Integer);
    db->Close();
    if (!ok || (applied != 0 && applied != 1))
        return StoryEffectReceiptLoadResult::Failed;
    status = applied != 0 ? StoryEffectReceiptStatus::Applied : StoryEffectReceiptStatus::Pending;
    return StoryEffectReceiptLoadResult::Found;
}

bool SQLiteStoryRepository::ListPendingEffects(
    const std::string& playerKey,
    std::vector<StoryEffectReceiptEntry>& receipts)
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
            "SELECT StoryId,NodeId,EffectKey,Applied FROM StoryEffectReceipt "
            "WHERE PlayerKey=? AND Applied=0 ORDER BY StoryId,NodeId,EffectKey") ||
        !db->BindInputParameter(player, 1, PARAMTYPE_String) ||
        !db->Execute(false))
    {
        db->Close();
        return false;
    }

    while (db->NextRow())
    {
        StoryEffectReceiptEntry entry;
        char effect[256] = {0};
        int applied = -1;
        if (!db->GetData(&entry.storyId, sizeof(entry.storyId), 1, PARAMTYPE_Integer) ||
            !db->GetData(&entry.nodeId, sizeof(entry.nodeId), 2, PARAMTYPE_Integer) ||
            !db->GetData(effect, sizeof(effect), 3, PARAMTYPE_String) ||
            !db->GetData(&applied, sizeof(applied), 4, PARAMTYPE_Integer) ||
            entry.storyId <= 0 || entry.nodeId <= 0 || effect[0] == 0 || applied != 0)
        {
            receipts.clear();
            db->Close();
            return false;
        }
        entry.effectKey = effect;
        entry.status = StoryEffectReceiptStatus::Pending;
        receipts.push_back(entry);
    }

    const char* error = db->GetLastError();
    const bool ok = !error || error[0] == 0;
    db->Close();
    if (!ok)
        receipts.clear();
    return ok;
}

bool SQLiteStoryRepository::AcknowledgePendingEffect(const std::string& playerKey,
                                                      StoryId storyId,
                                                      StoryNodeId nodeId,
                                                      const std::string& effectKey)
{
    StoryEffectReceiptStatus status = StoryEffectReceiptStatus::Pending;
    const StoryEffectReceiptLoadResult loaded =
        GetEffectReceiptStatus(playerKey, storyId, nodeId, effectKey, status);
    if (loaded != StoryEffectReceiptLoadResult::Found)
        return false;
    if (status == StoryEffectReceiptStatus::Applied)
        return true;
    return MarkEffectApplied(playerKey, storyId, nodeId, effectKey);
}

}
}
