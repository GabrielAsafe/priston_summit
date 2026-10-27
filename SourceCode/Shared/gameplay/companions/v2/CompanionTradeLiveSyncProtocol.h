#pragma once
// SUMMIT development-only: coordinator contract for one live player / offline
// Companion exchange. This is NOT a permission to mutate DATs or inventories.
// Network peers are NOT trusted for authoritative record contents. Fresh save
// acceptance and a compare-and-swap journal receipt are mandatory.
// No IO, no sockets, no globally held locks, no data writes.
#include <array>
#include <cstdint>
#include <cstring>
#include <set>
#include <string>

namespace gameplay { namespace companions { namespace v2 {

struct TradeSyncIdentity {
    std::uint32_t playerSerial = 0;
    std::uint32_t connectGeneration = 0;
    std::uint64_t requestId = 0;
    std::uint64_t challenge = 0; // cryptographically unpredictable, assigned by SERVER
    bool Valid() const { return playerSerial && requestId && challenge; }
    bool operator==(const TradeSyncIdentity& b) const {
        return playerSerial == b.playerSerial && connectGeneration == b.connectGeneration &&
               requestId == b.requestId && challenge == b.challenge;
    }
};

struct TradeSyncPairKey {
    std::string account, player, companion;
    bool Valid() const {
        return !account.empty() && !player.empty() && !companion.empty() &&
            player != companion && account.size() < 64 &&
            player.size() < 32 && companion.size() < 32;
    }
    bool operator==(const TradeSyncPairKey& b) const {
        return account == b.account && player == b.player && companion == b.companion;
    }
};

using TradeSyncDigest = std::array<std::uint8_t, 32>; // SHA-256; computed from EXACT record image
inline bool ValidTradeSyncDigest(const TradeSyncDigest& d) {
    std::uint8_t observed = 0;
    for (const auto v : d) observed |= v;
    return observed != 0;
}

struct TradeSyncRecordProof {
    TradeSyncIdentity identity{};
    std::uint64_t saveSequence = 0; // server-side accepted complete save sequence
    std::uint64_t ledgerRevision = 0;
    TradeSyncDigest recordSha256{};
    TradeSyncDigest identitySha256{}; // canonical item identities calculated by server
    bool Valid() const { return identity.Valid() && saveSequence && ledgerRevision &&
        ValidTradeSyncDigest(recordSha256) && ValidTradeSyncDigest(identitySha256); }
};

struct TradeSyncDurableReceipt {
    TradeSyncIdentity identity{};
    TradeSyncPairKey pair{};
    std::uint64_t journalTxnId = 0;
    TradeSyncDigest beforeSha256{};
    TradeSyncDigest afterSha256{};
    TradeSyncDigest companionAfterSha256{};
    // Canonical count/bag-split/encoded-item SHA-256 from the durable player
    // after-image. Full record SHA may change on the next normal save because
    // SaveTime, position, EXP etc. can legitimately differ.
    TradeSyncDigest playerInventoryAfterSha256{};
    bool Valid() const { return identity.Valid() && pair.Valid() && journalTxnId &&
        ValidTradeSyncDigest(beforeSha256) && ValidTradeSyncDigest(afterSha256) &&
        ValidTradeSyncDigest(companionAfterSha256) &&
        ValidTradeSyncDigest(playerInventoryAfterSha256); }
};

enum class TradeSyncPhase : std::uint8_t {
    Empty, AwaitingClientQuiesce, AwaitingAcceptedSave, AwaitingServerCapture,
    AwaitingDurableCommit, AwaitingClientApply, Complete, Aborted, RecoveryRequired
};
enum class TradeSyncCode : std::uint8_t {
    Ok, WrongState, WrongSession, Expired, InvalidProof, MismatchedRecord,
    ReplayOrConflict, RecoveryRequired
};

class TradeSyncExchange final {
public:
    TradeSyncExchange() = default;
    TradeSyncPhase Phase() const { return phase_; }
    const TradeSyncIdentity& Identity() const { return id_; }
    const TradeSyncPairKey& Pair() const { return pair_; }
    bool HasCommittedReceipt() const { return durable_; }
    // Caller uses a monotonic clock; overflow and expiry fail closed. A
    // process restart must recover journal state before beginning a new trade.
    TradeSyncCode Begin(TradeSyncIdentity id, TradeSyncPairKey pair,
                        std::uint64_t now, std::uint64_t expiresAt,
                        std::uint64_t latestAcceptedSequence) {
        if (phase_ != TradeSyncPhase::Empty && phase_ != TradeSyncPhase::Complete &&
            phase_ != TradeSyncPhase::Aborted) return TradeSyncCode::WrongState;
        if (!id.Valid() || !pair.Valid() || !now || expiresAt <= now ||
            expiresAt - now > 60000 || !latestAcceptedSequence)
            return TradeSyncCode::InvalidProof;
        // Bounded in-process replay defense; after restart, durable journal
        // recovery MUST check persisted requests before accepting new ones.
        if (used_.count(NonceKey{id.playerSerial, id.connectGeneration,
                                 id.requestId, id.challenge}))
            return TradeSyncCode::ReplayOrConflict;
        if (used_.size() >= 4096) return TradeSyncCode::ReplayOrConflict;
        used_.insert(NonceKey{id.playerSerial, id.connectGeneration,
                              id.requestId, id.challenge});
        id_ = id;
        pair_ = std::move(pair);
        startedWithSequence_ = latestAcceptedSequence;
        deadline_ = expiresAt;
        accepted_ = TradeSyncRecordProof{};
        receipt_ = TradeSyncDurableReceipt{};
        durable_ = false;
        phase_ = TradeSyncPhase::AwaitingClientQuiesce;
        return TradeSyncCode::Ok;
    }
    // Quiesce ACK alone is not authentication of a current inventory. The
    // server must observe a NEW, validated save after this acknowledgement.
    TradeSyncCode ClientQuiesced(TradeSyncIdentity id, std::uint64_t now) {
        TradeSyncCode checked = Check(id, now);
        if (checked != TradeSyncCode::Ok) return checked;
        if (phase_ != TradeSyncPhase::AwaitingClientQuiesce) return TradeSyncCode::WrongState;
        phase_ = TradeSyncPhase::AwaitingAcceptedSave;
        return TradeSyncCode::Ok;
    }
    TradeSyncCode AcceptedSave(const TradeSyncRecordProof& proof, std::uint64_t now) {
        TradeSyncCode checked = Check(proof.identity, now);
        if (checked != TradeSyncCode::Ok) return checked;
        if (phase_ != TradeSyncPhase::AwaitingAcceptedSave) return TradeSyncCode::WrongState;
        if (!proof.Valid() || proof.saveSequence <= startedWithSequence_)
            return TradeSyncCode::InvalidProof;
        accepted_ = proof;
        phase_ = TradeSyncPhase::AwaitingServerCapture;
        return TradeSyncCode::Ok;
    }
    // Must be called after capture under the *same exclusive reservation*
    // that protects the proposed durable commit. Server recomputes both hashes.
    TradeSyncCode CapturedUnderLease(TradeSyncIdentity id,
            const TradeSyncRecordProof& newlyVerified, std::uint64_t now) {
        TradeSyncCode checked = Check(id, now);
        if (checked != TradeSyncCode::Ok) return checked;
        if (phase_ != TradeSyncPhase::AwaitingServerCapture) return TradeSyncCode::WrongState;
        if (!newlyVerified.Valid() || !(newlyVerified.identity == accepted_.identity) ||
            newlyVerified.saveSequence != accepted_.saveSequence ||
            newlyVerified.ledgerRevision != accepted_.ledgerRevision ||
            newlyVerified.recordSha256 != accepted_.recordSha256 ||
            newlyVerified.identitySha256 != accepted_.identitySha256)
            return TradeSyncCode::MismatchedRecord;
        phase_ = TradeSyncPhase::AwaitingDurableCommit;
        return TradeSyncCode::Ok;
    }
    // ONLY a verified, durable journal receipt from the server's backend may
    // call this. Never call based on a client acknowledgement or UI response.
    TradeSyncCode JournalCommitted(TradeSyncIdentity id,
            const TradeSyncDurableReceipt& receipt, std::uint64_t now) {
        TradeSyncCode checked = Check(id, now);
        if (checked != TradeSyncCode::Ok) return checked;
        if (phase_ != TradeSyncPhase::AwaitingDurableCommit) return TradeSyncCode::WrongState;
        if (!receipt.Valid() || !(receipt.identity == id) ||
            !(receipt.pair == pair_) ||
            receipt.beforeSha256 != accepted_.recordSha256)
            return TradeSyncCode::InvalidProof;
        receipt_ = receipt;
        durable_ = true;
        phase_ = TradeSyncPhase::AwaitingClientApply;
        return TradeSyncCode::Ok;
    }
    // Client acknowledgement alone has NO authority. The SERVER must compute
    // the canonical inventory digest from a NEW authenticated accepted save
    // before invoking ClientApplied. Never use client-supplied digest directly.
    // The journal remains the sole authority on committed ownership.
    TradeSyncCode ClientApplied(TradeSyncIdentity id, const TradeSyncDigest& serverVerifiedInventoryAfter,
                                std::uint64_t journalTxnId, std::uint64_t now) {
        TradeSyncCode checked = Check(id, now);
        if (checked != TradeSyncCode::Ok) return checked;
        if (phase_ != TradeSyncPhase::AwaitingClientApply) return TradeSyncCode::WrongState;
        if (!durable_ || journalTxnId != receipt_.journalTxnId ||
            serverVerifiedInventoryAfter != receipt_.playerInventoryAfterSha256)
            return TradeSyncCode::MismatchedRecord;
        phase_ = TradeSyncPhase::Complete;
        return TradeSyncCode::Ok;
    }
    // After a disconnect or process restart, the durable journal is consulted
    // FIRST. This only rebinds delivery to a newly authenticated session;
    // never invokes the durable commit a second time. Caller MUST independently
    // verify journal authenticity, account/character ownership and complete
    // before/after images. A client-supplied receipt is never accepted here.
    TradeSyncCode ReattachCommittedFromJournal(
            TradeSyncIdentity newAuthenticatedSession, TradeSyncPairKey authenticatedPair,
            const TradeSyncDurableReceipt& journalReceipt,
            std::uint64_t now, std::uint64_t expiresAt)
    {
        if (!newAuthenticatedSession.Valid() || !authenticatedPair.Valid() ||
            !journalReceipt.Valid() || !(authenticatedPair == journalReceipt.pair) ||
            !now || expiresAt <= now || expiresAt - now > 60000)
            return TradeSyncCode::InvalidProof;
        if (phase_ != TradeSyncPhase::Empty &&
            phase_ != TradeSyncPhase::RecoveryRequired)
            return TradeSyncCode::WrongState;
        if (durable_ && (receipt_.journalTxnId != journalReceipt.journalTxnId ||
            // A reconnect can have a new authenticated session identity.
            // The durable journal transaction and exact after-images, not the
            // old socket-generation, bind redelivery to the same trade.
            receipt_.beforeSha256 != journalReceipt.beforeSha256 ||
            receipt_.afterSha256 != journalReceipt.afterSha256 ||
            receipt_.companionAfterSha256 != journalReceipt.companionAfterSha256 ||
            receipt_.playerInventoryAfterSha256 != journalReceipt.playerInventoryAfterSha256 ||
            !(pair_ == authenticatedPair)))
            return TradeSyncCode::ReplayOrConflict;
        const NonceKey key{newAuthenticatedSession.playerSerial,
             newAuthenticatedSession.connectGeneration,
             newAuthenticatedSession.requestId, newAuthenticatedSession.challenge};
        if (used_.count(key) || used_.size() >= 4096)
            return TradeSyncCode::ReplayOrConflict;
        used_.insert(key);
        id_ = newAuthenticatedSession;
        pair_ = std::move(authenticatedPair);
        receipt_ = journalReceipt;
        deadline_ = expiresAt;
        durable_ = true;
        phase_ = TradeSyncPhase::AwaitingClientApply;
        return TradeSyncCode::Ok;
    }
    // Pending uncommitted trades can be abandoned. Committed trades MUST be
    // recovered, reconciled and delivered; no rollback based on a timeout.
    TradeSyncCode Disconnect(TradeSyncIdentity id) {
        if (!(id == id_)) return TradeSyncCode::WrongSession;
        if (phase_ == TradeSyncPhase::Complete || phase_ == TradeSyncPhase::Aborted ||
            phase_ == TradeSyncPhase::Empty) return TradeSyncCode::WrongState;
        phase_ = durable_ ? TradeSyncPhase::RecoveryRequired : TradeSyncPhase::Aborted;
        return durable_ ? TradeSyncCode::RecoveryRequired : TradeSyncCode::Ok;
    }
    // Passing time must never silently release an already-durable trade.
    TradeSyncCode AdvanceClock(std::uint64_t now) {
        if (!now || !deadline_ || now <= deadline_) return TradeSyncCode::Ok;
        if (phase_ == TradeSyncPhase::Complete || phase_ == TradeSyncPhase::Aborted ||
            phase_ == TradeSyncPhase::Empty) return TradeSyncCode::Ok;
        phase_ = durable_ ? TradeSyncPhase::RecoveryRequired : TradeSyncPhase::Aborted;
        return durable_ ? TradeSyncCode::RecoveryRequired : TradeSyncCode::Expired;
    }
private:
    TradeSyncCode Check(TradeSyncIdentity id, std::uint64_t now) {
        if (!(id == id_)) return TradeSyncCode::WrongSession;
        if (!now || !deadline_ || now > deadline_) {
            (void)AdvanceClock(now ? now : deadline_ + 1);
            return durable_ ? TradeSyncCode::RecoveryRequired : TradeSyncCode::Expired;
        }
        if (phase_ == TradeSyncPhase::RecoveryRequired) return TradeSyncCode::RecoveryRequired;
        return TradeSyncCode::Ok;
    }
    struct NonceKey {
        std::uint32_t serial, generation;
        std::uint64_t requestId, nonce;
        bool operator<(const NonceKey& other) const {
            if(serial != other.serial) return serial < other.serial;
            if(generation != other.generation) return generation < other.generation;
            if(requestId != other.requestId) return requestId < other.requestId;
            return nonce < other.nonce;
        }
    };
    TradeSyncIdentity id_{};
    TradeSyncPairKey pair_{};
    std::set<NonceKey> used_;
    std::uint64_t deadline_ = 0, startedWithSequence_ = 0;
    TradeSyncRecordProof accepted_{};
    TradeSyncDurableReceipt receipt_{};
    bool durable_ = false;
    TradeSyncPhase phase_ = TradeSyncPhase::Empty;
};

}}} // namespace gameplay::companions::v2
