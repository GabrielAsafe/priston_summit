#pragma once

#include <string>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace gathering
{
struct HarvestPlayerSnapshot
{
    PlayerId player;
    std::string playerName;
    int area;
    int x;
    int y;
    int z;

    HarvestPlayerSnapshot()
        : area(-1), x(0), y(0), z(0)
    {
    }
};

class IHarvestPlayerQuery
{
public:
    virtual ~IHarvestPlayerQuery() {}
    virtual bool TryGet(PlayerId player, HarvestPlayerSnapshot& out) const = 0;
};
}
}
