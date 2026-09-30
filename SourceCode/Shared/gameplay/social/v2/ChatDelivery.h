#pragma once

#include <cstddef>
#include <string>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace social
{
enum class ChatDeliveryKind
{
    Alert,
    Whisper
};

enum class ChatDeliveryCode
{
    Delivered,
    PlayerNotFound,
    EmptyMessage,
    TransportRejected
};

struct ChatMessage
{
    std::string text;
    int color;
    ChatDeliveryKind kind;

    ChatMessage(const std::string& textValue = std::string(),
                int colorValue = 0,
                ChatDeliveryKind kindValue = ChatDeliveryKind::Alert)
        : text(textValue), color(colorValue), kind(kindValue)
    {
    }
};

struct ChatDeliveryResult
{
    ChatDeliveryCode code;
    std::size_t attempted;
    std::size_t delivered;

    ChatDeliveryResult()
        : code(ChatDeliveryCode::EmptyMessage), attempted(0u), delivered(0u)
    {
    }

    bool Delivered() const { return code == ChatDeliveryCode::Delivered; }
};

class IChatDeliveryService
{
public:
    virtual ~IChatDeliveryService() {}
    virtual ChatDeliveryResult SendToPlayer(PlayerId player, const ChatMessage& message) = 0;
    virtual ChatDeliveryResult Broadcast(const ChatMessage& message) = 0;
};
}
}
