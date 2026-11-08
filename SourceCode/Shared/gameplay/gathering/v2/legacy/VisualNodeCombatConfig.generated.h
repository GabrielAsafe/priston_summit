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
    bool physicalOnly; // SUMMIT417A_BASE_NODES
};
static const VisualNodeSpec kVisualNodes[] = {
    {910001u, "raw_ore_crystalline", summit_professions::kMining, 58, 251671, 627, 225035, "Mineiro das Cinzas", "PR101", 0x03080100u, 10000u, 30000u, false},
    {910002u, "herb_bundle", summit_professions::kForaging, 58, 251524, 627, 225037, "\xc1rvore Maldita", "BI103", 0x080B0300u, 10000u, 30000u, false},
    {910003u, "common_logs", summit_professions::kWoodcutting, 58, 251360, 627, 225033, "\xc1rvore Maldita", "TW102", 0x01074D00u, 10000u, 30000u, false},
    {910004u, "river_trout", summit_professions::kFishing, 58, 251380, 866, 226790, "Tartaruga Blindada", "BI102", 0x080B0200u, 10000u, 30000u, false},
    {920001u, "arcane_wood", summit_professions::kWoodcutting, 58, 251260, 866, 226694, "\xC1rvore Maldita", "CI120", 0x080B9300u, 10000u, 30000u, true},
    {920002u, "common_logs", summit_professions::kWoodcutting, 58, 251308, 866, 226694, "\xC1rvore Maldita", "CI113", 0x080B8C00u, 10000u, 30000u, true},
    {920003u, "dark_essence", summit_professions::kForaging, 58, 251356, 866, 226694, "Mineiro das Cinzas", "CI151", 0x080BB200u, 10000u, 30000u, true},
    {920004u, "dense_resin", summit_professions::kWoodcutting, 58, 251404, 866, 226694, "\xC1rvore Maldita", "CI117", 0x080B9000u, 10000u, 30000u, true},
    {920005u, "energetic_sap", summit_professions::kWoodcutting, 58, 251452, 866, 226694, "\xC1rvore Maldita", "CI119", 0x080B9200u, 10000u, 30000u, true},
    {920006u, "illusory_essence", summit_professions::kForaging, 58, 251500, 866, 226694, "Mineiro das Cinzas", "CI153", 0x080BB400u, 10000u, 30000u, true},
    {920007u, "mystic_resin", summit_professions::kWoodcutting, 58, 251260, 866, 226742, "\xC1rvore Maldita", "CI121", 0x080B9400u, 10000u, 30000u, true},
    {920008u, "ore_amethyst", summit_professions::kMining, 58, 251308, 866, 226742, "Mineiro das Cinzas", "CI123", 0x080B9600u, 10000u, 30000u, true},
    {920009u, "ore_citrine", summit_professions::kMining, 58, 251452, 866, 226742, "Mineiro das Cinzas", "CI128", 0x080B9B00u, 10000u, 30000u, true},
    {920010u, "ore_emerald", summit_professions::kMining, 58, 251500, 866, 226742, "Mineiro das Cinzas", "CI130", 0x080B9D00u, 10000u, 30000u, true},
    {920011u, "ore_gold", summit_professions::kMining, 58, 251260, 866, 226790, "Mineiro das Cinzas", "CI125", 0x080B9800u, 10000u, 30000u, true},
    {920012u, "ore_ruby", summit_professions::kMining, 58, 251308, 866, 226790, "Mineiro das Cinzas", "CI129", 0x080B9C00u, 10000u, 30000u, true},
    {920013u, "ore_sapphire", summit_professions::kMining, 58, 251452, 866, 226790, "Mineiro das Cinzas", "CI127", 0x080B9A00u, 10000u, 30000u, true},
    {920014u, "ore_silver", summit_professions::kMining, 58, 251500, 866, 226790, "Mineiro das Cinzas", "CI124", 0x080B9700u, 10000u, 30000u, true},
    {920015u, "ore_topaz", summit_professions::kMining, 58, 251260, 866, 226838, "Mineiro das Cinzas", "CI126", 0x080B9900u, 10000u, 30000u, true},
    {920016u, "purified_salt", summit_professions::kForaging, 58, 251308, 866, 226838, "Mineiro das Cinzas", "CI152", 0x080BB300u, 10000u, 30000u, true},
    {920017u, "raw_ore_arcane", summit_professions::kMining, 58, 251356, 866, 226838, "Mineiro das Cinzas", "CI107", 0x080B8600u, 10000u, 30000u, true},
    {920018u, "raw_ore_crystalline", summit_professions::kMining, 58, 251404, 866, 226838, "Mineiro das Cinzas", "CI101", 0x080B8000u, 10000u, 30000u, true},
    {920019u, "raw_ore_deep", summit_professions::kMining, 58, 251452, 866, 226838, "Mineiro das Cinzas", "CI105", 0x080B8400u, 10000u, 30000u, true},
    {920020u, "raw_ore_energetic", summit_professions::kMining, 58, 251500, 866, 226838, "Mineiro das Cinzas", "CI103", 0x080B8200u, 10000u, 30000u, true},
    {920021u, "raw_ore_primordial", summit_professions::kMining, 58, 251260, 866, 226886, "Mineiro das Cinzas", "CI111", 0x080B8A00u, 10000u, 30000u, true},
    {920022u, "raw_ore_runic", summit_professions::kMining, 58, 251308, 866, 226886, "Mineiro das Cinzas", "CI109", 0x080B8800u, 10000u, 30000u, true},
    {920023u, "resistant_logs", summit_professions::kWoodcutting, 58, 251356, 866, 226886, "\xC1rvore Maldita", "CI116", 0x080B8F00u, 10000u, 30000u, true},
    {920024u, "sap", summit_professions::kWoodcutting, 58, 251404, 866, 226886, "\xC1rvore Maldita", "CI114", 0x080B8D00u, 10000u, 30000u, true},
    {920025u, "tree_heart", summit_professions::kWoodcutting, 58, 251452, 866, 226886, "\xC1rvore Maldita", "CI122", 0x080B9500u, 10000u, 30000u, true},
    {920026u, "war_fragment", summit_professions::kForaging, 58, 251500, 866, 226886, "Mineiro das Cinzas", "CI155", 0x080BB600u, 10000u, 30000u, true},
};
static const std::size_t kVisualNodeCount = sizeof(kVisualNodes)/sizeof(kVisualNodes[0]);
}

namespace summit_professions {
static const char* const kVisualModelPaths[] = {
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/monplant/monplant-d.ini",
    "char/monster/monplant/monplant-d.ini",
    "char/monster/turtlecannon/turtlecannon.ini"
    "char/monster/monplant/monplant-d.ini",
    "char/monster/monplant/monplant-d.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/monplant/monplant-d.ini",
    "char/monster/monplant/monplant-d.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/monplant/monplant-d.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/undeadminer/undeadminer.ini",
    "char/monster/monplant/monplant-d.ini",
    "char/monster/monplant/monplant-d.ini",
    "char/monster/monplant/monplant-d.ini",
    "char/monster/undeadminer/undeadminer.ini",
};
}
