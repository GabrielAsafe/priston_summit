#pragma once
// SUMMIT 350B: journal-based worker orchestration, testable without game state.
// This does NOT by itself activate trades. The runtime factory must install
// a snapshot-only backend that also owns BOTH character-write/login gates.
#include "gameplay/companions/v2/CompanionTradeDetachedWorker.h"
#include "gameplay/companions/v2/CompanionTradeJournal.h"
#include <memory>
namespace gameplay { namespace companions { namespace v2 {

class IDetachedJournalContext {
public:
    virtual ~IDetachedJournalContext() {}
    virtual ICompanionTradeJournalPort& Port() = 0;
    virtual bool Healthy() const = 0;
};
class IDetachedJournalContextFactory {
public:
    virtual ~IDetachedJournalContextFactory() {}
    // Returns a self-contained context with ownership of storage backend,
    // full pair lease and journal. Must NEVER reference a live player/socket.
    virtual std::unique_ptr<IDetachedJournalContext> Open(const DetachedTradeJob&) = 0;
};

class DetachedJournalExecutor350B final : public IDetachedTradeExecutor {
public:
    explicit DetachedJournalExecutor350B(IDetachedJournalContextFactory& factory)
        : factory_(factory) {}
    DetachedTradeOutcome Execute(const DetachedTradeJob& job) override {
        if(!job.session.Valid() || !job.tradeNonce || !job.leaseToken ||
           !job.acceptedSaveSequence || job.accountName.empty() ||
           job.ownerName.empty() || job.companionName.empty() ||
           job.ownerName==job.companionName || job.acceptedOwnerRecord.empty() ||
           !ValidTradeConfirmRequest(job.request))
            return DetachedTradeOutcome::Refused(-76);
        // Once the context has opened a journal or touched the filesystem,
        // exceptions and ambiguous errors MUST retain the participant gate.
        auto ctx=factory_.Open(job);
        if(!ctx || !ctx->Healthy()) return DetachedTradeOutcome::Unknown();
        CompanionTradeJournalCoordinator coordinator(ctx->Port());
        const auto pending=ctx->Port().PendingForPair(job.session.owner,
                                                       job.session.companion);
        if(!ctx->Healthy()) return DetachedTradeOutcome::Unknown();
        TradeJournalResult result=TradeJournalResult::Rejected;
        if(pending.present) {
            // An unrelated pending trade MUST be recovered for safety, but
            // must never be acknowledged as the current request's success.
            const bool sameNonce=pending.plan.offer.tradeNonce==job.tradeNonce;
            result=coordinator.RecoverPair(job.session.owner,job.session.companion);
            if(result==TradeJournalResult::Committed ||
               result==TradeJournalResult::AlreadyCommitted)
                return sameNonce ? DetachedTradeOutcome::Committed(true) :
                    DetachedTradeOutcome::Unknown();
            // The intent existed on disk: even a rejected/failed recovery must
            // quarantine the pair, not release it as a normal refusal.
            return DetachedTradeOutcome::Unknown();
        } else {
            CompanionTradeOffer offer;
            offer.authenticatedOwner=job.session.owner;
            offer.companionCharacter=job.session.companion;
            offer.tradeNonce=job.tradeNonce;
            // This digest is an authenticated-request identity, NOT proof of
            // the actual item data; Prepare() must verify both records.
            auto mix=[](std::uint64_t h,std::uint64_t v) {
                for(int n=0;n<8;++n){h^=(v>>(8*n))&255u;h*=UINT64_C(1099511628211);} return h;
            };
            offer.offerDigest=mix(mix(mix(job.request.ownerRevision,
                    job.request.companionRevision),job.request.itemCode),
                    (static_cast<std::uint64_t>(job.request.itemKey)<<32)|job.request.itemChecksum);
            offer.offerDigest=mix(offer.offerDigest,static_cast<std::uint64_t>(job.request.side));
            if(!offer.offerDigest) offer.offerDigest=1;
            offer.offerRevision=static_cast<std::uint32_t>(mix(job.request.ownerRevision,
                    job.request.companionRevision));
            if(!offer.offerRevision) offer.offerRevision=1;
            result=coordinator.Commit(offer,true);
        }
        if(!ctx->Healthy()) return DetachedTradeOutcome::Unknown();
        if(result==TradeJournalResult::Committed)
            return DetachedTradeOutcome::Committed(pending.present);
        if(result==TradeJournalResult::AlreadyCommitted)
            return DetachedTradeOutcome::Committed(true);
        if(result==TradeJournalResult::RecoveryRequired || result==TradeJournalResult::Busy)
            return DetachedTradeOutcome::Unknown();
        // Rejection has no durable intent only when journal remains healthy.
        return DetachedTradeOutcome::Refused(-60);
    }
private:
    IDetachedJournalContextFactory& factory_;
};

}}}
