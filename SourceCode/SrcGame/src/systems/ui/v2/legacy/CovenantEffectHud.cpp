#include "systems/ui/v2/hud/CovenantEffectHud.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>

#include <windows.h>

#include "compat/legacy/sin/sinLinkHeader.h"
#include "systems/commerce/premium/HaPremiumItem.h"
#include "systems/rendering/2d/legacy/drawsub.h"

namespace ui::v2
{
namespace
{
struct HudEffect
{
    int materialIndex;
    bool detrimental;
    char text[128];
};

const char* CovenantName(int id)
{
    switch (id)
    {
        case 1: return "Hunters";
        case 2: return "Gatherers";
        case 3: return "Artisans";
        case 4: return "Fortune";
        case 5: return "Ascendants";
        default: return "Covenant";
    }
}

bool Contains(const std::string& value, const char* token)
{
    return value.find(token) != std::string::npos;
}

bool IsDetrimental(const CovenantUiModifier& modifier)
{
    if (modifier.valueBasisPoints < 0)
        return true;
    // Positive values on incoming damage are a deliberate Covenant tradeoff.
    if (Contains(modifier.statKey, "damage_taken"))
        return modifier.valueBasisPoints > 0;
    return false;
}

int MaterialFor(const CovenantUiModifier& modifier, bool detrimental)
{
    if (detrimental)
        return nsPremiumItem::MANA_REDUCE_P - 1;
    if (Contains(modifier.statKey, "xp"))
        return nsPremiumItem::EXPUP_POTION - 1;
    if (Contains(modifier.statKey, "drop") || Contains(modifier.statKey, "loot"))
        return nsPremiumItem::THIRD_EYES - 1;
    if (Contains(modifier.statKey, "gather") || Contains(modifier.statKey, "survey"))
        return nsPremiumItem::PHENIX_PET - 1;
    if (Contains(modifier.statKey, "craft") || Contains(modifier.statKey, "artisan"))
        return nsPremiumItem::MANA_RECHAR_P - 1;
    if (Contains(modifier.statKey, "hunt") || Contains(modifier.statKey, "trophy"))
        return nsPremiumItem::MIGHT_OF_AWELL - 1;
    return nsPremiumItem::PHENIX_PET - 1;
}

void FriendlyStat(const std::string& key, char* out, std::size_t outSize)
{
    const char* text = key.c_str();
    if (key == "xp.monster_gain") text = "Monster XP";
    else if (key == "monster.drop_chance") text = "Monster Drop";
    else if (key == "combat.damage_taken") text = "Damage Taken";
    else if (key == "loot.extra_roll_chance") text = "Extra Loot Roll";
    else if (key == "hunt.drop_chance") text = "Hunt Drop";
    else if (key == "hunt.trophy_chance") text = "Trophy Chance";
    else if (key == "hunt.trophy_refining_enabled") text = "Trophy Refining";
    else if (key == "hunt.mark_enabled") text = "Hunter's Mark";
    else if (key == "gathering.surveyor_enabled") text = "Surveyor";
    else if (key == "gathering.yield") text = "Gathering Yield";
    else if (key == "gathering.extra_yield_chance") text = "Extra Gathering";
    else if (key == "gathering.special_material_chance") text = "Special Materials";
    else if (key == "gathering.refined_material_chance") text = "Refined Materials";
    else if (key == "crafting.reforge_enabled") text = "Reforge";
    else if (key == "crafting.forgemaster_enabled") text = "Forgemaster";
    else if (key == "crafting.master_artisan_enabled") text = "Master Artisan";
    else if (key == "vendor.blacksmith_sell_value" || key == "crafting.npc_sell_value") text = "Blacksmith Sale Value";
    std::snprintf(out, outSize, "%s", text);
}

void FormatEffect(const CovenantUiModifier& modifier, HudEffect& effect)
{
    effect.detrimental = IsDetrimental(modifier);
    effect.materialIndex = MaterialFor(modifier, effect.detrimental);
    char stat[72] = {0};
    FriendlyStat(modifier.statKey, stat, sizeof(stat));

    // Flat feature flags are displayed as enabled; percentage operations use
    // basis points where 1000 == 10.00%.
    if (modifier.operation == 0 && (modifier.valueBasisPoints == 10000 || modifier.valueBasisPoints == 1))
        std::snprintf(effect.text, sizeof(effect.text), "%s: enabled", stat);
    else
    {
        const double value = static_cast<double>(modifier.valueBasisPoints) / 100.0;
        std::snprintf(effect.text, sizeof(effect.text), "%s: %+.2f%%", stat, value);
    }
}

void DrawTooltip(int x, int y, const char* text, bool detrimental)
{
    if (!text || !text[0])
        return;
    const int textWidth = std::min(300, 18 + static_cast<int>(std::strlen(text)) * 7);
    const int tx = std::clamp(x - textWidth + 32, 8, std::max(8, WinSizeX - textWidth - 8));
    const int ty = std::max(8, y + 8);
    dsDrawColorBox(D3DCOLOR_RGBA(18, 22, 30, 220), tx - 6, ty - 4, textWidth + 12, 24);
    SetFontTextColor32(RGB(0, 0, 0));
    dsTextLineOut32(tx + 1, ty + 1, text, static_cast<int>(std::strlen(text)));
    SetFontTextColor32(detrimental ? RGB(255, 112, 112) : RGB(142, 235, 170));
    dsTextLineOut32(tx, ty, text, static_cast<int>(std::strlen(text)));
    SetFontTextColor32(RGB(255, 255, 255));
}
}

void CovenantEffectHud::Render(const CovenantUiSnapshot& snapshot) const
{
    if (!snapshot.valid || snapshot.activeCovenantId <= 0)
        return;

    // Premium upkeep already occupies up to three right-side columns. Put
    // Covenant effects in the next column while retaining its 32px/36px rhythm.
    const int baseX = WinSizeX - 164;
    const int baseY = WinSizeY / 2 - 145;
    const int maxVisible = 8;
    const int count = std::min(snapshot.activeModifierCount, maxVisible);

    for (int i = 0; i < count; ++i)
    {
        HudEffect effect{};
        FormatEffect(snapshot.activeModifiers[static_cast<std::size_t>(i)], effect);
        if (effect.materialIndex < 0 || effect.materialIndex >= SINUPKEEPITEM_MAX)
            continue;
        const int material = chaPremiumitem.UpKeepItemTGAImage[effect.materialIndex];
        if (!material)
            continue;
        const int x = baseX;
        const int y = baseY + i * 36;
        dsDrawColorBox(D3DCOLOR_RGBA(18, 22, 30, 190), x - 1, y - 1, 34, 34);
        dsDrawTexImage(material, x, y, 32, 32, 255);
        if (pRealCursorPos.x >= x && pRealCursorPos.x < x + 32 &&
            pRealCursorPos.y >= y && pRealCursorPos.y < y + 32)
            DrawTooltip(x, y, effect.text, effect.detrimental);
    }

    // Active Tier III powers get a Phoenix-style icon beneath passive effects.
    if (snapshot.activePower.activeUntil > snapshot.serverNow && count < maxVisible)
    {
        const int material = chaPremiumitem.UpKeepItemTGAImage[nsPremiumItem::PHENIX_PET - 1];
        if (material)
        {
            const int x = baseX;
            const int y = baseY + count * 36;
            dsDrawColorBox(D3DCOLOR_RGBA(18, 22, 30, 190), x - 1, y - 1, 34, 34);
            dsDrawTexImage(material, x, y, 32, 32, 255);
            if (pRealCursorPos.x >= x && pRealCursorPos.x < x + 32 &&
                pRealCursorPos.y >= y && pRealCursorPos.y < y + 32)
            {
                char text[128] = {0};
                const long long remain = snapshot.activePower.activeUntil - snapshot.serverNow;
                std::snprintf(text, sizeof(text), "%s - %s (%llds)", CovenantName(snapshot.activeCovenantId),
                              snapshot.activePower.key.c_str(), remain > 0 ? remain : 0);
                DrawTooltip(x, y, text, false);
            }
        }
    }
}
}
