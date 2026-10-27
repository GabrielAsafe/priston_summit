#pragma once
// SUMMIT323: strictly read-only, full-identity source inventory snapshot.
// No trade is possible from this snapshot alone. It does not authorize writes.
// DAT decoding and account authorization live in the server legacy adapter.
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {

// Same category rule used by rsTradeInvenItem: the legacy player trade
// processes 0x04xxxxxx potion/stack records by quantity, not by instance key.
// Companion transfers do NOT move these records yet. They may coexist with
// an eligible uniquely identified item and must be preserved byte-for-byte.
inline bool IsLegacyPotionStack350N(std::uint32_t itemCode) {
    return (itemCode & UINT32_C(0xff000000)) == UINT32_C(0x04000000);
}
// 350O: legacy DATs can contain records without a complete instance ID.
// They are PASSIVE: preserve their original bytes and bag, but NEVER select
// them as a transferable instance. No item ID may be synthesized.
inline bool HasCompleteTradeIdentity350O(std::uint32_t code,
        std::uint32_t key, std::uint32_t checksum) {
    return code != 0 && key != 0 && checksum != 0 &&
           !IsLegacyPotionStack350N(code);
}
inline bool IsPassiveTradeRecord350O(std::uint32_t code,
        std::uint32_t key, std::uint32_t checksum) {
    return IsLegacyPotionStack350N(code) || !key || !checksum;
}

struct CompanionTradeItemRecord {
    std::uint32_t recordIndex = 0;
    std::uint32_t itemCode = 0;
    std::uint32_t itemKey = 0;
    std::uint32_t itemChecksum = 0;
    std::int32_t position = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
    // Full original encoded record, NOT just an item code; never send to clients.
    std::vector<std::uint8_t> encodedRecord;
    bool inSecondaryBag = false;
};

struct CompanionTradeInventorySnapshot {
    std::vector<CompanionTradeItemRecord> records;
    std::uint32_t primaryCount = 0;
    std::uint32_t secondaryCount = 0;
    bool hasMissingInstanceIdentity = false;
    bool hasDuplicateInstanceIdentity = false;
    bool hasCursorItem = false;
    bool hasUnknownPosition = false;
    bool ambiguousSegments = false;
    bool TradeEligible() const {
        // Missing IDs are permitted only as immutable passive records.
        // The move planner separately REJECTS selecting them and verifies
        // that their encoded bytes remain unchanged in the same participant.
        return !hasDuplicateInstanceIdentity && !hasCursorItem &&
               !hasUnknownPosition && !ambiguousSegments;
    }
};

enum class CompanionTradeInventoryReadCode {
    Ok, InvalidEnvelope, InvalidItemCount, TruncatedRecord,
    CodecRejected, MissingItemCode, ExtraTrailingData
};

// No dependency on legacy structs: decoded item fields are supplied by a
// server-only codec. Never silently skip records or coalesce duplicate items.
struct CompanionTradeDecodedItem {
    std::uint32_t code = 0, key = 0, checksum = 0;
    std::int32_t position = 0, x = 0, y = 0;
};

template<class Decode>
CompanionTradeInventoryReadCode ReadCompanionTradeInventory(
    const std::uint8_t* data, std::size_t dataBytes, int itemCount,
    int primaryCount, int maximumItems, Decode decode,
    CompanionTradeInventorySnapshot& out)
{
    if (itemCount < 0 || primaryCount < 0 || primaryCount > itemCount ||
        maximumItems <= 0 || itemCount > maximumItems)
        return CompanionTradeInventoryReadCode::InvalidItemCount;
    if ((!data && dataBytes) || (itemCount == 0 && dataBytes != 0) ||
        (itemCount != 0 && (!data || dataBytes == 0)))
        return CompanionTradeInventoryReadCode::InvalidEnvelope;

    CompanionTradeInventorySnapshot candidate;
    // The canonical legacy save writes all A records, sets ItemSubStart,
    // then writes all B records.  ItemSubStart==0 is a normal and unambiguous
    // state: A is empty, and every record belongs to B.  Rejecting it breaks
    // trade after players move their entire Companion inventory to B.
    candidate.ambiguousSegments = false;
    candidate.primaryCount = static_cast<std::uint32_t>(primaryCount);
    candidate.secondaryCount = static_cast<std::uint32_t>(itemCount - primaryCount);
    candidate.records.reserve(static_cast<std::size_t>(itemCount));

    std::size_t offset = 0;
    for (int i = 0; i < itemCount; ++i) {
        if (dataBytes - offset < sizeof(std::int32_t))
            return CompanionTradeInventoryReadCode::TruncatedRecord;
        std::int32_t encodedSize = 0;
        std::memcpy(&encodedSize, data + offset, sizeof(encodedSize));
        if (encodedSize <= static_cast<std::int32_t>(sizeof(encodedSize)) ||
            static_cast<std::size_t>(encodedSize) > dataBytes - offset)
            return CompanionTradeInventoryReadCode::TruncatedRecord;

        CompanionTradeDecodedItem decoded;
        if (!decode(data + offset, static_cast<std::size_t>(encodedSize), decoded))
            return CompanionTradeInventoryReadCode::CodecRejected;
        if (!decoded.code)
            return CompanionTradeInventoryReadCode::MissingItemCode;

        CompanionTradeItemRecord item;
        item.recordIndex = static_cast<std::uint32_t>(i);
        item.itemCode = decoded.code;
        item.itemKey = decoded.key;
        item.itemChecksum = decoded.checksum;
        item.position = decoded.position;
        item.x = decoded.x;
        item.y = decoded.y;
        item.inSecondaryBag = i >= primaryCount;
        item.encodedRecord.assign(data + offset, data + offset + encodedSize);
        if ((!decoded.key || !decoded.checksum) &&
            !IsLegacyPotionStack350N(decoded.code))
            candidate.hasMissingInstanceIdentity = true;
        if (decoded.position == -1) candidate.hasCursorItem = true;
        if (decoded.position < -1 || decoded.position > 16)
            candidate.hasUnknownPosition = true;
        for (const auto& previous : candidate.records) {
            if (HasCompleteTradeIdentity350O(item.itemCode,item.itemKey,item.itemChecksum) &&
                previous.itemCode == item.itemCode &&
                previous.itemKey == item.itemKey &&
                previous.itemChecksum == item.itemChecksum)
                candidate.hasDuplicateInstanceIdentity = true;
        }
        candidate.records.push_back(std::move(item));
        offset += static_cast<std::size_t>(encodedSize);
    }
    if (offset != dataBytes)
        return CompanionTradeInventoryReadCode::ExtraTrailingData;
    out = std::move(candidate); // Failed reads never change caller's output.
    return CompanionTradeInventoryReadCode::Ok;
}

}}} // gameplay::companions::v2
