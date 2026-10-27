#include "gameplay/companions/v2/CompanionTradeExactPairBackend.h"
#include <utility>
#include <cassert>

namespace gameplay { namespace companions { namespace v2 {

CompanionTradeExactPairBackend::CompanionTradeExactPairBackend(
    ICompanionTradeExactPairHost& host) : host_(host) {}

CompanionTradeExactPairBackend::~CompanionTradeExactPairBackend() {
    // A backend may never outlive an outstanding journal pair lease.
    assert(!locked_);
}

bool CompanionTradeExactPairBackend::PairMatches(std::uint64_t owner,
                                               std::uint64_t companion) const {
    return locked_ && lockOwner_ == std::this_thread::get_id() &&
           owner_ == owner && companion_ == companion;
}

bool CompanionTradeExactPairBackend::LockPair(std::uint64_t owner,
                                              std::uint64_t companion) {
    if (!owner || !companion || owner == companion || !mutex_.try_lock())
        return false;
    if (locked_) { mutex_.unlock(); return false; }
    if (!host_.AcquireExternalLease(owner, companion)) {
        mutex_.unlock();
        return false;
    }
    std::string ownerFile, companionFile;
    if (!host_.TrustedPathsUnderLease(owner, companion, ownerFile, companionFile) ||
        ownerFile.empty() || companionFile.empty() || ownerFile == companionFile ||
        !host_.StillOwnsExternalLease(owner, companion)) {
        host_.ReleaseExternalLease(owner, companion);
        mutex_.unlock();
        return false;
    }
    owner_ = owner;
    companion_ = companion;
    ownerPath_.swap(ownerFile);
    companionPath_.swap(companionFile);
    locked_ = true;
    lockOwner_ = std::this_thread::get_id();
    hasPrepared_ = false;
    prepared_ = CompanionTradePrepared{};
    return true;
}

void CompanionTradeExactPairBackend::UnlockPair(std::uint64_t owner,
                                                std::uint64_t companion) {
    // Valid coordinator calls UnlockPair on the same thread as LockPair.
    // If a wrong pair is supplied, do not silently unlock another trade.
    if (!PairMatches(owner, companion)) return;
    hasPrepared_ = false;
    prepared_ = CompanionTradePrepared{};
    ownerPath_.clear(); companionPath_.clear();
    locked_ = false;
    lockOwner_ = std::thread::id{};
    owner_ = 0; companion_ = 0;
    host_.ReleaseExternalLease(owner, companion);
    mutex_.unlock();
}

bool CompanionTradeExactPairBackend::Prepare(const CompanionTradeOffer& offer,
                                             CompanionTradePrepared& out) {
    if (!PairMatches(offer.authenticatedOwner, offer.companionCharacter) ||
        !host_.StillOwnsExternalLease(owner_, companion_) || hasPrepared_)
        return false;
    CompanionTradePrepared candidate;
    if (!host_.BuildPreparedUnderLease(offer, candidate) ||
        !SameOffer(offer, candidate.offer) ||
        candidate.ownerBefore.empty() || candidate.ownerAfter.empty() ||
        candidate.companionBefore.empty() || candidate.companionAfter.empty() ||
        candidate.ownerBefore == candidate.ownerAfter ||
        candidate.companionBefore == candidate.companionAfter)
        return false;
    // Real bytes MUST be identical to the authenticated before-images before
    // the journal publishes an intent. This rejects dirty/stale DATs.
    std::vector<std::uint8_t> ownerBytes, companionBytes;
    if (!CompanionTradeExactRecordStore::Read(ownerPath_, ownerBytes) ||
        !CompanionTradeExactRecordStore::Read(companionPath_, companionBytes) ||
        ownerBytes != candidate.ownerBefore ||
        companionBytes != candidate.companionBefore ||
        !host_.StillOwnsExternalLease(owner_, companion_))
        return false;
    prepared_ = candidate;
    hasPrepared_ = true;
    out = std::move(candidate);
    return true;
}

TradeApplyResult CompanionTradeExactPairBackend::ApplyExact(
    TradeParticipant participant, const CompanionTradePrepared& plan) {
    if (!PairMatches(plan.offer.authenticatedOwner, plan.offer.companionCharacter) ||
        !host_.StillOwnsExternalLease(owner_, companion_))
        return TradeApplyResult::IoUncertain;
    // Recovery invokes ApplyExact after restart, without Prepare. The sealed
    // journal's exact before/after bytes, not a client packet, are the source.
    // Never allow a different prepared plan within the same pair lease.
    if (hasPrepared_ && (!SameOffer(prepared_.offer, plan.offer) ||
        prepared_.ownerBefore != plan.ownerBefore ||
        prepared_.ownerAfter != plan.ownerAfter ||
        prepared_.companionBefore != plan.companionBefore ||
        prepared_.companionAfter != plan.companionAfter))
        return TradeApplyResult::Conflict;
    if (participant == TradeParticipant::OwnerOnline)
        return CompanionTradeExactRecordStore::Replace(ownerPath_, plan.ownerBefore, plan.ownerAfter);
    if (participant == TradeParticipant::CompanionOffline)
        return CompanionTradeExactRecordStore::Replace(companionPath_, plan.companionBefore, plan.companionAfter);
    return TradeApplyResult::Conflict;
}

}}}
