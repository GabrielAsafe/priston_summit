#pragma once

#include "gameplay/classes/v2/services/ClassService.h"

namespace gameplay
{
namespace classes
{
namespace legacy
{
inline bool ResolveCriticalScalingValue(const ClassService& service,
                                        int legacyJobCode,
                                        int strength,
                                        int spirit,
                                        int talent,
                                        int dexterity,
                                        int health,
                                        int& out)
{
    CharacterAttribute attribute = CharacterAttribute::Unspecified;
    if (!service.TryCriticalScalingAttribute(legacyJobCode, attribute))
        return false;

    switch (attribute)
    {
    case CharacterAttribute::Strength: out = strength; return true;
    case CharacterAttribute::Spirit: out = spirit; return true;
    case CharacterAttribute::Talent: out = talent; return true;
    case CharacterAttribute::Dexterity: out = dexterity; return true;
    case CharacterAttribute::Health: out = health; return true;
    case CharacterAttribute::Unspecified: break;
    }
    return false;
}
}
}
}
