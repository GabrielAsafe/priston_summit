#pragma once

#include <cstdint>
#include <string>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace events
{
struct MonsterKilledEvent
{
    PlayerId player;
    EntityId monster;
    bool partyCredit;
    bool monsterBoss;
    std::string playerName;
    std::string monsterName;
    std::uint32_t monsterCode;
    int area;

    MonsterKilledEvent(PlayerId playerValue = PlayerId(),
                       EntityId monsterValue = EntityId(),
                       bool partyCreditValue = false,
                       bool monsterBossValue = false,
                       const std::string& playerNameValue = std::string(),
                       const std::string& monsterNameValue = std::string(),
                       std::uint32_t monsterCodeValue = 0u,
                       int areaValue = -1)
        : player(playerValue),
          monster(monsterValue),
          partyCredit(partyCreditValue),
          monsterBoss(monsterBossValue),
          playerName(playerNameValue),
          monsterName(monsterNameValue),
          monsterCode(monsterCodeValue),
          area(areaValue)
    {
    }
};
}
}
