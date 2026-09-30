#pragma once

#include <cstdint>
#include <string>

#include "gameplay/core/v2/GameplayIds.h"
#include "gameplay/world/v2/WorldPosition.h"

namespace gameplay
{
namespace player
{
struct PlayerSnapshot
{
    PlayerHandle handle;
    std::string playerKey;      // persistent character key used by modern repositories
    std::string accountKey;     // account identity when a legacy record lookup requires it
    std::string characterName;
    int level;
    int legacyJobCode;
    std::int64_t experience;
    std::int64_t gold;
    WorldPosition position;
    bool online;
    bool alive;
    bool hasCombatTarget;

    PlayerSnapshot()
        : handle(), level(0), legacyJobCode(0), experience(0), gold(0), position(),
          online(false), alive(false), hasCombatTarget(false) {}

    PlayerId Id() const { return handle.id; }
};

struct CharacterSnapshot
{
    PlayerId playerId;
    std::string playerKey;
    std::string name;
    int level;
    int legacyJobCode;
    int life;
    int maxLife;
    int mana;
    int maxMana;

    CharacterSnapshot()
        : playerId(), level(0), legacyJobCode(0), life(0), maxLife(0), mana(0), maxMana(0) {}
};
}
}
