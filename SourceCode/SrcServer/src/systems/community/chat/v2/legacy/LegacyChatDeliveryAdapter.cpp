#include "systems/community/chat/v2/legacy/LegacyChatDeliveryAdapter.h"

#include "systems/network/protocol/server/opcodes.h"

#include <cstdio>

#include "systems/network/transport/legacy/smwsock.h"
#include "systems/rendering/legacy_3d/smd3d.h"
#include "systems/gameplay/character/core/character.h"
#include "core/server/legacy_runtime/onserver.h"

namespace gameplay
{
namespace social
{
namespace legacy
{
namespace
{
bool BuildPacket(const ChatMessage& message, TRANS_CHATMESSAGE& packet)
{
    if(message.text.empty())
        return false;

    std::snprintf(packet.szMessage, sizeof(packet.szMessage), "%s", message.text.c_str());
    packet.code = message.kind == ChatDeliveryKind::Alert
                      ? OpCode::SEND_ALERT
                      : OpCode::OPCODE_WHISPERMESSAGE;
    packet.size = 32 + lstrlen(packet.szMessage);
    packet.dwIP = static_cast<DWORD>(message.color);
    packet.dwObjectSerial = 0;
    return true;
}
}

LegacyChatDeliveryAdapter::LegacyChatDeliveryAdapter(
    const player::legacy::LegacyOnlinePlayerRegistry& registry)
    : registry_(registry)
{
}

ChatDeliveryResult LegacyChatDeliveryAdapter::SendToPlayer(PlayerId player, const ChatMessage& message)
{
    return SendToLegacyPlayer(registry_.Resolve(player), message);
}

ChatDeliveryResult LegacyChatDeliveryAdapter::SendToLegacyPlayer(rsPLAYINFO* legacyPlayer,
                                                                  const ChatMessage& message)
{
    ChatDeliveryResult result;
    if(message.text.empty())
    {
        result.code = ChatDeliveryCode::EmptyMessage;
        return result;
    }

    if(!legacyPlayer || !legacyPlayer->lpsmSock)
    {
        result.code = ChatDeliveryCode::PlayerNotFound;
        return result;
    }

    TRANS_CHATMESSAGE packet = {};
    if(!BuildPacket(message, packet))
    {
        result.code = ChatDeliveryCode::EmptyMessage;
        return result;
    }

    result.attempted = 1u;
    if(legacyPlayer->lpsmSock->Send((char*)&packet, packet.size, TRUE) == TRUE)
    {
        result.delivered = 1u;
        result.code = ChatDeliveryCode::Delivered;
    }
    else
    {
        result.code = ChatDeliveryCode::TransportRejected;
    }
    return result;
}

ChatDeliveryResult LegacyChatDeliveryAdapter::Broadcast(const ChatMessage& message)
{
    ChatDeliveryResult result;
    if(message.text.empty())
    {
        result.code = ChatDeliveryCode::EmptyMessage;
        return result;
    }

    TRANS_CHATMESSAGE packet = {};
    if(!BuildPacket(message, packet))
    {
        result.code = ChatDeliveryCode::EmptyMessage;
        return result;
    }

    for(int index = 0; index < CONNECTMAX; ++index)
    {
        rsPLAYINFO* legacyPlayer = &rsPlayInfo[index];
        if(!legacyPlayer->lpsmSock)
            continue;

        ++result.attempted;
        if(legacyPlayer->lpsmSock->Send((char*)&packet, packet.size, TRUE) == TRUE)
            ++result.delivered;
    }

    result.code = result.delivered > 0u || result.attempted == 0u
                      ? ChatDeliveryCode::Delivered
                      : ChatDeliveryCode::TransportRejected;
    return result;
}

LegacyChatDeliveryAdapter& ServerLegacyChatDeliveryAdapter()
{
    static LegacyChatDeliveryAdapter adapter(player::legacy::ServerLegacyOnlinePlayerRegistry());
    return adapter;
}
}
}
}
