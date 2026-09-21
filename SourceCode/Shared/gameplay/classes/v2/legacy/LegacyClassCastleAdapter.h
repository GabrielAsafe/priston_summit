#pragma once

#include <cstdint>

namespace gameplay
{
namespace classes
{
namespace legacy
{
// Bless Castle class-target crystals are legacy content, not class identity.
// Keep their fixed item->class mapping behind this adapter so the server runtime
// does not enumerate concrete JOBCODE_* values. A future class only participates
// when Castle content explicitly assigns it a crystal/item code.
inline bool TryBlessCastleClassStoneTarget(std::uint32_t itemCode, int& legacyJobCode)
{
    switch (itemCode)
    {
    case 0x080A0900u: legacyJobCode = 1; return true; // Fighter
    case 0x080A0A00u: legacyJobCode = 2; return true; // Mechanician
    case 0x080A0B00u: legacyJobCode = 4; return true; // Pikeman
    case 0x080A0C00u: legacyJobCode = 3; return true; // Archer
    case 0x080A0D00u: legacyJobCode = 6; return true; // Knight
    case 0x080A0E00u: legacyJobCode = 5; return true; // Atalanta
    case 0x080A0F00u: legacyJobCode = 7; return true; // Magician
    case 0x080A1000u: legacyJobCode = 8; return true; // Priestess
    default: break;
    }
    legacyJobCode = 0;
    return false;
}
}
}
}
