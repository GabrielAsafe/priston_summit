#pragma once
// SUMMIT321: Connects PATCH320's approval protocol to the V2 journal coordinator.
// Adapter wiring is deliberately NOT a runtime socket/character-record adapter.
#include "gameplay/companions/v2/CompanionTradeJournal.h"

namespace gameplay { namespace companions { namespace v2 {

class CompanionTradeAtomicJournalBridge final : public ICompanionTradeAtomicPort {
public:
    explicit CompanionTradeAtomicJournalBridge(ICompanionTradeJournalPort& storage)
        : storage_(storage), coordinator_(storage) {}

    CompanionTradeReceipt FindReceipt(std::uint64_t nonce) const override {
        return storage_.Receipt(nonce);
    }
    CompanionTradePreflight Preflight(const CompanionTradeOffer& offer) override {
        if(!offer.tradeNonce || !offer.authenticatedOwner || !offer.companionCharacter ||
           offer.authenticatedOwner == offer.companionCharacter || !offer.offerDigest || !offer.offerRevision)
            return CompanionTradePreflight::InventoryUnavailable;
        if(storage_.PendingForPair(offer.authenticatedOwner, offer.companionCharacter).present)
            return CompanionTradePreflight::Busy;
        // Ownership, capacity, online inventory generation, and item identity
        // remain server adapter responsibilities. They must be revalidated
        // under the pair lock when coordinator invokes Prepare.
        return CompanionTradePreflight::Ready;
    }
    CompanionTradeAtomicResult CommitExactlyOnce(const CompanionTradeOffer& offer) override {
        // Called only by CompanionTradeAutoAcceptance::OwnerAccepted AFTER
        // owner authentication; never expose this bridge directly to packets.
        const TradeJournalResult result = coordinator_.Commit(offer, true);
        if(result == TradeJournalResult::Committed ||
           result == TradeJournalResult::AlreadyCommitted)
            return CompanionTradeAtomicResult::Committed;
        if(result == TradeJournalResult::RecoveryRequired)
            return CompanionTradeAtomicResult::RecoveryRequired;
        return CompanionTradeAtomicResult::Rejected;
    }
    TradeJournalResult RecoverPair(std::uint64_t owner, std::uint64_t companion) {
        return coordinator_.RecoverPair(owner, companion);
    }
private:
    ICompanionTradeJournalPort& storage_;
    CompanionTradeJournalCoordinator coordinator_;
};

}}} // namespace gameplay::companions::v2
