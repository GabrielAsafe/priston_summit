#pragma once

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace events
{
enum class WorldEventAdmissionStatus
{
    Allowed,
    PlayerUnavailable,
    RemoteAddressAlreadyPresent
};

struct WorldEventAdmissionRequest
{
    PlayerId player;
    MapId occupiedMap;
    bool singleCharacterPerRemoteAddress;

    WorldEventAdmissionRequest(PlayerId playerValue = PlayerId(),
                               MapId occupiedMapValue = MapId(),
                               bool singleCharacterPerRemoteAddressValue = false)
        : player(playerValue), occupiedMap(occupiedMapValue),
          singleCharacterPerRemoteAddress(singleCharacterPerRemoteAddressValue) {}
};

class IWorldEventAdmissionService
{
public:
    virtual ~IWorldEventAdmissionService() {}
    virtual WorldEventAdmissionStatus Check(const WorldEventAdmissionRequest& request) const = 0;
};
}
}
