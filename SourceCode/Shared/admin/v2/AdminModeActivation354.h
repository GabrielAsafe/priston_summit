#pragma once

namespace gameplay { namespace admin {
// Authentication is authoritative. Repeating /ativar must never silently
// revoke a correctly authenticated GM session (SUMMIT354).
struct AdminModeActivation354
{
    int effectiveLevel;
    bool authorized;
    bool alreadyActive;
};

inline AdminModeActivation354 ResolveAdminModeActivation354(
    int verifiedLevel, int currentLevel, int playerLevel, int maxAdminLevel) noexcept
{
    if (verifiedLevel <= playerLevel || verifiedLevel > maxAdminLevel)
        return {playerLevel, false, false};
    return {verifiedLevel, true, currentLevel == verifiedLevel};
}

}} // namespace gameplay::admin
