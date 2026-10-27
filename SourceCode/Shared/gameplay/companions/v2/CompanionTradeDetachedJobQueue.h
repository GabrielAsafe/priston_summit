#pragma once
// SUMMIT 350A: session-free bounded ADMISSION MAILBOX, NOT A TRADE ENGINE.
//
// A caller may enqueue only AFTER validating a live, authenticated account,
// offline companion, immutable server-side save snapshot and request identity
// under the world/session lock. The job owns every value. No rsPLAYINFO*,
// socket or incoming packet-buffer reference may cross this boundary.
//
// This class performs ZERO persistence, sends ZERO packets and launches ZERO
// threads. Future integration must supply: an owned/joined worker, exclusive
// lease, account-level login gate, exact buffer reconciliation, durable journal,
// and delivery revalidation under cSerSection. It is deliberately not wired
// into the 350A production request handler, which refuses transactions.
#include "gameplay/companions/v2/CompanionTradeConfirmPolicy.h"
#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace gameplay { namespace companions { namespace v2 {

struct DetachedTradeSession {
    std::uint32_t serial = 0;
    std::uint32_t generation = 0;
    // Server-authenticated, non-zero account and participant IDs.
    std::uint64_t account = 0;
    std::uint64_t owner = 0;
    std::uint64_t companion = 0;
    bool Valid() const {
        return serial && account && owner && companion && owner != companion;
    }
    bool SameConnection(const DetachedTradeSession& rhs) const {
        return serial == rhs.serial && generation == rhs.generation &&
               account == rhs.account && owner == rhs.owner;
    }
};

struct DetachedTradeJob {
    DetachedTradeSession session;
    // An owned, by-value copy; never a packet-buffer reference.
    protocol::CompanionTradeConfirmPacket request{};
    std::uint64_t acceptedSaveSequence = 0;
    std::uint64_t leaseToken = 0;
    std::uint64_t tradeNonce = 0;
    // Capture only from server-authenticated admission while under the world
    // lock; verified immutable accepted save, not network-supplied item bytes.
    std::string accountName, ownerName, companionName;
    std::vector<std::uint8_t> acceptedOwnerRecord;
    // SUMMIT350H: strictly server-derived paths; never provided by the wire.
    // The pair gate and session lease must be acquired before enqueue.
    std::string trustedOwnerPath, trustedCompanionPath;
    // The runtime may still decline admission until it proves all gates.

    // A future persistence worker must also receive an independently verified,
    // immutable normalized DAT image and trusted paths. Those are NOT provided
    // by this prototype. Do not commit using this structure alone.
};

enum class DetachedAdmission {
    Accepted, Invalid, BusyPair, BusyCharacter, Full, Closed
};

class DetachedTradeJobQueue final {
public:
    explicit DetachedTradeJobQueue(std::size_t capacity = 8)
        : capacity_(capacity ? capacity : 1) {}
    DetachedTradeJobQueue(const DetachedTradeJobQueue&) = delete;
    DetachedTradeJobQueue& operator=(const DetachedTradeJobQueue&) = delete;

    DetachedAdmission Enqueue(const DetachedTradeJob& job) {
        if(!job.session.Valid() || !job.acceptedSaveSequence ||
           !ValidTradeConfirmRequest(job.request)) return DetachedAdmission::Invalid;
        std::lock_guard<std::mutex> lock(mutex_);
        if(closed_) return DetachedAdmission::Closed;
        const Pair pair(job.session.owner, job.session.companion);
        if(active_.find(pair) != active_.end()) return DetachedAdmission::BusyPair;
        for(const auto& existing : active_) {
            if(existing.second.session.account == job.session.account &&
               (existing.second.session.owner == job.session.owner ||
                existing.second.session.owner == job.session.companion ||
                existing.second.session.companion == job.session.owner ||
                existing.second.session.companion == job.session.companion))
                return DetachedAdmission::BusyCharacter;
        }
        if(active_.size() >= capacity_) return DetachedAdmission::Full;
        active_.emplace(pair, job); // holds reservation across queued + running
        queue_.push_back(pair);
        return DetachedAdmission::Accepted;
    }

    // Called by one controlled worker outside the world lock; still retains
    // the admission reservation until Complete(). Nonblocking when empty.
    bool TryNext(DetachedTradeJob& out) {
        std::lock_guard<std::mutex> lock(mutex_);
        if(queue_.empty()) return false;
        const Pair pair = queue_.front();
        queue_.pop_front();
        const auto it = active_.find(pair);
        if(it == active_.end()) return false;
        out = it->second;
        return true;
    }

    // Worker gives completion back without touching any legacy pointer.
    // A terminal result is retained pending main-thread session validation.
    // During uncertain commit the status MUST be -74, never a false failure.
    bool Complete(const DetachedTradeJob& job, int status) {
        std::lock_guard<std::mutex> lock(mutex_);
        const Pair pair(job.session.owner, job.session.companion);
        const auto it=active_.find(pair);
        if(it==active_.end() ||
           !it->second.session.SameConnection(job.session) ||
           it->second.session.companion != job.session.companion ||
           it->second.request.requestId != job.request.requestId ||
           completed_.find(pair) != completed_.end()) return false;
        completed_.emplace(pair, Completion{it->second, status});
        if(MayHaveDurableChange(status)) durableHold_.insert(pair);
        return true;
    }

    // Run only with an independently resolved, authenticated LIVE session,
    // after reacquiring cSerSection. Do not match by socket pointer alone.
    // Completion is removed by world-thread pump; its caller MUST independently
    // resolve session serial, generation, account and both characters.
    bool TryTakeCompletion(DetachedTradeJob& job, int& status) {
        std::lock_guard<std::mutex> lock(mutex_);
        if(completed_.empty()) return false;
        auto it=completed_.begin();
        job=it->second.job;
        status=it->second.status;
        if(!MayHaveDurableChange(status)) active_.erase(it->first);
        completed_.erase(it);
        return true;
    }
    bool TakeForSession(const DetachedTradeSession& session,
                        std::uint64_t requestId, int& status) {
        std::lock_guard<std::mutex> lock(mutex_);
        for(auto it=completed_.begin();it!=completed_.end();++it) {
            if(it->second.job.session.SameConnection(session) &&
               it->second.job.session.companion == session.companion &&
               it->second.job.request.requestId == requestId) {
                status=it->second.status;
                // Uncertain or committed outcomes MUST retain the pair gate
                // until external journal verification on a safe login path.
                if(!MayHaveDurableChange(status)) active_.erase(it->first);
                completed_.erase(it);
                return true;
            }
        }
        return false;
    }

    // Prevent delivering to a recycled connection. An incomplete, possibly
    // durable operation remains reserved for journal recovery, not cancelled.
    void DiscardFinishedForOldSession(const DetachedTradeSession& old) {
        std::lock_guard<std::mutex> lock(mutex_);
        for(auto it=completed_.begin();it!=completed_.end();) {
            if(it->second.job.session.SameConnection(old)) {
                if(!MayHaveDurableChange(it->second.status)) active_.erase(it->first);
                it=completed_.erase(it);
            } else ++it;
        }
    }

    // ONLY AFTER journal recovery, both record image checks and the
    // account-level login gate have succeeded. Not called in 350A runtime.
    bool ReleaseAfterVerifiedRecovery(std::uint64_t owner,
                                      std::uint64_t companion) {
        std::lock_guard<std::mutex> lock(mutex_);
        const Pair pair(owner,companion);
        if(completed_.find(pair)!=completed_.end()) return false;
        const auto it=active_.find(pair);
        if(it==active_.end()) return false;
        // A running or queued job must NEVER be released by this method.
        if(durableHold_.erase(pair)!=1) return false;
        return active_.erase(pair)==1;
    }

    void CloseAdmission() {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_=true;
    }
    std::size_t QueuedCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
    std::size_t Outstanding() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return active_.size();
    }
private:
    using Pair=std::pair<std::uint64_t,std::uint64_t>;
    static bool MayHaveDurableChange(int status) {
        return status==-74 || status==-75 || status==1 || status==2 || status==3;
    }
    struct Completion { DetachedTradeJob job; int status; };
    mutable std::mutex mutex_;
    std::size_t capacity_;
    bool closed_=false;
    std::deque<Pair> queue_;
    std::map<Pair,DetachedTradeJob> active_;
    std::map<Pair,Completion> completed_;
    std::set<Pair> durableHold_;
};

}}} // namespaces
