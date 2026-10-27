#pragma once
// SUMMIT: exact, durable replacement of ONE opaque legacy character record.
// Production integration MUST provide authenticated paths, a full-pair lock,
// a sealed write-ahead journal, and serialization of all online mutations.
// This type neither acquires game-session locks nor enables live trade.
#include "gameplay/companions/v2/CompanionTradeJournal.h"
#include <cstdint>
#include <string>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {
class CompanionTradeExactRecordStore final {
public:
    // Image limits intentionally generous enough for the legacy record but
    // bounded before reads or allocations. Never give paths from packets.
    static constexpr std::size_t kMaximumRecordBytes = 8u*1024u*1024u;
    // Caller MUST own the exclusive, cross-participant lease continuously,
    // including both ApplyExact calls and any journal recovery.
    static TradeApplyResult Replace(const std::string& trustedPath,
        const std::vector<std::uint8_t>& exactBefore,
        const std::vector<std::uint8_t>& exactAfter);
    // Read verifies regular-file semantics, disallows symlink/hardlink aliasing,
    // and refuses overlong/truncated records. Paths are trusted server data.
    static bool Read(const std::string& trustedPath,
        std::vector<std::uint8_t>& output);
};
}}}
