#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "gameplay/gathering/v2/GatheringTypes.h"

namespace gameplay
{
namespace gathering
{
struct GatheringInventoryEntry
{
    std::string resourceKey;
    int quantity;
    GatheringInventoryEntry() : quantity(0) {}
};

class SQLiteGatheringRepository
{
public:
    bool Ready();
    bool LoadContent(std::vector<ResourceDefinition>& resources,
                     std::vector<GatheringNodeDefinition>& nodes);
    bool AddInventory(const std::string& playerKey, const std::string& resourceKey, int amount);
    bool GetInventory(const std::string& playerKey, std::vector<GatheringInventoryEntry>& out);
    bool AddNode(GatheringNodeDefinition& node);
};
}
}
