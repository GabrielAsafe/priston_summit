#pragma once

#include <cstdint>

namespace authentication::v2
{
enum class DuplicateLoginDecision
{
    NoConflict = 0,
    RejectActiveSession,
    RetireStaleAndRejectNewSession
};

struct DuplicateLoginContext
{
    bool sameRuntimeIdentity = false;
    bool sameAccount = false;
    std::uint32_t existingLastReceiveTime = 0u;
    std::uint32_t now = 0u;
};

class DuplicateLoginPolicy final
{
public:
    static constexpr std::uint32_t StaleAfterMilliseconds = 60u * 1000u;

    static DuplicateLoginDecision Evaluate(const DuplicateLoginContext& context) noexcept
    {
        if(!context.sameRuntimeIdentity || !context.sameAccount)
            return DuplicateLoginDecision::NoConflict;

        // Preserve the legacy unsigned-tick comparison instead of converting
        // this migration into a clock/wraparound behavior change.
        if((context.existingLastReceiveTime + StaleAfterMilliseconds) < context.now)
            return DuplicateLoginDecision::RetireStaleAndRejectNewSession;

        return DuplicateLoginDecision::RejectActiveSession;
    }
};
}
