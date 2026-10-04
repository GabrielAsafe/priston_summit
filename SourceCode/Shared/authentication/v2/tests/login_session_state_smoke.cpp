#include "authentication/v2/LoginSessionState.h"

#include <cassert>

using authentication::v2::LoginSessionEvent;
using authentication::v2::LoginSessionId;
using authentication::v2::LoginSessionStage;
using authentication::v2::LoginSessionState;
using authentication::v2::LoginSessionStateMachine;

int main()
{
    LoginSessionStateMachine machine;
    LoginSessionState state(LoginSessionId(77));
    assert(state.stage == LoginSessionStage::Connected);
    assert(state.MustUseTransientDelivery());
    assert(!state.HasStablePlayerIdentity());

    state = machine.Apply(state, LoginSessionEvent::BeginHandshake).state;
    state = machine.Apply(state, LoginSessionEvent::AuthenticationAccepted).state;
    state = machine.Apply(state, LoginSessionEvent::CharacterListPublished).state;
    state = machine.Apply(state, LoginSessionEvent::CharacterSelected).state;
    state = machine.Apply(state, LoginSessionEvent::CharacterRecordRequested).state;
    assert(state.stage == LoginSessionStage::CharacterRecordLoading);
    assert(state.MustUseTransientDelivery());

    const authentication::v2::LoginSessionTransitionResult early =
        machine.Apply(state, LoginSessionEvent::WorldAdmissionRequested, gameplay::PlayerId(900));
    assert(!early.Applied());
    assert(early.code == authentication::v2::LoginSessionTransitionCode::PlayerIdentityTooEarly);

    const authentication::v2::LoginSessionTransitionResult missing =
        machine.Apply(state, LoginSessionEvent::PlayerRegistered);
    assert(!missing.Applied());

    state = machine.Apply(state, LoginSessionEvent::PlayerRegistered, gameplay::PlayerId(900)).state;
    assert(state.HasStablePlayerIdentity());
    assert(!state.MustUseTransientDelivery());
    state = machine.Apply(state, LoginSessionEvent::WorldAdmissionRequested).state;
    state = machine.Apply(state, LoginSessionEvent::WorldAdmissionAccepted).state;
    assert(state.stage == LoginSessionStage::InWorld);

    state = machine.Apply(state, LoginSessionEvent::ReconnectStarted).state;
    assert(state.stage == LoginSessionStage::Reconnecting);
    assert(!state.player.Valid());
    assert(state.MustUseTransientDelivery());
    state = machine.Apply(state, LoginSessionEvent::PlayerRegistered, gameplay::PlayerId(901)).state;
    assert(state.player.value == 901u);

    state = machine.Apply(state, LoginSessionEvent::Close).state;
    assert(state.stage == LoginSessionStage::Closed);
    assert(!state.session.Valid() || !state.MustUseTransientDelivery());
    return 0;
}
