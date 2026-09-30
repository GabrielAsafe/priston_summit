#pragma once

#include "gameplay/companions/CompanionProtocol.h"
#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace companions
{
class ICompanionUIIngress
{
public:
    virtual ~ICompanionUIIngress() {}
    virtual void HandleAction(PlayerId owner, const protocol::CompanionUIActionPacket& packet) = 0;
};
}
}
