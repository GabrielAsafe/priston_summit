#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "systems/ui/v2/rendering/TextureHandle.h"

namespace ui::v2
{
enum class ContextKind : std::uint8_t
{
    None = 0,
    Merchant,
    PlayerMerchant,
    PersonalShop,
    Restoration,
    Trade,
    Warehouse,
    Mixing,
    Aging,
    Smelting,
    Manufacture,
    MixtureReset,
    SkinChange,
    CoinShop,
    TimeCoinShop
};

struct ContextItemSnapshot
{
    int index = -1;
    std::string name;
    std::string caption;
    int price = 0;
    bool interactive = true;
    bool remote = false;
    bool selected = false;
    bool classCompatible = true;
    bool requirementsMet = true;
    std::string restriction;
    int gridX = -1;
    int gridY = -1;
    int gridWidth = 1;
    int gridHeight = 1;
    TextureHandle texture{};
};

struct ContextSlotSnapshot
{
    int gridX = 0;
    int gridY = 0;
    int gridWidth = 1;
    int gridHeight = 1;
    std::string label;
    TextureHandle hintTexture{};
    bool required = false;
    bool special = false;
};

struct ContextActionSnapshot
{
    int id = 0;
    std::string label;
    bool enabled = true;
    bool selected = false;
};

struct ContextSnapshot
{
    bool valid = false;
    ContextKind kind = ContextKind::None;
    std::string title;
    std::string subtitle;
    int money = 0;
    int fee = 0;
    std::string currency = "Gold";
    std::string feeCurrency = "Gold";
    std::string statusText;
    int gridColumns = 0;
    int gridRows = 0;
    bool showEmptyGrid = false;
    std::vector<ContextSlotSnapshot> slots;
    std::vector<ContextItemSnapshot> items;
    std::vector<ContextActionSnapshot> actions;
};
}
