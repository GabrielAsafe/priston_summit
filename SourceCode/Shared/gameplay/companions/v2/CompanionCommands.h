#pragma once

#include <cstdint>
#include <string>

#include "gameplay/companions/v2/CompanionQuery.h"
#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace companions
{
class ICompanionCommands
{
public:
    virtual ~ICompanionCommands() {}

    virtual int Select(PlayerId owner, const std::string& characterName) = 0;
    virtual int Summon(PlayerId owner, const std::string& characterName) = 0;
    virtual int SummonSelected(PlayerId owner) = 0;
    virtual int Dismiss(PlayerId owner, const std::string& characterName) = 0;
    virtual int DismissAll(PlayerId owner) = 0;
    virtual int SetMode(PlayerId owner, CompanionBehaviorMode mode) = 0;
    virtual int UseCrystal(PlayerId owner, std::uint32_t itemCode) = 0;
};
}
}
