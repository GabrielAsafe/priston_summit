#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "systems/ui/v2/rendering/TextureHandle.h"

namespace ui::v2
{

enum class DetailLineTone : std::uint8_t
{
    Normal = 0,
    Muted,
    Accent,
    Positive,
    Danger,
    Header
};

struct DetailLineSnapshot
{
    std::string left;
    std::string right;
    DetailLineTone tone = DetailLineTone::Normal;
};

enum class CharacterAttribute : std::uint8_t
{
    Strength = 0,
    Spirit,
    Talent,
    Dexterity,
    Health
};

struct AttributeImpactSnapshot
{
    float life = 0.0f;
    float mana = 0.0f;
    float stamina = 0.0f;
    float attackRating = 0.0f;
    float defense = 0.0f;
    float maxWeight = 0.0f;
    bool affectsDamage = false;
    bool affectsAbsorption = false;
    bool affectsMoveSpeed = false;
};

struct CharacterStatsSnapshot
{
    bool valid = false;
    std::string name;
    std::string jobName;
    int level = 0;
    int strength = 0;
    int spirit = 0;
    int talent = 0;
    int dexterity = 0;
    int health = 0;
    int availablePoints = 0;

    int lifeCurrent = 0;
    int lifeMax = 0;
    int manaCurrent = 0;
    int manaMax = 0;
    int staminaCurrent = 0;
    int staminaMax = 0;

    int attackRating = 0;
    int damageMin = 0;
    int damageMax = 0;
    int defense = 0;
    int absorption = 0;
    int moveSpeed = 0;
    int critical = 0;
    int block = 0;
    int weightCurrent = 0;
    int weightMax = 0;
    std::array<int, 8> resistances{};
    std::array<AttributeImpactSnapshot, 5> attributeImpacts{};
};

struct SkillEntrySnapshot
{
    int index = -1;
    std::uint32_t code = 0;
    std::string name;
    std::string description;
    int rank = 0;
    int requiredLevel = 0;
    int shortcut = 0;
    int mouseBinding = 0;
    bool visible = false;
    bool unlocked = false;
    bool learned = false;
    bool canUpgrade = false;
    bool leftSelected = false;
    bool rightSelected = false;
    int cooldownGauge = 35;
    bool cooldownReady = true;
    TextureHandle icon{};
};

struct SkillBookSnapshot
{
    bool valid = false;
    int tier1Points = 0;
    int tier4Points = 0;
    int tier5Points = 0;
    std::vector<SkillEntrySnapshot> skills;
};

struct InventoryItemSnapshot
{
    int index = -1;
    std::uint32_t code = 0;
    std::string name;
    int widthPixels = 22;
    int heightPixels = 22;
    int gridX = -1;
    int gridY = -1;
    int equipmentSlot = 0; // legacy ItemPosition, 1-based. 0 == bag.
    int potionCount = 0;
    // SUMMIT442A: UI-only CI hints. NOT authentication or crafting authority.
    bool nativeCiQuantity = false;
    bool ciQuantityInvalid = false;
    bool ciMutationBlocked = false;
    int levelRequirement = 0;
    int weight = 0;
    int durabilityCurrent = 0;
    int durabilityMax = 0;
    bool usable = true;
    bool equipped = false;
    bool questItem = false;
    bool listedForPersonalShop = false;
    TextureHandle texture{};
};

struct InventoryDragSnapshot
{
    bool active = false;
    std::string name;
    int widthPixels = 22;
    int heightPixels = 22;
    TextureHandle texture{};
};

struct InventorySnapshot
{
    bool valid = false;
    int columns = 12;
    int rows = 6;
    int questColumns = 6;
    int questRows = 3;
    int activeBagPage = 1;
    int activeWeaponSet = 1;
    int gold = 0;
    int weightCurrent = 0;
    int weightMax = 0;
    bool locked = false;
    bool costumeMode = false;
    bool personalShopOpen = false;
    bool blacksmithOpen = false;
    std::vector<InventoryItemSnapshot> items;
    InventoryDragSnapshot drag;
};
}
