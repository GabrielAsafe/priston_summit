#include "systems/social/clan/v2/legacy/LegacyClanChatAdapter.h"

#include <cstdio>

#include "gameplay/social/clan/v2/ClanQuery.h"
#include "gameplay/social/v2/ChatDelivery.h"
#include "systems/community/chat/v2/legacy/LegacyChatDeliveryAdapter.h"
#include "systems/gameplay/player/v2/legacy/LegacyPlayerAdapter.h"
#include "systems/social/clan/v2/legacy/LegacyClanQueryAdapter.h"

namespace gameplay
{
namespace social
{
namespace clan
{
namespace legacy
{
LegacyClanChatAdapter::LegacyClanChatAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players)
    : players_(players)
{
}

ClanChatResult LegacyClanChatAdapter::Broadcast(PlayerId sender, const std::string& text)
{
    ClanChatResult result;
    if(!sender.Valid() || text.empty())
        return result;

    ClanMembershipSnapshot senderClan;
    if(!ServerLegacyClanQueryAdapter().TryGetMembership(sender, senderClan) || !senderClan.IsMember())
        return result;
    result.senderIsMember = true;

    player::PlayerSnapshot senderPlayer;
    if(!player::legacy::ServerLegacyPlayerAdapter().TryGet(sender, senderPlayer))
        return result;

    const std::string formatted = senderPlayer.characterName + "> " + text;
    const ChatMessage message(formatted, 5, ChatDeliveryKind::Whisper);
    const std::vector<PlayerHandle> online = players_.OnlineHandles();
    for(std::vector<PlayerHandle>::const_iterator it = online.begin(); it != online.end(); ++it)
    {
        ClanMembershipSnapshot recipientClan;
        if(!ServerLegacyClanQueryAdapter().TryGetMembership(it->id, recipientClan) ||
           !SameClan(senderClan, recipientClan))
            continue;

        ++result.attempted;
        if(::gameplay::social::legacy::ServerLegacyChatDeliveryAdapter().SendToPlayer(it->id, message).Delivered())
            ++result.delivered;
    }
    return result;
}

LegacyClanChatAdapter& ServerLegacyClanChatAdapter()
{
    static LegacyClanChatAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
}
