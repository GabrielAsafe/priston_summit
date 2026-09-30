#pragma once

#include <cstddef>
#include <string>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace social
{
namespace clan
{
struct ClanChatResult
{
    bool senderIsMember;
    std::size_t attempted;
    std::size_t delivered;

    ClanChatResult() : senderIsMember(false), attempted(0u), delivered(0u) {}
};

class IClanChatService
{
public:
    virtual ~IClanChatService() {}
    virtual ClanChatResult Broadcast(PlayerId sender, const std::string& text) = 0;
};
}
}
}
