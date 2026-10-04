#include "systems/commerce/v2/legacy/LegacyVipEntitlementRepository.h"

#include <cstdio>

#include "core/server/legacy_sin_runtime/sinLinkHeader.h"
#include "systems/persistence/database/core/Database.h"

namespace gameplay
{
namespace commerce
{
namespace legacy
{
bool LegacyVipEntitlementRepository::TryGetVip(PlayerId player, VipEntitlementSnapshot& out)
{
    rsPLAYINFO* legacyPlayer = players_.Resolve(player);
    if(!legacyPlayer || !legacyPlayer->getName() || !legacyPlayer->getName()[0])
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ServerDB);
    if(!db || !db->Open())
        return false;

    char playerName[32] = {0};
    lstrcpyn(playerName, legacyPlayer->getName(), sizeof(playerName));
    if(db->Prepare("DELETE FROM Vip WHERE PlayerName=? AND datetime('now','localtime') >= DateEnd"))
    {
        db->BindInputParameter(playerName, 1, PARAMTYPE_String);
        (void)db->Execute(false);
    }

    out = VipEntitlementSnapshot();
    out.player = player;
    if(db->Prepare("SELECT IdVip FROM Vip WHERE PlayerName=?"))
    {
        db->BindInputParameter(playerName, 1, PARAMTYPE_String);
        if(db->Execute())
        {
            db->GetData(&out.tier, sizeof(out.tier), 1, PARAMTYPE_Integer);
            out.active = true;
        }
    }
    db->Close();
    return true;
}

bool LegacyVipEntitlementRepository::GrantVip(PlayerId player, int tier, int days)
{
    rsPLAYINFO* legacyPlayer = players_.Resolve(player);
    if(!legacyPlayer || !legacyPlayer->getName() || !legacyPlayer->getName()[0])
        return false;

    std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_ServerDB);
    if(!db || !db->Open())
        return false;

    char query[255] = {0};
    std::snprintf(query, sizeof(query),
                  "INSERT INTO Vip (PlayerName,IdVip,DateStart,DateEnd) values (?, ?, datetime('now','localtime'), datetime('now','localtime','+%d day'))",
                  days);
    char playerName[32] = {0};
    lstrcpyn(playerName, legacyPlayer->getName(), sizeof(playerName));
    bool ok = false;
    if(db->Prepare(query))
    {
        db->BindInputParameter(playerName, 1, PARAMTYPE_String);
        db->BindInputParameter(&tier, 2, PARAMTYPE_Integer);
        ok = db->Execute(false) == TRUE;
    }
    db->Close();
    return ok;
}

PremiumEntitlementService& ServerPremiumEntitlementService()
{
    static LegacyVipEntitlementRepository repository(player::legacy::ServerLegacyOnlinePlayerRegistry());
    static PremiumEntitlementService service(repository);
    return service;
}
}
}
}
