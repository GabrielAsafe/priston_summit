#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace companions
{
class ICompanionLifecycle
{
public:
    virtual ~ICompanionLifecycle() {}
    virtual void OwnerDisconnected(PlayerHandle owner) = 0;
};
}
}
