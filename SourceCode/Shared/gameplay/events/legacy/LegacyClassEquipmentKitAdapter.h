#pragma once

#include <cstddef>
#include <cstdint>

namespace gameplay
{
namespace events
{
namespace legacy
{
enum class LegacyEquipmentGrantKind
{
    SpecializedItem,
    AgedSpecializedItem
};

struct LegacyEquipmentGrant
{
    std::uint32_t itemCode;
    int specializationJobCode;
    LegacyEquipmentGrantKind kind;
    int agingLevel;

    LegacyEquipmentGrant()
        : itemCode(0u), specializationJobCode(0),
          kind(LegacyEquipmentGrantKind::SpecializedItem), agingLevel(0) {}

    LegacyEquipmentGrant(std::uint32_t code, int jobCode,
                         LegacyEquipmentGrantKind grantKind, int aging)
        : itemCode(code), specializationJobCode(jobCode),
          kind(grantKind), agingLevel(aging) {}
};

// CaixaItensRandom is historical event content. The server runtime should not
// enumerate concrete classes just to decide which bundle to grant. These
// fixed-capacity specs preserve the old item order exactly and make support
// for future classes an explicit opt-in at this content boundary.
struct LegacyEquipmentKit
{
    static const std::size_t kCapacity = 16u;
    LegacyEquipmentGrant grants[kCapacity];
    std::size_t count;

    LegacyEquipmentKit() : count(0u) {}

    bool Add(std::uint32_t itemCode, int specializationJobCode,
             LegacyEquipmentGrantKind kind, int agingLevel = 0)
    {
        if(count >= kCapacity)
            return false;
        grants[count++] = LegacyEquipmentGrant(
            itemCode, specializationJobCode, kind, agingLevel);
        return true;
    }
};

inline bool IsLegacyKitJob(int jobCode)
{
    switch(jobCode)
    {
    case 1: case 2: case 3: case 4:
    case 5: case 6: case 7: case 8:
        return true;
    default:
        return false;
    }
}

inline bool IsLegacyCasterKitJob(int jobCode)
{
    return jobCode == 7 || jobCode == 8;
}

inline bool AppendHighTierWeapons(int jobCode, LegacyEquipmentKit& out)
{
    switch(jobCode)
    {
    case 1: // Fighter
        return out.Add(0x01014100u, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 20) &&
               out.Add(0x01011900u, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 20);
    case 2: // Mechanician
        return out.Add(0x01024100u, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 20) &&
               out.Add(0x01050F00u, jobCode, LegacyEquipmentGrantKind::SpecializedItem);
    case 3: // Archer
        return out.Add(0x01064100u, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 20);
    case 4: // Pikeman
        return out.Add(0x01050F00u, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 20);
    case 5: // Atalanta
        return out.Add(0x01084100u, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 20);
    case 6: // Knight
        return out.Add(0x01072C00u, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 20) &&
               out.Add(0x01074200u, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 20);
    case 7: // Magician
    case 8: // Priestess
        return out.Add(0x01042A00u, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 20) &&
               out.Add(0x01044200u, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 20);
    default:
        return false;
    }
}

inline bool BuildHighTierLegacyClassKit(int jobCode, LegacyEquipmentKit& out)
{
    out.count = 0u;
    if(!IsLegacyKitJob(jobCode))
        return false;

    const std::uint32_t armor = IsLegacyCasterKitJob(jobCode) ? 0x02052A00u : 0x02012A00u;
    if(!out.Add(armor, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 20) ||
       !out.Add(0x02022B00u, jobCode, LegacyEquipmentGrantKind::SpecializedItem) ||
       !out.Add(0x02032B00u, jobCode, LegacyEquipmentGrantKind::SpecializedItem) ||
       !out.Add(0x03022B00u, jobCode, LegacyEquipmentGrantKind::SpecializedItem) ||
       !out.Add(0x03042D00u, jobCode, LegacyEquipmentGrantKind::SpecializedItem) ||
       !out.Add(0x03042D00u, jobCode, LegacyEquipmentGrantKind::SpecializedItem) ||
       !out.Add(0x03012D00u, jobCode, LegacyEquipmentGrantKind::SpecializedItem) ||
       !out.Add(0x030A0700u, jobCode, LegacyEquipmentGrantKind::SpecializedItem) ||
       !out.Add(0x030A0700u, jobCode, LegacyEquipmentGrantKind::SpecializedItem) ||
       !out.Add(0x02351100u, jobCode, LegacyEquipmentGrantKind::SpecializedItem) ||
       !out.Add(0x02041800u, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 20))
        return false;

    return AppendHighTierWeapons(jobCode, out);
}

inline bool AppendDivineTierWeapon(int jobCode, LegacyEquipmentKit& out)
{
    std::uint32_t weapon = 0u;
    switch(jobCode)
    {
    case 1: weapon = 0x01010E00u; break; // Axe
    case 2: weapon = 0x01030F00u; break; // Hammer
    case 3: weapon = 0x01061000u; break; // Bow
    case 4: weapon = 0x01050F00u; break; // Pike
    case 5: weapon = 0x01080F00u; break; // Javelin
    case 6: weapon = 0x01071100u; break; // Sword
    case 7:
    case 8: weapon = 0x01040F00u; break; // Staff
    default: return false;
    }
    return out.Add(weapon, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 8);
}

inline bool BuildDivineLegacyClassKit(int jobCode, LegacyEquipmentKit& out)
{
    out.count = 0u;
    if(!IsLegacyKitJob(jobCode))
        return false;

    const bool caster = IsLegacyCasterKitJob(jobCode);
    const std::uint32_t armor = caster ? 0x02051000u : 0x02011000u;
    const std::uint32_t shield = caster ? 0x02040800u : 0x02040E00u;
    if(!out.Add(armor, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 8) ||
       !out.Add(0x02020E00u, jobCode, LegacyEquipmentGrantKind::SpecializedItem) ||
       !out.Add(0x02030E00u, jobCode, LegacyEquipmentGrantKind::SpecializedItem) ||
       !out.Add(0x03020E00u, jobCode, LegacyEquipmentGrantKind::SpecializedItem) ||
       !out.Add(shield, jobCode, LegacyEquipmentGrantKind::AgedSpecializedItem, 8) ||
       !AppendDivineTierWeapon(jobCode, out))
        return false;

    for(int i = 0; i < 5; ++i)
    {
        if(!out.Add(0x080B0800u, 0, LegacyEquipmentGrantKind::SpecializedItem))
            return false;
    }
    return true;
}

inline bool TryBuildRandomBoxClassKit(int boxType, int jobCode, LegacyEquipmentKit& out)
{
    switch(boxType)
    {
    case 11: return BuildHighTierLegacyClassKit(jobCode, out);
    case 14: return BuildDivineLegacyClassKit(jobCode, out);
    default:
        out.count = 0u;
        return false;
    }
}
}
}
}
