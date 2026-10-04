#pragma once

#include <cstdio>
#include <memory>
#include <string>

#include "Shared/gameplay/commerce/v2/RestorationTransactionService.h"
#include "systems/persistence/database/core/Database.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"

int CreateCommandItem(rsPLAYINFO* lpPlayInfo, rsPLAYINFO* lpPlayInfo2, char* szItem);

namespace gameplay::commerce::legacy
{
class LegacyRestorationClaimStore final : public IRestorationClaimStore
{
public:
    bool Consume(const RestorationRecordSnapshot& record) override
    {
        std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_SecurityDB);
        if(!db || !db->Open())
            return false;
        if(!db->BeginTransaction())
        {
            db->Close();
            return false;
        }

        char account[64] = {0};
        char character[64] = {0};
        std::snprintf(account, sizeof(account), "%s", record.account.c_str());
        std::snprintf(character, sizeof(character), "%s", record.character.c_str());
        int head = static_cast<int>(record.head);
        int checksum = static_cast<int>(record.checksum);

        const bool exists = db->Prepare(
                "SELECT 1 FROM AgingFailed WHERE PlayerID=? AND PlayerName=? AND Head=? AND ChkSum=?") &&
            db->BindInputParameter(account, 1, PARAMTYPE_String) &&
            db->BindInputParameter(character, 2, PARAMTYPE_String) &&
            db->BindInputParameter(&head, 3, PARAMTYPE_Integer) &&
            db->BindInputParameter(&checksum, 4, PARAMTYPE_Integer) &&
            db->Execute();
        if(!exists)
        {
            db->RollbackTransaction();
            db->Close();
            return false;
        }

        const bool removed = db->Prepare(
                "DELETE FROM AgingFailed WHERE PlayerID=? AND PlayerName=? AND Head=? AND ChkSum=?") &&
            db->BindInputParameter(account, 1, PARAMTYPE_String) &&
            db->BindInputParameter(character, 2, PARAMTYPE_String) &&
            db->BindInputParameter(&head, 3, PARAMTYPE_Integer) &&
            db->BindInputParameter(&checksum, 4, PARAMTYPE_Integer) &&
            db->Execute(false);
        if(!removed || !db->CommitTransaction())
        {
            db->RollbackTransaction();
            db->Close();
            return false;
        }
        db->Close();
        return true;
    }

    bool Restore(const RestorationRecordSnapshot& record) override
    {
        std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_SecurityDB);
        if(!db || !db->Open())
            return false;

        char account[64] = {0};
        char character[64] = {0};
        char itemName[128] = {0};
        std::snprintf(account, sizeof(account), "%s", record.account.c_str());
        std::snprintf(character, sizeof(character), "%s", record.character.c_str());
        std::snprintf(itemName, sizeof(itemName), "%s", record.itemName.c_str());
        int aging = record.itemAging;
        int head = static_cast<int>(record.head);
        int checksum = static_cast<int>(record.checksum);

        const bool restored = db->Prepare("INSERT INTO AgingFailed VALUES (?,?,?,?,?,?,GETDATE())") &&
            db->BindInputParameter(account, 1, PARAMTYPE_String) &&
            db->BindInputParameter(character, 2, PARAMTYPE_String) &&
            db->BindInputParameter(itemName, 3, PARAMTYPE_String) &&
            db->BindInputParameter(&aging, 4, PARAMTYPE_Integer) &&
            db->BindInputParameter(&head, 5, PARAMTYPE_Integer) &&
            db->BindInputParameter(&checksum, 6, PARAMTYPE_Integer) &&
            db->Execute(false);
        db->Close();
        return restored;
    }
};

class LegacyRestorationGrant final : public IRestorationGrant
{
public:
    explicit LegacyRestorationGrant(rsPLAYINFO* player) : player_(player) {}

    bool Grant(const RestorationRecordSnapshot& record) override
    {
        if(!player_ || !player_->lpsmSock)
            return false;
        char itemRef[64] = {0};
        std::snprintf(itemRef, sizeof(itemRef), "@%u@%u", record.head, record.checksum);
        return CreateCommandItem(player_, player_, itemRef) == TRUE;
    }

private:
    rsPLAYINFO* player_;
};
}
