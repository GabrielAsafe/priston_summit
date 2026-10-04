#pragma once

#include <cstring>
#include <memory>

#include "gameplay/commerce/v2/PremiumPurchaseReplayGuard.h"
#include "systems/persistence/database/core/Database.h"

namespace gameplay::commerce::legacy
{
class SQLitePremiumPurchaseReplayStore final : public IPremiumPurchaseReplayStore
{
public:
    PremiumPurchaseReplayResult Reserve(const PremiumPurchaseOperation& operation) override
    {
        if(operation.purchaserKey.empty() || operation.itemCode.empty())
            return PremiumPurchaseReplayResult(PremiumPurchaseReplayCode::InvalidOperation);
        if(operation.requestId == 0u)
            return PremiumPurchaseReplayResult(PremiumPurchaseReplayCode::LegacyUntracked);

        std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_CoinShopDB);
        if(!db || !db->Open() || !db->BeginTransaction())
        {
            if(db) db->Close();
            return PremiumPurchaseReplayResult(PremiumPurchaseReplayCode::InvalidOperation);
        }

        ExistingReceipt existing;
        const ReadCode read = Read(*db, operation, existing);
        if(read == ReadCode::Found)
        {
            const bool committed = db->CommitTransaction();
            db->Close();
            if(!committed)
                return PremiumPurchaseReplayResult(PremiumPurchaseReplayCode::InvalidOperation);
            if(existing.itemCode != operation.itemCode)
                return PremiumPurchaseReplayResult(PremiumPurchaseReplayCode::RequestConflict);
            return PremiumPurchaseReplayResult(existing.committed
                ? PremiumPurchaseReplayCode::DuplicateCommitted
                : PremiumPurchaseReplayCode::DuplicateInFlight);
        }
        if(read == ReadCode::Failed || !InsertReserved(*db, operation))
        {
            (void)db->RollbackTransaction();
            db->Close();
            return PremiumPurchaseReplayResult(PremiumPurchaseReplayCode::InvalidOperation);
        }

        const bool committed = db->CommitTransaction();
        db->Close();
        return PremiumPurchaseReplayResult(committed
            ? PremiumPurchaseReplayCode::Reserved
            : PremiumPurchaseReplayCode::InvalidOperation);
    }

    bool Commit(const PremiumPurchaseOperation& operation) override
    {
        if(operation.requestId == 0u)
            return true;
        std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_CoinShopDB);
        if(!db || !db->Open() || !db->BeginTransaction())
        {
            if(db) db->Close();
            return false;
        }

        ExistingReceipt existing;
        const ReadCode read = Read(*db, operation, existing);
        if(read != ReadCode::Found || existing.itemCode != operation.itemCode)
        {
            (void)db->RollbackTransaction();
            db->Close();
            return false;
        }
        if(existing.committed)
        {
            const bool ok = db->CommitTransaction();
            db->Close();
            return ok;
        }

        const bool updated = UpdateState(*db, operation, 1);
        const bool committed = updated && db->CommitTransaction();
        if(!committed)
            (void)db->RollbackTransaction();
        db->Close();
        return committed;
    }

    bool Abort(const PremiumPurchaseOperation& operation) override
    {
        if(operation.requestId == 0u)
            return true;
        std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_CoinShopDB);
        if(!db || !db->Open() || !db->BeginTransaction())
        {
            if(db) db->Close();
            return false;
        }

        ExistingReceipt existing;
        const ReadCode read = Read(*db, operation, existing);
        if(read == ReadCode::NotFound)
        {
            const bool ok = db->CommitTransaction();
            db->Close();
            return ok;
        }
        if(read != ReadCode::Found || existing.itemCode != operation.itemCode || existing.committed)
        {
            (void)db->RollbackTransaction();
            db->Close();
            return false;
        }

        const bool removed = DeleteReserved(*db, operation);
        const bool committed = removed && db->CommitTransaction();
        if(!committed)
            (void)db->RollbackTransaction();
        db->Close();
        return committed;
    }

private:
    enum class ReadCode { Found, NotFound, Failed };
    struct ExistingReceipt
    {
        std::string itemCode;
        bool committed = false;
    };

    static void CopyText(char* destination, std::size_t capacity, const std::string& value)
    {
        if(!destination || capacity == 0u)
            return;
        std::memset(destination, 0, capacity);
        std::strncpy(destination, value.c_str(), capacity - 1u);
    }

    static bool BindIdentity(SQLConnection& db,
                             const PremiumPurchaseOperation& operation,
                             int firstPosition = 1)
    {
        char purchaser[128];
        CopyText(purchaser, sizeof(purchaser), operation.purchaserKey);
        int scope = static_cast<int>(operation.scope);
        int requestId = static_cast<int>(operation.requestId);
        return db.BindInputParameter(purchaser, firstPosition, PARAMTYPE_String) &&
               db.BindInputParameter(&scope, firstPosition + 1, PARAMTYPE_Integer) &&
               db.BindInputParameter(&requestId, firstPosition + 2, PARAMTYPE_Integer);
    }

    static ReadCode Read(SQLConnection& db,
                         const PremiumPurchaseOperation& operation,
                         ExistingReceipt& receipt)
    {
        receipt = ExistingReceipt{};
        if(!db.Prepare("SELECT ItemCode,State FROM PremiumPurchaseReceipt WHERE PurchaserKey=? AND Scope=? AND RequestId=?") ||
           !BindIdentity(db, operation))
            return ReadCode::Failed;
        if(!db.Execute())
            return (db.GetLastError() && db.GetLastError()[0]) ? ReadCode::Failed : ReadCode::NotFound;

        char itemCode[64] = {0};
        int state = 0;
        if(!db.GetData(itemCode, sizeof(itemCode), 1, PARAMTYPE_String) ||
           !db.GetData(&state, sizeof(state), 2, PARAMTYPE_Integer) ||
           (state != 0 && state != 1))
            return ReadCode::Failed;
        receipt.itemCode = itemCode;
        receipt.committed = state != 0;
        return ReadCode::Found;
    }

    static bool InsertReserved(SQLConnection& db, const PremiumPurchaseOperation& operation)
    {
        if(!db.Prepare("INSERT INTO PremiumPurchaseReceipt(PurchaserKey,Scope,RequestId,ItemCode,State,CreatedAt,UpdatedAt) VALUES(?,?,?,?,0,datetime('now','localtime'),datetime('now','localtime'))"))
            return false;
        char purchaser[128];
        char itemCode[64];
        CopyText(purchaser, sizeof(purchaser), operation.purchaserKey);
        CopyText(itemCode, sizeof(itemCode), operation.itemCode);
        int scope = static_cast<int>(operation.scope);
        int requestId = static_cast<int>(operation.requestId);
        return db.BindInputParameter(purchaser, 1, PARAMTYPE_String) &&
               db.BindInputParameter(&scope, 2, PARAMTYPE_Integer) &&
               db.BindInputParameter(&requestId, 3, PARAMTYPE_Integer) &&
               db.BindInputParameter(itemCode, 4, PARAMTYPE_String) &&
               db.Execute(false);
    }

    static bool UpdateState(SQLConnection& db,
                            const PremiumPurchaseOperation& operation,
                            int state)
    {
        if(!db.Prepare("UPDATE PremiumPurchaseReceipt SET State=?,UpdatedAt=datetime('now','localtime') WHERE PurchaserKey=? AND Scope=? AND RequestId=? AND ItemCode=?"))
            return false;
        char purchaser[128];
        char itemCode[64];
        CopyText(purchaser, sizeof(purchaser), operation.purchaserKey);
        CopyText(itemCode, sizeof(itemCode), operation.itemCode);
        int scope = static_cast<int>(operation.scope);
        int requestId = static_cast<int>(operation.requestId);
        return db.BindInputParameter(&state, 1, PARAMTYPE_Integer) &&
               db.BindInputParameter(purchaser, 2, PARAMTYPE_String) &&
               db.BindInputParameter(&scope, 3, PARAMTYPE_Integer) &&
               db.BindInputParameter(&requestId, 4, PARAMTYPE_Integer) &&
               db.BindInputParameter(itemCode, 5, PARAMTYPE_String) &&
               db.Execute(false);
    }

    static bool DeleteReserved(SQLConnection& db, const PremiumPurchaseOperation& operation)
    {
        if(!db.Prepare("DELETE FROM PremiumPurchaseReceipt WHERE PurchaserKey=? AND Scope=? AND RequestId=? AND ItemCode=? AND State=0"))
            return false;
        char purchaser[128];
        char itemCode[64];
        CopyText(purchaser, sizeof(purchaser), operation.purchaserKey);
        CopyText(itemCode, sizeof(itemCode), operation.itemCode);
        int scope = static_cast<int>(operation.scope);
        int requestId = static_cast<int>(operation.requestId);
        return db.BindInputParameter(purchaser, 1, PARAMTYPE_String) &&
               db.BindInputParameter(&scope, 2, PARAMTYPE_Integer) &&
               db.BindInputParameter(&requestId, 3, PARAMTYPE_Integer) &&
               db.BindInputParameter(itemCode, 4, PARAMTYPE_String) &&
               db.Execute(false);
    }
};
}
