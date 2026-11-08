#include "systems/gameplay/gathering/v2/SQLiteGatheringRepository.h"

#include <cstring>
#include <memory>

#include "systems/persistence/database/core/Database.h"

namespace gameplay
{
namespace gathering
{
namespace
{
bool OpenDb(std::shared_ptr<SQLConnection>& db)
{
    db = SQLConnection::GetConnection(DATABASEID_ServerDB);
    return db && db->Open();
}

void CopyText(char* dst, std::size_t size, const std::string& value)
{
    std::memset(dst, 0, size);
    if (size)
        std::strncpy(dst, value.c_str(), size - 1);
}

bool ReadInt(SQLConnection& db, int column, int& value)
{
    return db.GetData(&value, sizeof(value), column, PARAMTYPE_Integer) != FALSE;
}

bool ReadText(SQLConnection& db, int column, std::string& value)
{
    char buffer[128] = {0};
    if (!db.GetData(buffer, sizeof(buffer), column, PARAMTYPE_String))
        return false;
    value = buffer;
    return true;
}
}

bool SQLiteGatheringRepository::Ready()
{
    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;
    int resources = 0;
    const bool ok = db->Prepare("SELECT COUNT(*) FROM GatheringResourceDefinition WHERE Enabled=1") &&
                    db->Execute(false) && db->NextRow() &&
                    db->GetData(&resources, sizeof(resources), 1, PARAMTYPE_Integer);
    db->Close();
    return ok && resources > 0;
}

bool SQLiteGatheringRepository::LoadContent(std::vector<ResourceDefinition>& resources,
                                            std::vector<GatheringNodeDefinition>& nodes)
{
    resources.clear();
    nodes.clear();
    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db))
        return false;

    bool ok = db->Prepare(
        "SELECT ResourceId,ResourceKey,Kind,RequiredProfession,RequiredLevel,MinimumYield,MaximumYield "
        "FROM GatheringResourceDefinition WHERE Enabled=1 ORDER BY ResourceId") && db->Execute(false);
    while (ok && db->NextRow())
    {
        ResourceDefinition r;
        int resourceId = 0;
        int kind = 0;
        int profession = 0;
        if (!ReadInt(*db,1,resourceId) || !ReadText(*db,2,r.key) ||
            !ReadInt(*db,3,kind) || !ReadInt(*db,4,profession) ||
            !ReadInt(*db,5,r.requiredLevel) || !ReadInt(*db,6,r.minimumYield) ||
            !ReadInt(*db,7,r.maximumYield))
        { ok = false; break; }
        r.resourceId = static_cast<std::uint32_t>(resourceId);
        r.kind = static_cast<GatheringKind>(kind);
        r.requiredProfession = static_cast<professions::ProfessionType>(profession);
        resources.push_back(r);
    }
    if (ok)
        ok = db->Prepare(
            "SELECT NodeId,ResourceId,Area,X,Y,Z,InteractionRadius,RespawnMilliseconds "
            "FROM GatheringNodeDefinition WHERE Enabled=1 ORDER BY NodeId") && db->Execute(false);
    while (ok && db->NextRow())
    {
        GatheringNodeDefinition n;
        int nodeId = 0;
        int resourceId = 0;
        int respawn = 0;
        if (!ReadInt(*db,1,nodeId) || !ReadInt(*db,2,resourceId) ||
            !ReadInt(*db,3,n.area) || !ReadInt(*db,4,n.x) || !ReadInt(*db,5,n.y) ||
            !ReadInt(*db,6,n.z) || !ReadInt(*db,7,n.interactionRadius) || !ReadInt(*db,8,respawn))
        { ok = false; break; }
        n.nodeId = static_cast<std::uint32_t>(nodeId);
        n.resourceId = static_cast<std::uint32_t>(resourceId);
        n.respawnMilliseconds = respawn > 0 ? static_cast<std::uint64_t>(respawn) : 0;
        nodes.push_back(n);
    }
    db->Close();
    return ok;
}

bool SQLiteGatheringRepository::AddInventory(const std::string& playerKey,
                                             const std::string& resourceKey,
                                             int amount)
{
    if (playerKey.empty() || resourceKey.empty() || amount <= 0)
        return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db)) return false;
    char player[256], resource[128];
    CopyText(player,sizeof(player),playerKey); CopyText(resource,sizeof(resource),resourceKey);
    // One atomic SQLite UPSERT. A SELECT+INSERT OR REPLACE could overwrite
    // another concurrently completed harvest with an obsolete quantity.
    const bool wrote = db->Prepare(
        "INSERT INTO GatheringPlayerInventory(PlayerKey,ResourceKey,Quantity,UpdatedAt) "
        "VALUES(?,?,?,datetime('now','localtime')) "
        "ON CONFLICT(PlayerKey,ResourceKey) DO UPDATE SET "
        "Quantity=Quantity+excluded.Quantity, "
        "UpdatedAt=excluded.UpdatedAt "
        "WHERE Quantity>=0 AND "
        "Quantity<=2147483647-excluded.Quantity") &&
        db->BindInputParameter(player,1,PARAMTYPE_String) &&
        db->BindInputParameter(resource,2,PARAMTYPE_String) &&
        db->BindInputParameter(&amount,3,PARAMTYPE_Integer) && db->Execute(false);
    int changed = 0;
    const bool ok = wrote && db->Prepare("SELECT changes()") &&
        db->Execute(false) && db->NextRow() &&
        ReadInt(*db,1,changed) && changed == 1;
    db->Close();
    return ok;
}


// 372A: one SQLite transaction for the main material plus optional weighted
// secondary materials. Never perform a DAT write or award XP before commit.
bool SQLiteGatheringRepository::AddHarvestBundle(const std::string& playerKey,
    const std::string& resourceKey, int amount, std::uint32_t roll,
    int defaultXP, int& experienceAward)
{
    experienceAward = 0;
    if (playerKey.empty() || playerKey.size() > 255 || resourceKey.empty() ||
        resourceKey.size() > 127 || amount <= 0 || amount > 1000000) return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db) || !db->BeginTransaction()) return false;
    struct Drop { std::string key; int amount; };
    std::vector<Drop> drops;
    drops.push_back(Drop{resourceKey,amount});
    char resource[128]={0},player[256]={0};
    CopyText(resource,sizeof(resource),resourceKey);
    CopyText(player,sizeof(player),playerKey);
    int xp=defaultXP;
    bool ok = db->Prepare("SELECT XP FROM GatheringXP378 WHERE ResourceKey=? AND Enabled=1") &&
              db->BindInputParameter(resource,1,PARAMTYPE_String) && db->Execute(false);
    if (ok && db->NextRow()) ok = ReadInt(*db,1,xp);
    if (ok) ok = db->Prepare(
        "SELECT ExtraResourceKey,ChancePer10000,MinAmount,MaxAmount "
        "FROM GatheringBonusDrop378 WHERE PrimaryResourceKey=? AND Enabled=1 ORDER BY ExtraResourceKey") &&
        db->BindInputParameter(resource,1,PARAMTYPE_String) && db->Execute(false);
    std::uint32_t selector=roll;
    while (ok && db->NextRow())
    {
        std::string key; int chance=0,minAmount=0,maxAmount=0;
        ok=ReadText(*db,1,key) && ReadInt(*db,2,chance) &&
            ReadInt(*db,3,minAmount) && ReadInt(*db,4,maxAmount);
        if (!ok || key.empty() || key.size() > 127 || chance < 0 || chance > 10000 ||
            minAmount <= 0 || maxAmount < minAmount || maxAmount > 100) { ok=false;break; }
        selector=selector*1664525u+1013904223u;
        if (selector % 10000u < static_cast<std::uint32_t>(chance))
        {
            selector=selector*1664525u+1013904223u;
            drops.push_back(Drop{key,minAmount+static_cast<int>(selector %
                static_cast<std::uint32_t>(maxAmount-minAmount+1))});
        }
        if(drops.size() > 16) {ok=false;break;}
    }
    for (std::size_t i=0;ok && i<drops.size();++i)
    {
        char key[128]={0};CopyText(key,sizeof(key),drops[i].key);
        int count=drops[i].amount;
        ok=db->Prepare("INSERT INTO GatheringPlayerInventory(PlayerKey,ResourceKey,Quantity,UpdatedAt) "
            "VALUES(?,?,?,datetime('now')) ON CONFLICT(PlayerKey,ResourceKey) DO UPDATE SET "
            "Quantity=Quantity+excluded.Quantity,UpdatedAt=excluded.UpdatedAt "
            "WHERE Quantity>=0 AND Quantity<=2147483647-excluded.Quantity") &&
           db->BindInputParameter(player,1,PARAMTYPE_String) &&
           db->BindInputParameter(key,2,PARAMTYPE_String) &&
           db->BindInputParameter(&count,3,PARAMTYPE_Integer) && db->Execute(false);
        int changed=0;
        if(ok) ok=db->Prepare("SELECT changes()") && db->Execute(false) &&
            db->NextRow() && ReadInt(*db,1,changed) && changed==1;
    }
    if (ok && xp>=0 && xp<=1000 && db->CommitTransaction())
    { db->Close(); experienceAward=xp; return true; }
    (void)db->RollbackTransaction();db->Close();return false;
}

bool SQLiteGatheringRepository::GetInventory(const std::string& playerKey,
                                             std::vector<GatheringInventoryEntry>& out)
{
    out.clear();
    if (playerKey.empty()) return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db)) return false;
    char player[256]; CopyText(player,sizeof(player),playerKey);
    bool ok = db->Prepare(
        "SELECT ResourceKey,Quantity FROM GatheringPlayerInventory WHERE PlayerKey=? AND Quantity>0 ORDER BY ResourceKey") &&
        db->BindInputParameter(player,1,PARAMTYPE_String) && db->Execute(false);
    while (ok && db->NextRow())
    {
        GatheringInventoryEntry e;
        if (!ReadText(*db,1,e.resourceKey) || !ReadInt(*db,2,e.quantity)) { ok=false; break; }
        out.push_back(e);
    }
    db->Close();
    return ok;
}

bool SQLiteGatheringRepository::AddNode(GatheringNodeDefinition& node)
{
    if (node.resourceId == 0 || node.area < 0) return false;
    std::shared_ptr<SQLConnection> db;
    if (!OpenDb(db)) return false;
    if (node.nodeId == 0)
    {
        int maxId = 0;
        if (!(db->Prepare("SELECT COALESCE(MAX(NodeId),0) FROM GatheringNodeDefinition") && db->Execute(false) &&
              db->NextRow() && db->GetData(&maxId,sizeof(maxId),1,PARAMTYPE_Integer)))
        { db->Close(); return false; }
        node.nodeId = static_cast<std::uint32_t>(maxId + 1);
    }
    int nodeId=(int)node.nodeId, resourceId=(int)node.resourceId, area=node.area, x=node.x,y=node.y,z=node.z,
        radius=node.interactionRadius, respawn=(int)node.respawnMilliseconds, enabled=1;
    const bool ok = db->Prepare(
        "INSERT OR REPLACE INTO GatheringNodeDefinition"
        "(NodeId,ResourceId,Area,X,Y,Z,InteractionRadius,RespawnMilliseconds,Enabled) VALUES(?,?,?,?,?,?,?,?,?)") &&
        db->BindInputParameter(&nodeId,1,PARAMTYPE_Integer) && db->BindInputParameter(&resourceId,2,PARAMTYPE_Integer) &&
        db->BindInputParameter(&area,3,PARAMTYPE_Integer) && db->BindInputParameter(&x,4,PARAMTYPE_Integer) &&
        db->BindInputParameter(&y,5,PARAMTYPE_Integer) && db->BindInputParameter(&z,6,PARAMTYPE_Integer) &&
        db->BindInputParameter(&radius,7,PARAMTYPE_Integer) && db->BindInputParameter(&respawn,8,PARAMTYPE_Integer) &&
        db->BindInputParameter(&enabled,9,PARAMTYPE_Integer) && db->Execute(false);
    db->Close();
    return ok;
}
}
}
