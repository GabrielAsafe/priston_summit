#include "authentication/v2/LoginSessionDeliveryPolicy.h"

#include <cassert>

using authentication::v2::LoginSessionDeliveryPolicy;
using authentication::v2::LoginSessionDeliveryRoute;
using authentication::v2::LoginSessionEvent;
using authentication::v2::LoginSessionId;
using authentication::v2::LoginSessionState;
using authentication::v2::LoginSessionStateMachine;

int main()
{
    LoginSessionDeliveryPolicy policy;
    LoginSessionStateMachine machine;
    LoginSessionState state(LoginSessionId(55u));

    assert(policy.RouteFor(state) == LoginSessionDeliveryRoute::TransientSession);

    state = machine.Apply(state, LoginSessionEvent::BeginHandshake).state;
    state = machine.Apply(state, LoginSessionEvent::AuthenticationAccepted).state;
    state = machine.Apply(state, LoginSessionEvent::CharacterListPublished).state;
    state = machine.Apply(state, LoginSessionEvent::CharacterSelected).state;
    state = machine.Apply(state, LoginSessionEvent::CharacterRecordRequested).state;
    assert(policy.RequiresTransientSession(state));

    state = machine.Apply(state, LoginSessionEvent::PlayerRegistered, gameplay::PlayerId(500u)).state;
    assert(policy.RouteFor(state) == LoginSessionDeliveryRoute::RegisteredPlayer);

    state = machine.Apply(state, LoginSessionEvent::WorldAdmissionRequested).state;
    state = machine.Apply(state, LoginSessionEvent::WorldAdmissionAccepted).state;
    assert(policy.RouteFor(state) == LoginSessionDeliveryRoute::RegisteredPlayer);

    state = machine.Apply(state, LoginSessionEvent::ReconnectStarted).state;
    assert(policy.RouteFor(state) == LoginSessionDeliveryRoute::TransientSession);
    assert(!state.player.Valid());

    state = machine.Apply(state, LoginSessionEvent::Close).state;
    assert(policy.RouteFor(state) == LoginSessionDeliveryRoute::Unavailable);
    return 0;
}
