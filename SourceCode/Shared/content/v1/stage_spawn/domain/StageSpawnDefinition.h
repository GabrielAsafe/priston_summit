#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace summit
{
namespace content
{
namespace stage_spawn
{

// Authoring/runtime-neutral representation of one legacy SPM document.
// `legacyNameBytes` deliberately keeps the historical byte identity while
// monsters are still resolved through smCHAR_INFO::szName. Once monsters have
// stable content IDs this compatibility field can be retired without changing
// the source interface.
struct StageSpawnActorDefinition
{
    std::string monsterId;
    std::string legacyNameBytes;
    std::int32_t weight = 0;
};

struct StageSpawnBossDefinition
{
    std::string masterMonsterId;
    std::string slaveMonsterId;
    std::string masterLegacyNameBytes;
    std::string slaveLegacyNameBytes;
    std::int32_t slaveCount = 0;
    std::vector<std::uint32_t> openHours;
};

struct StageSpawnDefinition
{
    std::string id;
    std::string logicalPath;
    std::int32_t limitMax = 0;
    std::int32_t openInterval = 0x7F;
    std::uint32_t intervalMilliseconds = 0;
    std::int32_t openLimit = 3;
    std::vector<StageSpawnActorDefinition> actors;
    std::vector<StageSpawnBossDefinition> bosses;
};

} // namespace stage_spawn
} // namespace content
} // namespace summit
