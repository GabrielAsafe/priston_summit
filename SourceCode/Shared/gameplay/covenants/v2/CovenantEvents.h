#pragma once

#include <string>

#include "gameplay/covenants/v2/CovenantTypes.h"

namespace gameplay
{
namespace covenants
{
struct CovenantJoinedEvent
{
    std::string playerKey;
    CovenantId covenantId;
};

struct CovenantLeftEvent
{
    std::string playerKey;
    CovenantId covenantId;
};

struct CovenantActivatedEvent
{
    std::string playerKey;
    CovenantId covenantId;
    CovenantId previousCovenantId;

    CovenantActivatedEvent() : covenantId(0), previousCovenantId(0) {}
};

struct CovenantReputationChangedEvent
{
    std::string playerKey;
    CovenantId covenantId;
    int oldReputation;
    int newReputation;
    std::string sourceKey;
};

struct CovenantRankChangedEvent
{
    std::string playerKey;
    CovenantId covenantId;
    CovenantRankId oldRankId;
    CovenantRankId newRankId;
};
}
}
