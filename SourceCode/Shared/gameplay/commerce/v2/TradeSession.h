#pragma once

#include <cstdint>

#include "gameplay/commerce/v2/TradeService.h"

namespace gameplay
{
namespace commerce
{
class ITradeReplayGuard
{
public:
    virtual ~ITradeReplayGuard() {}

    virtual TradeCommitGuardResult Check(
        PlayerHandle sender,
        const TradeCommitFingerprint& fingerprint,
        std::uint32_t nowMs) const = 0;

    virtual void MarkCommitted(
        PlayerHandle sender,
        const TradeCommitFingerprint& fingerprint,
        std::uint32_t nowMs) = 0;

    virtual std::size_t Clear(PlayerHandle sender) = 0;
};
}
}
