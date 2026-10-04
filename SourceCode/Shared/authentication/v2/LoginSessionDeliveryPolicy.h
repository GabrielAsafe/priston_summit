#pragma once

#include "authentication/v2/LoginSessionState.h"

namespace authentication
{
namespace v2
{
enum class LoginSessionDeliveryRoute
{
    Unavailable = 0,
    TransientSession,
    RegisteredPlayer
};

// GREEN routing policy only. It deliberately knows nothing about legacy
// socket/player structs: AMBER adapters decide how a transient session or registered
// PlayerId is delivered. Character-record/login traffic must remain on the
// transient route until the session state proves player identity is stable.
class LoginSessionDeliveryPolicy final
{
public:
    LoginSessionDeliveryRoute RouteFor(const LoginSessionState& state) const
    {
        if(!state.session.Valid() || state.stage == LoginSessionStage::Closed)
            return LoginSessionDeliveryRoute::Unavailable;
        if(state.HasStablePlayerIdentity())
            return LoginSessionDeliveryRoute::RegisteredPlayer;
        return LoginSessionDeliveryRoute::TransientSession;
    }

    bool RequiresTransientSession(const LoginSessionState& state) const
    {
        return RouteFor(state) == LoginSessionDeliveryRoute::TransientSession;
    }
};
}
}
