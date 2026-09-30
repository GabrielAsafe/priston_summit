#pragma once

#include <cstdint>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace social
{
namespace clan
{
struct ClanId
{
    std::uint32_t value;
    explicit constexpr ClanId(std::uint32_t valueValue = 0u) : value(valueValue) {}
    constexpr bool Valid() const { return value != 0u; }
};

inline constexpr bool operator==(ClanId lhs, ClanId rhs) { return lhs.value == rhs.value; }
inline constexpr bool operator!=(ClanId lhs, ClanId rhs) { return !(lhs == rhs); }

struct ClanMembershipSnapshot
{
    PlayerId player;
    ClanId clan;
    std::int32_t mark;

    ClanMembershipSnapshot() : player(), clan(), mark(0) {}
    bool IsMember() const { return player.Valid() && clan.Valid(); }
};

class IClanQuery
{
public:
    virtual ~IClanQuery() {}
    virtual bool TryGetMembership(PlayerId player, ClanMembershipSnapshot& out) const = 0;
};

inline bool SameClan(const ClanMembershipSnapshot& lhs, const ClanMembershipSnapshot& rhs)
{
    return lhs.IsMember() && rhs.IsMember() && lhs.clan == rhs.clan;
}
}
}
}
