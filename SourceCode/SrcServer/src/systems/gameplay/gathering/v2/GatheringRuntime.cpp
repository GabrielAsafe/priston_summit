#include "systems/gameplay/gathering/v2/GatheringRuntime.h"

#include <algorithm>
#include <cstdint>
#include <limits>

#include "gameplay/covenants/v2/CovenantCatalog.h"
#include "systems/gameplay/mining/v2/MiningRuntime.h"
#include "systems/gameplay/professions/v2/ProfessionRuntime.h"
#include "systems/gameplay/gathering/v2/legacy/LegacyHarvestPlayerQueryAdapter.h"
#include "systems/gameplay/progression/v2/ProgressionRuntime.h"

namespace gameplay
{
namespace gathering
{
namespace
{
bool& Initialized()
{
    static bool value = false;
    return value;
}

professions::ProfessionType ProfessionFor(const ResourceDefinition& resource)
{
    if (resource.requiredProfession != professions::ProfessionType::None)
        return resource.requiredProfession;
    if (resource.kind == GatheringKind::Woodcutting)
        return professions::ProfessionType::Lumberjack;
    if (resource.kind == GatheringKind::Fishing)
        return professions::ProfessionType::Fisher;
    if (resource.kind == GatheringKind::Mining)
        return professions::ProfessionType::Miner;
    return professions::ProfessionType::None;
}

std::uint64_t DistanceSquared(int x, int y, int z, const GatheringNodeDefinition& node)
{
    const std::int64_t dx = static_cast<std::int64_t>(x) - node.x;
    const std::int64_t dy = static_cast<std::int64_t>(y) - node.y;
    const std::int64_t dz = static_cast<std::int64_t>(z) - node.z;
    return static_cast<std::uint64_t>(dx*dx + dy*dy + dz*dz);
}

void ApplyCovenantGathering(const std::string& playerKey,
                            std::uint32_t roll,
                            HarvestResult& result)
{
    if (!result.Succeeded()) return;
    const int baseAmount = result.amount;
    const double adjusted = progression::ApplyServerCovenantModifierValue(
        playerKey, "gathering.yield", "gathering", static_cast<double>(result.amount));
    result.amount = adjusted >= 1.0 ? static_cast<int>(adjusted) : 1;

    covenants::CovenantModifierAggregate extra;
    if (progression::GetServerCovenantModifierAggregate(
            playerKey, "gathering.extra_yield_chance", "gathering", extra) && extra.matched)
    {
        double chance = extra.flat + extra.additivePercent;
        if (extra.multiplicativeFactor != 1.0) chance *= extra.multiplicativeFactor;
        chance = std::max(0.0, std::min(1.0, chance));
        const std::uint32_t r = roll * 1103515245u + 12345u;
        if ((r % 10000u) < static_cast<std::uint32_t>(chance * 10000.0))
            result.amount += baseAmount;
    }

    covenants::CovenantModifierAggregate special;
    if (progression::GetServerCovenantModifierAggregate(
            playerKey, "gathering.special_material_chance", "gathering", special) && special.matched)
    {
        double chance = special.flat + special.additivePercent;
        if (special.multiplicativeFactor != 1.0) chance *= special.multiplicativeFactor;
        chance = std::max(0.0, std::min(1.0, chance));
        const std::uint32_t r = roll * 1664525u + 1013904223u;
        if ((r % 10000u) < static_cast<std::uint32_t>(chance * 10000.0))
            (void)progression::AddServerCovenantInventoryItem(
                playerKey, covenants::catalog::kGatherers,
                std::string("special_material:") + result.resourceKey, 1);
    }

    covenants::CovenantModifierAggregate refined;
    if (progression::GetServerCovenantModifierAggregate(
            playerKey, "gathering.refined_material_chance", "gathering", refined) && refined.matched)
    {
        double chance = refined.flat + refined.additivePercent;
        if (refined.multiplicativeFactor != 1.0) chance *= refined.multiplicativeFactor;
        chance = std::max(0.0, std::min(1.0, chance));
        const std::uint32_t r = roll * 22695477u + 1u;
        if ((r % 10000u) < static_cast<std::uint32_t>(chance * 10000.0))
            (void)progression::AddServerCovenantInventoryItem(
                playerKey, covenants::catalog::kGatherers,
                std::string("refined_material:") + result.resourceKey, 1);
    }
    (void)progression::RecordServerCovenantActivity(playerKey, "gathering.harvest", 1);
}
}

GatheringService& ServerGatheringService()
{
    static GatheringService service;
    return service;
}

SQLiteGatheringRepository& ServerGatheringRepository()
{
    static SQLiteGatheringRepository repository;
    return repository;
}

bool InitializeServerGatheringRuntime(std::string* error)
{
    if (Initialized()) return true;
    if (!ServerGatheringRepository().Ready())
    {
        if (error) *error = "Gathering SQLite repository/content is not ready";
        return false;
    }
    std::vector<ResourceDefinition> resources;
    std::vector<GatheringNodeDefinition> nodes;
    if (!ServerGatheringRepository().LoadContent(resources, nodes))
    {
        if (error) *error = "failed to load Gathering resource/node content";
        return false;
    }

    ServerGatheringService().Clear();
    for (std::vector<ResourceDefinition>::const_iterator r=resources.begin(); r!=resources.end(); ++r)
    {
        if (r->kind == GatheringKind::Mining)
            (void)mining::ServerMiningService().RegisterResource(*r);
        else
            (void)ServerGatheringService().RegisterResource(*r);
    }
    for (std::vector<GatheringNodeDefinition>::const_iterator n=nodes.begin(); n!=nodes.end(); ++n)
    {
        ResourceDefinition resource;
        if (mining::ServerMiningService().Gathering().GetResource(n->resourceId, resource))
            (void)mining::ServerMiningService().RegisterNode(*n);
        else if (ServerGatheringService().GetResource(n->resourceId, resource))
            (void)ServerGatheringService().RegisterNode(*n);
    }
    Initialized() = true;
    return true;
}

HarvestResult HarvestGatheringNodeForPlayer(PlayerId playerId,
                                             std::uint32_t nodeId,
                                             std::uint64_t nowMilliseconds,
                                             std::uint32_t roll)
{
    HarvestResult invalid;
    invalid.nodeId = nodeId;
    HarvestPlayerSnapshot player;
    if (!legacy::ServerLegacyHarvestPlayerQueryAdapter().TryGet(playerId, player))
    { invalid.status = HarvestStatus::InvalidPlayer; return invalid; }
    ResourceDefinition resource;
    GatheringNodeDefinition node;
    if (!ServerGatheringService().GetNode(nodeId,node) || !ServerGatheringService().GetResource(node.resourceId,resource))
    { invalid.status = HarvestStatus::UnknownNode; return invalid; }

    HarvestRequest request;
    request.playerName = player.playerName;
    request.profession = ProfessionFor(resource);
    request.professionLevel = request.profession == professions::ProfessionType::None ? 0 :
        professions::ServerProfessionService().GetLevel(request.playerName, request.profession);
    request.nodeId = nodeId;
    request.area = player.area;
    request.x = player.x;
    request.y = player.y;
    request.z = player.z;
    request.nowMilliseconds = nowMilliseconds;
    request.roll = roll;
    HarvestResult result = ServerGatheringService().Harvest(request);
    if (result.Succeeded())
    {
        ApplyCovenantGathering(request.playerName, roll, result);
        (void)ServerGatheringRepository().AddInventory(request.playerName, result.resourceKey, result.amount);
    }
    return result;
}

bool FindNearestGatheringNode(int area, int x, int y, int z,
                              GatheringKind kind, bool exactKind,
                              GatheringNodeDefinition& outNode,
                              ResourceDefinition& outResource)
{
    std::vector<GatheringNodeDefinition> nodes;
    std::vector<ResourceDefinition> resources;
    if (kind == GatheringKind::Mining)
    {
        mining::ServerMiningService().ListNodes(nodes);
        mining::ServerMiningService().ListResources(resources);
    }
    else
    {
        ServerGatheringService().ListNodes(nodes);
        ServerGatheringService().ListResources(resources);
    }
    bool found=false;
    std::uint64_t best=std::numeric_limits<std::uint64_t>::max();
    for (std::vector<GatheringNodeDefinition>::const_iterator n=nodes.begin(); n!=nodes.end(); ++n)
    {
        if (n->area != area) continue;
        ResourceDefinition resource;
        bool has=false;
        for (std::vector<ResourceDefinition>::const_iterator r=resources.begin(); r!=resources.end(); ++r)
            if (r->resourceId == n->resourceId) { resource=*r; has=true; break; }
        if (!has || (exactKind && resource.kind != kind) || (!exactKind && resource.kind == GatheringKind::Mining)) continue;
        const std::uint64_t d=DistanceSquared(x,y,z,*n);
        if (d<best) { best=d; outNode=*n; outResource=resource; found=true; }
    }
    return found;
}

bool AddServerGatheringNode(const std::string& resourceKey,
                            int area, int x, int y, int z,
                            int interactionRadius,
                            std::uint64_t respawnMilliseconds,
                            GatheringNodeDefinition& outNode,
                            std::string* error)
{
    std::vector<ResourceDefinition> resources;
    std::vector<GatheringNodeDefinition> ignored;
    if (!ServerGatheringRepository().LoadContent(resources, ignored))
    { if(error)*error="failed to read Gathering content"; return false; }
    ResourceDefinition resource;
    bool found=false;
    for (std::vector<ResourceDefinition>::const_iterator r=resources.begin(); r!=resources.end(); ++r)
        if (r->key == resourceKey) { resource=*r; found=true; break; }
    if (!found) { if(error)*error="unknown gathering resource key"; return false; }
    GatheringNodeDefinition node;
    node.resourceId=resource.resourceId; node.area=area; node.x=x; node.y=y; node.z=z;
    node.interactionRadius=interactionRadius>0?interactionRadius:160;
    node.respawnMilliseconds=respawnMilliseconds>0?respawnMilliseconds:30000;
    if (!ServerGatheringRepository().AddNode(node)) { if(error)*error="failed to persist gathering node"; return false; }
    if (resource.kind == GatheringKind::Mining)
        (void)mining::ServerMiningService().RegisterNode(node);
    else
        (void)ServerGatheringService().RegisterNode(node);
    outNode=node;
    return true;
}
}
}
