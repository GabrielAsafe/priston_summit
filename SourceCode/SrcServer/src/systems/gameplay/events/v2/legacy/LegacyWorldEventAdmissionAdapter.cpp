#include "systems/gameplay/events/v2/legacy/LegacyWorldEventAdmissionAdapter.h"

#include <cctype>

#include "systems/gameplay/player/v2/legacy/LegacyPlayerAdapter.h"
#include "systems/network/v2/legacy/LegacyConnectionQuery.h"

namespace gameplay
{
namespace events
{
namespace legacy
{
namespace
{
bool EqualsIgnoreCase(const std::string& left, const std::string& right)
{
    if(left.size() != right.size())
        return false;
    for(std::size_t index = 0; index < left.size(); ++index)
    {
        const unsigned char lhs = static_cast<unsigned char>(left[index]);
        const unsigned char rhs = static_cast<unsigned char>(right[index]);
        if(std::tolower(lhs) != std::tolower(rhs))
            return false;
    }
    return true;
}
}

LegacyWorldEventAdmissionAdapter::LegacyWorldEventAdmissionAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& onlinePlayers,
    const network::IConnectionQuery& connections,
    const player::IPlayerQuery& players)
    : onlinePlayers_(onlinePlayers), connections_(connections), players_(players)
{
}

WorldEventAdmissionStatus LegacyWorldEventAdmissionAdapter::Check(
    const WorldEventAdmissionRequest& request) const
{
    if(!request.player.Valid() || !request.occupiedMap.Valid())
        return WorldEventAdmissionStatus::PlayerUnavailable;
    if(!request.singleCharacterPerRemoteAddress)
        return WorldEventAdmissionStatus::Allowed;

    network::ConnectionSnapshot requesterConnection;
    if(!connections_.TryGet(request.player, requesterConnection) ||
       !requesterConnection.connected || requesterConnection.remoteAddress.empty())
        return WorldEventAdmissionStatus::PlayerUnavailable;

    const std::vector<PlayerHandle> online = onlinePlayers_.OnlineHandles();
    for(std::vector<PlayerHandle>::const_iterator it = online.begin(); it != online.end(); ++it)
    {
        network::ConnectionSnapshot otherConnection;
        if(!connections_.TryGet(it->id, otherConnection) || !otherConnection.connected)
            continue;
        if(!EqualsIgnoreCase(otherConnection.remoteAddress, requesterConnection.remoteAddress))
            continue;

        player::PlayerSnapshot otherPlayer;
        if(!players_.TryGet(*it, otherPlayer))
            return WorldEventAdmissionStatus::Allowed;

        // Preserve the legacy loop exactly: it stops on the first matching IP,
        // even when that first matching character is outside the event map.
        return otherPlayer.position.map == request.occupiedMap
                   ? WorldEventAdmissionStatus::RemoteAddressAlreadyPresent
                   : WorldEventAdmissionStatus::Allowed;
    }

    return WorldEventAdmissionStatus::Allowed;
}

const LegacyWorldEventAdmissionAdapter& ServerLegacyWorldEventAdmissionAdapter()
{
    static LegacyWorldEventAdmissionAdapter adapter(
        player::legacy::ServerLegacyOnlinePlayerRegistry(),
        network::legacy::ServerLegacyConnectionQuery(),
        player::legacy::ServerLegacyPlayerAdapter());
    return adapter;
}
}
}
}
