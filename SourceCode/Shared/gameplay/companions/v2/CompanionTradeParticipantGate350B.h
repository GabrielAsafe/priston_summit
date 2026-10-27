#pragma once
// SUMMIT 350B: independent in-memory login-vs-trade exclusion.
// No disk, sockets, rsPLAYINFO, or mutex callbacks. Does not replace the
// durable journal recovery that executes after a server restart.
#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <utility>
namespace gameplay { namespace companions { namespace v2 {

class TradeParticipantGate350B final {
public:
    using Pair=std::pair<std::uint64_t,std::uint64_t>;
    class LoadGuard final {
    public:
        LoadGuard(TradeParticipantGate350B& gate, std::uint64_t participant)
            : gate_(&gate), participant_(participant), ok_(gate.BeginLoad(participant)) {}
        ~LoadGuard() { if(ok_) gate_->EndLoad(participant_); }
        LoadGuard(const LoadGuard&)=delete;
        LoadGuard& operator=(const LoadGuard&)=delete;
        bool Allowed() const { return ok_; }
    private:
        TradeParticipantGate350B* gate_;
        std::uint64_t participant_;
        bool ok_;
    };

    // SUMMIT350F: explicit admission for OFFLINE character DAT mutations.
    // The gate is acquired before disk I/O and held until the final rename/
    // flush, so a pair reservation cannot race a writer already in progress.
    // Neither BeginWrite nor TryReserve waits for I/O: unsafe operations
    // fail immediately instead of blocking the world thread.
    class WriteGuard final {
    public:
        WriteGuard(TradeParticipantGate350B& gate, std::uint64_t participant)
            : gate_(&gate), participant_(participant), allowed_(gate.BeginWrite(participant)) {}
        ~WriteGuard() { if(allowed_) gate_->EndWrite(participant_); }
        WriteGuard(const WriteGuard&)=delete;
        WriteGuard& operator=(const WriteGuard&)=delete;
        bool Allowed() const { return allowed_; }
    private:
        TradeParticipantGate350B* gate_;
        std::uint64_t participant_;
        bool allowed_;
    };

    // SUMMIT350G: account-ambiguous legacy DAT writer exclusion.
    // A legacy entrypoint may know only a character name, or can modify
    // multiple characters (import and character-list maintenance). Such a
    // writer acquires a *global* nonblocking hold BEFORE examining files.
    // This refuses an overlapping pair reservation without trying to infer
    // a trusted account from disk or a packet. Never hold the gate's mutex
    // across disk I/O; BeginQueuedWrite/FinishQueuedWrite only change counters.
    class UnknownWriterGuard350G final {
    public:
        explicit UnknownWriterGuard350G(TradeParticipantGate350B& gate)
            : gate_(gate), allowed_(gate_.BeginQueuedWrite(0)) {}
        ~UnknownWriterGuard350G() {
            if(allowed_) (void)gate_.FinishQueuedWrite(0, durable_);
        }
        UnknownWriterGuard350G(const UnknownWriterGuard350G&)=delete;
        UnknownWriterGuard350G& operator=(const UnknownWriterGuard350G&)=delete;
        bool Allowed() const { return allowed_; }
        // Once an entrypoint has published an uncertain partial write, do
        // not authorize any new trade until an operator repairs persistence.
        void MarkUncertain() { durable_=false; }
    private:
        TradeParticipantGate350B& gate_;
        bool allowed_;
        bool durable_=true;
    };

    // Track a deferred legacy save from enqueue until the last byte was
    // written. While queued, it is unsafe to start a two-DAT transaction.
    // participant==0 means the queued item lacked a trusted account/name;
    // then ALL trades fail closed until that unknown write is finished.
    bool BeginQueuedWrite(std::uint64_t participant) {
        std::lock_guard<std::mutex> guard(mutex_);
        if(!participant) {
            if(!active_.empty()) return false;
            ++unknownQueued_;
            return true;
        }
        if(Blocked(participant)) return false;
        ++queued_[participant];
        return true;
    }
    bool FinishQueuedWrite(std::uint64_t participant, bool persisted) {
        std::lock_guard<std::mutex> guard(mutex_);
        if(!participant) {
            if(!unknownQueued_) return false;
            --unknownQueued_;
            if(!persisted) failedUnknownWrite_=true;
            return true;
        }
        auto it=queued_.find(participant);
        if(it==queued_.end() || !it->second) return false;
        if(!--it->second) queued_.erase(it);
        if(!persisted) failedQueued_.insert(participant);
        return true;
    }

    // SUMMIT350K: report the precise reason atomically under the SAME gate
    // mutex as the reservation. Diagnostics never contain names or paths.
    // Numeric values are stable for trade admission console diagnosis.
    enum class ReservationBlock350K : int {
        None=0, InvalidPair=1, UnknownQueued=2, UnknownFailed=3,
        OwnerFailed=4, CompanionFailed=5, OwnerQueued=6, CompanionQueued=7,
        OwnerReserved=8, CompanionReserved=9, OwnerLoading=10,
        CompanionLoading=11, OwnerWriting=12, CompanionWriting=13
    };
    bool TryReserve(std::uint64_t owner,std::uint64_t companion,
                    ReservationBlock350K* diagnostic=nullptr) {
        if(diagnostic) *diagnostic=ReservationBlock350K::None;
        const auto deny=[diagnostic](ReservationBlock350K why)->bool {
            if(diagnostic) *diagnostic=why;
            return false;
        };
        if(!owner || !companion || owner==companion)
            return deny(ReservationBlock350K::InvalidPair);
        std::lock_guard<std::mutex> guard(mutex_);
        if(unknownQueued_) return deny(ReservationBlock350K::UnknownQueued);
        if(failedUnknownWrite_) return deny(ReservationBlock350K::UnknownFailed);
        if(failedQueued_.count(owner)) return deny(ReservationBlock350K::OwnerFailed);
        if(failedQueued_.count(companion)) return deny(ReservationBlock350K::CompanionFailed);
        if(queued_.count(owner)) return deny(ReservationBlock350K::OwnerQueued);
        if(queued_.count(companion)) return deny(ReservationBlock350K::CompanionQueued);
        if(Blocked(owner)) return deny(ReservationBlock350K::OwnerReserved);
        if(Blocked(companion)) return deny(ReservationBlock350K::CompanionReserved);
        if(loading_.count(owner)) return deny(ReservationBlock350K::OwnerLoading);
        if(loading_.count(companion)) return deny(ReservationBlock350K::CompanionLoading);
        if(writing_.count(owner)) return deny(ReservationBlock350K::OwnerWriting);
        if(writing_.count(companion)) return deny(ReservationBlock350K::CompanionWriting);
        active_.insert(Pair(owner,companion));
        return true;
    }

    // A successful DAT write is NOT a failed save merely because its optional
    // legacy pre-write backup could not be copied. Failing that backup is
    // worth logging, but cannot justify permanent session quarantine when
    // the real DAT fwrite+fclose both succeeded.
    static bool CompletedDatWrite350K(bool datWriteSucceeded,bool closeSucceeded,
                                      bool /*legacyBackupSucceeded*/) {
        return datWriteSucceeded && closeSucceeded;
    }
    // Only after a provably pre-intent refusal, or a checked durable receipt
    // and quarantine of the OLD player session. Not on IO uncertainty.
    bool ReleaseVerified(std::uint64_t owner,std::uint64_t companion) {
        std::lock_guard<std::mutex> guard(mutex_);
        return active_.erase(Pair(owner,companion))==1;
    }
    bool IsReserved(std::uint64_t participant) const {
        std::lock_guard<std::mutex> guard(mutex_);
        return Blocked(participant);
    }
private:
    bool BeginWrite(std::uint64_t participant) {
        if(!participant) return false;
        std::lock_guard<std::mutex> guard(mutex_);
        if(Blocked(participant)) return false;
        ++writing_[participant];
        return true;
    }
    void EndWrite(std::uint64_t participant) {
        std::lock_guard<std::mutex> guard(mutex_);
        auto it=writing_.find(participant);
        if(it!=writing_.end() && !--it->second) writing_.erase(it);
    }
    bool BeginLoad(std::uint64_t participant) {
        if(!participant) return false;
        std::lock_guard<std::mutex> guard(mutex_);
        if(Blocked(participant)) return false;
        ++loading_[participant];
        return true;
    }
    void EndLoad(std::uint64_t participant) {
        std::lock_guard<std::mutex> guard(mutex_);
        auto it=loading_.find(participant);
        if(it!=loading_.end() && !--it->second) loading_.erase(it);
    }
    bool Blocked(std::uint64_t participant) const {
        for(const auto& pair:active_)
            if(pair.first==participant || pair.second==participant) return true;
        return false;
    }
    mutable std::mutex mutex_;
    std::set<Pair> active_;
    std::map<std::uint64_t,unsigned> loading_;
    std::map<std::uint64_t,unsigned> writing_;
    std::map<std::uint64_t,unsigned> queued_;
    std::set<std::uint64_t> failedQueued_;
    unsigned unknownQueued_=0;
    bool failedUnknownWrite_=false;
};

inline TradeParticipantGate350B& ServerTradeParticipantGate350B() {
    static TradeParticipantGate350B singleton;
    return singleton;
}
}}}
