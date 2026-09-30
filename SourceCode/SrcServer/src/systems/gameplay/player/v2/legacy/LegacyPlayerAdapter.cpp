#include "systems/gameplay/player/v2/legacy/LegacyPlayerAdapter.h"

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
LegacyPlayerAdapter::LegacyPlayerAdapter(const LegacyOnlinePlayerRegistry& registry)
    : registry_(registry)
{
}

bool LegacyPlayerAdapter::Translate(const rsPLAYINFO* player, PlayerSnapshot& out) const
{
    const PlayerHandle handle = registry_.HandleOf(player);
    if (!handle.Valid())
        return false;

    PlayerSnapshot snapshot;
    snapshot.handle = handle;
    snapshot.playerKey = player->getName() ? player->getName() : "";
    snapshot.accountKey = player->getAccount() ? player->getAccount() : "";
    snapshot.characterName = snapshot.playerKey;
    snapshot.level = player->smCharInfo.Level;
    snapshot.legacyJobCode = static_cast<int>(player->smCharInfo.JOB_CODE);
    snapshot.experience = static_cast<std::int64_t>(player->m_Experience);
    snapshot.gold = static_cast<std::int64_t>(player->ServerMoney);
    snapshot.position = WorldPosition(MapId(static_cast<std::uint32_t>(player->Position.Area)),
                                      player->Position.x,
                                      player->Position.y,
                                      player->Position.z);
    snapshot.online = true;
    snapshot.alive = player->smCharInfo.Life[0] > 0;
    snapshot.hasCombatTarget = player->lpChrTarget != 0;
    out = snapshot;
    return true;
}

bool LegacyPlayerAdapter::TranslateCharacter(const rsPLAYINFO* player, CharacterSnapshot& out) const
{
    const PlayerId id = registry_.IdOf(player);
    if (!id.Valid())
        return false;

    CharacterSnapshot snapshot;
    snapshot.playerId = id;
    snapshot.playerKey = player->getName() ? player->getName() : "";
    snapshot.name = snapshot.playerKey;
    snapshot.level = player->smCharInfo.Level;
    snapshot.legacyJobCode = static_cast<int>(player->smCharInfo.JOB_CODE);
    snapshot.life = player->smCharInfo.Life[0];
    snapshot.maxLife = player->smCharInfo.Life[1];
    snapshot.mana = player->smCharInfo.Mana[0];
    snapshot.maxMana = player->smCharInfo.Mana[1];
    out = snapshot;
    return true;
}

bool LegacyPlayerAdapter::TryGet(PlayerId player, PlayerSnapshot& out) const
{
    return Translate(registry_.Resolve(player), out);
}

bool LegacyPlayerAdapter::TryGet(PlayerHandle player, PlayerSnapshot& out) const
{
    return Translate(registry_.Resolve(player), out);
}

bool LegacyPlayerAdapter::TryGetByKey(const std::string& playerKey, PlayerSnapshot& out) const
{
    return Translate(registry_.ResolveByPlayerKey(playerKey), out);
}

bool LegacyPlayerAdapter::IsOnline(PlayerId player) const
{
    return registry_.IsOnline(player);
}

bool LegacyPlayerAdapter::TryGetCharacter(PlayerId player, CharacterSnapshot& out) const
{
    return TranslateCharacter(registry_.Resolve(player), out);
}

bool LegacyPlayerAdapter::TryGetLegacy(const rsPLAYINFO* player, PlayerSnapshot& out) const
{
    return Translate(player, out);
}

const LegacyPlayerAdapter& ServerLegacyPlayerAdapter()
{
    static LegacyPlayerAdapter adapter(ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
