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
    int current = 0;
    bool ok = db->Prepare("SELECT Quantity FROM GatheringPlayerInventory WHERE PlayerKey=? AND ResourceKey=?") &&
              db->BindInputParameter(player,1,PARAMTYPE_String) &&
              db->BindInputParameter(resource,2,PARAMTYPE_String) && db->Execute(false);
    if (ok && db->NextRow())
        ok = db->GetData(&current,sizeof(current),1,PARAMTYPE_Integer) != FALSE;
    if (!ok) { db->Close(); return false; }
    int next = current + amount;
    ok = db->Prepare(
        "INSERT OR REPLACE INTO GatheringPlayerInventory(PlayerKey,ResourceKey,Quantity,UpdatedAt) "
        "VALUES(?,?,?,datetime('now','localtime'))") &&
        db->BindInputParameter(player,1,PARAMTYPE_String) &&
        db->BindInputParameter(resource,2,PARAMTYPE_String) &&
        db->BindInputParameter((void*)&next,3,PARAMTYPE_Integer) && db->Execute(false);
    db->Close();
    return ok;
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
