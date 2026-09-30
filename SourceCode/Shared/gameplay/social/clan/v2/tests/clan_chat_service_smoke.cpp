#include <cassert>
#include "gameplay/social/clan/v2/ClanChatService.h"

namespace
{
class FakeClanChat final : public gameplay::social::clan::IClanChatService
{
public:
    gameplay::social::clan::ClanChatResult Broadcast(gameplay::PlayerId sender, const std::string& text) override
    {
        gameplay::social::clan::ClanChatResult result;
        result.senderIsMember = sender.Valid();
        if(result.senderIsMember && !text.empty()) { result.attempted = 2u; result.delivered = 2u; }
        return result;
    }
};
}
int main()
{
    FakeClanChat chat;
    gameplay::social::clan::ClanChatResult result = chat.Broadcast(gameplay::PlayerId(7u), "hello");
    assert(result.senderIsMember && result.delivered == 2u);
    return 0;
}
