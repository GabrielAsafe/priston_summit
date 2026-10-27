#pragma once
// SUMMIT327: server-side OBSERVATION ONLY of save ingress and DAT flush activity.
// This fence tracks received save fragments and legacy buffer flushes without
// changing, delaying or vetoing writes. It is NOT a durable transaction lock.
// In particular, it cannot authorize a trade or authenticate client item bytes.
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <map>

namespace gameplay { namespace companions { namespace v2 {

struct SaveSessionKey {
    std::uint32_t serial;
    std::uint32_t generation;
    SaveSessionKey(std::uint32_t id = 0, std::uint32_t gen = 0)
        : serial(id), generation(gen) {}
    // Generation zero is legal on the first legacy connection.
    bool Valid() const { return serial != 0u; }
    bool operator<(const SaveSessionKey& rhs) const {
        return serial < rhs.serial || (serial == rhs.serial && generation < rhs.generation);
    }
};

struct SaveReadOnlyEpoch {
    SaveSessionKey session;
    std::uint64_t revision;
    SaveReadOnlyEpoch() : session(), revision(0u) {}
    bool CanCommitTrade() const { return false; }
};

// Minimal C++11 spin lock, avoiding a dependency on MinGW's optional
// std::mutex threading backend. Critical sections never contain disk I/O.
class SaveFenceSpinLock final {
public:
    SaveFenceSpinLock() : busy_(ATOMIC_FLAG_INIT) {}
    void lock() {
        while (busy_.test_and_set(std::memory_order_acquire)) {}
    }
    void unlock() { busy_.clear(std::memory_order_release); }
private:
    SaveFenceSpinLock(const SaveFenceSpinLock&) = delete;
    SaveFenceSpinLock& operator=(const SaveFenceSpinLock&) = delete;
    std::atomic_flag busy_;
};
class SaveFenceGuard final {
public:
    explicit SaveFenceGuard(SaveFenceSpinLock& lock) : lock_(lock) { lock_.lock(); }
    ~SaveFenceGuard() { lock_.unlock(); }
private:
    SaveFenceGuard(const SaveFenceGuard&) = delete;
    SaveFenceGuard& operator=(const SaveFenceGuard&) = delete;
    SaveFenceSpinLock& lock_;
};

class SaveActivityFence final {
public:
    // A bounded registry: an exhausted registry fails closed for observations,
    // while never interfering with normal character persistence.
    static const std::size_t kMaximumSessions = 4096u;

    SaveActivityFence() : serial_(0u), untrackedWriters_(0u) {}
    SaveActivityFence(const SaveActivityFence&) = delete;
    SaveActivityFence& operator=(const SaveActivityFence&) = delete;

    bool BeginReceive(SaveSessionKey key, int index, int total) {
        if (!key.Valid()) return false;
        SaveFenceGuard lock(mutex_);
        State* state = FindOrCreate(key);
        if (!state) return false;
        ++state->receivers;
        // The sequence is quarantined until a successfully validated final
        // fragment, including after interruption, invalid ordering or a
        // changed declared fragment count.
        state->incomplete = true;
        if (index == 0) {
            state->sequenceValid = total > 0;
            state->expectedPart = 1;
            state->declaredTotal = total;
        } else if (!state->sequenceValid || total != state->declaredTotal ||
                   index != state->expectedPart || index < 0 || index >= total) {
            state->sequenceValid = false;
        } else {
            ++state->expectedPart;
        }
        state->revision = Next();
        return true;
    }

    void EndReceive(SaveSessionKey key, bool validatedComplete, bool finalPart) {
        if (!key.Valid()) return;
        SaveFenceGuard lock(mutex_);
        const auto found = sessions_.find(key);
        if (found == sessions_.end() || !found->second.receivers) return;
        State& state = found->second;
        if (validatedComplete && finalPart && state.sequenceValid &&
            state.expectedPart == state.declaredTotal) {
            state.incomplete = false;
            state.expectedPart = 0;
            state.declaredTotal = 0;
            state.sequenceValid = false;
            state.completed = true;
            // A new sequence is issued ONLY after the final fragment has
            // passed the authenticated legacy record validator. A fragment,
            // failed validation or a DAT flush never issues a save receipt.
            state.acceptedSaveSequence = Next();
        } else if (finalPart) {
            state.incomplete = true;
            state.sequenceValid = false;
        }
        --state.receivers;
        state.revision = Next();
    }

    bool BeginFlush(SaveSessionKey key) {
        if (!key.Valid()) return false;
        SaveFenceGuard lock(mutex_);
        State* state = FindOrCreate(key);
        if (!state) return false;
        ++state->flushes;
        state->revision = Next();
        return true;
    }

    void EndFlush(SaveSessionKey key) {
        if (!key.Valid()) return;
        SaveFenceGuard lock(mutex_);
        const auto found = sessions_.find(key);
        if (found == sessions_.end() || !found->second.flushes) return;
        --found->second.flushes;
        found->second.revision = Next();
    }

    // SUMMIT328: admission serialized by the SAME mutex as trade reservation.
    // This is a prerequisite for a transaction, NOT a trade permission. Normal
    // writes run unmodified unless a separate server coordinator holds a lease.
    // If tracking capacity is exhausted, writes still proceed but are counted
    // globally and NO new reservation may begin while they are in flight.
    bool BeginMutation(SaveSessionKey key, bool& tracked) {
        tracked = false;
        SaveFenceGuard lock(mutex_);
        State* state = key.Valid() ? FindOrCreate(key) : nullptr;
        if (!state) {
            ++untrackedWriters_;
            return true;
        }
        if (state->reserved || state->sealedForReconnect || state->writers == UINT32_MAX)
            return false;
        ++state->writers;
        state->revision = Next();
        tracked = true;
        return true;
    }

    void EndMutation(SaveSessionKey key, bool tracked) {
        SaveFenceGuard lock(mutex_);
        if (!tracked) {
            if (untrackedWriters_) --untrackedWriters_;
            return;
        }
        const auto it = sessions_.find(key);
        if (it != sessions_.end() && it->second.writers) {
            --it->second.writers;
            it->second.revision = Next();
        }
    }

    // A lease can only be acquired after a fully validated save and when the
    // per-session mutation count is zero. This does NOT cover uninstrumented
    // legacy mutation paths or authenticate item payloads; do not enable trade.
    bool TryReserve(SaveSessionKey key, std::uint64_t& leaseToken) {
        if (!key.Valid()) return false;
        SaveFenceGuard lock(mutex_);
        const auto found = sessions_.find(key);
        if (found == sessions_.end() || untrackedWriters_ ||
            found->second.reserved || found->second.sealedForReconnect || !Idle(found->second)) return false;
        found->second.reserved = true;
        found->second.leaseToken = Next();
        found->second.revision = Next();
        leaseToken = found->second.leaseToken;
        return true;
    }

    // Once a pairwise transfer has been DURABLY committed, the old running
    // client cannot safely send more inventory saves. Quarantine that exact
    // connection and require a new login to load the committed DAT. This is
    // a functional (relogin-based) delivery mode, NOT a live inventory ack.
    bool SealCommittedForReconnect(SaveSessionKey key, std::uint64_t token) {
        if(!key.Valid() || !token) return false;
        SaveFenceGuard lock(mutex_);
        auto found=sessions_.find(key);
        if(found==sessions_.end() || !found->second.reserved ||
           found->second.leaseToken!=token) return false;
        found->second.sealedForReconnect=true;
        found->second.revision=Next();
        return true;
    }

    // SUMMIT350Q: only call AFTER the authenticated in-place client apply
    // and the server has rebased both its ledger and accepted record image.
    // The first following save has a separate canonical-inventory replay gate.
    bool ReleaseAfterVerifiedLiveApply(SaveSessionKey key, std::uint64_t token) {
        if(!key.Valid() || !token) return false;
        SaveFenceGuard lock(mutex_);
        const auto it=sessions_.find(key);
        if(it==sessions_.end() || !it->second.reserved ||
           !it->second.sealedForReconnect || it->second.leaseToken!=token ||
           it->second.receivers || it->second.flushes || it->second.writers ||
           it->second.incomplete) return false;
        it->second.sealedForReconnect=false;
        it->second.reserved=false;
        it->second.leaseToken=0;
        it->second.revision=Next();
        return true;
    }

    bool ReleaseReservation(SaveSessionKey key, std::uint64_t leaseToken) {
        if (!key.Valid() || !leaseToken) return false;
        SaveFenceGuard lock(mutex_);
        const auto found = sessions_.find(key);
        if (found == sessions_.end() || !found->second.reserved ||
            found->second.leaseToken != leaseToken) return false;
        found->second.reserved = false;
        found->second.leaseToken = 0;
        found->second.revision = Next();
        return true;
    }

    bool TryObserve(SaveSessionKey key, SaveReadOnlyEpoch& out) const {
        if (!key.Valid()) return false;
        SaveFenceGuard lock(mutex_);
        const auto found = sessions_.find(key);
        if (found == sessions_.end() || !Idle(found->second)) return false;
        SaveReadOnlyEpoch candidate;
        candidate.session = key;
        candidate.revision = found->second.revision;
        out = candidate;
        return true;
    }

    // Server-only monotonic sequence for a complete validated save. A trade
    // coordinator may compare this to the sequence observed before requesting
    // a fresh client save. This is not item authority or proof of an atomic
    // pair write; the caller still needs a frozen session and journal commit.
    bool TryAcceptedSaveSequence(SaveSessionKey key, std::uint64_t& sequence) const {
        if (!key.Valid()) return false;
        SaveFenceGuard lock(mutex_);
        const auto found = sessions_.find(key);
        if (found == sessions_.end() || !Idle(found->second) ||
            !found->second.acceptedSaveSequence) return false;
        sequence = found->second.acceptedSaveSequence;
        return true;
    }

    // SUMMIT350I: the legacy save writer may attest which accepted ingress
    // sequence it is physically flushing. This is observation only: it does
    // NOT grant a trade lease and cannot make an unverified DAT authoritative.
    // Its witness is checked AGAINST the exclusive lease before admission.
    bool TryActiveFlushAcceptedSequence(SaveSessionKey key,
                                        std::uint64_t& sequence) const {
        if(!key.Valid()) return false;
        SaveFenceGuard lock(mutex_);
        const auto found=sessions_.find(key);
        if(found==sessions_.end()) return false;
        const State& state=found->second;
        if(!state.flushes || !state.completed || state.receivers ||
           state.incomplete || state.reserved || !state.acceptedSaveSequence)
            return false;
        sequence=state.acceptedSaveSequence;
        return true;
    }

    // Only the holder of the live exclusive lease may read this sequence while
    // reserved. Used to prove a save cannot slip between preflight and capture.
    bool TryLeasedAcceptedSaveSequence(SaveSessionKey key, std::uint64_t token,
                                       std::uint64_t& sequence) const {
        if (!key.Valid() || !token) return false;
        SaveFenceGuard lock(mutex_);
        const auto found = sessions_.find(key);
        if (found == sessions_.end()) return false;
        const State& state = found->second;
        if (!state.reserved || state.leaseToken != token ||
            !state.completed || state.receivers || state.flushes ||
            state.writers || state.incomplete || !state.acceptedSaveSequence)
            return false;
        sequence = state.acceptedSaveSequence;
        return true;
    }

    bool IsSealedForReconnect(SaveSessionKey key) const {
        SaveFenceGuard lock(mutex_);
        const auto found=sessions_.find(key);
        return found!=sessions_.end() && found->second.sealedForReconnect;
    }

    bool StillStable(const SaveReadOnlyEpoch& prior) const {
        if (!prior.session.Valid() || !prior.revision) return false;
        SaveFenceGuard lock(mutex_);
        const auto found = sessions_.find(prior.session);
        return found != sessions_.end() && Idle(found->second) &&
            found->second.revision == prior.revision;
    }

private:
    struct State {
        std::uint64_t revision = 0;
        std::uint32_t receivers = 0;
        std::uint32_t flushes = 0;
        std::uint32_t writers = 0;
        bool reserved = false;
        bool sealedForReconnect = false;
        std::uint64_t leaseToken = 0;
        std::uint64_t acceptedSaveSequence = 0;
        int expectedPart = 0;
        int declaredTotal = 0;
        bool sequenceValid = false;
        bool incomplete = false;
        bool completed = false;
    };
    static bool Idle(const State& state) {
        return state.completed && !state.receivers && !state.flushes &&
            !state.writers && !state.reserved &&
            !state.incomplete && !state.sealedForReconnect;
    }
    std::uint64_t Next() {
        ++serial_;
        if (!serial_) ++serial_;
        return serial_;
    }
    State* FindOrCreate(SaveSessionKey key) {
        auto found = sessions_.find(key);
        if (found != sessions_.end()) return &found->second;
        if (sessions_.size() >= kMaximumSessions) {
            // Evict only an inactive session. Any outstanding read-only epoch
            // to an evicted key will fail validation (unique revision counter).
            auto unused = sessions_.begin();
            while (unused != sessions_.end() &&
                   (unused->second.receivers || unused->second.flushes ||
                    unused->second.writers || unused->second.reserved)) ++unused;
            if (unused == sessions_.end()) return nullptr;
            sessions_.erase(unused);
        }
        auto inserted = sessions_.emplace(key, State());
        return &inserted.first->second;
    }
    mutable SaveFenceSpinLock mutex_;
    std::map<SaveSessionKey, State> sessions_;
    std::uint64_t serial_;
    std::uint32_t untrackedWriters_;
};

inline SaveActivityFence& ServerSaveActivityFence() {
    static SaveActivityFence singleton;
    return singleton;
}

// Admission RAII: one writer is counted for the entire legacy function,
// including direct DAT I/O and nested ledger mutations. If a lease is active,
// the caller MUST return before touching the legacy record/ledger.
class SaveMutationAdmission final {
public:
    SaveMutationAdmission(SaveActivityFence& fence, SaveSessionKey key)
        : fence_(fence), key_(key), tracked_(false),
          admitted_(fence_.BeginMutation(key_, tracked_)) {}
    ~SaveMutationAdmission() {
        if (admitted_) fence_.EndMutation(key_, tracked_);
    }
    SaveMutationAdmission(const SaveMutationAdmission&) = delete;
    SaveMutationAdmission& operator=(const SaveMutationAdmission&) = delete;
    bool Admitted() const { return admitted_; }
private:
    SaveActivityFence& fence_;
    SaveSessionKey key_;
    bool tracked_;
    bool admitted_;
};

// RAII for coordinator integration. Currently no runtime code creates a lease;
// adding the lease alone MUST NOT be interpreted as a safe transfer.
class SaveTradeReservationLease final {
public:
    SaveTradeReservationLease(SaveActivityFence& fence, SaveSessionKey key)
        : fence_(fence), key_(key), token_(0), acquired_(fence_.TryReserve(key_, token_)) {}
    ~SaveTradeReservationLease() {
        if (acquired_) (void)fence_.ReleaseReservation(key_, token_);
    }
    SaveTradeReservationLease(const SaveTradeReservationLease&) = delete;
    SaveTradeReservationLease& operator=(const SaveTradeReservationLease&) = delete;
    bool Acquired() const { return acquired_; }
    std::uint64_t Token() const { return acquired_ ? token_ : 0; }
    bool AcceptedSaveSequence(std::uint64_t& out) const {
        return acquired_ && fence_.TryLeasedAcceptedSaveSequence(key_, token_, out);
    }
    bool SealForReconnect() {
        return acquired_ && fence_.SealCommittedForReconnect(key_,token_);
    }
    bool CanCommitTrade() const { return false; }
private:
    SaveActivityFence& fence_;
    SaveSessionKey key_;
    std::uint64_t token_;
    bool acquired_;
};

class SaveReceiveActivityScope final {
public:
    SaveReceiveActivityScope(SaveActivityFence& fence, SaveSessionKey session,
                             int index, int total)
        : fence_(fence), session_(session), active_(fence_.BeginReceive(session, index, total)),
          complete_(false), finalPart_(total > 0 && index >= 0 && index == total - 1) {}
    ~SaveReceiveActivityScope() { if (active_) fence_.EndReceive(session_, complete_, finalPart_); }
    SaveReceiveActivityScope(const SaveReceiveActivityScope&) = delete;
    SaveReceiveActivityScope& operator=(const SaveReceiveActivityScope&) = delete;
    void MarkValidatedComplete() { complete_ = true; }
private:
    SaveActivityFence& fence_;
    SaveSessionKey session_;
    bool active_;
    bool complete_;
    bool finalPart_;
};

class SaveFlushActivityScope final {
public:
    SaveFlushActivityScope(SaveActivityFence& fence, SaveSessionKey session)
        : fence_(fence), session_(session), active_(fence_.BeginFlush(session)) {}
    ~SaveFlushActivityScope() { if (active_) fence_.EndFlush(session_); }
    SaveFlushActivityScope(const SaveFlushActivityScope&) = delete;
    SaveFlushActivityScope& operator=(const SaveFlushActivityScope&) = delete;
private:
    SaveActivityFence& fence_;
    SaveSessionKey session_;
    bool active_;
};

}}} // namespace gameplay::companions::v2
