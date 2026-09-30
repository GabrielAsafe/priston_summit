#pragma once

#include <string>

#include "gameplay/professions/v2/ProfessionTypes.h"

namespace gameplay
{
namespace events
{
struct ProfessionProgressedEvent
{
    std::string playerName;
    professions::ProfessionType profession;
    professions::ProfessionProgress previous;
    professions::ProfessionProgress current;

    ProfessionProgressedEvent()
        : profession(professions::ProfessionType::None), previous(), current() {}

    ProfessionProgressedEvent(const std::string& playerNameValue,
                              professions::ProfessionType professionValue,
                              const professions::ProfessionProgress& previousValue,
                              const professions::ProfessionProgress& currentValue)
        : playerName(playerNameValue), profession(professionValue),
          previous(previousValue), current(currentValue) {}
};
}
}
