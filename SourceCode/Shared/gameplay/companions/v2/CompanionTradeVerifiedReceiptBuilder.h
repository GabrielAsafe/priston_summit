#pragma once
// Derives a delivery receipt from a *verified durable journal* plus its
// exact prepared before/after images. The journal lookup must come from the
// server's trusted disk journal, NEVER from a network peer. The second
// callback MUST confirm the complete plan images against a persisted,
// immutable disk record, not merely match offer fields. Older journal
// receipts do not contain these hashes and deleted intents cannot satisfy
// this proof without separate retained delivery evidence. This builder is
// intentionally read-only: a receipt cannot cause the journal to commit.
#include "gameplay/companions/v2/CompanionTradeLiveSyncProtocol.h"
#include "gameplay/companions/v2/CompanionTradeJournal.h"
#include "gameplay/companions/v2/CompanionTradeSha256.h"
#include "gameplay/companions/v2/CompanionTradeCanonicalInventoryDigest.h"
#include <cstdint>
namespace gameplay { namespace companions { namespace v2 {
class TradeVerifiedReceiptBuilder final {
public:
    template<class AuthenticatedIdentityLookup, class VerifiedDurablePlanLookup>
    static bool Reconstruct(const TradeSyncIdentity& session,
        const TradeSyncPairKey& pair,
        const CompanionTradePrepared& journaledPlan,
        const CompanionTradeReceipt& verifiedDiskReceipt,
        const BufferedTradeEnvelopeLayout& playerLayout,
        AuthenticatedIdentityLookup isAuthenticatedPair,
        VerifiedDurablePlanLookup matchesDurablePlanOnDisk,
        TradeSyncDurableReceipt& result) {
        if (!session.Valid() || !pair.Valid() ||
            !TradeReceiptMatches(verifiedDiskReceipt,journaledPlan.offer) ||
            !journaledPlan.offer.tradeNonce ||
            !isAuthenticatedPair(pair,journaledPlan.offer.authenticatedOwner,
                                 journaledPlan.offer.companionCharacter) ||
            !matchesDurablePlanOnDisk(journaledPlan) ||
            journaledPlan.ownerBefore.empty() || journaledPlan.ownerAfter.empty() ||
            journaledPlan.companionBefore.empty() || journaledPlan.companionAfter.empty())
            return false;
        TradeSyncDurableReceipt candidate{};
        candidate.identity=session;
        candidate.pair=pair;
        candidate.journalTxnId=journaledPlan.offer.tradeNonce;
        candidate.beforeSha256=TradeSha256::Hash(journaledPlan.ownerBefore.data(),
                                                  journaledPlan.ownerBefore.size());
        candidate.afterSha256=TradeSha256::Hash(journaledPlan.ownerAfter.data(),
                                                 journaledPlan.ownerAfter.size());
        candidate.companionAfterSha256=TradeSha256::Hash(journaledPlan.companionAfter.data(),
                                                          journaledPlan.companionAfter.size());
        if (!TradeCanonicalInventoryDigest::Compute(journaledPlan.ownerAfter,
                  playerLayout,candidate.playerInventoryAfterSha256) || !candidate.Valid())
            return false;
        result=std::move(candidate);
        return true;
    }
};
}}}
