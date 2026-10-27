#pragma once
// Production-oriented, exact-file journal backend. NOT an admission route or
// online-session integration: the host port MUST lock every player and
// companion mutation, supply authenticated paths, and reconcile the online
// save buffer with durable bytes BEFORE preparing an offer.
#include "gameplay/companions/v2/CompanionTradeDiskJournal.h"
#include "gameplay/companions/v2/CompanionTradeExactRecordStore.h"
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

namespace gameplay { namespace companions { namespace v2 {

// The host is responsible for an authenticated pair lease covering external
// legacy writers (not just this backend). No identifiers or file paths may
// come from the untrusted wire without independent account validation.
class ICompanionTradeExactPairHost {
public:
    virtual ~ICompanionTradeExactPairHost() {}
    virtual bool AcquireExternalLease(std::uint64_t owner,
                                      std::uint64_t companion) = 0;
    virtual void ReleaseExternalLease(std::uint64_t owner,
                                      std::uint64_t companion) = 0;
    // Returns fully trusted, distinct, pre-existing, regular character DATs.
    // Any dirty online buffer must be flushed under the external lease FIRST.
    virtual bool TrustedPathsUnderLease(std::uint64_t owner,
                                        std::uint64_t companion,
                                        std::string& ownerPath,
                                        std::string& companionPath) = 0;
    // Revalidate authenticated session, item identity, current save, capacity,
    // actual encoded payloads and conservation while external lease is held.
    // Returns complete before/after byte images and EXACT same offer.
    virtual bool BuildPreparedUnderLease(const CompanionTradeOffer& offer,
                                         CompanionTradePrepared& prepared) = 0;
    // Required before each durable write. Must fail if session was recycled,
    // client freeze lost or any non-journal mutation path became unprotected.
    virtual bool StillOwnsExternalLease(std::uint64_t owner,
                                        std::uint64_t companion) = 0;
};

class CompanionTradeExactPairBackend final : public ICompanionTradeSnapshotBackend {
public:
    explicit CompanionTradeExactPairBackend(ICompanionTradeExactPairHost& host);
    ~CompanionTradeExactPairBackend() override;
    bool LockPair(std::uint64_t owner, std::uint64_t companion) override;
    void UnlockPair(std::uint64_t owner, std::uint64_t companion) override;
    bool Prepare(const CompanionTradeOffer&, CompanionTradePrepared&) override;
    TradeApplyResult ApplyExact(TradeParticipant,
                                const CompanionTradePrepared&) override;
private:
    ICompanionTradeExactPairHost& host_;
    // Serializing all pair operations within the server is conservative and
    // avoids a process-local data race. External lease must cover OTHER code.
    std::recursive_mutex mutex_;
    bool locked_ = false;
    std::thread::id lockOwner_{};
    std::uint64_t owner_ = 0, companion_ = 0;
    std::string ownerPath_, companionPath_;
    CompanionTradePrepared prepared_{};
    bool hasPrepared_ = false;
    bool PairMatches(std::uint64_t owner, std::uint64_t companion) const;
};

}}} // namespace gameplay::companions::v2
