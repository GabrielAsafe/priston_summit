#pragma once

#include <string>

#include "gameplay/core/v2/GameplayIds.h"
#include "gameplay/world/v2/WorldPosition.h"

namespace gameplay
{
namespace world
{
enum class SpawnStatus
{
    Spawned = 0,
    InvalidRequest,
    OwnerUnavailable,
    NotSpawned
};

struct SpawnRequest
{
    std::string templateName;
    WorldPosition position;
    PlayerId owner;

    SpawnRequest() : templateName(), position(), owner() {}
    SpawnRequest(const std::string& name, const WorldPosition& at, PlayerId ownerValue = PlayerId())
        : templateName(name), position(at), owner(ownerValue) {}

    bool Valid() const { return !templateName.empty() && position.map.Valid(); }
};

struct SpawnResult
{
    SpawnStatus status;
    EntityId entity;

    SpawnResult(SpawnStatus value = SpawnStatus::InvalidRequest, EntityId entityValue = EntityId())
        : status(value), entity(entityValue) {}

    bool Succeeded() const { return status == SpawnStatus::Spawned && entity.Valid(); }
};

class ISpawnService
{
public:
    virtual ~ISpawnService() {}
    virtual SpawnResult Spawn(const SpawnRequest& request) = 0;
};
}
}
