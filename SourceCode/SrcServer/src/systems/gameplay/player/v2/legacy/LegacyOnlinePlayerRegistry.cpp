#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

#include <cctype>

// Legacy character.h is not self-contained. Preserve the server include order
// at the AMBER boundary; no GREEN header sees these dependencies.
#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace player
{
namespace legacy
{
namespace
{
bool EqualsIgnoreCase(const char* lhs, const std::string& rhs)
{
    if (!lhs)
        return false;

    std::size_t index = 0u;
    for (; lhs[index] && index < rhs.size(); ++index)
    {
        const unsigned char a = static_cast<unsigned char>(lhs[index]);
        const unsigned char b = static_cast<unsigned char>(rhs[index]);
        if (std::tolower(a) != std::tolower(b))
            return false;
    }
    return lhs[index] == 0 && index == rhs.size();
}

bool IsLive(const rsPLAYINFO* player)
{
    return player && player->lpsmSock && player->dwObjectSerial != 0u;
}
}

PlayerId LegacyOnlinePlayerRegistry::IdOf(const rsPLAYINFO* player) const
{
    return IsLive(player) ? PlayerId(static_cast<std::uint32_t>(player->dwObjectSerial)) : PlayerId();
}

PlayerHandle LegacyOnlinePlayerRegistry::HandleOf(const rsPLAYINFO* player) const
{
    if (!IsLive(player))
        return PlayerHandle();
    return PlayerHandle(PlayerId(static_cast<std::uint32_t>(player->dwObjectSerial)),
                        static_cast<std::uint32_t>(player->dwConnectCount));
}

rsPLAYINFO* LegacyOnlinePlayerRegistry::Resolve(PlayerId player) const
{
    if (!player.Valid())
        return 0;
    rsPLAYINFO* resolved = srFindUserFromSerial(static_cast<DWORD>(player.value));
    return IsLive(resolved) ? resolved : 0;
}

rsPLAYINFO* LegacyOnlinePlayerRegistry::Resolve(PlayerHandle player) const
{
    rsPLAYINFO* resolved = Resolve(player.id);
    if (!resolved || static_cast<std::uint32_t>(resolved->dwConnectCount) != player.generation)
        return 0;
    return resolved;
}

rsPLAYINFO* LegacyOnlinePlayerRegistry::ResolveByPlayerKey(const std::string& playerKey) const
{
    if (playerKey.empty())
        return 0;

    for (int i = 0; i < CONNECTMAX; ++i)
    {
        rsPLAYINFO* player = &rsPlayInfo[i];
        if (!IsLive(player))
            continue;
        if (EqualsIgnoreCase(player->getName(), playerKey))
            return player;
    }
    return 0;
}

std::vector<PlayerHandle> LegacyOnlinePlayerRegistry::OnlineHandles() const
{
    std::vector<PlayerHandle> handles;
    handles.reserve(CONNECTMAX);
    for (int i = 0; i < CONNECTMAX; ++i)
    {
        const PlayerHandle handle = HandleOf(&rsPlayInfo[i]);
        if (handle.Valid())
            handles.push_back(handle);
    }
    return handles;
}

bool LegacyOnlinePlayerRegistry::IsOnline(PlayerId player) const
{
    return Resolve(player) != 0;
}

bool LegacyOnlinePlayerRegistry::IsCurrent(PlayerHandle player) const
{
    return Resolve(player) != 0;
}

const LegacyOnlinePlayerRegistry& ServerLegacyOnlinePlayerRegistry()
{
    static LegacyOnlinePlayerRegistry registry;
    return registry;
}
}
}
}
