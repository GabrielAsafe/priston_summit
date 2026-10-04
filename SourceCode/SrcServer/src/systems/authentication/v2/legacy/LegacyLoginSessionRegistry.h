#pragma once

#include <cstdint>
#include <map>
#include <mutex>

#include "authentication/v2/LoginSessionState.h"
#include "core/server/legacy_sin_runtime/sinLinkHeader.h"

namespace authentication
{
namespace v2
{
namespace legacy
{
class LegacyLoginSessionRegistry final
{
public:
    LoginSessionId Open(rsPLAYINFO* player)
    {
        if(!player)
            return LoginSessionId();
        const LoginSessionId id = IdOf(player);
        std::lock_guard<std::mutex> lock(mutex_);
        sessions_[id.value] = LoginSessionState(id);
        return id;
    }

    bool BeginHandshake(rsPLAYINFO* player)
    {
        return Apply(player, LoginSessionEvent::BeginHandshake);
    }

    bool MarkAuthenticatedAndCharacterListReady(rsPLAYINFO* player)
    {
        if(!Apply(player, LoginSessionEvent::AuthenticationAccepted))
            return false;
        return Apply(player, LoginSessionEvent::CharacterListPublished);
    }

    bool BeginCharacterRecordLoad(rsPLAYINFO* player)
    {
        if(!Apply(player, LoginSessionEvent::CharacterSelected))
            return false;
        return Apply(player, LoginSessionEvent::CharacterRecordRequested);
    }

    bool RegisterPlayer(rsPLAYINFO* player)
    {
        if(!player || !player->dwObjectSerial)
            return false;
        return Apply(player,
                     LoginSessionEvent::PlayerRegistered,
                     gameplay::PlayerId(static_cast<std::uint32_t>(player->dwObjectSerial)));
    }

    bool RequestWorldAdmission(rsPLAYINFO* player)
    {
        return Apply(player, LoginSessionEvent::WorldAdmissionRequested);
    }

    bool AcceptWorldAdmission(rsPLAYINFO* player)
    {
        return Apply(player, LoginSessionEvent::WorldAdmissionAccepted);
    }

    bool BeginReconnect(rsPLAYINFO* player)
    {
        return Apply(player, LoginSessionEvent::ReconnectStarted);
    }

    bool TryGet(rsPLAYINFO* player, LoginSessionState& out) const
    {
        if(!player)
            return false;
        const LoginSessionId id = IdOf(player);
        std::lock_guard<std::mutex> lock(mutex_);
        const std::map<std::uint64_t, LoginSessionState>::const_iterator found = sessions_.find(id.value);
        if(found == sessions_.end())
            return false;
        out = found->second;
        return true;
    }

    void Close(rsPLAYINFO* player)
    {
        if(!player)
            return;
        const LoginSessionId id = IdOf(player);
        std::lock_guard<std::mutex> lock(mutex_);
        std::map<std::uint64_t, LoginSessionState>::iterator found = sessions_.find(id.value);
        if(found == sessions_.end())
            return;
        found->second = machine_.Apply(found->second, LoginSessionEvent::Close).state;
        sessions_.erase(found);
    }

    static LoginSessionId IdOf(const rsPLAYINFO* player)
    {
        // dwConnectCount is assigned before the connection is published and is
        // incremented for every accepted socket.  +1 keeps zero reserved as an
        // invalid GREEN identity while preserving the legacy generation value.
        return player ? LoginSessionId(static_cast<std::uint64_t>(player->dwConnectCount) + 1u)
                      : LoginSessionId();
    }

private:
    bool Apply(rsPLAYINFO* player,
               LoginSessionEvent event,
               gameplay::PlayerId gameplayPlayer = gameplay::PlayerId())
    {
        if(!player)
            return false;
        const LoginSessionId id = IdOf(player);
        std::lock_guard<std::mutex> lock(mutex_);
        std::map<std::uint64_t, LoginSessionState>::iterator found = sessions_.find(id.value);
        if(found == sessions_.end())
            return false;
        const LoginSessionTransitionResult result = machine_.Apply(found->second, event, gameplayPlayer);
        if(!result.Applied())
            return false;
        found->second = result.state;
        return true;
    }

    mutable std::mutex mutex_;
    std::map<std::uint64_t, LoginSessionState> sessions_;
    LoginSessionStateMachine machine_;
};

inline LegacyLoginSessionRegistry& ServerLegacyLoginSessionRegistry()
{
    static LegacyLoginSessionRegistry registry;
    return registry;
}
}
}
}
