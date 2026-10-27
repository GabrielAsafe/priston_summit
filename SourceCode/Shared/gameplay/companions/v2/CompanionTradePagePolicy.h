#pragma once
// SUMMIT trade UI: validates wire-only previews. No item payloads or mutation.
#include "gameplay/companions/CompanionProtocol.h"
#include <cstddef>
#include <cstdint>

namespace gameplay { namespace companions { namespace v2 {

inline bool ValidTradePreviewPage(const protocol::CompanionTradePagePacket& p)
{
    if(p.size != static_cast<std::int32_t>(sizeof(p)) ||
       p.code != protocol::OPCODE_COMPANION_TRADE_PAGE ||
       p.version != protocol::COMPANION_TRADE_PAGE_VERSION ||
       p.side < 0 || p.side > 1 || p.page < 0 || p.page > 1000 ||
       p.totalItems < 0 || p.totalItems > 4096 ||
       p.itemCount < 0 || p.itemCount > protocol::COMPANION_TRADE_PAGE_SIZE)
        return false;
    if(p.status != 1)
        return p.itemCount == 0 && p.totalItems == 0;
    const std::int64_t start =
        static_cast<std::int64_t>(p.page) * protocol::COMPANION_TRADE_PAGE_SIZE;
    if((p.totalItems == 0 && (p.page != 0 || p.itemCount != 0)) ||
       (p.totalItems > 0 && (start >= p.totalItems ||
        p.itemCount != static_cast<int>(
             ((p.totalItems - start) >= protocol::COMPANION_TRADE_PAGE_SIZE)
             ? protocol::COMPANION_TRADE_PAGE_SIZE : (p.totalItems - start)))))
        return false;
    for(int i = 0; i < p.itemCount; ++i)
    {
        // Anonymous legacy records may be displayed as non-selectable rows.
        // They MUST NOT be treated as transferable instances.
        if(p.items[i].index < 0 || !p.items[i].itemCode) return false;
        for(int j = 0; j < i; ++j)
            if(p.items[i].index == p.items[j].index) return false;
    }
    return true;
}

}}} // namespace gameplay::companions::v2
