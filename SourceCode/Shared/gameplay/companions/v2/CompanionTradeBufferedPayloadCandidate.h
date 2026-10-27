#pragma once
// SUMMIT326: bounded, read-only decoding of a staged legacy save envelope.
// A staged save originates with the CLIENT and may be stale: neither a decoded
// payload nor an exact online-identity match permits a player/Companion trade.
// Input MUST be a stable, already-copied, server-local byte buffer. NEVER pass
// an unsynchronized live rsPLAYINFO::lpRecordDataBuff pointer into this API.
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

#include "gameplay/companions/v2/CompanionTradeInventorySnapshot.h"
#include "gameplay/companions/v2/CompanionTradeOnlineDatReconciliation.h"

namespace gameplay { namespace companions { namespace v2 {

enum class BufferedTradeCaptureCode {
    InvalidEnvelope, CharacterMismatch, MalformedItems, UnsafeInventory,
    OnlineChanged, IdentityMismatch, CandidateReadOnly
};

struct BufferedTradeEnvelopeLayout {
    std::size_t sizeOffset, itemCountOffset, primaryOffset, dataSizeOffset;
    std::size_t nameOffset, nameBytes, dataOffset, maxBytes;
    BufferedTradeEnvelopeLayout(std::size_t sizeAt, std::size_t countAt,
        std::size_t primaryAt, std::size_t dataSizeAt,
        std::size_t nameAt, std::size_t nameLen, std::size_t dataAt,
        std::size_t maximum)
        : sizeOffset(sizeAt), itemCountOffset(countAt), primaryOffset(primaryAt),
          dataSizeOffset(dataSizeAt), nameOffset(nameAt), nameBytes(nameLen),
          dataOffset(dataAt), maxBytes(maximum) {}
};

struct BufferedTradeCapture {
    BufferedTradeCaptureCode code = BufferedTradeCaptureCode::InvalidEnvelope;
    std::uint64_t onlineRevision = 0;
    std::uint32_t observedItems = 0;
    // Full encoded records are a READ-ONLY candidate, not authenticated state.
    CompanionTradeInventorySnapshot candidate;
    bool HasFullEncodedCandidate() const {
        return code == BufferedTradeCaptureCode::CandidateReadOnly;
    }
    bool HasAuthenticatedLivePayloads() const { return false; }
    bool CanCommitTransfer() const { return false; }
};

struct BufferedTradePayloadPolicy {
    static bool FieldWithin(std::size_t position, std::size_t length,
                            std::size_t available) {
        return position <= available && length <= available - position;
    }
    static bool ReadI32(const std::uint8_t* raw, std::size_t length,
                        std::size_t offset, std::int32_t& value) {
        if (!FieldWithin(offset, sizeof(value), length)) return false;
        std::memcpy(&value, raw + offset, sizeof(value));
        return true;
    }
    template <class Decode>
    static BufferedTradeCaptureCode DecodeBounded(
        const std::uint8_t* bytes, std::size_t availableBytes,
        const BufferedTradeEnvelopeLayout& layout, const char* expectedCharacter,
        int maximumItems, Decode decode, CompanionTradeInventorySnapshot& out,
        CompanionTradeInventorySnapshot* unsafeDetails350N=nullptr)
    {
        if (!bytes || !expectedCharacter || !*expectedCharacter ||
            availableBytes == 0 || availableBytes > layout.maxBytes ||
            layout.dataOffset > availableBytes || layout.maxBytes == 0 ||
            layout.nameBytes < 2 ||
            !FieldWithin(layout.nameOffset, layout.nameBytes, layout.dataOffset))
            return BufferedTradeCaptureCode::InvalidEnvelope;
        std::int32_t recordBytes = 0, itemCount = 0, primary = 0, dataBytes = 0;
        if (!ReadI32(bytes, layout.dataOffset, layout.sizeOffset, recordBytes) ||
            !ReadI32(bytes, layout.dataOffset, layout.itemCountOffset, itemCount) ||
            !ReadI32(bytes, layout.dataOffset, layout.primaryOffset, primary) ||
            !ReadI32(bytes, layout.dataOffset, layout.dataSizeOffset, dataBytes) ||
            recordBytes < 0 || dataBytes < 0 ||
            static_cast<std::size_t>(recordBytes) > availableBytes ||
            static_cast<std::size_t>(recordBytes) < layout.dataOffset ||
            static_cast<std::size_t>(dataBytes) >
                static_cast<std::size_t>(recordBytes) - layout.dataOffset)
            return BufferedTradeCaptureCode::InvalidEnvelope;
        const char* name = reinterpret_cast<const char*>(bytes + layout.nameOffset);
        const void* terminator = std::memchr(name, 0, layout.nameBytes);
        if (!terminator) return BufferedTradeCaptureCode::CharacterMismatch;
        const std::size_t nameLength = static_cast<const char*>(terminator) - name;
        const std::size_t trustedLength = std::strlen(expectedCharacter);
        if (!nameLength || nameLength != trustedLength ||
            std::memcmp(name, expectedCharacter, nameLength) != 0)
            return BufferedTradeCaptureCode::CharacterMismatch;
        CompanionTradeInventorySnapshot decoded;
        if (ReadCompanionTradeInventory(bytes + layout.dataOffset,
                static_cast<std::size_t>(dataBytes), itemCount, primary,
                maximumItems, decode, decoded) != CompanionTradeInventoryReadCode::Ok)
            return BufferedTradeCaptureCode::MalformedItems;
        if (!decoded.TradeEligible()) {
            if(unsafeDetails350N) *unsafeDetails350N=decoded;
            return BufferedTradeCaptureCode::UnsafeInventory;
        }
        out = std::move(decoded); // Caller output unchanged on failure.
        return BufferedTradeCaptureCode::CandidateReadOnly;
    }

    static BufferedTradeCapture CheckIdentity(
        const OnlineTradeIdentitySnapshot& before,
        const OnlineTradeIdentitySnapshot& after,
        const CompanionTradeInventorySnapshot& candidate,
        std::uint32_t capacity)
    {
        BufferedTradeCapture result;
        if (!OnlineTradeIdentityPolicy::ExactlyMatches(before, after)) {
            result.code = BufferedTradeCaptureCode::OnlineChanged;
            return result;
        }
        const OnlineTradePersistedCheck compared =
            OnlineTradePersistedPolicy::Compare(after, candidate, capacity);
        if (compared.code == OnlineTradePersistedCode::PersistedUnsafe) {
            result.code = BufferedTradeCaptureCode::UnsafeInventory;
            return result;
        }
        if (compared.code != OnlineTradePersistedCode::MatchReadOnly) {
            result.code = BufferedTradeCaptureCode::IdentityMismatch;
            return result;
        }
        result.code = BufferedTradeCaptureCode::CandidateReadOnly;
        result.onlineRevision = after.revision;
        result.observedItems = compared.itemsCompared;
        result.candidate = candidate;
        return result;
    }
};

}}} // gameplay::companions::v2
