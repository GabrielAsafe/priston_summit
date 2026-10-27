#pragma once
// SUMMIT322: durable *journal* only. Never mutates a player inventory or DAT.
// All domain logic stays in CompanionTradeJournal.h. Character IO must be wired
// behind ICompanionTradeSnapshotBackend and MUST honor its full-pair lock.
#include "gameplay/companions/v2/CompanionTradeJournal.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace gameplay { namespace companions { namespace v2 {

class ICompanionTradeSnapshotBackend {
public:
    virtual ~ICompanionTradeSnapshotBackend() {}
    // Must protect every save, logout, equip, trade, etc., not just trade calls.
    virtual bool LockPair(std::uint64_t owner, std::uint64_t companion) = 0;
    virtual void UnlockPair(std::uint64_t owner, std::uint64_t companion) = 0;
    // Must validate owner, full item identity, equip state, balance, capacity,
    // current generation and offline companion before computing snapshots.
    virtual bool Prepare(const CompanionTradeOffer&, CompanionTradePrepared&) = 0;
    // Idempotent read-compare-exact replace with durably persisted target;
    // NO speculative mutation on Conflict or IoUncertain.
    virtual TradeApplyResult ApplyExact(TradeParticipant, const CompanionTradePrepared&) = 0;
};

// Root directory must exist, be private to the server, and not be a symlink.
// Files are immutable, published by an atomic no-replace operation; partially
// written temp files are NOT interpreted as committed intents/receipts.
// Unexpected/corrupt final records fail closed (never treated as absent).
class CompanionTradeDiskJournal final : public ICompanionTradeJournalPort {
public:
    CompanionTradeDiskJournal(std::string root, ICompanionTradeSnapshotBackend& backend);
    bool LockPair(std::uint64_t owner, std::uint64_t companion) override;
    void UnlockPair(std::uint64_t owner, std::uint64_t companion) override;
    CompanionTradeReceipt Receipt(std::uint64_t nonce) override;
    CompanionTradePending PendingForPair(std::uint64_t owner, std::uint64_t companion) override;
    // Retain the exact prepared after-images after clearing a committed
    // intent, so a disconnected client can recover delivery after restart.
    // Requires a valid durable receipt AND an immutable archived plan.
    // Absent/legacy archives fail closed; this method never grants items.
    bool CompletedPlan(std::uint64_t nonce, CompanionTradePrepared& out);
    bool Prepare(const CompanionTradeOffer&, CompanionTradePrepared&) override;
    bool SealIntent(const CompanionTradePrepared&) override;
    TradeApplyResult ApplyExact(TradeParticipant, const CompanionTradePrepared&) override;
    bool SealReceipt(const CompanionTradePrepared&) override;
    bool RemoveIntent(const CompanionTradePrepared&) override;
    // Operational health indicator: any IO/corruption error poisons this
    // instance; caller must halt participant mutations and request recovery.
    bool Healthy() const { return healthy_; }
private:
    std::string root_;
    ICompanionTradeSnapshotBackend& backend_;
    bool healthy_;
    void Poison() { healthy_ = false; }
    std::string IntentPath(std::uint64_t owner, std::uint64_t companion) const;
    std::string ReceiptPath(std::uint64_t nonce) const;
    std::string ArchivePath(std::uint64_t nonce) const;
};

}}} // namespace gameplay::companions::v2
