#pragma once

#include "gameplay/social/clan/v2/ClanChatService.h"
#include "systems/gameplay/player/v2/legacy/LegacyOnlinePlayerRegistry.h"

namespace gameplay
{
namespace social
{
namespace clan
{
namespace legacy
{
class LegacyClanChatAdapter final : public IClanChatService
{
public:
    explicit LegacyClanChatAdapter(const player::legacy::LegacyOnlinePlayerRegistry& players);
    ClanChatResult Broadcast(PlayerId sender, const std::string& text) override;

private:
    const player::legacy::LegacyOnlinePlayerRegistry& players_;
};

LegacyClanChatAdapter& ServerLegacyClanChatAdapter();
}
}
}
}
