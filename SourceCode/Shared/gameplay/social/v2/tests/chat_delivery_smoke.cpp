#include "gameplay/social/v2/ChatDelivery.h"

#include <cassert>
#include <iostream>

namespace
{
class FakeChatDelivery final : public gameplay::social::IChatDeliveryService
{
public:
    gameplay::social::ChatDeliveryResult SendToPlayer(
        gameplay::PlayerId player,
        const gameplay::social::ChatMessage& message) override
    {
        gameplay::social::ChatDeliveryResult result;
        result.attempted = 1u;
        result.delivered = player.Valid() && !message.text.empty() ? 1u : 0u;
        result.code = result.delivered
                          ? gameplay::social::ChatDeliveryCode::Delivered
                          : gameplay::social::ChatDeliveryCode::PlayerNotFound;
        return result;
    }

    gameplay::social::ChatDeliveryResult Broadcast(
        const gameplay::social::ChatMessage& message) override
    {
        gameplay::social::ChatDeliveryResult result;
        result.attempted = 3u;
        result.delivered = message.text.empty() ? 0u : 3u;
        result.code = result.delivered
                          ? gameplay::social::ChatDeliveryCode::Delivered
                          : gameplay::social::ChatDeliveryCode::EmptyMessage;
        return result;
    }
};
}

int main()
{
    using namespace gameplay;
    using namespace gameplay::social;

    FakeChatDelivery delivery;
    ChatDeliveryResult one = delivery.SendToPlayer(
        PlayerId(7u), ChatMessage("hello", 2, ChatDeliveryKind::Alert));
    assert(one.Delivered());
    assert(one.attempted == 1u && one.delivered == 1u);

    ChatDeliveryResult all = delivery.Broadcast(
        ChatMessage("notice", 4, ChatDeliveryKind::Whisper));
    assert(all.Delivered());
    assert(all.delivered == 3u);

    std::cout << "chat delivery smoke: PASS\n";
    return 0;
}
