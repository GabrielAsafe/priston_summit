#pragma once
// SUMMIT321: trade journal domain coordinator (no DAT, sockets, legacy rules or UI).
// NOT A LIVE ADAPTER. Durable semantics are REQUIREMENTS on ICompanionTradeJournalPort.
// No automatic acceptance or success is reported before a durable receipt exists.

#include <cstdint>
#include <vector>
#include "gameplay/companions/v2/CompanionTradeApproval.h"

namespace gameplay { namespace companions { namespace v2 {

enum class TradeParticipant { OwnerOnline, CompanionOffline };

enum class TradeApplyResult { AppliedDurably, AlreadyApplied, Conflict, IoUncertain };
enum class TradeJournalResult {
    Committed, AlreadyCommitted, Rejected, Busy, RecoveryRequired
};

// These are OPAQUE complete persistent images; the application adapter must
// produce a validated, pairwise balanced transfer using real item instances.
// Merely swapping item codes is forbidden. Never trust an image from the wire.
struct CompanionTradePrepared {
    CompanionTradeOffer offer;
    std::vector<std::uint8_t> ownerBefore, ownerAfter;
    std::vector<std::uint8_t> companionBefore, companionAfter;
};

struct CompanionTradePending {
    bool present = false;
    CompanionTradePrepared plan;
};

class ICompanionTradeJournalPort {
public:
    virtual ~ICompanionTradeJournalPort() {}
    // Pair lock must serialize ALL inventory/save/equip/logout mutations for
    // both participants until commit or recovery releases the pair.
    virtual bool LockPair(std::uint64_t owner, std::uint64_t companion) = 0;
    virtual void UnlockPair(std::uint64_t owner, std::uint64_t companion) = 0;
    // Durable receipts must survive process restart; nonce collision with a
    // different offer is a hard reject, not an idempotent success.
    virtual CompanionTradeReceipt Receipt(std::uint64_t nonce) = 0;
    // Pending intent discovery must survive restart. Reject new mutations
    // involving a pair with any unresolved intent.
    virtual CompanionTradePending PendingForPair(std::uint64_t owner,
                                                  std::uint64_t companion) = 0;
    // Prepare under lock, from AUTHENTICATED and CURRENT server inventory data;
    // verify ownership, tradeable flags, unique item identities, slot capacity,
    // source inventory revision, and item conservation. No write in Prepare.
    virtual bool Prepare(const CompanionTradeOffer&, CompanionTradePrepared&) = 0;
    // Must flush both complete before/after snapshots and offer to a durable
    // write-ahead journal BEFORE either participant can be modified.
    virtual bool SealIntent(const CompanionTradePrepared&) = 0;
    // Atomic replacement (or authoritative online mutation plus durable save),
    // with compare-before/after and fsync/FlushFileBuffers. IoUncertain means
    // the caller MUST leave the intent for recovery and block further writes.
    virtual TradeApplyResult ApplyExact(TradeParticipant participant,
                                       const CompanionTradePrepared&) = 0;
    // Receipt MUST be durable before returning true. Companion acceptance
    // becomes visible to the player only after this succeeds.
    virtual bool SealReceipt(const CompanionTradePrepared&) = 0;
    // Optional cleanup. If it fails but receipt is durable, recovery must
    // ignore this completed journal rather than overwrite later inventories.
    virtual bool RemoveIntent(const CompanionTradePrepared&) = 0;
};

class CompanionTradeJournalCoordinator final {
public:
    explicit CompanionTradeJournalCoordinator(ICompanionTradeJournalPort& port) : port_(port) {}

    TradeJournalResult Commit(const CompanionTradeOffer& offer, bool confirmedByOwner) {
        if(!confirmedByOwner || !Valid(offer)) return TradeJournalResult::Rejected;
        PairGuard guard(port_, offer.authenticatedOwner, offer.companionCharacter);
        if(!guard.acquired) return TradeJournalResult::Busy;
        const CompanionTradeReceipt receipt = port_.Receipt(offer.tradeNonce);
        if(receipt.found)
            return TradeReceiptMatches(receipt, offer)
                ? TradeJournalResult::AlreadyCommitted : TradeJournalResult::Rejected;
        if(port_.PendingForPair(offer.authenticatedOwner, offer.companionCharacter).present)
            return TradeJournalResult::RecoveryRequired;
        CompanionTradePrepared plan;
        if(!port_.Prepare(offer, plan) || !SameOffer(offer, plan.offer) ||
           !ImagesPresent(plan))
            return TradeJournalResult::Rejected;
        // Once an intent is durable, a rejection cannot be safely reported;
        // failures during either write are RECOVERY REQUIRED.
        if(!port_.SealIntent(plan)) return TradeJournalResult::RecoveryRequired;
        return Finish(plan);
    }

    // Called before allowing any participant mutations after restart.
    TradeJournalResult RecoverPair(std::uint64_t owner, std::uint64_t companion) {
        if(!owner || !companion || owner == companion) return TradeJournalResult::Rejected;
        PairGuard guard(port_, owner, companion);
        if(!guard.acquired) return TradeJournalResult::Busy;
        const CompanionTradePending pending = port_.PendingForPair(owner, companion);
        if(!pending.present) return TradeJournalResult::Rejected;
        if(!Valid(pending.plan.offer) || pending.plan.offer.authenticatedOwner != owner ||
           pending.plan.offer.companionCharacter != companion || !ImagesPresent(pending.plan))
            return TradeJournalResult::RecoveryRequired;
        const CompanionTradeReceipt receipt = port_.Receipt(pending.plan.offer.tradeNonce);
        if(receipt.found) {
            if(!TradeReceiptMatches(receipt, pending.plan.offer))
                return TradeJournalResult::RecoveryRequired;
            // Receipt means finalization was durable. Never replay old snapshots
            // over potential later transactions; cleanup only.
            (void)port_.RemoveIntent(pending.plan);
            return TradeJournalResult::AlreadyCommitted;
        }
        return Finish(pending.plan);
    }

private:
    class PairGuard {
    public:
        ICompanionTradeJournalPort& port;
        std::uint64_t owner, companion;
        bool acquired;
        PairGuard(ICompanionTradeJournalPort& p, std::uint64_t o, std::uint64_t c)
            : port(p), owner(o), companion(c), acquired(p.LockPair(o,c)) {}
        ~PairGuard() { if(acquired) port.UnlockPair(owner, companion); }
        PairGuard(const PairGuard&) = delete;
        PairGuard& operator=(const PairGuard&) = delete;
    };
    static bool Valid(const CompanionTradeOffer& o) {
        return o.tradeNonce && o.authenticatedOwner && o.companionCharacter &&
               o.authenticatedOwner != o.companionCharacter && o.offerDigest && o.offerRevision;
    }
    static bool ImagesPresent(const CompanionTradePrepared& p) {
        return !p.ownerBefore.empty() && !p.ownerAfter.empty() &&
               !p.companionBefore.empty() && !p.companionAfter.empty();
    }
    TradeJournalResult Finish(const CompanionTradePrepared& p) {
        const TradeApplyResult offline = port_.ApplyExact(TradeParticipant::CompanionOffline, p);
        if(offline != TradeApplyResult::AppliedDurably &&
           offline != TradeApplyResult::AlreadyApplied)
            return TradeJournalResult::RecoveryRequired;
        const TradeApplyResult online = port_.ApplyExact(TradeParticipant::OwnerOnline, p);
        if(online != TradeApplyResult::AppliedDurably &&
           online != TradeApplyResult::AlreadyApplied)
            return TradeJournalResult::RecoveryRequired;
        if(!port_.SealReceipt(p)) return TradeJournalResult::RecoveryRequired;
        (void)port_.RemoveIntent(p);
        return TradeJournalResult::Committed;
    }
    ICompanionTradeJournalPort& port_;
};

}}} // gameplay::companions::v2
