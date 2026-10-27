#pragma once
// SUMMIT 350D: builds the *proposed* exact owner DAT image from an accepted
// legacy save buffer without mutating the original, and binds it by SHA-256.
// NOT a proof of disk commit, authenticated save receipt, or live session.
// The caller MUST independently hold the appropriate world/session lock and
// SaveActivityFence lease; it must supply the actual allocation size from the
// server allocation site (never infer allocation size from record->size), a
// server-validated save sequence, and a trusted potion/force normalizer.
//
// The future worker may receive the resulting OWNED bytes by value, but MUST
// still check the existing owner DAT + companion DAT under an exclusive pair
// lease, prepare exact images, seal an intent and recover before login.
#include "gameplay/companions/v2/CompanionTradeSha256.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {

struct TradeLegacyRecordLayout350D {
    std::size_t declaredSizeOffset = 0;
    std::size_t minimumWriteBytes = 0;
    std::size_t gameSaveOffset = 0;
    std::size_t gameSaveBytes = 0;
    std::size_t inventoryEndOffset = 0;
};

struct TradeNormalizedRecord350D {
    std::vector<std::uint8_t> bytes;
    TradeSha256Digest digest{};
    std::size_t declaredBytes = 0;
    std::size_t exactWriteBytes = 0;
};

enum class TradeRecordNormalizeCode350D {
    Ready, MissingSource, UnprovenCapacity, InvalidLayout, InvalidDeclaredSize,
    FailedServerNormalization, InventoryModified
};

// The normalizer must only update the provided game-save region. The live
// `rsSaveServerPotion`/`rsSaveServerForce` adapters are server-only functions;
// no client-supplied values are accepted as replacement fields here.
// Failure does not modify output. This operation intentionally performs no IO.
template<class TrustedNormalizeGameSave>
TradeRecordNormalizeCode350D BuildNormalizedRecord350D(
    const std::uint8_t* acknowledgedRecord,
    std::size_t independentlyKnownAllocationBytes,
    const TradeLegacyRecordLayout350D& layout,
    TrustedNormalizeGameSave normalize,
    TradeNormalizedRecord350D& output)
{
    const std::size_t maxBytes = 8u * 1024u * 1024u;
    if(!acknowledgedRecord) return TradeRecordNormalizeCode350D::MissingSource;
    if(independentlyKnownAllocationBytes < 4 ||
       independentlyKnownAllocationBytes > maxBytes)
        return TradeRecordNormalizeCode350D::UnprovenCapacity;
    if(layout.declaredSizeOffset > independentlyKnownAllocationBytes-4 ||
       layout.minimumWriteBytes < 4 ||
       layout.minimumWriteBytes > independentlyKnownAllocationBytes ||
       !layout.gameSaveBytes ||
       layout.gameSaveOffset > independentlyKnownAllocationBytes ||
       layout.gameSaveBytes > independentlyKnownAllocationBytes-layout.gameSaveOffset ||
       layout.inventoryEndOffset > independentlyKnownAllocationBytes ||
       layout.gameSaveOffset + layout.gameSaveBytes > layout.minimumWriteBytes)
        return TradeRecordNormalizeCode350D::InvalidLayout;

    // Read exactly four bytes; never dereference an unaligned legacy int.
    std::int32_t declared = 0;
    std::memcpy(&declared, acknowledgedRecord+layout.declaredSizeOffset, 4);
    if(declared <= 0 || static_cast<std::size_t>(declared) > independentlyKnownAllocationBytes ||
       static_cast<std::size_t>(declared) < layout.inventoryEndOffset ||
       static_cast<std::size_t>(declared) < layout.gameSaveOffset+layout.gameSaveBytes)
        return TradeRecordNormalizeCode350D::InvalidDeclaredSize;
    const std::size_t exactBytes = std::max(static_cast<std::size_t>(declared),
                                            layout.minimumWriteBytes);
    // The legacy writer persists max(record->size, srRECORD_DEFAULT_SIZE).
    // This number is NOT the same thing as the bytes allocated for the record.
    if(exactBytes > independentlyKnownAllocationBytes)
        return TradeRecordNormalizeCode350D::UnprovenCapacity;
    TradeNormalizedRecord350D candidate;
    candidate.declaredBytes = static_cast<std::size_t>(declared);
    candidate.exactWriteBytes = exactBytes;
    candidate.bytes.assign(acknowledgedRecord, acknowledgedRecord+exactBytes);
    std::vector<std::uint8_t> original = candidate.bytes;
    // Ensure the callback receives only an owned copy, never the live record.
    if(!normalize(candidate.bytes.data()+layout.gameSaveOffset,
                  layout.gameSaveBytes))
        return TradeRecordNormalizeCode350D::FailedServerNormalization;
    // If a future normalizer accidentally clobbers inventory, size, equipment
    // or any other field outside GameSaveInfo, refuse to use its image.
    for(std::size_t i=0;i<exactBytes;++i) {
        if(i>=layout.gameSaveOffset &&
           i<layout.gameSaveOffset+layout.gameSaveBytes) continue;
        if(candidate.bytes[i]!=original[i])
            return TradeRecordNormalizeCode350D::InventoryModified;
    }
    candidate.digest = TradeSha256::Hash(candidate.bytes.data(),candidate.bytes.size());
    output = std::move(candidate);
    return TradeRecordNormalizeCode350D::Ready;
}

}}} // namespace gameplay::companions::v2
