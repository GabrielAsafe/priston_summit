#pragma once
// Development-only, NOT linked to the live client yet. A fail-closed gate for
// delivering an *already committed* two-record trade to a running client.
// The caller MUST authenticate the transport and independently verify a real
// durable journal receipt. No method here can move items or approve a trade.
// The apply callback must stage/validate INVENTORY ONLY and commit atomically;
// NEVER invoke the legacy full-character ResotrRecordData mid-game.
#include "gameplay/companions/v2/CompanionTradeLiveSyncProtocol.h"
#include "gameplay/companions/v2/CompanionTradeBufferedPayloadCandidate.h"
#include "gameplay/companions/v2/CompanionTradeSha256.h"
#include "gameplay/companions/v2/CompanionTradeCanonicalInventoryDigest.h"
#include <vector>
#include <cstdint>
#include <cstring>
#include <utility>

namespace gameplay { namespace companions { namespace v2 {
enum class TradeDeliveryState : std::uint8_t {
    Empty, Prepared, AwaitingFreshSave, Acknowledged, RecoveryRequired
};
enum class TradeDeliveryResult : std::uint8_t {
    Ready, WrongState, InvalidReceipt, WrongSession, WrongPair,
    StaleClient, PayloadTampered, InvalidInventory, UnrelatedFieldsChanged,
    ApplyFailed, SaveMismatch, RecoveryRequired, Acknowledged
};

class TradeClientDeliveryGate final {
public:
    TradeDeliveryState State() const { return state_; }
    static bool ComputeInventoryDigest(const std::vector<std::uint8_t>& image,
        const BufferedTradeEnvelopeLayout& layout, TradeSha256Digest& out) {
        return TradeCanonicalInventoryDigest::Compute(image,layout,out);
    }
    // Layout contract is for a complete TRANS_RECORD_DATA image. We enforce
    // byte-for-byte preservation of non-inventory fields; the sizes/counts and
    // bounded item-data region are the only permitted changes.
    template <class Decode>
    TradeDeliveryResult Prepare(const TradeSyncIdentity& authenticatedSession,
        const TradeSyncPairKey& authenticatedPair,
        const TradeSyncDurableReceipt& serverVerifiedJournalReceipt,
        const std::vector<std::uint8_t>& locallyFrozenRecord,
        const std::vector<std::uint8_t>& durablePlayerAfterRecord,
        const BufferedTradeEnvelopeLayout& layout,
        const char* localCharacterName, int itemLimit, Decode decode)
    {
        if (state_ != TradeDeliveryState::Empty) return TradeDeliveryResult::WrongState;
        if (!serverVerifiedJournalReceipt.Valid()) return TradeDeliveryResult::InvalidReceipt;
        if (!(serverVerifiedJournalReceipt.identity == authenticatedSession) ||
            !authenticatedSession.Valid()) return TradeDeliveryResult::WrongSession;
        if (!(serverVerifiedJournalReceipt.pair == authenticatedPair) ||
            !authenticatedPair.Valid() || !localCharacterName ||
            authenticatedPair.player != localCharacterName) return TradeDeliveryResult::WrongPair;
        if (locallyFrozenRecord.size() < layout.dataOffset ||
            durablePlayerAfterRecord.size() < layout.dataOffset ||
            locallyFrozenRecord.size() > layout.maxBytes ||
            durablePlayerAfterRecord.size() > layout.maxBytes)
            return TradeDeliveryResult::PayloadTampered;
        if (TradeSha256::Hash(locallyFrozenRecord.data(),locallyFrozenRecord.size()) !=
                serverVerifiedJournalReceipt.beforeSha256)
            return TradeDeliveryResult::StaleClient;
        if (TradeSha256::Hash(durablePlayerAfterRecord.data(),durablePlayerAfterRecord.size()) !=
                serverVerifiedJournalReceipt.afterSha256)
            return TradeDeliveryResult::PayloadTampered;
        CompanionTradeInventorySnapshot pre, post;
        if (BufferedTradePayloadPolicy::DecodeBounded(locallyFrozenRecord.data(),
             locallyFrozenRecord.size(),layout,localCharacterName,itemLimit,decode,pre) !=
                BufferedTradeCaptureCode::CandidateReadOnly || !pre.TradeEligible() ||
            BufferedTradePayloadPolicy::DecodeBounded(durablePlayerAfterRecord.data(),
             durablePlayerAfterRecord.size(),layout,localCharacterName,itemLimit,decode,post) !=
                BufferedTradeCaptureCode::CandidateReadOnly || !post.TradeEligible())
            return TradeDeliveryResult::InvalidInventory;
        if (!UnrelatedFieldsIdentical(locallyFrozenRecord,durablePlayerAfterRecord,layout))
            return TradeDeliveryResult::UnrelatedFieldsChanged;
        TradeSha256Digest inventoryHash{};
        if (!TradeCanonicalInventoryDigest::Compute(durablePlayerAfterRecord,layout,inventoryHash))
            return TradeDeliveryResult::InvalidInventory;
        if (inventoryHash != serverVerifiedJournalReceipt.playerInventoryAfterSha256)
            return TradeDeliveryResult::PayloadTampered;
        delivery_ = durablePlayerAfterRecord;
        inventoryHash_ = inventoryHash;
        session_ = authenticatedSession;
        journal_ = serverVerifiedJournalReceipt.journalTxnId;
        state_ = TradeDeliveryState::Prepared;
        return TradeDeliveryResult::Ready;
    }

    // Callback is trusted client implementation, not executable code sent by
    // server. If it fails, state is QUARANTINED; no blind retry or rollback.
    template <class AtomicInventoryOnlyApply>
    TradeDeliveryResult Apply(AtomicInventoryOnlyApply atomicApply) {
        if (state_ != TradeDeliveryState::Prepared) return TradeDeliveryResult::WrongState;
        if (!atomicApply(delivery_)) {
            state_=TradeDeliveryState::RecoveryRequired;
            return TradeDeliveryResult::ApplyFailed;
        }
        state_=TradeDeliveryState::AwaitingFreshSave;
        return TradeDeliveryResult::Ready;
    }

    // An acknowledgement is only sent after another fresh, validated save of
    // the CLIENT inventory. Non-inventory fields (XP, location, SaveTime) may
    // legitimately differ; compare the canonical item payload exactly.
    TradeDeliveryResult ValidateFreshSave(const TradeSyncIdentity& authenticatedSession,
        std::uint64_t journalTxnId, const std::vector<std::uint8_t>& freshSave,
        const BufferedTradeEnvelopeLayout& layout) {
        if (state_ == TradeDeliveryState::RecoveryRequired)
            return TradeDeliveryResult::RecoveryRequired;
        if (state_ != TradeDeliveryState::AwaitingFreshSave)
            return TradeDeliveryResult::WrongState;
        if (!(authenticatedSession == session_)) return TradeDeliveryResult::WrongSession;
        if (journalTxnId != journal_) return TradeDeliveryResult::InvalidReceipt;
        TradeSha256Digest observed{};
        if (!TradeCanonicalInventoryDigest::Compute(freshSave,layout,observed) || observed!=inventoryHash_)
            return TradeDeliveryResult::SaveMismatch;
        state_=TradeDeliveryState::Acknowledged;
        return TradeDeliveryResult::Acknowledged;
    }
private:
    static bool UnrelatedFieldsIdentical(const std::vector<std::uint8_t>& a,
        const std::vector<std::uint8_t>& b,const BufferedTradeEnvelopeLayout& layout) {
        // Compare common fixed header bytes except four 32-bit length/count
        // fields. Never permit item-only refresh to modify XP/stat fields.
        if (a.size()!=b.size() || layout.dataOffset>a.size()) return false;
        for(std::size_t i=0;i<layout.dataOffset;++i) {
            bool modified=false;
            const std::size_t fields[]={layout.sizeOffset,layout.itemCountOffset,
                                       layout.primaryOffset,layout.dataSizeOffset};
            for(const auto start : fields) if (start<=i && i-start<4) modified=true;
            if (!modified && a[i]!=b[i]) return false;
        }
        // The fixed layout has no non-item fields after the item-data array.
        // The caller must supply maxBytes and a layout covering exactly that
        // region. Any extra bytes are compared to reject arbitrary mutation.
        return true;
    }
    TradeDeliveryState state_=TradeDeliveryState::Empty;
    TradeSyncIdentity session_{};
    std::uint64_t journal_=0;
    TradeSha256Digest inventoryHash_{};
    std::vector<std::uint8_t> delivery_;
};
}}}
