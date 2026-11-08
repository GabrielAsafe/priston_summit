#pragma once

#include <cstdint>
#include <string>

#include "gameplay/professions/v2/ProfessionTypes.h"

namespace gameplay
{
namespace gathering
{
enum class GatheringKind
{
    Mining,
    Fishing,
    Woodcutting,
    Foraging,
    Custom
};

struct ResourceDefinition
{
    std::uint32_t resourceId;
    std::string key;
    GatheringKind kind;
    professions::ProfessionType requiredProfession;
    int requiredLevel;
    int minimumYield;
    int maximumYield;

    ResourceDefinition()
        : resourceId(0), kind(GatheringKind::Custom),
          requiredProfession(professions::ProfessionType::None),
          requiredLevel(0), minimumYield(1), maximumYield(1) {}
};

struct GatheringNodeDefinition
{
    std::uint32_t nodeId;
    std::uint32_t resourceId;
    int area;
    int x;
    int y;
    int z;
    int interactionRadius;
    std::uint64_t respawnMilliseconds;

    GatheringNodeDefinition()
        : nodeId(0), resourceId(0), area(-1), x(0), y(0), z(0),
          interactionRadius(0), respawnMilliseconds(0) {}
};

struct HarvestRequest
{
    std::string playerName;
    professions::ProfessionType profession;
    int professionLevel;
    std::uint32_t nodeId;
    int area;
    int x;
    int y;
    int z;
    std::uint64_t nowMilliseconds;
    std::uint32_t roll;

    HarvestRequest()
        : profession(professions::ProfessionType::None), professionLevel(0),
          nodeId(0), area(-1), x(0), y(0), z(0), nowMilliseconds(0), roll(0) {}
};

enum class HarvestStatus
{
    Success,
    InvalidPlayer,
    UnknownNode,
    UnknownResource,
    WrongArea,
    TooFar,
    WrongProfession,
    ProfessionLevelTooLow,
    Depleted,
    PersistenceFailed
};

struct HarvestResult
{
    HarvestStatus status;
    std::uint32_t nodeId;
    std::uint32_t resourceId;
    std::string resourceKey;
    int amount;
    int area;
    std::uint64_t nextAvailableAt;

    HarvestResult()
        : status(HarvestStatus::UnknownNode), nodeId(0), resourceId(0),
          amount(0), area(-1), nextAvailableAt(0) {}

    bool Succeeded() const { return status == HarvestStatus::Success; }
};
}
}
