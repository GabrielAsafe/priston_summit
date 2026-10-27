#pragma once

// SUMMIT320: domain-only, fail-closed Companion acceptance contract.
// This is NOT the live trade transport or an offline DAT writer.
// The live adapter must provide durable, idempotent, two-inventory commit
// before this service can be invoked by the server.

#include <cstdint>

namespace gameplay { namespace companions { namespace v2 {

struct CompanionTradeOffer {
    std::uint64_t tradeNonce;
    std::uint64_t authenticatedOwner;
    std::uint64_t companionCharacter;
    std::uint64_t offerDigest;
    std::uint32_t offerRevision;
    CompanionTradeOffer()
        : tradeNonce(0), authenticatedOwner(0), companionCharacter(0),
          offerDigest(0), offerRevision(0) {}
};

inline bool SameOffer(const CompanionTradeOffer& a, const CompanionTradeOffer& b) {
    return a.tradeNonce == b.tradeNonce &&
           a.authenticatedOwner == b.authenticatedOwner &&
           a.companionCharacter == b.companionCharacter &&
           a.offerDigest == b.offerDigest &&
           a.offerRevision == b.offerRevision;
}

// These checks must be made by the server from trusted state, not the UI:
// ownership; companion runtime quiesced (summoned or dismissed); offer and inventory fingerprints;
// source/destination inventory locks; available capacity; and a durable
// recovery journal for the pair of inventories.
enum class CompanionTradePreflight {
    Ready, NotOwner, CompanionRuntimeNotQuiesced, OfferChanged,
    InventoryUnavailable, CapacityExceeded, NoDurableCommit, Busy
};

enum class CompanionTradeAtomicResult { Committed, Rejected, RecoveryRequired };

struct CompanionTradeReceipt {
    bool found;
    bool committed;
    std::uint64_t offerDigest;
    std::uint64_t authenticatedOwner;
    std::uint64_t companionCharacter;
    std::uint32_t offerRevision;
    CompanionTradeReceipt() : found(false), committed(false), offerDigest(0),
        authenticatedOwner(0), companionCharacter(0), offerRevision(0) {}
};
inline bool TradeReceiptMatches(const CompanionTradeReceipt& r,
                               const CompanionTradeOffer& o) {
    return r.found && r.committed && r.offerDigest == o.offerDigest &&
           r.authenticatedOwner == o.authenticatedOwner &&
           r.companionCharacter == o.companionCharacter &&
           r.offerRevision == o.offerRevision;
}

class ICompanionTradeAtomicPort {
public:
    virtual ~ICompanionTradeAtomicPort() {}
    // The receipt lookup MUST be durable across server restarts.
    virtual CompanionTradeReceipt FindReceipt(std::uint64_t tradeNonce) const = 0;
    virtual CompanionTradePreflight Preflight(const CompanionTradeOffer&) = 0;
    // Commit EXACTLY ONCE and durably; return RecoveryRequired if outcome uncertain.
    virtual CompanionTradeAtomicResult CommitExactlyOnce(const CompanionTradeOffer&) = 0;
};

enum class CompanionTradeAcceptCode {
    OfferReady, InvalidOffer, NoPendingOffer, OwnerConfirmationMissing,
    OfferChanged, RejectedByAuthority, RecoveryRequired, Committed,
    AlreadyCommitted
};

struct CompanionTradeAcceptResult {
    CompanionTradeAcceptCode code;
    bool companionAccepted;
    bool inventoryCommitted;
    explicit CompanionTradeAcceptResult(CompanionTradeAcceptCode c,
                                        bool accept = false, bool committed = false)
        : code(c), companionAccepted(accept), inventoryCommitted(committed) {}
};

class CompanionTradeAutoAcceptance final {
public:
    explicit CompanionTradeAutoAcceptance(ICompanionTradeAtomicPort& port)
        : port_(port), pending_(false), recoveryRequired_(false), offer_() {}

    CompanionTradeAcceptResult Begin(const CompanionTradeOffer& offer) {
        if(!Valid(offer)) return CompanionTradeAcceptResult(CompanionTradeAcceptCode::InvalidOffer);
        if(recoveryRequired_) return CompanionTradeAcceptResult(CompanionTradeAcceptCode::RecoveryRequired);
        if(pending_) return CompanionTradeAcceptResult(CompanionTradeAcceptCode::RejectedByAuthority);
        const CompanionTradeReceipt receipt = port_.FindReceipt(offer.tradeNonce);
        if(receipt.found) {
            if(TradeReceiptMatches(receipt, offer))
                return CompanionTradeAcceptResult(CompanionTradeAcceptCode::AlreadyCommitted, true, true);
            return CompanionTradeAcceptResult(CompanionTradeAcceptCode::RejectedByAuthority);
        }
        pending_ = true;
        offer_ = offer;
        return CompanionTradeAcceptResult(CompanionTradeAcceptCode::OfferReady);
    }

    // Called ONLY after the server has authenticated the owner's explicit
    // acceptance of the unchanged offer. Companion acceptance is automatic
    // as part of the same authoritative commit, never a forged legacy socket.
    CompanionTradeAcceptResult OwnerAccepted(const CompanionTradeOffer& signedOff,
                                            bool authenticatedOwnerConfirmed) {
        if(!pending_) return CompanionTradeAcceptResult(CompanionTradeAcceptCode::NoPendingOffer);
        if(recoveryRequired_) return CompanionTradeAcceptResult(CompanionTradeAcceptCode::RecoveryRequired);
        if(!authenticatedOwnerConfirmed)
            return CompanionTradeAcceptResult(CompanionTradeAcceptCode::OwnerConfirmationMissing);
        if(!SameOffer(offer_, signedOff))
            return CompanionTradeAcceptResult(CompanionTradeAcceptCode::OfferChanged);
        const CompanionTradeReceipt receipt = port_.FindReceipt(offer_.tradeNonce);
        if(receipt.found) {
            if(TradeReceiptMatches(receipt, offer_)) {
                pending_ = false;
                return CompanionTradeAcceptResult(CompanionTradeAcceptCode::AlreadyCommitted, true, true);
            }
            return CompanionTradeAcceptResult(CompanionTradeAcceptCode::RejectedByAuthority);
        }
        if(port_.Preflight(offer_) != CompanionTradePreflight::Ready)
            return CompanionTradeAcceptResult(CompanionTradeAcceptCode::RejectedByAuthority);
        const CompanionTradeAtomicResult committed = port_.CommitExactlyOnce(offer_);
        if(committed == CompanionTradeAtomicResult::RecoveryRequired) {
            recoveryRequired_ = true;
            return CompanionTradeAcceptResult(CompanionTradeAcceptCode::RecoveryRequired);
        }
        if(committed != CompanionTradeAtomicResult::Committed)
            return CompanionTradeAcceptResult(CompanionTradeAcceptCode::RejectedByAuthority);
        pending_ = false;
        return CompanionTradeAcceptResult(CompanionTradeAcceptCode::Committed, true, true);
    }

    void Cancel() { if(!recoveryRequired_) pending_ = false; }
    bool NeedsRecovery() const { return recoveryRequired_; }
    bool HasPendingOffer() const { return pending_; }

private:
    static bool Valid(const CompanionTradeOffer& o) {
        return o.tradeNonce != 0 && o.authenticatedOwner != 0 &&
               o.companionCharacter != 0 && o.authenticatedOwner != o.companionCharacter &&
               o.offerDigest != 0 && o.offerRevision != 0;
    }
    ICompanionTradeAtomicPort& port_;
    bool pending_;
    bool recoveryRequired_;
    CompanionTradeOffer offer_;
};

}}} // namespaces
