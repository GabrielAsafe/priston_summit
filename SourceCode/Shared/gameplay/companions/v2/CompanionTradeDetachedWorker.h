#pragma once
// SUMMIT 350B: fully owned, bounded, JOINED background coordinator.
// Infrastructure only: NEVER enables a live trade on its own.
// No rsPLAYINFO, socket, pointers into network buffers, or Win32 I/O here.
// The caller must separately prove a trusted complete character snapshot,
// fence legacy saves, gate logins for BOTH participants (including restart),
// and handle pending durable journal recovery BEFORE using a real executor.
#include "gameplay/companions/v2/CompanionTradeDetachedJobQueue.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>
#include <stdexcept>

namespace gameplay { namespace companions { namespace v2 {

// A terminal result is only non-durable if no intent was ever published.
// Never classify IO uncertainty as a normal rejection.
struct DetachedTradeOutcome {
    int status;
    bool durableOrUncertain;
    DetachedTradeOutcome(int code = -74, bool possiblyDurable = true)
        : status(code), durableOrUncertain(possiblyDurable) {}
    static DetachedTradeOutcome Refused(int code) {
        return DetachedTradeOutcome(code < 0 && code != -74 && code != -75 ? code : -76, false);
    }
    static DetachedTradeOutcome Committed(bool recovered = false) {
        return DetachedTradeOutcome(recovered ? 3 : 2, true);
    }
    static DetachedTradeOutcome Unknown() { return DetachedTradeOutcome(-74, true); }
};

// The only implementation allowed on the worker thread. No legacy globals or
// live-session lookups. Any implementation must keep a durable pair/login gate
// while the intent can still be recovered. Execute MAY block: never called
// while the caller holds cSerSection.
class IDetachedTradeExecutor {
public:
    virtual ~IDetachedTradeExecutor() {}
    virtual DetachedTradeOutcome Execute(const DetachedTradeJob& immutableJob) = 0;
};

// One owned worker per server; no detached threads, no unsafe shutdown races.
// Accepted jobs are drained on Stop(). The caller must stop it *outside*
// cSerSection, after new network admission has ceased. Results remain in the
// mailbox until consumed by the session-validated main thread.
class DetachedTradeWorker final {
public:
    explicit DetachedTradeWorker(IDetachedTradeExecutor& executor, std::size_t capacity = 8)
        : executor_(executor), queue_(capacity) {}
    DetachedTradeWorker(const DetachedTradeWorker&) = delete;
    DetachedTradeWorker& operator=(const DetachedTradeWorker&) = delete;
    ~DetachedTradeWorker() { Stop(); }

    // Start only at a server lifecycle phase in which thread creation is safe.
    // Not lazily under the global world lock.
    bool Start() {
        std::lock_guard<std::mutex> guard(wakeMutex_);
        if(running_ || stopping_) return false;
        try {
            worker_ = std::thread(&DetachedTradeWorker::Run, this);
            running_ = true;
            return true;
        } catch(...) { return false; }
    }

    DetachedAdmission Enqueue(const DetachedTradeJob& job) {
        {
            std::lock_guard<std::mutex> guard(wakeMutex_);
            if(!running_ || stopping_) return DetachedAdmission::Closed;
        }
        DetachedAdmission admitted = queue_.Enqueue(job);
        if(admitted == DetachedAdmission::Accepted) wake_.notify_one();
        return admitted;
    }

    bool TryTakeCompletion(DetachedTradeJob& job, int& status) {
        return queue_.TryTakeCompletion(job,status);
    }
    bool TakeForSession(const DetachedTradeSession& session,
                        std::uint64_t requestId, int& status) {
        return queue_.TakeForSession(session, requestId, status);
    }
    void DiscardFinishedForOldSession(const DetachedTradeSession& old) {
        queue_.DiscardFinishedForOldSession(old);
    }
    bool ReleaseAfterVerifiedRecovery(std::uint64_t owner, std::uint64_t companion) {
        return queue_.ReleaseAfterVerifiedRecovery(owner, companion);
    }
    std::size_t Outstanding() const { return queue_.Outstanding(); }

    void Stop() {
        std::thread joinable;
        {
            std::lock_guard<std::mutex> guard(wakeMutex_);
            if(!running_) return;
            stopping_ = true;
            queue_.CloseAdmission();
            joinable = std::move(worker_);
        }
        wake_.notify_all();
        if(joinable.joinable()) joinable.join();
        {
            std::lock_guard<std::mutex> guard(wakeMutex_);
            running_ = false;
        }
    }

private:
    void Run() {
        while(true) {
            DetachedTradeJob job;
            if(queue_.TryNext(job)) {
                DetachedTradeOutcome result = DetachedTradeOutcome::Unknown();
                try { result = executor_.Execute(job); }
                catch(...) { result = DetachedTradeOutcome::Unknown(); }
                // A custom executor is never allowed to call a possibly
                // durable change a safe rejection. Explicit flag wins.
                int status = result.durableOrUncertain &&
                    result.status != 2 && result.status != 3 ? -74 : result.status;
                if(status == 0 || status == 1 || status == -75) status = -74;
                (void)queue_.Complete(job, status);
                continue;
            }
            std::unique_lock<std::mutex> lock(wakeMutex_);
            if(stopping_ && queue_.QueuedCount() == 0) break;
            // A notification between TryNext and this wait is safe: the
            // predicate observes queue state under its own mutex.
            wake_.wait(lock, [this] { return stopping_ || queue_.QueuedCount() != 0; });
        }
    }
    IDetachedTradeExecutor& executor_;
    DetachedTradeJobQueue queue_;
    mutable std::mutex wakeMutex_;
    std::condition_variable wake_;
    std::thread worker_;
    bool running_ = false;
    bool stopping_ = false;
};

}}} // gameplay::companions::v2
