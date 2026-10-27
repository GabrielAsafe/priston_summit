#pragma once
// Server/client packet envelope validation. No DAT, gameplay, or I/O.
// A confirm request can only become a trade after a separate authoritative
// backend has locked both participants and durably applied BOTH records.
#include "gameplay/companions/CompanionProtocol.h"
#include <cstring>
namespace gameplay { namespace companions { namespace v2 {
inline bool ValidTradeConfirmRequest(const protocol::CompanionTradeConfirmPacket& p) {
    return p.size == static_cast<std::int32_t>(sizeof(p)) &&
        p.code == protocol::OPCODE_COMPANION_TRADE_CONFIRM &&
        p.version == protocol::COMPANION_TRADE_CONFIRM_VERSION &&
        (p.side == 0 || p.side == 1) && p.itemIndex >= 0 &&
        p.itemIndex < 4096 && p.itemCode && p.itemKey && p.itemChecksum && p.ownerRevision &&
        p.companionRevision && p.requestId && p.characterName[0] &&
        std::memchr(p.characterName, 0, sizeof(p.characterName)) != nullptr;
}
// Exact source-item match; item codes alone are not unique instances.
inline bool MatchesTradeSelectedInstance(
    const protocol::CompanionTradeConfirmPacket& request,
    std::uint32_t recordIndex, std::uint32_t code,
    std::uint32_t key, std::uint32_t checksum)
{
    return request.itemIndex >= 0 &&
           static_cast<std::uint32_t>(request.itemIndex) == recordIndex &&
           request.itemCode == code && request.itemKey == key &&
           request.itemChecksum == checksum;
}

inline bool ValidTradeResult(const protocol::CompanionTradeResultPacket& p) {
    return p.size == static_cast<std::int32_t>(sizeof(p)) &&
        p.code == protocol::OPCODE_COMPANION_TRADE_RESULT &&
        p.version == protocol::COMPANION_TRADE_CONFIRM_VERSION &&
        p.requestId && (p.status < 0 || p.status == 1 || p.status == 2 || p.status == 3) && p.characterName[0] &&
        std::memchr(p.characterName, 0, sizeof(p.characterName)) != nullptr;
}
}}}
