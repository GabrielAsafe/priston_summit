#pragma once
// GENERATED_BY_SUMMIT_PROFESSIONS_V2_VISUAL_NODE_V1
// Rebuild using instalar_nodos3d.py --regenerate --apply after changing the canonical JSON.
#include <cstdint>
#include <cstddef>
namespace summit_professions
{
enum VisualNodeKind { kMining=0, kFishing=1, kWoodcutting=2, kForaging=3 };
struct VisualNodeSpec
{
    std::uint32_t nodeId;
    const char* resourceKey;
    VisualNodeKind kind;
    int map, x, y, z;
    const char* monsterTemplate;
    const char* itemTextCode;
    std::uint32_t legacyItemCode;
    std::uint32_t itemChancePer10000;
    std::uint32_t respawnMs;
};
static const VisualNodeSpec kVisualNodes[] = {
    {910001u, "iron_ore", summit_professions::kMining, 58, 251671, 627, 225035, "Mineiro das Cinzas", "PR101", 0x03080100u, 10000u, 30000u},
    {910002u, "herb_bundle", summit_professions::kForaging, 58, 251524, 627, 225037, "\xc1rvore Maldita", "BI103", 0x080B0300u, 10000u, 30000u},
    {910003u, "timber", summit_professions::kWoodcutting, 58, 251360, 627, 225033, "\xc1rvore Maldita", "TW102", 0x01074D00u, 10000u, 30000u},
    {910004u, "river_trout", summit_professions::kFishing, 58, 251380, 866, 226790, "Tartaruga Blindada", "BI102", 0x080B0200u, 10000u, 30000u}
};
static const std::size_t kVisualNodeCount = sizeof(kVisualNodes)/sizeof(kVisualNodes[0]);
}

namespace summit_professions {
static const char* const kVisualModelPaths[] = {
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/monplant/monplant-d.ini",
    "char/monster/monplant/monplant-d.ini",
    "char/monster/turtlecannon/turtlecannon.ini"
};
}
