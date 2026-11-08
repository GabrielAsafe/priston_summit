#pragma once
// SUMMIT431A. REAL on-disk, single-DAT write-ahead journal.
// NO game runtime integration. All mutating calls require a host-owned exclusive
// admission lease that covers every online/offline writer and delayed save.
// The caller MUST seal the old client session for reconnect BEFORE committing.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace gameplay { namespace crafting { namespace v2 {
using CraftBytes431 = std::vector<std::uint8_t>;

struct CraftDatPlan431 {
    std::uint64_t characterId = 0;  // authenticated server-side identity
    std::uint64_t requestNonce = 0; // server-generated, globally unique
    std::uint64_t recipeRevision = 0; // version/fingerprint of validated JSON recipe
    CraftBytes431 before; // exact complete authenticated DAT
    CraftBytes431 after;  // 430A-authenticated full DAT after-image
};

enum class CraftCommitState431 {
    Committed, AlreadyCommitted, NoPending, Rejected, RecoveryRequired
};

// Integration MUST enforce these requirements, not just return true.
// Exclusive admission covers save packet reception, queued saves, offline writes,
// trades, logout and character loading. A sealed session remains quarantined
// after this scope exits until a new authenticated login reloads the DAT.
class ICraftDatAdmission431 {
public:
    virtual ~ICraftDatAdmission431() {}
    virtual bool LockCharacter(std::uint64_t characterId) = 0;
    virtual bool QuiescedAllWriters(std::uint64_t characterId) = 0;
    virtual bool SealOldSessionForReconnect(std::uint64_t characterId) = 0;
    virtual void UnlockCharacter(std::uint64_t characterId) = 0;
    // Trusted server-derived path. Never construct from a client packet.
    virtual std::string TrustedDatPath(std::uint64_t characterId) = 0;
};

class CraftSingleDatJournal431 final {
public:
    explicit CraftSingleDatJournal431(std::string journalRoot);
    // Owns the lease during entire operation. BEFORE/AFTER must be independently
    // authenticated and validated using the 430A image builder.
    CraftCommitState431 Commit(const CraftDatPlan431&, ICraftDatAdmission431&);
    // Must run as a pre-login recovery step, before old client save is admitted.
    // Fails closed on an unknown/corrupt journal or an unexpected DAT image.
    CraftCommitState431 Recover(std::uint64_t characterId, ICraftDatAdmission431&);
    // Read-only: validates receipt+archived plan and identity; doesn't grant items.
    bool DurableReceipt(std::uint64_t requestNonce, std::uint64_t characterId,
                        std::uint64_t recipeRevision);
    bool Healthy() const { return healthy_; }
private:
    std::string root_;
    bool healthy_ = false;
    void Poison() { healthy_=false; }
    std::string Intent(std::uint64_t characterId) const;
    std::string Archive(std::uint64_t nonce) const;
    std::string Receipt(std::uint64_t nonce) const;
    CraftCommitState431 Finish(const CraftDatPlan431&, const std::string& datPath);
};

}}} // gameplay::crafting::v2
