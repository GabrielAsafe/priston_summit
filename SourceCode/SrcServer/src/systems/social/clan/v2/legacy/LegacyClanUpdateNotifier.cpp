#include "systems/social/clan/v2/legacy/LegacyClanUpdateNotifier.h"

#include <vector>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"
#include "systems/network/protocol/server/opcodes.h"
#include "systems/network/v2/legacy/LegacyPacketSender.h"

rsPLAYINFO* FindUserFromCodeName(char* szName, DWORD dwSpeedSum);

namespace gameplay
{
namespace social
{
namespace clan
{
namespace legacy
{
LegacyClanUpdateNotifier::LegacyClanUpdateNotifier(
    const player::legacy::LegacyOnlinePlayerRegistry& players,
    network::IPacketSender& packets)
    : players_(players), packets_(packets)
{
}

ClanUpdateNotifyResult LegacyClanUpdateNotifier::Notify(
    PlayerId source, const std::vector<ClanUpdateRecipient>& recipients)
{
    ClanUpdateNotifyResult result;
    if(!source.Valid())
        return result;

    smTRANS_COMMAND packet = {};
    packet.code = OpCode::OPCODE_CLAN_UPDATE;
    packet.size = sizeof(smTRANS_COMMAND);
    packet.WParam = static_cast<DWORD>(source.value);

    for(std::vector<ClanUpdateRecipient>::const_iterator it = recipients.begin(); it != recipients.end(); ++it)
    {
        if(it->characterName.empty())
            continue;
        ++result.attempted;
        std::vector<char> mutableName(it->characterName.begin(), it->characterName.end());
        mutableName.push_back('\0');
        rsPLAYINFO* target = FindUserFromCodeName(mutableName.data(), static_cast<DWORD>(it->verification));
        const PlayerId targetId = players_.IdOf(target);
        if(!targetId.Valid())
            continue;
        if(packets_.Send(targetId, network::PacketView(&packet, sizeof(packet), packet.code)).Succeeded())
            ++result.delivered;
    }
    return result;
}

LegacyClanUpdateNotifier& ServerLegacyClanUpdateNotifier()
{
    static LegacyClanUpdateNotifier notifier(
        player::legacy::ServerLegacyOnlinePlayerRegistry(),
        network::legacy::ServerLegacyPacketSender());
    return notifier;
}
}
}
}
}
