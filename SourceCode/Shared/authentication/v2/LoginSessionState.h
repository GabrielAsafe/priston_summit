#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace authentication
{
namespace v2
{
struct LoginSessionId
{
    std::uint64_t value;
    explicit constexpr LoginSessionId(std::uint64_t valueValue = 0u) : value(valueValue) {}
    constexpr bool Valid() const { return value != 0u; }
};

inline constexpr bool operator==(LoginSessionId lhs, LoginSessionId rhs) { return lhs.value == rhs.value; }
inline constexpr bool operator!=(LoginSessionId lhs, LoginSessionId rhs) { return !(lhs == rhs); }

// Pre-player identity is intentionally separate from gameplay::PlayerId.  A
// session is valid from socket admission onward, while PlayerId only becomes
// authoritative after character-record promotion/registration.
enum class LoginSessionStage
{
    Connected = 0,
    Handshaking,
    Authenticated,
    CharacterListReady,
    CharacterSelected,
    CharacterRecordLoading,
    PlayerRegistered,
    WorldAdmissionPending,
    InWorld,
    Reconnecting,
    Closed
};

enum class LoginSessionEvent
{
    BeginHandshake = 0,
    AuthenticationAccepted,
    CharacterListPublished,
    CharacterSelected,
    CharacterRecordRequested,
    PlayerRegistered,
    WorldAdmissionRequested,
    WorldAdmissionAccepted,
    ReconnectStarted,
    Close
};

enum class LoginSessionTransitionCode
{
    Applied = 0,
    InvalidSession,
    InvalidTransition,
    MissingPlayerIdentity,
    PlayerIdentityTooEarly
};

struct LoginSessionState
{
    LoginSessionId session;
    LoginSessionStage stage;
    gameplay::PlayerId player;

    LoginSessionState()
        : session(), stage(LoginSessionStage::Closed), player() {}

    explicit LoginSessionState(LoginSessionId sessionValue)
        : session(sessionValue), stage(sessionValue.Valid() ? LoginSessionStage::Connected : LoginSessionStage::Closed), player() {}

    bool HasStablePlayerIdentity() const
    {
        return player.Valid() &&
               (stage == LoginSessionStage::PlayerRegistered ||
                stage == LoginSessionStage::WorldAdmissionPending ||
                stage == LoginSessionStage::InWorld);
    }

    bool MustUseTransientDelivery() const
    {
        return session.Valid() && stage != LoginSessionStage::Closed && !HasStablePlayerIdentity();
    }
};

struct LoginSessionTransitionResult
{
    LoginSessionTransitionCode code;
    LoginSessionState state;

    LoginSessionTransitionResult(LoginSessionTransitionCode codeValue,
                                 const LoginSessionState& stateValue)
        : code(codeValue), state(stateValue) {}

    bool Applied() const { return code == LoginSessionTransitionCode::Applied; }
};

class LoginSessionStateMachine final
{
public:
    LoginSessionTransitionResult Apply(const LoginSessionState& current,
                                       LoginSessionEvent event,
                                       gameplay::PlayerId player = gameplay::PlayerId()) const
    {
        if(!current.session.Valid())
            return LoginSessionTransitionResult(LoginSessionTransitionCode::InvalidSession, current);

        LoginSessionState next = current;
        if(event == LoginSessionEvent::Close)
        {
            next.stage = LoginSessionStage::Closed;
            next.player = gameplay::PlayerId();
            return LoginSessionTransitionResult(LoginSessionTransitionCode::Applied, next);
        }

        if(event != LoginSessionEvent::PlayerRegistered && player.Valid() && !current.HasStablePlayerIdentity())
            return LoginSessionTransitionResult(LoginSessionTransitionCode::PlayerIdentityTooEarly, current);

        switch(current.stage)
        {
        case LoginSessionStage::Connected:
            if(event == LoginSessionEvent::BeginHandshake)
                next.stage = LoginSessionStage::Handshaking;
            else
                return Invalid(current);
            break;
        case LoginSessionStage::Handshaking:
            if(event == LoginSessionEvent::AuthenticationAccepted)
                next.stage = LoginSessionStage::Authenticated;
            else
                return Invalid(current);
            break;
        case LoginSessionStage::Authenticated:
            if(event == LoginSessionEvent::CharacterListPublished)
                next.stage = LoginSessionStage::CharacterListReady;
            else if(event == LoginSessionEvent::ReconnectStarted)
                next.stage = LoginSessionStage::Reconnecting;
            else
                return Invalid(current);
            break;
        case LoginSessionStage::CharacterListReady:
            if(event == LoginSessionEvent::CharacterSelected)
                next.stage = LoginSessionStage::CharacterSelected;
            else if(event == LoginSessionEvent::ReconnectStarted)
                next.stage = LoginSessionStage::Reconnecting;
            else
                return Invalid(current);
            break;
        case LoginSessionStage::CharacterSelected:
            if(event == LoginSessionEvent::CharacterRecordRequested)
                next.stage = LoginSessionStage::CharacterRecordLoading;
            else if(event == LoginSessionEvent::ReconnectStarted)
                next.stage = LoginSessionStage::Reconnecting;
            else
                return Invalid(current);
            break;
        case LoginSessionStage::CharacterRecordLoading:
        case LoginSessionStage::Reconnecting:
            if(event != LoginSessionEvent::PlayerRegistered)
                return Invalid(current);
            if(!player.Valid())
                return LoginSessionTransitionResult(LoginSessionTransitionCode::MissingPlayerIdentity, current);
            next.player = player;
            next.stage = LoginSessionStage::PlayerRegistered;
            break;
        case LoginSessionStage::PlayerRegistered:
            if(event == LoginSessionEvent::WorldAdmissionRequested)
                next.stage = LoginSessionStage::WorldAdmissionPending;
            else if(event == LoginSessionEvent::ReconnectStarted)
            {
                next.stage = LoginSessionStage::Reconnecting;
                next.player = gameplay::PlayerId();
            }
            else
                return Invalid(current);
            break;
        case LoginSessionStage::WorldAdmissionPending:
            if(event == LoginSessionEvent::WorldAdmissionAccepted)
                next.stage = LoginSessionStage::InWorld;
            else if(event == LoginSessionEvent::ReconnectStarted)
            {
                next.stage = LoginSessionStage::Reconnecting;
                next.player = gameplay::PlayerId();
            }
            else
                return Invalid(current);
            break;
        case LoginSessionStage::InWorld:
            if(event == LoginSessionEvent::ReconnectStarted)
            {
                next.stage = LoginSessionStage::Reconnecting;
                next.player = gameplay::PlayerId();
            }
            else
                return Invalid(current);
            break;
        case LoginSessionStage::Closed:
        default:
            return Invalid(current);
        }

        return LoginSessionTransitionResult(LoginSessionTransitionCode::Applied, next);
    }

private:
    static LoginSessionTransitionResult Invalid(const LoginSessionState& state)
    {
        return LoginSessionTransitionResult(LoginSessionTransitionCode::InvalidTransition, state);
    }
};
}
}
