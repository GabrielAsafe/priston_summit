#pragma once

#include "gameplay/core/v2/GameplayIds.h"
#include "gameplay/covenants/v2/CovenantUIProtocol.h"

namespace gameplay
{
namespace covenants
{
class ICovenantUIIngress
{
public:
    virtual ~ICovenantUIIngress() {}
    virtual void HandleAction(PlayerId owner,
                              const protocol::CovenantUIActionPacket& packet) = 0;
};
}
}
