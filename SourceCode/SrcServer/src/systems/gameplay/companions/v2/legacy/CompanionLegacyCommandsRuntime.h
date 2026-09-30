#pragma once

#include <cstdint>

class rsPLAYINFO;

namespace gameplay
{
namespace companions
{
enum CompanionMode
{
    COMPANION_MODE_AGGRESSIVE = 0,
    COMPANION_MODE_FOLLOW = 1,
    COMPANION_MODE_PASSIVE = 2
};

int Summon(rsPLAYINFO* owner, const char* characterName);
int SummonSelected(rsPLAYINFO* owner);
int Select(rsPLAYINFO* owner, const char* characterName);
int Dismiss(rsPLAYINFO* owner, const char* characterName);
int DismissAll(rsPLAYINFO* owner);
int SetMode(rsPLAYINFO* owner, CompanionMode mode);
int HandleCrystalUse(rsPLAYINFO* owner, std::uint32_t itemCode);
}
}
