#pragma once

namespace gameplay
{
namespace combat
{
class DeathAttributionPolicy
{
public:
    static bool ShouldApplyDeathExperiencePenalty(bool attackerIsUser, bool pkModeAttackerIsUser)
    {
        return !attackerIsUser && !pkModeAttackerIsUser;
    }

    static bool ShouldRecordPlayerKill(bool attackerIsUser, bool pkModeAttackerIsUser)
    {
        return attackerIsUser || pkModeAttackerIsUser;
    }
};
}
}
