#pragma once

#include <cstdint>
#include <string>

class rsPLAYINFO;
class smCHAR;

namespace gameplay
{
namespace events
{
struct MonsterKilledEvent
{
    rsPLAYINFO* player;
    smCHAR* monster;
    bool partyCredit;

    // Stable event snapshot for new systems. Consumers that do not need the
    // legacy objects can use these values and stay decoupled from smPacket.
    std::string playerName;
    std::string monsterName;
    std::uint32_t monsterCode;
    int area;

    MonsterKilledEvent(rsPLAYINFO* playerValue,
                       smCHAR* monsterValue,
                       bool partyCreditValue,
                       const std::string& playerNameValue,
                       const std::string& monsterNameValue,
                       std::uint32_t monsterCodeValue,
                       int areaValue)
        : player(playerValue),
          monster(monsterValue),
          partyCredit(partyCreditValue),
          playerName(playerNameValue),
          monsterName(monsterNameValue),
          monsterCode(monsterCodeValue),
          area(areaValue)
    {
    }
};
}
}
