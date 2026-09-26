#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace summit
{
namespace content
{
namespace map
{

enum MapProfile
{
    MapProfileUnknown = 0,
    MapProfileClient = 1,
    MapProfileServer = 2
};

enum MapFeature
{
    MapFeatureWorldCupField = 1u << 0
};

struct MapStageObjectDefinition
{
    std::string assetPath;
    std::int32_t bipAnimation = 0;
    std::uint32_t requireFeatureMask = 0;
    std::uint32_t forbidFeatureMask = 0;
};

struct MapStartPointDefinition
{
    std::int32_t x = 0;
    std::int32_t z = 0;
};

struct MapGateDefinition
{
    std::int32_t targetLegacyFieldCode = -1;
    std::int32_t x = 0;
    std::int32_t z = 0;
    std::int32_t y = 0;
};

struct MapWarpGateDefinition
{
    std::int32_t x = 0;
    std::int32_t z = 0;
    std::int32_t y = 0;
    std::int32_t size = 0;
    std::int32_t height = 0;
    std::int32_t limitLevel = 0;
    std::int32_t specialEffect = 0;
    std::vector<MapGateDefinition> outGates;
};

struct MapAmbientDefinition
{
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t z = 0;
    std::int32_t radius = 0;
    std::int32_t soundCode = 0;
};

struct MapDefinition
{
    std::string id;
    std::int32_t legacyFieldCode = -1;
    std::string stagePath;
    std::string mapImageId;
    bool hasMapImageId = false;

    std::int32_t state = 0;
    std::int32_t backImageCode[3] = {0, 0, 0};
    std::int32_t backMusicCode = 0;
    std::int32_t fieldEvent = 0;
    std::int32_t serverCode = 0;
    std::int32_t limitLevel = 0;
    std::int32_t fieldSight = 0;
    std::int32_t centerX = 0;
    std::int32_t centerZ = 0;

    std::vector<MapStageObjectDefinition> stageObjects;
    std::vector<MapStartPointDefinition> startPoints;
    std::vector<MapGateDefinition> gates;
    std::vector<MapWarpGateDefinition> warpGates;
    std::vector<MapAmbientDefinition> ambient;

    // The legacy InitField() contains four LimitLevel writes that occur without
    // an active warp gate. They are preserved explicitly until their intended
    // semantics are resolved instead of being silently discarded.
    bool hasLegacyOrphanWarpLimitLevel = false;
    std::int32_t legacyOrphanWarpLimitLevel = 0;
};

struct MapCatalogDefinition
{
    MapProfile profile = MapProfileUnknown;
    std::uint32_t featureMask = 0;
    std::int32_t dynamicFieldCode = -1;
    std::uint32_t staticFieldCount = 0;
    std::vector<MapDefinition> maps;
};

} // namespace map
} // namespace content
} // namespace summit
