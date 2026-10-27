#pragma once
#include <cstdint>
namespace gameplay { namespace companions { namespace v2 {
// Normal client admission MUST reject an open cursor/legacy inventory owner
// before freezing the bags; after a durable commit, retry only while sealed.
inline bool TradeLiveCanStart350X(bool cursor, bool trade, bool warehouse) {
    return !cursor && !trade && !warehouse;
}
inline bool TradeLiveRetryDue350X(std::uint32_t now,
                                  std::uint32_t previous,
                                  std::uint32_t interval) {
    return previous==0 || static_cast<std::uint32_t>(now-previous)>=interval;
}
}}}
