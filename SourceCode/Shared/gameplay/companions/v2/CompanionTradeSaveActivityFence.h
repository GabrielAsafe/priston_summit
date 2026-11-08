#pragma once
// SUMMIT327: server-side OBSERVATION ONLY of save ingress and DAT flush activity.
// This fence tracks received save fragments and legacy buffer flushes without
// changing, delaying or vetoing writes. It is NOT a durable transaction lock.
// In particular, it cannot authorize a trade or authenticate client item bytes.
#include <atomic>
#include <cstdio>  // SUMMIT451A: privacy-safe peer receipt diagnostics
#include <cstddef>
#include <cstdint>
#include <map>
#include <cstdint>

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

// SUMMIT448A: reason codes for an ONLINE craft lease refusal; no account data.
enum class SaveLeaseReason448 {
    Ready, InvalidSession, NotSaved, IncompleteSave, Receiving,
    Flushing, Mutating, Reserved, Sealed, UntrackedWriter
};
inline const char* SaveLeaseReasonName448(SaveLeaseReason448 v) {
    switch(v) {
      case SaveLeaseReason448::Ready: return "ready";
      case SaveLeaseReason448::InvalidSession: return "invalid_session";
      case SaveLeaseReason448::NotSaved: return "no_validated_save";
      case SaveLeaseReason448::IncompleteSave: return "incomplete_save";
      case SaveLeaseReason448::Receiving: return "save_receiving";
      case SaveLeaseReason448::Flushing: return "dat_flushing";
      case SaveLeaseReason448::Mutating: return "save_writer_active";
      case SaveLeaseReason448::Reserved: return "session_reserved";
      case SaveLeaseReason448::Sealed: return "session_sealed";
      case SaveLeaseReason448::UntrackedWriter: return "untracked_writer_active";
    }
    return "unknown";
}

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

    // SUMMIT451A: return true ONLY when this final fragment has just issued
    // a fresh, validated receipt (never reuse a previous accepted sequence).
    bool EndReceive(SaveSessionKey key, bool validatedComplete, bool finalPart) {
        if (!key.Valid()) return false;
        SaveFenceGuard lock(mutex_);
        const auto found = sessions_.find(key);
        if (found == sessions_.end() || !found->second.receivers) return false;
        State& state = found->second;
        const bool acceptedNow = validatedComplete && finalPart &&
            state.sequenceValid && state.expectedPart == state.declaredTotal;
        if (acceptedNow) {
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
        return acceptedNow;
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
        bool credited451=false;
        bool refused451=false;
        {
            SaveFenceGuard lock(mutex_);
            if (!tracked) {
                if (untrackedWriters_) --untrackedWriters_;
                return;
            }
            const auto it = sessions_.find(key);
            if (it != sessions_.end() && it->second.writers) {
                State& source=it->second;
                --source.writers;
                source.revision=Next();
                // SUMMIT451A: save receive scope ends BEFORE this writer guard.
                // Credit the other authenticated socket only after this LAST
                // writer is gone. Never bypass the existing lease requirements.
                if (!source.writers && source.pendingPeer451.Valid()) {
                    const SaveSessionKey peer=source.pendingPeer451;
                    const std::uint64_t witness=source.pendingSequence451;
                    source.pendingPeer451=SaveSessionKey();
                    source.pendingSequence451=0u;
                    refused451=true;
                    if (peer.Valid() && !(peer.serial==key.serial &&
                         peer.generation==key.generation) && !untrackedWriters_ &&
                        Idle(source) && source.acceptedSaveSequence==witness) {
                        // Do not evict the source receipt if registry is full.
                        State* target=nullptr;
                        const auto existing=sessions_.find(peer);
                        if(existing!=sessions_.end()) target=&existing->second;
                        else if(sessions_.size()<kMaximumSessions)
                            target=FindOrCreate(peer);
                        if(target && !target->reserved && !target->sealedForReconnect &&
                           !target->receivers && !target->flushes &&
                           !target->writers && !target->incomplete) {
                            target->completed=true;
                            target->acceptedSaveSequence=Next();
                            target->revision=Next();
                            credited451=true;
                            refused451=false;
                        }
                    }
                }
            }
        }
        // No character name, account, socket pointer or item bytes in logs.
        if(credited451) std::fprintf(stderr,
            "[SUMMIT451][SAVE] peer_receipt=credited_after_writer\n");
        else if(refused451) std::fprintf(stderr,
            "[SUMMIT451][SAVE] peer_receipt=refused_after_writer\n");
    }

    // A completed receive may queue a witness while its legacy save writer
    // still owns the source. Do not copy receipt here: EndMutation verifies
    // that the writer really finished and the receipt did not change.
    bool QueueValidatedPeer451(SaveSessionKey sourceKey, SaveSessionKey peer) {
        if(!sourceKey.Valid() || !peer.Valid() ||
           (sourceKey.serial==peer.serial && sourceKey.generation==peer.generation))
            return false;
        SaveFenceGuard lock(mutex_);
        if(untrackedWriters_) return false;
        const auto sourceIt=sessions_.find(sourceKey);
        if(sourceIt==sessions_.end()) return false;
        State& source=sourceIt->second;
        if(!source.completed || source.incomplete || source.receivers ||
           source.flushes || source.writers!=1u || source.reserved ||
           source.sealedForReconnect || !source.acceptedSaveSequence ||
           source.pendingPeer451.Valid()) return false;
        source.pendingPeer451=peer;
        source.pendingSequence451=source.acceptedSaveSequence;
        return true;
    }

    // SUMMIT452A: the server itself binds its authenticated /craft request to
    // the next validated legacy save for the SAME owned participant.
    // The source DATA socket need not be discoverable using FindUserFromName.
    // This registry NEVER grants a lease or credits a receipt on registration.
    bool ExpectValidatedCraftSave452(std::uint64_t participant,SaveSessionKey target,
                                      std::uint32_t tick) {
        if(!participant || !target.Valid()) return false;
        SaveFenceGuard lock(mutex_);
        if(untrackedWriters_)return false;
        State* s=FindOrCreate(target);
        if(!s||s->reserved||s->sealedForReconnect||s->receivers||s->writers||s->flushes)
            return false;
        if(expectedCraft452_.size()>=kMaximumSessions &&
           expectedCraft452_.find(participant)==expectedCraft452_.end())return false;
        ExpectedCraft452 request;
        request.target=target;
        request.sinceSequence=serial_;
        request.tick=tick;
        expectedCraft452_[participant]=request;
        return true;
    }

    enum class ParticipantReceipt452 { NoRequest, Queued, SameSocket, Refused, Expired };
    // Called ONLY after the legacy record validator accepted its FINAL part.
    // Source is still inside its sole SaveMutationAdmission writer; real credit
    // will occur in EndMutation after that writer is released (SUMMIT451A).
    ParticipantReceipt452 QueueValidatedCraftSave452(SaveSessionKey source,
           std::uint64_t participant,std::uint32_t tick) {
        if(!source.Valid()||!participant)return ParticipantReceipt452::Refused;
        SaveFenceGuard lock(mutex_);
        const auto it=expectedCraft452_.find(participant);
        if(it==expectedCraft452_.end())return ParticipantReceipt452::NoRequest;
        const ExpectedCraft452 expected=it->second;
        if(static_cast<std::uint32_t>(tick-expected.tick)>120000u) {
            expectedCraft452_.erase(it);return ParticipantReceipt452::Expired;
        }
        const auto si=sessions_.find(source);
        if(si==sessions_.end()||untrackedWriters_)return ParticipantReceipt452::Refused;
        State& s=si->second;
        if(!s.completed||s.incomplete||s.receivers||s.flushes||s.writers!=1u||
           s.reserved||s.sealedForReconnect||!s.acceptedSaveSequence||
           s.acceptedSaveSequence<=expected.sinceSequence)
            return ParticipantReceipt452::Refused;
        const SaveSessionKey target=expected.target;
        if(target.serial==source.serial && target.generation==source.generation) {
            expectedCraft452_.erase(it);
            return ParticipantReceipt452::SameSocket;
        }
        if(s.pendingPeer451.Valid())return ParticipantReceipt452::Refused;
        // Verify the target is still a valid unsealed fence session. Final
        // authentication of the live socket is checked again by CraftHost.Session.
        const auto ti=sessions_.find(target);
        if(ti==sessions_.end()||ti->second.reserved||ti->second.sealedForReconnect||
           ti->second.receivers||ti->second.flushes||ti->second.writers||
           ti->second.incomplete) return ParticipantReceipt452::Refused;
        s.pendingPeer451=target;
        s.pendingSequence451=s.acceptedSaveSequence;
        expectedCraft452_.erase(it);
        return ParticipantReceipt452::Queued;
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

    // SUMMIT448A: a read-only diagnostic snapshot under the same fence lock.
    // Never interprets lack of a save as authorisation to mutate the DAT.
    SaveLeaseReason448 DiagnoseLease448(SaveSessionKey key) const {
        if(!key.Valid()) return SaveLeaseReason448::InvalidSession;
        SaveFenceGuard lock(mutex_);
        if(untrackedWriters_) return SaveLeaseReason448::UntrackedWriter;
        const auto it=sessions_.find(key);
        if(it==sessions_.end()) return SaveLeaseReason448::NotSaved;
        const State& s=it->second;
        if(s.sealedForReconnect) return SaveLeaseReason448::Sealed;
        if(s.reserved) return SaveLeaseReason448::Reserved;
        if(s.receivers) return SaveLeaseReason448::Receiving;
        if(s.flushes) return SaveLeaseReason448::Flushing;
        if(s.writers) return SaveLeaseReason448::Mutating;
        if(s.incomplete) return SaveLeaseReason448::IncompleteSave;
        if(!s.completed || !s.acceptedSaveSequence) return SaveLeaseReason448::NotSaved;
        return SaveLeaseReason448::Ready;
    }

    // Rate-limited request admission. This NEVER grants a craft lease; it
    // merely allows the server to ask a connected client for an ordinary save.
    // A subsequent validated client record remains mandatory for TryReserve.
    bool TryRequestFreshSave448(SaveSessionKey key, std::uint32_t tickMs,
                                std::uint32_t minimumDelayMs=8000u) {
        if(!key.Valid() || minimumDelayMs<1000u) return false;
        SaveFenceGuard lock(mutex_);
        if(untrackedWriters_) return false;
        State* s=FindOrCreate(key);
        if(!s || s->reserved || s->sealedForReconnect || s->receivers ||
           s->flushes || s->writers) return false;
        if(s->completed && !s->incomplete && s->acceptedSaveSequence) return false;
        if(s->refreshRequested &&
           static_cast<std::uint32_t>(tickMs-s->lastRefreshTick)<minimumDelayMs) return false;
        s->lastRefreshTick=tickMs;
        s->refreshRequested=true;
        // No save state is marked accepted here, and no revision is trusted.
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

    // SUMMIT449A: the normal legacy client may send its authenticated DAT
    // fragments on a DATA socket while /craft make uses the PLAY socket.
    // Credit the play-session fence ONLY AFTER the original source session
    // has completed its fully validated final fragment. The caller must
    // independently authenticate that both sockets own the SAME character.
    // This is a save receipt, not permission to rewrite item bytes or DAT.
    bool CreditPeerValidatedSave449(SaveSessionKey source, SaveSessionKey peer) {
        if(!source.Valid() || !peer.Valid() ||
           (source.serial==peer.serial && source.generation==peer.generation))
            return false;
        SaveFenceGuard lock(mutex_);
        if(untrackedWriters_) return false;
        const auto sourceIt=sessions_.find(source);
        if(sourceIt==sessions_.end() || !Idle(sourceIt->second) ||
           !sourceIt->second.acceptedSaveSequence) return false;
        State* target=FindOrCreate(peer);
        if(!target || target->reserved || target->sealedForReconnect ||
           target->receivers || target->flushes || target->writers ||
           target->incomplete) return false;
        target->completed=true;
        target->acceptedSaveSequence=Next();
        target->revision=Next();
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
    struct ExpectedCraft452 {
        SaveSessionKey target;
        std::uint64_t sinceSequence=0;
        std::uint32_t tick=0;
    };
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
        bool refreshRequested = false; // SUMMIT448A: request throttle only
        std::uint32_t lastRefreshTick = 0u;
        SaveSessionKey pendingPeer451;
        std::uint64_t pendingSequence451 = 0u;
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
    std::map<std::uint64_t, ExpectedCraft452> expectedCraft452_;
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
    ~SaveReceiveActivityScope() {
        if(active_) {
            const bool freshAccepted451=fence_.EndReceive(session_,complete_,finalPart_);
            if(freshAccepted451) {
                bool handled452=false;
                if(participant452_) {
                    const auto result452=fence_.QueueValidatedCraftSave452(
                        session_,participant452_,participantTick452_);
                    if(result452==SaveActivityFence::ParticipantReceipt452::Queued) {
                        std::fprintf(stderr,"[SUMMIT452][SAVE] requested_participant=queued\n");
                        handled452=true;
                    } else if(result452==SaveActivityFence::ParticipantReceipt452::SameSocket) {
                        std::fprintf(stderr,"[SUMMIT452][SAVE] requested_participant=same_socket\n");
                        handled452=true;
                    } else {
                        std::fprintf(stderr,"[SUMMIT452][SAVE] requested_participant=no_match_or_refused code=%d\n",
                            static_cast<int>(result452));
                    }
                }
                if(!handled452 && peer449_.Valid()) {
                    if(fence_.QueueValidatedPeer451(session_,peer449_)) {
                        std::fprintf(stderr,"[SUMMIT451][SAVE] peer_receipt=queued_until_writer_end\n");
                        handled452=true;
                    } else {
                        std::fprintf(stderr,"[SUMMIT451][SAVE] peer_receipt=queue_refused\n");
                    }
                }
                if(!handled452) {
                    // The source socket still retains its own valid receipt.
                    std::fprintf(stderr,"[SUMMIT451][SAVE] validated_final=1 peer_candidate=0\n");
                }
            } else if(complete_ && finalPart_) {
                // The legacy validator returned success, but the observed
                // save fragment sequence did not form a fresh fence receipt.
                std::fprintf(stderr,
                    "[SUMMIT451][SAVE] final_receipt=sequence_refused\n");
            }
        }
    }
    SaveReceiveActivityScope(const SaveReceiveActivityScope&) = delete;
    SaveReceiveActivityScope& operator=(const SaveReceiveActivityScope&) = delete;
    void MarkValidatedComplete() { complete_ = true; }
    // Only the already validated final fragment may name an authenticated
    // counterpart; credit is deferred until EndReceive has issued a receipt.
    void DeferValidatedPeer449(SaveSessionKey peer) {
        if(active_ && complete_ && finalPart_ && peer.Valid()) peer449_=peer;
    }
    void DeferValidatedParticipant452(std::uint64_t participant,std::uint32_t tick) {
        if(active_ && complete_ && finalPart_ && participant) { participant452_=participant; participantTick452_=tick; }
    }
private:
    SaveActivityFence& fence_;
    SaveSessionKey session_;
    bool active_;
    bool complete_;
    bool finalPart_;
    SaveSessionKey peer449_;
    std::uint64_t participant452_=0u;
    std::uint32_t participantTick452_=0u;
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
