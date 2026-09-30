#pragma once

#include <string>

#include "gameplay/player/v2/PlayerSnapshot.h"

namespace gameplay
{
namespace player
{
class IPlayerQuery
{
public:
    virtual ~IPlayerQuery() {}

    virtual bool TryGet(PlayerId player, PlayerSnapshot& out) const = 0;
    virtual bool TryGet(PlayerHandle player, PlayerSnapshot& out) const = 0;
    virtual bool TryGetByKey(const std::string& playerKey, PlayerSnapshot& out) const = 0;
    virtual bool IsOnline(PlayerId player) const = 0;
};

class ICharacterQuery
{
public:
    virtual ~ICharacterQuery() {}
    virtual bool TryGetCharacter(PlayerId player, CharacterSnapshot& out) const = 0;
};
}
}
