#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "gameplay/core/v2/GameplayIds.h"

namespace gameplay
{
namespace social
{
namespace clan
{
struct ClanUpdateRecipient
{
    std::string characterName;
    std::uint32_t verification;

    ClanUpdateRecipient(const std::string& name = std::string(), std::uint32_t token = 0u)
        : characterName(name), verification(token) {}
};

struct ClanUpdateNotifyResult
{
    std::size_t attempted;
    std::size_t delivered;
    ClanUpdateNotifyResult() : attempted(0u), delivered(0u) {}
};

class IClanUpdateNotifier
{
public:
    virtual ~IClanUpdateNotifier() {}
    virtual ClanUpdateNotifyResult Notify(PlayerId source,
                                          const std::vector<ClanUpdateRecipient>& recipients) = 0;
};
}
}
}
